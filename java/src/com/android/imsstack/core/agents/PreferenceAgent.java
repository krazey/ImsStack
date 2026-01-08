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

import android.content.Context;
import android.content.SharedPreferences;
import android.content.SharedPreferences.Editor;
import android.text.TextUtils;

import com.android.imsstack.base.AppContext;
import com.android.imsstack.util.ImsLog;

import java.util.Locale;

/**
 * A class to provide an interface to read/write the preferences of ImsStack.
 */
public class PreferenceAgent implements PreferenceInterface {
    private static final String DEFAULT_PREFERENCES = "imsstack_preferences";

    public PreferenceAgent() {
    }

    @Override
    public void init(Context context) {
    }

    @Override
    public void cleanup() {
    }

    @Override
    public String getString(String key, int slotId) {
        return getString(DEFAULT_PREFERENCES, key, slotId);
    }

    @Override
    public String getString(String fileName, String key, int slotId) {
        ImsLog.d(this, slotId, "Pref: getString - file=" + fileName + ", key=" + key);

        if (TextUtils.isEmpty(key)) {
            return null;
        }

        SharedPreferences sp = getSharedPreferences(fileName, slotId);
        return sp.getString(key, null);
    }

    @Override
    public boolean putString(String key, String value, int slotId) {
        return putString(DEFAULT_PREFERENCES, key, value, slotId);
    }

    @Override
    public boolean putString(String fileName, String key, String value, int slotId) {
        ImsLog.d(this, slotId, "Pref: putString - file=" + fileName
                + ", key=" + key + ", value=" + value);

        if (TextUtils.isEmpty(key) || value == null) {
            return false;
        }

        Editor editor = getSharedPreferences(fileName, slotId).edit();
        editor.putString(key, value);
        editor.apply();
        return true;
    }

    @Override
    public void remove(String key, int slotId) {
        remove(DEFAULT_PREFERENCES, key, slotId);
    }

    @Override
    public void remove(String fileName, String key, int slotId) {
        ImsLog.d(this, slotId, "Pref: remove - file=" + fileName + ", key=" + key);

        if (TextUtils.isEmpty(key)) {
            return;
        }

        Editor editor = getSharedPreferences(fileName, slotId).edit();
        editor.remove(key);
        editor.apply();
    }

    private SharedPreferences getSharedPreferences(String fileName, int slotId) {
        return AppContext.getInstance().getSharedPreferences(
                getFileName(fileName, slotId), Context.MODE_PRIVATE);
    }

    private static String getFileName(String fileName, int slotId) {
        if (TextUtils.isEmpty(fileName)) {
            fileName = DEFAULT_PREFERENCES;
        }
        return String.format(Locale.US, "%s_%d", fileName, slotId);
    }
}
