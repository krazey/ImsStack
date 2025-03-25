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

import android.telephony.Annotation.CallState;
import android.telephony.Annotation.NetworkType;
import android.telephony.TelephonyManager;
import android.telephony.TelephonyManager.SimState;
import android.telephony.emergency.EmergencyNumber;

import androidx.annotation.NonNull;

import java.util.List;

/**
 * An interface for accessing the Telephony or SIM states through {@link TelephonyManager} or
 * {@link SubscriptionManager}.
 */
public interface TelephonyInterface extends IAgent {
    /**
     * Returns the Telephony call state for CS calls.
     *
     * @return The Telephony call state.
     */
    @CallState int getCsCallState();

    /**
     * Returns the Telephony call state for CS calls of other slots.
     *
     * @return The Telephony call state of other slot.
     */
    @CallState int getCsCallStateInOtherSlot();

    /**
     * Returns the current data network type.
     *
     * @return The network type.
     */
    @NetworkType int getNetworkType();

    /**
     * Returns the current voice network type.
     *
     * @return The voice network type.
     */
    @NetworkType int getVoiceNetworkType();

    /**
     * Returns the state of the device SIM card.
     *
     * @return The SIM card's state.
     */
    @SimState int getSimState();

    /**
     * Returns the IMEI (International Mobile Equipment Identity).
     * Returns null if IMEI is not available.
     */
    String getImei();

    /**
     * Returns the software version number for the device, for example, the IMEI/SV for GSM phones.
     * Returns null if the software version is not available.
     */
    String getDeviceSoftwareVersion();

    /**
     * Returns the phone number, or an empty string if not available.
     */
    String getPhoneNumber();

    /**
     * Returns the unique subscriber ID, for example, the IMSI for a GSM phone.
     * Returns null if it is unavailable.
     */
    String getSubscriberId();

    /**
     * Returns the MCC+MNC (Mobile Country Code + Mobile Network Code) of the provider of the SIM.
     * 5 or 6 decimal digits.
     */
    String getSimOperator();

    /**
     * Returns the MCC (Mobile Country Code) of the provider of the SIM.
     */
    String getSimMcc();

    /**
     * Returns the MNC (Mobile Network Code) of the provider of the SIM.
     */
    String getSimMnc();

    /**
     * Returns the ISO-3166-1 alpha-2 country code equivalent for the SIM provider's country code.
     */
    String getSimCountryIso();

    /**
     * Returns the serial number of the SIM, if applicable.
     * Returns null if it is unavailable.
     */
    String getSimSerialNumber();

    /**
     * Returns the Group Identifier Level1 for a GSM phone.
     * Returns null if it is unavailable.
     */
    String getSimGid1();

    /**
     * Returns the Service Provider Name (SPN).
     */
    String getSimOperatorName();

    /**
     * Returns the MCC+MNC (Mobile Country Code + Mobile Network Code) of the current registered
     * operator. 5 or 6 decimal digits.
     */
    String getNetworkOperator();

    /**
     * Returns the MCC (Mobile Country Code) of the current registered operator.
     */
    String getNetworkMcc();

    /**
     * Returns the MNC (Mobile Network Code) of the current registered operator.
     */
    String getNetworkMnc();

    /**
     * Returns the ISO-3166-1 alpha-2 country code equivalent of the MCC (Mobile Country Code) of
     * the current registered operator or the cell nearby, if available.
     */
    String getNetworkCountryIso();

    /**
     * Checks whether the specified number is an emergency number or not.
     *
     * @return {@code true} if the number is an emergency number, {@code false} otherwise.
     */
    boolean isEmergencyNumber(String number);

    /**
     * Get the emergency number list based on current locale, sim, default, modem and network.
     *
     * @return The list of {@link EmergencyNumber}
     */
    @NonNull List<EmergencyNumber> getEmergencyNumberList();

    /** Returns a string represented by the given call state. */
    static String callStateToString(@CallState int state) {
        switch (state) {
            case TelephonyManager.CALL_STATE_RINGING:
                return "RINGING";
            case TelephonyManager.CALL_STATE_OFFHOOK:
                return "OFFHOOK";
            default:
                return "IDLE";
        }
    }
}
