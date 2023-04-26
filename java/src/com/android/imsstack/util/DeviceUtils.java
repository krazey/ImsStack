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
package com.android.imsstack.util;

import android.os.Environment;
import android.provider.Settings;

/**
 * A class for providing the device related information.
 */
public final class DeviceUtils {
    /**
     * Returns the current device name.
     *
     * @return A device name.
     */
    public static String getDeviceName() {
        String deviceName = Settings.Global.getString(
                AppContext.getInstance().getContentResolver(), Settings.Global.DEVICE_NAME);

        if (deviceName == null) {
            ImsLog.w("Device name is empty.");
            deviceName = "";
        }

        return deviceName;
    }

    /**
     * Returns the external storage path.
     *
     * @return An external storage path.
     */
    public static String getExternalStoragePath() {
        String state = Environment.getExternalStorageState();

        if (!Environment.MEDIA_MOUNTED.equalsIgnoreCase(state)) {
            return "";
        }

        return Environment.getExternalStorageDirectory().getAbsolutePath();
    }

    private DeviceUtils() {}
}
