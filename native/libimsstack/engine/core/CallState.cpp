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
#include "ServiceMemory.h"
#include "ServiceTrace.h"

#include "CallState.h"
#include "ISipHeader.h"
#include "ISipMessage.h"
#include "SipDebug.h"
#include "SipStatusCode.h"

__IMS_TRACE_TAG_IMS_CORE__;

// clang-format off
PUBLIC GLOBAL
IMS_SINT32 CallState::s_nStateOnSent[CallState::STATE_MAX][CallState::MESSAGE_MAX] =
{
    // STATE_INVALID
    {
        CallState::STATE_INVALID,
        CallState::STATE_INVALID,
        CallState::STATE_INVALID,
        CallState::STATE_INVALID,
        CallState::STATE_INVALID,
        CallState::STATE_INVALID,
        CallState::STATE_INVALID
    },
};

PUBLIC GLOBAL
IMS_SINT32 CallState::s_nStateOnReceived[CallState::STATE_MAX][CallState::MESSAGE_MAX] =
{
    // STATE_INVALID
    {
        CallState::STATE_INVALID,
        CallState::STATE_INVALID,
        CallState::STATE_INVALID,
        CallState::STATE_INVALID,
        CallState::STATE_INVALID,
        CallState::STATE_INVALID,
        CallState::STATE_INVALID
    },
};
// clang-format on

PUBLIC
CallState::CallState() :
        m_nState(STATE_IDLE)
{
    InitializeStateTable();
}

PUBLIC
IMS_BOOL CallState::UpdateState(IN const ISipMessage* piSipMsg, IN IMS_SINT32 nMode)
{
    // MSG_MODE_CANCEL_RECEIVED, MSG_MODE_BYE_RECEIVED
    IMS_SINT32 nSipMsg = TranslateMessage(piSipMsg);

    if (nSipMsg == MESSAGE_INVALID)
    {
        IMS_TRACE_I(
                "CALL_STATE - NO TRANSITION (%s)", piSipMsg->GetMethod().ToString().GetStr(), 0, 0);
        return IMS_TRUE;
    }

    /*
     * Responses to CANCEL should not modify the call state.
     * The final response to the INVITE causes the appropriate state transition.
     * And also, if response to BYE comes when the call state is not in BYE sent stage,
     * then it means the BYE is sent to cancel the INVITE.
     * So, don't update the call state in those cases.
     */
    if (piSipMsg->GetType() == ISipMessage::TYPE_RESPONSE)
    {
        const SipMethod& objMethod = piSipMsg->GetMethod();

        if (objMethod.Equals(SipMethod::CANCEL) ||
                (objMethod.Equals(SipMethod::BYE) && (m_nState != STATE_BYE_SENT) &&
                        (m_nState != STATE_BYE_RECEIVED)))
        {
            IMS_TRACE_D("NO STATE TRANSITION - RESPONSE TO CANCELLING the INVITE", 0, 0, 0);
            return IMS_TRUE;
        }
    }

    // If the message sent and the current state map to a valid state transition,
    // then change the state.
    if (nMode == MODE_SENT)
    {
        // When INVITE is received previously, can UAS send a CANCEL request to it ?

        if (s_nStateOnSent[m_nState][nSipMsg] != STATE_INVALID)
        {
            // TRACE
            PrintStateChanged(piSipMsg, m_nState, s_nStateOnSent[m_nState][nSipMsg]);

            // Update the state
            m_nState = s_nStateOnSent[m_nState][nSipMsg];

            // ACK request to non-2xx will be ignored by J180
            if (m_nState == STATE_REINVITE_NON2XX_SENT)
            {
                if (s_nStateOnReceived[m_nState][MESSAGE_ACK] != STATE_INVALID)
                {
                    // TRACE
                    PrintStateChanged(
                            piSipMsg, m_nState, s_nStateOnReceived[m_nState][MESSAGE_ACK]);

                    // Update the state
                    m_nState = s_nStateOnReceived[m_nState][MESSAGE_ACK];
                }
            }
        }
    }
    // If the message received and the current state map to a valid state transition,
    // then change the state.
    else if (nMode == MODE_RECEIVED)
    {
        // When INVITE is received previously, can UAS send a CANCEL request to it ?

        if (s_nStateOnReceived[m_nState][nSipMsg] != STATE_INVALID)
        {
            // TRACE
            PrintStateChanged(piSipMsg, m_nState, s_nStateOnReceived[m_nState][nSipMsg]);

            // Update the state
            m_nState = s_nStateOnReceived[m_nState][nSipMsg];

            // ACK request to non-2xx will be sent by J180 automatically
            if ((m_nState == STATE_INVITE_NON2XX_RECEIVED) ||
                    (m_nState == STATE_REINVITE_NON2XX_RECEIVED))
            {
                if (s_nStateOnSent[m_nState][MESSAGE_ACK] != STATE_INVALID)
                {
                    // TRACE
                    PrintStateChanged(piSipMsg, m_nState, s_nStateOnSent[m_nState][MESSAGE_ACK]);

                    // Update the state
                    m_nState = s_nStateOnSent[m_nState][MESSAGE_ACK];
                }
            }
        }
    }
    else
    {
        IMS_TRACE_E(0, "Message mode is invalid", 0, 0, 0);
    }

    return IMS_TRUE;
}

PRIVATE GLOBAL void CallState::InitializeStateTable()
{
    static IMS_BOOL bInitialized = IMS_FALSE;
    IMS_SINT32 i;
    IMS_SINT32 j;

    if (bInitialized)
    {
        return;
    }

    for (i = 0; i < STATE_MAX; ++i)
    {
        for (j = 0; j < MESSAGE_MAX; ++j)
        {
            s_nStateOnSent[i][j] = STATE_INVALID;
        }
    }

    for (i = 0; i < STATE_MAX; ++i)
    {
        for (j = 0; j < MESSAGE_MAX; ++j)
        {
            s_nStateOnReceived[i][j] = STATE_INVALID;
        }
    }

    ////////////////////////////////////////////////////////////
    //// Initialize the state machine for message sent

    // SENT : STATE_IDLE
    s_nStateOnSent[STATE_IDLE][MESSAGE_INVITE] = STATE_INVITE_SENT;

    // SENT : STATE_INVITE_SENT

    // SENT : STATE_INVITE_1XX_RECEIVED
    s_nStateOnSent[STATE_INVITE_1XX_RECEIVED][MESSAGE_BYE] = STATE_INVITE_1XX_RECEIVED;

    // SENT : STATE_INVITE_2XX_RECEIVED
    s_nStateOnSent[STATE_INVITE_2XX_RECEIVED][MESSAGE_ACK] = STATE_ESTABLISHED;

    // SENT : STATE_INVITE_NON2XX_RECEIVED
    s_nStateOnSent[STATE_INVITE_NON2XX_RECEIVED][MESSAGE_ACK] = STATE_IDLE;

    // SENT : STATE_INVITE_RECEIVED
    s_nStateOnSent[STATE_INVITE_RECEIVED][MESSAGE_1XX] = STATE_INVITE_1XX_SENT;
    s_nStateOnSent[STATE_INVITE_RECEIVED][MESSAGE_2XX] = STATE_INVITE_2XX_SENT;
    s_nStateOnSent[STATE_INVITE_RECEIVED][MESSAGE_NON2XX] = STATE_INVITE_NON2XX_SENT;

    // SENT : PROVISIONAL_RESP_SENT
    s_nStateOnSent[STATE_INVITE_1XX_SENT][MESSAGE_1XX] = STATE_INVITE_1XX_SENT;
    s_nStateOnSent[STATE_INVITE_1XX_SENT][MESSAGE_2XX] = STATE_INVITE_2XX_SENT;
    s_nStateOnSent[STATE_INVITE_1XX_SENT][MESSAGE_NON2XX] = STATE_INVITE_NON2XX_SENT;

    // SENT : STATE_INVITE_2XX_SENT
    s_nStateOnSent[STATE_INVITE_2XX_SENT][MESSAGE_BYE] = STATE_BYE_SENT;

    // SENT : STATE_INVITE_NON2XX_SENT

    // SENT : STATE_REINVITE_SENT
    s_nStateOnSent[STATE_REINVITE_SENT][MESSAGE_BYE] = STATE_BYE_SENT;

    // SENT : STATE_REINVITE_1XX_RECEIVED
    s_nStateOnSent[STATE_REINVITE_1XX_RECEIVED][MESSAGE_BYE] = STATE_BYE_SENT;

    // SENT : STATE_REINVITE_2XX_RECEIVED
    s_nStateOnSent[STATE_REINVITE_2XX_RECEIVED][MESSAGE_ACK] = STATE_ESTABLISHED;

    // SENT : STATE_REINVITE_NON2XX_RECEIVED
    s_nStateOnSent[STATE_REINVITE_NON2XX_RECEIVED][MESSAGE_ACK] = STATE_ESTABLISHED;

    // SENT : STATE_REINVITE_RECEIVED
    s_nStateOnSent[STATE_REINVITE_RECEIVED][MESSAGE_1XX] = STATE_REINVITE_1XX_SENT;
    s_nStateOnSent[STATE_REINVITE_RECEIVED][MESSAGE_2XX] = STATE_REINVITE_2XX_SENT;
    s_nStateOnSent[STATE_REINVITE_RECEIVED][MESSAGE_NON2XX] = STATE_REINVITE_NON2XX_SENT;

    // SENT : STATE_REINVITE_1XX_SENT
    s_nStateOnSent[STATE_REINVITE_1XX_SENT][MESSAGE_1XX] = STATE_REINVITE_1XX_SENT;
    s_nStateOnSent[STATE_REINVITE_1XX_SENT][MESSAGE_2XX] = STATE_REINVITE_2XX_SENT;
    s_nStateOnSent[STATE_REINVITE_1XX_SENT][MESSAGE_NON2XX] = STATE_REINVITE_NON2XX_SENT;

    // SENT : STATE_REINVITE_2XX_SENT

    // SENT : STATE_REINVITE_NON2XX_SENT

    // SENT : STATE_ESTABLISHED
    s_nStateOnSent[STATE_ESTABLISHED][MESSAGE_INVITE] = STATE_REINVITE_SENT;
    s_nStateOnSent[STATE_ESTABLISHED][MESSAGE_BYE] = STATE_BYE_SENT;

    // SENT : STATE_INVITE_CANCELLED
    s_nStateOnSent[STATE_INVITE_CANCELLED][MESSAGE_2XX] = STATE_INVITE_2XX_SENT;
    s_nStateOnSent[STATE_INVITE_CANCELLED][MESSAGE_NON2XX] = STATE_INVITE_NON2XX_SENT;

    // SENT : STATE_REINVITE_CANCELLED
    s_nStateOnSent[STATE_REINVITE_CANCELLED][MESSAGE_2XX] = STATE_REINVITE_2XX_SENT;
    s_nStateOnSent[STATE_REINVITE_CANCELLED][MESSAGE_NON2XX] = STATE_REINVITE_NON2XX_SENT;

    // SENT : STATE_BYE_SENT

    // SENT : STATE_BYE_RECEIVED
    s_nStateOnSent[STATE_BYE_RECEIVED][MESSAGE_2XX] = STATE_TERMINATED;
    s_nStateOnSent[STATE_BYE_RECEIVED][MESSAGE_NON2XX] = STATE_ESTABLISHED;

    // SENT : STATE_TERMINATED

    ////////////////////////////////////////////////////////////
    //// Initialize the state machine for message received

    // RECEIVED : STATE_IDLE
    s_nStateOnReceived[STATE_IDLE][MESSAGE_INVITE] = STATE_INVITE_RECEIVED;

    // RECEIVED : STATE_INVITE_SENT
    s_nStateOnReceived[STATE_INVITE_SENT][MESSAGE_1XX] = STATE_INVITE_1XX_RECEIVED;
    s_nStateOnReceived[STATE_INVITE_SENT][MESSAGE_2XX] = STATE_INVITE_2XX_RECEIVED;
    s_nStateOnReceived[STATE_INVITE_SENT][MESSAGE_NON2XX] = STATE_INVITE_NON2XX_RECEIVED;

    // RECEIVED : STATE_INVITE_1XX_RECEIVED
    s_nStateOnReceived[STATE_INVITE_1XX_RECEIVED][MESSAGE_1XX] = STATE_INVITE_1XX_RECEIVED;
    s_nStateOnReceived[STATE_INVITE_1XX_RECEIVED][MESSAGE_2XX] = STATE_INVITE_2XX_RECEIVED;
    s_nStateOnReceived[STATE_INVITE_1XX_RECEIVED][MESSAGE_NON2XX] = STATE_INVITE_NON2XX_RECEIVED;

    // RECEIVED : STATE_INVITE_2XX_RECEIVED
    s_nStateOnReceived[STATE_INVITE_2XX_RECEIVED][MESSAGE_BYE] = STATE_BYE_RECEIVED;

    // RECEIVED : STATE_INVITE_NON2XX_RECEIVED

    // RECEIVED : STATE_INVITE_RECEIVED
    s_nStateOnReceived[STATE_INVITE_RECEIVED][MESSAGE_CANCEL] = STATE_INVITE_CANCELLED;

    // RECEIVED : STATE_INVITE_1XX_SENT
    s_nStateOnReceived[STATE_INVITE_1XX_SENT][MESSAGE_CANCEL] = STATE_INVITE_CANCELLED;
    s_nStateOnReceived[STATE_INVITE_1XX_SENT][MESSAGE_BYE] = STATE_INVITE_CANCELLED;

    // RECEIVED : STATE_INVITE_2XX_SENT
    s_nStateOnReceived[STATE_INVITE_2XX_SENT][MESSAGE_CANCEL] = STATE_INVITE_2XX_SENT;
    s_nStateOnReceived[STATE_INVITE_2XX_SENT][MESSAGE_ACK] = STATE_ESTABLISHED;
    s_nStateOnReceived[STATE_INVITE_2XX_SENT][MESSAGE_BYE] = STATE_INVITE_2XX_SENT;

    // RECEIVED : STATE_INVITE_NON2XX_SENT
    s_nStateOnReceived[STATE_INVITE_NON2XX_SENT][MESSAGE_CANCEL] = STATE_INVITE_NON2XX_SENT;
    s_nStateOnReceived[STATE_INVITE_NON2XX_SENT][MESSAGE_ACK] = STATE_IDLE;
    s_nStateOnReceived[STATE_INVITE_NON2XX_SENT][MESSAGE_BYE] = STATE_INVITE_NON2XX_SENT;

    // RECEIVED : STATE_REINVITE_SENT
    s_nStateOnReceived[STATE_REINVITE_SENT][MESSAGE_BYE] = STATE_BYE_RECEIVED;
    s_nStateOnReceived[STATE_REINVITE_SENT][MESSAGE_1XX] = STATE_REINVITE_1XX_RECEIVED;
    s_nStateOnReceived[STATE_REINVITE_SENT][MESSAGE_2XX] = STATE_REINVITE_2XX_RECEIVED;
    s_nStateOnReceived[STATE_REINVITE_SENT][MESSAGE_NON2XX] = STATE_REINVITE_NON2XX_RECEIVED;

    // RECEIVED : REINVITE_1XX_RECEIVED
    s_nStateOnReceived[STATE_REINVITE_1XX_RECEIVED][MESSAGE_BYE] = STATE_BYE_RECEIVED;
    s_nStateOnReceived[STATE_REINVITE_1XX_RECEIVED][MESSAGE_1XX] = STATE_REINVITE_1XX_RECEIVED;
    s_nStateOnReceived[STATE_REINVITE_1XX_RECEIVED][MESSAGE_2XX] = STATE_REINVITE_2XX_RECEIVED;
    s_nStateOnReceived[STATE_REINVITE_1XX_RECEIVED][MESSAGE_NON2XX] =
            STATE_REINVITE_NON2XX_RECEIVED;

    // RECEIVED : STATE_REINVITE_2XX_RECEIVED
    s_nStateOnReceived[STATE_REINVITE_2XX_RECEIVED][MESSAGE_BYE] = STATE_BYE_RECEIVED;

    // RECEIVED : STATE_REINVITE_NON2XX_RECEIVED
    s_nStateOnReceived[STATE_REINVITE_NON2XX_RECEIVED][MESSAGE_BYE] = STATE_BYE_RECEIVED;

    // RECEIVED : STATE_REINVITE_RECEIVED
    s_nStateOnReceived[STATE_REINVITE_RECEIVED][MESSAGE_CANCEL] = STATE_REINVITE_CANCELLED;
    s_nStateOnReceived[STATE_REINVITE_RECEIVED][MESSAGE_BYE] = STATE_BYE_RECEIVED;

    // RECEIVED : STATE_REINVITE_1XX_SENT
    s_nStateOnReceived[STATE_REINVITE_1XX_SENT][MESSAGE_CANCEL] = STATE_REINVITE_CANCELLED;
    s_nStateOnReceived[STATE_REINVITE_1XX_SENT][MESSAGE_BYE] = STATE_BYE_RECEIVED;

    // RECEIVED : STATE_REINVITE_2XX_SENT
    s_nStateOnReceived[STATE_REINVITE_2XX_SENT][MESSAGE_CANCEL] = STATE_REINVITE_2XX_SENT;
    s_nStateOnReceived[STATE_REINVITE_2XX_SENT][MESSAGE_ACK] = STATE_ESTABLISHED;
    s_nStateOnReceived[STATE_REINVITE_2XX_SENT][MESSAGE_BYE] = STATE_REINVITE_2XX_SENT;

    // RECEIVED : STATE_REINVITE_NON2XX_SENT
    s_nStateOnReceived[STATE_REINVITE_NON2XX_SENT][MESSAGE_CANCEL] = STATE_REINVITE_NON2XX_SENT;
    s_nStateOnReceived[STATE_REINVITE_NON2XX_SENT][MESSAGE_ACK] = STATE_ESTABLISHED;
    s_nStateOnReceived[STATE_REINVITE_NON2XX_SENT][MESSAGE_BYE] = STATE_REINVITE_NON2XX_SENT;

    // RECEIVED : STATE_ESTABLISHED
    s_nStateOnReceived[STATE_ESTABLISHED][MESSAGE_INVITE] = STATE_REINVITE_RECEIVED;
    s_nStateOnReceived[STATE_ESTABLISHED][MESSAGE_CANCEL] = STATE_ESTABLISHED;
    s_nStateOnReceived[STATE_ESTABLISHED][MESSAGE_ACK] = STATE_ESTABLISHED;
    s_nStateOnReceived[STATE_ESTABLISHED][MESSAGE_BYE] = STATE_BYE_RECEIVED;

    // RECEIVED : STATE_INVITE_CANCELLED
    s_nStateOnReceived[STATE_INVITE_CANCELLED][MESSAGE_BYE] = STATE_INVITE_CANCELLED;
    s_nStateOnReceived[STATE_INVITE_CANCELLED][MESSAGE_CANCEL] = STATE_INVITE_CANCELLED;

    // RECEIVED : STATE_REINVITE_CANCELLED

    // RECEIVED : STATE_BYE_SENT
    s_nStateOnReceived[STATE_BYE_SENT][MESSAGE_ACK] = STATE_BYE_SENT;
    s_nStateOnReceived[STATE_BYE_SENT][MESSAGE_BYE] = STATE_BYE_RECEIVED;
    s_nStateOnReceived[STATE_BYE_SENT][MESSAGE_2XX] = STATE_TERMINATED;
    s_nStateOnReceived[STATE_BYE_SENT][MESSAGE_NON2XX] = STATE_ESTABLISHED;

    // RECEIVED : STATE_BYE_RECEIVED

    // RECEIVED : STATE_TERMINATED

    bInitialized = IMS_TRUE;
}

PRIVATE GLOBAL void CallState::PrintStateChanged(
        IN const ISipMessage* piSipMsg, IN IMS_SINT32 nState, IN IMS_SINT32 nNextState)
{
    // clang-format off
    static const IMS_CHAR* STATE[] = {
            "STATE_INVALID",
            "STATE_IDLE",
            "STATE_INVITE_SENT",
            "STATE_INVITE_1XX_RECEIVED",
            "STATE_INVITE_2XX_RECEIVED",
            "STATE_INVITE_NON2XX_RECEIVED",
            "STATE_INVITE_RECEIVED",
            "STATE_INVITE_1XX_SENT",
            "STATE_INVITE_2XX_SENT",
            "STATE_INVITE_NON2XX_SENT",
            "STATE_REINVITE_SENT",
            "STATE_REINVITE_1XX_RECEIVED",
            "STATE_REINVITE_2XX_RECEIVED",
            "STATE_REINVITE_NON2XX_RECEIVED",
            "STATE_REINVITE_RECEIVED",
            "STATE_REINVITE_1XX_SENT",
            "STATE_REINVITE_2XX_SENT",
            "STATE_REINVITE_NON2XX_SENT",
            "STATE_ESTABLISHED",
            "STATE_INVITE_CANCELLED",
            "STATE_REINVITE_CANCELLED",
            "STATE_BYE_SENT",
            "STATE_BYE_RECEIVED",
            "STATE_TERMINATED"
    };
    // clang-format on

    AString strCallId = piSipMsg->GetHeader(ISipHeader::CALL_ID);

    if (!((nState > STATE_INVALID) && (nState < STATE_MAX)) ||
            !((nNextState > STATE_INVALID) && (nNextState < STATE_MAX)))
    {
        IMS_TRACE_I("CALL_STATE : %s - %d >> %d", SipDebug::GetCharA1(strCallId.GetStr(), 8, '@'),
                nState, nNextState);
        return;
    }

    IMS_TRACE_I("CALL_STATE : %s - %s >> %s", SipDebug::GetCharA1(strCallId.GetStr(), 8, '@'),
            STATE[nState], STATE[nNextState]);
}

PRIVATE GLOBAL IMS_SINT32 CallState::TranslateMessage(IN const ISipMessage* piSipMsg)
{
    IMS_SINT32 nMsgType = piSipMsg->GetType();
    const SipMethod& objMethod = piSipMsg->GetMethod();

    if (nMsgType == ISipMessage::TYPE_REQUEST)
    {
        if (objMethod.Equals(SipMethod::INVITE))
        {
            return MESSAGE_INVITE;
        }
        else if (objMethod.Equals(SipMethod::CANCEL))
        {
            return MESSAGE_CANCEL;
        }
        else if (objMethod.Equals(SipMethod::ACK))
        {
            return MESSAGE_ACK;
        }
        else if (objMethod.Equals(SipMethod::BYE))
        {
            return MESSAGE_BYE;
        }
        else
        {
            return MESSAGE_INVALID;
        }
    }
    else if (nMsgType == ISipMessage::TYPE_RESPONSE)
    {
        // If the method is not INVITE/CANCEL/BYE/ACK, ignore the message for the call state
        if (!objMethod.Equals(SipMethod::INVITE) && !objMethod.Equals(SipMethod::CANCEL) &&
                !objMethod.Equals(SipMethod::ACK) && !objMethod.Equals(SipMethod::BYE))
        {
            return MESSAGE_INVALID;
        }

        IMS_SINT32 nStatusCode = piSipMsg->GetStatusCode();

        if (SipStatusCode::Is1XX(nStatusCode))
        {
            return MESSAGE_1XX;
        }
        else if (SipStatusCode::IsFinalSuccess(nStatusCode))
        {
            return MESSAGE_2XX;
        }
        else
        {
            return MESSAGE_NON2XX;
        }
    }
    else
    {
        return MESSAGE_INVALID;
    }
}
