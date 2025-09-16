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
#ifndef SDP_MEDIA_PARAMETER_H_
#define SDP_MEDIA_PARAMETER_H_

#include "SdpConnection.h"
#include "SdpMediaDescription.h"
#include "offeranswer/SdpMediaFormat.h"
#include "offeranswer/SdpParameter.h"
#include "offeranswer/SdpPrecondition.h"

#define __IMS_SDP_PRECONDITION__

class SdpMediaParameter : public SdpParameter
{
public:
    explicit SdpMediaParameter(IN IMS_SINT32 nMid);
    SdpMediaParameter(IN const SdpMediaParameter& other);
    ~SdpMediaParameter() override;

public:
    SdpMediaParameter& operator=(IN const SdpMediaParameter& other);

public:
    // SdpParameter class
    /**
     * @brief Returns the connection address which resides on the first line
     *        in the media-level parameter.
     */
    const AString& GetConnectionAddress() const override;

    /**
     * @brief Returns the SDP message from the media-level parameter.
     */
    AString ToSdp() const override;

    /**
     * @brief Negotiates the media-level parameters through the SDP offer/answer exchange.
     */
    IMS_SINT32 Compare(IN IMS_BOOL bInitialOffer, IN IMS_BOOL bIsOffer,
            IN const SdpMediaParameter* pPeerParam, OUT SdpMediaParameter*& pNegotiatedPeerParam,
            OUT SdpMediaParameter*& pProposalParam);

    /**
     * @brief Creates the media-level parameter from the SDP media description.
     */
    IMS_BOOL Create(IN const SdpMediaDescription& objMediaDescription);

    /**
     * @brief Returns the attribute value which appears in the attribute, "a=mid:".
     */
    inline const AString& GetAttributeMid() const { return m_strAttrMid; }

    /**
     * @brief Returns the list of SdpConnection object from the media-level parameter.
     */
    inline const ImsList<SdpConnection>& GetConnections() const { return m_objConnections; }

    /**
     * @brief Returns the SdpMedia object from the media-level parameter.
     */
    inline const SdpMedia& GetMedia() const { return m_objMedia; }

    /**
     * @brief Returns the media format which the specified media type & value matches.
     */
    const SdpMediaFormat* GetMediaFormat(IN IMS_SINT32 nType, IN const AString& strValue) const;

    /**
     * @brief Returns the media format which the specified media type & value matches.
     */
    SdpMediaFormat* GetMediaFormat(IN IMS_SINT32 nType, IN const AString& strValue);

    /**
     * @brief Returns all the media formats which the media-level parameter has.
     */
    inline const ImsList<SdpMediaFormat*>& GetMediaFormats() const { return m_objMediaFormats; }

    /**
     * @brief Returns the media identifier for this media-level parameter.
     */
    inline IMS_SINT32 GetMid() const { return m_nMid; }

    /**
     * @brief Checks if the connection line contains or not.
     */
    inline IMS_BOOL IsConnectionPresent() const { return (m_objConnections.GetSize() > 0); }

    /**
     * @brief Checks if the media can be acceptable.
     */
    inline IMS_BOOL IsMediaAccepted() const { return (m_objMedia.GetPort() != 0); }

    /**
     * @brief Checks if the mid attribute contains.
     */
    inline IMS_BOOL IsMidPresent() const { return m_abAttributeContains[ATTR_MID]; }

    /**
     * @brief Checks if the qos precondition contains.
     */
    IMS_BOOL IsQosPreconditionPresent() const;

    /**
     * @brief Marks the current media parameter as a rejected or removed.
     */
    void MarkRejectedOrRemoved();

    /**
     * @brief Removes all the connection lines from this media parameter.
     */
    void RemoveConnections();

    /**
     * @brief Removes the media format which the specified media type & value matches.
     */
    void RemoveMediaFormat(IN IMS_SINT32 nType, IN const AString& strValue);

    /**
     * @brief Sets the "mid" attribute to the media-level parameter.
     */
    void SetAttributeMid(IN const AString& strAttrMid);

    /**
     * @brief Sets the connection line to the media-level parameter.
     */
    IMS_BOOL SetConnectionAddress(IN const AString& strAddress);

    /**
     * @brief Sets the media (m-line) to the media-level parameter.
     */
    IMS_BOOL SetMedia(IN IMS_SINT32 nType, IN IMS_SINT32 nPort, IN IMS_SINT32 nTransportProtocol,
            IN const AStringArray& objFormats);

    /**
     * @brief Sets mid if it's changed.
     */
    void SetMid(IN IMS_SINT32 nMid);

    /**
     * @brief Updates the properties from the specified media-level parameter.
     */
    void UpdateProperties(IN const SdpMediaParameter& objMediaParam);

    // IMS_SDP_PRECONDITION
    /**
     * @brief Returns the 'qos' precondition of this media-level parameter.
     */
    SdpPrecondition* GetPrecondition(
            IN IMS_SINT32 nAttribute, IN IMS_SINT32 nType = SdpPrecondition::TYPE_QOS) const;

    /**
     * @brief Removes the 'qos' precondition of the media-level parameter.
     */
    void RemovePrecondition(
            IN IMS_SINT32 nAttribute, IN IMS_SINT32 nType = SdpPrecondition::TYPE_QOS);

    /**
     * @brief Sets the 'qos' precondition attribute to the media-level parameter.
     */
    IMS_BOOL SetPrecondition(IN IMS_SINT32 nAttribute, IN SdpPrecondition* pPrecondition);

    /**
     * @brief Extracts the media format from the SDP media description.
     */
    static IMS_BOOL ExtractMediaFormat(IN const SdpMediaDescription& objMediaDesc,
            OUT ImsList<SdpMediaFormat*>& objOutFormats);

    /**
     * @brief Creates the 'qos' precondition from the SDP attributes.
     */
    static SdpPrecondition* CreatePrecondition(
            IN const ImsList<SdpAttribute>& objAttributes, OUT ImsList<SdpAttribute>& objQosAttrs);

private:
    /**
     * @brief Clears the properties for the media-level parameter.
     */
    void Clear() override;

    /**
     * @brief Checks if the direction attribute is required or not when forming a-line(direction).
     */
    IMS_BOOL IsDirectionAttributeRequired() const override;

    /**
     * @brief Clears the media formats from the list of media format.
     */
    static void ClearMediaFormat(IN_OUT ImsList<SdpMediaFormat*>& objMediaFormats);

    /**
     * @brief Copies all the media formats.
     */
    static IMS_BOOL CopyMediaFormat(IN const ImsList<SdpMediaFormat*>& objInFormats,
            OUT ImsList<SdpMediaFormat*>& objOutFormats);

    /**
     * @brief Checks if the same AVCodec is present or not.
     *        It will just check the payload type for each media format.
     */
    static IMS_BOOL IsSameAvCodecPresent(
            IN const ImsList<SdpMediaFormat*>& objFormats, IN const SdpMediaFormat* pMediaFormat);

    /**
     * @brief Checks if the same non-AVCodec is present or not.
     *        It will just check the payload type for each media format.
     */
    static IMS_BOOL IsSameNonAvCodecPresent(
            IN const ImsList<SdpMediaFormat*>& objFormats, IN const SdpMediaFormat* pMediaFormat);

    /**
     * @brief Clears the 'qos' precondition from the SDP media parameter.
     */
    static void ClearPrecondition(IN_OUT SdpMediaParameter* pMediaParam);

    /**
     * @brief Copies the 'qos' precondition from the SDP media parameter.
     */
    static IMS_BOOL CopyPrecondition(
            IN const SdpMediaParameter& objMediaParam, IN_OUT SdpMediaParameter& objOutMediaParam);

#if defined(__SDP_CORRECT_FMTP_FOR_DUPLICATE_PAYLOAD_TYPES__)
    // 4 workaround solution for multiple fmtp
    static void CorrectFmtps(
            IN ImsList<IMS_SINT32>& objPayloadTypes4Fmtp, IN_OUT ImsList<SdpAttribute>& objFmtps);
#endif

private:
    // Attribute flags
    enum
    {
        ATTR_MID,
        ATTR_PTIME,
        ATTR_QOS_CONF,
        ATTR_QOS_CURR,
        ATTR_QOS_DES,
        ATTR_RCTP,
        ATTR_MAX
    };

    IMS_SINT32 m_nMid;

    // m-line (media)
    SdpMedia m_objMedia;

    // c-line (media connection information)
    ImsList<SdpConnection> m_objConnections;
    ImsList<SdpConnection> m_objPrevConnections;

    // a-line (media attributes)

    // Codecs (in case of RTP/AVP or RTP/SAVP) & format-specific parameters
    // a=rtpmap:xxxx , a=fmtp:xxxx
    ImsList<SdpMediaFormat*> m_objMediaFormats;

    // Attribute flags
    IMS_BOOL m_abAttributeContains[ATTR_MAX];

    // MID (a=mid:xxxx)
    AString m_strAttrMid;

    // QoS attributes (a=curr:xxxx, a=des:xxxx, a=conf:xxxx)
    // IMS_SDP_PRECONDITION
    SdpPrecondition* m_pCurrentStatus;
    SdpPrecondition* m_pDesiredStatus;
    SdpPrecondition* m_pConfirmedStatus;
};

#endif
