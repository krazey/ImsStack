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

import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertNotEquals;
import static org.junit.Assert.assertNotNull;
import static org.junit.Assert.assertTrue;
import static org.junit.Assert.fail;
import static org.mockito.ArgumentMatchers.any;
import static org.mockito.ArgumentMatchers.eq;
import static org.mockito.Mockito.atLeast;
import static org.mockito.Mockito.verify;
import static org.mockito.Mockito.verifyNoMoreInteractions;
import static org.mockito.Mockito.when;

import android.content.Context;
import android.os.Handler;
import android.os.Message;
import android.telephony.CarrierConfigManager;
import android.telephony.ims.ImsCallForwardInfo;
import android.telephony.ims.ImsReasonInfo;
import android.telephony.ims.ImsSsInfo;
import android.testing.TestableLooper;
import android.text.TextUtils;

import com.android.imsstack.core.agents.ConfigAgent;
import com.android.imsstack.core.config.CarrierConfig;
import com.android.imsstack.enabler.ssc.data.ErrorResponseData;
import com.android.imsstack.enabler.ssc.data.SscRequestResult;
import com.android.imsstack.enabler.ssc.data.SscServiceData;
import com.android.imsstack.enabler.ssc.data.SscServiceQueryData;
import com.android.imsstack.imsservice.mmtel.ut.base.UtListener;

import org.junit.After;
import org.junit.Before;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.junit.runners.JUnit4;
import org.mockito.ArgumentCaptor;
import org.mockito.Captor;
import org.mockito.Mock;
import org.mockito.MockitoAnnotations;
import org.w3c.dom.Document;
import org.xml.sax.InputSource;

import java.io.StringReader;

import javax.xml.parsers.DocumentBuilder;
import javax.xml.parsers.DocumentBuilderFactory;

@RunWith(JUnit4.class)
@TestableLooper.RunWithLooper
public class SscServiceImplTest {
    private final static int SLOT_0 = 0;
    //private static final int SLOT_1 = 1;

    private SscServiceImpl mSscServiceImpl;

    private int mQueryCount; // increase after every startGetTransaction case
    private int mUpdateCount; // increase after every startPutTransaction case
    private Handler mCallbackHandler;
    private Handler mRequestHandler;
    private TestableLooper mLooper;

    @Mock private Context mockContext;

    @Mock private CarrierConfig mockCarrierConfig;
    @Mock private ConfigAgent mockConfigAgent;
    @Mock private SscServiceState mockSscServiceState;
    @Mock private SscTransactionFactory mockSscTransactionFactory;
    @Mock private SscTransaction mockSscTransaction;
    @Mock private UtListener mockUtListener;

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

    @Before
    public void setup() {
        MockitoAnnotations.initMocks(this);

        mQueryCount = 1;
        mUpdateCount = 1;
        mSscServiceImpl = new SscServiceImpl(SLOT_0);
        SscXmlFormat.init(SLOT_0);

        // mockConfigAgent should be set before starting SscServiceImpl
        SscConfig.setConfigAgent(SLOT_0, mockConfigAgent);
        when(mockConfigAgent.getCarrierConfig()).thenReturn(mockCarrierConfig);
        when(mockCarrierConfig.getBoolean(
                eq(CarrierConfigManager.KEY_CARRIER_SUPPORTS_SS_OVER_UT_BOOL))).thenReturn(true);

        mSscServiceImpl.start(mockContext);
        mSscServiceImpl.setListener(mockUtListener);
        mSscServiceImpl.setSscTransactionFactory(mockSscTransactionFactory);

        SscServiceStateAgent.getInstance().setSscServiceState(SLOT_0, mockSscServiceState);
        when(mockSscServiceState.isUtAvailable()).thenReturn(true);

        mRequestHandler = mSscServiceImpl.getRequestHandler();
        try {
            mLooper = new TestableLooper(mRequestHandler.getLooper());
        } catch (Exception e) {
            fail("Fail to get looper from handler");
        }

        mLooper.processAllMessages();

        mCallbackHandler = mSscServiceImpl.getCallBackHandler();
        when(mockSscTransactionFactory.getSscTransaction(eq(SLOT_0), any()))
                .thenReturn(mockSscTransaction);
    }

    @After
    public void tearDown() {
        mSscServiceImpl.close();
    }

    @Test
    public void testBasicOperation_utNotAvailable() {
        when(mockSscServiceState.isUtAvailable()).thenReturn(false);

        int tId = mSscServiceImpl.queryCallForward(SscConstant.CONDITION_CFU, "");

        mLooper.processAllMessages();
        verify(mockUtListener).utConfigurationQueryFailed(eq(tId), any());
        verifyNoMoreInteractions(mockSscTransaction);
    }

    @Test
    public void testBasicOperation_informmErrorPhrase() {
        when(mockCarrierConfig.getBoolean(
                eq(CarrierConfig.Assets.KEY_UT_DISPLAY_ERROR_PHRASE_WITH_409_ERROR_BOOL)))
                .thenReturn(true);
        mHttpErrorResponse = SscConstant.HTTP_CONFLICT;
        mErrorPhrase = "check error phrase";

        int tId = mSscServiceImpl.queryCallForward(SscConstant.CONDITION_CFU, "");
        processEntireXmlDocQueryAsFailure();

        mLooper.processAllMessages();
        verify(mockUtListener).utConfigurationQueryFailed(eq(tId), captorReasonInfo.capture());

        ImsReasonInfo reasonInfo = captorReasonInfo.getValue();
        assertNotNull(reasonInfo);
        assertEquals(reasonInfo.getCode(), ImsReasonInfo.CODE_UT_NETWORK_ERROR);
        assertEquals(reasonInfo.getExtraMessage(), mErrorPhrase);

        verify(mockSscTransaction).close();
        verifyNoMoreInteractions(mockSscTransaction);
    }

    @Test
    public void testBasicOperation_notInformmErrorPhrase() {
        when(mockCarrierConfig.getBoolean(
                eq(CarrierConfig.Assets.KEY_UT_DISPLAY_ERROR_PHRASE_WITH_409_ERROR_BOOL)))
                .thenReturn(false);
        mHttpErrorResponse = SscConstant.HTTP_CONFLICT;
        mErrorPhrase = "check error phrase";

        int tId = mSscServiceImpl.queryCallForward(SscConstant.CONDITION_CFU, "");
        processEntireXmlDocQueryAsFailure();

        mLooper.processAllMessages();
        verify(mockUtListener).utConfigurationQueryFailed(eq(tId), captorReasonInfo.capture());

        ImsReasonInfo reasonInfo = captorReasonInfo.getValue();
        assertNotNull(reasonInfo);
        assertNotEquals(reasonInfo.getExtraMessage(), mErrorPhrase);

        verify(mockSscTransaction).close();
        verifyNoMoreInteractions(mockSscTransaction);
    }

    @Test
    public void testBasicOperation_errorReasonCsfb() {
        when(mockCarrierConfig.getBoolean(
                eq(CarrierConfigManager.ImsSs.KEY_USE_CSFB_ON_XCAP_OVER_UT_FAILURE_BOOL)))
                .thenReturn(true);
        mHttpErrorResponse = SscConstant.HTTP_CONFLICT;

        int tId = mSscServiceImpl.queryCallForward(SscConstant.CONDITION_CFU, "");
        processEntireXmlDocQueryAsFailure();

        mLooper.processAllMessages();
        verify(mockUtListener).utConfigurationQueryFailed(eq(tId), captorReasonInfo.capture());

        ImsReasonInfo reasonInfo = captorReasonInfo.getValue();
        assertNotNull(reasonInfo);
        assertEquals(reasonInfo.getCode(), ImsReasonInfo.CODE_LOCAL_CALL_CS_RETRY_REQUIRED);
        assertNotEquals(reasonInfo.getExtraMessage(), mErrorPhrase);

        verify(mockSscTransaction).close();
        verifyNoMoreInteractions(mockSscTransaction);
    }

    @Test
    public void testQueryCallBarringForServiceClass_singleRequestIcb() {
        mIsCbRuleActivated = false;
        int queryCondition = SscConstant.CONDITION_BIC_WR;

        int tId = mSscServiceImpl.queryCallBarringForServiceClass(queryCondition,
                SscServiceClassUtil.SERVICE_CLASS_VOICE);
        processEntireXmlDocQueryAsSuccess();
        processGetTransactionAsSuccess(ESsType.ICB, SscConstant.EVENT_SSC_QUERY_CB,
                queryCondition);

        mLooper.processAllMessages();
        verify(mockUtListener).utConfigurationCallBarringQueried(eq(tId), captorSsInfos.capture());

        ImsSsInfo[] cbInfos = captorSsInfos.getValue();
        assertNotNull(cbInfos);

        int xmlStatus = mIsCbRuleActivated ? SscConstant.STATUS_ENABLE : SscConstant.STATUS_DISABLE;
        assertEquals(cbInfos[0].mStatus, xmlStatus);

        verify(mockSscTransaction).close();
        verifyNoMoreInteractions(mockSscTransaction);
    }

    @Test
    public void testQueryCallBarringForServiceClass_singleRequestOcb() {
        mIsCbRuleActivated = true;
        int queryCondition = SscConstant.CONDITION_BAOC;

        int tId = mSscServiceImpl.queryCallBarringForServiceClass(queryCondition,
                SscServiceClassUtil.SERVICE_CLASS_VOICE);
        processEntireXmlDocQueryAsSuccess();
        processGetTransactionAsSuccess(ESsType.OCB, SscConstant.EVENT_SSC_QUERY_CB,
                queryCondition);

        mLooper.processAllMessages();
        verify(mockUtListener).utConfigurationCallBarringQueried(eq(tId), captorSsInfos.capture());

        ImsSsInfo[] cbInfos = captorSsInfos.getValue();
        assertNotNull(cbInfos);

        int xmlStatus = mIsCbRuleActivated ? SscConstant.STATUS_ENABLE : SscConstant.STATUS_DISABLE;
        assertEquals(cbInfos[0].mStatus, xmlStatus);

        verify(mockSscTransaction).close();
        verifyNoMoreInteractions(mockSscTransaction);
    }

    @Test
    public void testQueryCallBarringForServiceClass_entireDocumentQueryFailure() {
        int tId = mSscServiceImpl.queryCallBarringForServiceClass(SscConstant.CONDITION_BIC_WR,
                SscServiceClassUtil.SERVICE_CLASS_VOICE);
        processEntireXmlDocQueryAsFailure();

        mLooper.processAllMessages();
        verify(mockUtListener).utConfigurationQueryFailed(eq(tId), any());
        verify(mockSscTransaction).close();
        verifyNoMoreInteractions(mockSscTransaction);
    }

    @Test
    public void testQueryCallBarringForServiceClass_queryFailure() {
        int tId = mSscServiceImpl.queryCallBarringForServiceClass(SscConstant.CONDITION_BOIC,
                SscServiceClassUtil.SERVICE_CLASS_VOICE);
        processEntireXmlDocQueryAsSuccess();
        processGetTransactionAsFailure(ESsType.OCB, SscConstant.EVENT_SSC_QUERY_CB,
                SscConstant.CONDITION_BOIC);

        mLooper.processAllMessages();
        verify(mockUtListener).utConfigurationQueryFailed(eq(tId), any());
        verify(mockSscTransaction).close();
        verifyNoMoreInteractions(mockSscTransaction);
    }

    @Test
    public void testQueryCallForward_singleRequest() {
        int queryCondition = SscConstant.CONDITION_CFU;

        int tId = mSscServiceImpl.queryCallForward(queryCondition, "");
        processEntireXmlDocQueryAsSuccess();
        processGetTransactionAsSuccess(ESsType.CF, SscConstant.EVENT_SSC_QUERY_CF,
                queryCondition);

        mLooper.processAllMessages();
        verify(mockUtListener).utConfigurationCallForwardQueried(eq(tId), captorCfInfos.capture());

        ImsCallForwardInfo[] cfInfos = captorCfInfos.getValue();
        assertNotNull(cfInfos);
        assertEquals(cfInfos[0].mCondition, queryCondition);

        int xmlStatus = mIsCfRuleActivated ? SscConstant.STATUS_ENABLE : SscConstant.STATUS_DISABLE;
        assertEquals(cfInfos[0].mStatus, xmlStatus);
        if (!TextUtils.isEmpty(mForwardNumber)) {
            assertTrue(mForwardNumber.contains(cfInfos[0].mNumber));
        }

        verify(mockSscTransaction).close();
        verifyNoMoreInteractions(mockSscTransaction);
    }

    @Test
    public void testQueryCallForward_multipleRequest() {
        when(mockCarrierConfig.getBoolean(
                eq(CarrierConfig.Assets.KEY_UT_QUERY_CF_ALL_AND_CF_ALL_CONDITIONAL_SUPPORT_BOOL)))
                .thenReturn(true);

        int responseCount = 1;
        for (int c = SscConstant.CONDITION_CFU; c <= SscConstant.CONDITION_CFNL; c++) {
            int tId = mSscServiceImpl.queryCallForward(c, "");

            if (SscXmlGov.getInstance(SLOT_0).isXmlDataPresent() == false) {
                processEntireXmlDocQueryAsSuccess();
            }

            processGetTransactionAsSuccess(ESsType.CF, SscConstant.EVENT_SSC_QUERY_CF,
                    c);

            mLooper.processAllMessages();
            verify(mockUtListener).utConfigurationCallForwardQueried(eq(tId), any());
            verify(mockSscTransaction, atLeast(responseCount)).close();
            responseCount++;
        }

        verifyNoMoreInteractions(mockSscTransaction);
    }

    @Test
    public void testQueryCallForward_entireDocumentQueryFailure() {
        int tId = mSscServiceImpl.queryCallForward(SscConstant.CONDITION_CFU, "");
        processEntireXmlDocQueryAsFailure();

        mLooper.processAllMessages();
        verify(mockUtListener).utConfigurationQueryFailed(eq(tId), any());
        verify(mockSscTransaction).close();
        verifyNoMoreInteractions(mockSscTransaction);
    }

    @Test
    public void testQueryCallForward_queryFailure() {
        int tId = mSscServiceImpl.queryCallForward(SscConstant.CONDITION_CFU, "");
        processEntireXmlDocQueryAsSuccess();
        processGetTransactionAsFailure(ESsType.CF, SscConstant.EVENT_SSC_QUERY_CF,
                SscConstant.CONDITION_CFU);

        mLooper.processAllMessages();
        verify(mockUtListener).utConfigurationQueryFailed(eq(tId), any());
        verify(mockSscTransaction).close();
        verifyNoMoreInteractions(mockSscTransaction);
    }

    @Test
    public void testQueryCallForward_invalidParameter() {
        when(mockCarrierConfig.getBoolean(
                eq(CarrierConfig.Assets.KEY_UT_QUERY_CF_ALL_AND_CF_ALL_CONDITIONAL_SUPPORT_BOOL)))
                .thenReturn(false);

        int invalidParameter = SscConstant.CONDITION_CFA;
        int tId = mSscServiceImpl.queryCallForward(invalidParameter, "");

        mLooper.processAllMessages();
        verifyNoMoreInteractions(mockSscTransaction);
        verifyNoMoreInteractions(mockUtListener);
    }

    @Test
    public void testQueryCallWaiting_success() {
        int tId = mSscServiceImpl.queryCallWaiting();
        processEntireXmlDocQueryAsSuccess();
        processGetTransactionAsSuccess(ESsType.CW, SscConstant.EVENT_SSC_QUERY_CW, -1);

        mLooper.processAllMessages();
        verify(mockUtListener).utConfigurationCallWaitingQueried(eq(tId), captorSsInfos.capture());

        ImsSsInfo[] cwInfos = captorSsInfos.getValue();
        assertNotNull(cwInfos);

        int xmlStatus = mIsCwEnabled ? SscConstant.STATUS_ENABLE : SscConstant.STATUS_DISABLE;
        assertEquals(cwInfos[0].mStatus, xmlStatus);

        verify(mockSscTransaction).close();
    }

    @Test
    public void testQueryCallWaiting_failure() {
        int tId = mSscServiceImpl.queryCallWaiting();
        processEntireXmlDocQueryAsSuccess();
        processGetTransactionAsFailure(ESsType.CW, SscConstant.EVENT_SSC_QUERY_CW, -1);

        mLooper.processAllMessages();
        verify(mockUtListener).utConfigurationQueryFailed(eq(tId), any());
        verify(mockSscTransaction).close();
    }

    @Test
    public void testQueryClir_success() {
        int tId = mSscServiceImpl.queryCLIR();
        processEntireXmlDocQueryAsSuccess();
        processGetTransactionAsSuccess(ESsType.OIR, SscConstant.EVENT_SSC_QUERY_OIR, -1);

        mLooper.processAllMessages();
        verify(mockUtListener)
                .lineIdentificationSupplementaryServiceResponse(eq(tId), captorSsInfo.capture());

        ImsSsInfo ssInfo = captorSsInfo.getValue();
        assertNotNull(ssInfo);

        int outGoingState = (mDefaultBehaviour == SscXmlFormat.PRESENTATION_NOT_RESTRICTED
                ? SscConstant.OIR_SUPPRESSION : SscConstant.OIR_INVOCATION);
        assertEquals(ssInfo.getClirOutgoingState(), outGoingState);

        verify(mockSscTransaction).close();
    }

    @Test
    public void testQueryClir_failure() {
        int tId = mSscServiceImpl.queryCLIR();
        processEntireXmlDocQueryAsSuccess();
        processGetTransactionAsFailure(ESsType.OIR, SscConstant.EVENT_SSC_QUERY_OIR, -1);

        mLooper.processAllMessages();
        verify(mockUtListener).utConfigurationQueryFailed(eq(tId), any());
        verify(mockSscTransaction).close();
    }

    @Test
    public void testQueryClip_success() {
        int tId = mSscServiceImpl.queryCLIP();
        processEntireXmlDocQueryAsSuccess();
        processGetTransactionAsSuccess(ESsType.OIP, SscConstant.EVENT_SSC_QUERY_OIP, -1);

        mLooper.processAllMessages();
        verify(mockUtListener)
                .lineIdentificationSupplementaryServiceResponse(eq(tId), captorSsInfo.capture());

        ImsSsInfo ssInfo = captorSsInfo.getValue();
        assertNotNull(ssInfo);

        int xmlStatus = mIsOipEnabled ? SscConstant.STATUS_ENABLE : SscConstant.STATUS_DISABLE;
        assertEquals(ssInfo.mStatus, xmlStatus);

        verify(mockSscTransaction).close();
    }

    @Test
    public void testQueryClip_failure() {
        int tId = mSscServiceImpl.queryCLIP();
        processEntireXmlDocQueryAsSuccess();
        processGetTransactionAsFailure(ESsType.OIP, SscConstant.EVENT_SSC_QUERY_OIP, -1);

        mLooper.processAllMessages();
        verify(mockUtListener).utConfigurationQueryFailed(eq(tId), any());
        verify(mockSscTransaction).close();
    }

    @Test
    public void testQueryColr_success() {
        mDefaultBehaviour = SscXmlFormat.PRESENTATION_RESTRICTED;

        int tId = mSscServiceImpl.queryCOLR();
        processEntireXmlDocQueryAsSuccess();
        processGetTransactionAsSuccess(ESsType.TIR, SscConstant.EVENT_SSC_QUERY_TIR, -1);

        mLooper.processAllMessages();
        verify(mockUtListener)
                .lineIdentificationSupplementaryServiceResponse(eq(tId), captorSsInfo.capture());

        ImsSsInfo ssInfo = captorSsInfo.getValue();
        assertNotNull(ssInfo);

        int provisionedStatus = (mDefaultBehaviour == SscXmlFormat.PRESENTATION_NOT_RESTRICTED
                ? SscConstant.TIR_NOT_PROVISIONED : SscConstant.TIR_PROVISIONED);
        assertEquals(ssInfo.getProvisionStatus(), provisionedStatus);

        verify(mockSscTransaction).close();
    }

    @Test
    public void testQueryColr_failure() {
        int tId = mSscServiceImpl.queryCOLR();
        processEntireXmlDocQueryAsSuccess();
        processGetTransactionAsFailure(ESsType.TIR, SscConstant.EVENT_SSC_QUERY_TIR, -1);

        mLooper.processAllMessages();
        verify(mockUtListener).utConfigurationQueryFailed(eq(tId), any());
        verify(mockSscTransaction).close();
    }

    @Test
    public void testqueryColp_success() {
        int tId = mSscServiceImpl.queryCOLP();
        processEntireXmlDocQueryAsSuccess();
        processGetTransactionAsSuccess(ESsType.TIP, SscConstant.EVENT_SSC_QUERY_TIP, -1);

        mLooper.processAllMessages();
        verify(mockUtListener)
                .lineIdentificationSupplementaryServiceResponse(eq(tId), captorSsInfo.capture());

        ImsSsInfo ssInfo = captorSsInfo.getValue();
        assertNotNull(ssInfo);

        int xmlStatus = mIsTipEnabled ? SscConstant.STATUS_ENABLE : SscConstant.STATUS_DISABLE;
        assertEquals(ssInfo.mStatus, xmlStatus);

        verify(mockSscTransaction).close();
    }

    @Test
    public void testqueryColp_failure() {
        int tId = mSscServiceImpl.queryCOLP();
        processEntireXmlDocQueryAsSuccess();
        processGetTransactionAsFailure(ESsType.TIP, SscConstant.EVENT_SSC_QUERY_TIP, -1);

        mLooper.processAllMessages();
        verify(mockUtListener).utConfigurationQueryFailed(eq(tId), any());
        verify(mockSscTransaction).close();
    }

    @Test
    public void testUpdateCallBarringWithPassword_singleRequestIcb() {
        int tId = mSscServiceImpl.updateCallBarringWithPassword(SscConstant.CONDITION_BIC_WR,
                SscConstant.STATUS_ENABLE, null, SscServiceClassUtil.SERVICE_CLASS_VOICE, null);
        processEntireXmlDocQueryAsSuccess();
        processPutTransactionAsSuccess(ESsType.ICB, SscConstant.EVENT_SSC_UPDATE_CB,
                SscConstant.CONDITION_BIC_WR);

        mLooper.processAllMessages();
        verify(mockUtListener).utConfigurationUpdated(eq(tId));
        verify(mockSscTransaction).close();
        verifyNoMoreInteractions(mockSscTransaction);
    }

    @Test
    public void testUpdateCallBarringWithPassword_insertNewRule() {
        mIsIcbRulesExist = false;

        when(mockCarrierConfig.getBoolean(eq(CarrierConfig.Assets.KEY_UT_INSERT_NEW_RULE_BOOL)))
                .thenReturn(true);

        int tId = mSscServiceImpl.updateCallBarringWithPassword(SscConstant.CONDITION_BAIC,
                SscConstant.STATUS_ENABLE, null, SscServiceClassUtil.SERVICE_CLASS_VOICE, null);
        processEntireXmlDocQueryAsSuccess();
        processPutTransactionAsSuccess(ESsType.ICB, SscConstant.EVENT_SSC_INSERT_CB,
                SscConstant.CONDITION_BAIC);

        mLooper.processAllMessages();
        verify(mockUtListener).utConfigurationUpdated(eq(tId));
        verify(mockSscTransaction).close();
        verifyNoMoreInteractions(mockSscTransaction);
    }

    @Test
    public void testUpdateCallBarringWithPassword_entireDocumentQueryFailure() {
        int tId = mSscServiceImpl.updateCallBarringWithPassword(SscConstant.CONDITION_BIC_WR,
                SscConstant.STATUS_ENABLE, null, SscServiceClassUtil.SERVICE_CLASS_VOICE, null);
        processEntireXmlDocQueryAsFailure();

        mLooper.processAllMessages();
        verify(mockUtListener).utConfigurationUpdateFailed(eq(tId), any());
        verify(mockSscTransaction).close();
        verifyNoMoreInteractions(mockSscTransaction);
    }

    @Test
    public void testUpdateCallBarringWithPassword_updateFailure() {
        int tId = mSscServiceImpl.updateCallBarringWithPassword(SscConstant.CONDITION_BIC_WR,
                SscConstant.STATUS_ENABLE, null, SscServiceClassUtil.SERVICE_CLASS_VOICE, null);
        processEntireXmlDocQueryAsSuccess();
        processPutTransactionAsFailure(ESsType.ICB, SscConstant.EVENT_SSC_UPDATE_CB,
                SscConstant.CONDITION_BIC_WR);

        mLooper.processAllMessages();
        verify(mockUtListener).utConfigurationUpdateFailed(eq(tId), any());
        verify(mockSscTransaction).close();
        verifyNoMoreInteractions(mockSscTransaction);
    }

    @Test
    public void testUpdateCallForward_singleRequest() {
        mIsCfnlRuleExist = false;

        int tId = mSscServiceImpl.updateCallForward(SscConstant.ACTION_REGISTRATION,
                SscConstant.CONDITION_CFNRC, "+1234567890", 0, 0);
        processEntireXmlDocQueryAsSuccess();
        processPutTransactionAsSuccess(ESsType.CF, SscConstant.EVENT_SSC_UPDATE_CF,
                SscConstant.CONDITION_CFNRC);

        mLooper.processAllMessages();
        verify(mockUtListener).utConfigurationUpdated(eq(tId));
        verify(mockSscTransaction).close();
        verifyNoMoreInteractions(mockSscTransaction);
    }

    @Test
    public void testUpdateCallForward_insertNewRuleSet() {
        mIsCfRuleSetExist = false;

        when(mockCarrierConfig.getBoolean(eq(CarrierConfig.Assets.KEY_UT_INSERT_NEW_RULE_BOOL)))
                .thenReturn(true);

        int tId = mSscServiceImpl.updateCallForward(SscConstant.ACTION_REGISTRATION,
                SscConstant.CONDITION_CFNR, "+1234567890", 0, 0);
        processEntireXmlDocQueryAsSuccess();
        processPutTransactionAsSuccess(ESsType.CF, SscConstant.EVENT_SSC_INSERT_CF,
                SscConstant.CONDITION_CFNR);

        mLooper.processAllMessages();
        verify(mockUtListener).utConfigurationUpdated(eq(tId));
        verify(mockSscTransaction).close();
        verifyNoMoreInteractions(mockSscTransaction);
    }

    @Test
    public void testUpdateCallForward_insertNewRule() {
        mIsCfRuleSetExist = true;
        mIsCfbRuleExist = false;

        when(mockCarrierConfig.getBoolean(eq(CarrierConfig.Assets.KEY_UT_INSERT_NEW_RULE_BOOL)))
                .thenReturn(true);

        int tId = mSscServiceImpl.updateCallForward(SscConstant.ACTION_ACTIVATION,
                SscConstant.CONDITION_CFB, null, 0, 0);
        processEntireXmlDocQueryAsSuccess();
        processPutTransactionAsSuccess(ESsType.CF, SscConstant.EVENT_SSC_INSERT_CF,
                SscConstant.CONDITION_CFB);

        mLooper.processAllMessages();
        verify(mockUtListener).utConfigurationUpdated(eq(tId));
        verify(mockSscTransaction).close();
        verifyNoMoreInteractions(mockSscTransaction);
    }

    @Test
    public void testUpdateCallForward_notInsertNewRuleForCfnrTimer() {
        mIsCfRuleSetExist = false;

        when(mockCarrierConfig.getBoolean(eq(CarrierConfig.Assets.KEY_UT_INSERT_NEW_RULE_BOOL)))
                .thenReturn(true);

        int tId = mSscServiceImpl.updateCallForward(SscConstant.ACTION_REGISTRATION,
                SscConstant.CONDITION_CFNR, "+1234567890", 0, 25);
        processEntireXmlDocQueryAsSuccess();
        processPutTransactionAsSuccess(ESsType.CF, SscConstant.EVENT_SSC_INSERT_CF,
                SscConstant.CONDITION_CFNR);
        processPutTransactionAsSuccess(ESsType.CF, SscConstant.EVENT_SSC_UPDATE_CF,
                SscConstant.CONDITION_CFNR_TIMER);

        mLooper.processAllMessages();
        verify(mockUtListener).utConfigurationUpdated(eq(tId));
        verify(mockSscTransaction).close();
        verifyNoMoreInteractions(mockSscTransaction);
    }

    @Test
    public void testUpdateCallForward_CfnrcAndCfnl() {
        mIsCfnlRuleExist = true;

        when(mockCarrierConfig.getBoolean(
                eq(CarrierConfig.Assets.KEY_UT_SUPPORT_CF_ACTION_ERASURE_BOOL))).thenReturn(true);

        int tId = mSscServiceImpl.updateCallForward(SscConstant.ACTION_ERASURE,
                SscConstant.CONDITION_CFNRC, null, 0, 0);
        processEntireXmlDocQueryAsSuccess();
        processPutTransactionAsSuccess(ESsType.CF, SscConstant.EVENT_SSC_UPDATE_CF,
                SscConstant.CONDITION_CFNRC);
        processPutTransactionAsSuccess(ESsType.CF, SscConstant.EVENT_SSC_UPDATE_CF,
                SscConstant.CONDITION_CFNL);

        mLooper.processAllMessages();
        verify(mockUtListener).utConfigurationUpdated(eq(tId));
        verify(mockSscTransaction).close();
        verifyNoMoreInteractions(mockSscTransaction);
    }

    @Test
    public void testUpdateCallForward_CfnrAndTimer() {
        mIsTimerInCfnr = false;

        int tId = mSscServiceImpl.updateCallForward(SscConstant.ACTION_ACTIVATION,
                SscConstant.CONDITION_CFNR, null, 0, 20);
        processEntireXmlDocQueryAsSuccess();
        processPutTransactionAsSuccess(ESsType.CF, SscConstant.EVENT_SSC_UPDATE_CF,
                SscConstant.CONDITION_CFNR);
        processPutTransactionAsSuccess(ESsType.CF, SscConstant.EVENT_SSC_UPDATE_CF,
                SscConstant.CONDITION_CFNR_TIMER);

        mLooper.processAllMessages();
        verify(mockUtListener).utConfigurationUpdated(eq(tId));
        verify(mockSscTransaction).close();
        verifyNoMoreInteractions(mockSscTransaction);
    }

    @Test
    public void testUpdateCallForward_CfnrWithTimer() {
        mIsTimerInCfnr = true;

        int tId = mSscServiceImpl.updateCallForward(SscConstant.ACTION_ACTIVATION,
                SscConstant.CONDITION_CFNR, null, 0, 20);
        processEntireXmlDocQueryAsSuccess();

        processPutTransactionAsSuccess(ESsType.CF, SscConstant.EVENT_SSC_UPDATE_CF,
                SscConstant.CONDITION_CFNR);

        mLooper.processAllMessages();
        verify(mockUtListener).utConfigurationUpdated(eq(tId));
        verify(mockSscTransaction).close();
        verifyNoMoreInteractions(mockSscTransaction);
    }

    @Test
    public void testUpdateCallForward_CfAll() {
        mIsTimerInCfnr = true;
        mIsCfnlRuleExist = false;

        int tId = mSscServiceImpl.updateCallForward(SscConstant.ACTION_DEACTIVATION,
                SscConstant.CONDITION_CFA, null, 0, 20);

        for (int c = SscConstant.CONDITION_CFNRC; c >= SscConstant.CONDITION_CFU; c--) {
            if (SscXmlGov.getInstance(SLOT_0).isXmlDataPresent() == false) {
                processEntireXmlDocQueryAsSuccess();
            }

            processPutTransactionAsSuccess(ESsType.CF, SscConstant.EVENT_SSC_UPDATE_CF,
                    c);
        }

        mLooper.processAllMessages();
        verify(mockUtListener).utConfigurationUpdated(eq(tId));
        verify(mockSscTransaction).close();
        verifyNoMoreInteractions(mockSscTransaction);
    }

    @Test
    public void testUpdateCallForward_entireDocumentQueryFailure() {
        int tId = mSscServiceImpl.updateCallForward(SscConstant.ACTION_DEACTIVATION,
                SscConstant.CONDITION_CFA, null, 0, 20);
        processEntireXmlDocQueryAsFailure();

        mLooper.processAllMessages();
        verify(mockUtListener).utConfigurationUpdateFailed(eq(tId), any());
        verify(mockSscTransaction).close();
        verifyNoMoreInteractions(mockSscTransaction);
    }

    @Test
    public void testUpdateCallForward_updateFailure() {
        int tId = mSscServiceImpl.updateCallForward(SscConstant.ACTION_ACTIVATION,
                SscConstant.CONDITION_CFB, null, 0, 20);
        processEntireXmlDocQueryAsSuccess();
        processPutTransactionAsFailure(ESsType.CF, SscConstant.EVENT_SSC_UPDATE_CF,
                SscConstant.CONDITION_CFB);

        mLooper.processAllMessages();
        verify(mockUtListener).utConfigurationUpdateFailed(eq(tId), any());
        verify(mockSscTransaction).close();
        verifyNoMoreInteractions(mockSscTransaction);
    }

    @Test
    public void testUpdateCallForward_invalidParameter() {
        when(mockCarrierConfig.getBoolean(
                eq(CarrierConfig.Assets.KEY_UT_SUPPORT_CF_ACTION_ERASURE_BOOL))).thenReturn(false);

        int invalidParameter = SscConstant.ACTION_ERASURE;
        int tId = mSscServiceImpl.updateCallForward(invalidParameter, SscConstant.CONDITION_CFB,
                null, 0, 20);

        mLooper.processAllMessages();
        verifyNoMoreInteractions(mockSscTransaction);
        verifyNoMoreInteractions(mockUtListener);
    }

    @Test
    public void testUpdateCallWaiting_success() {
        int tId = mSscServiceImpl.updateCallWaiting(true, SscServiceClassUtil.SERVICE_CLASS_VOICE);
        processEntireXmlDocQueryAsSuccess();
        processPutTransactionAsSuccess(ESsType.CW, SscConstant.EVENT_SSC_UPDATE_CW, -1);

        mLooper.processAllMessages();
        verify(mockUtListener).utConfigurationUpdated(eq(tId));
        verify(mockSscTransaction).close();
    }

    @Test
    public void testUpdateCallWaiting_failure() {
        int tId = mSscServiceImpl.updateCallWaiting(true, SscServiceClassUtil.SERVICE_CLASS_VOICE);
        processEntireXmlDocQueryAsSuccess();
        processPutTransactionAsFailure(ESsType.CW, SscConstant.EVENT_SSC_UPDATE_CW, -1);

        mLooper.processAllMessages();
        verify(mockUtListener).utConfigurationUpdateFailed(eq(tId), any());
        verify(mockSscTransaction).close();
    }

    @Test
    public void testUpdateClir_success() {
        int tId = mSscServiceImpl.updateCLIR(SscConstant.OIR_INVOCATION);
        processEntireXmlDocQueryAsSuccess();
        processPutTransactionAsSuccess(ESsType.OIR, SscConstant.EVENT_SSC_UPDATE_OIR, -1);

        mLooper.processAllMessages();
        verify(mockUtListener).utConfigurationUpdated(eq(tId));
        verify(mockSscTransaction).close();
    }

    @Test
    public void testUpdateClir_failure() {
        int tId = mSscServiceImpl.updateCLIR(SscConstant.OIR_INVOCATION);
        processEntireXmlDocQueryAsSuccess();
        processPutTransactionAsFailure(ESsType.OIR, SscConstant.EVENT_SSC_UPDATE_OIR, -1);

        mLooper.processAllMessages();
        verify(mockUtListener).utConfigurationUpdateFailed(eq(tId), any());
        verify(mockSscTransaction).close();
    }

    @Test
    public void testUpdateClip_success() {
        int tId = mSscServiceImpl.updateCLIP(true);
        processEntireXmlDocQueryAsSuccess();
        processPutTransactionAsSuccess(ESsType.OIP, SscConstant.EVENT_SSC_UPDATE_OIP, -1);

        mLooper.processAllMessages();
        verify(mockUtListener).utConfigurationUpdated(eq(tId));
        verify(mockSscTransaction).close();
    }

    @Test
    public void testUpdateClip_failure() {
        int tId = mSscServiceImpl.updateCLIP(true);
        processEntireXmlDocQueryAsSuccess();
        processPutTransactionAsFailure(ESsType.OIP, SscConstant.EVENT_SSC_UPDATE_OIP, -1);

        mLooper.processAllMessages();
        verify(mockUtListener).utConfigurationUpdateFailed(eq(tId), any());
        verify(mockSscTransaction).close();
    }

    @Test
    public void testUpdateColr_success() {
        int tId = mSscServiceImpl.updateCOLR(SscConstant.TIR_PROVISIONED);
        processEntireXmlDocQueryAsSuccess();
        processPutTransactionAsSuccess(ESsType.TIR, SscConstant.EVENT_SSC_UPDATE_TIR, -1);

        mLooper.processAllMessages();
        verify(mockUtListener).utConfigurationUpdated(eq(tId));
        verify(mockSscTransaction).close();
    }

    @Test
    public void testUpdateColr_failure() {
        int tId = mSscServiceImpl.updateCOLR(SscConstant.TIR_PROVISIONED);
        processEntireXmlDocQueryAsSuccess();
        processPutTransactionAsFailure(ESsType.TIR, SscConstant.EVENT_SSC_UPDATE_TIR, -1);

        mLooper.processAllMessages();
        verify(mockUtListener).utConfigurationUpdateFailed(eq(tId), any());
        verify(mockSscTransaction).close();
    }

    @Test
    public void testUpdateColp_success() {
        int tId = mSscServiceImpl.updateCOLP(true);
        processEntireXmlDocQueryAsSuccess();
        processPutTransactionAsSuccess(ESsType.TIP, SscConstant.EVENT_SSC_UPDATE_TIP, -1);

        mLooper.processAllMessages();
        verify(mockUtListener).utConfigurationUpdated(eq(tId));
        verify(mockSscTransaction).close();
    }

    @Test
    public void testUpdateColp_failure() {
        int tId = mSscServiceImpl.updateCOLP(true);
        processEntireXmlDocQueryAsSuccess();
        processPutTransactionAsFailure(ESsType.TIP, SscConstant.EVENT_SSC_UPDATE_TIP, -1);

        mLooper.processAllMessages();
        verify(mockUtListener).utConfigurationUpdateFailed(eq(tId), any());
        verify(mockSscTransaction).close();
    }

    private void processEntireXmlDocQueryAsSuccess() {
        mLooper.processAllMessages();
        verify(mockSscTransaction, atLeast(mQueryCount))
                .startGetTransaction(captorQueryData.capture());
        mQueryCount++;

        SscServiceQueryData capturedData = captorQueryData.getValue();
        assertEquals(capturedData.getSsType(), ESsType.NONE);
        assertEquals(capturedData.getEventNumber(), SscConstant.EVENT_SSC_QUERY_DOCUMENT);

        capturedData.setResponseCode(200);
        Document doc = createEntireXmldoc();
        SscXmlGov.getInstance(SLOT_0).parseXmlStream(capturedData, doc);

        Message msg = Message.obtain();
        msg.what = SscConstant.EVENT_SSC_QUERY_DOCUMENT;
        msg.obj =  new SscRequestResult(SLOT_0, capturedData.getTransactionId(),
                SscConstant.REQUEST_SUCCESS, 200, -1);

        mCallbackHandler.sendMessage(msg);
    }

    private void processEntireXmlDocQueryAsFailure() {
        mLooper.processAllMessages();
        verify(mockSscTransaction, atLeast(mQueryCount))
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
        verify(mockSscTransaction, atLeast(mQueryCount))
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
        verify(mockSscTransaction, atLeast(mQueryCount))
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

        verify(mockSscTransaction, atLeast(mUpdateCount))
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
        verify(mockSscTransaction, atLeast(mUpdateCount))
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
            factory.setNamespaceAware(true);
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