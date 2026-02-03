/*
 * Copyright (C) 2026 The Android Open Source Project
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

package com.android.imsstack.core.agents;

import android.annotation.SuppressLint;
import android.content.Context;
import android.os.Handler;
import android.telephony.TelephonyCallback;
import android.telephony.TelephonyManager;

import androidx.annotation.NonNull;

import com.android.imsstack.base.AppContext;
import com.android.imsstack.base.MSimUtils;
import com.android.imsstack.base.TelephonyManagerProxy;
import com.android.imsstack.util.ImsLog;

import java.time.Duration;
import java.util.HashMap;
import java.util.Map;
import java.util.Set;
import java.util.concurrent.CopyOnWriteArraySet;

/**
 * A class for providing an interface to monitor the emergency state.
 */
public class EmergencyStateAgent implements EmergencyStateInterface {
    private final Object mLock = new Object();
    private final Set<EmergencyStateListener> mListeners =
            new CopyOnWriteArraySet<EmergencyStateListener>();
    private final int mSlotId;
    private Sim.Listener mSimListener;
    private EmergencyModeListener mEmergencyModeListener;
    private final Set<Integer> mEmergencyModes = new CopyOnWriteArraySet<>();
    private final Map<Integer, Long> mEmergencyCallbackModes = new HashMap<>();

    public EmergencyStateAgent(int slotId) {
        mSlotId = slotId;
    }

    @Override
    public void init(Context context) {
        mEmergencyModeListener = new EmergencyModeListener(MSimUtils.getSubId(mSlotId));
        mEmergencyModeListener.registerCallbacks();

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
    }

    @Override
    public void cleanup() {
        if (mSimListener != null) {
            SimInterface sim = AgentFactory.getInstance().getAgent(SimInterface.class, mSlotId);
            if (sim != null) {
                sim.removeListener(mSimListener);
            }
            mSimListener = null;
        }

        mListeners.clear();

        if (mEmergencyModeListener != null) {
            mEmergencyModeListener.unregisterCallbacks();
            mEmergencyModeListener.dispose();
            mEmergencyModeListener = null;
        }

        mEmergencyModes.clear();
        mEmergencyCallbackModes.clear();
    }

    @Override
    public void addListener(@NonNull EmergencyStateListener listener) {
        boolean isChanged = mListeners.add(listener);

        if (isChanged) {
            notifyCurrentStateIfPresent(listener);
        }
    }

    @Override
    public void removeListener(@NonNull EmergencyStateListener listener) {
        mListeners.remove(listener);
    }

    @Override
    public boolean isInEmergencyMode(int type) {
        return mEmergencyModes.contains(type);
    }

    private boolean isInEmergencyCallbackMode(int type) {
        return mEmergencyCallbackModes.containsKey(type);
    }

    private void notifyEmergencyModeChanged(
            @TelephonyManager.DomainSelectionEmergencyType int type, boolean entered) {
        for (EmergencyStateListener l : mListeners) {
            l.onEmergencyModeChanged(type, entered);
        }
    }

    private void notifyEmergencyCallbackModeChanged(
            @TelephonyManager.EmergencyCallbackModeType int type,
            EmergencyCallbackModeState state, long duration) {
        for (EmergencyStateListener l : mListeners) {
            l.onEmergencyCallbackModeChanged(type, state, duration);
        }
    }

    private void notifyCurrentStateIfPresent(EmergencyStateListener listener) {
        for (int type : mEmergencyModes) {
            listener.onEmergencyModeChanged(type, true);
        }

        for (Map.Entry<Integer, Long> entry : mEmergencyCallbackModes.entrySet()) {
            listener.onEmergencyCallbackModeChanged(
                    entry.getKey(), EmergencyCallbackModeState.START, entry.getValue());
        }
    }

    private void handleSimStateChanged() {
        SimInterface sim = AgentFactory.getInstance().getAgent(SimInterface.class, mSlotId);

        // Don't check sim.isSimLoadCompleted() since the emergency call can be initiated w/o UICC.
        if (sim == null) {
            return;
        }

        synchronized (mLock) {
            int subId = sim.getSubId();

            if (subId == mEmergencyModeListener.getSubId()) {
                // no-op
                ImsLog.w(this, mSlotId, "Subscription is not changed; subId=" + subId);
                return;
            }

            ImsLog.i(this, mSlotId, "handleSimStateChanged: subId=" + subId);
            // Do not clear mEmergencyModes or mEmergencyCallbackModes here. Both are managed based
            // on the SIM slot rather than the Subscription ID; therefore, these states must persist
            // across Subscription ID changes.
            mEmergencyModeListener.unregisterCallbacks();
            mEmergencyModeListener.dispose();

            mEmergencyModeListener = new EmergencyModeListener(subId);
            mEmergencyModeListener.registerCallbacks();
        }
    }

    private static TelephonyManagerProxy getTelephonyManagerProxy(int subId) {
        if (!MSimUtils.isValidSubId(subId)) {
            return AppContext.getInstance().getSystemServiceProxy(TelephonyManagerProxy.class);
        }

        return AppContext.getTelephonyManagerProxy(subId);
    }

    @SuppressLint("HandlerLeak")
    private final class EmergencyModeListener extends Handler {
        private final int mSubId;
        private final DomainSelectionEmergencyModeListener mDomainSelectionListener =
                new DomainSelectionEmergencyModeListener();
        private final EmergencyCallbackModeListener mEmergencyCallbackModeListener =
                new EmergencyCallbackModeListener();

        EmergencyModeListener(int subId) {
            super(AppContext.getInstance().getMainLooper());
            mSubId = subId;
            ImsLog.d(this, mSlotId, "EmergencyModeListener: subId=" + subId);
        }

        public void dispose() {
            removeCallbacksAndMessages(null);
        }

        public int getSubId() {
            return mSubId;
        }

        public void registerCallbacks() {
            TelephonyManagerProxy tmp = getTelephonyManagerProxy(getSubId());
            tmp.registerTelephonyCallback(this::post, mDomainSelectionListener);
            tmp.registerTelephonyCallback(this::post, mEmergencyCallbackModeListener);
        }

        public void unregisterCallbacks() {
            TelephonyManagerProxy tmp = getTelephonyManagerProxy(getSubId());
            tmp.unregisterTelephonyCallback(mDomainSelectionListener);
            tmp.unregisterTelephonyCallback(mEmergencyCallbackModeListener);
        }

        private final class DomainSelectionEmergencyModeListener extends TelephonyCallback
                implements TelephonyCallback.DomainSelectionEmergencyModeListener {
            @Override
            public void onDomainSelectionEmergencyModeEntered(
                    @TelephonyManager.DomainSelectionEmergencyType int type,
                    int slotIndex, int subscriptionId) {
                if (slotIndex != mSlotId) {
                    return;
                }
                if (isInEmergencyMode(type)) {
                    ImsLog.d(this, mSlotId, "Same ECM state: type=" + type);
                    return;
                }
                ImsLog.i(this, mSlotId, "onDomainSelectionEmergencyModeEntered: type=" + type
                        + ", subId=" + subscriptionId);
                mEmergencyModes.add(type);
                notifyEmergencyModeChanged(type, true);
            }

            @Override
            public void onDomainSelectionEmergencyModeExited(
                    @TelephonyManager.DomainSelectionEmergencyType int type,
                    int slotIndex, int subscriptionId) {
                if (slotIndex != mSlotId) {
                    return;
                }
                if (!isInEmergencyMode(type)) {
                    ImsLog.d(this, mSlotId, "Same ECM state: type=" + type);
                    return;
                }
                ImsLog.i(this, mSlotId, "onDomainSelectionEmergencyModeExited: type=" + type
                        + ", subId=" + subscriptionId);
                mEmergencyModes.remove(type);
                notifyEmergencyModeChanged(type, false);
            }
        }

        private final class EmergencyCallbackModeListener extends TelephonyCallback implements
                TelephonyCallback.EmergencyCallbackModeListener {
            @Override
            public void onCallbackModeStarted(@TelephonyManager.EmergencyCallbackModeType int type,
                    @NonNull Duration timerDuration, int subId) {
                if (subId != MSimUtils.INVALID_SUB_ID && subId != mSubId) {
                    return;
                }

                if (isInEmergencyCallbackMode(type)) {
                    ImsLog.d(this, mSlotId, "Same ECBM state: type=" + type);
                    return;
                }

                ImsLog.i(this, mSlotId, "onCallbackModeStarted() type: " + type);
                long duration = timerDuration.toSeconds();
                mEmergencyCallbackModes.put(type, duration);
                notifyEmergencyCallbackModeChanged(
                        type, EmergencyCallbackModeState.START, duration);
            }

            @Override
            public void onCallbackModeRestarted(
                    @TelephonyManager.EmergencyCallbackModeType int type,
                    @NonNull Duration timerDuration, int subId) {
                if (subId != MSimUtils.INVALID_SUB_ID && subId != mSubId) {
                    return;
                }

                ImsLog.i(this, mSlotId, "onCallbackModeRestarted() type: " + type);
                mEmergencyCallbackModes.put(type, timerDuration.toSeconds());
                notifyEmergencyCallbackModeChanged(
                        type, EmergencyCallbackModeState.START, timerDuration.toSeconds());
            }

            @Override
            public void onCallbackModeStopped(@TelephonyManager.EmergencyCallbackModeType int type,
                    @TelephonyManager.EmergencyCallbackModeStopReason int reason, int subId) {
                if (subId != MSimUtils.INVALID_SUB_ID && subId != mSubId) {
                    return;
                }

                ImsLog.i(this, mSlotId, "onCallbackModeStopped() type: " + type + ",reason: "
                        + reason);

                mEmergencyCallbackModes.remove(type);
                EmergencyCallbackModeState state = EmergencyCallbackModeState.STOP;
                if (reason == TelephonyManager.STOP_REASON_OUTGOING_EMERGENCY_CALL_INITIATED
                        || reason == TelephonyManager.STOP_REASON_EMERGENCY_SMS_SENT) {
                    state = EmergencyCallbackModeState.STOP_BY_EMERGENCY;
                }
                notifyEmergencyCallbackModeChanged(type, state, 0);
            }
        }
    }
}
