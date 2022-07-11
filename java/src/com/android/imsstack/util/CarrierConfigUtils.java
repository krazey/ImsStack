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

import android.os.PersistableBundle;
import android.telephony.CarrierConfigManager;

/**
 * This class provides the APIs to access Carrier Configuration.
 *   : operator-app > oem-default-app > google-aosp-app > static-default
 */
public final class CarrierConfigUtils {
    public static boolean getBoolean(String key) {
        return getBoolean(key, MSimUtils.getImsDefaultSubId());
    }

    public static boolean getBoolean(String key, int subId) {
        CarrierConfigManager ccm = AppContext.getInstance().getSystemService(
                CarrierConfigManager.class);

        PersistableBundle b = null;

        if (ccm != null) {
            // If an invalid subId is used, this bundle will contain default values.
            b = ccm.getConfigForSubId(subId);
        }

        if (b != null) {
            return b.getBoolean(key);
        }

        // Return static default defined in CarrierConfigManager.
        return CarrierConfigManager.getDefaultConfig().getBoolean(key);
    }

    public static boolean getBooleanForSlot(String key, int slotId) {
        // FIXME: slotId & phoneId is same
        return getBoolean(key, MSimUtils.getSubId(slotId));
    }

    public static int getInt(String key) {
        return getInt(key, MSimUtils.getImsDefaultSubId());
    }

    public static int getInt(String key, int subId) {
        CarrierConfigManager ccm = AppContext.getInstance().getSystemService(
                CarrierConfigManager.class);

        PersistableBundle b = null;

        if (ccm != null) {
            // If an invalid subId is used, this bundle will contain default values.
            b = ccm.getConfigForSubId(subId);
        }

        if (b != null) {
            return b.getInt(key);
        }

        // Return static default defined in CarrierConfigManager.
        return CarrierConfigManager.getDefaultConfig().getInt(key);
    }

    public static int getIntForSlot(String key, int slotId) {
        // FIXME: slotId & phoneId is same
        return getInt(key, MSimUtils.getSubId(slotId));
    }

    public static String getString(String key) {
        return getString(key, MSimUtils.getImsDefaultSubId());
    }

    public static String getString(String key, int subId) {
        CarrierConfigManager ccm = AppContext.getInstance().getSystemService(
                CarrierConfigManager.class);

        PersistableBundle b = null;

        if (ccm != null) {
            // If an invalid subId is used, this bundle will contain default values.
            b = ccm.getConfigForSubId(subId);
        }

        if (b != null) {
            return b.getString(key);
        }

        // Return static default defined in CarrierConfigManager.
        return CarrierConfigManager.getDefaultConfig().getString(key);
    }

    public static String getStringForSlot(String key, int slotId) {
        // FIXME: slotId & phoneId is same
        return getString(key, MSimUtils.getSubId(slotId));
    }
}
