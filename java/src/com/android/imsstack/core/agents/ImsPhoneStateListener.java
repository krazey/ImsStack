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
package com.android.imsstack.core.agents;

import android.annotation.NonNull;
import android.telephony.Annotation.CallState;
import android.telephony.Annotation.SrvccState;
import android.telephony.BarringInfo;
import android.telephony.CellInfo;
import android.telephony.PreciseCallState;
import android.telephony.PreciseDataConnectionState;
import android.telephony.ServiceState;
import android.telephony.SignalStrength;

import java.util.List;

/**
 * An interface to monitor the phone state events.
 */
public interface ImsPhoneStateListener {
    /**
     * No flags to listen. This is used for unregister the listener.
     */
    int LISTEN_NONE = 0;
    /**
     * A flag to listen the service state.
     */
    int LISTEN_SERVICE_STATE = 0x00000001;
    /**
     * A flag to listen the call state.
     */
    int LISTEN_CALL_STATE = 0x00000002;
    /**
     * A flag to listen the precise call state.
     */
    int LISTEN_PRECISE_CALL_STATE = 0x00000004;
    /**
     * A flag to listen the SRVCC state.
     */
    int LISTEN_SRVCC_STATE = 0x00000008;
    /**
     * A flag to listen the cell information.
     */
    int LISTEN_CELL_INFO = 0x00000010;
    /**
     * A flag to listen the signal strengths.
     */
    int LISTEN_SIGNAL_STRENGTHS = 0x00000020;
    /**
     * A flag to listen the precise data connection state.
     */
    int LISTEN_PRECISE_DATA_CONNECTION_STATE = 0x00000040;
    /**
     * A flag to listen the barring information.
     */
    int LISTEN_BARRING_INFO = 0x00000080;

    /**
     * Called when the service state is changed.
     *
     * @param serviceState The {@link ServiceState} to be notified.
     */
    default void onServiceStateChanged(@NonNull ServiceState serviceState) {
    }

    /**
     * Called when the call state is changed.
     *
     * @param state The current call state.
     */
    default void onCallStateChanged(@CallState int state) {
    }

    /**
     * Called when the precise call state is changed.
     *
     * @param callState The {@link PreciseCallState} to be notified.
     */
    default void onPreciseCallStateChanged(@NonNull PreciseCallState callState) {
    }

    /**
     * Called when the SRVCC state is changed.
     *
     * @param state The SRVCC state.
     */
    default void onSrvccStateChanged(@SrvccState int state) {
    }

    /**
     * Called when the cell information is changed.
     *
     * @param cellInfos The list of {@link CellInfo} to be notified.
     */
    default void onCellInfoChanged(@NonNull List<CellInfo> cellInfos) {
    }

    /**
     * Called when the signal strengths is changed.
     *
     * @param signalStrength The {@link SignalStrength} to be notified.
     */
    default void onSignalStrengthsChanged(@NonNull SignalStrength signalStrength) {
    }

    /**
     * Called when the precise data connection state is changed.
     *
     * @param dataConnectionState The {@link PreciseDataConnectionState} to be notified.
     */
    default void onPreciseDataConnectionStateChanged(
            @NonNull PreciseDataConnectionState dataConnectionState)  {
    }

    /**
     * Called when the barring information is changed.
     *
     * @param barringInfo The {@link BarringInfo} to be notified.
     */
    default void onBarringInfoChanged(@NonNull BarringInfo barringInfo) {
    }
}
