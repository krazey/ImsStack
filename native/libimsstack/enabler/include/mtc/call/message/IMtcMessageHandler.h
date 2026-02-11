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

#ifndef INTERFACE_MTC_MESSAGE_HANDLER_H_
#define INTERFACE_MTC_MESSAGE_HANDLER_H_

#include "ImsTypeDef.h"

class IMessage;

/**
 * Defines the types of SIP requests relevant to a call session.
 */
enum class RequestType
{
    START,
    PRACK,
    EARLY_UPDATE,
    ACK,
    UPDATE,
    CANCEL_UPDATE,
    TERMINATE,
};

/**
 * Defines the types of SIP responses relevant to a call session.
 */
enum class ResponseType
{
    PROVISIONAL_RESPONSE,
    PRACK_RESPONSE,
    EARLY_UPDATE_RESPONSE,
    ACCEPT,
    REJECT,
    ACCEPT_UPDATE,
};

/**
 * @brief Defines an interface for components that handle incoming SIP messages to update their
 *        internal state.
 *
 * Implementers of this interface can inspect incoming SIP requests and responses
 * to track call state, media parameters, or other session-related information.
 */
class IMtcMessageHandler
{
public:
    virtual ~IMtcMessageHandler() {}

    /**
     * @brief Handles an incoming request from the remote party.
     *
     * @param eType The type of the incoming request.
     * @param objRequest The incoming SIP request message.
     */
    virtual void HandleRequest(IN RequestType eType, IN const IMessage& objRequest) = 0;

    /**
     * @brief Handles an incoming response from the remote party.
     *
     * @param eType The type of the incoming response.
     * @param objResponse The incoming SIP response message.
     */
    virtual void HandleResponse(IN ResponseType eType, IN const IMessage& objResponse) = 0;
};

/**
 * @brief Defines an interface for components that format outgoing SIP messages.
 *
 * Implementers can add, remove, or modify headers and body parts of an outgoing SIP message before
 * it is sent to the network.
 */
class IMtcMessageFormatter
{
public:
    virtual ~IMtcMessageFormatter() {}

    /**
     * @brief Formats an outgoing request message before it is sent.
     *
     * @param eType The type of the outgoing request.
     * @param objRequest The outgoing SIP request message to be formatted.
     */
    virtual void FormatRequest(IN RequestType eType, IN_OUT IMessage& objRequest) = 0;

    /**
     * @brief Formats an outgoing response message before it is sent.
     *
     * @param eType The type of the outgoing response.
     * @param objResponse The outgoing SIP response message to be formatted.
     */
    virtual void FormatResponse(IN ResponseType eType, IN_OUT IMessage& objResponse) = 0;
};

#endif
