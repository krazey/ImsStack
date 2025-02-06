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
 * Defines the constant values for the system interface.
 */
public interface SystemConstants {
    /**
     * Category for system interface.
     */
    int CATEGORY_BASE = 0x00000000;
    int CATEGORY_NETWORK = 0x00010000;
    int CATEGORY_WIFI = 0x00020000;
    int CATEGORY_POWER = 0x00040000;
    int CATEGORY_TIMER = 0x00080000;
    int CATEGORY_CONFIG = 0x00100000;
    int CATEGORY_EVENT = 0x00200000;
    int CATEGORY_ISIM_EVENT = 0x00400000;
    int CATEGORY_USIM_EVENT = 0x00800000;
    int CATEGORY_RADIO = 0x01000000;

    /**
     * Methods for the system information's notification (Java to Native)
     */
    int NOTIFY_AIRPLANE_MODE_CHANGED = CATEGORY_NETWORK + 1;
    int NOTIFY_TIMER_EXPIRED = CATEGORY_TIMER + 2;
    int NOTIFY_BATTERY_LEVEL_CHANGED = CATEGORY_POWER + 3;
    int NOTIFY_DATA_CONNECTION_FAILED = CATEGORY_NETWORK + 4;
    int NOTIFY_DATA_CONNECTION_STATE_CHANGED = CATEGORY_NETWORK + 5;
    int NOTIFY_NETWORK_TYPE_CHANGED = CATEGORY_NETWORK + 6;
    int NOTIFY_SERVICE_STATE_CHANGED = CATEGORY_NETWORK + 7;
    int NOTIFY_WIFI_STATE_CHANGED = CATEGORY_WIFI + 8;
    int NOTIFY_WIFI_CONNECTION_STATE_CHANGED = CATEGORY_WIFI + 9;
    int NOTIFY_CONFIGURATION_CHANGED = CATEGORY_CONFIG + 10;
    int NOTIFY_EVENT = CATEGORY_EVENT + 11;
    int NOTIFY_ISIM_EVENT = CATEGORY_ISIM_EVENT + 12;
    int NOTIFY_USIM_EVENT = CATEGORY_USIM_EVENT + 13;
    int NOTIFY_DATA_CONNECTION_IPCAN_CHANGED = CATEGORY_NETWORK + 14;
    int NOTIFY_VOICE_NETWORK_TYPE_CHANGED = CATEGORY_NETWORK + 15;
    int NOTIFY_RADIO_EVENT = CATEGORY_RADIO + 16;

    /**
     * Methods for the system access (Native to Java)
     */

    /**
     * Common information
     */
    int GET_UUID = CATEGORY_BASE + 1;
    int GET_BATTERY_LEVEL = CATEGORY_BASE + 2;

    /**
     * Device & UICC-related information
     */
    int GET_DEVICE_ID = CATEGORY_BASE + 101;
    int GET_DEVICE_NAME = CATEGORY_BASE + 102;
    int GET_DEVICE_SOFTWARE_VERSION = CATEGORY_BASE + 103;
    int GET_EXTERNAL_STORAGE_PATH = CATEGORY_BASE + 104;
    int GET_PHONE_NUMBER = CATEGORY_BASE + 105;
    int GET_SUBSCRIBER_ID = CATEGORY_BASE + 106;
    int GET_SIM_MCC = CATEGORY_BASE + 107;
    int GET_SIM_MNC = CATEGORY_BASE + 108;
    int GET_SIM_COUNTRY_ISO = CATEGORY_BASE + 109;
    int GET_NETWORK_COUNTRY_ISO = CATEGORY_BASE + 110;

    // For UICC (ISIM)
    int GET_ISIM_STATE = CATEGORY_BASE + 151;
    int GET_ISIM_RECORD = CATEGORY_BASE + 152;
    int REQUEST_ISIM_AUTH = CATEGORY_BASE + 153;
    // For UICC (USIM)
    int REQUEST_USIM_AUTH = CATEGORY_BASE + 154;

    /**
     * Call-related information
     */
    int GET_CS_CALL_STATE = CATEGORY_BASE + 201;
    int IS_EMERGENCY_NUMBER = CATEGORY_BASE + 202;
    int GET_TTY_MODE = CATEGORY_BASE + 203;
    int GET_RTT_MODE = CATEGORY_BASE + 204;
    int GET_CS_CALL_STATE_IN_OTHER_SLOT = CATEGORY_BASE + 205;
    int IS_CROSS_SIM_REDIALING_AVAILABLE = CATEGORY_BASE + 206;

    /**
     * Network-related information (mobile)
     */
    int GET_NETWORK_TYPE = CATEGORY_BASE + 401;
    int GET_ROAMING_STATE = CATEGORY_BASE + 402;
    int GET_SERVICE_STATE = CATEGORY_BASE + 403;
    int GET_ACCESS_NETWORK_INFO = CATEGORY_BASE + 404;
    int REQUEST_NETWORK = CATEGORY_BASE + 405;
    int RELEASE_NETWORK = CATEGORY_BASE + 406;
    int GET_APN_NAME = CATEGORY_BASE + 407;
    int GET_DATA_CONNECTION_STATE = CATEGORY_BASE + 408;
    int GET_LOCAL_ADDRESS = CATEGORY_BASE + 409;
    int GET_PCSCF_ADDRESSES = CATEGORY_BASE + 410;
    int GET_IPCAN_CATEGORY = CATEGORY_BASE + 411;
    int IS_IMS_VOICE_CALL_SUPPORTED = CATEGORY_BASE + 412;
    int GET_IFACE_NAME = CATEGORY_BASE + 413;
    int GET_LAST_ACCESS_NETWORK_INFO = CATEGORY_BASE + 414;
    int GET_IFACE_ID = CATEGORY_BASE + 415;
    int GET_HOST_BY_NAME = CATEGORY_BASE + 416;
    int IS_IMS_EMERGENCY_CALL_SUPPORTED = CATEGORY_BASE + 417;
    int GET_VOICE_NETWORK_TYPE = CATEGORY_BASE + 418;
    int IS_EMERGENCY_ONLY = CATEGORY_BASE + 419;
    int IS_MOBILE_DATA_ENABLED = CATEGORY_BASE + 420;
    int GET_MOCN_PLMN_INFO = CATEGORY_BASE + 421;
    int GET_VOICE_SERVICE_STATE = CATEGORY_BASE + 422;
    int GET_VOICE_ROAMING_TYPE = CATEGORY_BASE + 423;
    int GET_DATA_ROAMING_TYPE = CATEGORY_BASE + 424;
    int GET_MTU = CATEGORY_BASE + 425;
    int IS_EMERGENCY_ATTACH_SUPPORTED = CATEGORY_BASE + 426;
    int BIND_SOCKET = CATEGORY_BASE + 427;
    int IS_IPV6_PREFERRED = CATEGORY_BASE + 428;

    /**
     * WiFi-related information
     */
    int GET_WIFI_BSS_ID = CATEGORY_BASE + 501;
    int GET_WIFI_CONNECTION_STATE = CATEGORY_BASE + 502;
    int GET_WIFI_STATE = CATEGORY_BASE + 503;
    int GET_WIFI_SSID = CATEGORY_BASE + 504;

    /**
     * Timer APIs
     */
    int SET_TIMER = CATEGORY_BASE + 601;
    int KILL_TIMER = CATEGORY_BASE + 602;

    /**
     * Configuration-related information
     */
    int GET_PREFERENCE = CATEGORY_BASE + 701;
    int SET_PREFERENCE = CATEGORY_BASE + 702;
    int GET_PRIVATE_PROPERTY = CATEGORY_BASE + 704;
    int SET_PRIVATE_PROPERTY = CATEGORY_BASE + 705;
    int GET_CARRIER_CONFIG = CATEGORY_BASE + 706;

    /**
     * Event control (from native to java)
     */
    int SEND_EVENT = CATEGORY_BASE + 801;
    int SET_EVENT = CATEGORY_BASE + 802;
    int RESET_EVENT = CATEGORY_BASE + 803;

    /**
     * WFC information
     */
    int IS_WFC_ENABLED = CATEGORY_BASE + 1101;
    int GET_WFC_PREFERENCES = CATEGORY_BASE + 1102;
    int IS_WFC_PROVISIONED = CATEGORY_BASE + 1103;
    int GET_WFC_ADDRESS_ID = CATEGORY_BASE + 1104;

    /**
     * Location information
     */
    int START_LISTENING_FOR_LOCATION = CATEGORY_BASE + 1201;
    int STOP_LISTENING_FOR_LOCATION = CATEGORY_BASE + 1202;
    int GET_LAST_KNOWN_LOCATION = CATEGORY_BASE + 1203;
    int START_INSTANT_LOCATION_UPDATE = CATEGORY_BASE + 1204;

    /**
     * Ims radio interface
     */
    int START_IMS_TRAFFIC = CATEGORY_BASE + 1301;
    int STOP_IMS_TRAFFIC = CATEGORY_BASE + 1302;
    int TRIGGER_EPS_FALLBACK = CATEGORY_BASE + 1303;
    int SET_TRAFFIC_PRIORITY = CATEGORY_BASE + 1304;

    ////
    // IpSec
    ////
    int ADD_IPSEC_SA_PARAMETER = CATEGORY_BASE + 1501;
    int REMOVE_IPSEC_SA_PARAMETER = CATEGORY_BASE + 1502;
    int APPLY_IPSEC_SA = CATEGORY_BASE + 1503;
    int REMOVE_IPSEC_SA = CATEGORY_BASE + 1504;
}
