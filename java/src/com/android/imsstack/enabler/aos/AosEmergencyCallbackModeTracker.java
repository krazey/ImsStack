/*
 * Copyright (C) 2024 The Android Open Source Project
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
import android.telephony.TelephonyCallback;
import android.telephony.TelephonyManager;

import androidx.annotation.NonNull;

import com.android.imsstack.base.AppContext;
import com.android.imsstack.base.MSimUtils;
import com.android.imsstack.base.TelephonyManagerProxy;
import com.android.imsstack.core.agents.AgentFactory;
import com.android.imsstack.core.agents.Sim;
import com.android.imsstack.core.agents.SimInterface;
import com.android.imsstack.enabler.aos.IAosInfo.EmergencyCallbackModeState;
import com.android.imsstack.enabler.aos.IAosInfo.EmergencyCallbackModeType;
import com.android.imsstack.util.ImsLog;

import java.time.Duration;

/**
 * This class provides emergency callback mode information through a {@link TelephonyCallback} for
 * emergency callback mode.
 */
public class AosEmergencyCallbackModeTracker {
    private final int mSlotId;
    private int mSubId = MSimUtils.INVALID_SUB_ID;

    private Handler mHandler;
    private Sim.Listener mSimListener;
    private TelephonyCallback mTelephonyCallback;

    AosEmergencyCallbackModeTracker(int slotId) {
        mSlotId = slotId;
    }

    /**
     * Initializes the internal resources.
     */
    public void init() {
        mHandler = new Handler(Looper.myLooper());
        mSubId = MSimUtils.getSubId(mSlotId);
        registerTelephonyCallback();

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

        unregisterTelephonyCallback();

        if (mHandler != null) {
            mHandler.removeCallbacksAndMessages(null);
            mHandler = null;
        }
    }

    private void updateEmergencyCallbackMode(@TelephonyManager.EmergencyCallbackModeType int type,
            EmergencyCallbackModeState state, long duration) {

        EmergencyCallbackModeType emergencyCbmType =
                (type == TelephonyManager.EMERGENCY_CALLBACK_MODE_CALL)
                ? EmergencyCallbackModeType.CALL : EmergencyCallbackModeType.SMS;

        IAosInfo aosInfo = AosFactory.getInstance().getAosInfo(mSlotId);
        if (aosInfo != null) {
            aosInfo.notifyEmergencyCallbackModeChanged(emergencyCbmType, state, duration);
        }
    }

    private void handleSimStateChanged() {
        SimInterface sim = AgentFactory.getInstance().getAgent(SimInterface.class, mSlotId);

        if (sim == null || !sim.isSimLoadCompleted()) {
            return;
        }

        int subId = sim.getSubId();

        if (mSubId == subId || subId == MSimUtils.INVALID_SUB_ID) {
            return;
        }

        ImsLog.i(this, mSlotId, "handleSimStateChanged: subId=" + subId);
        unregisterTelephonyCallback();
        mSubId = subId;
        registerTelephonyCallback();
    }

    private void registerTelephonyCallback() {
        if (mTelephonyCallback == null) {
            mTelephonyCallback = new EmergencyCallbackModeCallback();
            TelephonyManagerProxy tmp =
                    AppContext.getTelephonyManagerProxy(mSubId);
            tmp.registerTelephonyCallback(mHandler::post, mTelephonyCallback);
        }
    }

    private void unregisterTelephonyCallback() {
        if (mTelephonyCallback != null) {
            TelephonyManagerProxy tmp =
                    AppContext.getTelephonyManagerProxy(mSubId);
            tmp.unregisterTelephonyCallback(mTelephonyCallback);
            mTelephonyCallback = null;
        }
    }

    private class EmergencyCallbackModeCallback extends TelephonyCallback implements
            TelephonyCallback.EmergencyCallbackModeListener {
        @Override
        public void onCallbackModeStarted(@TelephonyManager.EmergencyCallbackModeType int type,
                @NonNull Duration timerDuration, int subId) {
            if (subId != mSubId) {
                return;
            }

            ImsLog.i(this, mSlotId, "onCallbackModeStarted() type: " + type);
            updateEmergencyCallbackMode(
                    type, EmergencyCallbackModeState.START, timerDuration.toSeconds());
        }

        @Override
        public void onCallbackModeRestarted(@TelephonyManager.EmergencyCallbackModeType int type,
                @NonNull Duration timerDuration, int subId) {
            if (subId != mSubId) {
                return;
            }

            ImsLog.i(this, mSlotId, "onCallbackModeRestarted() type: " + type);
            updateEmergencyCallbackMode(
                    type, EmergencyCallbackModeState.START, timerDuration.toSeconds());
        }

        @Override
        public void onCallbackModeStopped(@TelephonyManager.EmergencyCallbackModeType int type,
                @TelephonyManager.EmergencyCallbackModeStopReason int reason, int subId) {
            if (subId != mSubId) {
                return;
            }

            ImsLog.i(this, mSlotId, "onCallbackModeStopped() type: " + type + ",reason: " + reason);

            EmergencyCallbackModeState state = EmergencyCallbackModeState.STOP;
            if (reason == TelephonyManager.STOP_REASON_OUTGOING_EMERGENCY_CALL_INITIATED
                    || reason == TelephonyManager.STOP_REASON_EMERGENCY_SMS_SENT) {
                state = EmergencyCallbackModeState.STOP_BY_EMERGENCY;
            }
            updateEmergencyCallbackMode(type, state, 0);
        }
    }
}
