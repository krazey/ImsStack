package com.android.imsstack.enabler.ssc;

import com.android.imsstack.core.agents.AgentFactory;
import com.android.imsstack.core.agents.SubsInfoInterface;
import com.android.imsstack.core.agents.agentif.IAlarmTimer;
import com.android.imsstack.core.agents.agentif.ISIMState;
import com.android.imsstack.core.agents.dcm.DCFactory;
import com.android.imsstack.core.agents.dcm.DCNetWatcher;
import com.android.imsstack.core.agents.dcmif.EApnType;
import com.android.imsstack.core.agents.dcmif.IApn;
import com.android.imsstack.core.agents.dcmif.IDCApn;
import com.android.imsstack.core.agents.dcmif.IDCNetWatcher;
import com.android.imsstack.imsservice.mmtel.ut.UtFactory;
import com.android.imsstack.imsservice.mmtel.ut.base.UtInterface;
import com.android.imsstack.util.ImsExtApi;
import com.android.imsstack.util.ImsLog;
import com.android.imsstack.util.MSimUtils;
import com.android.internal.telephony.IccCardConstants;

import android.os.AsyncResult;
import android.os.Handler;
import android.os.Message;

public class SscServiceState {
    // Constants--------------------------------------------------
    private static final int EVENT_PDN_CONNECTION_FAILED = 1001;
    private static final int EVENT_ALARM_TIMER_EXPIRED = 1002;
    private static final int EVENT_AIRPLANE_MODE_CHANGED = 1003;
    private static final int EVENT_UT_BLOCK_TIMER_EXPIRED = 1004;
    private static final int EVENT_SIM_STATE_CHANGED = 1005;
    // Variables--------------------------------------------------
    private int mSlotId = -1;
    private int mCurrentRat = -1;
    private int mTimerId = -1;
    private int mUtBlockTimerId = -1;
    private int mUtBlockReason = SscConstant.BLOCK_REASON_NONE;

    private boolean mAllSRVAddrTried = false;
    private boolean mIsAfterSimRemoved = false;
    private boolean mIsInitialQueryDone = false;

    private Handler mHandler = null;

    // Public methods --------------------------------------------
    public void init(int slotId) {
        ImsLog.d("");

        mSlotId = slotId;

        // Load and Set Rule Id
        SscXmlFormat.init(mSlotId);

        mHandler = new SscServiceStateHandler();

        IDCNetWatcher dnw = (IDCNetWatcher)DCFactory.getDC(DCFactory.NETWORK_WATCHER, mSlotId);
        if (dnw != null) {
            dnw.registerForPdnConnectionFailed(mHandler, EVENT_PDN_CONNECTION_FAILED , null);
            dnw.registerForAirplaneModeChanged(mHandler, EVENT_AIRPLANE_MODE_CHANGED, null);
        } else {
            ImsLog.e("DCNetWatcher is null");
        }

        ISIMState ss = (ISIMState)AgentFactory.getAgent(AgentFactory.SIM_STATE, mSlotId);
        if (ss != null) {
            ss.registerForSimStateChanged(mHandler, EVENT_SIM_STATE_CHANGED, null);
        }
    }

    public void deInit() {
        resetAllUtStatus();

        IDCNetWatcher dnw = (IDCNetWatcher)DCFactory.getDC(DCFactory.NETWORK_WATCHER, mSlotId);
        if (dnw != null) {
            dnw.unregisterForPdnConnectionFailed(mHandler);
            dnw.unregisterForRatChanged(mHandler);
            dnw.unregisterForAirplaneModeChanged(mHandler);
        }

        ISIMState ss = (ISIMState)AgentFactory.getAgent(AgentFactory.SIM_STATE, mSlotId);
        if (ss != null) {
            ss.unregisterForSimStateChanged(mHandler);
        }
    }

    public boolean isUtAvailable() {
        /*
        boolean admin_ims = false;
        SubsInfoInterface subsInfo = AgentFactory.getInstance().getAgent(
                SubsInfoInterface.class, mSlotId);
        if (subsInfo != null) {
            admin_ims = subsInfo.isImsEnabled();
        }

        if (admin_ims != true) {
            ImsLog.i("Admin Ims : false");
            return false;
        }
         */

        if (!SscConfig.isUtSupported(mSlotId)) {
            ImsLog.i("Ut not supported");
            return false;
        }

        if ((MSimUtils.isMultiImsEnabled() == false) && MSimUtils.isMultiImsEnabledOnDssv()) {
            if (MSimUtils.getDefaultDataSubId() != MSimUtils.getSubId(mSlotId)) {
                ImsLog.w(mSlotId, "DSSV-DV :: Ut blocked for non-DDS slot");
                return false;
            }
        }

        if (mUtBlockReason != SscConstant.BLOCK_REASON_NONE) {
            ImsLog.w("mUtBlockReason = " + mUtBlockReason);
            return false;
        }

        IDCApn dcApn = (IDCApn)DCFactory.getDC(DCFactory.APN, mSlotId);
        IApn apn = (dcApn != null) ? dcApn.getApnControl(EApnType.XCAP.getType()) : null;
        boolean pdnBlocked = (apn != null) ? apn.isESMCausePermanentFailure() : false;
        if (pdnBlocked) {
            ImsLog.w("pdnBlocked");
            return false;
        }

        ImsLog.i("Ut is available now");
        return true;
    }

    private void resetAllUtStatus() {
        ImsLog.d("");

        mUtBlockReason = SscConstant.BLOCK_REASON_NONE;

        mIsInitialQueryDone = false;

        ISscAuthAgent authAgent = SscAuthAgent.getInstance(mSlotId);
        authAgent.setIsCredentialInfoUpdated(false);
        authAgent.setETag("");

        stopUtBlockTimer(true);
        updateUtServiceFeature();

        SscXmlFormat.reset(mSlotId);
    }

    private void setUtBlock(int nBlockReason, boolean isEnable) {
        ImsLog.d("mUtBlockReason : " + mUtBlockReason + " nBlockReason : " + nBlockReason
                + " isEnable : " + isEnable);

        if (SscConfig.isCsfbSupported(mSlotId) == false) {
            if (nBlockReason != SscConstant.BLOCK_REASON_BY_RESPONSE_CODE_TEMP
                    && nBlockReason != SscConstant.BLOCK_REASON_PDN_CONNECTION_FAILURE
                    && nBlockReason != SscConstant.BLOCK_REASON_BY_RESPONSE_CODE_PERM) {
                // TODO: Need to check requirements for GBA and PDN connection failure.
                // do not block when connection error if CSFB not supported
                return;
            }
        }

        if (isEnable) {
            mUtBlockReason |= nBlockReason;
        } else {
            mUtBlockReason &= ~nBlockReason;
        }

        updateUtServiceFeature();

        if (!isEnable) {
            // don't need to start block timer
            return;
        }

        switch (nBlockReason) {
            case SscConstant.BLOCK_REASON_GBA_FAILURE : // fall-through
            case SscConstant.BLOCK_REASON_DNS_QUERY_FAILURE : // fall-through
            case SscConstant.BLOCK_REASON_PDN_CONNECTION_TIMER_EXPIRED : // fall-through
            case SscConstant.BLOCK_REASON_SOCKET_CONNECTION_TIMER_EXPIRED :
                startUtBlockTimer(SscConfig.getTimerForTempBlockWithAnyReason(mSlotId));
                break;
            case SscConstant.BLOCK_REASON_PDN_CONNECTION_FAILURE :
            case SscConstant.BLOCK_REASON_BY_RESPONSE_CODE_TEMP :
                startUtBlockTimer(SscConfig.getTimerForTempBlock(mSlotId));
                break;
            case SscConstant.BLOCK_REASON_BY_RESPONSE_CODE_PERM :
                // don't start timer
                break;
            default :
                ImsLog.e("worng block reason");
                break;
        }
    }

    private boolean isUtBlock(int nBlockReason) {
        return ((mUtBlockReason & nBlockReason) > 0) ? true : false;
    }

    // This method called by Transaction
    public void setErrorResponseCode(int responseCode) {
        if (SscConfig.isTemporaryErrorCode(mSlotId, responseCode)) {
            setUtBlock(SscConstant.BLOCK_REASON_BY_RESPONSE_CODE_TEMP, true);
        } else if (SscConfig.isPermanentErrorCode(mSlotId, responseCode)) {
            setUtBlock(SscConstant.BLOCK_REASON_BY_RESPONSE_CODE_PERM, true);
        }
    }

    // This method called by HTTPConnection
    public void setDNSQueryFailed(boolean input) {
        setUtBlock(SscConstant.BLOCK_REASON_DNS_QUERY_FAILURE, input);
    }

    public void setGBARequestFailed(boolean input) {
        setUtBlock(SscConstant.BLOCK_REASON_GBA_FAILURE, input);
    }

    public void setPDNConnectionTimerExpired(boolean input) {
        setUtBlock(SscConstant.BLOCK_REASON_PDN_CONNECTION_TIMER_EXPIRED, input);
    }

    public void setSocketConnectionExpired(boolean input) {
        setUtBlock(SscConstant.BLOCK_REASON_SOCKET_CONNECTION_TIMER_EXPIRED, input);
    }

    public void setAllSRVAddrTried(boolean input) {
        mAllSRVAddrTried = input;
    }

    public void setIsInitialQueryDone(boolean input) {
        mIsInitialQueryDone = input;
    }

    public boolean getPdnConnectionFailed() {
        return isUtBlock(SscConstant.BLOCK_REASON_PDN_CONNECTION_FAILURE);
    }

    public boolean getDNSQueryFailed() {
        return isUtBlock(SscConstant.BLOCK_REASON_DNS_QUERY_FAILURE);
    }

    public boolean getGBARequestFailed() {
        return isUtBlock(SscConstant.BLOCK_REASON_GBA_FAILURE);
    }

    public boolean getPDNConnectionTimerExpired() {
        return isUtBlock(SscConstant.BLOCK_REASON_PDN_CONNECTION_TIMER_EXPIRED);
    }

    public boolean getAllSRVAddrTried() {
        return mAllSRVAddrTried;
    }

    public boolean getSocketConnectionExpired() {
        return isUtBlock(SscConstant.BLOCK_REASON_SOCKET_CONNECTION_TIMER_EXPIRED);
    }

    public boolean getIsInitialQueryDone() {
        return mIsInitialQueryDone;
    }

    private void startUtBlockTimer(long duration) {
        IAlarmTimer atm = (IAlarmTimer)AgentFactory.getAgent(AgentFactory.ALARM_TIMER, mSlotId);
        if (atm == null) {
            ImsLog.e("AlamTimerManager is null");
            return;
        }

        if (mUtBlockTimerId != -1) {
            stopUtBlockTimer(false);
        }

        mUtBlockTimerId = atm.getTimerId();
        if (mUtBlockTimerId <= 0) {
            ImsLog.e("Retry timer id is invalid");
            return;
        }

        atm.registerForTimerExpired(mUtBlockTimerId, mHandler, EVENT_UT_BLOCK_TIMER_EXPIRED, null);

        if (!atm.startTimer(mUtBlockTimerId, duration)) {
            stopUtBlockTimer(false);
            ImsLog.e(" Starting a validity timer failed");
            return;
        }

        ImsLog.i(EVENT_UT_BLOCK_TIMER_EXPIRED + " timer is started :: "
                + "tid[" + mUtBlockTimerId + "], duration[" + duration + "]");
    }

    private void stopUtBlockTimer(boolean stopRequired) {
        if (mUtBlockTimerId <= 0) {
            return;
        }

        IAlarmTimer atm = (IAlarmTimer)AgentFactory.getAgent(AgentFactory.ALARM_TIMER, mSlotId);
        if (atm == null) {
            ImsLog.e(" AlamTimerManager is null");
            return;
        }

        if (stopRequired) {
            atm.stopTimer(mUtBlockTimerId);
        }

        atm.unregisterForTimerExpired(mUtBlockTimerId, mHandler);
        mUtBlockTimerId = (-1);
    }

    private void updateUtServiceFeature() {
        UtInterface utInterface = UtFactory.getInstance().getUtInterface(mSlotId);
        if (utInterface != null) {
            utInterface.onServiceStateChanged();
        }
    }

    private class SscServiceStateHandler extends Handler {
        @Override
        public void handleMessage(Message msg) {
            if (msg == null) {
                return;
            }

            ImsLog.d("Message : " + msg.what);

            switch(msg.what) {
                case EVENT_PDN_CONNECTION_FAILED:
                {
                    AsyncResult ar = (AsyncResult)msg.obj;
                    if (ar == null) {
                        return;
                    }

                    EApnType apnType = (EApnType)ar.result;
                    if (apnType == null) {
                        return;
                    }

                    ImsLog.d("ApnType : " + apnType.toString());
                    // Change only for XCAP apn type.
                    if (apnType == EApnType.XCAP) {
                        setUtBlock(SscConstant.BLOCK_REASON_PDN_CONNECTION_FAILURE, true);
                    }
                    break;
                }
                case EVENT_UT_BLOCK_TIMER_EXPIRED:
                {
                    ISscAuthAgent authAgent = SscAuthAgent.getInstance(mSlotId);
                    if (isUtBlock(SscConstant.BLOCK_REASON_GBA_FAILURE) == true
                            && authAgent.isCredentialInfoUpdated()) {
                        authAgent.setIsCredentialInfoUpdated(false);
                    }

                    int allTempBlockReasons = SscConstant.BLOCK_REASON_DNS_QUERY_FAILURE
                            | SscConstant.BLOCK_REASON_PDN_CONNECTION_TIMER_EXPIRED
                            | SscConstant.BLOCK_REASON_SOCKET_CONNECTION_TIMER_EXPIRED
                            | SscConstant.BLOCK_REASON_PDN_CONNECTION_FAILURE
                            | SscConstant.BLOCK_REASON_GBA_FAILURE
                            | SscConstant.BLOCK_REASON_BY_RESPONSE_CODE_TEMP;
                    setUtBlock(allTempBlockReasons, false);
                    break;
                }
                case EVENT_AIRPLANE_MODE_CHANGED:
                {
                    IDCNetWatcher dcnw = (IDCNetWatcher)DCFactory.getDC(
                            DCFactory.NETWORK_WATCHER, mSlotId);
                    if (dcnw != null) {
                        if (dcnw.isAirplaneMode()) {
                            setUtBlock(SscConstant.BLOCK_REASON_PDN_CONNECTION_FAILURE, false);
                            setUtBlock(SscConstant.BLOCK_REASON_BY_RESPONSE_CODE_PERM, false);
                        }
                    }
                    break;
                }
                case EVENT_SIM_STATE_CHANGED:
                {
                    ISIMState ss = (ISIMState)AgentFactory.getAgent(AgentFactory.SIM_STATE, mSlotId);
                    if (ss == null) {
                        ImsLog.e("ISIMState is null");
                        break;
                    }

                    if (ImsExtApi.Uicc.SIM_REMOVED.equalsIgnoreCase(ss.getIccState())) {
                        resetAllUtStatus();
                        mIsAfterSimRemoved = true;
                    } else if (mIsAfterSimRemoved && IccCardConstants.INTENT_VALUE_ICC_LOADED.equalsIgnoreCase(ss.getIccState())) {
                        mIsAfterSimRemoved = false;
                    }
                    break;
                }
                default:
                    ImsLog.e("Invalid Message");
                    break;
            }

            IDCApn dcapn = (IDCApn)DCFactory.getDC(DCFactory.APN, mSlotId);
            IApn apn = (dcapn != null) ? dcapn.getApnControl(EApnType.XCAP.getType()) : null;
            boolean pdnBlockedBy33 = (apn != null) ? apn.isESMCausePermanentFailure() : false;
            ImsLog.d("mUtBlockReason = " + mUtBlockReason + ", pdnBlockedBy33 = " + pdnBlockedBy33 + ")");
        }
    }
}
