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

import android.telephony.CellInfo;
import android.telephony.PreciseCallState;
import android.telephony.PreciseDataConnectionState;
import android.telephony.ServiceState;
import android.telephony.SignalStrength;

import com.android.imsstack.core.agents.dcmif.EApnType;

import java.util.List;

/**
 * This class provides an interface to receive the phone state events.
 */
public class ImsPhoneStateListener {
    public static final int LISTEN_NONE = 0;
    public static final int LISTEN_SERVICE_STATE = 0x00000001;
    public static final int LISTEN_CALL_STATE = 0x00000002;
    public static final int LISTEN_PRECISE_CALL_STATE = 0x00000004;
    public static final int LISTEN_SRVCC_STATE = 0x00000008;
    public static final int LISTEN_CELL_INFO = 0x00000010;
    public static final int LISTEN_SIGNAL_STRENGTHS = 0x00000020;
    public static final int LISTEN_PCSCF_ADDRESS_INFO = 0x00000040;
    public static final int LISTEN_PRECISE_DATA_CONNECTION_STATE = 0x00000080;

    /**
     * Invokes when service state is changed.
     */
    public void onServiceStateChanged(ServiceState serviceState) {
        // no-op
    }

    /**
     * Invokes when call state is changed.
     */
    public void onCallStateChanged(int state, String incomingNumber) {
        // no-op
    }

    /**
     * Invokes when precise call state is changed.
     */
    public void onPreciseCallStateChanged(PreciseCallState callState) {
        // no-op
    }

    /**
     * Invokes when SRVCC state is changed.
     */
    public void onSrvccStateChanged(int state) {
        // no-op
    }

    /**
     * Invokes when cell info. is changed.
     */
    public void onCellInfoChanged(List<CellInfo> cellInfo) {
        // no-op
    }

    /**
     * Invokes when signal strengths is changed.
     */
    public void onSignalStrengthsChanged(SignalStrength signalStrength) {
        // no-op
    }

    /**
     * Invokes when pdn handover info is changed.
     */
    public void onPcscfUpdated(List<String> pcscf) {
        // no-op
    }

    /**
     * Invokes when precise data connection state is changed.
     */
    public void onPreciseDataConnectionStateChanged(
            PreciseDataConnectionState dataConnectionState)  {
        // no-op
    }

    protected static boolean isApnTypeIms(String apnSettingType) {
        int apnType = EApnType.getTypeFromApnSettingType(apnSettingType);
        return EApnType.IMS.getType() == apnType;
    }
}
