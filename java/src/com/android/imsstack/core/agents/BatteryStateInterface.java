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
package com.android.imsstack.core.agents;

/**
 * This provides an interface to check and control the battery status of the device.
 */
public interface BatteryStateInterface extends IAgent {
    /** Indicates that the battery level couldn't be recognized. */
    int INVALID_BATTERY_LEVEL = -1;

    /**
     * Returns the current battery level.
     */
    int getBatteryLevel();

    /**
     * Checks whether the current battery level is low or not.
     */
    boolean isLowBattery();

    /**
     * Checks whether the device is connected by USB cable or charger.
     */
    boolean isPowerPlugged();

    /**
     * Notifies the low battery state to the native layer.
     */
    void notifyLowBatteryState(int slotId);
}
