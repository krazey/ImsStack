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
package com.android.imsstack.core.agents.dcmif;

import android.telephony.TelephonyManager;

public enum EDataState {

    DATA_STATE_DISCONNECTED     (0),
    DATA_STATE_CONNECTED        (2),
    DATA_STATE_CONNECT_FAILED   (4),
    DATA_STATE_IP_CHANGED       (5),
    DATA_STATE_PCSCF_CHANGED    (6);

    private final int mState;

    EDataState(int mState) {
        this.mState = mState;
    }

    /**
     * Converts data connection state of the TelephonyManager to internal state.
     *
     * @param dataState data connection state of the TelephonyManager.
     */
    public static int convertFromTMtoImsType(int dataState) {
        if (dataState == TelephonyManager.DATA_CONNECTED
                || dataState == TelephonyManager.DATA_SUSPENDED) {
            return DATA_STATE_CONNECTED.getState();
        }

        return DATA_STATE_DISCONNECTED.getState();
    }

    /**
     * Converts the internal state of integer type to enum type.
     *
     * @param dataState internal state of integer type.
     */
    public static EDataState convertIntTypeToEnum(int dataState) {
        if (DATA_STATE_CONNECTED.getState() == dataState) {
            return EDataState.DATA_STATE_CONNECTED;
        } else if (DATA_STATE_DISCONNECTED.getState() == dataState) {
            return EDataState.DATA_STATE_DISCONNECTED;
        } else if (DATA_STATE_CONNECT_FAILED.getState() == dataState) {
            return EDataState.DATA_STATE_CONNECT_FAILED;
        } else if (DATA_STATE_IP_CHANGED.getState() == dataState) {
            return EDataState.DATA_STATE_IP_CHANGED;
        }

        return EDataState.DATA_STATE_DISCONNECTED;
    }

    public int getState() {
        return this.mState;
    }
}
