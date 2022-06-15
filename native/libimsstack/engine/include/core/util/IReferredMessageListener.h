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
#ifndef INTERFACE_REFERRED_MESSAGE_LISTENER_H_
#define INTERFACE_REFERRED_MESSAGE_LISTENER_H_

#include "SubState.h"

class ISipMessage;

class IReferredMessageListener
{
public:
    /**
     * @brief Notifies the SIP response message to the remote endpoint with the substate, "active".
     *
     * @param piSipMsg The SIP message to be notified
     */
    virtual void ReferredMessage_NotifyOnActive(IN ISipMessage* piSipMsg) = 0;

    /**
     * @brief Notifies the SIP response message to the remote endpoint with the substate,
     *        "terminated".
     *
     * If the reason code is not REASON_NONE, the reason parameter will be included
     * in the NOTIFY request.
     *
     * @param nReasonCode The reason code, defined in SubState.h
     * @param piSipMsg The SIP message to be notified
     */
    virtual void ReferredMessage_NotifyOnTerminated(
            IN IMS_SINT32 nReasonCode = SubState::REASON_NONE,
            IN ISipMessage* piSipMsg = IMS_NULL) = 0;
};

#endif
