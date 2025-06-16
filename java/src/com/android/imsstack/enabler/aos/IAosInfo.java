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
package com.android.imsstack.enabler.aos;

import android.content.Intent;

import androidx.annotation.NonNull;

import com.android.imsstack.core.agents.Sim;

/**
 * This class provides the interworking interface between Java and native layer
 * for AoS(Always On Service) functionalities.
 */
public interface IAosInfo {

    /**
     * Registers a new Listener to receive AoS Information updates.
     *
     * @param listener {@link IAosInfoListener} to listen to the events of this object.
     * @throws NullPointerException if the listener is null.
     */
    void addListener(@NonNull IAosInfoListener listener);

    /**
     * Removes a listener previously registered with {@link #addListener(IAosInfoListener)}.
     *
     * @param listener {@link IAosInfoListener} previously registered.
     * @throws NullPointerException if the listener is null.
     */
    void removeListener(@NonNull IAosInfoListener listener);

    /**
     * Called to notify the change of data roaming setting.
     * Native Listener : IAosServiceSettingListener.
     *
     * @param isAllowed {@code isAllowed} is {@code true} if allowed, {@code false} if not allowed.
     */
    void notifyDataRoamingSetting(boolean isAllowed);

    /**
     * Called to notify the change of mobile data setting.
     * Native Listener : IAosServiceSettingListener.
     *
     * @param isOn {@code isOn} is {@code true} if on, {@code false} if off.
     */
    void notifyMobileDataSetting(boolean isOn);

    /**
     * Called to notify the change of roaming preferred voice network.
     * Native Listener : IAosServiceSettingListener.
     *
     * @param state {@code state} is type of {@link RoamingPreferredVoiceNetwork}.
     */
    void notifyRoamingPreferredVoiceNetwork(RoamingPreferredVoiceNetwork state);

    /**
     * Called to notify the change of service setting.
     * Native Listener : IAosServiceSettingListener.
     *
     * @param state {@code state} is type of {@link ServiceSetting}.
     * @param serviceBits {@code serviceBits} is one of
     *          {@link com.android.imsstack.enabler.aos.IAosRegistrationListener.FeatureTagMask}.
     *          Valid values are the following : ALL(0xFFFFFFFF), MMTEL(0x00000001),
     *          VIDEO(0x00000002), SMSIP(0x00000020), and so on
     */
    void notifyServiceSetting(ServiceSetting state, int serviceBits);

    /**
     * Called to notify the change of TTY setting.
     * Native Listener : IAosServiceSettingListener.
     *
     * @param isOn {@code isOn} is {@code true} if on, {@code false} if off.
     */
    void notifyTtySetting(boolean isOn);

    /**
     * Called to notify the change of video setting.
     * Native Listener : IAosServiceSettingListener.
     *
     * @param isOn {@code isOn} is {@code true} if on, {@code false} if off.
     */
    void notifyVideoSetting(boolean isOn);

    /**
     * Called to notify the change of VoLTE setting.
     * Native Listener : IAosServiceSettingListener.
     *
     * @param isOn {@code isOn} is {@code true} if on, {@code false} if off.
     */
    void notifyVolteSetting(boolean isOn);

    /**
     * Called to notify the change of WFC setting.
     * Native Listener : IAosServiceSettingListener.
     *
     * @param isOn {@code isOn} is {@code true} if on, {@code false} if off.
     */
    void notifyWfcSetting(boolean isOn);


    /**
     * Called to notify the start of AoS Service.
     * Native Listener : IAosServicePhoneListener.
     */
    void notifyAosStart();

    /**
     * Called to notify the change of ISIM state.
     * Native Listener : IAosServicePhoneListener.
     *
     * @param state {@code state} The ISIM state. Valid values are
     *         {@link Sim#ISIM_STATE_UNKNOWN},
     *         {@link Sim#ISIM_STATE_NOT_PRESENT},
     *         {@link Sim#ISIM_STATE_NOT_READY},
     *         {@link Sim#ISIM_STATE_LOADED},
     *         {@link Sim#ISIM_STATE_REFRESH_STARTED},
     *         {@link Sim#ISIM_STATE_REFRESH_COMPLETED},
     *         {@link Sim#ISIM_STATE_REMOVED}.
     */
    void notifyIsimState(@Sim.IsimState int state);

    /**
     * Called to notify the change of mobile data limit.
     * Native Listener : IAosServicePhoneListener.
     *
     * @param isLimited {@code isLimited} is {@code true} if limited, {@code false} if not limited.
     */
    void notifyMobileDataLimit(boolean isLimited);

    /**
     * Called to notify the change of network capability.
     * Native Listener : IAosServicePhoneListener.
     *
     * @param isOn {@code isOn} is {@code true} if on, {@code false} if off.
     */
    void notifyNetworkVideoCapability(boolean isOn);

    /**
     * Called to notify the change of phone number state.
     * Native Listener : IAosServicePhoneListener.
     *
     * @param isRefresh {@code isRefresh} {@code true} if refresh action, {@code false} if initial
     * action.
     * @param state {@code state} is type of {@link PhoneNumberState}
     */
    void notifyPhoneNumberState(boolean isRefresh, PhoneNumberState state);

    /**
     * Called to notify the power off.
     * Native Listener : IAosServicePhoneListener.
     *
     */
    void notifyPowerOff();

    /**
     * Notifies the listener of a change in carrier signal PCO value.
     * Native Listener: IAosServicePhoneListener
     *
     * @param intent Intent containing the carrier signal PCO value information.
     * @throws NullPointerException if {@code intent} is null.
     */
    void notifyCarrierSignalPcoValueChanged(@NonNull Intent intent);

    /**
     * Called to notify the update of emergency callback mode.
     * Native Listener : IAosEmergencyListener.
     *
     * @param type {@code type} is callback mode entry {@link EmergencyCallbackModeType}
     * @param state {@code state} is type of {@link EmergencyCallbackModeState}.
     * @param duration is the number of seconds remaining in the emergency callback mode.
     */
    void notifyEmergencyCallbackModeChanged(
            EmergencyCallbackModeType type, EmergencyCallbackModeState state, long duration);

    /**
     * Called to notify the change of NAS security algorithm.
     *
     * @param isNullAlgo {@code isNullAlgo} {@code true} if NAS security algorithm is null,
     * {@code false} if it's not null.
     */
    void notifyNasSecurityAlgorithmChanged(boolean isNullAlgo);

    /**
     * Represents the preferred voice network for roaming.
     */
    enum RoamingPreferredVoiceNetwork {

        CELLULAR(0),
        WIFI(1);

        private final int mValue;

        RoamingPreferredVoiceNetwork(int value) {
            mValue = value;
        }

        /**
         * Returns the integer value associated with this network type.
         *
         * @return The integer value.
         */
        public int getValue() {
            return mValue;
        }
    }

    /**
     * Represents the service setting.
     */
    enum ServiceSetting {

        OFF(0),
        ON(1),
        PRESENTITY(2);

        private final int mValue;

        ServiceSetting(int value) {
            mValue = value;
        }

        /**
         * Returns the integer value associated with this service setting.
         *
         * @return The integer value.
         */
        public int getValue() {
            return mValue;
        }
    }

    /**
     * Represents the status of location information.
     */
    enum LocationInfo {

        FIXED(1),
        COUNTRY_CHANGED(2),
        CHANGED(3),
        AVAILABLE(4);

        private final int mValue;

        LocationInfo(int value) {
            mValue = value;
        }

        /**
         * Returns the integer value associated with this location status.
         *
         * @return The integer value.
         */
        public int getValue() {
            return mValue;
        }
    }

    /**
     * Represents the state of a phone number.
     */
    enum PhoneNumberState {

        SIM_LOADED(0),
        RETRY_SUCCESS(1),
        RETRY_FAILURE(2);

        private final int mValue;

        PhoneNumberState(int value) {
            mValue = value;
        }

        /**
         * Returns the integer value associated with this phone number state.
         *
         * @return The integer value.
         */
        public int getValue() {
            return mValue;
        }
    }

    /*
     * Represents the connection status through cross sim
     */
    enum CrossSimStatus {

        DATA_DISCONNECTED(0),
        DATA_CONNECTED(1);

        private final int mValue;

        CrossSimStatus(int value) {
            mValue = value;
        }

        /**
         * Returns the integer value associated with the connection of cross sim.
         *
         * @return The integer value.
         */
        public int getValue() {
            return mValue;
        }
    }

    /**
     * Represents the type of emergency callback mode.
     */
    enum EmergencyCallbackModeType {

        CALL(1),
        SMS(2);

        private final int mValue;

        EmergencyCallbackModeType(int value) {
            mValue = value;
        }

        /**
         * Returns the integer value associated with this emergency callback mode type.
         *
         * @return The integer value.
         */
        public int getValue() {
            return mValue;
        }
    }

    /**
     * Represents the state of the emergency callback mode.
     */
    enum EmergencyCallbackModeState {

        STOP(0),
        START(1),
        STOP_BY_EMERGENCY(2);

        private final int mValue;

        EmergencyCallbackModeState(int value) {
            mValue = value;
        }

        /**
         * Returns the integer value associated with this emergency callback mode state.
         *
         * @return The integer value.
         */
        public int getValue() {
            return mValue;
        }
    }
}
