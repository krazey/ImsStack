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
#ifndef SDP_PARAMETER_H_
#define SDP_PARAMETER_H_

#include "SdpDescription.h"

class SdpParameter
{
public:
    SdpParameter();
    SdpParameter(IN const SdpParameter& other);
    virtual ~SdpParameter();

public:
    SdpParameter& operator=(IN const SdpParameter& other);

public:
    /**
     * @brief Returns the connection address which resides on the first line
     *        in the current SDP order.
     */
    inline virtual const AString& GetConnectionAddress() const { return AString::ConstNull(); }

    /**
     * @brief Returns the SDP message from the current SDP parameter.
     */
    virtual AString ToSdp() const;

    /**
     * @brief Adds the attribute line to the SDP parameter.
     */
    IMS_BOOL AddAttribute(IN const SdpAttribute& objAttribute);

    /**
     * @brief Adds the bandwidth line to the SDP parameter.
     */
    IMS_BOOL AddBandwidth(IN const SdpBandwidth& objBandwidth);

    /**
     * @brief Checks if the specified attribute line contains or not.
     */
    IMS_BOOL Contains(IN const SdpAttribute& objAttribute);

    /**
     * @brief Checks if the specified bandwidth line contains or not.
     */
    IMS_BOOL Contains(IN const SdpBandwidth& objBandwidth);

    /**
     * @brief Creates a SDP parameter from the SDP description.
     */
    IMS_BOOL Create(IN const SdpDescription& objDescription);

    /**
     * @brief Returns the SdpAttribute object which the specified type matches.
     */
    const SdpAttribute* GetAttribute(IN IMS_SINT32 nAttribute) const;

    /**
     * @brief Returns the SdpAttribute object which the specified name matches.
     */
    const SdpAttribute* GetAttribute(IN const AString& strAttribute) const;

    /**
     * @brief Returns all the SdpAttribute objects.
     */
    inline const ImsList<SdpAttribute>& GetAttributes() const { return m_objAttributes; }

    /**
     * @brief Returns all the SdpAttribute objects which the specified type matches.
     */
    ImsList<SdpAttribute> GetAttributes(IN IMS_SINT32 nAttribute) const;

    /**
     * @brief Returns all the SdpAttribute objects which the specified name matches.
     */
    ImsList<SdpAttribute> GetAttributes(IN const AString& strAttribute) const;

    /**
     * @brief Returns the SdpBandwidth object which the specified type matches.
     */
    const SdpBandwidth* GetBandwidth(IN IMS_SINT32 nType) const;

    /**
     * @brief Returns the SdpBandwidth object which the specified name matches.
     */
    const SdpBandwidth* GetBandwidth(IN const AString& strType) const;

    /**
     * @brief Returns all the SdpBandwidth objects.
     */
    inline const ImsList<SdpBandwidth>& GetBandwidths() const { return m_objBandwidths; }

    /**
     * @brief Returns the direction value of the SDP parameter.
     */
    inline IMS_SINT32 GetDirection() const { return m_nDirection; }

    /**
     * @brief Returns the SdpEncryptionKey object.
     */
    inline const SdpEncryptionKey* GetEncryptionKey() const { return m_pKey; }

    /**
     * @brief Returns the SdpInformation object.
     */
    inline const SdpInformation* GetInformation() const { return m_pInformation; }

    /**
     * @brief Returns the value of "connection" attribute field.
     *
     * @return The connection value.\n
     *         #Sdp#CONNECTION_NEW\n
     *         #Sdp#CONNECTION_EXISTING
     */
    inline IMS_SINT32 GetAttributeConnection() const { return m_nAttrConnection; }

    /**
     * @brief Returns the value of "setup" attribute field.
     *
     * @return The setup value.\n
     *         #Sdp#SETUP_ACTIVE\n
     *         #Sdp#SETUP_PASSIVE\n
     *         #Sdp#SETUP_ACTPASS\n
     *         #Sdp#SETUP_HOLDCONN
     */
    inline IMS_SINT32 GetAttributeSetup() const { return m_nAttrSetup; }

    /**
     * @brief Negotiates the direction value of the SDP parameter.
     */
    void NegotiateDirection(IN IMS_SINT32 nCurrentDirection);

    /**
     * @brief Removes the attribute field which the specified type matches.
     *        It only removes the first matched attribute field.
     */
    void RemoveAttribute(IN IMS_SINT32 nAttribute);

    /**
     * @brief Removes the attribute field which the specified SdpAttribute object matches.
     */
    void RemoveAttribute(IN const SdpAttribute& objAttribute);

    /**
     * @brief Removes all the attribute fields which the specified type matches.
     */
    void RemoveAttributes(IN IMS_SINT32 nAttribute);

    /**
     * @brief Removes all the attribute fields.
     */
    void RemoveAttributes();

    /**
     * @brief Removes all the attribute fields.
     */
    void RemoveEncryptionKey();

    /**
     * @brief Removes the information field.
     */
    void RemoveInformation();

    /**
     * @brief Sets the bandwidth fields with the list of SdpBandwidth object.
     */
    void SetBandwidths(IN const ImsList<SdpBandwidth>& objBandwidths);

    /**
     * @brief Sets the direction value.
     */
    void SetDirection(IN IMS_SINT32 nDirection);

    /**
     * @brief Sets the encryption key field with the specified SdpEncryptionKey object.
     */
    void SetEncryptionKey(IN const SdpEncryptionKey& objEncryptionKey);

    /**
     * @brief Sets the information field with the specified SdpInformation object.
     */
    void SetInformation(IN const SdpInformation& objInformation);

    /**
     * @brief Sets the "connection" attribute field with the specified connection value.
     *
     * @param nAttrConnection The connection value.\n
     *                        #Sdp#CONNECTION_NEW\n
     *                        #Sdp#CONNECTION_EXISTING
     */
    void SetAttributeConnection(IN IMS_SINT32 nAttrConnection);

    /**
     * @brief Sets the "setup" attribute field with the specified setup value.
     *
     * @param nAttrSetup The setup value.\n
     *                   #Sdp#SETUP_ACTIVE\n
     *                   #Sdp#SETUP_PASSIVE\n
     *                   #Sdp#SETUP_ACTPASS\n
     *                   #Sdp#SETUP_HOLDCONN
     */
    void SetAttributeSetup(IN IMS_SINT32 nAttrSetup);

    /**
     * @brief Updates the direction of the current SDP parameter.
     */
    void UpdateDirection();

    /**
     * @brief Updates the direction from the specified SDP parameter.
     */
    void UpdateDirection(IN const SdpParameter& objParam);

    /**
     * @brief Updates the direction according to the peer & proposal SDP parameter.
     */
    void UpdateDirection(IN const SdpParameter& objPeer, OUT SdpParameter& objProposal) const;

    /**
     * @brief Validates the direction according to the peer SDP parameter.
     */
    IMS_BOOL ValidateDirection(IN const SdpParameter* pPeer) const;

protected:
    /**
     * @brief Clears all the properties for the current SDP parameter.
     */
    virtual void Clear();

    /**
     * @brief Checks if the direction attribute is required or not when forming a-line(direction).
     */
    inline virtual IMS_BOOL IsDirectionAttributeRequired() const { return IMS_TRUE; }

    /**
     * @brief Clears all the properties for the current SDP parameter.
     */
    void ClearAllParameters();

    /**
     * @brief Checks if the specified type contains or not.
     */
    IMS_BOOL Contains(IN IMS_SINT32 nType) const;

    /**
     * @brief Updates the properties with the specified SDP parameter.
     */
    void UpdateProperties(IN const SdpParameter& objParam);

    /**
     * @brief Validates the direction according to the current & offered direction.
     */
    static IMS_BOOL ValidateDirection(
            IN IMS_SINT32 nCurrentDirection, IN IMS_SINT32 nOfferDirection);

protected:
    IMS_BOOL m_abLineContains[Sdp::TYPE_MAX];

    IMS_BOOL m_bDirectionPresent;
    // a-line for direction
    IMS_SINT32 m_nDirection;
    // Stores the previous direction type - used for resuming the muted (held) stream back
    IMS_SINT32 m_nPreviousDirection;

    // maxprate

    // TCP connection attributes : setup, connection
    // active, passive, actpass, holdconn
    IMS_SINT32 m_nAttrSetup;
    // new, existing
    IMS_SINT32 m_nAttrConnection;

private:
    // Attribute flags
    enum
    {
        ATTR_CONNECTION,
        ATTR_SETUP,
        ATTR_MAX
    };

    IMS_BOOL m_abAttributeContains[ATTR_MAX];

    // Session-Level: session information, Media-Level: media title
    SdpInformation* m_pInformation;

    // Bandwidth information lines
    ImsList<SdpBandwidth> m_objBandwidths;

    // Encryption key
    SdpEncryptionKey* m_pKey;

    // Session or media attribute lines
    ImsList<SdpAttribute> m_objAttributes;
};

#endif
