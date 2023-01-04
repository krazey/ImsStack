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
import com.android.imsstack.core.agents.IWifiState;
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
    // internal events
    private static final int EVENT_UT_CAPABILITY_CHANGED = 1000;
    @VisibleForTesting
    protected static final int EVENT_UT_BLOCK_TIMER_EXPIRED = 1001;

    // external events
    @VisibleForTesting
    protected static final int EVENT_WIFI_STATE_CHANGED = 2000;
    @VisibleForTesting
    protected static final int EVENT_AIRPLANE_MODE_CHANGED = 2001;
    @VisibleForTesting
    protected static final int EVENT_DATA_RAT_CHANGED = 2002;
    @VisibleForTesting
    protected static final int EVENT_DATA_ROAMING_STATE_CHANGED = 2003;

    private final int mSlotId;
    @VisibleForTesting
    final SscServiceStateHandler mHandler;
    @VisibleForTesting
    final SscSimStateListener mSimStateListener;
    @VisibleForTesting
    final SscRegiStateListener mRegiStateListener;

    @VisibleForTesting
    boolean mUtAvailability = false;
    private int mUtBlockTimerId = -1;
    @VisibleForTesting
    int mUtBlockReason = SscConstant.BLOCK_REASON_NONE;

    SscServiceState(int slotId, Looper looper) {
        mSlotId = slotId;
        mHandler = new SscServiceStateHandler(looper);
        mSimStateListener = new SscSimStateListener();
        mRegiStateListener = new SscRegiStateListener();
    }

    protected void init() {
        ImsLog.d(mSlotId, "");

        IWifiState ws = getWifiStateAgent();
        if (ws != null) {
            ws.registerForWifiStateChanged(mHandler, EVENT_WIFI_STATE_CHANGED, null);
        }

        IDcNetWatcher dnw = getDcNetWatcher();
        if (dnw != null) {
            dnw.registerForAirplaneModeChanged(mHandler, EVENT_AIRPLANE_MODE_CHANGED, null);
            dnw.registerForRatChanged(mHandler, EVENT_DATA_RAT_CHANGED, null);
            dnw.registerForRoamingStateChanged(mHandler, EVENT_DATA_ROAMING_STATE_CHANGED, null);
        }

        SimInterface sim = AgentFactory.getInstance().getAgent(SimInterface.class, mSlotId);
        if (sim != null) {
            sim.addListener(mSimStateListener);
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

        IWifiState ws = getWifiStateAgent();
        if (ws != null) {
            ws.unregisterForWifiStateChanged(mHandler);
        }

        IDcNetWatcher dnw = getDcNetWatcher();
        if (dnw != null) {
            dnw.unregisterForAirplaneModeChanged(mHandler);
            dnw.unregisterForRatChanged(mHandler);
            dnw.unregisterForRoamingStateChanged(mHandler);
        }

        SimInterface sim = AgentFactory.getInstance().getAgent(SimInterface.class, mSlotId);
        if (sim != null) {
            sim.removeListener(mSimStateListener);
        }

        IAosRegistration aosService = AosFactory.getInstance().getAosRegistration(mSlotId);
        if (aosService != null) {
            aosService.removeListener(mRegiStateListener);
        }
    }

    protected boolean isUtAvailable() {
        ImsLog.i(mSlotId, "isUtAvailable : " + mUtAvailability);
        return mUtAvailability;
    }

    protected void setErrorResponseCode(int responseCode) {
        if (SscConfig.isPermanentErrorCode(mSlotId, responseCode)) {
            setUtBlockReason(SscConstant.BLOCK_REASON_BY_RESPONSE_CODE_PERM);
        } else if (SscConfig.isTemporaryErrorCode(mSlotId, responseCode)) {
            setUtBlockReason(SscConstant.BLOCK_REASON_BY_RESPONSE_CODE_TEMP);
        }
    }

    protected void setPdnConnectionFailed(int smCause) {
        if (SscConfig.isPermanentBlockSmCause(mSlotId, smCause)) {
            setUtBlockReason(SscConstant.BLOCK_REASON_PDN_CONNECTION_FAILURE_PERM);
        } else if (SscConfig.isTemporaryBlockSmCause(mSlotId, smCause)) {
            setUtBlockReason(SscConstant.BLOCK_REASON_PDN_CONNECTION_FAILURE_TEMP);
        }
    }

    protected void setDnsQueryFailed(boolean input) {
        if (input) {
            setUtBlockReason(SscConstant.BLOCK_REASON_DNS_QUERY_FAILURE);
        } else {
            resetUtBlockReason(SscConstant.BLOCK_REASON_DNS_QUERY_FAILURE);
        }
    }

    protected void setGbaRequestFailed(boolean input) {
        if (input) {
            setUtBlockReason(SscConstant.BLOCK_REASON_GBA_FAILURE);
        } else {
            resetUtBlockReason(SscConstant.BLOCK_REASON_GBA_FAILURE);
        }
    }

    protected void setPdnConnectionTimeout(boolean input) {
        if (input) {
            setUtBlockReason(SscConstant.BLOCK_REASON_PDN_CONNECTION_TIMEOUT);
        } else {
            resetUtBlockReason(SscConstant.BLOCK_REASON_PDN_CONNECTION_TIMEOUT);
        }
    }

    protected void setSocketConnectionExpired(boolean input) {
        if (input) {
            setUtBlockReason(SscConstant.BLOCK_REASON_SOCKET_CONNECTION_TIMEOUT);
        } else {
            resetUtBlockReason(SscConstant.BLOCK_REASON_SOCKET_CONNECTION_TIMEOUT);
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

    private boolean getCurrentUtAvailability() {
        if (!SscConfig.isUtSupported(mSlotId)) {
            ImsLog.i(mSlotId, "Ut not supported");
            return false;
        }

        if (!isValidNetwork()) {
            ImsLog.w(mSlotId, "invalid network");
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

    private boolean isValidNetwork() {
        IWifiState ws = getWifiStateAgent();
        if (ws != null && ws.isWifiConnected()) {
            if (SscConfig.isSupportedNetwork(mSlotId, SscConstant.NETWORK_TYPE_IWLAN)) {
                ImsLog.d(mSlotId, "support Ut over wifi");
                return true;
            }
        }

        IDcNetWatcher dnw = getDcNetWatcher();
        if (dnw == null) {
            ImsLog.w(mSlotId, "DcNetWatcher is null");
            return false;
        }

        if (dnw.isRoaming()) {
            if (!SscConfig.isUtSupportedWhenRoaming(mSlotId)) {
                ImsLog.w(mSlotId, "Ut not supported when roaming");
                return false;
            }
        }

        int dataRat = dnw.getNetworkType();
        int accessNetworkType = SscUtils.convertToAccessNetworkType(dataRat);
        ImsLog.d(mSlotId, "accessNetworkType : " + accessNetworkType);

        return SscConfig.isSupportedNetwork(mSlotId, accessNetworkType);
    }

    private void setUtBlockReason(int blockReason) {
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

    private void resetUtBlockReason(int blockReason) {
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
    protected IWifiState getWifiStateAgent() {
        return (IWifiState) AgentFactory.getAgent(AgentFactory.WIFI_STATE, mSlotId);
    }

    @VisibleForTesting
    protected IDcNetWatcher getDcNetWatcher() {
        return (IDcNetWatcher) DcFactory.getDc(DcFactory.NETWORK_WATCHER, mSlotId);
    }

    /**
     * It informs of Ut capability has been changed after one second when the change occurs by
     * external conditions, such as data state, Wi-Fi connection state, and IMS registration state.
     */
    private void handleUtFeatureCapabilityChanged() {
        if (mHandler.hasMessages(EVENT_UT_CAPABILITY_CHANGED)) {
            mHandler.removeMessages(EVENT_UT_CAPABILITY_CHANGED);
        }

        mHandler.sendEmptyMessageDelayed(EVENT_UT_CAPABILITY_CHANGED, 1000); // 1s
    }

    private void notifyUtFeatureCapabilityChanged() {
        boolean currentUtAvailability = getCurrentUtAvailability();
        if (mUtAvailability == currentUtAvailability) {
            return;
        }

        mUtAvailability = currentUtAvailability;

        IUtInterface utInterface = UtFactory.getInstance().getUtInterface(mSlotId);
        if (utInterface != null) {
            utInterface.onServiceStateChanged();
        }
    }

    private class SscServiceStateHandler extends Handler {
        SscServiceStateHandler(Looper looper) {
            super(looper);
        }

        @Override
        public void handleMessage(Message msg) {
            if (msg == null) {
                return;
            }

            ImsLog.d(mSlotId, "Message : " + msg.what);

            switch(msg.what) {
                case EVENT_UT_CAPABILITY_CHANGED:
                    notifyUtFeatureCapabilityChanged();
                    break;
                case EVENT_UT_BLOCK_TIMER_EXPIRED:
                    handleBlockTimerExpired();
                    break;
                case EVENT_AIRPLANE_MODE_CHANGED:
                    handleAirplaneModeChanged();
                    break;
                case EVENT_WIFI_STATE_CHANGED: // FALL-THROUGH
                case EVENT_DATA_RAT_CHANGED: // FALL-THROUGH
                case EVENT_DATA_ROAMING_STATE_CHANGED:
                    handleUtFeatureCapabilityChanged();
                    break;
                default:
                    ImsLog.e(mSlotId, "Invalid Message");
                    break;
            }
        }

        private void handleBlockTimerExpired() {
            int allTempBlockReasons = SscConstant.BLOCK_REASON_DNS_QUERY_FAILURE
                    | SscConstant.BLOCK_REASON_PDN_CONNECTION_TIMEOUT
                    | SscConstant.BLOCK_REASON_SOCKET_CONNECTION_TIMEOUT
                    | SscConstant.BLOCK_REASON_PDN_CONNECTION_FAILURE_TEMP
                    | SscConstant.BLOCK_REASON_GBA_FAILURE
                    | SscConstant.BLOCK_REASON_BY_RESPONSE_CODE_TEMP;
            resetUtBlockReason(allTempBlockReasons);
        }

        private void handleAirplaneModeChanged() {
            IDcNetWatcher dnw = getDcNetWatcher();
            if (dnw != null) {
                if (dnw.isAirplaneMode()) {
                    // Reset permanent block reasons and PDN dependent block reasons.
                    resetUtBlockReason(SscConstant.BLOCK_REASON_PDN_CONNECTION_FAILURE_PERM);
                    resetUtBlockReason(SscConstant.BLOCK_REASON_PDN_CONNECTION_FAILURE_TEMP);
                    resetUtBlockReason(SscConstant.BLOCK_REASON_BY_RESPONSE_CODE_PERM);

                    handleUtFeatureCapabilityChanged();
                }
            }
        }
    }

    private final class SscSimStateListener implements Sim.Listener {

        @Override
        public void onSimCardStateChanged() {
            ImsLog.d(mSlotId, "onSimCardStateChanged");

            SimInterface sim = AgentFactory.getInstance().getAgent(SimInterface.class, mSlotId);
            if (sim != null) {
                if (sim.getSimCardState() == Sim.STATE_ABSENT) {
                    resetAllUtStatus();
                }
            }
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