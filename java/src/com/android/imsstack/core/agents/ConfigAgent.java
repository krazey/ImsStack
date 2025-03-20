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

import android.annotation.NonNull;
import android.annotation.XmlRes;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.content.res.XmlResourceParser;
import android.os.PersistableBundle;
import android.telephony.CarrierConfigManager;
import android.text.TextUtils;

import com.android.imsstack.R;
import com.android.imsstack.base.AppContext;
import com.android.imsstack.base.ImsPrivateProperties;
import com.android.imsstack.base.SystemServiceProxy.CarrierConfigManagerProxy;
import com.android.imsstack.base.TelephonyManagerProxy;
import com.android.imsstack.core.carrier.SimCarrierId;
import com.android.imsstack.core.config.CarrierConfig;
import com.android.imsstack.core.config.ConfigXmlUtils;
import com.android.imsstack.util.ImsLog;
import com.android.internal.annotations.VisibleForTesting;

import org.xmlpull.v1.XmlPullParser;
import org.xmlpull.v1.XmlPullParserException;
import org.xmlpull.v1.XmlPullParserFactory;

import java.io.FileNotFoundException;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.util.Arrays;
import java.util.Set;
import java.util.concurrent.CopyOnWriteArraySet;
import java.util.regex.Matcher;
import java.util.regex.Pattern;
import java.util.stream.Stream;

/** A class for providing the configuration related information. */
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

    private final Set<Listener> mListeners = new CopyOnWriteArraySet<>();
    private final int mSlotId;
    private final CarrierConfig mCarrierConfig;
    private final IntentReceiver mIntentReceiver;
    private volatile boolean mConfigLoaded;
    private PersistableBundle mDefaultImsConfig;
    private PersistableBundle mDefaultPublicConfig;
    private PersistableBundle mTestConfig;
    private XmlPullParserFactory mFactory;

    public ConfigAgent(int slotId) {
        mSlotId = slotId;
        mCarrierConfig = new CarrierConfig();
        mIntentReceiver = new IntentReceiver();
    }

    @Override
    public void init(Context context) {
        mDefaultImsConfig = readCarrierConfigFromAsset(
                CarrierConfig.DEFAULT_CARRIER_CONFIG_FILE, null);
        mDefaultPublicConfig = readCarrierConfigFromAsset(
                CarrierConfig.DEFAULT_PUBLIC_CARRIER_CONFIG_FILE, null);
        mIntentReceiver.register();
    }

    @Override
    public void cleanup() {
        mIntentReceiver.unregister();
        mListeners.clear();
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

        try (InputStream is = AppContext.getInstance().openFileInput(
                CarrierConfig.TEST_CARRIER_CONFIG_FILE)) {
            mTestConfig = PersistableBundle.readFromStream(is);
        } catch (FileNotFoundException e) {
            ImsLog.d(this, mSlotId, "readTestConfig: not found");
            mTestConfig = new PersistableBundle();
        } catch (Exception e) {
            ImsLog.d(this, mSlotId, "readTestConfig: " + e.toString());
            mTestConfig = new PersistableBundle();
        }

        return mTestConfig;
    }

    @Override
    public boolean writeTestConfig(PersistableBundle config) {
        if (config == null) {
            return false;
        }

        AppContext.getInstance().deleteFile(CarrierConfig.TEST_CARRIER_CONFIG_FILE);

        if (!config.isEmpty()) {
            try (OutputStream os = AppContext.getInstance().openFileOutput(
                    CarrierConfig.TEST_CARRIER_CONFIG_FILE,
                    Context.MODE_APPEND)) {
                config.writeToStream(os);
                ImsLog.d(this, mSlotId, "writeTestConfig: Ok");
                return true;
            } catch (IOException e) {
                ImsLog.d(this, mSlotId, "writeTestConfig: " + e.toString());
            }

            return false;
        } else {
            return true;
        }
    }

    @Override
    public boolean isConfigLoaded() {
        return mConfigLoaded;
    }

    @Override
    public void addListener(@NonNull Listener listener) {
        if (listener == null) {
            throw new IllegalArgumentException("Listener must not be null.");
        }

        mListeners.add(listener);
    }

    @Override
    public void removeListener(@NonNull Listener listener) {
        if (listener == null) {
            throw new IllegalArgumentException("Listener must not be null.");
        }

        mListeners.remove(listener);
    }

    @VisibleForTesting
    protected void notifyCarrierConfigChanged(int subId) {
        ImsLog.i(this, mSlotId, "notifyCarrierConfigChanged: subId=" + subId);

        AppContext.runTask(() -> {
            for (Listener l : mListeners) {
                l.onCarrierConfigChanged(mSlotId, subId);
            }
        }, 0);
    }

    /**
     * Updates the carrier configuration for the given subscription and carrier identifier.
     *
     * @param subId The subscription id.
     * @param id The SIM carrier identifier.
     */
    public void updateCarrierConfig(int subId, SimCarrierId id) {
        PersistableBundle config = new PersistableBundle();

        // Sets a default internal configuration.
        config.putAll(mDefaultImsConfig);

        // Sets the internal carrier configuration.
        // 1) Precedence: specific-carrier-id > carrier-id > carrier-id-from-sim-mcc-mnc.
        // 2) When carrier-id is unknown, mcc-mnc based XML will be used.
        PersistableBundle internalConfig;

        // Sets the parent carrier configuration first if present.
        if (id.getSpecificCarrierId() != SimCarrierId.UNKNOWN_ID
                && id.getCarrierId() != id.getSpecificCarrierId()) {
            SimCarrierId parentId = new SimCarrierId.Builder()
                    .setCarrierId(id.getCarrierId())
                    .build();
            internalConfig = readCarrierConfig(subId, parentId);
            CarrierConfig.overrideNestedBundles(config, internalConfig);
            config.putAll(internalConfig);
        }

        internalConfig = readCarrierConfig(subId, id);
        CarrierConfig.overrideNestedBundles(config, internalConfig);
        config.putAll(internalConfig);

        // Overrides the specific carrier configuration values if present.
        internalConfig = readCarrierConfigFromRes(R.xml.carrier_config_override, id);
        CarrierConfig.overrideNestedBundles(config, internalConfig);
        config.putAll(internalConfig);

        // Sets the public carrier configuration from CarrierConfigManager.
        PersistableBundle publicConfig = getCarrierConfig(subId);
        config.putAll(publicConfig);

        // Sets the internal public carrier configuration.
        if (config.getBoolean(CarrierConfig.KEY_IMS_OVERRIDE_PUBLIC_CONFIG_BOOL, true)) {
            ImsLog.d(this, mSlotId, "Overriding public configs...");
            publicConfig = new PersistableBundle(mDefaultPublicConfig);
            CarrierConfig.overrideNestedBundles(config, publicConfig);
            config.putAll(publicConfig);

            publicConfig = readCarrierConfig(subId, id, false);
            CarrierConfig.overrideNestedBundles(config, publicConfig);
            config.putAll(publicConfig);
        }

        // Loads override configs in the hidden key of CarrierConfigManager
        overrideHiddenConfigs(subId, config);

        ImsLog.d(this, mSlotId, "updateCarrierConfig: " + config.toString());

        java.util.Set<String> keys = config.keySet();

        for (String key : keys) {
            ImsLog.d(this, mSlotId, key + "=" + CarrierConfig.getValue(config, key));
        }

        overrideTestConfigs(config);

        mCarrierConfig.setConfig(config, mSlotId);

        if (!mConfigLoaded) {
            mConfigLoaded = true;
        }

        notifyCarrierConfigChanged(subId);
    }

    @VisibleForTesting
    protected void overrideHiddenConfigs(int subId, PersistableBundle config) {
        // Load the settings from the AP IMS hidden key of CarrierConfigManager if it exists
        if (!config.containsKey(CarrierConfig.ApIms.KEY_CARRIER_CONFIG_BUNDLE)) {
            return;
        }
        PersistableBundle configsInHiddenKey = config.getPersistableBundle(
                CarrierConfig.ApIms.KEY_CARRIER_CONFIG_BUNDLE);
        config.remove(CarrierConfig.ApIms.KEY_CARRIER_CONFIG_BUNDLE);
        if (configsInHiddenKey.isEmpty()) {
            return;
        }

        ImsLog.i(this, mSlotId, "Override internal settings for the hidden key");
        CarrierConfig.overrideNestedBundles(config, configsInHiddenKey);
        config.putAll(configsInHiddenKey);
    }

    private void overrideTestConfigs(PersistableBundle config) {
        ImsLog.i(this, mSlotId, "overrideTestConfigs...");

        boolean usePredefinedUaString = ImsPrivateProperties.Persistent.getBoolean(
                ImsPrivateProperties.Persistent.KEY_USE_PREDEFINED_UA_STRING, false, mSlotId);

        if (usePredefinedUaString) {
            String uaString = ImsPrivateProperties.Persistent.get(
                    ImsPrivateProperties.Persistent.KEY_CONFIG_UA_STRING, mSlotId);

            if (!TextUtils.isEmpty(uaString)) {
                ImsLog.d(this, mSlotId, "Use predefined UA string: " + uaString);
                config.putString(CarrierConfigManager.Ims.KEY_IMS_USER_AGENT_STRING, uaString);
            }
        }

        // test-carrier-config
        PersistableBundle testConfig = readTestConfig();

        if (!testConfig.isEmpty()) {
            PersistableBundle newTestConfig = testConfig.deepCopy();
            CarrierConfig.overrideNestedBundles(config, newTestConfig);
            config.putAll(newTestConfig);

            java.util.Set<String> keys = testConfig.keySet();

            for (String key : keys) {
                ImsLog.d(this, mSlotId, key + "=" + CarrierConfig.getValue(testConfig, key));
            }
        }
    }

    private PersistableBundle getCarrierConfig(int subId) {
        String[] internalConfigKeys = mDefaultImsConfig.keySet().toArray(new String[0]);
        String[] imsConfigKeys = Stream.of(CarrierConfig.IMS_COMMON_KEYS,
                CarrierConfig.IMS_PREFIX_KEYS, CarrierConfig.IMS_VOICE_PREFIX_KEYS,
                CarrierConfig.IMS_SMS_PREFIX_KEYS, CarrierConfig.IMS_RTT_PREFIX_KEYS,
                CarrierConfig.IMS_EMERGENCY_PREFIX_KEYS, CarrierConfig.IMS_VT_PREFIX_KEYS,
                CarrierConfig.IMS_WFC_PREFIX_KEYS, CarrierConfig.IMS_SS_PREFIX_KEYS,
                internalConfigKeys,
                new String[] {CarrierConfig.ApIms.KEY_CARRIER_CONFIG_BUNDLE})
                    .flatMap(Stream::of)
                    .toArray(String[]::new);

        CarrierConfigManagerProxy ccmp =
                AppContext.getInstance().getSystemServiceProxy(CarrierConfigManagerProxy.class);

        // If an invalid subId is used, this bundle will contain default values.
        return ccmp.getConfigForSubId(subId, imsConfigKeys);
    }

    @VisibleForTesting
    protected PersistableBundle readCarrierConfig(int subId, SimCarrierId id) {
        return readCarrierConfig(subId, id, true);
    }

    private PersistableBundle readCarrierConfig(int subId, SimCarrierId id, boolean isInternal) {
        String fileName = getCarrierConfigFile(subId, id,
                isInternal ? CarrierConfig.CARRIER_CONFIG : CarrierConfig.PUBLIC_CARRIER_CONFIG);

        if (TextUtils.isEmpty(fileName)) {
            ImsLog.d(this, mSlotId, "readCarrierConfig: No matched carrier configuration - " + id);
            return new PersistableBundle();
        }

        ImsLog.d(this, mSlotId, "readCarrierConfig: " + fileName);

        return readCarrierConfigFromAsset(fileName, id);
    }

    private PersistableBundle readCarrierConfigFromAsset(String fileName, SimCarrierId id) {
        try (InputStream is = AppContext.getInstance().getAssets().open(fileName)) {
            synchronized (this) {
                if (mFactory == null) {
                    mFactory = XmlPullParserFactory.newInstance();
                }
            }

            XmlPullParser parser = mFactory.newPullParser();
            parser.setInput(is, "utf-8");

            return readConfigFromXml(parser, id);
        } catch (IllegalArgumentException | IOException | XmlPullParserException e) {
            ImsLog.e(this, mSlotId, "readCarrierConfigFromAsset: " + e.toString());
            return new PersistableBundle();
        }
    }

    private PersistableBundle readCarrierConfigFromRes(@XmlRes int resId, SimCarrierId id) {
        try (XmlResourceParser parser = AppContext.getInstance().getResources().getXml(resId)) {
            return readConfigFromXml(parser, id);
        } catch (IOException | XmlPullParserException e) {
            ImsLog.e(this, mSlotId, "readCarrierConfigFromRes: " + e.toString());
            return new PersistableBundle();
        }
    }

    private String getCarrierConfigFile(int subId, SimCarrierId id, @NonNull String path) {
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
                files = AppContext.getInstance().getAssets().list(path);
            } catch (IOException e) {
                ImsLog.e(this, mSlotId, "getCarrierConfigFile: " + e);
                return null;
            }

            TelephonyManagerProxy tmp = AppContext.getTelephonyManagerProxy(subId);
            String prefixForSpecificCarrierId = CARRIER_ID_PREFIX + id.getSpecificCarrierId() + "_";
            String prefixForCarrierId = CARRIER_ID_PREFIX + id.getCarrierId() + "_";
            String prefixForMccMncCarrierId =
                    CARRIER_ID_PREFIX + tmp.getCarrierIdFromSimMccMnc() + "_";
            String fileNameForSpecificCarrierId = null;
            String fileNameForCarrierId = null;
            String fileNameForMccMncCarrierId = null;

            for (String file : files) {
                if (file.startsWith(prefixForSpecificCarrierId)) {
                    fileNameForSpecificCarrierId = file;
                    break;
                } else if (file.startsWith(prefixForCarrierId)) {
                    fileNameForCarrierId = file;
                } else if (file.startsWith(prefixForMccMncCarrierId)) {
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

        return path + "/" + fileName;
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

    private boolean checkFilters(XmlPullParser parser, SimCarrierId id) {
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
                    ImsLog.w(this, mSlotId, "Unknown attribute: " + attribute + "=" + value);
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
        final String spnEmptyMatch = "null";
        boolean matchFound = false;
        String currentSpn = id.getSpn();

        if (spnEmptyMatch.equalsIgnoreCase(xmlSpn)) {
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

            AppContext.getInstance().getBroadcastReceiverProxy().registerReceiver(this, filter);
        }

        public void unregister() {
            AppContext.getInstance().getBroadcastReceiverProxy().unregisterReceiver(this);
        }

        @Override
        public void onReceive(Context context, Intent intent) {
            ImsLog.d(this, mSlotId, "onReceive: " + intent);

            String action = intent.getAction();

            if (ACTION_TEST_CARRIER_CONFIG_PUT.equals(action)) {
                String key = intent.getStringExtra(KEY_NAME);

                if (ImsPrivateProperties.Persistent.isConfigProperty(key)) {
                    String value = intent.getStringExtra(KEY_VALUE);

                    ImsLog.d(this, mSlotId, "TestCarrierConfigPut: [" + key + "=" + value + "]");

                    if (value == null) {
                        value = "";
                    }

                    ImsPrivateProperties.Persistent.set(key, value, mSlotId);
                } else {
                    putConfig(key, intent);
                }
            } else if (ACTION_TEST_CARRIER_CONFIG_APPLY.equals(action)) {
                ImsLog.d(this, mSlotId, "TestCarrierConfigApply");
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
            } else if (key.equals(
                    CarrierConfigManager.KEY_IGNORE_DATA_ENABLED_CHANGED_FOR_VIDEO_CALLS)) {
                boolean value = intent.getBooleanExtra(KEY_VALUE, false);
                valueForLog = String.valueOf(value);
                mConfig.putBoolean(key, value);
                mTestConfig.putBoolean(key, value);
                mCarrierConfig.getConfig().putBoolean(key, value);
            }

            ImsLog.d(this, mSlotId, "TestCarrierConfigPut: [" + key + "=" + valueForLog + "]");
        }
    }
}
