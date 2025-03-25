/*
 * Copyright (C) 2023 The Android Open Source Project
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
import android.telephony.Annotation.CallState;
import android.telephony.Annotation.NetworkType;
import android.telephony.PhoneNumberUtils;
import android.telephony.SubscriptionManager;
import android.telephony.TelephonyManager;
import android.telephony.TelephonyManager.SimState;
import android.telephony.emergency.EmergencyNumber;
import android.util.SparseArray;

import androidx.annotation.NonNull;

import com.android.imsstack.base.AppContext;
import com.android.imsstack.base.DeviceConfig;
import com.android.imsstack.base.MSimUtils;
import com.android.imsstack.base.SystemServiceProxy.SubscriptionManagerProxy;
import com.android.imsstack.base.TelephonyManagerProxy;
import com.android.imsstack.core.agents.dcm.DcFactory;
import com.android.imsstack.core.agents.dcmif.IDcNetWatcher;
import com.android.imsstack.util.ImsLog;

import java.util.Collections;
import java.util.List;

/**
 * A class for accessing the Telephony or SIM states through {@link TelephonyManager} or
 * {@link SubscriptionManager}.
 */
public class TelephonyAgent implements TelephonyInterface {
    private final int mSlotId;
    private final SparseArray<TelephonyManagerProxy> mTelephonyManagerProxies = new SparseArray<>();

    public TelephonyAgent(int slotId) {
        mSlotId = slotId;
    }

    @Override
    public void init(Context context) {
        // Cache the TelephonyManagerProxy object.
        getTelephonyManagerProxy();
    }

    @Override
    public void cleanup() {
        mTelephonyManagerProxies.clear();
    }

    @Override
    public @CallState int getCsCallState() {
        PhoneStateInterface phoneState = AgentFactory.getInstance().getAgent(
                PhoneStateInterface.class, mSlotId);
        return (phoneState != null)
                ? phoneState.getCsCallState()
                : TelephonyManager.CALL_STATE_IDLE;
    }

    @Override
    public @CallState int getCsCallStateInOtherSlot() {
        int simCount = DeviceConfig.getActiveSimCount();

        for (int i = 0; i < simCount; ++i) {
            if (mSlotId != i) {
                PhoneStateInterface phoneState = AgentFactory.getInstance().getAgent(
                        PhoneStateInterface.class, i);
                int state = phoneState != null
                        ? phoneState.getCsCallState()
                        : TelephonyManager.CALL_STATE_IDLE;
                if (state != TelephonyManager.CALL_STATE_IDLE) {
                    return state;
                }
            }
        }

        return TelephonyManager.CALL_STATE_IDLE;
    }

    @Override
    public @NetworkType int getNetworkType() {
        TelephonyManagerProxy tmp = getTelephonyManagerProxy();
        int networkType = tmp.getDataNetworkType(mSlotId);

        if (networkType == TelephonyManager.NETWORK_TYPE_IWLAN) {
            networkType = getCellularDataNetworkType();
        }

        IDcNetWatcher dnw = DcFactory.getDcAgent(IDcNetWatcher.class, mSlotId);
        if (dnw != null) {
            if (is5G(networkType) && !dnw.is5GRequired()) {
                networkType = TelephonyManager.NETWORK_TYPE_LTE;
            }
            dnw.setRatFromTelephonyManager(networkType);
        }

        return networkType;
    }

    @Override
    public @NetworkType int getVoiceNetworkType() {
        TelephonyManagerProxy tmp = getTelephonyManagerProxy();
        int voiceNetworkType = tmp.getVoiceNetworkType(mSlotId);

        IDcNetWatcher dnw = DcFactory.getDcAgent(IDcNetWatcher.class, mSlotId);
        if (dnw != null) {
            if (is5G(voiceNetworkType) && !dnw.is5GRequired()) {
                voiceNetworkType = TelephonyManager.NETWORK_TYPE_LTE;
            }
            dnw.setVoiceRatFromTelephonyManager(voiceNetworkType);
        }

        return voiceNetworkType;
    }

    @Override
    public @SimState int getSimState() {
        TelephonyManagerProxy tmp = getTelephonyManagerProxy();
        return tmp.getSimState(mSlotId);
    }

    @Override
    public String getImei() {
        TelephonyManagerProxy tmp = getTelephonyManagerProxy();
        return tmp.getImei(mSlotId);
    }

    @Override
    public String getDeviceSoftwareVersion() {
        TelephonyManagerProxy tmp = getTelephonyManagerProxy();
        return (tmp != null) ? tmp.getDeviceSoftwareVersion(mSlotId) : null;
    }

    @Override
    public String getPhoneNumber() {
        // If the phone type is CDMA-LTE,
        // CarrierConfigManager#KEY_USE_USIM_BOOL may need to be enabled
        // to read the phone number from USIM application.
        int subId = MSimUtils.getSubId(mSlotId);
        SubscriptionManagerProxy smp =
                AppContext.getInstance().getSystemServiceProxy(SubscriptionManagerProxy.class);
        return smp.getPhoneNumber(subId, SubscriptionManager.PHONE_NUMBER_SOURCE_UICC);
    }

    @Override
    public String getSubscriberId() {
        TelephonyManagerProxy tmp = getTelephonyManagerProxy();
        return tmp.getSubscriberId();
    }

    @Override
    public String getSimOperator() {
        TelephonyManagerProxy tmp = getTelephonyManagerProxy();
        return tmp.getSimOperator();
    }

    @Override
    public String getSimMcc() {
        return getMcc(getSimOperator());
    }

    @Override
    public String getSimMnc() {
        return getMnc(getSimOperator());
    }

    @Override
    public String getSimCountryIso() {
        TelephonyManagerProxy tmp = getTelephonyManagerProxy();
        return tmp.getSimCountryIso();
    }

    @Override
    public String getSimSerialNumber() {
        TelephonyManagerProxy tmp = getTelephonyManagerProxy();
        return tmp.getSimSerialNumber();
    }

    @Override
    public String getSimGid1() {
        TelephonyManagerProxy tmp = getTelephonyManagerProxy();
        return tmp.getGroupIdLevel1();
    }

    @Override
    public String getSimOperatorName() {
        TelephonyManagerProxy tmp = getTelephonyManagerProxy();
        return tmp.getSimOperatorName();
    }

    @Override
    public String getNetworkOperator() {
        TelephonyManagerProxy tmp = getTelephonyManagerProxy();
        return tmp.getNetworkOperator();
    }

    @Override
    public String getNetworkMcc() {
        return getMcc(getNetworkOperator());
    }

    @Override
    public String getNetworkMnc() {
        return getMnc(getNetworkOperator());
    }

    @Override
    public String getNetworkCountryIso() {
        TelephonyManagerProxy tmp = getTelephonyManagerProxy();
        return tmp.getNetworkCountryIso();
    }

    @Override
    public boolean isEmergencyNumber(String number) {
        TelephonyManagerProxy tmp = getTelephonyManagerProxy();

        final String eNumber = PhoneNumberUtils.stripSeparators(number);
        boolean isEmergencyNumber = tmp.isEmergencyNumber(eNumber);

        ImsLog.d(this, mSlotId, "isEmergencyNumber: " + isEmergencyNumber
                + " (number=" + number + ", eNumber=" + eNumber + ")");

        return isEmergencyNumber;
    }

    @Override
    public @NonNull List<EmergencyNumber> getEmergencyNumberList() {
        TelephonyManagerProxy tmp = getTelephonyManagerProxy();
        return tmp.getEmergencyNumberList().getOrDefault(
                MSimUtils.getSubId(mSlotId), Collections.emptyList());
    }

    private @NetworkType int getCellularDataNetworkType() {
        PhoneStateInterface phoneState = AgentFactory.getInstance().getAgent(
                PhoneStateInterface.class, mSlotId);
        return (phoneState != null)
                ? phoneState.getCellularDataNetworkType()
                : TelephonyManager.NETWORK_TYPE_UNKNOWN;
    }

    private TelephonyManagerProxy getTelephonyManagerProxy() {
        int subId = MSimUtils.getSubId(mSlotId);
        if (MSimUtils.isValidSubId(subId)) {
            TelephonyManagerProxy tmp = mTelephonyManagerProxies.get(subId);
            if (tmp == null) {
                ImsLog.i(this, mSlotId, "TelephonyManager created for sub" + subId);
                tmp = AppContext.getTelephonyManagerProxy(subId);
                mTelephonyManagerProxies.put(subId, tmp);
            }
            return tmp;
        }

        return AppContext.getInstance().getSystemServiceProxy(TelephonyManagerProxy.class);
    }

    private static String getMcc(String mccmnc) {
        if (mccmnc == null || mccmnc.length() < 5) {
            return null;
        }
        return mccmnc.substring(0, 3);
    }

    private static String getMnc(String mccmnc) {
        if (mccmnc == null || mccmnc.length() < 5) {
            return null;
        }
        return mccmnc.substring(3);
    }

    private static boolean is5G(int networkType) {
        return networkType == TelephonyManager.NETWORK_TYPE_NR;
    }
}
