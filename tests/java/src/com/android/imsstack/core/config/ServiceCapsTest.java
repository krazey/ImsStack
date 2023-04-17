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

import android.content.Context;
import android.os.PersistableBundle;
import android.telephony.CarrierConfigManager;
import android.test.suitebuilder.annotation.SmallTest;

import com.android.imsstack.ContextFixture;

import org.junit.After;
import org.junit.Before;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.junit.runners.JUnit4;

@RunWith(JUnit4.class)
public class ServiceCapsTest {
    private static final int SLOT0 = 0;
    private static final int SUB_ID = 1;

    private ContextFixture mContextFixture;
    private Context mContext;
    private PersistableBundle mConfigBundle;

    @Before
    public void setup() {
        mContextFixture = new ContextFixture();
        mContext = mContextFixture.getTestDouble();
        when(mContext.getResources().getBoolean(anyInt())).thenReturn(true);
        mConfigBundle = new PersistableBundle();
        CarrierConfigManager ccm = mContext.getSystemService(CarrierConfigManager.class);
        when(ccm.getConfigForSubId(eq(SUB_ID), any())).thenReturn(mConfigBundle);
    }

    @After
    public  void tearDown() {
        ServiceCaps.clear();
        mConfigBundle = null;
        mContext = null;
        mContextFixture = null;
    }

    @Test
    @SmallTest
    public void testGetServiceCaps() {
        ServiceCaps sc = ServiceCaps.getServiceCaps(SLOT0);

        assertNotNull(sc);
        assertFalse(sc.isVoLteEnabled());
        assertFalse(sc.isVtEnabled());
        assertFalse(sc.isWfcEnabled());
        assertNotNull(sc.toString());
        assertFalse(ServiceCaps.isVoLteEnabledByPlatform(SLOT0));
        assertFalse(ServiceCaps.isVtEnabledByPlatform(SLOT0));
        assertFalse(ServiceCaps.isWfcEnabledByPlatform(SLOT0));
    }

    @Test
    @SmallTest
    public void testUpdateServiceCapabilitiesWhenCarrierConfigEnabled() {
        setUpServiceCapabilities(true);
        ServiceCaps.updateServiceCapabilities(mContext, SLOT0, SUB_ID);
        ServiceCaps sc = ServiceCaps.getServiceCaps(SLOT0);

        assertNotNull(sc);
        assertTrue(sc.isVoLteEnabled());
        assertTrue(sc.isVtEnabled());
        assertTrue(sc.isWfcEnabled());
        assertNotNull(sc.toString());
        assertTrue(ServiceCaps.isVoLteEnabledByPlatform(SLOT0));
        assertTrue(ServiceCaps.isVtEnabledByPlatform(SLOT0));
        assertTrue(ServiceCaps.isWfcEnabledByPlatform(SLOT0));
    }

    @Test
    @SmallTest
    public void testUpdateServiceCapabilitiesWhenCarrierConfigDisabled() {
        setUpServiceCapabilities(false);
        ServiceCaps.updateServiceCapabilities(mContext, SLOT0, SUB_ID);
        ServiceCaps sc = ServiceCaps.getServiceCaps(SLOT0);

        assertNotNull(sc);
        assertFalse(sc.isVoLteEnabled());
        assertFalse(sc.isVtEnabled());
        assertFalse(sc.isWfcEnabled());
        assertNotNull(sc.toString());
        assertFalse(ServiceCaps.isVoLteEnabledByPlatform(SLOT0));
        assertFalse(ServiceCaps.isVtEnabledByPlatform(SLOT0));
        assertFalse(ServiceCaps.isWfcEnabledByPlatform(SLOT0));
    }

    private void setUpServiceCapabilities(boolean enabled) {
        mConfigBundle.putBoolean(CarrierConfigManager.KEY_CARRIER_VOLTE_AVAILABLE_BOOL, enabled);
        mConfigBundle.putBoolean(CarrierConfigManager.KEY_CARRIER_VT_AVAILABLE_BOOL, enabled);
        mConfigBundle.putBoolean(CarrierConfigManager.KEY_CARRIER_WFC_IMS_AVAILABLE_BOOL, enabled);
    }
}
