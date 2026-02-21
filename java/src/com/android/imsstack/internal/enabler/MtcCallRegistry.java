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
package com.android.imsstack.internal.enabler;

import android.util.SparseArray;

import androidx.annotation.VisibleForTesting;

import com.android.imsstack.base.DeviceConfig;
import com.android.imsstack.enabler.mtc.Call;

import java.util.Set;
import java.util.concurrent.CopyOnWriteArraySet;

/**
 * A registry that manages listeners for MTC call lifecycle events.
 *
 * This class provides a singleton instance per slot ID and notifies registered listeners when
 * {@link Call} objects are created or destroyed.
 */
public class MtcCallRegistry {

    /**
     * An interface for listening to {@link Call} lifecycle events.
     */
    public interface Listener {
        /**
         * Invoked when a {@link Call} is created.
         *
         * @param call the call that was created
         */
        default void onCallCreated(Call call) {
        }

        /**
         * Invoked when a {@link Call} is destroyed.
         *
         * @param call the call that was destroyed
         */
        default void onCallDestroyed(Call call) {
        }
    }

    // Maps a slot ID to its corresponding MtcCallRegistry singleton instance.
    private static SparseArray<MtcCallRegistry> sMtcCallRegistry =
            new SparseArray<>(DeviceConfig.getSupportedSimCount());
    // The set of registered listeners. CopyOnWriteArraySet is used for thread-safe iteration.
    private final Set<Listener> mListeners = new CopyOnWriteArraySet<>();

    /**
     * Returns an {@link MtcCallRegistry} instance for the given slot ID.
     *
     * This method provides a singleton instance per slot ID. If an instance for the given
     * {@code slotId} does not exist, a new one is created.
     *
     * @param slotId the slot ID for which to retrieve the registry
     * @return the unique {@link MtcCallRegistry} instance for the given slot ID
     */
    public static MtcCallRegistry getInstance(int slotId) {
        MtcCallRegistry mtcCallRegistry;

        synchronized (sMtcCallRegistry) {
            mtcCallRegistry = sMtcCallRegistry.get(slotId);
            if (mtcCallRegistry == null) {
                mtcCallRegistry = new MtcCallRegistry();
                sMtcCallRegistry.put(slotId, mtcCallRegistry);
            }
        }

        return mtcCallRegistry;
    }

    /**
     * Clears all registry instances. For testing purposes only.
     */
    @VisibleForTesting
    public static void clearInstances() {
        synchronized (sMtcCallRegistry) {
            sMtcCallRegistry.clear();
        }
    }

    /**
     * Adds a listener to monitor {@link Call} lifecycle events.
     *
     * @param listener the listener to be added
     */
    public void addListener(Listener listener) {
        mListeners.add(listener);
    }

    /**
     * Removes a listener that was previously added.
     *
     * @param listener the listener to be removed
     */
    public void removeListener(Listener listener) {
        mListeners.remove(listener);
    }

    /**
     * Notifies listeners when a call is created.
     *
     * @param call the call that was created
     */
    public void notifyCallCreated(Call call) {
        for (Listener l : mListeners) {
            l.onCallCreated(call);
        }
    }

    /**
     * Notifies listeners when a call is destroyed.
     *
     * @param call the call that was destroyed
     */
    public void notifyCallDestroyed(Call call) {
        for (Listener l : mListeners) {
            l.onCallDestroyed(call);
        }
    }
}
