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
#ifndef INTERFACE_SIP_CONFIG_V_H_
#define INTERFACE_SIP_CONFIG_V_H_

#include "AString.h"

class IConfigurable;

class ISipConfigV
{
public:
    /**
     * @brief Returns the configurable interface from the sip (service-specific) configuration.
     *
     * @return An IConfigurable instance.
     */
    virtual IConfigurable* GetConfigurable() const = 0;

    /**
     * @brief Returns the options for feature tags to handle the caller preference / capability.
     *
     * @return Options for feature tags.\n
     *         #FEATURE_TAG_EVENT\n
     *         #FEATURE_TAG_MEDIA_BASIC\n
     *         #FEATURE_TAG_MEDIA_FRAMED\n
     *         #FEATURE_TAG_MEDIA_STREAM\n
     *         #FEATURE_TAG_MEDIA_STREAM_AUDIO\n
     *         #FEATURE_TAG_MEDIA_STREAM_VIDEO\n
     *         #FEATURE_TAG_IARI_REQUIRE\n
     *         #FEATURE_TAG_IARI_EXPLICIT\n
     *         #FEATURE_TAG_ALL\n
     *         #FEATURE_TAG_DEFAULT
     */
    virtual IMS_UINT32 GetFeatureTagOptions() const = 0;

    /**
     * @brief Returns the target number format of the SIP request message.
     *
     * @return An SIP target number format.\n
     *         #TARGET_NUMBER_FORMAT_LOCAL\n
     *         #TARGET_NUMBER_FORMAT_GLOBAL
     */
    virtual IMS_SINT32 GetTargetNumberFormat() const = 0;

    /**
     * @brief Returns the preferred uri scheme of the SIP request message.
     *
     * @return A preferred SIP target URI scheme.\n
     *         #TARGET_SCHEME_SIP\n
     *         #TARGET_SCHEME_TEL
     */
    virtual IMS_SINT32 GetTargetScheme() const = 0;

    /**
     * @brief Returns the SIP transaction timer value for a given timer type.
     *
     * @param nType The SIP timer type.\n
     *              #TIMER_T1\n
     *              #TIMER_T2\n
     *              #TIMER_T4\n
     *              #TIMER_A\n
     *              #TIMER_B\n
     *              #TIMER_C\n
     *              #TIMER_D\n
     *              #TIMER_E\n
     *              #TIMER_F\n
     *              #TIMER_G\n
     *              #TIMER_H\n
     *              #TIMER_I\n
     *              #TIMER_J\n
     *              #TIMER_K
     * @return A timer value for a given timer type.
     */
    virtual IMS_SINT32 GetTimerValue(IN IMS_SINT32 nType) const = 0;

public:
    /// Target (request-uri) URI scheme
    enum
    {
        TARGET_SCHEME_SIP = 0,
        TARGET_SCHEME_TEL
    };

    /// Target number format
    enum
    {
        TARGET_NUMBER_FORMAT_LOCAL = 0,
        TARGET_NUMBER_FORMAT_GLOBAL
    };

    /// SIP transaction timer type
    enum
    {
        TIMER_T1 = 0,
        TIMER_T2,
        TIMER_T4,
        TIMER_A,
        TIMER_B,
        TIMER_C,
        TIMER_D,
        TIMER_E,
        TIMER_F,
        TIMER_G,
        TIMER_H,
        TIMER_I,
        TIMER_J,
        TIMER_K
    };

    /// Optional media feature tags (IETF)
    /// Default value (J281-compliant) : (FEATURE_TAG_ALL & ~FEATURE_TAG_IARI_EXPLICIT)
    enum
    {
        FEATURE_TAG_NONE = 0x00000000,

        FEATURE_TAG_EVENT = 0x00000001,
        FEATURE_TAG_MEDIA_BASIC = 0x00000002,
        FEATURE_TAG_MEDIA_FRAMED = 0x00000004,
        FEATURE_TAG_MEDIA_STREAM = 0x00000008,
        FEATURE_TAG_MEDIA_STREAM_AUDIO = 0x00000100,
        FEATURE_TAG_MEDIA_STREAM_VIDEO = 0x00000200,

        /// As default, IARI sets 'require' parameter
        /// If this flag is not defined, IARI will not contain 'require' parameter
        FEATURE_TAG_IARI_REQUIRE = 0x01000000,
        /// 'explicit' parameter for IARI
        FEATURE_TAG_IARI_EXPLICIT = 0x02000000,

        FEATURE_TAG_ALL = 0xFFFFFFFF,

        FEATURE_TAG_DEFAULT = (FEATURE_TAG_ALL & (~FEATURE_TAG_IARI_EXPLICIT)),
        FEATURE_TAG_MMTEL_DEFAULT = (FEATURE_TAG_MEDIA_STREAM | FEATURE_TAG_MEDIA_STREAM_AUDIO |
                FEATURE_TAG_MEDIA_STREAM_VIDEO | FEATURE_TAG_IARI_REQUIRE |
                FEATURE_TAG_IARI_EXPLICIT)
    };
};

#endif
