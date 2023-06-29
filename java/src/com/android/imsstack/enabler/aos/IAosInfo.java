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

/**
 * This class provides the interworking interface between Java and native layer
 * for AoS(Always On Service) functionalities.
 */
public interface IAosInfo {

    /**
     * Registers a new Listener to receive AoS Information updates.
     *
     * @param listener {@link IAosInfoListener} to listen to the events of this object.
     */
    void addListener(IAosInfoListener listener);

    /**
     * Removes a listener previously registered with {@link #addListener(IAosInfoListener)}.
     *
     * @param listener {@link IAosInfoListener} previously registered.
     */
    void removeListener(IAosInfoListener listener);

    /**
     * Called to notify the change of airplane setting.
     * Native Listener : IAosServiceSettingListener.
     *
     * @param isOn {@code isOn} is {@code true} if on, {@code false} if off.
     */
    void notifyAirplaneSetting(boolean isOn);

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
    void notifyRoamingPreferredVoiceNetwork(int state);

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
    void notifyServiceSetting(int state, int serviceBits);

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
     * Called to notify the failure of IPCAN Handover.
     * Native Listener : IAosServicePhoneListener.
     *
     * @param targetNetwork {@code targetNetwork} is the technology that has failed to be changed
     * to. One of {@link com.android.imsstack.core.agents.dcmif.IApn#IPCAN_CATEGORY_XXX}.
     * @param causeCode handover failure cause code. one of {@link android.telephony.DataFailCause}.
     */
    void notifyIpcanHandoverFailure(int targetNetwork, int causeCode);

    /**
     * Called to notify the change of ISIM state.
     * Native Listener : IAosServicePhoneListener.
     *
     * @param state {@code state} is type of {@link IsimState}.
     */
    void notifyIsimState(int state);

    /**
     * Called to notify the change of location information.
     * Native Listener : IAosServicePhoneListener.
     *
     * @param state {@code state} is type of {@link LocationInfo}.
     */
    void notifyLocationInfo(int state);

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
    void notifyPhoneNumberState(boolean isRefresh, int state);

    /**
     * Called to notify the change of PLMN.
     * Native Listener : IAosServicePhoneListener.
     *
     */
    void notifyPlmnChanged();

    /**
     * Called to notify the power off.
     * Native Listener : IAosServicePhoneListener.
     *
     */
    void notifyPowerOff();

    /**
     * Called to notify the change of precise call state.
     * Native Listener : IAosServicePhoneListener.
     *
     * @param state {@code state} is type of {@link PreciseCallState}.
     */
    void notifyPreciseCallState(int state);

    /**
     * Called to notify the change of carrier signal PCO value.
     * Native Listener : IAosServicePhoneListener.
     *
     * @param intent Intent for Carrier signal pco value.
     */
    void notifyCarrierSignalPcoValueChanged(Intent intent);

    /**
     * Called to notify the update of emergency callback mode.
     * Native Listener : IAosEmergencyListener.
     *
     * @param type {@code type} is callback mode entry {@link EmcCallbackType}
     * @param state {@code state} is type of {@link EmcCallbackMode}.
     * @param duration is the number of seconds remaining in the emergency callback mode.
     */
    void notifyEmcCallbackModeChanged(int type, int state, long duration);

    /**
     * Called to notify the change of CrossSim connection status.
     *
     * @param isConnectedOverCrossSim
     * {@code isConnectedOverCrossSim} is {@code true} if CrossSim is used for IMS service,
     * {@code false} if CrossSim feature is not used for IMS service
     */
    void notifyCrossSimStatus(boolean isConnectedOverCrossSim);

    /**
     * Roaming Preferred Voice Network
     */
    class RoamingPreferredVoiceNetwork {

        public static final int CELLULAR = 0;
        public static final int WIFI = 1;
    }

    /**
     * Service setting
     */
    class ServiceSetting {

        public static final int OFF = 0;
        public static final int ON = 1;
        public static final int PRESENTITY = 2;
    }

    /**
     * ISIM State
     */
    class IsimState {

        public static final int NOT_PRESENT = 0;
        public static final int NOT_READY = 1;
        public static final int LOADED = 2;
        public static final int REFRESH_STARTED = 3;
        public static final int REFRESH_COMPLETED = 4;
    }

    /**
     * Location Information
     */
    class LocationInfo {

        public static final int FIXED = 1;
        public static final int COUNTRY_CHANGED = 2;
        public static final int CHANGED = 3;
        public static final int AVAILABLE = 4;
    }

    /**
     * PhoneNumber state
     */
    class PhoneNumberState {

        public static final int SIM_LOADED = 0;
        public static final int RETRY_SUCCESS = 1;
        public static final int RETRY_FAILURE = 2;
    }

    /**
     * Precise call state
     */
    class PreciseCallState {

        public static final int NOT_VALID = -1;
        public static final int IDLE = 0;
        public static final int ACTIVE = 1;
        public static final int HOLDING = 2;
        public static final int DIALING = 3;
        public static final int ALERTING = 4;
        public static final int INCOMING = 5;
        public static final int WAITING = 6;
        public static final int DISCONNECTED = 7;
        public static final int DISCONNECTING = 8;
    }

    /**
     * Emergency callback mode type
     */
    class EmcCallbackModeType {
        public static final int CALL = 1;
        public static final int SMS = 2;
    };

    /**
     * Emergency callback mode state
     */
    class EmcCallbackMode {
        public static final int STOP = 0;
        public static final int START = 1;
        public static final int STOP_BY_EMC = 2;
    };
}