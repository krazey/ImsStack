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
import static org.mockito.ArgumentMatchers.anyInt;
import static org.mockito.ArgumentMatchers.anyLong;
import static org.mockito.ArgumentMatchers.anyString;
import static org.mockito.ArgumentMatchers.eq;
import static org.mockito.Mockito.atLeast;
import static org.mockito.Mockito.doAnswer;
import static org.mockito.Mockito.doNothing;
import static org.mockito.Mockito.never;
import static org.mockito.Mockito.verify;
import static org.mockito.Mockito.verifyNoMoreInteractions;
import static org.mockito.Mockito.when;

import android.os.Handler;
import android.os.Message;
import android.telephony.CarrierConfigManager;
import android.telephony.TelephonyManager;

import com.android.imsstack.core.agents.AgentFactory;
import com.android.imsstack.core.agents.ConfigInterface;
import com.android.imsstack.core.agents.GbaInterface;
import com.android.imsstack.core.agents.GbaInterface.GbaCredentials;
import com.android.imsstack.core.agents.ImsRadioInterface;
import com.android.imsstack.core.agents.ImsRadioInterface.ConnectionListener;
import com.android.imsstack.core.config.CarrierConfig;
import com.android.imsstack.enabler.ssc.data.CbServiceUpdateData;
import com.android.imsstack.enabler.ssc.data.CfServiceUpdateData;
import com.android.imsstack.enabler.ssc.data.SscData;
import com.android.imsstack.enabler.ssc.data.SscRequestResult;
import com.android.imsstack.enabler.ssc.data.SscServiceData;
import com.android.imsstack.enabler.ssc.data.SscServiceQueryData;

import org.junit.After;
import org.junit.Before;
import org.junit.Rule;
import org.junit.Test;
import org.junit.rules.Timeout;
import org.junit.runner.RunWith;
import org.junit.runners.JUnit4;
import org.mockito.ArgumentCaptor;
import org.mockito.Captor;
import org.mockito.Mock;
import org.mockito.MockitoAnnotations;
import org.mockito.stubbing.Answer;
import org.w3c.dom.Document;
import org.xml.sax.InputSource;

import java.io.StringReader;
import java.util.concurrent.TimeUnit;

import javax.xml.parsers.DocumentBuilder;
import javax.xml.parsers.DocumentBuilderFactory;

@RunWith(JUnit4.class)
public class SscTransactionTest {
    @Rule public Timeout timeout = new Timeout(5, TimeUnit.SECONDS);

    private static final int SLOT_0 = 0;

    private FakeSscTransaction mSscTransaction;
    private int mGbaMode = SscConfig.GBA_ME;
    private int mTransactionId = 1;
    private String mDefaultXui = "tel:+1234567890";
    private String mDefaultRequestUri = "/simservs.ngn.etsi.org/users/" + mDefaultXui
            + "/simservs.xml";

    @Captor ArgumentCaptor<Message> mMessageCaptor;
    @Captor ArgumentCaptor<ImsRadioInterface.ConnectionListener> mConnectionListenerCaptor;

    @Mock private CarrierConfig mMockCarrierConfig;
    @Mock private ConfigInterface mMockConfigInterface;
    @Mock private GbaInterface mMockGbaAgent;
    @Mock private ImsRadioInterface mMockImsRadioInterface;
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

        when(mMockSscConnection.isConnected()).thenReturn(true);
        when(mMockSscAuthAgent.isCredentialInfoUpdated()).thenReturn(false);
        when(mMockSscXui.getXui(eq(SLOT_0), eq(null))).thenReturn(mDefaultXui);
        when(mMockSscUrl.getQueryUri(any(), eq(mDefaultXui))).thenReturn(mDefaultRequestUri);
        when(mMockSscUrl.getUpdateUri(any(), eq(mDefaultXui))).thenReturn(mDefaultRequestUri);
        when(mMockConfigInterface.getCarrierConfig()).thenReturn(mMockCarrierConfig);
        when(mMockCarrierConfig.getInt(eq(CarrierConfigManager.KEY_GBA_MODE_INT)))
                .thenReturn(mGbaMode);
        when(mMockSscConnection.getNetworkType()).thenReturn(TelephonyManager.NETWORK_TYPE_LTE);
        when(mMockImsRadioInterface.isImsTrafficAllowed(ImsRadioInterface.TRAFFIC_TYPE_UT_XCAP))
                .thenReturn(true);
        doAnswer((Answer<Void>) (invocation) -> {
            ConnectionListener connectionListener = mConnectionListenerCaptor.getValue();
            if (connectionListener != null) {
                connectionListener.onConnectionSetupPrepared();
            }
            return null;
        }).when(mMockImsRadioInterface).startImsTraffic(anyInt(), anyInt(), anyInt(),
                mConnectionListenerCaptor.capture());

        AgentFactory.getInstance()
                .setAgent(ImsRadioInterface.class, mMockImsRadioInterface, SLOT_0);

        SscConfig.setConfigInterface(SLOT_0, mMockConfigInterface);
        ((SscNetConnectionGov) SscNetConnectionGov.getInstance()).setSscNetConnection(SLOT_0,
                mMockSscConnection);

        mSscTransaction = new FakeSscTransaction(SLOT_0, mMockCallbackHandler);
    }

    @After
    public void tearDown() {
        AgentFactory.getInstance().setAgent(ImsRadioInterface.class, null, SLOT_0);
        mSscTransaction.close();
        SscConfig.clear(SLOT_0);
    }

    @Test
    public void startTransaction_requestToConnectFailure() {
        when(mMockSscConnection.isConnected()).thenReturn(false);
        when(mMockSscConnection.connect(anyLong())).thenReturn(false);

        mSscTransaction.startGetTransaction(getQueryData(SscConstant.CONDITION_CFU));
        sleepToWaitThreadRun();

        verifyTransactionFailure(SscConstant.EVENT_SSC_QUERY_CF, false, false);
    }

    @Test
    public void startTransaction_pdnDisconnected() {
        when(mMockSscConnection.isConnected()).thenReturn(false);
        when(mMockSscConnection.connect(anyLong())).thenReturn(true);

        mSscTransaction.startGetTransaction(getQueryData(SscConstant.CONDITION_CFU));
        sleepToWaitThreadRun();
        triggerCallbackMessage(SscNetConnection.EVENT_PDN_DISCONNECTED);

        verifyTransactionFailure(SscConstant.EVENT_SSC_QUERY_CF, false, false);
    }

    @Test
    public void startTransaction_pdnRequestTimeout() {
        when(mMockSscConnection.isConnected()).thenReturn(false);
        when(mMockSscConnection.connect(anyLong())).thenReturn(true);

        mSscTransaction.startGetTransaction(getQueryData(SscConstant.CONDITION_CFU));
        sleepToWaitThreadRun();
        triggerCallbackMessage(SscNetConnection.EVENT_PDN_REQUEST_TIMEOUT);

        verifyTransactionFailure(SscConstant.EVENT_SSC_QUERY_CF, false, false);
    }

    @Test
    public void startTransaction_pdnConnectionFailure() {
        when(mMockSscConnection.isConnected()).thenReturn(false);
        when(mMockSscConnection.connect(anyLong())).thenReturn(true);

        mSscTransaction.startGetTransaction(getQueryData(SscConstant.CONDITION_CFU));
        sleepToWaitThreadRun();
        triggerCallbackMessage(SscNetConnection.EVENT_PDN_CONNECTION_FAILED);

        verifyTransactionFailure(SscConstant.EVENT_SSC_QUERY_CF, false, false);
    }

    @Test
    public void startTransaction_imsRadioNull() {
        AgentFactory.getInstance().setAgent(ImsRadioInterface.class, null, SLOT_0);
        mSscTransaction = new FakeSscTransaction(SLOT_0, mMockCallbackHandler);

        mSscTransaction.startGetTransaction(getQueryData(SscConstant.CONDITION_CFU));
        sleepToWaitThreadRun();

        verifyTransactionFailure(SscConstant.EVENT_SSC_QUERY_CF, false, false);
    }

    @Test
    public void startTransaction_xcapTrafficNotAllowed() {
        when(mMockImsRadioInterface.isImsTrafficAllowed(ImsRadioInterface.TRAFFIC_TYPE_UT_XCAP))
                .thenReturn(false);

        mSscTransaction.startGetTransaction(getQueryData(SscConstant.CONDITION_CFU));
        sleepToWaitThreadRun();

        verifyTransactionFailure(SscConstant.EVENT_SSC_QUERY_CF, false, false);
    }

    @Test
    public void startTransaction_xcapTrafficNotAllowedButConnectedOverIwlan() {
        when(mMockSscConnection.getNetworkType()).thenReturn(TelephonyManager.NETWORK_TYPE_IWLAN);
        when(mMockImsRadioInterface.isImsTrafficAllowed(ImsRadioInterface.TRAFFIC_TYPE_UT_XCAP))
                .thenReturn(false);

        mSscTransaction.startGetTransaction(getQueryData(SscConstant.CONDITION_CFU));
        sleepToWaitThreadRun();

        verify(mMockImsRadioInterface).startImsTraffic(eq(ImsRadioInterface.TRAFFIC_TYPE_UT_XCAP),
                eq(ImsRadioInterface.ACCESS_NETWORK_TYPE_IWLAN),
                eq(ImsRadioInterface.DIRECTION_MO), any());
    }

    @Test
    public void startTransaction_xcapTrafficConnectionFailed() {
        doAnswer((Answer<Void>) (invocation) -> {
            ConnectionListener connectionListener = mConnectionListenerCaptor.getValue();
            if (connectionListener != null) {
                connectionListener.onConnectionFailed(0, 0, 0);
            }
            return null;
        }).when(mMockImsRadioInterface).startImsTraffic(anyInt(), anyInt(), anyInt(),
                mConnectionListenerCaptor.capture());

        mSscTransaction.startGetTransaction(getQueryData(SscConstant.CONDITION_CFU));
        sleepToWaitThreadRun();

        verifyTransactionFailure(SscConstant.EVENT_SSC_QUERY_CF, false, false);
    }

    @Test
    public void startTransaction_onConnectionSetupPreparedTwice() {
        doAnswer((Answer<Void>) (invocation) -> {
            ConnectionListener connectionListener = mConnectionListenerCaptor.getValue();
            if (connectionListener != null) {
                connectionListener.onConnectionSetupPrepared(); // 1st
                connectionListener.onConnectionSetupPrepared(); // 2nd
            }
            return null;
        }).when(mMockImsRadioInterface).startImsTraffic(anyInt(), anyInt(), anyInt(),
                mConnectionListenerCaptor.capture());

        mSscTransaction.startGetTransaction(getQueryData(SscConstant.CONDITION_CFU));
        sleepToWaitThreadRun();

        // sendRequest must be called only once
        verify(mMockSscHttpConnectionGov)
                .sendRequest(anyInt(), anyInt(), anyString(), anyString(), anyString());
    }

    @Test
    public void startTransaction_pdnIpcanChanged() {
        when(mMockSscConnection.getNetworkType())
                .thenReturn(TelephonyManager.NETWORK_TYPE_LTE, TelephonyManager.NETWORK_TYPE_LTE,
                        TelephonyManager.NETWORK_TYPE_IWLAN);
        doNothing().when(mMockImsRadioInterface)
                .startImsTraffic(anyInt(), anyInt(), anyInt(), any());

        mSscTransaction.startGetTransaction(getQueryData(SscConstant.CONDITION_CFU));
        sleepToWaitThreadRun();

        verify(mMockImsRadioInterface).startImsTraffic(eq(ImsRadioInterface.TRAFFIC_TYPE_UT_XCAP),
                eq(ImsRadioInterface.ACCESS_NETWORK_TYPE_EUTRAN),
                eq(ImsRadioInterface.DIRECTION_MO), any());

        triggerCallbackMessage(SscNetConnection.EVENT_PDN_IPCAN_CHANGED);

        verify(mMockImsRadioInterface).startImsTraffic(eq(ImsRadioInterface.TRAFFIC_TYPE_UT_XCAP),
                eq(ImsRadioInterface.ACCESS_NETWORK_TYPE_IWLAN),
                eq(ImsRadioInterface.DIRECTION_MO), any());
    }

    @Test
    public void sendRequest_invalidXui() {
        when(mMockSscXui.getXui(eq(SLOT_0), eq(null))).thenReturn(null);

        mSscTransaction.startGetTransaction(getQueryData(SscConstant.CONDITION_CFU));
        sleepToWaitThreadRun();
        waitThreadWorkFinished();

        verify(mMockSscXui).getXui(eq(SLOT_0), eq(null));

        verifyTransactionFailure(SscConstant.EVENT_SSC_QUERY_CF, true, false);
    }

    @Test
    public void sendRequest_invalidRequestUri() {
        when(mMockSscUrl.getQueryUri(any(), anyString())).thenReturn(null);

        mSscTransaction.startGetTransaction(getQueryData(SscConstant.CONDITION_CFU));
        sleepToWaitThreadRun();
        waitThreadWorkFinished();

        verify(mMockSscUrl).getQueryUri(any(), anyString());

        verifyTransactionFailure(SscConstant.EVENT_SSC_QUERY_CF, true, false);
    }

    @Test
    public void sendRequest_gbaFailure() {
        String nafFqdn = "xcap.3gpp.com";
        String securityProtocol = "TLS_NULL_WITH_NULL_NULL";
        int appType = SscConstant.APPTYPE_ISIM;

        when(mMockSscAuthAgent.isCredentialInfoUpdated()).thenReturn(false, true);
        when(mMockSscHttpConnectionGov.sendRequest(eq(SLOT_0),
                eq(ISscHttpConnection.HTTP_REQUEST_GET), eq(mDefaultRequestUri), eq(mDefaultXui),
                eq(""))).thenReturn(SscConstant.HTTP_UNAUTHORIZED);
        when(mMockSscUtils.getTelephonySimType(eq(SLOT_0))).thenReturn(appType);
        when(mMockSscAuthAgent.getNafFqdn()).thenReturn(nafFqdn);
        when(mMockSscAuthAgent.getCipherSuite()).thenReturn(securityProtocol);
        when(mMockGbaAgent.getGbaKey(eq(appType), eq(mGbaMode), anyBoolean(), eq(nafFqdn),
                eq(securityProtocol), eq(true), anyInt()))
                .thenReturn(new GbaCredentials(GbaInterface.GBA_FAILURE_REASON_KEY_INVALID));

        mSscTransaction.startGetTransaction(getQueryData(SscConstant.CONDITION_CFU));
        sleepToWaitThreadRun();
        waitThreadWorkFinished();

        verify(mMockSscAuthAgent, atLeast(1)).isCredentialInfoUpdated();
        verify(mMockSscHttpConnectionGov).sendRequest(eq(SLOT_0),
                eq(ISscHttpConnection.HTTP_REQUEST_GET), eq(mDefaultRequestUri), eq(mDefaultXui),
                eq(""));
        verify(mMockSscAuthAgent, atLeast(2)).isCredentialInfoUpdated();
        verify(mMockSscAuthAgent).getNafFqdn();
        verify(mMockSscAuthAgent).getCipherSuite();
        verify(mMockGbaAgent).getGbaKey(eq(appType), eq(mGbaMode), anyBoolean(), eq(nafFqdn),
                eq(securityProtocol), eq(true), anyInt());
        verify(mMockSscAuthAgent).setIsCredentialInfoUpdated(eq(false));
        verify(mMockSscServiceStateAgent).setGbaRequestFailed(eq(SLOT_0), eq(true));

        verifyTransactionFailure(SscConstant.EVENT_SSC_QUERY_CF, true, false);
    }

    @Test
    public void sendRequest_unauthorizedTwiceWithGbaKey() {
        String nafFqdn = "xcap.3gpp.com";
        String securityProtocol = "TLS_NULL_WITH_NULL_NULL";
        int appType = SscConstant.APPTYPE_USIM;

        when(mMockSscAuthAgent.isCredentialInfoUpdated()).thenReturn(false, true);
        when(mMockSscHttpConnectionGov.sendRequest(SLOT_0, ISscHttpConnection.HTTP_REQUEST_GET,
                mDefaultRequestUri, mDefaultXui, "")).thenReturn(SscConstant.HTTP_UNAUTHORIZED);
        when(mMockSscUtils.getTelephonySimType(eq(SLOT_0))).thenReturn(appType);
        when(mMockSscAuthAgent.getNafFqdn()).thenReturn(nafFqdn);
        when(mMockSscAuthAgent.getCipherSuite()).thenReturn(securityProtocol);
        when(mMockGbaAgent.getGbaKey(eq(appType), eq(mGbaMode), anyBoolean(), eq(nafFqdn),
                eq(securityProtocol), eq(true), anyInt()))
                .thenReturn(new GbaCredentials("B-TID", "Ks_NAF_KEY"));

        mSscTransaction.startGetTransaction(getQueryData(SscConstant.CONDITION_CFU));
        sleepToWaitThreadRun();
        waitThreadWorkFinished();

        verify(mMockSscAuthAgent, atLeast(1)).isCredentialInfoUpdated();
        verify(mMockSscHttpConnectionGov, atLeast(1)).sendRequest(SLOT_0,
                ISscHttpConnection.HTTP_REQUEST_GET, mDefaultRequestUri, mDefaultXui, "");
        verify(mMockSscAuthAgent, atLeast(2)).isCredentialInfoUpdated();
        verify(mMockGbaAgent).getGbaKey(eq(appType), eq(mGbaMode), anyBoolean(), eq(nafFqdn),
                eq(securityProtocol), eq(true), anyInt());
        verify(mMockSscHttpConnectionGov, atLeast(2)).sendRequest(SLOT_0,
                ISscHttpConnection.HTTP_REQUEST_GET, mDefaultRequestUri, mDefaultXui, "");
        verify(mMockSscAuthAgent).setIsCredentialInfoUpdated(eq(false));

        verifyTransactionFailure(SscConstant.EVENT_SSC_QUERY_CF, true, false);
    }

    @Test
    public void sendRequest_requestFailure() {
        when(mMockSscHttpConnectionGov.sendRequest(SLOT_0, ISscHttpConnection.HTTP_REQUEST_GET,
                mDefaultRequestUri, mDefaultXui, ""))
                .thenReturn(ISscHttpConnection.HTTP_REQUEST_FAILED_UNSPECIFIED);

        mSscTransaction.startGetTransaction(getQueryData(SscConstant.CONDITION_CFU));
        sleepToWaitThreadRun();
        waitThreadWorkFinished();

        verify(mMockSscHttpConnectionGov).sendRequest(SLOT_0, ISscHttpConnection.HTTP_REQUEST_GET,
                mDefaultRequestUri, mDefaultXui, "");
        verify(mMockSscServiceStateAgent, never()).setDnsQueryFailed(anyInt(), anyBoolean());
        verify(mMockSscServiceStateAgent, never())
                .setSocketConnectionExpired(anyInt(), anyBoolean());

        verifyTransactionFailure(SscConstant.EVENT_SSC_QUERY_CF, true, false);
    }

    @Test
    public void sendRequest_requestFailureByDns() {
        when(mMockSscHttpConnectionGov.sendRequest(SLOT_0, ISscHttpConnection.HTTP_REQUEST_GET,
                mDefaultRequestUri, mDefaultXui, ""))
                .thenReturn(ISscHttpConnection.HTTP_REQUEST_FAILED_BY_DNS);

        mSscTransaction.startGetTransaction(getQueryData(SscConstant.CONDITION_CFU));
        sleepToWaitThreadRun();
        waitThreadWorkFinished();

        verify(mMockSscHttpConnectionGov).sendRequest(SLOT_0, ISscHttpConnection.HTTP_REQUEST_GET,
                mDefaultRequestUri, mDefaultXui, "");
        verify(mMockSscServiceStateAgent).setDnsQueryFailed(eq(SLOT_0), eq(true));

        verifyTransactionFailure(SscConstant.EVENT_SSC_QUERY_CF, true, false);
    }

    @Test
    public void sendRequest_requestFailureBySocketTimeout() {
        when(mMockSscHttpConnectionGov.sendRequest(SLOT_0, ISscHttpConnection.HTTP_REQUEST_GET,
                mDefaultRequestUri, mDefaultXui, ""))
                .thenReturn(ISscHttpConnection.HTTP_REQUEST_FAILED_BY_TIMEOUT);

        mSscTransaction.startGetTransaction(getQueryData(SscConstant.CONDITION_CFU));
        sleepToWaitThreadRun();
        waitThreadWorkFinished();

        verify(mMockSscHttpConnectionGov).sendRequest(SLOT_0, ISscHttpConnection.HTTP_REQUEST_GET,
                mDefaultRequestUri, mDefaultXui, "");
        verify(mMockSscServiceStateAgent).setSocketConnectionExpired(eq(SLOT_0), eq(true));

        verifyTransactionFailure(SscConstant.EVENT_SSC_QUERY_CF, true, false);
    }

    @Test
    public void startGetTransaction_httpErrorResponseWithErrorPhrase() {
        int httpErrorResponse = SscConstant.HTTP_CONFLICT;
        SscServiceQueryData queryData = getQueryData(SscConstant.CONDITION_CFU);

        when(mMockSscHttpConnectionGov.sendRequest(SLOT_0, ISscHttpConnection.HTTP_REQUEST_GET,
                mDefaultRequestUri, mDefaultXui, "")).thenReturn(httpErrorResponse);
        when(mMockSscHttpConnectionGov.getInputStream(eq(SLOT_0))).thenReturn(getInputStream());
        when(mMockSscXmlGov.parseXmlStream(eq(queryData), any()))
                .thenReturn(parseXmlStream(queryData));

        mSscTransaction.startGetTransaction(queryData);
        sleepToWaitThreadRun();
        waitThreadWorkFinished();

        verify(mMockSscHttpConnectionGov).sendRequest(SLOT_0, ISscHttpConnection.HTTP_REQUEST_GET,
                mDefaultRequestUri, mDefaultXui, "");
        verify(mMockSscServiceStateAgent).setErrorResponseCode(eq(SLOT_0), eq(httpErrorResponse));
        verify(mMockSscHttpConnectionGov).getInputStream(eq(SLOT_0));
        verify(mMockSscXmlGov).parseXmlStream(eq(queryData), any());

        verifyTransactionFailure(SscConstant.EVENT_SSC_QUERY_CF, true, true);
    }

    @Test
    public void startGetTransaction_httpErrorResponseWithoutErrorPhrase() {
        int httpErrorResponse = SscConstant.HTTP_CONFLICT;
        when(mMockSscHttpConnectionGov.sendRequest(SLOT_0, ISscHttpConnection.HTTP_REQUEST_GET,
                mDefaultRequestUri, mDefaultXui, "")).thenReturn(httpErrorResponse);
        when(mMockSscHttpConnectionGov.getInputStream(eq(SLOT_0))).thenReturn(null);

        mSscTransaction.startGetTransaction(getQueryData(SscConstant.CONDITION_CFU));
        sleepToWaitThreadRun();
        waitThreadWorkFinished();

        verify(mMockSscHttpConnectionGov).sendRequest(SLOT_0, ISscHttpConnection.HTTP_REQUEST_GET,
                mDefaultRequestUri, mDefaultXui, "");
        verify(mMockSscServiceStateAgent).setErrorResponseCode(eq(SLOT_0), eq(httpErrorResponse));
        verify(mMockSscHttpConnectionGov).getInputStream(eq(SLOT_0));

        verifyTransactionFailure(SscConstant.EVENT_SSC_QUERY_CF, true, false);
    }

    @Test
    public void startGetTransaction_httpErrorResponseForbidden() {
        int httpErrorResponse = SscConstant.HTTP_FORBIDDEN;
        when(mMockSscHttpConnectionGov.sendRequest(SLOT_0, ISscHttpConnection.HTTP_REQUEST_GET,
                mDefaultRequestUri, mDefaultXui, "")).thenReturn(httpErrorResponse);

        mSscTransaction.startGetTransaction(getQueryData(SscConstant.CONDITION_CFU));
        sleepToWaitThreadRun();
        waitThreadWorkFinished();

        verify(mMockSscHttpConnectionGov).sendRequest(SLOT_0, ISscHttpConnection.HTTP_REQUEST_GET,
                mDefaultRequestUri, mDefaultXui, "");
        verify(mMockSscAuthAgent).setIsCredentialInfoUpdated(eq(false));
        verify(mMockSscServiceStateAgent).setErrorResponseCode(eq(SLOT_0), eq(httpErrorResponse));

        verifyTransactionFailure(SscConstant.EVENT_SSC_QUERY_CF, true, false);
    }

    @Test
    public void startGetTransaction_successButParsingError() {
        int httpSuccessResponse = SscConstant.HTTP_OK;
        SscServiceQueryData queryData = getQueryData(SscConstant.CONDITION_CFU);

        when(mMockSscHttpConnectionGov.sendRequest(SLOT_0, ISscHttpConnection.HTTP_REQUEST_GET,
                mDefaultRequestUri, mDefaultXui, "")).thenReturn(httpSuccessResponse);
        when(mMockSscHttpConnectionGov.getInputStream(eq(SLOT_0))).thenReturn(getInputStream());
        when(mMockSscXmlGov.parseXmlStream(eq(queryData), any())).thenReturn(null);

        mSscTransaction.startGetTransaction(queryData);
        sleepToWaitThreadRun();
        waitThreadWorkFinished();

        verifyTransactionFailure(SscConstant.EVENT_SSC_QUERY_CF, true, false);
    }

    @Test
    public void startGetTransaction_success() {
        int httpSuccessResponse = SscConstant.HTTP_OK;
        SscServiceQueryData queryData = getQueryData(SscConstant.CONDITION_CFU);

        when(mMockSscConnection.isConnected()).thenReturn(false, true);
        when(mMockSscHttpConnectionGov.sendRequest(SLOT_0, ISscHttpConnection.HTTP_REQUEST_GET,
                mDefaultRequestUri, mDefaultXui, "")).thenReturn(httpSuccessResponse);
        when(mMockSscHttpConnectionGov.getInputStream(eq(SLOT_0))).thenReturn(getInputStream());
        when(mMockSscXmlGov.parseXmlStream(eq(queryData), any()))
                .thenReturn(parseXmlStream(queryData));
        doAnswer((Answer<Boolean>) (invocation) -> {
            mSscTransaction.getTransactionHandler()
                    .sendEmptyMessage(SscNetConnection.EVENT_PDN_CONNECTED);
            return true;
        }).when(mMockSscConnection).connect(anyLong());

        mSscTransaction.startGetTransaction(queryData);
        sleepToWaitThreadRun();
        waitThreadWorkFinished();

        verify(mMockSscHttpConnectionGov).sendRequest(SLOT_0, ISscHttpConnection.HTTP_REQUEST_GET,
                mDefaultRequestUri, mDefaultXui, "");
        verify(mMockSscXmlGov).parseXmlStream(eq(queryData), any());

        verifyTransactionSuccess(SscConstant.EVENT_SSC_QUERY_CF,
                ISscHttpConnection.HTTP_REQUEST_GET);
    }

    @Test
    public void startPutTransaction_creatingXmlBodyError() {
        SscServiceData updateData = getUpdateData(SscConstant.CONDITION_CFU,
                SscConstant.ACTION_ACTIVATION, "+1234567890", 0);

        when(mMockSscXmlGov.createXmlStream(eq(updateData))).thenReturn(null);

        mSscTransaction.startPutTransaction(updateData);
        sleepToWaitThreadRun();
        waitThreadWorkFinished();

        verify(mMockSscXmlGov).createXmlStream(eq(updateData));

        verifyTransactionFailure(SscConstant.EVENT_SSC_UPDATE_CF, true, false);
    }

    @Test
    public void startPutTransaction_httpErrorResponseWithErrorPhrase() {
        int httpErrorResponse = SscConstant.HTTP_CONFLICT;
        String xmlBody = createXmlStream();
        SscServiceData updateData = getUpdateData(SscConstant.CONDITION_CFU,
                SscConstant.ACTION_ACTIVATION, "+1234567890", 0);

        when(mMockSscXmlGov.createXmlStream(eq(updateData))).thenReturn(xmlBody);
        when(mMockSscHttpConnectionGov.sendRequest(SLOT_0, ISscHttpConnection.HTTP_REQUEST_PUT,
                mDefaultRequestUri, mDefaultXui, xmlBody)).thenReturn(httpErrorResponse);
        when(mMockSscHttpConnectionGov.getInputStream(eq(SLOT_0))).thenReturn(getInputStream());
        when(mMockSscXmlGov.parseXmlStream(eq(updateData), any()))
                .thenReturn(parseXmlStream(updateData));

        mSscTransaction.startPutTransaction(updateData);
        sleepToWaitThreadRun();
        waitThreadWorkFinished();

        verify(mMockSscHttpConnectionGov).sendRequest(SLOT_0, ISscHttpConnection.HTTP_REQUEST_PUT,
                mDefaultRequestUri, mDefaultXui, xmlBody);
        verify(mMockSscServiceStateAgent).setErrorResponseCode(eq(SLOT_0), eq(httpErrorResponse));
        verify(mMockSscHttpConnectionGov).getInputStream(eq(SLOT_0));
        verify(mMockSscXmlGov).parseXmlStream(eq(updateData), any());

        verifyTransactionFailure(SscConstant.EVENT_SSC_UPDATE_CF, true, true);
    }

    @Test
    public void startPutTransaction_success() {
        int httpSuccessResponse = SscConstant.HTTP_OK;
        String xmlBody = createXmlStream();
        SscServiceData updateData = getUpdateData(SscConstant.CONDITION_CFU,
                SscConstant.ACTION_ACTIVATION, "+1234567890", 0);

        when(mMockSscXmlGov.createXmlStream(eq(updateData))).thenReturn(xmlBody);
        when(mMockSscHttpConnectionGov.sendRequest(SLOT_0, ISscHttpConnection.HTTP_REQUEST_PUT,
                mDefaultRequestUri, mDefaultXui, xmlBody)).thenReturn(httpSuccessResponse);
        when(mMockSscHttpConnectionGov.getInputStream(eq(SLOT_0))).thenReturn(null);

        mSscTransaction.startPutTransaction(updateData);
        sleepToWaitThreadRun();
        waitThreadWorkFinished();

        verify(mMockSscHttpConnectionGov).sendRequest(SLOT_0, ISscHttpConnection.HTTP_REQUEST_PUT,
                mDefaultRequestUri, mDefaultXui, xmlBody);

        verifyTransactionSuccess(SscConstant.EVENT_SSC_UPDATE_CF,
                ISscHttpConnection.HTTP_REQUEST_PUT);
    }

    @Test
    public void startPutTransaction_successForInsert() {
        int httpSuccessResponse = SscConstant.HTTP_OK;
        String xmlBody = createXmlStream();
        SscServiceData insertData = getInsertData(ESsType.CF);

        when(mMockSscXmlGov.createXmlStream(eq(insertData))).thenReturn(xmlBody);
        when(mMockSscHttpConnectionGov.sendRequest(SLOT_0, ISscHttpConnection.HTTP_REQUEST_PUT,
                mDefaultRequestUri, mDefaultXui, xmlBody)).thenReturn(httpSuccessResponse);
        when(mMockSscHttpConnectionGov.getInputStream(eq(SLOT_0))).thenReturn(null);

        mSscTransaction.startPutTransaction(insertData);
        sleepToWaitThreadRun();
        waitThreadWorkFinished();

        verify(mMockSscHttpConnectionGov).sendRequest(SLOT_0, ISscHttpConnection.HTTP_REQUEST_PUT,
                mDefaultRequestUri, mDefaultXui, xmlBody);
        verify(mMockSscXmlGov).updateTagsAndRules();

        verifyTransactionSuccess(SscConstant.EVENT_SSC_INSERT_CF,
                ISscHttpConnection.HTTP_REQUEST_PUT);
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
        when(mMockSscHttpConnectionGov.sendRequest(SLOT_0, ISscHttpConnection.HTTP_REQUEST_PUT,
                mDefaultRequestUri, mDefaultXui, xmlBody)).thenReturn(httpSuccessResponse);
        when(mMockSscHttpConnectionGov.getInputStream(eq(SLOT_0))).thenReturn(null);

        mSscTransaction.startPutTransaction(updateData);
        sleepToWaitThreadRun();
        waitThreadWorkFinished();

        verify(mMockSscHttpConnectionGov).sendRequest(SLOT_0, ISscHttpConnection.HTTP_REQUEST_PUT,
                mDefaultRequestUri, mDefaultXui, xmlBody);
        assertEquals(false, SscXmlFormat.getIsNoReplyTimerOmitted(SLOT_0));

        verifyTransactionSuccess(SscConstant.EVENT_SSC_UPDATE_CF,
                ISscHttpConnection.HTTP_REQUEST_PUT);
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
        when(mMockSscHttpConnectionGov.sendRequest(SLOT_0, ISscHttpConnection.HTTP_REQUEST_PUT,
                requestUri, sipXui, xmlBody)).thenReturn(httpSuccessResponse);
        when(mMockSscHttpConnectionGov.getInputStream(eq(SLOT_0))).thenReturn(null);

        mSscTransaction.startPutTransaction(updateData);
        sleepToWaitThreadRun();
        waitThreadWorkFinished();

        verify(mMockSscHttpConnectionGov).sendRequest(SLOT_0, ISscHttpConnection.HTTP_REQUEST_PUT,
                requestUri, sipXui, xmlBody);

        verifyTransactionSuccess(SscConstant.EVENT_SSC_UPDATE_CB,
                ISscHttpConnection.HTTP_REQUEST_PUT);
    }

    private void verifyTransactionSuccess(int requestedEventNum, int requestType) {
        verify(mMockCallbackHandler).sendMessage(mMessageCaptor.capture());

        Message msg = mMessageCaptor.getValue();
        assertNotNull(msg);
        assertEquals(requestedEventNum, msg.what);

        SscRequestResult rr = (SscRequestResult) msg.obj;
        assertNotNull(rr);
        assertEquals(SLOT_0, rr.getSlotId());
        assertEquals(mTransactionId, rr.getTransactionId());
        assertEquals(SscConstant.REQUEST_SUCCESS, rr.getResultState());
        if (requestType == ISscHttpConnection.HTTP_REQUEST_GET) {
            SscServiceData errorPhraseData = rr.getSscServiceData();
            assertNotNull(errorPhraseData);
        } else if (requestType == ISscHttpConnection.HTTP_REQUEST_PUT) {
            assertNull(rr.getSscServiceData());
        }

        verify(mMockImsRadioInterface).stopImsTraffic(any());
        verifyNoMoreInteractions(mMockCallbackHandler);
        verifyNoMoreInteractions(mMockSscServiceStateAgent);
    }

    private void verifyTransactionFailure(int requestedEventNum, boolean xcapTrafficNotified,
            boolean isErrorPhraseExist) {
        verify(mMockCallbackHandler).sendMessage(mMessageCaptor.capture());

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

        if (xcapTrafficNotified) {
            verify(mMockImsRadioInterface).stopImsTraffic(any());
        } else {
            verify(mMockImsRadioInterface, never()).stopImsTraffic(any());
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
        return SscXmlGovTest.createQueryData(ESsType.CF, mTransactionId, condition,
                SscServiceClassUtil.SERVICE_CLASS_NONE);
    }

    private SscServiceData getUpdateData(int condition, int action, String number,
            int timeSeconds) {
        return new CfServiceUpdateData(SLOT_0, ESsType.CF,
                SscConstant.EVENT_SSC_UPDATE_CF, mTransactionId, action, condition,
                number, timeSeconds, SscServiceClassUtil.SERVICE_CLASS_CALL);
    }

    private SscServiceData getInsertData(ESsType ssType) {
        return SscXmlGovTest.createInsertData(ssType, mTransactionId, SscConstant.ACTION_ACTIVATION,
                SscConstant.CONDITION_CFU, null, SscServiceClassUtil.SERVICE_CLASS_NONE);
    }

    private void sleepToWaitThreadRun() {
        while (mSscTransaction.getTransactionThread().isAlive()
                && mSscTransaction.getTransactionHandler() == null) {
            android.os.SystemClock.sleep(50);
        }
    }

    /*
     * transaction is processed in an other thread. It should wait the thread to be finished when
     * there is a message in the transaction handler.
     */
    private void waitThreadWorkFinished() {
        try {
            mSscTransaction.getTransactionThread().join();
        } catch (InterruptedException e) {
            // do nothing
        }
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
            fail("getInputStream : " + e);
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
