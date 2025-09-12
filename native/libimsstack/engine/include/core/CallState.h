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
#ifndef CALL_STATE_H_
#define CALL_STATE_H_

#include "ImsTypeDef.h"

class ISipMessage;

class CallState
{
public:
    CallState();
    ~CallState() = default;

    CallState(IN const CallState&) = delete;
    CallState& operator=(IN const CallState&) = delete;

public:
    inline IMS_SINT32 GetState() const { return m_nState; }
    IMS_BOOL UpdateState(IN const ISipMessage* piSipMsg, IN IMS_SINT32 nMode);

private:
    static void InitializeStateTable();
    static void PrintStateChanged(
            IN const ISipMessage* piSipMsg, IN IMS_SINT32 nState, IN IMS_SINT32 nNextState);
    static IMS_SINT32 TranslateMessage(IN const ISipMessage* piSipMsg);

public:
    /// Internal states for tracking the session state
    enum
    {
        STATE_INVALID = 0,

        /// Idle
        STATE_IDLE = 1,
        /// INVITE sent
        STATE_INVITE_SENT,
        /// Provisional response received to INVITE sent
        STATE_INVITE_1XX_RECEIVED,
        /// Successful final response received to INVITE sent
        STATE_INVITE_2XX_RECEIVED,
        /// Failure final response received to INVITE sent
        STATE_INVITE_NON2XX_RECEIVED,
        /// INVITE received
        STATE_INVITE_RECEIVED,
        /// Provisional response sent to INVITE received
        STATE_INVITE_1XX_SENT,
        /// Successful final response sent to INVITE received
        STATE_INVITE_2XX_SENT,
        /// Failure final response sent to INVITE received
        STATE_INVITE_NON2XX_SENT,
        /// re-INVITE sent
        STATE_REINVITE_SENT,
        /// Provisional response received to re-INVITE sent
        STATE_REINVITE_1XX_RECEIVED,
        /// Successful final response received to re-INVITE sent
        STATE_REINVITE_2XX_RECEIVED,
        /// Failure final response received to re-INVITE sent
        STATE_REINVITE_NON2XX_RECEIVED,
        /// re-INVITE received
        STATE_REINVITE_RECEIVED,
        /// Provisional response sent to re-INVITE received
        STATE_REINVITE_1XX_SENT,
        /// Successful final response sent to re-INVITE received
        STATE_REINVITE_2XX_SENT,
        /// Failure final response sent to re-INVITE received
        STATE_REINVITE_NON2XX_SENT,

        /// ACK received / sent
        STATE_ESTABLISHED,

        /// If CANCEL/BYE is received when a final response to INVITE has not been sent
        STATE_INVITE_CANCELLED,
        /// If CANCEL/BYE is received when a final response to re-INVITE has not been sent
        STATE_REINVITE_CANCELLED,
        /// BYE sent
        STATE_BYE_SENT,
        /// BYE received
        STATE_BYE_RECEIVED,
        /// 23: Final response received/sent for BYE request
        STATE_TERMINATED,

        STATE_MAX
    };

    /// Message mode used to determine whether the state is being changed
    /// on receipt/sending of a SIP message.
    enum
    {
        /// Message is being sent out
        MODE_SENT,
        /// Message was just received for this session
        MODE_RECEIVED,
        /// CANCEL received before INVITE was completed
        MODE_CANCEL_RECEIVED,
        /// BYE received before INVITE was completed
        MODE_BYE_RECEIVED
    };

private:
    /// Trigger evenst for call state transition
    enum
    {
        MESSAGE_INVALID = (-1),

        MESSAGE_INVITE = 0,
        MESSAGE_CANCEL,
        MESSAGE_ACK,
        MESSAGE_BYE,
        MESSAGE_1XX,
        MESSAGE_2XX,
        MESSAGE_NON2XX,

        MESSAGE_MAX
    };

    static IMS_SINT32 s_nStateOnSent[STATE_MAX][MESSAGE_MAX];
    static IMS_SINT32 s_nStateOnReceived[STATE_MAX][MESSAGE_MAX];

    IMS_SINT32 m_nState;
};

#endif
