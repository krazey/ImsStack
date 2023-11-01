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
import static org.mockito.ArgumentMatchers.eq;
import static org.mockito.Mockito.when;

import android.content.Context;
import android.os.PersistableBundle;
import android.telephony.CarrierConfigManager;
import android.telephony.SubscriptionManager;

import androidx.test.filters.SmallTest;

import com.android.imsstack.ContextFixture;
import com.android.imsstack.base.AppContext;
import com.android.imsstack.base.MSimUtils;

import org.junit.After;
import org.junit.Before;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.junit.runners.JUnit4;

@RunWith(JUnit4.class)
public class FeatureConfigTest {
    private static final int SLOT0 = 0;
    private static final int[] SUB_ID = { 1 };

    private ContextFixture mContextFixture;

    @Before
    public void setUp() throws Exception {
        mContextFixture = new ContextFixture();
        AppContext.init(mContextFixture.getTestDouble());
    }

    @After
    public void tearDown() throws Exception {
        mContextFixture = null;
        AppContext.deinit();
    }

    @Test
    @SmallTest
    public void testInitialValue() {
        assertFalse(FeatureConfig.isEnabled(SLOT0, ""));
        assertFalse(FeatureConfig.isEnabled(SLOT0, FeatureConfig.VOLTE));
        assertFalse(FeatureConfig.isEnabled(SLOT0, FeatureConfig.SMS));
        assertFalse(FeatureConfig.isEnabled(SLOT0, FeatureConfig.UCE));
        assertFalse(FeatureConfig.isEnabled(SLOT0, FeatureConfig.VOWIFI));
        assertFalse(FeatureConfig.isEnabled(SLOT0, FeatureConfig.VT));

        // When feature does not exist.
        assertFalse(FeatureConfig.isEnabled(MSimUtils.INVALID_SLOT_ID, FeatureConfig.VOLTE));
    }

    @Test
    @SmallTest
    public void testInit() {
        Context context = mContextFixture.getTestDouble();
        SubscriptionManager sm = context.getSystemService(SubscriptionManager.class);
        when(sm.getSubscriptionIds(eq(SLOT0))).thenReturn(SUB_ID);

        PersistableBundle bundle = new PersistableBundle();
        bundle.putBoolean(CarrierConfigManager.KEY_CARRIER_VOLTE_AVAILABLE_BOOL, true);
        bundle.putBoolean(CarrierConfigManager.KEY_CARRIER_VT_AVAILABLE_BOOL, true);
        bundle.putBoolean(CarrierConfigManager.ImsSms.KEY_SMS_OVER_IMS_SUPPORTED_BOOL, true);
        bundle.putBoolean(CarrierConfigManager.KEY_CARRIER_WFC_IMS_AVAILABLE_BOOL, true);
        bundle.putBoolean(CarrierConfigManager.Ims.KEY_ENABLE_PRESENCE_PUBLISH_BOOL, true);
        bundle.putBoolean(CarrierConfigManager.KEY_USE_RCS_SIP_OPTIONS_BOOL, true);
        CarrierConfigManager ccm = context.getSystemService(CarrierConfigManager.class);
        when(ccm.getConfigForSubId(SUB_ID[0])).thenReturn(bundle);

        FeatureConfig.init(SLOT0);
        assertTrue(FeatureConfig.isEnabled(SLOT0, FeatureConfig.VOLTE));
        assertTrue(FeatureConfig.isEnabled(SLOT0, FeatureConfig.SMS));
        assertTrue(FeatureConfig.isEnabled(SLOT0, FeatureConfig.UCE));
        assertTrue(FeatureConfig.isEnabled(SLOT0, FeatureConfig.VOWIFI));
        assertTrue(FeatureConfig.isEnabled(SLOT0, FeatureConfig.VT));

        bundle.clear();
        FeatureConfig.init(SLOT0);

        assertFalse(FeatureConfig.isEnabled(SLOT0, FeatureConfig.VOLTE));
        assertFalse(FeatureConfig.isEnabled(SLOT0, FeatureConfig.SMS));
        assertFalse(FeatureConfig.isEnabled(SLOT0, FeatureConfig.UCE));
        assertFalse(FeatureConfig.isEnabled(SLOT0, FeatureConfig.VOWIFI));
        assertFalse(FeatureConfig.isEnabled(SLOT0, FeatureConfig.VT));

        mContextFixture.setSystemService(Context.CARRIER_CONFIG_SERVICE, null);
        FeatureConfig.init(SLOT0);

        assertFalse(FeatureConfig.isEnabled(SLOT0, FeatureConfig.VOLTE));
        assertFalse(FeatureConfig.isEnabled(SLOT0, FeatureConfig.SMS));
        assertFalse(FeatureConfig.isEnabled(SLOT0, FeatureConfig.UCE));
        assertFalse(FeatureConfig.isEnabled(SLOT0, FeatureConfig.VOWIFI));
        assertFalse(FeatureConfig.isEnabled(SLOT0, FeatureConfig.VT));
    }
}
