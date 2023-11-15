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

package com.android.imsstack.core.config;

import static org.junit.Assert.assertFalse;
import static org.junit.Assert.assertNotNull;
import static org.junit.Assert.assertTrue;
import static org.mockito.ArgumentMatchers.any;
import static org.mockito.ArgumentMatchers.anyInt;
import static org.mockito.ArgumentMatchers.eq;
import static org.mockito.Mockito.when;

import android.os.PersistableBundle;
import android.telephony.CarrierConfigManager;

import androidx.test.filters.SmallTest;

import com.android.imsstack.ContextFixture;
import com.android.imsstack.base.SystemServiceProxy.CarrierConfigManagerProxy;
import com.android.imsstack.base.TestAppContext;

import org.junit.After;
import org.junit.Before;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.junit.runners.JUnit4;

@RunWith(JUnit4.class)
public class ServiceCapsTest {
    private PersistableBundle mConfigBundle;
    private TestAppContext mTestAppContext;

    @Before
    public void setUp() {
        mConfigBundle = new PersistableBundle();
        mTestAppContext = new TestAppContext(new ContextFixture().getTestDouble());
        mTestAppContext.setUp();
        when(mTestAppContext.getContext().getResources().getBoolean(anyInt())).thenReturn(true);
        CarrierConfigManagerProxy ccmp =
                mTestAppContext.getSystemServiceProxy(CarrierConfigManagerProxy.class);
        when(ccmp.getConfigForSubId(eq(TestAppContext.SUB_ID_1), any())).thenReturn(mConfigBundle);
    }

    @After
    public void tearDown() {
        ServiceCaps.clear();
        mConfigBundle = null;
        mTestAppContext.tearDown();
        mTestAppContext = null;
    }

    @Test
    @SmallTest
    public void testGetServiceCaps() {
        ServiceCaps sc = ServiceCaps.getServiceCaps(TestAppContext.SLOT0);

        assertNotNull(sc);
        assertFalse(sc.isVoLteEnabled());
        assertFalse(sc.isVtEnabled());
        assertFalse(sc.isWfcEnabled());
        assertNotNull(sc.toString());
        assertFalse(ServiceCaps.isVoLteEnabledByPlatform(TestAppContext.SLOT0));
        assertFalse(ServiceCaps.isVtEnabledByPlatform(TestAppContext.SLOT0));
        assertFalse(ServiceCaps.isWfcEnabledByPlatform(TestAppContext.SLOT0));
    }

    @Test
    @SmallTest
    public void testUpdateServiceCapabilitiesWhenCarrierConfigEnabled() {
        setUpServiceCapabilities(true);
        ServiceCaps.updateServiceCapabilities(
                mTestAppContext.getContext(), TestAppContext.SLOT0, TestAppContext.SUB_ID_1);
        ServiceCaps sc = ServiceCaps.getServiceCaps(TestAppContext.SLOT0);

        assertNotNull(sc);
        assertTrue(sc.isVoLteEnabled());
        assertTrue(sc.isVtEnabled());
        assertTrue(sc.isWfcEnabled());
        assertNotNull(sc.toString());
        assertTrue(ServiceCaps.isVoLteEnabledByPlatform(TestAppContext.SLOT0));
        assertTrue(ServiceCaps.isVtEnabledByPlatform(TestAppContext.SLOT0));
        assertTrue(ServiceCaps.isWfcEnabledByPlatform(TestAppContext.SLOT0));
    }

    @Test
    @SmallTest
    public void testUpdateServiceCapabilitiesWhenCarrierConfigDisabled() {
        setUpServiceCapabilities(false);
        ServiceCaps.updateServiceCapabilities(
                mTestAppContext.getContext(), TestAppContext.SLOT0, TestAppContext.SUB_ID_1);
        ServiceCaps sc = ServiceCaps.getServiceCaps(TestAppContext.SLOT0);

        assertNotNull(sc);
        assertFalse(sc.isVoLteEnabled());
        assertFalse(sc.isVtEnabled());
        assertFalse(sc.isWfcEnabled());
        assertNotNull(sc.toString());
        assertFalse(ServiceCaps.isVoLteEnabledByPlatform(TestAppContext.SLOT0));
        assertFalse(ServiceCaps.isVtEnabledByPlatform(TestAppContext.SLOT0));
        assertFalse(ServiceCaps.isWfcEnabledByPlatform(TestAppContext.SLOT0));
    }

    private void setUpServiceCapabilities(boolean enabled) {
        mConfigBundle.putBoolean(CarrierConfigManager.KEY_CARRIER_VOLTE_AVAILABLE_BOOL, enabled);
        mConfigBundle.putBoolean(CarrierConfigManager.KEY_CARRIER_VT_AVAILABLE_BOOL, enabled);
        mConfigBundle.putBoolean(CarrierConfigManager.KEY_CARRIER_WFC_IMS_AVAILABLE_BOOL, enabled);
    }
}
