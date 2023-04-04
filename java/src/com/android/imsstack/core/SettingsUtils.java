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
package com.android.imsstack.core;

import android.content.ContentResolver;
import android.content.Context;
import android.database.ContentObserver;
import android.provider.Settings;
import android.telephony.SubscriptionManager;

/**
 * This class provides the APIs to access Settings.
 */
public final class SettingsUtils {
    /** ContentObserver for Settings provider */
    public static void registerObserverForGlobal(ContentResolver cr,
            String key, ContentObserver observer) {
        if (cr == null) {
            return;
        }

        cr.registerContentObserver(Settings.Global.getUriFor(key), true, observer);
    }

    /** ContentObserver for Secure items. */
    public static void registerObserverForSecure(ContentResolver cr,
            String key, ContentObserver observer) {
        if (cr == null) {
            return;
        }

        cr.registerContentObserver(Settings.Secure.getUriFor(key), true, observer);
    }

    /** ContentObserver for System items. */
    public static void registerObserverForSystem(ContentResolver cr,
            String key, ContentObserver observer) {
        if (cr == null) {
            return;
        }

        cr.registerContentObserver(Settings.System.getUriFor(key), true, observer);
    }

    /** Unregister the content observer previously set. */
    public static void unregisterObserver(ContentResolver cr, ContentObserver observer) {
        if (cr == null || observer == null) {
            return;
        }

        cr.unregisterContentObserver(observer);
    }

    /** Registers the content observer for call settings. */
    public static void registerObserverForCallSettings(Context c,
            String key, ContentObserver observer, int slotId) {
        ContentResolver cr = c.getContentResolver();

        cr.registerContentObserver(
                SubscriptionManager.CONTENT_URI, true, observer);
    }
}
