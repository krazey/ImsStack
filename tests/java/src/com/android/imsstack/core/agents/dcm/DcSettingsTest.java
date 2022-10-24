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
package com.android.imsstack.core.agents.dcm;

import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertFalse;
import static org.junit.Assert.assertTrue;
import static org.mockito.Matchers.anyBoolean;
import static org.mockito.Matchers.anyInt;
import static org.mockito.Matchers.eq;
import static org.mockito.Mockito.when;

import android.content.Context;
import android.telephony.AccessNetworkConstants;
import android.telephony.CarrierConfigManager;

import com.android.imsstack.core.agents.ConfigInterface;
import com.android.imsstack.core.agents.dcmif.EApnType;
import com.android.imsstack.core.agents.dcmif.IDcNetWatcher;
import com.android.imsstack.core.config.CarrierConfig;

import org.junit.After;
import org.junit.Before;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.junit.runners.JUnit4;
import org.mockito.Mock;
import org.mockito.MockitoAnnotations;

@RunWith(JUnit4.class)
public class DcSettingsTest {
    private static final int SLOT_0 = 0;
    private FakeDcSettings mDcSettingsUT;

    @Mock private Context mMockContext;
    @Mock private CarrierConfig mMockCarrierConfig;
    @Mock private ConfigInterface mMockConfigInterface;
    @Mock private IDcNetWatcher mMockIDcNetWatcher;

    @Before
    public void setUp() throws Exception {
        MockitoAnnotations.initMocks(this);

        when(mMockConfigInterface.getCarrierConfig()).thenReturn(mMockCarrierConfig);

        mDcSettingsUT = new FakeDcSettings(SLOT_0);
        mDcSettingsUT.init(mMockContext);
    }

    @After
    public void tearDown() throws Exception {
        mDcSettingsUT.cleanup();
        mDcSettingsUT = null;
    }

    @Test
    public void testIsRoamingAllowed() throws Exception {
        when(mMockCarrierConfig.getBoolean(
                eq(CarrierConfigManager.ImsVoice.KEY_CARRIER_VOLTE_ROAMING_AVAILABLE_BOOL),
                anyBoolean()))
                .thenReturn(true)
                .thenReturn(false);

        assertTrue(mDcSettingsUT.isRoamingAllowed());
        assertFalse(mDcSettingsUT.isRoamingAllowed());
    }

    @Test
    public void testIsVopsRequired() throws Exception {
        int[] supportWithoutVopsAtHome = {CarrierConfigManager.Ims.NETWORK_TYPE_HOME};

        when(mMockCarrierConfig.getBoolean(
                eq(CarrierConfig.Assets.KEY_IGNORE_VOPS_FOR_VOLTE_ENABLE_BOOL), anyBoolean()))
                .thenReturn(false)
                .thenReturn(false)
                .thenReturn(true)
                .thenReturn(true);
        when(mMockIDcNetWatcher.isRoaming())
                .thenReturn(true)
                .thenReturn(false);
        when(mMockCarrierConfig.getIntArray(
                eq(CarrierConfigManager.Ims.KEY_IMS_PDN_ENABLED_IN_NO_VOPS_SUPPORT_INT_ARRAY)))
                .thenReturn(supportWithoutVopsAtHome)
                .thenReturn(supportWithoutVopsAtHome);

        // configured to not ignore VoPS and VoPS is not required for PDN
        assertTrue(mDcSettingsUT.isVopsRequired());

        // configured to not ignore VoPS and VoPS is required for PDN
        assertTrue(mDcSettingsUT.isVopsRequired());

        // configured to ignore VoPS and VoPS is required for PDN
        assertTrue(mDcSettingsUT.isVopsRequired());

        // configured to ignore VoPS and VoPS is not required for PDN
        assertFalse(mDcSettingsUT.isVopsRequired());
    }

    @Test
    public void testIsVopsRequiredForPdn() throws Exception {
        int[] empty = {};
        int[] all = {CarrierConfigManager.Ims.NETWORK_TYPE_HOME,
                CarrierConfigManager.Ims.NETWORK_TYPE_ROAMING};

        when(mMockIDcNetWatcher.isRoaming())
                .thenReturn(false)
                .thenReturn(false)
                .thenReturn(true)
                .thenReturn(true);
        when(mMockCarrierConfig.getIntArray(
                eq(CarrierConfigManager.Ims.KEY_IMS_PDN_ENABLED_IN_NO_VOPS_SUPPORT_INT_ARRAY)))
                .thenReturn(empty)
                .thenReturn(all)
                .thenReturn(empty)
                .thenReturn(all);

        // Home Network and configured to not support without VoPS
        assertTrue(mDcSettingsUT.isVopsRequiredForPdn());

        // Home Network and configured to support without VoPS
        assertFalse(mDcSettingsUT.isVopsRequiredForPdn());

        // Roaming Network and configured to not support without VoPS
        assertTrue(mDcSettingsUT.isVopsRequiredForPdn());

        // Roaming Network and configured to support without VoPS
        assertFalse(mDcSettingsUT.isVopsRequiredForPdn());
    }

    @Test
    public void testGetImsSupportedRats() throws Exception {
        int[] emptyList = {}, availableList = {AccessNetworkConstants.AccessNetworkType.EUTRAN};

        when(mMockCarrierConfig.getIntArray(
                eq(CarrierConfigManager.Ims.KEY_SUPPORTED_RATS_INT_ARRAY)))
                .thenReturn(emptyList)
                .thenReturn(availableList);

        int[] unAvailableRats = mDcSettingsUT.getImsSupportedRats();
        assertEquals(unAvailableRats.length, emptyList.length);

        int[] availableRats = mDcSettingsUT.getImsSupportedRats();
        assertEquals(availableRats.length, availableList.length);
    }

    @Test
    public void testGetPreferredIpVersion() throws Exception {
        when(mMockCarrierConfig.getInt(
                eq(CarrierConfig.Assets.KEY_IMS_PREFERRED_IPTYPE_INT), anyInt()))
                .thenReturn(CarrierConfig.Assets.IPV4_PREFERRED)
                .thenReturn(CarrierConfig.Assets.IPV6_PREFERRED);

        assertEquals(mDcSettingsUT.getPreferredIpVersion(), CarrierConfig.Assets.IPV4_PREFERRED);
        assertEquals(mDcSettingsUT.getPreferredIpVersion(), CarrierConfig.Assets.IPV6_PREFERRED);
    }

    @Test
    public void testGetEmergencyPreferredIpVersion() throws Exception {
        when(mMockCarrierConfig.getInt(
                eq(CarrierConfig.Assets.KEY_EMC_PREFERRED_IPTYPE_INT), anyInt()))
                .thenReturn(CarrierConfig.Assets.IPV4_PREFERRED)
                .thenReturn(CarrierConfig.Assets.IPV6_PREFERRED);

        assertEquals(mDcSettingsUT.getEmergencyPreferredIpVersion(),
                CarrierConfig.Assets.IPV4_PREFERRED);
        assertEquals(mDcSettingsUT.getEmergencyPreferredIpVersion(),
                CarrierConfig.Assets.IPV6_PREFERRED);
    }

    @Test
    public void testIsPermanentFailure() throws Exception {
        int permanentFailureCause = 33;
        int[] emptyList = {}, availableList = {permanentFailureCause};

        when(mMockCarrierConfig.getIntArray(
                eq(CarrierConfig.Assets.KEY_PERMANENT_PDN_FAILURE_INT_ARRAY)))
                .thenReturn(emptyList)
                .thenReturn(availableList);

        assertFalse(mDcSettingsUT.isPermanentFailure(EApnType.IMS, permanentFailureCause));
        assertTrue(mDcSettingsUT.isPermanentFailure(EApnType.IMS, permanentFailureCause));

        assertFalse(mDcSettingsUT.isPermanentFailure(EApnType.INTERNET, permanentFailureCause));
        assertFalse(mDcSettingsUT.isPermanentFailure(EApnType.EMERGENCY, permanentFailureCause));
    }

    @Test
    public void testFailToGetCarrierConfig() throws Exception {
        int permanentFailureCause = 33;

        when(mMockConfigInterface.getCarrierConfig()).thenReturn(null);
        mDcSettingsUT.init(mMockContext);

        assertTrue(mDcSettingsUT.isRoamingAllowed());
        assertTrue(mDcSettingsUT.isVopsRequired());
        assertTrue(mDcSettingsUT.isVopsRequiredForPdn());
        int[] availableRats = mDcSettingsUT.getImsSupportedRats();
        assertEquals(availableRats.length, 0);
        assertEquals(mDcSettingsUT.getPreferredIpVersion(), CarrierConfig.Assets.IPV6_PREFERRED);
        assertEquals(mDcSettingsUT.getEmergencyPreferredIpVersion(),
                CarrierConfig.Assets.IPV6_PREFERRED);
        assertFalse(mDcSettingsUT.isPermanentFailure(EApnType.IMS, permanentFailureCause));
    }

    private class FakeDcSettings extends DcSettings {
        private FakeDcSettings(int slotId) {
            super(slotId);
        }

        @Override
        protected ConfigInterface getConfigInterface(int slotId) {
            return mMockConfigInterface;
        }

        @Override
        protected IDcNetWatcher getDcNetWatcher(int slotId) {
            return mMockIDcNetWatcher;
        }
    }
}
