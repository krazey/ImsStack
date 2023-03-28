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
import android.preference.CheckBoxPreference;
import android.preference.EditTextPreference;
import android.preference.ListPreference;
import android.preference.Preference;
import android.preference.PreferenceActivity;
import android.telephony.SubscriptionManager;
import android.telephony.ims.ImsException;
import android.telephony.ims.ImsManager;
import android.telephony.ims.ImsMmTelManager;
import android.view.Window;
import android.view.WindowManager.LayoutParams;
import android.widget.Toast;

import com.android.imsstack.R;
import com.android.imsstack.enabler.IUIMS;
import com.android.imsstack.enabler.aos.AosFactory;
import com.android.imsstack.enabler.aos.IAosInfo;
import com.android.imsstack.util.AppContext;
import com.android.imsstack.util.ImsLog;
import com.android.imsstack.util.ImsPrivateProperties;
import com.android.imsstack.util.LogUtils;
import com.android.imsstack.util.MSimUtils;
import com.android.imsstack.util.SystemUtils;

@SuppressWarnings("deprecation")
public class TestConfigMenu extends PreferenceActivity {
    // Main tree
    private static final String KEY_TEST_IMS_DISABLED = "test_ims_disabled";
    private static final String KEY_TEST_DEBUG_ENABLED = "test_debug_enabled";
    private static final String KEY_TEST_TESTMODE_ENABLED = "test_testmode_enabled";
    private static final String KEY_TEST_WIFI_TEST_ENABLED = "test_wifi_test_enabled";
    private static final String KEY_TEST_IMS_HAL_ENABLED = "test_ims_hal_enabled";
    private static final String KEY_TEST_CROSS_SIM_ENABLED = "test_cross_sim_enabled";
    private static final String KEY_TEST_CARRIER_SIGNAL_PCO_ENABLED =
            "test_carrier_signal_pco_enabled";
    private static final String KEY_TEST_PCSCF_ADDRESS = "test_pcscf_address";
    private static final String KEY_TEST_IMS_DEREGISTER = "test_ims_deregister";
    private static final String KEY_TEST_LOG_OPTIONS = "test_log_options";
    private static final String KEY_TEST_RESTART_IMSSTACK = "test_restart_imsstack";
    private static final String KEY_TEST_CLEAR_CONFIG = "test_clear_config";

    // Sub-tree for test_subscriber
    private static final String KEY_SUBSCRIBER_HOME_DOMAIN_NAME = "subscriber_home_domain_name";
    private static final String KEY_SUBSCRIBER_IMPI = "subscriber_impi";
    private static final String KEY_SUBSCRIBER_IMPU = "subscriber_impu";
    // Sub-tree for test_user_agent
    private static final String KEY_USER_AGENT_USE_PREDEFINED_UA_STRING =
            "user_agent_use_predefined_ua_string";
    private static final String KEY_USER_AGENT_UA_STRING = "user_agent_ua_string";
    private static final String KEY_NR_DUPLEX_MODE = "nr_duplex_mode";

    private int mSlotId = 0;

    private CheckBoxPreference mImsDisabled;
    private CheckBoxPreference mDebugEnabled;
    private CheckBoxPreference mTestModeEnabled;
    private CheckBoxPreference mWifiTestEnabled;
    private CheckBoxPreference mImsHalTestEnabled;
    private CheckBoxPreference mCrossSimEnabled;
    private CheckBoxPreference mCarrierSignalPcoEnabled;
    private EditTextPreference mHomeDomainName;
    private EditTextPreference mImpi;
    private EditTextPreference mImpu;
    private EditTextPreference mPcscfAddress;
    private CheckBoxPreference mUsePredefinedUaString;
    private EditTextPreference mUaString;
    private EditTextPreference mNrDuplexMode;
    private ListPreference mImsDeregister;
    private EditTextPreference mLogOptions;
    private ListPreference mRestartImsStack;
    private ListPreference mClearTestConfig;

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

        addPreferencesFromResource(R.xml.test_config_menu);

        initPreferences();
    }

    @Override
    public void onPause() {
        super.onPause();

        ImsLog.d(mSlotId, "TestModeMenu: onPause");
    }

    @Override
    public void onResume() {
        super.onResume();

        ImsLog.d(mSlotId, "TestModeMenu: onResume");
    }

    @Override
    protected boolean isValidFragment(String fragmentName) {
        return fragmentName != null;
    }

    private void initPreferences() {
        mImsDisabled = (CheckBoxPreference) findPreference(KEY_TEST_IMS_DISABLED);

        if (mImsDisabled != null) {
            boolean imsDisabled = ImsPrivateProperties.Persistent.getBoolean(
                    ImsPrivateProperties.Persistent.KEY_TEST_IMS_DISABLED, mSlotId);
            mImsDisabled.setChecked(imsDisabled);
            mImsDisabled.setOnPreferenceChangeListener(new CheckBoxItemChangeListener());
        }

        mDebugEnabled = (CheckBoxPreference) findPreference(KEY_TEST_DEBUG_ENABLED);

        if (mDebugEnabled != null) {
            boolean debugEnabled = ImsPrivateProperties.Persistent.getBoolean(
                    ImsPrivateProperties.Persistent.KEY_TEST_DEBUG_ENABLED, mSlotId);
            mDebugEnabled.setChecked(debugEnabled);
            mDebugEnabled.setOnPreferenceChangeListener(new CheckBoxItemChangeListener());
        }

        mTestModeEnabled = (CheckBoxPreference) findPreference(KEY_TEST_TESTMODE_ENABLED);

        if (mTestModeEnabled != null) {
            boolean testModeEnabled = ImsPrivateProperties.Persistent.getBoolean(
                    ImsPrivateProperties.Persistent.KEY_TEST_TESTMODE_ENABLED, mSlotId);
            mTestModeEnabled.setChecked(testModeEnabled);
            mTestModeEnabled.setOnPreferenceChangeListener(new CheckBoxItemChangeListener());
        }

        mWifiTestEnabled = (CheckBoxPreference) findPreference(KEY_TEST_WIFI_TEST_ENABLED);

        if (mWifiTestEnabled != null) {
            boolean wifiTestEnabled = (ImsPrivateProperties.Persistent.getInt(
                    ImsPrivateProperties.Persistent.KEY_WIFI_TEST, 0, mSlotId) == 1);
            mWifiTestEnabled.setChecked(wifiTestEnabled);
            mWifiTestEnabled.setOnPreferenceChangeListener(new CheckBoxItemChangeListener());
        }

        mImsHalTestEnabled = (CheckBoxPreference) findPreference(KEY_TEST_IMS_HAL_ENABLED);

        if (mImsHalTestEnabled != null) {
            boolean imsHalTestEnabled = (ImsPrivateProperties.Persistent.getInt(
                    ImsPrivateProperties.Persistent.KEY_IMS_HAL_TEST, 0, mSlotId) == 1);
            mImsHalTestEnabled.setChecked(imsHalTestEnabled);
            mImsHalTestEnabled.setOnPreferenceChangeListener(new CheckBoxItemChangeListener());
        }

        mCrossSimEnabled = (CheckBoxPreference) findPreference(KEY_TEST_CROSS_SIM_ENABLED);

        if (mCrossSimEnabled != null) {
            boolean crossSimEnabled = false;
            ImsMmTelManager imsMmTelMgr = getImsMmTelManager();
            if (imsMmTelMgr != null) {
                try {
                    crossSimEnabled = imsMmTelMgr.isCrossSimCallingEnabled();
                } catch (ImsException exception) {
                    // do noting
                }
            }

            mCrossSimEnabled.setChecked(crossSimEnabled);
            mCrossSimEnabled.setOnPreferenceChangeListener(new CheckBoxItemChangeListener());
        }

        mCarrierSignalPcoEnabled =
                (CheckBoxPreference) findPreference(KEY_TEST_CARRIER_SIGNAL_PCO_ENABLED);

        if (mCarrierSignalPcoEnabled != null) {
            boolean carrierSignalPcoEnabled = (ImsPrivateProperties.Persistent.getInt(
                    ImsPrivateProperties.Persistent.KEY_CARRIER_SIGNAL_PCO_TEST,
                    0, mSlotId) == 1);
            mCarrierSignalPcoEnabled.setChecked(carrierSignalPcoEnabled);
            mCarrierSignalPcoEnabled.setOnPreferenceChangeListener(
                    new CheckBoxItemChangeListener());
        }

        mHomeDomainName = (EditTextPreference) findPreference(KEY_SUBSCRIBER_HOME_DOMAIN_NAME);

        if (mHomeDomainName != null) {
            String homeDomainName = ImsPrivateProperties.Persistent.get(
                    ImsPrivateProperties.Persistent.KEY_CONFIG_HOME_DOMAIN_NAME, mSlotId);
            mHomeDomainName.setText(homeDomainName);
            mHomeDomainName.setSummary(homeDomainName);
            mHomeDomainName.setOnPreferenceChangeListener(new EditTextItemChangeListener());
        }

        mImpi = (EditTextPreference) findPreference(KEY_SUBSCRIBER_IMPI);

        if (mImpi != null) {
            String impi = ImsPrivateProperties.Persistent.get(
                    ImsPrivateProperties.Persistent.KEY_CONFIG_IMPI, mSlotId);
            mImpi.setText(impi);
            mImpi.setSummary(impi);
            mImpi.setOnPreferenceChangeListener(new EditTextItemChangeListener());
        }

        mImpu = (EditTextPreference) findPreference(KEY_SUBSCRIBER_IMPU);

        if (mImpu != null) {
            String impu = ImsPrivateProperties.Persistent.get(
                    ImsPrivateProperties.Persistent.KEY_CONFIG_IMPU_LIST, mSlotId);
            mImpu.setText(impu);
            mImpu.setSummary(impu);
            mImpu.setOnPreferenceChangeListener(new EditTextItemChangeListener());
        }

        mPcscfAddress = (EditTextPreference) findPreference(KEY_TEST_PCSCF_ADDRESS);

        if (mPcscfAddress != null) {
            String pcscfAddress = ImsPrivateProperties.Persistent.get(
                    ImsPrivateProperties.Persistent.KEY_CONFIG_PCSCF_ADDRESS_LIST, mSlotId);
            mPcscfAddress.setText(pcscfAddress);
            mPcscfAddress.setSummary(pcscfAddress);
            mPcscfAddress.setOnPreferenceChangeListener(new EditTextItemChangeListener());
        }

        mUsePredefinedUaString = (CheckBoxPreference)
                findPreference(KEY_USER_AGENT_USE_PREDEFINED_UA_STRING);

        if (mUsePredefinedUaString != null) {
            boolean usePredefinedUaString = ImsPrivateProperties.Persistent.getBoolean(
                    ImsPrivateProperties.Persistent.KEY_USE_PREDEFINED_UA_STRING, mSlotId);
            mUsePredefinedUaString.setChecked(usePredefinedUaString);
            mUsePredefinedUaString.setOnPreferenceChangeListener(new CheckBoxItemChangeListener());
        }

        mUaString = (EditTextPreference) findPreference(KEY_USER_AGENT_UA_STRING);

        if (mUaString != null) {
            String uaString = ImsPrivateProperties.Persistent.get(
                    ImsPrivateProperties.Persistent.KEY_CONFIG_UA_STRING, mSlotId);
            mUaString.setText(uaString);
            mUaString.setSummary(uaString);
            mUaString.setOnPreferenceChangeListener(new EditTextItemChangeListener());
        }

        mNrDuplexMode = (EditTextPreference) findPreference(KEY_NR_DUPLEX_MODE);

        if (mNrDuplexMode != null) {
            String nrMode = ImsPrivateProperties.Persistent.get(
                    ImsPrivateProperties.Persistent.KEY_CONFIG_NR_DUPLEX_MODE, mSlotId);
            mNrDuplexMode.setText(nrMode);
            mNrDuplexMode.setSummary(nrMode);
            mNrDuplexMode.setOnPreferenceChangeListener(new EditTextItemChangeListener());
        }

        mImsDeregister = (ListPreference) findPreference(KEY_TEST_IMS_DEREGISTER);

        if (mImsDeregister != null) {
            String imsDeregister = ImsPrivateProperties.Persistent.get(
                    ImsPrivateProperties.Persistent.KEY_TEST_IMS_DEREGISTER, "NO", mSlotId);
            mImsDeregister.setValue(imsDeregister);
            mImsDeregister.setSummary(getImsDeregisterSummary(imsDeregister));
            mImsDeregister.setOnPreferenceChangeListener(new ListItemChangeListener());
        }

        mLogOptions = (EditTextPreference) findPreference(KEY_TEST_LOG_OPTIONS);

        if (mLogOptions != null) {
            String logOptions = ImsPrivateProperties.Persistent.get(
                    ImsPrivateProperties.Persistent.KEY_TEST_LOG_OPTIONS,
                    LogUtils.DEFAULT_LOG_OPTIONS, mSlotId);
            mLogOptions.setText(logOptions);
            mLogOptions.setSummary(logOptions);
            mLogOptions.setOnPreferenceChangeListener(new EditTextItemChangeListener());
        }

        mRestartImsStack = (ListPreference) findPreference(KEY_TEST_RESTART_IMSSTACK);

        if (mRestartImsStack != null) {
            mRestartImsStack.setValue("");
            mRestartImsStack.setOnPreferenceChangeListener(new ListItemChangeListener());
        }

        mClearTestConfig = (ListPreference) findPreference(KEY_TEST_CLEAR_CONFIG);

        if (mClearTestConfig != null) {
            mClearTestConfig.setValue("");
            mClearTestConfig.setOnPreferenceChangeListener(new ListItemChangeListener());
        }
    }

    private ImsMmTelManager getImsMmTelManager() {
        int subId = MSimUtils.getSubId(mSlotId);

        if (!SubscriptionManager.isUsableSubscriptionId(subId)) {
            return null;
        }

        ImsManager imsMgr = AppContext.getInstance().getSystemService(ImsManager.class);
        return (imsMgr == null) ? null : imsMgr.getImsMmTelManager(subId);
    }

    private String getImsDeregisterSummary(String value) {
        return ("YES".equals(value)) ? "YES : The IMS service is blocked" :
                "NO : The IMS service is unblocked";
    }

    private final class CheckBoxItemChangeListener
            implements Preference.OnPreferenceChangeListener {
        @Override
        public boolean onPreferenceChange(Preference preference, Object newValue) {
            boolean value = Boolean.valueOf(newValue.toString());
            boolean isValueTypeInt = false;
            String key = null;

            switch (preference.getKey()) {
                case KEY_TEST_IMS_DISABLED:
                    key = ImsPrivateProperties.Persistent.KEY_TEST_IMS_DISABLED;
                    break;
                case KEY_TEST_DEBUG_ENABLED:
                    key = ImsPrivateProperties.Persistent.KEY_TEST_DEBUG_ENABLED;
                    break;
                case KEY_TEST_TESTMODE_ENABLED:
                    key = ImsPrivateProperties.Persistent.KEY_TEST_TESTMODE_ENABLED;
                    break;
                case KEY_TEST_WIFI_TEST_ENABLED:
                    key = ImsPrivateProperties.Persistent.KEY_WIFI_TEST;
                    isValueTypeInt = true;
                    break;
                case KEY_TEST_IMS_HAL_ENABLED:
                    key = ImsPrivateProperties.Persistent.KEY_IMS_HAL_TEST;
                    isValueTypeInt = true;
                    break;
                case KEY_TEST_CROSS_SIM_ENABLED:
                    ImsMmTelManager imsMmTelMgr = getImsMmTelManager();
                    if (imsMmTelMgr != null) {
                        try {
                            imsMmTelMgr.setCrossSimCallingEnabled(value);
                        } catch (ImsException exception) {
                            // do noting
                        }
                    }
                    break;
                case KEY_TEST_CARRIER_SIGNAL_PCO_ENABLED:
                    key = ImsPrivateProperties.Persistent.KEY_CARRIER_SIGNAL_PCO_TEST;
                    isValueTypeInt = true;
                    break;
                case KEY_USER_AGENT_USE_PREDEFINED_UA_STRING:
                    key = ImsPrivateProperties.Persistent.KEY_USE_PREDEFINED_UA_STRING;
                    break;
                default:
                    break;
            }

            if (key != null) {
                ImsLog.d(mSlotId, "TestConfig: " + preference.getKey() + "=" + value);

                if (isValueTypeInt) {
                    ImsPrivateProperties.Persistent.setInt(key, value ? 1 : 0, mSlotId);
                } else {
                    ImsPrivateProperties.Persistent.setBoolean(key, value, mSlotId);
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
            String key = null;

            switch (preference.getKey()) {
                case KEY_SUBSCRIBER_HOME_DOMAIN_NAME:
                    key = ImsPrivateProperties.Persistent.KEY_CONFIG_HOME_DOMAIN_NAME;
                    break;
                case KEY_SUBSCRIBER_IMPI:
                    key = ImsPrivateProperties.Persistent.KEY_CONFIG_IMPI;
                    break;
                case KEY_SUBSCRIBER_IMPU:
                    key = ImsPrivateProperties.Persistent.KEY_CONFIG_IMPU_LIST;
                    break;
                case KEY_TEST_PCSCF_ADDRESS:
                    key = ImsPrivateProperties.Persistent.KEY_CONFIG_PCSCF_ADDRESS_LIST;
                    break;
                case KEY_USER_AGENT_UA_STRING:
                    key = ImsPrivateProperties.Persistent.KEY_CONFIG_UA_STRING;
                    break;
                case KEY_NR_DUPLEX_MODE:
                    key = ImsPrivateProperties.Persistent.KEY_CONFIG_NR_DUPLEX_MODE;
                    break;
                case KEY_TEST_LOG_OPTIONS:
                    if (value != null && !value.startsWith("0x")) {
                        value = "0x" + value;
                    }

                    int logOptions = SystemUtils.hexStringToInt(value);

                    if (logOptions >= 0 && value.length() <= 10) {
                        key = ImsPrivateProperties.Persistent.KEY_TEST_LOG_OPTIONS;
                    } else {
                        ImsLog.d(mSlotId, "TestConfig: invalid log-options - " + value);
                        return false;
                    }
                    break;
                default:
                    break;
            }

            if (key != null) {
                ImsLog.d(mSlotId, "TestConfig: " + preference.getKey() + "=" + value);
                ImsPrivateProperties.Persistent.set(key, value, mSlotId);

                preference.setSummary(value);
            }
            return true;
        }
    }

    private final class ListItemChangeListener
            implements Preference.OnPreferenceChangeListener {
        @Override
        public boolean onPreferenceChange(Preference preference, Object newValue) {
            String value = newValue.toString();

            if (KEY_TEST_RESTART_IMSSTACK.equals(preference.getKey())) {
                if ("1".equals(value)) {
                    ImsLog.d(mSlotId, "TestConfig: restart ImsStack.");

                    Toast.makeText(TestConfigMenu.this,
                            "ImsStack restarted...", Toast.LENGTH_SHORT).show();
                    android.os.Process.killProcess(android.os.Process.myPid());
                }
            } else if (KEY_TEST_CLEAR_CONFIG.equals(preference.getKey())) {
                if ("1".equals(value)) {
                    ImsPrivateProperties.Persistent.removeTestProperties(mSlotId);
                    ImsLog.d(mSlotId, "TestConfig: test config cleared.");
                }
            } else if (KEY_TEST_IMS_DEREGISTER.equals(preference.getKey())) {
                ImsLog.d(mSlotId, "TestConfig: " + preference.getKey() + "=" + value);
                ImsPrivateProperties.Persistent.set(
                        ImsPrivateProperties.Persistent.KEY_TEST_IMS_DEREGISTER, value, mSlotId);
                preference.setSummary(getImsDeregisterSummary(value));

                IAosInfo aosInfo = AosFactory.getInstance().getAosInfo(mSlotId);
                if (aosInfo != null) {
                    if ("YES".equals(value)) {
                        aosInfo.notifyServiceSetting(
                                IAosInfo.ServiceSetting.OFF, IUIMS.M_SERVICE_ALL);
                        Toast.makeText(TestConfigMenu.this,
                                "Send IMS deregister message!", Toast.LENGTH_SHORT).show();
                    } else {
                        aosInfo.notifyServiceSetting(
                                IAosInfo.ServiceSetting.ON, IUIMS.M_SERVICE_ALL);
                    }
                }
            }

            return true;
        }
    }
}
