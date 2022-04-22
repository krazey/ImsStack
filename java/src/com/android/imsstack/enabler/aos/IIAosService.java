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

public interface IIAosService {

    public static final int EVENT_J2N = 1000 + 70;
    public static final int EVENT_N2J = 1100 + 70;

    public static final int EVENT_J2N_INFO = EVENT_J2N + 20;
    public static final int EVENT_N2J_INFO = EVENT_N2J + 20;

    /**
     * Messages from Java layer to native layer
     */
    /// IAosRegistration(Java) -> IAosRegistrationControlListener(Native)
    public static final int J2N_REQUEST_REGISTRATION = EVENT_J2N + 1;
    public static final int J2N_REQUEST_DEREGISTRATION = EVENT_J2N + 2;
    public static final int J2N_REQUEST_FULL_REGISTRATION = EVENT_J2N + 3;
    public static final int J2N_REQUEST_CAPABILITIES_CHANGED = EVENT_J2N + 4;

    /// IAosInfo(Java) -> IAosServiceSettingListener(Native)
    public static final int J2N_NOTIFY_AIRPLANE_SETTING = EVENT_J2N_INFO + 1;
    public static final int J2N_NOTIFY_DATA_ROAMING_SETTING = EVENT_J2N_INFO + 2;
    public static final int J2N_NOTIFY_MOBILE_DATA_SETTING = EVENT_J2N_INFO + 3;
    public static final int J2N_NOTIFY_ROAMING_PREFERRED_VOICE_NETWORK = EVENT_J2N_INFO + 4;
    public static final int J2N_NOTIFY_SERVICE_SETTING = EVENT_J2N_INFO + 5;
    public static final int J2N_NOTIFY_TTY_SETTING = EVENT_J2N_INFO + 6;
    public static final int J2N_NOTIFY_VIDEO_SETTING = EVENT_J2N_INFO + 7;
    public static final int J2N_NOTIFY_VOLTE_SETTING = EVENT_J2N_INFO + 8;
    public static final int J2N_NOTIFY_WFC_SETTING = EVENT_J2N_INFO + 9;

    /// IAosInfo(Java) -> IAosServicePhoneListener(Native)
    public static final int J2N_NOTIFY_AOS_START = EVENT_J2N_INFO + 10;
    public static final int J2N_NOTIFY_IPCAN_HANDOVER_FAILURE = EVENT_J2N_INFO + 11;
    public static final int J2N_NOTIFY_ISIM_STATE = EVENT_J2N_INFO + 12;
    public static final int J2N_NOTIFY_LOCATION_INFO = EVENT_J2N_INFO + 13;
    public static final int J2N_NOTIFY_MOBILE_DATA_LIMIT = EVENT_J2N_INFO + 14;
    public static final int J2N_NOTIFY_NETWORK_VIDEO_CAPABILITY = EVENT_J2N_INFO + 15;
    public static final int J2N_NOTIFY_PHONE_NUMBER_STATE = EVENT_J2N_INFO + 16;
    public static final int J2N_NOTIFY_PLMN_CHANGED = EVENT_J2N_INFO + 17;
    public static final int J2N_NOTIFY_POWER_OFF = EVENT_J2N_INFO + 18;
    public static final int J2N_NOTIFY_PRECISE_CALL_STATE = EVENT_J2N_INFO + 19;

    /**
     * Messages from native layer to java layer
     */
    /// AosService(Native) -> IAosRegistrationListener(Java)
    public static final int N2J_NOTIFY_REGISTERED = EVENT_N2J + 1;
    public static final int N2J_NOTIFY_REGISTERING = EVENT_N2J + 2;
    public static final int N2J_NOTIFY_DEREGISTERED = EVENT_N2J + 3;
    public static final int N2J_NOTIFY_TECHNOLOGY_CHANGE_FAILED = EVENT_N2J + 4;
    public static final int N2J_NOTIFY_ASSOCIATED_URI_CHANGED = EVENT_N2J + 5;
    public static final int N2J_NOTIFY_CAPABILITIES_UPDATE_FAILED = EVENT_N2J + 6;

    /// AosService(Native) -> IAosInfoListener(Java) or Send it directly without a listener
    public static final int N2J_NOTIFY_AOS_ISIM_STATE = EVENT_N2J_INFO + 1;
    public static final int N2J_NOTIFY_REG_EVENT_STATE = EVENT_N2J_INFO + 2;
    public static final int N2J_REQUEST_PHONE_NUMBER_RETRY = EVENT_N2J_INFO + 3;
    public static final int N2J_REQUEST_WIFI_SERVICE = EVENT_N2J_INFO + 4;
}