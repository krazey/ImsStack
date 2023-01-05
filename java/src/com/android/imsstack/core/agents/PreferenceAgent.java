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

import com.android.imsstack.system.ISystemAPIPreference;
import com.android.imsstack.system.SystemInterface;
import com.android.imsstack.util.ImsLog;
import com.android.imsstack.util.MSimUtils;

import java.util.Locale;

/**
 * A class to provide an interface to read/write the preferences of ImsStack.
 */
public class PreferenceAgent implements IPreference, ISystemAPIPreference {
    private static final String IMS_PREFERENCES = "ims_preferences";

    // IPhoneInfoSubscriber#PREFERENCE_VALUE_ENTYPE
    private static final int TYPE_STRING = 0;
    private static final int TYPE_BOOL = 1;
    private static final int TYPE_INT = 2;
    private static final int TYPE_LONG = 3;
    private static final int TYPE_FLOAT = 4;

    private static IPreference mPreferenceAgent;
    private Context mContext;

    public PreferenceAgent() {
    }

    public static IPreference getInstance() {
        if (mPreferenceAgent == null) {
            mPreferenceAgent = new PreferenceAgent();
        }

        return mPreferenceAgent;
    }

    @Override
    public void init(Context context) {
        mContext = context;

        SystemInterface.getInstance().setISystemAPIPreference(this);
    }

    @Override
    public void cleanup() {
        SystemInterface.getInstance().setISystemAPIPreference(null);
    }

    @Override
    public boolean getPreferenceBoolValue(String key) {
        return getPreferenceBoolValue(IMS_PREFERENCES, key, MSimUtils.DEFAULT_SLOT_ID);
    }

    @Override
    public boolean getPreferenceBoolValue(String fileName, String key) {
        return getPreferenceBoolValue(fileName, key, MSimUtils.DEFAULT_SLOT_ID);
    }

    @Override
    public boolean getPreferenceBoolValue(String key, int slotId) {
        return getPreferenceBoolValue(IMS_PREFERENCES, key, slotId);
    }

    @Override
    public boolean getPreferenceBoolValue(String fileName, String key, int slotId) {
        ImsLog.d("getPreferenceBoolValue() : FileName/Key/SlotId - " + fileName + "/" + key + "/" + slotId);

        if (TextUtils.isEmpty(key)) {
            return false;
        }

        if (TextUtils.isEmpty(fileName)) {
            return getPreferenceBoolValue(key, slotId);
        }

        if (mContext == null) {
            return false;
        }

        SharedPreferences objSharedPreferences = mContext.getSharedPreferences(
                getFileName(fileName, slotId), Context.MODE_PRIVATE);
        if (null == objSharedPreferences) {
            ImsLog.w("Shared Preference obj is null.");
            return false;
        }

        try {
            return objSharedPreferences.getBoolean(key, false);
        }
        catch (Exception e) { // ClassCastException
            ImsLog.e(" " + e);
        }

        return false;
    }

    @Override
    public String getPreferenceStrValue(String key) {
        return getPreferenceStrValue(IMS_PREFERENCES, key, MSimUtils.DEFAULT_SLOT_ID);
    }

    @Override
    public String getPreferenceStrValue(String fileName, String key) {
        return getPreferenceStrValue(fileName, key, MSimUtils.DEFAULT_SLOT_ID);
    }

    @Override
    public String getPreferenceStrValue(String key, int slotId) {
        return getPreferenceStrValue(IMS_PREFERENCES, key, slotId);
    }

    @Override
    public String getPreferenceStrValue(String fileName, String key, int slotId) {
        ImsLog.d("getPreferenceStrValue() : FileName/Key/SlotId - " + fileName + "/" + key + "/" + slotId);

        if (TextUtils.isEmpty(key)) {
            return null;
        }

        if (TextUtils.isEmpty(fileName)) {
            return getPreferenceStrValue(key, slotId);
        }

        if (mContext == null) {
            return null;
        }

        SharedPreferences objSharedPreferences = mContext.getSharedPreferences(
                getFileName(fileName, slotId), Context.MODE_PRIVATE);
        if (null == objSharedPreferences) {
            ImsLog.w("Shared Preference obj is null.");
            return null;
        }

        try {
            return objSharedPreferences.getString(key, "");
        }
        catch (Exception e) { // ClassCastException
            ImsLog.e("" + e);
        }

        return null;
    }

    @Override
    public void setPreferenceStrValue(String key, String value) {
        setPreferenceStrValue(IMS_PREFERENCES, key, value, MSimUtils.DEFAULT_SLOT_ID);
    }

    @Override
    public void setPreferenceStrValue(String fileName, String key, String value) {
        setPreferenceStrValue(fileName, key, value, MSimUtils.DEFAULT_SLOT_ID);
    }

    @Override
    public void setPreferenceStrValue(String key, String value, int slotId) {
        setPreferenceStrValue(IMS_PREFERENCES, key, value, slotId);
    }

    @Override
    public void setPreferenceStrValue(String fileName, String key, String value, int slotId) {
        ImsLog.d("setPreferenceStrValue() : FileName/Key/Value/SlotId - " + fileName + "/" + key + "/" + value + "/" + slotId);

        if (TextUtils.isEmpty(key)) {
            return;
        }

        if (value == null) {
            return;
        }

        if (TextUtils.isEmpty(fileName)) {
            setPreferenceStrValue(key, value, slotId);
            return;
        }

        if (mContext == null) {
            return;
        }

        SharedPreferences objSharedPreferences = mContext.getSharedPreferences(
                getFileName(fileName, slotId), Context.MODE_PRIVATE);
        if (null == objSharedPreferences) {
            return;
        }

        Editor objEditor = objSharedPreferences.edit();
        if (objEditor == null) {
            ImsLog.w("Editor is null");
            return;
        }
        objEditor.putString(key, value);
        objEditor.apply();
    }

    @Override
    public String getPreference4Sys(String fileName, String key, int valueType, int slotId) {
        ImsLog.i("Preference :: FileName=" + fileName
            + ", Key=" + key + ", ValueType=" + valueType + ", SlotId=" + slotId );

        if (TextUtils.isEmpty(fileName) || TextUtils.isEmpty(key)) {
            ImsLog.w("Invalid File & key value");
            return "";
        }

        if (!isValidValueType(valueType)) {
            ImsLog.w("Invalid parameter");
            return "";
        }

        if (valueType == TYPE_STRING) {
            return getPreferenceStrValue(fileName, key, slotId);
        }

        ImsLog.w("Unsupported Value Type");
        return "";
    }

    @Override
    public int setPreference4Sys(String fileName, String key, int valueType, String value, int slotId) {
        ImsLog.d("Preference :: FileName=" + fileName
                + ", Key=" + key + ", Value=" + value + ", ValueType=" + valueType + ", SlotId=" + slotId);

        if (TextUtils.isEmpty(fileName) || TextUtils.isEmpty(key)) {
            ImsLog.w("Invalid parameter");
            return 0;
        }

        if (!isValidValueType(valueType )) {
            ImsLog.w("Invalid parameter");
            return 0;
        }

        if (valueType == TYPE_STRING) {
            setPreferenceStrValue(fileName, key, value, slotId);
            return 1;
        }

        ImsLog.w("Unsupported Value Type");
        return 0;
    }

    @Override
    public void removePreferenceValue(String key) {
        removePreferenceValue(IMS_PREFERENCES, key, MSimUtils.DEFAULT_SLOT_ID);
    }

    @Override
    public void removePreferenceValue(String fileName, String key) {
        removePreferenceValue(fileName, key, MSimUtils.DEFAULT_SLOT_ID);
    }

    @Override
    public void removePreferenceValue(String key, int slotId) {
        removePreferenceValue(IMS_PREFERENCES, key, slotId);
    }

    @Override
    public void removePreferenceValue(String fileName, String key, int slotId) {
        ImsLog.d("removePreferenceValue() : FileName/Key/SlotId - " + fileName + "/" + key + "/" + slotId);

        if (TextUtils.isEmpty(key)) {
            return;
        }

        if (TextUtils.isEmpty(fileName)) {
            removePreferenceValue(key, slotId);
            return;
        }

        if (mContext == null) {
            return;
        }

        SharedPreferences objSharedPreferences = mContext.getSharedPreferences(
                fileName, Context.MODE_PRIVATE);
        if (null == objSharedPreferences) {
            return;
        }

        Editor objEditor = objSharedPreferences.edit();
        if (objEditor == null) {
            ImsLog.w("Editor is null");
            return;
        }
        objEditor.remove(key);
        objEditor.apply();
    }

    private static String getFileName(String fileName, int slotId) {
        return String.format(Locale.US, "%s_%d", fileName, slotId);
    }

    private static boolean isValidValueType(int valueType) {
        switch (valueType) {
            case TYPE_STRING: // fall-through
            case TYPE_BOOL: // fall-through
            case TYPE_INT: // fall-through
            case TYPE_LONG: // fall-through
            case TYPE_FLOAT:
                return true;
            default:
                return false;
        }
    }
}
