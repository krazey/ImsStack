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
     * Returns the Wi-Fi interface.
     *
     * @return A WifiInterface object.
     */
    WifiInterface getWifiInterface();
}
