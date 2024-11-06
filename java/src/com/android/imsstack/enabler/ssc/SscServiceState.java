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

import static android.telephony.ServiceState.STATE_IN_SERVICE;
import static android.telephony.ims.feature.CapabilityChangeRequest.CapabilityPair;
import static android.telephony.ims.feature.MmTelFeature.MmTelCapabilities.CAPABILITY_TYPE_UT;
import static android.telephony.ims.feature.MmTelFeature.MmTelCapabilities.CAPABILITY_TYPE_VOICE;
import static android.telephony.ims.stub.ImsRegistrationImplBase.REGISTRATION_TECH_CROSS_SIM;

import android.annotation.NonNull;
import android.net.ConnectivityManager;
import android.net.Network;
import android.net.NetworkCapabilities;
import android.net.NetworkSpecifier;
import android.net.TelephonyNetworkSpecifier;
import android.net.Uri;
import android.os.Handler;
import android.os.Looper;
import android.os.Message;
import android.telephony.TelephonyCallback;

import com.android.imsstack.base.AppContext;
import com.android.imsstack.base.MSimUtils;
import com.android.imsstack.base.SystemServiceProxy.ConnectivityManagerProxy;
import com.android.imsstack.base.TelephonyManagerProxy;
import com.android.imsstack.core.agents.AgentFactory;
import com.android.imsstack.core.agents.ConfigInterface;
import com.android.imsstack.core.agents.Sim;
import com.android.imsstack.core.agents.SimInterface;
import com.android.imsstack.core.agents.TimerInterface;
import com.android.imsstack.core.agents.WifiInterface;
import com.android.imsstack.core.agents.dcm.DcFactory;
import com.android.imsstack.core.agents.dcmif.IDcNetWatcher;
import com.android.imsstack.enabler.aos.AosFactory;
import com.android.imsstack.enabler.aos.IAosRegistration;
import com.android.imsstack.enabler.aos.IAosRegistrationListener;
import com.android.imsstack.enabler.aos.IAosRegistrationListener.RegistrationState;
import com.android.imsstack.imsservice.mmtel.ut.UtFactory;
import com.android.imsstack.imsservice.mmtel.ut.base.IUtInterface;
import com.android.imsstack.util.ImsLog;
import com.android.internal.annotations.VisibleForTesting;

import java.util.List;
import java.util.Set;

public class SscServiceState {
    // internal events
    @VisibleForTesting
    protected static final int EVENT_UT_CAPABILITY_CHANGED = 1000;

    // external events
    @VisibleForTesting
    protected static final int EVENT_AIRPLANE_MODE_CHANGED = 2000;
    @VisibleForTesting
    protected static final int EVENT_DATA_SERVICE_STATE_CHANGED = 2001;
    @VisibleForTesting
    protected static final int EVENT_DATA_NETWORK_TYPE_CHANGED = 2002;
    @VisibleForTesting
    protected static final int EVENT_DATA_ROAMING_STATE_CHANGED = 2003;
    @VisibleForTesting
    protected static final int EVENT_CROSS_SIM_DATA_STATE_CHANGED = 2004;

    private final int mSlotId;
    private int mSubId = MSimUtils.INVALID_SUB_ID;
    @VisibleForTesting
    final SscServiceStateHandler mHandler;
    @VisibleForTesting
    final SscSimStateListener mSimStateListener;
    @VisibleForTesting
    final SscCarrierConfigListener mCarrierConfigListener;
    final SscTimerListener mTimerListener;
    @VisibleForTesting
    final IDcNetWatcher.Listener mNetWatcherListener;
    @VisibleForTesting
    SscWifiListener mWifiListener;
    @VisibleForTesting
    SscRegiStateListener mRegiStateListener = null;
    @VisibleForTesting
    SscMobileDataStateListener mMobileDataStateListener = null;
    @VisibleForTesting
    SscCrossSimDataStateListener mCrossSimDataStateListener =  null;

    private boolean mIsInitialized = false;
    @VisibleForTesting
    boolean mIsUtFeatureEnabled = true;
    @VisibleForTesting
    boolean mIsCrossSimFeatureEnabled = false;
    @VisibleForTesting
    boolean mUtAvailability = false;
    private long mUtBlockTimerId = TimerInterface.INVALID_TID;
    @VisibleForTesting
    int mUtBlockReason = SscConstant.BLOCK_REASON_NONE;

    SscServiceState(int slotId, Looper looper) {
        mSlotId = slotId;
        mHandler = new SscServiceStateHandler(looper);
        mSimStateListener = new SscSimStateListener();
        mCarrierConfigListener = new SscCarrierConfigListener();
        mTimerListener = new SscTimerListener();
        mNetWatcherListener = new NetWatcherListener();
    }

    protected void init() {
        ImsLog.d(mSlotId, "");

        IDcNetWatcher dnw = getDcNetWatcher();
        if (dnw != null) {
            dnw.addListener(mNetWatcherListener);
        }

        SimInterface sim = getSimInterface();
        if (sim != null) {
            mSubId = sim.getSubId();
            sim.addListener(mSimStateListener);
        }

        ConfigInterface config = getConfigInterface();
        if (config != null) {
            config.addListener(mCarrierConfigListener);
        }

        // register listeners related to carrier configuration
        registerWifiConnectionStateChangedEvent();
        registerImsRegistrationStateListener();
        registerMobileDataStateListener(mSubId);
        registerCrossSimDataStateListener();

        mIsInitialized = true;
        handleUtFeatureCapabilityChanged();
    }

    protected void deInit() {
        ImsLog.d(mSlotId, "");

        resetAllUtStatus();

        IDcNetWatcher dnw = getDcNetWatcher();
        if (dnw != null) {
            dnw.removeListener(mNetWatcherListener);
        }

        SimInterface sim = getSimInterface();
        if (sim != null) {
            sim.removeListener(mSimStateListener);
        }

        ConfigInterface config = getConfigInterface();
        if (config != null) {
            config.removeListener(mCarrierConfigListener);
        }

        // unregister listeners related to carrier configuration
        unregisterWifiConnectionStateChangedEvent();
        unregisterImsRegistrationStateListener();
        unregisterMobileDataStateListener(mSubId);
        unregisterCrossSimDataStateListener();

        mIsInitialized = false;
        notifyUtFeatureCapabilityChanged();
    }

    protected boolean isUtAvailable() {
        ImsLog.i(mSlotId, "isUtAvailable : " + mUtAvailability);
        return mUtAvailability;
    }

    /**
     * Updating Ut feature capability when capabilities are changed. Currently, radio technologies
     * of Uts are ignored because they're not updated properly. Only CrossSim technology for voice
     * is handled to check if Ut can support over CrossSim or not.
     * See also {@link com.android.ims.ImsManager#updateUtFeatureValue} and
     * {@link com.android.ims.ImsManager.updateCrossSimFeatureAndProvisionedValues}
     *
     * @param enabledCaps list of CapabilityPair of features which are enabled.
     * @param disabledCaps list of CapabilityPair of features which are disabled.
     */
    protected void changeCapabilities(List<CapabilityPair> enabledCaps,
            List<CapabilityPair> disabledCaps) {
        boolean utEnabled = enabledCaps.stream().anyMatch(capabilityPair ->
                capabilityPair.getCapability() == CAPABILITY_TYPE_UT);
        boolean utDisabled = disabledCaps.stream().anyMatch(capabilityPair ->
                capabilityPair.getCapability() == CAPABILITY_TYPE_UT);

        if (utEnabled) {
            ImsLog.d(mSlotId, "Ut enabled");
            mIsUtFeatureEnabled = true;
        } else if (utDisabled) {
            ImsLog.d(mSlotId, "Ut Disabled");
            mIsUtFeatureEnabled = false;
        }

        boolean crossSimEnabled = enabledCaps.stream().anyMatch(capabilityPair ->
                (capabilityPair.getCapability() == CAPABILITY_TYPE_VOICE)
                        && (capabilityPair.getRadioTech() == REGISTRATION_TECH_CROSS_SIM));
        boolean crossSimDisabled = disabledCaps.stream().anyMatch(capabilityPair ->
                (capabilityPair.getCapability() == CAPABILITY_TYPE_VOICE)
                        && (capabilityPair.getRadioTech() == REGISTRATION_TECH_CROSS_SIM));

        if (crossSimEnabled) {
            ImsLog.d(mSlotId, "CrossSim enabled");
            mIsCrossSimFeatureEnabled = true;
        } else if (crossSimDisabled) {
            ImsLog.d(mSlotId, "CrossSim disabled");
            mIsCrossSimFeatureEnabled = false;
        }

        handleUtFeatureCapabilityChanged();
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

    protected void resetAllUtStatus() {
        ImsLog.d("");

        mUtBlockReason = SscConstant.BLOCK_REASON_NONE;

        ISscAuthAgent authAgent = SscAuthAgent.getInstance(mSlotId);
        authAgent.setIsCredentialInfoUpdated(false);
        authAgent.setETag("");

        stopUtBlockTimer();
    }

    private boolean getCurrentUtAvailability() {
        if (!SscConfig.isUtSupported(mSlotId)) {
            ImsLog.i(mSlotId, "Ut not supported");
            return false;
        }

        if (!mIsInitialized) {
            ImsLog.i(mSlotId, "Not initialized");
            return false;
        }

        if (!mIsUtFeatureEnabled) {
            ImsLog.i(mSlotId, "Ut feature disabled");
            return false;
        }

        if (!SscConfig.isUtSupportedWhenPsDataOff(mSlotId)) {
            if (mMobileDataStateListener != null
                    && !mMobileDataStateListener.getUserMobileDataState()) {
                ImsLog.d(mSlotId, "PS Data off");
                return false;
            }
        }

        if (SscConfig.isImsRegistrationRequired(mSlotId)) {
            if (mRegiStateListener != null && !mRegiStateListener.getImsRegistrationState()) {
                ImsLog.d(mSlotId, "Ims not registered");
                return false;
            }
        }

        if (mUtBlockReason != SscConstant.BLOCK_REASON_NONE) {
            ImsLog.d(mSlotId, "mUtBlockReason = " + getBlockedReasonString(mUtBlockReason));
            return false;
        }

        if (!isValidNetwork()) {
            ImsLog.d(mSlotId, "invalid network");
            return false;
        }

        ImsLog.i(mSlotId, "Ut is available now");

        return true;
    }

    private boolean isValidNetwork() {
        if (SscConfig.isSupportedNetwork(mSlotId, SscConstant.NETWORK_TYPE_IWLAN)) {
            WifiInterface wifi = getWifiInterface();
            if (wifi != null && wifi.isWifiConnected()) {
                ImsLog.d(mSlotId, "support Ut over wifi");
                return true;
            }

            if (mIsCrossSimFeatureEnabled) {
                if (mCrossSimDataStateListener != null
                        && mCrossSimDataStateListener.getCrossSimDataState()) {
                    ImsLog.d(mSlotId, "support Ut over cross SIM");
                    return true;
                }
            }
        }

        IDcNetWatcher dnw = getDcNetWatcher();
        if (dnw == null) {
            ImsLog.d(mSlotId, "DcNetWatcher is null");
            return false;
        }

        if (dnw.getDataServiceState() != STATE_IN_SERVICE) {
            ImsLog.d(mSlotId, "Out of service");
            return false;
        }

        if (dnw.isRoaming()) {
            if (!SscConfig.isUtSupportedWhenRoaming(mSlotId)) {
                ImsLog.d(mSlotId, "Ut not supported when roaming");
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

        switch (blockReason) {
            case SscConstant.BLOCK_REASON_GBA_FAILURE : // fall-through
            case SscConstant.BLOCK_REASON_DNS_QUERY_FAILURE : // fall-through
            case SscConstant.BLOCK_REASON_SOCKET_CONNECTION_TIMEOUT : // fall-through
            case SscConstant.BLOCK_REASON_PDN_CONNECTION_TIMEOUT : {
                long blockTimeMilliSeconds = SscConfig.getTimerForTempBlockWithAnyReason(mSlotId);
                if (blockTimeMilliSeconds > 0) {
                    startUtBlockTimer(blockTimeMilliSeconds);
                    mUtBlockReason |= blockReason;
                    notifyUtFeatureCapabilityChanged();
                }
                break;
            }
            case SscConstant.BLOCK_REASON_PDN_CONNECTION_FAILURE_TEMP : // fall-through
            case SscConstant.BLOCK_REASON_BY_RESPONSE_CODE_TEMP : {
                long blockTimeMilliSeconds = SscConfig.getTimerForTempBlock(mSlotId);
                if (blockTimeMilliSeconds > 0) {
                    startUtBlockTimer(blockTimeMilliSeconds);
                    mUtBlockReason |= blockReason;
                    notifyUtFeatureCapabilityChanged();
                }
                break;
            }
            case SscConstant.BLOCK_REASON_PDN_CONNECTION_FAILURE_PERM : // fall-through
            case SscConstant.BLOCK_REASON_BY_RESPONSE_CODE_PERM : {
                // don't start timer
                mUtBlockReason |= blockReason;
                notifyUtFeatureCapabilityChanged();
                break;
            }
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

    private void startUtBlockTimer(long duration) {
        TimerInterface timer = getTimerInterface();
        if (timer == null) {
            ImsLog.e(mSlotId, "TimerInterface is null");
            return;
        }

        stopUtBlockTimer();

        mUtBlockTimerId = timer.startTimer(duration, mTimerListener);
        if (mUtBlockTimerId == TimerInterface.INVALID_TID) {
            ImsLog.e(mSlotId, "Starting Ut block timer failed");
            return;
        }

        ImsLog.i(mSlotId, "Ut block timer is started :: "
                + "tid[" + mUtBlockTimerId + "], duration[" + duration + "]");
    }

    private void stopUtBlockTimer() {
        if (mUtBlockTimerId == TimerInterface.INVALID_TID) {
            return;
        }

        TimerInterface timer = getTimerInterface();
        if (timer != null) {
            timer.stopTimer(mUtBlockTimerId);
        }
        mUtBlockTimerId = TimerInterface.INVALID_TID;
    }

    private TimerInterface getTimerInterface() {
        return AgentFactory.getInstance().getAgent(TimerInterface.class);
    }

    private WifiInterface getWifiInterface() {
        return AgentFactory.getInstance().getAgent(WifiInterface.class);
    }

    private SimInterface getSimInterface() {
        return AgentFactory.getInstance().getAgent(SimInterface.class, mSlotId);
    }

    private ConfigInterface getConfigInterface() {
        return AgentFactory.getInstance().getAgent(ConfigInterface.class, mSlotId);
    }

    private IAosRegistration getAosRegistration() {
        return AosFactory.getInstance().getAosRegistration(mSlotId);
    }

    private IDcNetWatcher getDcNetWatcher() {
        return DcFactory.getDcAgent(IDcNetWatcher.class, mSlotId);
    }

    private void registerWifiConnectionStateChangedEvent() {
        if (!SscConfig.isSupportedNetwork(mSlotId, SscConstant.NETWORK_TYPE_IWLAN)) {
            return;
        }

        if (mWifiListener == null) {
            mWifiListener = new SscWifiListener();
        }

        WifiInterface wifi = getWifiInterface();
        if (wifi != null) {
            wifi.addListener(mWifiListener);
        }
    }

    private void unregisterWifiConnectionStateChangedEvent() {
        if (mWifiListener != null) {
            WifiInterface wifi = getWifiInterface();
            if (wifi != null) {
                wifi.removeListener(mWifiListener);
            }
            mWifiListener = null;
        }
    }

    private void registerImsRegistrationStateListener() {
        if (!SscConfig.isImsRegistrationRequired(mSlotId)) {
            return;
        }

        IAosRegistration aosService = getAosRegistration();
        if (aosService != null) {
            boolean isRegistered =
                    aosService.getRegistrationState() == RegistrationState.REGISTERED;
            mRegiStateListener =  new SscRegiStateListener(isRegistered);
            aosService.addListener(mRegiStateListener);
        }
    }

    private void unregisterImsRegistrationStateListener() {
        if (mRegiStateListener == null) {
            return;
        }

        IAosRegistration aosService = getAosRegistration();
        if (aosService != null) {
            aosService.removeListener(mRegiStateListener);
            mRegiStateListener = null;
        }
    }

    private void registerMobileDataStateListener(int subId) {
        if (SscConfig.isUtSupportedWhenPsDataOff(mSlotId)) {
            return;
        }

        if (subId != MSimUtils.INVALID_SUB_ID) {
            TelephonyManagerProxy tmp = AppContext.getTelephonyManagerProxy(subId);
            mMobileDataStateListener = new SscMobileDataStateListener();
            tmp.registerTelephonyCallback(AppContext.getInstance().getMainExecutor(),
                    mMobileDataStateListener);
        }
    }

    private void unregisterMobileDataStateListener(int subId) {
        if (mMobileDataStateListener == null) {
            return;
        }

        if (subId != MSimUtils.INVALID_SUB_ID) {
            TelephonyManagerProxy tmp = AppContext.getTelephonyManagerProxy(subId);
            tmp.unregisterTelephonyCallback(mMobileDataStateListener);
            mMobileDataStateListener = null;
        }
    }

    private void registerCrossSimDataStateListener() {
        if (!SscConfig.isCrossSimFeatureEnabled(mSlotId)) {
            return;
        }

        ConnectivityManagerProxy cmp =
                AppContext.getInstance().getSystemServiceProxy(ConnectivityManagerProxy.class);
        try {
            mCrossSimDataStateListener = new SscCrossSimDataStateListener();
            cmp.registerDefaultNetworkCallback(mCrossSimDataStateListener, mHandler);
        } catch (Exception e) {
            ImsLog.e(mSlotId, e.toString());
        }
    }

    private void unregisterCrossSimDataStateListener() {
        if (mCrossSimDataStateListener == null) {
            return;
        }

        ConnectivityManagerProxy cmp =
                AppContext.getInstance().getSystemServiceProxy(ConnectivityManagerProxy.class);
        try {
            cmp.unregisterNetworkCallback(mCrossSimDataStateListener);
            mCrossSimDataStateListener = null;
        } catch (Exception e) {
            ImsLog.e(mSlotId, e.toString());
        }
    }

    private synchronized void updateSubscription(int subId) {
        ImsLog.d(mSlotId, "old subId = " + mSubId + ", new subId = " + subId);

        if (mSubId == subId || subId == MSimUtils.INVALID_SUB_ID) {
            return;
        }

        unregisterMobileDataStateListener(mSubId);

        mSubId = subId;
        registerMobileDataStateListener(mSubId);
    }

    private void handleAirplaneModeChanged() {
        IDcNetWatcher dnw = getDcNetWatcher();
        if (dnw != null && dnw.isAirplaneMode()) {
            // Reset permanent block reasons and PDN dependent block reasons.
            resetUtBlockReason(SscConstant.BLOCK_REASON_PDN_CONNECTION_FAILURE_PERM);
            resetUtBlockReason(SscConstant.BLOCK_REASON_PDN_CONNECTION_FAILURE_TEMP);
            resetUtBlockReason(SscConstant.BLOCK_REASON_BY_RESPONSE_CODE_PERM);

            handleUtFeatureCapabilityChanged();
        }
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
                case EVENT_AIRPLANE_MODE_CHANGED:
                    handleAirplaneModeChanged();
                    break;
                case EVENT_DATA_SERVICE_STATE_CHANGED: // FALL-THROUGH
                case EVENT_DATA_NETWORK_TYPE_CHANGED: // FALL-THROUGH
                case EVENT_DATA_ROAMING_STATE_CHANGED: // FALL-THROUGH
                case EVENT_CROSS_SIM_DATA_STATE_CHANGED:
                    handleUtFeatureCapabilityChanged();
                    break;
                default:
                    ImsLog.e(mSlotId, "Invalid Message");
                    break;
            }
        }
    }

    private final class SscTimerListener implements TimerInterface.Listener {
        @Override
        public void onTimerExpired(long tid) {
            mHandler.post(() -> {
                if (tid == mUtBlockTimerId) {
                    int allTempBlockReasons = SscConstant.BLOCK_REASON_DNS_QUERY_FAILURE
                            | SscConstant.BLOCK_REASON_PDN_CONNECTION_TIMEOUT
                            | SscConstant.BLOCK_REASON_SOCKET_CONNECTION_TIMEOUT
                            | SscConstant.BLOCK_REASON_PDN_CONNECTION_FAILURE_TEMP
                            | SscConstant.BLOCK_REASON_GBA_FAILURE
                            | SscConstant.BLOCK_REASON_BY_RESPONSE_CODE_TEMP;
                    resetUtBlockReason(allTempBlockReasons);
                }
            });
        }
    }

    private final class SscSimStateListener implements Sim.Listener {

        @Override
        public void onSimCardStateChanged() {
            SimInterface sim = getSimInterface();
            if (sim != null) {
                int simCardState = sim.getSimCardState();
                ImsLog.d(mSlotId, Sim.stateToString(simCardState));

                if (simCardState == Sim.STATE_ABSENT) {
                    resetAllUtStatus();
                    handleUtFeatureCapabilityChanged();
                } else if (simCardState == Sim.STATE_LOADED) {
                    updateSubscription(sim.getSubId());
                    handleUtFeatureCapabilityChanged();
                }
            }
        }
    }

    private final class SscCarrierConfigListener implements ConfigInterface.Listener {

        @Override
        public void onCarrierConfigChanged(int slotId, int subId) {
            ImsLog.d(mSlotId, "slotId : " + slotId + ", subId : " + subId);

            unregisterWifiConnectionStateChangedEvent();
            unregisterImsRegistrationStateListener();
            unregisterMobileDataStateListener(mSubId);
            unregisterCrossSimDataStateListener();

            registerWifiConnectionStateChangedEvent();
            registerImsRegistrationStateListener();
            registerMobileDataStateListener(mSubId);
            registerCrossSimDataStateListener();

            handleUtFeatureCapabilityChanged();
        }
    }

    private final class SscWifiListener implements WifiInterface.Listener {
        @Override
        public void onWifiConnectionStateChanged() {
            handleUtFeatureCapabilityChanged();
        }
    }

    @VisibleForTesting
    final class SscRegiStateListener implements IAosRegistrationListener {
        @VisibleForTesting
        boolean mImsRegistrationState;

        SscRegiStateListener(boolean isRegistered) {
            mImsRegistrationState = isRegistered;
        }

        @Override
        public void notifyRegistered(int regType, NetworkType networkType, int featureTagBits,
                java.util.Set<String> featureTags) {
            if (regType != RegistrationType.NORMAL) {
                return;
            }

            ImsLog.d(mSlotId, "Registered : network = " + networkType.toString());

            if (!mImsRegistrationState) {
                mImsRegistrationState = true;
                handleUtFeatureCapabilityChanged();
            }
        }

        @Override
        public void notifyRegistering(int regType, NetworkType networkType, int featureTagBits,
                java.util.Set<String> featureTags) {
            if (regType != RegistrationType.NORMAL) {
                return;
            }

            // do nothing
        }

        @Override
        public void notifyDeregistered(
                int regType, NetworkType networkType, ReasonCode reason, String message) {
            if (regType != RegistrationType.NORMAL) {
                return;
            }

            ImsLog.d(mSlotId, "Deregistered : reason = " + reason.toString());

            if (mImsRegistrationState) {
                mImsRegistrationState = false;
                handleUtFeatureCapabilityChanged();
            }
        }

        @Override
        public void notifyTechnologyChangeFailed(
                int regType, NetworkType networkType, ReasonCode reason, String message) {
            if (regType != RegistrationType.NORMAL) {
                return;
            }

            // do nothing
        }

        @Override
        public void notifyAssociatedUriChanged(android.net.Uri[] uris) {
            // do nothing
        }

        @Override
        public void notifyCapabilitiesUpdateFailed(
                int capabilities, NetworkType networkType, int reason) {
            // do nothing
        }

        @Override
        public void notifyCapabilitiesUpdated(IAosRegistration.CapabilityPairs pairs) {
            // Do nothing.
        }

        @Override
        public void notifyRegEventStateChanged(int statusCode, @NonNull Set<Uri> impus){
            // do nothing
        }

        public boolean getImsRegistrationState() {
            return mImsRegistrationState;
        }
    }

    @VisibleForTesting
    final class SscMobileDataStateListener extends TelephonyCallback implements
            TelephonyCallback.UserMobileDataStateListener {

        @VisibleForTesting
        boolean mMobileDataState = false;

        @Override
        public void onUserMobileDataStateChanged(boolean enabled) {
            ImsLog.d(mSlotId, "Mobile Data switch = " + enabled);

            if (mMobileDataState == enabled) {
                return;
            }

            mMobileDataState = enabled;
            handleUtFeatureCapabilityChanged();
        }

        public boolean getUserMobileDataState() {
            return mMobileDataState;
        }
    }

    final class SscCrossSimDataStateListener extends ConnectivityManager.NetworkCallback {

        @VisibleForTesting
        boolean mCrossSimDataAvailable = false;

        @Override
        public void onLost(Network network) {
            if (!mCrossSimDataAvailable) {
                return;
            }

            ImsLog.d(mSlotId, "Cross SIM data not available : " + network);

            mCrossSimDataAvailable = false;
            // TODO: call handleUtFeatureCapabilityChanged here.
            mHandler.sendEmptyMessage(EVENT_CROSS_SIM_DATA_STATE_CHANGED);
        }

        @Override
        public void onCapabilitiesChanged(Network network, NetworkCapabilities capabilities) {
            boolean isAvailable = isCrossSimDataAvailable(capabilities);
            if (mCrossSimDataAvailable == isAvailable) {
                return;
            }

            ImsLog.d(mSlotId, "network = " + network + ", capabilities = " + capabilities);

            mCrossSimDataAvailable = isAvailable;
            // TODO: call handleUtFeatureCapabilityChanged here.
            mHandler.sendEmptyMessage(EVENT_CROSS_SIM_DATA_STATE_CHANGED);
        }

        private boolean isCrossSimDataAvailable(NetworkCapabilities networkCapabilities) {
            if (!networkCapabilities.hasTransport(NetworkCapabilities.TRANSPORT_CELLULAR)) {
                return false;
            }

            NetworkSpecifier specifier = networkCapabilities.getNetworkSpecifier();
            if (!(specifier instanceof TelephonyNetworkSpecifier)) {
                return false;
            }

            int connectedSubId = ((TelephonyNetworkSpecifier) specifier).getSubscriptionId();
            if (connectedSubId == mSubId) {
                return false;
            }

            ImsLog.d(mSlotId, "Cross SIM data available");
            return true;
        }

        public boolean getCrossSimDataState() {
            return mCrossSimDataAvailable;
        }
    }

    private final class NetWatcherListener implements IDcNetWatcher.Listener {
        @Override
        public void onDataServiceStateChanged(int state) {
            mHandler.sendEmptyMessage(EVENT_DATA_SERVICE_STATE_CHANGED);
        }

        @Override
        public void onDataNetworkTypeChanged() {
            mHandler.sendEmptyMessage(EVENT_DATA_NETWORK_TYPE_CHANGED);
        }

        @Override
        public void onRoamingStateChanged(boolean roaming) {
            if (SscConfig.isUtSupportedWhenRoaming(mSlotId)) {
                return;
            }
            mHandler.sendEmptyMessage(EVENT_DATA_ROAMING_STATE_CHANGED);
        }

        @Override
        public void onAirplaneModeChanged(boolean airplaneMode) {
            mHandler.sendEmptyMessage(EVENT_AIRPLANE_MODE_CHANGED);
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
