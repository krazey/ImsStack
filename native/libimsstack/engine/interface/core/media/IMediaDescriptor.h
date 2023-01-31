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
#ifndef INTERFACE_MEDIA_DESCRIPTOR_H_
#define INTERFACE_MEDIA_DESCRIPTOR_H_

#include "IpAddress.h"

#include "SdpAttribute.h"
#include "SdpBandwidth.h"
#include "SdpMedia.h"
#include "offeranswer/SdpMediaFormat.h"
#include "offeranswer/SdpPrecondition.h"

/**
 * @brief IMediaDescriptor is an interface towards the media parts of the SDP.
 *
 * The interface is most useful for applications that, within the realm of a composed capability
 * (ICSI, IARI and/or other service level feature tags), need to manipulate media-related fields
 * of the session SDP in general, and add new application-specific attributes in particular.
 *
 * NOTE:\n
 *   The getters of the interface read fields from the last received SDP.\n The setters modify
 *   all remaining SDPs to be sent during the lifetime of the media in the session,
 *   or until superseded by other modifications.
 *
 * RESERVED ATTRIBUTES :\n
 *     des, curr, conf, mid, ice-pwd, ice-ufrag, candidate, remote-candidates, sendonly,\n
 *     recvonly, sendrecv, inactive, csup, creq, acap, tcap, pcfg, acfg, 3gpp_sync_info
 *
 *     - RTP/AVP (StreamMedia) : rtpmap, fmtp, dccp-service-code, rtcp-mux, rtp-fb,
 *         ptime, maxptime, framesize, framerate, quality
 *     - TCP (BasicReliableMedia) : setup, connection
 *     - TCP/MSRP (FramedMedia) : setup, connection, accept-types, accept-wrapped-types,
 *         max-size, path
 */
class IMediaDescriptor
{
protected:
    virtual ~IMediaDescriptor() = default;

public:
    /**
     * @brief Adds an attribute (a=) to the IMedia.
     *
     * Adding attributes that the IMS engine can set, such as
     * "sendonly", "recvonly", "sendrecv" will lead to an ILLEGAL_ARGUMENT error.
     *
     * For example, AddAttribute("max-size:4096"); -> "a=max-size:4096".
     *
     * NOTE: If the IMedia is in STATE_ACTIVE, the attribute will be set on the proposal media
     * until the ISession has been updated.\n The proposal media can be retrieved with the
     * GetProposal() method on the IMeida interface.
     *
     * The following exception can be throwed,
     *     - ILLEGAL_ARGUMENT
     *         - if the strAttribute argument is NULL
     *         - if the syntax of the strAttribute argument is invalid
     *         - if the strAttribute already exists in the IMedia
     *         - if the strAttribute is reserved for the IMS engine
     *     - ILLEGAL_STATE
     *         - if the IMedia is not in STATE_INACTIVE or STATE_ACTIVE state
     *     - GENERAL_ERROR
     *         - if the internal error occurred
     *
     * @param strAttribute Attribute to be added
     * @return If it succeeds, return IMS_SUCCESS. Otherwise, returns IMS_FAILURE.
     */
    virtual IMS_RESULT AddAttribute(IN const AString& strAttribute) = 0;

    /**
     * @brief Returns all attributes (a=) for the IMedia.
     *
     * If there are no attributes, an empty string array will be returned.
     *
     * @return List of all attribute values.
     */
    virtual IMSList<AString> GetAttributes() const = 0;

    /**
     * @brief Returns the proposed bandwidth (b=) to be used by the media.
     *
     * An empty string array will be returned if the bandwidth information could not be retrieved.
     *
     * @return List of all bandwidth values.
     */
    virtual IMSList<AString> GetBandwidthInfo() const = 0;

    /**
     * @brief Returns the contents of the media description field (m=) of the current incoming SDP
     *        for this media.
     *
     * Example of an SDP m-line that may be returned for an audio stream media:
     * "audio 4000 RTP/AVP 97 101".
     *
     * @return Media description line as string.
     */
    virtual AString GetMediaDescription() const = 0;

    /**
     * @brief Returns the title (i=) of the IMedia.
     *
     * @return Title of IMedia or null if the title has not been set.
     */
    virtual AString GetMediaTitle() const = 0;

    /**
     * @brief Removes an attribute (a=) form the IMedia.
     *
     * For example, if the descriptor has an attribute, "a=max-size:4096"
     * and the RemoveAttribute("max-size:4096") is called, it will remove that attribute line.
     *
     * NOTE: If the IMedia is in STATE_ACTIVE, the attribute will be removed on the proposal media
     * until the ISession has been updated.\n The proposal media can be retrieved with the
     * GetProposal() method on IMedia interface.
     *
     * The following exception can be throwed,
     *     - ILLEGAL_ARGUMENT
     *         - if the strAttribute argument is NULL
     *         - if the syntax of the strAttribute argument is invalid
     *         - if the strAttribute does not exist in the IMedia
     *         - if the strAttribute is reserved for the IMS engine
     *     - ILLEGAL_STATE
     *         - if the IMedia is not in STATE_INACTIVE or STATE_ACTIVE state
     *     - GENERAL_ERROR
     *         - if the internal error occurred
     *
     * @param strAttribute Attribute to be removed
     * @return If it succeeds, return IMS_SUCCESS. Otherwise, returns IMS_FAILURE.
     */
    virtual IMS_RESULT RemoveAttribute(IN const AString& strAttribute) = 0;

    /**
     * @brief Sets the proposed bandwidth (b=) to be used by the media.
     *
     * The following exception can be throwed,
     *     - ILLEGAL_ARGUMENT
     *         - if the syntax of the strBandwidthInfos argument is invalid
     *     - ILLEGAL_STATE
     *         - if the IMedia is not in STATE_INACTIVE or STATE_ACTIVE state
     *     - GENERAL_ERROR
     *         - if the internal error occurred
     *
     * @param strBandwidthInfos List of bandwidth attributes to be set
     * @return If it succeeds, return IMS_SUCCESS. Otherwise, returns IMS_FAILURE.
     */
    virtual IMS_RESULT SetBandwidthInfo(IN const IMSList<AString>& strBandwidthInfos) = 0;

    /**
     * @brief Sets a title (i=) to the IMedia.
     *
     * The following exception can be throwed,
     *     - ILLEGAL_ARGUMENT
     *         - if the syntax of the strBandwidthInfos argument is invalid
     *     - ILLEGAL_STATE
     *         - if the IMedia is not in STATE_INACTIVE or STATE_ACTIVE state
     *     - GENERAL_ERROR
     *         - if the internal error occurred
     *
     * @param strTitle Title value to be set
     * @return If it succeeds, return IMS_SUCCESS. Otherwise, returns IMS_FAILURE.
     */
    virtual IMS_RESULT SetMediaTitle(IN const AString& strTitle) = 0;

    /**
     * @brief Adds an attribute (a=) to the IMedia.
     *
     * For example, AddAttribute(SdpAttribute::MAX_SIZE, "4096"); -> "a=max-size:4096".
     *
     * NOTE: If the IMedia is in STATE_ACTIVE, the attribute will be set on the proposal media
     * until the ISession has been updated.\n The proposal media can be retrieved with the
     * GetProposal() method on the IMeida interface.
     *
     * The following attribute is not allowed:
     *     - SdpAttribute::RECVONLY
     *     - SdpAttribute::SENDRECV
     *     - SdpAttribute::SENDONLY
     *     - SdpAttribute::SETUP
     *     - SdpAttribute::CONNECTION
     *
     * The following exception can be throwed,
     *     - ILLEGAL_ARGUMENT
     *         - if the nType argument is not allowed
     *         - if the syntax of strAttrValue is invalid
     *         - if the attribute already exists
     *     - ILLEGAL_STATE
     *         - if the IMedia is not in STATE_INACTIVE or STATE_ACTIVE state
     *     - GENERAL_ERROR
     *         - if the internal error occurred
     *
     * @param nType Attribute type; SdpAttribute::PTIME, ... in SdpAttribute.h
     * @param strAttrValue Attribute value to be set (string value only)
     * @param strType Name of unknown attribute type
     * @return If it succeeds, return IMS_SUCCESS. Otherwise, returns IMS_FAILURE.
     */
    virtual IMS_RESULT AddAttribute(IN IMS_SINT32 nType, IN const AString& strAttrValue,
            IN const AString& strType = AString::ConstNull()) = 0;

    /**
     * @brief Adds an attribute (a=) to the IMedia.
     *
     * For example,\n
     *     AddAttributeInt(SdpAttribute::SETUP, Sdp::SETUP_ACTIVE); -> "a=setup:active".
     *     AddAttributeInt(SdpAttribute::MAX_SIZE, 4096); -> "a=max-size:4096".
     *
     * NOTE: If the IMedia is in STATE_ACTIVE, the attribute will be set on the proposal media
     * until the ISession has been updated.\n The proposal media can be retrieved with the
     * GetProposal() method on the IMeida interface.
     *
     * The following attribute is allowed:
     *     - SdpAttribute::SETUP
     *     - SdpAttribute::CONNECTION
     *
     * The following attribute is not allowed:
     *     - SdpAttribute::RECVONLY
     *     - SdpAttribute::SENDRECV
     *     - SdpAttribute::SENDONLY
     *
     * The following exception can be throwed,
     *     - ILLEGAL_ARGUMENT
     *         - if the nType argument is not allowed
     *         - if the syntax of strAttrValue is invalid
     *         - if the attribute already exists
     *     - ILLEGAL_STATE
     *         - if the IMedia is not in STATE_INACTIVE or STATE_ACTIVE state
     *     - GENERAL_ERROR
     *         - if the internal error occurred
     *
     * @param nType Attribute type; SdpAttribute::PTIME, ... in SdpAttribute.h
     * @param nAttrValue Attribute value to be set (integer value only)
     * @param strType Name of unknown attribute type
     * @return If it succeeds, return IMS_SUCCESS. Otherwise, returns IMS_FAILURE.
     */
    virtual IMS_RESULT AddAttributeInt(IN IMS_SINT32 nType, IN IMS_SINT32 nAttrValue,
            IN const AString& strType = AString::ConstNull()) = 0;

    /**
     * @brief Sets the proposed bandwidth (b=) to be used by the media.
     *
     * For example, AddBandwidth(SdpBandwidth::AS, 128); -> "b=AS:128".
     *
     * The following exception can be throwed,
     *     - ILLEGAL_ARGUMENT
     *         - if the nType or nBandwidth argument is invalid
     *     - ILLEGAL_STATE
     *         - if the IMedia is not in STATE_INACTIVE or STATE_ACTIVE state
     *     - GENERAL_ERROR
     *         - if the internal error occurred
     *
     * @param nType Bandwidth type; SdpBandwidth::TYPE_AS, ... in SdpBandwidth.h
     * @param nBandwidth Bandwidth value to be set
     * @param strType Name of unknown bandwidth type
     * @return If it succeeds, return IMS_SUCCESS. Otherwise, returns IMS_FAILURE.
     */
    virtual IMS_RESULT AddBandwidth(IN IMS_SINT32 nType, IN IMS_SINT32 nBandwidth,
            IN const AString& strType = AString::ConstNull()) = 0;

    /**
     * @brief Returns the specified attribute (a=) for the IMedia.
     *
     * If there is no attribute, a NULL string will be returned.\n
     * If attribute is other type and field only type, an EMPTY string will be returned.
     *
     * NOTE:
     * The following attribute is not allowed:
     *     - SdpAttribute::RECVONLY
     *     - SdpAttribute::SENDRECV
     *     - SdpAttribute::SENDONLY
     *     - SdpAttribute::SETUP
     *     - SdpAttribute::CONNECTION
     *
     * @param nType Attribute type; SdpAttribute::PTIME, ... in SdpAttribute.h
     * @param strType Name of unknown attribute type
     * @return Attribute value or null if not present.
     */
    virtual const AString& GetAttribute(
            IN IMS_SINT32 nType, IN const AString& strType = AString::ConstNull()) const = 0;

    /**
     * @brief Returns the specified attributes (a=) for the IMedia.
     *
     * If there is no attribute, an empty string list will be returned.
     *
     * NOTE:
     * The following attribute is not allowed:
     *     - SdpAttribute::RECVONLY
     *     - SdpAttribute::SENDRECV
     *     - SdpAttribute::SENDONLY
     *     - SdpAttribute::SETUP
     *     - SdpAttribute::CONNECTION
     *
     * @param nType Attribute type; SdpAttribute::PTIME, ... in SdpAttribute.h
     * @param strType Name of unknown attribute type
     * @return Attribute value or null if not present.
     */
    virtual IMSList<AString> GetAttributes(
            IN IMS_SINT32 nType, IN const AString& strType = AString::ConstNull()) const = 0;

    /**
     * @brief Returns the specified attribute (a=) for the IMedia.
     *
     * If there is no attribute, INVALID_VALUE will be returned.
     *
     * NOTE:
     * The following attribute is allowed:
     *     - SdpAttribute::SETUP (SETUP_ACTIVE, ... in SDP.h)
     *     - SdpAttribute::CONNECTION (CONNECTION_NEW, ... in SDP.h)
     *
     * The following attribute is not allowed:
     *     - SdpAttribute::RECVONLY
     *     - SdpAttribute::SENDRECV
     *     - SdpAttribute::SENDONLY
     *
     * @param nType Attribute type; SdpAttribute::PTIME, ... in SdpAttribute.h
     * @param strType Name of unknown attribute type
     * @return Attribute value as integer or INVALID_VALUE if not present.
     */
    virtual IMS_SINT32 GetAttributeInt(
            IN IMS_SINT32 nType, IN const AString& strType = AString::ConstNull()) const = 0;

    /**
     * @brief Returns the proposed bandwidth (b=) to be used by the media.
     *
     * An INVALID_VALUE will be returned if the bandwidth information could not be retrieved.
     *
     * @param nType Bandwidth type; SdpBandwidth::TYPE_AS, ... in SdpBandwidth.h
     * @param strType Name of unknown bandwidth type
     * @return Bandwidth value as integer or INVALID_VALUE if not present.
     */
    virtual IMS_SINT32 GetBandwidth(
            IN IMS_SINT32 nType, IN const AString& strType = AString::ConstNull()) const = 0;

    /**
     * @brief Returns the direction of the media.
     *
     * If the direction attribute is not present, DIRECTION_NONE will be returned.
     *
     * @return Current media direction. (DIRECTION_RECVONLY, ... in SDP.h)
     */
    virtual IMS_SINT32 GetDirection() const = 0;

    /**
     * @brief Returns the contents of the media description field (m=) of
     *       the current incoming SDP for this media.
     *
     * @return Pointer to SdpMedia object
     */
    virtual const SdpMedia* GetMediaDescriptionEx() const = 0;

    /**
     * @brief Returns the media formats (rtpmap/fmtp/...) of the media.
     *
     * @return List of SdpMediaFormat object.
     */
    virtual const IMSList<SdpMediaFormat*>& GetMediaFormats() const = 0;

    /**
     * @brief Removes an attribute (a=) form the IMedia.
     *
     * NOTE: If the IMedia is in STATE_ACTIVE, the attribute will be removed on the proposal media
     * until the ISession has been updated.\n The proposal media can be retrieved with the
     * GetProposal() method on IMedia interface.
     *
     * The following exception can be throwed,
     *     - ILLEGAL_ARGUMENT
     *         - if the attribute argument is invalid
     *     - ILLEGAL_STATE
     *         - if the IMedia is not in STATE_INACTIVE or STATE_ACTIVE state
     *     - GENERAL_ERROR
     *         - if the internal error occurred
     *
     * @param objAttribute Attribute to be removed
     * @return If it succeeds, return IMS_SUCCESS. Otherwise, returns IMS_FAILURE.
     */
    virtual IMS_RESULT RemoveAttribute(IN const SdpAttribute& objAttribute) = 0;

    /**
     * @brief Removes an attribute (a=) form the IMedia.
     *
     * If nType is ATTRIBUTE_ALL, then all the attributes will be removed except for :
     *     - sendrecv, sendonly, recvonly, setup, connection, qos, mid
     *
     * NOTE: If the IMedia is in STATE_ACTIVE, the attribute will be removed on the proposal media
     * until the ISession has been updated.\n The proposal media can be retrieved with the
     * GetProposal() method on IMedia interface.
     *
     * The following exception can be throwed,
     *     - ILLEGAL_ARGUMENT
     *         - if the attribute argument is invalid
     *     - ILLEGAL_STATE
     *         - if the IMedia is not in STATE_INACTIVE or STATE_ACTIVE state
     *     - GENERAL_ERROR
     *         - if the internal error occurred
     *
     * @param nType Attribute type; SdpAttribute::PTIME, ... in SdpAttribute.h
     * @param strAttrValue Attribute value to be set (string value only)
     * @param strType Name of unknown attribute type
     * @return If it succeeds, return IMS_SUCCESS. Otherwise, returns IMS_FAILURE.
     */
    virtual IMS_RESULT RemoveAttribute(IN IMS_SINT32 nType,
            IN const AString& strAttrValue = AString::ConstNull(),
            IN const AString& strType = AString::ConstNull()) = 0;

    /**
     * @brief Returns the specified media formats (rtpmap/fmtp/...) of the media.
     *
     * The following exception can be throwed,
     *     - ILLEGAL_ARGUMENT
     *         - if the nType or strValue argument is invalid
     *     - ILLEGAL_STATE
     *         - if the IMedia is not in STATE_INACTIVE or STATE_ACTIVE state
     *     - GENERAL_ERROR
     *         - if the internal error occurred
     *
     * @param nType Media format type; SdpMediaFormat::TYPE_RTP, ... SdpMediaFormat.h
     * @param strValue Value of media format (payload type, '*', ...)
     * @return If it succeeds, return IMS_SUCCESS. Otherwise, returns IMS_FAILURE.
     */
    virtual IMS_RESULT RemoveMediaFormat(IN IMS_SINT32 nType, IN const AString& strValue) = 0;

    /**
     * @brief Sets the connection address of the media.
     *
     * The following exception can be throwed,
     *     - ILLEGAL_ARGUMENT
     *         - if the strAddress argument is invalid
     *     - ILLEGAL_STATE
     *         - if the IMedia is not in STATE_INACTIVE or STATE_ACTIVE state
     *     - GENERAL_ERROR
     *         - if the internal error occurred
     *
     * @param strAddress Numeric IP address
     * @return If it succeeds, return IMS_SUCCESS. Otherwise, returns IMS_FAILURE.
     */
    virtual IMS_RESULT SetConnectionAddress(IN const AString& strAddress) = 0;

    /**
     * @brief Sets the direction of the media.
     *
     * The following exception can be throwed,
     *     - ILLEGAL_ARGUMENT
     *         - if the nDirection argument is invalid
     *     - ILLEGAL_STATE
     *         - if the IMedia is not in STATE_INACTIVE or STATE_ACTIVE state
     *     - GENERAL_ERROR
     *         - if the internal error occurred
     *
     * @param nDirection DIRECTION_RECVONLY, ... in SDP.h
     * @return If it succeeds, return IMS_SUCCESS. Otherwise, returns IMS_FAILURE.
     */
    virtual IMS_RESULT SetDirection(IN IMS_SINT32 nDirection) = 0;

    /**
     * @brief Sets the contents of the media description field (m=) of the current SDP
     *       for this media.
     *
     * The following exception can be throwed,
     *     - ILLEGAL_ARGUMENT
     *         - if the media description argument is invalid
     *     - ILLEGAL_STATE
     *         - if the IMedia is not in STATE_INACTIVE or STATE_ACTIVE state
     *     - GENERAL_ERROR
     *         - if the internal error occurred
     *
     * @param nType Media format type; SdpMedia::TYPE_AUDIO, ... SdpMedia.h
     * @param nPort Port number
     * @param nTransportProtocol Transport protocol type;
     *                           SdpMedia::TRANSPORT_RTP_AVP, ... SdpMedia.h
     * @param objFormats List of formats
     * @return If it succeeds, return IMS_SUCCESS. Otherwise, returns IMS_FAILURE.
     */
    virtual IMS_RESULT SetMediaDescription(IN IMS_SINT32 nType, IN IMS_SINT32 nPort,
            IN IMS_SINT32 nTransportProtocol, IN const AStringArray& objFormats) = 0;

    /**
     * @brief This method will set/modify the format for this media.
     *
     * Currently, it only supports the media types, audio & video (RTP/AVP).
     *
     * The following exception can be throwed,
     *     - ILLEGAL_ARGUMENT
     *         - if the media format argument is invalid
     *     - ILLEGAL_STATE
     *         - if the IMedia is not in STATE_INACTIVE or STATE_ACTIVE state
     *     - GENERAL_ERROR
     *         - if the internal error occurred
     *
     * @param pMediaFormat Pointer to SdpMediaFormat; if rtpmap, then SdpAvCodec
     * @return If it succeeds, return IMS_SUCCESS. Otherwise, returns IMS_FAILURE.
     */
    virtual IMS_RESULT SetMediaFormat(IN const SdpMediaFormat* pMediaFormat) = 0;

    /**
     * @brief This method will set/modify the format for this media.
     *
     * Currently, it only supports the media types, audio & video (RTP/AVP).
     *
     * The following exception can be throwed,
     *     - ILLEGAL_ARGUMENT
     *         - if the media format argument is invalid
     *     - ILLEGAL_STATE
     *         - if the IMedia is not in STATE_INACTIVE or STATE_ACTIVE state
     *     - GENERAL_ERROR
     *         - if the internal error occurred
     *
     * @param nType Media format type; SdpMediaFormat::TYPE_RTP, ... SdpMediaFormat.h
     * @param strValue Value of media format (payload type, '*', ...)
     * @param strAnyMap Any MAP string for this media (rtpmap, ...) excluding the format\n
     *                  ("a=rtpmap:97 AMR/8000" -> "AMR/8000")
     * @param strFmtp Format specific parameter for this media (fmtp) excluding the format\n
     *                ("a=fmtp:97 mode-set=12" -> "mode-set=12")
     * @return If it succeeds, return IMS_SUCCESS. Otherwise, returns IMS_FAILURE.
     */
    virtual IMS_RESULT SetMediaFormat(IN IMS_SINT32 nType, IN const AString& strValue,
            IN const AString& strAnyMap, IN const AString& strFmtp) = 0;

    /**
     * @brief Sets the port number of the media description field (m=) of the current SDP
     *        for this media.
     *
     * @param nPort Port number
     * @return If it succeeds, return IMS_SUCCESS. Otherwise, returns IMS_FAILURE.
     */
    virtual IMS_RESULT SetPort(IN IMS_SINT32 nPort) = 0;

    /**
     * @brief Returns the contents of the media description field (m=) of the current SDP view
     *        for this media.
     *
     * It returns the value of a local current view instead of remote peer view.
     *
     * @return Pointer to SdpMedia object.
     */
    virtual const SdpMedia* GetMediaDescriptionExAsLocal() const = 0;

    /**
     * @brief Returns the IP address of the local endpoint.
     *
     * @return IP address of the local endpoint.
     */
    virtual IpAddress GetLocalAddress() const = 0;

    /**
     * @brief Returns the port number of the local endpoint.
     *
     * @return Port number of the local endpoint.
     */
    virtual IMS_SINT32 GetLocalPort() const = 0;

    /**
     * @brief Returns the IP address of the remote endpoint.
     *
     * It will return the media-level connection info. if it is present.
     *
     * @return IP address of the remote endpoint.
     */
    virtual IpAddress GetRemoteAddress() const = 0;

    /**
     * @brief Returns the connection address of the remote endpoint as string.
     *
     * It will return the media-level connection info. if it is present.
     *
     * @return Connection address of the remote endpoint.
     */
    virtual const AString& GetRemoteAddressAsString() const = 0;

    /**
     * @brief Returns the port number of the remote endpoint.
     *
     * @return Port number of the remote endpoint.
     */
    virtual IMS_SINT32 GetRemotePort() const = 0;

    /**
     * @brief Returns the specified precondition from the peer media parameter.
     *
     * @param nAttribute Type of attribute (CURR, DES, CONF in SdpAttribute.h)
     * @param nType Type of precondition (qos, ...)
     * @return Pointer to SdpPrecondition.
     * @note IMS_SDP_PRECONDITION
     */
    virtual const SdpPrecondition* GetPrecondition(
            IN IMS_SINT32 nAttribute, IN IMS_SINT32 nType = SdpPrecondition::TYPE_QOS) const = 0;

    /**
     * @brief Removes the specified precondition from the local media parameter.
     *
     * @param nAttribute Type of attribute (CURR, DES, CONF in SdpAttribute.h)
     * @param nType Type of precondition (qos, ...)
     * @return If it succeeds, return IMS_SUCCESS. Otherwise, returns IMS_FAILURE.
     * @note IMS_SDP_PRECONDITION
     */
    virtual IMS_RESULT RemovePrecondition(
            IN IMS_SINT32 nAttribute, IN IMS_SINT32 nType = SdpPrecondition::TYPE_QOS) = 0;

    /**
     * @brief Sets the specified precondition to the local media parameter.
     *
     * @param nAttribute Type of attribute (CURR, DES, CONF in SdpAttribute.h)
     * @param pPrecondition Precondition to be set
     * @return If it succeeds, return IMS_SUCCESS. Otherwise, returns IMS_FAILURE.
     * @note IMS_SDP_PRECONDITION
     */
    virtual IMS_RESULT SetPrecondition(
            IN IMS_SINT32 nAttribute, IN const SdpPrecondition* pPrecondition) = 0;

public:
    /// Return value of integer value
    enum
    {
        INVALID_VALUE = (-1)
    };
};

#endif
