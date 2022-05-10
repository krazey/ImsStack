package com.android.imsstack.util;

import android.content.ContentResolver;
import android.content.Context;
import android.database.ContentObserver;
import android.net.Uri;
import android.provider.Settings;
import android.telecom.TelecomManager;
import android.telephony.CarrierConfigManager;
import android.telephony.SubscriptionManager;

import com.android.ims.ImsConfig;

//import com.android.imsstack.external.constants.SettingsConstants;

/**
 * This class provides the APIs to access Settings.
 */
public final class SettingsUtils {
    public static class SettingsConstants {
        public static class System {
            public static final String VOWIFI_MDN = "vowifi_mdn";
            public static final String ALLOW_VOLTE_PROVISIONING = "allow_volte_provisioning";
            public static final String RTT_OPTION = "rtt_option";
            public static final String RTT_OPERATION_MODE = "rtt_operation_mode";
        }

        public static class Secure {
            public static final String DATA_LTE_ROAMING = "data_lte_roaming";
            public static final String DATA_NETWORK_VIDEO_CALLING_STATUS_ROAMING =
                    "data_network_video_calling_status_roaming";
            public static final String DATA_NETWORK_ENHANCED_4G_LTE_MODE =
                    "data_network_enhanced_4g_lte_mode";
            public static final String DATA_NETWORK_VIDEO_CALLING_STATUS =
                    "data_network_video_calling_status";
        }

        public static final class Global {
            public static final String ROAMING_HDVOICE_ENABLED =
                    "roaming_hdvoice_enabled";
            public static final String WFC_IMS_MODE_SIM2 =
                    "wfc_ims_mode_sim2";
            public static final String WFC_IMS_ENABLED_SIM2 =
                    "wfc_ims_enabled_sim2";
            public static final String WFC_IMS_ROAMING_ENABLED_SIM2 =
                    "wfc_ims_roaming_enabled_sim2";
        }
    }

    public static final int VALUE_NOT_INITIALIZED = -1;
    public static final int VALUE_ON = 1;
    public static final int VALUE_OFF = 0;

    public static final String DATA_ROAMING_ENHANCED_4G_LTE_MODE
            = "data_roaming_enhanced_4g_lte_mode";
    public static final String DATA_NETWORK_ENHANCED_4G_LTE_MODE2
            = "data_network_enhanced_4g_lte_mode2";
    public static final String DATA_NETWORK_VIDEO_CALLING_STATUS2
            = "data_network_video_calling_status2";

    public static class WfcMode {
        public static final int NONE = (-1);
        public static final int WIFI_ONLY
                = ImsConfig.WfcModeFeatureValueConstants.WIFI_ONLY;
        public static final int CELLULAR_PREFERRED
                = ImsConfig.WfcModeFeatureValueConstants.CELLULAR_PREFERRED;
        public static final int WIFI_PREFERRED
                = ImsConfig.WfcModeFeatureValueConstants.WIFI_PREFERRED;
        public static final int IMS_PREFERRED
                = 3; // ImsConfig.WfcModeFeatureValueConstants.IMS_PREFERRED;
    }

    private static final boolean IS_DUAL_WFC_SCENARIO_SUPPORTED
            = isDualWFCScenarioSupported();

    public static int getBootCount(ContentResolver cr) {
        return Settings.Global.getInt(cr, Settings.Global.BOOT_COUNT, -1);
    }

    public static int getLocationMode(ContentResolver cr) {
        return Settings.Secure.getInt(cr, Settings.Secure.LOCATION_MODE,
                Settings.Secure.LOCATION_MODE_OFF);
    }

    public static int getTtyMode(ContentResolver cr) {
        return Settings.Secure.getInt(
                cr, Settings.Secure.PREFERRED_TTY_MODE, TelecomManager.TTY_MODE_OFF);
    }

    public static String getVoWIFIMDN(ContentResolver cr) {
        return Settings.System.getString(cr, SettingsConstants.System.VOWIFI_MDN);
    }

    public static boolean isAirplaneModeOn(ContentResolver cr) {
        return Settings.Global.getInt(cr, Settings.Global.AIRPLANE_MODE_ON, 0) == 1;
    }

    /** IMS specific API */
    public static boolean isDataLteRoaming(ContentResolver cr) {
        return Settings.Secure.getInt(cr, SettingsConstants.Secure.DATA_LTE_ROAMING, 1) == 1;
    }

    public static boolean isDataNetworkVideoCallingStatusRoaming(ContentResolver cr) {
        return Settings.Secure.getInt(cr,
                SettingsConstants.Secure.DATA_NETWORK_VIDEO_CALLING_STATUS_ROAMING, 0) == 1;
    }

    public static boolean isDataRoamingEnabled(ContentResolver cr) {
        return Settings.Global.getInt(cr, Settings.Global.DATA_ROAMING, 0) == 1;
    }

    /** IMS specific API */
    public static boolean isDataRoamingEnhanced4GLteMode(ContentResolver cr) {
        return Settings.Secure.getInt(cr, DATA_ROAMING_ENHANCED_4G_LTE_MODE, 0) == 1;
    }

    public static boolean isDeviceProvisioned(ContentResolver cr) {
        return Settings.Global.getInt(cr, Settings.Global.DEVICE_PROVISIONED, 0) == 1;
    }

    public static boolean isRoamingHDVoiceEnabled(ContentResolver cr) {
        if (ImsProperties.TARGET_OPERATOR.equals("DCM")) {
            return Settings.Global.getInt(cr,
                    SettingsConstants.Global.ROAMING_HDVOICE_ENABLED, 1) == 1;
        } else {
            return Settings.Global.getInt(cr,
                    SettingsConstants.Global.ROAMING_HDVOICE_ENABLED, 0) == 1;
        }
    }

    public static boolean isUserSetupCompleted(ContentResolver cr) {
        return Settings.Secure.getInt(cr, Settings.Secure.USER_SETUP_COMPLETE, 0) == 1;
    }

    /** IMS specific API for Multi-IMS **/
    public static String getKeyForDataNetworkEnhanced4GLteMode(int slotId) {
        if (ImsConstants.USE_GOOGLE_NATIVE_APPS) {
            return SubscriptionManager.ENHANCED_4G_MODE_ENABLED;
        } else {
            if (MSimUtils.isMultiImsEnabled() && slotId == 1) {
                return DATA_NETWORK_ENHANCED_4G_LTE_MODE2;
            } else {
                return SettingsConstants.Secure.DATA_NETWORK_ENHANCED_4G_LTE_MODE;
            }
        }
    }

    public static String getKeyForVtImsEnabled(int slotId) {
        if (ImsConstants.USE_GOOGLE_NATIVE_APPS) {
            return SubscriptionManager.VT_IMS_ENABLED;
        } else {
            if (MSimUtils.isMultiImsEnabled() && slotId == 1) {
                return DATA_NETWORK_VIDEO_CALLING_STATUS2;
            } else {
                return SettingsConstants.Secure.DATA_NETWORK_VIDEO_CALLING_STATUS;
            }
        }
    }

    public static String getKeyForWFCImsMode(int slotId) {
        if (ImsConstants.USE_GOOGLE_NATIVE_APPS) {
            return SubscriptionManager.WFC_IMS_MODE;
        } else {
            if ((MSimUtils.isMultiImsEnabled() || IS_DUAL_WFC_SCENARIO_SUPPORTED) && slotId == 1) {
                return SettingsConstants.Global.WFC_IMS_MODE_SIM2;
            } else {
                return Settings.Global.WFC_IMS_MODE;
            }
        }
    }

    public static String getKeyForWFCImsEnabled(int slotId) {
        if (ImsConstants.USE_GOOGLE_NATIVE_APPS) {
            return SubscriptionManager.WFC_IMS_ENABLED;
        } else {
            if ((MSimUtils.isMultiImsEnabled() || IS_DUAL_WFC_SCENARIO_SUPPORTED) && slotId == 1) {
                return SettingsConstants.Global.WFC_IMS_ENABLED_SIM2;
            } else {
                return Settings.Global.WFC_IMS_ENABLED;
            }
        }
    }

    public static String getKeyForWFCImsRoamingEnabled(int slotId) {
        if (ImsConstants.USE_GOOGLE_NATIVE_APPS) {
            return SubscriptionManager.WFC_IMS_ROAMING_ENABLED;
        } else {
            if ((MSimUtils.isMultiImsEnabled() || IS_DUAL_WFC_SCENARIO_SUPPORTED) && slotId == 1) {
                return SettingsConstants.Global.WFC_IMS_ROAMING_ENABLED_SIM2;
            } else {
                return Settings.Global.WFC_IMS_ROAMING_ENABLED;
            }
        }
    }
    private static boolean isDualWFCScenarioSupported() {
        return  false;
    }

    // Type: int - 2=Wi-Fi preferred, 1=Cellular preferred, 0=Wi-Fi only
    public static int getWFCImsMode(Context c, int slotId) {
        if (c == null) {
            return getDefaultWfcImsMode(MSimUtils.getSubId(slotId));
        }

        if (ImsConstants.USE_GOOGLE_NATIVE_APPS) {
            return getWfcImsMode(c, slotId);
        } else {
            return Settings.Global.getInt(c.getContentResolver(),
                    getKeyForWFCImsMode(slotId), getDefaultWfcImsMode(slotId));
        }
    }

    public static int getWFCImsRoamingMode(Context c, int slotId) {
        if (c == null) {
            return getDefaultWfcImsRoamingMode(MSimUtils.getSubId(slotId));
        }

        if (ImsConstants.USE_GOOGLE_NATIVE_APPS) {
            return getWfcImsRoamingMode(c, slotId);
        } else {
            // Not implemented
            return 0;
        }
    }

    public static boolean isDataNetworkEnhanced4GLteMode(Context c, int slotId) {
        if (c == null) {
            return getDefaultEnhanced4GModeEnabled(MSimUtils.getSubId(slotId)) == VALUE_ON;
        }

        if (ImsConstants.USE_GOOGLE_NATIVE_APPS) {
            return isEnhanced4GModeEnabled(c, slotId);
        } else {
            return Settings.Secure.getInt(c.getContentResolver(),
                    getKeyForDataNetworkEnhanced4GLteMode(slotId),
                    getDefaultEnhanced4GModeEnabled(MSimUtils.getSubId(slotId))) == VALUE_ON;
        }
    }

    public static boolean isDataNetworkVideoCallingStatus(Context c, int slotId) {
        if (c == null) {
            return true;
        }

        if (ImsConstants.USE_GOOGLE_NATIVE_APPS) {
            return isVtImsEnabled(c, slotId);
        } else {
            // FIXME: it needs to be enhanced for multi-SIM device
            return Settings.Secure.getInt(c.getContentResolver(),
                    SettingsConstants.Secure.DATA_NETWORK_VIDEO_CALLING_STATUS,
                    0) == VALUE_ON;
        }
    }

    public static boolean isWFCImsEnabled(Context c, int slotId) {
        if (c == null) {
            return getDefaultWfcImsEnabled(MSimUtils.getSubId(slotId)) == VALUE_ON;
        }

        if (ImsConstants.USE_GOOGLE_NATIVE_APPS) {
            return isWfcImsEnabled(c, slotId);
        } else {
            return Settings.Global.getInt(c.getContentResolver(),
                    getKeyForWFCImsEnabled(slotId),
                    getDefaultWfcImsEnabled(slotId)) == VALUE_ON;
        }
    }

    public static boolean isWFCImsRoamingEnabled(Context c, int slotId) {
        if (c == null) {
            return getDefaultWfcImsRoamingEnabled(MSimUtils.getSubId(slotId)) == VALUE_ON;
        }

        if (ImsConstants.USE_GOOGLE_NATIVE_APPS) {
            return isWfcImsRoamingEnabled(c, slotId);
        } else {
            return Settings.Global.getInt(c.getContentResolver(),
                    getKeyForWFCImsRoamingEnabled(slotId),
                    getDefaultWfcImsRoamingEnabled(slotId)) == VALUE_ON;
        }
    }

    public static boolean isMobileDataEnabled(ContentResolver cr) {
        return Settings.Global.getInt(cr, Settings.Global.MOBILE_DATA, 0) == 1;
    }

    public static boolean putLocationMode(ContentResolver cr, int mode) {
        return Settings.Secure.putInt(cr, Settings.Secure.LOCATION_MODE, mode);
    }

    public static int getDefaultEnhanced4GModeEnabled(int subId) {
        return CarrierConfigUtils.getBoolean(
                CarrierConfigManager.KEY_ENHANCED_4G_LTE_ON_BY_DEFAULT_BOOL,
                subId) ? VALUE_ON : VALUE_OFF;
    }

    public static int getDefaultWfcImsEnabled(int subId) {
        return CarrierConfigUtils.getBoolean(
                CarrierConfigManager.KEY_CARRIER_DEFAULT_WFC_IMS_ENABLED_BOOL,
                subId) ? VALUE_ON : VALUE_OFF;
    }

    public static int getDefaultWfcImsMode(int subId) {
        return CarrierConfigUtils.getInt(
                CarrierConfigManager.KEY_CARRIER_DEFAULT_WFC_IMS_MODE_INT, subId);
    }

    public static int getDefaultWfcImsRoamingEnabled(int subId) {
        return CarrierConfigUtils.getBoolean(
                CarrierConfigManager.KEY_CARRIER_DEFAULT_WFC_IMS_ROAMING_ENABLED_BOOL,
                subId) ? VALUE_ON : VALUE_OFF;
    }

    public static int getDefaultWfcImsRoamingMode(int subId) {
        return CarrierConfigUtils.getInt(
                CarrierConfigManager.KEY_CARRIER_DEFAULT_WFC_IMS_ROAMING_MODE_INT, subId);
    }

    public static boolean isEnhanced4GLteModeEditable(int subId) {
        return CarrierConfigUtils.getBoolean(
                CarrierConfigManager.KEY_EDITABLE_ENHANCED_4G_LTE_BOOL, subId);
    }

    public static boolean isWfcModeEditable(int subId) {
        return CarrierConfigUtils.getBoolean(
                CarrierConfigManager.KEY_EDITABLE_WFC_MODE_BOOL, subId);
    }

    /** IMS specific API */
    public static void putAllowVoLteProvisioning(ContentResolver cr, int value) {
        Settings.System.putInt(cr, SettingsConstants.System.ALLOW_VOLTE_PROVISIONING, value);
    }

    public static int getAllowVoLteProvisioning(ContentResolver cr) {
        return Settings.System.getInt(cr, SettingsConstants.System.ALLOW_VOLTE_PROVISIONING, -1);
    }

    public static int getRttOption(ContentResolver cr) {
        return Settings.System.getInt(cr, SettingsConstants.System.RTT_OPTION, -1);
    }

    public static int getRttOperationMode(ContentResolver cr) {
        return Settings.System.getInt(cr, SettingsConstants.System.RTT_OPERATION_MODE, -1);
    }

    /** New APIs for P-OS */
    public static boolean isEnhanced4GModeEnabled(Context c, int slotId) {
        int subId = MSimUtils.getSubId(slotId);
        int setting = SubscriptionManager.getIntegerSubscriptionProperty(subId,
                SubscriptionManager.ENHANCED_4G_MODE_ENABLED, VALUE_NOT_INITIALIZED, c);

        if (!isEnhanced4GLteModeEditable(subId)
                || (setting == VALUE_NOT_INITIALIZED)) {
            return (getDefaultEnhanced4GModeEnabled(subId) == VALUE_ON);
        }

        return (setting == VALUE_ON);
    }

    public static boolean isVtImsEnabled(Context c, int slotId) {
        int subId = MSimUtils.getSubId(slotId);
        int setting = SubscriptionManager.getIntegerSubscriptionProperty(subId,
                SubscriptionManager.VT_IMS_ENABLED, VALUE_NOT_INITIALIZED, c);

        return (setting == VALUE_NOT_INITIALIZED) || (setting == VALUE_ON);
    }

    public static boolean isWfcImsEnabled(Context c, int slotId) {
        int subId = MSimUtils.getSubId(slotId);
        int setting = SubscriptionManager.getIntegerSubscriptionProperty(subId,
                SubscriptionManager.WFC_IMS_ENABLED, VALUE_NOT_INITIALIZED, c);

        if (setting == VALUE_NOT_INITIALIZED) {
            return (getDefaultWfcImsEnabled(subId) == VALUE_ON);
        }

        return (setting == VALUE_ON);
    }

    public static boolean isWfcImsRoamingEnabled(Context c, int slotId) {
        int subId = MSimUtils.getSubId(slotId);
        int setting = SubscriptionManager.getIntegerSubscriptionProperty(subId,
                SubscriptionManager.WFC_IMS_ROAMING_ENABLED, VALUE_NOT_INITIALIZED, c);

        if (setting == VALUE_NOT_INITIALIZED) {
            return (getDefaultWfcImsRoamingEnabled(subId) == VALUE_ON);
        }

        return (setting == VALUE_ON);
    }

    public static int getWfcImsMode(Context c, int slotId) {
        int subId = MSimUtils.getSubId(slotId);
        int setting = SubscriptionManager.getIntegerSubscriptionProperty(subId,
                SubscriptionManager.WFC_IMS_MODE, VALUE_NOT_INITIALIZED, c);

        if (!isWfcModeEditable(subId)
                || (setting == VALUE_NOT_INITIALIZED)) {
            return getDefaultWfcImsMode(subId);
        }

        return setting;
    }

    public static int getWfcImsRoamingMode(Context c, int slotId) {
        int subId = MSimUtils.getSubId(slotId);
        int setting = SubscriptionManager.getIntegerSubscriptionProperty(subId,
                SubscriptionManager.WFC_IMS_ROAMING_MODE, VALUE_NOT_INITIALIZED, c);

        if (setting == VALUE_NOT_INITIALIZED) {
            return getDefaultWfcImsRoamingMode(subId);
        }

        return setting;
    }

    /** ContentObserver for Settings provider */
    public static void registerObserverForGlobal(ContentResolver cr,
            String key, ContentObserver observer) {
        if (cr == null) {
            return;
        }

        cr.registerContentObserver(Settings.Global.getUriFor(key), true, observer);
    }

    public static void registerObserverForGlobalAsUser(ContentResolver cr,
            String key, ContentObserver observer, int userId) {
        if (cr == null) {
            return;
        }

        cr.registerContentObserver(Settings.Global.getUriFor(key), true, observer, userId);
    }

    public static void registerObserverForSecure(ContentResolver cr,
            String key, ContentObserver observer) {
        if (cr == null) {
            return;
        }

        cr.registerContentObserver(Settings.Secure.getUriFor(key), true, observer);
    }

    public static void registerObserverForSecureAsUser(ContentResolver cr,
            String key, ContentObserver observer, int userId) {
        if (cr == null) {
            return;
        }

        cr.registerContentObserver(Settings.Secure.getUriFor(key), true, observer, userId);
    }

    public static void registerObserverForSystem(ContentResolver cr,
            String key, ContentObserver observer) {
        if (cr == null) {
            return;
        }

        cr.registerContentObserver(Settings.System.getUriFor(key), true, observer);
    }

    public static void registerObserverForSystemAsUser(ContentResolver cr,
            String key, ContentObserver observer, int userId) {
        if (cr == null) {
            return;
        }

        cr.registerContentObserver(Settings.System.getUriFor(key), true, observer, userId);
    }

    public static void unregisterObserver(ContentResolver cr, ContentObserver observer) {
        if (cr == null || observer == null) {
            return;
        }

        cr.unregisterContentObserver(observer);
    }

    /** New APIs for P-OS */
    public static void registerObserverForCallSettings(Context c,
            String key, ContentObserver observer, int slotId) {
        ContentResolver cr = c.getContentResolver();

        if (ImsConstants.USE_GOOGLE_NATIVE_APPS) {
            cr.registerContentObserver(
                    SubscriptionManager.CONTENT_URI, true, observer);
        } else {
            if (key.equals(getKeyForDataNetworkEnhanced4GLteMode(slotId))
                    || key.equals(getKeyForVtImsEnabled(slotId))) {
                registerObserverForSecure(cr, key, observer);
            } else {
                registerObserverForGlobal(cr, key, observer);
            }
        }
    }

    // Call settings {
    public static class CallSettings {
        public final static Uri CONTENT_URI = Uri.parse(
                "content://com.android.phone.CallSettingsProvider/callsettings");
        public final static String KEY_KT_HD_VOICE_SETTING = "KT_hd_voice_setting";
        public final static String KEY_SHOW_INDICATOR_VOLTE_ICON = "show_indicator_volte_icon";
        public final static String KEY_CALL_ORDER_PRIORITY = "call_order_priority";
        public final static String KEY_IMS_ADMIN_TESTMODE = "ims_admin_testmode";

        public static final String KEY_NAME = "name";
        public static final String KEY_VALUE_INT = "value_int";
        public static final String KEY_VALUE_STR = "value_str";

        public static int getInt(Context c, String key, int def) {
            String selection = KEY_NAME + "=" + "'" + key + "'";
            return DBUtils.CP.getInt(c.getContentResolver(),
                    CONTENT_URI, selection, KEY_VALUE_INT, def);
        }

        public static String getString(Context c, String key, String def) {
            String selection = KEY_NAME + "=" + "'" + key + "'";
            return DBUtils.CP.getString(c.getContentResolver(),
                    CONTENT_URI, selection, KEY_VALUE_STR, def);
        }

        public static void registerObserver(Context c, ContentObserver observer) {
            if (ImsConstants.USE_GOOGLE_NATIVE_APPS) {
                // Call settings is not supported.
                return;
            }

            if (c == null || observer == null) {
                return;
            }

            c.getContentResolver().registerContentObserver(CONTENT_URI, false, observer);
        }

        public static void unregisterObserver(Context c, ContentObserver observer) {
            if (ImsConstants.USE_GOOGLE_NATIVE_APPS) {
                // Call settings is not supported.
                return;
            }

            if (c == null || observer == null) {
                return;
            }

            c.getContentResolver().unregisterContentObserver(observer);
        }
    }
    // Call settings }
}
