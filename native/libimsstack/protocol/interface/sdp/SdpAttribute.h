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
#ifndef SDP_ATTRIBUTE_H_
#define SDP_ATTRIBUTE_H_

#include "AString.h"

#include "SdpLine.h"

class SdpAttribute : public SdpLine
{
public:
    SdpAttribute();
    SdpAttribute(IN const SdpAttribute& other);
    ~SdpAttribute() override;

public:
    SdpAttribute& operator=(IN const SdpAttribute& other);

public:
    // SdpLine class
    /**
     * @brief Decodes the attribute line ("a=") in the session description.
     *        The strValue contains a full attribute line without "a=".
     */
    IMS_BOOL Decode(IN const AString& strValue) override;

    /**
     * @brief Encodes the attribute line ("a=") in the session description.
     *        The returned value contains a full attribute line with "a=".
     */
    AString Encode() const override;

    /**
     * @brief Returns the full attribute line without "a=".
     */
    AString GetValue() const override;

    /**
     * @brief Checks if the specified attribute is equals.
     */
    IMS_BOOL Equals(IN const SdpAttribute* pAttribute) const;

    /**
     * @brief Returns the type of the current attribute as an enumeration.
     */
    inline IMS_SINT32 GetAttribute() const { return m_nAttribute; }

    /**
     * @brief Returns the type of the current attribute as a string.
     */
    inline const AString& GetAttributeName() const { return m_strAttribute; }

    /**
     * @brief Returns the value of the current attribute.
     */
    const AString& GetAttributeValue() const { return m_strAttrValue; }

    /**
     * @brief Sets the type & value of the attribute.
     */
    IMS_BOOL SetValue(IN IMS_SINT32 nAttribute, IN const AString& strAttrValue,
            IN const AString& strAttribute = AString::ConstNull());

    /**
     * @brief Converts the direction type to the attribute type.
     *
     * The following direction can be allowed:
     *     #Sdp#DIRECTION_INACTIVE
     *     #Sdp#DIRECTION_RECVONLY
     *     #Sdp#DIRECTION_SENDONLY
     *     #Sdp#DIRECTION_SENDRECV
     */
    static IMS_SINT32 ConvertDirectionToAttribute(IN IMS_SINT32 nDirection);

    /**
     * @brief Converts the attribute type to the direction type.
     *
     * The following attribute can be allowed:
     *     #INACTIVE
     *     #RECVONLY
     *     #SENDONLY
     *     #SENDRECV
     */
    static IMS_SINT32 ConvertAttributeToDirection(IN IMS_SINT32 nAttribute);

    /**
     * @brief Gets the specified attribute name.
     */
    static const IMS_CHAR* GetAttributeName(IN IMS_SINT32 nAttribute);

private:
    /**
     * @brief Checks if the specified attribute is a value token.
     */
    static IMS_SINT32 IsValueToken(
            IN IMS_SINT32 nAttribute, IN const AString& strAttribute = AString::ConstNull());

    /**
     * @brief Returns the format of the attribute (name only or name & value pair).
     */
    static IMS_SINT32 GetFormat(IN IMS_SINT32 nFormat, IN IMS_SINT32 nAttribute,
            IN const AString& strAttribute = AString::ConstNull());

public:
    // Type of attribute
    enum
    {
        ATTRIBUTE_INVALID = (-1),
        CAT = 0,
        KEYWDS,
        TOOL,
        PTIME,
        MAXPTIME,
        RTPMAP,
        RECVONLY,
        SENDRECV,
        SENDONLY,
        INACTIVE,
        ORIENT,
        TYPE,
        CHARSET,
        SDPLANG,
        LANG,
        FRAMERATE,
        QUALITY,
        FMTP,
        // Extensions
        CURR,                  // RFC 3312, Integration of Resource Management and SIP
        DES,                   // RFC 3312, Integration of Resource Management and SIP
        CONF,                  // RFC 3312, Integration of Resource Management and SIP
        MID,                   // RFC 3388, Grouping of Media Lines in SDP
        GROUP,                 // RFC 3388, Grouping of Media Lines in SDP
        RTCP,                  // RFC 3605, RTCP attribute in SDP
        RTCP_XR,               // RFC 3611, RTP Control Protocol Extended Reports (RTCP XR)
        MAX_PRATE,             // RFC 3890, Bandwidth Modifier for SDP
        SETUP,                 // RFC 4145, TCP-Based Media Transport in the SDP
        CONNECTION,            // RFC 4145, TCP-Based Media Transport in the SDP
        LABEL,                 // RFC 4574, SDP Label Attribute
        RTCP_FB,               // RFC 4585, RCTP - Based Feedback (RTP/AVPF)
        CONTENT,               // RFC 4796, SDP Content Attribute
        ACCEPT_TYPES,          // RFC 4975, The Message Session Relay Protocol (MSRP)
        ACCEPT_WRAPPED_TYPES,  // RFC 4975, The Message Session Relay Protocol (MSRP)
        MAX_SIZE,              // RFC 4975, The Message Session Relay Protocol (MSRP)
        PATH,                  // RFC 4975, The Message Session Relay Protocol (MSRP)
        CANDIDATE,      // RFC 5245, ICE : A Protocol for NAT Traversal for Offer/Answer Protocols
        FILE_SELECTOR,  // RFC 5547, SDP Offer/Answer to Enable File Transfer
        FILE_TRANSFER_ID,  // RFC 5547, SDP Offer/Answer to Enable File Transfer
        FILE_DISPOSITION,  // RFC 5547, SDP Offer/Answer to Enable File Transfer
        FILE_DATE,         // RFC 5547, SDP Offer/Answer to Enable File Transfer
        FILE_ICON,         // RFC 5547, SDP Offer/Answer to Enable File Transfer
        FILE_RANGE,        // RFC 5547, SDP Offer/Answer to Enable File Transfer
        CSUP,              // RFC 5939, SDP Capability Negotiation
        CREQ,              // RFC 5939, SDP Capability Negotiation
        ACAP,              // RFC 5939, SDP Capability Negotiation
        TCAP,              // RFC 5939, SDP Capability Negotiation
        PCFG,              // RFC 5939, SDP Capability Negotiation
        ACFG,              // RFC 5939, SDP Capability Negotiation
        // <payload type number> <width>-<height>
        FRAMESIZE,  // RFC 6064, SDP and RTSP Extensions Defined for 3GPP PSS and MBMS
        IMAGEATTR,  // RFC 6236, Negotiation of Generic Image Attributes in the SDP
        CRYPTO,     // RFC 4568, SDP Security Descriptions for Media Streams
        A_3GE2AE,   // TS 24.229, 3GPP End-To-Access-Edge security-indicator
        ANBR,       // TS 26.114, Access Network Bitrate Recommendation
        ATTRIBUTE_OTHER,
        ATTRIBUTE_ALL,  // Special attribute type for RemoveAll operation
        ATTRIBUTE_MAX
    };

    // Forming method of attribute
    enum
    {
        // a=<attribute>
        FORMAT_PROPERTY_ATTRIBUTE,
        // a=<attribute>:<value>
        FORMAT_VALUE_ATTRIBUTE
    };

private:
    enum
    {
        // No token value
        VALUE_NO_TOKEN = 0,
        // Token only
        VALUE_TOKEN,
        // Token & SP
        VALUE_TOKEN_SP
    };

    static const IMS_CHAR* ATTRIBUTE[ATTRIBUTE_MAX];

    // Attribute format
    IMS_SINT32 m_nFormat;
    // Attribute type as enumeration
    IMS_SINT32 m_nAttribute;
    // Attribute type as string
    AString m_strAttribute;
    // Attribute value
    // If attribute is a property type, then it will be an empty string instead of null string.
    AString m_strAttrValue;
};

#endif
