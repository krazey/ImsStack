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

enum class ResponseType
{
    PROVISIONAL_RESPONSE,
    PRACK_RESPONSE,
    EARLY_UPDATE_RESPONSE,
    ACCEPT,
    REJECT,
    ACCEPT_UPDATE,
};

/*
 * This class can update its status from the incoming messages.
 */
class IMtcMessageHandler
{
public:
    virtual ~IMtcMessageHandler() {}

    /**
     * Updates its state from the given request from the remote.
     */
    virtual void HandleRequest(IN RequestType eType, IN const IMessage& objRequest) = 0;

    /**
     * Updates its state from the given response from the remote.
     */
    virtual void HandleResponse(IN ResponseType eType, IN const IMessage& objResponse) = 0;
};

/*
 * This class formats outgoing requests and responses.
 */
class IMtcMessageFormatter
{
public:
    virtual ~IMtcMessageFormatter() {}

    /**
     * Formats the given request message before the request is sent to the remote.
     */
    virtual void FormatRequest(IN RequestType eType, IN_OUT IMessage& objRequest) = 0;

    /**
     * Formats the given response message before the response is sent to the remote.
     */
    virtual void FormatResponse(IN ResponseType eType, IN_OUT IMessage& objResponse) = 0;
};

#endif
