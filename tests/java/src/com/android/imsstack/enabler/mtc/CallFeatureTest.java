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

package com.android.imsstack.enabler.mtc;

import static org.junit.Assert.assertFalse;
import static org.junit.Assert.assertTrue;
import static org.mockito.Mockito.when;

import android.telephony.CarrierConfigManager;

import com.android.imsstack.core.agents.AgentFactory;
import com.android.imsstack.core.agents.ConfigInterface;
import com.android.imsstack.core.config.CarrierConfig;

import org.junit.After;
import org.junit.Before;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.junit.runners.JUnit4;
import org.mockito.Mockito;

@RunWith(JUnit4.class)
public class CallFeatureTest {
    private static final int SLOT_ID = 0;
    private CarrierConfig mMockCarrierConfig;
    private ConfigInterface mMockConfigInterface;

    @Before
    public void setUp() throws Exception {
        mMockCarrierConfig = Mockito.mock(CarrierConfig.class);
        mMockConfigInterface = Mockito.mock(ConfigInterface.class);
        when(mMockConfigInterface.getCarrierConfig()).thenReturn(mMockCarrierConfig);
        AgentFactory.getInstance().setAgent(ConfigInterface.class, mMockConfigInterface, SLOT_ID);
    }

    @After
    public void tearDown() throws Exception {
        AgentFactory.getInstance().setAgent(ConfigInterface.class, null, SLOT_ID);
    }

    @Test
    public void testCallFeature() {
        int[] intArray = {0, 1};
        int[] emptyArray = new int[0];

        when(mMockCarrierConfig.getIntArray(
                CarrierConfigManager.ImsVoice.KEY_EVS_PAYLOAD_TYPE_INT_ARRAY))
                .thenReturn(intArray)
                .thenReturn(emptyArray);
        assertTrue(CallFeature.isAudioEvsSupported(SLOT_ID));
        assertFalse(CallFeature.isAudioEvsSupported(SLOT_ID));

        assertFalse(CallFeature.isVideoHevcSupported(SLOT_ID));

        when(mMockCarrierConfig.getBoolean(
                CarrierConfig.Assets.KEY_AUDIO_HOLD_WITH_DIRECTION_INACTIVE_BOOL))
                .thenReturn(true)
                .thenReturn(false);
        assertTrue(CallFeature.isCallHoldUsingInactive(SLOT_ID));
        assertFalse(CallFeature.isCallHoldUsingInactive(SLOT_ID));

        when(mMockCarrierConfig.getBoolean(
                CarrierConfig.ImsVoice.KEY_INCOMING_RESUME_EVENT_SUPPORT_BOOL))
                .thenReturn(true)
                .thenReturn(false);
        assertTrue(CallFeature.isIncomingResumeEventSupported(SLOT_ID));
        assertFalse(CallFeature.isIncomingResumeEventSupported(SLOT_ID));

        when(mMockCarrierConfig.getIntArray(
                CarrierConfigManager.ImsVoice.KEY_SRVCC_TYPE_INT_ARRAY))
                .thenReturn(intArray)
                .thenReturn(emptyArray);
        assertTrue(CallFeature.isSrvccSupported(SLOT_ID));
        assertFalse(CallFeature.isSrvccSupported(SLOT_ID));

        when(mMockCarrierConfig.getBoolean(
                CarrierConfigManager.KEY_CARRIER_VOLTE_TTY_SUPPORTED_BOOL, false))
                .thenReturn(true)
                .thenReturn(false);
        assertTrue(CallFeature.isTtySupported(SLOT_ID));
        assertFalse(CallFeature.isTtySupported(SLOT_ID));

        when(mMockCarrierConfig.getBoolean(
                CarrierConfigManager.KEY_RTT_SUPPORTED_BOOL, false))
                .thenReturn(true)
                .thenReturn(false);
        assertTrue(CallFeature.isRttSupported(SLOT_ID));
        assertFalse(CallFeature.isRttSupported(SLOT_ID));

        when(mMockCarrierConfig.getBoolean(
                CarrierConfig.Assets.KEY_VIDEO_HOLD_WITH_DIRECTION_INACTIVE_BOOL))
                .thenReturn(true)
                .thenReturn(false);
        assertTrue(CallFeature.isVideoDirectionInactiveOnVideoCallHold(SLOT_ID));
        assertFalse(CallFeature.isVideoDirectionInactiveOnVideoCallHold(SLOT_ID));

        when(mMockCarrierConfig.getBoolean(
                CarrierConfig.Assets.KEY_TEXT_HOLD_WITH_DIRECTION_INACTIVE_BOOL))
                .thenReturn(true)
                .thenReturn(false);
        assertTrue(CallFeature.isTextDirectionInactiveOnRttCallHold(SLOT_ID));
        assertFalse(CallFeature.isTextDirectionInactiveOnRttCallHold(SLOT_ID));

        when(mMockCarrierConfig.getBoolean(
                CarrierConfig.ImsVt.KEY_DYNAMIC_VIDEO_QUALITY_SUPPORTED_BOOL))
                .thenReturn(true)
                .thenReturn(false);
        assertTrue(CallFeature.isDynamicVideoQualitySupported(SLOT_ID));
        assertFalse(CallFeature.isDynamicVideoQualitySupported(SLOT_ID));

        when(mMockCarrierConfig.getBoolean(
                CarrierConfig.ImsVt.KEY_ONE_WAY_VIDEO_CALL_BY_LOCAL_END_SUPPORTED_BOOL))
                .thenReturn(true)
                .thenReturn(false);
        assertTrue(CallFeature.isOneWayVideoCallByLocalEndSupported(SLOT_ID));
        assertFalse(CallFeature.isOneWayVideoCallByLocalEndSupported(SLOT_ID));

        when(mMockCarrierConfig.getBoolean(
                CarrierConfig.ImsVt.KEY_ONE_WAY_VIDEO_CALL_BY_REMOTE_END_SUPPORTED_BOOL))
                .thenReturn(true)
                .thenReturn(false);
        assertTrue(CallFeature.isOneWayVideoCallByRemoteEndSupported(SLOT_ID));
        assertFalse(CallFeature.isOneWayVideoCallByRemoteEndSupported(SLOT_ID));

        when(mMockCarrierConfig.getBoolean(
                CarrierConfig.ImsVoice.KEY_NOTIFY_CONF_STATE_WHEN_ANONYMOUS_USER_BOOL))
                .thenReturn(true)
                .thenReturn(false);
        assertTrue(CallFeature.isNotifyConfStateWhenAnonymousUser(SLOT_ID));
        assertFalse(CallFeature.isNotifyConfStateWhenAnonymousUser(SLOT_ID));

        when(mMockCarrierConfig.getBoolean(
                CarrierConfig.ImsVoice.KEY_CALL_MERGEABLE_ON_CONFERENCE_ON_HOLD_BOOL))
                .thenReturn(true)
                .thenReturn(false);
        assertTrue(CallFeature.isCallMergeableOnConferenceOnHold(SLOT_ID));
        assertFalse(CallFeature.isCallMergeableOnConferenceOnHold(SLOT_ID));

        when(mMockCarrierConfig.getBoolean(
                CarrierConfigManager.ImsWfc.KEY_EMERGENCY_CALL_OVER_EMERGENCY_PDN_BOOL))
                .thenReturn(true)
                .thenReturn(false);
        assertTrue(CallFeature.isWiFiEmcOverEmergencyPdn(SLOT_ID));
        assertFalse(CallFeature.isWiFiEmcOverEmergencyPdn(SLOT_ID));
    }
}
