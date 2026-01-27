/*
 * Copyright (C) 2026 The Android Open Source Project
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

import static android.telephony.TelephonyManager.DomainSelectionEmergencyType;
import static android.telephony.TelephonyManager.EmergencyCallbackModeType;

import androidx.annotation.NonNull;

/**
 * An interface for monitoring the emergency state such as emergency mode entered/exited.
 */
public interface EmergencyStateInterface extends IAgent {
    /**
     * Represents the state of the emergency callback mode.
     */
    enum EmergencyCallbackModeState {

        STOP(0),
        START(1),
        STOP_BY_EMERGENCY(2);

        private final int mValue;

        EmergencyCallbackModeState(int value) {
            mValue = value;
        }

        /**
         * Returns the integer value associated with this emergency callback mode state.
         *
         * @return The integer value.
         */
        public int getValue() {
            return mValue;
        }

        /**
         * Returns a string representation of this state, including both its
         * symbolic name and its integer value (e.g., "START(1)").
         * This format is useful for logging, debugging, and dumpsys output,
         * providing more context than just the name.
         *
         * @return The string representation of the enum in "NAME(VALUE)" format
         * (e.g., "START(1)").
         */
        @Override
        public String toString() {
            return this.name() + "(" + mValue + ")";
        }
    }

    /**
     * An interface to monitor the emergency state events.
     */
    interface EmergencyStateListener {
        /**
         * Called when the emergency mode is changed.
         *
         * @param type The emergency mode type.
         * @param entered {@code true} if the emergency mode is entered, {@code false} otherwise.
         */
        void onEmergencyModeChanged(@DomainSelectionEmergencyType int type, boolean entered);

        /**
         * Called to notify the update of emergency callback mode.
         *
         * @param type {@code type} is callback mode entry {@link EmergencyCallbackModeType}
         * @param state {@code state} is type of {@link EmergencyCallbackModeState}.
         * @param duration is the number of seconds remaining in the emergency callback mode.
         */
        default void onEmergencyCallbackModeChanged(@EmergencyCallbackModeType int type,
                EmergencyCallbackModeState state, long duration) {}
    }

    /**
     * Adds the listener to monitor the emergency state.
     *
     * @param listener A {@link EmergencyStateListener} instance to be added.
     */
    void addListener(@NonNull EmergencyStateListener listener);

    /**
     * Removes the listener to monitor the emergency state.
     *
     * @param listener A {@link EmergencyStateListener} instance to be removed.
     */
    void removeListener(@NonNull EmergencyStateListener listener);

    /**
     * Checks if the emergency mode is entered for the specified type.
     *
     * @param type The emergency mode type.
     * @return {@code true} if the emergency mode is entered, {@code false} otherwise.
     */
    boolean isInEmergencyMode(@DomainSelectionEmergencyType int type);
}
