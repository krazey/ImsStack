/*
 * Copyright (C) 2023 The Android Open Source Project
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
package com.android.imsstack.base;

import androidx.annotation.NonNull;
import androidx.annotation.VisibleForTesting;

import com.android.imsstack.util.Log;

/**
 * A utility class that provides the device configuration.
 */
public class DeviceConfig {
    private static int sActiveSimCount = 1;
    private static int sSupportedSimCount = 1;

    /**
     * Initializes the device configuration.
     *
     * @param appContext The {@link AppContext} object.
     */
    public static void init(@NonNull AppContext appContext) {
        TelephonyManagerProxy tmp = appContext.getSystemServiceProxy(TelephonyManagerProxy.class);
        sActiveSimCount = tmp.getActiveModemCount();
        sSupportedSimCount = tmp.getSupportedModemCount();
        Log.d(DeviceConfig.class, "DeviceConfig: activeSimCount=" + sActiveSimCount
                + ", supportedSimCount=" + sSupportedSimCount);
    }

    /**
     * Returns the number of logical SIMs currently configured to be activated.
     */
    public static int getActiveSimCount() {
        return sActiveSimCount;
    }

    /**
     * Returns how many logical SIM can be potentially active simultaneously, in terms of hardware
     * capability.
     * It might return different value from {@link #getActiveSimCount}. For example, for a
     * dual-SIM capable device operating in single SIM mode (only one logical modem is turned on),
     * {@link #getActiveSimCount} returns 1 while this API returns 2.
     */
    public static int getSupportedSimCount() {
        return sSupportedSimCount;
    }

    /**
     * Checks if the device supports the multiple SIM cards.
     */
    public static boolean isMultiSimEnabled() {
        return getActiveSimCount() > 1;
    }

    /**
     * Sets the SIM count of the device for testing.
     *
     * @param activeSimCount The active modem count.
     * @param supportedSimCount The supported modem count.
     */
    @VisibleForTesting
    public static void setSimCount(int activeSimCount, int supportedSimCount) {
        sActiveSimCount = activeSimCount;
        sSupportedSimCount = supportedSimCount;
    }

    private DeviceConfig() {}
}
