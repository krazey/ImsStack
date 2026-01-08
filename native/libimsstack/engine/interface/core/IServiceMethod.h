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
#ifndef INTERFACE_SERVICE_METHOD_H_
#define INTERFACE_SERVICE_METHOD_H_

#include "AString.h"

#include "base/IMethod.h"

class IMessage;

/**
 * @brief This class provides an interface to manipulate the next outgoing request message
 *        and to inspect previously sent request and response messages.
 *
 * The headers and body parts that are set will be transmitted
 * in the next message that is triggered by an interface method.
 *
 * @see IMessage
 */
class IServiceMethod : public IMethod
{
protected:
    ~IServiceMethod() override = default;

public:
    /**
     * @brief This method returns a handle to the next outgoing request Message
     *        within this ServiceMethod to be manipulated by the local endpoint.
     *
     * @return Pointer to IMessage that has a next outgoing request message.
     */
    virtual IMessage* GetNextRequest() = 0;

    /**
     * @brief This method returns a handle to the next outgoing response Message
     *        within this ServiceMethod to be manipulated by the local endpoint.
     *
     * @return Pointer to IMessage that has a next outgoing response message.
     */
    virtual IMessage* GetNextResponse() = 0;

    /**
     * @brief This method enables the user to inspect a previously sent or received
     *        request message.
     *
     * It is only possible to inspect the last request of each interface method identifier.\n
     * This method will return null if the interface method identifier has not been sent/received.
     *
     * @param nServiceMethod Type of service method that defined in IMessage\n
     *                       #IMessage#CAPABILITIES_QUERY\n
     *                       #IMessage#PAGEMESSAGE_SEND\n
     *                       #IMessage#PUBLICATION_PUBLISH\n
     *                       #IMessage#PUBLICATION_UNPUBLISH\n
     *                       #IMessage#REFERENCE_REFER\n
     *                       #IMessage#SESSION_START\n
     *                       #IMessage#SESSION_UPDATE\n
     *                       #IMessage#SESSION_TERMINATE\n
     *                       #IMessage#SUBSCRIPTION_SUBSCRIBE\n
     *                       #IMessage#SUBSCRIPTION_UNSUBSCRIBE\n
     *                       #IMessage#SUBSCRIPTION_POLL\n
     *                       #IMessage#SESSION_ACK\n
     *                       #IMessage#SESSION_PRACK\n
     *                       #IMessage#SESSION_EARLY_UPDATE\n
     *                       #IMessage#SESSION_CANCEL\n
     *                       #IMessage#SESSION_STALE_UPDATE
     * @return Pointer to IMessage that has the previous request message.
     */
    virtual IMessage* GetPreviousRequest(IN IMS_SINT32 nServiceMethod) const = 0;

    /**
     * @brief This method enables the user to inspect previously the most recent sent or received
     *        response messages.
     *
     * It is only possible to inspect the response for the last request
     * of each interface method identifier.\n
     * This method will return null if the interface method identifier has not been sent/received.
     *
     * @param nServiceMethod Type of service method that defined in IMessage\n
     *                       #IMessage#CAPABILITIES_QUERY\n
     *                       #IMessage#PAGEMESSAGE_SEND\n
     *                       #IMessage#PUBLICATION_PUBLISH\n
     *                       #IMessage#PUBLICATION_UNPUBLISH\n
     *                       #IMessage#REFERENCE_REFER\n
     *                       #IMessage#SESSION_START\n
     *                       #IMessage#SESSION_UPDATE\n
     *                       #IMessage#SESSION_TERMINATE\n
     *                       #IMessage#SUBSCRIPTION_SUBSCRIBE\n
     *                       #IMessage#SUBSCRIPTION_UNSUBSCRIBE\n
     *                       #IMessage#SUBSCRIPTION_POLL\n
     *                       #IMessage#SESSION_ACK\n
     *                       #IMessage#SESSION_PRACK\n
     *                       #IMessage#SESSION_EARLY_UPDATE\n
     *                       #IMessage#SESSION_CANCEL\n
     *                       #IMessage#SESSION_STALE_UPDATE
     * @return Pointer to IMessage that locates at the first position
     *         among the previous response messages.
     */
    virtual IMessage* GetPreviousResponse(IN IMS_SINT32 nServiceMethod) const = 0;

    /**
     * @brief This method enables the user to inspect previously sent or received
     *        response messages.
     *
     * It is only possible to inspect the response(s) for the last request
     * of each interface method identifier.\n
     * This method will return null if the interface method identifier has not been sent/received.
     *
     * @param nServiceMethod Type of service method that defined in IMessage\n
     *                       #IMessage#CAPABILITIES_QUERY\n
     *                       #IMessage#PAGEMESSAGE_SEND\n
     *                       #IMessage#PUBLICATION_PUBLISH\n
     *                       #IMessage#PUBLICATION_UNPUBLISH\n
     *                       #IMessage#REFERENCE_REFER\n
     *                       #IMessage#SESSION_START\n
     *                       #IMessage#SESSION_UPDATE\n
     *                       #IMessage#SESSION_TERMINATE\n
     *                       #IMessage#SUBSCRIPTION_SUBSCRIBE\n
     *                       #IMessage#SUBSCRIPTION_UNSUBSCRIBE\n
     *                       #IMessage#SUBSCRIPTION_POLL\n
     *                       #IMessage#SESSION_ACK\n
     *                       #IMessage#SESSION_PRACK\n
     *                       #IMessage#SESSION_EARLY_UPDATE\n
     *                       #IMessage#SESSION_CANCEL\n
     *                       #IMessage#SESSION_STALE_UPDATE
     * @return List of pointer to IMessage that has the previous response messages.
     */
    virtual ImsList<IMessage*> GetPreviousResponses(IN IMS_SINT32 nServiceMethod) const = 0;

    /**
     * @brief Returns the remote user identity of this IServiceMethod.
     *
     * NOTE:\n
     * It returns the trusted user identity or identities of the remote endpoint.
     * The value is an array of all P-Asserted-Identity header values (display name & URI)
     * that can be found in the SIP message exchanged with the remote in the order
     * they have been found.
     *
     * @return List of trusted user identity; P-Asserted-Identity header field.
     */
    virtual ImsList<AString> GetRemoteUserId() const = 0;
};

#endif
