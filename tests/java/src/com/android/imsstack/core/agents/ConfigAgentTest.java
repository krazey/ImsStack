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

import static com.android.imsstack.base.TestAppContext.SLOT0;
import static com.android.imsstack.base.TestAppContext.SLOT1;
import static com.android.imsstack.base.TestAppContext.SUB_ID_1;
import static com.android.imsstack.base.TestAppContext.SUB_ID_2;
import static com.android.imsstack.base.ImsPrivateProperties.Persistent.KEY_CONFIG_IMPI;

import static org.junit.Assert.assertArrayEquals;
import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertFalse;
import static org.junit.Assert.assertNotNull;
import static org.junit.Assert.assertNull;
import static org.junit.Assert.assertThrows;
import static org.junit.Assert.assertTrue;
import static org.mockito.ArgumentMatchers.any;
import static org.mockito.ArgumentMatchers.anyBoolean;
import static org.mockito.ArgumentMatchers.anyInt;
import static org.mockito.ArgumentMatchers.anyString;
import static org.mockito.ArgumentMatchers.eq;
import static org.mockito.Mockito.doReturn;
import static org.mockito.Mockito.mock;
import static org.mockito.Mockito.never;
import static org.mockito.Mockito.spy;
import static org.mockito.Mockito.times;
import static org.mockito.Mockito.verify;
import static org.mockito.Mockito.verifyNoMoreInteractions;
import static org.mockito.Mockito.when;

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.content.SharedPreferences;
import android.content.res.AssetManager;
import android.content.res.XmlResourceParser;
import android.os.Bundle;
import android.os.PersistableBundle;
import android.telephony.SubscriptionManager;
import android.testing.AndroidTestingRunner;
import android.testing.TestableLooper;

import androidx.test.filters.SmallTest;
import androidx.test.platform.app.InstrumentationRegistry;

import com.android.imsstack.ContextFixture;
import com.android.imsstack.R;
import com.android.imsstack.base.MSimUtils;
import com.android.imsstack.base.SystemServiceProxy.CarrierConfigManagerProxy;
import com.android.imsstack.base.SystemServiceProxy.SubscriptionManagerProxy;
import com.android.imsstack.base.TestAppContext;
import com.android.imsstack.core.carrier.SimCarrierId;
import com.android.imsstack.core.config.CarrierConfig;
import com.android.imsstack.system.ISystem;
import com.android.imsstack.system.SystemInterface;
import com.android.imsstack.util.Log;

import org.junit.After;
import org.junit.Before;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.mockito.ArgumentCaptor;
import org.mockito.Mock;
import org.mockito.MockitoAnnotations;

import java.io.ByteArrayInputStream;
import java.io.FileInputStream;
import java.io.FileNotFoundException;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.OutputStream;
import java.nio.charset.StandardCharsets;
import java.util.Arrays;

@RunWith(AndroidTestingRunner.class)
@TestableLooper.RunWithLooper
public class ConfigAgentTest {
    private static final int TEST_CARRIER_ID = 1;
    private static final String TEST_ICCID = "1234";
    private static final String CACHED_CONFIG_FILE =
            "carrier_config_slot0_" + TEST_ICCID + "_" + TEST_CARRIER_ID + ".xml";
    private static final String IMS_CONFIG_OVERRIDE_FILE = "ims-config-override.xml";

    private static final String KEY_TEST_BOOL = "ims.test_bool";
    private static final String KEY_TEST_INT = "ims.test_int";
    private static final String KEY_TEST_LONG = "ims.test_long";
    private static final String KEY_TEST_DOUBLE = "ims.test_double";
    private static final String KEY_TEST_STRING = "ims.test_string";
    private static final String KEY_TEST_INT_ARRAY = "ims.test_int_array";
    private static final String KEY_TEST_LONG_ARRAY = "ims.test_long_array";
    private static final String KEY_TEST_DOUBLE_ARRAY = "ims.test_double_array";
    private static final String KEY_TEST_STRING_ARRAY = "ims.test_string_array";
    private static final String KEY_TEST_FLOAT = "ims.test_float";
    private static final String KEY_TEST_BUNDLE = "ims.test_bundle";

    private static final boolean VALUE_TEST_BOOL = true;
    private static final int VALUE_TEST_INT = 10;
    private static final long VALUE_TEST_LONG = 100L;
    private static final double VALUE_TEST_DOUBLE = 1000.0;
    private static final String VALUE_TEST_STRING = "test";
    private static final int[] VALUE_TEST_INT_ARRAY = new int[] { 10, 20 };
    private static final long[] VALUE_TEST_LONG_ARRAY = new long[] { 100L, 200L };
    private static final double[] VALUE_TEST_DOUBLE_ARRAY = new double[] { 1000.0, 2000.0 };
    private static final String[] VALUE_TEST_STRING_ARRAY = new String[] { "test", "config test" };
    private static final float VALUE_TEST_FLOAT = 3.14f;
    private static final String VALUE_CONFIG_IMPI = "impi";

    @Mock private FileInputStream mFileIs;
    @Mock private FileOutputStream mFileOs;
    @Mock private ConfigInterface.Listener mListener;
    @Mock private SystemInterface mSystemInterface;
    @Mock private ISystem mSystem;

    private Context mContext;
    private XmlResourceParser mCarrierConfigOverrideParser;
    private TestableLooper mTestableLooper;
    private TestAppContext mTestAppContext;
    private SubscriptionManagerProxy mSubscriptionManagerProxy;
    private ConfigAgent mConfigAgent;

    @Before
    public void setUp() throws Exception {
        MockitoAnnotations.initMocks(this);

        mTestableLooper = TestableLooper.get(this);
        mTestAppContext = new TestAppContext(new ContextFixture().getTestDouble());
        mTestAppContext.setUpWithLooper(mTestableLooper.getLooper());
        mContext = mTestAppContext.getContext();
        mSubscriptionManagerProxy =
                mTestAppContext.getSystemServiceProxy(SubscriptionManagerProxy.class);

        mCarrierConfigOverrideParser = InstrumentationRegistry.getInstrumentation().getContext()
                .getResources().getXml(R.xml.carrier_config_override);
        when(mContext.getResources().getXml(eq(R.xml.carrier_config_override)))
                .thenReturn(mCarrierConfigOverrideParser);
        doReturn(mFileIs).when(mContext).openFileInput(any());
        doReturn(mFileOs).when(mContext).openFileOutput(any(), anyInt());
        doReturn(true).when(mContext).deleteFile(any());
        doReturn(new String[] { CACHED_CONFIG_FILE }).when(mContext).fileList();
        when(mContext.getAssets().list(any())).thenReturn(new String[0]);
        SystemInterface.setSystemInterface(mSystemInterface);
        when(mSystemInterface.getSystem(eq(SLOT0))).thenReturn(mSystem);
        when(mSubscriptionManagerProxy.getDefaultVoiceSubscriptionId()).thenReturn(SUB_ID_1);

        mConfigAgent = new ConfigAgent(SLOT0);
    }

    @After
    public void tearDown() throws Exception {
        mCarrierConfigOverrideParser = null;

        if (mConfigAgent != null) {
            mConfigAgent.cleanup();
            mConfigAgent = null;
        }

        mTestAppContext.tearDown();
        mTestAppContext = null;
        mTestableLooper = null;
        mContext = null;
    }

    @Test
    @SmallTest
    public void testNotifyCarrierConfigChanged() {
        mConfigAgent.addListener(mListener);
        mConfigAgent.notifyCarrierConfigChanged(SUB_ID_1);
        processAllMessages();

        verify(mListener).onCarrierConfigChanged(eq(SLOT0), eq(SUB_ID_1));

        mConfigAgent.removeListener(mListener);

        verifyNoMoreInteractions(mListener);
    }

    @Test
    @SmallTest
    public void testNotifyCarrierConfigChangedWhenInvalidSubscription() {
        mConfigAgent.addListener(mListener);
        mConfigAgent.notifyCarrierConfigChanged(SubscriptionManager.INVALID_SUBSCRIPTION_ID);
        processAllMessages();

        verify(mListener).onCarrierConfigChanged(
                eq(SLOT0), eq(SubscriptionManager.INVALID_SUBSCRIPTION_ID));

        mConfigAgent.removeListener(mListener);

        verifyNoMoreInteractions(mListener);
    }

    @Test
    @SmallTest
    public void testAddListenerWhenListenerNull() throws IllegalArgumentException {
        assertThrows(IllegalArgumentException.class, () -> {
            mConfigAgent.addListener(null);
        });

        mConfigAgent.addListener(mListener);
    }

    @Test
    @SmallTest
    public void testRemoveListenerWhenListenerNull() throws IllegalArgumentException {
        assertThrows(IllegalArgumentException.class, () -> {
            mConfigAgent.removeListener(null);
        });

        mConfigAgent.removeListener(mListener);
    }

    @Test
    @SmallTest
    public void testGetCarrierConfigWhenUpdateCarrierConfig() {
        mConfigAgent.init(mContext);

        SimCarrierId scid = new SimCarrierId.Builder().build();
        mConfigAgent.updateCarrierConfig(SUB_ID_1, scid);

        CarrierConfigManagerProxy ccmp =
                mTestAppContext.getSystemServiceProxy(CarrierConfigManagerProxy.class);
        // 1st: public carrier configs, 2nd: internal carrier configs
        verify(ccmp, times(2)).getConfigForSubId(eq(SUB_ID_1), any());
    }

    @Test
    @SmallTest
    public void testOverrideHiddenBundleWhenHiddenKeyExists() {
        // CarrierConfigManager bundle contains the AP IMS hidden key
        PersistableBundle configBundle = buildLoadedConfigBundle();
        addHiddenKeyToBundle(configBundle);

        // Test overrideHiddenConfigs()
        mConfigAgent.overrideHiddenConfigs(SUB_ID_1, configBundle);

        // overrideHiddenConfigs() should update private keys from a content of the hidden key
        verifyPublicConfigIsNotUpdated(configBundle);
        verifyHiddenConfigIsUpdated(configBundle);
        verifyHiddenConfigIsNotExist(configBundle);
    }

    @Test
    @SmallTest
    public void testOverrideHiddenBundleWhenHiddenKeyNotExists() {
        // CarrierConfigManager bundle does not have the AP IMS hidden key
        PersistableBundle configBundle = buildLoadedConfigBundle();

        // Test overrideHiddenConfigs()
        mConfigAgent.overrideHiddenConfigs(SUB_ID_1, configBundle);

        // overrideHiddenConfigs() should not update private keys from the hidden key bundle
        verifyPublicConfigIsNotUpdated(configBundle);
        verifyHiddenConfigIsNotUpdated(configBundle);
        verifyHiddenConfigIsNotExist(configBundle);
    }

    @Test
    @SmallTest
    public void testOverrideHiddenBundleWhenHiddenKeyEmpty() {
        // CarrierConfigManager bundle contains the AP IMS hidden key, but it is empty
        PersistableBundle configBundle = buildLoadedConfigBundle();
        configBundle.putPersistableBundle(CarrierConfig.ApIms.KEY_CARRIER_CONFIG_BUNDLE,
                new PersistableBundle());

        // Test overrideHiddenConfigs()
        mConfigAgent.overrideHiddenConfigs(SUB_ID_1, configBundle);

        // overrideHiddenConfigs() should not update private keys from the hidden key bundle
        verifyPublicConfigIsNotUpdated(configBundle);
        verifyHiddenConfigIsNotUpdated(configBundle);
        verifyHiddenConfigIsNotExist(configBundle);
    }

    @Test
    @SmallTest
    public void testOverrideConfigWithHiddenKeyWhenCarrierConfigHasHiddenKey() {
        // CarrierConfig has public IMS keys and the hidden key
        PersistableBundle carrierConfigBundle = buildCarrierConfigBundleWithPublicKeys();
        addHiddenKeyToBundle(carrierConfigBundle);
        setUpCarrierConfig(carrierConfigBundle);

        // Asset has private keys
        ConfigAgent spyConfigAgent = spy(mConfigAgent);
        doReturn(buildAssetConfigBundle()).when(spyConfigAgent).readCarrierConfig(anyInt(), any());

        // Test updateCarrierConfig()
        spyConfigAgent.init(mContext);
        SimCarrierId scid = new SimCarrierId.Builder().build();
        spyConfigAgent.updateCarrierConfig(SUB_ID_1, scid);

        // updateCarrierConfig() should save public IMS keys to CarrierConfig
        CarrierConfig carrierConfig = spyConfigAgent.getCarrierConfig();
        PersistableBundle resultConfig = carrierConfig.getConfig();
        verifyPublicConfigIsUpdated(resultConfig);
        // updateCarrierConfig() should override private keys by a content of the hidden key
        verifyHiddenConfigIsUpdated(resultConfig);
        verifyHiddenConfigIsNotExist(resultConfig);
    }

    @Test
    @SmallTest
    public void testUpdateCarrierConfigWithParentChildRelationship() throws IOException {
        AssetManager am = mContext.getAssets();
        mConfigAgent.init(mContext);

        final String[] testConfigFiles = new String[] {
                "carrier_config_carrierid_20000_Test1.xml",
                "carrier_config_carrierid_20001_Test2.xml" };
        final String keyInt = "ims.ims_test_int";
        final String keyBool = "ims.ims_test_bool";
        final String keyString = "ims.ims_test_string";
        final int valueInt = 200;
        final boolean valueBool = false;
        final String valueString = "ua-string";
        final String xml1 =
                "<?xml version=\"1.0\" encoding=\"utf-8\" standalone=\"yes\"?>\n"
                + "<carrier_config>\n"
                + "<int-array name=\"" + CarrierConfig.KEY_IMS_PARENT_CARRIER_IDS_INT_ARRAY
                        + "\" num=\"0\">\n"
                + "</int-array>\n"
                + "<int name=\"" + keyInt + "\" value=\"100\"/>\n"
                + "<boolean name=\"" + keyBool + "\" value=\"true\"/>\n"
                + "<string name=\"" + keyString + "\">" + valueString + "</string>\n"
                + "</carrier_config>";
        final String xml2 =
                "<?xml version=\"1.0\" encoding=\"utf-8\" standalone=\"yes\"?>\n"
                + "<carrier_config>\n"
                + "<int-array name=\"" + CarrierConfig.KEY_IMS_PARENT_CARRIER_IDS_INT_ARRAY
                        + "\" num=\"1\">\n"
                + "<item value=\"20000\"/>\n"
                + "</int-array>\n"
                + "<int name=\"" + keyInt + "\" value=\"" + valueInt + "\"/>\n"
                + "<boolean name=\"" + keyBool + "\" value=\"" + valueBool + "\"/>\n"
                + "</carrier_config>";
        when(am.list(eq(CarrierConfig.CARRIER_CONFIG))).thenReturn(testConfigFiles);
        when(am.list(eq(CarrierConfig.PUBLIC_CARRIER_CONFIG))).thenReturn(new String[0]);
        ByteArrayInputStream configXml1 =
                new ByteArrayInputStream(xml1.getBytes(StandardCharsets.UTF_8));
        ByteArrayInputStream configXml2 =
                new ByteArrayInputStream(xml2.getBytes(StandardCharsets.UTF_8));
        when(am.open(eq(CarrierConfig.CARRIER_CONFIG + "/" + testConfigFiles[0])))
                .thenReturn(configXml1);
        when(am.open(eq(CarrierConfig.CARRIER_CONFIG + "/" + testConfigFiles[1])))
                .thenReturn(configXml2);
        SimCarrierId scid = new SimCarrierId.Builder()
                .setCarrierId(20001)
                .build();
        mConfigAgent.updateCarrierConfig(SUB_ID_1, scid);

        // 1st: parent carrier configs, 2nd: current carrier configs
        verify(am, times(2)).list(eq(CarrierConfig.CARRIER_CONFIG));

        CarrierConfig cc = mConfigAgent.getCarrierConfig();
        assertEquals(valueInt, cc.getInt(keyInt));
        assertEquals(valueBool, cc.getBoolean(keyBool));
        assertEquals(valueString, cc.getString(keyString));
    }

    @Test
    @SmallTest
    public void testSaveCachedConfigWhenConfigEnabled() throws FileNotFoundException {
        PersistableBundle config = new PersistableBundle();
        config.putBoolean(
                CarrierConfig.KEY_IMS_USE_CONFIG_OF_LAST_INSERTED_SIM_WHEN_NO_SIM_BOOL, true);
        setUpCarrierConfig(config);
        mConfigAgent.init(mContext);

        SimCarrierId scid = new SimCarrierId.Builder()
                .setCarrierId(TEST_CARRIER_ID)
                .setSpecificCarrierId(TEST_CARRIER_ID)
                .setIccId(TEST_ICCID)
                .setSimState(SimCarrierId.SIM_LOADED)
                .build();
        mConfigAgent.updateCarrierConfig(SUB_ID_1, scid);

        // Delete config file
        verify(mContext).deleteFile(eq(CACHED_CONFIG_FILE));
        // Save config file.
        verify(mContext).openFileOutput(eq(CACHED_CONFIG_FILE), anyInt());
    }

    @Test
    @SmallTest
    public void testSaveCachedConfigWhenConfigDisabled() throws FileNotFoundException {
        mConfigAgent.init(mContext);

        SimCarrierId scid = new SimCarrierId.Builder()
                .setCarrierId(TEST_CARRIER_ID)
                .setSpecificCarrierId(TEST_CARRIER_ID)
                .setIccId(TEST_ICCID)
                .setSimState(SimCarrierId.SIM_LOADED)
                .build();
        mConfigAgent.updateCarrierConfig(SUB_ID_1, scid);

        // Delete config file
        verify(mContext).deleteFile(eq(CACHED_CONFIG_FILE));
        verify(mContext, never()).openFileOutput(eq(CACHED_CONFIG_FILE), anyInt());
    }

    @Test
    @SmallTest
    public void testReadCachedConfigOnSimAbsent() throws FileNotFoundException {
        mConfigAgent.init(mContext);

        SimCarrierId scid = new SimCarrierId.Builder()
                .setSimState(SimCarrierId.SIM_ABSENT)
                .build();
        mConfigAgent.updateCarrierConfig(SUB_ID_1, scid);

        // Read config file.
        verify(mContext).openFileInput(eq(CACHED_CONFIG_FILE));
    }

    @Test
    @SmallTest
    public void testReadCachedConfigOnSimLocked() throws FileNotFoundException {
        mConfigAgent.init(mContext);

        // Same ICCID.
        SimCarrierId scid = new SimCarrierId.Builder()
                .setIccId(TEST_ICCID)
                .setSimState(SimCarrierId.SIM_LOCKED)
                .build();

        mConfigAgent.updateCarrierConfig(SUB_ID_1, scid);

        // Read config file.
        verify(mContext).openFileInput(eq(CACHED_CONFIG_FILE));
    }

    @Test
    @SmallTest
    public void testReadCachedConfigOnSimLockedAndDifferentIccId() throws FileNotFoundException {
        mConfigAgent.init(mContext);

        // Different ICCID
        SimCarrierId scid = new SimCarrierId.Builder()
                .setIccId(TEST_ICCID + "5")
                .setSimState(SimCarrierId.SIM_LOCKED)
                .build();

        mConfigAgent.updateCarrierConfig(SUB_ID_1, scid);

        // No interactions.
        verify(mContext, never()).openFileInput(eq(CACHED_CONFIG_FILE));
    }

    @Test
    @SmallTest
    public void testConfigCommandWhenCarrierConfigNotLoadedOrUserBuild() {
        mConfigAgent.init(mContext);

        BroadcastReceiver br = getBroadcastReceiver();

        Intent intent = mock(Intent.class);
        br.onReceive(mContext, intent);

        verify(intent, never()).getAction();
    }

    @Test
    @SmallTest
    public void testConfigCommandWhenSlotNotMatched() {
        BroadcastReceiver br = setUpConfigCommand();
        BroadcastReceiver spyBr = spy(br);
        Intent intent = mock(Intent.class);
        when(intent.getAction()).thenReturn(ConfigAgent.ACTION_GET_CONFIG);

        // When the slot is different.
        when(intent.getIntExtra(eq(ConfigAgent.KEY_SLOT_ID), anyInt())).thenReturn(SLOT1);

        br.onReceive(mContext, intent);

        verify(spyBr, never()).setResultCode(anyInt());

        // When the default voice subscription id is set to different slot.
        when(intent.getIntExtra(eq(ConfigAgent.KEY_SLOT_ID), anyInt())).thenReturn(SLOT0);
        when(mSubscriptionManagerProxy.getDefaultVoiceSubscriptionId()).thenReturn(SUB_ID_2);

        br.onReceive(mContext, intent);

        verify(spyBr, never()).setResultCode(anyInt());
    }

    @Test
    @SmallTest
    public void testConfigCommandWhenNoSim() {
        BroadcastReceiver br = setUpConfigCommand();

        Intent intent = mock(Intent.class);
        when(intent.getAction()).thenReturn(ConfigAgent.ACTION_GET_CONFIG);
        when(mSubscriptionManagerProxy.getSubscriptionId(eq(SLOT0)))
                .thenReturn(MSimUtils.INVALID_SUB_ID);

        br.onReceive(mContext, intent);

        // Default slot id is used.
        assertEquals(ConfigAgent.RESULT_OK, br.getResultCode());
    }

    @Test
    @SmallTest
    public void testConfigCommandSetConfig() {
        BroadcastReceiver receiver = setUpConfigCommand();

        testSetConfig(receiver);
    }

    @Test
    @SmallTest
    public void testConfigCommandSetConfigFromXml() {
        BroadcastReceiver receiver = setUpConfigCommand();

        testSetConfig(receiver);
        String configOverrideFile = createImsConfigOverrideFile();

        assertNotNull(configOverrideFile);

        Bundle extras = new Bundle();
        extras.putString(ConfigAgent.KEY_CONFIG_FROM_XML, configOverrideFile);

        Intent intent = mock(Intent.class);
        when(intent.getAction()).thenReturn(ConfigAgent.ACTION_SET_CONFIG);
        when(intent.getIntExtra(eq(ConfigAgent.KEY_SLOT_ID), anyInt())).thenReturn(SLOT0);
        when(intent.getBooleanExtra(eq(ConfigAgent.KEY_COMMITTABLE), anyBoolean()))
                .thenReturn(true);
        when(intent.getExtras()).thenReturn(extras);

        receiver.onReceive(mContext, intent);

        assertEquals(ConfigAgent.RESULT_COMMITTABLE_OK, receiver.getResultCode());

        PersistableBundle config = mConfigAgent.getCarrierConfig().getConfig();
        assertEquals(VALUE_TEST_INT, config.getInt(KEY_TEST_INT));
        assertEquals(VALUE_TEST_STRING, config.getString(KEY_TEST_STRING));

        PersistableBundle bundleConfig = config.getPersistableBundle(KEY_TEST_BUNDLE);
        assertNotNull(bundleConfig);
        assertEquals(VALUE_TEST_INT, bundleConfig.getInt(KEY_TEST_INT));
        assertEquals(VALUE_TEST_STRING, bundleConfig.getString(KEY_TEST_STRING));
        assertTrue(Arrays.equals(
                VALUE_TEST_INT_ARRAY, bundleConfig.getIntArray(KEY_TEST_INT_ARRAY)));
    }

    @Test
    @SmallTest
    public void testConfigCommandSetConfigForStringArrayWithComma() {
        final String[] actualValues = new String[] { "380:5,0", "380:6,1" };
        final String[] valuesWithComma = new String[] { "380:5\\,0", "380:6\\,1" };
        BroadcastReceiver receiver = setUpConfigCommand();
        Bundle extras = new Bundle();
        extras.putStringArray(KEY_TEST_STRING_ARRAY, valuesWithComma);

        Intent intent = mock(Intent.class);
        when(intent.getAction()).thenReturn(ConfigAgent.ACTION_SET_CONFIG);
        when(intent.getIntExtra(eq(ConfigAgent.KEY_SLOT_ID), anyInt())).thenReturn(SLOT0);
        when(intent.getBooleanExtra(eq(ConfigAgent.KEY_COMMITTABLE), anyBoolean()))
                .thenReturn(true);
        when(intent.getExtras()).thenReturn(extras);

        receiver.onReceive(mContext, intent);

        assertEquals(ConfigAgent.RESULT_COMMITTABLE_OK, receiver.getResultCode());

        PersistableBundle config = mConfigAgent.getCarrierConfig().getConfig();
        assertTrue(Arrays.equals(actualValues, config.getStringArray(KEY_TEST_STRING_ARRAY)));
    }

    @Test
    @SmallTest
    public void testConfigCommandGetConfig() {
        BroadcastReceiver receiver = setUpConfigCommand();

        testSetConfig(receiver);

        Intent intent = mock(Intent.class);
        when(intent.getAction()).thenReturn(ConfigAgent.ACTION_GET_CONFIG);
        when(intent.getIntExtra(eq(ConfigAgent.KEY_SLOT_ID), anyInt())).thenReturn(SLOT0);
        when(intent.getBooleanExtra(eq(ConfigAgent.KEY_COMMITTABLE), anyBoolean()))
                .thenReturn(false);

        receiver.onReceive(mContext, intent);

        assertEquals(ConfigAgent.RESULT_OK, receiver.getResultCode());
        // "config_keys" is not specified.
        assertNull(receiver.getResultData());

        SharedPreferences sharedPref = mock(SharedPreferences.class);
        doReturn(sharedPref).when(mContext).getSharedPreferences(anyString(), anyInt());
        doReturn(VALUE_CONFIG_IMPI).when(sharedPref).getString(eq(KEY_CONFIG_IMPI), anyString());
        when(intent.getStringArrayExtra(eq(ConfigAgent.KEY_CONFIG_KEYS)))
                .thenReturn(
                        new String[] {
                            KEY_TEST_BOOL,
                            KEY_TEST_INT,
                            KEY_TEST_LONG,
                            KEY_TEST_DOUBLE,
                            KEY_TEST_STRING,
                            KEY_TEST_INT_ARRAY,
                            KEY_TEST_LONG_ARRAY,
                            KEY_TEST_DOUBLE_ARRAY,
                            KEY_TEST_STRING_ARRAY,
                            KEY_CONFIG_IMPI
                        });

        receiver.onReceive(mContext, intent);

        assertEquals(ConfigAgent.RESULT_OK, receiver.getResultCode());

        String resultData = receiver.getResultData();
        assertNotNull(resultData);
        assertTrue(resultData.contains(KEY_TEST_BOOL));
        assertTrue(resultData.contains(KEY_TEST_INT));
        assertTrue(resultData.contains(KEY_TEST_LONG));
        assertTrue(resultData.contains(KEY_TEST_DOUBLE));
        assertTrue(resultData.contains(KEY_TEST_STRING));
        assertTrue(resultData.contains(KEY_TEST_INT_ARRAY));
        assertTrue(resultData.contains(KEY_TEST_LONG_ARRAY));
        assertTrue(resultData.contains(KEY_TEST_DOUBLE_ARRAY));
        assertTrue(resultData.contains(KEY_TEST_STRING_ARRAY));
        assertTrue(resultData.contains(KEY_CONFIG_IMPI));
    }

    @Test
    @SmallTest
    public void testConfigCommandClearConfig() {
        BroadcastReceiver receiver = setUpConfigCommand();

        testSetConfig(receiver);

        Intent intent = mock(Intent.class);
        when(intent.getAction()).thenReturn(ConfigAgent.ACTION_CLEAR_CONFIG);
        when(intent.getIntExtra(eq(ConfigAgent.KEY_SLOT_ID), anyInt())).thenReturn(SLOT0);
        when(intent.getBooleanExtra(eq(ConfigAgent.KEY_COMMITTABLE), anyBoolean()))
                .thenReturn(true);

        // Clear the specified configs.
        when(intent.getStringArrayExtra(eq(ConfigAgent.KEY_CONFIG_KEYS)))
                .thenReturn(
                        new String[] {
                            KEY_TEST_BOOL,
                            KEY_TEST_INT,
                        });

        receiver.onReceive(mContext, intent);

        assertEquals(ConfigAgent.RESULT_COMMITTABLE_OK, receiver.getResultCode());

        PersistableBundle config = mConfigAgent.getCarrierConfig().getConfig();
        assertFalse(config.containsKey(KEY_TEST_BOOL));
        assertFalse(config.containsKey(KEY_TEST_INT));
        assertTrue(config.containsKey(KEY_TEST_LONG));
        assertTrue(config.containsKey(KEY_TEST_DOUBLE));
        assertTrue(config.containsKey(KEY_TEST_STRING));
        assertTrue(config.containsKey(KEY_TEST_INT_ARRAY));
        assertTrue(config.containsKey(KEY_TEST_LONG_ARRAY));
        assertTrue(config.containsKey(KEY_TEST_DOUBLE_ARRAY));
        assertTrue(config.containsKey(KEY_TEST_STRING_ARRAY));

        // Clear all configs.
        when(intent.getStringArrayExtra(eq(ConfigAgent.KEY_CONFIG_KEYS)))
                .thenReturn(null);
        receiver.onReceive(mContext, intent);

        assertEquals(ConfigAgent.RESULT_COMMITTABLE_OK, receiver.getResultCode());

        config = mConfigAgent.getCarrierConfig().getConfig();
        assertFalse(config.containsKey(KEY_TEST_BOOL));
        assertFalse(config.containsKey(KEY_TEST_INT));
        assertFalse(config.containsKey(KEY_TEST_LONG));
        assertFalse(config.containsKey(KEY_TEST_DOUBLE));
        assertFalse(config.containsKey(KEY_TEST_STRING));
        assertFalse(config.containsKey(KEY_TEST_INT_ARRAY));
        assertFalse(config.containsKey(KEY_TEST_LONG_ARRAY));
        assertFalse(config.containsKey(KEY_TEST_DOUBLE_ARRAY));
        assertFalse(config.containsKey(KEY_TEST_STRING_ARRAY));
    }

    private void setUpCarrierConfig(PersistableBundle config) {
        CarrierConfigManagerProxy ccmp =
                mTestAppContext.getSystemServiceProxy(CarrierConfigManagerProxy.class);
        doReturn(config).when(ccmp).getConfigForSubId(anyInt(), any());
    }

    private BroadcastReceiver setUpConfigCommand() {
        mConfigAgent.init(mContext);
        SimCarrierId scid = new SimCarrierId.Builder()
                .setCarrierId(TEST_CARRIER_ID)
                .setSpecificCarrierId(TEST_CARRIER_ID)
                .setIccId(TEST_ICCID)
                .setSimState(SimCarrierId.SIM_LOADED)
                .build();
        mConfigAgent.updateCarrierConfig(SUB_ID_1, scid);
        return getBroadcastReceiver();
    }

    private BroadcastReceiver getBroadcastReceiver() {
        ArgumentCaptor<BroadcastReceiver> receiverCaptor =
                ArgumentCaptor.forClass(BroadcastReceiver.class);
        verify(mTestAppContext.getBroadcastReceiverProxy())
                .registerReceiver(receiverCaptor.capture(), any(IntentFilter.class));
        BroadcastReceiver br = receiverCaptor.getValue();
        br.setPendingResult(new BroadcastReceiver.PendingResult(
                /* resultCode */ 0,
                /* resultData*/ null,
                /* resultExtras */ null,
                /* type */ 0,
                /* ordered */ true,
                /* sticky */ false,
                /* token */ null,
                /* userId */ 0,
                /* flags */ 0));
        return br;
    }

    private void testSetConfig(BroadcastReceiver receiver) {
        Bundle extras = new Bundle();
        extras.putBoolean(KEY_TEST_BOOL, VALUE_TEST_BOOL);
        extras.putInt(KEY_TEST_INT, VALUE_TEST_INT);
        extras.putLong(KEY_TEST_LONG, VALUE_TEST_LONG);
        extras.putDouble(KEY_TEST_DOUBLE, VALUE_TEST_DOUBLE);
        extras.putString(KEY_TEST_STRING, VALUE_TEST_STRING);
        extras.putIntArray(KEY_TEST_INT_ARRAY, VALUE_TEST_INT_ARRAY);
        extras.putLongArray(KEY_TEST_LONG_ARRAY, VALUE_TEST_LONG_ARRAY);
        extras.putDoubleArray(KEY_TEST_DOUBLE_ARRAY, VALUE_TEST_DOUBLE_ARRAY);
        extras.putStringArray(KEY_TEST_STRING_ARRAY, VALUE_TEST_STRING_ARRAY);
        extras.putFloat(KEY_TEST_FLOAT, VALUE_TEST_FLOAT);
        extras.putString(KEY_CONFIG_IMPI, VALUE_CONFIG_IMPI);
        extras.putString(ConfigAgent.KEY_CONFIG_FROM_XML,
                "/product/etc/" + IMS_CONFIG_OVERRIDE_FILE);

        SharedPreferences sharedPref = mock(SharedPreferences.class);
        SharedPreferences.Editor spEditor = mock(SharedPreferences.Editor.class);
        doReturn(sharedPref).when(mContext).getSharedPreferences(anyString(), anyInt());
        doReturn(spEditor).when(sharedPref).edit();

        Intent intent = mock(Intent.class);
        when(intent.getAction()).thenReturn(ConfigAgent.ACTION_SET_CONFIG);
        when(intent.getIntExtra(eq(ConfigAgent.KEY_SLOT_ID), anyInt())).thenReturn(SLOT0);
        when(intent.getBooleanExtra(eq(ConfigAgent.KEY_COMMITTABLE), anyBoolean()))
                .thenReturn(true);
        when(intent.getExtras()).thenReturn(extras);

        receiver.onReceive(mContext, intent);

        assertEquals(ConfigAgent.RESULT_COMMITTABLE_OK, receiver.getResultCode());
        assertNotNull(receiver.getResultData());
        // unprocessed key: ims.test_float
        assertTrue(receiver.getResultData().contains(KEY_TEST_FLOAT));
        // unprocessed key: ConfigAgent#KEY_CONFIG_FROM_XML because the file doesn't exist.
        assertTrue(receiver.getResultData().contains(ConfigAgent.KEY_CONFIG_FROM_XML));
        verify(spEditor).putString(eq(KEY_CONFIG_IMPI), eq(VALUE_CONFIG_IMPI));

        PersistableBundle config = mConfigAgent.getCarrierConfig().getConfig();
        assertEquals(VALUE_TEST_BOOL, config.getBoolean(KEY_TEST_BOOL));
        assertEquals(VALUE_TEST_INT, config.getInt(KEY_TEST_INT));
        assertEquals(VALUE_TEST_LONG, config.getLong(KEY_TEST_LONG));
        assertEquals(0, Double.compare(VALUE_TEST_DOUBLE, config.getDouble(KEY_TEST_DOUBLE)));
        assertEquals(VALUE_TEST_STRING, config.getString(KEY_TEST_STRING));
        assertTrue(Arrays.equals(VALUE_TEST_INT_ARRAY, config.getIntArray(KEY_TEST_INT_ARRAY)));
        assertTrue(Arrays.equals(VALUE_TEST_LONG_ARRAY, config.getLongArray(KEY_TEST_LONG_ARRAY)));
        assertTrue(Arrays.equals(
                VALUE_TEST_DOUBLE_ARRAY, config.getDoubleArray(KEY_TEST_DOUBLE_ARRAY)));
        assertTrue(Arrays.equals(
                VALUE_TEST_STRING_ARRAY, config.getStringArray(KEY_TEST_STRING_ARRAY)));
    }

    private String createImsConfigOverrideFile() {
        Context c = InstrumentationRegistry.getInstrumentation().getContext();
        c.deleteFile(IMS_CONFIG_OVERRIDE_FILE);

        PersistableBundle bundleConfig = new PersistableBundle();
        bundleConfig.putInt(KEY_TEST_INT, VALUE_TEST_INT);
        bundleConfig.putString(KEY_TEST_STRING, VALUE_TEST_STRING);
        bundleConfig.putIntArray(KEY_TEST_INT_ARRAY, VALUE_TEST_INT_ARRAY);

        PersistableBundle config = new PersistableBundle();
        config.putInt(KEY_TEST_INT, VALUE_TEST_INT);
        config.putString(KEY_TEST_STRING, VALUE_TEST_STRING);
        config.putPersistableBundle(KEY_TEST_BUNDLE, bundleConfig);

        try (OutputStream os = c.openFileOutput(IMS_CONFIG_OVERRIDE_FILE, Context.MODE_APPEND)) {
            config.writeToStream(os);
            return c.getFilesDir().getAbsolutePath() + "/" + IMS_CONFIG_OVERRIDE_FILE;
        } catch (IOException e) {
            Log.dc(this, "createImsConfigOverride: " + e.toString());
            return null;
        }
    }

    private void processAllMessages() {
        while (!mTestableLooper.getLooper().getQueue().isIdle()) {
            mTestableLooper.processAllMessages();
        }
    }

    private static PersistableBundle buildLoadedConfigBundle() {
        PersistableBundle configBundle = new PersistableBundle();

        configBundle.putInt("TEST_KEY_PUBLIC_INT", 1);
        configBundle.putInt("TEST_KEY_PRIVATE_INT", 0);
        configBundle.putString("TEST_KEY_PUBLIC_STRING", "static_value");
        configBundle.putString("TEST_KEY_PRIVATE_STRING", "default_value");
        configBundle.putBoolean("TEST_KEY_PUBLIC_BOOLEAN", false);
        configBundle.putBoolean("TEST_KEY_PRIVATE_BOOLEAN", true);
        configBundle.putIntArray("TEST_KEY_PUBLIC_INT_ARRAY", new int[]{0, 1});
        configBundle.putIntArray("TEST_KEY_PRIVATE_INT_ARRAY", null);
        PersistableBundle nestedBundle = new PersistableBundle();
        nestedBundle.putInt("NESTED_KEY_INT", -1);
        configBundle.putPersistableBundle("TEST_KEY_PRIVATE_BUNDLE", nestedBundle);

        return configBundle;
    }

    private static PersistableBundle buildCarrierConfigBundleWithPublicKeys() {
        PersistableBundle carrierConfigBundle = new PersistableBundle();

        carrierConfigBundle.putInt("TEST_KEY_PUBLIC_INT", 100);
        carrierConfigBundle.putString("TEST_KEY_PUBLIC_STRING", "public_value");
        carrierConfigBundle.putBoolean("TEST_KEY_PUBLIC_BOOLEAN", true);
        carrierConfigBundle.putIntArray("TEST_KEY_PUBLIC_INT_ARRAY", new int[]{9999});

        return carrierConfigBundle;
    }

    private static void addHiddenKeyToBundle(PersistableBundle bundle) {
        PersistableBundle hiddenKeyBundle = new PersistableBundle();

        hiddenKeyBundle.putInt("TEST_KEY_PRIVATE_INT", 1);
        hiddenKeyBundle.putString("TEST_KEY_PRIVATE_STRING", "overridden_value");
        hiddenKeyBundle.putBoolean("TEST_KEY_PRIVATE_BOOLEAN", false);
        hiddenKeyBundle.putIntArray("TEST_KEY_PRIVATE_INT_ARRAY", new int[]{3600, 1800});
        PersistableBundle nestedBundleInHidden = new PersistableBundle();
        nestedBundleInHidden.putInt("NESTED_KEY_INT", 11);
        nestedBundleInHidden.putString("NESTED_KEY_STRING", "volte");
        hiddenKeyBundle.putPersistableBundle("TEST_KEY_PRIVATE_BUNDLE", nestedBundleInHidden);

        bundle.putPersistableBundle(CarrierConfig.ApIms.KEY_CARRIER_CONFIG_BUNDLE,
                hiddenKeyBundle);
    }

    private static PersistableBundle buildAssetConfigBundle() {
        PersistableBundle configBundle = new PersistableBundle();

        configBundle.putInt("TEST_KEY_PRIVATE_INT", 0);
        configBundle.putString("TEST_KEY_PRIVATE_STRING", "default_value");
        configBundle.putBoolean("TEST_KEY_PRIVATE_BOOLEAN", true);
        configBundle.putIntArray("TEST_KEY_PRIVATE_INT_ARRAY", null);
        PersistableBundle nestedBundle = new PersistableBundle();
        nestedBundle.putInt("NESTED_KEY_INT", -1);
        configBundle.putPersistableBundle("TEST_KEY_PRIVATE_BUNDLE", nestedBundle);

        return configBundle;
    }

    private static void verifyPublicConfigIsUpdated(PersistableBundle configBundle) {
        assertEquals(100, configBundle.getInt("TEST_KEY_PUBLIC_INT"));
        assertEquals("public_value", configBundle.getString("TEST_KEY_PUBLIC_STRING"));
        assertEquals(true, configBundle.getBoolean("TEST_KEY_PUBLIC_BOOLEAN"));
        assertArrayEquals(new int[]{9999}, configBundle.getIntArray("TEST_KEY_PUBLIC_INT_ARRAY"));
    }

    private static void verifyPublicConfigIsNotUpdated(PersistableBundle configBundle) {
        assertEquals(1, configBundle.getInt("TEST_KEY_PUBLIC_INT"));
        assertEquals("static_value", configBundle.getString("TEST_KEY_PUBLIC_STRING"));
        assertEquals(false, configBundle.getBoolean("TEST_KEY_PUBLIC_BOOLEAN"));
        assertArrayEquals(new int[]{0, 1}, configBundle.getIntArray("TEST_KEY_PUBLIC_INT_ARRAY"));
    }

    private static void verifyHiddenConfigIsUpdated(PersistableBundle configBundle) {
        assertEquals(1, configBundle.getInt("TEST_KEY_PRIVATE_INT"));
        assertEquals("overridden_value", configBundle.getString("TEST_KEY_PRIVATE_STRING"));
        assertEquals(false, configBundle.getBoolean("TEST_KEY_PRIVATE_BOOLEAN"));
        assertArrayEquals(new int[]{3600, 1800},
                configBundle.getIntArray("TEST_KEY_PRIVATE_INT_ARRAY"));
        PersistableBundle outputNestedBundle =
                configBundle.getPersistableBundle("TEST_KEY_PRIVATE_BUNDLE");
        assertEquals(11, outputNestedBundle.getInt("NESTED_KEY_INT"));
        assertEquals("volte", outputNestedBundle.getString("NESTED_KEY_STRING"));
    }

    private static void verifyHiddenConfigIsNotUpdated(PersistableBundle configBundle) {
        assertEquals(0, configBundle.getInt("TEST_KEY_PRIVATE_INT"));
        assertEquals("default_value", configBundle.getString("TEST_KEY_PRIVATE_STRING"));
        assertEquals(true, configBundle.getBoolean("TEST_KEY_PRIVATE_BOOLEAN"));
        assertEquals(null, configBundle.getIntArray("TEST_KEY_PRIVATE_INT_ARRAY"));
        PersistableBundle outputNestedBundle =
                configBundle.getPersistableBundle("TEST_KEY_PRIVATE_BUNDLE");
        assertEquals(-1, outputNestedBundle.getInt("NESTED_KEY_INT"));
    }

    private static void verifyHiddenConfigIsNotExist(PersistableBundle configBundle) {
        assertFalse(configBundle.containsKey(CarrierConfig.ApIms.KEY_CARRIER_CONFIG_BUNDLE));
    }
}
