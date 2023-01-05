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
#ifndef INTERFACE_MESSAGE_H_
#define INTERFACE_MESSAGE_H_

#include "SipMethod.h"

class IMessageBodyPart;
class ISipMessage;

/**
 * @brief This class provides functionality to manipulate headers and body parts
 *        of SIP messages.
 *
 * @see IServiceMethod, IMessageBodyPart, ISipMessage
 */
class IMessage
{
protected:
    virtual ~IMessage() = default;

public:
    /**
     * @brief Adds a header value, either on a new header or appending a new value
     *        to an already existing header.
     *
     * @param strName The header name, in full or compact form
     * @param strValue The header value
     * @return If it succeeds, returns IMS_SUCCESS. Otherwise, returns IMS_FAILURE.
     */
    virtual IMS_RESULT AddHeader(IN const AString& strName, IN const AString& strValue) = 0;

    /**
     * @brief Creates a new IMessageBodyPart and adds it to the message.
     *
     * @return Pointer to new IMessageBodyPart.
     */
    virtual IMessageBodyPart* CreateBodyPart() = 0;

    /**
     * @brief Returns all body parts that are added to the message.
     *
     * @return List of pointer to IMessageBodyPart.
     */
    virtual IMSList<IMessageBodyPart*> GetBodyParts() const = 0;

    /**
     * @brief Returns the value(s) of a header in this message.
     *
     * @param strName The header name, in full or compact form
     * @return List of SIP header field(s).
     */
    virtual IMSList<AString> GetHeaders(IN const AString& strName) const = 0;

    /**
     * @brief Returns the SIP message for this IMessage.
     *
     * @return Pointer to ISipMessage.
     */
    virtual ISipMessage* GetMessage() const = 0;

    /**
     * @brief Returns the SIP method for this IMessage.
     *
     * @return Reference to SipMethod.
     */
    virtual const SipMethod& GetMethod() const = 0;

    /**
     * @brief Returns the reason phrase of the response.
     *
     * @return The reason phrase, or null if the reason phrase is not available.
     */
    virtual const AString& GetReasonPhrase() const = 0;

    /**
     * @brief Returns the current state of this IMessage.
     *
     * @return The current state of this message.\n
     *         #STATE_UNSENT\n
     *         #STATE_SENT\n
     *         #STATE_RECEIVED
     */
    virtual IMS_SINT32 GetState() const = 0;

    /**
     * @brief Returns the status code of the response.
     *
     * @return The status code, or 0 if the status code is not available.
     */
    virtual IMS_SINT32 GetStatusCode() const = 0;

public:
    /// Identifier of IMessage for each IServiceMethod
    enum
    {
        SERVICEMETHOD_INVALID = 0,
        CAPABILITIES_QUERY = 1,         ///< ICapabilities, OPTIONS
        PAGEMESSAGE_SEND = 2,           ///< IPageMessage, MESSAGE
        PUBLICATION_PUBLISH = 3,        ///< IPublication, PUBLISH
        PUBLICATION_UNPUBLISH = 4,      ///< IPublication, PUBLISH
        REFERENCE_REFER = 5,            ///< IReference, REFER
        SESSION_START = 6,              ///< ISession, INVITE
        SESSION_UPDATE = 7,             ///< ISession, INVITE or UPDATE
        SESSION_TERMINATE = 8,          ///< ISession, BYE
        SUBSCRIPTION_SUBSCRIBE = 9,     ///< ISubscription, SUBSCRIBE
        SUBSCRIPTION_UNSUBSCRIBE = 10,  ///< ISubscription, SUBSCRIBE
        SUBSCRIPTION_POLL = 11,         ///< ISubscription, SUBSCRIBE

        // IMS extension
        SESSION_ACK = 12,      ///< ISession, ACK
        SESSION_PRACK,         ///< ISession, PRACK
        SESSION_EARLY_UPDATE,  ///< ISession, early UPDATE
        /// CANCEL (re-INVITE only) operation during an active call
        SESSION_CANCEL,  ///< ISession, CANCEL
        /// RACE_CONDITION : SESSION_UPDATE (200 OK to re-INVITE & incoming re-INVITE request)
        /// To store the previous SESSION_UPDATE message before setting a new request
        SESSION_STALE_UPDATE,  ///< ISession, stale UPDATE

        SERVICEMETHOD_MAX
    };

    /// Type of IMessage's state
    enum
    {
        /// When message is not sent
        STATE_UNSENT = 1,
        /// When message is sent
        STATE_SENT = 2,
        /// When message is received
        STATE_RECEIVED = 3
    };
};

#endif
