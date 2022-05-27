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
    private boolean isCfnlProvisioned = false;
    private boolean isTimerInCfnr = false;
    private boolean isCbRuleActivated = false;
    private boolean isCfRuleActivated = false;
    private boolean isCwEnabled = false;
    private boolean isOipEnabled = false;
    private boolean isTipEnabled = false;
    private String errorPhrase = "";
    private String forwardNumber = "tel:+1234567890";
    private String defaultBehaviour = SscXmlFormat.PRESENTATION_NOT_RESTRICTED;
    private int mHttpSuccessResponse = SscConstant.HTTP_OK;
    private int mHttpErrorResponse = SscConstant.HTTP_CONFLICT;

    @Before
    public void setup() {
        MockitoAnnotations.initMocks(this);

        mQueryCount = 1;
        mUpdateCount = 1;
        mSscServiceImpl = new SscServiceImpl(SLOT_0);

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
    public void testBasicOperation_informErrorPhrase() {
        when(mockCarrierConfig.getBoolean(
                eq(CarrierConfig.Assets.KEY_UT_DISPLAY_ERROR_PHRASE_WITH_409_ERROR_BOOL)))
                .thenReturn(true);
        mHttpErrorResponse = SscConstant.HTTP_CONFLICT;
        errorPhrase = "check error phrase";

        int tId = mSscServiceImpl.queryCallForward(SscConstant.CONDITION_CFU, "");
        processEntireXmlDocQueryAsFailure();

        mLooper.processAllMessages();
        verify(mockUtListener).utConfigurationQueryFailed(eq(tId), captorReasonInfo.capture());

        ImsReasonInfo reasonInfo = captorReasonInfo.getValue();
        assertNotNull(reasonInfo);
        assertEquals(reasonInfo.getCode(), ImsReasonInfo.CODE_UT_NETWORK_ERROR);
        assertEquals(reasonInfo.getExtraMessage(), errorPhrase);

        verify(mockSscTransaction).close();
        verifyNoMoreInteractions(mockSscTransaction);
    }

    @Test
    public void testBasicOperation_notInformErrorPhrase() {
        when(mockCarrierConfig.getBoolean(
                eq(CarrierConfig.Assets.KEY_UT_DISPLAY_ERROR_PHRASE_WITH_409_ERROR_BOOL)))
                .thenReturn(false);
        mHttpErrorResponse = SscConstant.HTTP_CONFLICT;
        errorPhrase = "check error phrase";

        int tId = mSscServiceImpl.queryCallForward(SscConstant.CONDITION_CFU, "");
        processEntireXmlDocQueryAsFailure();

        mLooper.processAllMessages();
        verify(mockUtListener).utConfigurationQueryFailed(eq(tId), captorReasonInfo.capture());

        ImsReasonInfo reasonInfo = captorReasonInfo.getValue();
        assertNotNull(reasonInfo);
        assertNotEquals(reasonInfo.getExtraMessage(), errorPhrase);

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
        assertNotEquals(reasonInfo.getExtraMessage(), errorPhrase);

        verify(mockSscTransaction).close();
        verifyNoMoreInteractions(mockSscTransaction);
    }

    @Test
    public void testQueryCallBarringForServiceClass_singleRequestIcb() {
        isCbRuleActivated = false;
        int queryCondition = SscConstant.CONDITION_BIC_WR;

        int tId = mSscServiceImpl.queryCallBarringForServiceClass(queryCondition,
                SscServiceClassUtil.SERVICE_CLASS_VOICE);
        processEntireXmlDocQueryAsSuccess();
        processGetTransactionAsSuccess(ESsType.ICB, SscConstant.EVENT_SSC_QUERY_CALL_BARRING,
                queryCondition);

        mLooper.processAllMessages();
        verify(mockUtListener).utConfigurationCallBarringQueried(eq(tId), captorSsInfos.capture());

        ImsSsInfo[] cbInfos = captorSsInfos.getValue();
        assertNotNull(cbInfos);

        int xmlStatus = isCbRuleActivated ? SscConstant.STATUS_ENABLE : SscConstant.STATUS_DISABLE;
        assertEquals(cbInfos[0].mStatus, xmlStatus);

        verify(mockSscTransaction).close();
        verifyNoMoreInteractions(mockSscTransaction);
    }

    @Test
    public void testQueryCallBarringForServiceClass_singleRequestOcb() {
        isCbRuleActivated = true;
        int queryCondition = SscConstant.CONDITION_BAOC;

        int tId = mSscServiceImpl.queryCallBarringForServiceClass(queryCondition,
                SscServiceClassUtil.SERVICE_CLASS_VOICE);
        processEntireXmlDocQueryAsSuccess();
        processGetTransactionAsSuccess(ESsType.OCB, SscConstant.EVENT_SSC_QUERY_CALL_BARRING,
                queryCondition);

        mLooper.processAllMessages();
        verify(mockUtListener).utConfigurationCallBarringQueried(eq(tId), captorSsInfos.capture());

        ImsSsInfo[] cbInfos = captorSsInfos.getValue();
        assertNotNull(cbInfos);

        int xmlStatus = isCbRuleActivated ? SscConstant.STATUS_ENABLE : SscConstant.STATUS_DISABLE;
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
        processGetTransactionAsFailure(ESsType.OCB, SscConstant.EVENT_SSC_QUERY_CALL_BARRING,
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
        processGetTransactionAsSuccess(ESsType.CF, SscConstant.EVENT_SSC_QUERY_CALL_FORWARDING,
                queryCondition);

        mLooper.processAllMessages();
        verify(mockUtListener).utConfigurationCallForwardQueried(eq(tId), captorCfInfos.capture());

        ImsCallForwardInfo[] cfInfos = captorCfInfos.getValue();
        assertNotNull(cfInfos);
        assertEquals(cfInfos[0].mCondition, queryCondition);

        int xmlStatus = isCfRuleActivated ? SscConstant.STATUS_ENABLE : SscConstant.STATUS_DISABLE;
        assertEquals(cfInfos[0].mStatus, xmlStatus);
        if (TextUtils.isEmpty(forwardNumber)) {
            assertTrue(forwardNumber.contains(cfInfos[0].mNumber));
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

            processGetTransactionAsSuccess(ESsType.CF, SscConstant.EVENT_SSC_QUERY_CALL_FORWARDING,
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
        processGetTransactionAsFailure(ESsType.CF, SscConstant.EVENT_SSC_QUERY_CALL_FORWARDING,
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
        processGetTransactionAsSuccess(ESsType.CW, SscConstant.EVENT_SSC_QUERY_CALL_WAITING, -1);

        mLooper.processAllMessages();
        verify(mockUtListener).utConfigurationCallWaitingQueried(eq(tId), captorSsInfos.capture());

        ImsSsInfo[] cwInfos = captorSsInfos.getValue();
        assertNotNull(cwInfos);

        int xmlStatus = isCwEnabled ? SscConstant.STATUS_ENABLE : SscConstant.STATUS_DISABLE;
        assertEquals(cwInfos[0].mStatus, xmlStatus);

        verify(mockSscTransaction).close();
    }

    @Test
    public void testQueryCallWaiting_failure() {
        int tId = mSscServiceImpl.queryCallWaiting();
        processEntireXmlDocQueryAsSuccess();
        processGetTransactionAsFailure(ESsType.CW, SscConstant.EVENT_SSC_QUERY_CALL_WAITING, -1);

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

        int outGoingState = (defaultBehaviour == SscXmlFormat.PRESENTATION_NOT_RESTRICTED ?
                SscConstant.OIR_SUPPRESSION : SscConstant.OIR_INVOCATION);
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

        int xmlStatus = isOipEnabled ? SscConstant.STATUS_ENABLE : SscConstant.STATUS_DISABLE;
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
        defaultBehaviour = SscXmlFormat.PRESENTATION_RESTRICTED;

        int tId = mSscServiceImpl.queryCOLR();
        processEntireXmlDocQueryAsSuccess();
        processGetTransactionAsSuccess(ESsType.TIR, SscConstant.EVENT_SSC_QUERY_TIR, -1);

        mLooper.processAllMessages();
        verify(mockUtListener)
                .lineIdentificationSupplementaryServiceResponse(eq(tId), captorSsInfo.capture());

        ImsSsInfo ssInfo = captorSsInfo.getValue();
        assertNotNull(ssInfo);

        int provisionedStatus = (defaultBehaviour == SscXmlFormat.PRESENTATION_NOT_RESTRICTED ?
                SscConstant.TIR_NOT_PROVISIONED : SscConstant.TIR_PROVISIONED);
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

        int xmlStatus = isTipEnabled ? SscConstant.STATUS_ENABLE : SscConstant.STATUS_DISABLE;
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
        processPutTransactionAsSuccess(ESsType.ICB, SscConstant.EVENT_SSC_UPDATE_CALL_BARRING,
                SscConstant.CONDITION_BIC_WR);

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
        processPutTransactionAsFailure(ESsType.ICB, SscConstant.EVENT_SSC_UPDATE_CALL_BARRING,
                SscConstant.CONDITION_BIC_WR);

        mLooper.processAllMessages();
        verify(mockUtListener).utConfigurationUpdateFailed(eq(tId), any());
        verify(mockSscTransaction).close();
        verifyNoMoreInteractions(mockSscTransaction);
    }

    @Test
    public void testUpdateCallForward_singleRequest() {
        isCfnlProvisioned = false;

        int tId = mSscServiceImpl.updateCallForward(SscConstant.ACTION_REGISTRATION,
                SscConstant.CONDITION_CFNRC, "+1234567890", 0, 0);
        processEntireXmlDocQueryAsSuccess();
        processPutTransactionAsSuccess(ESsType.CF, SscConstant.EVENT_SSC_UPDATE_CALL_FORWARD,
                SscConstant.CONDITION_CFNRC);

        mLooper.processAllMessages();
        verify(mockUtListener).utConfigurationUpdated(eq(tId));
        verify(mockSscTransaction).close();
        verifyNoMoreInteractions(mockSscTransaction);
    }

    @Test
    public void testUpdateCallForward_CfnrcAndCfnl() {
        isCfnlProvisioned = true;

        when(mockCarrierConfig.getBoolean(
                eq(CarrierConfig.Assets.KEY_UT_SUPPORT_CF_ACTION_ERASURE_BOOL))).thenReturn(true);

        int tId = mSscServiceImpl.updateCallForward(SscConstant.ACTION_ERASURE,
                SscConstant.CONDITION_CFNRC, null, 0, 0);
        processEntireXmlDocQueryAsSuccess();
        processPutTransactionAsSuccess(ESsType.CF, SscConstant.EVENT_SSC_UPDATE_CALL_FORWARD,
                SscConstant.CONDITION_CFNRC);
        processPutTransactionAsSuccess(ESsType.CF, SscConstant.EVENT_SSC_UPDATE_CALL_FORWARD,
                SscConstant.CONDITION_CFNL);

        mLooper.processAllMessages();
        verify(mockUtListener).utConfigurationUpdated(eq(tId));
        verify(mockSscTransaction).close();
        verifyNoMoreInteractions(mockSscTransaction);
    }

    @Test
    public void testUpdateCallForward_CfnrAndTimer() {
        isTimerInCfnr = false;

        int tId = mSscServiceImpl.updateCallForward(SscConstant.ACTION_ACTIVATION,
                SscConstant.CONDITION_CFNR, null, 0, 20);
        processEntireXmlDocQueryAsSuccess();
        processPutTransactionAsSuccess(ESsType.CF, SscConstant.EVENT_SSC_UPDATE_CALL_FORWARD,
                SscConstant.CONDITION_CFNR);
        processPutTransactionAsSuccess(ESsType.CF, SscConstant.EVENT_SSC_UPDATE_CALL_FORWARD,
                SscConstant.CONDITION_CFNR_TIMER);

        mLooper.processAllMessages();
        verify(mockUtListener).utConfigurationUpdated(eq(tId));
        verify(mockSscTransaction).close();
        verifyNoMoreInteractions(mockSscTransaction);
    }

    @Test
    public void testUpdateCallForward_CfnrWithTimer() {
        isTimerInCfnr = true;

        int tId = mSscServiceImpl.updateCallForward(SscConstant.ACTION_ACTIVATION,
                SscConstant.CONDITION_CFNR, null, 0, 20);
        processEntireXmlDocQueryAsSuccess();

        processPutTransactionAsSuccess(ESsType.CF, SscConstant.EVENT_SSC_UPDATE_CALL_FORWARD,
                SscConstant.CONDITION_CFNR);

        mLooper.processAllMessages();
        verify(mockUtListener).utConfigurationUpdated(eq(tId));
        verify(mockSscTransaction).close();
        verifyNoMoreInteractions(mockSscTransaction);
    }

    @Test
    public void testUpdateCallForward_CfAll() {
        isTimerInCfnr = true;
        isCfnlProvisioned = false;

        int tId = mSscServiceImpl.updateCallForward(SscConstant.ACTION_DEACTIVATION,
                SscConstant.CONDITION_CFA, null, 0, 20);

        for (int c = SscConstant.CONDITION_CFNRC; c >= SscConstant.CONDITION_CFU; c--) {
            if (SscXmlGov.getInstance(SLOT_0).isXmlDataPresent() == false) {
                processEntireXmlDocQueryAsSuccess();
            }

            processPutTransactionAsSuccess(ESsType.CF, SscConstant.EVENT_SSC_UPDATE_CALL_FORWARD,
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
        processPutTransactionAsFailure(ESsType.CF, SscConstant.EVENT_SSC_UPDATE_CALL_FORWARD,
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
        processPutTransactionAsSuccess(ESsType.CW, SscConstant.EVENT_SSC_UPDATE_CALL_WAITING, -1);

        mLooper.processAllMessages();
        verify(mockUtListener).utConfigurationUpdated(eq(tId));
        verify(mockSscTransaction).close();
    }

    @Test
    public void testUpdateCallWaiting_failure() {
        int tId = mSscServiceImpl.updateCallWaiting(true, SscServiceClassUtil.SERVICE_CLASS_VOICE);
        processEntireXmlDocQueryAsSuccess();
        processPutTransactionAsFailure(ESsType.CW, SscConstant.EVENT_SSC_UPDATE_CALL_WAITING, -1);

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
        assertEquals(capturedData.getEventNumber(), SscConstant.EVENT_SSC_BASE);

        capturedData.setResponseCode(200);
        Document doc = createEntireXmldoc();
        SscXmlGov.getInstance(SLOT_0).setXmlData(doc);
        SscXmlGov.getInstance(SLOT_0).parseXmlStream(capturedData, doc);

        Message msg = Message.obtain();
        msg.what = SscConstant.EVENT_SSC_BASE;
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
        assertEquals(capturedData.getEventNumber(), SscConstant.EVENT_SSC_BASE);

        capturedData.setResponseCode(mHttpErrorResponse);

        Message msg = Message.obtain();
        msg.what = SscConstant.EVENT_SSC_BASE;
        SscRequestResult rr = new SscRequestResult(SLOT_0, capturedData.getTransactionId(),
                SscConstant.REQUEST_FAILURE, mHttpErrorResponse, -1);
        if (!TextUtils.isEmpty(errorPhrase)) {
            rr.setSscServiceData(new ErrorResponseData(SLOT_0, capturedData.getSsType(),
                    capturedData.getEventNumber(), capturedData.getTransactionId(),
                    SscConstant.STATUS_DISABLE, capturedData.getResponseCode(), errorPhrase));
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

    private void processPutTransactionAsSuccess(ESsType validSsType, int validEventNum,
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
        String xml = "<ss:simservs xmlns:SOAP-ENV=\"http://schemas.xmlsoap.org/soap/envelope/\" "
                + "xmlns:SOAP-ENC=\"http://schemas.xmlsoap.org/soap/encoding/\" "
                + "xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\" "
                + "xmlns:xsd=\"http://www.w3.org/2001/XMLSchema\" "
                + "xmlns:cp=\"urn:ietf:params:xml:ns:common-policy\" "
                + "xmlns:ss=\"http://uri.etsi.org/ngn/params/xml/simservs/xcap\" "
                + "xmlns:ocp=\"urn:oma:xml:xdm:common-policy\" "
                + "xmlns:utns=\"urn:com:att:tlv:utx\" "
                + "xmlns:xe=\"urn:ietf:params:xml:ns:xcap-error\" "
                + "xmlns:data=\"http://com/alu/icm/fs5000dbv5_0/data\" "
                + "xmlns:prs=\"http://www.nokia.com/prs/SubscriptionAPI\"> \n"
                //ICB
                + "<ss:incoming-communication-barring active=\"true\">\n"
                + "<cp:ruleset xmlns:cp=\"urn:ietf:params:xml:ns:common-policy\">\n"
                + "<cp:rule id=\"call-barring-all-incoming\">\n"
                + "<cp:conditions>\n"
                + (isCbRuleActivated ? "" : "<ss:rule-deactivated/>\n")
                + "</cp:conditions>\n"
                + "<cp:actions>\n"
                + "<ss:allow>false</ss:allow>\n"
                + "</cp:actions>\n"
                + "</cp:rule>\n"
                + "<cp:rule id=\"call-barring-incoming-in-roaming\">\n"
                + "<cp:conditions>\n"
                + (isCbRuleActivated ? "" : "<ss:rule-deactivated/>\n")
                + "<ss:roaming/>\n"
                + "</cp:conditions>\n"
                + "<cp:actions>\n"
                + "<ss:allow>false</ss:allow>\n"
                + "</cp:actions>\n"
                + "</cp:rule>\n"
                + "</cp:ruleset>\n"
                + "</ss:incoming-communication-barring>\n"
                //OCB
                + "<ss:outgoing-communication-barring active=\"true\">\n"
                + "<cp:ruleset xmlns:cp=\"urn:ietf:params:xml:ns:common-policy\">\n"
                + "<cp:rule id=\"call-barring-all-outgoing\">\n"
                + "<cp:conditions>\n"
                + (isCbRuleActivated ? "" : "<ss:rule-deactivated/>\n")
                + "</cp:conditions>\n"
                + "<cp:actions>\n"
                + "<ss:allow>false</ss:allow>\n"
                + "</cp:actions>\n"
                + "</cp:rule>\n"
                + "<cp:rule id=\"call-barring-outgoing-international\">\n"
                + "<cp:conditions>\n"
                + (isCbRuleActivated ? "" : "<ss:rule-deactivated/>\n")
                + "<ss:international/>\n"
                + "</cp:conditions>\n"
                + "<cp:actions>\n"
                + "<ss:allow>false</ss:allow>\n"
                + "</cp:actions>\n"
                + "</cp:rule>\n"
                + "<cp:rule id=\"call-barring-outgoing-internationalExHC\">\n"
                + "<cp:conditions>\n"
                + (isCbRuleActivated ? "" : "<ss:rule-deactivated/>\n")
                + "<ss:international-exHC/>\n"
                + "</cp:conditions>\n"
                + "<cp:actions>\n"
                + "<ss:allow>false</ss:allow>\n"
                + "</cp:actions>\n"
                + "</cp:rule>\n"
                + "</cp:ruleset>\n"
                + "</ss:outgoing-communication-barring>"
                // CW
                + "<ss:communication-waiting active=\""
                        + (isCwEnabled ? "true" : "false") + "\"/> \n"
                // CD
                + "<ss:communication-diversion active=\"true\"> \n"
                + (isTimerInCfnr ? "" : "<ss:NoReplyTimer>20</ss:NoReplyTimer> \n")
                + "<cp:ruleset> \n"
                + "<cp:rule id=\"call-diversion-unconditional\"> \n"
                + "<cp:conditions> \n"
                + (isCfRuleActivated ? "" : "<ss:rule-deactivated/>\n")
                + "</cp:conditions> \n"
                + "<cp:actions> \n"
                + "<ss:forward-to> \n"
                + "<ss:target>" + forwardNumber + "</ss:target> \n"
                + "<ss:notify-caller>true</ss:notify-caller> \n"
                + "<ss:reveal-identity-to-caller>true</ss:reveal-identity-to-caller>\n"
                + "<ss:reveal-served-user-identity-to-caller>false"
                + "</ss:reveal-served-user-identity-to-caller>\n"
                + "<ss:notify-served-user>false</ss:notify-served-user> \n"
                + "<ss:notify-served-user-on-outbound-call>false"
                + "</ss:notify-served-user-on-outbound-call> \n"
                + "<ss:reveal-identity-to-target>false</ss:reveal-identity-to-target> \n"
                + "</ss:forward-to> \n"
                + "</cp:actions> \n"
                + "</cp:rule> \n"
                + "<cp:rule id=\"call-diversion-busy\"> \n"
                + "<cp:conditions> \n"
                + (isCfRuleActivated ? "" : "<ss:rule-deactivated/>\n")
                + "<ss:busy/> \n"
                + "</cp:conditions> \n"
                + "<cp:actions> \n"
                + "<ss:forward-to> \n"
                + "<ss:target>" + forwardNumber + "</ss:target> \n"
                + "<ss:notify-caller>true</ss:notify-caller> \n"
                + "<ss:reveal-identity-to-caller>true</ss:reveal-identity-to-caller> \n"
                + "<ss:reveal-served-user-identity-to-caller>false"
                + "</ss:reveal-served-user-identity-to-caller> \n"
                + "<ss:reveal-identity-to-target>false</ss:reveal-identity-to-target> \n"
                + "</ss:forward-to> \n"
                + "</cp:actions> \n"
                + "</cp:rule> \n"
                + "<cp:rule id=\"call-diversion-no-reply\"> \n"
                + "<cp:conditions> \n"
                + (isCfRuleActivated ? "" : "<ss:rule-deactivated/>\n")
                + "<ss:no-answer/> \n"
                + "</cp:conditions> \n"
                + "<cp:actions> \n"
                + "<ss:forward-to> \n"
                + "<ss:target>" + forwardNumber + "</ss:target> \n"
                + (isTimerInCfnr ? "<ss:NoReplyTimer>20</ss:NoReplyTimer> \n" : "")
                + "<ss:notify-caller>true</ss:notify-caller> \n"
                + "<ss:reveal-identity-to-caller>true</ss:reveal-identity-to-caller> \n"
                + "<ss:reveal-served-user-identity-to-caller>false"
                + "</ss:reveal-served-user-identity-to-caller> \n"
                + "<ss:reveal-identity-to-target>false</ss:reveal-identity-to-target> \n"
                + "</ss:forward-to> \n"
                + "</cp:actions> \n"
                + "</cp:rule> \n"
                + "<cp:rule id=\"call-diversion-not-reachable\"> \n"
                + "<cp:conditions> \n"
                + (isCfRuleActivated ? "" : "<ss:rule-deactivated/>\n")
                + "<ss:not-reachable/> \n"
                + "</cp:conditions> \n"
                + "<cp:actions> \n"
                + "<ss:forward-to> \n"
                + "<ss:target>" + forwardNumber + "</ss:target> \n"
                + "<ss:notify-caller>true</ss:notify-caller> \n"
                + "<ss:reveal-identity-to-caller>true</ss:reveal-identity-to-caller> \n"
                + "<ss:reveal-served-user-identity-to-caller>false"
                + "</ss:reveal-served-user-identity-to-caller> \n"
                + "<ss:reveal-identity-to-target>false</ss:reveal-identity-to-target> \n"
                + "</ss:forward-to> \n"
                + "</cp:actions> \n"
                + "</cp:rule> \n"
                + (isCfnlProvisioned ? "<cp:rule id=\"call-diversion-not-loggedin\"> \n"
                        + "<cp:conditions> \n"
                        + (isCfRuleActivated ? "" : "<ss:rule-deactivated/>\n")
                        + "<ss:not-registered/> \n"
                        + "</cp:conditions> \n"
                        + "<cp:actions> \n"
                        + "<ss:forward-to> \n"
                        + "<ss:target>" + forwardNumber + "</ss:target> \n"
                        + "<ss:notify-caller>true</ss:notify-caller> \n"
                        + "<ss:reveal-identity-to-caller>true</ss:reveal-identity-to-caller> \n"
                        + "<ss:reveal-served-user-identity-to-caller>false"
                        + "</ss:reveal-served-user-identity-to-caller> \n"
                        + "<ss:reveal-identity-to-target>false</ss:reveal-identity-to-target> \n"
                        + "</ss:forward-to> \n"
                        + "</cp:actions> \n"
                        + "</cp:rule> \n" : "")
                + "</cp:ruleset> \n"
                + "</ss:communication-diversion> \n"
                + "<ss:communication-diversion-serv-cap active=\"true\"> \n"
                + "<ss:serv-cap-conditions> \n"
                + "<ss:serv-cap-anonymous provisioned=\"false\"/> \n"
                + "<ss:serv-cap-busy provisioned=\"true\"/> \n"
                + "<ss:serv-cap-external-list provisioned=\"false\"/> \n"
                + "<ss:serv-cap-identity provisioned=\"false\"/> \n"
                + "<ss:serv-cap-media> \n"
                + "<ss:media>audio</ss:media> \n"
                + "</ss:serv-cap-media> \n"
                + "<ss:serv-cap-not-registered provisioned=\"false\"/> \n"
                + "<ss:serv-cap-no-answer provisioned=\"true\"/> \n"
                + "<ss:serv-cap-not-reachable provisioned=\"true\"/> \n"
                + "<ss:serv-cap-presence-status provisioned=\"false\"/> \n"
                + "<ss:serv-cap-rule-deactivated provisioned=\"true\"/>\n"
                + "<ss:serv-cap-selective provisioned=\"false\"/> \n"
                + "<ss:serv-cap-validity provisioned=\"false\"/> \n"
                + "<ss:serv-cap-unconditional provisioned=\"true\"/> \n"
                + "<ss:serv-cap-default provisioned=\"true\"/> \n"
                + "</ss:serv-cap-conditions> \n"
                + "<ss:serv-cap-actions> \n"
                + "<ss:serv-cap-target> \n"
                + "<ss:telephony-type/> \n"
                + "</ss:serv-cap-target> \n"
                + "<ss:serv-cap-notify-caller provisioned=\"false\"/> \n"
                + "<ss:serv-cap-notify-served-user provisioned=\"false\"/> \n"
                + "<ss:serv-cap-notify-served-user-on-outbound-call provisioned=\"false\"/> \n"
                + "<ss:serv-cap-reveal-identity-to-caller provisioned=\"false\"/> \n"
                + "<ss:serv-cap-reveal-served-user-identity-to-caller provisioned=\"false\"/> \n"
                + "<ss:serv-cap-reveal-identity-to-target provisioned=\"false\"/> \n"
                + "</ss:serv-cap-actions> \n"
                + "</ss:communication-diversion-serv-cap> \n"
                // OIP
                + "<ss:originating-identity-presentation active=\""
                        + (isOipEnabled ? "true" : "false") + "\"/> \n"
                // OIR
                + "<ss:originating-identity-presentation-restriction active=\"true\"> \n"
                + "<ss:default-behaviour>" + defaultBehaviour + "</ss:default-behaviour> \n"
                + "</ss:originating-identity-presentation-restriction> \n"
                // TIP
                + "<ss:terminating-identity-presentation active=\""
                        + (isTipEnabled ? "true" : "false") + "\"/> \n"
                // TIR
                + "<ss:terminating-identity-presentation-restriction active=\"true\"> \n"
                + "<ss:default-behaviour>" + defaultBehaviour + "</ss:default-behaviour> \n"
                + "</ss:terminating-identity-presentation-restriction> \n"
                + "</ss:simservs> \n";

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