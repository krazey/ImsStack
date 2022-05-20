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
import com.android.imsstack.core.agents.TRMAgent;
import com.android.imsstack.core.agents.agentif.IGBA;
import com.android.imsstack.enabler.ssc.data.CbServiceUpdateData;
import com.android.imsstack.enabler.ssc.data.CfServiceUpdateData;
import com.android.imsstack.enabler.ssc.data.SscData;
import com.android.imsstack.enabler.ssc.data.SscRequestResult;
import com.android.imsstack.enabler.ssc.data.SscServiceData;
import com.android.imsstack.enabler.ssc.data.SscServiceQueryData;
import com.android.imsstack.util.ImsLog;

import org.w3c.dom.Document;

import java.lang.ref.WeakReference;

public class SscTransaction {
    public static final int EVENT_SRV_RETRY_REQUIRED = 1001;

    protected SscXmlGov mXMLGov = null;

    protected Handler mTransactionHandler = null;
    protected Handler mSscServiceImplHandler = null;

    protected Thread mSscTransactionThread = null;
    protected MmtelTransaction mTransaction = null;

    protected int mEventNumber = 0;
    protected int mTransactionId = 0;
    protected ESsType mSsType = ESsType.NONE;
    protected int mRequestType = 0;

    protected int mTimerId = -1;
    protected int mSlotId = -1;

    public SscTransaction(int slotId, Handler handler) {
        mSlotId = slotId;
        mSscServiceImplHandler = handler;
        mXMLGov = SscXmlGov.getInstance(slotId);
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
        mSsType = data.getSsType();
        mRequestType = SscHttpConnection.HTTP_GET_REQUEST;
        mSscTransactionThread.start();
    }

    public void startPutTransaction(SscServiceData data) {
        ImsLog.d("");

        mSscTransactionThread = new SscTransactionThread(this);
        mTransaction = new PutTransaction(data);
        mEventNumber = data.getEventNumber();
        mTransactionId = data.getTransactionId();
        mSsType = data.getSsType();
        mRequestType = SscHttpConnection.HTTP_PUT_REQUEST;
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

    protected boolean isTrmAvailable() {
        return TRMAgent.getInstance().isServiceAvailable(mSlotId, TRMAgent.SERVICE_UT);
    }

    protected boolean isTrmSupported() {
        return TRMAgent.getInstance().isTRMSupported();
    }

    protected final class SscTransactionThread extends Thread {
        private SscTransaction SscTransaction = null;

        public SscTransactionThread(SscTransaction SscTransaction) {
            super("SscTransactionThread");
            this.SscTransaction = SscTransaction;
        }

        public void run() {
            Looper.prepare();

            ImsLog.d("SscTransactionThread is running ... (" + android.os.Process.myTid() + ")");
            mTransactionHandler = new TransactionHandler(SscTransaction);
            SscNetConnectionGov.getInstance().setCallbackHandler(mSlotId, mTransactionHandler);
            mTransaction.startTransaction();

            Looper.loop();
        }
    }

    protected boolean isSRVRetryRequired(int responseCode) {
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

    abstract private class MmtelTransaction {
        abstract protected String getRequestUri(String xui);
        abstract protected String getXmlBody();
        abstract protected String getPassword();
        abstract protected void processResponse(ISscHttpConnectionGov httpConnection,
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
            String body = getXmlBody();
            if (body == null) {
                ImsLog.e(mSlotId, "Invalid body");
                sendFailMessageToServiceImpl(mEventNumber, mTransactionId);
                return;
            }

            String xui = SscXui.getXUI(mSlotId, getPassword());
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

            ISscHttpConnectionGov httpConnection = SscHttpConnectionGov.getInstance();
            httpConnection.setXuiValue(mSlotId, xui);
            getGbaKey(false);

            int responseCode = httpConnection.sendRequest(mSlotId, mRequestType, requestUri, body);
            ImsLog.i(mSlotId, "response Code : " + responseCode);

            if (responseCode == SscConstant.HTTP_UNAUTHORIZED) {
                if (SscConfig.isGbaSupported(mSlotId)) {
                    if (getGbaKey(true) == false) {
                        SscServiceStateAgent.getInstance().setGbaRequestFailed(mSlotId, true);
                        sendFailMessageToServiceImpl(mEventNumber, mTransactionId);
                        return;
                    }

                    responseCode = httpConnection.sendRequest(mSlotId, mRequestType, requestUri,
                            body);
                    if (responseCode == SscConstant.HTTP_UNAUTHORIZED) { // 401 again
                        ISscAuthAgent authAgent = SscAuthAgent.getInstance(mSlotId);
                        authAgent.setIsCredentialInfoUpdated(false);
                    }
                }
            }

            if (isSRVRetryRequired(responseCode)) {
                ImsLog.d(mSlotId, "SRV Retry Needed");
                if (mTransactionHandler != null) {
                    mTransactionHandler.sendEmptyMessage(EVENT_SRV_RETRY_REQUIRED);
                } else {
                    ImsLog.e(mSlotId, "mTransactionHandler is null");
                    sendFailMessageToServiceImpl(mEventNumber, mTransactionId);
                }
                return;
            }

            if (responseCode == SscHttpConnection.REQUEST_FAILED) {
                ImsLog.e(mSlotId, "Connection failed");
                SscServiceStateAgent.getInstance().setSocketConnectionExpired(mSlotId, true);
                sendFailMessageToServiceImpl(mEventNumber, mTransactionId);
                return;
            }

            processResponse(httpConnection, responseCode);
        }
    }

    protected class GetTransaction extends MmtelTransaction {
        SscServiceQueryData mData = null;

        public GetTransaction(SscServiceQueryData data) {
            this.mData = data;
        }

        @Override
        protected String getRequestUri(String xui) {
            return SscUrl.getInstance().getQueryUri(mData, xui);
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
                SscServiceStateAgent.getInstance().setErrorResponseCode(mSlotId, responseCode);
            }

            Document doc = httpConnection.getInputStream(mSlotId);
            SscServiceData dataFromServer = null;
            if (doc != null) {
                if (mData.getSsType() == ESsType.NONE && responseCode == 200) {
                    mXMLGov.setXmlData(doc);
                }
                dataFromServer = mXMLGov.parseXMLStream(mData, doc);
            }

            if (dataFromServer == null) {
                ImsLog.e(mSlotId, "dataFromServer is null");
                resultState = SscConstant.REQUEST_FAILURE;
            }

            sendMessageToServiceImpl(mEventNumber, mTransactionId, resultState, responseCode,
                    dataFromServer);
        }
    }

    protected class PutTransaction extends MmtelTransaction {
        SscServiceData mData = null;

        public PutTransaction(SscData SscData) {
            this.mData = (SscServiceData) SscData;
        }

        @Override
        protected String getRequestUri(String xui) {
            return SscUrl.getInstance().getUpdateUri(mData, xui);
        }

        @Override
        protected String getXmlBody() {
            return mXMLGov.createXMLStream(mData);
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
                SscServiceStateAgent.getInstance().setErrorResponseCode(mSlotId, responseCode);
            }

            Document doc = httpConnection.getInputStream(mSlotId);
            SscServiceData dataFromServer = null;
            SscXmlGov.getInstance(mSlotId).updateXmlData(responseCode);
            if (resultState == SscConstant.REQUEST_FAILURE && doc != null) {
                dataFromServer = mXMLGov.parseXMLStream(mData, doc);
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
            }

            sendMessageToServiceImpl(mEventNumber, mTransactionId, resultState, responseCode,
                    dataFromServer);
        }
    }

    private boolean getGbaKey(boolean isForced) {
        IGBA gba = (IGBA) AgentFactory.getAgent(AgentFactory.GBA, mSlotId);
        if (gba == null) {
            return false;
        }

        ISscAuthAgent authAgent = SscAuthAgent.getInstance(mSlotId);
        if (authAgent.isCredentialInfoUpdated() == false) {
            return false;
        }

        int appType = SscUtils.getTelephonySimType(mSlotId);
        int gbaMode = SscConfig.getGbaMode(mSlotId);
        boolean isTls = SscConfig.isTls(mSlotId);
        String nafFqdn = authAgent.getNafFqdnFromRealm();
        String securityProtocol = authAgent.getCipherSuite();

        Pair<String, String> keyPair = gba.getGbaKey(appType, gbaMode, isTls, nafFqdn,
                securityProtocol, isForced);
        if (keyPair == null || keyPair.first == null || keyPair.second == null) {
            ImsLog.e(mSlotId, "gba failure");
            authAgent.setIsCredentialInfoUpdated(false);
            return false;
        }

        authAgent.setGbaKeys(keyPair.first, keyPair.second);
        return true;
    }

    public static class TransactionHandler extends Handler {
        private final WeakReference<SscTransaction> mService;

        TransactionHandler(SscTransaction service) {
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
                if (SscServiceStateAgent.getInstance().getAllSrvAddrTried(mSlotId) == true) {
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