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

import android.content.Context;
import android.os.Handler;
import android.os.Looper;
import android.os.Message;
import android.os.Process;
import android.telephony.ims.ImsCallForwardInfo;
import android.telephony.ims.ImsReasonInfo;
import android.telephony.ims.ImsSsInfo;
import android.text.TextUtils;

import com.android.imsstack.core.agents.AgentFactory;
import com.android.imsstack.core.agents.SubsInfoInterface;
import com.android.imsstack.core.agents.agentif.ISubscription;
import com.android.imsstack.core.agents.agentif.SubscriptionListener;
import com.android.imsstack.core.agents.dcmif.EApnType;
import com.android.imsstack.enabler.ssc.data.CbServiceData;;
import com.android.imsstack.enabler.ssc.data.CbServiceQueryData;
import com.android.imsstack.enabler.ssc.data.CbServiceUpdateData;
import com.android.imsstack.enabler.ssc.data.CfServiceData;;
import com.android.imsstack.enabler.ssc.data.CfServiceQueryData;
import com.android.imsstack.enabler.ssc.data.CfServiceUpdateData;
import com.android.imsstack.enabler.ssc.data.CwServiceData;
import com.android.imsstack.enabler.ssc.data.ErrorResponseData;
import com.android.imsstack.enabler.ssc.data.OipServiceData;
import com.android.imsstack.enabler.ssc.data.OirServiceData;
import com.android.imsstack.enabler.ssc.data.SscData;
import com.android.imsstack.enabler.ssc.data.SscRequestData;
import com.android.imsstack.enabler.ssc.data.SscRequestResult;
import com.android.imsstack.enabler.ssc.data.SscRuleData;
import com.android.imsstack.enabler.ssc.data.SscRuleElement;
import com.android.imsstack.enabler.ssc.data.SscServiceData;
import com.android.imsstack.enabler.ssc.data.SscServiceQueryData;
import com.android.imsstack.enabler.ssc.data.TipServiceData;
import com.android.imsstack.enabler.ssc.data.TirServiceData;
import com.android.imsstack.imsservice.mmtel.ut.base.UtInterfaceBase;
import com.android.imsstack.util.ImsLog;
import com.android.imsstack.util.MessageExecutor;
import com.android.internal.annotations.VisibleForTesting;

import java.util.ArrayList;
import java.util.concurrent.ConcurrentLinkedDeque;

/*
public class SscServiceImpl extends IImsUt.Stub implements Closeable
*/
public class SscServiceImpl extends UtInterfaceBase {
    private static final int EVENT_UT_TRANSACTION_STARTED = 1001;
    private static final int EVENT_UT_INITIALIZE_MODULES = 1002;

    private static final int REQUEST_TYPE_QUERY = 0;
    private static final int REQUEST_TYPE_UPDATE = 1;

    private Context mContext = null;
    private SscTransactionFactory mSscTransactionFactory = null;
    private SscTransaction mSscTransaction = null;

    private SscServiceThread mSscServiceThread = null;
    private SscRequestHandler mSscRequestHandler = null;
    private SscCallbackHandler mSscCallbackHandler = null;

    private ConcurrentLinkedDeque<SscRequestData> mSscRequestQueue = null;

    private Object lock = new Object();

    private int mSlotId = -1;

    public SscServiceImpl(int slotId) {
        mSlotId = slotId;
        SscConfig.init(mSlotId);
        mSscRequestQueue = new ConcurrentLinkedDeque<SscRequestData>();
        setSscTransactionFactory(new SscTransactionFactory());
    }

    @Override
    public boolean isUtAvailable() {
        return SscServiceStateAgent.getInstance().isUtAvailable(mSlotId);
    }

    @Override
    public void start(Context context) {
        ImsLog.d(mSlotId, "");

        mContext = context;
        if (mContext == null) {
            ImsLog.e("Context is null");
            return;
        }

        if (SscConfig.isUtSupported(mSlotId) == false) {
            ImsLog.w("XCAP/Ut is disabled");
            return;
        }

        setNetworkType();

        mSscServiceThread = new SscServiceThread();
        mSscServiceThread.start();

        // To Avoid race condition we are making current thread to wait
        // till all handlers are Initialized
        try {
            synchronized (lock) {
                ImsLog.d("Current thread is blocked::startService");
                lock.wait(3000);
                ImsLog.d("Current thread lock is released::startService");
            }
        } catch (InterruptedException ie) {
            ie.printStackTrace();
        }

        //Register for DDS Change Event
        ISubscription subscription
                = (ISubscription)AgentFactory.getAgent(AgentFactory.SUBSCRIPTION, mSlotId);
        if (subscription != null) {
            subscription.addListener(mDataSubListener);
        }
    }

    @Override
    public void close() {
        ImsLog.d(mSlotId, "");

        SscConfig.clear(mSlotId);
        SscXmlGov.getInstance(mSlotId).clear();

        ISubscription subscription
                = (ISubscription)AgentFactory.getAgent(AgentFactory.SUBSCRIPTION, mSlotId);
        if (subscription != null) {
            subscription.removeListener(mDataSubListener);
        }

        SscServiceStateAgent.getInstance().deInit(mSlotId);

        if (mSscTransaction != null) {
            mSscTransaction.close();
            mSscTransaction = null;
        }

        if (mSscRequestHandler != null) {
            mSscRequestHandler.getLooper().quit();
        }

        mSscRequestQueue.clear();
    }

    private SubscriptionListener mDataSubListener = new SubscriptionListener() {
        public void onDefaultDataSubscriptionChanged(int subId) {
            ISubscription isub
                    = (ISubscription)AgentFactory.getAgent(AgentFactory.SUBSCRIPTION, mSlotId);
            ImsLog.i("onDefaultDataSubscriptionChanged :: subId=" + subId);

            if (isub == null) {
                ImsLog.i("onDefaultDataSubscriptionChanged :: isub is null");
                return;
            }

            SscAuthAgent.getInstance(mSlotId).setIsCredentialInfoUpdated(false);
        }
    };

    private void setNetworkType() {
        ISscNetConnectionGov netConnGov = SscNetConnectionGov.getInstance();
        ISscHttpConnectionGov httpConnectionGov = SscHttpConnectionGov.getInstance();

        SubsInfoInterface subsInfo = AgentFactory.getInstance().getAgent(
                SubsInfoInterface.class, mSlotId);
        String pdntype = SscConfig.getUtPdnType(mSlotId);
        ImsLog.d("pdntype = " + pdntype);
        if (pdntype != null && pdntype.equals("mobile_internet")) {
            netConnGov.init(mSlotId, EApnType.INTERNET);
            httpConnectionGov.open(mSlotId, EApnType.INTERNET);
        } else {
            netConnGov.init(mSlotId, EApnType.XCAP);
            httpConnectionGov.open(mSlotId, EApnType.XCAP);
        }
    }

    private void handleInvalidRequest(int tId, int requestType) {
        if (mUtListener != null) {
            ImsReasonInfo ri = new ImsReasonInfo(ImsReasonInfo.CODE_UT_OPERATION_NOT_ALLOWED,
                    ImsReasonInfo.CODE_UNSPECIFIED, null);
            if (requestType == REQUEST_TYPE_QUERY) {
                postAndRunTask(new Runnable() {
                        @Override
                        public void run() {
                            mUtListener.utConfigurationQueryFailed(tId, ri);
                        }
                });
            } else if (requestType == REQUEST_TYPE_UPDATE) {
                postAndRunTask(new Runnable() {
                        @Override
                        public void run() {
                            mUtListener.utConfigurationUpdateFailed(tId, ri);
                        }
                });
            }
        }
    }

    private void postAndRunTask(Runnable task) {
        if (mSscCallbackHandler != null) {
            MessageExecutor messageExecutor = new MessageExecutor(mSscCallbackHandler.getLooper());
            messageExecutor.execute(task);
        }
    }

    @VisibleForTesting
    public void setSscTransactionFactory(SscTransactionFactory transactionFactory) {
        mSscTransactionFactory = transactionFactory;
    }

    @VisibleForTesting
    public SscRequestHandler getRequestHandler() {
        return mSscRequestHandler;
    }

    @VisibleForTesting
    public SscCallbackHandler getCallBackHandler() {
        return mSscCallbackHandler;
    }

    @Override
    public void queryCallBarring(int tId, int condition) {
        queryCallBarringForServiceClass(tId, condition, SscServiceClassUtil.SERVICE_CLASS_NONE);
    }

    @Override
    public void queryCallBarringForServiceClass(int tId, int condition, int serviceClass) {
        // Check valid service class or not
        boolean isValid = SscServiceClassUtil.isValid(serviceClass);
        if (isValid == false) {
            ImsLog.e(mSlotId, "Invalid serviceclass " + serviceClass);
            handleInvalidRequest(tId, REQUEST_TYPE_QUERY);
            return;
        }

        // remove service classes except voice and video
        serviceClass = SscServiceClassUtil.removeInvalidServiceClass(serviceClass);

        SscRequestData requestData = new SscRequestData(tId);

        if (SscXmlGov.getInstance(mSlotId).isXmlDataPresent() == false) {
            requestData.offerSscData(new SscServiceQueryData(mSlotId, ESsType.NONE,
                    SscConstant.EVENT_SSC_QUERY_DOCUMENT, tId, 0));
        }

        switch(condition) {
            case SscConstant.CONDITION_BAOC:
            case SscConstant.CONDITION_BOIC:
            case SscConstant.CONDITION_BOIC_EXHC:
                requestData.offerSscData(new CbServiceQueryData(mSlotId, ESsType.OCB,
                        SscConstant.EVENT_SSC_QUERY_CB, tId, condition, serviceClass));
                break;
            case SscConstant.CONDITION_BAIC:
            case SscConstant.CONDITION_BIC_WR:
            case SscConstant.CONDITION_ACR:
                requestData.offerSscData(new CbServiceQueryData(mSlotId, ESsType.ICB,
                        SscConstant.EVENT_SSC_QUERY_CB, tId, condition, serviceClass));
                break;
            default:
                ImsLog.e(mSlotId, "Invalid CB condition : " + condition);
                handleInvalidRequest(tId, REQUEST_TYPE_QUERY);
                return;
        }

        postRequestMessage(requestData);
    }

    @Override
    public void queryCallForward(int tId, int condition, String number) {
        queryCallForwardForServiceClass(tId, condition, number,
                SscServiceClassUtil.SERVICE_CLASS_NONE);
    }

    //@Override
    private void queryCallForwardForServiceClass(int tId, int condition, String number,
            int serviceClass) {
        if (condition == SscConstant.CONDITION_CFA || condition == SscConstant.CONDITION_CFAC) {
            if (SscConfig.isCfQueryAllAndCfAllConditionalSupported(mSlotId) == false) {
                ImsLog.d(mSlotId, "isCfQueryAllAndCfAllConditionalSupported is false");
                handleInvalidRequest(tId, REQUEST_TYPE_QUERY);
                return;
            }
        }

        // Check valid service class or not
        boolean isValid = SscServiceClassUtil.isValid(serviceClass);
        if (isValid == false) {
            ImsLog.e(mSlotId, "Invalid serviceClass " + serviceClass);
            handleInvalidRequest(tId, REQUEST_TYPE_QUERY);
            return;
        }

        // remove service classes except voice and video
        serviceClass = SscServiceClassUtil.removeInvalidServiceClass(serviceClass);

        SscRequestData requestData = new SscRequestData(tId);

        if (SscXmlGov.getInstance(mSlotId).isXmlDataPresent() == false) {
            requestData.offerSscData(new SscServiceQueryData(mSlotId, ESsType.NONE,
                    SscConstant.EVENT_SSC_QUERY_DOCUMENT, tId, 0));
        }

        switch (condition) {
            case SscConstant.CONDITION_CFU:
            case SscConstant.CONDITION_CFB:
            case SscConstant.CONDITION_CFNR:
            case SscConstant.CONDITION_CFNRC:
            case SscConstant.CONDITION_CFA:
            case SscConstant.CONDITION_CFAC:
            case SscConstant.CONDITION_CFNL:
                requestData.offerSscData(new CfServiceQueryData(mSlotId, ESsType.CF,
                        SscConstant.EVENT_SSC_QUERY_CF, tId, condition, number, serviceClass));
                break;
            default :
                ImsLog.e(mSlotId, "Invalid CF condition : " + condition);
                handleInvalidRequest(tId, REQUEST_TYPE_QUERY);
                return;
        }

        postRequestMessage(requestData);
    }

    @Override
    public void queryCallWaiting(int tId) {
        SscRequestData requestData = new SscRequestData(tId);

        if (SscXmlGov.getInstance(mSlotId).isXmlDataPresent() == false) {
            requestData.offerSscData(new SscServiceQueryData(mSlotId, ESsType.NONE,
                    SscConstant.EVENT_SSC_QUERY_DOCUMENT, tId, 0));
        }

        requestData.offerSscData(new SscServiceQueryData(mSlotId, ESsType.CW,
                SscConstant.EVENT_SSC_QUERY_CW, tId, -1));

        postRequestMessage(requestData);
    }

    @Override
    public void queryCLIR(int tId) {
        SscRequestData requestData = new SscRequestData(tId);

        if (SscXmlGov.getInstance(mSlotId).isXmlDataPresent() == false) {
            requestData.offerSscData(new SscServiceQueryData(mSlotId, ESsType.NONE,
                    SscConstant.EVENT_SSC_QUERY_DOCUMENT, tId, 0));
        }

        requestData.offerSscData(new SscServiceQueryData(mSlotId, ESsType.OIR,
                SscConstant.EVENT_SSC_QUERY_OIR, tId, -1));

        postRequestMessage(requestData);
    }

    @Override
    public void queryCLIP(int tId) {
        SscRequestData requestData = new SscRequestData(tId);

        if (SscXmlGov.getInstance(mSlotId).isXmlDataPresent() == false) {
            requestData.offerSscData(new SscServiceQueryData(mSlotId, ESsType.NONE,
                    SscConstant.EVENT_SSC_QUERY_DOCUMENT, tId, 0));
        }

        requestData.offerSscData(new SscServiceQueryData(mSlotId, ESsType.OIP,
                SscConstant.EVENT_SSC_QUERY_OIP, tId, -1));

        postRequestMessage(requestData);
    }

    @Override
    public void queryCOLR(int tId) {
        SscRequestData requestData = new SscRequestData(tId);

        if (SscXmlGov.getInstance(mSlotId).isXmlDataPresent() == false) {
            requestData.offerSscData(new SscServiceQueryData(mSlotId, ESsType.NONE,
                    SscConstant.EVENT_SSC_QUERY_DOCUMENT, tId, 0));
        }

        requestData.offerSscData(new SscServiceQueryData(mSlotId, ESsType.TIR,
                SscConstant.EVENT_SSC_QUERY_TIR, tId, -1));

        postRequestMessage(requestData);
    }

    @Override
    public void queryCOLP(int tId) {
        SscRequestData requestData = new SscRequestData(tId);

        if (SscXmlGov.getInstance(mSlotId).isXmlDataPresent() == false) {
            requestData.offerSscData(new SscServiceQueryData(mSlotId, ESsType.NONE,
                    SscConstant.EVENT_SSC_QUERY_DOCUMENT, tId, 0));
        }

        requestData.offerSscData(new SscServiceQueryData(mSlotId, ESsType.TIP,
                SscConstant.EVENT_SSC_QUERY_TIP, tId, -1));

        postRequestMessage(requestData);
    }

    @Override
    public void updateCallBarring(int tId, int condition, int action, String[] barringList) {
        updateCallBarringWithPassword(tId, condition, action, barringList,
                SscServiceClassUtil.SERVICE_CLASS_NONE, null);
    }

    @Override
    public void updateCallBarringForServiceClass(int tId, int condition, int action,
            String[] barringList, int serviceClass) {
        updateCallBarringWithPassword(tId, condition, action, barringList, serviceClass, null);
    }

    @Override
    public void updateCallBarringWithPassword(int tId, int condition, int action,
            String[] barringList, int serviceClass, String password) {
        if (action != SscConstant.STATUS_ENABLE && action != SscConstant.STATUS_DISABLE) {
            ImsLog.e(mSlotId, "Invalid action : " + action);
            handleInvalidRequest(tId, REQUEST_TYPE_UPDATE);
            return;
        }

        // Check valid service class or not
        boolean isValid = SscServiceClassUtil.isValid(serviceClass);
        if (isValid == false) {
            ImsLog.e(mSlotId, "Invalid serviceClass: " + serviceClass);
            handleInvalidRequest(tId, REQUEST_TYPE_UPDATE);
            return;
        }

        // remove service classes except voice and video
        serviceClass = SscServiceClassUtil.removeInvalidServiceClass(serviceClass);

        SscRequestData requestData = new SscRequestData(tId);

        if (SscXmlGov.getInstance(mSlotId).isXmlDataPresent() == false) {
            requestData.offerSscData(new SscServiceQueryData(mSlotId, ESsType.NONE,
                    SscConstant.EVENT_SSC_QUERY_DOCUMENT, tId, 0));
        }

        switch (condition) {
            case SscConstant.CONDITION_BAOC :
            case SscConstant.CONDITION_BOIC :
            case SscConstant.CONDITION_BOIC_EXHC :
                requestData.offerSscData(new CbServiceUpdateData(mSlotId, ESsType.OCB,
                        SscConstant.EVENT_SSC_UPDATE_CB,  tId, action, condition, barringList,
                        serviceClass, password));
                break;
            case SscConstant.CONDITION_BAIC :
            case SscConstant.CONDITION_BIC_WR :
                requestData.offerSscData(new CbServiceUpdateData(mSlotId, ESsType.ICB,
                        SscConstant.EVENT_SSC_UPDATE_CB, tId, action, condition, barringList,
                        serviceClass, password));
                break;
            default:
                ImsLog.e(mSlotId, "Invalid  condition : " + condition);
                handleInvalidRequest(tId, REQUEST_TYPE_UPDATE);
                return;
        }

        postRequestMessage(requestData);
    }

    @Override
    public void updateCallForward(int tId, int action, int condition, String number,
            int serviceClass, int timeSeconds) {
        if (action == SscConstant.ACTION_ERASURE) {
            if (SscConfig.isCfActionErasureSupported(mSlotId) == false) {
                ImsLog.e(mSlotId, "isCfActionErasureSupported is false");
                handleInvalidRequest(tId, REQUEST_TYPE_UPDATE);
                return;
            }
        }

        switch (action) {
            case SscConstant.ACTION_ACTIVATION:
            case SscConstant.ACTION_REGISTRATION:
                if (TextUtils.isEmpty(number)) {
                    action = SscConstant.ACTION_ACTIVATION;
                } else {
                    action = SscConstant.ACTION_REGISTRATION;
                }
                break;
            case SscConstant.ACTION_DEACTIVATION:
            case SscConstant.ACTION_ERASURE:
                // ignore cfnr timer when deactivation or erasure
                timeSeconds = 0;
                break;
            default:
                ImsLog.e(mSlotId, "Invalid action");
                handleInvalidRequest(tId, REQUEST_TYPE_UPDATE);
                return;
        }

        if (timeSeconds > 0) {
            if (timeSeconds < SscConstant.CFNR_TIMER_MIN
                    || timeSeconds > SscConstant.CFNR_TIMER_MAX) {
                ImsLog.e(mSlotId, "Invalid timer : " + timeSeconds);
                handleInvalidRequest(tId, REQUEST_TYPE_UPDATE);
                return;
            }
        }

        // Check valid service class or not
        boolean isValid = SscServiceClassUtil.isValid(serviceClass);
        if (isValid == false) {
            ImsLog.e(mSlotId, "Invalid serviceClass " + serviceClass);
            handleInvalidRequest(tId, REQUEST_TYPE_UPDATE);
            return;
        }

        // remove service classes except voice and video
        serviceClass = SscServiceClassUtil.removeInvalidServiceClass(serviceClass);

        SscRequestData requestData = new SscRequestData(tId);

        if (SscXmlGov.getInstance(mSlotId).isXmlDataPresent() == false) {
            requestData.offerSscData(new SscServiceQueryData(mSlotId, ESsType.NONE,
                    SscConstant.EVENT_SSC_QUERY_DOCUMENT, tId, 0));
        }

        switch (condition) {
            case SscConstant.CONDITION_CFU:
            case SscConstant.CONDITION_CFB:
            case SscConstant.CONDITION_CFNR:
            case SscConstant.CONDITION_CFNRC:
            case SscConstant.CONDITION_CFNL:
                requestData.offerSscData(new CfServiceUpdateData(mSlotId, ESsType.CF,
                        SscConstant.EVENT_SSC_UPDATE_CF, tId, action, condition, number,
                        timeSeconds, serviceClass));
                break;
            case SscConstant.CONDITION_CFA:
            case SscConstant.CONDITION_CFAC:
                for (int i = SscConstant.CONDITION_CFNRC; i > SscConstant.CONDITION_CFU; i--) {
                    requestData.offerSscData(new CfServiceUpdateData(
                            mSlotId, ESsType.CF, SscConstant.EVENT_SSC_UPDATE_CF, tId, action, i,
                            number, timeSeconds, serviceClass));
                }
                if (condition == SscConstant.CONDITION_CFA) {
                    requestData.offerSscData(new CfServiceUpdateData(mSlotId, ESsType.CF,
                            SscConstant.EVENT_SSC_UPDATE_CF, tId, action,
                            SscConstant.CONDITION_CFU, number, timeSeconds, serviceClass));
                }
                break;
            default:
                ImsLog.e(mSlotId, "Invalid Condition : " + condition);
                handleInvalidRequest(tId, REQUEST_TYPE_UPDATE);
                return;
        }

        postRequestMessage(requestData);
    }

    @Override
    public void updateCallWaiting(int tId, boolean enable, int serviceClass) {
        SscRequestData requestData = new SscRequestData(tId);

        if (SscXmlGov.getInstance(mSlotId).isXmlDataPresent() == false) {
            requestData.offerSscData(new SscServiceQueryData(mSlotId, ESsType.NONE,
                    SscConstant.EVENT_SSC_QUERY_DOCUMENT, tId, 0));
        }

        requestData.offerSscData(new CwServiceData(mSlotId, ESsType.CW,
                SscConstant.EVENT_SSC_UPDATE_CW, tId, (enable ? 1 : 0)));

        postRequestMessage(requestData);
    }

    @Override
    public void updateCLIR(int tId, int clirMode) {
        SscRequestData requestData = new SscRequestData(tId);

        if (SscXmlGov.getInstance(mSlotId).isXmlDataPresent() == false) {
            requestData.offerSscData(new SscServiceQueryData(mSlotId, ESsType.NONE,
                    SscConstant.EVENT_SSC_QUERY_DOCUMENT, tId, 0));
        }

        requestData.offerSscData(new OirServiceData(mSlotId, ESsType.OIR,
                SscConstant.EVENT_SSC_UPDATE_OIR, tId, clirMode, clirMode, clirMode));

        postRequestMessage(requestData);
    }

    @Override
    public void updateCLIP(int tId, boolean enable) {
        SscRequestData requestData = new SscRequestData(tId);

        if (SscXmlGov.getInstance(mSlotId).isXmlDataPresent() == false) {
            requestData.offerSscData(new SscServiceQueryData(mSlotId, ESsType.NONE,
                    SscConstant.EVENT_SSC_QUERY_DOCUMENT, tId, 0));
        }

        requestData.offerSscData(new OipServiceData(mSlotId, ESsType.OIP,
                SscConstant.EVENT_SSC_UPDATE_OIP, tId, (enable ? 1 : 0)));

        postRequestMessage(requestData);
    }

    @Override
    public void updateCOLR(int tId, int presentation) {
        SscRequestData requestData = new SscRequestData(tId);

        if (SscXmlGov.getInstance(mSlotId).isXmlDataPresent() == false) {
            requestData.offerSscData(new SscServiceQueryData(mSlotId, ESsType.NONE,
                    SscConstant.EVENT_SSC_QUERY_DOCUMENT, tId, 0));
        }

        requestData.offerSscData(new TirServiceData(mSlotId, ESsType.TIR,
                SscConstant.EVENT_SSC_UPDATE_TIR, tId, presentation, presentation));

        postRequestMessage(requestData);
    }

    @Override
    public void updateCOLP(int tId, boolean enable) {
        SscRequestData requestData = new SscRequestData(tId);

        if (SscXmlGov.getInstance(mSlotId).isXmlDataPresent() == false) {
            requestData.offerSscData(new SscServiceQueryData(mSlotId, ESsType.NONE,
                    SscConstant.EVENT_SSC_QUERY_DOCUMENT, tId, 0));
        }

        requestData.offerSscData(new TipServiceData(mSlotId, ESsType.TIP,
                SscConstant.EVENT_SSC_UPDATE_TIP, tId, (enable ? 1 : 0)));

        postRequestMessage(requestData);
    }

    private void startTransaction(SscData data) {
        ImsLog.d("");
        if (data instanceof SscServiceQueryData) { // Query Case
            mSscTransaction = mSscTransactionFactory.getSscTransaction(data.getSlotId(),
                    mSscCallbackHandler);
            mSscTransaction.startGetTransaction((SscServiceQueryData)data);
        } else if (data instanceof SscServiceData) { // Update case
            mSscTransaction = mSscTransactionFactory.getSscTransaction(data.getSlotId(),
                    mSscCallbackHandler);
            mSscTransaction.startPutTransaction((SscServiceData)data);
        } else {
            ImsLog.e(mSlotId, "Invalid Request Type");
            postFailResponseMessage(data);
        }
    }

    private void processQueueData() {
        SscRequestData requestData = mSscRequestQueue.peekFirst();
        if (requestData == null) {
            return;
        }

        SscData sscData = requestData.peakSscData();
        if (sscData == null) {
            mSscRequestQueue.pollFirst();
            processQueueData();
            return;
        }

        if (isUtAvailable() == false) {
            ImsLog.w(mSlotId, "Clear pending data due to Ut is not available");
            postFailResponseMessage(sscData);
        } else if (sscData.getSsType() != ESsType.NONE
                && !SscXmlGov.getInstance(mSlotId).isXmlDataPresent()) {
            ImsLog.w(mSlotId, "Clear pending data due to entire query failed");
            postFailResponseMessage(sscData);
        } else {
            adjustEvent(sscData);

            Message msg = Message.obtain(mSscRequestHandler, EVENT_UT_TRANSACTION_STARTED, sscData);
            mSscRequestHandler.sendMessage(msg);
        }
    }

    private void postRequestMessage(SscRequestData requestData) {
        mSscRequestQueue.offerLast(requestData);
        if (mSscRequestQueue.size() > 1) {
            ImsLog.d(mSlotId, "Queueing...");
            return;
        }

        processQueueData();
    }

    private void postFailResponseMessage(SscData requestData) {
        SscRequestResult rr = new SscRequestResult(requestData.getSlotId(),
                requestData.getTransactionId(), SscConstant.REQUEST_FAILURE, 0, -1);

        Message msg = Message.obtain(mSscCallbackHandler, requestData.getEventNumber(), rr);
        mSscCallbackHandler.sendMessage(msg);
    }

    /**
     * This method adjusts event number of SscData to comply with following requirement according to
     * KEY_UT_INSERT_NEW_RULE_BOOL.
     * For versions earlier than IR.92 v9.0, the UE must insert a new <rule> element with a rule
     * ID different from any existing rule ID in the XML document.
     * For IR.92 v10 and later, the UE must consider that the supplementary service is
     * not provisioned for the user and must not insert a new <rule> element with a rule ID
     * different from any existing rule ID in the XML document.
     * @param sscData event number is updated to process insert operation
     */
    private void adjustEvent(SscData sscData) {
        if (!SscConfig.insertNewRule(sscData.getSlotId())) {
            return;
        }

        if (sscData.getEventNumber() != SscConstant.EVENT_SSC_UPDATE_CB
                && sscData.getEventNumber() != SscConstant.EVENT_SSC_UPDATE_CF) {
            return;
        }

        SscServiceData data = (SscServiceData) sscData;
        if (sscData.getEventNumber() == SscConstant.EVENT_SSC_UPDATE_CF
                && data.getCondition() == SscConstant.CONDITION_CFNR_TIMER) {
            return;
        }

        int mediaType = (data.getServiceClass() == SscServiceClassUtil.SERVICE_CLASS_DATA)
                ? SscXmlFormat.MEDIA_TYPE_VIDEO : SscXmlFormat.MEDIA_TYPE_AUDIO;
        String ruleId = SscXmlFormat.getRuleId(data.getSlotId(), mediaType,
                data.getSsType().getSsName(), data.getCondition());
        if (ruleId != null) {
            return;
        }

        if (sscData.getEventNumber() == SscConstant.EVENT_SSC_UPDATE_CB) {
            ImsLog.w(mSlotId, "Need to insert new rule ID for CB");
            sscData.setEventNumber(SscConstant.EVENT_SSC_INSERT_CB);
        } else if (sscData.getEventNumber() == SscConstant.EVENT_SSC_UPDATE_CF) {
            ImsLog.w(mSlotId, "Need to insert new rule ID for CF");
            sscData.setEventNumber(SscConstant.EVENT_SSC_INSERT_CF);
        }
    }

    private final class SscRequestHandler extends Handler {
        private SscRequestHandler(Looper looper) {
            super(looper);
        }

        @Override
        public void handleMessage(Message msg) {
            if (msg == null) {
                ImsLog.e(mSlotId, "Message is null");
                return;
            }

            ImsLog.d(mSlotId, "Message : " + msg.what);
            switch(msg.what) {
                case EVENT_UT_TRANSACTION_STARTED: {
                    SscData requestData = (SscData)msg.obj;
                    if (requestData != null) {
                         // before starting transaction, set flag as false.
                         //SscDnsQuery.getInstance().setNAFFailed(false);
                         startTransaction(requestData);
                    }
                    break;
                }
                case EVENT_UT_INITIALIZE_MODULES: {
                    SscServiceStateAgent.getInstance().init(mSlotId);

                    // Release the Lock afer the module Initialization
                    // to avoid initializers malfunction
                    synchronized (lock) {
                        lock.notifyAll();
                        ImsLog.d(mSlotId, "Lock is released from handleMessage::SscRequestHandler");
                    }
                    break;
                }
                default:
                    ImsLog.w(mSlotId, "Invalid Message");
                    break;
            }
        }
    }

    private final class SscCallbackHandler extends Handler {
        private SscCallbackHandler(Looper looper) {
            super(looper);
        }

        @Override
        public void handleMessage(Message msg) {
            if (msg == null) {
                ImsLog.e(mSlotId, "Message is null");
                return;
            }

            ImsLog.d(mSlotId, "Message : " + msg.what);
            SscRequestResult rr = (SscRequestResult) msg.obj;
            if (rr == null) {
                ImsLog.e(mSlotId, "SscRequestResult is null");
                return;
            }

            SscRequestData requestData = mSscRequestQueue.peekFirst();
            if (requestData == null) {
                return;
            }

            int resultState = rr.getResultState();
            if (resultState == SscConstant.REQUEST_FAILURE) {
                if (rr.getCode() == SscConstant.HTTP_PRECONDITION_FAILURE) {
                    if (handlePreconditionFailure(requestData) == true) {
                        ImsLog.d(mSlotId, "handlePreconditionFailure");
                        return;
                    }
                }

                if (handleRetryWhenFailure(requestData) == true) {
                    ImsLog.d(mSlotId, "Need to retry. retryCount = " + requestData.getRetryCount());
                    return;
                }
            }

            if (resultState == SscConstant.REQUEST_SUCCESS) {
                if (handleAdditionalRequestWhenSuccess(requestData) == true) {
                    ImsLog.d(mSlotId, "Need to send additional request");
                    return;
                }
            }

            int eventNum = msg.what;
            if (eventNum == SscConstant.EVENT_SSC_QUERY_DOCUMENT) {
                if (resultState == SscConstant.REQUEST_FAILURE) {
                    // set actual event requested
                    requestData.pollSscData();
                    SscData SscData = requestData.peakSscData();
                    if (SscData != null) {
                        eventNum = SscData.getEventNumber();
                    }
                }
            }

            notifyRequestResult(eventNum, rr.getTransactionId(), resultState,
                    rr.getSscServiceData());

            if (mSscTransaction != null) {
                mSscTransaction.close();
                mSscTransaction = null;
            }

            mSscRequestQueue.pollFirst();
            processQueueData();
        }

        private boolean handlePreconditionFailure(SscRequestData requestData) {
            if (requestData.getPreconditionFailedCount() > 0) {
                return false;
            }

            requestData.increasePreconditionFailedCount();
            requestData.offerSscDataFirst(new SscServiceQueryData(mSlotId, ESsType.NONE,
                    SscConstant.EVENT_SSC_QUERY_DOCUMENT, requestData.getTransactionId(), 0));
            processQueueData();
            return true;
        }

        private boolean handleRetryWhenFailure(SscRequestData requestData) {
            if (requestData.getRetryCount() >= SscConfig.getMaxRetryCount(mSlotId)) {
                return false;
            }

            requestData.increaseRetryCount();
            processQueueData();
            return true;
        }

        private boolean handleAdditionalRequestWhenSuccess(SscRequestData requestData) {
            SscData previousRequest = requestData.pollSscData();
            if (previousRequest instanceof CfServiceUpdateData) {
                CfServiceUpdateData cfUpdateData = (CfServiceUpdateData) previousRequest;
                if (cfUpdateData.getCondition() == SscConstant.CONDITION_CFNRC) {
                    if (SscXmlFormat.getCfnlExist(mSlotId)) {
                        requestData.offerSscDataFirst(new CfServiceUpdateData(mSlotId, ESsType.CF,
                                SscConstant.EVENT_SSC_UPDATE_CF, cfUpdateData.getTransactionId(),
                                cfUpdateData.getState(), SscConstant.CONDITION_CFNL,
                                cfUpdateData.getForwardToNumber(), cfUpdateData.getReplyTimer(),
                                cfUpdateData.getServiceClass()));
                    }
                } else if (cfUpdateData.getCondition() == SscConstant.CONDITION_CFNR) {
                    if (SscXmlFormat.getIsNoReplyTimerInRule(mSlotId) == false &&
                            cfUpdateData.getReplyTimer() > 0) {
                        requestData.offerSscDataFirst(new CfServiceUpdateData(mSlotId, ESsType.CF,
                                SscConstant.EVENT_SSC_UPDATE_CF, cfUpdateData.getTransactionId(),
                                cfUpdateData.getState(), SscConstant.CONDITION_CFNR_TIMER,
                                cfUpdateData.getForwardToNumber(), cfUpdateData.getReplyTimer(),
                                cfUpdateData.getServiceClass()));
                    }
                }
            }

            SscData nextRequest = requestData.peakSscData();
            if (nextRequest == null) {
                return false;
            }

            processQueueData();
            return true;
        }

        private void notifyRequestResult(int eventNum, int transactionId, int state,
                SscServiceData data) {
            ImsLog.d(mSlotId, "eventNum : " + eventNum + ", tId : " + transactionId);
            try {
                switch (eventNum) {
                    case SscConstant.EVENT_SSC_QUERY_DOCUMENT:
                        // do nothing
                        break;
                    case SscConstant.EVENT_SSC_QUERY_CB:
                    case SscConstant.EVENT_SSC_QUERY_CF:
                    case SscConstant.EVENT_SSC_QUERY_CW:
                    case SscConstant.EVENT_SSC_QUERY_OIR:
                    case SscConstant.EVENT_SSC_QUERY_OIP:
                    case SscConstant.EVENT_SSC_QUERY_TIR:
                    case SscConstant.EVENT_SSC_QUERY_TIP:
                        if (state == SscConstant.REQUEST_FAILURE) {
                            onConfigurationQueryFailed(transactionId, data);
                        } else if (state == SscConstant.REQUEST_SUCCESS) {
                            onConfigurationQueried(transactionId, data);
                        }
                        break;
                    case SscConstant.EVENT_SSC_UPDATE_CB:
                    case SscConstant.EVENT_SSC_UPDATE_CF:
                    case SscConstant.EVENT_SSC_UPDATE_CW:
                    case SscConstant.EVENT_SSC_UPDATE_OIR:
                    case SscConstant.EVENT_SSC_UPDATE_OIP:
                    case SscConstant.EVENT_SSC_UPDATE_TIR:
                    case SscConstant.EVENT_SSC_UPDATE_TIP:
                    case SscConstant.EVENT_SSC_INSERT_CB:
                    case SscConstant.EVENT_SSC_INSERT_CF:
                        if (state == SscConstant.REQUEST_FAILURE) {
                            onConfigurationUpdateFailed(transactionId, data);
                        } else if (state == SscConstant.REQUEST_SUCCESS) {
                            onConfigurationUpdated(transactionId);
                        }
                        break;
                    default:
                        ImsLog.e(mSlotId, "Invalid Message");
                        break;
                }
            } catch (Exception e) {
                ImsLog.e(e.toString());
                e.printStackTrace();
            }
        }

        private void onConfigurationUpdated(final int id) {
            if (mUtListener == null) {
                ImsLog.d(mSlotId, "UtListener is null");
                return;
            }

            mUtListener.utConfigurationUpdated(id);
        }

        private void onConfigurationUpdateFailed(final int id, final SscServiceData data) {
            if (mUtListener == null) {
                ImsLog.d(mSlotId, "UtListener is null");
                return;
            }

            ImsReasonInfo ri = createImsReasonInfo(data);
            mUtListener.utConfigurationUpdateFailed(id, ri);
        }

        /*
        private Bundle getUtConfigurationQueryICBBundle(SscServiceData data) {
            ImsLog.d("");

            Bundle bundle = new Bundle();
            if (data == null) {
                ImsLog.e("SscServiceData is null");
                return bundle;
            }

            ICBAServiceData cbData = (ICBAServiceData)data;
            ArrayList<SscRuleDataICB> ruleSet = cbData.getRuleSet();
            if (ruleSet == null) {
                ImsLog.e("ruleSet is null");
                return bundle;
            }

            int ruleSetSize = ruleSet.size();
            if (ruleSetSize == 0) {
                ImsLog.e("Ruleset length is null");
                return bundle;
            }

            ImsLog.d("Ruleset length: " + ruleSetSize);
            ImsIcbInfo ssInfo[] = new ImsIcbInfo[ruleSetSize];

            int type = 0;
            int state = 0;
            String condition = null;
            String ruleId = null;

            for (int i = 0; i < ruleSetSize; i++) {
                SscRuleDataICB ruleSetICB = ruleSet.get(i);

                type = ruleSetICB.getICBType();
                state = ruleSetICB.getRuleState();
                if (ruleSet.get(i).getICBType() == SscRuleDataICB.TYPE_ANONYMOUS) {
                    condition = "anonymous";
                    ruleId = "anonymous-call-rejection";
                }
                else {
                    condition = ruleSetICB.getOneId();
                    ruleId = ruleSetICB.getRuleId();
                }

                ssInfo[i] = new ImsIcbInfo(type, state, condition, ruleId);
            }

            bundle.putParcelableArray(ImsIcbInfo.class.getSimpleName(), ssInfo);

            return bundle;
        }
        */

        private void onConfigurationQueried(final int id, SscServiceData data) {
            if (mUtListener == null) {
                ImsLog.d(mSlotId, "UtListener is null");
                return;
            }

            if (data == null) {
                ImsLog.e(mSlotId, "SscServiceData is null");
                return;
            }

            switch (data.getSsType()) {
                case OCB:
                case ICB:
                    final ImsSsInfo[] cbInfo = createCallBarringInfo(data);
                    mUtListener.utConfigurationCallBarringQueried(id, cbInfo);
                    break;
                case CF:
                    final ImsCallForwardInfo[] cfInfo = createCallForwardInfo(data);
                    mUtListener.utConfigurationCallForwardQueried(id, cfInfo);
                    break;
                case CW:
                    final ImsSsInfo[] cwInfo = createCallWaitingInfo(data);
                    mUtListener.utConfigurationCallWaitingQueried(id, cwInfo);
                    break;
                case OIR:
                case OIP:
                case TIR:
                case TIP:
                    final ImsSsInfo ssInfo = createLineIdentificationInfo(data);
                    mUtListener.lineIdentificationSupplementaryServiceResponse(id, ssInfo);
                    break;
                case ICBA:
                    /*
                    final ImsIcbInfo[] icbInfo = createExtendCallBarringInfo(data);
                    Bundle bundle = new Bundle();
                    bundle.putParcelableArray("ImsIcbInfo", icbInfo);
                    mUtListener.utConfigurationQueried(id, bundle);
                     */
                    break;
                default:
                    ImsLog.e(mSlotId, "Invalid SscServiceData");
                    return;
            }
        }

        private void onConfigurationQueryFailed(final int id, final SscServiceData data) {
            if (mUtListener == null) {
                ImsLog.d(mSlotId, "UtListener is null");
                return;
            }

            ImsReasonInfo ri = createImsReasonInfo(data);
            mUtListener.utConfigurationQueryFailed(id, ri);
        }

        private ImsSsInfo[] createCallBarringInfo(SscServiceData data) {
            ImsLog.d(mSlotId, "");

            if (data == null) {
                ImsLog.e(mSlotId, "SscServiceData is null");
                return null;
            }

            if (data.getSsType() != ESsType.OCB && data.getSsType() != ESsType.ICB) {
                ImsLog.e(mSlotId, "Invalid SsType");
                return null;
            }

            CbServiceData cbData = (CbServiceData)data;
            if (cbData.getRuleSet() == null || cbData.getRuleSet().size() <= 0) {
                ImsLog.e(mSlotId, "CB ruleset is null or empty");
                ImsSsInfo cbInfo[] = new ImsSsInfo[1];

                // No RuleSet case : status_disable
                cbInfo[0] = new ImsSsInfo.Builder(SscConstant.STATUS_DISABLE).build();
                return cbInfo;
            }

            int ruleSetSize = cbData.getRuleSet().size();
            ImsSsInfo cbInfo[] = new ImsSsInfo[ruleSetSize];
            for (int i = 0; i < ruleSetSize; i++) {
                SscRuleData ruleData = cbData.getRuleSet().get(i);
                cbInfo[i] = new ImsSsInfo.Builder(ruleData.getState()).build();
                ImsLog.d(cbInfo[i].toString());
            }

            return cbInfo;
        }
/*
        private ImsIcbInfo[] createExtendCallBarringInfo(SscServiceData data) {
            ImsLog.d("");

            if (data == null) {
                ImsLog.e("SscServiceData is null !!!");
                return null;
            }

            if (data.getSsType() != ESsType.ICBA) {
                ImsLog.e("Invalid SStype");
                return null;
            }

            CbServiceData icbData = (CbServiceData)data;
            if (icbData.getRuleSet() == null) {
                ImsLog.e("CB ruleset is null !!!");
                ImsIcbInfo icbInfo[] = new ImsIcbInfo[1];

                // No RuleSet case
                //icbInfo[0] = new ImsIcbInfo()
                icbInfo[0] = new ImsIcbInfo(0, 0, null, null);
                // Disabled
                //icbInfo[0].mStatus = 0;
                return icbInfo;
            }

            int ruleSetSize = icbData.getRuleSet().size();
            if (ruleSetSize <= 0) {
                ImsLog.e("CallBarring Data is null !!!");
                return null;
            }

            ImsIcbInfo icbInfo[] = new ImsIcbInfo[ruleSetSize];

            // 1. Setting RuleSet
            for (int i = 0; i < ruleSetSize; i++) {
                //icbInfo[i] = new ImsIcbInfo();
                icbInfo[i] = new ImsIcbInfo(0, 0, null, null);
                SscRuleData ruleData = icbData.getRuleSet().get(i);
                setIncomingCallBarringInfo(icbInfo[i], ruleData);
            }

            return icbInfo;
        }
*/
/*
        private void setIncomingCallBarringInfo(ImsIcbInfo ssInfo, SscRuleData ruleData) {
            if (ssInfo != null && ruleData != null) {
                //ssInfo.state = ruleData.getState();
                //ImsLog.d("ICB.mStatus : " + ssInfo.mStatus);
            }
            else {
                ImsLog.e("ImsIcbInfo and SscRuleData are null");
            }
        }
*/

        private ImsCallForwardInfo[] createCallForwardInfo(SscServiceData data) {
            ImsLog.d(mSlotId, "");

            if (data.getSsType() != ESsType.CF) {
                ImsLog.e(mSlotId, "Invalid SStype");
                return null;
            }

            CfServiceData cfData = (CfServiceData)data;
            if (cfData.getRuleSet() == null || cfData.getRuleSet().size() <= 0) {
                ImsLog.e(mSlotId, "CF ruleset is null or empty");
                ImsCallForwardInfo cfInfo[] = new ImsCallForwardInfo[1];
                // No RuleSet case
                cfInfo[0] = new ImsCallForwardInfo(cfData.getCondition(),
                        SscConstant.STATUS_DISABLE, ImsCallForwardInfo.TYPE_OF_ADDRESS_UNKNOWN,
                        SscServiceClassUtil.SERVICE_CLASS_NONE, "", 0);

                return cfInfo;
            }

            int ruleSetSize = cfData.getRuleSet().size();
            ImsCallForwardInfo cfInfo[] = new ImsCallForwardInfo[ruleSetSize];
            for (int i = 0; i < ruleSetSize; i++) {
                SscRuleData ruleData = cfData.getRuleSet().get(i);
                int reason = ruleData.getSsCondition();
                int status = ruleData.getState();
                int serviceClass = ruleData.getServiceClass();

                String number = ruleData.getForwardToNumber();
                if (TextUtils.isEmpty(number)) {
                    number = "";
                    // IR92 - in case of empty target, consider CF is disabled
                    status = SscConstant.STATUS_DISABLE;
                }

                int toA = ImsCallForwardInfo.TYPE_OF_ADDRESS_UNKNOWN;
                if (number.startsWith("+")) {
                    toA = ImsCallForwardInfo.TYPE_OF_ADDRESS_INTERNATIONAL;
                }

                int noplyTimerSec = -1;
                if (ruleData.getSsCondition() == SscConstant.CONDITION_CFNR) {
                    noplyTimerSec = cfData.getNoReplyTimer();
                }

                cfInfo[i] = new ImsCallForwardInfo(reason, status, toA, serviceClass, number,
                        noplyTimerSec);
                ImsLog.d(cfInfo[i].toString());
            }

            return cfInfo;
        }

        private String getValueOfElement(String key, ArrayList<SscRuleElement> elementList) {
            if (key == null || elementList == null) {
                ImsLog.e(mSlotId, "key or elementList is null");
                return null;
            }

            for (int i = 0; i < elementList.size(); i++) {
                SscRuleElement element = elementList.get(i);
                if (element.getKey().endsWith(key)) {
                    return element.getValue();
                }
            }

            return null;
        }

        private ImsSsInfo[] createCallWaitingInfo(SscServiceData data) {
            if (data.getSsType() != ESsType.CW) {
                ImsLog.e(mSlotId, "Invalid SStype");
                return null;
            }

            ImsSsInfo[] ssInfo = new ImsSsInfo[1];
            ssInfo[0] = new ImsSsInfo.Builder(data.getState()).build();

            return ssInfo;
        }

        private ImsSsInfo createLineIdentificationInfo(SscServiceData data) {
            if (data.getSsType() == ESsType.OIR) {
                OirServiceData oirData = (OirServiceData)data;
                return new ImsSsInfo.Builder(oirData.getState())
                        .setClirInterrogationStatus(oirData.getProvisionStatus()) // m
                        .setClirOutgoingState(oirData.getOutgoingState()).build(); // n
            } else if (data.getSsType() == ESsType.TIR) {
                TirServiceData tirData = (TirServiceData)data;
                return new ImsSsInfo.Builder(tirData.getState())
                        .setProvisionStatus(tirData.getProvisionStatus()).build(); // m
            }

            return new ImsSsInfo.Builder(data.getState()).build();
        }

        private ImsReasonInfo createImsReasonInfo(SscServiceData data) {
            int reasonCode = ImsReasonInfo.CODE_UT_NETWORK_ERROR;
            if (SscConfig.isCsfbSupported(mSlotId)) {
                reasonCode = ImsReasonInfo.CODE_LOCAL_CALL_CS_RETRY_REQUIRED;
            }

            String errorPhrase = null;
            if (SscConfig.isErrorPhraseDisplayedWith409(mSlotId)) {
                if (data != null) {
                    ErrorResponseData errData = (ErrorResponseData) data;
                    if (errData.getErrorCode() == SscConstant.HTTP_CONFLICT) {
                        errorPhrase = errData.getErrorPhrase();
                    }
                }
            }

            ImsLog.d(mSlotId, "reasonCode = " + reasonCode + ", errorPhrase = " + errorPhrase);

            return new ImsReasonInfo(reasonCode, ImsReasonInfo.CODE_UNSPECIFIED, errorPhrase);
        }
    }

    private final class SscServiceThread extends Thread {
        public SscServiceThread() {
            super("SSCService");
        }

        public void run() {
            Looper.prepare();

            ImsLog.d(mSlotId, "SscServiceThread is running ... (" + Process.myTid() + ")");

            mSscRequestHandler = new SscRequestHandler(Looper.myLooper());
            mSscCallbackHandler = new SscCallbackHandler(Looper.myLooper());
            mSscRequestHandler.sendEmptyMessage(EVENT_UT_INITIALIZE_MODULES);

            Looper.loop();
        }
    }
}