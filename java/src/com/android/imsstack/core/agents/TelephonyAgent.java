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
import android.util.SparseArray;

import com.android.imsstack.core.agents.dcm.DcFactory;
import com.android.imsstack.core.agents.dcmif.IDcNetWatcher;
import com.android.imsstack.util.AppContext;
import com.android.imsstack.util.ImsLog;
import com.android.imsstack.util.MSimUtils;

/**
 * A class for accessing the Telephony or SIM states through {@link TelephonyManager} or
 * {@link SubscriptionManager}.
 */
public class TelephonyAgent implements TelephonyInterface {
    private final int mSlotId;
    private final SparseArray<TelephonyManager> mTelephonyManagers = new SparseArray<>();

    public TelephonyAgent(int slotId) {
        mSlotId = slotId;
    }

    @Override
    public void init(Context context) {
        // Cache the TelephonyManager object.
        getTelephonyManager();
    }

    @Override
    public void cleanup() {
        mTelephonyManagers.clear();
    }

    @Override
    public @CallState int getCsCallState() {
        return TelephonyManager.CALL_STATE_IDLE;
        /** TODO(b/281756154): need to integrate with the IMS calls.
        TelephonyManager tm = getTelephonyManager();
        return (tm != null) ? tm.getCallStateForSubscription() : TelephonyManager.CALL_STATE_IDLE;
        */
    }

    @Override
    public @CallState int getCsCallStateInOtherSlot() {
        return TelephonyManager.CALL_STATE_IDLE;
        /** TODO(b/281756154): need to integrate with the IMS calls.
        TelephonyManager tm = getTelephonyManager();
        return (tm != null) ? tm.getCallStateForSubscription() : TelephonyManager.CALL_STATE_IDLE;
        */
    }

    @Override
    public @NetworkType int getNetworkType() {
        TelephonyManager tm = getTelephonyManager();
        int networkType = (tm != null)
                ? tm.getDataNetworkType()
                : TelephonyManager.NETWORK_TYPE_UNKNOWN;

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
        TelephonyManager tm = getTelephonyManager();
        int voiceNetworkType = (tm != null)
                ? tm.getVoiceNetworkType()
                : TelephonyManager.NETWORK_TYPE_UNKNOWN;

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
        TelephonyManager tm = getTelephonyManager();
        return (tm != null) ? tm.getSimState(mSlotId) : TelephonyManager.SIM_STATE_UNKNOWN;
    }

    @Override
    public String getImei() {
        TelephonyManager tm = getTelephonyManager();
        return (tm != null) ? tm.getImei(mSlotId) : null;
    }

    @Override
    public String getDeviceSoftwareVersion() {
        TelephonyManager tm = getTelephonyManager();
        return (tm != null) ? tm.getDeviceSoftwareVersion(mSlotId) : null;
    }

    @Override
    public String getPhoneNumber() {
        // If the phone type is CDMA-LTE,
        // CarrierConfigManager#KEY_USE_USIM_BOOL may need to be enabled
        // to read the phone number from USIM application.
        int subId = MSimUtils.getSubId(mSlotId);
        SubscriptionManager sm =
                AppContext.getInstance().getSystemService(SubscriptionManager.class);

        return (sm != null)
                ? sm.getPhoneNumber(subId, SubscriptionManager.PHONE_NUMBER_SOURCE_UICC)
                : "";
    }

    @Override
    public String getSubscriberId() {
        TelephonyManager tm = getTelephonyManager();
        return (tm != null) ? tm.getSubscriberId() : null;
    }

    @Override
    public String getSimOperator() {
        TelephonyManager tm = getTelephonyManager();
        return (tm != null) ? tm.getSimOperator() : null;
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
        TelephonyManager tm = getTelephonyManager();
        return (tm != null) ? tm.getSimCountryIso() : "";
    }

    @Override
    public String getSimSerialNumber() {
        TelephonyManager tm = getTelephonyManager();
        return (tm != null) ? tm.getSimSerialNumber() : null;
    }

    @Override
    public String getSimGid1() {
        TelephonyManager tm = getTelephonyManager();
        return (tm != null) ? tm.getGroupIdLevel1() : null;
    }

    @Override
    public String getSimOperatorName() {
        TelephonyManager tm = getTelephonyManager();
        return (tm != null) ? tm.getSimOperatorName() : null;
    }

    @Override
    public String getNetworkOperator() {
        TelephonyManager tm = getTelephonyManager();
        return (tm != null) ? tm.getNetworkOperator() : null;
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
        TelephonyManager tm = getTelephonyManager();
        return (tm != null) ? tm.getNetworkCountryIso() : "";
    }

    @Override
    public boolean isEmergencyNumber(String number) {
        TelephonyManager tm = getTelephonyManager();

        final String eNumber = PhoneNumberUtils.stripSeparators(number);
        boolean isEmergencyNumber = (tm != null) ? tm.isEmergencyNumber(eNumber) : false;

        ImsLog.d(mSlotId, "isEmergencyNumber: " + isEmergencyNumber
                + " (number=" + number + ", eNumber=" + eNumber + ")");

        return isEmergencyNumber;
    }

    private @NetworkType int getCellularDataNetworkType() {
        PhoneStateInterface phoneState = AgentFactory.getInstance().getAgent(
                PhoneStateInterface.class, mSlotId);
        return (phoneState != null)
                ? phoneState.getCellularDataNetworkType()
                : TelephonyManager.NETWORK_TYPE_UNKNOWN;
    }

    private TelephonyManager getTelephonyManager() {
        int subId = MSimUtils.getSubId(mSlotId);
        if (MSimUtils.isValidSubId(subId)) {
            TelephonyManager tm = mTelephonyManagers.get(subId);
            if (tm == null) {
                ImsLog.i(mSlotId, "TelephonyManager created for sub" + subId);
                tm = AppContext.getTelephonyManager(subId);
                mTelephonyManagers.put(subId, tm);
            }
            return tm;
        }

        return AppContext.getTelephonyManager();
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
