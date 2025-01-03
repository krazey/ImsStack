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

import static android.telephony.TelephonyManager.NETWORK_TYPE_IWLAN;

import android.os.Handler;
import android.os.Looper;
import android.os.Message;
import android.text.TextUtils;

import com.android.imsstack.core.agents.AgentFactory;
import com.android.imsstack.core.agents.GbaInterface;
import com.android.imsstack.core.agents.GbaInterface.GbaCredentials;
import com.android.imsstack.core.agents.ImsRadioInterface;
import com.android.imsstack.core.agents.ImsRadioInterface.ConnectionListener;
import com.android.imsstack.enabler.ssc.data.CbServiceUpdateData;
import com.android.imsstack.enabler.ssc.data.CfServiceUpdateData;
import com.android.imsstack.enabler.ssc.data.SscData;
import com.android.imsstack.enabler.ssc.data.SscRequestResult;
import com.android.imsstack.enabler.ssc.data.SscServiceData;
import com.android.imsstack.enabler.ssc.data.SscServiceQueryData;
import com.android.imsstack.util.ImsLog;
import com.android.internal.annotations.VisibleForTesting;

import org.w3c.dom.Document;

public class SscTransaction {
    public static final int EVENT_SEND_HTTP_REQUEST = 1001;
    private static final int DEFAULT_GBA_TIMEOUT_SEC = 30;
    private static final int DEFAULT_PDN_CONNECTION_TIMEOUT_MS = 30 * 1000; // 30 sec

    private final int mSlotId;
    private final SscXmlGov mXmlGov;
    private final ImsRadioInterface mImsRadio;

    private int mEventNumber = 0;
    private int mTransactionId = 0;
    private boolean mXcapTrafficNotified = false;
    private boolean mXcapTrafficStarted = false;

    private Handler mTransactionHandler = null;
    private Handler mSscServiceImplHandler = null;

    private Thread mSscTransactionThread = null;
    private HttpTransaction mTransaction = null;

    /**
     * This is listener to check if the XCAP traffic can be processed now.
     */
    private final ConnectionListener mConnectionListener = new ConnectionListener() {
        @Override
        public void onConnectionFailed(int failureReason, int causeCode, int waitTimeMillis) {
            ImsLog.e(mSlotId, "starting XCAP traffic failed. failureReason = " + failureReason
                    + ", causeCode = " + causeCode);
            sendFailMessageToServiceImpl(mEventNumber, mTransactionId);
        }

        @Override
        public void onConnectionSetupPrepared() {
            if (!mXcapTrafficStarted) {
                if (mTransactionHandler != null) {
                    // mXcapTrafficStarted will never be false once it's set as true to prevent
                    // unintended HTTP requests for one XCAP transaction
                    mXcapTrafficStarted = true;
                    mTransactionHandler.sendEmptyMessage(EVENT_SEND_HTTP_REQUEST);
                }
            }
        }
    };

    public SscTransaction(int slotId, Handler handler) {
        mSlotId = slotId;
        mXmlGov = getSscXmlGov();
        mImsRadio = AgentFactory.getInstance().getAgent(ImsRadioInterface.class, slotId);
        mSscServiceImplHandler = handler;
    }

    public void close() {
        ImsLog.d(mSlotId, "");
        if (mTransactionHandler != null) {
            mTransactionHandler.removeCallbacksAndMessages(null);
            mTransactionHandler.getLooper().quit();
            mTransactionHandler = null;
        }

        if (mImsRadio != null) {
            if (mXcapTrafficNotified) {
                mImsRadio.stopImsTraffic(mConnectionListener);
                mXcapTrafficNotified = false;
            }
        }

        SscNetConnectionGov.getInstance().setCallbackHandler(mSlotId, null);
    }

    public void startGetTransaction(SscServiceQueryData data) {
        ImsLog.d("");

        mSscTransactionThread = new SscTransactionThread();
        mTransaction = new GetTransaction(data);
        mEventNumber = data.getEventNumber();
        mTransactionId = data.getTransactionId();
        mSscTransactionThread.start();
    }

    public void startPutTransaction(SscServiceData data) {
        ImsLog.d("");

        mSscTransactionThread = new SscTransactionThread();
        mTransaction = new PutTransaction(data);
        mEventNumber = data.getEventNumber();
        mTransactionId = data.getTransactionId();
        mSscTransactionThread.start();
    }

    private void sendMessageToServiceImpl(int eventNum, int transactionId, int resultState,
            int responseCode, SscServiceData data) {
        ImsLog.d("");
        Message msg = Message.obtain();
        msg.what = eventNum;
        SscRequestResult rr = new SscRequestResult(mSlotId, transactionId, resultState,
                responseCode, -1);
        if (data != null) {
            rr.setSscServiceData(data);
        }
        msg.obj = rr;

        mSscServiceImplHandler.sendMessage(msg);
        close();
    }

    private void sendFailMessageToServiceImpl(int eventNum, int transactionId) {
        ImsLog.d("");
        Message msg = Message.obtain();
        msg.what = eventNum;
        msg.obj = new SscRequestResult(mSlotId, transactionId, SscConstant.REQUEST_FAILURE, 0, -1);

        mSscServiceImplHandler.sendMessage(msg);
        close();
    }

    private abstract class HttpTransaction {
        protected abstract int getRequestType();
        protected abstract String getRequestUri(String xui);
        protected abstract String getXmlBody();
        protected abstract String getPassword();
        protected abstract void processResponse(ISscHttpConnectionGov httpConnection,
                int responseCode);

        public void startTransaction() {
            ImsLog.d(mSlotId, "");

            ISscNetConnectionGov netConnectionGov = SscNetConnectionGov.getInstance();
            if (netConnectionGov.isConnected(mSlotId)) {
                netConnectionGov.refreshConnectionTimer(mSlotId);
            } else {
                ImsLog.i(mSlotId, "PDN is not connected. Trying to Connect");
                if (!netConnectionGov.connect(mSlotId, DEFAULT_PDN_CONNECTION_TIMEOUT_MS)) {
                    ImsLog.i(mSlotId, "PDN connection fail");
                    sendFailMessageToServiceImpl(mEventNumber, mTransactionId);
                }
                return;
            }

            if (mImsRadio == null) {
                ImsLog.e(mSlotId, "Can't start the XCAP traffic");
                sendFailMessageToServiceImpl(mEventNumber, mTransactionId);
                return;
            }

            // If XCAP is connected over cellular, it needs to check priorities for ongoing traffics
            if (SscNetConnectionGov.getInstance().getNetworkType(mSlotId) != NETWORK_TYPE_IWLAN) {
                if (!mImsRadio.isImsTrafficAllowed(ImsRadioInterface.TRAFFIC_TYPE_UT_XCAP)) {
                    ImsLog.e(mSlotId, "XCAP traffic not allowed");
                    sendFailMessageToServiceImpl(mEventNumber, mTransactionId);
                    return;
                }
            }

            stratXcapTraffic();
        }

        private void sendRequest() {
            String xui = getSscXui().getXui(mSlotId, getPassword());
            if (TextUtils.isEmpty(xui)) {
                ImsLog.e(mSlotId, "Invalid XUI");
                sendFailMessageToServiceImpl(mEventNumber, mTransactionId);
                return;
            }

            String requestUri = getRequestUri(xui);
            if (TextUtils.isEmpty(requestUri)) {
                ImsLog.e(mSlotId, "Invalid requestUri");
                sendFailMessageToServiceImpl(mEventNumber, mTransactionId);
                return;
            }

            String body = getXmlBody();
            if (body == null) {
                ImsLog.e(mSlotId, "Invalid body");
                sendFailMessageToServiceImpl(mEventNumber, mTransactionId);
                return;
            }

            boolean gbaEnabled = SscConfig.isGbaSupported(mSlotId);
            if (gbaEnabled) {
                getGbaKey(false);
            }

            ISscHttpConnectionGov httpConnection = getSscHttpConnectionGov();
            int responseCode = httpConnection.sendRequest(mSlotId, getRequestType(), requestUri,
                    xui, body);
            ImsLog.i(mSlotId, "response Code : " + responseCode);

            if (responseCode == SscConstant.HTTP_UNAUTHORIZED) {
                if (gbaEnabled) {
                    if (!getGbaKey(true)) {
                        getSscServiceStateAgent().setGbaRequestFailed(mSlotId, true);
                        sendFailMessageToServiceImpl(mEventNumber, mTransactionId);
                        return;
                    }

                    responseCode = httpConnection.sendRequest(mSlotId, getRequestType(), requestUri,
                            xui, body);
                }
            }

            if (responseCode < ISscHttpConnection.HTTP_REQUEST_FAILED_MAX) {
                if (responseCode == ISscHttpConnection.HTTP_REQUEST_FAILED_BY_DNS) {
                    ImsLog.e(mSlotId, "DNS failed");
                    getSscServiceStateAgent().setDnsQueryFailed(mSlotId, true);
                } else if (responseCode == ISscHttpConnection.HTTP_REQUEST_FAILED_BY_TIMEOUT) {
                    ImsLog.e(mSlotId, "Connection failed");
                    getSscServiceStateAgent().setSocketConnectionExpired(mSlotId, true);
                }

                sendFailMessageToServiceImpl(mEventNumber, mTransactionId);
                return;
            }

            if (responseCode == SscConstant.HTTP_UNAUTHORIZED // 401 again
                    || responseCode == SscConstant.HTTP_FORBIDDEN) {
                getSscAuthAgent().setIsCredentialInfoUpdated(false);
            }

            processResponse(httpConnection, responseCode);
        }
    }

    private final class GetTransaction extends HttpTransaction {
        SscServiceQueryData mData = null;

        public GetTransaction(SscServiceQueryData data) {
            this.mData = data;
        }

        @Override
        protected int getRequestType() {
            return ISscHttpConnection.HTTP_REQUEST_GET;
        }

        @Override
        protected String getRequestUri(String xui) {
            return getSscUrl().getQueryUri(mData, xui);
        }

        @Override
        protected String getXmlBody() {
            return "";
        }

        @Override
        protected String getPassword() {
            return null;
        }

        @Override
        protected void processResponse(ISscHttpConnectionGov httpConnection, int responseCode) {
            mData.setResponseCode(responseCode);
            int resultState = SscConstant.REQUEST_SUCCESS;

            if (responseCode < 200 || responseCode >= 300) {
                ImsLog.d(mSlotId, "not received 200 OK");
                resultState = SscConstant.REQUEST_FAILURE;
                getSscServiceStateAgent().setErrorResponseCode(mSlotId, responseCode);
            }

            Document doc = httpConnection.getInputStream(mSlotId);
            SscServiceData dataFromServer = null;
            if (doc != null) {
                dataFromServer = mXmlGov.parseXmlStream(mData, doc);
            }

            if (dataFromServer == null) {
                ImsLog.e(mSlotId, "dataFromServer is null");
                resultState = SscConstant.REQUEST_FAILURE;
            }

            sendMessageToServiceImpl(mEventNumber, mTransactionId, resultState, responseCode,
                    dataFromServer);
        }
    }

    private final class PutTransaction extends HttpTransaction {
        SscServiceData mData = null;

        PutTransaction(SscData sscData) {
            this.mData = (SscServiceData) sscData;
        }

        @Override
        protected int getRequestType() {
            return ISscHttpConnection.HTTP_REQUEST_PUT;
        }

        @Override
        protected String getRequestUri(String xui) {
            return getSscUrl().getUpdateUri(mData, xui);
        }

        @Override
        protected String getXmlBody() {
            return mXmlGov.createXmlStream(mData);
        }

        @Override
        protected String getPassword() {
            return (mData instanceof CbServiceUpdateData)
                    ? ((CbServiceUpdateData)mData).getPassword() : null;
        }

        @Override
        protected void processResponse(ISscHttpConnectionGov httpConnection, int responseCode) {
            mData.setResponseCode(responseCode);
            int resultState = SscConstant.REQUEST_SUCCESS;
            if (responseCode < 200 || responseCode >= 300) {
                ImsLog.d(mSlotId, "not received 200 OK");
                resultState = SscConstant.REQUEST_FAILURE;
                getSscServiceStateAgent().setErrorResponseCode(mSlotId, responseCode);
            }

            Document doc = httpConnection.getInputStream(mSlotId);
            SscServiceData dataFromServer = null;
            mXmlGov.syncCachedDataWithUpdatedData(responseCode);
            if (resultState == SscConstant.REQUEST_FAILURE && doc != null) {
                dataFromServer = mXmlGov.parseXmlStream(mData, doc);
                if (dataFromServer == null) {
                    ImsLog.e(mSlotId, "dataFromServer is null");
                }
            }

            if (resultState == SscConstant.REQUEST_SUCCESS) {
                if (mData instanceof CfServiceUpdateData) {
                    if (mData.getCondition() == SscConstant.CONDITION_CFNR_TIMER) {
                        if (SscXmlFormat.getIsNoReplyTimerOmitted(mSlotId)) {
                            SscXmlFormat.setIsNoReplyTimerOmitted(mSlotId, false);
                        }
                    }
                }

                if (mData.getEventNumber() == SscConstant.EVENT_SSC_INSERT_CB
                        || mData.getEventNumber() == SscConstant.EVENT_SSC_INSERT_CF) {
                    mXmlGov.updateTagsAndRules();
                }
            }

            sendMessageToServiceImpl(mEventNumber, mTransactionId, resultState, responseCode,
                    dataFromServer);
        }
    }

    @VisibleForTesting
    protected Handler getTransactionHandler() {
        return mTransactionHandler;
    }

    @VisibleForTesting
    protected Thread getTransactionThread() {
        return mSscTransactionThread;
    }

    @VisibleForTesting
    protected SscXmlGov getSscXmlGov() {
        return SscXmlGov.getInstance(mSlotId);
    }

    @VisibleForTesting
    protected SscXui getSscXui() {
        return SscXui.getInstance();
    }

    @VisibleForTesting
    protected SscUrl getSscUrl() {
        return SscUrl.getInstance();
    }

    @VisibleForTesting
    protected SscServiceStateAgent getSscServiceStateAgent() {
        return SscServiceStateAgent.getInstance();
    }

    @VisibleForTesting
    protected SscUtils getSscUtils() {
        return SscUtils.getInstance();
    }

    @VisibleForTesting
    protected ISscHttpConnectionGov getSscHttpConnectionGov() {
        return SscHttpConnectionGov.getInstance();
    }

    @VisibleForTesting
    protected ISscAuthAgent getSscAuthAgent() {
        return SscAuthAgent.getInstance(mSlotId);
    }

    @VisibleForTesting
    protected GbaInterface getGbaAgent() {
        return AgentFactory.getInstance().getAgent(GbaInterface.class, mSlotId);
    }

    private boolean getGbaKey(boolean forceBootStrapping) {
        GbaInterface gbaAgent = getGbaAgent();
        if (gbaAgent == null) {
            return false;
        }

        ISscAuthAgent authAgent = getSscAuthAgent();
        if (!authAgent.isCredentialInfoUpdated()) {
            return false;
        }

        int appType = getSscUtils().getTelephonySimType(mSlotId);
        int gbaMode = SscConfig.getGbaMode(mSlotId);
        boolean isTls = SscConfig.isTls(mSlotId);
        String nafFqdn = authAgent.getNafFqdn();
        String securityProtocol = authAgent.getCipherSuite();

        GbaCredentials gbaCredentials = gbaAgent.getGbaKey(appType, gbaMode, isTls, nafFqdn,
                securityProtocol, forceBootStrapping, DEFAULT_GBA_TIMEOUT_SEC);
        if (gbaCredentials == null || gbaCredentials.getResult() == GbaInterface.RESULT_FAILURE) {
            ImsLog.e(mSlotId, "Getting gba key failure");
            authAgent.setIsCredentialInfoUpdated(false);
            return false;
        }

        authAgent.setGbaKeys(gbaCredentials.getTransactionId(), gbaCredentials.getKey());
        return true;
    }

    // Actual XCAP traffic will start once onConnectionSetupPrepared()
    private void stratXcapTraffic() {
        if (mImsRadio != null) {
            int networkType = SscNetConnectionGov.getInstance().getNetworkType(mSlotId);
            int convertedNetworkType = getSscUtils().convertToImsRadioNetworkType(networkType);
            ImsLog.d(mSlotId, "access network type is : " + convertedNetworkType);

            mImsRadio.startImsTraffic(ImsRadioInterface.TRAFFIC_TYPE_UT_XCAP, convertedNetworkType,
                    ImsRadioInterface.DIRECTION_MO, mConnectionListener);
            mXcapTrafficNotified = true;
        }
    }

    private final class SscTransactionThread extends Thread {
        SscTransactionThread() {
            super("SscTransactionThread");
        }

        public void run() {
            Looper.prepare();

            mTransactionHandler = new TransactionHandler(Looper.myLooper());
            SscNetConnectionGov.getInstance().setCallbackHandler(mSlotId, mTransactionHandler);
            mTransaction.startTransaction();

            Looper.loop();
        }
    }

    private class TransactionHandler extends Handler {
        TransactionHandler(Looper looper) {
            super(looper);
        }

        @Override
        public void handleMessage(Message msg) {
            ImsLog.d("SscTransactionHandler - what=" + msg.what);

            switch (msg.what) {
                case EVENT_SEND_HTTP_REQUEST:
                    ImsLog.d("sendRequest");
                    mTransaction.sendRequest();
                    break;
                case SscNetConnection.EVENT_PDN_CONNECTED:
                    ImsLog.d("PDN Connected");
                    mTransaction.startTransaction();
                    break;
                case SscNetConnection.EVENT_PDN_DISCONNECTED:
                    ImsLog.d("PDN Disconnected");
                    sendFailMessageToServiceImpl(mEventNumber, mTransactionId);
                    break;
                case SscNetConnection.EVENT_PDN_IPCAN_CHANGED:
                    ImsLog.d("PDN IPCAN Changed");
                    if (mXcapTrafficNotified) {
                        stratXcapTraffic();
                    }
                    break;
                case SscNetConnection.EVENT_PDN_REQUEST_TIMEOUT:
                    ImsLog.d("Connection Timeout");
                    sendFailMessageToServiceImpl(mEventNumber, mTransactionId);
                    break;
                case SscNetConnection.EVENT_PDN_CONNECTION_FAILED:
                    ImsLog.d("Connection Failed");
                    sendFailMessageToServiceImpl(mEventNumber, mTransactionId);
                    break;
                default:
                    // do nothing
                    break;
            }
        }
    }
}
