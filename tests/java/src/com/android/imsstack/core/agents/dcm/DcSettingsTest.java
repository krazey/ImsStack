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
import static org.mockito.ArgumentMatchers.anyBoolean;
import static org.mockito.ArgumentMatchers.anyInt;
import static org.mockito.ArgumentMatchers.eq;
import static org.mockito.Mockito.when;

import android.content.Context;
import android.telephony.AccessNetworkConstants;
import android.telephony.CarrierConfigManager;

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

import java.util.List;

@RunWith(JUnit4.class)
public class DcSettingsTest {
    private static final int SLOT_0 = 0;
    private FakeDcSettings mDcSettingsUT;

    @Mock private Context mMockContext;
    @Mock private CarrierConfig mMockCarrierConfig;
    @Mock private IDcNetWatcher mMockIDcNetWatcher;

    @Before
    public void setUp() throws Exception {
        MockitoAnnotations.initMocks(this);

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
    public void testIsVopsIgnored() throws Exception {
        when(mMockCarrierConfig.getBoolean(
                eq(CarrierConfig.ImsVoice.KEY_IGNORE_VOPS_FOR_VOLTE_ENABLE_BOOL), anyBoolean()))
                .thenReturn(false)
                .thenReturn(true);

        // configured to not ignore VoPS
        assertFalse(mDcSettingsUT.isVopsIgnored());

        // configured to ignore VoPS
        assertTrue(mDcSettingsUT.isVopsIgnored());
    }

    @Test
    public void testIsImsPdnRequestWithoutMmtelRequired() throws Exception {
        when(mMockCarrierConfig.getBoolean(
                eq(CarrierConfig.Ims.KEY_REQUEST_IMS_PDN_WITHOUT_MMTEL_BOOL), anyBoolean()))
                .thenReturn(true)
                .thenReturn(false);

        assertTrue(mDcSettingsUT.isImsPdnRequestWithoutMmtelRequired());
        assertFalse(mDcSettingsUT.isImsPdnRequestWithoutMmtelRequired());
    }

    @Test
    public void testGetImsSupportedAccessNetworks() throws Exception {
        int[] emptyList = {};
        int[] availableList = {AccessNetworkConstants.AccessNetworkType.EUTRAN};
        when(mMockCarrierConfig.getIntArray(
                eq(CarrierConfigManager.Ims.KEY_SUPPORTED_RATS_INT_ARRAY)))
                .thenReturn(emptyList)
                .thenReturn(availableList);

        List<Integer> imsSupportedAccessNetworks = mDcSettingsUT.getImsSupportedAccessNetworks();
        assertTrue(imsSupportedAccessNetworks.isEmpty());

        imsSupportedAccessNetworks = mDcSettingsUT.getImsSupportedAccessNetworks();
        assertEquals(imsSupportedAccessNetworks.size(), availableList.length);
    }

    @Test
    public void testIsCrossSimEnabledByPlatform() throws Exception {
        when(mMockCarrierConfig.getBoolean(
                eq(CarrierConfigManager.KEY_CARRIER_CROSS_SIM_IMS_AVAILABLE_BOOL), anyBoolean()))
                .thenReturn(false)
                .thenReturn(true);
        assertFalse(mDcSettingsUT.isCrossSimEnabledByPlatform());
        assertTrue(mDcSettingsUT.isCrossSimEnabledByPlatform());
    }

    @Test
    public void testIsEmergencyCallbackModeSupported() throws Exception {
        when(mMockCarrierConfig.getBoolean(
                eq(CarrierConfigManager.ImsEmergency.KEY_EMERGENCY_CALLBACK_MODE_SUPPORTED_BOOL),
                anyBoolean()))
                .thenReturn(true)
                .thenReturn(false);
        assertTrue(mDcSettingsUT.isEmergencyCallbackModeSupported());
        assertFalse(mDcSettingsUT.isEmergencyCallbackModeSupported());
    }

    @Test
    public void testGetPreferredIpVersion() throws Exception {
        when(mMockCarrierConfig.getInt(
                eq(CarrierConfig.Ims.KEY_IMS_PREFERRED_IPTYPE_INT), anyInt()))
                .thenReturn(CarrierConfig.Ims.IPV4_PREFERRED)
                .thenReturn(CarrierConfig.Ims.IPV6_PREFERRED);

        assertEquals(mDcSettingsUT.getPreferredIpVersion(), CarrierConfig.Ims.IPV4_PREFERRED);
        assertEquals(mDcSettingsUT.getPreferredIpVersion(), CarrierConfig.Ims.IPV6_PREFERRED);
    }

    @Test
    public void testGetEmergencyPreferredIpVersion() throws Exception {
        when(mMockCarrierConfig.getInt(
                eq(CarrierConfig.ImsEmergency.KEY_EPDN_PREFERRED_IPTYPE_INT), anyInt()))
                .thenReturn(CarrierConfig.Ims.IPV4_PREFERRED)
                .thenReturn(CarrierConfig.Ims.IPV6_PREFERRED);

        assertEquals(mDcSettingsUT.getEmergencyPreferredIpVersion(),
                CarrierConfig.Ims.IPV4_PREFERRED);
        assertEquals(mDcSettingsUT.getEmergencyPreferredIpVersion(),
                CarrierConfig.Ims.IPV6_PREFERRED);
    }

    @Test
    public void testIsPermanentFailure() throws Exception {
        int permanentFailureCause = 33;
        int[] emptyList = {}, availableList = {permanentFailureCause};

        when(mMockCarrierConfig.getIntArray(
                eq(CarrierConfig.Ims.KEY_PERMANENT_PDN_FAILURE_INT_ARRAY)))
                .thenReturn(emptyList)
                .thenReturn(availableList);

        assertFalse(mDcSettingsUT.isPermanentFailure(EApnType.IMS, permanentFailureCause));
        assertTrue(mDcSettingsUT.isPermanentFailure(EApnType.IMS, permanentFailureCause));

        assertFalse(mDcSettingsUT.isPermanentFailure(EApnType.INTERNET, permanentFailureCause));
        assertFalse(mDcSettingsUT.isPermanentFailure(EApnType.EMERGENCY, permanentFailureCause));
    }

    @Test
    public void testIsCrossStackRedialCause() throws Exception {
        int crossStackRedialCause = 5;
        int[] emptyList = {}, availableList = {crossStackRedialCause};

        when(mMockCarrierConfig.getIntArray(
                eq(CarrierConfig.ImsEmergency
                        .KEY_EPDN_REJECT_CAUSES_FOR_CROSS_STACK_REDIAL_INT_ARRAY)))
                .thenReturn(emptyList)
                .thenReturn(availableList);

        assertFalse(
                mDcSettingsUT.isCrossStackRedialCause(EApnType.EMERGENCY, crossStackRedialCause));
        assertTrue(
                mDcSettingsUT.isCrossStackRedialCause(EApnType.EMERGENCY, crossStackRedialCause));

        assertFalse(
                mDcSettingsUT.isCrossStackRedialCause(EApnType.INTERNET, crossStackRedialCause));
        assertFalse(mDcSettingsUT.isCrossStackRedialCause(EApnType.IMS, crossStackRedialCause));
    }

    @Test
    public void testDisableN1ModeOnImsPduReject() throws Exception {
        when(mMockCarrierConfig.getBoolean(
                eq(CarrierConfig.Ims.KEY_DISABLE_N1_MODE_ON_IMS_PDU_ESTABLISH_FAILURE_BOOL),
                        anyBoolean()))
                .thenReturn(false)
                .thenReturn(true);
        assertFalse(mDcSettingsUT.disableN1ModeOnImsPduEstablishFailure());
        assertTrue(mDcSettingsUT.disableN1ModeOnImsPduEstablishFailure());
    }

    @Test
    public void testFailToGetCarrierConfig() throws Exception {
        int permanentFailureCause = 33;

        mMockCarrierConfig = null;
        mDcSettingsUT.init(mMockContext);

        assertTrue(mDcSettingsUT.isRoamingAllowed());
        assertTrue(mDcSettingsUT.isVopsIgnored());
        assertTrue(mDcSettingsUT.getImsSupportedAccessNetworks().isEmpty());
        assertFalse(mDcSettingsUT.isCrossSimEnabledByPlatform());
        assertFalse(mDcSettingsUT.isEmergencyCallbackModeSupported());
        assertEquals(mDcSettingsUT.getPreferredIpVersion(), CarrierConfig.Ims.IPV6_PREFERRED);
        assertEquals(mDcSettingsUT.getEmergencyPreferredIpVersion(),
                CarrierConfig.Ims.IPV6_PREFERRED);
        assertFalse(mDcSettingsUT.isPermanentFailure(EApnType.IMS, permanentFailureCause));
        assertFalse(mDcSettingsUT.isCrossStackRedialCause(EApnType.EMERGENCY,
                permanentFailureCause));
        assertFalse(mDcSettingsUT.disableN1ModeOnImsPduEstablishFailure());
    }

    private class FakeDcSettings extends DcSettings {
        private FakeDcSettings(int slotId) {
            super(slotId);
        }

        @Override
        protected CarrierConfig getCarrierConfig(int slotId) {
            return mMockCarrierConfig;
        }

        @Override
        protected IDcNetWatcher getDcNetWatcher(int slotId) {
            return mMockIDcNetWatcher;
        }
    }
}
