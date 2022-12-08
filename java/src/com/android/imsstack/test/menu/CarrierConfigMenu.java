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
package com.android.imsstack.test.menu;

import android.os.Bundle;
import android.os.PersistableBundle;
import android.preference.EditTextPreference;
import android.preference.ListPreference;
import android.preference.Preference;
import android.preference.PreferenceActivity;
import android.telephony.CarrierConfigManager;
import android.telephony.ims.ImsManager;
import android.telephony.ims.ProvisioningManager;
import android.util.ArraySet;
import android.util.SparseArray;
import android.view.Window;
import android.view.WindowManager.LayoutParams;
import android.widget.Toast;

import com.android.imsstack.R;
import com.android.imsstack.core.ConfigLoader;
import com.android.imsstack.core.agents.AgentFactory;
import com.android.imsstack.core.agents.ConfigInterface;
import com.android.imsstack.core.config.CarrierConfig;
import com.android.imsstack.core.config.ConfigXmlUtils;
import com.android.imsstack.util.AppContext;
import com.android.imsstack.util.ImsLog;
import com.android.imsstack.util.ImsPrivateProperties;
import com.android.imsstack.util.IoUtils;
import com.android.imsstack.util.MSimUtils;

import org.xmlpull.v1.XmlPullParser;
import org.xmlpull.v1.XmlPullParserException;
import org.xmlpull.v1.XmlPullParserFactory;

import java.io.IOException;
import java.io.InputStream;
import java.util.Arrays;
import java.util.Set;

public class CarrierConfigMenu extends PreferenceActivity {
    private static final String PUBLIC_CARRIER_CONFIG_FILE =
            "carrier_config/aosp_carrier_config.xml";

    private static final String KEY_CLEAR_TEST_CONFIG = "clear_test_config";
    private static final String KEY_TEST_CARRIER_ID = "test_carrier_id";
    private static final String KEY_VOLTE_PROVISIONING = "volte_provisioning";
    private static final String KEY_ASSETS_PREFIX = "assets_";
    private static final String KEY_CONFIG_BOOLEAN_ITEMS = "config_boolean_items";
    private static final String KEY_CONFIG_BOOLEAN_VALUE = "config_boolean_value";
    private static final String KEY_CONFIG_INT_ITEMS = "config_int_items";
    private static final String KEY_CONFIG_INT_VALUE = "config_int_value";
    private static final String KEY_CONFIG_STRING_ITEMS = "config_string_items";
    private static final String KEY_CONFIG_STRING_VALUE = "config_string_value";
    private static final String KEY_CONFIG_INT_ARRAY_ITEMS = "config_int_array_items";
    private static final String KEY_CONFIG_INT_ARRAY_VALUE = "config_int_array_value";
    private static final String KEY_CONFIG_STRING_ARRAY_ITEMS = "config_string_array_items";
    private static final String KEY_CONFIG_STRING_ARRAY_VALUE = "config_string_array_value";
    private static final String KEY_LIST_PREFERENCES[] =
        {
            KEY_CONFIG_BOOLEAN_ITEMS,
            KEY_CONFIG_INT_ITEMS,
            KEY_CONFIG_STRING_ITEMS,
            KEY_CONFIG_INT_ARRAY_ITEMS,
            KEY_CONFIG_STRING_ARRAY_ITEMS,
            KEY_ASSETS_PREFIX + KEY_CONFIG_BOOLEAN_ITEMS,
            KEY_ASSETS_PREFIX + KEY_CONFIG_INT_ITEMS,
            KEY_ASSETS_PREFIX + KEY_CONFIG_STRING_ITEMS,
            KEY_ASSETS_PREFIX + KEY_CONFIG_INT_ARRAY_ITEMS,
            KEY_ASSETS_PREFIX + KEY_CONFIG_STRING_ARRAY_ITEMS
        };
    private static final String KEY_EDIT_TEXT_PREFERENCES[] =
        {
            KEY_CONFIG_BOOLEAN_VALUE,
            KEY_CONFIG_INT_VALUE,
            KEY_CONFIG_STRING_VALUE,
            KEY_CONFIG_INT_ARRAY_VALUE,
            KEY_CONFIG_STRING_ARRAY_VALUE,
            KEY_ASSETS_PREFIX + KEY_CONFIG_BOOLEAN_VALUE,
            KEY_ASSETS_PREFIX + KEY_CONFIG_INT_VALUE,
            KEY_ASSETS_PREFIX + KEY_CONFIG_STRING_VALUE,
            KEY_ASSETS_PREFIX + KEY_CONFIG_INT_ARRAY_VALUE,
            KEY_ASSETS_PREFIX + KEY_CONFIG_STRING_ARRAY_VALUE
        };

    private static final int CONFIG_I_BOOLEAN = 0;
    private static final int CONFIG_I_INT = 1;
    private static final int CONFIG_I_STRING = 2;
    private static final int CONFIG_I_INT_ARRAY = 3;
    private static final int CONFIG_I_STRING_ARRAY = 4;
    private static final int ASSETS_CONFIG_I_BOOLEAN = 5;
    private static final int ASSETS_CONFIG_I_INT = 6;
    private static final int ASSETS_CONFIG_I_STRING = 7;
    private static final int ASSETS_CONFIG_I_INT_ARRAY = 8;
    private static final int ASSETS_CONFIG_I_STRING_ARRAY = 9;
    private static final int CONFIG_I_MAX = 10;

    private static SparseArray<ArraySet<String>> sConfigKeys = null;

    private int mSlotId = 0;

    private PersistableBundle mTestConfig;
    private final SparseArray<ListPreference> mConfigItems = new SparseArray<>(CONFIG_I_MAX);
    private final SparseArray<EditTextPreference> mConfigValues = new SparseArray<>(CONFIG_I_MAX);
    private ListPreference mClearTestConfig;
    private ListPreference mTestCarrierId;
    private ListPreference mVoLteProvisioning;
    private boolean mConfigChanged = false;

    @SuppressWarnings("deprecation")
    @Override
    public void onCreate(Bundle savedInstanceState) {
        ImsLog.d("");

        super.onCreate(savedInstanceState);

        Window wd = getWindow();

        if (wd != null) {
            LayoutParams layoutParams = wd.getAttributes();
            wd.setAttributes(layoutParams);
        }

        mSlotId = getIntent().getIntExtra(
                MSimUtils.EXTRA_KEY_SLOT_ID,
                MSimUtils.INVALID_SLOT_ID);

        if (mSlotId < 0) {
            ImsLog.d("Invalid slot-id=" + mSlotId);
        }

        addPreferencesFromResource(R.xml.carrier_config_menu);

        initTestConfig();

        initPreferences();
    }

    @Override
    public void onPause() {
        super.onPause();

        ImsLog.d(mSlotId, "CarrierConfigMenu: onPause");

        boolean isConfigChanged = mConfigChanged;

        storeTestConfig();

        if (isConfigChanged) {
            ConfigLoader.notifyCarrierConfigChanged(mSlotId);
        }
    }

    @Override
    public void onResume() {
        super.onResume();

        ImsLog.d(mSlotId, "CarrierConfigMenu: onResume");

        initTestConfig();
    }

    @Override
    protected boolean isValidFragment(String fragmentName) {
        return fragmentName != null;
    }

    private void initTestConfig() {
        if (mTestConfig != null) {
            return;
        }

        ConfigInterface config = AgentFactory.getInstance().getAgent(
                ConfigInterface.class, mSlotId);

        if (config != null) {
            mTestConfig = config.readTestConfig();
        } else {
            mTestConfig = new PersistableBundle();
        }
    }

    private void storeTestConfig() {
        if (mTestConfig == null) {
            return;
        }

        if (!mConfigChanged) {
            return;
        }

        ConfigInterface config = AgentFactory.getInstance().getAgent(
                ConfigInterface.class, mSlotId);

        if (config != null) {
            if (config.writeTestConfig(mTestConfig)) {
                mConfigChanged = false;
                ImsLog.d(mSlotId, "storeTestConfig: success.");
            }
        }
    }

    private void initPreferences() {
        for (int i = 0; i < CONFIG_I_MAX; ++i) {
            ListPreference itemList = (ListPreference)
                    findPreference(KEY_LIST_PREFERENCES[i]);
            mConfigItems.put(i, itemList);

            if (itemList != null) {
                itemList.setValue("");
                itemList.setOnPreferenceChangeListener(new ConfigItemChangeListener(i));
            }
        }

        initConfigItems();

        for (int i = 0; i < CONFIG_I_MAX; ++i) {
            EditTextPreference valueEdit = (EditTextPreference)
                    findPreference(KEY_EDIT_TEXT_PREFERENCES[i]);
            mConfigValues.put(i, valueEdit);

            if (valueEdit != null) {
                valueEdit.setText("");
                valueEdit.setOnPreferenceChangeListener(new ConfigValueChangeListener(i));
            }
        }

        mClearTestConfig = (ListPreference) findPreference(KEY_CLEAR_TEST_CONFIG);

        if (mClearTestConfig != null) {
            mClearTestConfig.setValue("");
            mClearTestConfig.setOnPreferenceChangeListener(new ListItemChangeListener());
        }

        mTestCarrierId = (ListPreference) findPreference(KEY_TEST_CARRIER_ID);

        if (mTestCarrierId != null) {
            int carrierId = ImsPrivateProperties.Persistent.getInt(
                    ImsPrivateProperties.Persistent.KEY_TEST_CARRIER_ID, mSlotId);

            mTestCarrierId.setValue(String.valueOf(carrierId));
            mTestCarrierId.setSummary(mTestCarrierId.getEntry());
            mTestCarrierId.setOnPreferenceChangeListener(new ListItemChangeListener());
        }

        mVoLteProvisioning = (ListPreference) findPreference(KEY_VOLTE_PROVISIONING);

        if (mVoLteProvisioning != null) {
            mVoLteProvisioning.setValue("-1");
            mVoLteProvisioning.setOnPreferenceChangeListener(new ListItemChangeListener());
        }
    }

    private final class ListItemChangeListener
            implements Preference.OnPreferenceChangeListener {
        @Override
        public boolean onPreferenceChange(Preference preference, Object newValue) {
            String value = newValue.toString();

            if (KEY_CLEAR_TEST_CONFIG.equals(preference.getKey())) {
                if ("1".equals(value)) {
                    mConfigChanged = false;

                    if (mTestConfig != null) {
                        mTestConfig.clear();
                    }

                    ConfigInterface config = AgentFactory.getInstance().getAgent(
                            ConfigInterface.class, mSlotId);

                    if (config != null) {
                        config.writeTestConfig(mTestConfig);
                    }

                    ImsLog.d(mSlotId, "CarrierConfig: test config cleared.");
                }
            } else if (KEY_TEST_CARRIER_ID.equals(preference.getKey())) {
                mTestCarrierId.setValue(value);
                mTestCarrierId.setSummary(mTestCarrierId.getEntry());

                int oldCarrierId = ImsPrivateProperties.Persistent.getInt(
                        ImsPrivateProperties.Persistent.KEY_TEST_CARRIER_ID, mSlotId);
                int newCarrierId = parseInt(value, 0);

                if (newCarrierId != oldCarrierId) {
                    ImsLog.d(mSlotId, "TestCarrierId: " + oldCarrierId + " >> " + newCarrierId);
                    ImsPrivateProperties.Persistent.setInt(
                            ImsPrivateProperties.Persistent.KEY_TEST_CARRIER_ID,
                            newCarrierId, mSlotId);
                }
            } else if (KEY_VOLTE_PROVISIONING.equals(preference.getKey())) {
                mVoLteProvisioning.setValue(value);
                mVoLteProvisioning.setSummary(mVoLteProvisioning.getEntry());

                int provisioningStatus = parseInt(value, 0);
                int subId = MSimUtils.getSubId(mSlotId);

                ConfigInterface config = AgentFactory.getInstance().getAgent(
                        ConfigInterface.class, mSlotId);
                CarrierConfig cc = (config != null) ? config.getCarrierConfig() : null;

                if (cc != null && cc.getBoolean(
                        CarrierConfigManager.KEY_CARRIER_VOLTE_PROVISIONING_REQUIRED_BOOL)) {
                    ImsLog.d(mSlotId, "VoLte-Provisioning-Required: subId="
                            + subId + ", status=" + provisioningStatus);

                    ImsManager imsMngr =
                            AppContext.getInstance().getSystemService(ImsManager.class);

                    if (imsMngr != null) {
                        try {
                            ProvisioningManager pm = imsMngr.getProvisioningManager(subId);

                            if (pm != null) {
                                pm.setProvisioningIntValue(
                                        ProvisioningManager.KEY_VOLTE_PROVISIONING_STATUS,
                                        provisioningStatus);
                                pm.setProvisioningIntValue(
                                        ProvisioningManager.KEY_VT_PROVISIONING_STATUS,
                                        provisioningStatus);
                                pm.setProvisioningIntValue(
                                        ProvisioningManager.KEY_VOICE_OVER_WIFI_ENABLED_OVERRIDE,
                                        provisioningStatus);
                            }
                        } catch (Throwable t) {
                            ImsLog.e(mSlotId, "setProvisioningValue: " + t);
                        }
                    }
                }
            }

            return true;
        }
    }

    private final class ConfigItemChangeListener
            implements Preference.OnPreferenceChangeListener {
        private final int mConfigType;

        public ConfigItemChangeListener(int configType) {
            mConfigType = configType;
        }

        @Override
        public boolean onPreferenceChange(Preference preference, Object newValue) {
            String key = newValue.toString();

            ImsLog.d(mSlotId, "onConfigItemChanged: key=" + key);

            ConfigInterface config = AgentFactory.getInstance().getAgent(
                    ConfigInterface.class, mSlotId);
            String value = "";

            if (config != null) {
                CarrierConfig cc = config.getCarrierConfig();

                if (cc != null) {
                    switch (mConfigType) {
                        case CONFIG_I_BOOLEAN: // FALL-THROUGH
                        case ASSETS_CONFIG_I_BOOLEAN: {
                            boolean booleanValue = cc.getBoolean(key);
                            value = String.valueOf(booleanValue);
                            break;
                        }
                        case CONFIG_I_INT: // FALL-THROUGH
                        case ASSETS_CONFIG_I_INT: {
                            int intValue = cc.getInt(key);

                            if (intValue >= 0) {
                                value = String.valueOf(intValue);
                            }
                            break;
                        }
                        case CONFIG_I_STRING: // FALL-THROUGH
                        case ASSETS_CONFIG_I_STRING: {
                            String stringValue = cc.getString(key);

                            if (stringValue != null) {
                                value = stringValue;
                            }
                            break;
                        }
                        case CONFIG_I_INT_ARRAY: // FALL-THROUGH
                        case ASSETS_CONFIG_I_INT_ARRAY: {
                            int[] intArrayValue = cc.getIntArray(key);

                            if (intArrayValue != null) {
                                value = Arrays.toString(intArrayValue);
                            }
                            break;
                        }
                        case CONFIG_I_STRING_ARRAY: // FALL-THROUGH
                        case ASSETS_CONFIG_I_STRING_ARRAY: {
                            String[] stringArrayValue = cc.getStringArray(key);

                            if (stringArrayValue != null) {
                                value = Arrays.toString(stringArrayValue);
                            }
                            break;
                        }
                    }
                }
            }

            ListPreference itemList = mConfigItems.valueAt(mConfigType);

            if (itemList != null) {
                itemList.setSummary(key);
            }

            EditTextPreference valueEdit = mConfigValues.valueAt(mConfigType);

            if (valueEdit != null) {
                valueEdit.setDialogMessage(key);
                valueEdit.setSummary(value);

                if (value.equals("[]")) {
                    valueEdit.setText("");
                } else if (value.startsWith("[") && value.endsWith("]")) {
                    valueEdit.setText(value.substring(1, value.length() - 1));
                } else {
                    valueEdit.setText(value);
                }
            }

            return true;
        }
    }

    private final class ConfigValueChangeListener
            implements Preference.OnPreferenceChangeListener {
        private final int mConfigType;

        public ConfigValueChangeListener(int configType) {
            mConfigType = configType;
        }

        @Override
        public boolean onPreferenceChange(Preference preference, Object newValue) {
            ListPreference itemList = mConfigItems.valueAt(mConfigType);
            String key = (itemList != null) ? itemList.getValue() : null;
            String newConfigValue = newValue.toString();

            ImsLog.d(mSlotId, "onConfigValueChanged: value=" + newConfigValue + ", key=" + key);

            if (key == null) {
                Toast.makeText(AppContext.getInstance(), "CarrierConfig: key is invalid",
                        Toast.LENGTH_LONG).show();
                return false;
            }

            ConfigInterface config = AgentFactory.getInstance().getAgent(
                    ConfigInterface.class, mSlotId);

            if (config != null) {
                CarrierConfig cc = config.getCarrierConfig();

                switch (mConfigType) {
                    case CONFIG_I_BOOLEAN: // FALL-THROUGH
                    case ASSETS_CONFIG_I_BOOLEAN: {
                        boolean value = Boolean.valueOf(newConfigValue);
                        cc.getConfig().putBoolean(key, value);
                        mTestConfig.putBoolean(key, value);
                        break;
                    }
                    case CONFIG_I_INT: // FALL-THROUGH
                    case ASSETS_CONFIG_I_INT: {
                        int value = parseInt(newConfigValue, -1);
                        cc.getConfig().putInt(key, value);
                        mTestConfig.putInt(key, value);
                        break;
                    }
                    case CONFIG_I_STRING: // FALL-THROUGH
                    case ASSETS_CONFIG_I_STRING: {
                        cc.getConfig().putString(key, newConfigValue);
                        mTestConfig.putString(key, newConfigValue);
                        break;
                    }
                    case CONFIG_I_INT_ARRAY: // FALL-THROUGH
                    case ASSETS_CONFIG_I_INT_ARRAY: {
                        int[] value = toIntArray(newConfigValue);
                        cc.getConfig().putIntArray(key, value);
                        mTestConfig.putIntArray(key, value);
                        break;
                    }
                    case CONFIG_I_STRING_ARRAY: // FALL-THROUGH
                    case ASSETS_CONFIG_I_STRING_ARRAY: {
                        String[] value = toStringArray(newConfigValue);
                        cc.getConfig().putStringArray(key, value);
                        mTestConfig.putStringArray(key, value);
                        break;
                    }
                }

                mConfigChanged = true;
            }

            EditTextPreference valueEdit = mConfigValues.valueAt(mConfigType);

            if (valueEdit != null) {
                if (mConfigType == CONFIG_I_INT_ARRAY
                        || mConfigType == ASSETS_CONFIG_I_INT_ARRAY
                        || mConfigType == CONFIG_I_STRING_ARRAY
                        || mConfigType == ASSETS_CONFIG_I_STRING_ARRAY) {
                    valueEdit.setSummary("[" + newConfigValue + "]");
                } else {
                    valueEdit.setSummary(newConfigValue);
                }
            }

            return true;
        }
    }

    private void initConfigItems() {
        SparseArray<ArraySet<String>> configKeys = getConfigKeys();

        if (configKeys != null) {
            for (int i = 0; i < CONFIG_I_MAX; ++i) {
                ArraySet<String> keys = configKeys.valueAt(i);
                String[] entries = keys.toArray(new String[0]);

                ListPreference itemList = mConfigItems.valueAt(i);

                if (itemList != null) {
                    itemList.setEntries(entries);
                    itemList.setEntryValues(entries);
                }
            }
        }
    }

    private static SparseArray<ArraySet<String>> getConfigKeys() {
        if (sConfigKeys == null) {
            ArraySet<String> publicConfigKeys = new ArraySet<>();
            ArraySet<String> assetsConfigKeys = new ArraySet<>();

            readConfigKeys(PUBLIC_CARRIER_CONFIG_FILE, publicConfigKeys);
            readConfigKeys(CarrierConfig.DEFAULT_CARRIER_CONFIG_FILE, assetsConfigKeys);

            ImsLog.i("ConfigKeys: public=" + publicConfigKeys.size() +
                    ", assets=" + assetsConfigKeys.size());

            sConfigKeys = new SparseArray<>();

            for (int i = 0; i < CONFIG_I_MAX; ++i) {
                sConfigKeys.put(i, new ArraySet<>());
            }

            distributeConfigKeys(publicConfigKeys,
                    sConfigKeys.valueAt(CONFIG_I_BOOLEAN),
                    sConfigKeys.valueAt(CONFIG_I_INT),
                    sConfigKeys.valueAt(CONFIG_I_STRING),
                    sConfigKeys.valueAt(CONFIG_I_INT_ARRAY),
                    sConfigKeys.valueAt(CONFIG_I_STRING_ARRAY));

            distributeConfigKeys(assetsConfigKeys,
                    sConfigKeys.valueAt(ASSETS_CONFIG_I_BOOLEAN),
                    sConfigKeys.valueAt(ASSETS_CONFIG_I_INT),
                    sConfigKeys.valueAt(ASSETS_CONFIG_I_STRING),
                    sConfigKeys.valueAt(ASSETS_CONFIG_I_INT_ARRAY),
                    sConfigKeys.valueAt(ASSETS_CONFIG_I_STRING_ARRAY));
        }

        return sConfigKeys;
    }

    private static void readConfigKeys(String fileName, ArraySet<String> configKeys) {
        InputStream is = null;

        try {
            is = AppContext.getInstance().getAssets().open(fileName);

            XmlPullParserFactory factory = XmlPullParserFactory.newInstance();
            XmlPullParser parser = factory.newPullParser();
            parser.setInput(is, "utf-8");

            int event;

            while (((event = parser.next()) != XmlPullParser.END_DOCUMENT)) {
                if (event == XmlPullParser.START_TAG
                        && "carrier_config".equals(parser.getName())) {
                    ConfigXmlUtils.readConfigKeys(parser, configKeys);
                }
            }
        } catch (IOException | XmlPullParserException e) {
            ImsLog.e("readConfigKeys: " + e.toString());
        } finally {
            IoUtils.closeQuietly(is);
        }
    }

    private static void distributeConfigKeys(ArraySet<String> configKeys,
            Set<String> booleanKeys, Set<String> intKeys, Set<String> stringKeys,
            Set<String> intArrayKeys, Set<String> stringArrayKeys) {
        for (int i = 0; i < configKeys.size(); ++i) {
            String key = configKeys.valueAt(i);

            if (key.endsWith("_string")) {
                stringKeys.add(key);
            } else if (key.endsWith("_string_array")) {
                stringArrayKeys.add(key);
            } else if (key.endsWith("_int")) {
                intKeys.add(key);
            } else if (key.endsWith("_bool") || key.endsWith("_boolean")) {
                booleanKeys.add(key);
            } else if (key.endsWith("_int_array")) {
                intArrayKeys.add(key);
            }
        }
    }

    private static int[] toIntArray(String value) {
        String[] stringArray = toStringArray(value);
        int[] intArray = new int[stringArray.length];

        for (int i = 0; i < intArray.length; ++i) {
            intArray[i] = parseInt(stringArray[i], -1);
        }

        return intArray;
    }

    private static String[] toStringArray(String value) {
        String[] stringArray = value.split("\\s*,\\s*");

        if (stringArray == null) {
            return new String[0];
        }

        return stringArray;
    }

    private static int parseInt(String value, int defaultValue) {
        int intValue = defaultValue;

        try {
            intValue = Integer.parseInt(value);
        } catch (NumberFormatException e) {
            ImsLog.w("parseInt: value=" + value + ", " + e);
        }

        return intValue;
    }
}
