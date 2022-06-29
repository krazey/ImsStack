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

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.os.PersistableBundle;
import android.telephony.CarrierConfigManager;
import android.telephony.TelephonyManager;
import android.text.TextUtils;

import com.android.imsstack.core.config.CarrierConfig;
import com.android.imsstack.core.config.ConfigXmlUtils;
import com.android.imsstack.util.AppContext;
import com.android.imsstack.util.ImsLog;
import com.android.imsstack.util.ImsPrivateProperties;
import com.android.imsstack.util.IoUtils;
import com.android.imsstack.util.SimCarrierId;

import org.xmlpull.v1.XmlPullParser;
import org.xmlpull.v1.XmlPullParserException;
import org.xmlpull.v1.XmlPullParserFactory;

import java.io.FileNotFoundException;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.util.Arrays;
import java.util.regex.Matcher;
import java.util.regex.Pattern;

public class ConfigAgent implements ConfigInterface {
    private static final String CARRIER_ID_PREFIX = "carrier_config_carrierid_";
    private static final String MCC_MNC_PREFIX = "carrier_config_mccmnc_";

    /** Intent for testing purpose. */
    private static final String ACTION_TEST_CARRIER_CONFIG_PUT =
            "com.android.imsstack.TEST_CARRIER_CONFIG_PUT";
    private static final String ACTION_TEST_CARRIER_CONFIG_APPLY =
            "com.android.imsstack.TEST_CARRIER_CONFIG_APPLY";
    /** Extra parameters for ACTION_TEST_CARRIER_CONFIG_PUT */
    private static final String KEY_NAME = "name";
    private static final String KEY_VALUE = "value";

    private final int mSlotId;
    private final CarrierConfig mCarrierConfig;
    private final IntentReceiver mIntentReceiver;
    private PersistableBundle mDefaultImsConfig;
    private PersistableBundle mTestConfig;
    private XmlPullParserFactory mFactory;

    public ConfigAgent(int slotId) {
        mSlotId = slotId;
        mCarrierConfig = new CarrierConfig();
        mIntentReceiver = new IntentReceiver();
    }

    @Override
    public void init(Context context) {
        mDefaultImsConfig = loadCarrierConfigFromXml(
                CarrierConfig.DEFAULT_CARRIER_CONFIG_FILE, null);
        mIntentReceiver.register();
    }

    @Override
    public void cleanup() {
        mIntentReceiver.unregister();
    }

    @Override
    public CarrierConfig getCarrierConfig() {
        return mCarrierConfig;
    }

    @Override
    public PersistableBundle readTestConfig() {
        if (mTestConfig != null) {
            return mTestConfig;
        }

        InputStream is = null;

        try {
            is = AppContext.get().openFileInput(CarrierConfig.TEST_CARRIER_CONFIG_FILE);
            mTestConfig = PersistableBundle.readFromStream(is);
        } catch (FileNotFoundException e) {
            ImsLog.d(mSlotId, "readTestConfig: not found");
            mTestConfig = new PersistableBundle();
        } catch (IOException e) {
            ImsLog.d(mSlotId, "readTestConfig: " + e.toString());
            mTestConfig = new PersistableBundle();
        } finally {
            IoUtils.closeQuietly(is);
        }

        return mTestConfig;
    }

    @Override
    public boolean writeTestConfig(PersistableBundle config) {
        if (config == null) {
            return false;
        }

        AppContext.get().deleteFile(CarrierConfig.TEST_CARRIER_CONFIG_FILE);

        if (!config.isEmpty()) {
            OutputStream os = null;

            try {
                os = AppContext.get().openFileOutput(
                        CarrierConfig.TEST_CARRIER_CONFIG_FILE,
                        Context.MODE_APPEND);
                config.writeToStream(os);
                ImsLog.d(mSlotId, "writeTestConfig: Ok");
                return true;
            } catch (IOException e) {
                ImsLog.d(mSlotId, "writeTestConfig: " + e.toString());
            } finally {
                IoUtils.closeQuietly(os);
            }

            return false;
        } else {
            return true;
        }
    }

    public void updateCarrierConfig(int subId, SimCarrierId id) {
        // Loads IMS carrier configuration from CarrierConfigManager.
        PersistableBundle config = getCarrierConfig(subId);

        // Sets a default configuration from assets.
        config.putAll(mDefaultImsConfig);

        // Loads IMS specific carrier configuration from asset.
        PersistableBundle configFromAsset = loadCarrierConfig(subId, id);

        config.putAll(configFromAsset);

        ImsLog.d(mSlotId, "updateCarrierConfig: " + config.toString());

        // After debugging, let's remove this because it uses a deprecated API. {
        java.util.Set<String> keys = config.keySet();

        for (String key : keys) {
            ImsLog.d(mSlotId, "carrier-config: " + key + "=" + config.get(key));
        }
        // }

        overrideTestConfigs(config);

        mCarrierConfig.setConfig(config, mSlotId);
    }

    private void overrideTestConfigs(PersistableBundle config) {
        ImsLog.i(mSlotId, "overrideTestConfigs...");

        boolean usePredefinedUaString = ImsPrivateProperties.Persistent.getBoolean(
                ImsPrivateProperties.Persistent.KEY_USE_PREDEFINED_UA_STRING, false, mSlotId);

        if (usePredefinedUaString) {
            String uaString = ImsPrivateProperties.Persistent.get(
                    ImsPrivateProperties.Persistent.KEY_CONFIG_UA_STRING, mSlotId);

            if (!TextUtils.isEmpty(uaString)) {
                ImsLog.d(mSlotId, "Use predefined UA string: " + uaString);
                config.putString(CarrierConfigManager.Ims.KEY_IMS_USER_AGENT_STRING, uaString);
            }
        }

        // test-carrier-config
        PersistableBundle testConfig = readTestConfig();

        if (!testConfig.isEmpty()) {
            config.putAll(testConfig);
        }
    }

    private PersistableBundle getCarrierConfig(int subId) {
        CarrierConfigManager ccm = AppContext.getSystemService(CarrierConfigManager.class);
        PersistableBundle configs = new PersistableBundle();

        if (ccm != null) {
            // If an invalid subId is used, this bundle will contain default values.
            PersistableBundle b = ccm.getConfigForSubId(subId);

            if (b != null) {
                // IMS common configuration from all the carrier configs.
                for (String key : CarrierConfig.IMS_COMMON_KEYS) {
                    addConfig(key, b, configs);
                }

                // IMS configuration with the IMS prefix from all the carrier configs.
                for (String key : b.keySet()) {
                    if (isKeyForIms(key)) {
                        addConfig(key, b, configs);
                    }
                }
            }
        }

        return configs;
    }

    private PersistableBundle loadCarrierConfig(int subId, SimCarrierId id) {
        String fileName = getCarrierConfigFile(subId, id);

        if (TextUtils.isEmpty(fileName)) {
            ImsLog.d(mSlotId, "loadCarrierConfig: No matched carrier configuration.");
            return new PersistableBundle();
        }

        ImsLog.d(mSlotId, "loadCarrierConfigFromXml: " + fileName);

        return loadCarrierConfigFromXml(fileName, id);
    }

    private PersistableBundle loadCarrierConfigFromXml(String fileName, SimCarrierId id) {
        InputStream is = null;

        try {
            is = AppContext.get().getAssets().open(fileName);

            synchronized (this) {
                if (mFactory == null) {
                    mFactory = XmlPullParserFactory.newInstance();
                }
            }

            XmlPullParser parser = mFactory.newPullParser();
            parser.setInput(is, "utf-8");

            return readConfigFromXml(parser, id);
        } catch (IOException | XmlPullParserException e) {
            ImsLog.e(mSlotId, "loadCarrierConfigFromXml: " + e.toString());
            return new PersistableBundle();
        } finally {
            IoUtils.closeQuietly(is);
        }
    }

    private String getCarrierConfigFile(int subId, SimCarrierId id) {
        String fileName = null;

        if (id.getCarrierId() == SimCarrierId.UNKNOWN_ID) {
            if (TextUtils.isEmpty(id.getMcc()) || TextUtils.isEmpty(id.getMnc())) {
                // A default configuration will be always loaded.
                // So, this needs to be returned as null.
            } else {
                fileName = MCC_MNC_PREFIX + id.getMcc() + id.getMnc() + ".xml";
            }
        } else {
            String[] files = null;

            try {
                files = AppContext.get().getAssets().list(CarrierConfig.CARRIER_CONFIG);
            } catch (IOException e) {
                ImsLog.e(mSlotId, "getCarrierConfigFile: " + e);
                return null;
            }

            TelephonyManager tm = AppContext.getTelephonyManager(subId);
            int mccMncCarrierId = (tm != null) ? tm.getCarrierIdFromSimMccMnc() : 0;
            int testCarrierId = ImsPrivateProperties.Persistent.getInt(
                    ImsPrivateProperties.Persistent.KEY_TEST_CARRIER_ID, mSlotId);
            int specificCarrierId = (testCarrierId > 0) ?
                    testCarrierId : id.getSpecificCarrierId();
            String fileNameForSpecificCarrierId = null;
            String fileNameForCarrierId = null;
            String fileNameForMccMncCarrierId = null;

            for (String file : files) {
                if (file.startsWith(CARRIER_ID_PREFIX + specificCarrierId + "_")) {
                    fileNameForSpecificCarrierId = file;
                    break;
                } else if (file.startsWith(CARRIER_ID_PREFIX + id.getCarrierId() + "_")) {
                    fileNameForCarrierId = file;
                } else if (file.startsWith(CARRIER_ID_PREFIX + mccMncCarrierId + "_")) {
                    fileNameForMccMncCarrierId = file;
                }
            }

            if (!TextUtils.isEmpty(fileNameForSpecificCarrierId)) {
                fileName = fileNameForSpecificCarrierId;
            } else if (!TextUtils.isEmpty(fileNameForCarrierId)) {
                fileName = fileNameForCarrierId;
            } else if (!TextUtils.isEmpty(fileNameForMccMncCarrierId)) {
                fileName = fileNameForMccMncCarrierId;
            }
        }

        if (TextUtils.isEmpty(fileName)) {
            return null;
        }

        return CarrierConfig.CARRIER_CONFIG + "/" + fileName;
    }

    private PersistableBundle readConfigFromXml(XmlPullParser parser, SimCarrierId id)
            throws IOException, XmlPullParserException {
        PersistableBundle config = new PersistableBundle();
        int event;

        while (((event = parser.next()) != XmlPullParser.END_DOCUMENT)) {
            if (event == XmlPullParser.START_TAG
                    && "carrier_config".equals(parser.getName())) {
                if (!checkFilters(parser, id)) {
                    continue;
                }

                PersistableBundle configFragment = ConfigXmlUtils.readConfig(parser);
                config.putAll(configFragment);
            }
        }

        return config;
    }

    private static void addConfig(String key,
            PersistableBundle src, PersistableBundle dst) {
        if (key.endsWith("_string")) {
            dst.putString(key, src.getString(key));
        } else if (key.endsWith("_string_array")) {
            dst.putStringArray(key, src.getStringArray(key));
        } else if (key.endsWith("_int")) {
            dst.putInt(key, src.getInt(key));
        } else if (key.endsWith("_long")) {
            dst.putLong(key, src.getLong(key));
        } else if (key.endsWith("_double")) {
            dst.putDouble(key, src.getDouble(key));
        } else if (key.endsWith("_bool") || key.endsWith("_boolean")) {
            dst.putBoolean(key, src.getBoolean(key));
        } else if (key.endsWith("_int_array")) {
            dst.putIntArray(key, src.getIntArray(key));
        } else if (key.endsWith("_double_array")) {
            dst.putDoubleArray(key, src.getDoubleArray(key));
        } else if (key.endsWith("_bool_array")) {
            dst.putBooleanArray(key, src.getBooleanArray(key));
        } else if (key.endsWith("_long_array")) {
            dst.putLongArray(key, src.getLongArray(key));
        } else if (key.endsWith("_bundle")) {
            dst.putPersistableBundle(key, src.getPersistableBundle(key));
        }
    }

    private static boolean isKeyForIms(String key) {
        for (String prefix : CarrierConfig.IMS_KEY_PREFIXES) {
            if (key.startsWith(prefix)) {
                return true;
            }
        }

        return false;
    }

    private static boolean checkFilters(XmlPullParser parser, SimCarrierId id) {
        for (int i = 0; i < parser.getAttributeCount(); ++i) {
            boolean result = true;
            String attribute = parser.getAttributeName(i);
            String value = parser.getAttributeValue(i);
            switch (attribute) {
                case "mcc":
                    result = (id == null) || value.equals(id.getMcc());
                    break;
                case "mnc":
                    result = (id == null) || value.equals(id.getMnc());
                    break;
                case "gid1":
                    result = (id == null) || value.equalsIgnoreCase(id.getGid1());
                    break;
                case "spn":
                    result = (id == null) || matchOnSpn(value, id);
                    break;
                case "imsi":
                    result = (id == null) || matchOnImsi(value, id);
                    break;
                case "cid":
                    result = (id == null) || matchOnCarrierId(value, id);
                    break;
                default:
                    ImsLog.w("Unknown attribute: " + attribute + "=" + value);
                    result = false;
                    break;
            }

            if (!result) {
                return false;
            }
        }
        return true;
    }

    private static boolean matchOnCarrierId(String xmlCid, SimCarrierId id) {
        int cid = -1;

        try {
            cid = Integer.parseInt(xmlCid);
        } catch (NumberFormatException e) {
            return false;
        }

        return cid == id.getCarrierId() || cid == id.getSpecificCarrierId();
    }

    private static boolean matchOnImsi(String xmlImsi, SimCarrierId id) {
        boolean matchFound = false;
        String currentImsi = id.getImsi();

        if (currentImsi != null) {
            Pattern imsiPattern = Pattern.compile(xmlImsi, Pattern.CASE_INSENSITIVE);
            Matcher matcher = imsiPattern.matcher(currentImsi);
            matchFound = matcher.matches();
        }

        return matchFound;
    }

    private static boolean matchOnSpn(String xmlSpn, SimCarrierId id) {
        final String SPN_EMPTY_MATCH = "null";
        boolean matchFound = false;
        String currentSpn = id.getSpn();

        if (SPN_EMPTY_MATCH.equalsIgnoreCase(xmlSpn)) {
            if (TextUtils.isEmpty(currentSpn)) {
                matchFound = true;
            }
        } else if (currentSpn != null) {
            Pattern spnPattern = Pattern.compile(xmlSpn, Pattern.CASE_INSENSITIVE);
            Matcher matcher = spnPattern.matcher(currentSpn);
            matchFound = matcher.matches();
        }

        return matchFound;
    }

    private final class IntentReceiver extends BroadcastReceiver {
        private PersistableBundle mConfig;

        public void register() {
            IntentFilter filter = new IntentFilter();
            filter.addAction(ACTION_TEST_CARRIER_CONFIG_PUT);
            filter.addAction(ACTION_TEST_CARRIER_CONFIG_APPLY);

            AppContext.get().registerReceiver(this, filter, null,
                    AppContext.getMainHandler(), Context.RECEIVER_EXPORTED);
        }

        public void unregister() {
            AppContext.get().unregisterReceiver(this);
        }

        @Override
        public void onReceive(Context context, Intent intent) {
            ImsLog.d(mSlotId, "onReceive: " + intent);

            String action = intent.getAction();

            if (ACTION_TEST_CARRIER_CONFIG_PUT.equals(action)) {
                String key = intent.getStringExtra(KEY_NAME);

                if (ImsPrivateProperties.Persistent.isConfigProperty(key)) {
                    String value = intent.getStringExtra(KEY_VALUE);

                    ImsLog.d(mSlotId, "TestCarrierConfigPut: [" + key + "=" + value + "]");

                    if (value == null) {
                        value = "";
                    }

                    ImsPrivateProperties.Persistent.set(key, value, mSlotId);
                } else {
                    putConfig(key, intent);
                }
            } else if (ACTION_TEST_CARRIER_CONFIG_APPLY.equals(action)) {
                ImsLog.d(mSlotId, "TestCarrierConfigApply");
                if (mConfig != null) {
                    writeTestConfig(mTestConfig);
                    mConfig = null;
                }
            }
        }

        private void putConfig(String key, Intent intent) {
            String valueForLog = "(unknown)";

            if (mConfig == null) {
                mConfig = new PersistableBundle();
            }

            if (mTestConfig == null) {
                readTestConfig();
            }

            if (key.endsWith("_string")) {
                String value = intent.getStringExtra(KEY_VALUE);
                valueForLog = (value == null) ? "(null)" : value;
                mConfig.putString(key, value);
                mTestConfig.putString(key, value);
                mCarrierConfig.getConfig().putString(key, value);
            } else if (key.endsWith("_string_array")) {
                String[] value = intent.getStringArrayExtra(KEY_VALUE);
                valueForLog = (value == null) ? "(null)" : Arrays.toString(value);
                mConfig.putStringArray(key, value);
                mTestConfig.putStringArray(key, value);
                mCarrierConfig.getConfig().putStringArray(key, value);
            } else if (key.endsWith("_int")) {
                int value = intent.getIntExtra(KEY_VALUE, -1);
                valueForLog = String.valueOf(value);
                mConfig.putInt(key, value);
                mTestConfig.putInt(key, value);
                mCarrierConfig.getConfig().putInt(key, value);
            } else if (key.endsWith("_long")) {
                long value = intent.getLongExtra(KEY_VALUE, -1L);
                valueForLog = String.valueOf(value);
                mConfig.putLong(key, value);
                mTestConfig.putLong(key, value);
                mCarrierConfig.getConfig().putLong(key, value);
            } else if (key.endsWith("_double")) {
                double value = intent.getDoubleExtra(KEY_VALUE, 0);
                valueForLog = String.valueOf(value);
                mConfig.putDouble(key, value);
                mTestConfig.putDouble(key, value);
                mCarrierConfig.getConfig().putDouble(key, value);
            } else if (key.endsWith("_bool") || key.endsWith("_boolean")) {
                boolean value = intent.getBooleanExtra(KEY_VALUE, false);
                valueForLog = String.valueOf(value);
                mConfig.putBoolean(key, value);
                mTestConfig.putBoolean(key, value);
                mCarrierConfig.getConfig().putBoolean(key, value);
            } else if (key.endsWith("_int_array")) {
                int[] value = intent.getIntArrayExtra(KEY_VALUE);
                valueForLog = (value == null) ? "(null)" : Arrays.toString(value);
                mConfig.putIntArray(key, value);
                mTestConfig.putIntArray(key, value);
                mCarrierConfig.getConfig().putIntArray(key, value);
            } else if (key.endsWith("_double_array")) {
                double[] value = intent.getDoubleArrayExtra(KEY_VALUE);
                valueForLog = (value == null) ? "(null)" : Arrays.toString(value);
                mConfig.putDoubleArray(key, value);
                mTestConfig.putDoubleArray(key, value);
                mCarrierConfig.getConfig().putDoubleArray(key, value);
            } else if (key.endsWith("_long_array")) {
                long[] value = intent.getLongArrayExtra(KEY_VALUE);
                valueForLog = (value == null) ? "(null)" : Arrays.toString(value);
                mConfig.putLongArray(key, value);
                mTestConfig.putLongArray(key, value);
                mCarrierConfig.getConfig().putLongArray(key, value);
            }

            ImsLog.d(mSlotId, "TestCarrierConfigPut: [" + key + "=" + valueForLog + "]");
        }
    }
}
