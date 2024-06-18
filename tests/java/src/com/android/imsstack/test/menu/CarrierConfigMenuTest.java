/*
 * Copyright (C) 2023 The Android Open Source Project
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

import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertFalse;
import static org.junit.Assert.assertNotNull;
import static org.junit.Assert.assertTrue;
import static org.mockito.ArgumentMatchers.any;
import static org.mockito.ArgumentMatchers.anyInt;
import static org.mockito.ArgumentMatchers.eq;
import static org.mockito.Mockito.times;
import static org.mockito.Mockito.verify;
import static org.mockito.Mockito.when;

import android.app.Instrumentation;
import android.content.Intent;
import android.os.PersistableBundle;
import android.preference.EditTextPreference;
import android.preference.ListPreference;
import android.preference.Preference;
import android.telephony.CarrierConfigManager;
import android.telephony.ims.ProvisioningManager;

import androidx.test.filters.LargeTest;
import androidx.test.filters.SmallTest;
import androidx.test.platform.app.InstrumentationRegistry;

import com.android.imsstack.ContextFixture;
import com.android.imsstack.TestApplication;
import com.android.imsstack.base.ImsPrivateProperties;
import com.android.imsstack.base.MSimUtils;
import com.android.imsstack.base.SystemServiceProxy.ImsManagerProxy;
import com.android.imsstack.base.SystemServiceProxy.ProvisioningManagerProxy;
import com.android.imsstack.base.TestAppContext;
import com.android.imsstack.core.agents.AgentFactory;
import com.android.imsstack.core.agents.ConfigInterface;
import com.android.imsstack.core.config.CarrierConfig;
import com.android.imsstack.core.config.ConfigXmlUtils;
import com.android.imsstack.system.ISystem;
import com.android.imsstack.system.SystemInterface;

import org.junit.After;
import org.junit.Before;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.junit.runners.JUnit4;
import org.mockito.Mock;
import org.mockito.MockitoAnnotations;
import org.xmlpull.v1.XmlPullParser;
import org.xmlpull.v1.XmlPullParserException;
import org.xmlpull.v1.XmlPullParserFactory;

import java.io.IOException;
import java.io.InputStream;
import java.util.List;

@RunWith(JUnit4.class)
public class CarrierConfigMenuTest {
    private static final int MAX_MODEM_COUNT = 1;
    private static final int SLOT0 = 0;
    private static final int[] SUB_ID = { 1 };
    private static final String TEST_CONFIG_BOOLEAN =
            CarrierConfig.Ims.KEY_SIP_COMPACT_FORM_ENABLED_BOOL;
    private static final String TEST_CONFIG_INT =
            CarrierConfig.Ims.KEY_ISIM_INDEX_FOR_IMPU_INT;
    private static final String TEST_CONFIG_STRING =
            CarrierConfig.ImsVoice.KEY_CALL_TERMINATE_REASON_HEADER_USER_ENDS_CALL_STRING;
    private static final String TEST_CONFIG_INT_ARRAY =
            CarrierConfig.Ims.KEY_PCSCF_DISCOVERY_METHOD_INT_ARRAY;
    private static final String TEST_CONFIG_STRING_ARRAY =
            CarrierConfig.ImsVt.KEY_VIDEO_CODEC_FRAME_SIZE_STRING_ARRAY;
    private static final String TEST_ASSETS_CONFIG_BOOLEAN =
            CarrierConfig.Assets.KEY_SUPPORT_SDP_PRECONDITION_BOOL;
    private static final String TEST_ASSETS_CONFIG_INT =
            CarrierConfig.Assets.KEY_SIP_TIMER_100_TRYING_MILLIS_INT;
    private static final String TEST_ASSETS_CONFIG_STRING =
            CarrierConfig.Assets.KEY_HEVC_SPROP_PARAMETER_SETS_STRING;
    private static final String TEST_ASSETS_CONFIG_INT_ARRAY =
            CarrierConfig.Assets.KEY_SUPPORTED_ROAMING_RATS_INT_ARRAY;
    private static final String TEST_ASSETS_CONFIG_STRING_ARRAY =
            CarrierConfig.Assets.KEY_CARRIER_SPECIFIC_SIP_HEADERS_STRING_ARRAY;
    private static final String TEST_CONFIG_PAYLOAD_TYPE = "100";
    private static final String TEST_CONFIG_BUNDLE =
            CarrierConfigManager.ImsVoice.KEY_AUDIO_CODEC_CAPABILITY_PAYLOAD_TYPES_BUNDLE;
    private static final String TEST_CONFIG_BUNDLE_PLAYLOAD_DESC =
            CarrierConfigManager.ImsVoice.KEY_AMRWB_PAYLOAD_DESCRIPTION_BUNDLE;
    private static final String TEST_CONFIG_BUNDLE_WITH_BUNDLE =
            TEST_CONFIG_BUNDLE_PLAYLOAD_DESC + "/" + TEST_CONFIG_PAYLOAD_TYPE;
    private static final String TEST_CONFIG_BUNDLE_BOOL = "test_config_bundle_bool";
    private static final String TEST_CONFIG_BUNDLE_INT = "test_config_bundle_int";
    private static final String TEST_CONFIG_BUNDLE_STRING = "test_config_bundle_string";
    private static final String TEST_CONFIG_BUNDLE_INT_ARRAY = "test_config_bundle_int_array";
    private static final String TEST_CONFIG_BUNDLE_STRING_ARRAY = "test_config_bundle_string_array";

    @Mock private ProvisioningManagerProxy mProvisioningManagerProxy;
    @Mock private ISystem mSystem;
    @Mock private SystemInterface mSystemInterface;
    @Mock private ConfigInterface mConfigInterface;

    private Instrumentation mInstrumentation;
    private TestAppContext mTestAppContext;
    private PersistableBundle mTestConfig;
    private CarrierConfig mCarrierConfig;
    private CarrierConfigMenu mCarrierConfigMenu;

    @Before
    public void setUp() throws Exception {
        MockitoAnnotations.initMocks(this);
        mInstrumentation = InstrumentationRegistry.getInstrumentation();

        mTestAppContext = new TestAppContext(new ContextFixture().getTestDouble());
        mTestAppContext.setUp();

        ImsManagerProxy imsManagerProxy =
                mTestAppContext.getSystemServiceProxy(ImsManagerProxy.class);
        when(imsManagerProxy.getProvisioningManagerProxy(eq(SUB_ID[0])))
                .thenReturn(mProvisioningManagerProxy);
        when(mSystemInterface.getSystem(eq(SLOT0))).thenReturn(mSystem);
        SystemInterface.setSystemInterface(mSystemInterface);
        AgentFactory.getInstance().setAgent(ConfigInterface.class, mConfigInterface, SLOT0);
        setUpConfig();
    }

    @After
    public void tearDown() throws Exception {
        AgentFactory.getInstance().setAgent(ConfigInterface.class, null, SLOT0);
        SystemInterface.setSystemInterface(null);
        mProvisioningManagerProxy = null;
        mSystem = null;
        mSystemInterface = null;
        mConfigInterface = null;
        mCarrierConfig = null;
        mTestConfig = null;
        mCarrierConfigMenu = null;
        mInstrumentation = null;
        mTestAppContext.tearDown();
        mTestAppContext = null;
    }

    @Test
    @LargeTest
    public void testOnCreate() {
        setUpActivity(false);
        // onCreate will be invoked when calling startActivity(...).
        verify(mConfigInterface).readTestConfig();
        verify(mConfigInterface).getCarrierConfig();
    }

    @Test
    @LargeTest
    public void testOnPauseAndResume() {
        setUpActivity(false);
        mCarrierConfigMenu.setConfigChanged(true);

        mInstrumentation.runOnMainSync(() -> {
            mInstrumentation.callActivityOnPause(mCarrierConfigMenu);
        });

        verify(mConfigInterface).writeTestConfig(any(PersistableBundle.class));
        verify(mSystem).notifyConfigurationChanged(anyInt());

        mInstrumentation.runOnMainSync(() -> {
            mInstrumentation.callActivityOnResume(mCarrierConfigMenu);
        });

        // These are called when onCreate is invoked.
        verify(mConfigInterface).readTestConfig();
        verify(mConfigInterface).getCarrierConfig();
    }

    @Test
    @SmallTest
    public void testIsValidFragment() {
        setUpActivity(true);

        assertTrue(mCarrierConfigMenu.isValidFragment("test"));
        assertFalse(mCarrierConfigMenu.isValidFragment(null));
    }

    @Test
    @LargeTest
    public void testOnPreferenceClick() {
        setUpActivity(false);
        Preference preference = mCarrierConfigMenu.findPreference(
                CarrierConfigMenu.KEY_DUMP_CONFIG);

        assertNotNull(preference);

        Preference.OnPreferenceClickListener listener = preference.getOnPreferenceClickListener();

        mInstrumentation.runOnMainSync(() -> {
            listener.onPreferenceClick(preference);
        });

        // onCreate / onPreferenceClick
        verify(mConfigInterface, times(2)).getCarrierConfig();
    }

    @Test
    @LargeTest
    public void testOnPreferenceChangeForListItem() {
        setUpActivity(false);

        mInstrumentation.runOnMainSync(() -> {
            Preference preference = mCarrierConfigMenu.findPreference(
                    CarrierConfigMenu.KEY_CLEAR_TEST_CONFIG);

            assertNotNull(preference);

            Preference.OnPreferenceChangeListener listener =
                    preference.getOnPreferenceChangeListener();
            listener.onPreferenceChange(preference, Integer.toString(1));

            verify(mConfigInterface).writeTestConfig(any(PersistableBundle.class));

            preference = mCarrierConfigMenu.findPreference(CarrierConfigMenu.KEY_TEST_CARRIER_ID);

            assertNotNull(preference);

            int testCarrierId = 1111;
            listener = preference.getOnPreferenceChangeListener();
            listener.onPreferenceChange(preference, Integer.toString(testCarrierId));
            int carrierId = ImsPrivateProperties.Persistent.getInt(
                    ImsPrivateProperties.Persistent.KEY_TEST_CARRIER_ID, SLOT0);

            assertEquals(testCarrierId, carrierId);

            preference = mCarrierConfigMenu.findPreference(
                    CarrierConfigMenu.KEY_VOLTE_PROVISIONING);

            assertNotNull(preference);

            int provisioningStatus = 1;
            listener = preference.getOnPreferenceChangeListener();
            listener.onPreferenceChange(preference, Integer.toString(provisioningStatus));

            verify(mProvisioningManagerProxy).setProvisioningIntValue(
                    eq(ProvisioningManager.KEY_VOLTE_PROVISIONING_STATUS), eq(provisioningStatus));
            verify(mProvisioningManagerProxy).setProvisioningIntValue(
                    eq(ProvisioningManager.KEY_VT_PROVISIONING_STATUS), eq(provisioningStatus));
            verify(mProvisioningManagerProxy).setProvisioningIntValue(
                    eq(ProvisioningManager.KEY_VOICE_OVER_WIFI_ENABLED_OVERRIDE),
                    eq(provisioningStatus));
        });
    }

    @Test
    @LargeTest
    public void testOnPreferenceChangeForConfigItem() {
        setUpActivity(false);
        List<String> itemPrefKeys = getConfigItemPreferenceKeys();
        List<String> valuePrefKeys = getConfigValuePreferenceKeys();
        List<String> configKeys = getConfigKeys();

        mInstrumentation.runOnMainSync(() -> {
            for (int i = 0; i < itemPrefKeys.size(); ++i) {
                ListPreference itemList = (ListPreference)
                        mCarrierConfigMenu.findPreference(itemPrefKeys.get(i));
                EditTextPreference valueEdit = (EditTextPreference)
                        mCarrierConfigMenu.findPreference(valuePrefKeys.get(i));

                assertNotNull(itemList);
                assertNotNull(valueEdit);

                Preference.OnPreferenceChangeListener listener =
                        itemList.getOnPreferenceChangeListener();
                listener.onPreferenceChange(itemList, configKeys.get(i));

                assertEquals(configKeys.get(i), itemList.getSummary());
                assertEquals(configKeys.get(i), valueEdit.getDialogMessage());
            }
        });
    }

    @Test
    @LargeTest
    public void testOnPreferenceChangeForConfigValue() {
        setUpActivity(false);
        List<String> itemPrefKeys = getConfigItemPreferenceKeys();
        List<String> valuePrefKeys = getConfigValuePreferenceKeys();
        List<String> configKeys = getConfigKeys();
        List<String> configValues = List.of(
                "true",
                "0",
                "normal",
                "0, 1",
                "96 480 640",
                "true",
                "200",
                "AAAAAUIBAQFgAAADALAAAAMAAAMAWqAPCAKBZa7kyS7gC7QoSg==",
                "1, 2, 3",
                "P-VoLte-Info, P-Last-Access-Network-Info");

        mInstrumentation.runOnMainSync(() -> {
            for (int i = 0; i < itemPrefKeys.size(); ++i) {
                ListPreference itemList = (ListPreference)
                        mCarrierConfigMenu.findPreference(itemPrefKeys.get(i));
                EditTextPreference valueEdit = (EditTextPreference)
                        mCarrierConfigMenu.findPreference(valuePrefKeys.get(i));

                assertNotNull(itemList);
                assertNotNull(valueEdit);

                String newValue = configValues.get(i);
                itemList.setValue(configKeys.get(i));
                Preference.OnPreferenceChangeListener listener =
                        valueEdit.getOnPreferenceChangeListener();
                listener.onPreferenceChange(valueEdit, newValue);

                String storedValue = CarrierConfig.getValue(
                        mCarrierConfigMenu.getTestConfig(), configKeys.get(i));
                storedValue = removeSquareBrackets(storedValue);
                assertTrue(mCarrierConfigMenu.isConfigChanged());
                assertEquals(storedValue, removeSquareBrackets(valueEdit.getSummary()));
                assertEquals(storedValue, newValue);
            }
        });
    }

    @Test
    @LargeTest
    public void testOnPreferenceChangeForConfigBundleKey() {
        setUpActivity(false);
        ListPreference keyList = (ListPreference) mCarrierConfigMenu.findPreference(
                CarrierConfigMenu.KEY_CONFIG_BUNDLE_KEYS);
        ListPreference itemList = (ListPreference) mCarrierConfigMenu.findPreference(
                CarrierConfigMenu.KEY_CONFIG_BUNDLE_ITEMS);
        EditTextPreference valueEdit = (EditTextPreference) mCarrierConfigMenu.findPreference(
                CarrierConfigMenu.KEY_CONFIG_BUNDLE_VALUE);

        assertNotNull(keyList);
        assertNotNull(itemList);
        assertNotNull(valueEdit);

        Preference.OnPreferenceChangeListener listener = keyList.getOnPreferenceChangeListener();
        mInstrumentation.runOnMainSync(() -> {
            listener.onPreferenceChange(keyList, TEST_CONFIG_BUNDLE_WITH_BUNDLE);
        });

        assertEquals(TEST_CONFIG_BUNDLE_WITH_BUNDLE, keyList.getSummary());
        assertEquals("", itemList.getSummary());
        assertEquals("", valueEdit.getDialogMessage());
        assertEquals("", valueEdit.getSummary());
        assertEquals("", valueEdit.getText());
    }

    @Test
    @LargeTest
    public void testOnPreferenceChangeForConfigBundleItem() {
        setUpActivity(false);
        ListPreference keyList = (ListPreference) mCarrierConfigMenu.findPreference(
                CarrierConfigMenu.KEY_CONFIG_BUNDLE_KEYS);
        keyList.setValue(TEST_CONFIG_BUNDLE_WITH_BUNDLE);
        ListPreference itemList = (ListPreference) mCarrierConfigMenu.findPreference(
                CarrierConfigMenu.KEY_CONFIG_BUNDLE_ITEMS);
        EditTextPreference valueEdit = (EditTextPreference) mCarrierConfigMenu.findPreference(
                CarrierConfigMenu.KEY_CONFIG_BUNDLE_VALUE);
        List<String> configKeys = getBundleConfigKeys();

        mInstrumentation.runOnMainSync(() -> {
            for (String configKey : configKeys) {
                Preference.OnPreferenceChangeListener listener =
                        itemList.getOnPreferenceChangeListener();
                listener.onPreferenceChange(itemList, configKey);

                assertEquals(configKey, itemList.getSummary());
                assertEquals(configKey, valueEdit.getDialogMessage());
                assertNotNull(valueEdit.getText());
                assertFalse(valueEdit.getText().isEmpty());
            }
        });
    }

    @Test
    @LargeTest
    public void testOnPreferenceChangeForConfigBundleValue() {
        setUpActivity(false);
        ListPreference keyList = (ListPreference) mCarrierConfigMenu.findPreference(
                CarrierConfigMenu.KEY_CONFIG_BUNDLE_KEYS);
        keyList.setValue(TEST_CONFIG_BUNDLE_WITH_BUNDLE);
        ListPreference itemList = (ListPreference) mCarrierConfigMenu.findPreference(
                CarrierConfigMenu.KEY_CONFIG_BUNDLE_ITEMS);
        EditTextPreference valueEdit = (EditTextPreference) mCarrierConfigMenu.findPreference(
                CarrierConfigMenu.KEY_CONFIG_BUNDLE_VALUE);
        List<String> configKeys = getBundleConfigKeys();
        List<String> configValues = List.of(
                "true",
                "1",
                "test-string",
                "100, 101",
                "abc, def");
        PersistableBundle payloadDescPb = mCarrierConfig.getConfig().getPersistableBundle(
                TEST_CONFIG_BUNDLE_PLAYLOAD_DESC);
        PersistableBundle testConfig = payloadDescPb.getPersistableBundle(TEST_CONFIG_PAYLOAD_TYPE);

        mInstrumentation.runOnMainSync(() -> {
            for (int i = 0; i < configKeys.size(); ++i) {
                String configKey = configKeys.get(i);
                String newValue = configValues.get(i);
                itemList.setValue(configKey);
                Preference.OnPreferenceChangeListener listener =
                        valueEdit.getOnPreferenceChangeListener();
                listener.onPreferenceChange(valueEdit, newValue);

                String storedValue = CarrierConfig.getValue(testConfig, configKey);
                storedValue = removeSquareBrackets(storedValue);
                assertTrue(mCarrierConfigMenu.isConfigChanged());
                assertEquals(storedValue, removeSquareBrackets(valueEdit.getSummary()));
                assertEquals(storedValue, newValue);
            }
        });
    }

    private void setUpActivity(boolean isSmallTest) {
        Intent intent = new Intent(mInstrumentation.getTargetContext(), CarrierConfigMenu.class);
        intent.putExtra(MSimUtils.EXTRA_KEY_SLOT_ID, SLOT0);

        if (isSmallTest) {
            mInstrumentation.runOnMainSync(() -> {
                try {
                    mCarrierConfigMenu = (CarrierConfigMenu) mInstrumentation.newActivity(
                            getClass().getClassLoader(), CarrierConfigMenu.class.getName(), intent);
                } catch (Exception e) {
                    throw new RuntimeException(e); // nothing to do
                }
            });
        } else {
            intent.addFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
            mCarrierConfigMenu = (CarrierConfigMenu) mInstrumentation.startActivitySync(intent);
        }

        assertNotNull(mCarrierConfigMenu);
    }

    private void setUpConfig() {
        mTestConfig = new PersistableBundle();
        PersistableBundle defaultConfig;

        try (InputStream is = TestApplication.getAppContext().getAssets().open(
                CarrierConfig.DEFAULT_CARRIER_CONFIG_FILE)) {
            XmlPullParserFactory factory = XmlPullParserFactory.newInstance();
            XmlPullParser parser = factory.newPullParser();
            parser.setInput(is, "utf-8");
            defaultConfig = readConfigFromXml(parser);
        } catch (IOException | XmlPullParserException e) {
            defaultConfig = new PersistableBundle();
        }

        mCarrierConfig = new CarrierConfig();
        mCarrierConfig.setConfig(defaultConfig, SLOT0);
        when(mConfigInterface.readTestConfig()).thenReturn(mTestConfig);
        when(mConfigInterface.getCarrierConfig()).thenReturn(mCarrierConfig);

        mCarrierConfig.getConfig().putBoolean(
                CarrierConfigManager.KEY_CARRIER_VOLTE_PROVISIONING_REQUIRED_BOOL, true);
        PersistableBundle descPb = mCarrierConfig.getConfig().getPersistableBundle(
                TEST_CONFIG_BUNDLE_PLAYLOAD_DESC);
        PersistableBundle payloadPb = descPb.getPersistableBundle(TEST_CONFIG_PAYLOAD_TYPE);
        if (payloadPb == null) {
            payloadPb = new PersistableBundle();
            descPb.putPersistableBundle(TEST_CONFIG_PAYLOAD_TYPE, payloadPb);
        }
        payloadPb.putBoolean(TEST_CONFIG_BUNDLE_BOOL, false);
        payloadPb.putInt(TEST_CONFIG_BUNDLE_INT, 100);
        payloadPb.putString(TEST_CONFIG_BUNDLE_STRING, "test-string-default");
        payloadPb.putIntArray(TEST_CONFIG_BUNDLE_INT_ARRAY, new int[] { 1, 2 });
        payloadPb.putStringArray(TEST_CONFIG_BUNDLE_STRING_ARRAY,
                new String[] { "a", "b", "c", "d" });
    }

    private static PersistableBundle readConfigFromXml(XmlPullParser parser)
            throws IOException, XmlPullParserException {
        PersistableBundle config = new PersistableBundle();
        int event;

        while (((event = parser.next()) != XmlPullParser.END_DOCUMENT)) {
            if (event == XmlPullParser.START_TAG
                    && "carrier_config".equals(parser.getName())) {
                PersistableBundle configFragment = ConfigXmlUtils.readConfig(parser);
                config.putAll(configFragment);
            }
        }

        return config;
    }

    private static String removeSquareBrackets(CharSequence cs) {
        return removeSquareBrackets(cs.toString());
    }

    private static String removeSquareBrackets(String s) {
        if (s != null && s.startsWith("[") && s.endsWith("]")) {
            return s.substring(1, s.length() - 1);
        }
        return s;
    }

    private static List<String> getConfigItemPreferenceKeys() {
        return List.of(
                CarrierConfigMenu.KEY_CONFIG_BOOLEAN_ITEMS,
                CarrierConfigMenu.KEY_CONFIG_INT_ITEMS,
                CarrierConfigMenu.KEY_CONFIG_STRING_ITEMS,
                CarrierConfigMenu.KEY_CONFIG_INT_ARRAY_ITEMS,
                CarrierConfigMenu.KEY_CONFIG_STRING_ARRAY_ITEMS,
                CarrierConfigMenu.KEY_ASSETS_PREFIX + CarrierConfigMenu.KEY_CONFIG_BOOLEAN_ITEMS,
                CarrierConfigMenu.KEY_ASSETS_PREFIX + CarrierConfigMenu.KEY_CONFIG_INT_ITEMS,
                CarrierConfigMenu.KEY_ASSETS_PREFIX + CarrierConfigMenu.KEY_CONFIG_STRING_ITEMS,
                CarrierConfigMenu.KEY_ASSETS_PREFIX + CarrierConfigMenu.KEY_CONFIG_INT_ARRAY_ITEMS,
                CarrierConfigMenu.KEY_ASSETS_PREFIX
                        + CarrierConfigMenu.KEY_CONFIG_STRING_ARRAY_ITEMS);
    }

    private static List<String> getConfigValuePreferenceKeys() {
        return List.of(
                CarrierConfigMenu.KEY_CONFIG_BOOLEAN_VALUE,
                CarrierConfigMenu.KEY_CONFIG_INT_VALUE,
                CarrierConfigMenu.KEY_CONFIG_STRING_VALUE,
                CarrierConfigMenu.KEY_CONFIG_INT_ARRAY_VALUE,
                CarrierConfigMenu.KEY_CONFIG_STRING_ARRAY_VALUE,
                CarrierConfigMenu.KEY_ASSETS_PREFIX + CarrierConfigMenu.KEY_CONFIG_BOOLEAN_VALUE,
                CarrierConfigMenu.KEY_ASSETS_PREFIX + CarrierConfigMenu.KEY_CONFIG_INT_VALUE,
                CarrierConfigMenu.KEY_ASSETS_PREFIX + CarrierConfigMenu.KEY_CONFIG_STRING_VALUE,
                CarrierConfigMenu.KEY_ASSETS_PREFIX + CarrierConfigMenu.KEY_CONFIG_INT_ARRAY_VALUE,
                CarrierConfigMenu.KEY_ASSETS_PREFIX
                        + CarrierConfigMenu.KEY_CONFIG_STRING_ARRAY_VALUE);
    }

    private static List<String> getConfigKeys() {
        return List.of(
                TEST_CONFIG_BOOLEAN,
                TEST_CONFIG_INT,
                TEST_CONFIG_STRING,
                TEST_CONFIG_INT_ARRAY,
                TEST_CONFIG_STRING_ARRAY,
                TEST_ASSETS_CONFIG_BOOLEAN,
                TEST_ASSETS_CONFIG_INT,
                TEST_ASSETS_CONFIG_STRING,
                TEST_ASSETS_CONFIG_INT_ARRAY,
                TEST_ASSETS_CONFIG_STRING_ARRAY);
    }

    private static List<String> getBundleConfigKeys() {
        return List.of(
                TEST_CONFIG_BUNDLE_BOOL,
                TEST_CONFIG_BUNDLE_INT,
                TEST_CONFIG_BUNDLE_STRING,
                TEST_CONFIG_BUNDLE_INT_ARRAY,
                TEST_CONFIG_BUNDLE_STRING_ARRAY);
    }
}
