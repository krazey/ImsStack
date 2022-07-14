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
import android.telephony.SubscriptionManager;
import android.telephony.TelephonyManager;
import android.text.TextUtils;

import com.android.imsstack.core.agents.agentif.ITelephonyState;
import com.android.imsstack.core.agents.agentif.ITelephonySubscriber;
import com.android.imsstack.system.ISystem;
import com.android.imsstack.system.ISystemAPITelephonySubscriber;
import com.android.imsstack.system.SystemInterface;
import com.android.imsstack.util.AppContext;
import com.android.imsstack.util.ImsLog;
import com.android.imsstack.util.MSimUtils;

import java.util.Hashtable;
import java.util.Locale;

public class TelephonySubscriberAgent implements ITelephonySubscriber,
        ISystemAPITelephonySubscriber {

    // Constants--------------------------------------------------
    private static final String MNC_KT = "08";
    private static final String MNC_LGU = "06";
    private static final String MNC_SKT = "05";
    private static final String MNC_ATT = "410";
    private static final String MNC_ATT_LAP = "180";
    private static final String MNC_ATT_PRIVATE = "380";
    private static final String MNC_ATT_TESTBED = "41";
    private static final String MNC_TMO = "260";
    private static final String MNC_MPCS = "660";

    private static final int ECC_PRIORITY_UICC = 0;
    private static final int ECC_PRIORITY_NETWORK = 1;

    // <MCC, Country-ISO>
    private Hashtable<String, String> mCountryIsoTable = new Hashtable<String, String>();

    // Variables--------------------------------------------------
    private int mModemEccPriority = ECC_PRIORITY_NETWORK;

    private final int mSlotId;

    // Static loading materials ----------------------------------
    // Public methods --------------------------------------------

    public TelephonySubscriberAgent(int slotId) {
        mSlotId = slotId;
    }

    @Override
    public void init(Context context) {
        ImsLog.d(mSlotId, "");

        initCountryIsoTable();

        ISystem system = SystemInterface.getInstance().getSystem(mSlotId);
        if (system != null) {
            system.setISystemAPITelephonySubscriber(this);
        }
    }

    @Override
    public void cleanup() {
        ISystem system = SystemInterface.getInstance().getSystem(mSlotId);
        if (system != null) {
            system.setISystemAPITelephonySubscriber(null);
        }

        mCountryIsoTable.clear();
    }

    // Interface implementation methods --------------------------
    @Override
    public String getMccMnc(boolean fromSim) {
        TelephonyManager tm = getTelephonyManagerOnSimConfig();

        if (tm == null) {
            return null;
        }

        String operator = null;

        if (fromSim) {
            operator = tm.getSimOperator();
        } else {
            // Availability: Only when user is registered to a network
            ITelephonyState tsa = (ITelephonyState)AgentFactory.getAgent(
                AgentFactory.TELEPHONY_STATE, mSlotId);

            if (tsa != null) {
                if (tsa.getNetworkType() != TelephonyManager.NETWORK_TYPE_UNKNOWN) {
                    operator = tm.getNetworkOperator();
                }
            }
        }

        return operator;
    }

    @Override
    public String getMccMnc(int subId) {
        TelephonyManager tm = getTelephonyManager(subId);
        return (tm != null) ? tm.getSimOperator() : null;
    }

    @Override
    public String getMcc(boolean fromSim) {
        String operator = getMccMnc(fromSim);

        if (operator == null) {
            return null;
        }

        if (operator.length() < 5) {
            return null;
        }

        return operator.substring(0, 3);
    }

    @Override
    public String getMnc(boolean fromSim) {
        String operator = getMccMnc(fromSim);

        if (operator == null) {
            return null;
        }

        if (operator.length() < 5) {
            return null;
        }

        return operator.substring(3);
    }

    @Override
    public String getSimOperatorInternal() {
        String strMnc = getMnc(true);

        if (TextUtils.isEmpty(strMnc)) {
            ImsLog.w(mSlotId, "strMnc is null");
            return null;
        }

        String strOperator = null;
        if (strMnc.equals(TelephonySubscriberAgent.MNC_KT)) {
            strOperator = "KT";
        }
        else if (strMnc.equals(TelephonySubscriberAgent.MNC_SKT) ) {
            strOperator = "SKT";
        }
        else if (strMnc.equals(TelephonySubscriberAgent.MNC_LGU) ) {
            strOperator = "LGU";
        }
        else if (strMnc.equals(TelephonySubscriberAgent.MNC_ATT) ||
            strMnc.equals(TelephonySubscriberAgent.MNC_ATT_LAP) ||
            strMnc.equals(TelephonySubscriberAgent.MNC_ATT_PRIVATE) ||
            strMnc.equals(TelephonySubscriberAgent.MNC_ATT_TESTBED ) ){
            strOperator = "ATT";
        }
        else if (strMnc.equals(TelephonySubscriberAgent.MNC_TMO)
                || strMnc.equals(TelephonySubscriberAgent.MNC_MPCS)) {
            strOperator = "TMO";
        }
        else {
            // no-op
        }

        return strOperator;
    }

    @Override
    public String getCountryIso(boolean fromSim) {
        TelephonyManager tm = getTelephonyManagerOnSimConfig();

        if (tm == null) {
            return null;
        }

        String countryIso = "";

        if (fromSim) {
            countryIso = tm.getSimCountryIso();
        } else {
            // Availability: Only when user is registered to a network
            ITelephonyState tsa = (ITelephonyState)AgentFactory.getAgent(
                AgentFactory.TELEPHONY_STATE, mSlotId);

            if ((tsa != null)
                    && (tsa.getNetworkType() != TelephonyManager.NETWORK_TYPE_UNKNOWN)) {
                countryIso = tm.getNetworkCountryIso();
            }
        }

        if (TextUtils.isEmpty(countryIso)) {
            String mcc = getMcc(fromSim);

            if (TextUtils.isEmpty(mcc)) {
                ImsLog.w(mSlotId, "MCC is null");
                return null;
            }

            countryIso = mCountryIsoTable.get(mcc);
        }

        return (countryIso != null) ? countryIso.toUpperCase(Locale.ROOT) : countryIso;
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
                : null;
    }

    @Override
    public String getSimSerialNumber() {
        TelephonyManager tm = getTelephonyManagerOnSimConfig();
        return (tm != null) ? tm.getSimSerialNumber() : null;
    }

    @Override
    public String getSubscriberId() {
        TelephonyManager tm = getTelephonyManagerOnSimConfig();
        return (tm != null) ? tm.getSubscriberId() : null;
    }

    @Override
    public String getDeviceId() {
        TelephonyManager tm = getTelephonyManager();
        return (tm != null) ? tm.getImei(mSlotId) : null;
    }

    @Override
    public String getGroupIdLevel1() {
        TelephonyManager tm = getTelephonyManagerOnSimConfig();
        return (tm != null) ? tm.getGroupIdLevel1() : null;
    }

    @Override
    public String getSimOperatorName() {
        TelephonyManager tm = getTelephonyManagerOnSimConfig();
        return (tm != null) ? tm.getSimOperatorName() : null;
    }

    @Override
    public String getDeviceId4Sys() {
        return getDeviceId();
    }

    @Override
    public String getDeviceSoftwareVersion4Sys() {
        TelephonyManager tm = getTelephonyManager();
        return (tm != null) ? tm.getDeviceSoftwareVersion(mSlotId) : null;
    }

    @Override
    public String getSubscriberId4Sys() {
        return getSubscriberId();
    }

    @Override
    public String getMcc4Sys(boolean fromSim) {
        return getMcc(fromSim);
    }

    @Override
    public String getMnc4Sys(boolean fromSim) {
         return getMnc(fromSim);
    }

    @Override
    public String getPhoneNUmberExcludingNationalPrefix4Sys() {
        String phoneNumber = getPhoneNumber();

        // Now, only for domestic operators
        if (phoneNumber != null && phoneNumber.length() >= 3) {
            if (phoneNumber.startsWith("+82")) {
                phoneNumber = "0" + phoneNumber.substring(3);
            }
        }

        return phoneNumber;
    }

    @Override
    public String getOperator4Sys() {
        return getSimOperatorInternal();
    }

    @Override
    public String getCountry4Sys() {
        return getCountryIso(true);
    }

    @Override
    public String getNetworkCountry4Sys() {
        return getCountryIso(false);
    }

    @Override
    public String getEmergencyNumberListFromSIM4Sys() {
        return "";
    }

    @Override
    public int getEmergencyPriorityFromModem4Sys() {
        return mModemEccPriority;
    }

    @Override
    public boolean isUiccGbaSupported4Sys() {
        SimInterface sim = AgentFactory.getInstance().getAgent(SimInterface.class, mSlotId);
        return (sim != null) ? sim.isGbaAvailable() : false;
    }

    // Private/Protected methods ---------------------------------
    private TelephonyManager getTelephonyManagerOnSimConfig() {
        if (MSimUtils.isMultiSimEnabled()) {
            return getTelephonyManager(MSimUtils.getSubId(mSlotId));
        }

        return getTelephonyManager();
    }

    private static TelephonyManager getTelephonyManager() {
        return AppContext.getTelephonyManager();
    }

    private static TelephonyManager getTelephonyManager(int subId) {
        return AppContext.getTelephonyManager(subId);
    }

    private void initCountryIsoTable() {
        mCountryIsoTable.put("206", "FR");
        mCountryIsoTable.put("302", "CA");
        mCountryIsoTable.put("310", "US");
        mCountryIsoTable.put("311", "US");
        mCountryIsoTable.put("450", "KR");
        mCountryIsoTable.put("001", "ZZ");
        mCountryIsoTable.put("000", "ZZ");
    }
}
