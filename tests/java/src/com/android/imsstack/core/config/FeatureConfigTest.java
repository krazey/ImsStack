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
import static org.junit.Assert.assertTrue;
import static org.mockito.ArgumentMatchers.any;
import static org.mockito.ArgumentMatchers.eq;
import static org.mockito.Mockito.when;

import android.os.PersistableBundle;
import android.telephony.CarrierConfigManager;

import androidx.test.filters.SmallTest;

import com.android.imsstack.base.MSimUtils;
import com.android.imsstack.base.SystemServiceProxy.CarrierConfigManagerProxy;
import com.android.imsstack.base.TestAppContext;

import org.junit.After;
import org.junit.Before;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.junit.runners.JUnit4;

@RunWith(JUnit4.class)
public class FeatureConfigTest {
    private TestAppContext mTestAppContext;

    @Before
    public void setUp() throws Exception {
        mTestAppContext = new TestAppContext();
        mTestAppContext.setUp();
    }

    @After
    public void tearDown() throws Exception {
        mTestAppContext.tearDown();
        mTestAppContext = null;
    }

    @Test
    @SmallTest
    public void testInitialValue() {
        assertFalse(FeatureConfig.isEnabled(TestAppContext.SLOT0, ""));
        assertFalse(FeatureConfig.isEnabled(TestAppContext.SLOT0, FeatureConfig.VOLTE));
        assertFalse(FeatureConfig.isEnabled(TestAppContext.SLOT0, FeatureConfig.SMS));
        assertFalse(FeatureConfig.isEnabled(TestAppContext.SLOT0, FeatureConfig.UCE));
        assertFalse(FeatureConfig.isEnabled(TestAppContext.SLOT0, FeatureConfig.VOWIFI));
        assertFalse(FeatureConfig.isEnabled(TestAppContext.SLOT0, FeatureConfig.VT));

        // When feature does not exist.
        assertFalse(FeatureConfig.isEnabled(MSimUtils.INVALID_SLOT_ID, FeatureConfig.VOLTE));
    }

    @Test
    @SmallTest
    public void testInit() {
        PersistableBundle bundle = new PersistableBundle();
        bundle.putBoolean(CarrierConfigManager.KEY_CARRIER_VOLTE_AVAILABLE_BOOL, true);
        bundle.putBoolean(CarrierConfigManager.KEY_CARRIER_VT_AVAILABLE_BOOL, true);
        bundle.putBoolean(CarrierConfigManager.ImsSms.KEY_SMS_OVER_IMS_SUPPORTED_BOOL, true);
        bundle.putBoolean(CarrierConfigManager.KEY_CARRIER_WFC_IMS_AVAILABLE_BOOL, true);
        bundle.putBoolean(CarrierConfigManager.Ims.KEY_ENABLE_PRESENCE_PUBLISH_BOOL, true);
        bundle.putBoolean(CarrierConfigManager.KEY_USE_RCS_SIP_OPTIONS_BOOL, true);
        CarrierConfigManagerProxy ccmp =
                mTestAppContext.getSystemServiceProxy(CarrierConfigManagerProxy.class);
        when(ccmp.getConfigForSubId(eq(TestAppContext.SUB_ID_1), any())).thenReturn(bundle);

        FeatureConfig.init(TestAppContext.SLOT0);
        assertTrue(FeatureConfig.isEnabled(TestAppContext.SLOT0, FeatureConfig.VOLTE));
        assertTrue(FeatureConfig.isEnabled(TestAppContext.SLOT0, FeatureConfig.SMS));
        assertTrue(FeatureConfig.isEnabled(TestAppContext.SLOT0, FeatureConfig.UCE));
        assertTrue(FeatureConfig.isEnabled(TestAppContext.SLOT0, FeatureConfig.VOWIFI));
        assertTrue(FeatureConfig.isEnabled(TestAppContext.SLOT0, FeatureConfig.VT));

        bundle.clear();
        FeatureConfig.init(TestAppContext.SLOT0);

        assertFalse(FeatureConfig.isEnabled(TestAppContext.SLOT0, FeatureConfig.VOLTE));
        assertFalse(FeatureConfig.isEnabled(TestAppContext.SLOT0, FeatureConfig.SMS));
        assertFalse(FeatureConfig.isEnabled(TestAppContext.SLOT0, FeatureConfig.UCE));
        assertFalse(FeatureConfig.isEnabled(TestAppContext.SLOT0, FeatureConfig.VOWIFI));
        assertFalse(FeatureConfig.isEnabled(TestAppContext.SLOT0, FeatureConfig.VT));
    }
}
