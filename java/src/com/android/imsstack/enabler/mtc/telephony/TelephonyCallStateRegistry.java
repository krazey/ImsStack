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
package com.android.imsstack.enabler.mtc.telephony;

import android.telephony.TelephonyManager;

import com.android.imsstack.core.ImsGlobal;
import com.android.imsstack.enabler.IBaseContext;
import com.android.imsstack.enabler.mtc.CallTracker;
import com.android.imsstack.enabler.mtc.ICallStateTracker;
import com.android.imsstack.util.ImsLog;

public final class TelephonyCallStateRegistry {
    /**
     * This is used to pass the call state to RIL (not AFRW) for VZW requirement.
     */
    public static final int CALL_STATE_CS_RETRY = 3;

    private static final int CALL_STATE_FOR_RIL = 0x0001;
    private static final int CALL_STATE_IGNORE_CS_RETRY_FOR_RIL = 0x0002;

    private final IBaseContext mContext;
    private final ICallStateTracker mCallStateTracker;
    private TelephonyCallState mCallState = new TelephonyCallState();
    private int mNotifications = 0;

    public TelephonyCallStateRegistry(IBaseContext context, ICallStateTracker csTracker) {
        mContext = context;
        mCallStateTracker = csTracker;

        int slotId = mContext.getSlotId();

        if (ImsGlobal.isOperator(slotId, "VZW") || ImsGlobal.isOperator(slotId, "SPR")) {
            mNotifications |= CALL_STATE_FOR_RIL;
        }

        // Initialize the call state for RIL(modem)
        notifyCallStateForRIL(mCallState.getState());
    }

    public void dispose() {
        mCallState.setState(TelephonyManager.CALL_STATE_IDLE);
        mCallState.setIncomingNumber("");

        notifyCallStateForRIL(mCallState.getState());
    }

    public TelephonyCallState getCallState() {
        return mCallState;
    }

    public boolean isCallStateRequired() {
        return isCallStateRequiredForRIL();
    }

    /**
     * Updates the call state for telephony or/and RIL if it's required.
     *
     * @param state the call state
     *      CallTracker#CALL_STATE_IDLE (0)
     *      CallTracker#CALL_STATE_RINGING (1)
     *      CallTracker#CALL_STATE_OFFHOOK (2)
     * @param incomingNumber the incoming number; it's valid in RINGING state
     */
    public void updateCallState(int state, String incomingNumber) {
        state = convertCallStateToTelephonyCallState(state);

        if (checkAndUpdateCallState(state, incomingNumber)) {
            notifyCallStateForRIL(mCallState.getState());
        }
    }

    public void updateNonTelephonyCallState(int state) {
        if (state == CALL_STATE_CS_RETRY) {
            if (!isCallStateIgnoreCSRetryRequiredForRIL()) {
                notifyCallStateForRIL(state);
            }
        }
    }

    public static int convertCallStateToTelephonyCallState(int state) {
        if (state == CallTracker.CALL_STATE_IDLE) {
            return TelephonyManager.CALL_STATE_IDLE;
        } else if (state == CallTracker.CALL_STATE_RINGING) {
            return TelephonyManager.CALL_STATE_RINGING;
        } else if (state == CallTracker.CALL_STATE_OFFHOOK) {
            return TelephonyManager.CALL_STATE_OFFHOOK;
        } else if (state == CallTracker.CALL_STATE_RINGBACK) {
            return TelephonyManager.CALL_STATE_OFFHOOK;
        } else {
            return TelephonyManager.CALL_STATE_IDLE;
        }
    }

    private boolean checkAndUpdateCallState(int state, String incomingNumber) {
        if (state == mCallState.getState()) {
            return false;
        }

        logi("TelephonyCallState :: " + mCallState.getState() + " >> " + state);

        mCallState.setState(state);
        mCallState.setIncomingNumber(incomingNumber);

        return true;
    }

    private boolean isCallStateRequiredForRIL() {
        return (mNotifications & CALL_STATE_FOR_RIL) != 0;
    }

    private boolean isCallStateIgnoreCSRetryRequiredForRIL() {
        return (mNotifications & CALL_STATE_IGNORE_CS_RETRY_FOR_RIL) != 0;
    }

    private void notifyCallStateForRIL(int state) {
        if (!isCallStateRequiredForRIL()) {
            return;
        }

        if (state == TelephonyManager.CALL_STATE_IDLE) {
            if (mCallStateTracker.hasEstablishedCall()) {
                logi("TelephonyCallState :: Session exists on IDLE - Ignored");
                return;
            }
        }

        logi("notifyCallStateForRIL :: state=" + state);
    }

    private static void logi(String s) {
        ImsLog.i("[GII-TEL] " + s);
    }
}
