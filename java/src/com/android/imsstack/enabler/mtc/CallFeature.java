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

import android.telephony.CarrierConfigManager;

import com.android.imsstack.core.agents.AgentFactory;
import com.android.imsstack.core.agents.ConfigInterface;
import com.android.imsstack.core.config.CarrierConfig;

import java.util.Arrays;

public final class CallFeature {

    private CallFeature() {
        // no-op
    }

    public static boolean isAudioEvsSupported(int slotId) {
        int[] evsCodecs = getConfigInterface(slotId).getCarrierConfig()
                .getIntArray(CarrierConfigManager.ImsVoice.KEY_EVS_PAYLOAD_TYPE_INT_ARRAY);

        if (evsCodecs != null) {
            return (evsCodecs.length > 0) ? true : false;
        }

        return false;
    }

    /**
     * Checks if the carrier supports HEVC(H.264) video codec or not.
     *
     * @param slotId The slot-id to be checked.
     * @return true if the carrier supports HEVC, false otherwise.
     */
    public static boolean isVideoHevcSupported(int slotId) {
        return false;
    }

    public static boolean isCallHoldUsingInactive(int slotId) {
        return getConfigInterface(slotId).getCarrierConfig()
                .getBoolean(CarrierConfig.Assets.KEY_AUDIO_HOLD_WITH_DIRECTION_INACTIVE_BOOL);
    }

    public static boolean isIncomingResumeEventSupported(int slotId) {
        return getConfigInterface(slotId).getCarrierConfig()
                .getBoolean(CarrierConfig.ImsVoice.KEY_INCOMING_RESUME_EVENT_SUPPORT_BOOL);
    }

    public static boolean isSrvccSupported(int slotId) {
        int[] srvccType = getConfigInterface(slotId).getCarrierConfig()
                .getIntArray(CarrierConfigManager.ImsVoice.KEY_SRVCC_TYPE_INT_ARRAY);
        if (srvccType != null) {
            return Arrays.stream(srvccType).anyMatch(i -> i
                    == CarrierConfigManager.ImsVoice.BASIC_SRVCC_SUPPORT);
        } else {
            return false;
        }
    }

    public static boolean isTtySupported(int slotId) {
        return getConfigInterface(slotId).getCarrierConfig()
                .getBoolean(CarrierConfigManager.KEY_CARRIER_VOLTE_TTY_SUPPORTED_BOOL, false);
    }

    public static boolean isRttSupported(int slotId) {
        return getConfigInterface(slotId).getCarrierConfig()
                .getBoolean(CarrierConfigManager.KEY_RTT_SUPPORTED_BOOL, false);
    }

    public static boolean isVideoDirectionInactiveOnVideoCallHold(int slotId) {
        return getConfigInterface(slotId).getCarrierConfig()
                .getBoolean(CarrierConfig.Assets.KEY_VIDEO_HOLD_WITH_DIRECTION_INACTIVE_BOOL);
    }

    public static boolean isTextDirectionInactiveOnRttCallHold(int slotId) {
        return getConfigInterface(slotId).getCarrierConfig()
                .getBoolean(CarrierConfig.Assets.KEY_TEXT_HOLD_WITH_DIRECTION_INACTIVE_BOOL);
    }

    public static boolean isDynamicVideoQualitySupported(int slotId) {
        return getConfigInterface(slotId).getCarrierConfig()
                .getBoolean(CarrierConfig.ImsVt.KEY_DYNAMIC_VIDEO_QUALITY_SUPPORTED_BOOL);
    }

    public static boolean isOneWayVideoCallByLocalEndSupported(int slotId) {
        return getConfigInterface(slotId).getCarrierConfig()
                .getBoolean(CarrierConfig.ImsVt.KEY_ONE_WAY_VIDEO_CALL_BY_LOCAL_END_SUPPORTED_BOOL);
    }

    public static boolean isOneWayVideoCallByRemoteEndSupported(int slotId) {
        return getConfigInterface(slotId).getCarrierConfig()
                .getBoolean(
                    CarrierConfig.ImsVt.KEY_ONE_WAY_VIDEO_CALL_BY_REMOTE_END_SUPPORTED_BOOL);
    }

    public static boolean isNotifyConfStateWhenAnonymousUser(int slotId) {
        return getConfigInterface(slotId).getCarrierConfig()
                .getBoolean(CarrierConfig.ImsVoice.KEY_NOTIFY_CONF_STATE_WHEN_ANONYMOUS_USER_BOOL);
    }

    public static boolean isCallMergeableOnConferenceOnHold(int slotId) {
        return getConfigInterface(slotId).getCarrierConfig()
                .getBoolean(CarrierConfig.ImsVoice.KEY_CALL_MERGEABLE_ON_CONFERENCE_ON_HOLD_BOOL);
    }

    /**
     * Checks if the carrier supports WiFi emergency call over emergency PDN or not.
     *
     * @param slotId The slot-id to be checked.
     * @return true if the carrier supports emergency PDN, false otherwise.
     */
    public static boolean isWiFiEmcOverEmergencyPdn(int slotId) {
        return getConfigInterface(slotId).getCarrierConfig()
                .getBoolean(
                    CarrierConfigManager.ImsWfc.KEY_EMERGENCY_CALL_OVER_EMERGENCY_PDN_BOOL);
    }

    /**
     * Returns the configuration interface.
     *
     * @param slotId The slot-id to be retrieved.
     * @return A ConfigInterface instance.
     */
    private static ConfigInterface getConfigInterface(int slotId) {
        return AgentFactory.getInstance().getAgent(ConfigInterface.class, slotId);
    }
}
