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
#ifndef SDP_MEDIA_FORMAT_H_
#define SDP_MEDIA_FORMAT_H_

#include "AString.h"

class SdpMediaFormatParameter;

class SdpMediaFormat
{
public:
    explicit SdpMediaFormat(IN IMS_SINT32 nType = TYPE_OTHER);
    SdpMediaFormat(IN const SdpMediaFormat& other);
    virtual ~SdpMediaFormat();

public:
    SdpMediaFormat& operator=(IN const SdpMediaFormat& other);

public:
    /**
     * @brief Checks if the specified media format equals or not.
     */
    virtual IMS_BOOL Equals(IN const SdpMediaFormat* pFormat) const;
    /**
     * @brief Checks if the media format has any attribute field.
     */
    inline virtual IMS_BOOL HasAttribute() const { return IMS_FALSE; }
    /**
     * @brief Sets the media-dedicated parameters for this media format.
     */
    inline virtual IMS_BOOL SetParameters(
            IN const AString& /*strAttrAnyMap*/, IN const AString& /*strAttrFmtp*/)
    {
        return IMS_FALSE;
    }
    /**
     * @brief Sets the value for this media format.
     */
    virtual IMS_BOOL SetValue(IN const AString& strFormat);
    /**
     * @brief Returns all the SDP lines for this media format according to the SDP format.
     */
    inline virtual AString ToSdp() const { return AString::ConstNull(); }

    /**
     * @brief Returns the type for this media format.
     *
     * @return The transport type of this media.\n
     *         #TYPE_RTP\n
     *         #TYPE_MSRP\n
     *         #TYPE_UDP\n
     *         #TYPE_TCP
     */
    inline IMS_SINT32 GetType() const { return m_nType; }

    /**
     * @brief Returns the value for this media format.
     *
     * TYPE_RTP -> payload type
     * TYPE_MSRP -> "*"
     * TYPE_UDP -> ??
     * TYPE_TCP -> ??
     *
     * @return The value of this media format.
     */
    inline const AString& GetValue() const { return m_strValue; }

    // Additional payload specific parameters
    /**
     * @brief Adds an additional payload specific parameter.
     */
    void AddExtraParameter(IN SdpMediaFormatParameter* pParameter);
    /**
     * @brief Returns all the additional payload specific parameters for this media format.
     */
    inline const ImsList<SdpMediaFormatParameter*>& GetExtraParameters() const
    {
        return m_objExtraParameters;
    }

public:
    // Type of a media format
    enum
    {
        TYPE_RTP,  // audio / video
        TYPE_MSRP,
        TYPE_UDP,
        TYPE_TCP,
        TYPE_OTHER
    };

private:
    IMS_SINT32 m_nType;
    AString m_strValue;

    // Additional payload specific parameters
    ImsList<SdpMediaFormatParameter*> m_objExtraParameters;
};

#endif
