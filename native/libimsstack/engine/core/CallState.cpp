/*
    Author
    <table>
    date      author                    description
    --------  --------------            ----------
    20090922  toastops@                 Created
    </table>

    Description

*/

#include "ServiceMemory.h"
#include "ServiceTrace.h"
#include "ISipHeader.h"
#include "ISipMessage.h"
#include "SipStatusCode.h"
#include "SipDebug.h"
#include "CallState.h"

__IMS_TRACE_TAG_IMS_CORE__;

PUBLIC GLOBAL IMS_SINT32 CallState::STATE_ON_SENT[CallState::STATE_MAX][CallState::MESSAGE_MAX] =
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

PUBLIC GLOBAL IMS_SINT32
CallState::STATE_ON_RECEIVED[CallState::STATE_MAX][CallState::MESSAGE_MAX] =
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

PUBLIC
CallState::CallState() :
        nState(STATE_IDLE)
{
    InitializeStateTable();
}

PUBLIC
CallState::~CallState() {}

PUBLIC
IMS_SINT32 CallState::GetState() const
{
    //---------------------------------------------------------------------------------------------

    return nState;
}

PUBLIC
IMS_BOOL CallState::UpdateState(IN CONST ISipMessage* piSIPMsg, IN IMS_SINT32 nMode)
{
    // MSG_MODE_CANCEL_RECEIVED, MSG_MODE_BYE_RECEIVED
    IMS_SINT32 nSIPMsg = TranslateMessage(piSIPMsg);

    //---------------------------------------------------------------------------------------------

    if (nSIPMsg == MESSAGE_INVALID)
    {
        IMS_TRACE_I(
                "CALL_STATE - NO TRANSITION (%s)", piSIPMsg->GetMethod().ToString().GetStr(), 0, 0);
        return IMS_TRUE;
    }

    /*
     * Responses to CANCEL should not modify the call state.
     * The final response to the INVITE causes the appropriate state transition.
     * And also, if response to BYE comes when the call state is not in BYE sent stage,
     * then it means the BYE is sent to cancel the INVITE.
     * So, don't update the call state in those cases.
     */
    if (piSIPMsg->GetType() == ISipMessage::TYPE_RESPONSE)
    {
        const SipMethod& objMethod = piSIPMsg->GetMethod();

        if (objMethod.Equals(SipMethod::CANCEL) ||
                (objMethod.Equals(SipMethod::BYE) && (nState != STATE_BYE_SENT) &&
                        (nState != STATE_BYE_RECEIVED)))
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

        if (STATE_ON_SENT[nState][nSIPMsg] != STATE_INVALID)
        {
            // TRACE
            PrintStateChanged(piSIPMsg, nState, STATE_ON_SENT[nState][nSIPMsg]);

            // Update the state
            nState = STATE_ON_SENT[nState][nSIPMsg];

            // ACK request to non-2xx will be ignored by J180
            if (nState == STATE_REINVITE_NON2XX_SENT)
            {
                if (STATE_ON_RECEIVED[nState][MESSAGE_ACK] != STATE_INVALID)
                {
                    // TRACE
                    PrintStateChanged(piSIPMsg, nState, STATE_ON_RECEIVED[nState][MESSAGE_ACK]);

                    // Update the state
                    nState = STATE_ON_RECEIVED[nState][MESSAGE_ACK];
                }
            }
        }
    }
    // If the message received and the current state map to a valid state transition,
    // then change the state.
    else if (nMode == MODE_RECEIVED)
    {
        // When INVITE is received previously, can UAS send a CANCEL request to it ?

        if (STATE_ON_RECEIVED[nState][nSIPMsg] != STATE_INVALID)
        {
            // TRACE
            PrintStateChanged(piSIPMsg, nState, STATE_ON_RECEIVED[nState][nSIPMsg]);

            // Update the state
            nState = STATE_ON_RECEIVED[nState][nSIPMsg];

            // ACK request to non-2xx will be sent by J180 automatically
            if ((nState == STATE_INVITE_NON2XX_RECEIVED) ||
                    (nState == STATE_REINVITE_NON2XX_RECEIVED))
            {
                if (STATE_ON_SENT[nState][MESSAGE_ACK] != STATE_INVALID)
                {
                    // TRACE
                    PrintStateChanged(piSIPMsg, nState, STATE_ON_SENT[nState][MESSAGE_ACK]);

                    // Update the state
                    nState = STATE_ON_SENT[nState][MESSAGE_ACK];
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

    //---------------------------------------------------------------------------------------------

    if (bInitialized)
        return;

    for (i = 0; i < STATE_MAX; ++i)
    {
        for (j = 0; j < MESSAGE_MAX; ++j)
        {
            STATE_ON_SENT[i][j] = STATE_INVALID;
        }
    }

    for (i = 0; i < STATE_MAX; ++i)
    {
        for (j = 0; j < MESSAGE_MAX; ++j)
        {
            STATE_ON_RECEIVED[i][j] = STATE_INVALID;
        }
    }

    ////////////////////////////////////////////////////////////
    //// Initialize the state machine for message sent

    // SENT : STATE_IDLE
    STATE_ON_SENT[STATE_IDLE][MESSAGE_INVITE] = STATE_INVITE_SENT;

    // SENT : STATE_INVITE_SENT

    // SENT : STATE_INVITE_1XX_RECEIVED
    STATE_ON_SENT[STATE_INVITE_1XX_RECEIVED][MESSAGE_BYE] = STATE_INVITE_1XX_RECEIVED;

    // SENT : STATE_INVITE_2XX_RECEIVED
    STATE_ON_SENT[STATE_INVITE_2XX_RECEIVED][MESSAGE_ACK] = STATE_ESTABLISHED;

    // SENT : STATE_INVITE_NON2XX_RECEIVED
    STATE_ON_SENT[STATE_INVITE_NON2XX_RECEIVED][MESSAGE_ACK] = STATE_IDLE;

    // SENT : STATE_INVITE_RECEIVED
    STATE_ON_SENT[STATE_INVITE_RECEIVED][MESSAGE_1XX] = STATE_INVITE_1XX_SENT;
    STATE_ON_SENT[STATE_INVITE_RECEIVED][MESSAGE_2XX] = STATE_INVITE_2XX_SENT;
    STATE_ON_SENT[STATE_INVITE_RECEIVED][MESSAGE_NON2XX] = STATE_INVITE_NON2XX_SENT;

    // SENT : PROVISIONAL_RESP_SENT
    STATE_ON_SENT[STATE_INVITE_1XX_SENT][MESSAGE_1XX] = STATE_INVITE_1XX_SENT;
    STATE_ON_SENT[STATE_INVITE_1XX_SENT][MESSAGE_2XX] = STATE_INVITE_2XX_SENT;
    STATE_ON_SENT[STATE_INVITE_1XX_SENT][MESSAGE_NON2XX] = STATE_INVITE_NON2XX_SENT;

    // SENT : STATE_INVITE_2XX_SENT
    STATE_ON_SENT[STATE_INVITE_2XX_SENT][MESSAGE_BYE] = STATE_BYE_SENT;

    // SENT : STATE_INVITE_NON2XX_SENT

    // SENT : STATE_REINVITE_SENT
    STATE_ON_SENT[STATE_REINVITE_SENT][MESSAGE_BYE] = STATE_BYE_SENT;

    // SENT : STATE_REINVITE_1XX_RECEIVED
    STATE_ON_SENT[STATE_REINVITE_1XX_RECEIVED][MESSAGE_BYE] = STATE_BYE_SENT;

    // SENT : STATE_REINVITE_2XX_RECEIVED
    STATE_ON_SENT[STATE_REINVITE_2XX_RECEIVED][MESSAGE_ACK] = STATE_ESTABLISHED;

    // SENT : STATE_REINVITE_NON2XX_RECEIVED
    STATE_ON_SENT[STATE_REINVITE_NON2XX_RECEIVED][MESSAGE_ACK] = STATE_ESTABLISHED;

    // SENT : STATE_REINVITE_RECEIVED
    STATE_ON_SENT[STATE_REINVITE_RECEIVED][MESSAGE_1XX] = STATE_REINVITE_1XX_SENT;
    STATE_ON_SENT[STATE_REINVITE_RECEIVED][MESSAGE_2XX] = STATE_REINVITE_2XX_SENT;
    STATE_ON_SENT[STATE_REINVITE_RECEIVED][MESSAGE_NON2XX] = STATE_REINVITE_NON2XX_SENT;

    // SENT : STATE_REINVITE_1XX_SENT
    STATE_ON_SENT[STATE_REINVITE_1XX_SENT][MESSAGE_1XX] = STATE_REINVITE_1XX_SENT;
    STATE_ON_SENT[STATE_REINVITE_1XX_SENT][MESSAGE_2XX] = STATE_REINVITE_2XX_SENT;
    STATE_ON_SENT[STATE_REINVITE_1XX_SENT][MESSAGE_NON2XX] = STATE_REINVITE_NON2XX_SENT;

    // SENT : STATE_REINVITE_2XX_SENT

    // SENT : STATE_REINVITE_NON2XX_SENT

    // SENT : STATE_ESTABLISHED
    STATE_ON_SENT[STATE_ESTABLISHED][MESSAGE_INVITE] = STATE_REINVITE_SENT;
    STATE_ON_SENT[STATE_ESTABLISHED][MESSAGE_BYE] = STATE_BYE_SENT;

    // SENT : STATE_INVITE_CANCELLED
    STATE_ON_SENT[STATE_INVITE_CANCELLED][MESSAGE_2XX] = STATE_INVITE_2XX_SENT;
    STATE_ON_SENT[STATE_INVITE_CANCELLED][MESSAGE_NON2XX] = STATE_INVITE_NON2XX_SENT;

    // SENT : STATE_REINVITE_CANCELLED
    STATE_ON_SENT[STATE_REINVITE_CANCELLED][MESSAGE_2XX] = STATE_REINVITE_2XX_SENT;
    STATE_ON_SENT[STATE_REINVITE_CANCELLED][MESSAGE_NON2XX] = STATE_REINVITE_NON2XX_SENT;

    // SENT : STATE_BYE_SENT

    // SENT : STATE_BYE_RECEIVED
    STATE_ON_SENT[STATE_BYE_RECEIVED][MESSAGE_2XX] = STATE_TERMINATED;
    STATE_ON_SENT[STATE_BYE_RECEIVED][MESSAGE_NON2XX] = STATE_ESTABLISHED;

    // SENT : STATE_TERMINATED

    ////////////////////////////////////////////////////////////
    //// Initialize the state machine for message received

    // RECEIVED : STATE_IDLE
    STATE_ON_RECEIVED[STATE_IDLE][MESSAGE_INVITE] = STATE_INVITE_RECEIVED;

    // RECEIVED : STATE_INVITE_SENT
    STATE_ON_RECEIVED[STATE_INVITE_SENT][MESSAGE_1XX] = STATE_INVITE_1XX_RECEIVED;
    STATE_ON_RECEIVED[STATE_INVITE_SENT][MESSAGE_2XX] = STATE_INVITE_2XX_RECEIVED;
    STATE_ON_RECEIVED[STATE_INVITE_SENT][MESSAGE_NON2XX] = STATE_INVITE_NON2XX_RECEIVED;

    // RECEIVED : STATE_INVITE_1XX_RECEIVED
    STATE_ON_RECEIVED[STATE_INVITE_1XX_RECEIVED][MESSAGE_1XX] = STATE_INVITE_1XX_RECEIVED;
    STATE_ON_RECEIVED[STATE_INVITE_1XX_RECEIVED][MESSAGE_2XX] = STATE_INVITE_2XX_RECEIVED;
    STATE_ON_RECEIVED[STATE_INVITE_1XX_RECEIVED][MESSAGE_NON2XX] = STATE_INVITE_NON2XX_RECEIVED;

    // RECEIVED : STATE_INVITE_2XX_RECEIVED
    STATE_ON_RECEIVED[STATE_INVITE_2XX_RECEIVED][MESSAGE_BYE] = STATE_BYE_RECEIVED;

    // RECEIVED : STATE_INVITE_NON2XX_RECEIVED

    // RECEIVED : STATE_INVITE_RECEIVED
    STATE_ON_RECEIVED[STATE_INVITE_RECEIVED][MESSAGE_CANCEL] = STATE_INVITE_CANCELLED;

    // RECEIVED : STATE_INVITE_1XX_SENT
    STATE_ON_RECEIVED[STATE_INVITE_1XX_SENT][MESSAGE_CANCEL] = STATE_INVITE_CANCELLED;
    STATE_ON_RECEIVED[STATE_INVITE_1XX_SENT][MESSAGE_BYE] = STATE_INVITE_CANCELLED;

    // RECEIVED : STATE_INVITE_2XX_SENT
    STATE_ON_RECEIVED[STATE_INVITE_2XX_SENT][MESSAGE_CANCEL] = STATE_INVITE_2XX_SENT;
    STATE_ON_RECEIVED[STATE_INVITE_2XX_SENT][MESSAGE_ACK] = STATE_ESTABLISHED;
    STATE_ON_RECEIVED[STATE_INVITE_2XX_SENT][MESSAGE_BYE] = STATE_INVITE_2XX_SENT;

    // RECEIVED : STATE_INVITE_NON2XX_SENT
    STATE_ON_RECEIVED[STATE_INVITE_NON2XX_SENT][MESSAGE_CANCEL] = STATE_INVITE_NON2XX_SENT;
    STATE_ON_RECEIVED[STATE_INVITE_NON2XX_SENT][MESSAGE_ACK] = STATE_IDLE;
    STATE_ON_RECEIVED[STATE_INVITE_NON2XX_SENT][MESSAGE_BYE] = STATE_INVITE_NON2XX_SENT;

    // RECEIVED : STATE_REINVITE_SENT
    STATE_ON_RECEIVED[STATE_REINVITE_SENT][MESSAGE_BYE] = STATE_BYE_RECEIVED;
    STATE_ON_RECEIVED[STATE_REINVITE_SENT][MESSAGE_1XX] = STATE_REINVITE_1XX_RECEIVED;
    STATE_ON_RECEIVED[STATE_REINVITE_SENT][MESSAGE_2XX] = STATE_REINVITE_2XX_RECEIVED;
    STATE_ON_RECEIVED[STATE_REINVITE_SENT][MESSAGE_NON2XX] = STATE_REINVITE_NON2XX_RECEIVED;

    // RECEIVED : REINVITE_1XX_RECEIVED
    STATE_ON_RECEIVED[STATE_REINVITE_1XX_RECEIVED][MESSAGE_BYE] = STATE_BYE_RECEIVED;
    STATE_ON_RECEIVED[STATE_REINVITE_1XX_RECEIVED][MESSAGE_1XX] = STATE_REINVITE_1XX_RECEIVED;
    STATE_ON_RECEIVED[STATE_REINVITE_1XX_RECEIVED][MESSAGE_2XX] = STATE_REINVITE_2XX_RECEIVED;
    STATE_ON_RECEIVED[STATE_REINVITE_1XX_RECEIVED][MESSAGE_NON2XX] = STATE_REINVITE_NON2XX_RECEIVED;

    // RECEIVED : STATE_REINVITE_2XX_RECEIVED
    STATE_ON_RECEIVED[STATE_REINVITE_2XX_RECEIVED][MESSAGE_BYE] = STATE_BYE_RECEIVED;

    // RECEIVED : STATE_REINVITE_NON2XX_RECEIVED
    STATE_ON_RECEIVED[STATE_REINVITE_NON2XX_RECEIVED][MESSAGE_BYE] = STATE_BYE_RECEIVED;

    // RECEIVED : STATE_REINVITE_RECEIVED
    STATE_ON_RECEIVED[STATE_REINVITE_RECEIVED][MESSAGE_CANCEL] = STATE_REINVITE_CANCELLED;
    STATE_ON_RECEIVED[STATE_REINVITE_RECEIVED][MESSAGE_BYE] = STATE_BYE_RECEIVED;

    // RECEIVED : STATE_REINVITE_1XX_SENT
    STATE_ON_RECEIVED[STATE_REINVITE_1XX_SENT][MESSAGE_CANCEL] = STATE_REINVITE_CANCELLED;
    STATE_ON_RECEIVED[STATE_REINVITE_1XX_SENT][MESSAGE_BYE] = STATE_BYE_RECEIVED;

    // RECEIVED : STATE_REINVITE_2XX_SENT
    STATE_ON_RECEIVED[STATE_REINVITE_2XX_SENT][MESSAGE_CANCEL] = STATE_REINVITE_2XX_SENT;
    STATE_ON_RECEIVED[STATE_REINVITE_2XX_SENT][MESSAGE_ACK] = STATE_ESTABLISHED;
    STATE_ON_RECEIVED[STATE_REINVITE_2XX_SENT][MESSAGE_BYE] = STATE_REINVITE_2XX_SENT;

    // RECEIVED : STATE_REINVITE_NON2XX_SENT
    STATE_ON_RECEIVED[STATE_REINVITE_NON2XX_SENT][MESSAGE_CANCEL] = STATE_REINVITE_NON2XX_SENT;
    STATE_ON_RECEIVED[STATE_REINVITE_NON2XX_SENT][MESSAGE_ACK] = STATE_ESTABLISHED;
    STATE_ON_RECEIVED[STATE_REINVITE_NON2XX_SENT][MESSAGE_BYE] = STATE_REINVITE_NON2XX_SENT;

    // RECEIVED : STATE_ESTABLISHED
    STATE_ON_RECEIVED[STATE_ESTABLISHED][MESSAGE_INVITE] = STATE_REINVITE_RECEIVED;
    STATE_ON_RECEIVED[STATE_ESTABLISHED][MESSAGE_CANCEL] = STATE_ESTABLISHED;
    STATE_ON_RECEIVED[STATE_ESTABLISHED][MESSAGE_ACK] = STATE_ESTABLISHED;
    STATE_ON_RECEIVED[STATE_ESTABLISHED][MESSAGE_BYE] = STATE_BYE_RECEIVED;

    // RECEIVED : STATE_INVITE_CANCELLED
    STATE_ON_RECEIVED[STATE_INVITE_CANCELLED][MESSAGE_BYE] = STATE_INVITE_CANCELLED;
    STATE_ON_RECEIVED[STATE_INVITE_CANCELLED][MESSAGE_CANCEL] = STATE_INVITE_CANCELLED;

    // RECEIVED : STATE_REINVITE_CANCELLED

    // RECEIVED : STATE_BYE_SENT
    STATE_ON_RECEIVED[STATE_BYE_SENT][MESSAGE_ACK] = STATE_BYE_SENT;
    STATE_ON_RECEIVED[STATE_BYE_SENT][MESSAGE_BYE] = STATE_BYE_RECEIVED;
    STATE_ON_RECEIVED[STATE_BYE_SENT][MESSAGE_2XX] = STATE_TERMINATED;
    STATE_ON_RECEIVED[STATE_BYE_SENT][MESSAGE_NON2XX] = STATE_ESTABLISHED;

    // RECEIVED : STATE_BYE_RECEIVED

    // RECEIVED : STATE_TERMINATED

    bInitialized = IMS_TRUE;
}

PRIVATE GLOBAL void CallState::PrintStateChanged(
        IN CONST ISipMessage* piSIPMsg, IN IMS_SINT32 nState, IN IMS_SINT32 nNextState)
{
    static const IMS_CHAR* STATE[] = {"STATE_INVALID", "STATE_IDLE", "STATE_INVITE_SENT",
            "STATE_INVITE_1XX_RECEIVED", "STATE_INVITE_2XX_RECEIVED",
            "STATE_INVITE_NON2XX_RECEIVED", "STATE_INVITE_RECEIVED", "STATE_INVITE_1XX_SENT",
            "STATE_INVITE_2XX_SENT", "STATE_INVITE_NON2XX_SENT", "STATE_REINVITE_SENT",
            "STATE_REINVITE_1XX_RECEIVED", "STATE_REINVITE_2XX_RECEIVED",
            "STATE_REINVITE_NON2XX_RECEIVED", "STATE_REINVITE_RECEIVED", "STATE_REINVITE_1XX_SENT",
            "STATE_REINVITE_2XX_SENT", "STATE_REINVITE_NON2XX_SENT", "STATE_ESTABLISHED",
            "STATE_INVITE_CANCELLED", "STATE_REINVITE_CANCELLED", "STATE_BYE_SENT",
            "STATE_BYE_RECEIVED", "STATE_TERMINATED"};

    AString strCallId = piSIPMsg->GetHeader(ISipHeader::CALL_ID);

    //-----------------------------------------------------------------------------------------

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

PRIVATE GLOBAL IMS_SINT32 CallState::TranslateMessage(IN CONST ISipMessage* piSIPMsg)
{
    IMS_SINT32 nMsgType = piSIPMsg->GetType();
    const SipMethod& objMethod = piSIPMsg->GetMethod();

    //---------------------------------------------------------------------------------------------

    if (nMsgType == ISipMessage::TYPE_REQUEST)
    {
        if (objMethod.Equals(SipMethod::INVITE))
            return MESSAGE_INVITE;
        else if (objMethod.Equals(SipMethod::CANCEL))
            return MESSAGE_CANCEL;
        else if (objMethod.Equals(SipMethod::ACK))
            return MESSAGE_ACK;
        else if (objMethod.Equals(SipMethod::BYE))
            return MESSAGE_BYE;
        else
            return MESSAGE_INVALID;
    }
    else if (nMsgType == ISipMessage::TYPE_RESPONSE)
    {
        // If the method is not INVITE/CANCEL/BYE/ACK, ignore the message for the call state
        if (!objMethod.Equals(SipMethod::INVITE) && !objMethod.Equals(SipMethod::CANCEL) &&
                !objMethod.Equals(SipMethod::ACK) && !objMethod.Equals(SipMethod::BYE))
        {
            return MESSAGE_INVALID;
        }

        IMS_SINT32 nStatusCode = piSIPMsg->GetStatusCode();

        if (SipStatusCode::Is1XX(nStatusCode))
            return MESSAGE_1XX;
        else if (SipStatusCode::IsFinalSuccess(nStatusCode))
            return MESSAGE_2XX;
        else
            return MESSAGE_NON2XX;
    }
    else
    {
        return MESSAGE_INVALID;
    }
}
