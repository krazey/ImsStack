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
    private static final String LOCAL_FILE_NAME = "rcs_provisioning_config.xml";
    private static final String LOCAL_KEY_PREFIX = "key_";
    private static final String LOCAL_KEY_RCS_VERSION = "key_rcs_version";
    private static final String LOCAL_KEY_RCS_PROFILE = "key_rcs_profile";
    private static final String LOCAL_KEY_CLIENT_VENDOR = "key_rcs_client_vendor";
    private static final String LOCAL_KEY_CLIENT_VERSION = "key_rcs_client_version";
    private static final String LOCAL_KEY_RCS_ENABLED_BY_USER = "key_rcs_enabled_by_user";
    private static final String LOCAL_KEY_VALIDATION_TIME = "key_validation_time";
    private static final String LOCAL_KEY_LAST_UPDATE_TIME = "key_last_updated_time";

    private static ConfigContainer sInstance;

    private final Context mContext;

    private PersistableBundle mConfig;

    @VisibleForTesting
    public ConfigContainer(Context context) {
        mContext = context;
        readDataFromFile();
    }

    /**
     * get the instance
     */
    public static ConfigContainer getInstance(Context context) {
        if (sInstance == null) {
            sInstance = new ConfigContainer(context);
        }

        return sInstance;
    }

    /**
     * update AcServiceClientInfo for subId
     * @param subId subscribe Id
     * @param clientInfo container has client information
     */
    public void updateClientInfo(int subId, AcServiceClientInfo clientInfo) {
        String key = LOCAL_KEY_PREFIX + subId;

        PersistableBundle pb = getDataForSubscription(key);
        pb.putString(LOCAL_KEY_RCS_VERSION, clientInfo.getRcsVersion());
        pb.putString(LOCAL_KEY_RCS_PROFILE, clientInfo.getRcsProfile());
        pb.putString(LOCAL_KEY_CLIENT_VENDOR, clientInfo.getClientVendor());
        pb.putString(LOCAL_KEY_CLIENT_VERSION, clientInfo.getClientVersion());
        pb.putBoolean(LOCAL_KEY_RCS_ENABLED_BY_USER, clientInfo.isRcsEnabledByUser());

        ImsLog.i(pb.toString());

        saveDataToFile();
    }

    /**
     * get AcServiceClientInfo for subId
     * @param subId subscribe Id
     * @return container has client information corresponding to subId
     */
    public AcServiceClientInfo getClientInfo(int subId) {
        String key = LOCAL_KEY_PREFIX + subId;

        PersistableBundle pb = getDataForSubscription(key);
        return new AcServiceClientInfo(
                pb.getString(LOCAL_KEY_RCS_VERSION),
                pb.getString(LOCAL_KEY_RCS_PROFILE),
                pb.getString(LOCAL_KEY_CLIENT_VENDOR),
                pb.getString(LOCAL_KEY_CLIENT_VERSION),
                pb.getBoolean(LOCAL_KEY_RCS_ENABLED_BY_USER));
    }

    /**
     * update validation time for subId
     * @param subId subscribe Id
     * @param time validation time
     */
    public void updateValidationTime(int subId, long time) {
        String key = LOCAL_KEY_PREFIX + subId;

        PersistableBundle pb = getDataForSubscription(key);
        pb.putLong(LOCAL_KEY_VALIDATION_TIME, time);

        saveDataToFile();
    }

    /**
     * get validation time for subId
     * @param subId subscribe Id
     * @return validation time corresponding to subId, otherwise 0L
     */
    public Long getValidationTime(int subId) {
        String key = LOCAL_KEY_PREFIX + subId;

        PersistableBundle pb = getDataForSubscription(key);

        return pb.getLong(LOCAL_KEY_VALIDATION_TIME);
    }

    /**
     * set last update time as now for subId
     * @param subId subscribe Id
     */
    public void updateLastUpdatedTime(int subId) {
        String key = LOCAL_KEY_PREFIX + subId;

        PersistableBundle pb = getDataForSubscription(key);
        pb.putLong(LOCAL_KEY_LAST_UPDATE_TIME, getCurrentTimeMillis());

        saveDataToFile();
    }

    /**
     * get last updated time for subId
     * @param subId subscribe Id
     * @return last updated time corresponding to subId, otherwise 0L
     */
    public Long getLastUpdatedTime(int subId) {
        String key = LOCAL_KEY_PREFIX + subId;

        PersistableBundle pb = getDataForSubscription(key);

        return pb.getLong(LOCAL_KEY_LAST_UPDATE_TIME);
    }

    /*
     * Returns associated data
     * if data is not exist, create, put and return
     */
    private PersistableBundle getDataForSubscription(String key) {
        PersistableBundle pb = mConfig.getPersistableBundle(key);
        if (pb == null) {
            ImsLog.i("pb is null");
            pb = new PersistableBundle();
            mConfig.putPersistableBundle(key, pb);
        }
        return pb;
    }

    @VisibleForTesting
    protected long getCurrentTimeMillis() {
        return System.currentTimeMillis();
    }

    private void readDataFromFile() {
        try {
            File file = new File(mContext.getFilesDir(), LOCAL_FILE_NAME);
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
        }
    }

    private void saveDataToFile() {
        try {
            File file = new File(mContext.getFilesDir(), LOCAL_FILE_NAME);
            FileOutputStream outputStream = new FileOutputStream(file);
            mConfig.writeToStream(outputStream);
            outputStream.close();
            ImsLog.i("save data to file : " + mConfig.toString());
        } catch (Exception e) {
            ImsLog.e(e.getMessage());
        }
    }
}
