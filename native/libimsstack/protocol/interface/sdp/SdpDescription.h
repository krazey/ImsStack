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
#ifndef SDP_DESCRIPTION_H_
#define SDP_DESCRIPTION_H_

#include "Sdp.h"
#include "SdpAttribute.h"
#include "SdpBandwidth.h"

class SdpEncryptionKey;
class SdpInformation;

class SdpDescription
{
public:
    SdpDescription();
    SdpDescription(IN const SdpDescription& other);
    virtual ~SdpDescription();

public:
    SdpDescription& operator=(IN const SdpDescription& other);

public:
    /**
     * @brief Decodes the SDP lines in the session description.
     *        It can be used in the session-level or media-level description.
     */
    virtual IMS_BOOL Decode(IN const AStringArray& objLines, IN IMS_SINT32 nStartLine = 0,
            IN IMS_SINT32 nEndLine = -1);

    /**
     * @brief Encodes the SDP lines in the session description.
     */
    virtual AString Encode() const;

    /**
     * @brief Adds a new attribute to the SDP description.
     */
    IMS_BOOL AddAttribute(IN const SdpAttribute& objAttribute);

    /**
     * @brief Adds a new bandwidth to the SDP description.
     */
    IMS_BOOL AddBandwidth(IN const SdpBandwidth& objBandwidth);

    /**
     * @brief Checks if the SDP description contains the specified line.
     */
    IMS_BOOL Contains(IN IMS_SINT32 nLine) const;

    /**
     * @brief Checks if the SDP description contains the specified attribute line.
     */
    IMS_BOOL Contains(IN const SdpAttribute& objAttribute);

    /**
     * @brief Checks if the SDP description contains the specified bandwidth line.
     */
    IMS_BOOL Contains(IN const SdpBandwidth& objBandwidth);

    /**
     * @brief Returns the SdpAttribute object from the specified attribute type (integer value).
     *
     * NOTE: The pre-defined attribute type is in SdpAttribute.h
     */
    const SdpAttribute* GetAttribute(IN IMS_SINT32 nAttribute) const;

    /**
     * @brief Returns the SdpAttribute object from the specified attribute type (string value).
     */
    const SdpAttribute* GetAttribute(IN const AString& strAttribute) const;

    /**
     * @brief Returns all the SdpAttribute objects.
     */
    inline const ImsList<SdpAttribute>& GetAttributes() const { return m_objAttributes; }

    /**
     * @brief Returns the list of SdpAttribute object from the specified attribute type
     *        (integer value).
     */
    ImsList<SdpAttribute> GetAttributes(IN IMS_SINT32 nAttribute) const;

    /**
     * @brief Returns the list of SdpAttribute object from the specified attribute type
     *        (string value).
     */
    ImsList<SdpAttribute> GetAttributes(IN const AString& strAttribute) const;

    /**
     * @brief Returns all the SdpBandwidth objects.
     */
    inline const ImsList<SdpBandwidth>& GetBandwidths() const { return m_objBandwidths; }

    /**
     * @brief Returns the direction value in this description.
     */
    IMS_SINT32 GetDirection() const;

    /**
     * @brief Returns the encryption key object from the current description.
     */
    inline const SdpEncryptionKey* GetEncryptionKey() const { return m_pEncryptionKey; }

    /**
     * @brief Returns the information object from the current description.
     */
    inline const SdpInformation* GetInformation() const { return m_pInformation; }

    /**
     * @brief Removes the attributes that match with the specified attribute.
     */
    void RemoveAttributes(IN IMS_SINT32 nAttribute);

    /**
     * @brief Removes the attribute that it matches with the specified attribute.
     */
    void RemoveAttribute(IN const SdpAttribute& objAttribute);

    /**
     * @brief Sets the list of SdpAttribute object.
     */
    void SetAttributes(IN const ImsList<SdpAttribute>& objAttributes);

    /**
     * @brief Sets the list of SdpBandwidth object.
     */
    void SetBandwidths(IN const ImsList<SdpBandwidth>& objBandwidths);

    /**
     * @brief Sets the encryption key.
     */
    void SetEncryptionKey(IN const SdpEncryptionKey& objEncryptionKey);

    /**
     * @brief Sets the information field.
     */
    void SetInformation(IN const SdpInformation& objInformation);

protected:
    IMS_BOOL m_abLineContains[Sdp::TYPE_MAX];

private:
    // Session-Level: session information, Media-Level: media title
    SdpInformation* m_pInformation;

    // Bandwidth information lines
    ImsList<SdpBandwidth> m_objBandwidths;

    // Encryption key
    SdpEncryptionKey* m_pEncryptionKey;

    // Session or media attribute lines
    ImsList<SdpAttribute> m_objAttributes;
};

#endif
