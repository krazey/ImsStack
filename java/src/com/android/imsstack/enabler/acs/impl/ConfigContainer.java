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

import android.content.Context;
import android.os.PersistableBundle;
import android.text.TextUtils;

import com.android.imsstack.enabler.acs.AcServiceClientInfo;
import com.android.imsstack.util.ImsLog;
import com.android.internal.annotations.VisibleForTesting;

import java.io.File;
import java.io.FileInputStream;
import java.io.FileOutputStream;

/**
 * This class handles the provisioning config data based on subId
 * To handle all of related data for each subId with one xml file, this class will be worked static.
 *    client info : message client including rcs enabled or disabled
 *    provisioning validation : provisioning data expire time
 *    last updated time each provisioning data : condition which provisioning data will be removed
 *    etc.
 */
public class ConfigContainer {
    private static final String LOCAL_FILE_NAME_PREFIX = "rcs_provisioning_config_";
    private static final String LOCAL_FILE_NAME_POSTFIX = ".xml";
    private static final String LOCAL_KEY_AC_VERSION = "version";
    private static final String LOCAL_KEY_AC_VALIDITY = "validity";
    private static final String LOCAL_KEY_AC_TOKEN = "token";

    private static final String LOCAL_KEY_RCS_VERSION = "key_rcs_version";
    private static final String LOCAL_KEY_RCS_PROFILE = "key_rcs_profile";
    private static final String LOCAL_KEY_CLIENT_VENDOR = "key_rcs_client_vendor";
    private static final String LOCAL_KEY_CLIENT_VERSION = "key_rcs_client_version";
    private static final String LOCAL_KEY_RCS_ENABLED_BY_USER = "key_rcs_enabled_by_user";
    private static final String LOCAL_KEY_VALIDATION_TIME = "key_validation_time";
    private static final String LOCAL_KEY_LAST_UPDATE_TIME = "key_last_updated_time";

    private final Context mContext;
    private final String mFileName;
    private final int mSlotId;
    private PersistableBundle mConfig;

    /**
     * create instance
     * @param context Context to access file
     * @param slotId SIM slotId or PhoneId
     * @param subId subscribe Id
     */
    public ConfigContainer(Context context, int slotId, int subId) {
        mContext = context;
        mFileName = LOCAL_FILE_NAME_PREFIX + subId + LOCAL_FILE_NAME_POSTFIX;
        mSlotId = slotId;

        readDataFromFile();
    }

    /**
     * get version value in provisioning data
     * @param defaultValue if to get from storage is fail then this value will be return
     * @return version value
     */
    public int getAcVersion(int defaultValue) {
        return mConfig.getInt(LOCAL_KEY_AC_VERSION, defaultValue);
    }

    /**
     * get validity value in provisioning data
     * @return validity value which is stored or 0L
     */
    public long getAcValidity() {
        return mConfig.getLong(LOCAL_KEY_AC_VALIDITY);
    }

    /**
     * get token value in provisioning data
     * @return token value which is stored or null
     */
    public String getAcToken() {
        return mConfig.getString(LOCAL_KEY_AC_TOKEN);
    }

    /**
     * update new version and validity value
     * @param newVersion version
     * @param newValidity validity
     */
    public void updateAcVersionValidity(int newVersion, long newValidity) {
        ImsLog.i(mSlotId, "old version : " + getAcVersion(0)
                + " new version : " + newVersion);

        ImsLog.i(mSlotId, "old validity : " + getAcValidity() + " new validity : " + newValidity);

        mConfig.putInt(LOCAL_KEY_AC_VERSION, newVersion);
        mConfig.putLong(LOCAL_KEY_AC_VALIDITY, newValidity);

        saveDataToFile();
    }

    /**
     * update new token value
     * @param newToken token
     */
    public void updateAcToken(String newToken) {
        ImsLog.i(mSlotId, "old token : " + getAcToken() + " new token : " + newToken);

        mConfig.putString(LOCAL_KEY_AC_TOKEN, newToken);

        saveDataToFile();
    }

    /**
     * reset the stored version, validity and token
     */
    public void resetAcValue() {
        mConfig.putInt(LOCAL_KEY_AC_VERSION, 0);
        mConfig.putLong(LOCAL_KEY_AC_VALIDITY, 0L);
        mConfig.putString(LOCAL_KEY_AC_TOKEN, null);
        mConfig.putLong(LOCAL_KEY_LAST_UPDATE_TIME, 0L);

        saveDataToFile();
    }

    /**
     * update AcServiceClientInfo
     * @param clientInfo container has client information
     */
    public void updateClientInfo(AcServiceClientInfo clientInfo) {
        mConfig.putString(LOCAL_KEY_RCS_VERSION, clientInfo.getRcsVersion());
        mConfig.putString(LOCAL_KEY_RCS_PROFILE, clientInfo.getRcsProfile());
        mConfig.putString(LOCAL_KEY_CLIENT_VENDOR, clientInfo.getClientVendor());
        mConfig.putString(LOCAL_KEY_CLIENT_VERSION, clientInfo.getClientVersion());
        mConfig.putBoolean(LOCAL_KEY_RCS_ENABLED_BY_USER, clientInfo.isRcsEnabledByUser());

        saveDataToFile();
    }

    /**
     * get AcServiceClientInfo
     * @return container has client information corresponding to subId
     */
    public AcServiceClientInfo getClientInfo() {
        String rcsVersion = mConfig.getString(LOCAL_KEY_RCS_VERSION);
        String rcsProfile = mConfig.getString(LOCAL_KEY_RCS_PROFILE);
        String clientVendor = mConfig.getString(LOCAL_KEY_CLIENT_VENDOR);
        String clientVersion = mConfig.getString(LOCAL_KEY_CLIENT_VERSION);

        if (TextUtils.isEmpty(rcsVersion) || TextUtils.isEmpty(rcsProfile)
                || TextUtils.isEmpty(clientVendor) || TextUtils.isEmpty(clientVersion)) {
            ImsLog.i(mSlotId, "not exist AcServiceClientInfo");
            return null;
        }

        return new AcServiceClientInfo(rcsVersion, rcsProfile, clientVendor, clientVersion,
                mConfig.getBoolean(LOCAL_KEY_RCS_ENABLED_BY_USER));
    }

    /**
     * reset the stored client info data
     */
    public void resetClientInfo() {
        mConfig.putString(LOCAL_KEY_RCS_VERSION, null);
        mConfig.putString(LOCAL_KEY_RCS_PROFILE, null);
        mConfig.putString(LOCAL_KEY_CLIENT_VENDOR, null);
        mConfig.putString(LOCAL_KEY_CLIENT_VERSION, null);
        mConfig.putBoolean(LOCAL_KEY_RCS_ENABLED_BY_USER, false);

        saveDataToFile();
    }

    /**
     * update validation time
     * @param time validation time
     */
    public void updateValidationTime(long time) {
        mConfig.putLong(LOCAL_KEY_VALIDATION_TIME, time);

        saveDataToFile();
    }

    /**
     * get validation time
     * @param defaultValue if to get value from stored data is failed, default value will be return
     * @return validation time corresponding to subId, otherwise 0L
     */
    public long getValidationTime(long defaultValue) {
        return mConfig.getLong(LOCAL_KEY_VALIDATION_TIME, defaultValue);
    }

    /**
     * set last update time as now
     */
    public void updateLastUpdatedTime() {
        mConfig.putLong(LOCAL_KEY_LAST_UPDATE_TIME, getCurrentTimeMillis());

        saveDataToFile();
    }

    /**
     * get last updated time for subId
     * @param defaultValue if to get value from stored data is failed, default value will be return
     * @return last updated time corresponding to subId, otherwise 0L
     */
    public long getLastUpdatedTime(long defaultValue) {
        return mConfig.getLong(LOCAL_KEY_LAST_UPDATE_TIME, defaultValue);
    }

    @VisibleForTesting
    public String getFileName() {
        return mFileName;
    }

    @VisibleForTesting
    protected long getCurrentTimeMillis() {
        return System.currentTimeMillis();
    }

    private void readDataFromFile() {
        try {
            File file = new File(mContext.getFilesDir(), mFileName);
            if (file.exists()) {
                FileInputStream inputStream = new FileInputStream(file);
                mConfig = PersistableBundle.readFromStream(inputStream);
                inputStream.close();
                ImsLog.i("read config from file : " + mConfig.toString());
            } else {
                mConfig = new PersistableBundle();
                ImsLog.i("file is not exist");
            }
        } catch (Exception e) {
            ImsLog.e(e.getMessage());
            mConfig = new PersistableBundle();
        }
    }

    private void saveDataToFile() {
        try {
            File file = new File(mContext.getFilesDir(), mFileName);
            FileOutputStream outputStream = new FileOutputStream(file);
            mConfig.writeToStream(outputStream);
            outputStream.close();
            ImsLog.i("save data to file : " + mConfig.toString());
        } catch (Exception e) {
            ImsLog.e(e.getMessage());
        }
    }
}
