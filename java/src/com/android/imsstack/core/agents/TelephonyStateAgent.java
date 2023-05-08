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

import android.content.Context;
import android.telephony.TelephonyManager;

import com.android.imsstack.core.agents.dcm.DcFactory;
import com.android.imsstack.core.agents.dcmif.IDcNetWatcher;
import com.android.imsstack.system.ISystem;
import com.android.imsstack.system.ISystemAPITelephonyState;
import com.android.imsstack.system.SystemInterface;
import com.android.imsstack.util.AppContext;
import com.android.imsstack.util.ImsLog;
import com.android.imsstack.util.MSimUtils;

public class TelephonyStateAgent implements ITelephonyState,ISystemAPITelephonyState {
    private final int mSlotId;
    private PhoneStateInterface mPhoneState;

    public TelephonyStateAgent(int slotId) {
        mSlotId = slotId;
    }

    @Override
    public void init(Context context) {
        ImsLog.d(mSlotId, "");

        mPhoneState = AgentFactory.getInstance().getAgent(PhoneStateInterface.class, mSlotId);

        ISystem system = SystemInterface.getInstance().getSystem(mSlotId);
        if (system != null) {
            system.setISystemAPITelephonyState(this);
        }
    }

    @Override
    public void cleanup() {
        ImsLog.d(mSlotId, "");

        ISystem system = SystemInterface.getInstance().getSystem(mSlotId);
        if (system != null) {
            system.setISystemAPITelephonyState(null);
        }
    }

    @Override
    public int getCallState() {
        ImsLog.d(mSlotId, "");
        // TODO: add CS call state using AOSP API
        return TelephonyManager.CALL_STATE_IDLE;
    }

    @Override
    public int getCallState(boolean fromSim) {
        if (fromSim != true) {
            return getCallState();
        }

        TelephonyManager tm = null;
        int subId = MSimUtils.getSubId(mSlotId);
        if (MSimUtils.isValidSubId(subId)) {
            tm = getTelephonyManager(subId);
        }

        if (tm == null) {
            return TelephonyManager.CALL_STATE_IDLE;
        }

        return tm.getCallStateForSubscription();
    }

    @Override
    public int getDataState() {
        TelephonyManager tm = getTelephonyManager();
        return (tm != null) ? tm.getDataState() : TelephonyManager.DATA_UNKNOWN;
    }

    @Override
    public int getSimState() {
        TelephonyManager tm = getTelephonyManager();
        return (tm != null) ? tm.getSimState(mSlotId) : TelephonyManager.SIM_STATE_UNKNOWN;
    }

    @Override
    public int getNetworkType() {
        TelephonyManager tm = null;
        int subId = MSimUtils.getSubId(mSlotId);
        if (MSimUtils.isValidSubId(subId)) {
            tm = getTelephonyManager(subId);
        }

        if (tm == null) {
            return TelephonyManager.NETWORK_TYPE_UNKNOWN;
        }

        int networkType = tm.getDataNetworkType();
        if (isIWLAN(networkType)) {
            networkType = getCellularNetworkType();
        }

        IDcNetWatcher dcnw = (IDcNetWatcher) DcFactory.getDc(DcFactory.NETWORK_WATCHER, mSlotId);
        if (dcnw != null) {
            if (is5G(networkType) && !dcnw.is5GRequired()) {
                networkType = TelephonyManager.NETWORK_TYPE_LTE;
            }

            dcnw.setRatFromTelephonyManager(networkType);
        }

        return networkType;
    }

    @Override
    public int getVoiceNetworkType() {
        TelephonyManager tm = getTelephonyManagerOnSimConfig();

        if (tm == null) {
            return TelephonyManager.NETWORK_TYPE_UNKNOWN;
        }

        int voiceNetworkType = tm.getVoiceNetworkType();

        IDcNetWatcher dcnw = (IDcNetWatcher) DcFactory.getDc(DcFactory.NETWORK_WATCHER, mSlotId);
        if (dcnw != null) {
            if (is5G(voiceNetworkType) && !dcnw.is5GRequired()) {
                voiceNetworkType = TelephonyManager.NETWORK_TYPE_LTE;
            }

            dcnw.setVoiceRatFromTelephonyManager(voiceNetworkType);
        }

        return voiceNetworkType;
    }

    @Override
    public int getNetworkType4Sys() {
        return getNetworkType();
    }

    @Override
    public int getVoiceNetworkType4Sys() {
        return getVoiceNetworkType();
    }

    @Override
    public int getCallState4Sys() {
        return getCallState();
    }

    private TelephonyManager getTelephonyManagerOnSimConfig() {
        if (MSimUtils.isMultiSimEnabled()) {
            return AppContext.getTelephonyManager(MSimUtils.getSubId(mSlotId));
        }

        return getTelephonyManager();
    }

    private int getCellularNetworkType() {
        return (mPhoneState != null) ? mPhoneState.getCellularDataNetworkType() :
                TelephonyManager.NETWORK_TYPE_UNKNOWN;
    }

    private static TelephonyManager getTelephonyManager() {
        return AppContext.getTelephonyManager();
    }

    private static TelephonyManager getTelephonyManager(int nSubId) {
        return AppContext.getTelephonyManager(nSubId);
    }

    private static boolean is5G(int network) {
        return (network == TelephonyManager.NETWORK_TYPE_NR);
    }

    private static boolean isIWLAN(int network) {
        return (network == TelephonyManager.NETWORK_TYPE_IWLAN);
    }
}
