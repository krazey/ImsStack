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

package com.android.imsstack.core.agents;

import android.annotation.NonNull;
import android.os.PersistableBundle;

import com.android.imsstack.core.config.CarrierConfig;

/** An interface for providing the configuration related information. */
public interface ConfigInterface extends IAgent {
    /**
     * A listener to monitor the change notification when the configuration is changed.
     */
    public interface Listener {
        /**
         * Notifies the caller that the carrier configuration is changed.
         *
         * @param slotId The slot id to notify of the change.
         * @param subId The subscription id to notify of the change.
         *              If the subscription is not detected successfully, this subscription id is
         *              set to {@link SubscriptionManager#INVALID_SUBSCRIPTION_ID}.
         */
        default void onCarrierConfigChanged(int slotId, int subId) {
        }
    }

    /**
     * Returns the carrier configuration of this interface.
     *
     * @return A CarrierConfig object.
     */
    CarrierConfig getCarrierConfig();

    /**
     * Reads the carrier configuration bundle for testing from the persistent storage.
     *
     * @return A PersistableBundle which contains the test configuration.
     */
    PersistableBundle readTestConfig();

    /**
     * Writes the carrier configuration for testing to the persistent storage.
     *
     * @param config A carrier configuration for testing.
     *
     * @return true if it's successfully written to the file, false otherwise.
     */
    boolean writeTestConfig(PersistableBundle config);

    /**
     * Checks if the carrier configuration is loaded or not.
     *
     * @return {@code true} if the configuration is loaded, {@code false} otherwise.
     */
    boolean isConfigLoaded();

    /**
     * Adds a listener to monitor the change notification of the configuration.
     *
     * @param listener A listener to be added.
     */
    void addListener(@NonNull Listener listener);

    /**
     * Removes the listener that was previously added.
     *
     * @param listener A listener to be removed.
     */
    void removeListener(@NonNull Listener listener);

    /**
     * Notifies the carrier configuration change to the native layer.
     */
    void notifyCarrierConfigChangedForNative();
}
