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
import static org.junit.Assert.assertNotNull;
import static org.junit.Assert.assertNull;
import static org.junit.Assert.fail;
import static org.mockito.ArgumentMatchers.any;
import static org.mockito.ArgumentMatchers.anyBoolean;
import static org.mockito.ArgumentMatchers.anyLong;
import static org.mockito.ArgumentMatchers.anyString;
import static org.mockito.ArgumentMatchers.eq;
import static org.mockito.Mockito.atLeast;
import static org.mockito.Mockito.verify;
import static org.mockito.Mockito.verifyNoMoreInteractions;
import static org.mockito.Mockito.when;

import android.os.Handler;
import android.os.Message;
import android.telephony.CarrierConfigManager;
import android.util.Pair;

import com.android.imsstack.core.agents.ConfigAgent;
import com.android.imsstack.core.agents.GbaInterface;
import com.android.imsstack.core.config.CarrierConfig;
import com.android.imsstack.enabler.ssc.data.CbServiceUpdateData;
import com.android.imsstack.enabler.ssc.data.CfServiceUpdateData;
import com.android.imsstack.enabler.ssc.data.SscData;
import com.android.imsstack.enabler.ssc.data.SscRequestResult;
import com.android.imsstack.enabler.ssc.data.SscServiceData;
import com.android.imsstack.enabler.ssc.data.SscServiceQueryData;

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
public class SscTransactionTest {
    private static final int SLOT_0 = 0;

    private FakeSscTransaction mSscTransaction;
    private int mGbaMode = SscConfig.GBA_ME;
    private int mTransactionId = 1;
    private String mDefaultXui = "tel:+1234567890";
    private String mDefaultRequestUri = "/simservs.ngn.etsi.org/users/" + mDefaultXui
            + "/simservs.xml";

    @Captor ArgumentCaptor<Message> mMessageCaptor;

    @Mock private CarrierConfig mMockCarrierConfig;
    @Mock private ConfigAgent mMockConfigAgent;
    @Mock private GbaInterface mMockGbaAgent;
    @Mock private Handler mMockCallbackHandler;
    @Mock private SscAuthAgent mMockSscAuthAgent;
    @Mock private SscHttpConnectionGov mMockSscHttpConnectionGov;
    @Mock private SscNetConnection mMockSscConnection;
    @Mock private SscServiceStateAgent mMockSscServiceStateAgent;
    @Mock private SscUrl mMockSscUrl;
    @Mock private SscUtils mMockSscUtils;
    @Mock private SscXmlGov mMockSscXmlGov;
    @Mock private SscXui mMockSscXui;

    @Before
    public void setup() {
        MockitoAnnotations.initMocks(this);

        when(mMockSscAuthAgent.isCredentialInfoUpdated()).thenReturn(false);
        when(mMockSscXui.getXui(eq(SLOT_0), eq(null))).thenReturn(mDefaultXui);
        when(mMockSscUrl.getQueryUri(any(), eq(mDefaultXui))).thenReturn(mDefaultRequestUri);
        when(mMockSscUrl.getUpdateUri(any(), eq(mDefaultXui))).thenReturn(mDefaultRequestUri);
        when(mMockConfigAgent.getCarrierConfig()).thenReturn(mMockCarrierConfig);
        when(mMockCarrierConfig.getInt(eq(CarrierConfigManager.KEY_GBA_MODE_INT)))
                .thenReturn(mGbaMode);
        when(mMockSscConnection.isConnected()).thenReturn(true);

        SscConfig.setConfigAgent(SLOT_0, mMockConfigAgent);
        ((SscNetConnectionGov) SscNetConnectionGov.getInstance()).setSscNetConnection(SLOT_0,
                mMockSscConnection);

        mSscTransaction = new FakeSscTransaction(SLOT_0, mMockCallbackHandler);
    }

    @After
    public void tearDown() {
        mSscTransaction.close();
    }

    /*
    @Test
    public void startTransaction_blockedByTrmAgent() {
        // TODO: TRM
    }
    */

    @Test
    public void startTransaction_requestToConnectFailure() {
        when(mMockSscConnection.isConnected()).thenReturn(false);
        when(mMockSscConnection.connect()).thenReturn(false);

        mSscTransaction.startGetTransaction(getQueryData(SscConstant.CONDITION_CFU));
        sleepToWaitThreadRun();

        processTransactionFailure(SscConstant.EVENT_SSC_QUERY_CF, false);
    }

    @Test
    public void startTransaction_pdnDisconnected() {
        when(mMockSscConnection.isConnected()).thenReturn(false);
        when(mMockSscConnection.connect()).thenReturn(true);

        mSscTransaction.startGetTransaction(getQueryData(SscConstant.CONDITION_CFU));
        sleepToWaitThreadRun();

        triggerCallbackMessage(SscNetConnection.EVENT_PDN_DISCONNECTED);

        processTransactionFailure(SscConstant.EVENT_SSC_QUERY_CF, false);
    }

    @Test
    public void startTransaction_pdnRequestTimeout() {
        when(mMockSscConnection.isConnected()).thenReturn(false);
        when(mMockSscConnection.connect()).thenReturn(true);

        mSscTransaction.startGetTransaction(getQueryData(SscConstant.CONDITION_CFU));
        sleepToWaitThreadRun();

        triggerCallbackMessage(SscNetConnection.EVENT_PDN_REQUEST_TIMEOUT);

        processTransactionFailure(SscConstant.EVENT_SSC_QUERY_CF, false);
    }

    @Test
    public void startTransaction_pdnConnectionFailure() {
        when(mMockSscConnection.isConnected()).thenReturn(false);
        when(mMockSscConnection.connect()).thenReturn(true);

        mSscTransaction.startGetTransaction(getQueryData(SscConstant.CONDITION_CFU));
        sleepToWaitThreadRun();

        triggerCallbackMessage(SscNetConnection.EVENT_PDN_CONNECTION_FAILED);

        processTransactionFailure(SscConstant.EVENT_SSC_QUERY_CF, false);
    }

    @Test
    public void startGetTransaction_wrongXui() {
        when(mMockSscXui.getXui(eq(SLOT_0), eq(null))).thenReturn(null);

        mSscTransaction.startGetTransaction(getQueryData(SscConstant.CONDITION_CFU));
        sleepToWaitThreadRun();

        verify(mMockSscXui).getXui(eq(SLOT_0), eq(null));

        processTransactionFailure(SscConstant.EVENT_SSC_QUERY_CF, false);
    }

    @Test
    public void startGetTransaction_wrongRequestUri() {
        when(mMockSscUrl.getQueryUri(any(), anyString())).thenReturn(null);

        mSscTransaction.startGetTransaction(getQueryData(SscConstant.CONDITION_CFU));
        sleepToWaitThreadRun();

        verify(mMockSscUrl).getQueryUri(any(), anyString());

        processTransactionFailure(SscConstant.EVENT_SSC_QUERY_CF, false);
    }

    @Test
    public void startGetTransaction_gbaFailure() {
        String nafFqdn = "xcap.3gpp.com";
        String securityProtocol = "TLS_NULL_WITH_NULL_NULL";
        int appType = SscConstant.APPTYPE_ISIM;

        when(mMockSscAuthAgent.isCredentialInfoUpdated()).thenReturn(false, true);
        when(mMockSscHttpConnectionGov.sendRequest(eq(SLOT_0),
                eq(SscHttpConnection.HTTP_GET_REQUEST), eq(mDefaultRequestUri), eq(mDefaultXui),
                eq(""))).thenReturn(SscConstant.HTTP_UNAUTHORIZED);
        when(mMockSscUtils.getTelephonySimType(eq(SLOT_0))).thenReturn(appType);
        when(mMockSscAuthAgent.getNafFqdnFromRealm()).thenReturn(nafFqdn);
        when(mMockSscAuthAgent.getCipherSuite()).thenReturn(securityProtocol);
        when(mMockGbaAgent.getGbaKey(eq(appType), eq(mGbaMode), anyBoolean(), eq(nafFqdn),
                eq(securityProtocol), eq(true))).thenReturn(null);

        mSscTransaction.startGetTransaction(getQueryData(SscConstant.CONDITION_CFU));
        sleepToWaitThreadRun();

        verify(mMockSscAuthAgent, atLeast(1)).isCredentialInfoUpdated();
        verify(mMockSscHttpConnectionGov).sendRequest(eq(SLOT_0),
                eq(SscHttpConnection.HTTP_GET_REQUEST), eq(mDefaultRequestUri), eq(mDefaultXui),
                eq(""));
        verify(mMockSscAuthAgent, atLeast(2)).isCredentialInfoUpdated();
        verify(mMockSscAuthAgent).getNafFqdnFromRealm();
        verify(mMockSscAuthAgent).getCipherSuite();
        verify(mMockGbaAgent).getGbaKey(eq(appType), eq(mGbaMode), anyBoolean(), eq(nafFqdn),
                eq(securityProtocol), eq(true));
        verify(mMockSscAuthAgent).setIsCredentialInfoUpdated(eq(false));
        verify(mMockSscServiceStateAgent).setGbaRequestFailed(eq(SLOT_0), eq(true));

        processTransactionFailure(SscConstant.EVENT_SSC_QUERY_CF, false);
    }

    @Test
    public void startGetTransaction_unauthorizedWithGbaKey() {
        String nafFqdn = "xcap.3gpp.com";
        String securityProtocol = "TLS_NULL_WITH_NULL_NULL";
        int appType = SscConstant.APPTYPE_USIM;

        when(mMockSscAuthAgent.isCredentialInfoUpdated()).thenReturn(false, true);
        when(mMockSscHttpConnectionGov.sendRequest(SLOT_0, SscHttpConnection.HTTP_GET_REQUEST,
                mDefaultRequestUri, mDefaultXui, "")).thenReturn(SscConstant.HTTP_UNAUTHORIZED);
        when(mMockSscUtils.getTelephonySimType(eq(SLOT_0))).thenReturn(appType);
        when(mMockSscAuthAgent.getNafFqdnFromRealm()).thenReturn(nafFqdn);
        when(mMockSscAuthAgent.getCipherSuite()).thenReturn(securityProtocol);
        when(mMockGbaAgent.getGbaKey(eq(appType), eq(mGbaMode), anyBoolean(), eq(nafFqdn),
                eq(securityProtocol), eq(true)))
                .thenReturn(new Pair<String, String>("B-TID", "Ks_NAF_KEY"));

        mSscTransaction.startGetTransaction(getQueryData(SscConstant.CONDITION_CFU));
        sleepToWaitThreadRun();

        verify(mMockSscAuthAgent, atLeast(1)).isCredentialInfoUpdated();
        verify(mMockSscHttpConnectionGov, atLeast(1)).sendRequest(SLOT_0,
                SscHttpConnection.HTTP_GET_REQUEST, mDefaultRequestUri, mDefaultXui, "");
        verify(mMockSscAuthAgent, atLeast(2)).isCredentialInfoUpdated();
        verify(mMockGbaAgent).getGbaKey(eq(appType), eq(mGbaMode), anyBoolean(), eq(nafFqdn),
                eq(securityProtocol), eq(true));
        verify(mMockSscHttpConnectionGov, atLeast(2)).sendRequest(SLOT_0,
                SscHttpConnection.HTTP_GET_REQUEST, mDefaultRequestUri, mDefaultXui, "");
        verify(mMockSscAuthAgent).setIsCredentialInfoUpdated(eq(false));

        processTransactionFailure(SscConstant.EVENT_SSC_QUERY_CF, false);
    }

    @Test
    public void startGetTransaction_requestFailure() {
        when(mMockSscHttpConnectionGov.sendRequest(SLOT_0, SscHttpConnection.HTTP_GET_REQUEST,
                mDefaultRequestUri, mDefaultXui, "")).thenReturn(SscHttpConnection.REQUEST_FAILED);

        mSscTransaction.startGetTransaction(getQueryData(SscConstant.CONDITION_CFU));
        sleepToWaitThreadRun();

        verify(mMockSscHttpConnectionGov).sendRequest(SLOT_0, SscHttpConnection.HTTP_GET_REQUEST,
                mDefaultRequestUri, mDefaultXui, "");
        verify(mMockSscServiceStateAgent).setSocketConnectionExpired(eq(SLOT_0), eq(true));

        processTransactionFailure(SscConstant.EVENT_SSC_QUERY_CF, false);
    }

    @Test
    public void startGetTransaction_requestFailureByDns() {
        when(mMockSscHttpConnectionGov.sendRequest(SLOT_0, SscHttpConnection.HTTP_GET_REQUEST,
                mDefaultRequestUri, mDefaultXui, ""))
                .thenReturn(SscHttpConnection.REQUEST_FAILED_BY_DNS);

        mSscTransaction.startGetTransaction(getQueryData(SscConstant.CONDITION_CFU));
        sleepToWaitThreadRun();

        verify(mMockSscHttpConnectionGov).sendRequest(SLOT_0, SscHttpConnection.HTTP_GET_REQUEST,
                mDefaultRequestUri, mDefaultXui, "");
        verify(mMockSscServiceStateAgent).setDnsQueryFailed(eq(SLOT_0), eq(true));

        processTransactionFailure(SscConstant.EVENT_SSC_QUERY_CF, false);
    }

    @Test
    public void startGetTransaction_httpErrorResponseWithErrorPhrase() {
        int httpErrorResponse = SscConstant.HTTP_CONFLICT;
        SscServiceQueryData queryData = getQueryData(SscConstant.CONDITION_CFU);

        when(mMockSscHttpConnectionGov.sendRequest(SLOT_0, SscHttpConnection.HTTP_GET_REQUEST,
                        mDefaultRequestUri, mDefaultXui, "")).thenReturn(httpErrorResponse);
        when(mMockSscHttpConnectionGov.getInputStream(eq(SLOT_0))).thenReturn(getInputStream());
        when(mMockSscXmlGov.parseXmlStream(eq(queryData), any()))
                .thenReturn(parseXmlStream(queryData));

        mSscTransaction.startGetTransaction(queryData);
        sleepToWaitThreadRun();

        verify(mMockSscHttpConnectionGov).sendRequest(SLOT_0, SscHttpConnection.HTTP_GET_REQUEST,
                mDefaultRequestUri, mDefaultXui, "");
        verify(mMockSscServiceStateAgent).setErrorResponseCode(eq(SLOT_0), eq(httpErrorResponse));
        verify(mMockSscHttpConnectionGov).getInputStream(eq(SLOT_0));
        verify(mMockSscXmlGov).parseXmlStream(eq(queryData), any());

        processTransactionFailure(SscConstant.EVENT_SSC_QUERY_CF, true);
    }

    @Test
    public void startGetTransaction_httpErrorResponseWithoutErrorPhrase() {
        int httpErrorResponse = SscConstant.HTTP_CONFLICT;
        when(mMockSscHttpConnectionGov.sendRequest(SLOT_0, SscHttpConnection.HTTP_GET_REQUEST,
                mDefaultRequestUri, mDefaultXui, "")).thenReturn(httpErrorResponse);
        when(mMockSscHttpConnectionGov.getInputStream(eq(SLOT_0))).thenReturn(null);

        mSscTransaction.startGetTransaction(getQueryData(SscConstant.CONDITION_CFU));
        sleepToWaitThreadRun();

        verify(mMockSscHttpConnectionGov).sendRequest(SLOT_0, SscHttpConnection.HTTP_GET_REQUEST,
                mDefaultRequestUri, mDefaultXui, "");
        verify(mMockSscServiceStateAgent).setErrorResponseCode(eq(SLOT_0), eq(httpErrorResponse));
        verify(mMockSscHttpConnectionGov).getInputStream(eq(SLOT_0));

        processTransactionFailure(SscConstant.EVENT_SSC_QUERY_CF, false);
    }

    @Test
    public void startGetTransaction_httpErrorResponseForbidden() {
        int httpErrorResponse = SscConstant.HTTP_FORBIDDEN;
        when(mMockSscHttpConnectionGov.sendRequest(SLOT_0, SscHttpConnection.HTTP_GET_REQUEST,
                mDefaultRequestUri, mDefaultXui, "")).thenReturn(httpErrorResponse);

        mSscTransaction.startGetTransaction(getQueryData(SscConstant.CONDITION_CFU));
        sleepToWaitThreadRun();

        verify(mMockSscHttpConnectionGov).sendRequest(SLOT_0, SscHttpConnection.HTTP_GET_REQUEST,
                mDefaultRequestUri, mDefaultXui, "");
        verify(mMockSscAuthAgent).setIsCredentialInfoUpdated(eq(false));
        verify(mMockSscServiceStateAgent).setErrorResponseCode(eq(SLOT_0), eq(httpErrorResponse));

        processTransactionFailure(SscConstant.EVENT_SSC_QUERY_CF, false);
    }

    @Test
    public void startGetTransaction_successButParsingError() {
        int httpSuccessResponse = SscConstant.HTTP_OK;
        SscServiceQueryData queryData = getQueryData(SscConstant.CONDITION_CFU);

        when(mMockSscHttpConnectionGov.sendRequest(SLOT_0, SscHttpConnection.HTTP_GET_REQUEST,
                mDefaultRequestUri, mDefaultXui, "")).thenReturn(httpSuccessResponse);
        when(mMockSscHttpConnectionGov.getInputStream(eq(SLOT_0))).thenReturn(getInputStream());
        when(mMockSscXmlGov.parseXmlStream(eq(queryData), any())).thenReturn(null);

        mSscTransaction.startGetTransaction(queryData);
        sleepToWaitThreadRun();

        processTransactionFailure(SscConstant.EVENT_SSC_QUERY_CF, false);
    }

    @Test
    public void startGetTransaction_success() {
        int httpSuccessResponse = SscConstant.HTTP_OK;
        SscServiceQueryData queryData = getQueryData(SscConstant.CONDITION_CFU);

        when(mMockSscConnection.isConnected()).thenReturn(false, true);
        when(mMockSscConnection.connect()).thenReturn(true);
        when(mMockSscHttpConnectionGov.sendRequest(SLOT_0, SscHttpConnection.HTTP_GET_REQUEST,
                mDefaultRequestUri, mDefaultXui, "")).thenReturn(httpSuccessResponse);
        when(mMockSscHttpConnectionGov.getInputStream(eq(SLOT_0))).thenReturn(getInputStream());
        when(mMockSscXmlGov.parseXmlStream(eq(queryData), any()))
                .thenReturn(parseXmlStream(queryData));

        mSscTransaction.startGetTransaction(queryData);
        sleepToWaitThreadRun();

        triggerCallbackMessage(SscNetConnection.EVENT_PDN_CONNECTED);

        verify(mMockSscHttpConnectionGov).sendRequest(SLOT_0, SscHttpConnection.HTTP_GET_REQUEST,
                mDefaultRequestUri, mDefaultXui, "");
        verify(mMockSscXmlGov).parseXmlStream(eq(queryData), any());

        processTransactionSuccess(SscConstant.EVENT_SSC_QUERY_CF,
                SscHttpConnection.HTTP_GET_REQUEST);
    }

    @Test
    public void startPutTransaction_creatingXmlBodyError() {
        SscServiceData updateData = getUpdateData(SscConstant.CONDITION_CFU,
                SscConstant.ACTION_ACTIVATION, "+1234567890", 0);

        when(mMockSscXmlGov.createXmlStream(eq(updateData))).thenReturn(null);

        mSscTransaction.startPutTransaction(updateData);
        sleepToWaitThreadRun();

        verify(mMockSscXmlGov).createXmlStream(eq(updateData));

        processTransactionFailure(SscConstant.EVENT_SSC_UPDATE_CF, false);
    }

    @Test
    public void startPutTransaction_httpErrorResponseWithErrorPhrase() {
        int httpErrorResponse = SscConstant.HTTP_CONFLICT;
        String xmlBody = createXmlStream();
        SscServiceData updateData = getUpdateData(SscConstant.CONDITION_CFU,
                SscConstant.ACTION_ACTIVATION, "+1234567890", 0);

        when(mMockSscXmlGov.createXmlStream(eq(updateData))).thenReturn(xmlBody);
        when(mMockSscHttpConnectionGov.sendRequest(SLOT_0, SscHttpConnection.HTTP_PUT_REQUEST,
                mDefaultRequestUri, mDefaultXui, xmlBody)).thenReturn(httpErrorResponse);
        when(mMockSscHttpConnectionGov.getInputStream(eq(SLOT_0))).thenReturn(getInputStream());
        when(mMockSscXmlGov.parseXmlStream(eq(updateData), any()))
                .thenReturn(parseXmlStream(updateData));

        mSscTransaction.startPutTransaction(updateData);
        sleepToWaitThreadRun();

        verify(mMockSscHttpConnectionGov).sendRequest(SLOT_0, SscHttpConnection.HTTP_PUT_REQUEST,
                mDefaultRequestUri, mDefaultXui, xmlBody);
        verify(mMockSscServiceStateAgent).setErrorResponseCode(eq(SLOT_0), eq(httpErrorResponse));
        verify(mMockSscHttpConnectionGov).getInputStream(eq(SLOT_0));
        verify(mMockSscXmlGov).parseXmlStream(eq(updateData), any());

        processTransactionFailure(SscConstant.EVENT_SSC_UPDATE_CF, true);
    }

    @Test
    public void startPutTransaction_success() {
        int httpSuccessResponse = SscConstant.HTTP_OK;
        String xmlBody = createXmlStream();
        SscServiceData updateData = getUpdateData(SscConstant.CONDITION_CFU,
                SscConstant.ACTION_ACTIVATION, "+1234567890", 0);

        when(mMockSscXmlGov.createXmlStream(eq(updateData))).thenReturn(xmlBody);
        when(mMockSscHttpConnectionGov.sendRequest(SLOT_0, SscHttpConnection.HTTP_PUT_REQUEST,
                mDefaultRequestUri, mDefaultXui, xmlBody)).thenReturn(httpSuccessResponse);
        when(mMockSscHttpConnectionGov.getInputStream(eq(SLOT_0))).thenReturn(null);

        mSscTransaction.startPutTransaction(updateData);
        sleepToWaitThreadRun();

        verify(mMockSscHttpConnectionGov).sendRequest(SLOT_0, SscHttpConnection.HTTP_PUT_REQUEST,
                mDefaultRequestUri, mDefaultXui, xmlBody);

        processTransactionSuccess(SscConstant.EVENT_SSC_UPDATE_CF,
                SscHttpConnection.HTTP_PUT_REQUEST);
    }

    @Test
    public void startPutTransaction_successForInsert() {
        int httpSuccessResponse = SscConstant.HTTP_OK;
        String xmlBody = createXmlStream();
        SscServiceData insertData = getInsertData(ESsType.CF);

        when(mMockSscXmlGov.createXmlStream(eq(insertData))).thenReturn(xmlBody);
        when(mMockSscHttpConnectionGov.sendRequest(SLOT_0, SscHttpConnection.HTTP_PUT_REQUEST,
                mDefaultRequestUri, mDefaultXui, xmlBody)).thenReturn(httpSuccessResponse);
        when(mMockSscHttpConnectionGov.getInputStream(eq(SLOT_0))).thenReturn(null);

        mSscTransaction.startPutTransaction(insertData);
        sleepToWaitThreadRun();

        verify(mMockSscHttpConnectionGov).sendRequest(SLOT_0, SscHttpConnection.HTTP_PUT_REQUEST,
                mDefaultRequestUri, mDefaultXui, xmlBody);
        verify(mMockSscXmlGov).updateTagsAndRules();

        processTransactionSuccess(SscConstant.EVENT_SSC_INSERT_CF,
                SscHttpConnection.HTTP_PUT_REQUEST);
    }

    @Test
    public void startPutTransaction_successForCfnrTimer() {
        int httpSuccessResponse = SscConstant.HTTP_OK;
        String xmlBody = createXmlStream();
        SscServiceData updateData = getUpdateData(SscConstant.CONDITION_CFNR_TIMER,
                SscConstant.ACTION_ACTIVATION, null, 25);
        SscXmlFormat.init(SLOT_0);
        SscXmlFormat.setIsNoReplyTimerOmitted(SLOT_0, true);

        when(mMockSscXmlGov.createXmlStream(eq(updateData))).thenReturn(xmlBody);
        when(mMockSscHttpConnectionGov.sendRequest(SLOT_0, SscHttpConnection.HTTP_PUT_REQUEST,
                mDefaultRequestUri, mDefaultXui, xmlBody)).thenReturn(httpSuccessResponse);
        when(mMockSscHttpConnectionGov.getInputStream(eq(SLOT_0))).thenReturn(null);

        mSscTransaction.startPutTransaction(updateData);
        sleepToWaitThreadRun();

        verify(mMockSscHttpConnectionGov).sendRequest(SLOT_0, SscHttpConnection.HTTP_PUT_REQUEST,
                mDefaultRequestUri, mDefaultXui, xmlBody);
        assertEquals(false, SscXmlFormat.getIsNoReplyTimerOmitted(SLOT_0));

        processTransactionSuccess(SscConstant.EVENT_SSC_UPDATE_CF,
                SscHttpConnection.HTTP_PUT_REQUEST);
    }

    @Test
    public void startPutTransaction_successWithCbPassword() {
        int httpSuccessResponse = SscConstant.HTTP_OK;
        String cbPassword = "1234";
        String sipXui = "sip:+1234567890@test.3gpp.com";
        String requestUri = "/simservs.ngn.etsi.org/users/" + sipXui + "/simservs.xml";
        String xmlBody = createXmlStream();
        SscServiceData updateData = new CbServiceUpdateData(SLOT_0, ESsType.OCB,
                SscConstant.EVENT_SSC_UPDATE_CB, mTransactionId,
                SscConstant.ACTION_ACTIVATION, SscConstant.CONDITION_BAIC, null,
                SscServiceClassUtil.SERVICE_CLASS_CALL, cbPassword);

        when(mMockSscXui.getXui(eq(SLOT_0), eq(cbPassword))).thenReturn(sipXui);
        when(mMockSscUrl.getUpdateUri(any(), eq(sipXui))).thenReturn(requestUri);
        when(mMockSscXmlGov.createXmlStream(eq(updateData))).thenReturn(xmlBody);
        when(mMockSscHttpConnectionGov.sendRequest(SLOT_0, SscHttpConnection.HTTP_PUT_REQUEST,
                requestUri, sipXui, xmlBody)).thenReturn(httpSuccessResponse);
        when(mMockSscHttpConnectionGov.getInputStream(eq(SLOT_0))).thenReturn(null);

        mSscTransaction.startPutTransaction(updateData);
        sleepToWaitThreadRun();

        verify(mMockSscHttpConnectionGov).sendRequest(SLOT_0, SscHttpConnection.HTTP_PUT_REQUEST,
                requestUri, sipXui, xmlBody);

        processTransactionSuccess(SscConstant.EVENT_SSC_UPDATE_CB,
                SscHttpConnection.HTTP_PUT_REQUEST);
    }

    private void processTransactionSuccess(int requestedEventNum, int requestType) {
        verify(mMockCallbackHandler).sendMessageAtTime(mMessageCaptor.capture(), anyLong());

        Message msg = mMessageCaptor.getValue();
        assertNotNull(msg);
        assertEquals(requestedEventNum, msg.what);

        SscRequestResult rr = (SscRequestResult) msg.obj;
        assertNotNull(rr);
        assertEquals(SLOT_0, rr.getSlotId());
        assertEquals(mTransactionId, rr.getTransactionId());
        assertEquals(SscConstant.REQUEST_SUCCESS, rr.getResultState());
        if (requestType == SscHttpConnection.HTTP_GET_REQUEST) {
            SscServiceData errorPhraseData = rr.getSscServiceData();
            assertNotNull(errorPhraseData);
        } else if (requestType == SscHttpConnection.HTTP_PUT_REQUEST) {
            assertNull(rr.getSscServiceData());
        }
        verifyNoMoreInteractions(mMockCallbackHandler);
        verifyNoMoreInteractions(mMockSscServiceStateAgent);
    }

    private void processTransactionFailure(int requestedEventNum, boolean isErrorPhraseExist) {
        verify(mMockCallbackHandler).sendMessageAtTime(mMessageCaptor.capture(), anyLong());

        Message msg = mMessageCaptor.getValue();
        assertNotNull(msg);
        assertEquals(requestedEventNum, msg.what);

        SscRequestResult rr = (SscRequestResult) msg.obj;
        assertNotNull(rr);
        assertEquals(SLOT_0, rr.getSlotId());
        assertEquals(mTransactionId, rr.getTransactionId());
        assertEquals(SscConstant.REQUEST_FAILURE, rr.getResultState());
        if (isErrorPhraseExist) {
            SscServiceData errorPhraseData = rr.getSscServiceData();
            assertNotNull(errorPhraseData);
        }
        verifyNoMoreInteractions(mMockCallbackHandler);
    }

    /**
     * Triggering handleMessage() in SscTransaction with a given event to handle callback event
     * This method must only be used after SscTransactionThread.start().
     */
    private void triggerCallbackMessage(int event) {
        mSscTransaction.getTransactionHandler().handleMessage(
                Message.obtain(mSscTransaction.getTransactionHandler(), event));
    }

    private SscServiceQueryData getQueryData(int condition) {
        return SscXmlGovTest.createQueryData(ESsType.CF, mTransactionId, condition);
    }

    private SscServiceData getUpdateData(int condition, int action, String number,
            int timeSeconds) {
        return new CfServiceUpdateData(SLOT_0, ESsType.CF,
                SscConstant.EVENT_SSC_UPDATE_CF, mTransactionId, action, condition,
                number, timeSeconds, SscServiceClassUtil.SERVICE_CLASS_CALL);
    }

    private SscServiceData getInsertData(ESsType ssType) {
        return SscXmlGovTest.createInsertData(ssType, mTransactionId, SscConstant.ACTION_ACTIVATION,
                SscConstant.CONDITION_CFU, null);
    }

    private void sleepToWaitThreadRun() {
        android.os.SystemClock.sleep(50);
    }

    private SscServiceData parseXmlStream(SscData data) {
        return new SscServiceData(data.getSlotId(), data.getSsType(), data.getEventNumber(),
                data.getTransactionId(), 0);
    }

    private String createXmlStream() {
        return "<ss:simservs />";
    }

    private Document getInputStream() {
        String xml = "<ss:simservs />";
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

    private class FakeSscTransaction extends SscTransaction {
        private FakeSscTransaction(int slotId, Handler handler) {
            super(slotId, handler);
        }

        @Override
        protected SscXmlGov getSscXmlGov() {
            if (super.getSscXmlGov() == null) {
                fail();
            }
            return mMockSscXmlGov;
        }

        @Override
        protected SscXui getSscXui() {
            if (super.getSscXui() == null) {
                fail();
            }
            return mMockSscXui;
        }

        @Override
        protected SscUrl getSscUrl() {
            if (super.getSscUrl() == null) {
                fail();
            }
            return mMockSscUrl;
        }

        @Override
        protected SscServiceStateAgent getSscServiceStateAgent() {
            if (super.getSscServiceStateAgent() == null) {
                fail();
            }
            return mMockSscServiceStateAgent;
        }

        @Override
        protected SscUtils getSscUtils() {
            if (super.getSscUtils() == null) {
                fail();
            }
            return mMockSscUtils;
        }

        @Override
        protected ISscHttpConnectionGov getSscHttpConnectionGov() {
            if (super.getSscHttpConnectionGov() == null) {
                fail();
            }
            return mMockSscHttpConnectionGov;
        }

        @Override
        protected ISscAuthAgent getSscAuthAgent() {
            if (super.getSscAuthAgent() == null) {
                fail();
            }
            return mMockSscAuthAgent;
        }

        @Override
        protected GbaInterface getGbaAgent() {
            return mMockGbaAgent;
        }
    }
}
