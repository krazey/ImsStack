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
import static com.android.imsstack.base.TestAppContext.SUB_ID_1;

import static org.junit.Assert.assertArrayEquals;
import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertFalse;
import static org.junit.Assert.assertThrows;
import static org.mockito.ArgumentMatchers.any;
import static org.mockito.ArgumentMatchers.anyInt;
import static org.mockito.ArgumentMatchers.eq;
import static org.mockito.Mockito.doReturn;
import static org.mockito.Mockito.spy;
import static org.mockito.Mockito.times;
import static org.mockito.Mockito.verify;
import static org.mockito.Mockito.verifyNoMoreInteractions;
import static org.mockito.Mockito.when;

import android.content.res.AssetManager;
import android.content.res.XmlResourceParser;
import android.os.PersistableBundle;
import android.telephony.SubscriptionManager;
import android.testing.AndroidTestingRunner;
import android.testing.TestableLooper;

import androidx.test.filters.SmallTest;
import androidx.test.platform.app.InstrumentationRegistry;

import com.android.imsstack.ContextFixture;
import com.android.imsstack.R;
import com.android.imsstack.base.SystemServiceProxy.CarrierConfigManagerProxy;
import com.android.imsstack.base.TestAppContext;
import com.android.imsstack.core.carrier.SimCarrierId;
import com.android.imsstack.core.config.CarrierConfig;

import org.junit.After;
import org.junit.Before;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.mockito.Mock;
import org.mockito.MockitoAnnotations;

import java.io.IOException;

@RunWith(AndroidTestingRunner.class)
@TestableLooper.RunWithLooper
public class ConfigAgentTest {
    @Mock private ConfigInterface.Listener mListener;

    private XmlResourceParser mCarrierConfigOverrideParser;
    private TestableLooper mTestableLooper;
    private TestAppContext mTestAppContext;
    private ConfigAgent mConfigAgent;

    @Before
    public void setUp() throws Exception {
        MockitoAnnotations.initMocks(this);

        mTestableLooper = TestableLooper.get(this);
        mTestAppContext = new TestAppContext(new ContextFixture().getTestDouble());
        mTestAppContext.setUpWithLooper(mTestableLooper.getLooper());

        mCarrierConfigOverrideParser = InstrumentationRegistry.getInstrumentation().getContext()
                .getResources().getXml(R.xml.carrier_config_override);
        when(mTestAppContext.getContext().getResources().getXml(eq(R.xml.carrier_config_override)))
                .thenReturn(mCarrierConfigOverrideParser);

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
        mConfigAgent.init(mTestAppContext.getContext());

        SimCarrierId scid = new SimCarrierId.Builder().build();
        mConfigAgent.updateCarrierConfig(SUB_ID_1, scid);

        CarrierConfigManagerProxy ccmp =
                mTestAppContext.getSystemServiceProxy(CarrierConfigManagerProxy.class);
        verify(ccmp).getConfigForSubId(eq(SUB_ID_1), any());
    }

    @Test
    @SmallTest
    public void testUpdateCarrierConfigWithCarrierIdAndSpecificCarrierId() throws IOException {
        AssetManager am = mTestAppContext.getContext().getAssets();
        mConfigAgent.init(mTestAppContext.getContext());

        when(am.list(eq(CarrierConfig.CARRIER_CONFIG))).thenReturn(new String[0]);
        SimCarrierId scid = new SimCarrierId.Builder()
                .setCarrierId(1)
                .setSpecificCarrierId(2)
                .build();
        mConfigAgent.updateCarrierConfig(SUB_ID_1, scid);

        CarrierConfigManagerProxy ccmp =
                mTestAppContext.getSystemServiceProxy(CarrierConfigManagerProxy.class);
        verify(ccmp).getConfigForSubId(eq(SUB_ID_1), any());
        verify(am, times(2)).list(eq(CarrierConfig.CARRIER_CONFIG));
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
        injectCarrierConfigBundlesToSystem(carrierConfigBundle);

        // Asset has private keys
        ConfigAgent spyConfigAgent = spy(mConfigAgent);
        doReturn(buildAssetConfigBundle()).when(spyConfigAgent).readCarrierConfig(anyInt(), any());

        // Test updateCarrierConfig()
        spyConfigAgent.init(mTestAppContext.getContext());
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

    private void injectCarrierConfigBundlesToSystem(PersistableBundle bundleWithImsKeys) {
        CarrierConfigManagerProxy ccmp =
                mTestAppContext.getSystemServiceProxy(CarrierConfigManagerProxy.class);
        doReturn(bundleWithImsKeys).when(ccmp).getConfigForSubId(anyInt(), any());
    }

    private void processAllMessages() {
        while (!mTestableLooper.getLooper().getQueue().isIdle()) {
            mTestableLooper.processAllMessages();
        }
    }
}
