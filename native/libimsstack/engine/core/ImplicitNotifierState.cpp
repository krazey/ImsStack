/*
    Author
    <table>
    date      author                    description
    --------  --------------            ----------
    20100420  hwangoo.park@             Created
    </table>

    Description

*/

#include "ServiceMemory.h"
#include "ServiceTrace.h"
#include "ISipHeader.h"
#include "Sip.h"
#include "SipParsingHelper.h"
#include "SipStatusCode.h"
#include "ImplicitNotifierState.h"

__IMS_TRACE_TAG_IMS_CORE__;

PUBLIC GLOBAL IMS_SINT32
ImplicitNotifierState::STATE[SubState::STATE_MAX][ImplicitNotifierState::MESSAGE_MAX] =
{
    // STATE_INVALID
    {
        SubState::STATE_INVALID,
        SubState::STATE_INVALID,
        SubState::STATE_INVALID,
        SubState::STATE_INVALID,
        SubState::STATE_INVALID,
        SubState::STATE_INVALID,
        SubState::STATE_INVALID,
        SubState::STATE_INVALID,
        SubState::STATE_INVALID,
        SubState::STATE_INVALID,
        SubState::STATE_INVALID,
        SubState::STATE_INVALID,
        SubState::STATE_INVALID
    },
};

PUBLIC
ImplicitNotifierState::ImplicitNotifierState() :
        SubState()
{
    InitializeStateTable();
}

PUBLIC VIRTUAL ImplicitNotifierState::~ImplicitNotifierState() {}

PUBLIC VIRTUAL IMS_BOOL ImplicitNotifierState::UpdateState(IN CONST ISipMessage* piSIPMsg)
{
    //---------------------------------------------------------------------------------------------

    if (piSIPMsg == IMS_NULL)
    {
        return IMS_FALSE;
    }

    const SipMethod& objMethod = piSIPMsg->GetMethod();

    // Update the subscription state information...
    if (objMethod.Equals(SipMethod::REFER))
    {
        // On REFER request received ...
        if (piSIPMsg->GetType() == ISipMessage::TYPE_REQUEST)
        {
            if (!UpdateOnREFERRequest(piSIPMsg))
            {
                IMS_TRACE_E(0, "Updating the notifier state on REFER request failed", 0, 0, 0);
                return IMS_FALSE;
            }
        }
        // On REFER response sent ...
        else
        {
            // Reset the flag for subscription duration changed
            SetDurationUpdated(IMS_FALSE);

            if (!UpdateOnREFERResponse(piSIPMsg))
            {
                IMS_TRACE_E(0, "Updating the notifier state on REFER response failed", 0, 0, 0);
                return IMS_FALSE;
            }
        }
    }
    else if (objMethod.Equals(SipMethod::NOTIFY))
    {
        // On NOTIFY request sent ...
        if (piSIPMsg->GetType() == ISipMessage::TYPE_REQUEST)
        {
            // Reset the flag for subscription duration changed
            SetDurationUpdated(IMS_FALSE);

            if (!UpdateOnNOTIFYRequest(piSIPMsg))
            {
                IMS_TRACE_E(0, "Updating the notifier state on NOTIFY request failed", 0, 0, 0);
                return IMS_FALSE;
            }
        }
        // On NOTIFY response received ...
        else
        {
            if (!UpdateOnNOTIFYResponse(piSIPMsg))
            {
                IMS_TRACE_E(0, "Updating the notifier state on NOTIFY response failed", 0, 0, 0);
                return IMS_FALSE;
            }
        }
    }

    IMS_SINT32 nSIPMsg = TranslateMessage(piSIPMsg);

    if (nSIPMsg == MESSAGE_INVALID)
    {
        IMS_TRACE_I(
                "SUBS_STATE - NO TRANSITION (%s)", piSIPMsg->GetMethod().ToString().GetStr(), 0, 0);
        return IMS_TRUE;
    }

    // If the subscription is an outgoing and the current state map to a valid state transition,
    // then change the state.
    IMS_SINT32 nState = GetState();

    if (STATE[nState][nSIPMsg] != STATE_INVALID)
    {
        SetState(piSIPMsg, STATE[nState][nSIPMsg]);
    }

    return IMS_TRUE;
}

PRIVATE
IMS_SINT32 ImplicitNotifierState::TranslateMessage(IN CONST ISipMessage* piSIPMsg)
{
    IMS_SINT32 nMsgType = piSIPMsg->GetType();
    const SipMethod& objMethod = piSIPMsg->GetMethod();

    //---------------------------------------------------------------------------------------------

    if (nMsgType == ISipMessage::TYPE_REQUEST)
    {
        if (objMethod.Equals(SipMethod::REFER))
        {
            return MESSAGE_REFER;
        }
        else if (objMethod.Equals(SipMethod::NOTIFY))
        {
            IMS_SINT32 nSubStateValue = GetSubState();

            if (nSubStateValue == SUB_STATE_ACTIVE)
                return MESSAGE_NOTIFY_ACTIVE;
            else if (nSubStateValue == SUB_STATE_PENDING)
                return MESSAGE_NOTIFY_PENDING;
            else if (nSubStateValue == SUB_STATE_TERMINATED)
                return MESSAGE_NOTIFY_TERMINATED;
        }
    }
    else if (nMsgType == ISipMessage::TYPE_RESPONSE)
    {
        IMS_SINT32 nStatusCode = piSIPMsg->GetStatusCode();

        if (objMethod.Equals(SipMethod::REFER))
        {
            if (SipStatusCode::Is1XX(nStatusCode))
            {
                return MESSAGE_REFER_1XX;
            }
            else if (nStatusCode == SipStatusCode::SC_200)
            {
                return MESSAGE_REFER_200;
            }
            else if (SipStatusCode::IsFinalSuccess(nStatusCode))
            {
                return MESSAGE_REFER_202;
            }
            else
            {
                if (nStatusCode == SipStatusCode::SC_481)
                {
                    return MESSAGE_REFER_481;
                }
                else
                {
                    return MESSAGE_REFER_NON2XX;
                }
            }
        }
        else if (objMethod.Equals(SipMethod::NOTIFY))
        {
            if (SipStatusCode::Is1XX(nStatusCode))
            {
                return MESSAGE_NOTIFY_1XX;
            }
            else if (SipStatusCode::IsFinalSuccess(nStatusCode))
            {
                if (GetSubState() == SUB_STATE_TERMINATED)
                    return MESSAGE_NOTIFY_XXX_TERMINATED;
                else
                    return MESSAGE_NOTIFY_2XX;
            }
            else
            {
                if (GetSubState() == SUB_STATE_TERMINATED)
                    return MESSAGE_NOTIFY_XXX_TERMINATED;
                else
                    return MESSAGE_NOTIFY_NON2XX;
            }
        }
    }

    return MESSAGE_INVALID;
}

PRIVATE
IMS_BOOL ImplicitNotifierState::UpdateOnNOTIFYRequest(IN CONST ISipMessage* piSIPMsg)
{
    AString strHeader;
    ISipHeader* piHeader;
    EventPackage* pEventPackage = GetEventPackage();

    //---------------------------------------------------------------------------------------------

    // In such a case (of a subscription-creating NOTIFY), Event header needs to be updated.
    if (pEventPackage->GetEventHeader() == IMS_NULL)
    {
        if (!piSIPMsg->IsHeaderPresent(ISipHeader::EVENT))
        {
            IMS_TRACE_E(0, "Mandatory header missing : Event header", 0, 0, 0);
            return IMS_FALSE;
        }

        strHeader = piSIPMsg->GetHeader(ISipHeader::EVENT);
        piHeader = SipParsingHelper::CreateHeader(ISipHeader::EVENT, strHeader);

        pEventPackage->SetEventHeader(piHeader);
    }

    // Extracts a subs-state & "expires" parameter from Subscription-State header
    strHeader = piSIPMsg->GetHeader(ISipHeader::SUBSCRIPTION_STATE);
    piHeader = SipParsingHelper::CreateHeader(ISipHeader::SUBSCRIPTION_STATE, strHeader);

    if (piHeader == IMS_NULL)
    {
        IMS_TRACE_E(0, "Mandatory header missing : Subscription-State header", 0, 0, 0);
        return IMS_FALSE;
    }

    IMS_SINT32 nSubStateValue = ExtractSubStateValue(piHeader);

    if ((nSubStateValue == SUB_STATE_ACTIVE) || (nSubStateValue == SUB_STATE_PENDING))
    {
        if (GetState() != STATE_TERMINATED)
        {
            IMS_SINT32 nSubscriptionDuration = ExtractExpiresParameter(piHeader);

            // No "expires" parameter or parsing error ...
            if (nSubscriptionDuration == NO_EXPIRES)
            {
                if (GetState() == STATE_SUBSCRIBING)
                {
                    // Early NOTIFY or NOTIFY because of forked SUBSCRIBE
                    // which created a new subscription.
                    // Even though the 'expires' parameter is not present,
                    // we need to start a refresh timer.
                    nSubscriptionDuration = pEventPackage->GetDuration();

                    if (nSubscriptionDuration == NO_EXPIRES)
                    {
                        nSubscriptionDuration = pEventPackage->GetDefaultDuration();
                    }

                    // Start or Re-start the refresh timer
                    if (!IsInstantSubscription())
                    {
                        SetDurationUpdated(IMS_TRUE);
                    }
                }
            }
            // If "expires" value is (valid & zero) or (non-zero), do not start a refresh timer.
            else
            {
                // Stop the refresh timer
                if (!IsInstantSubscription())
                {
                    SetDurationUpdated(IMS_TRUE);
                }
            }

            SetDuration(nSubscriptionDuration);
        }
        else
        {
            // State in STATE_TERMINATED
            // We don't expect that NOTIFY (active/pending) request will not be received
            // after the subscription is in the TERMINATED state.

            SetDuration(0);
        }
    }
    else if (nSubStateValue == SUB_STATE_TERMINATED)
    {
        // sub-state is in TERMINATED & "terminated" received ... how to do ???

        SetDuration(0);

        // Stop the refresh timer
        SetDurationUpdated(IMS_TRUE);
    }

    SetSubState(nSubStateValue);

    piHeader->Destroy();

    return IMS_TRUE;
}

PRIVATE
IMS_BOOL ImplicitNotifierState::UpdateOnNOTIFYResponse(IN CONST ISipMessage* piSIPMsg)
{
    IMS_SINT32 nStatusCode = piSIPMsg->GetStatusCode();

    //---------------------------------------------------------------------------------------------

    if (SipStatusCode::Is1XX(nStatusCode))
    {
        // Do nothing ...
        return IMS_TRUE;
    }
    else if (SipStatusCode::IsFinalSuccess(nStatusCode))
    {
    }
    else if ((nStatusCode == SipStatusCode::SC_401) || (nStatusCode == SipStatusCode::SC_407))
    {
        // Do nothing ...
        return IMS_TRUE;
    }
    else
    {
        // A NOTIFY request is considered failed if the response times out,
        // or a non-200 class response code is received which has no "Retry-After" header
        // and no implied further action which can be taken to retry the request
        // (e.g. 401 Unauthorized).
        //
        // If the NOTIFY request fails (as defined above) due to a timeout condition,
        // and the subscription was installed using a soft-state mechanism (such as SUBSCRIBE),
        // the notifier SHOULD remove the subscription.
        //
        // If the NOTIFY request fails (as defined above) due to an error response,
        // and the subscription was installed using a soft-state mechanism,
        // the notifier MUST remove the corresponding subscription.
        //
        // If a NOTIFY request receives a 481 response,
        // the notifier MUST remove the corresponding subscription even if such subscription was
        // installed by non-SUBSCRIBE means (such as an administrative interface).

        if (nStatusCode == SipStatusCode::SC_481)
        {
            // Transit the state to TERMINATED
            SetState(piSIPMsg, STATE_TERMINATED);
        }
        else
        {
            // If no "Retry-After" header ...
            if (!piSIPMsg->IsHeaderPresent(ISipHeader::RETRY_AFTER_ANY))
            {
                // Transit the state to TERMINATED
                SetState(piSIPMsg, STATE_TERMINATED);
            }
        }
    }

    return IMS_TRUE;
}

PRIVATE
IMS_BOOL ImplicitNotifierState::UpdateOnREFERRequest(IN CONST ISipMessage* piSIPMsg)
{
    //---------------------------------------------------------------------------------------------

    // Extracts an Event header
    if (GetState() == STATE_INIT)
    {
        AString strHeader;

        if (!piSIPMsg->IsHeaderPresent(ISipHeader::EVENT))
        {
            IMS_TRACE_D("No Event header in REFER", 0, 0, 0);
            strHeader = Sip::STR_REFER;
        }
        else
        {
            strHeader = piSIPMsg->GetHeader(ISipHeader::EVENT);
        }

        ISipHeader* piHeader = SipParsingHelper::CreateHeader(ISipHeader::EVENT, strHeader);

        GetEventPackage()->SetEventHeader(piHeader);
    }

    return IMS_TRUE;
}

PRIVATE
IMS_BOOL ImplicitNotifierState::UpdateOnREFERResponse(IN CONST ISipMessage* piSIPMsg)
{
    //---------------------------------------------------------------------------------------------

    if (GetState() == STATE_TERMINATED)
    {
        IMS_TRACE_D("Subscription is already in TERMINATED state...", 0, 0, 0);
        return IMS_TRUE;
    }

    IMS_SINT32 nStatusCode = piSIPMsg->GetStatusCode();

    if (SipStatusCode::Is1XX(nStatusCode))
    {
        // Do nothing ...
        return IMS_TRUE;
    }
    else if (SipStatusCode::IsFinalSuccess(nStatusCode))
    {
    }
    else if ((nStatusCode == SipStatusCode::SC_401) || (nStatusCode == SipStatusCode::SC_407))
    {
        if (GetOperation() == OPERATION_REMOVE)
        {
            // Transit the state to TERMINATED
            SetState(piSIPMsg, STATE_TERMINATED);
        }
        else
        {
            // In case of an initial SUBSCRIBE request sent ...
            if (GetState() == STATE_TERMINATED)
            {
                SetState(piSIPMsg, STATE_INIT);
            }
        }

        return IMS_TRUE;
    }
    else
    {
        if (GetState() == STATE_SUBSCRIBING)
        {
            // Transit the state to TERMINATED
            SetState(piSIPMsg, STATE_TERMINATED);
        }
    }

    return IMS_TRUE;
}

PRIVATE GLOBAL void ImplicitNotifierState::InitializeStateTable()
{
    static IMS_BOOL bInitialized = IMS_FALSE;
    IMS_SINT32 i;
    IMS_SINT32 j;

    //---------------------------------------------------------------------------------------------

    if (bInitialized)
        return;

    // ONLY OUTGOING SUBSCRIPTION WILL BE CONCERNED ...
    for (i = 0; i < STATE_MAX; ++i)
    {
        for (j = 0; j < MESSAGE_MAX; ++j)
        {
            STATE[i][j] = STATE_INVALID;
        }
    }

    ///////////////////////////////////////////////////////////////////////
    //// Initialize the OUTGOING SUBSCRIPTION's state machine for message sent/received

    /////////////////////////////
    // SUBSCRIBER : STATE_INIT

    // REFER
    STATE[STATE_INIT][MESSAGE_REFER] = STATE_SUBSCRIBING;

    // NOTIFY

    ////////////////////////////////////////
    // SUBSCRIBER : STATE_SUBSCRIBING

    // REFER
    STATE[STATE_SUBSCRIBING][MESSAGE_REFER_200] = STATE_ACTIVE;
    STATE[STATE_SUBSCRIBING][MESSAGE_REFER_202] = STATE_PENDING;
    STATE[STATE_SUBSCRIBING][MESSAGE_REFER_481] = STATE_TERMINATED;
    STATE[STATE_SUBSCRIBING][MESSAGE_REFER_NON2XX] = STATE_TERMINATED;

    // NOTIFY
    STATE[STATE_SUBSCRIBING][MESSAGE_NOTIFY_ACTIVE] = STATE_ACTIVE;
    STATE[STATE_SUBSCRIBING][MESSAGE_NOTIFY_PENDING] = STATE_PENDING;
    STATE[STATE_SUBSCRIBING][MESSAGE_NOTIFY_TERMINATED] = STATE_TERMINATED;

    /////////////////////////////////
    // SUBSCRIBER : STATE_PENDING

    // REFER
    STATE[STATE_PENDING][MESSAGE_REFER_481] = STATE_TERMINATED;

    // NOTIFY
    STATE[STATE_PENDING][MESSAGE_NOTIFY_ACTIVE] = STATE_ACTIVE;
    STATE[STATE_PENDING][MESSAGE_NOTIFY_XXX_TERMINATED] = STATE_TERMINATED;
    STATE[STATE_PENDING][MESSAGE_NOTIFY_NON2XX] = STATE_TERMINATED;

    ////////////////////////////////
    // SUBSCRIBER : STATE_ACTIVE

    // REFER

    // NOTIFY
    STATE[STATE_ACTIVE][MESSAGE_NOTIFY_PENDING] = STATE_PENDING;
    STATE[STATE_ACTIVE][MESSAGE_NOTIFY_XXX_TERMINATED] = STATE_TERMINATED;
    STATE[STATE_ACTIVE][MESSAGE_NOTIFY_NON2XX] = STATE_TERMINATED;

    ////////////////////////////////////
    // SUBSCRIBER : STATE_TERMINATED

    // REFER

    // NOTIFY

    bInitialized = IMS_TRUE;
}
