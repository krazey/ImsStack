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
package com.android.imsstack.base;

import android.telephony.SubscriptionManager;
import android.telephony.TelephonyManager;

import com.android.imsstack.base.SystemServiceProxy.SubscriptionManagerProxy;

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

    // Intent extra: Hidden menu
    public static final String EXTRA_KEY_SLOT_ID = "SLOT_ID";

    /** Returns the default data subscription id. */
    public static int getDefaultDataSubId() {
        SubscriptionManagerProxy smp =
                AppContext.getInstance().getSystemServiceProxy(SubscriptionManagerProxy.class);
        return smp.getDefaultDataSubscriptionId();
    }

    /** Returns the default subscription id that can be used by ImsStack. */
    public static int getImsDefaultSubId() {
        if (isMultiSimEnabled()) {
            return getDefaultDataSubId();
        } else {
            return getSubId(DEFAULT_PHONE_ID);
        }
    }

    /**
     * Returns the number of logical SIMs currently configured to be activated.
     */
    public static int getActiveSimCount() {
        return getTelephonyManagerProxy().getActiveModemCount();
    }

    /**
     * Returns how many logical SIM can be potentially active simultaneously, in terms of hardware
     * capability.
     * It might return different value from {@link #getActiveSimCount}. For example, for a
     * dual-SIM capable device operating in single SIM mode (only one logical modem is turned on),
     * {@link #getActiveSimCount} returns 1 while this API returns 2.
     */
    public static int getSupportedSimCount() {
        return getTelephonyManagerProxy().getSupportedModemCount();
    }

    /** Returns the phone id from the specified subscription id. */
    public static int getPhoneId(int subId) {
        return getPhoneId(subId, DEFAULT_PHONE_ID);
    }

    /** Returns the phone id from the specified subscription id. */
    public static int getPhoneId(int subId, int defaultPhoneId) {
        SubscriptionManagerProxy smp = getSubscriptionManagerProxy();
        int slotId = smp.getSlotIndex(subId);

        if (slotId < DEFAULT_PHONE_ID || slotId >= getActiveSimCount()) {
            // Set the phoneId as a default
            slotId = defaultPhoneId;
        }

        return slotId;
    }

    /** Returns a string representing the SIM state. */
    public static String getSimState(int slotId) {
        TelephonyManagerProxy tmp = getTelephonyManagerProxy();
        int simState = tmp.getSimState(slotId);

        if (simState == TelephonyManager.SIM_STATE_READY) {
            int subId = MSimUtils.getSubId(slotId);
            tmp = AppContext.getTelephonyManagerProxy(subId);

            int simAppState = tmp.getSimApplicationState();

            if (simAppState == TelephonyManager.SIM_STATE_LOADED) {
                simState = TelephonyManager.SIM_STATE_LOADED;
            }
        }

        switch (simState) {
            case TelephonyManager.SIM_STATE_ABSENT:
                return "ABSENT";
            case TelephonyManager.SIM_STATE_NOT_READY:
                return "NOT_READY";
            case TelephonyManager.SIM_STATE_LOADED:
                return "LOADED";
            case TelephonyManager.SIM_STATE_READY:
                return "READY";
            case TelephonyManager.SIM_STATE_PIN_REQUIRED: // FALL-THROUGH
            case TelephonyManager.SIM_STATE_PUK_REQUIRED: // FALL-THROUGH
            case TelephonyManager.SIM_STATE_NETWORK_LOCKED: // FALL-THROUGH
            case TelephonyManager.SIM_STATE_PERM_DISABLED:
                return "LOCKED";
            case TelephonyManager.SIM_STATE_CARD_IO_ERROR:
                return "CARD_IO_ERROR";
            case TelephonyManager.SIM_STATE_CARD_RESTRICTED:
                return "CARD_RESTRICTED";
            default:
                return "UNKNOWN";
        }
    }

    /** Returns the slot id from the specified subscription id. */
    public static int getSlotId(int subId) {
        return getSubscriptionManagerProxy().getSlotIndex(subId);
    }

    /** Returns the subscription id from the specified phone id. */
    public static int getSubId(int phoneId) {
        return getSubscriptionManagerProxy().getSubscriptionId(phoneId);
    }

    /** Checks whether SIM card is present or not. */
    public static boolean hasIccCard(int slotId) {
        TelephonyManagerProxy tmp = AppContext.getTelephonyManagerProxy(getSubId(slotId));
        return tmp.hasIccCard();
    }

    /** Checks if the multiple IMS is capable when the multiple SIMs support. */
    public static boolean isMultiImsEnabled() {
        return true;
    }

    /** Checks if the device supports the multiple SIM cards. */
    public static boolean isMultiSimEnabled() {
        return getActiveSimCount() > 1;
    }

    /** Checks if the specified subscription id is usable or not. */
    public static boolean isUsableSubId(int subId) {
        return getSubscriptionManagerProxy().isUsableSubscriptionId(subId);
    }

    /** Checks if the specified subscription id is valid or not. */
    public static boolean isValidSubId(int subId) {
        return getSubscriptionManagerProxy().isValidSubscriptionId(subId);
    }

    private static SubscriptionManagerProxy getSubscriptionManagerProxy() {
        return AppContext.getInstance().getSystemServiceProxy(SubscriptionManagerProxy.class);
    }

    private static TelephonyManagerProxy getTelephonyManagerProxy() {
        return AppContext.getInstance().getSystemServiceProxy(TelephonyManagerProxy.class);
    }

    private MSimUtils() {}
}
