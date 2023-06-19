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
package com.android.imsstack.system;

/**
 * This defines the event types and its related parameters for the operations
 * between the Java and native layer.
 */
public interface ImsEventDef {
    //// Events from Java to Native {

    // Low battery state.
    int IMS_EVENT_POWER_LOW_BATTERY = 0x00000001;
    /**
     * Parameter (param1) of {@link #IMS_EVENT_POWER_LOW_BATTERY}.
     */
    // Indicates that the battery health has changed to under the low threshold.
    int IMS_POWER_LOW_BATTERY = 0;
    // Indicates that the battery health has changed over the low threshold.
    int IMS_POWER_LOW_CHANGED = 1;

    // CS (Circuit-Switched) call state.
    int IMS_EVENT_CSCALL_STATE = 0x00000002;
    /**
     * Parameter (param1) of {@link #IMS_EVENT_CSCALL_STATE}.
     */
    // Indicates that CS call state is in IDLE.
    int IMS_CSCALL_STATE_IDLE = 0;
    // Indicates that CS call state in INCOMING.
    int IMS_CSCALL_STATE_INCOMING = 1;
    // Indicates that CS call state is in ACTIVE.
    int IMS_CSCALL_STATE_ACTIVE = 2;

    // Network roaming state.
    int IMS_EVENT_ROAMING_STATE = 0x00000004;
    /**
     * Parameter (param1 / param2) of {@link #IMS_EVENT_ROAMING_STATE}.
     * param1: PS roaming state.
     * param2: CS roaming state.
     */
    // Indicates that the network is not in roaming.
    int IMS_ROAMING_STATE_OFF = 0;
    // Indicates that the network is in roaming.
    int IMS_ROAMING_STATE_ON = 1;

    // Emergency callback mode state.
    int IMS_EVENT_ECM_STATE = 0x00000008;
    /**
     * Parameter (param1) of {@link #IMS_EVENT_ECM_STATE}.
     */
    // Indicates that the device enters the emergency callback mode.
    int IMS_ECM_STATE_OFF = 0;
    // Indicates that the device exits the emergency callback mode.
    int IMS_ECM_STATE_ON = 1;
    // Indicates that the device exits the emergency callback mode by initiating a new call.
    int IMS_ECM_STATE_OFF_BY_NEW_ECALL = 2;

    // Voice service state.
    int IMS_EVENT_VOICE_SERVICE_STATE = 0x00000010;
    /**
     * Parameter (param1) of {@link #IMS_EVENT_VOICE_SERVICE_STATE}.
     */
    // Indicates that the voice service is in service.
    int IMS_VOICE_SERVICE_IN_SERVICE = 0;
    // Indicates that the voice service is out of service.
    int IMS_VOICE_SERVICE_OUT_OF_SERVICE = 1;
    // Indicates that the voice service is in emergency only.
    int IMS_VOICE_SERVICE_EMERGENCY_ONLY = 2;
    // Indicates that the voice service is powered off.
    int IMS_VOICE_SERVICE_POWER_OFF = 3;

    // LTE network information.
    int IMS_EVENT_LTE_INFO = 0x00000020;
    /**
     * Parameter (param1) of {@link #IMS_EVENT_LTE_INFO}.
     */
    // Indicates that the LTE network state is unknown.
    int IMS_LTE_INFO_UNKNOWN = 0;
    // Indicates that the device has attached to the LTE network with EPS only.
    int IMS_LTE_INFO_EPS_ONLY_ATTACHED = 1;
    // Indicates that the device has attached to the LTE network with combined.
    int IMS_LTE_INFO_COMBINED_ATTACHED = 2;
    /**
     * Parameter (param2) of {@link #IMS_EVENT_LTE_INFO}.
     */
    // Indicates that the network is attached without special information.
    int IMS_LTE_INFO_EXTRA_NONE = 0x0;
    // Indicates that the network is attached with CSFB_NOT_PREFERRED option.
    int IMS_LTE_INFO_EXTRA_CSFB_NOT_PREFERRED = 0x1;
    // Indicates that the network is attached with SMS only option.
    int IMS_LTE_INFO_EXTRA_SMS_ONLY = 0x2;

    // NR network information.
    int IMS_EVENT_NR_INFO = 0x00000040;
    /**
     * Parameter (param1) of {@link #IMS_EVENT_NR_INFO}.
     */
    // Indicates that the NR network state is unknown.
    int IMS_NR_INFO_UNKNOWN = 0;
    // Indicates that the device has registered to the NR network.
    int IMS_NR_INFO_REGISTRATION = 1;
    // Indicates that the device has deregistered from the NR network.
    int IMS_NR_INFO_DEREGISTRATION = 2;
    // Indicates that the device has registered to the NR network for emergency only.
    int IMS_NR_INFO_EMERGENCY_REGISTRATION = 3;

    // Voice over PS indicator.
    int IMS_EVENT_IMS_VOICE_OVER_PS_STATE = 0x00000080;
    /**
     * Parameter (param1) of {@link #IMS_EVENT_IMS_VOICE_OVER_PS_STATE}.
     */
    // Indicates that the voice over PS indication is not enabled.
    int IMS_VOICE_OVER_PS_NOT_SUPPORTED = 0;
    // Indicates that the voice over PS indication is enabled.
    int IMS_VOICE_OVER_PS_SUPPORTED = 1;

    // LTE connection establishment state.
    int IMS_EVENT_LTE_STATE = 0x00000100;
    /**
     * Parameter (param1) of {@link #IMS_EVENT_LTE_STATE}.
     */
    // Indicates that the LTE connection establishment is failed with RACH reject.
    // With this reason, the param2 contains the wait time as milli-seconds.
    int IMS_LTE_RACH_REJECT_WITH_WAITTIME = 11;
    // Indicates that the LTE connection establishment is failed that RACH should be ignored
    // during T300 timer.
    int IMS_LTE_RACH_IGNORE_DURING_T300_3TIMES = 12;
    // Indicates that the LTE connection establishment is failed with SR reject with EMM9/EMM10.
    int IMS_LTE_SR_REJECT_WITH_EMM9_EMM10 = 13;
    // Indicates that the LTE connection establishment is failed with SR reject with EMM17
    // for 3 times.
    int IMS_LTE_SR_REJECT_WITH_EMM17_3TIMES = 14;
    // Indicates that the LTE connection establishment is failed that SR should be ignored
    // during 5 seconds.
    int IMS_LTE_SR_IGNORE_DURING_5SEC = 15;
    // Indicates that the LTE connection establishment is failed with MO DATA barring.
    int IMS_LTE_BARRING_MO_DATA = 16;
    // Indicates that the LTE connection establishment is failed with SR reject with EMM.
    int IMS_LTE_SR_REJECT_WITH_EMM = 17;

    // Wi-Fi calling settings.
    int IMS_EVENT_WFC_SETTING_CHANGED = 0x00000200;
    /**
     * Parameter (param1) of {@link #IMS_EVENT_WFC_SETTING_CHANGED}.
     * Wi-Fi calling setting enabled/disabled.
     */
    // Indicates that the Wi-Fi calling setting is enabled.
    int IMS_WFC_ON = 1;
    // Indicates that the Wi-Fi calling setting is disabled.
    int IMS_WFC_OFF = 0;
    /**
     * Parameter (param2) of {@link #IMS_EVENT_WFC_SETTING_CHANGED}.
     * Wi-Fi calling mode setting.
     */
    // Indicates that the Wi-Fi calling mode is set to Wi-Fi preferred.
    int MODE_WFC_PREFERRED = 2;
    // Indicates that the Wi-Fi calling mode is set to Wi-Fi only.
    int MODE_WFC_ONLY = 0;
    // Indicates that the Wi-Fi calling mode is set to cellular preferred.
    int MODE_CELLULAR_PREFERRED = 1;
    // Indicates that the Wi-Fi calling mode is set to the IMS preferred.
    int MODE_IMS_PREFERRED = 10;

    // VoLTE settings.
    int IMS_EVENT_VOLTE_SETTING = 0x00000400;
    /**
     * Parameter (param1) of {@link #IMS_EVENT_VOLTE_SETTING}.
     */
    // Indicates that the VoLTE setting is disabled.
    int IMS_VOLTE_SETTING_OFF = 0x00000000;
    // Indicates that the VoLTE setting is enabled.
    int IMS_VOLTE_SETTING_ON = 0x00000001;

    // AC barring state.
    int IMS_EVENT_AC_BARRING_STATE = 0x00000800;

    // IMS registration control event.
    int IMS_EVENT_REG_CONTROL = 0x00001000;
    /**
     * Parameter (param1) of {@link #IMS_EVENT_REG_CONTROL}.
     */
    // Indicates that the registration should be recovered.
    int IMS_REG_CONTROL_RECOVER = 1;
    // Indicates that the registration should be updated.
    int IMS_REG_CONTROL_UPDATE = 2;
    // Indicates that the registration should be destroyed.
    int IMS_REG_CONTROL_DESTROY = 3;
    // Indicates that the registration should be updated because of the IPCAN change.
    int IMS_REG_CONTROL_IPCAN = 4;
    // Indicates that the registration should be stopped.
    int IMS_REG_CONTROL_STOP = 5;
    // Indicates that the registration should be stopped for Wi-Fi calling.
    int IMS_REG_CONTROL_WIFICALL_STOP = 6;
    // Indicates that the registration should be retried with the next P-CSCF.
    int IMS_REG_CONTROL_PCSCF = 7;
    /**
     * Parameter (param2) of {@link #IMS_EVENT_REG_CONTROL}.
     */
    // Indicates that the data connection can be kept or not.
    // Used when the param1 is {@link #IMS_REG_CONTROL_RECOVER}.
    int IMS_REG_CONTROL_KEEP_DATA_CONNECTION = 11;
    // Indicates that the DCN should be handled or not.
    // Used when the param1 is {@link #IMS_REG_CONTROL_DESTROY}.
    int IMS_REG_CONTROL_DESTROY_DCN = 31;
    // Indicates that the registration should be stopped or not.
    // Used when the param1 is {@link #IMS_REG_CONTROL_IPCAN}.
    int IMS_REG_CONTROL_IPCAN_STOP = 41;
    // Indicates that the registration should be handed over from Wi-Fi to LTE.
    // Used when the param1 is {@link #IMS_REG_CONTROL_IPCAN}.
    int IMS_REG_CONTROL_IPCAN_WIFITOLTE = 42;
    // Indicates that the registration should be updated using the same P-CSCF.
    // Used when the param1 is {@link #IMS_REG_CONTROL_PCSCF}.
    int IMS_REG_CONTROL_PCSCF_SAME_CHANGED = 71;

    // RTT settings.
    int IMS_EVENT_RTT_SETTING = 0x00002000;
    /**
     * Parameter (param2) of {@link #IMS_EVENT_RTT_SETTING}.
     */
    // Indicates that the RTT mode is unknown.
    int IMS_RTT_MODE_NONE = -1;
    // Indicates that the RTT is not capable.
    int IMS_RTT_CAPABLE_OFF = 0;
    // Indicates that the RTT is visible during call.
    int IMS_RTT_VISIBLE_DURING_CALLS = 1;
    // Indicates that the RTT is always visible.
    int IMS_RTT_ALWAYS_VISIBLE = 2;

    //// }

    //// Events from Native to Java {

    // Event for notifying that the native service is successfully loaded.
    int IMS_EVENT_NATIVE_BOOT_COMPLETED = 0x00000001;
    // Event for acquiring the wake lock for the internal operations.
    int IMS_EVENT_WAKE_LOCK = 0x00000002;
    // Event for enabling / disabling the Wi-Fi service.
    int IMS_EVENT_WIFI_SERVICE = 0x00000004;
    /**
     * Parameter(param1) of {@link #IMS_EVENT_WIFI_SERVICE}.
     */
    // Indicates that the Wi-Fi service should be turned off.
    int IMS_WIFI_OFF = 0;
    // Indicates that the Wi-Fi service should be turned on.
    int IMS_WIFI_ON = 1;

    //// }
}
