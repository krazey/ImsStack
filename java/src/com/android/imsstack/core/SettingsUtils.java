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
package com.android.imsstack.core;

import android.content.ContentResolver;
import android.content.Context;
import android.database.ContentObserver;
import android.provider.Settings;
import android.telephony.CarrierConfigManager;
import android.telephony.SubscriptionManager;
import android.telephony.ims.ImsMmTelManager;

import com.android.imsstack.core.agents.AgentFactory;
import com.android.imsstack.core.agents.ConfigInterface;
import com.android.imsstack.core.config.CarrierConfig;
import com.android.imsstack.util.MSimUtils;

/**
 * This class provides the APIs to access Settings.
 */
public final class SettingsUtils {
    public static final int VALUE_NOT_INITIALIZED = -1;
    public static final int VALUE_ON = 1;
    public static final int VALUE_OFF = 0;

    /** Redefines the Wi-Fi calling mode. */
    public static class WfcMode {
        public static final int NONE = (-1);
        public static final int WIFI_ONLY = ImsMmTelManager.WIFI_MODE_WIFI_ONLY;
        public static final int CELLULAR_PREFERRED = ImsMmTelManager.WIFI_MODE_CELLULAR_PREFERRED;
        public static final int WIFI_PREFERRED = ImsMmTelManager.WIFI_MODE_WIFI_PREFERRED;
        public static final int IMS_PREFERRED = 3;
    }

    /** Returns the Wi-Fi calling mode. */
    // Type: int - 2=Wi-Fi preferred, 1=Cellular preferred, 0=Wi-Fi only
    public static int getWFCImsMode(Context c, int slotId) {
        if (c == null) {
            return getDefaultWfcImsMode(slotId);
        }

        return getWfcImsMode(c, slotId);
    }

    /** Returns the Wi-Fi calling mode in the roaming area. */
    public static int getWFCImsRoamingMode(Context c, int slotId) {
        if (c == null) {
            return getDefaultWfcImsRoamingMode(slotId);
        }

        return getWfcImsRoamingMode(c, slotId);
    }

    /** Returns the Wi-Fi calling enabled. */
    public static boolean isWFCImsEnabled(Context c, int slotId) {
        if (c == null) {
            return getDefaultWfcImsEnabled(slotId) == VALUE_ON;
        }

        return isWfcImsEnabled(c, slotId);
    }

    /** Returns the Wi-Fi calling enabled in the roaming area. */
    public static boolean isWFCImsRoamingEnabled(Context c, int slotId) {
        if (c == null) {
            return getDefaultWfcImsRoamingEnabled(slotId) == VALUE_ON;
        }

        return isWfcImsRoamingEnabled(c, slotId);
    }

    /** Returns a default Wi-Fi calling enabled. */
    public static int getDefaultWfcImsEnabled(int slotId) {
        ConfigInterface config = AgentFactory.getInstance().getAgent(ConfigInterface.class, slotId);
        CarrierConfig cc = (config != null) ? config.getCarrierConfig() : null;
        return (cc != null && cc.getBoolean(
                CarrierConfigManager.KEY_CARRIER_DEFAULT_WFC_IMS_ENABLED_BOOL))
                ? VALUE_ON : VALUE_OFF;
    }

    /** Returns a default Wi-Fi calling mode. */
    public static int getDefaultWfcImsMode(int slotId) {
        ConfigInterface config = AgentFactory.getInstance().getAgent(ConfigInterface.class, slotId);
        CarrierConfig cc = (config != null) ? config.getCarrierConfig() : null;

        if (cc != null) {
            return cc.getInt(CarrierConfigManager.KEY_CARRIER_DEFAULT_WFC_IMS_MODE_INT,
                    WfcMode.WIFI_PREFERRED);
        }
        return WfcMode.WIFI_PREFERRED;
    }

    /** Returns a default Wi-Fi calling enabled in the roaming area. */
    public static int getDefaultWfcImsRoamingEnabled(int slotId) {
        ConfigInterface config = AgentFactory.getInstance().getAgent(ConfigInterface.class, slotId);
        CarrierConfig cc = (config != null) ? config.getCarrierConfig() : null;
        return (cc != null && cc.getBoolean(
                CarrierConfigManager.KEY_CARRIER_DEFAULT_WFC_IMS_ROAMING_ENABLED_BOOL))
                ? VALUE_ON : VALUE_OFF;
    }

    /** Returns a default Wi-Fi calling mode in the roaming area. */
    public static int getDefaultWfcImsRoamingMode(int slotId) {
        ConfigInterface config = AgentFactory.getInstance().getAgent(ConfigInterface.class, slotId);
        CarrierConfig cc = (config != null) ? config.getCarrierConfig() : null;

        if (cc != null) {
            return cc.getInt(CarrierConfigManager.KEY_CARRIER_DEFAULT_WFC_IMS_ROAMING_MODE_INT,
                    WfcMode.WIFI_PREFERRED);
        }
        return WfcMode.WIFI_PREFERRED;
    }

    /** Checks whether the Wi-Fi calling mode setting is editable or not. */
    public static boolean isWfcModeEditable(int slotId) {
        ConfigInterface config = AgentFactory.getInstance().getAgent(ConfigInterface.class, slotId);
        CarrierConfig cc = (config != null) ? config.getCarrierConfig() : null;
        return cc != null && cc.getBoolean(CarrierConfigManager.KEY_EDITABLE_WFC_MODE_BOOL);
    }

    /** Checks if VT is enabled. */
    public static boolean isVtImsEnabled(Context c, int slotId) {
        int subId = MSimUtils.getSubId(slotId);
        int setting = SubscriptionManager.getIntegerSubscriptionProperty(subId,
                SubscriptionManager.VT_IMS_ENABLED, VALUE_NOT_INITIALIZED, c);

        return (setting == VALUE_NOT_INITIALIZED) || (setting == VALUE_ON);
    }

    /** Checks if Wi-Fi calling is enabled. */
    public static boolean isWfcImsEnabled(Context c, int slotId) {
        int subId = MSimUtils.getSubId(slotId);
        int setting = SubscriptionManager.getIntegerSubscriptionProperty(subId,
                SubscriptionManager.WFC_IMS_ENABLED, VALUE_NOT_INITIALIZED, c);

        if (setting == VALUE_NOT_INITIALIZED) {
            return (getDefaultWfcImsEnabled(slotId) == VALUE_ON);
        }

        return (setting == VALUE_ON);
    }

    /** Checks if Wi-Fi calling is enabled in the roaming area. */
    public static boolean isWfcImsRoamingEnabled(Context c, int slotId) {
        int subId = MSimUtils.getSubId(slotId);
        int setting = SubscriptionManager.getIntegerSubscriptionProperty(subId,
                SubscriptionManager.WFC_IMS_ROAMING_ENABLED, VALUE_NOT_INITIALIZED, c);

        if (setting == VALUE_NOT_INITIALIZED) {
            return (getDefaultWfcImsRoamingEnabled(slotId) == VALUE_ON);
        }

        return (setting == VALUE_ON);
    }

    /** Returns the Wi-Fi calling mode. */
    public static int getWfcImsMode(Context c, int slotId) {
        int subId = MSimUtils.getSubId(slotId);
        int setting = SubscriptionManager.getIntegerSubscriptionProperty(subId,
                SubscriptionManager.WFC_IMS_MODE, VALUE_NOT_INITIALIZED, c);

        if (!isWfcModeEditable(slotId)
                || (setting == VALUE_NOT_INITIALIZED)) {
            return getDefaultWfcImsMode(slotId);
        }

        return setting;
    }

    /** Returns the Wi-Fi calling mode in the roaming area. */
    public static int getWfcImsRoamingMode(Context c, int slotId) {
        int subId = MSimUtils.getSubId(slotId);
        int setting = SubscriptionManager.getIntegerSubscriptionProperty(subId,
                SubscriptionManager.WFC_IMS_ROAMING_MODE, VALUE_NOT_INITIALIZED, c);

        if (setting == VALUE_NOT_INITIALIZED) {
            return getDefaultWfcImsRoamingMode(slotId);
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

    /** ContentObserver for Secure items. */
    public static void registerObserverForSecure(ContentResolver cr,
            String key, ContentObserver observer) {
        if (cr == null) {
            return;
        }

        cr.registerContentObserver(Settings.Secure.getUriFor(key), true, observer);
    }

    /** ContentObserver for System items. */
    public static void registerObserverForSystem(ContentResolver cr,
            String key, ContentObserver observer) {
        if (cr == null) {
            return;
        }

        cr.registerContentObserver(Settings.System.getUriFor(key), true, observer);
    }

    /** Unregister the content observer previously set. */
    public static void unregisterObserver(ContentResolver cr, ContentObserver observer) {
        if (cr == null || observer == null) {
            return;
        }

        cr.unregisterContentObserver(observer);
    }

    /** Registers the content observer for call settings. */
    public static void registerObserverForCallSettings(Context c,
            String key, ContentObserver observer, int slotId) {
        ContentResolver cr = c.getContentResolver();

        cr.registerContentObserver(
                SubscriptionManager.CONTENT_URI, true, observer);
    }
}
