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

import android.content.Context;
import android.database.ContentObserver;
import android.net.Uri;
import android.provider.Settings;

import androidx.annotation.NonNull;

import com.android.imsstack.util.Log;

/**
 * An implementation class to access the content provider related APIs such as
 * {@link android.provider.Settings}.
 */
public class ContentProviderProxyImpl implements ContentProviderProxy {
    private final Context mContext;
    private final GlobalSettingsProxy mGlobalSettings;
    private final SecureSettingsProxy mSecureSettings;
    private final SystemSettingsProxy mSystemSettings;

    ContentProviderProxyImpl(Context context) {
        mContext = context;
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
        mContext.getContentResolver().registerContentObserver(contentUri, true, observer);
    }

    @Override
    public void unregisterContentObserver(@NonNull ContentObserver observer) {
        mContext.getContentResolver().unregisterContentObserver(observer);
    }

    abstract class SettingsProxyImpl implements SettingsProxy {
        @Override
        public void registerContentObserver(@NonNull String key,
                @NonNull ContentObserver observer) {
            ContentProviderProxyImpl.this.registerContentObserver(getUriFor(key), observer);
        }

        @Override
        public void unregisterContentObserver(@NonNull ContentObserver observer) {
            ContentProviderProxyImpl.this.unregisterContentObserver(observer);
        }

        protected abstract @NonNull Uri getUriFor(String key);
    }

    class GlobalSettingsProxy extends SettingsProxyImpl {
        @Override
        public int getInt(@NonNull String key, int defaultValue) {
            try {
                return Settings.Global.getInt(mContext.getContentResolver(), key, defaultValue);
            } catch (SecurityException e) {
                Log.e(this, "Global#getInt: " + key + ", exception=" + e, e);
                return defaultValue;
            }
        }

        @Override
        public String getString(@NonNull String key, String defaultValue) {
            try {
                String value = Settings.Global.getString(mContext.getContentResolver(), key);
                return value != null ? value : defaultValue;
            } catch (SecurityException e) {
                Log.e(this, "Global#getString " + key + ", exception=" + e, e);
                return defaultValue;
            }
        }

        @Override
        protected @NonNull Uri getUriFor(String key) {
            return Settings.Global.getUriFor(key);
        }
    }

    class SecureSettingsProxy extends SettingsProxyImpl {
        @Override
        public int getInt(@NonNull String key, int defaultValue) {
            try {
                return Settings.Secure.getInt(mContext.getContentResolver(), key, defaultValue);
            } catch (SecurityException e) {
                Log.e(this, "Secure#getInt: " + key + ", exception=" + e, e);
                return defaultValue;
            }
        }

        @Override
        public String getString(@NonNull String key, String defaultValue) {
            try {
                String value = Settings.Secure.getString(mContext.getContentResolver(), key);
                return value != null ? value : defaultValue;
            } catch (SecurityException e) {
                Log.e(this, "Secure#getString: " + key + ", exception=" + e, e);
                return defaultValue;
            }
        }

        @Override
        protected @NonNull Uri getUriFor(String key) {
            return Settings.Secure.getUriFor(key);
        }
    }

    class SystemSettingsProxy extends SettingsProxyImpl {
        @Override
        public int getInt(@NonNull String key, int defaultValue) {
            try {
                return Settings.System.getInt(mContext.getContentResolver(), key, defaultValue);
            } catch (SecurityException e) {
                Log.e(this, "System#getInt: " + key + ", exception=" + e, e);
                return defaultValue;
            }
        }

        @Override
        public String getString(@NonNull String key, String defaultValue) {
            try {
                String value = Settings.System.getString(mContext.getContentResolver(), key);
                return value != null ? value : defaultValue;
            } catch (SecurityException e) {
                Log.e(this, "System#getString: " + key + ", exception=" + e, e);
                return defaultValue;
            }
        }

        @Override
        protected @NonNull Uri getUriFor(String key) {
            return Settings.System.getUriFor(key);
        }
    }
}
