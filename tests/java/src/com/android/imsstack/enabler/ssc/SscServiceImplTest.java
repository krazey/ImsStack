/*
 * Copyright (C) 2022 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

package com.android.imsstack.enabler.ssc;

import static android.telephony.ims.feature.CapabilityChangeRequest.CapabilityPair;

import static com.android.imsstack.base.ImsPrivateProperties.Persistent.KEY_WIFI_TEST;

import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertFalse;
import static org.junit.Assert.assertNotEquals;
import static org.junit.Assert.assertNotNull;
import static org.junit.Assert.assertNull;
import static org.junit.Assert.assertTrue;
import static org.mockito.ArgumentMatchers.any;
import static org.mockito.ArgumentMatchers.anyInt;
import static org.mockito.ArgumentMatchers.anyString;
import static org.mockito.ArgumentMatchers.eq;
import static org.mockito.Mockito.atLeast;
import static org.mockito.Mockito.never;
import static org.mockito.Mockito.reset;
import static org.mockito.Mockito.verify;
import static org.mockito.Mockito.verifyNoMoreInteractions;
import static org.mockito.Mockito.when;

import android.content.Context;
import android.content.SharedPreferences;
import android.os.Handler;
import android.os.HandlerThread;
import android.os.Message;
import android.telephony.CarrierConfigManager;
import android.telephony.NetworkRegistrationInfo;
import android.telephony.ims.ImsCallForwardInfo;
import android.telephony.ims.ImsReasonInfo;
import android.telephony.ims.ImsSsInfo;
import android.testing.AndroidTestingRunner;
import android.testing.TestableLooper;
import android.text.TextUtils;

import com.android.imsstack.ContextFixture;
import com.android.imsstack.base.AppContext;
import com.android.imsstack.core.agents.AgentFactory;
import com.android.imsstack.core.agents.ConfigInterface;
import com.android.imsstack.core.agents.PhoneStateInterface;
import com.android.imsstack.core.agents.dcmif.EApnType;
import com.android.imsstack.core.config.CarrierConfig;
import com.android.imsstack.enabler.ssc.data.ErrorResponseData;
import com.android.imsstack.enabler.ssc.data.SscRequestResult;
import com.android.imsstack.enabler.ssc.data.SscServiceData;
import com.android.imsstack.enabler.ssc.data.SscServiceQueryData;
import com.android.imsstack.imsservice.mmtel.ut.base.IUtInterface;
import com.android.imsstack.imsservice.mmtel.ut.base.IUtInterface.CbData;
import com.android.imsstack.imsservice.mmtel.ut.base.IUtInterface.OipData;
import com.android.imsstack.imsservice.mmtel.ut.base.IUtInterface.OirData;
import com.android.imsstack.imsservice.mmtel.ut.base.IUtInterface.SupplementaryServiceConfiguration;
import com.android.imsstack.imsservice.mmtel.ut.base.IUtInterface.TipData;
import com.android.imsstack.imsservice.mmtel.ut.base.IUtInterface.TirData;
import com.android.imsstack.imsservice.mmtel.ut.base.IUtListener;
import com.android.imsstack.imsservice.mmtel.ut.base.IUtServiceStateListener;

import org.junit.After;
import org.junit.Before;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.mockito.ArgumentCaptor;
import org.mockito.Captor;
import org.mockito.Mock;
import org.mockito.MockitoAnnotations;
import org.w3c.dom.Document;
import org.xml.sax.InputSource;

import java.io.StringReader;
import java.util.ArrayList;
import java.util.List;

import javax.xml.parsers.DocumentBuilder;
import javax.xml.parsers.DocumentBuilderFactory;

@RunWith(AndroidTestingRunner.class)
@TestableLooper.RunWithLooper
public class SscServiceImplTest {
    private final static int SLOT_0 = 0;

    private SscServiceImpl mSscServiceImpl;

    private Context mContext;
    private int mQueryCount; // increase after every startGetTransaction case
    private int mUpdateCount; // increase after every startPutTransaction case
    private Handler mCallbackHandler;
    private TestableLooper mLooper;

    @Mock private CarrierConfig mMockCarrierConfig;
    @Mock private ConfigInterface mMockConfigInterface;
    @Mock private SharedPreferences mMockSharedPreferences;
    @Mock private SscServiceState mMockSscServiceState;
    @Mock private SscPreferenceHelper mMockSscPreferenceHelper;
    @Mock private SscTransactionFactory mMockSscTransactionFactory;
    @Mock private SscTransaction mMockSscTransaction;
    @Mock private IUtListener mMockUtListener;
    @Mock private IUtServiceStateListener mMockUtServiceStateListener;
    @Mock private PhoneStateInterface mMockPhoneStateInterface;
    @Mock private IUtInterface.TerminalBasedSupplementaryServiceConfigurationChangeListener
            mMockTbSscChangeListener;

    @Captor ArgumentCaptor<SscServiceQueryData> captorQueryData;
    @Captor ArgumentCaptor<SscServiceData> captorUpdateData;
    @Captor ArgumentCaptor<ImsCallForwardInfo[]> captorCfInfos;
    @Captor ArgumentCaptor<ImsReasonInfo> captorReasonInfo;
    @Captor ArgumentCaptor<ImsSsInfo> captorSsInfo;
    @Captor ArgumentCaptor<ImsSsInfo[]> captorSsInfos;
    @Captor ArgumentCaptor<List<SupplementaryServiceConfiguration>> mCaptorSscs;

    // test configurations
    private boolean mIsCfRuleSetExist = true;
    private boolean mIsIcbRulesExist = true;
    private boolean mIsCfnlRuleExist = false;
    private boolean mIsCfbRuleExist = true;
    private boolean mIsTimerInCfnr = false;
    private boolean mIsCbRuleActivated = false;
    private boolean mIsCfRuleActivated = false;
    private boolean mIsCwEnabled = false;
    private boolean mIsOipEnabled = false;
    private boolean mIsTipEnabled = false;
    private String mErrorPhrase = "";
    private String mForwardNumber = "tel:+1234567890";
    private String mDefaultBehaviour = SscXmlFormat.PRESENTATION_NOT_RESTRICTED;
    private int mHttpSuccessResponse = SscConstant.HTTP_OK;
    private int mHttpErrorResponse = SscConstant.HTTP_CONFLICT;
    private int[] mServerBasedServices = {
            CarrierConfigManager.ImsSs.SUPPLEMENTARY_SERVICE_CW,
            CarrierConfigManager.ImsSs.SUPPLEMENTARY_SERVICE_CF_ALL,
            CarrierConfigManager.ImsSs.SUPPLEMENTARY_SERVICE_CF_CFU,
            CarrierConfigManager.ImsSs.SUPPLEMENTARY_SERVICE_CF_ALL_CONDITONAL_FORWARDING,
            CarrierConfigManager.ImsSs.SUPPLEMENTARY_SERVICE_CF_CFB,
            CarrierConfigManager.ImsSs.SUPPLEMENTARY_SERVICE_CF_CFNRY,
            CarrierConfigManager.ImsSs.SUPPLEMENTARY_SERVICE_CF_CFNRC,
            CarrierConfigManager.ImsSs.SUPPLEMENTARY_SERVICE_CF_CFNL,
            CarrierConfigManager.ImsSs.SUPPLEMENTARY_SERVICE_IDENTIFICATION_OIP,
            CarrierConfigManager.ImsSs.SUPPLEMENTARY_SERVICE_IDENTIFICATION_TIP,
            CarrierConfigManager.ImsSs.SUPPLEMENTARY_SERVICE_IDENTIFICATION_OIR,
            CarrierConfigManager.ImsSs.SUPPLEMENTARY_SERVICE_IDENTIFICATION_TIR,
            CarrierConfigManager.ImsSs.SUPPLEMENTARY_SERVICE_CB_BAOC,
            CarrierConfigManager.ImsSs.SUPPLEMENTARY_SERVICE_CB_BOIC,
            CarrierConfigManager.ImsSs.SUPPLEMENTARY_SERVICE_CB_BOIC_EXHC,
            CarrierConfigManager.ImsSs.SUPPLEMENTARY_SERVICE_CB_BAIC,
            CarrierConfigManager.ImsSs.SUPPLEMENTARY_SERVICE_CB_BIC_ROAM,
            CarrierConfigManager.ImsSs.SUPPLEMENTARY_SERVICE_CB_ACR
    };

    @Before
    public void setup() throws Exception {
        MockitoAnnotations.initMocks(this);

        mContext = new ContextFixture().getTestDouble();
        AppContext.init(mContext);

        AgentFactory.getInstance()
                .setAgent(PhoneStateInterface.class, mMockPhoneStateInterface, SLOT_0);

        mQueryCount = 1;
        mUpdateCount = 1;
        mSscServiceImpl = new SscServiceImpl(SLOT_0);
        mSscServiceImpl.start(mContext);
        mSscServiceImpl.setListener(mMockUtListener);
        mSscServiceImpl.setSscTransactionFactory(mMockSscTransactionFactory);

        // mMockConfigInterface should be set after starting SscServiceImpl
        SscConfig.setConfigInterface(SLOT_0, mMockConfigInterface);
        when(mMockConfigInterface.getCarrierConfig()).thenReturn(mMockCarrierConfig);
        when(mMockCarrierConfig.getIntArray(
            CarrierConfigManager.ImsSs.KEY_UT_SERVER_BASED_SERVICES_INT_ARRAY))
            .thenReturn(mServerBasedServices);
        when(mMockCarrierConfig.getBoolean(eq(CarrierConfig.ImsSs.KEY_UT_SUPPORT_CFNR_TIMER_BOOL)))
                .thenReturn(true);

        when(mContext.getSharedPreferences(anyString(), anyInt()))
                .thenReturn(mMockSharedPreferences);
        when(mMockSharedPreferences.getString(eq(KEY_WIFI_TEST), anyString()))
                .thenReturn(String.valueOf(0));

        HandlerThread serviceThreadHandler = mSscServiceImpl.getServiceHandlerThread();
        mLooper = new TestableLooper(serviceThreadHandler.getLooper());

        mCallbackHandler = mSscServiceImpl.getCallBackHandler();
        when(mMockSscTransactionFactory.getSscTransaction(eq(SLOT_0), any()))
                .thenReturn(mMockSscTransaction);

        SscServiceStateAgent.getInstance().setSscServiceState(SLOT_0, mMockSscServiceState);
        when(mMockSscServiceState.isUtAvailable()).thenReturn(true);
    }

    @After
    public void tearDown() {
        mSscServiceImpl.close();
        mLooper.destroy();
        SscConfig.clear(SLOT_0);

        AgentFactory.getInstance().setAgent(PhoneStateInterface.class, null, SLOT_0);

        AppContext.deinit();
    }

    @Test
    public void testInitConnection_wifiTestOn() {
        when(mMockSharedPreferences.getString(eq(KEY_WIFI_TEST), anyString()))
                .thenReturn(String.valueOf(1));

        mSscServiceImpl.start(mContext);

        ISscNetConnection netConn = ((SscNetConnectionGov) SscNetConnectionGov.getInstance())
                .getSscNetConnection(SLOT_0);
        assertEquals(EApnType.WIFI, ((SscNetConnection) netConn).mApnType);
    }

    @Test
    public void testChangeCapabilities() {
        ArrayList<CapabilityPair> enabledCaps = new ArrayList<CapabilityPair>();
        ArrayList<CapabilityPair> disabledCaps = new ArrayList<CapabilityPair>();

        mSscServiceImpl.changeCapabilities(enabledCaps, disabledCaps);

        verify(mMockSscServiceState).changeCapabilities(enabledCaps, disabledCaps);
    }

    @Test
    public void testServiceStateChanged() {
        mSscServiceImpl.setServiceStateListener(mMockUtServiceStateListener);
        mSscServiceImpl.onServiceStateChanged();

        verify(mMockUtServiceStateListener).onServiceStateChanged();
    }

    @Test
    public void testIsUtAvailable_utNotAvailableAndTbOirNotEnabledAndCsfbSupported_returnFalse() {
        when(mMockCarrierConfig.getBoolean(
                CarrierConfigManager.KEY_CARRIER_SUPPORTS_SS_OVER_UT_BOOL)).thenReturn(true);
        when(mMockCarrierConfig.getIntArray(
                CarrierConfigManager.ImsSs.KEY_UT_TERMINAL_BASED_SERVICES_INT_ARRAY))
                .thenReturn(new int[] {});
        when(mMockCarrierConfig.getBoolean(
                eq(CarrierConfigManager.ImsSs.KEY_USE_CSFB_ON_XCAP_OVER_UT_FAILURE_BOOL)))
                .thenReturn(true);
        when(mMockSscServiceState.isUtAvailable()).thenReturn(false);

        boolean result = mSscServiceImpl.isUtAvailable();

        assertFalse(result);
    }

    @Test
    public void testIsUtAvailable_utNotAvailableButTbOirEnabled_returnTrue() {
        when(mMockCarrierConfig.getBoolean(
                CarrierConfigManager.KEY_CARRIER_SUPPORTS_SS_OVER_UT_BOOL)).thenReturn(true);
        when(mMockCarrierConfig.getIntArray(
                CarrierConfigManager.ImsSs.KEY_UT_TERMINAL_BASED_SERVICES_INT_ARRAY))
                .thenReturn(new int[] {
                        CarrierConfigManager.ImsSs.SUPPLEMENTARY_SERVICE_IDENTIFICATION_OIR});
        when(mMockCarrierConfig.getBoolean(
                eq(CarrierConfigManager.ImsSs.KEY_USE_CSFB_ON_XCAP_OVER_UT_FAILURE_BOOL)))
                .thenReturn(true);
        when(mMockSscServiceState.isUtAvailable()).thenReturn(false);

        boolean result = mSscServiceImpl.isUtAvailable();

        assertTrue(result);
    }

    @Test
    public void testIsUtAvailable_utNotAvailableButCsfbNotSupported_returnTrue() {
        when(mMockCarrierConfig.getBoolean(
                CarrierConfigManager.KEY_CARRIER_SUPPORTS_SS_OVER_UT_BOOL)).thenReturn(true);
        when(mMockCarrierConfig.getIntArray(
                CarrierConfigManager.ImsSs.KEY_UT_TERMINAL_BASED_SERVICES_INT_ARRAY))
                .thenReturn(new int[] {});
        when(mMockCarrierConfig.getBoolean(
                eq(CarrierConfigManager.ImsSs.KEY_USE_CSFB_ON_XCAP_OVER_UT_FAILURE_BOOL)))
                .thenReturn(false);
        when(mMockSscServiceState.isUtAvailable()).thenReturn(false);

        boolean result = mSscServiceImpl.isUtAvailable();

        assertTrue(result);
    }

    @Test
    public void testIsUtAvailable_tbOirEnabledAndCsfbNotSupportedButUtNotEnabled_returnFalse() {
        when(mMockCarrierConfig.getBoolean(
                CarrierConfigManager.KEY_CARRIER_SUPPORTS_SS_OVER_UT_BOOL)).thenReturn(false);
        when(mMockCarrierConfig.getIntArray(
                CarrierConfigManager.ImsSs.KEY_UT_TERMINAL_BASED_SERVICES_INT_ARRAY))
                .thenReturn(new int[] {
                        CarrierConfigManager.ImsSs.SUPPLEMENTARY_SERVICE_IDENTIFICATION_OIR});
        when(mMockCarrierConfig.getBoolean(
                eq(CarrierConfigManager.ImsSs.KEY_USE_CSFB_ON_XCAP_OVER_UT_FAILURE_BOOL)))
                .thenReturn(false);
        when(mMockSscServiceState.isUtAvailable()).thenReturn(false);

        boolean result = mSscServiceImpl.isUtAvailable();

        assertFalse(result);
    }

    @Test
    public void testBasicOperation_utNotAvailable() {
        when(mMockSscServiceState.isUtAvailable()).thenReturn(false);
        int tId = 1;

        mSscServiceImpl.queryCallForward(tId, SscConstant.CONDITION_CFU, "");

        mLooper.processAllMessages();
        verify(mMockUtListener).utConfigurationQueryFailed(eq(tId), any());
        verifyNoMoreInteractions(mMockSscTransaction);
    }

    @Test
    public void testBasicOperation_informErrorPhrase() {
        when(mMockCarrierConfig.getBoolean(
                eq(CarrierConfig.ImsSs.KEY_UT_DISPLAY_ERROR_PHRASE_WITH_409_ERROR_BOOL)))
                .thenReturn(true);
        mHttpErrorResponse = SscConstant.HTTP_CONFLICT;
        mErrorPhrase = "check error phrase";
        int tId = 1;

        mSscServiceImpl.queryCallForward(tId, SscConstant.CONDITION_CFU, "");
        processEntireXmlDocQueryAsFailure();

        mLooper.processAllMessages();
        verify(mMockUtListener).utConfigurationQueryFailed(eq(tId), captorReasonInfo.capture());

        ImsReasonInfo reasonInfo = captorReasonInfo.getValue();
        assertNotNull(reasonInfo);
        assertEquals(ImsReasonInfo.CODE_UT_NETWORK_ERROR, reasonInfo.getCode());
        assertEquals(mErrorPhrase, reasonInfo.getExtraMessage());

        verify(mMockSscTransaction).close();
        verifyNoMoreInteractions(mMockSscTransaction);
    }

    @Test
    public void testBasicOperation_ignoreErrorPhrase() {
        when(mMockCarrierConfig.getBoolean(
                eq(CarrierConfig.ImsSs.KEY_UT_DISPLAY_ERROR_PHRASE_WITH_409_ERROR_BOOL)))
                .thenReturn(false);
        mHttpErrorResponse = SscConstant.HTTP_CONFLICT;
        mErrorPhrase = "check error phrase";
        int tId = 1;

        mSscServiceImpl.queryCallForward(tId, SscConstant.CONDITION_CFU, "");
        processEntireXmlDocQueryAsFailure();

        mLooper.processAllMessages();
        verify(mMockUtListener).utConfigurationQueryFailed(eq(tId), captorReasonInfo.capture());

        ImsReasonInfo reasonInfo = captorReasonInfo.getValue();
        assertNotNull(reasonInfo);
        assertNull(reasonInfo.getExtraMessage());

        verify(mMockSscTransaction).close();
        verifyNoMoreInteractions(mMockSscTransaction);
    }

    @Test
    public void testBasicOperation_preconditionFailure() {
        mHttpErrorResponse = SscConstant.HTTP_PRECONDITION_FAILURE;
        int tId = 1;

        mSscServiceImpl.updateCallForward(tId, SscConstant.ACTION_ACTIVATION,
                SscConstant.CONDITION_CFB, null, 0, 20);

        // Entire Doc. query.
        processEntireXmlDocQueryAsSuccess();
        assertTrue(SscXmlGov.getInstance(SLOT_0).isXmlDataPresent());

        // Handle HTTP_PRECONDITION_FAILURE for PUT. Triggering the entire Doc. query again.
        processPutTransactionAsFailure(ESsType.CF, SscConstant.EVENT_SSC_UPDATE_CF,
                SscConstant.CONDITION_CFB);
        mLooper.processAllMessages();
        assertFalse(SscXmlGov.getInstance(SLOT_0).isXmlDataPresent());

        // 2nd entire Doc. query because of 412.
        processEntireXmlDocQueryAsSuccess();
        mLooper.processAllMessages();
        assertTrue(SscXmlGov.getInstance(SLOT_0).isXmlDataPresent());

        // 2nd handling HTTP_PRECONDITION_FAILURE for PUT. Considering it as a final failure.
        processPutTransactionAsFailure(ESsType.CF, SscConstant.EVENT_SSC_UPDATE_CF,
                SscConstant.CONDITION_CFB);
        mLooper.processAllMessages();
        assertFalse(SscXmlGov.getInstance(SLOT_0).isXmlDataPresent());

        verify(mMockUtListener).utConfigurationUpdateFailed(eq(tId), any());
        verify(mMockSscTransaction).close();
        verifyNoMoreInteractions(mMockSscTransaction);
    }

    @Test
    public void testBasicOperation_errorReasonCsfb() {
        when(mMockCarrierConfig.getBoolean(
                eq(CarrierConfigManager.ImsSs.KEY_USE_CSFB_ON_XCAP_OVER_UT_FAILURE_BOOL)))
                .thenReturn(true);
        mHttpErrorResponse = SscConstant.HTTP_CONFLICT;
        int tId = 1;

        mSscServiceImpl.queryCallForward(tId, SscConstant.CONDITION_CFU, "");
        processEntireXmlDocQueryAsFailure();

        mLooper.processAllMessages();
        verify(mMockUtListener).utConfigurationQueryFailed(eq(tId), captorReasonInfo.capture());

        ImsReasonInfo reasonInfo = captorReasonInfo.getValue();
        assertNotNull(reasonInfo);
        assertEquals(ImsReasonInfo.CODE_LOCAL_CALL_CS_RETRY_REQUIRED, reasonInfo.getCode());
        assertNotEquals(reasonInfo.getExtraMessage(), mErrorPhrase);

        verify(mMockSscTransaction).close();
        verifyNoMoreInteractions(mMockSscTransaction);
    }

    @Test
    public void testQueryCallBarringForServiceClass_singleRequestIcb() {
        mIsCbRuleActivated = false;
        int queryCondition = SscConstant.CONDITION_BIC_WR;
        int tId = 1;

        mSscServiceImpl.queryCallBarringForServiceClass(tId, queryCondition,
                SscServiceClassUtil.SERVICE_CLASS_VOICE);
        processEntireXmlDocQueryAsSuccess();
        processGetTransactionAsSuccess(ESsType.ICB, SscConstant.EVENT_SSC_QUERY_CB,
                queryCondition);

        mLooper.processAllMessages();
        verify(mMockUtListener).utConfigurationCallBarringQueried(eq(tId), captorSsInfos.capture());

        ImsSsInfo[] cbInfos = captorSsInfos.getValue();
        assertNotNull(cbInfos);

        int xmlStatus = mIsCbRuleActivated ? SscConstant.STATUS_ENABLE : SscConstant.STATUS_DISABLE;
        assertEquals(cbInfos[0].mStatus, xmlStatus);

        verify(mMockSscTransaction).close();
        verifyNoMoreInteractions(mMockSscTransaction);
    }

    @Test
    public void testQueryCallBarringForServiceClass_singleRequestOcb() {
        mIsCbRuleActivated = true;
        int queryCondition = SscConstant.CONDITION_BAOC;
        int tId = 1;

        mSscServiceImpl.queryCallBarringForServiceClass(tId, queryCondition,
                SscServiceClassUtil.SERVICE_CLASS_VOICE);
        processEntireXmlDocQueryAsSuccess();
        processGetTransactionAsSuccess(ESsType.OCB, SscConstant.EVENT_SSC_QUERY_CB,
                queryCondition);

        mLooper.processAllMessages();
        verify(mMockUtListener).utConfigurationCallBarringQueried(eq(tId), captorSsInfos.capture());

        ImsSsInfo[] cbInfos = captorSsInfos.getValue();
        assertNotNull(cbInfos);

        int xmlStatus = mIsCbRuleActivated ? SscConstant.STATUS_ENABLE : SscConstant.STATUS_DISABLE;
        assertEquals(cbInfos[0].mStatus, xmlStatus);

        verify(mMockSscTransaction).close();
        verifyNoMoreInteractions(mMockSscTransaction);
    }

    @Test
    public void testQueryCallBarringForServiceClass_notSupportedService() {
        when(mMockCarrierConfig.getIntArray(
                CarrierConfigManager.ImsSs.KEY_UT_SERVER_BASED_SERVICES_INT_ARRAY))
                .thenReturn(new int[] {});
        int tId = 1;

        mSscServiceImpl.queryCallBarringForServiceClass(tId, SscConstant.CONDITION_BIC_WR,
                SscServiceClassUtil.SERVICE_CLASS_VOICE);

        mLooper.processAllMessages();
        verify(mMockUtListener).utConfigurationQueryFailed(eq(tId), captorReasonInfo.capture());
        verifyNoMoreInteractions(mMockSscTransaction);

        ImsReasonInfo reasonInfo = captorReasonInfo.getValue();
        assertNotNull(reasonInfo);
        assertEquals(ImsReasonInfo.CODE_UT_OPERATION_NOT_ALLOWED, reasonInfo.getCode());
    }

    @Test
    public void testQueryCallBarringForServiceClass_entireDocumentQueryFailure() {
        int tId = 1;

        mSscServiceImpl.queryCallBarringForServiceClass(tId, SscConstant.CONDITION_BIC_WR,
                SscServiceClassUtil.SERVICE_CLASS_VOICE);
        processEntireXmlDocQueryAsFailure();

        mLooper.processAllMessages();
        verify(mMockUtListener).utConfigurationQueryFailed(eq(tId), any());
        verify(mMockSscTransaction).close();
        verifyNoMoreInteractions(mMockSscTransaction);
    }

    @Test
    public void testQueryCallBarringForServiceClass_queryFailure() {
        int tId = 1;

        mSscServiceImpl.queryCallBarringForServiceClass(tId, SscConstant.CONDITION_BOIC,
                SscServiceClassUtil.SERVICE_CLASS_VOICE);
        processEntireXmlDocQueryAsSuccess();
        processGetTransactionAsFailure(ESsType.OCB, SscConstant.EVENT_SSC_QUERY_CB,
                SscConstant.CONDITION_BOIC);

        mLooper.processAllMessages();
        verify(mMockUtListener).utConfigurationQueryFailed(eq(tId), any());
        verify(mMockSscTransaction).close();
        verifyNoMoreInteractions(mMockSscTransaction);
    }

    @Test
    public void testQueryCallBarringTb_notRegisteredReturnsDisabled() {
        int tId = 31;
        int condition = SscConstant.CONDITION_BAOC;
        int serviceClass = SscServiceClassUtil.SERVICE_CLASS_VOICE;
        when(mMockCarrierConfig.getIntArray(
                CarrierConfigManager.ImsSs.KEY_UT_TERMINAL_BASED_SERVICES_INT_ARRAY))
                .thenReturn(new int[]{
                        CarrierConfigManager.ImsSs.SUPPLEMENTARY_SERVICE_CB_BAOC});
        when(mMockSscPreferenceHelper.queryCb(eq(condition), eq(serviceClass)))
                .thenReturn(SscConstant.STATUS_NOT_REGISTERED);
        mSscServiceImpl.setSscPreferenceHelper(mMockSscPreferenceHelper);

        mSscServiceImpl.queryCallBarringForServiceClass(tId, condition, serviceClass);
        mLooper.processAllMessages();

        verify(mMockSscPreferenceHelper).queryCb(eq(condition), eq(serviceClass));
        verify(mMockUtListener).utConfigurationCallBarringQueried(eq(tId), captorSsInfos.capture());

        ImsSsInfo[] capturedSsInfos = captorSsInfos.getValue();
        assertNotNull(capturedSsInfos);
        assertEquals(1, capturedSsInfos.length);
        // If not registered, it should default to disabled.
        assertEquals(SscConstant.STATUS_DISABLE, capturedSsInfos[0].getStatus());

        verifyNoMoreInteractions(mMockSscTransaction);
    }

    @Test
    public void testQueryCallBarringTb_disabled() {
        int tId = 32;
        int condition = SscConstant.CONDITION_BOIC;
        int serviceClass = SscServiceClassUtil.SERVICE_CLASS_DATA;
        when(mMockCarrierConfig.getIntArray(
                CarrierConfigManager.ImsSs.KEY_UT_TERMINAL_BASED_SERVICES_INT_ARRAY))
                .thenReturn(new int[]{
                        CarrierConfigManager.ImsSs.SUPPLEMENTARY_SERVICE_CB_BOIC});
        when(mMockSscPreferenceHelper.queryCb(eq(condition), eq(serviceClass)))
                .thenReturn(SscConstant.STATUS_DISABLE);
        mSscServiceImpl.setSscPreferenceHelper(mMockSscPreferenceHelper);

        mSscServiceImpl.queryCallBarringForServiceClass(tId, condition, serviceClass);
        mLooper.processAllMessages();

        verify(mMockSscPreferenceHelper).queryCb(eq(condition), eq(serviceClass));
        verify(mMockUtListener).utConfigurationCallBarringQueried(eq(tId), captorSsInfos.capture());

        ImsSsInfo[] capturedSsInfos = captorSsInfos.getValue();
        assertNotNull(capturedSsInfos);
        assertEquals(1, capturedSsInfos.length);
        assertEquals(SscConstant.STATUS_DISABLE, capturedSsInfos[0].getStatus());

        verifyNoMoreInteractions(mMockSscTransaction);
    }

    @Test
    public void testQueryCallBarringTb_enabled() {
        int tId = 33;
        int condition = SscConstant.CONDITION_BOIC_EXHC;
        int serviceClass = SscServiceClassUtil.SERVICE_CLASS_VIDEO;
        when(mMockCarrierConfig.getIntArray(
                CarrierConfigManager.ImsSs.KEY_UT_TERMINAL_BASED_SERVICES_INT_ARRAY))
                .thenReturn(new int[]{
                        CarrierConfigManager.ImsSs.SUPPLEMENTARY_SERVICE_CB_BOIC_EXHC});
        when(mMockSscPreferenceHelper.queryCb(eq(condition), eq(serviceClass)))
                .thenReturn(SscConstant.STATUS_ENABLE);
        mSscServiceImpl.setSscPreferenceHelper(mMockSscPreferenceHelper);

        mSscServiceImpl.queryCallBarringForServiceClass(tId, condition, serviceClass);
        mLooper.processAllMessages();

        verify(mMockSscPreferenceHelper).queryCb(eq(condition), eq(serviceClass));
        verify(mMockUtListener).utConfigurationCallBarringQueried(eq(tId), captorSsInfos.capture());

        ImsSsInfo[] capturedSsInfos = captorSsInfos.getValue();
        assertNotNull(capturedSsInfos);
        assertEquals(1, capturedSsInfos.length);
        assertEquals(SscConstant.STATUS_ENABLE, capturedSsInfos[0].getStatus());

        verifyNoMoreInteractions(mMockSscTransaction);
    }

    @Test
    public void testQueryCallBarringTb_serviceClassNone_returnsForVoice() {
        int tId = 34;
        int condition = SscConstant.CONDITION_ACR;
        int serviceClass = SscServiceClassUtil.SERVICE_CLASS_NONE; // Input, should default to VOICE
        int expectedServiceClass = SscServiceClassUtil.SERVICE_CLASS_VOICE;

        when(mMockCarrierConfig.getIntArray(
                CarrierConfigManager.ImsSs.KEY_UT_TERMINAL_BASED_SERVICES_INT_ARRAY))
                .thenReturn(new int[]{
                        CarrierConfigManager.ImsSs.SUPPLEMENTARY_SERVICE_CB_ACR});
        when(mMockSscPreferenceHelper.queryCb(eq(condition), eq(expectedServiceClass)))
                .thenReturn(SscConstant.STATUS_ENABLE);
        mSscServiceImpl.setSscPreferenceHelper(mMockSscPreferenceHelper);

        mSscServiceImpl.queryCallBarringForServiceClass(tId, condition, serviceClass);
        mLooper.processAllMessages();

        verify(mMockSscPreferenceHelper).queryCb(eq(condition), eq(expectedServiceClass));
        verify(mMockUtListener).utConfigurationCallBarringQueried(eq(tId), captorSsInfos.capture());

        ImsSsInfo[] capturedSsInfos = captorSsInfos.getValue();
        assertNotNull(capturedSsInfos);
        assertEquals(1, capturedSsInfos.length);
        assertEquals(SscConstant.STATUS_ENABLE, capturedSsInfos[0].getStatus());

        verifyNoMoreInteractions(mMockSscTransaction);
    }

    @Test
    public void testQueryCallForward_singleRequest() {
        int queryCondition = SscConstant.CONDITION_CFU;
        int tId = 1;

        mSscServiceImpl.queryCallForward(tId, queryCondition, "");
        processEntireXmlDocQueryAsSuccess();
        processGetTransactionAsSuccess(ESsType.CF, SscConstant.EVENT_SSC_QUERY_CF,
                queryCondition);

        mLooper.processAllMessages();
        verify(mMockUtListener).utConfigurationCallForwardQueried(eq(tId), captorCfInfos.capture());

        ImsCallForwardInfo[] cfInfos = captorCfInfos.getValue();
        assertNotNull(cfInfos);
        assertEquals(queryCondition, cfInfos[0].mCondition);

        int xmlStatus = mIsCfRuleActivated ? SscConstant.STATUS_ENABLE : SscConstant.STATUS_DISABLE;
        assertEquals(cfInfos[0].mStatus, xmlStatus);
        if (!TextUtils.isEmpty(mForwardNumber)) {
            assertTrue(mForwardNumber.contains(cfInfos[0].mNumber));
        }

        verify(mMockSscTransaction).close();
        verifyNoMoreInteractions(mMockSscTransaction);
    }

    @Test
    public void testQueryCallForward_multipleRequest() {
        when(mMockCarrierConfig.getBoolean(
                eq(CarrierConfig.ImsSs.KEY_UT_QUERY_CF_ALL_AND_CF_ALL_CONDITIONAL_SUPPORT_BOOL)))
                .thenReturn(true);

        int responseCount = 1;
        int tId = 1;
        for (int c = SscConstant.CONDITION_CFU; c <= SscConstant.CONDITION_CFNL; c++) {
            tId++;
            mSscServiceImpl.queryCallForward(tId, c, "");

            if (!SscXmlGov.getInstance(SLOT_0).isXmlDataPresent()) {
                processEntireXmlDocQueryAsSuccess();
            }

            processGetTransactionAsSuccess(ESsType.CF, SscConstant.EVENT_SSC_QUERY_CF,
                    c);

            mLooper.processAllMessages();
            verify(mMockUtListener).utConfigurationCallForwardQueried(eq(tId), any());
            verify(mMockSscTransaction, atLeast(responseCount)).close();
            responseCount++;
        }

        verifyNoMoreInteractions(mMockSscTransaction);
    }

    @Test
    public void testQueryCallForward_notSupportedService() {
        when(mMockCarrierConfig.getIntArray(
                CarrierConfigManager.ImsSs.KEY_UT_SERVER_BASED_SERVICES_INT_ARRAY))
                .thenReturn(new int[] {});
        int tId = 1;

        mSscServiceImpl.queryCallForward(tId, SscConstant.CONDITION_CFU, "");

        mLooper.processAllMessages();
        verify(mMockUtListener).utConfigurationQueryFailed(eq(tId), captorReasonInfo.capture());
        verifyNoMoreInteractions(mMockSscTransaction);

        ImsReasonInfo reasonInfo = captorReasonInfo.getValue();
        assertNotNull(reasonInfo);
        assertEquals(ImsReasonInfo.CODE_UT_OPERATION_NOT_ALLOWED, reasonInfo.getCode());
    }

    @Test
    public void testQueryCallForward_entireDocumentQueryFailure() {
        int tId = 1;

        mSscServiceImpl.queryCallForward(tId, SscConstant.CONDITION_CFU, "");
        processEntireXmlDocQueryAsFailure();

        mLooper.processAllMessages();
        verify(mMockUtListener).utConfigurationQueryFailed(eq(tId), any());
        verify(mMockSscTransaction).close();
        verifyNoMoreInteractions(mMockSscTransaction);
    }

    @Test
    public void testQueryCallForward_queryFailure() {
        int tId = 1;

        mSscServiceImpl.queryCallForward(tId, SscConstant.CONDITION_CFU, "");
        processEntireXmlDocQueryAsSuccess();
        processGetTransactionAsFailure(ESsType.CF, SscConstant.EVENT_SSC_QUERY_CF,
                SscConstant.CONDITION_CFU);

        mLooper.processAllMessages();
        verify(mMockUtListener).utConfigurationQueryFailed(eq(tId), any());
        verify(mMockSscTransaction).close();
        verifyNoMoreInteractions(mMockSscTransaction);
    }

    @Test
    public void testQueryCallForward_queryCfaAndCfacNotSupported() {
        when(mMockCarrierConfig.getBoolean(
                eq(CarrierConfig.ImsSs.KEY_UT_QUERY_CF_ALL_AND_CF_ALL_CONDITIONAL_SUPPORT_BOOL)))
                .thenReturn(false);

        int tId = 1;

        mSscServiceImpl.queryCallForward(tId, SscConstant.CONDITION_CFA, "");

        mLooper.processAllMessages();
        verify(mMockUtListener).utConfigurationQueryFailed(eq(tId), any());
        verifyNoMoreInteractions(mMockSscTransaction);
    }

    @Test
    public void testQueryCallWaiting_success() {
        int tId = 1;

        mSscServiceImpl.queryCallWaiting(tId);
        processEntireXmlDocQueryAsSuccess();
        processGetTransactionAsSuccess(ESsType.CW, SscConstant.EVENT_SSC_QUERY_CW, -1);

        mLooper.processAllMessages();
        verify(mMockUtListener).utConfigurationCallWaitingQueried(eq(tId), captorSsInfos.capture());

        ImsSsInfo[] cwInfos = captorSsInfos.getValue();
        assertNotNull(cwInfos);

        int xmlStatus = mIsCwEnabled ? SscConstant.STATUS_ENABLE : SscConstant.STATUS_DISABLE;
        assertEquals(cwInfos[0].mStatus, xmlStatus);

        verify(mMockSscTransaction).close();
    }

    @Test
    public void testQueryCallWaiting_failure() {
        int tId = 1;

        mSscServiceImpl.queryCallWaiting(tId);
        processEntireXmlDocQueryAsSuccess();
        processGetTransactionAsFailure(ESsType.CW, SscConstant.EVENT_SSC_QUERY_CW, -1);

        mLooper.processAllMessages();
        verify(mMockUtListener).utConfigurationQueryFailed(eq(tId), any());
        verify(mMockSscTransaction).close();
    }

    @Test
    public void testQueryCallWaiting_notSupportedService() {
        when(mMockCarrierConfig.getIntArray(
                CarrierConfigManager.ImsSs.KEY_UT_SERVER_BASED_SERVICES_INT_ARRAY))
                .thenReturn(new int[] {});
        int tId = 1;

        mSscServiceImpl.queryCallWaiting(tId);

        mLooper.processAllMessages();
        verify(mMockUtListener).utConfigurationQueryFailed(eq(tId), captorReasonInfo.capture());
        verifyNoMoreInteractions(mMockSscTransaction);

        ImsReasonInfo reasonInfo = captorReasonInfo.getValue();
        assertNotNull(reasonInfo);
        assertEquals(ImsReasonInfo.CODE_UT_OPERATION_NOT_ALLOWED, reasonInfo.getCode());
    }

    @Test
    public void testQueryClir_success() {
        int tId = 1;

        mSscServiceImpl.queryCLIR(tId);
        processEntireXmlDocQueryAsSuccess();
        processGetTransactionAsSuccess(ESsType.OIR, SscConstant.EVENT_SSC_QUERY_OIR, -1);

        mLooper.processAllMessages();
        verify(mMockUtListener)
                .lineIdentificationSupplementaryServiceResponse(eq(tId), captorSsInfo.capture());

        ImsSsInfo ssInfo = captorSsInfo.getValue();
        assertNotNull(ssInfo);

        int outGoingState = SscXmlFormat.PRESENTATION_NOT_RESTRICTED.equals(mDefaultBehaviour)
                ? SscConstant.OIR_SUPPRESSION : SscConstant.OIR_INVOCATION;
        assertEquals(ssInfo.getClirOutgoingState(), outGoingState);

        verify(mMockSscTransaction).close();
    }

    @Test
    public void testQueryClir_failure() {
        int tId = 1;

        mSscServiceImpl.queryCLIR(tId);
        processEntireXmlDocQueryAsSuccess();
        processGetTransactionAsFailure(ESsType.OIR, SscConstant.EVENT_SSC_QUERY_OIR, -1);

        mLooper.processAllMessages();
        verify(mMockUtListener).utConfigurationQueryFailed(eq(tId), any());
        verify(mMockSscTransaction).close();
    }

    @Test
    public void testQueryClir_notSupportedService() {
        when(mMockCarrierConfig.getIntArray(
                CarrierConfigManager.ImsSs.KEY_UT_SERVER_BASED_SERVICES_INT_ARRAY))
                .thenReturn(new int[] {});
        int tId = 1;

        mSscServiceImpl.queryCLIR(tId);

        mLooper.processAllMessages();
        verify(mMockUtListener).utConfigurationQueryFailed(eq(tId), captorReasonInfo.capture());
        verifyNoMoreInteractions(mMockSscTransaction);

        ImsReasonInfo reasonInfo = captorReasonInfo.getValue();
        assertNotNull(reasonInfo);
        assertEquals(ImsReasonInfo.CODE_UT_OPERATION_NOT_ALLOWED, reasonInfo.getCode());
    }

    @Test
    public void testQueryClirTb_notRegisteredReturnsDefault() {
        when(mMockCarrierConfig.getIntArray(
                CarrierConfigManager.ImsSs.KEY_UT_TERMINAL_BASED_SERVICES_INT_ARRAY))
                .thenReturn(new int[] {
                        CarrierConfigManager.ImsSs.SUPPLEMENTARY_SERVICE_IDENTIFICATION_OIR});
        when(mMockSscPreferenceHelper.queryOir()).thenReturn(SscConstant.STATUS_NOT_REGISTERED);
        mSscServiceImpl.setSscPreferenceHelper(mMockSscPreferenceHelper);
        int tId = 1;

        mSscServiceImpl.queryCLIR(tId);
        mLooper.processAllMessages();

        verify(mMockUtListener)
                .lineIdentificationSupplementaryServiceResponse(eq(tId), captorSsInfo.capture());

        ImsSsInfo ssInfo = captorSsInfo.getValue();
        assertNotNull(ssInfo);
        assertEquals(SscConstant.OIR_TEMPORARY_MODE_PRESENTATION_ALLOWED,
                ssInfo.getClirInterrogationStatus());
        assertEquals(SscConstant.OIR_DEFAULT, ssInfo.getClirOutgoingState());

        verifyNoMoreInteractions(mMockSscTransaction);
    }

    @Test
    public void testQueryClirTb_default() {
        when(mMockCarrierConfig.getIntArray(
                CarrierConfigManager.ImsSs.KEY_UT_TERMINAL_BASED_SERVICES_INT_ARRAY))
                .thenReturn(new int[] {
                        CarrierConfigManager.ImsSs.SUPPLEMENTARY_SERVICE_IDENTIFICATION_OIR});
        when(mMockSscPreferenceHelper.queryOir()).thenReturn(SscConstant.OIR_DEFAULT);
        mSscServiceImpl.setSscPreferenceHelper(mMockSscPreferenceHelper);
        int tId = 1;

        mSscServiceImpl.queryCLIR(tId);
        mLooper.processAllMessages();

        verify(mMockUtListener)
                .lineIdentificationSupplementaryServiceResponse(eq(tId), captorSsInfo.capture());

        ImsSsInfo ssInfo = captorSsInfo.getValue();
        assertNotNull(ssInfo);
        assertEquals(SscConstant.OIR_TEMPORARY_MODE_PRESENTATION_ALLOWED,
                ssInfo.getClirInterrogationStatus());
        assertEquals(SscConstant.OIR_DEFAULT, ssInfo.getClirOutgoingState());
        verifyNoMoreInteractions(mMockSscTransaction);
    }

    @Test
    public void testQueryClirTb_queryNetworkForDefault() {
        mDefaultBehaviour = SscXmlFormat.PRESENTATION_RESTRICTED;
        when(mMockCarrierConfig.getIntArray(
                CarrierConfigManager.ImsSs.KEY_UT_TERMINAL_BASED_SERVICES_INT_ARRAY))
                .thenReturn(new int[] {
                        CarrierConfigManager.ImsSs.SUPPLEMENTARY_SERVICE_IDENTIFICATION_OIR});
        when(mMockCarrierConfig.getBoolean(
                eq(CarrierConfig.ImsSs.KEY_UT_NETWORK_QUERY_FOR_TB_OIR_NETWORK_DEFAULT_BOOL)))
                .thenReturn(true);
        when(mMockSscPreferenceHelper.queryOir()).thenReturn(SscConstant.OIR_DEFAULT);
        mSscServiceImpl.setSscPreferenceHelper(mMockSscPreferenceHelper);
        int tId = 1;

        mSscServiceImpl.queryCLIR(tId);
        processEntireXmlDocQueryAsSuccess();
        processGetTransactionAsSuccess(ESsType.OIR,
                SscConstant.EVENT_SSC_QUERY_OIR_TB_NETWORK_DEFAULT, -1);

        mLooper.processAllMessages();
        verify(mMockUtListener)
                .lineIdentificationSupplementaryServiceResponse(eq(tId), captorSsInfo.capture());

        ImsSsInfo ssInfo = captorSsInfo.getValue();
        assertNotNull(ssInfo);
        assertEquals(SscConstant.OIR_TEMPORARY_MODE_PRESENTATION_RESTRICTED,
                ssInfo.getClirInterrogationStatus());
        assertEquals(SscConstant.OIR_DEFAULT, ssInfo.getClirOutgoingState());
    }

    @Test
    public void testQueryClirTb_invocation() {
        when(mMockCarrierConfig.getIntArray(
                CarrierConfigManager.ImsSs.KEY_UT_TERMINAL_BASED_SERVICES_INT_ARRAY))
                .thenReturn(new int[] {
                        CarrierConfigManager.ImsSs.SUPPLEMENTARY_SERVICE_IDENTIFICATION_OIR});
        when(mMockSscPreferenceHelper.queryOir()).thenReturn(SscConstant.OIR_INVOCATION);
        mSscServiceImpl.setSscPreferenceHelper(mMockSscPreferenceHelper);
        int tId = 1;

        mSscServiceImpl.queryCLIR(tId);
        mLooper.processAllMessages();

        verify(mMockUtListener)
                .lineIdentificationSupplementaryServiceResponse(eq(tId), captorSsInfo.capture());

        ImsSsInfo ssInfo = captorSsInfo.getValue();
        assertNotNull(ssInfo);
        assertEquals(SscConstant.OIR_TEMPORARY_MODE_PRESENTATION_RESTRICTED,
                ssInfo.getClirInterrogationStatus());
        assertEquals(SscConstant.OIR_INVOCATION, ssInfo.getClirOutgoingState());
        verifyNoMoreInteractions(mMockSscTransaction);
    }

    @Test
    public void testQueryClirTb_suppression() {
        when(mMockCarrierConfig.getIntArray(
                CarrierConfigManager.ImsSs.KEY_UT_TERMINAL_BASED_SERVICES_INT_ARRAY))
                .thenReturn(new int[] {
                        CarrierConfigManager.ImsSs.SUPPLEMENTARY_SERVICE_IDENTIFICATION_OIR});
        when(mMockSscPreferenceHelper.queryOir()).thenReturn(SscConstant.OIR_SUPPRESSION);
        mSscServiceImpl.setSscPreferenceHelper(mMockSscPreferenceHelper);
        int tId = 1;

        mSscServiceImpl.queryCLIR(tId);
        mLooper.processAllMessages();

        verify(mMockUtListener)
                .lineIdentificationSupplementaryServiceResponse(eq(tId), captorSsInfo.capture());

        ImsSsInfo ssInfo = captorSsInfo.getValue();
        assertNotNull(ssInfo);
        assertEquals(SscConstant.OIR_TEMPORARY_MODE_PRESENTATION_ALLOWED,
                ssInfo.getClirInterrogationStatus());
        assertEquals(SscConstant.OIR_SUPPRESSION, ssInfo.getClirOutgoingState());
        verifyNoMoreInteractions(mMockSscTransaction);
    }

    @Test
    public void testQueryClir_noRequestToSever_invocation() {
        when(mMockCarrierConfig.getInt(
                CarrierConfig.ImsSs.KEY_UT_OIR_NETWORK_DEFAULT_OPERATION_INT))
                .thenReturn(SscConfig.OIR_NO_REQUEST_TO_SERVER);
        when(mMockSscPreferenceHelper.queryOir()).thenReturn(SscConstant.OIR_INVOCATION);
        mSscServiceImpl.setSscPreferenceHelper(mMockSscPreferenceHelper);
        int tId = 1;

        mSscServiceImpl.queryCLIR(tId);
        processEntireXmlDocQueryAsSuccess();
        processGetTransactionAsSuccess(ESsType.OIR, SscConstant.EVENT_SSC_QUERY_OIR, -1);

        mLooper.processAllMessages();
        verify(mMockUtListener)
                .lineIdentificationSupplementaryServiceResponse(eq(tId), captorSsInfo.capture());

        ImsSsInfo ssInfo = captorSsInfo.getValue();
        assertNotNull(ssInfo);

        int outGoingState = SscXmlFormat.PRESENTATION_NOT_RESTRICTED.equals(mDefaultBehaviour)
                ? SscConstant.OIR_SUPPRESSION : SscConstant.OIR_INVOCATION;
        assertEquals(ssInfo.getClirOutgoingState(), outGoingState);

        verify(mMockSscTransaction).close();
    }

    @Test
    public void testQueryClir_noRequestToSever_default() {
        when(mMockCarrierConfig.getInt(
                CarrierConfig.ImsSs.KEY_UT_OIR_NETWORK_DEFAULT_OPERATION_INT))
                .thenReturn(SscConfig.OIR_NO_REQUEST_TO_SERVER);
        when(mMockSscPreferenceHelper.queryOir()).thenReturn(SscConstant.OIR_DEFAULT);
        mSscServiceImpl.setSscPreferenceHelper(mMockSscPreferenceHelper);
        int tId = 1;

        mSscServiceImpl.queryCLIR(tId);
        processEntireXmlDocQueryAsSuccess();
        processGetTransactionAsSuccess(ESsType.OIR, SscConstant.EVENT_SSC_QUERY_OIR, -1);

        mLooper.processAllMessages();
        verify(mMockUtListener)
                .lineIdentificationSupplementaryServiceResponse(eq(tId), captorSsInfo.capture());

        ImsSsInfo ssInfo = captorSsInfo.getValue();
        assertNotNull(ssInfo);
        assertEquals(SscConstant.OIR_DEFAULT, ssInfo.getClirOutgoingState());

        verify(mMockSscTransaction).close();
    }

    @Test
    public void testQueryClip_success() {
        int tId = 1;

        mSscServiceImpl.queryCLIP(tId);
        processEntireXmlDocQueryAsSuccess();
        processGetTransactionAsSuccess(ESsType.OIP, SscConstant.EVENT_SSC_QUERY_OIP, -1);

        mLooper.processAllMessages();
        verify(mMockUtListener)
                .lineIdentificationSupplementaryServiceResponse(eq(tId), captorSsInfo.capture());

        ImsSsInfo ssInfo = captorSsInfo.getValue();
        assertNotNull(ssInfo);

        int xmlStatus = mIsOipEnabled ? SscConstant.STATUS_ENABLE : SscConstant.STATUS_DISABLE;
        assertEquals(ssInfo.mStatus, xmlStatus);

        verify(mMockSscTransaction).close();
    }

    @Test
    public void testQueryClip_failure() {
        int tId = 1;

        mSscServiceImpl.queryCLIP(tId);
        processEntireXmlDocQueryAsSuccess();
        processGetTransactionAsFailure(ESsType.OIP, SscConstant.EVENT_SSC_QUERY_OIP, -1);

        mLooper.processAllMessages();
        verify(mMockUtListener).utConfigurationQueryFailed(eq(tId), any());
        verify(mMockSscTransaction).close();
    }

    @Test
    public void testQueryClip_notSupportedService() {
        when(mMockCarrierConfig.getIntArray(
                CarrierConfigManager.ImsSs.KEY_UT_SERVER_BASED_SERVICES_INT_ARRAY))
                .thenReturn(new int[] {});
        int tId = 1;

        mSscServiceImpl.queryCLIP(tId);

        mLooper.processAllMessages();
        verify(mMockUtListener).utConfigurationQueryFailed(eq(tId), captorReasonInfo.capture());
        verifyNoMoreInteractions(mMockSscTransaction);

        ImsReasonInfo reasonInfo = captorReasonInfo.getValue();
        assertNotNull(reasonInfo);
        assertEquals(ImsReasonInfo.CODE_UT_OPERATION_NOT_ALLOWED, reasonInfo.getCode());
    }

    @Test
    public void testQueryClipTb_notRegisteredReturnsDisabled() {
        when(mMockCarrierConfig.getIntArray(
                CarrierConfigManager.ImsSs.KEY_UT_TERMINAL_BASED_SERVICES_INT_ARRAY))
                .thenReturn(new int[]{
                        CarrierConfigManager.ImsSs.SUPPLEMENTARY_SERVICE_IDENTIFICATION_OIP});
        when(mMockSscPreferenceHelper.queryOip()).thenReturn(SscConstant.STATUS_NOT_REGISTERED);
        mSscServiceImpl.setSscPreferenceHelper(mMockSscPreferenceHelper);
        int tId = 10;

        mSscServiceImpl.queryCLIP(tId);
        mLooper.processAllMessages();

        verify(mMockUtListener)
                .lineIdentificationSupplementaryServiceResponse(eq(tId), captorSsInfo.capture());

        ImsSsInfo ssInfo = captorSsInfo.getValue();
        assertNotNull(ssInfo);
        // If not registered, it should default to disabled.
        assertEquals(SscConstant.STATUS_DISABLE, ssInfo.getStatus());

        verifyNoMoreInteractions(mMockSscTransaction);
    }

    @Test
    public void testQueryClipTb_disabled() {
        when(mMockCarrierConfig.getIntArray(
                CarrierConfigManager.ImsSs.KEY_UT_TERMINAL_BASED_SERVICES_INT_ARRAY))
                .thenReturn(new int[]{
                        CarrierConfigManager.ImsSs.SUPPLEMENTARY_SERVICE_IDENTIFICATION_OIP});
        when(mMockSscPreferenceHelper.queryOip()).thenReturn(SscConstant.STATUS_DISABLE);
        mSscServiceImpl.setSscPreferenceHelper(mMockSscPreferenceHelper);
        int tId = 11;

        mSscServiceImpl.queryCLIP(tId);
        mLooper.processAllMessages();

        verify(mMockUtListener)
                .lineIdentificationSupplementaryServiceResponse(eq(tId), captorSsInfo.capture());

        ImsSsInfo ssInfo = captorSsInfo.getValue();
        assertNotNull(ssInfo);
        assertEquals(SscConstant.STATUS_DISABLE, ssInfo.getStatus());
        verifyNoMoreInteractions(mMockSscTransaction);
    }

    @Test
    public void testQueryClipTb_enabled() {
        when(mMockCarrierConfig.getIntArray(
                CarrierConfigManager.ImsSs.KEY_UT_TERMINAL_BASED_SERVICES_INT_ARRAY))
                .thenReturn(new int[]{
                        CarrierConfigManager.ImsSs.SUPPLEMENTARY_SERVICE_IDENTIFICATION_OIP});
        when(mMockSscPreferenceHelper.queryOip()).thenReturn(SscConstant.STATUS_ENABLE);
        mSscServiceImpl.setSscPreferenceHelper(mMockSscPreferenceHelper);
        int tId = 11;

        mSscServiceImpl.queryCLIP(tId);
        mLooper.processAllMessages();

        verify(mMockUtListener)
                .lineIdentificationSupplementaryServiceResponse(eq(tId), captorSsInfo.capture());

        ImsSsInfo ssInfo = captorSsInfo.getValue();
        assertNotNull(ssInfo);
        assertEquals(SscConstant.STATUS_ENABLE, ssInfo.getStatus());

        verifyNoMoreInteractions(mMockSscTransaction);
    }

    @Test
    public void testQueryColr_success() {
        mDefaultBehaviour = SscXmlFormat.PRESENTATION_RESTRICTED;
        int tId = 1;

        mSscServiceImpl.queryCOLR(tId);
        processEntireXmlDocQueryAsSuccess();
        processGetTransactionAsSuccess(ESsType.TIR, SscConstant.EVENT_SSC_QUERY_TIR, -1);

        mLooper.processAllMessages();
        verify(mMockUtListener)
                .lineIdentificationSupplementaryServiceResponse(eq(tId), captorSsInfo.capture());

        ImsSsInfo ssInfo = captorSsInfo.getValue();
        assertNotNull(ssInfo);

        int provisionedStatus = SscXmlFormat.PRESENTATION_NOT_RESTRICTED.equals(mDefaultBehaviour)
                ? SscConstant.TIR_NOT_PROVISIONED : SscConstant.TIR_PROVISIONED;
        assertEquals(ssInfo.getProvisionStatus(), provisionedStatus);

        verify(mMockSscTransaction).close();
    }

    @Test
    public void testQueryColr_failure() {
        int tId = 1;

        mSscServiceImpl.queryCOLR(tId);
        processEntireXmlDocQueryAsSuccess();
        processGetTransactionAsFailure(ESsType.TIR, SscConstant.EVENT_SSC_QUERY_TIR, -1);

        mLooper.processAllMessages();
        verify(mMockUtListener).utConfigurationQueryFailed(eq(tId), any());
        verify(mMockSscTransaction).close();
    }

    @Test
    public void testQueryColr_notSupportedService() {
        when(mMockCarrierConfig.getIntArray(
                CarrierConfigManager.ImsSs.KEY_UT_SERVER_BASED_SERVICES_INT_ARRAY))
                .thenReturn(new int[] {});
        int tId = 1;

        mSscServiceImpl.queryCOLR(tId);

        mLooper.processAllMessages();
        verify(mMockUtListener).utConfigurationQueryFailed(eq(tId), captorReasonInfo.capture());
        verifyNoMoreInteractions(mMockSscTransaction);

        ImsReasonInfo reasonInfo = captorReasonInfo.getValue();
        assertNotNull(reasonInfo);
        assertEquals(ImsReasonInfo.CODE_UT_OPERATION_NOT_ALLOWED, reasonInfo.getCode());
    }

    @Test
    public void testQueryColrTb_notRegisteredReturnsNotProvisioned() {
        when(mMockCarrierConfig.getIntArray(
                CarrierConfigManager.ImsSs.KEY_UT_TERMINAL_BASED_SERVICES_INT_ARRAY))
                .thenReturn(new int[]{
                        CarrierConfigManager.ImsSs.SUPPLEMENTARY_SERVICE_IDENTIFICATION_TIR});
        when(mMockSscPreferenceHelper.queryTir()).thenReturn(SscConstant.STATUS_NOT_REGISTERED);
        mSscServiceImpl.setSscPreferenceHelper(mMockSscPreferenceHelper);
        int tId = 11;

        mSscServiceImpl.queryCOLR(tId);
        mLooper.processAllMessages();

        verify(mMockUtListener)
                .lineIdentificationSupplementaryServiceResponse(eq(tId), captorSsInfo.capture());

        ImsSsInfo ssInfo = captorSsInfo.getValue();
        assertNotNull(ssInfo);
        assertEquals(SscConstant.STATUS_DISABLE, ssInfo.getStatus()); // Disabled if not registered.
        // If not registered, it should default to not provisioned.
        assertEquals(SscConstant.TIR_NOT_PROVISIONED, ssInfo.getProvisionStatus());

        verifyNoMoreInteractions(mMockSscTransaction);
    }

    @Test
    public void testQueryColrTb_notProvisioned() {
        when(mMockCarrierConfig.getIntArray(
                CarrierConfigManager.ImsSs.KEY_UT_TERMINAL_BASED_SERVICES_INT_ARRAY))
                .thenReturn(new int[]{
                        CarrierConfigManager.ImsSs.SUPPLEMENTARY_SERVICE_IDENTIFICATION_TIR});
        when(mMockSscPreferenceHelper.queryTir()).thenReturn(SscConstant.TIR_NOT_PROVISIONED);
        mSscServiceImpl.setSscPreferenceHelper(mMockSscPreferenceHelper);
        int tId = 12;

        mSscServiceImpl.queryCOLR(tId);
        mLooper.processAllMessages();

        verify(mMockUtListener)
                .lineIdentificationSupplementaryServiceResponse(eq(tId), captorSsInfo.capture());

        ImsSsInfo ssInfo = captorSsInfo.getValue();
        assertNotNull(ssInfo);
        assertEquals(SscConstant.STATUS_DISABLE, ssInfo.getStatus()); // Disabled if not provisioned
        assertEquals(SscConstant.TIR_NOT_PROVISIONED, ssInfo.getProvisionStatus());

        verifyNoMoreInteractions(mMockSscTransaction);
    }

    @Test
    public void testQueryColrTb_provisioned() {
        when(mMockCarrierConfig.getIntArray(
                CarrierConfigManager.ImsSs.KEY_UT_TERMINAL_BASED_SERVICES_INT_ARRAY))
                .thenReturn(new int[]{
                        CarrierConfigManager.ImsSs.SUPPLEMENTARY_SERVICE_IDENTIFICATION_TIR});
        when(mMockSscPreferenceHelper.queryTir()).thenReturn(SscConstant.TIR_PROVISIONED);
        mSscServiceImpl.setSscPreferenceHelper(mMockSscPreferenceHelper);
        int tId = 13;

        mSscServiceImpl.queryCOLR(tId);
        mLooper.processAllMessages();

        verify(mMockUtListener)
                .lineIdentificationSupplementaryServiceResponse(eq(tId), captorSsInfo.capture());

        ImsSsInfo ssInfo = captorSsInfo.getValue();
        assertNotNull(ssInfo);
        assertEquals(SscConstant.STATUS_ENABLE, ssInfo.getStatus()); // Enabled if provisioned.
        assertEquals(SscConstant.TIR_PROVISIONED, ssInfo.getProvisionStatus());

        verifyNoMoreInteractions(mMockSscTransaction);
    }

    @Test
    public void testqueryColp_success() {
        int tId = 1;

        mSscServiceImpl.queryCOLP(tId);
        processEntireXmlDocQueryAsSuccess();
        processGetTransactionAsSuccess(ESsType.TIP, SscConstant.EVENT_SSC_QUERY_TIP, -1);

        mLooper.processAllMessages();
        verify(mMockUtListener)
                .lineIdentificationSupplementaryServiceResponse(eq(tId), captorSsInfo.capture());

        ImsSsInfo ssInfo = captorSsInfo.getValue();
        assertNotNull(ssInfo);

        int xmlStatus = mIsTipEnabled ? SscConstant.STATUS_ENABLE : SscConstant.STATUS_DISABLE;
        assertEquals(ssInfo.mStatus, xmlStatus);

        verify(mMockSscTransaction).close();
    }

    @Test
    public void testqueryColp_failure() {
        int tId = 1;

        mSscServiceImpl.queryCOLP(tId);
        processEntireXmlDocQueryAsSuccess();
        processGetTransactionAsFailure(ESsType.TIP, SscConstant.EVENT_SSC_QUERY_TIP, -1);

        mLooper.processAllMessages();
        verify(mMockUtListener).utConfigurationQueryFailed(eq(tId), any());
        verify(mMockSscTransaction).close();
    }

    @Test
    public void testqueryColp_notSupportedService() {
        when(mMockCarrierConfig.getIntArray(
                CarrierConfigManager.ImsSs.KEY_UT_SERVER_BASED_SERVICES_INT_ARRAY))
                .thenReturn(new int[] {});
        int tId = 1;

        mSscServiceImpl.queryCOLP(tId);

        mLooper.processAllMessages();
        verify(mMockUtListener).utConfigurationQueryFailed(eq(tId), captorReasonInfo.capture());
        verifyNoMoreInteractions(mMockSscTransaction);

        ImsReasonInfo reasonInfo = captorReasonInfo.getValue();
        assertNotNull(reasonInfo);
        assertEquals(ImsReasonInfo.CODE_UT_OPERATION_NOT_ALLOWED, reasonInfo.getCode());
    }

    @Test
    public void testQueryColpTb_notRegisteredReturnsDisabled() {
        when(mMockCarrierConfig.getIntArray(
                CarrierConfigManager.ImsSs.KEY_UT_TERMINAL_BASED_SERVICES_INT_ARRAY))
                .thenReturn(new int[]{
                        CarrierConfigManager.ImsSs.SUPPLEMENTARY_SERVICE_IDENTIFICATION_TIP});
        when(mMockSscPreferenceHelper.queryTip()).thenReturn(SscConstant.STATUS_NOT_REGISTERED);
        mSscServiceImpl.setSscPreferenceHelper(mMockSscPreferenceHelper);
        int tId = 13;

        mSscServiceImpl.queryCOLP(tId);
        mLooper.processAllMessages();

        verify(mMockUtListener)
                .lineIdentificationSupplementaryServiceResponse(eq(tId), captorSsInfo.capture());

        ImsSsInfo ssInfo = captorSsInfo.getValue();
        assertNotNull(ssInfo);
        // If not registered, it should default to disabled.
        assertEquals(SscConstant.STATUS_DISABLE, ssInfo.getStatus());

        verifyNoMoreInteractions(mMockSscTransaction);
    }

    @Test
    public void testQueryColpTb_defaultDisabled() {
        when(mMockCarrierConfig.getIntArray(
                CarrierConfigManager.ImsSs.KEY_UT_TERMINAL_BASED_SERVICES_INT_ARRAY))
                .thenReturn(new int[]{
                        CarrierConfigManager.ImsSs.SUPPLEMENTARY_SERVICE_IDENTIFICATION_TIP});
        when(mMockSscPreferenceHelper.queryTip()).thenReturn(SscConstant.STATUS_DISABLE);
        mSscServiceImpl.setSscPreferenceHelper(mMockSscPreferenceHelper);
        int tId = 14;

        mSscServiceImpl.queryCOLP(tId);
        mLooper.processAllMessages();

        verify(mMockUtListener)
                .lineIdentificationSupplementaryServiceResponse(eq(tId), captorSsInfo.capture());

        ImsSsInfo ssInfo = captorSsInfo.getValue();
        assertNotNull(ssInfo);
        assertEquals(SscConstant.STATUS_DISABLE, ssInfo.getStatus());

        verifyNoMoreInteractions(mMockSscTransaction);
    }

    @Test
    public void testQueryColpTb_enabled() {
        when(mMockCarrierConfig.getIntArray(
                CarrierConfigManager.ImsSs.KEY_UT_TERMINAL_BASED_SERVICES_INT_ARRAY))
                .thenReturn(new int[]{
                        CarrierConfigManager.ImsSs.SUPPLEMENTARY_SERVICE_IDENTIFICATION_TIP});
        when(mMockSscPreferenceHelper.queryTip()).thenReturn(SscConstant.STATUS_ENABLE);
        mSscServiceImpl.setSscPreferenceHelper(mMockSscPreferenceHelper);
        int tId = 15;

        mSscServiceImpl.queryCOLP(tId);
        mLooper.processAllMessages();

        verify(mMockUtListener)
                .lineIdentificationSupplementaryServiceResponse(eq(tId), captorSsInfo.capture());

        ImsSsInfo ssInfo = captorSsInfo.getValue();
        assertNotNull(ssInfo);
        assertEquals(SscConstant.STATUS_ENABLE, ssInfo.getStatus());

        verifyNoMoreInteractions(mMockSscTransaction);
    }

    @Test
    public void testUpdateCallBarringWithPassword_singleRequestIcb() {
        int tId = 1;

        mSscServiceImpl.updateCallBarringWithPassword(tId, SscConstant.CONDITION_BIC_WR,
                SscConstant.STATUS_ENABLE, null, SscServiceClassUtil.SERVICE_CLASS_VOICE, null);
        processEntireXmlDocQueryAsSuccess();
        processPutTransactionAsSuccess(ESsType.ICB, SscConstant.EVENT_SSC_UPDATE_CB,
                SscConstant.CONDITION_BIC_WR);

        mLooper.processAllMessages();
        verify(mMockUtListener).utConfigurationUpdated(eq(tId));
        verify(mMockSscTransaction).close();
        verifyNoMoreInteractions(mMockSscTransaction);
    }

    @Test
    public void testUpdateCallBarringWithPassword_insertNewRule() {
        mIsIcbRulesExist = false;

        when(mMockCarrierConfig.getBoolean(eq(CarrierConfig.ImsSs.KEY_UT_INSERT_NEW_RULE_BOOL)))
                .thenReturn(true);
        int tId = 1;

        mSscServiceImpl.updateCallBarringWithPassword(tId, SscConstant.CONDITION_BAIC,
                SscConstant.STATUS_ENABLE, null, SscServiceClassUtil.SERVICE_CLASS_VOICE, null);
        processEntireXmlDocQueryAsSuccess();
        processPutTransactionAsSuccess(ESsType.ICB, SscConstant.EVENT_SSC_INSERT_CB,
                SscConstant.CONDITION_BAIC);

        mLooper.processAllMessages();
        verify(mMockUtListener).utConfigurationUpdated(eq(tId));
        verify(mMockSscTransaction).close();
        verifyNoMoreInteractions(mMockSscTransaction);
    }

    @Test
    public void testUpdateCallBarringWithPassword_entireDocumentQueryFailure() {
        int tId = 1;

        mSscServiceImpl.updateCallBarringWithPassword(tId, SscConstant.CONDITION_BIC_WR,
                SscConstant.STATUS_ENABLE, null, SscServiceClassUtil.SERVICE_CLASS_VOICE, null);
        processEntireXmlDocQueryAsFailure();

        mLooper.processAllMessages();
        verify(mMockUtListener).utConfigurationUpdateFailed(eq(tId), any());
        verify(mMockSscTransaction).close();
        verifyNoMoreInteractions(mMockSscTransaction);
    }

    @Test
    public void testUpdateCallBarringWithPassword_updateFailure() {
        int tId = 1;

        mSscServiceImpl.updateCallBarringWithPassword(tId, SscConstant.CONDITION_BIC_WR,
                SscConstant.STATUS_ENABLE, null, SscServiceClassUtil.SERVICE_CLASS_VOICE, null);
        processEntireXmlDocQueryAsSuccess();
        processPutTransactionAsFailure(ESsType.ICB, SscConstant.EVENT_SSC_UPDATE_CB,
                SscConstant.CONDITION_BIC_WR);

        mLooper.processAllMessages();
        verify(mMockUtListener).utConfigurationUpdateFailed(eq(tId), any());
        verify(mMockSscTransaction).close();
        verifyNoMoreInteractions(mMockSscTransaction);
    }

    @Test
    public void testUpdateCallBarringWithPassword_notSupportedService() {
        when(mMockCarrierConfig.getIntArray(
                CarrierConfigManager.ImsSs.KEY_UT_SERVER_BASED_SERVICES_INT_ARRAY))
                .thenReturn(new int[] {});
        int tId = 1;

        mSscServiceImpl.updateCallBarringWithPassword(tId, SscConstant.CONDITION_BIC_WR,
                SscConstant.STATUS_ENABLE, null, SscServiceClassUtil.SERVICE_CLASS_NONE, null);

        mLooper.processAllMessages();
        verify(mMockUtListener).utConfigurationUpdateFailed(eq(tId), captorReasonInfo.capture());
        verifyNoMoreInteractions(mMockSscTransaction);

        ImsReasonInfo reasonInfo = captorReasonInfo.getValue();
        assertNotNull(reasonInfo);
        assertEquals(ImsReasonInfo.CODE_UT_OPERATION_NOT_ALLOWED, reasonInfo.getCode());
    }

    @Test
    public void testUpdateCallBarringWithPassword_invalidAction() {
        int tId = 1;
        int invalidAction = SscConstant.STATUS_ENABLE + 1;

        mSscServiceImpl.updateCallBarringWithPassword(tId, SscConstant.CONDITION_BIC_WR,
                invalidAction, null, SscServiceClassUtil.SERVICE_CLASS_NONE, null);

        mLooper.processAllMessages();
        verify(mMockUtListener).utConfigurationUpdateFailed(eq(tId), captorReasonInfo.capture());
        verifyNoMoreInteractions(mMockSscTransaction);

        ImsReasonInfo reasonInfo = captorReasonInfo.getValue();
        assertNotNull(reasonInfo);
        assertEquals(ImsReasonInfo.CODE_UT_OPERATION_NOT_ALLOWED, reasonInfo.getCode());
    }

    @Test
    public void testUpdateCallBarringTb_enableSuccess_notifySscChanged() {
        int condition = SscConstant.CONDITION_BAOC;
        int serviceClass = SscServiceClassUtil.SERVICE_CLASS_VOICE;
        int action = SscConstant.ACTION_ACTIVATION; // STATUS_ENABLE.
        int expectedStatus = SscConstant.STATUS_ENABLE;
        when(mMockCarrierConfig.getIntArray(
                CarrierConfigManager.ImsSs.KEY_UT_TERMINAL_BASED_SERVICES_INT_ARRAY))
                .thenReturn(new int[]{
                        CarrierConfigManager.ImsSs.SUPPLEMENTARY_SERVICE_CB_BAOC});
        when(mMockSscPreferenceHelper.updateCb(eq(condition), eq(serviceClass), eq(expectedStatus)))
                .thenReturn(true);
        mSscServiceImpl.setSscPreferenceHelper(mMockSscPreferenceHelper);
        mSscServiceImpl.addTbSscChangeListener(mMockTbSscChangeListener);
        mLooper.processAllMessages();
        reset(mMockTbSscChangeListener);
        int tId = 70;

        mSscServiceImpl.updateCallBarringForServiceClass(
                tId, condition, action, null, serviceClass);
        mLooper.processAllMessages();

        verify(mMockSscPreferenceHelper)
                .updateCb(eq(condition), eq(serviceClass), eq(expectedStatus));
        verify(mMockUtListener).utConfigurationUpdated(eq(tId));
        verify(mMockTbSscChangeListener)
                .onSupplementaryServiceConfigurationChanged(mCaptorSscs.capture());

        List<SupplementaryServiceConfiguration> capturedList = mCaptorSscs.getValue();
        assertEquals(1, capturedList.size());
        assertTrue(capturedList.getFirst() instanceof CbData);
        CbData cbData = (CbData) capturedList.getFirst();
        assertEquals(condition, cbData.getCondition());
        assertEquals(serviceClass, cbData.getServiceClass());
        assertEquals(expectedStatus, cbData.getStatus());

        verifyNoMoreInteractions(mMockSscTransaction);
    }

    @Test
    public void testUpdateCallBarringTb_disableSuccess_notifySscChanged() {
        int condition = SscConstant.CONDITION_BAIC;
        int serviceClass = SscServiceClassUtil.SERVICE_CLASS_CALL;
        int action = SscConstant.ACTION_DEACTIVATION; // STATUS_DISABLE.
        int expectedStatus = SscConstant.STATUS_DISABLE;
        when(mMockCarrierConfig.getIntArray(
                CarrierConfigManager.ImsSs.KEY_UT_TERMINAL_BASED_SERVICES_INT_ARRAY))
                .thenReturn(new int[]{
                        CarrierConfigManager.ImsSs.SUPPLEMENTARY_SERVICE_CB_BAIC});
        when(mMockSscPreferenceHelper.updateCb(eq(condition), eq(serviceClass), eq(expectedStatus)))
                .thenReturn(true);
        mSscServiceImpl.setSscPreferenceHelper(mMockSscPreferenceHelper);
        mSscServiceImpl.addTbSscChangeListener(mMockTbSscChangeListener);
        mLooper.processAllMessages();
        reset(mMockTbSscChangeListener);
        int tId = 71;

        mSscServiceImpl.updateCallBarringForServiceClass(
                tId, condition, action, null, serviceClass);
        mLooper.processAllMessages();

        verify(mMockSscPreferenceHelper)
                .updateCb(eq(condition), eq(serviceClass), eq(expectedStatus));
        verify(mMockUtListener).utConfigurationUpdated(eq(tId));
        verify(mMockTbSscChangeListener)
                .onSupplementaryServiceConfigurationChanged(mCaptorSscs.capture());

        List<SupplementaryServiceConfiguration> capturedList = mCaptorSscs.getValue();
        assertEquals(1, capturedList.size());
        assertTrue(capturedList.getFirst() instanceof CbData);
        CbData cbData = (CbData) capturedList.getFirst();
        assertEquals(condition, cbData.getCondition());
        assertEquals(serviceClass, cbData.getServiceClass());
        assertEquals(expectedStatus, cbData.getStatus());

        verifyNoMoreInteractions(mMockSscTransaction);
    }

    @Test
    public void testUpdateCallBarringTb_serviceClassNone_updatesForVoice() {
        int condition = SscConstant.CONDITION_BAIC;
        int serviceClass = SscServiceClassUtil.SERVICE_CLASS_NONE; // Input, should default to VOICE
        int action = SscConstant.ACTION_DEACTIVATION; // STATUS_DISABLE.
        int expectedStatus = SscConstant.STATUS_DISABLE;
        int expectedServiceClass = SscServiceClassUtil.SERVICE_CLASS_VOICE;
        when(mMockCarrierConfig.getIntArray(
                CarrierConfigManager.ImsSs.KEY_UT_TERMINAL_BASED_SERVICES_INT_ARRAY))
                .thenReturn(new int[]{
                        CarrierConfigManager.ImsSs.SUPPLEMENTARY_SERVICE_CB_BAIC});
        when(mMockSscPreferenceHelper
                .updateCb(eq(condition), eq(expectedServiceClass), eq(expectedStatus)))
                .thenReturn(true);
        mSscServiceImpl.setSscPreferenceHelper(mMockSscPreferenceHelper);
        mSscServiceImpl.addTbSscChangeListener(mMockTbSscChangeListener);
        mLooper.processAllMessages();
        reset(mMockTbSscChangeListener);
        int tId = 71;

        mSscServiceImpl.updateCallBarringForServiceClass(
                tId, condition, action, null, serviceClass);
        mLooper.processAllMessages();

        verify(mMockSscPreferenceHelper)
                .updateCb(eq(condition), eq(expectedServiceClass), eq(expectedStatus));
        verify(mMockUtListener).utConfigurationUpdated(eq(tId));
        verify(mMockTbSscChangeListener)
                .onSupplementaryServiceConfigurationChanged(mCaptorSscs.capture());

        List<SupplementaryServiceConfiguration> capturedList = mCaptorSscs.getValue();
        assertEquals(1, capturedList.size());
        assertTrue(capturedList.getFirst() instanceof CbData);
        CbData cbData = (CbData) capturedList.getFirst();
        assertEquals(condition, cbData.getCondition());
        assertEquals(expectedServiceClass, cbData.getServiceClass());
        assertEquals(expectedStatus, cbData.getStatus());

        verifyNoMoreInteractions(mMockSscTransaction);
    }

    @Test
    public void testUpdateCallBarringTb_failure_doNotNotifySscChanged() {
        int condition = SscConstant.CONDITION_BOIC;
        int serviceClass = SscServiceClassUtil.SERVICE_CLASS_VOICE;
        int action = SscConstant.ACTION_ACTIVATION; // STATUS_ENABLE
        int expectedStatus = SscConstant.STATUS_ENABLE;
        when(mMockCarrierConfig.getIntArray(
                CarrierConfigManager.ImsSs.KEY_UT_TERMINAL_BASED_SERVICES_INT_ARRAY))
                .thenReturn(new int[]{
                        CarrierConfigManager.ImsSs.SUPPLEMENTARY_SERVICE_CB_BOIC});
        when(mMockSscPreferenceHelper.updateCb(eq(condition), eq(serviceClass), eq(expectedStatus)))
                .thenReturn(false);
        mSscServiceImpl.setSscPreferenceHelper(mMockSscPreferenceHelper);
        mSscServiceImpl.addTbSscChangeListener(mMockTbSscChangeListener);
        mLooper.processAllMessages();
        reset(mMockTbSscChangeListener);
        int tId = 72;

        mSscServiceImpl.updateCallBarringForServiceClass(
                tId, condition, action, null, serviceClass);
        mLooper.processAllMessages();

        verify(mMockSscPreferenceHelper)
                .updateCb(eq(condition), eq(serviceClass), eq(expectedStatus));
        verify(mMockUtListener).utConfigurationUpdateFailed(eq(tId), captorReasonInfo.capture());

        ImsReasonInfo reasonInfo = captorReasonInfo.getValue();
        assertNotNull(reasonInfo);
        assertEquals(ImsReasonInfo.CODE_LOCAL_INTERNAL_ERROR, reasonInfo.getCode());
        verify(mMockTbSscChangeListener, never()).onSupplementaryServiceConfigurationChanged(any());

        verifyNoMoreInteractions(mMockSscTransaction);
    }

    @Test
    public void testUpdateCallBarringTb_listenerNull_doNothing() {
        int condition = SscConstant.CONDITION_ACR;
        int serviceClass = SscServiceClassUtil.SERVICE_CLASS_VOICE;
        int action = SscConstant.ACTION_ACTIVATION;
        when(mMockCarrierConfig.getIntArray(
                CarrierConfigManager.ImsSs.KEY_UT_TERMINAL_BASED_SERVICES_INT_ARRAY))
                .thenReturn(new int[]{
                        CarrierConfigManager.ImsSs.SUPPLEMENTARY_SERVICE_CB_ACR});
        when(mMockSscPreferenceHelper.updateCb(anyInt(), anyInt(), anyInt())).thenReturn(true);
        mSscServiceImpl.setSscPreferenceHelper(mMockSscPreferenceHelper);
        mSscServiceImpl.setListener(null);
        mSscServiceImpl.addTbSscChangeListener(mMockTbSscChangeListener);
        mLooper.processAllMessages();
        reset(mMockTbSscChangeListener);
        int tId = 73;

        mSscServiceImpl.updateCallBarringForServiceClass(
                tId, condition, action, null, serviceClass);
        mLooper.processAllMessages();

        verify(mMockUtListener, never()).utConfigurationUpdated(anyInt());
        verify(mMockUtListener, never()).utConfigurationUpdateFailed(anyInt(), any());
        verify(mMockSscPreferenceHelper, never()).updateCb(anyInt(), anyInt(), anyInt());
        verify(mMockTbSscChangeListener, never()).onSupplementaryServiceConfigurationChanged(any());
        verifyNoMoreInteractions(mMockSscTransaction);
    }

    @Test
    public void testUpdateCallForward_singleRequest() {
        mIsCfnlRuleExist = false;
        int tId = 1;

        mSscServiceImpl.updateCallForward(tId, SscConstant.ACTION_REGISTRATION,
                SscConstant.CONDITION_CFNRC, "+1234567890", 0, 0);
        processEntireXmlDocQueryAsSuccess();
        processPutTransactionAsSuccess(ESsType.CF, SscConstant.EVENT_SSC_UPDATE_CF,
                SscConstant.CONDITION_CFNRC);

        mLooper.processAllMessages();
        verify(mMockUtListener).utConfigurationUpdated(eq(tId));
        verify(mMockSscTransaction).close();
        verifyNoMoreInteractions(mMockSscTransaction);
    }

    @Test
    public void testUpdateCallForward_insertRuleSet() {
        mIsCfRuleSetExist = false;

        when(mMockCarrierConfig.getBoolean(eq(CarrierConfig.ImsSs.KEY_UT_INSERT_NEW_RULE_BOOL)))
                .thenReturn(true);
        int tId = 1;

        mSscServiceImpl.updateCallForward(tId, SscConstant.ACTION_REGISTRATION,
                SscConstant.CONDITION_CFNR, "+1234567890", 0, 0);
        processEntireXmlDocQueryAsSuccess();
        processPutTransactionAsSuccess(ESsType.CF, SscConstant.EVENT_SSC_INSERT_CF,
                SscConstant.CONDITION_CFNR);

        mLooper.processAllMessages();
        verify(mMockUtListener).utConfigurationUpdated(eq(tId));
        verify(mMockSscTransaction).close();
        verifyNoMoreInteractions(mMockSscTransaction);
    }

    @Test
    public void testUpdateCallForward_insertRule() {
        mIsCfRuleSetExist = true;
        mIsCfbRuleExist = false;

        when(mMockCarrierConfig.getBoolean(eq(CarrierConfig.ImsSs.KEY_UT_INSERT_NEW_RULE_BOOL)))
                .thenReturn(true);
        int tId = 1;

        mSscServiceImpl.updateCallForward(tId, SscConstant.ACTION_ACTIVATION,
                SscConstant.CONDITION_CFB, null, 0, 0);
        processEntireXmlDocQueryAsSuccess();
        processPutTransactionAsSuccess(ESsType.CF, SscConstant.EVENT_SSC_INSERT_CF,
                SscConstant.CONDITION_CFB);

        mLooper.processAllMessages();
        verify(mMockUtListener).utConfigurationUpdated(eq(tId));
        verify(mMockSscTransaction).close();
        verifyNoMoreInteractions(mMockSscTransaction);
    }

    @Test
    public void testUpdateCallForward_notInsertRuleForCfnrTimer() {
        mIsCfRuleSetExist = false;

        when(mMockCarrierConfig.getBoolean(eq(CarrierConfig.ImsSs.KEY_UT_INSERT_NEW_RULE_BOOL)))
                .thenReturn(true);
        int tId = 1;

        mSscServiceImpl.updateCallForward(tId, SscConstant.ACTION_REGISTRATION,
                SscConstant.CONDITION_CFNR, "+1234567890", 0, 25);
        processEntireXmlDocQueryAsSuccess();
        processPutTransactionAsSuccess(ESsType.CF, SscConstant.EVENT_SSC_INSERT_CF,
                SscConstant.CONDITION_CFNR);
        processPutTransactionAsSuccess(ESsType.CF, SscConstant.EVENT_SSC_UPDATE_CF,
                SscConstant.CONDITION_CFNR_TIMER);

        mLooper.processAllMessages();
        verify(mMockUtListener).utConfigurationUpdated(eq(tId));
        verify(mMockSscTransaction).close();
        verifyNoMoreInteractions(mMockSscTransaction);
    }

    @Test
    public void testUpdateCallForward_CfnrcAndCfnl() {
        mIsCfnlRuleExist = true;

        when(mMockCarrierConfig.getBoolean(
                eq(CarrierConfig.ImsSs.KEY_UT_SUPPORT_CF_ACTION_ERASURE_BOOL))).thenReturn(true);
        int tId = 1;

        mSscServiceImpl.updateCallForward(tId, SscConstant.ACTION_ERASURE,
                SscConstant.CONDITION_CFNRC, null, 0, 0);
        processEntireXmlDocQueryAsSuccess();
        processPutTransactionAsSuccess(ESsType.CF, SscConstant.EVENT_SSC_UPDATE_CF,
                SscConstant.CONDITION_CFNRC);
        processPutTransactionAsSuccess(ESsType.CF, SscConstant.EVENT_SSC_UPDATE_CF,
                SscConstant.CONDITION_CFNL);

        mLooper.processAllMessages();
        verify(mMockUtListener).utConfigurationUpdated(eq(tId));
        verify(mMockSscTransaction).close();
        verifyNoMoreInteractions(mMockSscTransaction);
    }

    @Test
    public void testUpdateCallForward_CfnrAndTimer() {
        mIsTimerInCfnr = false;
        int tId = 1;

        mSscServiceImpl.updateCallForward(tId, SscConstant.ACTION_ACTIVATION,
                SscConstant.CONDITION_CFNR, null, 0, 20);
        processEntireXmlDocQueryAsSuccess();
        processPutTransactionAsSuccess(ESsType.CF, SscConstant.EVENT_SSC_UPDATE_CF,
                SscConstant.CONDITION_CFNR);
        processPutTransactionAsSuccess(ESsType.CF, SscConstant.EVENT_SSC_UPDATE_CF,
                SscConstant.CONDITION_CFNR_TIMER);

        mLooper.processAllMessages();
        verify(mMockUtListener).utConfigurationUpdated(eq(tId));
        verify(mMockSscTransaction).close();
        verifyNoMoreInteractions(mMockSscTransaction);
    }

    @Test
    public void testUpdateCallForward_Cfnr_CfnrTimerNotSupported() {
        mIsTimerInCfnr = false;
        int tId = 1;
        when(mMockCarrierConfig.getBoolean(
                eq(CarrierConfig.ImsSs.KEY_UT_SUPPORT_CFNR_TIMER_BOOL))).thenReturn(false);

        mSscServiceImpl.updateCallForward(tId, SscConstant.ACTION_ACTIVATION,
                SscConstant.CONDITION_CFNR, null, 0, 20);
        processEntireXmlDocQueryAsSuccess();
        processPutTransactionAsSuccess(ESsType.CF, SscConstant.EVENT_SSC_UPDATE_CF,
                SscConstant.CONDITION_CFNR);

        mLooper.processAllMessages();
        verify(mMockUtListener).utConfigurationUpdated(eq(tId));
        verify(mMockSscTransaction).close();
        verifyNoMoreInteractions(mMockSscTransaction);
    }

    @Test
    public void testUpdateCallForward_CfnrWithTimer() {
        mIsTimerInCfnr = true;
        int tId = 1;

        mSscServiceImpl.updateCallForward(tId, SscConstant.ACTION_ACTIVATION,
                SscConstant.CONDITION_CFNR, null, 0, 20);
        processEntireXmlDocQueryAsSuccess();
        processPutTransactionAsSuccess(ESsType.CF, SscConstant.EVENT_SSC_UPDATE_CF,
                SscConstant.CONDITION_CFNR);

        mLooper.processAllMessages();
        verify(mMockUtListener).utConfigurationUpdated(eq(tId));
        verify(mMockSscTransaction).close();
        verifyNoMoreInteractions(mMockSscTransaction);
    }

    @Test
    public void testUpdateCallForward_CfnrWithTimer_CfnrTimerNotSupported() {
        mIsTimerInCfnr = true;
        int tId = 1;
        when(mMockCarrierConfig.getBoolean(
                eq(CarrierConfig.ImsSs.KEY_UT_SUPPORT_CFNR_TIMER_BOOL))).thenReturn(false);

        mSscServiceImpl.updateCallForward(tId, SscConstant.ACTION_ACTIVATION,
                SscConstant.CONDITION_CFNR, null, 0, 20);
        processEntireXmlDocQueryAsSuccess();
        processPutTransactionAsSuccess(ESsType.CF, SscConstant.EVENT_SSC_UPDATE_CF,
                SscConstant.CONDITION_CFNR);

        mLooper.processAllMessages();
        verify(mMockUtListener).utConfigurationUpdated(eq(tId));
        verify(mMockSscTransaction).close();
        verifyNoMoreInteractions(mMockSscTransaction);
    }

    @Test
    public void testUpdateCallForward_CfAll() {
        mIsTimerInCfnr = true;
        mIsCfnlRuleExist = false;
        int tId = 1;

        mSscServiceImpl.updateCallForward(tId, SscConstant.ACTION_DEACTIVATION,
                SscConstant.CONDITION_CFA, null, 0, 20);

        for (int c = SscConstant.CONDITION_CFNRC; c >= SscConstant.CONDITION_CFU; c--) {
            if (!SscXmlGov.getInstance(SLOT_0).isXmlDataPresent()) {
                processEntireXmlDocQueryAsSuccess();
            }

            processPutTransactionAsSuccess(ESsType.CF, SscConstant.EVENT_SSC_UPDATE_CF,
                    c);
        }

        mLooper.processAllMessages();
        verify(mMockUtListener).utConfigurationUpdated(eq(tId));
        verify(mMockSscTransaction).close();
        verifyNoMoreInteractions(mMockSscTransaction);
    }

    @Test
    public void testUpdateCallForward_entireDocumentQueryFailure() {
        int tId = 1;

        mSscServiceImpl.updateCallForward(tId, SscConstant.ACTION_DEACTIVATION,
                SscConstant.CONDITION_CFA, null, 0, 20);
        processEntireXmlDocQueryAsFailure();

        mLooper.processAllMessages();
        verify(mMockUtListener).utConfigurationUpdateFailed(eq(tId), any());
        verify(mMockSscTransaction).close();
        verifyNoMoreInteractions(mMockSscTransaction);
    }

    @Test
    public void testUpdateCallForward_updateFailure() {
        int tId = 1;

        mSscServiceImpl.updateCallForward(tId, SscConstant.ACTION_ACTIVATION,
                SscConstant.CONDITION_CFB, null, 0, 20);
        processEntireXmlDocQueryAsSuccess();
        processPutTransactionAsFailure(ESsType.CF, SscConstant.EVENT_SSC_UPDATE_CF,
                SscConstant.CONDITION_CFB);

        mLooper.processAllMessages();
        verify(mMockUtListener).utConfigurationUpdateFailed(eq(tId), any());
        verify(mMockSscTransaction).close();
        verifyNoMoreInteractions(mMockSscTransaction);
    }

    @Test
    public void testUpdateCallForward_erasureNotSupported() {
        when(mMockCarrierConfig.getBoolean(
                eq(CarrierConfig.ImsSs.KEY_UT_SUPPORT_CF_ACTION_ERASURE_BOOL))).thenReturn(false);

        int tId = 1;

        mSscServiceImpl.updateCallForward(tId, SscConstant.ACTION_ERASURE,
                SscConstant.CONDITION_CFB, null, 0, 20);

        processEntireXmlDocQueryAsSuccess();
        processPutTransactionAsSuccess(ESsType.CF, SscConstant.EVENT_SSC_UPDATE_CF,
                SscConstant.CONDITION_CFB);

        mLooper.processAllMessages();
        verify(mMockUtListener).utConfigurationUpdated(eq(tId));
        verify(mMockSscTransaction).close();
        verifyNoMoreInteractions(mMockSscTransaction);
    }

    @Test
    public void testUpdateCallForward_notSupportedService() {
        when(mMockCarrierConfig.getIntArray(
                CarrierConfigManager.ImsSs.KEY_UT_SERVER_BASED_SERVICES_INT_ARRAY))
                .thenReturn(new int[] {});
        int tId = 1;

        mSscServiceImpl.updateCallForward(tId, SscConstant.ACTION_DEACTIVATION,
                SscConstant.CONDITION_CFA, null, 0, 20);

        mLooper.processAllMessages();
        verify(mMockUtListener).utConfigurationUpdateFailed(eq(tId), captorReasonInfo.capture());
        verifyNoMoreInteractions(mMockSscTransaction);

        ImsReasonInfo reasonInfo = captorReasonInfo.getValue();
        assertNotNull(reasonInfo);
        assertEquals(ImsReasonInfo.CODE_UT_OPERATION_NOT_ALLOWED, reasonInfo.getCode());
    }

    @Test
    public void testUpdateCallForward_invalidAction() {
        int tId = 1;
        int invalidAction = SscConstant.ACTION_ERASURE + 1;

        mSscServiceImpl.updateCallForward(tId, invalidAction,
                SscConstant.CONDITION_CFB, null, 0, 20);

        mLooper.processAllMessages();
        verify(mMockUtListener).utConfigurationUpdateFailed(eq(tId), captorReasonInfo.capture());
        verifyNoMoreInteractions(mMockSscTransaction);

        ImsReasonInfo reasonInfo = captorReasonInfo.getValue();
        assertNotNull(reasonInfo);
        assertEquals(ImsReasonInfo.CODE_UT_OPERATION_NOT_ALLOWED, reasonInfo.getCode());
    }

    @Test
    public void testUpdateCallForward_invalidTimer() {
        int tId = 1;
        int invalidTimer = SscConstant.CFNR_TIMER_MAX + 1;
        when(mMockCarrierConfig.getBoolean(
                eq(CarrierConfig.ImsSs.KEY_UT_SUPPORT_CF_ACTION_ERASURE_BOOL))).thenReturn(true);

        mSscServiceImpl.updateCallForward(tId, SscConstant.ACTION_REGISTRATION,
                SscConstant.CONDITION_CFB, null, 0, invalidTimer);

        mLooper.processAllMessages();
        verify(mMockUtListener).utConfigurationUpdateFailed(eq(tId), captorReasonInfo.capture());
        verifyNoMoreInteractions(mMockSscTransaction);

        ImsReasonInfo reasonInfo = captorReasonInfo.getValue();
        assertNotNull(reasonInfo);
        assertEquals(ImsReasonInfo.CODE_UT_OPERATION_NOT_ALLOWED, reasonInfo.getCode());
    }

    @Test
    public void testUpdateCallWaiting_success() {
        int tId = 1;

        mSscServiceImpl.updateCallWaiting(tId, true, SscServiceClassUtil.SERVICE_CLASS_VOICE);
        processEntireXmlDocQueryAsSuccess();
        processPutTransactionAsSuccess(ESsType.CW, SscConstant.EVENT_SSC_UPDATE_CW, -1);

        mLooper.processAllMessages();
        verify(mMockUtListener).utConfigurationUpdated(eq(tId));
        verify(mMockSscTransaction).close();
    }

    @Test
    public void testUpdateCallWaiting_failure() {
        int tId = 1;

        mSscServiceImpl.updateCallWaiting(tId, true, SscServiceClassUtil.SERVICE_CLASS_VOICE);
        processEntireXmlDocQueryAsSuccess();
        processPutTransactionAsFailure(ESsType.CW, SscConstant.EVENT_SSC_UPDATE_CW, -1);

        mLooper.processAllMessages();
        verify(mMockUtListener).utConfigurationUpdateFailed(eq(tId), any());
        verify(mMockSscTransaction).close();
    }

    @Test
    public void testUpdateCallWaiting_notSupportedService() {
        when(mMockCarrierConfig.getIntArray(
                CarrierConfigManager.ImsSs.KEY_UT_SERVER_BASED_SERVICES_INT_ARRAY))
                .thenReturn(new int[] {});
        int tId = 1;

        mSscServiceImpl.updateCallWaiting(tId, true, SscServiceClassUtil.SERVICE_CLASS_VOICE);

        mLooper.processAllMessages();
        verify(mMockUtListener).utConfigurationUpdateFailed(eq(tId), captorReasonInfo.capture());
        verifyNoMoreInteractions(mMockSscTransaction);

        ImsReasonInfo reasonInfo = captorReasonInfo.getValue();
        assertNotNull(reasonInfo);
        assertEquals(ImsReasonInfo.CODE_UT_OPERATION_NOT_ALLOWED, reasonInfo.getCode());
    }

    @Test
    public void testUpdateClir_success() {
        int tId = 1;

        mSscServiceImpl.updateCLIR(tId, SscConstant.OIR_INVOCATION);
        processEntireXmlDocQueryAsSuccess();
        processPutTransactionAsSuccess(ESsType.OIR, SscConstant.EVENT_SSC_UPDATE_OIR, -1);

        mLooper.processAllMessages();
        verify(mMockUtListener).utConfigurationUpdated(eq(tId));
        verify(mMockSscTransaction).close();
    }

    @Test
    public void testUpdateClir_failure() {
        int tId = 1;

        mSscServiceImpl.updateCLIR(tId, SscConstant.OIR_INVOCATION);
        processEntireXmlDocQueryAsSuccess();
        processPutTransactionAsFailure(ESsType.OIR, SscConstant.EVENT_SSC_UPDATE_OIR, -1);

        mLooper.processAllMessages();
        verify(mMockUtListener).utConfigurationUpdateFailed(eq(tId), any());
        verify(mMockSscTransaction).close();
    }

    @Test
    public void testUpdateClir_notSupportedService() {
        when(mMockCarrierConfig.getIntArray(
                CarrierConfigManager.ImsSs.KEY_UT_SERVER_BASED_SERVICES_INT_ARRAY))
                .thenReturn(new int[] {});
        int tId = 1;

        mSscServiceImpl.updateCLIR(tId, SscConstant.OIR_INVOCATION);

        mLooper.processAllMessages();
        verify(mMockUtListener).utConfigurationUpdateFailed(eq(tId), captorReasonInfo.capture());
        verifyNoMoreInteractions(mMockSscTransaction);

        ImsReasonInfo reasonInfo = captorReasonInfo.getValue();
        assertNotNull(reasonInfo);
        assertEquals(ImsReasonInfo.CODE_UT_OPERATION_NOT_ALLOWED, reasonInfo.getCode());
    }

    @Test
    public void testUpdateClirTb_success_notifySscChanged() {
        when(mMockCarrierConfig.getIntArray(
                CarrierConfigManager.ImsSs.KEY_UT_TERMINAL_BASED_SERVICES_INT_ARRAY))
                .thenReturn(new int[]{
                        CarrierConfigManager.ImsSs.SUPPLEMENTARY_SERVICE_IDENTIFICATION_OIR});
        when(mMockSscPreferenceHelper.updateOir(anyInt())).thenReturn(true);
        mSscServiceImpl.setSscPreferenceHelper(mMockSscPreferenceHelper);
        mSscServiceImpl.addTbSscChangeListener(mMockTbSscChangeListener);
        mLooper.processAllMessages();
        reset(mMockTbSscChangeListener);
        int tId = 1;

        mSscServiceImpl.updateCLIR(tId, SscConstant.OIR_INVOCATION);
        mLooper.processAllMessages();

        verify(mMockUtListener).utConfigurationUpdated(eq(tId));
        verify(mMockTbSscChangeListener)
                .onSupplementaryServiceConfigurationChanged(mCaptorSscs.capture());

        List<SupplementaryServiceConfiguration> capturedList = mCaptorSscs.getValue();
        assertEquals(1, capturedList.size());
        assertTrue(capturedList.getFirst() instanceof OirData);
        OirData oirData = (OirData) capturedList.getFirst();
        assertEquals(SscConstant.STATUS_ENABLE, oirData.getStatus());

        verifyNoMoreInteractions(mMockSscTransaction);
    }

    @Test
    public void testUpdateClirTb_csInServiceHome_successAndSyncWithCs() {
        when(mMockPhoneStateInterface.getCsNetworkRegistrationState())
                .thenReturn(NetworkRegistrationInfo.REGISTRATION_STATE_HOME);
        when(mMockCarrierConfig.getIntArray(
                CarrierConfigManager.ImsSs.KEY_UT_TERMINAL_BASED_SERVICES_INT_ARRAY))
                .thenReturn(new int[]{
                        CarrierConfigManager.ImsSs.SUPPLEMENTARY_SERVICE_IDENTIFICATION_OIR});
        when(mMockCarrierConfig.getBoolean(
                eq(CarrierConfig.ImsSs.KEY_UT_SYNC_WITH_CS_FOR_TB_SS_BOOL))).thenReturn(true);
        when(mMockSscPreferenceHelper.updateOir(anyInt())).thenReturn(true);
        mSscServiceImpl.setSscPreferenceHelper(mMockSscPreferenceHelper);
        int tId = 1;

        mSscServiceImpl.updateCLIR(tId, SscConstant.OIR_INVOCATION);
        mLooper.processAllMessages();

        verify(mMockUtListener).utConfigurationUpdateFailed(eq(tId), captorReasonInfo.capture());

        ImsReasonInfo reasonInfo = captorReasonInfo.getValue();
        assertNotNull(reasonInfo);
        assertEquals(ImsReasonInfo.CODE_LOCAL_CALL_CS_RETRY_REQUIRED, reasonInfo.getCode());

        verifyNoMoreInteractions(mMockSscTransaction);
    }

    @Test
    public void testUpdateClirTb_csInServiceRoaming_successAndSyncWithCs() {
        when(mMockPhoneStateInterface.getCsNetworkRegistrationState())
                .thenReturn(NetworkRegistrationInfo.REGISTRATION_STATE_HOME);
        when(mMockCarrierConfig.getIntArray(
                CarrierConfigManager.ImsSs.KEY_UT_TERMINAL_BASED_SERVICES_INT_ARRAY))
                .thenReturn(new int[]{
                        CarrierConfigManager.ImsSs.SUPPLEMENTARY_SERVICE_IDENTIFICATION_OIR});
        when(mMockCarrierConfig.getBoolean(
                eq(CarrierConfig.ImsSs.KEY_UT_SYNC_WITH_CS_FOR_TB_SS_BOOL))).thenReturn(true);
        when(mMockSscPreferenceHelper.updateOir(anyInt())).thenReturn(true);
        mSscServiceImpl.setSscPreferenceHelper(mMockSscPreferenceHelper);
        int tId = 1;

        mSscServiceImpl.updateCLIR(tId, SscConstant.OIR_INVOCATION);
        mLooper.processAllMessages();

        verify(mMockUtListener).utConfigurationUpdateFailed(eq(tId), captorReasonInfo.capture());

        ImsReasonInfo reasonInfo = captorReasonInfo.getValue();
        assertNotNull(reasonInfo);
        assertEquals(ImsReasonInfo.CODE_LOCAL_CALL_CS_RETRY_REQUIRED, reasonInfo.getCode());

        verifyNoMoreInteractions(mMockSscTransaction);
    }

    @Test
    public void testUpdateClirTb_csNotInService_successAndNotSyncWithCs() {
        when(mMockPhoneStateInterface.getCsNetworkRegistrationState())
                .thenReturn(NetworkRegistrationInfo.REGISTRATION_STATE_NOT_REGISTERED_OR_SEARCHING);
        when(mMockCarrierConfig.getIntArray(
                CarrierConfigManager.ImsSs.KEY_UT_TERMINAL_BASED_SERVICES_INT_ARRAY))
                .thenReturn(new int[]{
                        CarrierConfigManager.ImsSs.SUPPLEMENTARY_SERVICE_IDENTIFICATION_OIR});
        when(mMockCarrierConfig.getBoolean(
                eq(CarrierConfig.ImsSs.KEY_UT_SYNC_WITH_CS_FOR_TB_SS_BOOL))).thenReturn(true);
        when(mMockSscPreferenceHelper.updateOir(anyInt())).thenReturn(true);
        mSscServiceImpl.setSscPreferenceHelper(mMockSscPreferenceHelper);
        int tId = 1;

        mSscServiceImpl.updateCLIR(tId, SscConstant.OIR_INVOCATION);
        mLooper.processAllMessages();

        verify(mMockUtListener).utConfigurationUpdated(eq(tId));
        verifyNoMoreInteractions(mMockSscTransaction);
    }

    @Test
    public void testUpdateClirTb_failure_doNotNotifySscChanged() {
        when(mMockCarrierConfig.getIntArray(
                CarrierConfigManager.ImsSs.KEY_UT_TERMINAL_BASED_SERVICES_INT_ARRAY))
                .thenReturn(new int[]{
                        CarrierConfigManager.ImsSs.SUPPLEMENTARY_SERVICE_IDENTIFICATION_OIR});
        when(mMockSscPreferenceHelper.updateOir(anyInt())).thenReturn(false);
        mSscServiceImpl.setSscPreferenceHelper(mMockSscPreferenceHelper);
        mSscServiceImpl.addTbSscChangeListener(mMockTbSscChangeListener);
        mLooper.processAllMessages();
        reset(mMockTbSscChangeListener);
        int tId = 1;

        mSscServiceImpl.updateCLIR(tId, SscConstant.OIR_INVOCATION);
        mLooper.processAllMessages();

        verify(mMockUtListener).utConfigurationUpdateFailed(eq(tId), captorReasonInfo.capture());
        ImsReasonInfo reasonInfo = captorReasonInfo.getValue();
        assertNotNull(reasonInfo);
        assertEquals(ImsReasonInfo.CODE_LOCAL_INTERNAL_ERROR, reasonInfo.getCode());
        verify(mMockTbSscChangeListener, never()).onSupplementaryServiceConfigurationChanged(any());
        verifyNoMoreInteractions(mMockSscTransaction);
    }

    @Test
    public void testUpdateClirTb_listenerNull_doNothing() {
        when(mMockCarrierConfig.getIntArray(
                CarrierConfigManager.ImsSs.KEY_UT_TERMINAL_BASED_SERVICES_INT_ARRAY))
                .thenReturn(new int[]{
                        CarrierConfigManager.ImsSs.SUPPLEMENTARY_SERVICE_IDENTIFICATION_OIR});
        mSscServiceImpl.setSscPreferenceHelper(mMockSscPreferenceHelper);
        mSscServiceImpl.setListener(null);
        int tId = 1;

        mSscServiceImpl.updateCLIR(tId, SscConstant.OIR_INVOCATION);
        mLooper.processAllMessages();

        verify(mMockUtListener, never()).utConfigurationUpdated(anyInt());
        verify(mMockUtListener, never()).utConfigurationUpdateFailed(anyInt(), any());
        verify(mMockSscPreferenceHelper, never()).updateOir(anyInt());
        verify(mMockTbSscChangeListener, never()).onSupplementaryServiceConfigurationChanged(any());
        verifyNoMoreInteractions(mMockSscTransaction);
    }

    @Test
    public void testUpdateClir_noRequestToSeverAndInvocation_requestToNetwork() {
        when(mMockCarrierConfig.getInt(
                CarrierConfig.ImsSs.KEY_UT_OIR_NETWORK_DEFAULT_OPERATION_INT))
                .thenReturn(SscConfig.OIR_NO_REQUEST_TO_SERVER);
        when(mMockSscPreferenceHelper.updateOir(anyInt())).thenReturn(true);
        mSscServiceImpl.setSscPreferenceHelper(mMockSscPreferenceHelper);
        int tId = 1;

        mSscServiceImpl.updateCLIR(tId, SscConstant.OIR_INVOCATION);
        processEntireXmlDocQueryAsSuccess();
        processPutTransactionAsSuccess(ESsType.OIR, SscConstant.EVENT_SSC_UPDATE_OIR, -1);

        mLooper.processAllMessages();
        verify(mMockUtListener).utConfigurationUpdated(eq(tId));
        verify(mMockSscTransaction).close();
    }

    @Test
    public void testUpdateClir_noRequestToSeverAndDefault_noRequestToNetworkAndLocalUpdate() {
        when(mMockCarrierConfig.getInt(
                CarrierConfig.ImsSs.KEY_UT_OIR_NETWORK_DEFAULT_OPERATION_INT))
                .thenReturn(SscConfig.OIR_NO_REQUEST_TO_SERVER);
        when(mMockSscPreferenceHelper.updateOir(SscConstant.OIR_DEFAULT)).thenReturn(true);
        mSscServiceImpl.setSscPreferenceHelper(mMockSscPreferenceHelper);

        int tId = 1;
        mSscServiceImpl.updateCLIR(tId, SscConstant.OIR_DEFAULT);
        mLooper.processAllMessages();

        verify(mMockUtListener).utConfigurationUpdated(eq(tId));
        verifyNoMoreInteractions(mMockSscTransaction);
    }

    @Test
    public void testUpdateClip_success() {
        int tId = 1;

        mSscServiceImpl.updateCLIP(tId, true);
        processEntireXmlDocQueryAsSuccess();
        processPutTransactionAsSuccess(ESsType.OIP, SscConstant.EVENT_SSC_UPDATE_OIP, -1);

        mLooper.processAllMessages();
        verify(mMockUtListener).utConfigurationUpdated(eq(tId));
        verify(mMockSscTransaction).close();
    }

    @Test
    public void testUpdateClip_failure() {
        int tId = 1;

        mSscServiceImpl.updateCLIP(tId, true);
        processEntireXmlDocQueryAsSuccess();
        processPutTransactionAsFailure(ESsType.OIP, SscConstant.EVENT_SSC_UPDATE_OIP, -1);

        mLooper.processAllMessages();
        verify(mMockUtListener).utConfigurationUpdateFailed(eq(tId), any());
        verify(mMockSscTransaction).close();
    }

    @Test
    public void testUpdateClip_notSupportedService() {
        when(mMockCarrierConfig.getIntArray(
                CarrierConfigManager.ImsSs.KEY_UT_SERVER_BASED_SERVICES_INT_ARRAY))
                .thenReturn(new int[] {});
        int tId = 1;

        mSscServiceImpl.updateCLIP(tId, true);

        mLooper.processAllMessages();
        verify(mMockUtListener).utConfigurationUpdateFailed(eq(tId), captorReasonInfo.capture());
        verifyNoMoreInteractions(mMockSscTransaction);

        ImsReasonInfo reasonInfo = captorReasonInfo.getValue();
        assertNotNull(reasonInfo);
        assertEquals(ImsReasonInfo.CODE_UT_OPERATION_NOT_ALLOWED, reasonInfo.getCode());
    }

    @Test
    public void testUpdateClipTb_success_notifySscChanged() {
        when(mMockCarrierConfig.getIntArray(
                CarrierConfigManager.ImsSs.KEY_UT_TERMINAL_BASED_SERVICES_INT_ARRAY))
                .thenReturn(new int[]{
                        CarrierConfigManager.ImsSs.SUPPLEMENTARY_SERVICE_IDENTIFICATION_OIP});
        when(mMockSscPreferenceHelper.updateOip(eq(SscConstant.STATUS_ENABLE))).thenReturn(true);
        mSscServiceImpl.setSscPreferenceHelper(mMockSscPreferenceHelper);
        mSscServiceImpl.addTbSscChangeListener(mMockTbSscChangeListener);
        mLooper.processAllMessages();
        reset(mMockTbSscChangeListener);
        int tId = 40;

        mSscServiceImpl.updateCLIP(tId, true);
        mLooper.processAllMessages();

        verify(mMockSscPreferenceHelper).updateOip(eq(SscConstant.STATUS_ENABLE));
        verify(mMockUtListener).utConfigurationUpdated(eq(tId));
        verify(mMockTbSscChangeListener)
                .onSupplementaryServiceConfigurationChanged(mCaptorSscs.capture());

        List<SupplementaryServiceConfiguration> capturedList = mCaptorSscs.getValue();
        assertEquals(1, capturedList.size());
        assertTrue(capturedList.getFirst() instanceof OipData);
        OipData oipData = (OipData) capturedList.getFirst();
        assertEquals(SscConstant.STATUS_ENABLE, oipData.getStatus());

        verifyNoMoreInteractions(mMockSscTransaction);
    }

    @Test
    public void testUpdateClipTb_failure_doNotNotifySscChanged() {
        when(mMockCarrierConfig.getIntArray(
                CarrierConfigManager.ImsSs.KEY_UT_TERMINAL_BASED_SERVICES_INT_ARRAY))
                .thenReturn(new int[]{
                        CarrierConfigManager.ImsSs.SUPPLEMENTARY_SERVICE_IDENTIFICATION_OIP});
        when(mMockSscPreferenceHelper.updateOip(eq(SscConstant.STATUS_DISABLE))).thenReturn(false);
        mSscServiceImpl.setSscPreferenceHelper(mMockSscPreferenceHelper);
        mSscServiceImpl.addTbSscChangeListener(mMockTbSscChangeListener);
        mLooper.processAllMessages();
        reset(mMockTbSscChangeListener);
        int tId = 41;

        mSscServiceImpl.updateCLIP(tId, false);
        mLooper.processAllMessages();

        verify(mMockSscPreferenceHelper).updateOip(eq(SscConstant.STATUS_DISABLE));
        verify(mMockUtListener).utConfigurationUpdateFailed(eq(tId), captorReasonInfo.capture());

        ImsReasonInfo reasonInfo = captorReasonInfo.getValue();
        assertNotNull(reasonInfo);
        assertEquals(ImsReasonInfo.CODE_LOCAL_INTERNAL_ERROR, reasonInfo.getCode());

        verify(mMockTbSscChangeListener, never()).onSupplementaryServiceConfigurationChanged(any());
        verifyNoMoreInteractions(mMockSscTransaction);
    }

    @Test
    public void testUpdateClipTb_listenerNull_doNothing() {
        when(mMockCarrierConfig.getIntArray(
                CarrierConfigManager.ImsSs.KEY_UT_TERMINAL_BASED_SERVICES_INT_ARRAY))
                .thenReturn(new int[]{
                        CarrierConfigManager.ImsSs.SUPPLEMENTARY_SERVICE_IDENTIFICATION_OIP});
        mSscServiceImpl.setSscPreferenceHelper(mMockSscPreferenceHelper);
        mSscServiceImpl.setListener(null);
        int tId = 42;

        mSscServiceImpl.updateCLIP(tId, true);
        mLooper.processAllMessages();

        verify(mMockUtListener, never()).utConfigurationUpdated(anyInt());
        verify(mMockUtListener, never()).utConfigurationUpdateFailed(anyInt(), any());
        verify(mMockSscPreferenceHelper, never()).updateOip(anyInt());
        verify(mMockTbSscChangeListener, never()).onSupplementaryServiceConfigurationChanged(any());
        verifyNoMoreInteractions(mMockSscTransaction);
    }

    @Test
    public void testUpdateColr_success() {
        int tId = 1;

        mSscServiceImpl.updateCOLR(tId, SscConstant.TIR_PROVISIONED);
        processEntireXmlDocQueryAsSuccess();
        processPutTransactionAsSuccess(ESsType.TIR, SscConstant.EVENT_SSC_UPDATE_TIR, -1);

        mLooper.processAllMessages();
        verify(mMockUtListener).utConfigurationUpdated(eq(tId));
        verify(mMockSscTransaction).close();
    }

    @Test
    public void testUpdateColr_failure() {
        int tId = 1;

        mSscServiceImpl.updateCOLR(tId, SscConstant.TIR_PROVISIONED);
        processEntireXmlDocQueryAsSuccess();
        processPutTransactionAsFailure(ESsType.TIR, SscConstant.EVENT_SSC_UPDATE_TIR, -1);

        mLooper.processAllMessages();
        verify(mMockUtListener).utConfigurationUpdateFailed(eq(tId), any());
        verify(mMockSscTransaction).close();
    }

    @Test
    public void testUpdateColr_notSupportedService() {
        when(mMockCarrierConfig.getIntArray(
                CarrierConfigManager.ImsSs.KEY_UT_SERVER_BASED_SERVICES_INT_ARRAY))
                .thenReturn(new int[] {});
        int tId = 1;

        mSscServiceImpl.updateCOLR(tId, SscConstant.TIR_PROVISIONED);

        mLooper.processAllMessages();
        verify(mMockUtListener).utConfigurationUpdateFailed(eq(tId), captorReasonInfo.capture());
        verifyNoMoreInteractions(mMockSscTransaction);

        ImsReasonInfo reasonInfo = captorReasonInfo.getValue();
        assertNotNull(reasonInfo);
        assertEquals(ImsReasonInfo.CODE_UT_OPERATION_NOT_ALLOWED, reasonInfo.getCode());
    }

    @Test
    public void testUpdateColrTb_success_notifySscChanged() {
        int presentation = SscConstant.TIR_PROVISIONED;
        int expectedStatus = SscConstant.STATUS_ENABLE; // TIR_PROVISIONED maps to STATUS_ENABLE.

        when(mMockCarrierConfig.getIntArray(
                CarrierConfigManager.ImsSs.KEY_UT_TERMINAL_BASED_SERVICES_INT_ARRAY))
                .thenReturn(new int[]{
                        CarrierConfigManager.ImsSs.SUPPLEMENTARY_SERVICE_IDENTIFICATION_TIR});
        when(mMockSscPreferenceHelper.updateTir(eq(expectedStatus))).thenReturn(true);
        mSscServiceImpl.setSscPreferenceHelper(mMockSscPreferenceHelper);
        mSscServiceImpl.addTbSscChangeListener(mMockTbSscChangeListener);
        mLooper.processAllMessages();
        reset(mMockTbSscChangeListener);
        int tId = 50;

        mSscServiceImpl.updateCOLR(tId, presentation);
        mLooper.processAllMessages();

        verify(mMockSscPreferenceHelper).updateTir(eq(presentation));
        verify(mMockUtListener).utConfigurationUpdated(eq(tId));
        verify(mMockTbSscChangeListener)
                .onSupplementaryServiceConfigurationChanged(mCaptorSscs.capture());

        List<SupplementaryServiceConfiguration> capturedList = mCaptorSscs.getValue();
        assertEquals(1, capturedList.size());
        assertTrue(capturedList.getFirst() instanceof TirData);
        TirData tirData = (TirData) capturedList.getFirst();
        assertEquals(expectedStatus, tirData.getStatus());

        verifyNoMoreInteractions(mMockSscTransaction);
    }

    @Test
    public void testUpdateColrTb_failure_doNotNotifySscChanged() {
        int presentation = SscConstant.TIR_NOT_PROVISIONED;
        int expectedStatus = SscConstant.STATUS_DISABLE; // STATUS_DISABLE.
        when(mMockCarrierConfig.getIntArray(
                CarrierConfigManager.ImsSs.KEY_UT_TERMINAL_BASED_SERVICES_INT_ARRAY))
                .thenReturn(new int[]{
                        CarrierConfigManager.ImsSs.SUPPLEMENTARY_SERVICE_IDENTIFICATION_TIR});
        when(mMockSscPreferenceHelper.updateTir(eq(expectedStatus))).thenReturn(false);
        mSscServiceImpl.setSscPreferenceHelper(mMockSscPreferenceHelper);
        mSscServiceImpl.addTbSscChangeListener(mMockTbSscChangeListener);
        mLooper.processAllMessages();
        reset(mMockTbSscChangeListener);
        int tId = 51;

        mSscServiceImpl.updateCOLR(tId, presentation);
        mLooper.processAllMessages();

        verify(mMockSscPreferenceHelper).updateTir(eq(presentation));
        verify(mMockUtListener).utConfigurationUpdateFailed(eq(tId), captorReasonInfo.capture());

        ImsReasonInfo reasonInfo = captorReasonInfo.getValue();
        assertNotNull(reasonInfo);
        assertEquals(ImsReasonInfo.CODE_LOCAL_INTERNAL_ERROR, reasonInfo.getCode());

        verify(mMockTbSscChangeListener, never()).onSupplementaryServiceConfigurationChanged(any());
        verifyNoMoreInteractions(mMockSscTransaction);
    }

    @Test
    public void testUpdateColrTb_listenerNull_doNothing() {
        when(mMockCarrierConfig.getIntArray(
                CarrierConfigManager.ImsSs.KEY_UT_TERMINAL_BASED_SERVICES_INT_ARRAY))
                .thenReturn(new int[]{
                        CarrierConfigManager.ImsSs.SUPPLEMENTARY_SERVICE_IDENTIFICATION_TIR});
        mSscServiceImpl.setSscPreferenceHelper(mMockSscPreferenceHelper);
        mSscServiceImpl.setListener(null);
        int tId = 52;

        mSscServiceImpl.updateCOLR(tId, SscConstant.TIR_NOT_PROVISIONED);
        mLooper.processAllMessages();

        verify(mMockUtListener, never()).utConfigurationUpdated(anyInt());
        verify(mMockUtListener, never()).utConfigurationUpdateFailed(anyInt(), any());
        verify(mMockSscPreferenceHelper, never()).updateTir(anyInt());
        verify(mMockTbSscChangeListener, never()).onSupplementaryServiceConfigurationChanged(any());
        verifyNoMoreInteractions(mMockSscTransaction);
    }

    @Test
    public void testUpdateColp_success() {
        int tId = 1;

        mSscServiceImpl.updateCOLP(tId, true);
        processEntireXmlDocQueryAsSuccess();
        processPutTransactionAsSuccess(ESsType.TIP, SscConstant.EVENT_SSC_UPDATE_TIP, -1);

        mLooper.processAllMessages();
        verify(mMockUtListener).utConfigurationUpdated(eq(tId));
        verify(mMockSscTransaction).close();
    }

    @Test
    public void testUpdateColp_failure() {
        int tId = 1;

        mSscServiceImpl.updateCOLP(tId, true);
        processEntireXmlDocQueryAsSuccess();
        processPutTransactionAsFailure(ESsType.TIP, SscConstant.EVENT_SSC_UPDATE_TIP, -1);

        mLooper.processAllMessages();
        verify(mMockUtListener).utConfigurationUpdateFailed(eq(tId), any());
        verify(mMockSscTransaction).close();
    }

    @Test
    public void testUpdateColp_notSupportedService() {
        when(mMockCarrierConfig.getIntArray(
                CarrierConfigManager.ImsSs.KEY_UT_SERVER_BASED_SERVICES_INT_ARRAY))
                .thenReturn(new int[] {});
        int tId = 1;

        mSscServiceImpl.updateCOLP(tId, true);

        mLooper.processAllMessages();
        verify(mMockUtListener).utConfigurationUpdateFailed(eq(tId), captorReasonInfo.capture());
        verifyNoMoreInteractions(mMockSscTransaction);

        ImsReasonInfo reasonInfo = captorReasonInfo.getValue();
        assertNotNull(reasonInfo);
        assertEquals(ImsReasonInfo.CODE_UT_OPERATION_NOT_ALLOWED, reasonInfo.getCode());
    }

    @Test
    public void testUpdateColpTb_success_notifySscChanged() {
        int expectedStatus = SscConstant.STATUS_ENABLE;
        when(mMockCarrierConfig.getIntArray(
                CarrierConfigManager.ImsSs.KEY_UT_TERMINAL_BASED_SERVICES_INT_ARRAY))
                .thenReturn(new int[]{
                        CarrierConfigManager.ImsSs.SUPPLEMENTARY_SERVICE_IDENTIFICATION_TIP});
        when(mMockSscPreferenceHelper.updateTip(eq(expectedStatus))).thenReturn(true);
        mSscServiceImpl.setSscPreferenceHelper(mMockSscPreferenceHelper);
        mSscServiceImpl.addTbSscChangeListener(mMockTbSscChangeListener);
        mLooper.processAllMessages();
        reset(mMockTbSscChangeListener);
        int tId = 60;

        mSscServiceImpl.updateCOLP(tId, true);
        mLooper.processAllMessages();

        verify(mMockUtListener).utConfigurationUpdated(eq(tId));
        verify(mMockTbSscChangeListener)
                .onSupplementaryServiceConfigurationChanged(mCaptorSscs.capture());

        List<SupplementaryServiceConfiguration> capturedList = mCaptorSscs.getValue();
        assertEquals(1, capturedList.size());
        assertTrue(capturedList.getFirst() instanceof TipData);
        TipData tipData = (TipData) capturedList.getFirst();
        assertEquals(expectedStatus, tipData.getStatus());

        verifyNoMoreInteractions(mMockSscTransaction);
    }

    @Test
    public void testUpdateColpTb_failure_doNotNotifySscChanged() {
        int expectedStatus = SscConstant.STATUS_DISABLE;
        when(mMockCarrierConfig.getIntArray(
                CarrierConfigManager.ImsSs.KEY_UT_TERMINAL_BASED_SERVICES_INT_ARRAY))
                .thenReturn(new int[]{
                        CarrierConfigManager.ImsSs.SUPPLEMENTARY_SERVICE_IDENTIFICATION_TIP});
        when(mMockSscPreferenceHelper.updateTip(eq(expectedStatus))).thenReturn(false);
        mSscServiceImpl.setSscPreferenceHelper(mMockSscPreferenceHelper);
        mSscServiceImpl.addTbSscChangeListener(mMockTbSscChangeListener);
        mLooper.processAllMessages();
        reset(mMockTbSscChangeListener);
        int tId = 61;

        mSscServiceImpl.updateCOLP(tId, false);
        mLooper.processAllMessages();

        verify(mMockSscPreferenceHelper).updateTip(eq(expectedStatus));
        verify(mMockUtListener).utConfigurationUpdateFailed(eq(tId), captorReasonInfo.capture());

        ImsReasonInfo reasonInfo = captorReasonInfo.getValue();
        assertNotNull(reasonInfo);
        assertEquals(ImsReasonInfo.CODE_LOCAL_INTERNAL_ERROR, reasonInfo.getCode());

        verify(mMockTbSscChangeListener, never()).onSupplementaryServiceConfigurationChanged(any());
        verifyNoMoreInteractions(mMockSscTransaction);
    }

    @Test
    public void testUpdateColpTb_listenerNull_doNothing() {
        when(mMockCarrierConfig.getIntArray(
                CarrierConfigManager.ImsSs.KEY_UT_TERMINAL_BASED_SERVICES_INT_ARRAY))
                .thenReturn(new int[]{
                        CarrierConfigManager.ImsSs.SUPPLEMENTARY_SERVICE_IDENTIFICATION_TIP});
        mSscServiceImpl.setSscPreferenceHelper(mMockSscPreferenceHelper);
        mSscServiceImpl.setListener(null);
        int tId = 62;

        mSscServiceImpl.updateCOLP(tId, true);
        mLooper.processAllMessages();

        verify(mMockUtListener, never()).utConfigurationUpdated(anyInt());
        verify(mMockUtListener, never()).utConfigurationUpdateFailed(anyInt(), any());
        verify(mMockSscPreferenceHelper, never()).updateTip(anyInt());
        verify(mMockTbSscChangeListener, never()).onSupplementaryServiceConfigurationChanged(any());
        verifyNoMoreInteractions(mMockSscTransaction);
    }

    @Test
    public void testAddTbSscChangeListener_notifyTbSscStatus() {
        int boicExhcVoiceStatus = SscConstant.STATUS_ENABLE;
        int baicVideoStatus = SscConstant.STATUS_DISABLE;
        int bicWrVoiceStatus = SscConstant.STATUS_ENABLE;
        int oipStatus = SscConstant.STATUS_ENABLE;
        int tipStatus = SscConstant.STATUS_DISABLE;
        int oirStatus = SscConstant.STATUS_DISABLE; // SscConstant.OIR_SUPPRESSION.
        int tirStatus = SscConstant.STATUS_ENABLE; // SscConstant.TIR_PROVISIONED.
        when(mMockCarrierConfig.getIntArray(
                CarrierConfigManager.ImsSs.KEY_UT_TERMINAL_BASED_SERVICES_INT_ARRAY))
                .thenReturn(new int[]{
                        CarrierConfigManager.ImsSs.SUPPLEMENTARY_SERVICE_CB_BAOC,
                        CarrierConfigManager.ImsSs.SUPPLEMENTARY_SERVICE_CB_BOIC,
                        CarrierConfigManager.ImsSs.SUPPLEMENTARY_SERVICE_CB_BOIC_EXHC,
                        CarrierConfigManager.ImsSs.SUPPLEMENTARY_SERVICE_CB_BAIC,
                        CarrierConfigManager.ImsSs.SUPPLEMENTARY_SERVICE_CB_BIC_ROAM,
                        // Remove ACR to verify that it is not notified.
                        // CarrierConfigManager.ImsSs.SUPPLEMENTARY_SERVICE_CB_ACR,
                        CarrierConfigManager.ImsSs.SUPPLEMENTARY_SERVICE_IDENTIFICATION_OIP,
                        CarrierConfigManager.ImsSs.SUPPLEMENTARY_SERVICE_IDENTIFICATION_TIP,
                        CarrierConfigManager.ImsSs.SUPPLEMENTARY_SERVICE_IDENTIFICATION_OIR,
                        CarrierConfigManager.ImsSs.SUPPLEMENTARY_SERVICE_IDENTIFICATION_TIR,
                });
        when(mMockSscPreferenceHelper
                .queryCb(SscConstant.CONDITION_BAOC, SscServiceClassUtil.SERVICE_CLASS_VOICE))
                .thenReturn(SscConstant.STATUS_NOT_REGISTERED); // Not registered. Will not notify.
        when(mMockSscPreferenceHelper
                .queryCb(SscConstant.CONDITION_BAOC, SscServiceClassUtil.SERVICE_CLASS_VIDEO))
                .thenReturn(SscConstant.STATUS_NOT_REGISTERED); // Not registered. Will not notify.
        when(mMockSscPreferenceHelper
                .queryCb(SscConstant.CONDITION_BOIC, SscServiceClassUtil.SERVICE_CLASS_VOICE))
                .thenReturn(SscConstant.STATUS_NOT_REGISTERED); // Not registered. Will not notify.
        when(mMockSscPreferenceHelper
                .queryCb(SscConstant.CONDITION_BOIC, SscServiceClassUtil.SERVICE_CLASS_VIDEO))
                .thenReturn(SscConstant.STATUS_NOT_REGISTERED); // Not registered. Will not notify.
        when(mMockSscPreferenceHelper
                .queryCb(SscConstant.CONDITION_BOIC_EXHC, SscServiceClassUtil.SERVICE_CLASS_VOICE))
                .thenReturn(boicExhcVoiceStatus);
        when(mMockSscPreferenceHelper
                .queryCb(SscConstant.CONDITION_BOIC_EXHC, SscServiceClassUtil.SERVICE_CLASS_VIDEO))
                .thenReturn(SscConstant.STATUS_NOT_REGISTERED); // Not registered. Will not notify.
        when(mMockSscPreferenceHelper
                .queryCb(SscConstant.CONDITION_BAIC, SscServiceClassUtil.SERVICE_CLASS_VOICE))
                .thenReturn(SscConstant.STATUS_NOT_REGISTERED); // Not registered. Will not notify.
        when(mMockSscPreferenceHelper
                .queryCb(SscConstant.CONDITION_BAIC, SscServiceClassUtil.SERVICE_CLASS_VIDEO))
                .thenReturn(baicVideoStatus);
        when(mMockSscPreferenceHelper
                .queryCb(SscConstant.CONDITION_BIC_WR, SscServiceClassUtil.SERVICE_CLASS_VOICE))
                .thenReturn(bicWrVoiceStatus);
        when(mMockSscPreferenceHelper
                .queryCb(SscConstant.CONDITION_BIC_WR, SscServiceClassUtil.SERVICE_CLASS_VIDEO))
                .thenReturn(SscConstant.STATUS_NOT_REGISTERED); // Not registered. Will not notify.
        when(mMockSscPreferenceHelper
                .queryCb(SscConstant.CONDITION_ACR, SscServiceClassUtil.SERVICE_CLASS_VOICE))
                .thenReturn(SscConstant.STATUS_ENABLE); // Not supported. Will not notify.
        when(mMockSscPreferenceHelper
                .queryCb(SscConstant.CONDITION_ACR, SscServiceClassUtil.SERVICE_CLASS_VIDEO))
                .thenReturn(SscConstant.STATUS_DISABLE); // Not supported. Will not notify.
        when(mMockSscPreferenceHelper.queryOip()).thenReturn(oipStatus);
        when(mMockSscPreferenceHelper.queryTip()).thenReturn(tipStatus);
        when(mMockSscPreferenceHelper.queryOir()).thenReturn(SscConstant.OIR_SUPPRESSION);
        when(mMockSscPreferenceHelper.queryTir()).thenReturn(SscConstant.TIR_PROVISIONED);
        mSscServiceImpl.setSscPreferenceHelper(mMockSscPreferenceHelper);

        mSscServiceImpl.addTbSscChangeListener(mMockTbSscChangeListener);
        mLooper.processAllMessages();

        verify(mMockSscPreferenceHelper).queryCb(eq(SscConstant.CONDITION_BAOC),
                eq(SscServiceClassUtil.SERVICE_CLASS_VOICE));
        verify(mMockSscPreferenceHelper).queryCb(eq(SscConstant.CONDITION_BAOC),
                eq(SscServiceClassUtil.SERVICE_CLASS_VIDEO));
        verify(mMockSscPreferenceHelper).queryCb(eq(SscConstant.CONDITION_BOIC),
                eq(SscServiceClassUtil.SERVICE_CLASS_VOICE));
        verify(mMockSscPreferenceHelper).queryCb(eq(SscConstant.CONDITION_BOIC),
                eq(SscServiceClassUtil.SERVICE_CLASS_VIDEO));
        verify(mMockSscPreferenceHelper).queryCb(eq(SscConstant.CONDITION_BOIC_EXHC),
                eq(SscServiceClassUtil.SERVICE_CLASS_VOICE));
        verify(mMockSscPreferenceHelper).queryCb(eq(SscConstant.CONDITION_BOIC_EXHC),
                eq(SscServiceClassUtil.SERVICE_CLASS_VIDEO));
        verify(mMockSscPreferenceHelper).queryCb(eq(SscConstant.CONDITION_BAIC),
                eq(SscServiceClassUtil.SERVICE_CLASS_VOICE));
        verify(mMockSscPreferenceHelper).queryCb(eq(SscConstant.CONDITION_BAIC),
                eq(SscServiceClassUtil.SERVICE_CLASS_VIDEO));
        verify(mMockSscPreferenceHelper).queryCb(eq(SscConstant.CONDITION_BIC_WR),
                eq(SscServiceClassUtil.SERVICE_CLASS_VOICE));
        verify(mMockSscPreferenceHelper).queryCb(eq(SscConstant.CONDITION_BIC_WR),
                eq(SscServiceClassUtil.SERVICE_CLASS_VIDEO));
        verify(mMockSscPreferenceHelper).queryOip();
        verify(mMockSscPreferenceHelper).queryTip();
        verify(mMockSscPreferenceHelper).queryOir();
        verify(mMockSscPreferenceHelper).queryTir();
        verifyNoMoreInteractions(mMockSscPreferenceHelper);

        verify(mMockTbSscChangeListener)
                .onSupplementaryServiceConfigurationChanged(mCaptorSscs.capture());

        List<SupplementaryServiceConfiguration> capturedList = mCaptorSscs.getValue();
        // Expected count:
        // 1 for BOIC_EXHC (voice)
        // 1 for BAIC (video)
        // 1 for BIC_ROAM (voice)
        // 1 for OIP
        // 1 for OIR
        // 1 for TIP
        // 1 for TIR
        // Total = 7
        assertEquals(7, capturedList.size());

        // CarrierConfigManager.ImsSs.SUPPLEMENTARY_SERVICE_CB_BOIC_EXHC, Voice.
        assertTrue(capturedList.get(0) instanceof CbData);
        CbData cbData0 = (CbData) capturedList.get(0);
        assertEquals(SscConstant.CONDITION_BOIC_EXHC, cbData0.getCondition());
        assertEquals(boicExhcVoiceStatus, cbData0.getStatus());
        assertEquals(SscServiceClassUtil.SERVICE_CLASS_VOICE, cbData0.getServiceClass());

        // CarrierConfigManager.ImsSs.SUPPLEMENTARY_SERVICE_CB_BAIC, Video.
        assertTrue(capturedList.get(1) instanceof CbData);
        CbData cbData1 = (CbData) capturedList.get(1);
        assertEquals(SscConstant.CONDITION_BAIC, cbData1.getCondition());
        assertEquals(baicVideoStatus, cbData1.getStatus());
        assertEquals(SscServiceClassUtil.SERVICE_CLASS_VIDEO, cbData1.getServiceClass());

        // CarrierConfigManager.ImsSs.SUPPLEMENTARY_SERVICE_CB_BIC_ROAM, Voice.
        assertTrue(capturedList.get(2) instanceof CbData);
        CbData cbData2 = (CbData) capturedList.get(2);
        assertEquals(SscConstant.CONDITION_BIC_WR, cbData2.getCondition());
        assertEquals(bicWrVoiceStatus, cbData2.getStatus());
        assertEquals(SscServiceClassUtil.SERVICE_CLASS_VOICE, cbData2.getServiceClass());

        // CarrierConfigManager.ImsSs.SUPPLEMENTARY_SERVICE_IDENTIFICATION_OIP.
        assertTrue(capturedList.get(3) instanceof OipData);
        OipData oipData = (OipData) capturedList.get(3);
        assertEquals(oipStatus, oipData.getStatus());

        // CarrierConfigManager.ImsSs.SUPPLEMENTARY_SERVICE_IDENTIFICATION_OIR.
        assertTrue(capturedList.get(4) instanceof OirData);
        OirData oirData = (OirData) capturedList.get(4);
        assertEquals(oirStatus, oirData.getStatus());

        // CarrierConfigManager.ImsSs.SUPPLEMENTARY_SERVICE_IDENTIFICATION_TIP.
        assertTrue(capturedList.get(5) instanceof TipData);
        TipData tipData = (TipData) capturedList.get(5);
        assertEquals(tipStatus, tipData.getStatus());

        // CarrierConfigManager.ImsSs.SUPPLEMENTARY_SERVICE_IDENTIFICATION_TIR.
        assertTrue(capturedList.get(6) instanceof TirData);
        TirData tirData = (TirData) capturedList.get(6);
        assertEquals(tirStatus, tirData.getStatus());
    }

    @Test
    public void testAddTbSscChangeListener_noTbService_doNotNotifyTbSscStatus() {
        when(mMockCarrierConfig.getIntArray(
                CarrierConfigManager.ImsSs.KEY_UT_TERMINAL_BASED_SERVICES_INT_ARRAY))
                .thenReturn(new int[]{});
        mSscServiceImpl.setSscPreferenceHelper(mMockSscPreferenceHelper);

        mSscServiceImpl.addTbSscChangeListener(mMockTbSscChangeListener);
        mLooper.processAllMessages();

        verify(mMockTbSscChangeListener, never()).onSupplementaryServiceConfigurationChanged(any());
    }

    @Test
    public void testAddAndRemoveTbSscChangeListener() {
        mSscServiceImpl.addTbSscChangeListener(mMockTbSscChangeListener);
        assertEquals(1, mSscServiceImpl.mTbSscChangeListeners.size());

        mSscServiceImpl.removeTbSscChangeListener(mMockTbSscChangeListener);
        assertEquals(0, mSscServiceImpl.mTbSscChangeListeners.size());
    }

    private void processEntireXmlDocQueryAsSuccess() {
        mLooper.processAllMessages();
        verify(mMockSscTransaction, atLeast(mQueryCount))
                .startGetTransaction(captorQueryData.capture());
        mQueryCount++;

        SscServiceQueryData capturedData = captorQueryData.getValue();
        assertEquals(ESsType.NONE, capturedData.getSsType());
        assertEquals(SscConstant.EVENT_SSC_QUERY_DOCUMENT, capturedData.getEventNumber());

        capturedData.setResponseCode(SscConstant.HTTP_OK);
        Document doc = createEntireXmldoc();
        SscXmlGov.getInstance(SLOT_0).parseXmlStream(capturedData, doc);

        Message msg = Message.obtain();
        msg.what = SscConstant.EVENT_SSC_QUERY_DOCUMENT;
        msg.obj =  new SscRequestResult(SLOT_0, capturedData.getTransactionId(),
                SscConstant.REQUEST_SUCCESS, SscConstant.HTTP_OK, -1);

        mCallbackHandler.sendMessage(msg);
    }

    private void processEntireXmlDocQueryAsFailure() {
        mLooper.processAllMessages();
        verify(mMockSscTransaction, atLeast(mQueryCount))
                .startGetTransaction(captorQueryData.capture());
        mQueryCount++;

        SscServiceQueryData capturedData = captorQueryData.getValue();
        assertEquals(ESsType.NONE, capturedData.getSsType());
        assertEquals(SscConstant.EVENT_SSC_QUERY_DOCUMENT, capturedData.getEventNumber());

        capturedData.setResponseCode(mHttpErrorResponse);

        Message msg = Message.obtain();
        msg.what = SscConstant.EVENT_SSC_QUERY_DOCUMENT;
        SscRequestResult rr = new SscRequestResult(SLOT_0, capturedData.getTransactionId(),
                SscConstant.REQUEST_FAILURE, mHttpErrorResponse, -1);
        if (!TextUtils.isEmpty(mErrorPhrase)) {
            rr.setSscServiceData(new ErrorResponseData(SLOT_0, capturedData.getSsType(),
                    capturedData.getEventNumber(), capturedData.getTransactionId(),
                    SscConstant.STATUS_DISABLE, capturedData.getResponseCode(), mErrorPhrase));
        }
        msg.obj = rr;

        mCallbackHandler.sendMessage(msg);
    }

    private void processGetTransactionAsSuccess(ESsType validSsType, int validEventNum,
            int validCondition) {
        mLooper.processAllMessages();
        verify(mMockSscTransaction, atLeast(mQueryCount))
                .startGetTransaction(captorQueryData.capture());
        mQueryCount++;

        SscServiceQueryData capturedData = captorQueryData.getValue();
        assertEquals(capturedData.getSsType(), validSsType);
        assertEquals(capturedData.getEventNumber(), validEventNum);
        if (validCondition != -1) {
            assertEquals(capturedData.getCondition(), validCondition);
        }

        capturedData.setResponseCode(mHttpSuccessResponse);

        Message msg = Message.obtain();
        msg.what = capturedData.getEventNumber();
        SscRequestResult rr = new SscRequestResult(SLOT_0, capturedData.getTransactionId(),
                SscConstant.REQUEST_SUCCESS, mHttpSuccessResponse, -1);
        rr.setSscServiceData(SscXmlGov.getInstance(SLOT_0)
                .parseXmlStream(capturedData, createEntireXmldoc()));
        msg.obj = rr;

        mCallbackHandler.sendMessage(msg);
    }

    private void processGetTransactionAsFailure(ESsType validSsType, int validEventNum,
            int validCondition) {
        mLooper.processAllMessages();
        verify(mMockSscTransaction, atLeast(mQueryCount))
                .startGetTransaction(captorQueryData.capture());
        mQueryCount++;

        SscServiceQueryData capturedData = captorQueryData.getValue();
        assertEquals(validSsType, capturedData.getSsType());
        assertEquals(validEventNum, capturedData.getEventNumber());
        if (validCondition != -1) {
            assertEquals(validCondition, capturedData.getCondition());
        }

        capturedData.setResponseCode(mHttpErrorResponse);

        Message msg = Message.obtain();
        msg.what = capturedData.getEventNumber();
        msg.obj = new SscRequestResult(SLOT_0, capturedData.getTransactionId(),
                SscConstant.REQUEST_FAILURE, mHttpErrorResponse, -1);

        mCallbackHandler.sendMessage(msg);
    }

    private void processPutTransactionAsSuccess(ESsType validSsType, int validEventNum,
            int validCondition) {
        mLooper.processAllMessages();
        verify(mMockSscTransaction, atLeast(mUpdateCount))
                .startPutTransaction(captorUpdateData.capture());
        mUpdateCount++;

        SscServiceData capturedData = captorUpdateData.getValue();
        assertEquals(validSsType, capturedData.getSsType());
        assertEquals(validEventNum, capturedData.getEventNumber());
        if (validCondition != -1) {
            assertEquals(validCondition, capturedData.getCondition());
        }

        capturedData.setResponseCode(200);

        Message msg = Message.obtain();
        msg.what = capturedData.getEventNumber();
        msg.obj = new SscRequestResult(SLOT_0, capturedData.getTransactionId(),
                SscConstant.REQUEST_SUCCESS, 200, -1);

        mCallbackHandler.sendMessage(msg);
    }

    private void processPutTransactionAsFailure(ESsType validSsType, int validEventNum,
            int validCondition) {
        mLooper.processAllMessages();
        verify(mMockSscTransaction, atLeast(mUpdateCount))
                .startPutTransaction(captorUpdateData.capture());
        mUpdateCount++;

        SscServiceData capturedData = captorUpdateData.getValue();
        assertEquals(capturedData.getSsType(), validSsType);
        assertEquals(capturedData.getEventNumber(), validEventNum);
        if (validCondition != -1) {
            assertEquals(capturedData.getCondition(), validCondition);
        }

        capturedData.setResponseCode(mHttpErrorResponse);

        Message msg = Message.obtain();
        msg.what = capturedData.getEventNumber();
        msg.obj = new SscRequestResult(SLOT_0, capturedData.getTransactionId(),
                SscConstant.REQUEST_FAILURE, mHttpErrorResponse, -1);

        mCallbackHandler.sendMessage(msg);
    }

    private Document createEntireXmldoc() {
        String xml = "<ss:simservs xmlns:SOAP-ENV=\"http://schemas.xmlsoap.org/soap/envelope/\""
                + "xmlns:SOAP-ENC=\"http://schemas.xmlsoap.org/soap/encoding/\""
                + "xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\""
                + "xmlns:xsd=\"http://www.w3.org/2001/XMLSchema\""
                + "xmlns:cp=\"urn:ietf:params:xml:ns:common-policy\""
                + "xmlns:ss=\"http://uri.etsi.org/ngn/params/xml/simservs/xcap\""
                + "xmlns:ocp=\"urn:oma:xml:xdm:common-policy\""
                + "xmlns:utns=\"urn:com:att:tlv:utx\""
                + "xmlns:xe=\"urn:ietf:params:xml:ns:xcap-error\""
                + "xmlns:data=\"http://com/alu/icm/fs5000dbv5_0/data\""
                + "xmlns:prs=\"http://www.nokia.com/prs/SubscriptionAPI\">"
                //ICB
                + "<ss:incoming-communication-barring active=\"true\">"
                + "<cp:ruleset xmlns:cp=\"urn:ietf:params:xml:ns:common-policy\">"
                + (!mIsIcbRulesExist ? "" : "<cp:rule id=\"call-barring-all-incoming\">"
                + "<cp:conditions>"
                + (mIsCbRuleActivated ? "" : "<ss:rule-deactivated/>")
                + "</cp:conditions>"
                + "<cp:actions>"
                + "<ss:allow>false</ss:allow>"
                + "</cp:actions>"
                + "</cp:rule>"
                + "<cp:rule id=\"call-barring-incoming-in-roaming\">"
                + "<cp:conditions>"
                + (mIsCbRuleActivated ? "" : "<ss:rule-deactivated/>")
                + "<ss:roaming/>"
                + "</cp:conditions>"
                + "<cp:actions>"
                + "<ss:allow>false</ss:allow>"
                + "</cp:actions>"
                + "</cp:rule>")
                + "</cp:ruleset>"
                + "</ss:incoming-communication-barring>"
                //OCB
                + "<ss:outgoing-communication-barring active=\"true\">"
                + "<cp:ruleset xmlns:cp=\"urn:ietf:params:xml:ns:common-policy\">"
                + "<cp:rule id=\"call-barring-all-outgoing\">"
                + "<cp:conditions>"
                + (mIsCbRuleActivated ? "" : "<ss:rule-deactivated/>")
                + "</cp:conditions>"
                + "<cp:actions>"
                + "<ss:allow>false</ss:allow>"
                + "</cp:actions>"
                + "</cp:rule>"
                + "<cp:rule id=\"call-barring-outgoing-international\">"
                + "<cp:conditions>"
                + (mIsCbRuleActivated ? "" : "<ss:rule-deactivated/>")
                + "<ss:international/>"
                + "</cp:conditions>"
                + "<cp:actions>"
                + "<ss:allow>false</ss:allow>"
                + "</cp:actions>"
                + "</cp:rule>"
                + "<cp:rule id=\"call-barring-outgoing-internationalExHC\">"
                + "<cp:conditions>"
                + (mIsCbRuleActivated ? "" : "<ss:rule-deactivated/>")
                + "<ss:international-exHC/>"
                + "</cp:conditions>"
                + "<cp:actions>"
                + "<ss:allow>false</ss:allow>"
                + "</cp:actions>"
                + "</cp:rule>"
                + "</cp:ruleset>"
                + "</ss:outgoing-communication-barring>"
                // CW
                + "<ss:communication-waiting active=\""
                        + (mIsCwEnabled ? "true" : "false") + "\"/>"
                // CD
                + "<ss:communication-diversion active=\"true\">"
                + (mIsTimerInCfnr ? "" : "<ss:NoReplyTimer>20</ss:NoReplyTimer>")
                + (!mIsCfRuleSetExist ? "" : "<cp:ruleset>"
                + "<cp:rule id=\"call-diversion-unconditional\">"
                + "<cp:conditions>"
                + (mIsCfRuleActivated ? "" : "<ss:rule-deactivated/>")
                + "</cp:conditions>"
                + "<cp:actions>"
                + "<ss:forward-to>"
                + "<ss:target>" + mForwardNumber + "</ss:target>"
                + "<ss:notify-caller>true</ss:notify-caller>"
                + "<ss:reveal-identity-to-caller>true</ss:reveal-identity-to-caller>"
                + "<ss:reveal-served-user-identity-to-caller>false"
                + "</ss:reveal-served-user-identity-to-caller>"
                + "<ss:notify-served-user>false</ss:notify-served-user>"
                + "<ss:notify-served-user-on-outbound-call>false"
                + "</ss:notify-served-user-on-outbound-call>"
                + "<ss:reveal-identity-to-target>false</ss:reveal-identity-to-target>"
                + "</ss:forward-to>"
                + "</cp:actions>"
                + "</cp:rule>"
                + (!mIsCfbRuleExist ? "" : "<cp:rule id=\"call-diversion-busy\">"
                + "<cp:conditions>"
                + (mIsCfRuleActivated ? "" : "<ss:rule-deactivated/>")
                + "<ss:busy/>"
                + "</cp:conditions>"
                + "<cp:actions>"
                + "<ss:forward-to>"
                + "<ss:target>" + mForwardNumber + "</ss:target>"
                + "<ss:notify-caller>true</ss:notify-caller>"
                + "<ss:reveal-identity-to-caller>true</ss:reveal-identity-to-caller>"
                + "<ss:reveal-served-user-identity-to-caller>false"
                + "</ss:reveal-served-user-identity-to-caller>"
                + "<ss:reveal-identity-to-target>false</ss:reveal-identity-to-target>"
                + "</ss:forward-to>"
                + "</cp:actions>"
                + "</cp:rule>")
                + "<cp:rule id=\"call-diversion-no-reply\">"
                + "<cp:conditions>"
                + (mIsCfRuleActivated ? "" : "<ss:rule-deactivated/>")
                + "<ss:no-answer/>"
                + "</cp:conditions>"
                + "<cp:actions>"
                + "<ss:forward-to>"
                + "<ss:target>" + mForwardNumber + "</ss:target>"
                + (mIsTimerInCfnr ? "<ss:NoReplyTimer>20</ss:NoReplyTimer>" : "")
                + "<ss:notify-caller>true</ss:notify-caller>"
                + "<ss:reveal-identity-to-caller>true</ss:reveal-identity-to-caller>"
                + "<ss:reveal-served-user-identity-to-caller>false"
                + "</ss:reveal-served-user-identity-to-caller>"
                + "<ss:reveal-identity-to-target>false</ss:reveal-identity-to-target>"
                + "</ss:forward-to>"
                + "</cp:actions>"
                + "</cp:rule>"
                + "<cp:rule id=\"call-diversion-not-reachable\">"
                + "<cp:conditions>"
                + (mIsCfRuleActivated ? "" : "<ss:rule-deactivated/>")
                + "<ss:not-reachable/>"
                + "</cp:conditions>"
                + "<cp:actions>"
                + "<ss:forward-to>"
                + "<ss:target>" + mForwardNumber + "</ss:target>"
                + "<ss:notify-caller>true</ss:notify-caller>"
                + "<ss:reveal-identity-to-caller>true</ss:reveal-identity-to-caller>"
                + "<ss:reveal-served-user-identity-to-caller>false"
                + "</ss:reveal-served-user-identity-to-caller>"
                + "<ss:reveal-identity-to-target>false</ss:reveal-identity-to-target>"
                + "</ss:forward-to>"
                + "</cp:actions>"
                + "</cp:rule>"
                + (mIsCfnlRuleExist ? "<cp:rule id=\"call-diversion-not-loggedin\">"
                + "<cp:conditions>"
                + (mIsCfRuleActivated ? "" : "<ss:rule-deactivated/>")
                + "<ss:not-registered/>"
                + "</cp:conditions>"
                + "<cp:actions>"
                + "<ss:forward-to>"
                + "<ss:target>" + mForwardNumber + "</ss:target>"
                + "<ss:notify-caller>true</ss:notify-caller>"
                + "<ss:reveal-identity-to-caller>true</ss:reveal-identity-to-caller>"
                + "<ss:reveal-served-user-identity-to-caller>false"
                + "</ss:reveal-served-user-identity-to-caller>"
                + "<ss:reveal-identity-to-target>false</ss:reveal-identity-to-target>"
                + "</ss:forward-to>"
                + "</cp:actions>"
                + "</cp:rule>" : "")
                + "</cp:ruleset>")
                + "</ss:communication-diversion>"
                // CDSC
                + "<ss:communication-diversion-serv-cap active=\"true\">"
                + "<ss:serv-cap-conditions>"
                + "<ss:serv-cap-anonymous provisioned=\"false\"/>"
                + "<ss:serv-cap-busy provisioned=\"true\"/>"
                + "<ss:serv-cap-external-list provisioned=\"false\"/>"
                + "<ss:serv-cap-identity provisioned=\"false\"/>"
                + "<ss:serv-cap-media>"
                + "<ss:media>audio</ss:media>"
                + "</ss:serv-cap-media>"
                + "<ss:serv-cap-not-registered provisioned=\"false\"/>"
                + "<ss:serv-cap-no-answer provisioned=\"true\"/>"
                + "<ss:serv-cap-not-reachable provisioned=\"true\"/>"
                + "<ss:serv-cap-presence-status provisioned=\"false\"/>"
                + "<ss:serv-cap-rule-deactivated provisioned=\"true\"/>"
                + "<ss:serv-cap-selective provisioned=\"false\"/>"
                + "<ss:serv-cap-validity provisioned=\"false\"/>"
                + "<ss:serv-cap-unconditional provisioned=\"true\"/>"
                + "<ss:serv-cap-default provisioned=\"true\"/>"
                + "</ss:serv-cap-conditions>"
                + "<ss:serv-cap-actions>"
                + "<ss:serv-cap-target>"
                + "<ss:telephony-type/>"
                + "</ss:serv-cap-target>"
                + "<ss:serv-cap-notify-caller provisioned=\"false\"/>"
                + "<ss:serv-cap-notify-served-user provisioned=\"false\"/>"
                + "<ss:serv-cap-notify-served-user-on-outbound-call provisioned=\"false\"/>"
                + "<ss:serv-cap-reveal-identity-to-caller provisioned=\"false\"/>"
                + "<ss:serv-cap-reveal-served-user-identity-to-caller provisioned=\"false\"/>"
                + "<ss:serv-cap-reveal-identity-to-target provisioned=\"false\"/>"
                + "</ss:serv-cap-actions>"
                + "</ss:communication-diversion-serv-cap>"
                // OIP
                + "<ss:originating-identity-presentation active=\""
                        + (mIsOipEnabled ? "true" : "false") + "\"/>"
                // OIR
                + "<ss:originating-identity-presentation-restriction active=\"true\">"
                + "<ss:default-behaviour>" + mDefaultBehaviour + "</ss:default-behaviour>"
                + "</ss:originating-identity-presentation-restriction>"
                // TIP
                + "<ss:terminating-identity-presentation active=\""
                        + (mIsTipEnabled ? "true" : "false") + "\"/>"
                // TIR
                + "<ss:terminating-identity-presentation-restriction active=\"true\">"
                + "<ss:default-behaviour>" + mDefaultBehaviour + "</ss:default-behaviour>"
                + "</ss:terminating-identity-presentation-restriction>"
                + "</ss:simservs>";

        DocumentBuilderFactory factory = DocumentBuilderFactory.newInstance();
        DocumentBuilder builder;
        InputSource is;
        Document document;
        try {
            factory.setNamespaceAware(false);
            builder = factory.newDocumentBuilder();
            is = new InputSource(new StringReader(xml));
            document = builder.parse(is);
        } catch (Exception e) {
            e.printStackTrace();
            return null;
        }

        return document;
    }
}
