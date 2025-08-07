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

import static org.junit.Assert.assertFalse;
import static org.junit.Assert.assertNotNull;
import static org.junit.Assert.assertTrue;
import static org.mockito.ArgumentMatchers.any;
import static org.mockito.ArgumentMatchers.anyInt;
import static org.mockito.ArgumentMatchers.anyString;
import static org.mockito.ArgumentMatchers.eq;
import static org.mockito.Mockito.doAnswer;
import static org.mockito.Mockito.doReturn;
import static org.mockito.Mockito.verify;
import static org.mockito.Mockito.when;

import android.app.Instrumentation;
import android.content.Context;
import android.content.Intent;
import android.content.SharedPreferences;
import android.preference.CheckBoxPreference;
import android.preference.EditTextPreference;
import android.preference.ListPreference;
import android.preference.Preference;
import android.telephony.ims.ImsException;

import androidx.test.filters.LargeTest;
import androidx.test.filters.SmallTest;
import androidx.test.platform.app.InstrumentationRegistry;

import com.android.imsstack.base.ImsPrivateProperties;
import com.android.imsstack.base.MSimUtils;
import com.android.imsstack.base.SystemServiceProxy.ImsManagerProxy;
import com.android.imsstack.base.SystemServiceProxy.ImsMmTelManagerProxy;
import com.android.imsstack.base.TestAppContext;
import com.android.imsstack.enabler.aos.AosFactory;

import org.junit.After;
import org.junit.Before;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.junit.runners.JUnit4;
import org.mockito.Mock;
import org.mockito.MockitoAnnotations;

import java.util.Map;

@RunWith(JUnit4.class)
public class TestConfigMenuTest {
    private static final int SLOT0 = 0;
    private static final int[] SUB_ID = { 1 };
    // Pair of preference key and boolean property key.
    private static final Map<String, String> PREF_TO_PROP_BOOL = Map.ofEntries(
            Map.entry(TestConfigMenu.KEY_TEST_IMS_DISABLED,
                    ImsPrivateProperties.Persistent.KEY_TEST_IMS_DISABLED),
            Map.entry(TestConfigMenu.KEY_TEST_DEBUG_ENABLED,
                    ImsPrivateProperties.Persistent.KEY_TEST_DEBUG_ENABLED),
            Map.entry(TestConfigMenu.KEY_TEST_TESTMODE_ENABLED,
                    ImsPrivateProperties.Persistent.KEY_TEST_TESTMODE_ENABLED),
            Map.entry(TestConfigMenu.KEY_TEST_WIFI_TEST_ENABLED,
                    ImsPrivateProperties.Persistent.KEY_WIFI_TEST),
            Map.entry(TestConfigMenu.KEY_TEST_SIMULATED_IMS_HAL,
                    ImsPrivateProperties.Persistent.KEY_TEST_SIMULATED_IMS_HAL),
            Map.entry(TestConfigMenu.KEY_TEST_CARRIER_SIGNAL_PCO_ENABLED,
                    ImsPrivateProperties.Persistent.KEY_CARRIER_SIGNAL_PCO_TEST),
            Map.entry(TestConfigMenu.KEY_USER_AGENT_USE_PREDEFINED_UA_STRING,
                    ImsPrivateProperties.Persistent.KEY_USE_PREDEFINED_UA_STRING));
    // Pair of preference key and string property key.
    private static final Map<String, String> PREF_TO_PROP_STRING = Map.ofEntries(
            Map.entry(TestConfigMenu.KEY_TEST_PCSCF_ADDRESS,
                    ImsPrivateProperties.Persistent.KEY_CONFIG_PCSCF_ADDRESS_LIST),
            Map.entry(TestConfigMenu.KEY_TEST_IMS_DEREGISTER,
                    ImsPrivateProperties.Persistent.KEY_TEST_IMS_DEREGISTER),
            Map.entry(TestConfigMenu.KEY_TEST_LOG_OPTIONS,
                    ImsPrivateProperties.Persistent.KEY_TEST_LOG_OPTIONS),
            Map.entry(TestConfigMenu.KEY_SUBSCRIBER_HOME_DOMAIN_NAME,
                    ImsPrivateProperties.Persistent.KEY_CONFIG_HOME_DOMAIN_NAME),
            Map.entry(TestConfigMenu.KEY_SUBSCRIBER_IMPI,
                    ImsPrivateProperties.Persistent.KEY_CONFIG_IMPI),
            Map.entry(TestConfigMenu.KEY_SUBSCRIBER_IMPU,
                    ImsPrivateProperties.Persistent.KEY_CONFIG_IMPU_LIST),
            Map.entry(TestConfigMenu.KEY_USER_AGENT_UA_STRING,
                    ImsPrivateProperties.Persistent.KEY_CONFIG_UA_STRING),
            Map.entry(TestConfigMenu.KEY_NR_DUPLEX_MODE,
                    ImsPrivateProperties.Persistent.KEY_CONFIG_NR_DUPLEX_MODE));

    @Mock private SharedPreferences mSp;
    @Mock private SharedPreferences.Editor mSpEditor;
    @Mock private ImsMmTelManagerProxy mImsMmTelManagerProxy;

    private Instrumentation mInstrumentation;
    private TestAppContext mTestAppContext;
    private TestConfigMenu mTestConfigMenu;

    @Before
    public void setUp() throws Exception {
        MockitoAnnotations.initMocks(this);
        mInstrumentation = InstrumentationRegistry.getInstrumentation();

        mTestAppContext = new TestAppContext();
        mTestAppContext.setUp();

        ImsManagerProxy imsManagerProxy =
                mTestAppContext.getSystemServiceProxy(ImsManagerProxy.class);
        when(imsManagerProxy.getImsMmTelManagerProxy(eq(SUB_ID[0])))
                .thenReturn(mImsMmTelManagerProxy);
        setUpSharedPreferences(mTestAppContext.getContext());
    }

    @After
    public void tearDown() throws Exception {
        mSp = null;
        mSpEditor = null;
        mImsMmTelManagerProxy = null;
        mTestConfigMenu = null;
        mInstrumentation = null;
        mTestAppContext.tearDown();
        mTestAppContext = null;
    }

    @Test
    @LargeTest
    public void testOnCreate() throws ImsException {
        setUpActivity(false);

        // onCreate will be invoked when calling startActivity(...).
        verify(mImsMmTelManagerProxy).isCrossSimCallingEnabled();

        for (String propKey : PREF_TO_PROP_BOOL.values()) {
            verify(mSp).getString(eq(propKey), any());
        }

        for (String propKey : PREF_TO_PROP_STRING.values()) {
            verify(mSp).getString(eq(propKey), any());
        }
    }

    @Test
    @SmallTest
    public void testIsValidFragment() {
        setUpActivity(true);

        assertTrue(mTestConfigMenu.isValidFragment("test"));
        assertFalse(mTestConfigMenu.isValidFragment(null));
    }

    @Test
    @LargeTest
    public void testOnPreferenceChangeForCheckBox() throws ImsException {
        setUpActivity(false);

        mInstrumentation.runOnMainSync(() -> {
            for (Map.Entry<String, String> entry : PREF_TO_PROP_BOOL.entrySet()) {
                CheckBoxPreference preference = (CheckBoxPreference)
                        mTestConfigMenu.findPreference(entry.getKey());

                assertNotNull(preference);

                Preference.OnPreferenceChangeListener listener =
                        preference.getOnPreferenceChangeListener();
                listener.onPreferenceChange(preference, "true");

                verify(mSpEditor).putString(eq(entry.getValue()), anyString());
            }
        });

        CheckBoxPreference preference = (CheckBoxPreference)
                mTestConfigMenu.findPreference(TestConfigMenu.KEY_TEST_CROSS_SIM_ENABLED);
        Preference.OnPreferenceChangeListener listener =
                preference.getOnPreferenceChangeListener();

        assertNotNull(preference);

        mInstrumentation.runOnMainSync(() -> {
            listener.onPreferenceChange(preference, "true");
        });

        verify(mImsMmTelManagerProxy).setCrossSimCallingEnabled(eq(true));
    }

    @Test
    @LargeTest
    public void testOnPreferenceChangeForEditText() {
        setUpActivity(false);

        mInstrumentation.runOnMainSync(() -> {
            for (Map.Entry<String, String> entry : PREF_TO_PROP_STRING.entrySet()) {
                if (TestConfigMenu.KEY_TEST_IMS_DEREGISTER.equals(entry.getKey())) {
                    // ListPreference.
                    continue;
                }

                EditTextPreference preference = (EditTextPreference)
                        mTestConfigMenu.findPreference(entry.getKey());

                assertNotNull(preference);

                Preference.OnPreferenceChangeListener listener =
                        preference.getOnPreferenceChangeListener();

                if (TestConfigMenu.KEY_TEST_LOG_OPTIONS.equals(entry.getKey())) {
                    boolean result = listener.onPreferenceChange(preference, "01234567891");

                    assertFalse(result);

                    listener.onPreferenceChange(preference, "1234");

                    verify(mSpEditor).putString(eq(entry.getValue()), eq("0x1234"));
                } else {
                    listener.onPreferenceChange(preference, "test");

                    verify(mSpEditor).putString(eq(entry.getValue()), eq("test"));
                }
            }
        });
    }

    @Test
    @LargeTest
    public void testOnPreferenceChangeForList() {
        setUpActivity(false);
        AosFactory.getInstance().cleanup(SLOT0);

        mInstrumentation.runOnMainSync(() -> {
            ListPreference preference = (ListPreference)
                    mTestConfigMenu.findPreference(TestConfigMenu.KEY_TEST_IMS_DEREGISTER);

            assertNotNull(preference);

            Preference.OnPreferenceChangeListener listener =
                    preference.getOnPreferenceChangeListener();
            listener.onPreferenceChange(preference, "YES");

            verify(mSpEditor).putString(
                    eq(ImsPrivateProperties.Persistent.KEY_TEST_IMS_DEREGISTER), eq("YES"));

            preference = (ListPreference)
                    mTestConfigMenu.findPreference(TestConfigMenu.KEY_TEST_CLEAR_CONFIG);

            assertNotNull(preference);

            listener = preference.getOnPreferenceChangeListener();
            listener.onPreferenceChange(preference, "1");

            for (String key : ImsPrivateProperties.Persistent.TEST_PROPERTIES) {
                verify(mSpEditor).remove(key);
            }

            preference = (ListPreference)
                    mTestConfigMenu.findPreference(TestConfigMenu.KEY_TEST_RESTART_IMSSTACK);

            assertNotNull(preference);

            listener = preference.getOnPreferenceChangeListener();

            assertTrue(listener.onPreferenceChange(preference, "0"));
            // Expected: no actions.

            preference = (ListPreference)
                    mTestConfigMenu.findPreference(TestConfigMenu.KEY_TEST_DEBUG_SCREEN);

            assertNotNull(preference);

            listener = preference.getOnPreferenceChangeListener();

            assertTrue(listener.onPreferenceChange(preference, "0"));
            // Expected: no actions.
        });
    }

    private void setUpActivity(boolean isSmallTest) {
        Intent intent = new Intent(mInstrumentation.getTargetContext(), TestConfigMenu.class);
        intent.putExtra(MSimUtils.EXTRA_KEY_SLOT_ID, SLOT0);

        if (isSmallTest) {
            mInstrumentation.runOnMainSync(() -> {
                try {
                    mTestConfigMenu = (TestConfigMenu) mInstrumentation.newActivity(
                            getClass().getClassLoader(), TestConfigMenu.class.getName(), intent);
                } catch (Exception e) {
                    throw new RuntimeException(e); // nothing to do
                }
            });
        } else {
            intent.addFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
            mTestConfigMenu = (TestConfigMenu) mInstrumentation.startActivitySync(intent);
        }

        assertNotNull(mTestConfigMenu);
    }

    private void setUpSharedPreferences(Context context) {
        when(mSp.edit()).thenReturn(mSpEditor);
        doAnswer(invocation -> {
            return (String) invocation.getArgument(1);
        }).when(mSp).getString(anyString(), anyString());
        doReturn(mSp).when(context).getSharedPreferences(anyString(), anyInt());
    }
}
