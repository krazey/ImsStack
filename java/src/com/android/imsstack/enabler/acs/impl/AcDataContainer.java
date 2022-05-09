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

import android.annotation.Nullable;
import android.content.Context;
import android.os.PersistableBundle;
import android.util.Log;

import java.io.ByteArrayInputStream;
import java.io.File;
import java.io.FileInputStream;
import java.io.FileNotFoundException;
import java.io.FileOutputStream;
import java.io.IOException;
import java.util.Set;

/**
 * This class handles the RCS provisioning XML data and is created based on sub ID
 */
public class AcDataContainer {
    public  static final int VERSION_UNKNOWN = -99;
    public  static final long VALIDITY_UNKNOWN = -99l;

    private static final String TAG = AcDataContainer.class.getSimpleName();
    private static final String LOCAL_FILE_NAME_PREF = "rcs_provisioning_data_";
    private static final String VERSION = "version";
    private static final String VALIDITY = "validity";

    private final Context mContext;
    private final int mSubId;
    private final String mFileName;

    private PersistableBundle mProvisioningData;

    public AcDataContainer(Context context, int subId) {
        mContext = context;
        mSubId = subId;

        mFileName = LOCAL_FILE_NAME_PREF + subId + ".xml";
        readDataFromFile();
    }

    /**
     * Get copied provisioning data
     * @return PersistableBundle includes provisioning data, but if the internal provisioning data
     * is not valid then the null is returned.
     */
    public @Nullable PersistableBundle getProvisioningData() {
        if (mProvisioningData != null && !mProvisioningData.isEmpty()) {
            return new PersistableBundle(mProvisioningData);
        }

        loge("getProvisioningData : but provisioning data is empty.");

        return null;
    }

    /**
     * Set provisioning data
     */
    public void setProvisioningData(String data) {
        // TODO
        // check whole or partial data
        mProvisioningData = createPersistableBundleFromString(data);
        if (mProvisioningData != null) {
            saveDataToFile();
        } else {
            log("setProvisioningData : creating failed");
        }
    }

    /**
     * notify the stored provisioning data is not valid
     */
    public void resetProvisioningData() {
        mProvisioningData.clear();
    }

    /**
     * get version value in provisioning data
     * @return version value
     */
    public int getVersion() {
        return getVersion(mProvisioningData);
    }

    /**
     * get validity value in provisioning data
     * @return validity value
     */
    public long getValidity() {
        return getValidity(mProvisioningData);
    }

    /**
     * update new version and validity value in provisioning data
     * @param data xml data includes version and validity value
     * @return true if the operation is success, or false otherwise
     */
    public boolean updateVersionValidity(String data) {
        if (!isValidProvisioning()) {
            log("updateVersionValidity : provisioning data is empty");
            return false;
        }

        // create new PersistableBundle from string
        PersistableBundle newProvisioningData = createPersistableBundleFromString(data);
        if (newProvisioningData == null) {
            log("updateVersionValidity : create PersistableBundle failed from String");
            return false;
        }

        // get version validity from new PersistableBundle
        int version = getVersion(newProvisioningData);
        long validity = getValidity(newProvisioningData);
        if (version != VERSION_UNKNOWN && validity != VALIDITY_UNKNOWN) {
            // update version at exist Provisioning data
            log("updateVersionValidity : " + version + " " + validity);
            return setVersionValidity(version, validity);
        }

        log("updateVersionValidity : update version failed");
        return false;
    }

    /**
     * Get this object has valid provisioning data
     * @return true if object has valid provisioning, or false otherwise.
     */
    public boolean isValidProvisioning() {
        return !(mProvisioningData == null || mProvisioningData.isEmpty());
    }

    /**
     * Get file name which stored the provisioning data
     * @return file name
     */
    public String getFileName() {
        return mFileName;
    }

    private void readDataFromFile() {
        // load xml data from FS based on subId
        try {
            File file = new File(mContext.getFilesDir(), mFileName);
            FileInputStream inputStream = new FileInputStream(file);
            mProvisioningData = PersistableBundle.readFromStream(inputStream);
            inputStream.close();
        } catch (FileNotFoundException e) {
            log("readDataFromFile" + e.getMessage());
        } catch (IOException e) {
            log("readDataFromFile" + e.getMessage());
        }
    }

    private void saveDataToFile() {
        if (!isValidProvisioning()) {
            log("saveDataToFile : provisioning data is empty");
            return;
        }

        try {
            File file = new File(mContext.getFilesDir(), mFileName);
            FileOutputStream outputStream = new FileOutputStream(file);
            mProvisioningData.writeToStream(outputStream);
            outputStream.close();
            log("saveDataToFile : " + mFileName);
        } catch (IOException e) {
            log("saveDataToFile : " + e.getMessage());
        }
    }

    private PersistableBundle createPersistableBundleFromString(String in) {
        try {
            // create PersistableBundle from String data
            ByteArrayInputStream inputStream = new ByteArrayInputStream(in.getBytes());
            PersistableBundle out = PersistableBundle.readFromStream(inputStream);
            inputStream.close();
            return out;
        } catch (IOException e) {
            loge("createPersistableBundleFromString" + e.getMessage());
        }

        return null;
    }

    private int getVersion(PersistableBundle data) {
        Set<String> keys = data.keySet();
        for (String key : keys) {
            PersistableBundle sub = data.getPersistableBundle(key);
            int version = sub.getInt(VERSION, VERSION_UNKNOWN);

            // default value means not find appropriated PersistableBundle
            if (version != VERSION_UNKNOWN) {
                return version;
            }
        }

        return VERSION_UNKNOWN;
    }

    private long getValidity(PersistableBundle data) {
        Set<String> keys = data.keySet();
        for (String key : keys) {
            PersistableBundle sub = data.getPersistableBundle(key);
            long validity = sub.getLong(VERSION, VALIDITY_UNKNOWN);

            // default value means not find appropriated PersistableBundle
            if (validity != VALIDITY_UNKNOWN) {
                return validity;
            }
        }

        return VALIDITY_UNKNOWN;
    }

    private boolean setVersionValidity(int newVersion, long newValidity) {
        Set<String> keys = mProvisioningData.keySet();
        for (String key : keys) {
            PersistableBundle sub = mProvisioningData.getPersistableBundle(key);
            long validity = sub.getLong(VERSION, VALIDITY_UNKNOWN);

            // not equal default value means find appropriated PersistableBundle
            if (validity != VALIDITY_UNKNOWN) {
                sub.putInt(VERSION, newVersion);
                sub.putLong(VALIDITY, newValidity);

                saveDataToFile();

                return true;
            }
        }

        return false;
    }

    private void log(String msg) {
        Log.d(TAG, "[" + mSubId + "] " + msg);
    }

    private void loge(String msg) {
        Log.e(TAG, "[" + mSubId + "] " + msg);
    }
}
