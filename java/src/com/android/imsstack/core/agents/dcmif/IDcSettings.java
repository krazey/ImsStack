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

package com.android.imsstack.core.agents.dcmif;

import androidx.annotation.NonNull;

import java.util.List;

/**
 * This provides interface to check carrier configurations.
 */
public interface IDcSettings extends IDc {

    /**
     * Returns whether VoLTE service is allowed in roaming state.
     */
    boolean isRoamingAllowed();

    /**
     * Returns whether to ignore the VoPS supported indication passed from network
     * and always consider it supported.
     */
    boolean isVopsIgnored();

    /**
     * Returns whether IMS PDN request without MMTEL capability is required.
     */
    boolean isImsPdnRequestWithoutMmtelRequired();

    /**
     * Returns list of access network on which IMS is supported.
     */
    @NonNull
    List<Integer> getImsSupportedAccessNetworks();

    /**
     * Returns a platform configuration for Cross SIM.
     */
    boolean isCrossSimEnabledByPlatform();

    /**
     * Returns whether emergency callback mode is supported.
     */
    boolean isEmergencyCallbackModeSupported();

    /**
     * Returns preferred IP version for connection.
     */
    int getPreferredIpVersion();

    /**
     * Returns preferred IP version for emergency connection.
     */
    int getEmergencyPreferredIpVersion();

    /**
     * Returns whether the casueCode should be handled as permanent failure.
     */
    boolean isPermanentFailure(EApnType apnType, int causeCode);

    /**
     * Returns whether the causeCode requires a cross stack redial.
     */
    boolean isCrossStackRedialCause(EApnType apnType, int causeCode);

    /**
     * Returns whether N1 mode capability disabling is required when failing to establish IMS PDU.
     */
    boolean disableN1ModeOnImsPduEstablishFailure();
}
