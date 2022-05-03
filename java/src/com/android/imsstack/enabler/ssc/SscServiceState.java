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

import android.os.AsyncResult;
import android.os.Handler;
import android.os.Message;

import com.android.imsstack.core.agents.AgentFactory;
import com.android.imsstack.core.agents.agentif.IAlarmTimer;
import com.android.imsstack.core.agents.agentif.ISIMState;
import com.android.imsstack.core.agents.dcm.DCFactory;
import com.android.imsstack.core.agents.dcmif.EApnType;
import com.android.imsstack.core.agents.dcmif.IApn;
import com.android.imsstack.core.agents.dcmif.IDCApn;
import com.android.imsstack.core.agents.dcmif.IDCNetWatcher;
import com.android.imsstack.imsservice.mmtel.ut.UtFactory;
import com.android.imsstack.imsservice.mmtel.ut.base.UtInterface;
import com.android.imsstack.util.ImsExtApi;
import com.android.imsstack.util.ImsLog;
import com.android.imsstack.util.MSimUtils;
import com.android.internal.annotations.VisibleForTesting;

public class SscServiceState {
    @VisibleForTesting
    public static final int EVENT_UT_BLOCK_TIMER_EXPIRED = 1001;
    @VisibleForTesting
    public static final int EVENT_PDN_CONNECTION_FAILED = 1002;
    @VisibleForTesting
    public static final int EVENT_AIRPLANE_MODE_CHANGED = 1003;
    @VisibleForTesting
    public static final int EVENT_SIM_STATE_CHANGED = 1004;

    private int mSlotId = -1;
    private int mCurrentRat = -1;
    private int mTimerId = -1;
    private int mUtBlockTimerId = -1;
    private int mUtBlockReason = SscConstant.BLOCK_REASON_NONE;

    private boolean mAllSRVAddrTried = false;

    @VisibleForTesting
    public Handler mHandler = null;

    public void init(int slotId) {
        ImsLog.d("");
        mSlotId = slotId;
        mHandler = new SscServiceStateHandler();
        SscXmlFormat.init(mSlotId);

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

        updateUtServiceFeature();
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
            ImsLog.w("mUtBlockReason = " + getBlockedReasonString());
            return false;
        }

        ImsLog.i("Ut is available now");
        return true;
    }

    public void setErrorResponseCode(int responseCode) {
        if (SscConfig.isTemporaryErrorCode(mSlotId, responseCode)) {
            setUtBlock(SscConstant.BLOCK_REASON_BY_RESPONSE_CODE_TEMP);
        } else if (SscConfig.isPermanentErrorCode(mSlotId, responseCode)) {
            setUtBlock(SscConstant.BLOCK_REASON_BY_RESPONSE_CODE_PERM);
        }
    }

    public void setDnsQueryFailed(boolean input) {
        if (input) {
            setUtBlock(SscConstant.BLOCK_REASON_DNS_QUERY_FAILURE);
        } else {
            resetUtBlock(SscConstant.BLOCK_REASON_DNS_QUERY_FAILURE);
        }
    }

    public void setGbaRequestFailed(boolean input) {
        if (input) {
            setUtBlock(SscConstant.BLOCK_REASON_GBA_FAILURE);
        } else {
            resetUtBlock(SscConstant.BLOCK_REASON_GBA_FAILURE);
        }
    }

    public void setPdnConnectionTimerExpired(boolean input) {
        if (input) {
            setUtBlock(SscConstant.BLOCK_REASON_PDN_CONNECTION_TIMER_EXPIRED);
        } else {
            resetUtBlock(SscConstant.BLOCK_REASON_PDN_CONNECTION_TIMER_EXPIRED);
        }
    }

    public void setSocketConnectionExpired(boolean input) {
        if (input) {
            setUtBlock(SscConstant.BLOCK_REASON_SOCKET_CONNECTION_TIMER_EXPIRED);
        } else {
            resetUtBlock(SscConstant.BLOCK_REASON_SOCKET_CONNECTION_TIMER_EXPIRED);
        }
    }

    public void setAllSrvAddrTried(boolean input) {
        mAllSRVAddrTried = input;
    }

    public boolean getPdnConnectionFailed() {
        return isUtBlock(SscConstant.BLOCK_REASON_PDN_CONNECTION_FAILURE_TEMP
                | SscConstant.BLOCK_REASON_PDN_CONNECTION_FAILURE_PERM);
    }

    public boolean getDnsQueryFailed() {
        return isUtBlock(SscConstant.BLOCK_REASON_DNS_QUERY_FAILURE);
    }

    public boolean getGbaRequestFailed() {
        return isUtBlock(SscConstant.BLOCK_REASON_GBA_FAILURE);
    }

    public boolean getPdnConnectionTimerExpired() {
        return isUtBlock(SscConstant.BLOCK_REASON_PDN_CONNECTION_TIMER_EXPIRED);
    }

    public boolean getAllSrvAddrTried() {
        return mAllSRVAddrTried;
    }

    public boolean getSocketConnectionExpired() {
        return isUtBlock(SscConstant.BLOCK_REASON_SOCKET_CONNECTION_TIMER_EXPIRED);
    }

    private void resetAllUtStatus() {
        ImsLog.d("");

        mUtBlockReason = SscConstant.BLOCK_REASON_NONE;

        ISscAuthAgent authAgent = SscAuthAgent.getInstance(mSlotId);
        authAgent.setIsCredentialInfoUpdated(false);
        authAgent.setETag("");

        stopUtBlockTimer(true);
        updateUtServiceFeature();

        SscXmlFormat.reset(mSlotId);
    }

    private void setUtBlock(int nBlockReason) {
        ImsLog.d("mUtBlockReason : " + mUtBlockReason + " nBlockReason : " + nBlockReason);

        if (!SscConfig.isCsfbSupported(mSlotId)) {
            if (nBlockReason != SscConstant.BLOCK_REASON_PDN_CONNECTION_FAILURE_PERM
                    && nBlockReason != SscConstant.BLOCK_REASON_BY_RESPONSE_CODE_TEMP
                    && nBlockReason != SscConstant.BLOCK_REASON_BY_RESPONSE_CODE_PERM) {
                // do not block unless specific error reasons when CSFB not supported
                return;
            }
        }

        long blockTimeMilliSeconds = 0;
        switch (nBlockReason) {
            case SscConstant.BLOCK_REASON_GBA_FAILURE : // fall-through
            case SscConstant.BLOCK_REASON_DNS_QUERY_FAILURE : // fall-through
            case SscConstant.BLOCK_REASON_SOCKET_CONNECTION_TIMER_EXPIRED : // fall-through
            case SscConstant.BLOCK_REASON_PDN_CONNECTION_TIMER_EXPIRED : // fall-through
                blockTimeMilliSeconds = SscConfig.getTimerForTempBlockWithAnyReason(mSlotId);
                if (blockTimeMilliSeconds > 0) {
                    startUtBlockTimer(blockTimeMilliSeconds);
                    mUtBlockReason |= nBlockReason;
                    updateUtServiceFeature();
                }
                break;
            case SscConstant.BLOCK_REASON_PDN_CONNECTION_FAILURE_TEMP : // fall-through
            case SscConstant.BLOCK_REASON_BY_RESPONSE_CODE_TEMP :
                blockTimeMilliSeconds = SscConfig.getTimerForTempBlock(mSlotId);
                if (blockTimeMilliSeconds > 0) {
                    startUtBlockTimer(blockTimeMilliSeconds);
                    mUtBlockReason |= nBlockReason;
                    updateUtServiceFeature();
                }
                break;
            case SscConstant.BLOCK_REASON_PDN_CONNECTION_FAILURE_PERM : // fall-through
            case SscConstant.BLOCK_REASON_BY_RESPONSE_CODE_PERM :
                // don't start timer
                mUtBlockReason |= nBlockReason;
                updateUtServiceFeature();
                break;
            default :
                ImsLog.e("worng block reason");
                break;
        }
    }

    private void resetUtBlock(int nBlockReason) {
        ImsLog.d("mUtBlockReason : " + mUtBlockReason + " nBlockReason : " + nBlockReason);
        mUtBlockReason &= ~nBlockReason;
        updateUtServiceFeature();
    }

    private boolean isUtBlock(int nBlockReason) {
        return ((mUtBlockReason & nBlockReason) > 0) ? true : false;
    }

    private void startUtBlockTimer(long duration) {
        IAlarmTimer atm = getTimerAgent();
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

        IAlarmTimer atm = getTimerAgent();
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

    @VisibleForTesting
    protected IAlarmTimer getTimerAgent() {
        return (IAlarmTimer) AgentFactory.getAgent(AgentFactory.ALARM_TIMER, mSlotId);
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
                case EVENT_UT_BLOCK_TIMER_EXPIRED:
                    int allTempBlockReasons = SscConstant.BLOCK_REASON_DNS_QUERY_FAILURE
                            | SscConstant.BLOCK_REASON_PDN_CONNECTION_TIMER_EXPIRED
                            | SscConstant.BLOCK_REASON_SOCKET_CONNECTION_TIMER_EXPIRED
                            | SscConstant.BLOCK_REASON_PDN_CONNECTION_FAILURE_TEMP
                            | SscConstant.BLOCK_REASON_GBA_FAILURE
                            | SscConstant.BLOCK_REASON_BY_RESPONSE_CODE_TEMP;
                    resetUtBlock(allTempBlockReasons);
                    break;
                case EVENT_PDN_CONNECTION_FAILED:
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
                        IDCApn dcapn = (IDCApn) DCFactory.getDC(DCFactory.APN, mSlotId);
                        IApn apn = (dcapn != null)
                                ? dcapn.getApnControl(EApnType.XCAP.getType()) : null;
                        boolean isPermanentFailure = (apn != null)
                                ? apn.isESMCausePermanentFailure() : false;
                        if (isPermanentFailure) {
                            // TODO: how to reset permanent failure of ApnXcap?
                            setUtBlock(SscConstant.BLOCK_REASON_PDN_CONNECTION_FAILURE_PERM);
                        } else {
                            // TODO: Need to check KEY_UT_SM_CAUSE_TEMPORARY_BLOCK_INT_ARRAY
                            setUtBlock(SscConstant.BLOCK_REASON_PDN_CONNECTION_FAILURE_TEMP);
                        }
                    }
                    break;
                case EVENT_AIRPLANE_MODE_CHANGED:
                    IDCNetWatcher dcnw = (IDCNetWatcher)DCFactory.getDC(
                            DCFactory.NETWORK_WATCHER, mSlotId);
                    if (dcnw != null) {
                        if (dcnw.isAirplaneMode()) {
                            // TODO: need to reset permanent error here and ApnXcap
                            resetUtBlock(SscConstant.BLOCK_REASON_PDN_CONNECTION_FAILURE_TEMP);
                            resetUtBlock(SscConstant.BLOCK_REASON_BY_RESPONSE_CODE_PERM);
                        }
                    }
                    break;
                case EVENT_SIM_STATE_CHANGED:
                    ISIMState ss = (ISIMState)AgentFactory.getAgent(AgentFactory.SIM_STATE, mSlotId);
                    if (ss == null) {
                        ImsLog.e("ISIMState is null");
                        break;
                    }

                    if (ImsExtApi.Uicc.SIM_REMOVED.equalsIgnoreCase(ss.getIccState())) {
                        resetAllUtStatus();
                    }
                    break;
                default:
                    ImsLog.e("Invalid Message");
                    break;
            }

            ImsLog.d("mUtBlockReason = " + mUtBlockReason);
        }
    }

    private String getBlockedReasonString() {
        String reasons = "";
        if ((mUtBlockReason & SscConstant.BLOCK_REASON_GBA_FAILURE) > 0) {
            reasons += " BLOCK_REASON_GBA_FAILURE";
        }
        if ((mUtBlockReason & SscConstant.BLOCK_REASON_DNS_QUERY_FAILURE) > 0) {
            reasons += " BLOCK_REASON_DNS_QUERY_FAILURE";
        }
        if ((mUtBlockReason & SscConstant.BLOCK_REASON_SOCKET_CONNECTION_TIMER_EXPIRED) > 0) {
            reasons += " BLOCK_REASON_SOCKET_CONNECTION_TIMER_EXPIRED";
        }
        if ((mUtBlockReason & SscConstant.BLOCK_REASON_PDN_CONNECTION_TIMER_EXPIRED) > 0) {
            reasons += " BLOCK_REASON_PDN_CONNECTION_TIMER_EXPIRED";
        }
        if ((mUtBlockReason & SscConstant.BLOCK_REASON_PDN_CONNECTION_FAILURE_TEMP) > 0) {
            reasons += " BLOCK_REASON_PDN_CONNECTION_FAILURE_TEMP";
        }
        if ((mUtBlockReason & SscConstant.BLOCK_REASON_BY_RESPONSE_CODE_TEMP) > 0) {
            reasons += " BLOCK_REASON_BY_RESPONSE_CODE_TEMP";
        }
        if ((mUtBlockReason & SscConstant.BLOCK_REASON_PDN_CONNECTION_FAILURE_PERM) > 0) {
            reasons += " BLOCK_REASON_PDN_CONNECTION_FAILURE_PERM";
        }
        if ((mUtBlockReason & SscConstant.BLOCK_REASON_BY_RESPONSE_CODE_PERM) > 0) {
            reasons += " BLOCK_REASON_BY_RESPONSE_CODE_PERM";
        }

        return reasons;
    }
}