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

package com.android.imsstack.enabler.mtc;

import com.android.imsstack.enabler.mtc.reg.MtcServiceState;

public interface IServiceStateTracker {
    /**
     * Checks if the specified service is registered.
     *
     * @param serviceType IUMtcService.SERVICE_XXX
     */
    public boolean isServiceRegistered(int serviceType);

    /**
     * When an emergency call ends, if the emergency service state is ES_UNAVAILABLE, change it to
     * ES_IDLE to ensure the next emergency call attempt.
     *
     */
    void handleEmergencyCallDestroyed();

    /**
     * Adds a listener to monitor the Mtc service state change.
     *
     * @param listener The listener to be set.
     */
    void addListener(Listener listener);

    /**
     * Removes the listener that was previously set.
     *
     * @param listener The listener to be removed.
     */
    void removeListener(Listener listener);

    /**
     * Listener interface to receive the change notification of {@link MtcServiceState}.
     */
    interface Listener {
        /**
         * Invoked when emergency service state is changed.
         */
        default void onEmergencyServiceStateChanged(MtcServiceState serviceState) {
        }

        /**
         * Invoked when normal service state is changed.
         */
        default void onNormalServiceStateChanged(MtcServiceState serviceState) {
        }
    }
}
