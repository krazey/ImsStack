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
    // File name of the shared preference. Same file name is used regardless of slot ID.
    private static final String FILE_NAME = "ssc_preference";
    // Preference Key Prefixes.
    private static final String KEY_CB = "cb_sub"; // + condition + service class
    private static final String KEY_OIR = "oir_sub";
    private static final String KEY_OIP = "oip_sub";
    private static final String KEY_TIR = "tir_sub";
    private static final String KEY_TIP = "tip_sub";

    private final int mSlotId;

    public SscPreferenceHelper(int slotId) {
        this.mSlotId = slotId;
    }


    /**
     * Returns a call barring status from the shared preference file for a current subscription ID.
     * If the current subscription ID is invalid or there is no data in the shared preference, then
     * returns {@code SscConstant#STATUS_NOT_REGISTERED}.
     *
     * @param condition A call barring condition. See SscConstant#CONDITION_XXX.
     * @param serviceClass A call barring service class. See SscServiceClassUtil#SERVICE_CLASS_XXX.
     * @return A call barring status.
     */
    public int queryCb(int condition, int serviceClass) {
        int subId = getSubId();
        if (subId == MSimUtils.INVALID_SUB_ID) {
            ImsLog.e(mSlotId, "Invalid subId");
            return SscConstant.STATUS_NOT_REGISTERED;
        }

        return getInt(getCbKey(subId, condition, serviceClass), SscConstant.STATUS_NOT_REGISTERED);
    }

    /**
     * Puts a call barring status value to the shared preference file for a current subscription ID.
     *
     * @param condition A call barring condition. See SscConstant#CONDITION_XXX.
     * @param serviceClass A call barring service class.  See SscServiceClassUtil#SERVICE_CLASS_XXX.
     * @param status A call barring status.
     * @return {@code true} if the OIR mode is successfully set, {@code false} otherwise.
     */
    public boolean updateCb(int condition, int serviceClass, int status) {
        int subId = getSubId();
        if (subId == MSimUtils.INVALID_SUB_ID) {
            ImsLog.e(mSlotId, "Invalid subId");
            return false;
        }

        putInt(getCbKey(subId, condition, serviceClass), status);
        return true;
    }

    /**
     * Returns a OIR mode from the shared preference file for a current subscription ID.
     * If the current subscription ID is invalid or there is no data in the shared preference, then
     * returns {@code SscConstant#STATUS_NOT_REGISTERED}.
     *
     * @return A OIR mode.
     */
    public int queryOir() {
        int subId = getSubId();
        if (subId == MSimUtils.INVALID_SUB_ID) {
            ImsLog.e(mSlotId, "Invalid subId");
            return SscConstant.STATUS_NOT_REGISTERED;
        }

        return getInt(KEY_OIR + subId, SscConstant.STATUS_NOT_REGISTERED);
    }

    /**
     * Puts a OIR mode value to the shared preference file for a current subscription ID.
     *
     * @param oirMode A OIR mode.
     * @return {@code true} if the OIR mode is successfully set, {@code false} otherwise.
     */
    public boolean updateOir(int oirMode) {
        int subId = getSubId();
        if (subId == MSimUtils.INVALID_SUB_ID) {
            ImsLog.e(mSlotId, "Invalid subId");
            return false;
        }

        putInt(KEY_OIR + subId, oirMode);
        return true;
    }

    /**
     * Returns a OIP status from the shared preference file for a current subscription ID.
     * If the current subscription ID is invalid or there is no data in the shared preference, then
     * returns {@code SscConstant#STATUS_NOT_REGISTERED}.
     *
     * @return A OIP status.
     */
    public int queryOip() {
        int subId = getSubId();
        if (subId == MSimUtils.INVALID_SUB_ID) {
            ImsLog.e(mSlotId, "Invalid subId");
            return SscConstant.STATUS_NOT_REGISTERED;
        }

        return getInt(KEY_OIP + subId, SscConstant.STATUS_NOT_REGISTERED);
    }

    /**
     * Puts a OIP status value to the shared preference file for a current subscription ID.
     *
     * @param status A OIP status.
     * @return {@code true} if the OIP status is successfully set, {@code false} otherwise.
     */
    public boolean updateOip(int status) {
        int subId = getSubId();
        if (subId == MSimUtils.INVALID_SUB_ID) {
            ImsLog.e(mSlotId, "Invalid subId");
            return false;
        }

        putInt(KEY_OIP + subId, status);
        return true;
    }

    /**
     * Returns a TIR mode from the shared preference file for a current subscription ID.
     * If the current subscription ID is invalid or there is no data in the shared preference, then
     * returns {@code SscConstant#STATUS_NOT_REGISTERED}.
     *
     * @return A TIR mode.
     */
    public int queryTir() {
        int subId = getSubId();
        if (subId == MSimUtils.INVALID_SUB_ID) {
            ImsLog.e(mSlotId, "Invalid subId");
            return SscConstant.STATUS_NOT_REGISTERED;
        }

        return getInt(KEY_TIR + subId, SscConstant.STATUS_NOT_REGISTERED);
    }

    /**
     * Puts a TIR mode value to the shared preference file for a current subscription ID.
     *
     * @param tirMode A TIR mode.
     * @return {@code true} if the TIR mode is successfully set, {@code false} otherwise.
     */
    public boolean updateTir(int tirMode) {
        int subId = getSubId();
        if (subId == MSimUtils.INVALID_SUB_ID) {
            ImsLog.e(mSlotId, "Invalid subId");
            return false;
        }

        putInt(KEY_TIR + subId, tirMode);
        return true;
    }

    /**
     * Returns a TIP status from the shared preference file for a current subscription ID.
     * If the current subscription ID is invalid or there is no data in the shared preference, then
     * returns {@code SscConstant#STATUS_NOT_REGISTERED}.
     *
     * @return A TIP status.
     */
    public int queryTip() {
        int subId = getSubId();
        if (subId == MSimUtils.INVALID_SUB_ID) {
            ImsLog.e(mSlotId, "Invalid subId");
            return SscConstant.STATUS_NOT_REGISTERED;
        }

        return getInt(KEY_TIP + subId, SscConstant.STATUS_NOT_REGISTERED);
    }

    /**
     * Puts a TIP status value to the shared preference file for a current subscription ID.
     *
     * @param status A TIP status.
     * @return {@code true} if the TIP status is successfully set, {@code false} otherwise.
     */
    public boolean updateTip(int status) {
        int subId = getSubId();
        if (subId == MSimUtils.INVALID_SUB_ID) {
            ImsLog.e(mSlotId, "Invalid subId");
            return false;
        }

        putInt(KEY_TIP + subId, status);
        return true;
    }

    private int getInt(String key, int defaultValue) {
        SharedPreferences sp = getSharedPreferences();
        return sp.getInt(key, defaultValue);
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

    private static String getCbKey(int subId, int condition, int serviceClass) {
        return KEY_CB + subId + "_" + condition + "_" + serviceClass;
    }
}
