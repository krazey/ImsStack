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

package com.android.imsstack.core;

import static org.junit.Assert.assertFalse;
import static org.junit.Assert.assertTrue;
import static org.mockito.ArgumentMatchers.eq;
import static org.mockito.Mockito.when;

import android.telephony.CarrierConfigManager;
import android.test.suitebuilder.annotation.SmallTest;

import com.android.imsstack.core.agents.AgentFactory;
import com.android.imsstack.core.agents.ConfigInterface;
import com.android.imsstack.core.config.CarrierConfig;

import org.junit.After;
import org.junit.Before;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.junit.runners.JUnit4;
import org.mockito.Mock;
import org.mockito.MockitoAnnotations;

@RunWith(JUnit4.class)
public class CapabilityConfigsTest {
    private static final int SLOT0 = 0;

    @Mock private ConfigInterface mConfigInterface;
    @Mock private CarrierConfig mCarrierConfig;

    @Before
    public void setUp() throws Exception {
        MockitoAnnotations.initMocks(this);

        when(mConfigInterface.getCarrierConfig()).thenReturn(mCarrierConfig);
        AgentFactory.getInstance().setAgent(ConfigInterface.class, mConfigInterface, SLOT0);
    }

    @After
    public void tearDown() throws Exception {
        mCarrierConfig = null;
        mConfigInterface = null;
        AgentFactory.getInstance().setAgent(ConfigInterface.class, null, SLOT0);
    }

    @Test
    @SmallTest
    public void testCheckCapabilityWhenConfigInterfaceNull() {
        AgentFactory.getInstance().setAgent(ConfigInterface.class, null, SLOT0);

        assertFalse(CapabilityConfigs.isVoLteEnabled(SLOT0));
        assertFalse(CapabilityConfigs.isVtEnabled(SLOT0));
        assertFalse(CapabilityConfigs.isWfcEnabled(SLOT0));
        assertFalse(CapabilityConfigs.isUceEnabled(SLOT0));
        assertFalse(CapabilityConfigs.isRttEnabled(SLOT0));
        assertFalse(CapabilityConfigs.isVoNrEnabled(SLOT0));
        assertFalse(CapabilityConfigs.isVoLteRoamingEnabled(SLOT0));
    }

    @Test
    @SmallTest
    public void testCheckCapabilityWhenCarrierConfigEnabled() {
        setUpCarrierConfigs(true);

        assertTrue(CapabilityConfigs.isVoLteEnabled(SLOT0));
        assertTrue(CapabilityConfigs.isVtEnabled(SLOT0));
        assertTrue(CapabilityConfigs.isWfcEnabled(SLOT0));
        assertTrue(CapabilityConfigs.isUceEnabled(SLOT0));
        assertTrue(CapabilityConfigs.isRttEnabled(SLOT0));
        assertTrue(CapabilityConfigs.isVoNrEnabled(SLOT0));
        assertTrue(CapabilityConfigs.isVoLteRoamingEnabled(SLOT0));

        when(mCarrierConfig.getBoolean(
                eq(CarrierConfigManager.Ims.KEY_ENABLE_PRESENCE_PUBLISH_BOOL), eq(false)))
                .thenReturn(false);

        assertTrue(CapabilityConfigs.isUceEnabled(SLOT0));
    }

    @Test
    @SmallTest
    public void testCheckCapabilityWhenCarrierConfigDisabled() {
        setUpCarrierConfigs(false);

        assertFalse(CapabilityConfigs.isVoLteEnabled(SLOT0));
        assertFalse(CapabilityConfigs.isVtEnabled(SLOT0));
        assertFalse(CapabilityConfigs.isWfcEnabled(SLOT0));
        assertFalse(CapabilityConfigs.isUceEnabled(SLOT0));
        assertFalse(CapabilityConfigs.isRttEnabled(SLOT0));
        assertFalse(CapabilityConfigs.isVoNrEnabled(SLOT0));
        assertFalse(CapabilityConfigs.isVoLteRoamingEnabled(SLOT0));

        when(mCarrierConfig.getIntArray(
                eq(CarrierConfigManager.KEY_CARRIER_NR_AVAILABILITIES_INT_ARRAY)))
                .thenReturn(null);

        assertFalse(CapabilityConfigs.isVoNrEnabled(SLOT0));
    }

    private void setUpCarrierConfigs(boolean enabled) {
        when(mCarrierConfig.getBoolean(
                eq(CarrierConfigManager.KEY_CARRIER_VOLTE_AVAILABLE_BOOL), eq(false)))
                .thenReturn(enabled);
        when(mCarrierConfig.getBoolean(
                eq(CarrierConfigManager.KEY_CARRIER_VT_AVAILABLE_BOOL), eq(false)))
                .thenReturn(enabled);
        when(mCarrierConfig.getBoolean(
                eq(CarrierConfigManager.KEY_CARRIER_WFC_IMS_AVAILABLE_BOOL), eq(false)))
                .thenReturn(enabled);
        when(mCarrierConfig.getBoolean(
                eq(CarrierConfigManager.KEY_USE_RCS_SIP_OPTIONS_BOOL), eq(false)))
                .thenReturn(enabled);
        when(mCarrierConfig.getBoolean(
                eq(CarrierConfigManager.Ims.KEY_ENABLE_PRESENCE_PUBLISH_BOOL), eq(false)))
                .thenReturn(enabled);
        when(mCarrierConfig.getBoolean(
                eq(CarrierConfigManager.KEY_RTT_SUPPORTED_BOOL), eq(false)))
                .thenReturn(enabled);
        when(mCarrierConfig.getBoolean(
                eq(CarrierConfigManager.ImsVoice.KEY_CARRIER_VOLTE_ROAMING_AVAILABLE_BOOL),
                eq(false))).thenReturn(enabled);

        int[] nrAvailabilities = enabled
                ? new int[] { CarrierConfigManager.CARRIER_NR_AVAILABILITY_SA }
                : new int[] { CarrierConfigManager.CARRIER_NR_AVAILABILITY_NSA };
        when(mCarrierConfig.getIntArray(
                eq(CarrierConfigManager.KEY_CARRIER_NR_AVAILABILITIES_INT_ARRAY)))
                .thenReturn(nrAvailabilities);
    }
}
