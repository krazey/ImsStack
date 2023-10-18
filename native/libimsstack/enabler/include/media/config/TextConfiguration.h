/**
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

class TextConfiguration : public MediaConfiguration
{
public:
    /**
     * @brief Construct a new text configuration
     *
     * @param eSessionType mediasession type (as: text)
     */
    explicit TextConfiguration(MEDIA_CONTENT_TYPE eSessionType = MEDIA_TYPE_TEXT);
    /**
     * @brief Destroy the text configuration
     *
     */
    virtual ~TextConfiguration();
    /**
     * @brief Create codec using the configuration
     *
     * @param piCc configuration
     * @return IMS_BOOL Return true if the create function is executed without error
     * Return false if the create function is failed
     */
    IMS_BOOL Create(IN ICarrierConfig* piCc) override;
    /**
     * @brief Update codec using the configuration
     *
     * @param piCc configuration
     * @return IMS_BOOL Return true if the create function is executed without error
     * Return false if the create function is failed
     */
    IMS_BOOL Update(IN ICarrierConfig* piCc) override;
    /**
     * @brief Get the T140 payload type number
     *
     * @return IMS_SINT32 Return T140 payload type number
     */
    IMS_SINT32 GetT140PayloadType() const;
    /**
     * @brief Get the redundant payload type value
     *
     * @return IMS_SINT32 Return redendancy payload type number
     */
    IMS_SINT32 GetRedPayloadType() const;
    /**
     * @brief Get the Text dscp value
     *
     * @return IMS_SINT32 Return text dscp value
     */
    IMS_SINT32 GetTextDscp() const;
    /**
     * @brief Return whether textxodec emptyredundant is enabled
     *
     * @return IMS_BOOL Return true if textxodec emptyredundant is enabled
     * Return false if  textxodec emptyredundant is disabled
     */
    IMS_BOOL IsTextCodecEmptyRedundantEnabled() const;

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
