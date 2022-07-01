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
import android.os.Message;

import com.android.imsstack.core.agents.AgentFactory;
import com.android.imsstack.core.agents.Sim;
import com.android.imsstack.core.agents.SimInterface;
import com.android.imsstack.core.agents.agentif.IAlarmTimer;
import com.android.imsstack.core.agents.dcm.DcFactory;
import com.android.imsstack.core.agents.dcmif.EApnType;
import com.android.imsstack.core.agents.dcmif.IApn;
import com.android.imsstack.core.agents.dcmif.IDcApn;
import com.android.imsstack.core.agents.dcmif.IDcNetWatcher;
import com.android.imsstack.imsservice.mmtel.ut.UtFactory;
import com.android.imsstack.imsservice.mmtel.ut.base.UtInterface;
import com.android.imsstack.util.ImsLog;
import com.android.imsstack.util.MSimUtils;
import com.android.internal.annotations.VisibleForTesting;

public class SscServiceState {
    @VisibleForTesting
    public static final int EVENT_UT_BLOCK_TIMER_EXPIRED = 1001;
    @VisibleForTesting
    public static final int EVENT_AIRPLANE_MODE_CHANGED = 1002;

    private int mSlotId = -1;
    private int mCurrentRat = -1;
    private int mTimerId = -1;
    private int mUtBlockTimerId = -1;
    private int mUtBlockReason = SscConstant.BLOCK_REASON_NONE;

    private boolean mAllSRVAddrTried = false;

    @VisibleForTesting
    public Handler mHandler = null;

    protected void init(int slotId) {
        ImsLog.d("");
        mSlotId = slotId;
        mHandler = new SscServiceStateHandler();
        SscXmlFormat.init(mSlotId);

        IDcNetWatcher dnw = (IDcNetWatcher) DcFactory.getDc(DcFactory.NETWORK_WATCHER, mSlotId);
        if (dnw != null) {
            dnw.registerForAirplaneModeChanged(mHandler, EVENT_AIRPLANE_MODE_CHANGED, null);
        } else {
            ImsLog.e("DcNetWatcher is null");
        }

        SimInterface sim = AgentFactory.getInstance().getAgent(SimInterface.class, mSlotId);

        if (sim != null) {
            sim.addListener((SscServiceStateHandler) mHandler);
        }

        updateUtServiceFeature();
    }

    protected void deInit() {
        resetAllUtStatus();

        IDcNetWatcher dnw = (IDcNetWatcher) DcFactory.getDc(DcFactory.NETWORK_WATCHER, mSlotId);
        if (dnw != null) {
            dnw.unregisterForRatChanged(mHandler);
            dnw.unregisterForAirplaneModeChanged(mHandler);
        }

        SimInterface sim = AgentFactory.getInstance().getAgent(SimInterface.class, mSlotId);

        if (sim != null) {
            sim.removeListener((SscServiceStateHandler) mHandler);
        }
    }

    protected boolean isUtAvailable() {
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
            ImsLog.w("mUtBlockReason = " + getBlockedReasonString(mUtBlockReason));
            return false;
        }

        ImsLog.i("Ut is available now");
        return true;
    }

    protected void setErrorResponseCode(int responseCode) {
        if (SscConfig.isTemporaryErrorCode(mSlotId, responseCode)) {
            setUtBlock(SscConstant.BLOCK_REASON_BY_RESPONSE_CODE_TEMP);
        } else if (SscConfig.isPermanentErrorCode(mSlotId, responseCode)) {
            setUtBlock(SscConstant.BLOCK_REASON_BY_RESPONSE_CODE_PERM);
        }
    }

    protected void setPdnConnectionFailed(boolean isPermanent) {
        if (isPermanent) {
            setUtBlock(SscConstant.BLOCK_REASON_PDN_CONNECTION_FAILURE_PERM);
        } else {
            setUtBlock(SscConstant.BLOCK_REASON_PDN_CONNECTION_FAILURE_TEMP);
        }
    }

    protected void setDnsQueryFailed(boolean input) {
        if (input) {
            setUtBlock(SscConstant.BLOCK_REASON_DNS_QUERY_FAILURE);
        } else {
            resetUtBlock(SscConstant.BLOCK_REASON_DNS_QUERY_FAILURE);
        }
    }

    protected void setGbaRequestFailed(boolean input) {
        if (input) {
            setUtBlock(SscConstant.BLOCK_REASON_GBA_FAILURE);
        } else {
            resetUtBlock(SscConstant.BLOCK_REASON_GBA_FAILURE);
        }
    }

    protected void setPdnConnectionTimeout(boolean input) {
        if (input) {
            setUtBlock(SscConstant.BLOCK_REASON_PDN_CONNECTION_TIMEOUT);
        } else {
            resetUtBlock(SscConstant.BLOCK_REASON_PDN_CONNECTION_TIMEOUT);
        }
    }

    protected void setSocketConnectionExpired(boolean input) {
        if (input) {
            setUtBlock(SscConstant.BLOCK_REASON_SOCKET_CONNECTION_TIMEOUT);
        } else {
            resetUtBlock(SscConstant.BLOCK_REASON_SOCKET_CONNECTION_TIMEOUT);
        }
    }

    protected void setAllSrvAddrTried(boolean input) {
        mAllSRVAddrTried = input;
    }

    protected boolean getPdnConnectionFailed() {
        return isUtBlock(SscConstant.BLOCK_REASON_PDN_CONNECTION_FAILURE_TEMP
                | SscConstant.BLOCK_REASON_PDN_CONNECTION_FAILURE_PERM);
    }

    protected boolean getDnsQueryFailed() {
        return isUtBlock(SscConstant.BLOCK_REASON_DNS_QUERY_FAILURE);
    }

    protected boolean getGbaRequestFailed() {
        return isUtBlock(SscConstant.BLOCK_REASON_GBA_FAILURE);
    }

    protected boolean getPdnConnectionTimeout() {
        return isUtBlock(SscConstant.BLOCK_REASON_PDN_CONNECTION_TIMEOUT);
    }

    protected boolean getAllSrvAddrTried() {
        return mAllSRVAddrTried;
    }

    protected boolean getSocketConnectionExpired() {
        return isUtBlock(SscConstant.BLOCK_REASON_SOCKET_CONNECTION_TIMEOUT);
    }

    protected void resetAllUtStatus() {
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
        if ((mUtBlockReason & nBlockReason) > 0) {
            // Already blocked reason
            return;
        }

        ImsLog.d("SetReason : " + getBlockedReasonString(nBlockReason));

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
            case SscConstant.BLOCK_REASON_SOCKET_CONNECTION_TIMEOUT : // fall-through
            case SscConstant.BLOCK_REASON_PDN_CONNECTION_TIMEOUT : // fall-through
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
        if ((mUtBlockReason & nBlockReason) == 0) {
            // not blocked reason
            return;
        }

        ImsLog.d("ResetReason : " + getBlockedReasonString(nBlockReason));
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

    @VisibleForTesting
    protected IApn getApn(EApnType apnType) {
        if (apnType == null) {
            return null;
        }

        IDcApn dcapn = (IDcApn) DcFactory.getDc(DcFactory.APN, mSlotId);
        return (dcapn != null) ? dcapn.getApnControl(apnType.getType()) : null;
    }

    @VisibleForTesting
    protected IDcNetWatcher getDcNetWatcher() {
        return (IDcNetWatcher) DcFactory.getDc(DcFactory.NETWORK_WATCHER, mSlotId);
    }

    private void updateUtServiceFeature() {
        UtInterface utInterface = UtFactory.getInstance().getUtInterface(mSlotId);
        if (utInterface != null) {
            utInterface.onServiceStateChanged();
        }
    }

    private class SscServiceStateHandler extends Handler implements Sim.Listener {
        @Override
        public void onSimCardStateChanged() {
            SimInterface sim = AgentFactory.getInstance().getAgent(SimInterface.class, mSlotId);

            if (sim != null) {
                if (sim.getSimCardState() == Sim.STATE_ABSENT) {
                    resetAllUtStatus();
                }
            }
        }

        @Override
        public void handleMessage(Message msg) {
            if (msg == null) {
                return;
            }

            ImsLog.d("Message : " + msg.what);
            switch(msg.what) {
                case EVENT_UT_BLOCK_TIMER_EXPIRED:
                    int allTempBlockReasons = SscConstant.BLOCK_REASON_DNS_QUERY_FAILURE
                            | SscConstant.BLOCK_REASON_PDN_CONNECTION_TIMEOUT
                            | SscConstant.BLOCK_REASON_SOCKET_CONNECTION_TIMEOUT
                            | SscConstant.BLOCK_REASON_PDN_CONNECTION_FAILURE_TEMP
                            | SscConstant.BLOCK_REASON_GBA_FAILURE
                            | SscConstant.BLOCK_REASON_BY_RESPONSE_CODE_TEMP;
                    resetUtBlock(allTempBlockReasons);
                    break;
                case EVENT_AIRPLANE_MODE_CHANGED:
                    IDcNetWatcher dcnw = getDcNetWatcher();
                    if (dcnw != null) {
                        if (dcnw.isAirplaneMode()) {
                            IApn apn = getApn(EApnType.XCAP);
                            if (apn != null) {
                                apn.resetESMCausePermanentFailure();
                            }
                            resetUtBlock(SscConstant.BLOCK_REASON_PDN_CONNECTION_FAILURE_PERM);
                            resetUtBlock(SscConstant.BLOCK_REASON_PDN_CONNECTION_FAILURE_TEMP);
                            resetUtBlock(SscConstant.BLOCK_REASON_BY_RESPONSE_CODE_PERM);
                        }
                    }
                    break;
                default:
                    ImsLog.e("Invalid Message");
                    break;
            }

            ImsLog.d("mUtBlockReason = " + mUtBlockReason);
        }
    }

    private String getBlockedReasonString(long utBlockReason) {
        String reasons = "";
        if ((utBlockReason & SscConstant.BLOCK_REASON_GBA_FAILURE) > 0) {
            reasons += " BLOCK_REASON_GBA_FAILURE";
        }
        if ((utBlockReason & SscConstant.BLOCK_REASON_DNS_QUERY_FAILURE) > 0) {
            reasons += " BLOCK_REASON_DNS_QUERY_FAILURE";
        }
        if ((utBlockReason & SscConstant.BLOCK_REASON_SOCKET_CONNECTION_TIMEOUT) > 0) {
            reasons += " BLOCK_REASON_SOCKET_CONNECTION_TIMEOUT";
        }
        if ((utBlockReason & SscConstant.BLOCK_REASON_PDN_CONNECTION_TIMEOUT) > 0) {
            reasons += " BLOCK_REASON_PDN_CONNECTION_TIMEOUT";
        }
        if ((utBlockReason & SscConstant.BLOCK_REASON_PDN_CONNECTION_FAILURE_TEMP) > 0) {
            reasons += " BLOCK_REASON_PDN_CONNECTION_FAILURE_TEMP";
        }
        if ((utBlockReason & SscConstant.BLOCK_REASON_BY_RESPONSE_CODE_TEMP) > 0) {
            reasons += " BLOCK_REASON_BY_RESPONSE_CODE_TEMP";
        }
        if ((utBlockReason & SscConstant.BLOCK_REASON_PDN_CONNECTION_FAILURE_PERM) > 0) {
            reasons += " BLOCK_REASON_PDN_CONNECTION_FAILURE_PERM";
        }
        if ((utBlockReason & SscConstant.BLOCK_REASON_BY_RESPONSE_CODE_PERM) > 0) {
            reasons += " BLOCK_REASON_BY_RESPONSE_CODE_PERM";
        }

        return reasons;
    }
}