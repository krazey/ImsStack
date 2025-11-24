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
#ifndef INTERFACE_IMS_AOS_SERVICE_H_
#define INTERFACE_IMS_AOS_SERVICE_H_

#include "ImsMessageDef.h"
#include "ImsMap.h"

class IIAosService
{
public:
    static const IMS_SINT32 EVENT_J2N = IMS_MSG_BASE_SERVICE + 70;
    static const IMS_SINT32 EVENT_N2J = IMS_MSG_BASE_SERVICE + 170;

    static const IMS_SINT32 EVENT_J2N_INFO = EVENT_J2N + 20;
    static const IMS_SINT32 EVENT_N2J_INFO = EVENT_N2J + 20;

public:
    /**
     * Messages from Java layer to native layer
     */
    /// IAosRegistration(Java) -> IAosRegistrationControlListener(Native)
    static const IMS_SINT32 J2N_REQUEST_REGISTRATION = EVENT_J2N + 1;
    static const IMS_SINT32 J2N_REQUEST_DEREGISTRATION = EVENT_J2N + 2;
    static const IMS_SINT32 J2N_REQUEST_FULL_REGISTRATION = EVENT_J2N + 3;
    static const IMS_SINT32 J2N_REQUEST_CAPABILITIES_CHANGED = EVENT_J2N + 4;
    static const IMS_SINT32 J2N_REQUEST_CONTROL_REGISTRATION = EVENT_J2N + 5;
    static const IMS_SINT32 J2N_UPDATE_DATA_FAILURE_REASON = EVENT_J2N + 6;

    /// IAosInfo(Java) -> IAosServiceSettingListener(Native)
    static const IMS_SINT32 J2N_NOTIFY_AIRPLANE_SETTING = EVENT_J2N_INFO + 1;
    static const IMS_SINT32 J2N_NOTIFY_DATA_ROAMING_SETTING = EVENT_J2N_INFO + 2;
    static const IMS_SINT32 J2N_NOTIFY_MOBILE_DATA_SETTING = EVENT_J2N_INFO + 3;
    static const IMS_SINT32 J2N_NOTIFY_ROAMING_PREFERRED_VOICE_NETWORK = EVENT_J2N_INFO + 4;
    static const IMS_SINT32 J2N_NOTIFY_SERVICE_SETTING = EVENT_J2N_INFO + 5;
    static const IMS_SINT32 J2N_NOTIFY_TTY_SETTING = EVENT_J2N_INFO + 6;
    static const IMS_SINT32 J2N_NOTIFY_VIDEO_SETTING = EVENT_J2N_INFO + 7;
    static const IMS_SINT32 J2N_NOTIFY_VOLTE_SETTING = EVENT_J2N_INFO + 8;
    static const IMS_SINT32 J2N_NOTIFY_WFC_SETTING = EVENT_J2N_INFO + 9;

    /// IAosInfo(Java) -> IAosServicePhoneListener(Native)
    static const IMS_SINT32 J2N_NOTIFY_AOS_START = EVENT_J2N_INFO + 10;
    static const IMS_SINT32 J2N_NOTIFY_IPCAN_HANDOVER_FAILURE = EVENT_J2N_INFO + 11;
    static const IMS_SINT32 J2N_NOTIFY_ISIM_STATE = EVENT_J2N_INFO + 12;
    static const IMS_SINT32 J2N_NOTIFY_LOCATION_INFO = EVENT_J2N_INFO + 13;
    static const IMS_SINT32 J2N_NOTIFY_MOBILE_DATA_LIMIT = EVENT_J2N_INFO + 14;
    static const IMS_SINT32 J2N_NOTIFY_NETWORK_VIDEO_CAPABILITY = EVENT_J2N_INFO + 15;
    static const IMS_SINT32 J2N_NOTIFY_PHONE_NUMBER_STATE = EVENT_J2N_INFO + 16;
    static const IMS_SINT32 J2N_NOTIFY_PLMN_CHANGED = EVENT_J2N_INFO + 17;
    static const IMS_SINT32 J2N_NOTIFY_POWER_OFF = EVENT_J2N_INFO + 18;
    static const IMS_SINT32 J2N_NOTIFY_PRECISE_CALL_STATE = EVENT_J2N_INFO + 19;
    static const IMS_SINT32 J2N_NOTIFY_CARRIER_SIGNAL_PCO_VALUE_CHANGED = EVENT_J2N_INFO + 20;
    /// IAosInfo(Java) -> IAosEmergencyListener(Native)
    static const IMS_SINT32 J2N_NOTIFY_EMERGENCY_CALLBACK_MODE_CHANGED = EVENT_J2N_INFO + 21;
    /// IAosInfo(Java) -> IAosServicePhoneListener(Native)
    static const IMS_SINT32 J2N_NOTIFY_CROSS_SIM_STATUS = EVENT_J2N_INFO + 22;
    /// IAosInfo(Java) -> IAosServiceSettingListener(Native)
    static const IMS_SINT32 J2N_NOTIFY_WIFI_SETTING = EVENT_J2N_INFO + 23;
    /// IAosInfo(Java) -> IAosServicePhoneListener(Native)
    static const IMS_SINT32 J2N_NOTIFY_VOPS_STATE_CHANGED = EVENT_J2N_INFO + 24;
    static const IMS_SINT32 J2N_NOTIFY_NAS_ALGORITHM_CHANGED = EVENT_J2N_INFO + 25;
    static const IMS_SINT32 J2N_NOTIFY_ALLOWED_NETWORK_TYPES_CHANGED = EVENT_J2N_INFO + 26;
    static const IMS_SINT32 J2N_NOTIFY_EMERGENCY_REGISTRATION_STATE_CHANGED = EVENT_J2N_INFO + 27;
    static const IMS_SINT32 J2N_NOTIFY_SIM_STATE_CHANGED = EVENT_J2N_INFO + 28;

    /**
     * Messages from native layer to java layer
     */
    /// AosService(Native) -> IAosRegistrationListener(Java)
    static const IMS_SINT32 N2J_NOTIFY_REGISTERED = EVENT_N2J + 1;
    static const IMS_SINT32 N2J_NOTIFY_REGISTERING = EVENT_N2J + 2;
    static const IMS_SINT32 N2J_NOTIFY_DEREGISTERED = EVENT_N2J + 3;
    static const IMS_SINT32 N2J_NOTIFY_DEREGISTERING = EVENT_N2J + 4;
    static const IMS_SINT32 N2J_NOTIFY_TECHNOLOGY_CHANGE_FAILED = EVENT_N2J + 5;
    static const IMS_SINT32 N2J_NOTIFY_ASSOCIATED_URI_CHANGED = EVENT_N2J + 6;
    static const IMS_SINT32 N2J_NOTIFY_CAPABILITIES_UPDATE_FAILED = EVENT_N2J + 7;
    static const IMS_SINT32 N2J_NOTIFY_REG_EVENT_STATE = EVENT_N2J + 8;
    static const IMS_SINT32 N2J_NOTIFY_IMS_FEATURE_CHANGED = EVENT_N2J + 9;
    static const IMS_SINT32 N2J_NOTIFY_TRACE = EVENT_N2J + 10;

    /// AosService(Native) -> IAosInfoListener(Java)
    static const IMS_SINT32 N2J_NOTIFY_AOS_ISIM_STATE = EVENT_N2J_INFO + 1;
    static const IMS_SINT32 N2J_REQUEST_PHONE_NUMBER_RETRY = EVENT_N2J_INFO + 2;
    static const IMS_SINT32 N2J_REQUEST_WIFI_SERVICE = EVENT_N2J_INFO + 3;
};

#endif  // INTERFACE_IMS_AOS_SERVICE_H_
