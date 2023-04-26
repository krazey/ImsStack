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

public final class LogUtils {
    public static final int TRACE_OPTION_D = 0x00000001;
    public static final int TRACE_OPTION_E = 0x00000002;
    public static final int TRACE_OPTION_I = 0x00000004;

    // logcat + all levels(D / E / I / TEXT)
    public static final String DEFAULT_LOG_OPTIONS = "0x0001000F";

    /**
     * Returns the logging options.
     *
     * @param slotId The slot-id to be read.
     * @return The bitmasks representing the logging options.
     *         {@link #TRACE_OPTION_D},
     *         {@link #TRACE_OPTION_E},
     *         {@link #TRACE_OPTION_I}
     */
    public static int getLogOptions(int slotId) {
        String logOptions = ImsPrivateProperties.Persistent.get(
                ImsPrivateProperties.Persistent.KEY_TEST_LOG_OPTIONS, DEFAULT_LOG_OPTIONS,
                slotId);
        return SystemUtils.hexStringToInt(logOptions);
    }

    /**
     * Checks if the IMS's debug option is enabled or not.
     *
     * @param slotId The slot-id to be read.
     * @return true if IMS's debug option is enabled, false otherwise.
     */
    public static boolean isDebugOn(int slotId) {
        return ImsPrivateProperties.Persistent.getBoolean(
                ImsPrivateProperties.Persistent.KEY_TEST_DEBUG_ENABLED, slotId);
    }

    private LogUtils() {}
}
