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

public final class TelephonyCallState {
    private int mState = TelephonyManager.CALL_STATE_IDLE;
    private String mIncomingNumber = "";

    public TelephonyCallState() {
    }

    public TelephonyCallState(int state, String incomingNumber) {
        mState = state;
        mIncomingNumber = incomingNumber;
    }

    public String getIncomingNumber() {
        return mIncomingNumber;
    }

    /**
     * @return the call state
     *      TelephonyManager#CALL_STATE_IDLE (0)
     *      TelephonyManager#CALL_STATE_RINGING (1)
     *      TelephonyManager#CALL_STATE_OFFHOOK (2)
     */
    public int getState() {
        return mState;
    }

    /**
     * @param the call state
     *      TelephonyManager#CALL_STATE_IDLE (0)
     *      TelephonyManager#CALL_STATE_RINGING (1)
     *      TelephonyManager#CALL_STATE_OFFHOOK (2)
     */
    public void setState(int state) {
        mState = state;
    }

    public void setIncomingNumber(String number) {
        mIncomingNumber = number;
    }
}
