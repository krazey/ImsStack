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

/**
 * This provides interface to check carrier configurations.
 */
public interface IDcSettings extends IDc {

    /**
     * Return whether VoLTE service is allowed in roaming state
     */
    boolean isRoamingAllowed();

    /**
     * Return whether VoPS value should be notified when the VoPS is not supported
     */
    boolean isVopsRequired();

    /**
     * Return whether VoPS value should be checked when request IMS PDN
     */
    boolean isVopsRequiredForPdn();

    /**
     * Return list of RAT technologies on which IMS is supported
     */
    int[] getImsSupportedRats();

    /**
     * Returns a platform configuration for Cross SIM
     */
    boolean isCrossSimEnabledByPlatform();

    /**
     * Return preferred IP version for connection
     */
    int getPreferredIpVersion();

    /**
     * Return preferred IP version for emergency connection
     */
    int getEmergencyPreferredIpVersion();

    /**
     * Return whether the casueCode should be handled as permanent failure
     */
    boolean isPermanentFailure(EApnType apnType, int causeCode);
}
