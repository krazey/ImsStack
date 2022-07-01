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
package com.android.imsstack.enabler.acs.impl;

import android.annotation.Nullable;
import android.content.Context;
import android.os.PersistableBundle;

import com.android.imsstack.util.ImsLog;

import java.io.File;
import java.io.FileInputStream;
import java.io.FileOutputStream;
import java.io.IOException;

/**
 * This class handles the RCS provisioning XML data and is created based on sub ID
 */
public class DataContainer {
    public  static final int VERSION_UNKNOWN = -99;
    public  static final long VALIDITY_UNKNOWN = -99L;
    public  static final String TOKEN_UNKNOWN = "";

    private static final String LOCAL_FILE_NAME_PREF = "rcs_provisioning_data_";
    private static final String KEY_VERSION = "version";
    private static final String KEY_VALIDITY = "validity";
    private static final String KEY_TOKEN = "token";
    private static final String KEY_DATA = "data";

    private final Context mContext;
    private final int mSlotId;
    private final int mSubId;
    private final String mFileName;

    private PersistableBundle mPersistableBundle;

    public DataContainer(Context context, int slotId, int subId) {
        mContext = context;
        mSlotId = slotId;
        mSubId = subId;

        mFileName = LOCAL_FILE_NAME_PREF + subId + ".xml";
        readDataFromFile(mFileName);
    }

    /**
     * Get copied provisioning data
     * @return String includes provisioning data, but if the internal provisioning data
     * is not valid then the null is returned.
     */
    public @Nullable String getProvisioningData() {
        if (mPersistableBundle != null && !mPersistableBundle.isEmpty()) {
            return mPersistableBundle.getString(KEY_DATA);
        }

        ImsLog.i(mSlotId, "provisioning data is empty.");
        return null;
    }

    /**
     * Set provisioning data
     * @param data Provisioning data
     */
    public void setProvisioningData(String data) {
        if (mPersistableBundle == null) {
            mPersistableBundle = new PersistableBundle();
        }
        mPersistableBundle.putString(KEY_DATA, data);

        saveDataToFile(mFileName);
    }

    /**
     * notify the stored provisioning data is not valid
     */
    public void resetProvisioningData() {
        if (mPersistableBundle != null) {
            mPersistableBundle.putString(KEY_DATA, "");
            mPersistableBundle.putInt(KEY_VERSION, VERSION_UNKNOWN);
            mPersistableBundle.putLong(KEY_VALIDITY, VALIDITY_UNKNOWN);
            mPersistableBundle.putString(KEY_TOKEN, TOKEN_UNKNOWN);

            saveDataToFile(mFileName);
        }
    }

    /**
     * get version value in provisioning data
     * @return version value
     */
    public int getVersion() {
        if (mPersistableBundle == null) {
            ImsLog.i(mSlotId, "provisioning data is not available");
            return VERSION_UNKNOWN;
        }
        return mPersistableBundle.getInt(KEY_VERSION, VERSION_UNKNOWN);
    }

    /**
     * get validity value in provisioning data
     * @return validity value
     */
    public long getValidity() {
        if (mPersistableBundle == null) {
            ImsLog.i(mSlotId, "provisioning data is not available");
            return VALIDITY_UNKNOWN;
        }
        return mPersistableBundle.getLong(KEY_VALIDITY, VALIDITY_UNKNOWN);
    }

    /**
     * get token value in provisioning data
     * @return token value
     */
    public String getToken() {
        if (mPersistableBundle == null) {
            ImsLog.i("provisioning data is not available");
            return TOKEN_UNKNOWN;
        }
        return mPersistableBundle.getString(KEY_TOKEN, TOKEN_UNKNOWN);
    }

    /**
     * update new version and validity value
     * @param newVersion version
     * @param newValidity validity
     * @return true if the operation is success, or false otherwise
     */
    public boolean updateVersionValidity(int newVersion, long newValidity) {
        if (!isValidProvisioning()) {
            ImsLog.i(mSlotId, "provisioning data is not available");
            return false;
        }

        ImsLog.i(mSlotId, "old version : " + getVersion() + " new version : " + newVersion);

        ImsLog.i(mSlotId, "old validity : " + getValidity() + " new validity : " + newValidity);

        mPersistableBundle.putInt(KEY_VERSION, newVersion);
        mPersistableBundle.putLong(KEY_VALIDITY, newValidity);

        saveDataToFile(mFileName);

        return true;
    }

    /**
     * update new token value
     * @param newToken token
     * @return true if the operation is success, or false otherwise
     */
    public boolean updateToken(String newToken) {
        if (!isValidProvisioning()) {
            ImsLog.i(mSlotId, "provisioning data is not available");
            return false;
        }

        ImsLog.i(mSlotId, "old token : " + getToken() + " new token : " + newToken);

        mPersistableBundle.putString(KEY_TOKEN, newToken);

        saveDataToFile(mFileName);

        return true;
    }

    /**
     * Get this object has valid provisioning data
     * @return true if object has valid provisioning, or false otherwise.
     */
    public boolean isValidProvisioning() {
        return !(mPersistableBundle == null
                || mPersistableBundle.isEmpty()
                || mPersistableBundle.getString(KEY_DATA, null) == null);
    }

    /**
     * Get file name which stored the provisioning data
     * @return file name
     */
    public String getFileName() {
        return mFileName;
    }

    private void readDataFromFile(String fileName) {
        // load xml data from FS based on subId
        try {
            File file = new File(mContext.getFilesDir(), fileName);
            FileInputStream inputStream = new FileInputStream(file);
            mPersistableBundle = PersistableBundle.readFromStream(inputStream);
            inputStream.close();
            ImsLog.d(mSlotId, "file name : " + fileName);
        } catch (IOException e) {
            ImsLog.i(mSlotId, e.getMessage());
            mPersistableBundle = null;
        }
    }

    private void saveDataToFile(String fileName) {
        if (!isValidProvisioning()) {
            ImsLog.i(mSlotId, "provisioning data is empty");
            return;
        }

        try {
            File file = new File(mContext.getFilesDir(), fileName);
            FileOutputStream outputStream = new FileOutputStream(file);
            mPersistableBundle.writeToStream(outputStream);
            outputStream.close();
            ImsLog.d(mSlotId, "file name : " + fileName);
        } catch (IOException e) {
            ImsLog.i(mSlotId, e.getMessage());
        }
    }
}
