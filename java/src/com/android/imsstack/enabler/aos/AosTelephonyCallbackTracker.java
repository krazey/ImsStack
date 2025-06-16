/*
 * Copyright (C) 2025 The Android Open Source Project
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

package com.android.imsstack.enabler.aos;

import android.os.Handler;
import android.os.Looper;
import android.telephony.Annotation.CallState;
import android.telephony.SecurityAlgorithmUpdate;
import android.telephony.TelephonyCallback;
import android.telephony.TelephonyManager;
import android.telephony.emergency.EmergencyNumber;

import androidx.annotation.NonNull;
import androidx.annotation.VisibleForTesting;

import com.android.imsstack.base.AppContext;
import com.android.imsstack.base.MSimUtils;
import com.android.imsstack.base.TelephonyManagerProxy;
import com.android.imsstack.core.agents.AgentFactory;
import com.android.imsstack.core.agents.ConfigInterface;
import com.android.imsstack.core.agents.Sim;
import com.android.imsstack.core.agents.SimInterface;
import com.android.imsstack.core.agents.TelephonyInterface;
import com.android.imsstack.core.config.CarrierConfig;
import com.android.imsstack.util.ImsLog;

/**
 * This class provides the information through the {@link TelephonyCallback}.
 */
public class AosTelephonyCallbackTracker {
    private final Object mLock = new Object();
    private final int mSlotId;
    private int mSubId = MSimUtils.INVALID_SUB_ID;

    private Handler mHandler;
    private Sim.Listener mSimListener;
    private boolean mNullIntegrityAlgorithm = false;
    private int mCallState = TelephonyManager.CALL_STATE_IDLE;

    @VisibleForTesting
    protected EmergencyCallListener mEmergencyCallListener;
    @VisibleForTesting
    protected CallStateListener mCallStateListener;
    @VisibleForTesting
    protected SecurityAlgorithmsListener mSecurityAlgorithmsListener;

    AosTelephonyCallbackTracker(int slotId) {
        mSlotId = slotId;
    }

    /**
     * Initializes the internal resources.
     */
    public void init() {
        mHandler = new Handler(Looper.myLooper());
        mSimListener = new Sim.Listener() {
                @Override
                public void onSimStateChanged() {
                    handleSimStateChanged();
                }
            };
        SimInterface sim = AgentFactory.getInstance().getAgent(SimInterface.class, mSlotId);
        if (sim != null) {
            sim.addListener(mSimListener);
        }
        mSubId = MSimUtils.getSubId(mSlotId);
        if (mSubId != MSimUtils.INVALID_SUB_ID) {
            registerForEmergencyCall();
        }
    }

    /**
     * Clears all the resources
     */
    public void cleanup() {
        if (mSimListener != null) {
            SimInterface sim = AgentFactory.getInstance().getAgent(SimInterface.class, mSlotId);
            if (sim != null) {
                sim.removeListener(mSimListener);
            }
            mSimListener = null;
        }

        unregisterTelephonyCallbacks();

        if (mHandler != null) {
            mHandler.removeCallbacksAndMessages(null);
            mHandler = null;
        }
    }

    private void handleSimStateChanged() {
        SimInterface sim = AgentFactory.getInstance().getAgent(SimInterface.class, mSlotId);
        if (sim == null || !sim.isSimLoadCompleted()) {
            return;
        }

        synchronized (mLock) {
            int subId = sim.getSubId();

            if (mSubId == subId || subId == MSimUtils.INVALID_SUB_ID) {
                return;
            }

            ImsLog.i(this, mSlotId, "handleSimStateChanged: subId=" + subId);
            mSubId = subId;

            registerForEmergencyCall();

            if (mSecurityAlgorithmsListener != null) {
                unregisterForSecurityAlgorithms();
                registerForSecurityAlgorithms();
            }
        }
    }

    private void handleIdleCallState() {
        unregisterForCallState();
        unregisterForSecurityAlgorithms();

        IAosInfo aosInfo = AosFactory.getInstance().getAosInfo(mSlotId);
        if (aosInfo != null) {
            aosInfo.notifyNasSecurityAlgorithmChanged(false);
            mNullIntegrityAlgorithm = false;
        }
    }

    private void handleOutgoingEmergencyCall() {
        if (!isNasSecurityAlgorithmNotificationRequired()) {
            return;
        }

        if (MSimUtils.isValidSubId(mSubId)) {
            registerForCallState();
            registerForSecurityAlgorithms();
        }
    }

    private void handleSecurityAlgorithmsChanged(int algo) {
        IAosInfo aosInfo = AosFactory.getInstance().getAosInfo(mSlotId);
        if (aosInfo == null) {
            return;
        }

        boolean nullAlgo = isNullIntegrityAlgorithm(algo);
        if (mNullIntegrityAlgorithm != nullAlgo) {
            aosInfo.notifyNasSecurityAlgorithmChanged(nullAlgo);
            mNullIntegrityAlgorithm = nullAlgo;
        }
    }

    private boolean isConnectionEventForNas(int event) {
        if (event == SecurityAlgorithmUpdate.CONNECTION_EVENT_NAS_SIGNALLING_LTE) {
            return true;
        }
        if (event == SecurityAlgorithmUpdate.CONNECTION_EVENT_NAS_SIGNALLING_5G) {
            return true;
        }
        return false;
    }

    private boolean isNullIntegrityAlgorithm(int algo) {
        if (algo == SecurityAlgorithmUpdate.SECURITY_ALGORITHM_EEA0) {
            return true;
        }
        if (algo == SecurityAlgorithmUpdate.SECURITY_ALGORITHM_NEA0) {
            return true;
        }
        return false;
    }

    private boolean isNasSecurityAlgorithmNotificationRequired() {
        CarrierConfig cc = getCarrierConfig(mSlotId);
        return cc != null && cc.getBoolean(
                CarrierConfig.ImsEmergency.KEY_SUPPORT_EREG_WHEN_EATTACH_WITH_VALID_SIM_BOOL);
    }

    @VisibleForTesting
    protected void registerForCallState() {
        if (mCallStateListener == null) {
            mCallStateListener = new CallStateListener();
            mCallStateListener.register();
        }
    }

    private void unregisterForCallState() {
        if (mCallStateListener != null) {
            mCallStateListener.unregister();
            mCallStateListener = null;
        }
    }

    @VisibleForTesting
    protected void registerForEmergencyCall() {
        if (mEmergencyCallListener == null) {
            mEmergencyCallListener = new EmergencyCallListener();
            mEmergencyCallListener.register();
        }
    }

    private void unregisterForEmergencyCall() {
        if (mEmergencyCallListener != null) {
            mEmergencyCallListener.unregister();
            mEmergencyCallListener = null;
        }
    }

    @VisibleForTesting
    protected void registerForSecurityAlgorithms() {
        if (mSecurityAlgorithmsListener == null) {
            mSecurityAlgorithmsListener = new SecurityAlgorithmsListener(mSubId);
            mSecurityAlgorithmsListener.register();
        }
    }

    private void unregisterForSecurityAlgorithms() {
        if (mSecurityAlgorithmsListener != null) {
            mSecurityAlgorithmsListener.unregister();
            mSecurityAlgorithmsListener = null;
        }
    }

    private void unregisterTelephonyCallbacks() {
        unregisterForCallState();
        unregisterForEmergencyCall();
        unregisterForSecurityAlgorithms();
    }

    @VisibleForTesting
    protected CarrierConfig getCarrierConfig(int slotId) {
        ConfigInterface ci = AgentFactory.getInstance().getAgent(ConfigInterface.class, slotId);
        return (ci != null) ? ci.getCarrierConfig() : null;
    }

    private final class CallStateListener extends TelephonyCallback implements
            TelephonyCallback.CallStateListener {

        public void register() {
            TelephonyManagerProxy tmp = AppContext.getInstance().getSystemServiceProxy(
                    TelephonyManagerProxy.class);
            tmp.registerTelephonyCallback(mHandler::post, this);
        }

        public void unregister() {
            TelephonyManagerProxy tmp = AppContext.getInstance().getSystemServiceProxy(
                    TelephonyManagerProxy.class);
            tmp.unregisterTelephonyCallback(this);
        }

        @Override
        public void onCallStateChanged(@CallState int state) {
            ImsLog.i(this, mSlotId, "onCallStateChanged: "
                    + TelephonyInterface.callStateToString(mCallState) + " -> "
                    + TelephonyInterface.callStateToString(state));

            if (mCallState == state) {
                return;
            }
            mCallState = state;
            if (state == TelephonyManager.CALL_STATE_IDLE) {
                handleIdleCallState();
            }
        }
    }

    private class EmergencyCallListener extends TelephonyCallback implements
            TelephonyCallback.OutgoingEmergencyCallListener {

        public void register() {
            TelephonyManagerProxy tmp = AppContext.getInstance().getSystemServiceProxy(
                    TelephonyManagerProxy.class);
            tmp.registerTelephonyCallback(mHandler::post, this);
        }

        public void unregister() {
            TelephonyManagerProxy tmp = AppContext.getInstance().getSystemServiceProxy(
                    TelephonyManagerProxy.class);
            tmp.unregisterTelephonyCallback(this);
        }

        @Override
        public void onOutgoingEmergencyCall(@NonNull EmergencyNumber placedEmergencyNumber,
                int subscriptionId) {
            ImsLog.d(this, mSlotId, "onOutgoingEmergencyCall: subId=" + subscriptionId);

            if (subscriptionId == MSimUtils.INVALID_SUB_ID) {
                return;
            }

            if (mSlotId != MSimUtils.getSlotId(subscriptionId)) {
                return;
            }

            if (mSubId == MSimUtils.INVALID_SUB_ID) {
                return;
            }

            mHandler.post(() -> handleOutgoingEmergencyCall());
        }
    }

    private final class SecurityAlgorithmsListener extends TelephonyCallback implements
            TelephonyCallback.SecurityAlgorithmsListener {
        private final int mSubId;

        SecurityAlgorithmsListener(int subId) {
            mSubId = subId;
        }

        public void register() {
            TelephonyManagerProxy tmp = AppContext.getTelephonyManagerProxy(mSubId);
            tmp.registerTelephonyCallback(mHandler::post, this);
        }

        public void unregister() {
            TelephonyManagerProxy tmp = AppContext.getTelephonyManagerProxy(mSubId);
            tmp.unregisterTelephonyCallback(this);
        }

        @Override
        public void onSecurityAlgorithmsChanged(
                @NonNull SecurityAlgorithmUpdate securityAlgorithmUpdate) {
            ImsLog.i(this, mSlotId, "onSecurityAlgorithmsChanged: sa="
                    + securityAlgorithmUpdate);

            if (!isConnectionEventForNas(securityAlgorithmUpdate.getConnectionEvent())) {
                return;
            }

            mHandler.post(() -> handleSecurityAlgorithmsChanged(
                    securityAlgorithmUpdate.getIntegrity()));
        }
    }
}
