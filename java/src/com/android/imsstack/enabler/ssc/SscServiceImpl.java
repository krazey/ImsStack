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

import android.content.Context;
import android.os.Handler;
import android.os.HandlerThread;
import android.os.Looper;
import android.os.Message;
import android.telephony.ims.ImsCallForwardInfo;
import android.telephony.ims.ImsReasonInfo;
import android.telephony.ims.ImsSsInfo;
import android.text.TextUtils;

import com.android.imsstack.base.ImsPrivateProperties;
import com.android.imsstack.core.agents.AgentFactory;
import com.android.imsstack.core.agents.PhoneStateInterface;
import com.android.imsstack.core.agents.dcmif.EApnType;
import com.android.imsstack.enabler.ssc.data.CbServiceData;
import com.android.imsstack.enabler.ssc.data.CbServiceQueryData;
import com.android.imsstack.enabler.ssc.data.CbServiceUpdateData;
import com.android.imsstack.enabler.ssc.data.CfServiceData;
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
import com.android.imsstack.enabler.ssc.data.SscServiceData;
import com.android.imsstack.enabler.ssc.data.SscServiceQueryData;
import com.android.imsstack.enabler.ssc.data.TipServiceData;
import com.android.imsstack.enabler.ssc.data.TirServiceData;
import com.android.imsstack.imsservice.mmtel.ut.base.IUtInterface;
import com.android.imsstack.imsservice.mmtel.ut.base.IUtListener;
import com.android.imsstack.imsservice.mmtel.ut.base.IUtServiceStateListener;
import com.android.imsstack.util.ImsLog;
import com.android.imsstack.util.MessageExecutor;
import com.android.internal.annotations.VisibleForTesting;

import java.util.List;
import java.util.concurrent.ConcurrentLinkedDeque;

/**
 * Implementation of IUtInterface class that provides query and update APIs for supplementary
 * service configuration over Ut reference point using XCAP
 */
public class SscServiceImpl implements IUtInterface {
    private static final int EVENT_UT_TRANSACTION_STARTED = 1001;
    private static final int REQUEST_TYPE_QUERY = 0;
    private static final int REQUEST_TYPE_UPDATE = 1;

    private final int mSlotId;
    private final ConcurrentLinkedDeque<SscRequestData> mSscRequestQueue;

    private Context mContext = null;
    private IUtListener mUtListener = null;
    private IUtServiceStateListener mUtServiceStateListener = null;
    private SscPreferenceHelper mSscPreferenceHelper = null;
    private SscTransactionFactory mSscTransactionFactory = null;
    private SscTransaction mSscTransaction = null;

    private HandlerThread mSscServiceThread = null;
    private SscRequestHandler mSscRequestHandler = null;
    private SscCallbackHandler mSscCallbackHandler = null;

    public SscServiceImpl(int slotId) {
        mSlotId = slotId;
        mSscRequestQueue = new ConcurrentLinkedDeque<SscRequestData>();
        setSscPreferenceHelper(new SscPreferenceHelper(mSlotId));
        setSscTransactionFactory(new SscTransactionFactory());
    }

    @Override
    public boolean isUtAvailable() {
        if (SscConfig.isUtSupported(mSlotId)
                && isTerminalBasedService(ESsType.OIR, SscConstant.CONDITION_INVALID)) {
            ImsLog.d(mSlotId, "TB SS is enabled");
            return true;
        }

        return SscServiceStateAgent.getInstance().isUtAvailable(mSlotId);
    }

    @Override
    public void changeCapabilities(List<CapabilityPair> enabledCaps,
            List<CapabilityPair> disabledCaps) {
        SscServiceStateAgent.getInstance().changeCapabilities(mSlotId, enabledCaps, disabledCaps);
    }

    @Override
    public void start(Context context) {
        ImsLog.d(mSlotId, "");

        mContext = context;
        if (mContext == null) {
            ImsLog.e("Context is null");
            return;
        }

        SscConfig.init(mSlotId);
        SscXmlGov.getInstance(mSlotId).init();

        initConnections();

        mSscServiceThread = new HandlerThread("SscServiceImplThread");
        mSscServiceThread.start();

        mSscRequestHandler = new SscRequestHandler(mSscServiceThread.getLooper());
        mSscCallbackHandler = new SscCallbackHandler(mSscServiceThread.getLooper());
        SscServiceStateAgent.getInstance().init(mSlotId, mSscServiceThread.getLooper());
    }

    @Override
    public void setListener(IUtListener listener) {
        mUtListener = listener;
    }

    @Override
    public void setServiceStateListener(IUtServiceStateListener listener) {
        mUtServiceStateListener = listener;
    }

    @Override
    public void onServiceStateChanged() {
        if (mUtServiceStateListener != null) {
            mUtServiceStateListener.onServiceStateChanged();
        }
    }

    @Override
    public void close() {
        ImsLog.d(mSlotId, "");

        clearConnections();
        SscConfig.clear(mSlotId);
        SscXmlGov.getInstance(mSlotId).clear();
        SscServiceStateAgent.getInstance().deInit(mSlotId);

        if (mSscTransaction != null) {
            mSscTransaction.close();
            mSscTransaction = null;
        }

        if (mSscServiceThread != null) {
            mSscServiceThread.quit();
            mSscServiceThread = null;
        }

        mSscRequestQueue.clear();
    }

    private void initConnections() {
        ISscNetConnectionGov netConnGov = SscNetConnectionGov.getInstance();
        ISscHttpConnectionGov httpConnectionGov = SscHttpConnectionGov.getInstance();

        EApnType apnType = EApnType.XCAP;
        if (ImsPrivateProperties.Persistent.getInt(
                ImsPrivateProperties.Persistent.KEY_WIFI_TEST, mSlotId) == 1) {
            apnType = EApnType.WIFI;
        }

        netConnGov.init(mSlotId, apnType);
        httpConnectionGov.open(mSlotId, apnType);
    }

    private void clearConnections() {
        ISscNetConnectionGov netConnGov = SscNetConnectionGov.getInstance();
        netConnGov.cleanup(mSlotId);

        ISscHttpConnectionGov httpConnectionGov = SscHttpConnectionGov.getInstance();
        httpConnectionGov.close(mSlotId);
    }

    private void handleInvalidRequest(int tId, int requestType) {
        if (mUtListener != null) {
            ImsReasonInfo ri = new ImsReasonInfo(ImsReasonInfo.CODE_UT_OPERATION_NOT_ALLOWED,
                    ImsReasonInfo.CODE_UNSPECIFIED, null);
            if (requestType == REQUEST_TYPE_QUERY) {
                postAndRunTask(() -> mUtListener.utConfigurationQueryFailed(tId, ri));
            } else if (requestType == REQUEST_TYPE_UPDATE) {
                postAndRunTask(() -> mUtListener.utConfigurationUpdateFailed(tId, ri));
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
    protected void setSscPreferenceHelper(SscPreferenceHelper preferenceHelper) {
        mSscPreferenceHelper = preferenceHelper;
    }

    @VisibleForTesting
    protected void setSscTransactionFactory(SscTransactionFactory transactionFactory) {
        mSscTransactionFactory = transactionFactory;
    }

    @VisibleForTesting
    protected HandlerThread getServiceHandlerThread() {
        return mSscServiceThread;
    }

    @VisibleForTesting
    protected SscCallbackHandler getCallBackHandler() {
        return mSscCallbackHandler;
    }

    @Override
    public void queryCallBarring(int tId, int condition) {
        queryCallBarringForServiceClass(tId, condition, SscServiceClassUtil.SERVICE_CLASS_NONE);
    }

    @Override
    public void queryCallBarringForServiceClass(int tId, int condition, int serviceClass) {
        final ESsType ssType;

        if (condition == SscConstant.CONDITION_BAOC || condition == SscConstant.CONDITION_BOIC
                || condition == SscConstant.CONDITION_BOIC_EXHC) {
            ssType = ESsType.OCB;
        } else {
            // SscConstant.CONDITION_BAIC, SscConstant.CONDITION_BIC_WR, SscConstant.CONDITION_ACR
            ssType = ESsType.ICB;
        }

        if (!isServerBasedService(ssType, condition)) {
            ImsLog.e(mSlotId, "Invalid service request, condition : " + condition);
            handleInvalidRequest(tId, REQUEST_TYPE_QUERY);
            return;
        }

        SscRequestData requestData = new SscRequestData(tId);

        if (!SscXmlGov.getInstance(mSlotId).isXmlDataPresent()) {
            requestData.offerSscData(new SscServiceQueryData(mSlotId, ESsType.NONE,
                    SscConstant.EVENT_SSC_QUERY_DOCUMENT, tId, 0));
        }

        requestData.offerSscData(new CbServiceQueryData(mSlotId, ssType,
                SscConstant.EVENT_SSC_QUERY_CB, tId, condition, serviceClass));

        addRequestToQueue(requestData);
    }

    @Override
    public void queryCallForward(int tId, int condition, String number) {
        queryCallForwardForServiceClass(tId, condition, number,
                SscServiceClassUtil.SERVICE_CLASS_VOICE);
    }

    //@Override
    private void queryCallForwardForServiceClass(int tId, int condition, String number,
            int serviceClass) {
        if (!isServerBasedService(ESsType.CF, condition)) {
            ImsLog.e(mSlotId, "Invalid service request, condition : " + condition);
            handleInvalidRequest(tId, REQUEST_TYPE_QUERY);
            return;
        }

        if (condition == SscConstant.CONDITION_CFA || condition == SscConstant.CONDITION_CFAC) {
            if (!SscConfig.isCfQueryAllAndCfAllConditionalSupported(mSlotId)) {
                ImsLog.d(mSlotId, "isCfQueryAllAndCfAllConditionalSupported is false");
                handleInvalidRequest(tId, REQUEST_TYPE_QUERY);
                return;
            }
        }

        SscRequestData requestData = new SscRequestData(tId);

        if (!SscXmlGov.getInstance(mSlotId).isXmlDataPresent()) {
            requestData.offerSscData(new SscServiceQueryData(mSlotId, ESsType.NONE,
                    SscConstant.EVENT_SSC_QUERY_DOCUMENT, tId, 0));
        }

        requestData.offerSscData(new CfServiceQueryData(mSlotId, ESsType.CF,
                SscConstant.EVENT_SSC_QUERY_CF, tId, condition, number, serviceClass));

        addRequestToQueue(requestData);
    }

    @Override
    public void queryCallWaiting(int tId) {
        if (!isServerBasedService(ESsType.CW, SscConstant.CONDITION_INVALID)) {
            ImsLog.e(mSlotId, "Invalid service request");
            handleInvalidRequest(tId, REQUEST_TYPE_QUERY);
            return;
        }

        SscRequestData requestData = new SscRequestData(tId);

        if (!SscXmlGov.getInstance(mSlotId).isXmlDataPresent()) {
            requestData.offerSscData(new SscServiceQueryData(mSlotId, ESsType.NONE,
                    SscConstant.EVENT_SSC_QUERY_DOCUMENT, tId, 0));
        }

        requestData.offerSscData(new SscServiceQueryData(mSlotId, ESsType.CW,
                SscConstant.EVENT_SSC_QUERY_CW, tId, -1));

        addRequestToQueue(requestData);
    }

    @Override
    public void queryCLIR(int tId) {
        if (isTerminalBasedService(ESsType.OIR, SscConstant.CONDITION_INVALID)) {
            ImsLog.d(mSlotId, "TB OIR request");
            handleQueryClirTb(tId);
            return;
        }

        if (!isServerBasedService(ESsType.OIR, SscConstant.CONDITION_INVALID)) {
            ImsLog.e(mSlotId, "Invalid service request");
            handleInvalidRequest(tId, REQUEST_TYPE_QUERY);
            return;
        }

        SscRequestData requestData = new SscRequestData(tId);

        if (!SscXmlGov.getInstance(mSlotId).isXmlDataPresent()) {
            requestData.offerSscData(new SscServiceQueryData(mSlotId, ESsType.NONE,
                    SscConstant.EVENT_SSC_QUERY_DOCUMENT, tId, 0));
        }

        requestData.offerSscData(new SscServiceQueryData(mSlotId, ESsType.OIR,
                SscConstant.EVENT_SSC_QUERY_OIR, tId, -1));

        addRequestToQueue(requestData);
    }

    @Override
    public void queryCLIP(int tId) {
        if (!isServerBasedService(ESsType.OIP, SscConstant.CONDITION_INVALID)) {
            ImsLog.e(mSlotId, "Invalid service request");
            handleInvalidRequest(tId, REQUEST_TYPE_QUERY);
            return;
        }

        SscRequestData requestData = new SscRequestData(tId);

        if (!SscXmlGov.getInstance(mSlotId).isXmlDataPresent()) {
            requestData.offerSscData(new SscServiceQueryData(mSlotId, ESsType.NONE,
                    SscConstant.EVENT_SSC_QUERY_DOCUMENT, tId, 0));
        }

        requestData.offerSscData(new SscServiceQueryData(mSlotId, ESsType.OIP,
                SscConstant.EVENT_SSC_QUERY_OIP, tId, -1));

        addRequestToQueue(requestData);
    }

    @Override
    public void queryCOLR(int tId) {
        if (!isServerBasedService(ESsType.TIR, SscConstant.CONDITION_INVALID)) {
            ImsLog.e(mSlotId, "Invalid service request");
            handleInvalidRequest(tId, REQUEST_TYPE_QUERY);
            return;
        }

        SscRequestData requestData = new SscRequestData(tId);

        if (!SscXmlGov.getInstance(mSlotId).isXmlDataPresent()) {
            requestData.offerSscData(new SscServiceQueryData(mSlotId, ESsType.NONE,
                    SscConstant.EVENT_SSC_QUERY_DOCUMENT, tId, 0));
        }

        requestData.offerSscData(new SscServiceQueryData(mSlotId, ESsType.TIR,
                SscConstant.EVENT_SSC_QUERY_TIR, tId, -1));

        addRequestToQueue(requestData);
    }

    @Override
    public void queryCOLP(int tId) {
        if (!isServerBasedService(ESsType.TIP, SscConstant.CONDITION_INVALID)) {
            ImsLog.e(mSlotId, "Invalid service request");
            handleInvalidRequest(tId, REQUEST_TYPE_QUERY);
            return;
        }

        SscRequestData requestData = new SscRequestData(tId);

        if (!SscXmlGov.getInstance(mSlotId).isXmlDataPresent()) {
            requestData.offerSscData(new SscServiceQueryData(mSlotId, ESsType.NONE,
                    SscConstant.EVENT_SSC_QUERY_DOCUMENT, tId, 0));
        }

        requestData.offerSscData(new SscServiceQueryData(mSlotId, ESsType.TIP,
                SscConstant.EVENT_SSC_QUERY_TIP, tId, -1));

        addRequestToQueue(requestData);
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
        final ESsType ssType;

        if (condition == SscConstant.CONDITION_BAOC || condition == SscConstant.CONDITION_BOIC
                || condition == SscConstant.CONDITION_BOIC_EXHC) {
            ssType = ESsType.OCB;
        } else {
            // SscConstant.CONDITION_BAIC, SscConstant.CONDITION_BIC_WR, SscConstant.CONDITION_ACR
            ssType = ESsType.ICB;
        }

        if (!isServerBasedService(ssType, condition)) {
            ImsLog.e(mSlotId, "Invalid service request, condition : " + condition);
            handleInvalidRequest(tId, REQUEST_TYPE_UPDATE);
            return;
        }

        if (action != SscConstant.STATUS_ENABLE && action != SscConstant.STATUS_DISABLE) {
            ImsLog.e(mSlotId, "Invalid action : " + action);
            handleInvalidRequest(tId, REQUEST_TYPE_UPDATE);
            return;
        }

        SscRequestData requestData = new SscRequestData(tId);

        if (!SscXmlGov.getInstance(mSlotId).isXmlDataPresent()) {
            requestData.offerSscData(new SscServiceQueryData(mSlotId, ESsType.NONE,
                    SscConstant.EVENT_SSC_QUERY_DOCUMENT, tId, 0));
        }

        requestData.offerSscData(new CbServiceUpdateData(mSlotId, ssType,
                SscConstant.EVENT_SSC_UPDATE_CB,  tId, action, condition, barringList,
                serviceClass, password));

        addRequestToQueue(requestData);
    }

    @Override
    public void updateCallForward(int tId, int action, int condition, String number,
            int serviceClass, int timeSeconds) {
        if (!isServerBasedService(ESsType.CF, condition)) {
            ImsLog.e(mSlotId, "Invalid service request, condition : " + condition);
            handleInvalidRequest(tId, REQUEST_TYPE_UPDATE);
            return;
        }

        if (action == SscConstant.ACTION_ERASURE) {
            if (!SscConfig.isCfActionErasureSupported(mSlotId)) {
                ImsLog.d(mSlotId, "isCfActionErasureSupported is false");
                action = SscConstant.ACTION_DEACTIVATION;
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

            if (!SscConfig.isCfnrTimerSupported(mSlotId)) {
                ImsLog.d(mSlotId, "CFNR Timer is not supported");
                timeSeconds = 0;
            }
        }

        SscRequestData requestData = new SscRequestData(tId);

        if (!SscXmlGov.getInstance(mSlotId).isXmlDataPresent()) {
            requestData.offerSscData(new SscServiceQueryData(mSlotId, ESsType.NONE,
                    SscConstant.EVENT_SSC_QUERY_DOCUMENT, tId, 0));
        }

        if (condition == SscConstant.CONDITION_CFU || condition == SscConstant.CONDITION_CFB
                || condition == SscConstant.CONDITION_CFNR
                || condition == SscConstant.CONDITION_CFNRC
                || condition == SscConstant.CONDITION_CFNL) {
            requestData.offerSscData(new CfServiceUpdateData(mSlotId, ESsType.CF,
                    SscConstant.EVENT_SSC_UPDATE_CF, tId, action, condition, number,
                    timeSeconds, serviceClass));
        } else { // SscConstant.CONDITION_CFA, SscConstant.CONDITION_CFAC
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
        }

        addRequestToQueue(requestData);
    }

    @Override
    public void updateCallWaiting(int tId, boolean enable, int serviceClass) {
        if (!isServerBasedService(ESsType.CW, SscConstant.CONDITION_INVALID)) {
            handleInvalidRequest(tId, REQUEST_TYPE_UPDATE);
            return;
        }

        SscRequestData requestData = new SscRequestData(tId);

        if (!SscXmlGov.getInstance(mSlotId).isXmlDataPresent()) {
            requestData.offerSscData(new SscServiceQueryData(mSlotId, ESsType.NONE,
                    SscConstant.EVENT_SSC_QUERY_DOCUMENT, tId, 0));
        }

        requestData.offerSscData(new CwServiceData(mSlotId, ESsType.CW,
                SscConstant.EVENT_SSC_UPDATE_CW, tId, (enable ? 1 : 0)));

        addRequestToQueue(requestData);
    }

    @Override
    public void updateCLIR(int tId, int clirMode) {
        if (isTerminalBasedService(ESsType.OIR, SscConstant.CONDITION_INVALID)) {
            ImsLog.d(mSlotId, "TB OIR request");
            handleUpdateClirTb(tId, clirMode);
            return;
        }

        if (!isServerBasedService(ESsType.OIR, SscConstant.CONDITION_INVALID)) {
            ImsLog.e(mSlotId, "Invalid service request");
            handleInvalidRequest(tId, REQUEST_TYPE_UPDATE);
            return;
        }

        SscRequestData requestData = new SscRequestData(tId);

        if (!SscXmlGov.getInstance(mSlotId).isXmlDataPresent()) {
            requestData.offerSscData(new SscServiceQueryData(mSlotId, ESsType.NONE,
                    SscConstant.EVENT_SSC_QUERY_DOCUMENT, tId, 0));
        }

        requestData.offerSscData(new OirServiceData(mSlotId, ESsType.OIR,
                SscConstant.EVENT_SSC_UPDATE_OIR, tId, clirMode, 0, 0));

        addRequestToQueue(requestData);
    }

    @Override
    public void updateCLIP(int tId, boolean enable) {
        if (!isServerBasedService(ESsType.OIP, SscConstant.CONDITION_INVALID)) {
            ImsLog.e(mSlotId, "Invalid service request");
            handleInvalidRequest(tId, REQUEST_TYPE_UPDATE);
            return;
        }

        SscRequestData requestData = new SscRequestData(tId);

        if (!SscXmlGov.getInstance(mSlotId).isXmlDataPresent()) {
            requestData.offerSscData(new SscServiceQueryData(mSlotId, ESsType.NONE,
                    SscConstant.EVENT_SSC_QUERY_DOCUMENT, tId, 0));
        }

        requestData.offerSscData(new OipServiceData(mSlotId, ESsType.OIP,
                SscConstant.EVENT_SSC_UPDATE_OIP, tId, (enable ? 1 : 0)));

        addRequestToQueue(requestData);
    }

    @Override
    public void updateCOLR(int tId, int presentation) {
        if (!isServerBasedService(ESsType.TIR, SscConstant.CONDITION_INVALID)) {
            ImsLog.e(mSlotId, "Invalid service request");
            handleInvalidRequest(tId, REQUEST_TYPE_UPDATE);
            return;
        }

        SscRequestData requestData = new SscRequestData(tId);

        if (!SscXmlGov.getInstance(mSlotId).isXmlDataPresent()) {
            requestData.offerSscData(new SscServiceQueryData(mSlotId, ESsType.NONE,
                    SscConstant.EVENT_SSC_QUERY_DOCUMENT, tId, 0));
        }

        requestData.offerSscData(new TirServiceData(mSlotId, ESsType.TIR,
                SscConstant.EVENT_SSC_UPDATE_TIR, tId, presentation, 0));

        addRequestToQueue(requestData);
    }

    @Override
    public void updateCOLP(int tId, boolean enable) {
        if (!isServerBasedService(ESsType.TIP, SscConstant.CONDITION_INVALID)) {
            ImsLog.e(mSlotId, "Invalid service request");
            handleInvalidRequest(tId, REQUEST_TYPE_UPDATE);
            return;
        }

        SscRequestData requestData = new SscRequestData(tId);

        if (!SscXmlGov.getInstance(mSlotId).isXmlDataPresent()) {
            requestData.offerSscData(new SscServiceQueryData(mSlotId, ESsType.NONE,
                    SscConstant.EVENT_SSC_QUERY_DOCUMENT, tId, 0));
        }

        requestData.offerSscData(new TipServiceData(mSlotId, ESsType.TIP,
                SscConstant.EVENT_SSC_UPDATE_TIP, tId, (enable ? 1 : 0)));

        addRequestToQueue(requestData);
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

        if (!SscServiceStateAgent.getInstance().isUtAvailable(mSlotId)) {
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

    private void addRequestToQueue(SscRequestData requestData) {
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
     * For versions earlier than IR.92 v9.0, the UE must insert a new rule element with a rule ID
     * different from any existing rule ID in the XML document.
     * For IR.92 v10 and later, the UE must consider that the supplementary service is not
     * provisioned for the user and must not insert a new rule element with a rule ID different from
     * any existing rule ID in the XML document.
     *
     * @param sscData event number of sscData will be updated to process insert operation
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

        int mediaType = (data.getServiceClass() == SscServiceClassUtil.SERVICE_CLASS_VIDEO)
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

    private void handleQueryClirTb(int tId) {
        if (mUtListener == null) {
            return;
        }

        int clirMode = mSscPreferenceHelper.queryClir();
        if (clirMode < 0) {
            clirMode = SscConstant.OIR_DEFAULT;
        }

        if (clirMode == SscConstant.OIR_DEFAULT
                && SscConfig.isNetworkQueryForTbOirNetworkDefault(mSlotId)) {
            SscRequestData requestData = new SscRequestData(tId);

            if (!SscXmlGov.getInstance(mSlotId).isXmlDataPresent()) {
                requestData.offerSscData(new SscServiceQueryData(mSlotId, ESsType.NONE,
                        SscConstant.EVENT_SSC_QUERY_DOCUMENT, tId, 0));
            }

            requestData.offerSscData(new SscServiceQueryData(mSlotId, ESsType.OIR,
                    SscConstant.EVENT_SSC_QUERY_OIR_TB_NETWORK_DEFAULT, tId, -1));

            addRequestToQueue(requestData);
            return;
        }

        int outgoingState = clirMode; // 3GPP 27.007 7.7 n
        int provisionStatus = switch (clirMode) { // 3GPP 27.007 7.7 m
            case SscConstant.OIR_DEFAULT -> SscConstant.OIR_TEMPORARY_MODE_PRESENTATION_ALLOWED;
            case SscConstant.OIR_INVOCATION ->
                    SscConstant.OIR_TEMPORARY_MODE_PRESENTATION_RESTRICTED;
            case SscConstant.OIR_SUPPRESSION -> SscConstant.OIR_TEMPORARY_MODE_PRESENTATION_ALLOWED;
            default -> SscConstant.OIR_TEMPORARY_MODE_PRESENTATION_ALLOWED;
        };

        int state = (provisionStatus == SscConstant.OIR_TEMPORARY_MODE_PRESENTATION_RESTRICTED)
                ? SscConstant.STATUS_ENABLE : SscConstant.STATUS_DISABLE;

        postAndRunTask(() -> mUtListener.lineIdentificationSupplementaryServiceResponse(tId,
                new ImsSsInfo.Builder(state).setClirInterrogationStatus(provisionStatus)
                .setClirOutgoingState(outgoingState).build()));
    }

    private void handleUpdateClirTb(int tId, int clirMode) {
        if (mUtListener == null) {
            return;
        }

        boolean result = mSscPreferenceHelper.updateClir(clirMode);
        if (result) {
            if (SscConfig.isSyncWithCsForTbSs(mSlotId)) {
                // Invokes utConfigurationUpdateFailed() with CODE_LOCAL_CALL_CS_RETRY_REQUIRED
                // to trigger CSFB, then the modem will handle the request as well. It should
                // always be handled successfully in the modem side because it's a terminal-based
                // service request. The CSFB will be requested only when UE is in the CS voice
                // available network, or the Telephony will sync the state when UE camps in the CS
                // voice available network later.
                if (isCsVoiceNetworkRegistered()) {
                    ImsLog.d(mSlotId, "Sync CLIR");
                    ImsReasonInfo ri = new ImsReasonInfo(
                            ImsReasonInfo.CODE_LOCAL_CALL_CS_RETRY_REQUIRED,
                            ImsReasonInfo.CODE_UNSPECIFIED, null);
                    postAndRunTask(() -> mUtListener.utConfigurationUpdateFailed(tId, ri));
                    return;
                }
            }

            postAndRunTask(() -> mUtListener.utConfigurationUpdated(tId));
        } else {
            ImsLog.d(mSlotId, "ImsReasonInfo.CODE_LOCAL_INTERNAL_ERROR");
            ImsReasonInfo ri = new ImsReasonInfo(ImsReasonInfo.CODE_LOCAL_INTERNAL_ERROR,
                    ImsReasonInfo.CODE_UNSPECIFIED, null);
            postAndRunTask(() -> mUtListener.utConfigurationUpdateFailed(tId, ri));
        }
    }

    private boolean isServerBasedService(ESsType ssType, int condition) {
        int carrierConfigServiceType =
                SscUtils.getSupplementaryServiceTypeForCarrierConfig(ssType, condition);
        if (carrierConfigServiceType == SscConfig.SERVICE_TYPE_INVALID) {
            return false;
        }

        return SscConfig.isServerBasedService(mSlotId, carrierConfigServiceType);
    }

    private boolean isTerminalBasedService(ESsType ssType, int condition) {
        int carrierConfigServiceType =
                SscUtils.getSupplementaryServiceTypeForCarrierConfig(ssType, condition);
        if (carrierConfigServiceType == SscConfig.SERVICE_TYPE_INVALID) {
            return false;
        }

        return SscConfig.isTerminalBasedService(mSlotId, carrierConfigServiceType);
    }

    private boolean isCsVoiceNetworkRegistered() {
        PhoneStateInterface phoneState = AgentFactory.getInstance()
                .getAgent(PhoneStateInterface.class, mSlotId);
        if (phoneState == null) {
            return false;
        }

        int regState = phoneState.getCsNetworkRegistrationState();
        return (regState == android.telephony.NetworkRegistrationInfo.REGISTRATION_STATE_ROAMING)
                || (regState == android.telephony.NetworkRegistrationInfo.REGISTRATION_STATE_HOME);
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
                    SscData requestData = (SscData) msg.obj;
                    if (requestData != null) {
                         startTransaction(requestData);
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

            if (requestData.getTransactionId() != rr.getTransactionId()) {
                ImsLog.e(mSlotId, "invalid transaction : " + rr.getTransactionId());
                return;
            }

            int resultState = rr.getResultState();
            if (resultState == SscConstant.REQUEST_FAILURE) {
                if (rr.getCode() == SscConstant.HTTP_PRECONDITION_FAILURE) {
                    if (handlePreconditionFailure(requestData)) {
                        ImsLog.d(mSlotId, "handlePreconditionFailure");
                        return;
                    }
                }

                if (handleRetryWhenFailure(requestData)) {
                    ImsLog.d(mSlotId, "Need to retry. retryCount = " + requestData.getRetryCount());
                    return;
                }
            }

            if (resultState == SscConstant.REQUEST_SUCCESS) {
                if (handleAdditionalRequestWhenSuccess(requestData)) {
                    ImsLog.d(mSlotId, "Need to send additional request");
                    return;
                }
            }

            int eventNum = msg.what;
            if (eventNum == SscConstant.EVENT_SSC_QUERY_DOCUMENT) {
                if (resultState == SscConstant.REQUEST_FAILURE) {
                    // set actual event requested
                    requestData.pollSscData();
                    SscData sscData = requestData.peakSscData();
                    if (sscData != null) {
                        eventNum = sscData.getEventNumber();
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
                    if (!SscXmlFormat.getIsNoReplyTimerInRule(mSlotId)
                            && cfUpdateData.getReplyTimer() > 0) {
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
                    case SscConstant.EVENT_SSC_QUERY_OIR_TB_NETWORK_DEFAULT:
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
                ImsLog.d(mSlotId, "IUtListener is null");
                return;
            }

            mUtListener.utConfigurationUpdated(id);
        }

        private void onConfigurationUpdateFailed(final int id, final SscServiceData data) {
            if (mUtListener == null) {
                ImsLog.d(mSlotId, "IUtListener is null");
                return;
            }

            ImsReasonInfo ri = createImsReasonInfo(data);
            mUtListener.utConfigurationUpdateFailed(id, ri);
        }

        private void onConfigurationQueried(final int id, SscServiceData data) {
            if (mUtListener == null) {
                ImsLog.d(mSlotId, "IUtListener is null");
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
                default:
                    ImsLog.e(mSlotId, "Invalid SscServiceData");
                    break;
            }
        }

        private void onConfigurationQueryFailed(final int id, final SscServiceData data) {
            if (mUtListener == null) {
                ImsLog.d(mSlotId, "IUtListener is null");
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

            CbServiceData cbData = (CbServiceData) data;
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
                if (data.getEventNumber() == SscConstant.EVENT_SSC_QUERY_OIR_TB_NETWORK_DEFAULT) {
                    return new ImsSsInfo.Builder(oirData.getState())
                            .setClirInterrogationStatus(oirData.getProvisionStatus()) // m
                            .setClirOutgoingState(SscConstant.OIR_DEFAULT).build(); // n
                } else {
                    return new ImsSsInfo.Builder(oirData.getState())
                            .setClirInterrogationStatus(oirData.getProvisionStatus()) // m
                            .setClirOutgoingState(oirData.getOutgoingState()).build(); // n
                }
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
}
