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

import android.os.Handler;
import android.os.Looper;
import android.os.Message;
import android.text.TextUtils;
import android.util.Pair;

import com.android.imsstack.core.agents.AgentFactory;
import com.android.imsstack.core.agents.GbaInterface;
import com.android.imsstack.core.agents.TRMAgent;
import com.android.imsstack.core.agents.agentif.ITRM;
import com.android.imsstack.enabler.ssc.data.CbServiceUpdateData;
import com.android.imsstack.enabler.ssc.data.CfServiceUpdateData;
import com.android.imsstack.enabler.ssc.data.SscData;
import com.android.imsstack.enabler.ssc.data.SscRequestResult;
import com.android.imsstack.enabler.ssc.data.SscServiceData;
import com.android.imsstack.enabler.ssc.data.SscServiceQueryData;
import com.android.imsstack.util.ImsLog;
import com.android.internal.annotations.VisibleForTesting;

import org.w3c.dom.Document;

import java.lang.ref.WeakReference;

public class SscTransaction {
    public static final int EVENT_SRV_RETRY_REQUIRED = 1001;

    protected SscXmlGov mXmlGov = null;

    protected Handler mTransactionHandler = null;
    protected Handler mSscServiceImplHandler = null;

    protected Thread mSscTransactionThread = null;
    protected HttpTransaction mTransaction = null;

    protected int mEventNumber = 0;
    protected int mTransactionId = 0;

    protected int mTimerId = -1;
    protected int mSlotId = -1;

    public SscTransaction(int slotId, Handler handler) {
        mSlotId = slotId;
        mSscServiceImplHandler = handler;
        mXmlGov = getSscXmlGov();
    }

    public void close() {
        ImsLog.d(mSlotId, "");
        if (mTransactionHandler != null) {
            mTransactionHandler.getLooper().quit();
            mTransactionHandler = null;
        }

        SscNetConnectionGov.getInstance().setCallbackHandler(mSlotId, null);
    }

    public void startGetTransaction(SscServiceQueryData data) {
        ImsLog.d("");

        mSscTransactionThread = new SscTransactionThread(this);
        mTransaction = new GetTransaction(data);
        mEventNumber = data.getEventNumber();
        mTransactionId = data.getTransactionId();
        mSscTransactionThread.start();
    }

    public void startPutTransaction(SscServiceData data) {
        ImsLog.d("");

        mSscTransactionThread = new SscTransactionThread(this);
        mTransaction = new PutTransaction(data);
        mEventNumber = data.getEventNumber();
        mTransactionId = data.getTransactionId();
        mSscTransactionThread.start();
    }

    protected void sendMessageToServiceImpl(int eventNum, int transactionId, int resultState,
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

    protected void sendFailMessageToServiceImpl(int eventNum, int transactionId) {
        ImsLog.d("");
        Message msg = Message.obtain();
        msg.what = eventNum;
        msg.obj = new SscRequestResult(mSlotId, transactionId, SscConstant.REQUEST_FAILURE, 0, -1);

        mSscServiceImplHandler.sendMessage(msg);
        close();
    }

    protected final class SscTransactionThread extends Thread {
        private SscTransaction mSscTransaction = null;

        public SscTransactionThread(SscTransaction SscTransaction) {
            super("SscTransactionThread");
            this.mSscTransaction = SscTransaction;
        }

        public void run() {
            Looper.prepare();

            ImsLog.d("SscTransactionThread is running ... (" + android.os.Process.myTid() + ")");
            mTransactionHandler = new TransactionHandler(mSscTransaction, Looper.myLooper());
            SscNetConnectionGov.getInstance().setCallbackHandler(mSlotId, mTransactionHandler);
            mTransaction.startTransaction();

            Looper.loop();
        }
    }

    /* TODO: check when implementing NAPTR/SRV
    protected boolean isSrvRetryRequired(int responseCode) {
        if (SscConfig.isSrvRecordsRequired(mSlotId) == false) {
            return false;
        }
        if (responseCode == SscConstant.HTTP_PRECONDITION_FAILURE) {
            return false;
        }
        if (responseCode < 200 || responseCode >= 500 ||
                responseCode == SscHttpConnection.REQUEST_FAILED) {
            SscDnsQuery.getInstance().setNAFFailed(true);
            return true;
        }
        return false;
    }
     */

    private abstract class HttpTransaction {
        protected abstract int getRequestType();
        protected abstract String getRequestUri(String xui);
        protected abstract String getXmlBody();
        protected abstract String getPassword();
        protected abstract void processResponse(ISscHttpConnectionGov httpConnection,
                int responseCode);

        public void startTransaction() {
            ImsLog.d(mSlotId, "");

            if (isTrmSupported()) {
                if (!isTrmAvailable()) {
                    ImsLog.e(mSlotId, "TRM is not available");
                    sendFailMessageToServiceImpl(mEventNumber, mTransactionId);
                    return;
                }
            }

            ISscNetConnectionGov netConnectionGov = SscNetConnectionGov.getInstance();
            if (netConnectionGov.isConnected(mSlotId)) {
                netConnectionGov.refreshConnectionTimer(mSlotId);
            } else {
                ImsLog.i(mSlotId, "PDN is not connected. Trying to Connect");
                if (!netConnectionGov.connect(mSlotId)) {
                    ImsLog.i(mSlotId, "PDN connection fail");
                    sendFailMessageToServiceImpl(mEventNumber, mTransactionId);
                }
                return;
            }

            sendRequest();
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
            httpConnection.setXuiValue(mSlotId, xui);
            int responseCode = httpConnection.sendRequest(mSlotId, getRequestType(), requestUri,
                    body);
            ImsLog.i(mSlotId, "response Code : " + responseCode);

            if (responseCode == SscConstant.HTTP_UNAUTHORIZED) {
                if (gbaEnabled) {
                    if (getGbaKey(true) == false) {
                        getSscServiceStateAgent().setGbaRequestFailed(mSlotId, true);
                        sendFailMessageToServiceImpl(mEventNumber, mTransactionId);
                        return;
                    }

                    responseCode = httpConnection.sendRequest(mSlotId, getRequestType(), requestUri,
                            body);
                    if (responseCode == SscConstant.HTTP_UNAUTHORIZED) { // 401 again
                        ISscAuthAgent authAgent = getSscAuthAgent();
                        authAgent.setIsCredentialInfoUpdated(false);
                        sendFailMessageToServiceImpl(mEventNumber, mTransactionId);
                        return;
                    }
                }
            }
            /* TODO: TODO: check when implementing NAPTR/SRV
            if (isSrvRetryRequired(responseCode)) {
                ImsLog.d(mSlotId, "SRV Retry Needed");
                if (mTransactionHandler != null) {
                    mTransactionHandler.sendEmptyMessage(EVENT_SRV_RETRY_REQUIRED);
                } else {
                    ImsLog.e(mSlotId, "mTransactionHandler is null");
                    sendFailMessageToServiceImpl(mEventNumber, mTransactionId);
                }
                return;
            }
            */
            if (responseCode == SscHttpConnection.REQUEST_FAILED) {
                ImsLog.e(mSlotId, "Connection failed");
                getSscServiceStateAgent().setSocketConnectionExpired(mSlotId, true);
                sendFailMessageToServiceImpl(mEventNumber, mTransactionId);
                return;
            }

            processResponse(httpConnection, responseCode);
        }
    }

    protected class GetTransaction extends HttpTransaction {
        SscServiceQueryData mData = null;

        public GetTransaction(SscServiceQueryData data) {
            this.mData = data;
        }

        @Override
        protected int getRequestType() {
            return SscHttpConnection.HTTP_GET_REQUEST;
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

    protected class PutTransaction extends HttpTransaction {
        SscServiceData mData = null;

        public PutTransaction(SscData SscData) {
            this.mData = (SscServiceData) SscData;
        }

        @Override
        protected int getRequestType() {
            return SscHttpConnection.HTTP_PUT_REQUEST;
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

    private boolean isTrmAvailable() {
        return getTrmAgent().isServiceAvailable(mSlotId, TRMAgent.SERVICE_UT);
    }

    private boolean isTrmSupported() {
        return getTrmAgent().isTRMSupported();
    }

    @VisibleForTesting
    protected ITRM getTrmAgent() {
        return TRMAgent.getInstance();
    }

    @VisibleForTesting
    protected Handler getTransactionHandler() {
        return mTransactionHandler;
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
        if (authAgent.isCredentialInfoUpdated() == false) {
            return false;
        }

        int appType = getSscUtils().getTelephonySimType(mSlotId);
        int gbaMode = SscConfig.getGbaMode(mSlotId);
        boolean isTls = SscConfig.isTls(mSlotId);
        String nafFqdn = authAgent.getNafFqdnFromRealm();
        String securityProtocol = authAgent.getCipherSuite();

        Pair<String, String> keyPair = gbaAgent.getGbaKey(appType, gbaMode, isTls, nafFqdn,
                securityProtocol, forceBootStrapping);
        if (keyPair == null || keyPair.first == null || keyPair.second == null) {
            ImsLog.e(mSlotId, "Getting gba key failure");
            authAgent.setIsCredentialInfoUpdated(false);
            return false;
        }

        authAgent.setGbaKeys(keyPair.first, keyPair.second);
        return true;
    }

    public static class TransactionHandler extends Handler {
        private final WeakReference<SscTransaction> mService;

        public TransactionHandler(SscTransaction service, Looper looper) {
            super(looper);
            mService = new WeakReference<SscTransaction>(service);
        }

        @Override
        public void handleMessage(Message msg) {
            SscTransaction service = mService.get();
            if (service != null) {
                service.handleMessage(msg);
            }
        }
    }

    public void handleMessage(Message msg) {
        ImsLog.d("SscTransactionHandler - what=" + msg.what);

        switch (msg.what) {
            case SscNetConnection.EVENT_PDN_CONNECTED:
                ImsLog.d("PDN Connected");
                if (mSscTransactionThread == null) {
                    ImsLog.e("mSscTransactionThread is null");
                    return;
                }
                mTransaction.startTransaction();
                break;
            case SscNetConnection.EVENT_PDN_DISCONNECTED:
                ImsLog.d("PDN Disconnected");
                sendFailMessageToServiceImpl(mEventNumber, mTransactionId);
                break;
            case SscNetConnection.EVENT_PDN_CONNECTION_FAILED:
                ImsLog.d("Connection Failed");
                sendFailMessageToServiceImpl(mEventNumber, mTransactionId);
                break;
            case SscNetConnection.EVENT_PDN_CONNECTION_TIMEOUT:
                ImsLog.d("Connection Timeout");
                sendFailMessageToServiceImpl(mEventNumber, mTransactionId);
                break;
            /* TODO: check when implementing NAPTR/SRV
            case EVENT_SRV_RETRY_REQUIRED:
                if (getSscServiceStateAgent().getAllSrvAddrTried(mSlotId) == true) {
                    sendFailMessageToServiceImpl(mEventNumber, mTransactionId);
                } else {
                    SscAuthAgent.getInstance(mSlotId).setIsCredentialInfoUpdated(false);
                    mTransaction.startTransaction();
                }
                break;
            */
            default:
                ImsLog.e("Unhanddled Message :" + msg.what);
                break;
        }
    }
}