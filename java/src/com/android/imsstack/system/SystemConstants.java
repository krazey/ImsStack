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

public class SystemConstants {
    /**
     * Shared interface key value between native and java
     */
    public static final int SYSTEM_INTERFACE = 51;

    /**
     * Category for system interface.
     */
    public static final int CATEGORY_BASE = 0x00000000;
    public static final int CATEGORY_NETWORK = 0x00010000;
    public static final int CATEGORY_WIFI = 0x00020000;
    public static final int CATEGORY_CALL = 0x00040000;
    public static final int CATEGORY_POWER = 0x00080000;
    public static final int CATEGORY_TIMER = 0x00100000;
    public static final int CATEGORY_CONFIG = 0x00200000;
    public static final int CATEGORY_EVENT = 0x00400000;
    public static final int CATEGORY_ISIM_EVENT = 0x00800000;
    public static final int CATEGORY_USIM_EVENT = 0x01000000;
    public static final int CATEGORY_RADIO = 0x02000000;

    /**
     * Methods for the system information's notification (Java to Native)
     */
    public static final int NOTIFY_AIRPLANE_MODE_CHANGED = CATEGORY_NETWORK + 1;
    public static final int NOTIFY_TIMER_EXPIRED = CATEGORY_TIMER + 2;
    public static final int NOTIFY_BATTERY_LEVEL_CHANGED = CATEGORY_POWER + 3;
    public static final int NOTIFY_DATA_CONNECTION_FAILED = CATEGORY_NETWORK + 4;
    public static final int NOTIFY_DATA_CONNECTION_STATE_CHANGED = CATEGORY_NETWORK + 5;
    public static final int NOTIFY_NETWORK_TYPE_CHANGED = CATEGORY_NETWORK + 6;
    public static final int NOTIFY_SERVICE_STATE_CHANGED = CATEGORY_NETWORK + 7;
    public static final int NOTIFY_VOICE_CALL_STATE_CHANGED = CATEGORY_CALL + 8;
    public static final int NOTIFY_WIFI_STATE_CHANGED = CATEGORY_WIFI + 9;
    public static final int NOTIFY_WIFI_CONNECTION_STATE_CHANGED = CATEGORY_WIFI + 10;
    public static final int NOTIFY_CONFIGURATION_CHANGED = CATEGORY_CONFIG + 11;
    public static final int NOTIFY_EVENT = CATEGORY_EVENT + 12;
    public static final int NOTIFY_ISIM_EVENT = CATEGORY_ISIM_EVENT + 13;
    public static final int NOTIFY_USIM_EVENT = CATEGORY_USIM_EVENT + 14;
    public static final int NOTIFY_DATA_CONNECTION_IPCAN_CHANGED = CATEGORY_NETWORK + 15;
    public static final int NOTIFY_VOICE_NETWORK_TYPE_CHANGED = CATEGORY_NETWORK + 16;
    public static final int NOTIFY_RADIO_EVENT = CATEGORY_RADIO + 17;

    /**
     * Methods for the system access (Native to Java)
     */

    /**
     * Power-related information
     */
    public static final int GET_BATTERY_LEVEL = CATEGORY_BASE + 1;

    /**
     * Device & UICC-related information
     */
    public static final int GET_DEVICE_ID = CATEGORY_BASE + 103;
    public static final int GET_DEVICE_SOFTWARE_VERSION = CATEGORY_BASE + 104;
    public static final int GET_EXTERNAL_STORAGE_PATH = CATEGORY_BASE + 105;
    public static final int GET_PHONE_NUMBER = CATEGORY_BASE + 106;
    public static final int GET_SUBSCRIBER_ID = CATEGORY_BASE + 107;
    public static final int GET_MCC = CATEGORY_BASE + 108;
    public static final int GET_MNC = CATEGORY_BASE + 109;
    public static final int GET_OPERATOR = CATEGORY_BASE + 110;
    public static final int GET_COUNTRY = CATEGORY_BASE + 111;
    public static final int GET_DEVICE_NAME = CATEGORY_BASE + 112;
    public static final int GET_NETWORK_COUNTRY = CATEGORY_BASE + 113;
    public static final int GET_EMERGENCY_NUM_LIST_FROM_SIM = CATEGORY_BASE + 114;
    public static final int GET_EMERGENCY_PRIORITY_FROM_MODEM = CATEGORY_BASE + 115;
    public static final int GET_UICC_GBA_SUPPORT = CATEGORY_BASE + 116;

    // For UICC (ISIM)
    public static final int GET_ISIM_STATE = CATEGORY_BASE + 151;
    public static final int READ_ISIM_FILE_ATTR = CATEGORY_BASE + 152;
    public static final int READ_ISIM_RECORD = CATEGORY_BASE + 153;
    public static final int REQUEST_ISIM_AUTH = CATEGORY_BASE + 154;
    // For UICC (USIM)
    public static final int REQUEST_USIM_AUTH = CATEGORY_BASE + 155;

    /**
     * Call-related information
     */
    public static final int GET_CALL_STATE = CATEGORY_BASE + 201;
    public static final int IS_EMERGENCY_NUMBER = CATEGORY_BASE + 202;
    public static final int GET_TTY_MODE = CATEGORY_BASE + 203;
    public static final int GET_RTT_MODE = CATEGORY_BASE + 204;
    public static final int GET_CALL_STATE_IN_OTHER_SLOT = CATEGORY_BASE + 205;

    public static final int GET_DIGEST_SHA1 = CATEGORY_BASE + 303;

    /**
     * Network-related information (mobile)
     */
    public static final int GET_NETWORK_TYPE = CATEGORY_BASE + 401;
    public static final int GET_ROAMING_STATE = CATEGORY_BASE + 402;
    public static final int GET_SERVICE_STATE = CATEGORY_BASE + 403;
    public static final int GET_ACCESS_NETWORK_INFO = CATEGORY_BASE + 404;
    public static final int ACTIVATE_DATA_CONNECTION = CATEGORY_BASE + 405;
    public static final int DEACTIVATE_DATA_CONNECTION = CATEGORY_BASE + 406;
    public static final int GET_APN_NAME = CATEGORY_BASE + 407;
    public static final int GET_DATA_CONNECTION_STATE = CATEGORY_BASE + 408;
    public static final int GET_LOCAL_ADDRESS = CATEGORY_BASE + 409;
    public static final int GET_PCSCF_ADDRESSES = CATEGORY_BASE + 410;
    public static final int GET_IPCAN_CATEGORY = CATEGORY_BASE + 411;
    public static final int IS_IMS_VOICE_CALL_SUPPORTED = CATEGORY_BASE + 412;
    public static final int GET_IFACE_NAME = CATEGORY_BASE + 413;
    public static final int GET_LAST_ACCESS_NETWORK_INFO = CATEGORY_BASE + 414;
    public static final int GET_IFACE_ID = CATEGORY_BASE + 415;
    public static final int GET_HOST_BY_NAME = CATEGORY_BASE + 416;
    public static final int IS_IMS_EMERGENCY_CALL_SUPPORTED = CATEGORY_BASE + 417;
    public static final int GET_VOICE_NETWORK_TYPE = CATEGORY_BASE + 418;
    public static final int IS_LTE_EMERGENCY_ONLY = CATEGORY_BASE + 419;
    public static final int IS_MOBILE_DATA_ENABLED = CATEGORY_BASE + 420;
    public static final int GET_MOCN_PLMN_INFO = CATEGORY_BASE + 421;
    public static final int GET_VOICE_SERVICE_STATE = CATEGORY_BASE + 422;
    public static final int GET_VOICE_ROAMING_TYPE = CATEGORY_BASE + 423;
    public static final int GET_DATA_ROAMING_TYPE = CATEGORY_BASE + 424;
    public static final int GET_MTU = CATEGORY_BASE + 425;
    public static final int IS_EMERGENCY_ATTACH_SUPPORTED = CATEGORY_BASE + 426;
    public static final int BIND_SOCKET = CATEGORY_BASE + 427;

    /**
     * WiFi-related information
     */
    public static final int GET_WIFI_BSS_ID = CATEGORY_BASE + 501;
    public static final int GET_WIFI_CONNECTION_STATE = CATEGORY_BASE + 502;
    public static final int GET_WIFI_STATE = CATEGORY_BASE + 503;
    public static final int GET_WIFI_SSID = CATEGORY_BASE + 504;

    /**
     * Timer APIs
     */
    public static final int SET_TIMER = CATEGORY_BASE + 601;
    public static final int KILL_TIMER = CATEGORY_BASE + 602;

    /**
     * Configuration-related information
     */
    public static final int GET_PREFERENCE = CATEGORY_BASE + 701;
    public static final int SET_PREFERENCE = CATEGORY_BASE + 702;
    public static final int GET_PRIVATE_PROPERTY = CATEGORY_BASE + 704;
    public static final int SET_PRIVATE_PROPERTY = CATEGORY_BASE + 705;
    public static final int GET_CARRIER_CONFIG = CATEGORY_BASE + 706;

    /**
     * Event control (from native to java)
     */
    public static final int SEND_EVENT = CATEGORY_BASE + 801;
    public static final int SET_EVENT = CATEGORY_BASE + 802;
    public static final int RESET_EVENT = CATEGORY_BASE + 803;

    /**
     * WFC information
     */
    public static final int IS_WFC_ENABLED = CATEGORY_BASE + 1101;
    public static final int GET_WFC_PREFERENCES = CATEGORY_BASE + 1102;
    public static final int IS_WFC_PROVISIONED = CATEGORY_BASE + 1103;
    public static final int GET_WFC_ADDRESS_ID = CATEGORY_BASE + 1104;

    /**
     * Location information
     */
    public static final int START_LISTENING_FOR_LOCATION = CATEGORY_BASE + 1201;
    public static final int STOP_LISTENING_FOR_LOCATION = CATEGORY_BASE + 1202;
    public static final int GET_LAST_KNOWN_LOCATION = CATEGORY_BASE + 1203;
    public static final int START_INSTANT_LOCATION_UPDATE = CATEGORY_BASE + 1204;

    /**
     * Ims radio interface
     */
    public static final int START_IMS_TRAFFIC = CATEGORY_BASE + 1301;
    public static final int STOP_IMS_TRAFFIC = CATEGORY_BASE + 1302;
    public static final int TRIGGER_EPS_FALLBACK = CATEGORY_BASE + 1303;

    ////
    // IpSec
    ////
    public static final int ADD_IPSEC_SA_PARAMETER = CATEGORY_BASE + 1501;
    public static final int REMOVE_IPSEC_SA_PARAMETER = CATEGORY_BASE + 1502;
    public static final int APPLY_IPSEC_SA = CATEGORY_BASE + 1503;
    public static final int REMOVE_IPSEC_SA = CATEGORY_BASE + 1504;
}
