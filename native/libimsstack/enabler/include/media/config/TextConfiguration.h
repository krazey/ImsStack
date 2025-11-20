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

#ifndef TEXT_CONFIGURATION_H_
#define TEXT_CONFIGURATION_H_

#include "config/MediaConfiguration.h"

/**
 * @class TextConfiguration
 * @brief Manages Real-Time Text (RTT) specific media configurations.
 * @details This class holds all configuration parameters related to RTT streams,
 * such as payload types for T.140 and RED, DSCP values, and other codec settings.
 */
class TextConfiguration : public MediaConfiguration
{
public:
    /**
     * @brief Constructs a new TextConfiguration object.
     *
     * @param eSessionType The type of media session, typically MEDIA_TYPE_TEXT.
     */
    explicit TextConfiguration(MEDIA_CONTENT_TYPE eSessionType = MEDIA_TYPE_TEXT);

    /**
     * @brief Destroys the TextConfiguration object.
     */
    ~TextConfiguration() override;

    /**
     * @brief Initializes the text configuration by reading carrier-specific settings.
     *
     * @param piCc A pointer to the carrier configuration interface.
     * @return IMS_TRUE on success, IMS_FALSE on failure.
     */
    virtual IMS_BOOL Create(IN ICarrierConfig* piCc) override;

    /**
     * @brief Updates the text configuration with new carrier-specific settings.
     *
     * @param piCc A pointer to the carrier configuration interface.
     * @return IMS_TRUE on success, IMS_FALSE on failure.
     */
    virtual IMS_BOOL Update(IN ICarrierConfig* piCc) override;

    /**
     * @brief Gets the payload type number for the T.140 text codec.
     *
     * @return The T.140 payload type number.
     */
    virtual IMS_SINT32 GetT140PayloadType() const;

    /**
     * @brief Gets the payload type number for the redundant (RED) text codec.
     *
     * @return The RED payload type number.
     */
    virtual IMS_SINT32 GetRedPayloadType() const;

    /**
     * @brief Gets the Differentiated Services Code Point (DSCP) value for text RTP packets.
     *
     * @return The DSCP value.
     */
    virtual IMS_SINT32 GetTextDscp() const;

    /**
     * @brief Checks if sending empty redundant packets for the text codec is enabled.
     *
     * @return IMS_TRUE if enabled, otherwise IMS_FALSE.
     */
    virtual IMS_BOOL IsTextCodecEmptyRedundantEnabled() const;

    static const IMS_SINT32 NEED_TO_CHECK_I = 0;
    static const IMS_SINT32 DEFAULT_PAYLOAD_T140 = NEED_TO_CHECK_I;
    static const IMS_SINT32 DEFAULT_PAYLOAD_RED = NEED_TO_CHECK_I;
    static const IMS_SINT32 DEFAULT_TEXT_DSCP = 46;
    static const IMS_BOOL DEFAULT_EMPTY_REDUNDANT = IMS_TRUE;

protected:
    IMS_BOOL CreateCodecConfigs(IN ICarrierConfig* piCc) override;
    void ToDebugString() const override;

private:
    IMS_SINT32 m_nT140PayloadType;
    IMS_SINT32 m_nRedPayloadType;
    IMS_SINT32 m_nTextDscp;
    IMS_BOOL m_bTextCodecEmptyRedundantEnabled;
};

#endif
