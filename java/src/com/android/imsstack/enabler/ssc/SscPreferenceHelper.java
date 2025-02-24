/*
 * Copyright (C) 2025 The Android Open Source Project
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

package com.android.imsstack.enabler.ssc;

import android.content.Context;
import android.content.SharedPreferences;
import android.content.SharedPreferences.Editor;

import com.android.imsstack.base.AppContext;
import com.android.imsstack.base.MSimUtils;
import com.android.imsstack.util.ImsLog;

/**
 * A helper class to utilize the shared preference with the specific keys.
 */
public class SscPreferenceHelper {
    // File name of the shared preference. Same file name is used regardless of slot ID
    private static final String FILE_NAME = "ssc_preference";
    // Key used to read/write current CLIR setting.
    private static final String KEY_CLIR = "clir_sub";

    private final int mSlotId;

    public SscPreferenceHelper(int slotId) {
        this.mSlotId = slotId;
    }

    /**
     * Returns a CLIR mode from the shared preference file for a current subscription ID.
     * If the current subscription ID is invalid or there is no data in the shared preference, then
     * returns -1 value.
     *
     * @return A CLIR mode.
     */
    public int queryClir() {
        int subId = getSubId();
        if (subId == MSimUtils.INVALID_SUB_ID) {
            ImsLog.e(mSlotId, "Invalid subId");
            return -1;
        }

        return getInt(KEY_CLIR + subId);
    }

    /**
     * Puts a CLIR mode value to the shared preference file for a current subscription ID.
     *
     * @param clirMode A CLIR mode.
     * @return {@code true} if the CLIR mode is successfully set, {@code false} otherwise.
     */
    public boolean updateClir(int clirMode) {
        int subId = getSubId();
        if (subId == MSimUtils.INVALID_SUB_ID) {
            ImsLog.e(mSlotId, "Invalid subId");
            return false;
        }

        putInt(KEY_CLIR + subId, clirMode);
        return true;
    }

    private int getInt(String key) {
        SharedPreferences sp = getSharedPreferences();
        return sp.getInt(key, -1);
    }

    private void putInt(String key, int value) {
        Editor editor = getSharedPreferences().edit();
        editor.putInt(key, value);
        editor.apply();
    }

    private SharedPreferences getSharedPreferences() {
        return AppContext.getInstance().getSharedPreferences(FILE_NAME, Context.MODE_PRIVATE);
    }

    private int getSubId() {
        int subId = MSimUtils.getSubId(mSlotId);
        return MSimUtils.isValidSubId(subId) ? subId : MSimUtils.INVALID_SUB_ID;
    }
}
