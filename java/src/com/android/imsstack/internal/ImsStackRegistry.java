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
package com.android.imsstack.internal;

import android.util.SparseBooleanArray;

import com.android.imsstack.util.Log;

import java.util.Set;
import java.util.concurrent.CopyOnWriteArraySet;

/**
 * A main entry for initializing, starting, and stopping the internal modules of ImsStack.
 */
public class ImsStackRegistry {
    /**
     * A listener for monitoring the IMS service state.
     */
    public interface ImsServiceListener {
        /**
         * Called when IMS service is started on the specified slot.
         *
         * @param slotId The slot-id that started.
         */
        void onImsServiceStarted(int slotId);

        /**
         * Called when IMS service is stopped on the specified slot.
         *
         * @param slotId The slot-id that stopped.
         */
        void onImsServiceStopped(int slotId);
    }

    private static final SparseBooleanArray sImsServiceStates = new SparseBooleanArray();
    private static final Set<ImsServiceListener> sImsServiceListeners = new CopyOnWriteArraySet<>();

    private ImsStackRegistry() {}

    /**
     * Checks whether the IMS service is started on the specified slot.
     *
     * @param slotId The slot-id to be checked.
     * @return {@code true} if the IMS service is started, {@code false} otherwise.
     */
    public static boolean isImsServiceStarted(int slotId) {
        return sImsServiceStates.get(slotId);
    }

    /**
     * Sets the IMS service state.
     *
     * @param slotId The slot-id to be set.
     * @param state A flag specifying whether the IMS service is started or not.
     */
    public static void setImsServiceState(int slotId, boolean state) {
        boolean oldState = isImsServiceStarted(slotId);

        if (oldState != state) {
            Log.i(ImsStackRegistry.class, "ImsService-state=" + state);
            sImsServiceStates.put(slotId, state);
            notifyImsServiceState(slotId, state);
        }
    }

    /**
     * Adds the listener to monitor the IMS service state.
     *
     * @param listener The listener to be added.
     */
    public static void addImsServiceListener(ImsServiceListener listener) {
        sImsServiceListeners.add(listener);
    }

    /**
     * Removes the listener that was previously set.
     *
     * @param listener The listener to be removed.
     */
    public static void removeImsServiceListener(ImsServiceListener listener) {
        sImsServiceListeners.remove(listener);
    }

    private static void notifyImsServiceState(int slotId, boolean state) {
        for (ImsServiceListener listener : sImsServiceListeners) {
            if (state) {
                listener.onImsServiceStarted(slotId);
            } else {
                listener.onImsServiceStopped(slotId);
            }
        }
    }
}
