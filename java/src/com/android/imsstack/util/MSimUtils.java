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

package com.android.imsstack.util;

import android.content.Context;
import android.telephony.SubscriptionManager;
import android.telephony.TelephonyManager;

import com.android.imsstack.external.ims.ImsFeatureProvider;
import com.android.internal.telephony.IccCardConstants;

/**
 * This class provides the utility APIs to control the multi-sim.
 */
public final class MSimUtils {
    public static final int DEFAULT_PHONE_ID = 0;
    public static final int DEFAULT_SUB_ID = SubscriptionManager.DEFAULT_SUBSCRIPTION_ID;
    public static final int INVALID_PHONE_ID = (-1);
    public static final int INVALID_SLOT_ID = (-1);
    public static final int INVALID_SUB_ID = SubscriptionManager.INVALID_SUBSCRIPTION_ID;
    public static final int DEFAULT_SLOT_ID = 0;

    // Intent extra: WfcInformation, Hidden menu
    public static final String EXTRA_KEY_SLOT_ID = "SLOT_ID";

    public static final String PHONE_KEY = "phone";

    private static final int MAX_PHONE_COUNT_TRI_SIM = 3;
    // 0x7FFFFFFB
    private static final int DUMMY_SUB_ID_BASE =
            (SubscriptionManager.MAX_SUBSCRIPTION_ID_VALUE - MAX_PHONE_COUNT_TRI_SIM);
    private static final boolean MULTI_IMS = false;
    private static final boolean MULTI_LTE = false;
    private static boolean sSingleImsEnabledOnDsdv = false;
    private static int sMultiSim = (-1);

    public static int getDefaultDataSubId() {
        return SubscriptionManager.getDefaultDataSubscriptionId();
    }

    public static int getDefaultSubId() {
        return SubscriptionManager.getDefaultSubscriptionId();
    }

    /** Get default sub id */
    public static int getImsDefaultSubId() {
        if (isMultiSimEnabled()) {
            return getDefaultDataSubId();
        } else {
            return getSubId(DEFAULT_PHONE_ID);
        }
    }

    /** Get imsi */
    public static String getImsi(int subId) {
        TelephonyManager tm = AppContext.getTelephonyManager(subId);
        return (tm != null) ? tm.getSubscriberId() : null;
    }

    public static int getMaxSimSlot() {
        return getPhoneCount();
    }

    /** Get phone count */
    public static int getPhoneCount() {
        TelephonyManager tm = AppContext.getTelephonyManager();
        return (tm != null) ? tm.getActiveModemCount() : 1;
    }

    /** phone id */
    public static int getPhoneId(int subId) {
        return getPhoneId(subId, DEFAULT_PHONE_ID);
    }

    /** phone id */
    public static int getPhoneId(int subId, int defaultPhoneId) {
        int phoneId = SubscriptionManager.getSlotIndex(subId);

        if (phoneId < DEFAULT_PHONE_ID || phoneId >= getPhoneCount()) {
            // Set the phoneId as a default
            phoneId = defaultPhoneId;
        }

        return phoneId;
    }

    /** phone id for sub id */
    public static int getPhoneIdForDummySubId(int subId) {
        if (isDummySubId(subId)) {
            if (!isMultiSimEnabled()) {
                return DEFAULT_PHONE_ID;
            } else {
                return getPhoneId(subId, INVALID_PHONE_ID);
            }
        } else if (isValidSubId(subId)) {
            return getPhoneId(subId, INVALID_PHONE_ID);
        }

        return INVALID_PHONE_ID;
    }

    /** sim state */
    public static String getSimState(int slotId) {
        return ImsExtApi.Uicc.getSimState(slotId);
    }

    /** slot id */
    public static int getSlotId(int subId) {
        return SubscriptionManager.getSlotIndex(subId);
    }

    /** sub id */
    public static int getSubId(int phoneId) {
        SubscriptionManager sm = AppContext.get().getSystemService(SubscriptionManager.class);
        int[] subIds = (sm != null) ? sm.getSubscriptionIds(phoneId) : null;
        if ((subIds != null) && (subIds.length > 0)) {
            return subIds[0];
        }

        return INVALID_SUB_ID;
    }

    /** icc card */
    public static boolean hasIccCard(int slotId) {
        TelephonyManager tm = AppContext.getTelephonyManager(getSubId(slotId));
        return (tm != null) ? tm.hasIccCard() : false;
    }

    /** sub id */
    public static boolean isDummySubId(int subId) {
        // FIXME: for other chipset.
        /** QCT */
        return (subId >= DUMMY_SUB_ID_BASE)
                && (subId <= DEFAULT_SUB_ID);
    }

    public static boolean isMultiImsEnabled() {
        return MULTI_IMS || isSingleImsEnabledOnDsdv();
    }

    /** multiple lte is enabled */
    public static boolean isMultiLteEnabled() {
        // As a default, single LTE is required on dual SIM environment.
        return MULTI_LTE;
    }

    public static boolean isMultiSimEnabled() {
        return getMaxSimSlot() > 1;
    }

    /** sim loaded */
    public static boolean isSimLoaded(int slotId) {
        return IccCardConstants.INTENT_VALUE_ICC_LOADED.equals(getSimState(slotId));
    }

    /** valid sub id */
    public static boolean isValidSubId(int subId) {
        return SubscriptionManager.isUsableSubscriptionId(subId);
    }

    // DSDV-SV (Dual SIM Dual VoLTE - Single VoLTE) {
    // Assume that slot-0 only supports VoLTE.
    public static int getSlotIdForSingleImsOnDsdv() {
        return DEFAULT_SLOT_ID;
    }

    public static boolean isSingleImsEnabledOnDsdv() {
        return sSingleImsEnabledOnDsdv && isMultiLteEnabled();
    }

    /** support single ims on dsdv */
    public static boolean isSlotIdForSingleImsOnDsdv(int slotId) {
        return slotId == getSlotIdForSingleImsOnDsdv();
    }

    public static void setSingleImsEnabledOnDsdv(Context c) {
        sSingleImsEnabledOnDsdv = ImsFeatureProvider.hasFeature(c, ImsFeature.FEATURE_DSDV_SV);
    }
    // }

    // DSSV-DV (Dual SIM Single VoLTE - Dual VoLTE for emergency {
    public static boolean isMultiImsEnabledOnDssv() {
        return isMultiSimEnabled() && ImsProperties.TARGET_COUNTRY.equals("AU");
    }
    // }
}
