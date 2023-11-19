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
package com.android.imsstack.base;

import android.database.ContentObserver;
import android.net.Uri;

import androidx.annotation.NonNull;

/**
 * A proxy interface to access the content provider related APIs such as
 * {@link android.provider.Settings}.
 */
public interface ContentProviderProxy {
    /**
     * Returns the proxy interface for the global system settings.
     */
    @NonNull SettingsProxy getGlobalSettings();

    /**
     * Returns the proxy interface for the secure system settings.
     */
    @NonNull SettingsProxy getSecureSettings();

    /**
     * Returns the proxy interface for the system settings.
     */
    @NonNull SettingsProxy getSystemSettings();

    /**
     * Registers the {@link ContentObserver} for the specified {@code contentUri}.
     *
     * @param contentUri The content URI to register an observer.
     * @param observer The {@link ContentObserver} to register.
     */
    void registerContentObserver(@NonNull Uri contentUri, @NonNull ContentObserver observer);

    /**
     * Unregisters the {@link ContentObserver}.
     *
     * @param observer The {@link ContentObserver} to unregister.
     */
    void unregisterContentObserver(@NonNull ContentObserver observer);

    /**
     * A proxy interface for settings.
     */
    interface SettingsProxy {
        /**
         * Returns the settings value as an integer.
         *
         * @param key The name of the setting to retrieve.
         * @param defaultValue The value to return if the setting is not defined.
         * @return The setting's current value, or {@code defaultValue} if it is not defined.
         */
        int getInt(@NonNull String key, int defaultValue);

        /**
         * Returns the settings value as a string.
         *
         * @param key The name of the setting to retrieve.
         * @param defaultValue The value to return if the setting is not defined.
         * @return The setting's current value, or {@code defaultValue} if it is not defined.
         */
        String getString(@NonNull String key, String defaultValue);

        /**
         * Registers the {@link ContentObserver} for the specified {@code key}.
         *
         * @param key The name of the settings to register an observer.
         * @param observer The {@link ContentObserver} to register.
         */
        void registerContentObserver(@NonNull String key, @NonNull ContentObserver observer);

        /**
         * Unregisters the {@link ContentObserver}.
         *
         * @param observer The {@link ContentObserver} to unregister.
         */
        void unregisterContentObserver(@NonNull ContentObserver observer);
    }
}
