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
#ifndef SDP_BANDWIDTH_H_
#define SDP_BANDWIDTH_H_

#include "AString.h"

#include "SdpLine.h"

class SdpBandwidth : public SdpLine
{
public:
    SdpBandwidth();
    SdpBandwidth(IN const SdpBandwidth& other);
    ~SdpBandwidth() override;

public:
    SdpBandwidth& operator=(IN const SdpBandwidth& other);

public:
    // SdpLine class
    /**
     * @brief Decodes the bandwidth line ("b=") in the session description.
     *        The strValue contains a full bandwidth line without "b=".
     */
    IMS_BOOL Decode(IN const AString& strValue) override;

    /**
     * @brief Encodes the bandwidth line ("b=") in the session description.
     *        The returned value contains a full bandwidth line with "b=".
     */
    AString Encode() const override;

    /**
     * @brief Returns the full bandwidth line without "b=".
     */
    AString GetValue() const override;

    /**
     * @brief Checks if the specified bandwidth equals or not.
     */
    IMS_BOOL Equals(IN const SdpBandwidth* pBandwidth) const;

    /**
     * @brief Returns the bandwidth type as an integer value.
     */
    inline IMS_SINT32 GetType() const { return m_nType; }

    /**
     * @brief Returns the bandwidth type as a string value.
     */
    inline const AString& GetTypeName() const { return m_strType; }

    /**
     * @brief Returns the bandwidth value (in integer value).
     */
    inline IMS_SINT32 GetBandwidth() const { return m_nBandwidth; }

    /**
     * @brief Sets the bandwidth type & value.
     */
    IMS_BOOL SetValue(IN IMS_SINT32 nType, IN IMS_SINT32 nBandwidth,
            IN const AString& strType = AString::ConstNull());

public:
    enum
    {
        INVALID_BANDWIDTH = -1
    };

    enum
    {
        TYPE_AS,
        TYPE_CT,
        TYPE_RR,    // RFC 3556, SDP Bandwidth Modifiers for RTCP Bandwidth
        TYPE_RS,    // RFC 3556, SDP Bandwidth Modifiers for RTCP Bandwidth
        TYPE_TIAS,  // RFC 3890, Bandwidth Modifier for SDP
        TYPE_OTHER
    };

    static const IMS_CHAR TOKEN_AS[];
    static const IMS_CHAR TOKEN_CT[];
    static const IMS_CHAR TOKEN_RR[];
    static const IMS_CHAR TOKEN_RS[];
    static const IMS_CHAR TOKEN_TIAS[];

private:
    // Bandwidth type as enumeration
    IMS_SINT32 m_nType;
    // Bandwidth type as string
    AString m_strType;
    // Bandwidth value
    IMS_SINT32 m_nBandwidth;
};

#endif
