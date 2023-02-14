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
import static org.mockito.ArgumentMatchers.anyInt;
import static org.mockito.Mockito.doReturn;
import static org.mockito.Mockito.when;

import android.content.Context;
import android.os.PersistableBundle;
import android.telephony.CarrierConfigManager;
import android.telephony.SubscriptionManager;
import android.test.suitebuilder.annotation.SmallTest;

import com.android.imsstack.ContextFixture;
import com.android.imsstack.core.agents.AgentFactory;
import com.android.imsstack.core.agents.ConfigAgent;
import com.android.imsstack.core.agents.ConfigInterface;
import com.android.imsstack.util.AppContext;

import org.junit.After;
import org.junit.Before;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.junit.runners.JUnit4;
import org.mockito.Mockito;

@RunWith(JUnit4.class)
public class FeatureConfigTest {
    private static final int SLOT_ID = 0;
    private static final int[] SUB_ID = { 0 };
    private Context mContext;
    @Before
    public void setUp() throws Exception {
        mContext = new ContextFixture().getTestDouble();
        SubscriptionManager mockSubscriptionManager = mContext.getSystemService(
                SubscriptionManager.class);
        when(mockSubscriptionManager.getSubscriptionIds(anyInt()))
                .thenReturn(SUB_ID);
        AppContext.init(mContext);
    }

    @After
    public void tearDown() throws Exception {
        mContext = null;
        AppContext.deinit();
    }

    @Test
    @SmallTest
    public void isEnableTest() {
        assertFalse(FeatureConfig.isEnabled(SLOT_ID, null));
        assertFalse(FeatureConfig.isEnabled(SLOT_ID, ""));

        assertFalse(FeatureConfig.isEnabled(SLOT_ID, FeatureConfig.VOLTE));
        assertFalse(FeatureConfig.isEnabled(SLOT_ID, FeatureConfig.SMS));
        assertFalse(FeatureConfig.isEnabled(SLOT_ID, FeatureConfig.UCE));
        assertFalse(FeatureConfig.isEnabled(SLOT_ID, FeatureConfig.VOWIFI));
        assertFalse(FeatureConfig.isEnabled(SLOT_ID, FeatureConfig.VT));
    }

    @Test
    @SmallTest
    public void initTest() {
        ConfigAgent mockConfigAgent = Mockito.mock(ConfigAgent.class);
        CarrierConfig mockCarrierConfig = Mockito.mock(CarrierConfig.class);
        CarrierConfigManager mockCarrierConfigManager = mContext.getSystemService(
                CarrierConfigManager.class);
        PersistableBundle bundle = new PersistableBundle();
        bundle.putBoolean(CarrierConfigManager.KEY_CARRIER_VOLTE_AVAILABLE_BOOL, true);
        bundle.putBoolean(CarrierConfigManager.KEY_CARRIER_VT_AVAILABLE_BOOL, true);
        bundle.putBoolean(CarrierConfigManager.ImsSms.KEY_SMS_OVER_IMS_SUPPORTED_BOOL, true);
        bundle.putBoolean(CarrierConfigManager.KEY_CARRIER_WFC_IMS_AVAILABLE_BOOL, true);
        bundle.putBoolean(CarrierConfigManager.Ims.KEY_ENABLE_PRESENCE_PUBLISH_BOOL, true);
        bundle.putBoolean(CarrierConfigManager.KEY_USE_RCS_SIP_OPTIONS_BOOL, true);

        doReturn(bundle).when(mockCarrierConfigManager).getConfigForSubId(SUB_ID[0]);
        when(mockCarrierConfig.getConfig()).thenReturn(bundle);
        when(mockConfigAgent.getCarrierConfig()).thenReturn(mockCarrierConfig);
        AgentFactory.getInstance().setAgent(ConfigInterface.class, mockConfigAgent, SLOT_ID);
        FeatureConfig.init(SLOT_ID);
        assertTrue(FeatureConfig.isEnabled(SLOT_ID, FeatureConfig.VOLTE));
        assertTrue(FeatureConfig.isEnabled(SLOT_ID, FeatureConfig.SMS));
        assertTrue(FeatureConfig.isEnabled(SLOT_ID, FeatureConfig.UCE));
        assertTrue(FeatureConfig.isEnabled(SLOT_ID, FeatureConfig.VOWIFI));
        assertTrue(FeatureConfig.isEnabled(SLOT_ID, FeatureConfig.VT));
        AgentFactory.getInstance().setAgent(ConfigInterface.class, null, SLOT_ID);
    }

    @Test
    @SmallTest
    public void initUnavailableTest() {
        FeatureConfig.init(SLOT_ID);
        assertFalse(FeatureConfig.isEnabled(SLOT_ID, FeatureConfig.VOLTE));
        assertFalse(FeatureConfig.isEnabled(SLOT_ID, FeatureConfig.SMS));
        assertFalse(FeatureConfig.isEnabled(SLOT_ID, FeatureConfig.UCE));
        assertFalse(FeatureConfig.isEnabled(SLOT_ID, FeatureConfig.VOWIFI));
        assertFalse(FeatureConfig.isEnabled(SLOT_ID, FeatureConfig.VT));
    }
}
