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

import com.android.imsstack.core.agents.AgentFactory;
import com.android.imsstack.core.agents.IAlarmTimer;
import com.android.imsstack.core.agents.Sim;
import com.android.imsstack.core.agents.SimInterface;
import com.android.imsstack.core.agents.dcm.DcFactory;
import com.android.imsstack.core.agents.dcmif.IDcNetWatcher;
import com.android.imsstack.enabler.aos.AosFactory;
import com.android.imsstack.enabler.aos.IAosRegistration;
import com.android.imsstack.enabler.aos.IAosRegistrationListener;
import com.android.imsstack.imsservice.mmtel.ut.UtFactory;
import com.android.imsstack.imsservice.mmtel.ut.base.IUtInterface;
import com.android.imsstack.util.ImsLog;
import com.android.internal.annotations.VisibleForTesting;

public class SscServiceState {
    private static final int EVENT_UT_CAPABITILY_CHANGED = 1000;
    @VisibleForTesting
    protected static final int EVENT_UT_BLOCK_TIMER_EXPIRED = 1001;
    @VisibleForTesting
    protected static final int EVENT_AIRPLANE_MODE_CHANGED = 1002;

    private final int mSlotId;
    @VisibleForTesting
    final SscServiceStateHandler mHandler;
    @VisibleForTesting
    final SscRegiStateListener mRegiStateListener;

    private int mUtBlockTimerId = -1;
    private int mUtBlockReason = SscConstant.BLOCK_REASON_NONE;

    SscServiceState(int slotId, Looper looper) {
        mSlotId = slotId;
        mHandler = new SscServiceStateHandler(looper);
        mRegiStateListener = new SscRegiStateListener();
    }

    protected void init() {
        ImsLog.d(mSlotId, "");

        IDcNetWatcher dnw = (IDcNetWatcher) DcFactory.getDc(DcFactory.NETWORK_WATCHER, mSlotId);
        if (dnw != null) {
            dnw.registerForAirplaneModeChanged(mHandler, EVENT_AIRPLANE_MODE_CHANGED, null);
        }

        SimInterface sim = AgentFactory.getInstance().getAgent(SimInterface.class, mSlotId);
        if (sim != null) {
            sim.addListener(mHandler);
        }

        IAosRegistration aosService = AosFactory.getInstance().getAosRegistration(mSlotId);
        if (aosService != null) {
            aosService.addListener(mRegiStateListener);
        }

        handleUtFeatureCapabilityChanged();
    }

    protected void deInit() {
        ImsLog.d(mSlotId, "");

        resetAllUtStatus();

        IDcNetWatcher dnw = (IDcNetWatcher) DcFactory.getDc(DcFactory.NETWORK_WATCHER, mSlotId);
        if (dnw != null) {
            dnw.unregisterForRatChanged(mHandler);
            dnw.unregisterForAirplaneModeChanged(mHandler);
        }

        SimInterface sim = AgentFactory.getInstance().getAgent(SimInterface.class, mSlotId);
        if (sim != null) {
            sim.removeListener(mHandler);
        }

        IAosRegistration aosService = AosFactory.getInstance().getAosRegistration(mSlotId);
        if (aosService != null) {
            aosService.removeListener(mRegiStateListener);
        }
    }

    protected boolean isUtAvailable() {
        if (!SscConfig.isUtSupported(mSlotId)) {
            ImsLog.i(mSlotId, "Ut not supported");
            return false;
        }

        if (SscConfig.isImsRegistrationRequired(mSlotId)) {
            if (!mRegiStateListener.getImsRegistrationState()) {
                ImsLog.w(mSlotId, "Ims not registered");
                return false;
            }
        }

        if (mUtBlockReason != SscConstant.BLOCK_REASON_NONE) {
            ImsLog.w(mSlotId, "mUtBlockReason = " + getBlockedReasonString(mUtBlockReason));
            return false;
        }

        ImsLog.i(mSlotId, "Ut is available now");

        return true;
    }

    protected void setErrorResponseCode(int responseCode) {
        if (SscConfig.isPermanentErrorCode(mSlotId, responseCode)) {
            setUtBlock(SscConstant.BLOCK_REASON_BY_RESPONSE_CODE_PERM);
        } else if (SscConfig.isTemporaryErrorCode(mSlotId, responseCode)) {
            setUtBlock(SscConstant.BLOCK_REASON_BY_RESPONSE_CODE_TEMP);
        }
    }

    protected void setPdnConnectionFailed(int smCause) {
        if (SscConfig.isPermanentBlockSmCause(mSlotId, smCause)) {
            setUtBlock(SscConstant.BLOCK_REASON_PDN_CONNECTION_FAILURE_PERM);
        } else if (SscConfig.isTemporaryBlockSmCause(mSlotId, smCause)) {
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

    protected boolean getPdnConnectionFailed() {
        return isUtBlocked(SscConstant.BLOCK_REASON_PDN_CONNECTION_FAILURE_TEMP
                | SscConstant.BLOCK_REASON_PDN_CONNECTION_FAILURE_PERM);
    }

    protected boolean getDnsQueryFailed() {
        return isUtBlocked(SscConstant.BLOCK_REASON_DNS_QUERY_FAILURE);
    }

    protected boolean getGbaRequestFailed() {
        return isUtBlocked(SscConstant.BLOCK_REASON_GBA_FAILURE);
    }

    protected boolean getPdnConnectionTimeout() {
        return isUtBlocked(SscConstant.BLOCK_REASON_PDN_CONNECTION_TIMEOUT);
    }

    protected boolean getSocketConnectionExpired() {
        return isUtBlocked(SscConstant.BLOCK_REASON_SOCKET_CONNECTION_TIMEOUT);
    }

    protected void resetAllUtStatus() {
        ImsLog.d("");

        mUtBlockReason = SscConstant.BLOCK_REASON_NONE;

        ISscAuthAgent authAgent = SscAuthAgent.getInstance(mSlotId);
        authAgent.setIsCredentialInfoUpdated(false);
        authAgent.setETag("");

        stopUtBlockTimer(true);
        notifyUtFeatureCapabilityChanged();
    }

    private void setUtBlock(int blockReason) {
        if ((mUtBlockReason & blockReason) > 0) {
            // Already blocked reason
            return;
        }

        ImsLog.d(mSlotId, "SetReason : " + getBlockedReasonString(blockReason));

        if (!SscConfig.isCsfbSupported(mSlotId)) {
            if (blockReason != SscConstant.BLOCK_REASON_PDN_CONNECTION_FAILURE_PERM
                    && blockReason != SscConstant.BLOCK_REASON_BY_RESPONSE_CODE_TEMP
                    && blockReason != SscConstant.BLOCK_REASON_BY_RESPONSE_CODE_PERM) {
                // do not block unless specific error reasons when CSFB not supported
                return;
            }
        }

        long blockTimeMilliSeconds = 0;
        switch (blockReason) {
            case SscConstant.BLOCK_REASON_GBA_FAILURE : // fall-through
            case SscConstant.BLOCK_REASON_DNS_QUERY_FAILURE : // fall-through
            case SscConstant.BLOCK_REASON_SOCKET_CONNECTION_TIMEOUT : // fall-through
            case SscConstant.BLOCK_REASON_PDN_CONNECTION_TIMEOUT : // fall-through
                blockTimeMilliSeconds = SscConfig.getTimerForTempBlockWithAnyReason(mSlotId);
                if (blockTimeMilliSeconds > 0) {
                    startUtBlockTimer(blockTimeMilliSeconds);
                    mUtBlockReason |= blockReason;
                    notifyUtFeatureCapabilityChanged();
                }
                break;
            case SscConstant.BLOCK_REASON_PDN_CONNECTION_FAILURE_TEMP : // fall-through
            case SscConstant.BLOCK_REASON_BY_RESPONSE_CODE_TEMP :
                blockTimeMilliSeconds = SscConfig.getTimerForTempBlock(mSlotId);
                if (blockTimeMilliSeconds > 0) {
                    startUtBlockTimer(blockTimeMilliSeconds);
                    mUtBlockReason |= blockReason;
                    notifyUtFeatureCapabilityChanged();
                }
                break;
            case SscConstant.BLOCK_REASON_PDN_CONNECTION_FAILURE_PERM : // fall-through
            case SscConstant.BLOCK_REASON_BY_RESPONSE_CODE_PERM :
                // don't start timer
                mUtBlockReason |= blockReason;
                notifyUtFeatureCapabilityChanged();
                break;
            default :
                ImsLog.e(mSlotId, "wrong block reason");
                break;
        }
    }

    private void resetUtBlock(int blockReason) {
        if ((mUtBlockReason & blockReason) == 0) {
            // not blocked reason
            return;
        }

        ImsLog.d(mSlotId, "ResetReason : " + getBlockedReasonString(blockReason));
        mUtBlockReason &= ~blockReason;
        notifyUtFeatureCapabilityChanged();
    }

    private boolean isUtBlocked(int blockReason) {
        return (mUtBlockReason & blockReason) > 0;
    }

    private void startUtBlockTimer(long duration) {
        IAlarmTimer alarmTimer = getTimerAgent();
        if (alarmTimer == null) {
            ImsLog.e(mSlotId, "alarmTimer is null");
            return;
        }

        if (mUtBlockTimerId != -1) {
            stopUtBlockTimer(false);
        }

        mUtBlockTimerId = alarmTimer.getTimerId();
        if (mUtBlockTimerId <= 0) {
            ImsLog.e(mSlotId, "Retry timer id is invalid");
            return;
        }

        alarmTimer.registerForTimerExpired(mUtBlockTimerId, mHandler, EVENT_UT_BLOCK_TIMER_EXPIRED,
                null);

        if (!alarmTimer.startTimer(mUtBlockTimerId, duration)) {
            stopUtBlockTimer(false);
            ImsLog.e(mSlotId, "Starting a validity timer failed");
            return;
        }

        ImsLog.i(mSlotId, EVENT_UT_BLOCK_TIMER_EXPIRED + " timer is started :: "
                + "tid[" + mUtBlockTimerId + "], duration[" + duration + "]");
    }

    private void stopUtBlockTimer(boolean stopRequired) {
        if (mUtBlockTimerId <= 0) {
            return;
        }

        IAlarmTimer alarmTimer = getTimerAgent();
        if (alarmTimer == null) {
            ImsLog.e(mSlotId, "alarmTimer is null");
            return;
        }

        if (stopRequired) {
            alarmTimer.stopTimer(mUtBlockTimerId);
        }

        alarmTimer.unregisterForTimerExpired(mUtBlockTimerId, mHandler);
        mUtBlockTimerId = (-1);
    }

    @VisibleForTesting
    protected IAlarmTimer getTimerAgent() {
        return (IAlarmTimer) AgentFactory.getAgent(AgentFactory.ALARM_TIMER, mSlotId);
    }

    @VisibleForTesting
    protected IDcNetWatcher getDcNetWatcher() {
        return (IDcNetWatcher) DcFactory.getDc(DcFactory.NETWORK_WATCHER, mSlotId);
    }

    private void handleUtFeatureCapabilityChanged() {
        if (mHandler.hasMessages(EVENT_UT_CAPABITILY_CHANGED)) {
            mHandler.removeMessages(EVENT_UT_CAPABITILY_CHANGED);
        }

        mHandler.sendEmptyMessageDelayed(EVENT_UT_CAPABITILY_CHANGED, 1000); // 1s
    }

    private void notifyUtFeatureCapabilityChanged() {
        IUtInterface utInterface = UtFactory.getInstance().getUtInterface(mSlotId);
        if (utInterface != null) {
            utInterface.onServiceStateChanged();
        }
    }

    private class SscServiceStateHandler extends Handler implements Sim.Listener {
        SscServiceStateHandler(Looper looper) {
            super(looper);
        }

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

            ImsLog.d(mSlotId, "Message : " + msg.what);

            switch(msg.what) {
                case EVENT_UT_CAPABITILY_CHANGED:
                    notifyUtFeatureCapabilityChanged();
                    break;
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
                    IDcNetWatcher dnw = getDcNetWatcher();
                    if (dnw != null) {
                        if (dnw.isAirplaneMode()) {
                            resetUtBlock(SscConstant.BLOCK_REASON_PDN_CONNECTION_FAILURE_PERM);
                            resetUtBlock(SscConstant.BLOCK_REASON_PDN_CONNECTION_FAILURE_TEMP);
                            resetUtBlock(SscConstant.BLOCK_REASON_BY_RESPONSE_CODE_PERM);
                        }
                    }
                    break;
                default:
                    ImsLog.e(mSlotId, "Invalid Message");
                    break;
            }

            ImsLog.d(mSlotId, "mUtBlockReason = " + mUtBlockReason);
        }
    }

    @VisibleForTesting
    final class SscRegiStateListener implements IAosRegistrationListener {
        @VisibleForTesting
        boolean mImsRegistrationState = false;

        @Override
        public void notifyRegistered(int networkType, int featureTagBits,
                java.util.Set<String> featureTags) {
            ImsLog.d(mSlotId, "Registered : network = " + networkType);

            if (!mImsRegistrationState) {
                mImsRegistrationState = true;
                handleUtFeatureCapabilityChanged();
            }
        }

        @Override
        public void notifyRegistering(int networkType, int featureTagBits,
                java.util.Set<String> featureTags) {
            // do nothing
        }

        @Override
        public void notifyDeregistered(int networkType, int reason) {
            ImsLog.d(mSlotId, "Deregistered : reason = " + reason);

            if (mImsRegistrationState) {
                mImsRegistrationState = false;
                handleUtFeatureCapabilityChanged();
            }
        }

        @Override
        public void notifyTechnologyChangeFailed(int networkType, int causeCode) {
            // do nothing
        }

        @Override
        public void notifyAssociatedUriChanged(android.net.Uri[] uris) {
            // do nothing
        }

        @Override
        public void notifyCapabilitiesUpdateFailed(int capabilities, int networkType, int reason) {
            // do nothing
        }

        public boolean getImsRegistrationState() {
            return mImsRegistrationState;
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