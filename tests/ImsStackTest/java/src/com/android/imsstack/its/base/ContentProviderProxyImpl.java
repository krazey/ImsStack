/*
 * Copyright (C) 2024 The Android Open Source Project
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
package com.android.imsstack.its.base;

import android.database.ContentObserver;
import android.net.Uri;
import android.provider.Settings;
import android.util.ArrayMap;
import android.util.ArraySet;

import androidx.annotation.NonNull;

import com.android.imsstack.base.ContentProviderProxy;
import com.android.imsstack.util.Log;

import java.util.Map;

/**
 * An implementation class to access the content provider related APIs such as
 * {@link android.provider.Settings}.
 */
public class ContentProviderProxyImpl implements ContentProviderProxy {
    private final GlobalSettingsProxy mGlobalSettings;
    private final SecureSettingsProxy mSecureSettings;
    private final SystemSettingsProxy mSystemSettings;
    private final ArraySet<ContentObserverRecord> mContentObserverRecords = new ArraySet<>();

    ContentProviderProxyImpl() {
        mGlobalSettings = new GlobalSettingsProxy();
        mSecureSettings = new SecureSettingsProxy();
        mSystemSettings = new SystemSettingsProxy();
    }

    @Override
    public @NonNull SettingsProxy getGlobalSettings() {
        return mGlobalSettings;
    }

    @Override
    public @NonNull SettingsProxy getSecureSettings() {
        return mSecureSettings;
    }

    @Override
    public @NonNull SettingsProxy getSystemSettings() {
        return mSystemSettings;
    }

    @Override
    public void registerContentObserver(@NonNull Uri contentUri,
            @NonNull ContentObserver observer) {
        mContentObserverRecords.add(new ContentObserverRecord(contentUri, observer));
    }

    @Override
    public void unregisterContentObserver(@NonNull ContentObserver observer) {
        final ArraySet<ContentObserverRecord> recordsToRemove = new ArraySet<>();
        mContentObserverRecords.forEach((r) -> {
            if (r.hasContentObserver(observer)) {
                recordsToRemove.add(r);
            }
        });

        recordsToRemove.forEach(mContentObserverRecords::remove);
    }

    /**
     * Notifies the change of the specified content URI.
     */
    public void notifyChange(@NonNull Uri contentUri) {
        mContentObserverRecords.forEach((r) -> {
            if (r.hasUri(contentUri)) {
                r.dispatchChange();
            }
        });
    }

    /**
     * A base proxy implementation for SettingsProxy.
     */
    public abstract class SettingsProxyImpl implements SettingsProxy {
        private final Map<String, String> mSettings = new ArrayMap<>();

        @Override
        public void registerContentObserver(@NonNull String key,
                @NonNull ContentObserver observer) {
            ContentProviderProxyImpl.this.registerContentObserver(getUriFor(key), observer);
        }

        @Override
        public void unregisterContentObserver(@NonNull ContentObserver observer) {
            ContentProviderProxyImpl.this.unregisterContentObserver(observer);
        }

        @Override
        public int getInt(@NonNull String key, int defaultValue) {
            String value = mSettings.getOrDefault(key, String.valueOf(defaultValue));
            try {
                if (value != null) {
                    return Integer.parseInt(value);
                }
            } catch (NumberFormatException e) {
                Log.e(this, "getInt: " + key + ", exception=" + e, e);
            }

            return defaultValue;
        }

        @Override
        public String getString(@NonNull String key, String defaultValue) {
            return mSettings.getOrDefault(key, defaultValue);
        }

        /**
         * Sets the integer type setting.
         *
         * @param key The key to be set.
         * @param value The value to be set.
         */
        public void setInt(@NonNull String key, int value) {
            setString(key, String.valueOf(value));
        }

        /**
         * Sets the string type setting.
         *
         * @param key The key to be set.
         * @param value The value to be set.
         */
        public void setString(@NonNull String key, String value) {
            mSettings.put(key, value);
        }

        /**
         * Clears all the settings.
         */
        public void clearAll() {
            mSettings.clear();
        }

        protected abstract @NonNull Uri getUriFor(String key);
    }

    class GlobalSettingsProxy extends SettingsProxyImpl {
        @Override
        protected @NonNull Uri getUriFor(String key) {
            return Settings.Global.getUriFor(key);
        }
    }

    class SecureSettingsProxy extends SettingsProxyImpl {
        @Override
        protected @NonNull Uri getUriFor(String key) {
            return Settings.Secure.getUriFor(key);
        }
    }

    class SystemSettingsProxy extends SettingsProxyImpl {
        @Override
        protected @NonNull Uri getUriFor(String key) {
            return Settings.System.getUriFor(key);
        }
    }

    private static final class ContentObserverRecord {
        private final Uri mUri;
        private final ContentObserver mObserver;

        ContentObserverRecord(Uri uri, ContentObserver observer) {
            mUri = uri;
            mObserver = observer;
        }

        boolean hasContentObserver(ContentObserver observer) {
            return mObserver.equals(observer);
        }

        boolean hasUri(Uri uri) {
            return mUri.equals(uri);
        }

        void dispatchChange() {
            mObserver.dispatchChange(false, mUri);
        }
    }
}
