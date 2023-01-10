/*
 * Copyright (C) 2023 The Android Open Source Project
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
#ifndef INTERFACE_REASON_HEADER_SETTER_H_
#define INTERFACE_REASON_HEADER_SETTER_H_

#include "ImsTypeDef.h"

class ISipMessage;

/**
 * @brief This class provides a setter interface for SIP Reason header.
 */
class IReasonHeaderSetter
{
protected:
    virtual ~IReasonHeaderSetter() = default;

public:
    /**
     * @brief Sets the Reason header with the specified SIP message.
     *
     * @param piSipMsg The SIP message object
     * @param nTerminationReason The call termination reason\n
     *                           ISession#TERMINATION_REASON_UNKNOWN\n
     *                           ISession#TERMINATION_REASON_USER_ACTION\n
     *                           ISession#TERMINATION_REASON_REMOTE_ACTION\n
     *                           ISession#TERMINATION_REASON_REFRESH_408\n
     *                           ISession#TERMINATION_REASON_REFRESH_481\n
     *                           ISession#TERMINATION_REASON_REFRESH_TXN_TIMEOUT\n
     *                           ISession#TERMINATION_REASON_REFRESH_TIMEOUT\n
     *                           ISession#TERMINATION_REASON_SERVICE_CLOSED
     */
    virtual void ReasonHeaderSetter_SetHeader(
            IN ISipMessage* piSipMsg, IN IMS_SINT32 nTerminationReason) = 0;

    /**
     * @brief Sets a private(carrier-specific) Reason header to a new SIP message.
     *        If a private Reason header in the old SIP message is present, the setter should
     *        set this header to new SIP message. Otherwise, this method should do nothing.
     *
     * @param piOldSipMsg The old SIP message object
     * @param piNewSipMsg The new SIP message object
     */
    virtual void ReasonHeaderSetter_SetPrivateHeader(
            IN ISipMessage* piOldSipMsg, IN ISipMessage* piNewSipMsg) = 0;
};

#endif
