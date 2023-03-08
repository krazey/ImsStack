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
#ifndef INTERFACE_SESSION_DESCRIPTOR_H_
#define INTERFACE_SESSION_DESCRIPTOR_H_

#include "IpAddress.h"

#include "SdpAttribute.h"

/**
 * @brief This class provides an interface to the session-level part of incoming/outgoing SDP.
 *
 * On the originating endpoint, it is most useful for the application to use the setters
 * when the session is in the initiated state. The changes will then be in effect from
 * the start of the session. It is not possible to use the getters since there is no incoming SDP
 * at the first state. In an established session, the originating endpoint for a session update
 * may read and change the SDP before applying the update.
 *
 * On the terminating endpoint, the application can read the incoming SDP, but not set
 * an outgoing SDP in the ICoreService::SessionInvitationReceived() callback.
 * The reason is that all SDP carrying messages have already been exchanged
 * at the time of callback. If the terminating side wants to add attributes, it has to do so
 * when the session is established, and trigger a session update.
 *
 * NOTE:\n
 * The getters read from the last incoming SDP. The setters apply to all outgoing SDPs
 * in the current session, beginning with the one to be sent next.
 * Note that using the interface setters do not in itself trigger the IMS engine to send the SDP
 * in a SIP message. If there is a point in the session lifetime where the modifications
 * should cease, the application is responsible to use the setters again,
 * and make sure the SDP is sent.
 *
 * RESERVED ATTRIBUTES :\n
 * charset, charset:iso8895-1, group, maxprate, ice-lite, ice-mismatch, ice-options, ice-pwd,\n
 * ice-ufrag, inactive, sendonly, recvonly, sendrecv, csup, creq, acap, tcap
 */
class ISessionDescriptor
{
protected:
    virtual ~ISessionDescriptor() = default;

public:
    /**
     * @brief Adds an attribute (a=) to the ISession.
     *
     * Adding attributes that the IMS engine can set,
     * such as "sendonly", "recvonly", "sendrecv" will lead to an ILLEGAL_ARGUMENT error.
     *
     * For example, AddAttribute("type:videolive"); -> "a=type:videolive".
     *
     * The following exception can be throwed,
     *     - ILLEGAL_ARGUMENT
     *         - if the strAttribute argument is null
     *         - if the syntax of the strAttribute argument is invalid
     *         - if the strAttribute already exists in the ISession
     *         - if the strAttribute is reserved for the IMS engine
     *     - ILLEGAL_STATE
     *         - if the ISession is not in STATE_INITIATED or STATE_ESTABLISHED state
     *     - GENERAL_ERROR
     *         - if the internal error occurred
     *
     * @param strAttribute Attribute to be added
     * @return If the attribute is successfully set, returns IMS_SUCCESS.
     *         Otherwise, returns IMS_FAILURE.
     */
    virtual IMS_RESULT AddAttribute(IN const AString& strAttribute) = 0;

    /**
     * @brief Returns all attributes (a=) for the ISession.
     *
     * If there are no attributes, an empty string array will be returned.
     *
     * @return List of all attribute values.
     */
    virtual ImsList<AString> GetAttributes() const = 0;

    /**
     * @brief Returns the version(v=) of the SDP.
     *
     * @return Protocol version or null if the protocol version could not be retrieved.
     */
    virtual AString GetProtocolVersion() const = 0;

    /**
     * @brief Returns a unique identifier (o=) for the session.
     *
     * @return Session identifier or null if the session identifier could not be retrieved.
     */
    virtual const AString& GetSessionId() const = 0;

    /**
     * @brief Returns the textual information (i=) about the session.
     *
     * @return Session information or null if the session information could not be retrieved.
     */
    virtual AString GetSessionInfo() const = 0;

    /**
     * @brief Returns the textual session name (s=).
     *
     * @return Session name or null if the session name is not set.
     */
    virtual AString GetSessionName() const = 0;

    /**
     * @brief Removes an attribute (a=) from the session.
     *
     * For example, RemoveAttribute("type:videolive"); -> "a=type:videolive" will be removed.
     *
     * The following exception can be throwed,
     *     - ILLEGAL_ARGUMENT
     *         - if the strAttribute argument is null
     *         - if the syntax of the strAttribute argument is invalid
     *         - if the strAttribute does not exist in the ISession
     *         - if the strAttribute is reserved for the IMS engine
     *     - ILLEGAL_STATE
     *         - if the ISession is not in STATE_INITIATED or STATE_ESTABLISHED state
     *     - GENERAL_ERROR
     *         - if the internal error occurred
     *
     * @param strAttribute Attribute to be removed
     * @return If the attribute is successfully removed, returns IMS_SUCCESS.
     *         Otherwise, returns IMS_FAILURE.
     */
    virtual IMS_RESULT RemoveAttribute(IN const AString& strAttribute) = 0;

    /**
     * @brief Sets the textual information (i=) about the session.
     *
     * The following exception can be throwed,
     *     - ILLEGAL_ARGUMENT
     *         - if the strInfo argument is null
     *           or if the syntax of the strInfo argument is invalid
     *     - ILLEGAL_STATE
     *         - if the ISession is not in STATE_INITIATED state
     *     - GENERAL_ERROR
     *         - if the internal error occurred
     *
     * @param strInfo Session information
     * @return If the session information is successfully set, returns IMS_SUCCESS.
     *         Otherwise, returns IMS_FAILURE.
     */
    virtual IMS_RESULT SetSessionInfo(IN const AString& strInfo) = 0;

    /**
     * @brief Sets the name of the session (s=).
     *
     * The following exception can be throwed,
     *     - ILLEGAL_ARGUMENT
     *         - if the strName argument is null
     *           or if the syntax of the strName argument is invalid
     *     - ILLEGAL_STATE
     *         - if the ISession is not in STATE_INITIATED state
     *     - GENERAL_ERROR
     *         - if the internal error occurred
     *
     * @param strName Session name
     * @return If the session name is successfully set, returns IMS_SUCCESS.
     *         Otherwise, returns IMS_FAILURE.
     */
    virtual IMS_RESULT SetSessionName(IN const AString& strName) = 0;

    /**
     * @brief Adds an attribute (a=) to the ISession.
     *
     * For example, AddAttribute(SdpAttribute::MAX_SIZE, "4096"); -> "a=max-size:4096".
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
     *         - if the ISession is not in STATE_INITIATED or STATE_ESTABLISHED state
     *     - GENERAL_ERROR
     *         - if the internal error occurred
     *
     * @param nType Attribute type; SdpAttribute::PTIME, ... in SdpAttribute.h
     * @param strAttrValue Attribute value to be set (string value only)
     * @param strType Name of unknown attribute type
     * @return If the attribute is successfully added, returns IMS_SUCCESS.
     *         Otherwise, returns IMS_FAILURE.
     */
    virtual IMS_RESULT AddAttribute(IN IMS_SINT32 nType, IN const AString& strAttrValue,
            IN const AString& strType = AString::ConstNull()) = 0;

    /**
     * @brief Adds an attribute (a=) to the ISession.
     *
     * For example,\n
     *     AddAttributeInt(SdpAttribute::SETUP, Sdp::SETUP_ACTIVE); -> "a=setup:active".\n
     *     AddAttributeInt(SdpAttribute::MAX_SIZE, 4096); -> "a=max-size:4096".
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
     *         - if the ISession is not in STATE_INITIATED or STATE_ESTABLISHED state
     *     - GENERAL_ERROR
     *         - if the internal error occurred
     *
     * @param nType Attribute type; SdpAttribute::PTIME, ... in SdpAttribute.h
     * @param nAttrValue Attribute value to be set (integer value only)
     * @param strType Name of unknown attribute type
     * @return If the attribute is successfully added, returns IMS_SUCCESS.
     *         Otherwise, returns IMS_FAILURE.
     */
    virtual IMS_RESULT AddAttributeInt(IN IMS_SINT32 nType, IN IMS_SINT32 nAttrValue,
            IN const AString& strType = AString::ConstNull()) = 0;

    /**
     * @brief Sets the proposed bandwidth (b=) to be used by the session.
     *
     * For example, AddBandwidth(SdpBandwidth::AS, 128); -> "b=AS:128".
     *
     * The following exception can be throwed,
     *     - ILLEGAL_ARGUMENT
     *         - if the nType or nBandwidth argument is invalid
     *     - ILLEGAL_STATE
     *         - if the ISession is not in STATE_INITIATED or STATE_ESTABLISHED state
     *     - GENERAL_ERROR
     *         - if the internal error occurred
     *
     * @param nType Bandwidth type; SdpBandwidth::TYPE_AS, ... in SdpBandwidth.h
     * @param nBandwidth Bandwidth value to be set
     * @param strType Name of unknown bandwidth type
     * @return If the bandwidth is successfully added, returns IMS_SUCCESS.
     *         Otherwise, returns IMS_FAILURE.
     */
    virtual IMS_RESULT AddBandwidth(IN IMS_SINT32 nType, IN IMS_SINT32 nBandwidth,
            IN const AString& strType = AString::ConstNull()) = 0;

    /**
     * @brief Returns the specified attribute (a=) for the ISession.
     *
     * If there is no attribute, a null string will be returned.
     *
     * NOTE:\n
     * The following attribute is not allowed:
     *     - SdpAttribute::RECVONLY
     *     - SdpAttribute::SENDRECV
     *     - SdpAttribute::SENDONLY
     *     - SdpAttribute::SETUP
     *     - SdpAttribute::CONNECTION
     *
     * The following exception can be throwed,
     *     - ILLEGAL_ARGUMENT
     *         - if the nType or nBandwidth argument is invalid
     *     - ILLEGAL_STATE
     *         - if the ISession is not in STATE_INITIATED or STATE_ESTABLISHED state
     *     - GENERAL_ERROR
     *         - if the internal error occurred
     *
     * @param nType Attribute type; SdpAttribute::PTIME, ... in SdpAttribute.h
     * @param strType Name of unknown attribute type
     * @return If the attribute is successfully retrieved, returns IMS_SUCCESS.
     *         Otherwise, returns IMS_FAILURE.
     */
    virtual const AString& GetAttribute(
            IN IMS_SINT32 nType, IN const AString& strType = AString::ConstNull()) const = 0;

    /**
     * @brief Returns the specified attribute (a=) for the ISession.
     *
     * If there is no attribute, INVALID_VALUE will be returned.
     *
     * NOTE:\n
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
     * @brief Returns the direction of the session.
     *
     * If the direction attribute is not present, DIRECTION_NONE will be returned.
     *
     * @return The current direction of session (DIRECTION_RECVONLY, ... in SDP.h).
     */
    virtual IMS_SINT32 GetDirection() const = 0;

    /**
     * @brief Returns a session version field (o=) for the session.
     *
     * @return Session version or null if the session version could not be retrieved.
     */
    virtual const AString& GetSessionVersion() const = 0;

    /**
     * @brief Returns a username field (o=) for the session.
     *
     * @return username field in o-line.
     */
    virtual const AString& GetUsername() const = 0;

    /**
     * @brief Removes an attribute (a=) from the ISession.
     *
     * The following exception can be throwed,
     *     - ILLEGAL_ARGUMENT
     *         - if the attribute argument is invalid
     *     - ILLEGAL_STATE
     *         - if the ISession is not in STATE_INITIATED or STATE_ESTABLISHED state
     *     - GENERAL_ERROR
     *         - if the internal error occurred
     *
     * @param objAttribute Attribute to be removed
     * @return If the attribute is successfully removed, returns IMS_SUCCESS.
     *         Otherwise, returns IMS_FAILURE.
     */
    virtual IMS_RESULT RemoveAttribute(IN const SdpAttribute& objAttribute) = 0;

    /**
     * @brief Removes an attribute (a=) from the ISession.
     *
     * The following exception can be throwed,
     *     - ILLEGAL_ARGUMENT
     *         - if the attribute argument is invalid
     *     - ILLEGAL_STATE
     *         - if the ISession is not in STATE_INITIATED or STATE_ESTABLISHED state
     *     - GENERAL_ERROR
     *         - if the internal error occurred
     *
     * @param nType Attribute type; SdpAttribute::PTIME, ... in SdpAttribute.h
     * @param strAttrValue Attribute value to be set (string value only)
     * @param strType Name of unknown attribute type
     * @return If the attribute is successfully removed, returns IMS_SUCCESS.
     *         Otherwise, returns IMS_FAILURE.
     */
    virtual IMS_RESULT RemoveAttribute(IN IMS_SINT32 nType,
            IN const AString& strAttrValue = AString::ConstNull(),
            IN const AString& strType = AString::ConstNull()) = 0;

    /**
     * @brief Removes all the bandwidths from the ISession.
     *
     * The following exception can be throwed,
     *     - ILLEGAL_STATE
     *         - if the ISession is not in STATE_INITIATED or STATE_ESTABLISHED state
     *     - GENERAL_ERROR
     *         - if the internal error occurred
     *
     * @return If all the bandwidths are successfully removed, returns IMS_SUCCESS.
     *         Otherwise, returns IMS_FAILURE.
     */
    virtual IMS_RESULT RemoveAllBandwidths() = 0;

    /**
     * @brief Sets the connection address of the session.
     *
     * The following exception can be throwed,
     *     - ILLEGAL_ARGUMENT
     *         - if the connection argument is invalid
     *     - ILLEGAL_STATE
     *         - if the ISession is not in STATE_INITIATED or STATE_ESTABLISHED state
     *     - GENERAL_ERROR
     *         - if the internal error occurred
     *
     * @param strAddress Connection address
     * @return If the connection is successfully set, returns IMS_SUCCESS.
     *         Otherwise, returns IMS_FAILURE.
     */
    virtual IMS_RESULT SetConnectionAddress(IN const AString& strAddress) = 0;

    /**
     * @brief Sets the direction of the session.
     *
     * The following exception can be throwed,
     *     - ILLEGAL_ARGUMENT
     *         - if the direction argument is invalid
     *     - ILLEGAL_STATE
     *         - if the ISession is not in STATE_INITIATED or STATE_ESTABLISHED state
     *     - GENERAL_ERROR
     *         - if the internal error occurred
     *
     * @param nDirection SDP direction
     * @return If the direction is successfully set, returns IMS_SUCCESS.
     *         Otherwise, returns IMS_FAILURE.
     */
    virtual IMS_RESULT SetDirection(IN IMS_SINT32 nDirection) = 0;

    /**
     * @brief Sets the address information of origin line.
     *
     * The following exception can be throwed,
     *     - ILLEGAL_ARGUMENT
     *         - if the address argument is invalid
     *     - ILLEGAL_STATE
     *         - if the ISession is not in STATE_INITIATED or STATE_ESTABLISHED state
     *     - GENERAL_ERROR
     *         - if the internal error occurred
     *
     * @param strAddress Numeric IP address
     * @return If the address is successfully set, returns IMS_SUCCESS.
     *         Otherwise, returns IMS_FAILURE.
     */
    virtual IMS_RESULT SetOriginAddress(IN const AString& strAddress) = 0;

    /**
     * @brief Returns the IP address of the local endpoint.
     *
     * @return IP address of the local endpoint.
     */
    virtual IpAddress GetLocalAddress() const = 0;

    /**
     * @brief Returns the IP address of the remote endpoint.
     *
     * @return IP address of the remote endpoint.
     */
    virtual IpAddress GetRemoteAddress() const = 0;

    /**
     * @brief Returns the connection address of the remote endpoint as string.
     *
     * @return Connection address of the remote endpoint.
     */
    virtual const AString& GetRemoteAddressAsString() const = 0;

public:
    /// Return value of integer value
    enum
    {
        INVALID_VALUE = (-1)
    };
};

#endif
