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

import android.annotation.NonNull;
import android.os.Bundle;
import android.os.PersistableBundle;
import android.preference.EditTextPreference;
import android.preference.ListPreference;
import android.preference.Preference;
import android.telephony.CarrierConfigManager;
import android.telephony.CarrierConfigManager.ImsRtt;
import android.telephony.CarrierConfigManager.ImsVoice;
import android.telephony.CarrierConfigManager.ImsVt;
import android.telephony.ims.ProvisioningManager;
import android.text.TextUtils;
import android.util.SparseArray;
import android.view.Window;
import android.view.WindowManager.LayoutParams;

import com.android.imsstack.R;
import com.android.imsstack.ServiceLoader;
import com.android.imsstack.base.AppContext;
import com.android.imsstack.base.ImsPrivateProperties;
import com.android.imsstack.base.MSimUtils;
import com.android.imsstack.base.SystemServiceProxy.ImsManagerProxy;
import com.android.imsstack.base.SystemServiceProxy.ProvisioningManagerProxy;
import com.android.imsstack.core.agents.AgentFactory;
import com.android.imsstack.core.agents.ConfigInterface;
import com.android.imsstack.core.config.CarrierConfig;
import com.android.imsstack.core.config.CarrierConfig.Ims;
import com.android.imsstack.core.config.CarrierConfig.ImsWfc;
import com.android.imsstack.core.config.ConfigXmlUtils;
import com.android.imsstack.util.ImsLog;
import com.android.imsstack.util.Log;
import com.android.internal.annotations.VisibleForTesting;

import org.xmlpull.v1.XmlPullParser;
import org.xmlpull.v1.XmlPullParserException;
import org.xmlpull.v1.XmlPullParserFactory;

import java.io.IOException;
import java.io.InputStream;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.Collection;
import java.util.List;
import java.util.Map;
import java.util.Set;

public class CarrierConfigMenu extends AppCompatActivity {
    private static final String PUBLIC_CARRIER_CONFIG_FILE =
            "carrier_config/aosp_carrier_config.xml";

    protected static final String KEY_DUMP_CONFIG = "dump_config";
    protected static final String KEY_CLEAR_TEST_CONFIG = "clear_test_config";
    protected static final String KEY_TEST_CARRIER_ID = "test_carrier_id";
    protected static final String KEY_TEST_ENTERED_CARRIER_ID = "test_entered_carrier_id";
    protected static final String KEY_TEST_SPECIFIC_CARRIER_ID = "test_specific_carrier_id";
    protected static final String KEY_VOLTE_PROVISIONING = "volte_provisioning";
    protected static final String KEY_ASSETS_PREFIX = "assets_";
    protected static final String KEY_CONFIG_BOOLEAN_ITEMS = "config_boolean_items";
    protected static final String KEY_CONFIG_BOOLEAN_VALUE = "config_boolean_value";
    protected static final String KEY_CONFIG_INT_ITEMS = "config_int_items";
    protected static final String KEY_CONFIG_INT_VALUE = "config_int_value";
    protected static final String KEY_CONFIG_STRING_ITEMS = "config_string_items";
    protected static final String KEY_CONFIG_STRING_VALUE = "config_string_value";
    protected static final String KEY_CONFIG_INT_ARRAY_ITEMS = "config_int_array_items";
    protected static final String KEY_CONFIG_INT_ARRAY_VALUE = "config_int_array_value";
    protected static final String KEY_CONFIG_STRING_ARRAY_ITEMS = "config_string_array_items";
    protected static final String KEY_CONFIG_STRING_ARRAY_VALUE = "config_string_array_value";
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
    private static final Map<Integer, String> KEY_LIST_PREFERENCES = Map.ofEntries(
            Map.entry(CONFIG_I_BOOLEAN, KEY_CONFIG_BOOLEAN_ITEMS),
            Map.entry(CONFIG_I_INT, KEY_CONFIG_INT_ITEMS),
            Map.entry(CONFIG_I_STRING, KEY_CONFIG_STRING_ITEMS),
            Map.entry(CONFIG_I_INT_ARRAY, KEY_CONFIG_INT_ARRAY_ITEMS),
            Map.entry(CONFIG_I_STRING_ARRAY, KEY_CONFIG_STRING_ARRAY_ITEMS),
            Map.entry(ASSETS_CONFIG_I_BOOLEAN, KEY_ASSETS_PREFIX + KEY_CONFIG_BOOLEAN_ITEMS),
            Map.entry(ASSETS_CONFIG_I_INT, KEY_ASSETS_PREFIX + KEY_CONFIG_INT_ITEMS),
            Map.entry(ASSETS_CONFIG_I_STRING, KEY_ASSETS_PREFIX + KEY_CONFIG_STRING_ITEMS),
            Map.entry(ASSETS_CONFIG_I_INT_ARRAY, KEY_ASSETS_PREFIX + KEY_CONFIG_INT_ARRAY_ITEMS),
            Map.entry(ASSETS_CONFIG_I_STRING_ARRAY,
                    KEY_ASSETS_PREFIX + KEY_CONFIG_STRING_ARRAY_ITEMS));
    private static final Map<Integer, String> KEY_EDIT_TEXT_PREFERENCES = Map.ofEntries(
            Map.entry(CONFIG_I_BOOLEAN, KEY_CONFIG_BOOLEAN_VALUE),
            Map.entry(CONFIG_I_INT, KEY_CONFIG_INT_VALUE),
            Map.entry(CONFIG_I_STRING, KEY_CONFIG_STRING_VALUE),
            Map.entry(CONFIG_I_INT_ARRAY, KEY_CONFIG_INT_ARRAY_VALUE),
            Map.entry(CONFIG_I_STRING_ARRAY, KEY_CONFIG_STRING_ARRAY_VALUE),
            Map.entry(ASSETS_CONFIG_I_BOOLEAN, KEY_ASSETS_PREFIX + KEY_CONFIG_BOOLEAN_VALUE),
            Map.entry(ASSETS_CONFIG_I_INT, KEY_ASSETS_PREFIX + KEY_CONFIG_INT_VALUE),
            Map.entry(ASSETS_CONFIG_I_STRING, KEY_ASSETS_PREFIX + KEY_CONFIG_STRING_VALUE),
            Map.entry(ASSETS_CONFIG_I_INT_ARRAY, KEY_ASSETS_PREFIX + KEY_CONFIG_INT_ARRAY_VALUE),
            Map.entry(ASSETS_CONFIG_I_STRING_ARRAY,
                    KEY_ASSETS_PREFIX + KEY_CONFIG_STRING_ARRAY_VALUE));

    protected static final String KEY_CONFIG_BUNDLE_KEYS = "config_bundle_keys";
    protected static final String KEY_CONFIG_BUNDLE_ITEMS = "config_bundle_items";
    protected static final String KEY_CONFIG_BUNDLE_VALUE = "config_bundle_value";
    private static final int CONFIG_I_BUNDLE = 0;
    private static final int ASSETS_CONFIG_I_BUNDLE = 1;
    private static final int CONFIG_I_BUNDLE_MAX = 2;
    private static final Map<Integer, String> KEY_BUNDLE_KEY_LIST_PREFERENCES = Map.of(
            CONFIG_I_BUNDLE, KEY_CONFIG_BUNDLE_KEYS,
            ASSETS_CONFIG_I_BUNDLE, KEY_ASSETS_PREFIX + KEY_CONFIG_BUNDLE_KEYS);
    private static final Map<Integer, String> KEY_BUNDLE_LIST_PREFERENCES = Map.of(
            CONFIG_I_BUNDLE, KEY_CONFIG_BUNDLE_ITEMS,
            ASSETS_CONFIG_I_BUNDLE, KEY_ASSETS_PREFIX + KEY_CONFIG_BUNDLE_ITEMS);
    private static final Map<Integer, String> KEY_BUNDLE_EDIT_TEXT_PREFERENCES = Map.of(
            CONFIG_I_BUNDLE, KEY_CONFIG_BUNDLE_VALUE,
            ASSETS_CONFIG_I_BUNDLE, KEY_ASSETS_PREFIX + KEY_CONFIG_BUNDLE_VALUE);
    private static final List<String> ASSETS_BUNDLE_KEYS = Arrays.asList(
            Ims.KEY_EXTRA_REG_ERR_BUNDLE,
            Ims.KEY_NOTIFY_TERMINATED_FOR_INIT_REG_BUNDLE,
            Ims.KEY_PCSCF_RECOVERY_CONDITIONS_BUNDLE,
            Ims.KEY_REG_ERR_CODE_WITH_RA_TIME_BUNDLE,
            Ims.KEY_REG_RETRY_INTERVAL_BUNDLE,
            Ims.KEY_SUB_ERR_CODE_FOR_INIT_REG_BUNDLE,
            Ims.KEY_SUB_ERR_CODE_FOR_TERMINATED_BUNDLE,
            ImsWfc.KEY_WFC_ERR_MESSAGE_BUNDLE);
    private static final Map<String, List<String>> BUNDLE_ITEMS_MAP = Map.ofEntries(
            Map.entry(ImsVoice.KEY_AUDIO_CODEC_CAPABILITY_PAYLOAD_TYPES_BUNDLE, Arrays.asList(
                    ImsVoice.KEY_EVS_PAYLOAD_TYPE_INT_ARRAY,
                    ImsVoice.KEY_AMRWB_PAYLOAD_TYPE_INT_ARRAY,
                    ImsVoice.KEY_AMRNB_PAYLOAD_TYPE_INT_ARRAY,
                    ImsVoice.KEY_DTMFWB_PAYLOAD_TYPE_INT_ARRAY,
                    ImsVoice.KEY_DTMFNB_PAYLOAD_TYPE_INT_ARRAY)),
            Map.entry(ImsVt.KEY_VIDEO_CODEC_CAPABILITY_PAYLOAD_TYPES_BUNDLE,
                    Arrays.asList(CarrierConfig.ImsVt.KEY_HEVC_PAYLOAD_TYPE_INT_ARRAY,
                    ImsVt.KEY_H264_PAYLOAD_TYPE_INT_ARRAY)),
            Map.entry(ImsRtt.KEY_TEXT_CODEC_CAPABILITY_PAYLOAD_TYPES_BUNDLE, Arrays.asList(
                    ImsRtt.KEY_T140_PAYLOAD_TYPE_INT, ImsRtt.KEY_RED_PAYLOAD_TYPE_INT)),
            Map.entry(ImsVoice.KEY_EVS_PAYLOAD_DESCRIPTION_BUNDLE, Arrays.asList(
                    ImsVoice.KEY_EVS_CODEC_ATTRIBUTE_BANDWIDTH_INT,
                    ImsVoice.KEY_EVS_CODEC_ATTRIBUTE_BITRATE_INT_ARRAY,
                    ImsVoice.KEY_EVS_CODEC_ATTRIBUTE_CH_AW_RECV_INT,
                    ImsVoice.KEY_EVS_CODEC_ATTRIBUTE_HF_ONLY_INT,
                    ImsVoice.KEY_EVS_CODEC_ATTRIBUTE_DTX_BOOL,
                    ImsVoice.KEY_EVS_CODEC_ATTRIBUTE_DTX_RECV_BOOL,
                    ImsVoice.KEY_EVS_CODEC_ATTRIBUTE_MODE_SWITCH_INT,
                    ImsVoice.KEY_EVS_CODEC_ATTRIBUTE_CMR_INT,
                    ImsVoice.KEY_EVS_CODEC_ATTRIBUTE_CHANNELS_INT,
                    ImsVoice.KEY_CODEC_ATTRIBUTE_MODE_CHANGE_PERIOD_INT,
                    ImsVoice.KEY_CODEC_ATTRIBUTE_MODE_CHANGE_CAPABILITY_INT,
                    ImsVoice.KEY_CODEC_ATTRIBUTE_MODE_CHANGE_NEIGHBOR_INT)),
            Map.entry(ImsVoice.KEY_AMRWB_PAYLOAD_DESCRIPTION_BUNDLE, Arrays.asList(
                    ImsVoice.KEY_AMR_CODEC_ATTRIBUTE_PAYLOAD_FORMAT_INT,
                    ImsVoice.KEY_AMR_CODEC_ATTRIBUTE_MODESET_INT_ARRAY,
                    ImsVoice.KEY_CODEC_ATTRIBUTE_MODE_CHANGE_PERIOD_INT,
                    ImsVoice.KEY_CODEC_ATTRIBUTE_MODE_CHANGE_CAPABILITY_INT,
                    ImsVoice.KEY_CODEC_ATTRIBUTE_MODE_CHANGE_NEIGHBOR_INT)),
            Map.entry(ImsVoice.KEY_AMRNB_PAYLOAD_DESCRIPTION_BUNDLE, Arrays.asList(
                    ImsVoice.KEY_AMR_CODEC_ATTRIBUTE_PAYLOAD_FORMAT_INT,
                    ImsVoice.KEY_AMR_CODEC_ATTRIBUTE_MODESET_INT_ARRAY,
                    ImsVoice.KEY_CODEC_ATTRIBUTE_MODE_CHANGE_PERIOD_INT,
                    ImsVoice.KEY_CODEC_ATTRIBUTE_MODE_CHANGE_CAPABILITY_INT,
                    ImsVoice.KEY_CODEC_ATTRIBUTE_MODE_CHANGE_NEIGHBOR_INT)),
            Map.entry(ImsVt.KEY_H264_PAYLOAD_DESCRIPTION_BUNDLE, Arrays.asList(
                    ImsVt.KEY_H264_VIDEO_CODEC_ATTRIBUTE_PROFILE_LEVEL_ID_STRING,
                    ImsVt.KEY_VIDEO_CODEC_ATTRIBUTE_PACKETIZATION_MODE_INT,
                    ImsVt.KEY_VIDEO_CODEC_ATTRIBUTE_FRAME_RATE_INT,
                    ImsVt.KEY_VIDEO_CODEC_ATTRIBUTE_RESOLUTION_INT_ARRAY)),
            Map.entry(CarrierConfig.ImsVt.KEY_HEVC_PAYLOAD_DESCRIPTION_BUNDLE, Arrays.asList(
                    ImsVt.KEY_VIDEO_CODEC_ATTRIBUTE_PACKETIZATION_MODE_INT,
                    ImsVt.KEY_VIDEO_CODEC_ATTRIBUTE_FRAME_RATE_INT,
                    ImsVt.KEY_VIDEO_CODEC_ATTRIBUTE_RESOLUTION_INT_ARRAY,
                    CarrierConfig.ImsVt.KEY_HEVC_SPROP_PARAMETER_SETS_STRING,
                    CarrierConfig.ImsVt.KEY_HEVC_PROFILE_INT,
                    CarrierConfig.ImsVt.KEY_HEVC_LEVEL_INT)),
            Map.entry(Ims.KEY_EXTRA_REG_ERR_BUNDLE, Arrays.asList(
                    Ims.KEY_EXTRA_REG_ERR_CODE_AS_FAILURE_IN_ROAMING_FOR_UPDATE_BOOL,
                    Ims.KEY_EXTRA_REG_ERR_CODE_FOR_UPDATE_INT_ARRAY,
                    Ims.KEY_EXTRA_REG_ERR_CODE_INT_ARRAY,
                    Ims.KEY_EXTRA_REG_ERR_FINAL_TYPE_INT,
                    Ims.KEY_EXTRA_REG_ERR_MAX_CNT_INT,
                    Ims.KEY_EXTRA_REG_ERR_PCSCFS_REPEATED_CNT_FOR_EPS_5GS_ONLY_ATTACHED_INT,
                    Ims.KEY_EXTRA_REG_ERR_PCSCFS_REPEATED_CNT_FOR_LTE_COMBINED_ATTACHED_INT,
                    Ims.KEY_EXTRA_REG_ERR_POLICY_INT,
                    Ims.KEY_EXTRA_REG_ERR_RETRY_CNT_SHARED_FOR_REG_AND_SUB_BOOL,
                    Ims.KEY_EXTRA_REG_ERR_WAIT_TIME_SEC_INT_ARRAY)),
            Map.entry(Ims.KEY_NOTIFY_TERMINATED_FOR_INIT_REG_BUNDLE, Arrays.asList(
                    Ims.KEY_NOTIFY_TERMINATED_FOR_INIT_REG_USED_EVENT_INT_ARRAY,
                    Ims.KEY_NOTIFY_TERMINATED_FOR_INIT_REG_USED_EVENT_WITH_WAIT_TIME_INT_ARRAY,
                    Ims.KEY_NOTIFY_TERMINATED_FOR_INIT_REG_WITH_WAIT_TIME_INT)),
            Map.entry(Ims.KEY_PCSCF_RECOVERY_CONDITIONS_BUNDLE, Arrays.asList(
                    Ims.KEY_PCSCF_RECOVERY_MAX_CNT_INT,
                    Ims.KEY_PCSCF_RECOVERY_WAIT_TIME_SEC_INT,
                    Ims.KEY_PCSCF_RECOVERY_BASE_TIME_SEC_INT,
                    Ims.KEY_PCSCF_RECOVERY_MAX_TIME_SEC_INT)),
            Map.entry(Ims.KEY_REG_ERR_CODE_WITH_RA_TIME_BUNDLE, Arrays.asList(
                    Ims.KEY_REG_ERR_CODE_WITH_RA_TIME_FOR_UPDATE_INT_ARRAY,
                    Ims.KEY_REG_ERR_CODE_WITH_RA_TIME_INT_ARRAY,
                    Ims.KEY_REG_ERR_CODE_WITH_RA_TIME_ONLY_DEFINED_BOOL)),
            Map.entry(Ims.KEY_REG_RETRY_INTERVAL_BUNDLE, Arrays.asList(
                    Ims.KEY_REG_RETRY_INTERVAL_RANDOM_UPPER_VALUE_SEC_INT_ARRAY,
                    Ims.KEY_REG_RETRY_INTERVAL_SEC_INT_ARRAY,
                    Ims.KEY_REG_RETRY_INTERVAL_USED_FOR_SUB_BOOL)),
            Map.entry(Ims.KEY_SUB_ERR_CODE_FOR_INIT_REG_BUNDLE, Arrays.asList(
                    Ims.KEY_SUB_ERR_CODE_FOR_INIT_REG_INT_ARRAY,
                    Ims.KEY_SUB_ERR_CODE_FOR_INIT_REG_WITH_RETRY_MAX_CNT_INT)),
            Map.entry(Ims.KEY_SUB_ERR_CODE_FOR_TERMINATED_BUNDLE, Arrays.asList(
                    Ims.KEY_SUB_ERR_CODE_FOR_TERMINATED_INT_ARRAY,
                    Ims.KEY_SUB_ERR_CODE_FOR_TERMINATED_WITH_RETRY_MAX_CNT_INT)),
            Map.entry(ImsWfc.KEY_WFC_ERR_MESSAGE_BUNDLE, Arrays.asList(
                    ImsWfc.KEY_WFC_ERR_REG_403_STRING,
                    ImsWfc.KEY_WFC_ERR_REG_500_STRING,
                    ImsWfc.KEY_WFC_ERR_NOT_SUPPORTED_COUNTRY_STRING,
                    ImsWfc.KEY_WFC_ERR_SUB_403_STRING,
                    ImsWfc.KEY_WFC_ERR_NOTIFY_TERMINATED_STRING,
                    ImsWfc.KEY_WFC_ERR_OTHER_FAILURES_STRING)));
    private static final List<String> KEYS_OF_VALUE_USED_AS_BUNDLE_KEY = List.of(
            ImsVoice.KEY_EVS_PAYLOAD_TYPE_INT_ARRAY,
            ImsVoice.KEY_AMRWB_PAYLOAD_TYPE_INT_ARRAY,
            ImsVoice.KEY_AMRNB_PAYLOAD_TYPE_INT_ARRAY,
            CarrierConfig.ImsVt.KEY_HEVC_PAYLOAD_TYPE_INT_ARRAY,
            ImsVt.KEY_H264_PAYLOAD_TYPE_INT_ARRAY);

    private static SparseArray<List<String>> sConfigKeys = null;

    private int mSlotId = 0;

    private PersistableBundle mTestConfig;
    private final SparseArray<ListPreference> mConfigItems = new SparseArray<>(CONFIG_I_MAX);
    private final SparseArray<EditTextPreference> mConfigValues = new SparseArray<>(CONFIG_I_MAX);
    private final SparseArray<ListPreference> mConfigBundleKeys =
            new SparseArray<>(CONFIG_I_BUNDLE_MAX);
    private final SparseArray<ListPreference> mConfigBundleItems =
            new SparseArray<>(CONFIG_I_BUNDLE_MAX);
    private final SparseArray<EditTextPreference> mConfigBundleValues =
            new SparseArray<>(CONFIG_I_BUNDLE_MAX);
    private final List<String> mBundleKeys = new ArrayList<>();
    private ListPreference mClearTestConfig;
    private ListPreference mTestCarrierId;
    private EditTextPreference mTestEnteredCarrierId;
    private EditTextPreference mTestSpecificCarrierId;
    private ListPreference mVoLteProvisioning;
    private boolean mConfigChanged = false;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
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
    protected void onPause() {
        super.onPause();

        ImsLog.d(mSlotId, "CarrierConfigMenu: onPause");

        boolean isConfigChanged = isConfigChanged();

        storeTestConfig();

        if (isConfigChanged) {
            ServiceLoader.notifyCarrierConfigChanged(mSlotId);
        }
    }

    @Override
    protected void onResume() {
        super.onResume();

        ImsLog.d(mSlotId, "CarrierConfigMenu: onResume");

        initTestConfig();
    }

    @VisibleForTesting
    protected void setConfigChanged(boolean changed) {
        mConfigChanged = changed;
    }

    @VisibleForTesting
    protected boolean isConfigChanged() {
        return mConfigChanged;
    }

    @VisibleForTesting
    protected PersistableBundle getTestConfig() {
        return mTestConfig;
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

        createBundleKeys(false);
    }

    private void storeTestConfig() {
        if (!isConfigChanged()) {
            return;
        }

        ConfigInterface config = AgentFactory.getInstance().getAgent(
                ConfigInterface.class, mSlotId);

        if (config != null) {
            if (config.writeTestConfig(mTestConfig)) {
                setConfigChanged(false);
                ImsLog.d(mSlotId, "storeTestConfig: success.");
            }
        }
    }

    private void initPreferences() {
        for (int i = 0; i < CONFIG_I_MAX; ++i) {
            String key = KEY_LIST_PREFERENCES.get(i);
            if (key != null) {
                SearchableListPreference itemList = (SearchableListPreference) findPreference(key);
                mConfigItems.put(i, itemList);

                if (itemList != null) {
                    itemList.setValue("");
                    itemList.setOnPreferenceChangeListener(new ConfigItemChangeListener(i));
                }
            }
        }

        initConfigItems();

        for (int i = 0; i < CONFIG_I_MAX; ++i) {
            String key = KEY_EDIT_TEXT_PREFERENCES.get(i);
            if (key != null) {
                EditTextPreference valueEdit = (EditTextPreference) findPreference(key);
                mConfigValues.put(i, valueEdit);

                if (valueEdit != null) {
                    valueEdit.setText("");
                    valueEdit.setOnPreferenceChangeListener(new ConfigValueChangeListener(i));
                }
            }
        }

        for (int i = 0; i < CONFIG_I_BUNDLE_MAX; ++i) {
            String key = KEY_BUNDLE_KEY_LIST_PREFERENCES.get(i);
            if (key != null) {
                SearchableListPreference keyList = (SearchableListPreference) findPreference(key);
                mConfigBundleKeys.put(i, keyList);

                if (keyList != null) {
                    keyList.setValue("");
                    keyList.setOnPreferenceChangeListener(new ConfigBundleKeyChangeListener(i));
                }
            }
        }

        initBundleKeys();

        for (int i = 0; i < CONFIG_I_BUNDLE_MAX; ++i) {
            String key = KEY_BUNDLE_LIST_PREFERENCES.get(i);
            if (key != null) {
                ListPreference itemList = (ListPreference) findPreference(key);
                mConfigBundleItems.put(i, itemList);

                if (itemList != null) {
                    itemList.setValue("");
                    itemList.setOnPreferenceChangeListener(new ConfigBundleItemChangeListener(i));
                }
            }
        }

        for (int i = 0; i < CONFIG_I_BUNDLE_MAX; ++i) {
            String key = KEY_BUNDLE_EDIT_TEXT_PREFERENCES.get(i);
            if (key != null) {
                EditTextPreference valueEdit = (EditTextPreference) findPreference(key);
                mConfigBundleValues.put(i, valueEdit);

                if (valueEdit != null) {
                    valueEdit.setText("");
                    valueEdit.setOnPreferenceChangeListener(new ConfigBundleValueChangeListener(i));
                }
            }
        }

        Preference dumpConfig = findPreference(KEY_DUMP_CONFIG);
        if (dumpConfig != null) {
            dumpConfig.setOnPreferenceClickListener(new PreferenceClickListener());
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

        mTestEnteredCarrierId = (EditTextPreference) findPreference(KEY_TEST_ENTERED_CARRIER_ID);
        if (mTestEnteredCarrierId != null) {
            int carrierId = ImsPrivateProperties.Persistent.getInt(
                    ImsPrivateProperties.Persistent.KEY_TEST_CARRIER_ID, mSlotId);

            if (carrierId > 0) {
                mTestEnteredCarrierId.setText(String.valueOf(carrierId));
                mTestEnteredCarrierId.setSummary(String.valueOf(carrierId));
            } else {
                mTestEnteredCarrierId.setText("0");
                mTestEnteredCarrierId.setSummary("0");
            }
            mTestEnteredCarrierId.setOnPreferenceChangeListener(new EditTextItemChangeListener());
        }

        mTestSpecificCarrierId = (EditTextPreference) findPreference(KEY_TEST_SPECIFIC_CARRIER_ID);
        if (mTestSpecificCarrierId != null) {
            int specificCarrierId = ImsPrivateProperties.Persistent.getInt(
                    ImsPrivateProperties.Persistent.KEY_TEST_SPECIFIC_CARRIER_ID, mSlotId);

            if (specificCarrierId > 0) {
                mTestSpecificCarrierId.setText(String.valueOf(specificCarrierId));
                mTestSpecificCarrierId.setSummary(String.valueOf(specificCarrierId));
            } else {
                mTestSpecificCarrierId.setText("0");
                mTestSpecificCarrierId.setSummary("0");
            }
            mTestSpecificCarrierId.setOnPreferenceChangeListener(new EditTextItemChangeListener());
        }

        mVoLteProvisioning = (ListPreference) findPreference(KEY_VOLTE_PROVISIONING);
        if (mVoLteProvisioning != null) {
            mVoLteProvisioning.setValue("-1");
            mVoLteProvisioning.setOnPreferenceChangeListener(new ListItemChangeListener());
        }
    }

    private final class PreferenceClickListener implements Preference.OnPreferenceClickListener {
        @Override
        public boolean onPreferenceClick(@NonNull Preference preference) {
            if (KEY_DUMP_CONFIG.equals(preference.getKey())) {
                ConfigInterface config = AgentFactory.getInstance().getAgent(
                        ConfigInterface.class, mSlotId);
                CarrierConfig cc = (config != null) ? config.getCarrierConfig() : null;
                PersistableBundle pb = (cc != null) ? cc.getConfig() : null;

                if (pb != null) {
                    Set<String> keys = pb.keySet();
                    Log.i(this, "CarrierConfig(" + mSlotId + ") - starts");
                    for (String key : keys) {
                        Log.i(this, "CarrierConfig: " + key
                                + "=" + CarrierConfig.getValue(pb, key));
                    }
                    Log.i(this, "CarrierConfig(" + mSlotId + ") - ends");
                }
            }
            return true;
        }
    }

    private final class EditTextItemChangeListener
            implements Preference.OnPreferenceChangeListener {
        @Override
        public boolean onPreferenceChange(Preference preference, Object newValue) {
            String value = newValue.toString();

            if (KEY_TEST_SPECIFIC_CARRIER_ID.equals(preference.getKey())) {
                if (TextUtils.isEmpty(value)) {
                    value = "0";
                }
                mTestSpecificCarrierId.setText(value);
                mTestSpecificCarrierId.setSummary(value);

                int oldSpecificCarrierId = ImsPrivateProperties.Persistent.getInt(
                        ImsPrivateProperties.Persistent.KEY_TEST_SPECIFIC_CARRIER_ID, mSlotId);
                int newSpecificCarrierId = parseInt(value, 0);

                if (newSpecificCarrierId != oldSpecificCarrierId) {
                    ImsLog.d(mSlotId, "TestSpecificCarrierId: " + oldSpecificCarrierId
                            + " >> " + newSpecificCarrierId);
                    ImsPrivateProperties.Persistent.setInt(
                            ImsPrivateProperties.Persistent.KEY_TEST_SPECIFIC_CARRIER_ID,
                            newSpecificCarrierId, mSlotId);
                }
            } else if (KEY_TEST_ENTERED_CARRIER_ID.equals(preference.getKey())) {
                if (TextUtils.isEmpty(value)) {
                    value = "0";
                }
                mTestEnteredCarrierId.setText(value);
                mTestEnteredCarrierId.setSummary(value);

                mTestCarrierId.setValue(value);
                mTestCarrierId.setSummary(mTestCarrierId.getEntry());

                int oldCarrierId = ImsPrivateProperties.Persistent.getInt(
                        ImsPrivateProperties.Persistent.KEY_TEST_CARRIER_ID, mSlotId);
                int newCarrierId = parseInt(value, 0);

                if (newCarrierId != oldCarrierId) {
                    ImsLog.d(mSlotId, "TestEnteredCarrierId: " + oldCarrierId
                            + " >> " + newCarrierId);
                    ImsPrivateProperties.Persistent.setInt(
                            ImsPrivateProperties.Persistent.KEY_TEST_CARRIER_ID,
                            newCarrierId, mSlotId);
                }
            }
            return true;
        }
    }

    private final class ListItemChangeListener
            implements Preference.OnPreferenceChangeListener {
        @Override
        public boolean onPreferenceChange(@NonNull Preference preference, Object newValue) {
            String value = newValue.toString();

            if (KEY_CLEAR_TEST_CONFIG.equals(preference.getKey())) {
                if ("1".equals(value)) {
                    setConfigChanged(false);
                    mTestConfig.clear();

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

                mTestEnteredCarrierId.setText(value);
                mTestEnteredCarrierId.setSummary(value);

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

                if (cc != null && cc.isVoLteProvisioningRequired()) {
                    ImsLog.d(mSlotId, "VoLte-Provisioning-Required: subId="
                            + subId + ", status=" + provisioningStatus);

                    ImsManagerProxy imp =
                            AppContext.getInstance().getSystemServiceProxy(ImsManagerProxy.class);
                    try {
                        ProvisioningManagerProxy pmp = imp.getProvisioningManagerProxy(subId);

                        if (pmp != null) {
                            pmp.setProvisioningIntValue(
                                    ProvisioningManager.KEY_VOLTE_PROVISIONING_STATUS,
                                    provisioningStatus);
                            pmp.setProvisioningIntValue(
                                    ProvisioningManager.KEY_VT_PROVISIONING_STATUS,
                                    provisioningStatus);
                            pmp.setProvisioningIntValue(
                                    ProvisioningManager.KEY_VOICE_OVER_WIFI_ENABLED_OVERRIDE,
                                    provisioningStatus);
                        }
                    } catch (Throwable t) {
                        ImsLog.e(mSlotId, "setProvisioningValue: " + t);
                    }
                }
            }

            return true;
        }
    }

    private static class ConfigListener {
        protected final int mConfigType;

        ConfigListener(int configType) {
            mConfigType = configType;
        }
    }

    private final class ConfigItemChangeListener extends ConfigListener
            implements Preference.OnPreferenceChangeListener {
        ConfigItemChangeListener(int configType) {
            super(configType);
        }

        @Override
        public boolean onPreferenceChange(@NonNull Preference preference, Object newValue) {
            String key = newValue.toString();

            ImsLog.d(mSlotId, "onConfigItemChanged: key=" + key);

            ConfigInterface config = AgentFactory.getInstance().getAgent(
                    ConfigInterface.class, mSlotId);
            CarrierConfig cc = (config != null) ? config.getCarrierConfig() : null;
            String value = "";

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

    private final class ConfigValueChangeListener extends ConfigListener
            implements Preference.OnPreferenceChangeListener {
        ConfigValueChangeListener(int configType) {
            super(configType);
        }

        @Override
        public boolean onPreferenceChange(@NonNull Preference preference, Object newValue) {
            ListPreference itemList = mConfigItems.valueAt(mConfigType);
            String key = (itemList != null) ? itemList.getValue() : null;
            String newConfigValue = newValue.toString();

            ImsLog.d(mSlotId, "onConfigValueChanged: value=" + newConfigValue + ", key=" + key);

            if (key == null) {
                ImsLog.w(mSlotId, "CarrierConfig: key is invalid.");
                return false;
            }

            ConfigInterface config = AgentFactory.getInstance().getAgent(
                    ConfigInterface.class, mSlotId);
            CarrierConfig cc = (config != null) ? config.getCarrierConfig() : null;

            if (cc != null) {
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
                        newConfigValue = Arrays.toString(value);
                        break;
                    }
                    case CONFIG_I_STRING_ARRAY: // FALL-THROUGH
                    case ASSETS_CONFIG_I_STRING_ARRAY: {
                        String[] value = toStringArray(newConfigValue);
                        cc.getConfig().putStringArray(key, value);
                        mTestConfig.putStringArray(key, value);
                        newConfigValue = Arrays.toString(value);
                        break;
                    }
                    default:
                        ImsLog.e(mSlotId, "Unknown configuration type.");
                        return false;
                }

                setConfigChanged(true);
            }

            EditTextPreference valueEdit = mConfigValues.valueAt(mConfigType);

            if (valueEdit != null) {
                valueEdit.setSummary(newConfigValue);
            }

            return true;
        }
    }

    private final class ConfigBundleKeyChangeListener extends ConfigListener
            implements Preference.OnPreferenceChangeListener {
        ConfigBundleKeyChangeListener(int configType) {
            super(configType);
        }

        @Override
        public boolean onPreferenceChange(@NonNull Preference preference, Object newValue) {
            String key = newValue.toString();

            ImsLog.d(mSlotId, "onConfigBundleKeyChanged: key=" + key);

            ListPreference keyList = mConfigBundleKeys.valueAt(mConfigType);

            if (keyList != null) {
                keyList.setSummary(key);
            }

            ListPreference itemList = mConfigBundleItems.valueAt(mConfigType);

            if (itemList != null) {
                String rootKey = getFirstToken(key, "/");
                List<String> items = BUNDLE_ITEMS_MAP.get(rootKey);
                if (items != null) {
                    String[] entries = items.toArray(new String[0]);
                    itemList.setEntries(entries);
                    itemList.setEntryValues(entries);
                }
                itemList.setSummary("");
            }

            EditTextPreference valueEdit = mConfigBundleValues.valueAt(mConfigType);

            if (valueEdit != null) {
                valueEdit.setDialogMessage("");
                valueEdit.setSummary("");
                valueEdit.setText("");
            }

            return true;
        }
    }

    private final class ConfigBundleItemChangeListener extends ConfigListener
            implements Preference.OnPreferenceChangeListener {
        ConfigBundleItemChangeListener(int configType) {
            super(configType);
        }

        @Override
        public boolean onPreferenceChange(@NonNull Preference preference, Object newValue) {
            String key = newValue.toString();

            ImsLog.d(mSlotId, "onConfigBundleItemChanged: key=" + key);

            ListPreference keyList = mConfigBundleKeys.valueAt(mConfigType);
            String bundleKey = (keyList != null) ? keyList.getValue() : null;

            if (bundleKey == null) {
                ImsLog.w(mSlotId, "Bundle key is not found.");
                return false;
            }

            String rootBundleKey = getFirstToken(bundleKey, "/");
            String subBundleKey = getLastToken(bundleKey, "/");
            ConfigInterface config = AgentFactory.getInstance().getAgent(
                    ConfigInterface.class, mSlotId);
            CarrierConfig cc = (config != null) ? config.getCarrierConfig() : null;
            PersistableBundle pb = (cc != null) ? cc.getBundle(rootBundleKey) : null;

            if (pb != null && subBundleKey != null) {
                pb = pb.getPersistableBundle(subBundleKey);
            }

            String value = "";

            if (pb != null) {
                int configType = getConfigType(key);

                switch (configType) {
                    case CONFIG_I_BOOLEAN: {
                        boolean booleanValue = pb.getBoolean(key, false);
                        value = String.valueOf(booleanValue);
                        break;
                    }
                    case CONFIG_I_INT: {
                        int intValue = pb.getInt(key, -1);

                        if (intValue >= 0) {
                            value = String.valueOf(intValue);
                        }
                        break;
                    }
                    case CONFIG_I_STRING: {
                        String stringValue = pb.getString(key, null);

                        if (stringValue != null) {
                            value = stringValue;
                        }
                        break;
                    }
                    case CONFIG_I_INT_ARRAY: {
                        int[] intArrayValue = pb.getIntArray(key);

                        if (intArrayValue != null) {
                            value = Arrays.toString(intArrayValue);
                        }
                        break;
                    }
                    case CONFIG_I_STRING_ARRAY: {
                        String[] stringArrayValue = pb.getStringArray(key);

                        if (stringArrayValue != null) {
                            value = Arrays.toString(stringArrayValue);
                        }
                        break;
                    }
                }
            }

            ListPreference itemList = mConfigBundleItems.valueAt(mConfigType);

            if (itemList != null) {
                itemList.setSummary(key);
            }

            EditTextPreference valueEdit = mConfigBundleValues.valueAt(mConfigType);

            if (valueEdit != null) {
                valueEdit.setDialogMessage(key);
                valueEdit.setSummary(value);

                if (value.startsWith("[") && value.endsWith("]")) {
                    valueEdit.setText(value.substring(1, value.length() - 1));
                } else {
                    valueEdit.setText(value);
                }
            }

            return true;
        }
    }

    private final class ConfigBundleValueChangeListener extends ConfigListener
            implements Preference.OnPreferenceChangeListener {
        ConfigBundleValueChangeListener(int configType) {
            super(configType);
        }

        @Override
        public boolean onPreferenceChange(@NonNull Preference preference, Object newValue) {
            ListPreference keyList = mConfigBundleKeys.valueAt(mConfigType);
            String bundleKey = (keyList != null) ? keyList.getValue() : null;
            ListPreference itemList = mConfigBundleItems.valueAt(mConfigType);
            String key = (itemList != null) ? itemList.getValue() : null;
            String newConfigValue = newValue.toString();

            ImsLog.d(mSlotId, "onConfigBundleValueChanged: value=" + newConfigValue
                    + ", key=" + key + ", bundleKey=" + bundleKey);

            if (key == null) {
                ImsLog.w(mSlotId, "CarrierConfig: key is invalid.");
                return false;
            }

            if (bundleKey == null) {
                ImsLog.w(mSlotId, "Bundle key is not found.");
                return false;
            }

            String rootBundleKey = getFirstToken(bundleKey, "/");
            String subBundleKey = getLastToken(bundleKey, "/");
            ConfigInterface config = AgentFactory.getInstance().getAgent(
                    ConfigInterface.class, mSlotId);
            CarrierConfig cc = (config != null) ? config.getCarrierConfig() : null;
            PersistableBundle rootPb = (cc != null) ? cc.getBundle(rootBundleKey) : null;
            PersistableBundle subPb = null;
            boolean rootPbCreated = false;
            boolean subPbCreated = false;
            PersistableBundle testRootPb = mTestConfig.getPersistableBundle(rootBundleKey);
            PersistableBundle testSubPb = null;
            boolean testRootPbCreated = false;
            boolean testSubPbCreated = false;


            if (rootPb == null) {
                ImsLog.i(mSlotId, "New PersistableBundle is created: " + bundleKey);
                rootPb = new PersistableBundle();
                rootPbCreated = true;
            }

            if (testRootPb == null) {
                ImsLog.i(mSlotId, "New PersistableBundle(test) is created: " + bundleKey);
                testRootPb = new PersistableBundle();
                testRootPbCreated = true;
            }

            if (subBundleKey != null) {
                subPb = rootPb.getPersistableBundle(subBundleKey);

                if (subPb == null) {
                    ImsLog.i(mSlotId, "New PersistableBundle(sub) is created: " + bundleKey);
                    subPb = new PersistableBundle();
                    subPbCreated = true;
                }

                testSubPb = testRootPb.getPersistableBundle(subBundleKey);

                if (testSubPb == null) {
                    ImsLog.i(mSlotId, "New PersistableBundle(sub/test) is created: " + bundleKey);
                    testSubPb = new PersistableBundle();
                    testSubPbCreated = true;
                }
            }

            PersistableBundle pb = (subPb != null) ? subPb : rootPb;
            PersistableBundle testPb = (testSubPb != null) ? testSubPb : testRootPb;
            int configType = getConfigType(key);

            switch (configType) {
                case CONFIG_I_BOOLEAN: {
                    boolean value = Boolean.valueOf(newConfigValue);
                    pb.putBoolean(key, value);
                    testPb.putBoolean(key, value);
                    break;
                }
                case CONFIG_I_INT: {
                    int value = parseInt(newConfigValue, -1);
                    pb.putInt(key, value);
                    testPb.putInt(key, value);
                    break;
                }
                case CONFIG_I_STRING: {
                    pb.putString(key, newConfigValue);
                    testPb.putString(key, newConfigValue);
                    break;
                }
                case CONFIG_I_INT_ARRAY: {
                    int[] value = toIntArray(newConfigValue);
                    pb.putIntArray(key, value);
                    testPb.putIntArray(key, value);
                    newConfigValue = Arrays.toString(value);
                    break;
                }
                case CONFIG_I_STRING_ARRAY: {
                    String[] value = toStringArray(newConfigValue);
                    pb.putStringArray(key, value);
                    testPb.putStringArray(key, value);
                    newConfigValue = Arrays.toString(value);
                    break;
                }
                default:
                    ImsLog.e(mSlotId, "Unknown configuration type.");
                    return false;
            }

            if (subPbCreated) {
                rootPb.putPersistableBundle(subBundleKey, subPb);
            }

            if (rootPbCreated && cc != null) {
                cc.getConfig().putPersistableBundle(rootBundleKey, rootPb);
            }

            if (testSubPbCreated) {
                testRootPb.putPersistableBundle(subBundleKey, testSubPb);
            }

            if (testRootPbCreated) {
                mTestConfig.putPersistableBundle(rootBundleKey, testRootPb);
            }

            setConfigChanged(true);

            EditTextPreference valueEdit = mConfigBundleValues.valueAt(mConfigType);

            if (valueEdit != null) {
                valueEdit.setSummary(newConfigValue);
            }

            if (KEYS_OF_VALUE_USED_AS_BUNDLE_KEY.contains(key)) {
                createBundleKeys(true);
                initBundleKeys();
            }

            return true;
        }
    }

    private void createBundleKeys(boolean forceCreate) {
        if (forceCreate) {
            mBundleKeys.clear();
        }

        if (!mBundleKeys.isEmpty()) {
            return;
        }

        mBundleKeys.add(ImsVoice.KEY_AUDIO_CODEC_CAPABILITY_PAYLOAD_TYPES_BUNDLE);
        mBundleKeys.add(ImsVt.KEY_VIDEO_CODEC_CAPABILITY_PAYLOAD_TYPES_BUNDLE);
        mBundleKeys.add(ImsRtt.KEY_TEXT_CODEC_CAPABILITY_PAYLOAD_TYPES_BUNDLE);

        ConfigInterface config = AgentFactory.getInstance().getAgent(
                ConfigInterface.class, mSlotId);
        CarrierConfig cc = (config != null) ? config.getCarrierConfig() : null;

        if (cc == null) {
            ImsLog.w(mSlotId, "CarrierConfig is not found.");
            return;
        }

        PersistableBundle audioPayloadTypes = cc.getBundle(
                ImsVoice.KEY_AUDIO_CODEC_CAPABILITY_PAYLOAD_TYPES_BUNDLE);

        if (audioPayloadTypes != null) {
            int[] amrWbPayloadTypes = audioPayloadTypes.getIntArray(
                    ImsVoice.KEY_AMRWB_PAYLOAD_TYPE_INT_ARRAY);
            addPayloadDescriptionKeys(mBundleKeys,
                    ImsVoice.KEY_AMRWB_PAYLOAD_DESCRIPTION_BUNDLE,
                    amrWbPayloadTypes);

            int[] amrNbPayloadTypes = audioPayloadTypes.getIntArray(
                    ImsVoice.KEY_AMRNB_PAYLOAD_TYPE_INT_ARRAY);
            addPayloadDescriptionKeys(mBundleKeys,
                    ImsVoice.KEY_AMRNB_PAYLOAD_DESCRIPTION_BUNDLE,
                    amrNbPayloadTypes);

            int[] evsPayloadTypes = audioPayloadTypes.getIntArray(
                    ImsVoice.KEY_EVS_PAYLOAD_TYPE_INT_ARRAY);
            addPayloadDescriptionKeys(mBundleKeys,
                    ImsVoice.KEY_EVS_PAYLOAD_DESCRIPTION_BUNDLE,
                    evsPayloadTypes);
        }

        PersistableBundle videoPayloadTypes = cc.getBundle(
                ImsVt.KEY_VIDEO_CODEC_CAPABILITY_PAYLOAD_TYPES_BUNDLE);

        if (videoPayloadTypes != null) {
            int[] hevcPayloadTypes = videoPayloadTypes.getIntArray(
                    CarrierConfig.ImsVt.KEY_HEVC_PAYLOAD_TYPE_INT_ARRAY);
            addPayloadDescriptionKeys(mBundleKeys,
                    CarrierConfig.ImsVt.KEY_HEVC_PAYLOAD_DESCRIPTION_BUNDLE, hevcPayloadTypes);

            int[] h264PayloadTypes = videoPayloadTypes.getIntArray(
                    ImsVt.KEY_H264_PAYLOAD_TYPE_INT_ARRAY);
            addPayloadDescriptionKeys(mBundleKeys,
                    ImsVt.KEY_H264_PAYLOAD_DESCRIPTION_BUNDLE,
                    h264PayloadTypes);
        }
    }

    private void initBundleKeys() {
        SearchableListPreference keyList =
                (SearchableListPreference) mConfigBundleKeys.valueAt(CONFIG_I_BUNDLE);
        if (keyList != null) {
            String[] entries = mBundleKeys.toArray(new String[0]);
            keyList.setEntries(entries);
            keyList.setEntryValues(entries);
        }

        keyList = (SearchableListPreference) mConfigBundleKeys.valueAt(ASSETS_CONFIG_I_BUNDLE);
        if (keyList != null) {
            String[] entries = ASSETS_BUNDLE_KEYS.toArray(new String[0]);
            keyList.setEntries(entries);
            keyList.setEntryValues(entries);
        }
    }

    private static void addPayloadDescriptionKeys(List<String> bundleKeys, String rootKey,
            int[] payloadTypes) {
        if (payloadTypes != null) {
            for (int payloadType : payloadTypes) {
                bundleKeys.add(rootKey + "/" + String.valueOf(payloadType));
            }
        }
    }

    private void initConfigItems() {
        SparseArray<List<String>> configKeys = getConfigKeys();

        if (configKeys != null) {
            for (int i = 0; i < CONFIG_I_MAX; ++i) {
                Collection<String> keys = configKeys.valueAt(i);
                String[] entries = keys.toArray(new String[0]);

                SearchableListPreference itemList =
                        (SearchableListPreference) mConfigItems.valueAt(i);

                if (itemList != null) {
                    itemList.setEntries(entries);
                    itemList.setEntryValues(entries);
                }
            }
        }
    }

    private static SparseArray<List<String>> getConfigKeys() {
        if (sConfigKeys == null) {
            ArrayList<String> publicConfigKeys = new ArrayList<>();
            ArrayList<String> assetsConfigKeys = new ArrayList<>();

            readConfigKeys(PUBLIC_CARRIER_CONFIG_FILE, publicConfigKeys);
            readConfigKeys(CarrierConfig.DEFAULT_CARRIER_CONFIG_FILE, assetsConfigKeys);

            ImsLog.i("ConfigKeys: public=" + publicConfigKeys.size() +
                    ", assets=" + assetsConfigKeys.size());

            sConfigKeys = new SparseArray<>();

            for (int i = 0; i < CONFIG_I_MAX; ++i) {
                sConfigKeys.put(i, new ArrayList<>());
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

            for (int i = 0; i < CONFIG_I_MAX; ++i) {
                sConfigKeys.valueAt(i).sort(null);
            }
        }

        return sConfigKeys;
    }

    private static void readConfigKeys(String fileName, Collection<String> configKeys) {
        try (InputStream is = AppContext.getInstance().getAssets().open(fileName)) {
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
        } catch (IllegalArgumentException | IOException | XmlPullParserException e) {
            ImsLog.e("readConfigKeys: " + e.toString());
        }
    }

    private static void distributeConfigKeys(Collection<String> configKeys,
            Collection<String> booleanKeys, Collection<String> intKeys,
            Collection<String> stringKeys,
            Collection<String> intArrayKeys, Collection<String> stringArrayKeys) {
        for (String key : configKeys) {
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
            } else if (key.equals(
                    CarrierConfigManager.KEY_IGNORE_DATA_ENABLED_CHANGED_FOR_VIDEO_CALLS)) {
                booleanKeys.add(key);
            }
        }
    }

    private static int getConfigType(String key) {
        if (key.endsWith("_string")) {
            return CONFIG_I_STRING;
        } else if (key.endsWith("_string_array")) {
            return CONFIG_I_STRING_ARRAY;
        } else if (key.endsWith("_int")) {
            return CONFIG_I_INT;
        } else if (key.endsWith("_bool") || key.endsWith("_boolean")) {
            return CONFIG_I_BOOLEAN;
        } else if (key.endsWith("_int_array")) {
            return CONFIG_I_INT_ARRAY;
        }
        return CONFIG_I_MAX;
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

    private static String getFirstToken(@NonNull String s, @NonNull String delimiter) {
        int index = s.indexOf(delimiter);
        return (index < 0) ? s : s.substring(0, index);
    }

    private static String getLastToken(@NonNull String s, @NonNull String delimiter) {
        int index = s.indexOf(delimiter);
        return (index < 0) ? null : s.substring(index + 1, s.length());
    }
}
