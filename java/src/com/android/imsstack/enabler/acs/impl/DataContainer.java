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
import android.util.ArrayMap;
import android.util.Xml;

import com.android.imsstack.util.ImsLog;
import com.android.internal.annotations.VisibleForTesting;

import org.xmlpull.v1.XmlPullParser;
import org.xmlpull.v1.XmlPullParserException;

import java.io.ByteArrayInputStream;
import java.io.File;
import java.io.FileInputStream;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.nio.charset.StandardCharsets;
import java.util.ArrayList;
import java.util.List;
import java.util.Map;

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
        readDataFromFile();
    }

    /**
     * Get copied provisioning data
     * @return PersistableBundle includes provisioning data, but if the internal provisioning data
     * is not valid then the null is returned.
     */
    public @Nullable String getProvisioningData() {
        if (mPersistableBundle != null && !mPersistableBundle.isEmpty()) {
            return mPersistableBundle.getString(KEY_DATA);
        }

        ImsLog.e("[" + mSlotId + "] " + "provisioning data is empty.");
        return null;
    }

    /**
     * Set provisioning data
     */
    public void setProvisioningData(String data) {
        if (mPersistableBundle == null) {
            mPersistableBundle = new PersistableBundle();
        }
        mPersistableBundle.putString(KEY_DATA, data);

        // TODO : parsing version and validity
        int newVersion = 0;
        long newValidity = 0L;
        String newToken = "";

        try {
            List xmlData = parse(new ByteArrayInputStream(data.getBytes(StandardCharsets.UTF_8)));
            DataContainer.Characteristic c;
            for (Object obj : xmlData) {
                c = (DataContainer.Characteristic) obj;
                ImsLog.i(c.mType);
                ImsLog.i(c.mParms.toString());
            }
            newVersion = getIntFromXml(xmlData, KEY_VERSION, VERSION_UNKNOWN);
            newValidity = getLongFromXml(xmlData, KEY_VALIDITY, VALIDITY_UNKNOWN);
            newToken = getStringFromXml(xmlData, KEY_TOKEN, TOKEN_UNKNOWN);
        } catch (XmlPullParserException | IOException e) {
            ImsLog.e(e.getMessage());
            return;
        }

        ImsLog.i("[" + mSlotId + "] " + "version : " + newVersion
                + " validity : " + newValidity
                + " token : " + newToken);

        mPersistableBundle.putInt(KEY_VERSION, newVersion);
        mPersistableBundle.putLong(KEY_VALIDITY, newValidity);
        mPersistableBundle.putString(KEY_TOKEN, newToken);

        saveDataToFile();
    }

    /**
     * notify the stored provisioning data is not valid
     */
    public void resetProvisioningData() {
        if (mPersistableBundle != null) {
            mPersistableBundle.putString(KEY_DATA, null);
            mPersistableBundle.putInt(KEY_VERSION, VERSION_UNKNOWN);
            mPersistableBundle.putLong(KEY_VALIDITY, VALIDITY_UNKNOWN);

            saveDataToFile();
        }
    }

    /**
     * get version value in provisioning data
     * @return version value
     */
    public int getVersion() {
        if (mPersistableBundle == null) {
            ImsLog.i("provisioning data is not available");
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
            ImsLog.i("provisioning data is not available");
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
     * update new version and validity value in provisioning data
     * @param data xml data includes version and validity value
     * @return true if the operation is success, or false otherwise
     */
    public boolean updateVersionValidity(String data) {
        if (!isValidProvisioning()) {
            ImsLog.i("[" + mSlotId + "] " + "provisioning data is not available");
            return false;
        }

        // TODO : parsing new version and validity
        int newVersion = 0;
        long newValidity = 0L;

        ImsLog.i("[" + mSlotId + "] " + "version : " + newVersion
                + " validity : " + newValidity);

        mPersistableBundle.putInt(KEY_VERSION, newVersion);
        mPersistableBundle.putLong(KEY_VALIDITY, newValidity);

        saveDataToFile();

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

    private void readDataFromFile() {
        // load xml data from FS based on subId
        try {
            File file = new File(mContext.getFilesDir(), mFileName);
            FileInputStream inputStream = new FileInputStream(file);
            mPersistableBundle = PersistableBundle.readFromStream(inputStream);
            inputStream.close();
        } catch (IOException e) {
            ImsLog.e("[" + mSlotId + "] " + e.getMessage());
            mPersistableBundle = null;
        }
    }

    private void saveDataToFile() {
        if (!isValidProvisioning()) {
            ImsLog.i("[" + mSlotId + "] " + "provisioning data is empty");
            return;
        }

        try {
            File file = new File(mContext.getFilesDir(), mFileName);
            FileOutputStream outputStream = new FileOutputStream(file);
            mPersistableBundle.writeToStream(outputStream);
            outputStream.close();
            ImsLog.d("[" + mSlotId + "] " + "file name : " + mFileName);
        } catch (IOException e) {
            ImsLog.e("[" + mSlotId + "] " + e.getMessage());
        }
    }

    private int getIntFromXml(List list, String key, int defaultValue) {
        int ret = defaultValue;
        Characteristic c;

        for (Object obj : list) {
            c = (Characteristic) obj;
            if (c == null) {
                continue;
            }
            if (c.hasKey(key)) {
                ImsLog.i("matched key");
                String value = c.getValue(key);
                try {
                    ret = Integer.parseInt(value);
                } catch (Exception e) {
                    ImsLog.i(e.getMessage());
                }
            }
        }

        return ret;
    }

    private long getLongFromXml(List list, String key, long defaultValue) {
        long ret = defaultValue;
        Characteristic c;

        for (Object obj : list) {
            c = (Characteristic) obj;
            if (c == null) {
                continue;
            }
            if (c.hasKey(key)) {
                ImsLog.i("matched key");
                String value = c.getValue(key);
                try {
                    ret = Long.parseLong(value);
                } catch (Exception e) {
                    ImsLog.i(e.getMessage());
                }
            }
        }

        return ret;
    }

    private String getStringFromXml(List list, String key, String defaultValue) {
        Characteristic c;
        for (Object obj : list) {
            c = (Characteristic) obj;
            if (c == null) {
                continue;
            }
            if (c.hasKey(key)) {
                ImsLog.i("matched key");
                return c.getValue(key);
            }
        }
        return defaultValue;
    }

    @VisibleForTesting
    private List parse(InputStream in) throws XmlPullParserException, IOException {
        try {
            XmlPullParser parser = Xml.newPullParser();
            parser.setFeature(XmlPullParser.FEATURE_PROCESS_NAMESPACES, false);
            parser.setInput(in, null);
            parser.nextTag();
            return readProvisioningDoc(parser);
        } finally {
            in.close();
        }
    }

    private List readProvisioningDoc(XmlPullParser parser)
            throws XmlPullParserException, IOException {
        boolean endVers = false;
        boolean endToken = false;
        List characteristics = new ArrayList();

        parser.require(XmlPullParser.START_TAG, null, "wap-provisioningdoc");
        while (parser.next() != XmlPullParser.END_TAG) {
            if (parser.getEventType() != XmlPullParser.START_TAG) {
                continue;
            }
            String name = parser.getName();
            if (name.equals("characteristic")) {
                Characteristic characteristic = readCharacteristic(parser);
                characteristics.add(characteristic);
                if (!endVers && "VERS".equals(characteristic.mType)) {
                    endVers = true;
                }
                if (!endToken && "TOKEN".equals(characteristic.mType)) {
                    endToken = true;
                }
            } else {
                skip(parser);
            }

            if (endVers && endToken) {
                break;
            }
        }

        return characteristics;
    }

    private void skip(XmlPullParser parser) throws XmlPullParserException, IOException {
        if (parser.getEventType() != XmlPullParser.START_TAG) {
            throw new IllegalStateException();
        }
        int depth = 1;
        while (depth != 0) {
            switch (parser.next()) {
                case XmlPullParser.END_TAG:
                    depth--;
                    break;
                case XmlPullParser.START_TAG:
                    depth++;
                    break;
            }
        }
    }

    private Characteristic readCharacteristic(XmlPullParser parser)
            throws XmlPullParserException, IOException {
        // TODO : check this, do we need to call this?
        parser.require(XmlPullParser.START_TAG, null, "characteristic");

        int count = parser.getAttributeCount();
        String type = null;
        Characteristic current = null;

        if (count > 0) {
            for (int i = 0; i < count; i++) {
                String name = parser.getAttributeName(i);
                if ("type".equals(name)) {
                    type = parser.getAttributeValue(i);
                    current = new Characteristic(type);
                }
            }
        }
        int ii = 0;
        int endTag = parser.next();
        String endName = parser.getName();
        ImsLog.i("endTag " + endTag);
        ImsLog.i("name " + endName);
        while (!(endTag == XmlPullParser.END_TAG && endName.equals("characteristic"))) {
            ImsLog.i("loop " + ii++);
            if (parser.getEventType() == XmlPullParser.START_TAG) {
                String tag = parser.getName();
                if ("parm".equals(tag)) {
                    count = parser.getAttributeCount();
                    String key = null;
                    String value = null;
                    if (count > 1) {
                        for (int i = 0; i < count; i++) {
                            String name = parser.getAttributeName(i);
                            if ("name".equals(name)) {
                                key = parser.getAttributeValue(i);
                            } else if ("value".equals(name)) {
                                value = parser.getAttributeValue(i);
                            }
                        }
                    }
                    if (current != null && key != null && value != null) {
                        current.putParm(key, value);
                    }
                }
            }

            endTag = parser.next();
            endName = parser.getName();
            ImsLog.i("endTag " + endTag);
            ImsLog.i("name " + endName);
        }
        parser.require(XmlPullParser.END_TAG, null, "characteristic");

        return current;
    }

    /**
     * This class handles each characteristic block in RCS provisioning XML data
     */
    public static class Characteristic {
        public final String mType;
        public final Map<String, String> mParms = new ArrayMap<>();

        private Characteristic(String type) {
            mType = type;
        }

        private void putParm(String key, String value) {
            mParms.put(key, value);
        }

        private String getValue(String key) {
            String value = mParms.get(key);
            ImsLog.i("read : " + value);
            return value;
        }

        private boolean hasKey(String key) {
            return mParms.containsKey(key);
        }
    }
}
