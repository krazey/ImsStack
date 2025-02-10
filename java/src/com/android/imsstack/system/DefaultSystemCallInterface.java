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

package com.android.imsstack.system;

import com.android.imsstack.core.agents.WifiInterface;

/**
 * An interface to provide the system related APIs for the native service
 * regardless of the SIM slot.
 */
public interface DefaultSystemCallInterface {
    /**
     * Sets the device stay on until timer expired.
     *
     * @param timeoutMillis The timeout value in milli-seconds.
     */
    void acquireWakeLock(int timeoutMillis);

    /**
     * Starts a timer with the specified duration for the native service.
     *
     * @param tid The timer id to be started.
     * @param duration The timer duration as milli-seconds.
     * @return {@code true} if a timer is successfully started, {@code false} otherwise.
     */
    boolean startTimer(long tid, long duration);

    /**
     * Stops the specified timer for the native service.
     *
     * @param tid The timer id to be stopped.
     */
    void stopTimer(long tid);

    /**
     * Returns the current battery level.
     *
     * @return The current battery level.
     */
    int getBatteryLevel();

    /**
     * Returns the Wi-Fi interface.
     *
     * @return A WifiInterface object.
     */
    WifiInterface getWifiInterface();

    /**
     * Returns the current device name.
     *
     * @return A device name.
     */
    String getDeviceName();

    /**
     * Returns the external storage path.
     *
     * @return An external storage path.
     */
    String getExternalStoragePath();

    /**
     * Returns a string value from the specified preference file for a specified slot.
     * If {@code fileName} is empty, then returns a string value from the default preference file.
     *
     * @param fileName The shared preference file name.
     * @param key The key to retrieve.
     * @param slotId The slot-id to find a correct file.
     * @return A string value.
     */
    String getPreference(String fileName, String key, int slotId);

    /**
     * Puts a string value to the specified preference file for a specified slot.
     * If {@code fileName} is empty, then puts a string value to the default preference file.
     *
     * @param fileName The shared preference file name.
     * @param key The key to update.
     * @param value The string value.
     * @param slotId The slot-id to find a correct file.
     * @return {@code true} if the key and value is successfully set, {@code false} otherwise.
     */
    boolean setPreference(String fileName, String key, String value, int slotId);

    /**
     * Sets the traffic priority for a specified slot.
     *
     * @param priorityType The priority type of the traffic.
     * @param slotId The slot-id to be set.
     */
    void setTrafficPriority(int priorityType, int slotId);

    /**
     * Excluding the slot provided as an input parameter, the SIM state of other slots is checked
     * to determine and return the availability of cross SIM redialing.
     *
     * @param slotId The slot ID where the emergency call connection failed.
     * @return {@code true} if cross SIM redialing is available, {@code false} otherwise.
     */
    boolean isCrossSimRedialingAvailable(int slotId);
}
