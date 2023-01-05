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

/**
 * This provides an interface to access the subscription states.
 */
public interface ITelephonySubscriber extends IAgent {
    /**
     * Returns MCC/MNC value from SIM card if fromSim is 'true'
     * Returns MCC/MNC value from network operator if fromSim is 'false'
     */
    String getMccMnc(boolean fromSim);

    /**
     * Returns MCC/MNC value from target SIM card
     */
    String getMccMnc(int subId);

    /**
     * Returns MCC value from SIM card if fromSim is 'true'
     * Returns MCC value from network operator if fromSim is 'false'
     */
    String getMcc(boolean fromSim);

    /**
     * Returns MNC value from SIM card if fromSim is 'true'
     * Returns MNC value from network operator if fromSim is 'false'
     */
    String getMnc(boolean fromSim);

    /**
     * Returns Operator value based on MCC
     */
    String getSimOperatorInternal();

    /**
     * Returns Country Iso value via TelephonyManager
     * If it is not possible, return Country Iso value stored in TelephonySubscriberAgent.java class
     */
    String getCountryIso(boolean fromSim);

    /**
     * Returns line1number via TelephonyManager.
     */
    String getPhoneNumber();

    /**
     * Returns SimSerialNumber via TelephonyManager
     */
    String getSimSerialNumber();

    /**
     * Returns SubscriberId based on Active subId via TelephonyManager
     */
    String getSubscriberId();

    /**
     * Returns deviceId via TelephonyManager.
     */
    String getDeviceId();

    /**
     * Returns gid1 value from SIM.
     */
    String getGroupIdLevel1();

    /**
     * Returns operator name from SIM.
     */
    String getSimOperatorName();
}
