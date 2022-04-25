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
import com.android.imsstack.core.agents.agentif.IAlarmTimer;
import com.android.imsstack.core.agents.agentif.IGBA;
import com.android.imsstack.enabler.ssc.data.CbServiceUpdateData;
import com.android.imsstack.enabler.ssc.data.SscData;
import com.android.imsstack.enabler.ssc.data.SscServiceData;
import com.android.imsstack.enabler.ssc.data.SscServiceQueryData;
import com.android.imsstack.enabler.ssc.data.SscRequestResult;
import com.android.imsstack.util.ImsLog;

import java.lang.ref.WeakReference;
import org.w3c.dom.Document;

public class SscTransaction {
    // Constants--------------------------------------------------
    public static final int EVENT_DATA_STATE_CHANGED        = 1001;
    //public static final int EVENT_REQUEST_SUCCESS           = 1002;
    //public static final int EVENT_REQUEST_FAILED            = 1003;

    public static final int EVENT_CONNECTION_TIMER_EXPIRED  = 1101;

    //public static final int EVENT_CONNECTION_FAILED         = 2001;
    //public static final int EVENT_DATA_IS_NOT_VALID         = 2002;
    //public static final int EVENT_AUTHENTICATION_FAILED     = 2003;
    //public static final int EVENT_DB_UPDATE_FAILED          = 2004;
    //public static final int EVENT_START_PUT_TRANSACTION     = 2005;
    //public static final int EVENT_GOT_HTTP_REQUEST_RESPONSE = 2006;
    public static final int EVENT_SRV_RETRY_REQUIRED        = 2007;

    protected static final int CONNECTION_TIMER_VALUE       = 5000;
    protected static final int CONNECTION_RETRY_NUMBER      = 5;

    protected static final int HTTP_GET_REQUEST             = 10000;
    protected static final int HTTP_PUT_REQUEST             = 10001;

    // Variables--------------------------------------------------
    protected SscXmlGov mXMLGov = null;

    protected Handler mTransactionHandler = null;
    protected Handler mSscServiceImplHandler = null;

    protected Thread mSscTransactionThread = null;
    protected MmtelTransaction mTransaction = null;

    protected int mConnectionRetryCounter = 0;
    protected int mEventNumber = 0;
    protected int mTransactionId = 0;
    protected ESsType mSsType = ESsType.NONE;
    protected int mRequestType = 0;

    protected int mTimerId = -1;
    protected int mSlotId = -1;

    // Static loading materials ----------------------------------
    // Public methods --------------------------------------------
    public SscTransaction(int slotId, Handler handler) {
        mSlotId = slotId;
        mSscServiceImplHandler = handler;
        mXMLGov = SscXmlGov.getInstance(slotId);
    }

    public void close() {
        ImsLog.d(mSlotId, "");
        if (mTransactionHandler != null) {
            mTransactionHandler.getLooper().quit();
        }
    }

    public void startGetTransaction(SscServiceQueryData data) {
        ImsLog.d("");

        mSscTransactionThread = new SscTransactionThread(this);
        mTransaction = new GetTransaction(data);
        mEventNumber = data.getEventNumber();
        mTransactionId = data.getTransactionId();
        mSsType = data.getSsType();
        mRequestType = HTTP_GET_REQUEST;
        mConnectionRetryCounter = CONNECTION_RETRY_NUMBER;
        mSscTransactionThread.start();
    }

    public void startPutTransaction(SscServiceData data) {
        ImsLog.d("");

        mSscTransactionThread = new SscTransactionThread(this);
        mTransaction = new PutTransaction(data);
        mEventNumber = data.getEventNumber();
        mTransactionId = data.getTransactionId();
        mSsType = data.getSsType();
        mRequestType = HTTP_PUT_REQUEST;
        mConnectionRetryCounter = CONNECTION_RETRY_NUMBER;
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
            ISscNetConnectionGov netConnectionGov = SscNetConnectionGov.getInstance();
            ISscHttpConnectionGov httpConnection = SscHttpConnectionGov.getInstance();

            // TODO : null checking???
            netConnectionGov.setTransactionHandler(mSlotId, mTransactionHandler);
            httpConnection.setTransactionHandler(mSlotId, mTransactionHandler);
            mTransaction.startTransaction();

            Looper.loop();
        }
    }

    protected boolean isSRVRetryRequired(int responseCode) {
        if (SscConfig.isSrvRecordsRequired(mSlotId) == false) {
            return false;
        }
        if (responseCode == SscConstant.HTTP_NOT_MODIFIED ||
                responseCode == SscConstant.HTTP_PRECONDITION_FAILURE) {
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
                if (netConnectionGov.connect(mSlotId)) {
                    startConnectionTimer(CONNECTION_TIMER_VALUE, mSsType);
                } else {
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
                        SscServiceStateAgent.getInstance().setGBARequestFailed(mSlotId, true);
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

            if ((responseCode != SscConstant.HTTP_NOT_MODIFIED)
                    && (responseCode < 200 || responseCode >= 300)) {
                ImsLog.d(mSlotId, "not received 200 OK");
                resultState = SscConstant.REQUEST_FAILURE;
                SscServiceStateAgent.getInstance().setErrorResponseCode(mSlotId, responseCode);
            }

            Document doc = httpConnection.getInputStream(mSlotId);
            SscServiceData dataFromServer = null;
            if (doc != null || responseCode == SscConstant.HTTP_NOT_MODIFIED) {
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
        if (keyPair == null) {
            ImsLog.e(mSlotId, "gba failure");
            authAgent.setIsCredentialInfoUpdated(false);
            return false;
        }

        authAgent.setGbaKeys(keyPair.first, keyPair.second);
        return true;
    }

    // Timer for waiting pdn connection
    protected void startConnectionTimer(int duration, ESsType ssType) {
        IAlarmTimer atm = (IAlarmTimer)AgentFactory.getAgent(AgentFactory.ALARM_TIMER, mSlotId);
        if (atm == null) {
            ImsLog.e("AlamTimerManager is null");
            return;
        }

        mTimerId = atm.getTimerId();
        if (mTimerId <= 0) {
            ImsLog.e("Validity timer id is invalid");
            return;
        }

        atm.registerForTimerExpired(mTimerId, mTransactionHandler, EVENT_CONNECTION_TIMER_EXPIRED,
                (Object)ssType);

        if (!atm.startTimer(mTimerId, duration)) {
            stopConnectionTimer(false);
            ImsLog.d("Starting a validity timer failed");
            return;
        }

        ImsLog.i("Validity timer is started :: slotId/tid=" + mSlotId + "/" + mTimerId
                + ", duration=" + duration);
    }

    protected void stopConnectionTimer(boolean stopRequired) {
        if (mTimerId <= 0) {
            return;
        }

        IAlarmTimer atm = (IAlarmTimer)AgentFactory.getAgent(AgentFactory.ALARM_TIMER, mSlotId);
        if (atm == null) {
            ImsLog.e("AlamTimerManager is null");
            return;
        }

        if (stopRequired) {
            atm.stopTimer(mTimerId);
        }

        atm.unregisterForTimerExpired(mTimerId, mTransactionHandler);

        mTimerId = (-1);
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
        case EVENT_DATA_STATE_CHANGED:
            ImsLog.d("Data state has changed");
            stopConnectionTimer(true);
//            restart connection timer
//            startConnectionTimer(CONNECTION_TIMER_VALUE);
            if (mSscTransactionThread == null) {
                ImsLog.e("mSscTransactionThread is null");
                return;
            }

            // All request retry counter is consumed,
            // then request triggered by UI only can be reset the retry counter value.
//            mConnectionRetryCounter = 2;

            mTransaction.startTransaction();
            break;

        case EVENT_CONNECTION_TIMER_EXPIRED:
            ImsLog.d("Connection Timer Expired");
            ImsLog.w("mConnectionRetryCounter : " + mConnectionRetryCounter);

            if (SscServiceStateAgent.getInstance().getPdnConnectionFailed(mSlotId)) {
                ImsLog.d("XCAP PDN connection failed - stop transaction");

                stopConnectionTimer(true);

                sendFailMessageToServiceImpl(mEventNumber, mTransactionId);
            } else if (mConnectionRetryCounter > 0) {
                mConnectionRetryCounter--;
                mTransaction.startTransaction();
            } else {
                ISscNetConnectionGov netConnectionGov = SscNetConnectionGov.getInstance();
                if (netConnectionGov != null) {
                    netConnectionGov.disconnect(mSlotId);
                }
                stopConnectionTimer(true);
                SscServiceStateAgent.getInstance().setPDNConnectionTimerExpired(mSlotId, true);
                sendFailMessageToServiceImpl(mEventNumber, mTransactionId);
            }
            break;
/*
        case EVENT_GOT_HTTP_REQUEST_RESPONSE :
            ImsLog.d("GOT_HTTP_REQUEST_RESPONSE");
            String strResponseMsg = (String) msg.obj;
            ImsLog.d("strResponseMsg : " + strResponseMsg);
            break;
*/
        case EVENT_SRV_RETRY_REQUIRED :
            /* TODO: check when implementing NAPTR/SRV
            if (SscServiceStateAgent.getInstance().getAllSRVAddrTried(mSlotId) == true) {
                sendFailMessageToServiceImpl(mEventNumber, mTransactionId);
            } else {
                SscAuthAgent.getInstance(mSlotId).setIsCredentialInfoUpdated(false);
                mTransaction.startTransaction();
            }
             */
            break;

        default:
            ImsLog.e("Unhanddled Message :" + msg.what);
            break;
        }
    }
}