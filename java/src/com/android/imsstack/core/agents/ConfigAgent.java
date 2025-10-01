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

import android.annotation.XmlRes;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.content.res.XmlResourceParser;
import android.os.Bundle;
import android.os.PersistableBundle;
import android.telephony.CarrierConfigManager;
import android.text.TextUtils;
import android.util.ArraySet;

import androidx.annotation.NonNull;
import androidx.annotation.VisibleForTesting;

import com.android.imsstack.R;
import com.android.imsstack.base.AppContext;
import com.android.imsstack.base.DeviceConfig;
import com.android.imsstack.base.ImsPrivateProperties;
import com.android.imsstack.base.MSimUtils;
import com.android.imsstack.base.SystemServiceProxy.CarrierConfigManagerProxy;
import com.android.imsstack.base.TelephonyManagerProxy;
import com.android.imsstack.core.carrier.SimCarrierId;
import com.android.imsstack.core.config.CarrierConfig;
import com.android.imsstack.core.config.ConfigXmlUtils;
import com.android.imsstack.system.ISystem;
import com.android.imsstack.system.SystemInterface;
import com.android.imsstack.util.ImsLog;
import com.android.imsstack.util.ImsUtils;
import com.android.imsstack.util.IndentingPrintWriter;
import com.android.imsstack.util.Log;

import org.xmlpull.v1.XmlPullParser;
import org.xmlpull.v1.XmlPullParserException;
import org.xmlpull.v1.XmlPullParserFactory;

import java.io.FileInputStream;
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
    /** Private config commands for updating an internal configuration. */
    @VisibleForTesting
    protected static final String ACTION_GET_CONFIG =
            ImsUtils.PACKAGE_NAME + ".action.GET_CONFIG";
    @VisibleForTesting
    protected static final String ACTION_SET_CONFIG =
            ImsUtils.PACKAGE_NAME + ".action.SET_CONFIG";
    @VisibleForTesting
    protected static final String ACTION_CLEAR_CONFIG =
            ImsUtils.PACKAGE_NAME + ".action.CLEAR_CONFIG";
    @VisibleForTesting
    protected static final String KEY_SLOT_ID = "slot_id";
    @VisibleForTesting
    protected static final String KEY_COMMITTABLE = "committable";
    @VisibleForTesting
    protected static final String KEY_CONFIG_KEYS = "config_keys";
    @VisibleForTesting
    protected static final String KEY_CONFIG_FROM_XML = "config_from_xml";
    // Command is successfully executed.
    @VisibleForTesting
    protected static final int RESULT_OK = 1;
    // Command with committable flag is successfully executed.
    @VisibleForTesting
    protected static final int RESULT_COMMITTABLE_OK = 12;

    private static final String CARRIER_ID_PREFIX = "carrier_config_carrierid_";
    private static final String MCC_MNC_PREFIX = "carrier_config_mccmnc_";
    /** Intent for testing purpose. */
    // TODO: should be integrated with the above commands in the future.
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
    private SimCarrierId mCarrierId;
    private volatile boolean mConfigLoaded;
    private final PersistableBundle mDefaultInternalConfig = new PersistableBundle();
    private final PersistableBundle mCarrierInternalConfig = new PersistableBundle();
    private final PersistableBundle mCarrierInternalOverrideConfig = new PersistableBundle();
    private final PersistableBundle mDefaultPublicConfig = new PersistableBundle();
    private final PersistableBundle mCarrierPublicConfig = new PersistableBundle();
    private PersistableBundle mTestConfig;
    private XmlPullParserFactory mFactory;

    public ConfigAgent(int slotId) {
        mSlotId = slotId;
        mCarrierConfig = new CarrierConfig();
        mIntentReceiver = new IntentReceiver();
    }

    @Override
    public void init(Context context) {
        mDefaultInternalConfig.putAll(readCarrierConfigFromAsset(
                CarrierConfig.DEFAULT_CARRIER_CONFIG_FILE, null));
        mDefaultPublicConfig.putAll(readCarrierConfigFromAsset(
                CarrierConfig.DEFAULT_PUBLIC_CARRIER_CONFIG_FILE, null));
        mIntentReceiver.register();
    }

    @Override
    public void cleanup() {
        mIntentReceiver.unregister();
        mListeners.clear();
        mDefaultInternalConfig.clear();
        mCarrierInternalConfig.clear();
        mCarrierInternalOverrideConfig.clear();
        mDefaultPublicConfig.clear();
        mCarrierPublicConfig.clear();
    }

    @Override
    public void dump(@NonNull IndentingPrintWriter pw) {
        if (mSlotId == 0) {
            dumpCarrierConfig(pw, "DefaultInternalConfig", mDefaultInternalConfig);
            dumpCarrierConfig(pw, "DefaultPublicConfig", mDefaultPublicConfig);
            pw.flush();
        }
        pw.println("Slot" + mSlotId + ":");
        pw.increaseIndent();
        pw.println("configLoaded=" + mConfigLoaded);
        pw.println("carrierId=" + (mCarrierId != null ? mCarrierId : "unknown"));
        pw.println("cachedConfig=" + getFileNameForLogging(getCachedConfigFile(mSlotId)));
        pw.println();
        dumpCarrierConfig(pw, "CarrierInternalConfig", mCarrierInternalConfig);
        dumpCarrierConfig(pw, "CarrierPublicConfig", mCarrierPublicConfig);
        dumpCarrierConfig(pw, "CarrierInternalOverrideConfig", mCarrierInternalOverrideConfig);
        dumpCarrierConfig(pw, "TestOverrideConfig", mTestConfig);
        pw.decreaseIndent();
        pw.println();
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

    @Override
    public void notifyCarrierConfigChangedForNative() {
        ISystem system = SystemInterface.getInstance().getSystem(mSlotId);

        if (system != null) {
            system.notifyConfigurationChanged(0);
        }
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
        config.putAll(mDefaultInternalConfig);

        // Sets the internal carrier configuration.
        // 1) Precedence: specific-carrier-id > carrier-id > carrier-id-from-sim-mcc-mnc.
        // 2) When carrier-id is unknown, mcc-mnc based XML will be used.
        mCarrierInternalConfig.clear();
        PersistableBundle tempConfig = readCarrierConfig(subId, id);

        int[] parentCarrierIds = tempConfig.getIntArray(
                CarrierConfig.KEY_IMS_PARENT_CARRIER_IDS_INT_ARRAY);

        if (parentCarrierIds != null && parentCarrierIds.length > 0) {
            for (int cid : parentCarrierIds) {
                SimCarrierId parentCid = new SimCarrierId.Builder()
                        .setCarrierId(cid)
                        .build();
                PersistableBundle parentConfig = readCarrierConfig(subId, parentCid);
                CarrierConfig.overrideNestedBundles(mCarrierInternalConfig, parentConfig);
                mCarrierInternalConfig.putAll(parentConfig);
            }
        }

        CarrierConfig.overrideNestedBundles(mCarrierInternalConfig, tempConfig);
        mCarrierInternalConfig.putAll(tempConfig);
        tempConfig = mCarrierInternalConfig.deepCopy();
        CarrierConfig.overrideNestedBundles(config, tempConfig);
        config.putAll(tempConfig);

        // Overrides the specific carrier configuration values if present.
        tempConfig = readCarrierConfigFromRes(R.xml.carrier_config_override, id);
        mCarrierInternalConfig.putAll(tempConfig);
        CarrierConfig.overrideNestedBundles(config, tempConfig);
        config.putAll(tempConfig);

        // Sets the public carrier configuration from CarrierConfigManager.
        tempConfig = getCarrierConfig(subId);
        config.putAll(tempConfig);

        // Sets the internal carrier configuration from CarrierConfigManager.
        mCarrierInternalOverrideConfig.clear();
        tempConfig = getCarrierInternalConfig(subId);
        mCarrierInternalOverrideConfig.putAll(tempConfig);
        config.putAll(tempConfig);

        // Sets the internal public carrier configuration.
        mCarrierPublicConfig.clear();
        if (config.getBoolean(CarrierConfig.KEY_IMS_OVERRIDE_PUBLIC_CONFIG_BOOL, true)) {
            ImsLog.d(this, mSlotId, "Overriding public configs...");
            tempConfig = mDefaultPublicConfig.deepCopy();
            CarrierConfig.overrideNestedBundles(config, tempConfig);
            config.putAll(tempConfig);

            if (parentCarrierIds != null && parentCarrierIds.length > 0) {
                for (int cid : parentCarrierIds) {
                    SimCarrierId parentCid = new SimCarrierId.Builder()
                            .setCarrierId(cid)
                            .build();
                    PersistableBundle parentConfig = readCarrierConfig(subId, parentCid, false);
                    CarrierConfig.overrideNestedBundles(mCarrierPublicConfig, parentConfig);
                    mCarrierPublicConfig.putAll(parentConfig);
                }
            }

            tempConfig = readCarrierConfig(subId, id, false);
            CarrierConfig.overrideNestedBundles(mCarrierPublicConfig, tempConfig);
            mCarrierPublicConfig.putAll(tempConfig);
            tempConfig = mCarrierPublicConfig.deepCopy();
            CarrierConfig.overrideNestedBundles(config, tempConfig);
            config.putAll(tempConfig);
        }

        mIntentReceiver.setOriginalCarrierConfig(config);

        // Loads override configs in the hidden key of CarrierConfigManager
        overrideHiddenConfigs(subId, config);

        // test-carrier-config
        PersistableBundle testConfig = readTestConfig();

        if (id.isSimLoaded()) {
            deleteCachedConfigFile(mSlotId);

            if (shouldUseCachedConfigWhenNoSim(config)
                    || shouldUseCachedConfigWhenNoSim(testConfig)) {
                saveCachedConfigToXml(id, config);
            }
        } else {
            tempConfig = readCachedConfigFromXml(id);
            mCarrierInternalOverrideConfig.putAll(tempConfig);
            config.putAll(tempConfig);
        }

        ImsLog.d(this, mSlotId, "updateCarrierConfig: " + config.toString());

        overrideTestConfigs(config, testConfig);

        mCarrierConfig.setConfig(config, mSlotId);

        if (!mConfigLoaded) {
            mConfigLoaded = true;
        }
        mCarrierId = id;

        notifyCarrierConfigChanged(subId);
        notifyCarrierConfigChangedForNative();
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

    private void overrideTestConfigs(PersistableBundle config, PersistableBundle testConfig) {
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

        if (!testConfig.isEmpty()) {
            PersistableBundle newTestConfig = testConfig.deepCopy();
            CarrierConfig.overrideNestedBundles(config, newTestConfig);
            config.putAll(newTestConfig);

            for (String key : testConfig.keySet()) {
                ImsLog.d(this, mSlotId, key + "=" + CarrierConfig.getValue(testConfig, key));
            }
        }
    }

    private PersistableBundle getCarrierConfig(int subId) {
        String[] configKeys = Stream.of(CarrierConfig.IMS_COMMON_KEYS,
                CarrierConfig.IMS_PREFIX_KEYS, CarrierConfig.IMS_VOICE_PREFIX_KEYS,
                CarrierConfig.IMS_SMS_PREFIX_KEYS, CarrierConfig.IMS_RTT_PREFIX_KEYS,
                CarrierConfig.IMS_EMERGENCY_PREFIX_KEYS, CarrierConfig.IMS_VT_PREFIX_KEYS,
                CarrierConfig.IMS_WFC_PREFIX_KEYS, CarrierConfig.IMS_SS_PREFIX_KEYS,
                new String[] {CarrierConfig.ApIms.KEY_CARRIER_CONFIG_BUNDLE})
                    .flatMap(Stream::of)
                    .toArray(String[]::new);

        CarrierConfigManagerProxy ccmp =
                AppContext.getInstance().getSystemServiceProxy(CarrierConfigManagerProxy.class);

        // If an invalid subId is used, this bundle will contain default values.
        return ccmp.getConfigForSubId(subId, configKeys);
    }

    private PersistableBundle getCarrierInternalConfig(int subId) {
        String[] configKeys = mDefaultInternalConfig.keySet().toArray(new String[0]);
        CarrierConfigManagerProxy ccmp =
                AppContext.getInstance().getSystemServiceProxy(CarrierConfigManagerProxy.class);

        // If an invalid subId is used, this bundle will contain default values.
        PersistableBundle config = ccmp.getConfigForSubId(subId, configKeys);
        // Removes metadata keys.
        config.remove(CarrierConfigManager.KEY_CARRIER_CONFIG_APPLIED_BOOL);
        config.remove(CarrierConfigManager.KEY_CARRIER_CONFIG_VERSION_STRING);
        return config;
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

    private static void dumpCarrierConfig(@NonNull IndentingPrintWriter pw,
            @NonNull String tag, PersistableBundle config) {
        pw.println(tag + ":");
        if (config != null) {
            pw.increaseIndent();
            pw.increaseIndent();
            for (String key : getKeys(config)) {
                pw.println(key + "=" + CarrierConfig.getValue(config, key));
            }
            pw.decreaseIndent();
            pw.decreaseIndent();
        }
        pw.println();
    }

    private static @NonNull String[] getKeys(@NonNull PersistableBundle b) {
        String[] keys = b.keySet().toArray(new String[0]);
        Arrays.sort(keys);
        return keys;
    }

    private static boolean shouldUseCachedConfigWhenNoSim(PersistableBundle config) {
        return config.getBoolean(
                CarrierConfig.KEY_IMS_USE_CONFIG_OF_LAST_INSERTED_SIM_WHEN_NO_SIM_BOOL);
    }

    private PersistableBundle readCachedConfigFromXml(SimCarrierId id) {
        String fileName = getCachedConfigFile(mSlotId);

        if (fileName == null) {
            ImsLog.d(this, mSlotId, "No cached config");
            return PersistableBundle.EMPTY;
        }

        if (id.isSimLocked() && !fileName.contains(id.getIccId())) {
            ImsLog.d(this, mSlotId, "No cached config on SIM LOCKED");
            return PersistableBundle.EMPTY;
        }

        ImsLog.d(this, mSlotId, "readCachecConfig: " + getFileNameForLogging(fileName));

        try (InputStream is = AppContext.getInstance().openFileInput(fileName)) {
            return PersistableBundle.readFromStream(is);
        } catch (Exception e) {
            ImsLog.d(this, mSlotId, "No cached config: " + e.toString());
        }
        return PersistableBundle.EMPTY;
    }

    private void saveCachedConfigToXml(SimCarrierId id, PersistableBundle config) {
        String fileName = getFileNameForCachedConfig(mSlotId,
                id.getSpecificCarrierId(), id.getMcc() + id.getMnc(), id.getIccId());

        try (OutputStream os = AppContext.getInstance().openFileOutput(
                fileName, Context.MODE_APPEND)) {
            config.writeToStream(os);
            ImsLog.d(this, mSlotId, "saveCachecConfig: " + getFileNameForLogging(fileName));
        } catch (IOException e) {
            ImsLog.w(this, mSlotId, e.toString());
            deleteCachedConfigFile(mSlotId);
        }
    }

    private static String getCachedConfigFile(int slotId) {
        final String fileNamePrefix = getFileNamePrefixForCachedConfig(slotId);
        final String[] fileList = AppContext.getInstance().fileList();
        for (final String fileName : fileList) {
            if (fileName.startsWith(fileNamePrefix)) {
                return fileName;
            }
        }
        return null;
    }

    private static void deleteCachedConfigFile(int slotId) {
        final String fileNamePrefix = getFileNamePrefixForCachedConfig(slotId);
        final String[] fileList = AppContext.getInstance().fileList();
        for (final String fileName : fileList) {
            if (fileName.startsWith(fileNamePrefix)) {
                ImsLog.d(slotId, "deleteCachedConfig: " + getFileNameForLogging(fileName));
                AppContext.getInstance().deleteFile(fileName);
            }
        }
    }

    /** Builds a canonical file name for a cached config file. */
    private static String getFileNameForCachedConfig(
            int slotId, int cid, String mccmnc, String iccId) {
        return getFileNamePrefixForCachedConfig(slotId) + "_" + iccId + "_"
                + (cid != SimCarrierId.UNKNOWN_ID ? cid : mccmnc) + ".xml";
    }

    private static String getFileNamePrefixForCachedConfig(int slotId) {
        return "carrier_config_slot" + slotId;
    }

    private static String getFileNameForLogging(String fileName) {
        if (!TextUtils.isEmpty(fileName)) {
            String[] tokens = fileName.split("_");
            if (tokens != null && tokens.length > 2) {
                String iccid = tokens[tokens.length - 2];
                return getFileNameForLogging(fileName, iccid);
            }
            return fileName;
        }
        return fileName;
    }

    /** Masks most part of ICCID in the file name for logging on user build. */
    private static String getFileNameForLogging(String fileName, String iccId) {
        if (ImsLog.isDebuggable()) {
            return fileName;
        }
        String name = fileName;
        int length = (iccId != null) ? iccId.length() : 0;
        if (length > 5 && fileName != null) {
            name = fileName.replace(iccId.substring(5), "***************");
        }
        return name;
    }

    private static @NonNull String objectToString(Object o) {
        if (o == null) {
            return "(null)";
        }
        if (o.getClass().isArray()) {
            if (o instanceof int[]) {
                return Arrays.toString((int[]) o);
            } else if (o instanceof long[]) {
                return Arrays.toString((long[]) o);
            } else if (o instanceof String[]) {
                return Arrays.toString((String[]) o);
            } else if (o instanceof double[]) {
                return Arrays.toString((double[]) o);
            }
        }
        return String.valueOf(o);
    }

    private final class IntentReceiver extends BroadcastReceiver {
        private PersistableBundle mOriginalCarrierConfig;

        public void register() {
            IntentFilter filter = new IntentFilter();
            filter.addAction(ACTION_GET_CONFIG);
            filter.addAction(ACTION_SET_CONFIG);
            filter.addAction(ACTION_CLEAR_CONFIG);
            filter.addAction(ACTION_TEST_CARRIER_CONFIG_PUT);
            filter.addAction(ACTION_TEST_CARRIER_CONFIG_APPLY);

            AppContext.getInstance().getBroadcastReceiverProxy().registerReceiver(this, filter);
        }

        public void unregister() {
            AppContext.getInstance().getBroadcastReceiverProxy().unregisterReceiver(this);
        }

        /**
         * Sets the original carrier config when the carrier config is updated.
         * The specified config doesn't contain the test configuration.
         *
         * @param config The latest carrier config.
         */
        public void setOriginalCarrierConfig(PersistableBundle config) {
            if (ImsUtils.IS_USER) {
                return;
            }
            if (mOriginalCarrierConfig == null) {
                mOriginalCarrierConfig = config.deepCopy();
            } else {
                mOriginalCarrierConfig.clear();
                mOriginalCarrierConfig.putAll(config);
            }
        }

        @Override
        public void onReceive(Context context, Intent intent) {
            // Carrier config is not loaded yet, or user build.
            if (mOriginalCarrierConfig == null) {
                return;
            }

            String action = intent.getAction();
            int slotId = intent.getIntExtra(KEY_SLOT_ID, MSimUtils.INVALID_SLOT_ID);
            int targetSlotId = getTargetSlotId(slotId);
            boolean committable = intent.getBooleanExtra(KEY_COMMITTABLE, false);

            ImsLog.i(this, mSlotId, "onReceive: " + ImsLog.lastSubString(action, ".")
                    + ", slotId=" + slotId + ", targetSlotId=" + targetSlotId
                    + ", committable=" + committable);

            if (targetSlotId != mSlotId) {
                // Ignore the event; it's not for this slot's configuration.
                return;
            }

            int resultCode = RESULT_OK;

            if (ACTION_SET_CONFIG.equals(action)) {
                intent.removeExtra(KEY_SLOT_ID);
                intent.removeExtra(KEY_COMMITTABLE);
                Bundle config = intent.getExtras();
                if (config != null) {
                    Set<String> unprocessedKeys = setConfig(config);
                    if (!unprocessedKeys.isEmpty()) {
                        setResultData("unprocessed-keys=" + unprocessedKeys.toString());
                    }
                }
            } else if (ACTION_CLEAR_CONFIG.equals(action)) {
                clearConfig(intent.getStringArrayExtra(KEY_CONFIG_KEYS));
            } else if (ACTION_GET_CONFIG.equals(action)) {
                setResultData(getConfig(intent.getStringArrayExtra(KEY_CONFIG_KEYS)));
                // This is to prevent the config from being accidentally updated incorrectly.
                committable = false;
            } else if (ACTION_TEST_CARRIER_CONFIG_PUT.equals(action)) {
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
                committable = true;
            }

            if (committable) {
                resultCode = RESULT_COMMITTABLE_OK;
                commitConfig();
            }
            setResultCode(resultCode);
        }

        private void putConfig(String key, Intent intent) {
            String valueForLog = "(unknown)";

            if (key.endsWith("_string")) {
                String value = intent.getStringExtra(KEY_VALUE);
                valueForLog = (value == null) ? Log.NULL : value;
                mTestConfig.putString(key, value);
                mCarrierConfig.getConfig().putString(key, value);
            } else if (key.endsWith("_string_array")) {
                String[] value = intent.getStringArrayExtra(KEY_VALUE);
                valueForLog = (value == null) ? Log.NULL : Arrays.toString(value);
                mTestConfig.putStringArray(key, value);
                mCarrierConfig.getConfig().putStringArray(key, value);
            } else if (key.endsWith("_int")) {
                int value = intent.getIntExtra(KEY_VALUE, -1);
                valueForLog = String.valueOf(value);
                mTestConfig.putInt(key, value);
                mCarrierConfig.getConfig().putInt(key, value);
            } else if (key.endsWith("_long")) {
                long value = intent.getLongExtra(KEY_VALUE, -1L);
                valueForLog = String.valueOf(value);
                mTestConfig.putLong(key, value);
                mCarrierConfig.getConfig().putLong(key, value);
            } else if (key.endsWith("_double")) {
                double value = intent.getDoubleExtra(KEY_VALUE, 0);
                valueForLog = String.valueOf(value);
                mTestConfig.putDouble(key, value);
                mCarrierConfig.getConfig().putDouble(key, value);
            } else if (key.endsWith("_bool") || key.endsWith("_boolean")) {
                boolean value = intent.getBooleanExtra(KEY_VALUE, false);
                valueForLog = String.valueOf(value);
                mTestConfig.putBoolean(key, value);
                mCarrierConfig.getConfig().putBoolean(key, value);
            } else if (key.endsWith("_int_array")) {
                int[] value = intent.getIntArrayExtra(KEY_VALUE);
                valueForLog = (value == null) ? Log.NULL : Arrays.toString(value);
                mTestConfig.putIntArray(key, value);
                mCarrierConfig.getConfig().putIntArray(key, value);
            } else if (key.endsWith("_double_array")) {
                double[] value = intent.getDoubleArrayExtra(KEY_VALUE);
                valueForLog = (value == null) ? Log.NULL : Arrays.toString(value);
                mTestConfig.putDoubleArray(key, value);
                mCarrierConfig.getConfig().putDoubleArray(key, value);
            } else if (key.endsWith("_long_array")) {
                long[] value = intent.getLongArrayExtra(KEY_VALUE);
                valueForLog = (value == null) ? Log.NULL : Arrays.toString(value);
                mTestConfig.putLongArray(key, value);
                mCarrierConfig.getConfig().putLongArray(key, value);
            } else if (key.equals(
                    CarrierConfigManager.KEY_IGNORE_DATA_ENABLED_CHANGED_FOR_VIDEO_CALLS)) {
                boolean value = intent.getBooleanExtra(KEY_VALUE, false);
                valueForLog = String.valueOf(value);
                mTestConfig.putBoolean(key, value);
                mCarrierConfig.getConfig().putBoolean(key, value);
            }

            ImsLog.d(this, mSlotId, "TestCarrierConfigPut: [" + key + "=" + valueForLog + "]");
        }

        private void clearConfig(String[] configKeys) {
            if (mTestConfig.isEmpty()) {
                return;
            }

            ImsLog.d(this, mSlotId, "clearConfig: " + Arrays.toString(configKeys));

            if (configKeys == null) {
                mTestConfig.clear();
            } else {
                for (String key : configKeys) {
                    mTestConfig.remove(key);
                }
            }
        }

        private String getConfig(String[] configKeys) {
            ImsLog.d(this, mSlotId, "getConfig: " + Arrays.toString(configKeys));

            if (configKeys == null) {
                return null;
            }

            Bundle config = new Bundle(mCarrierConfig.getConfig());
            StringBuilder sb = new StringBuilder();

            for (String key : configKeys) {
                if (ImsPrivateProperties.Persistent.isConfigProperty(key)) {
                    sb.append(key);
                    sb.append("=");
                    sb.append(ImsPrivateProperties.Persistent.get(key, "", mSlotId));
                    sb.append(", ");
                    continue;
                }

                Object value = config.getParcelable(key, Object.class);
                sb.append(key);
                sb.append("=");
                sb.append(objectToString(value));
                sb.append(", ");
            }

            if (sb.length() > 2) {
                sb.delete(sb.length() - 2, sb.length());
            }

            return sb.toString();
        }

        /**
         * Sets the test configs.
         *
         * @param config The {@link Bundle} contains the configs.
         * @return A set of unprocessed keys.
         */
        private @NonNull Set<String> setConfig(@NonNull Bundle config) {
            Set<String> unprocessedKeys = new ArraySet<String>();

            for (String key : config.keySet()) {
                if (ImsPrivateProperties.Persistent.isConfigProperty(key)) {
                    setConfigProperty(key, config);
                    continue;
                }

                Object value = config.getParcelable(key, Object.class);

                ImsLog.d(this, mSlotId, "setConfig: " + key + "=" + objectToString(value));

                if (value instanceof Boolean) {
                    mTestConfig.putBoolean(key, (Boolean) value);
                } else if (value instanceof Integer) {
                    mTestConfig.putInt(key, (Integer) value);
                } else if (value instanceof Long) {
                    mTestConfig.putLong(key, (Long) value);
                } else if (value instanceof String) {
                    if (KEY_CONFIG_FROM_XML.equals(key)) {
                        if (!setConfigFromXml((String) value)) {
                            unprocessedKeys.add(key);
                        }
                    } else {
                        mTestConfig.putString(key, (String) value);
                    }
                } else if (value instanceof int[]) {
                    mTestConfig.putIntArray(key, (int[]) value);
                } else if (value instanceof long[]) {
                    mTestConfig.putLongArray(key, (long[]) value);
                } else if (value instanceof String[]) {
                    String[] stringArray = (String[]) value;
                    for (int i = 0; i < stringArray.length; ++i) {
                        String s = stringArray[i];
                        if (s.contains("\\")) {
                            ImsLog.d(this, mSlotId, "Escaping character detected: " + s);
                            stringArray[i] = s.replace("\\", "");
                        }
                    }
                    mTestConfig.putStringArray(key, stringArray);
                } else if (value instanceof Double) {
                    mTestConfig.putDouble(key, (Double) value);
                } else if (value instanceof double[]) {
                    mTestConfig.putDoubleArray(key, (double[]) value);
                } else {
                    unprocessedKeys.add(key);
                }
            }

            return unprocessedKeys;
        }

        private boolean setConfigFromXml(@NonNull String filePath) {
            ImsLog.d(this, mSlotId, "setConfigFromXml: " + filePath);

            PersistableBundle config = null;

            try (InputStream is = new FileInputStream(filePath)) {
                config = PersistableBundle.readFromStream(is);
            } catch (FileNotFoundException e) {
                ImsLog.d(this, mSlotId, "setConfigFromXml: not found");
            } catch (Exception e) {
                ImsLog.d(this, mSlotId, "setConfigFromXml: " + e.toString());
            }

            if (config == null) {
                return false;
            }

            mTestConfig.putAll(config);
            return true;
        }

        private void setConfigProperty(@NonNull String key, @NonNull Bundle config) {
            String value = config.getString(key, "");
            ImsLog.d(this, mSlotId, "setConfigProperty: " + key + "=" + value);
            ImsPrivateProperties.Persistent.set(key, value, mSlotId);
        }

        private void commitConfig() {
            ImsLog.d(this, mSlotId, "commitConfig");

            // Stores the config to the persistent storage.
            writeTestConfig(mTestConfig);

            // Recalculates the actual carrier config with the test config.
            PersistableBundle config = mOriginalCarrierConfig.deepCopy();
            overrideTestConfigs(config, mTestConfig);
            mCarrierConfig.setConfig(config, mSlotId);

            // Notifies listeners of the carrier config change.
            notifyCarrierConfigChanged(MSimUtils.getSubId(mSlotId));
            notifyCarrierConfigChangedForNative();
        }

        private int getTargetSlotId(int slotId) {
            if (slotId != MSimUtils.INVALID_SLOT_ID) {
                return slotId;
            }

            boolean allSimAbsent = true;
            int activeSimCount = DeviceConfig.getActiveSimCount();

            for (int i = 0; i < activeSimCount; ++i) {
                int subId = MSimUtils.getSubId(i);
                if (subId != MSimUtils.INVALID_SUB_ID) {
                    allSimAbsent = false;
                    break;
                }
            }

            if (allSimAbsent) {
                return MSimUtils.DEFAULT_SLOT_ID;
            } else if (MSimUtils.getSubId(mSlotId) == MSimUtils.getDefaultVoiceSubId()) {
                return mSlotId;
            }
            return MSimUtils.INVALID_SLOT_ID;
        }
    }
}
