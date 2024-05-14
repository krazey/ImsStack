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
import static org.junit.Assert.assertNotEquals;
import static org.junit.Assert.assertNotNull;
import static org.junit.Assert.assertNull;
import static org.junit.Assert.assertTrue;
import static org.mockito.ArgumentMatchers.any;
import static org.mockito.ArgumentMatchers.anyInt;
import static org.mockito.ArgumentMatchers.anyString;
import static org.mockito.ArgumentMatchers.eq;
import static org.mockito.Mockito.atLeast;
import static org.mockito.Mockito.verify;
import static org.mockito.Mockito.verifyNoMoreInteractions;
import static org.mockito.Mockito.when;

import android.content.Context;
import android.content.SharedPreferences;
import android.os.Handler;
import android.os.HandlerThread;
import android.os.Message;
import android.telephony.CarrierConfigManager;
import android.telephony.ims.ImsCallForwardInfo;
import android.telephony.ims.ImsReasonInfo;
import android.telephony.ims.ImsSsInfo;
import android.testing.AndroidTestingRunner;
import android.testing.TestableLooper;
import android.text.TextUtils;

import com.android.imsstack.ContextFixture;
import com.android.imsstack.base.AppContext;
import com.android.imsstack.core.agents.ConfigAgent;
import com.android.imsstack.core.agents.dcmif.EApnType;
import com.android.imsstack.core.config.CarrierConfig;
import com.android.imsstack.enabler.ssc.data.ErrorResponseData;
import com.android.imsstack.enabler.ssc.data.SscRequestResult;
import com.android.imsstack.enabler.ssc.data.SscServiceData;
import com.android.imsstack.enabler.ssc.data.SscServiceQueryData;
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
    @Mock private ConfigAgent mMockConfigAgent;
    @Mock private SharedPreferences mMockSharedPreferences;
    @Mock private SscServiceState mMockSscServiceState;
    @Mock private SscTransactionFactory mMockSscTransactionFactory;
    @Mock private SscTransaction mMockSscTransaction;
    @Mock private IUtListener mMockUtListener;
    @Mock private IUtServiceStateListener mMockUtServiceStateListener;

    @Captor ArgumentCaptor<SscServiceQueryData> captorQueryData;
    @Captor ArgumentCaptor<SscServiceData> captorUpdateData;
    @Captor ArgumentCaptor<ImsCallForwardInfo[]> captorCfInfos;
    @Captor ArgumentCaptor<ImsReasonInfo> captorReasonInfo;
    @Captor ArgumentCaptor<ImsSsInfo> captorSsInfo;
    @Captor ArgumentCaptor<ImsSsInfo[]> captorSsInfos;

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

        mQueryCount = 1;
        mUpdateCount = 1;
        mSscServiceImpl = new SscServiceImpl(SLOT_0);
        mSscServiceImpl.start(mContext);
        mSscServiceImpl.setListener(mMockUtListener);
        mSscServiceImpl.setSscTransactionFactory(mMockSscTransactionFactory);

        // mMockConfigAgent should be set after starting SscServiceImpl
        SscConfig.setConfigAgent(SLOT_0, mMockConfigAgent);
        when(mMockConfigAgent.getCarrierConfig()).thenReturn(mMockCarrierConfig);
        when(mMockCarrierConfig.getIntArray(
            CarrierConfigManager.ImsSs.KEY_UT_SERVER_BASED_SERVICES_INT_ARRAY))
            .thenReturn(mServerBasedServices);

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
                eq(CarrierConfig.Assets.KEY_UT_DISPLAY_ERROR_PHRASE_WITH_409_ERROR_BOOL)))
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
        assertEquals(reasonInfo.getCode(), ImsReasonInfo.CODE_UT_NETWORK_ERROR);
        assertEquals(reasonInfo.getExtraMessage(), mErrorPhrase);

        verify(mMockSscTransaction).close();
        verifyNoMoreInteractions(mMockSscTransaction);
    }

    @Test
    public void testBasicOperation_ignoreErrorPhrase() {
        when(mMockCarrierConfig.getBoolean(
                eq(CarrierConfig.Assets.KEY_UT_DISPLAY_ERROR_PHRASE_WITH_409_ERROR_BOOL)))
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
        assertEquals(reasonInfo.getCode(), ImsReasonInfo.CODE_LOCAL_CALL_CS_RETRY_REQUIRED);
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
        assertEquals(reasonInfo.getCode(), ImsReasonInfo.CODE_UT_OPERATION_NOT_ALLOWED);
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
        assertEquals(cfInfos[0].mCondition, queryCondition);

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
                eq(CarrierConfig.Assets.KEY_UT_QUERY_CF_ALL_AND_CF_ALL_CONDITIONAL_SUPPORT_BOOL)))
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
        assertEquals(reasonInfo.getCode(), ImsReasonInfo.CODE_UT_OPERATION_NOT_ALLOWED);
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
                eq(CarrierConfig.Assets.KEY_UT_QUERY_CF_ALL_AND_CF_ALL_CONDITIONAL_SUPPORT_BOOL)))
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
        assertEquals(reasonInfo.getCode(), ImsReasonInfo.CODE_UT_OPERATION_NOT_ALLOWED);
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
        assertEquals(reasonInfo.getCode(), ImsReasonInfo.CODE_UT_OPERATION_NOT_ALLOWED);
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
        assertEquals(reasonInfo.getCode(), ImsReasonInfo.CODE_UT_OPERATION_NOT_ALLOWED);
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
        assertEquals(reasonInfo.getCode(), ImsReasonInfo.CODE_UT_OPERATION_NOT_ALLOWED);
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
        assertEquals(reasonInfo.getCode(), ImsReasonInfo.CODE_UT_OPERATION_NOT_ALLOWED);
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

        when(mMockCarrierConfig.getBoolean(eq(CarrierConfig.Assets.KEY_UT_INSERT_NEW_RULE_BOOL)))
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
        assertEquals(reasonInfo.getCode(), ImsReasonInfo.CODE_UT_OPERATION_NOT_ALLOWED);
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
        assertEquals(reasonInfo.getCode(), ImsReasonInfo.CODE_UT_OPERATION_NOT_ALLOWED);
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

        when(mMockCarrierConfig.getBoolean(eq(CarrierConfig.Assets.KEY_UT_INSERT_NEW_RULE_BOOL)))
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

        when(mMockCarrierConfig.getBoolean(eq(CarrierConfig.Assets.KEY_UT_INSERT_NEW_RULE_BOOL)))
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

        when(mMockCarrierConfig.getBoolean(eq(CarrierConfig.Assets.KEY_UT_INSERT_NEW_RULE_BOOL)))
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
                eq(CarrierConfig.Assets.KEY_UT_SUPPORT_CF_ACTION_ERASURE_BOOL))).thenReturn(true);
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
                eq(CarrierConfig.Assets.KEY_UT_SUPPORT_CF_ACTION_ERASURE_BOOL))).thenReturn(false);

        int tId = 1;

        mSscServiceImpl.updateCallForward(tId, SscConstant.ACTION_ERASURE,
                SscConstant.CONDITION_CFB, null, 0, 20);

        mLooper.processAllMessages();
        verify(mMockUtListener).utConfigurationUpdateFailed(eq(tId), any());
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
        assertEquals(reasonInfo.getCode(), ImsReasonInfo.CODE_UT_OPERATION_NOT_ALLOWED);
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
        assertEquals(reasonInfo.getCode(), ImsReasonInfo.CODE_UT_OPERATION_NOT_ALLOWED);
    }

    @Test
    public void testUpdateCallForward_invalidTimer() {
        int tId = 1;
        int invalidTimer = SscConstant.CFNR_TIMER_MAX + 1;

        mSscServiceImpl.updateCallForward(tId, SscConstant.ACTION_ERASURE,
                SscConstant.CONDITION_CFB, null, 0, invalidTimer);

        mLooper.processAllMessages();
        verify(mMockUtListener).utConfigurationUpdateFailed(eq(tId), captorReasonInfo.capture());
        verifyNoMoreInteractions(mMockSscTransaction);

        ImsReasonInfo reasonInfo = captorReasonInfo.getValue();
        assertNotNull(reasonInfo);
        assertEquals(reasonInfo.getCode(), ImsReasonInfo.CODE_UT_OPERATION_NOT_ALLOWED);
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
        assertEquals(reasonInfo.getCode(), ImsReasonInfo.CODE_UT_OPERATION_NOT_ALLOWED);
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
        assertEquals(reasonInfo.getCode(), ImsReasonInfo.CODE_UT_OPERATION_NOT_ALLOWED);
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
        assertEquals(reasonInfo.getCode(), ImsReasonInfo.CODE_UT_OPERATION_NOT_ALLOWED);
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
        assertEquals(reasonInfo.getCode(), ImsReasonInfo.CODE_UT_OPERATION_NOT_ALLOWED);
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
        assertEquals(reasonInfo.getCode(), ImsReasonInfo.CODE_UT_OPERATION_NOT_ALLOWED);
    }

    private void processEntireXmlDocQueryAsSuccess() {
        mLooper.processAllMessages();
        verify(mMockSscTransaction, atLeast(mQueryCount))
                .startGetTransaction(captorQueryData.capture());
        mQueryCount++;

        SscServiceQueryData capturedData = captorQueryData.getValue();
        assertEquals(capturedData.getSsType(), ESsType.NONE);
        assertEquals(capturedData.getEventNumber(), SscConstant.EVENT_SSC_QUERY_DOCUMENT);

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
        assertEquals(capturedData.getSsType(), ESsType.NONE);
        assertEquals(capturedData.getEventNumber(), SscConstant.EVENT_SSC_QUERY_DOCUMENT);

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
