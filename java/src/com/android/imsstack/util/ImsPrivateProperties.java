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
package com.android.imsstack.util;

import android.content.Context;
import android.content.SharedPreferences;

import java.util.Locale;

/**
 * This class provides the wrapper APIs for IMS internal properties.
 * It's generally used by hidden menu.
 */
public final class ImsPrivateProperties {
    /** Returns the SIM country string. */
    public static String getSimCountry(int slotId) {
        try {
            return Persistent.get(AppContext.getInstance(),
                    Persistent.KEY_SIM_COUNTRY, "", slotId);
        } catch (Throwable t) {
            t.printStackTrace();
            return "";
        }
    }

    /** Returns the SIM operator string. */
    public static String getSimOperator(int slotId) {
        try {
            return Persistent.get(AppContext.getInstance(),
                    Persistent.KEY_SIM_OPERATOR, "", slotId);
        } catch (Throwable t) {
            t.printStackTrace();
            return "";
        }
    }

    /** Returns the SIM operator-sub string. */
    public static String getSimOperatorSub(int slotId) {
        try {
            return Persistent.get(AppContext.getInstance(),
                    Persistent.KEY_SIM_OPERATOR_SUB, "", slotId);
        } catch (Throwable t) {
            return "";
        }
    }

    public static class Ephemeral {
        /**
         * Codec information during IMS call.
         * Value: string
         */
        public static final String KEY_CODEC_IN_USE = "codec_in_use";
        /**
         * TTA version for KR operators.
         * Value: string
         */
        public static final String KEY_KR_TTA_VERSION = "kr_tta_version";
        /**
         * Configuration information for registration recovery interval.
         *    Retry Base Time
         *    Retry Max Time
         *    Consecutive Failure
         *    Upperbound Waiting Time
         *    Actual Waiting Time
         * Value: int
         */
        public static final String KEY_REG_0_BT = "reg_0_bt";
        public static final String KEY_REG_1_MT = "reg_1_mt";
        public static final String KEY_REG_2_CF = "reg_2_cf";
        public static final String KEY_REG_3_UWT = "reg_3_uwt";
        public static final String KEY_REG_4_AWT = "reg_4_awt";
        /**
         * sms network bind for KR operators
         * Value: boolean
         */
        public static final String KEY_SMS_NETWORK_REG_BIND = "sms_network_reg_bind";

        /**
         * Caches the default dialer information and it will be checked in active call.
         * Value: boolean
         */
        public static final String KEY_THIRD_PARTY_DIALER_FOR_VIDEO_CALL
                = "third_party_dialer_for_video_call";
        /**
         * Caches the video quality information for H.265 if operator provides this value.
         * It may be used for preview's video resolution for 3rd party dialer.
         * Value: String
         */
        public static final String KEY_H265_VIDEO_QUALITY = "h265_video_quality";

        private static final String NAME = "ephemeral_prop";

        public static String get(String key, int slotId) {
            return get(key, null, slotId);
        }

        public static boolean getBoolean(String key, int slotId) {
            return getBoolean(key, false, slotId);
        }

        public static int getInt(String key, int slotId) {
            return getInt(key, -1, slotId);
        }

        public static String get(String key, String defValue, int slotId) {
            SharedPreferences sp = getSharedPreferences(slotId);
            return sp.getString(key, defValue);
        }

        public static boolean getBoolean(String key, boolean defValue, int slotId) {
            return Boolean.valueOf(get(key, String.valueOf(defValue), slotId));
        }

        public static int getInt(String key, int defValue, int slotId) {
            return Integer.valueOf(get(key, String.valueOf(defValue), slotId));
        }

        public static void set(String key, String value, int slotId) {
            SharedPreferences.Editor editor = getSharedPreferences(slotId).edit();
            editor.putString(key, value);
            editor.commit();
        }

        public static void setBoolean(String key, boolean value, int slotId) {
            set(key, String.valueOf(value), slotId);
        }

        public static void setInt(String key, int value, int slotId) {
            set(key, String.valueOf(value), slotId);
        }

        public static void remove(String key, int slotId) {
            SharedPreferences.Editor editor = getSharedPreferences(slotId).edit();
            editor.remove(key);
            editor.commit();
        }

        public static void removeAll(int slotId) {
            SharedPreferences.Editor editor = getSharedPreferences(slotId).edit();
            editor.clear();
            editor.commit();
        }

        private static SharedPreferences getSharedPreferences(int slotId) {
            String name = String.format(Locale.US, "%s_%d", NAME, slotId);
            return AppContext.getInstance().getSharedPreferences(name, Context.MODE_PRIVATE);
        }

        private Ephemeral() {}
    }

    public static class Persistent {
        // Test properties
        /**
         * IMS preferred operator for test purpose.
         * Value: string
         */
        public static final String KEY_PREF_OPERATOR = "pref_operator";
        /**
         * IMS preferred country for test purpose.
         * Value: string
         */
        public static final String KEY_PREF_COUNTRY = "pref_country";
        /**
         * Enables / disables KR operator's USIM mobility enabler (KR enabler) for test purpose.
         * Value: boolean (true / false)
         */
        public static final String KEY_PREF_KR_ENABLER = "pref_kr_enabler";
        /**
         * Wi-Fi test configuration.
         * Value: int (1 / 0)
         */
        public static final String KEY_WIFI_TEST = "wifi_test";
        /**
         * IMS Traffic Hal test configuration.
         * Value: int (1 / 0)
         */
        public static final String KEY_IMS_HAL_TEST = "ims_hal_test";
        /**
         * Carrier signal PCO test configuration.
         * Value: int (1 / 0)
         */
        public static final String KEY_CARRIER_SIGNAL_PCO_TEST = "carrier_signal_pco_test";

        // Release properties
        /**
         * SIM operator
         * Value: string
         */
        public static final String KEY_SIM_OPERATOR = "sim_operator";
        /**
         * SIM operator-sub
         * Value: string
         */
        public static final String KEY_SIM_OPERATOR_SUB = "sim_operator_sub";
        /**
         * SIM country
         * Value: string
         */
        public static final String KEY_SIM_COUNTRY = "sim_country";

        /**
         * Tracks the last access network information.
         *    - TMUS
         * Value: string
         */
        public static final String KEY_LAST_ACCESS_NETWORK_INFO = "last_access_network_info";
        /**
         * Shows Wi-Fi calling information pop-up for one time.
         *    - ORG
         * Value: boolean (true / false)
         */
        public static final String KEY_SHOW_WFC_INFO = "show_wfc_info";
        /**
         * SIP device id (UUID-based) for multi-device requirement.
         * Value: string
         */
        public static final String KEY_SIP_DEVICE_ID = "sip_device_id";
        /**
         * Device's latest SW version.
         *    - TMUS
         * Value: string
         * SlotId SHOULD be a zero for this key because it's a device's information.
         */
        public static final String KEY_DEVICE_SW_VERSION = "device_sw_version";
        /**
         * A flag specifying whether IMS is disabled or not.
         * Value : boolean (true / false)
         */
        public static final String KEY_TEST_IMS_DISABLED = "test_ims_disabled";

        /**
         * A flag specifying whether a debug mode is enabled or not.
         * Value : boolean (true / false)
         */
        public static final String KEY_TEST_DEBUG_ENABLED = "test_debug_enabled";

        /**
         * A flag specifying whether test-mode is enabled or not.
         * Value : boolean (true / false)
         */
        public static final String KEY_TEST_TESTMODE_ENABLED = "test_testmode_enabled";

        /**
         * Keeps the flag to indicate whether the pre-defined User-Agent string is used.
         * Value : boolean (true / false)
         */
        public static final String KEY_USE_PREDEFINED_UA_STRING = "use_predefined_ua_string";

        /**
         * Keeps the pre-defined User-Agent string.
         * Value : string
         */
        public static final String KEY_CONFIG_UA_STRING = "config_ua_string";

        /**
         * Keeps the pre-defined NR duplex mode.
         * Value : string
         */
        public static final String KEY_CONFIG_NR_DUPLEX_MODE = "config_nr_duplex_mode";
        /**
         * Keeps the pre-defined ims deregister.
         * Value : string
         */
        public static final String KEY_TEST_IMS_DEREGISTER = "test_ims_deregister";
        /**
         * Keeps the log options for the ImsStack's logging.
         * Value : string (e.g. 0x0001000F)
         */
        public static final String KEY_TEST_LOG_OPTIONS = "test_log_options";

        /**
         * Stores VoWiFi's entitlement identifier(AT&T: E911 AID).
         * Value : string
         */
        public static final String KEY_VOWIFI_ENTITLEMENT_ID = "vowifi_entitlement_id";

        /**
         * Keeps the test carrier id.
         * Value : int
         */
        public static final String KEY_TEST_CARRIER_ID = "test_carrier_id";

        /**
         * Tracks the configuration items that need to be shared with the native layer.
         *  - ISIM/USIM enabled (that is currently used for IMS registration)
         *  - Primary public user identity (that will be used for IMS registration)
         */
        /**
         * Keeps the current ISIM status.
         * Value : boolean
         */
        public static final String KEY_ISIM_ENABLED = "isim_enabled";
        /**
         * Keeps the current USIM status.
         * Value : boolean
         */
        public static final String KEY_USIM_ENABLED = "usim_enabled";
        /**
         * Keeps primary public user identity that will be ued for IMS registration.
         * Value : string
         */
        public static final String KEY_PRIMARY_IMPU = "primary_impu";

        /**
         * Configuration items which can be provisioned for a test purpose.
         *  - List of P-CSCF address
         *  - IMPI (Private user identity)
         *  - List of IMPU (Public user identities)
         *  - Home domain name
         */
        public static final String KEY_CONFIG_PCSCF_ADDRESS_LIST = "config_pcscf_address_list";
        public static final String KEY_CONFIG_IMPI = "config_impi";
        public static final String KEY_CONFIG_IMPU_LIST = "config_impu_list";
        public static final String KEY_CONFIG_HOME_DOMAIN_NAME = "config_home_domain_name";

        public static final String[] CONFIG_PROPERTIES =
            {
                KEY_CONFIG_PCSCF_ADDRESS_LIST,
                KEY_CONFIG_IMPI,
                KEY_CONFIG_IMPU_LIST,
                KEY_CONFIG_HOME_DOMAIN_NAME
            };

        public static boolean isConfigProperty(String key) {
            for (String configKey : ImsPrivateProperties.Persistent.CONFIG_PROPERTIES) {
                if (configKey.equals(key)) {
                    return true;
                }
            }

            return false;
        }

        private static final String NAME = "persistent_prop";

        public static final String[] TEST_PROPERTIES =
            {
                KEY_PREF_OPERATOR,
                KEY_PREF_COUNTRY,
                KEY_PREF_KR_ENABLER,
                KEY_WIFI_TEST,
                KEY_IMS_HAL_TEST,
                KEY_CARRIER_SIGNAL_PCO_TEST,
                KEY_TEST_IMS_DISABLED,
                KEY_TEST_DEBUG_ENABLED,
                KEY_TEST_TESTMODE_ENABLED,
                KEY_USE_PREDEFINED_UA_STRING,
                KEY_CONFIG_NR_DUPLEX_MODE,
                KEY_TEST_CARRIER_ID,
                KEY_TEST_LOG_OPTIONS
            };

        public static String get(String key, int slotId) {
            return get(key, null, slotId);
        }

        public static boolean getBoolean(String key, int slotId) {
            return getBoolean(key, false, slotId);
        }

        public static int getInt(String key, int slotId) {
            return getInt(key, -1, slotId);
        }

        public static String get(String key, String defValue, int slotId) {
            return get(AppContext.getInstance(), key, defValue, slotId);
        }

        public static String get(Context c, String key, String defValue, int slotId) {
            SharedPreferences sp = getSharedPreferences(c, slotId);
            return (sp != null) ? sp.getString(key, defValue) : defValue;
        }

        public static boolean getBoolean(String key, boolean defValue, int slotId) {
            return Boolean.valueOf(get(key, String.valueOf(defValue), slotId));
        }

        public static int getInt(String key, int defValue, int slotId) {
            return Integer.valueOf(get(key, String.valueOf(defValue), slotId));
        }

        public static void set(String key, String value, int slotId) {
            set(AppContext.getInstance(), key, value, slotId);
        }

        public static void set(Context c, String key, String value, int slotId) {
            if (c == null) {
                return;
            }

            SharedPreferences sp = getSharedPreferences(c, slotId);

            if (sp != null) {
                SharedPreferences.Editor editor = sp.edit();
                editor.putString(key, value);
                editor.commit();
            }
        }

        public static void setBoolean(String key, boolean value, int slotId) {
            set(key, String.valueOf(value), slotId);
        }

        public static void setInt(String key, int value, int slotId) {
            set(key, String.valueOf(value), slotId);
        }

        public static void removeTestProperties(int slotId) {
            SharedPreferences.Editor editor = getSharedPreferences(
                    AppContext.getInstance(), slotId).edit();

            for (String prop : TEST_PROPERTIES) {
                editor.remove(prop);
            }

            editor.commit();
        }

        private static SharedPreferences getSharedPreferences(Context c, int slotId) {
            String name = String.format(Locale.US, "%s_%d", NAME, slotId);
            return c.getSharedPreferences(name, Context.MODE_PRIVATE);
        }

        private Persistent() {}
    }

    private ImsPrivateProperties() {}
}
