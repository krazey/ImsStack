/*
    Author
    <table>
    date      author                    description
    --------  --------------            ----------
    20100328  hwangoo.park@             Created
    </table>

    Description

*/

#include "ServiceMemory.h"
#include "ServiceTrace.h"
#include "ISipHeader.h"
#include "Sip.h"
#include "SipHeaderName.h"
#include "SipParsingHelper.h"
#include "SipStatusCode.h"
#include "SubscriberState.h"

__IMS_TRACE_TAG_IMS_CORE__;



PUBLIC GLOBAL
IMS_SINT32 SubscriberState::STATE[SubState::STATE_MAX][SubscriberState::MESSAGE_MAX] =
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
        SubState::STATE_INVALID,
        SubState::STATE_INVALID,
        SubState::STATE_INVALID,
        SubState::STATE_INVALID,
        SubState::STATE_INVALID,
        SubState::STATE_INVALID,
        SubState::STATE_INVALID
    },
};

//4 send it to "Util" directory ?
PUBLIC GLOBAL
const SIPHeaderProperty SubscriberState::RESTRICTED_HEADER_PROPERTIES[] =
{
    // Header type, Header name, Is single header ?
    { ISIPHeader::ACCEPT_CONTACT, IMS_NULL, IMS_FALSE },
    { ISIPHeader::AUTHORIZATION, IMS_NULL, IMS_FALSE },
    { ISIPHeader::ALLOW, IMS_NULL, IMS_FALSE },
    { ISIPHeader::CALL_ID, IMS_NULL, IMS_TRUE },
    { ISIPHeader::CONTACT_ANY, IMS_NULL, IMS_FALSE },
    { ISIPHeader::CONTENT_LENGTH, IMS_NULL, IMS_TRUE },
    { ISIPHeader::CSEQ, IMS_NULL, IMS_TRUE },
    { ISIPHeader::FROM, IMS_NULL, IMS_TRUE },
    { ISIPHeader::MAX_FORWARDS, IMS_NULL, IMS_TRUE },
    { ISIPHeader::MIN_EXPIRES, IMS_NULL, IMS_TRUE },
    { ISIPHeader::P_ACCESS_NETWORK_INFO, IMS_NULL, IMS_TRUE },
    { ISIPHeader::P_ASSERTED_IDENTITY, IMS_NULL, IMS_FALSE },
    { ISIPHeader::P_PREFERRED_IDENTITY, IMS_NULL, IMS_FALSE },
    { ISIPHeader::PROXY_AUTHORIZATION, IMS_NULL, IMS_FALSE },
    { ISIPHeader::ROUTE, IMS_NULL, IMS_FALSE },
    { ISIPHeader::TO, IMS_NULL, IMS_TRUE },
    { ISIPHeader::SECURITY_CLIENT, IMS_NULL, IMS_FALSE },
    { ISIPHeader::SECURITY_VERIFY, IMS_NULL, IMS_FALSE },
    { ISIPHeader::VIA, IMS_NULL, IMS_FALSE },
    // SIP: Content-Length header is handled as unknown header
    { ISIPHeader::UNKNOWN, SIPHeaderName::CONTENT_LENGTH, IMS_TRUE },
    { ISIPHeader::UNKNOWN, "l", IMS_TRUE }
};



PUBLIC
SubscriberState::SubscriberState()
    : SubState()
{
    InitializeStateTable();
}

PUBLIC VIRTUAL
SubscriberState::~SubscriberState()
{
}

PUBLIC VIRTUAL
IMS_BOOL SubscriberState::UpdateState(IN CONST ISIPMessage *piSIPMsg)
{
    //---------------------------------------------------------------------------------------------

    if (piSIPMsg == IMS_NULL)
    {
        return IMS_FALSE;
    }

    const SIPMethod &objMethod = piSIPMsg->GetMethod();

    // Update the subscription state information...
    if (objMethod.Equals(SIPMethod::SUBSCRIBE))
    {
        // On SUBSCRIBE request sent ...
        if (piSIPMsg->GetType() == ISIPMessage::TYPE_REQUEST)
        {
            if (!UpdateOnSUBSCRIBERequest(piSIPMsg))
            {
                IMS_TRACE_E(0, "Updating the subscriber state on SUBSCRIBE request failed",
                        0, 0, 0);
                return IMS_FALSE;
            }
        }
        // On SUBSCRIBE response received ...
        else
        {
            // Reset the flag for subscription duration changed
            SetDurationUpdated(IMS_FALSE);

            if (!UpdateOnSUBSCRIBEResponse(piSIPMsg))
            {
                IMS_TRACE_E(0, "Updating the subscriber state on SUBSCRIBE response failed",
                        0, 0, 0);
                return IMS_FALSE;
            }
        }
    }
    else if (objMethod.Equals(SIPMethod::NOTIFY))
    {
        // On NOTIFY request received ...
        if (piSIPMsg->GetType() == ISIPMessage::TYPE_REQUEST)
        {
            // Reset the flag for subscription duration changed
            SetDurationUpdated(IMS_FALSE);

            if (!UpdateOnNOTIFYRequest(piSIPMsg))
            {
                IMS_TRACE_E(0, "Updating the subscriber state on NOTIFY request failed", 0, 0, 0);
                return IMS_FALSE;
            }
        }
        // On NOTIFY response sent ...
        else
        {
            if (!UpdateOnNOTIFYResponse(piSIPMsg))
            {
                IMS_TRACE_E(0, "Updating the subscriber state on NOTIFY response failed", 0, 0, 0);
                return IMS_FALSE;
            }
        }
    }

    IMS_SINT32 nSIPMsg = TranslateMessage(piSIPMsg);

    if (nSIPMsg == MESSAGE_INVALID)
    {
        IMS_TRACE_I("SUBS_STATE - NO TRANSITION (%s)",
                piSIPMsg->GetMethod().ToString().GetStr(), 0, 0);
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

PROTECTED VIRTUAL
const SIPHeaderProperty* SubscriberState::GetRestrictedHeaders(OUT IMS_UINT32 &nCount) const
{
    //---------------------------------------------------------------------------------------------

    nCount = sizeof(RESTRICTED_HEADER_PROPERTIES) / sizeof(RESTRICTED_HEADER_PROPERTIES[0]);

    return &(RESTRICTED_HEADER_PROPERTIES[0]);
}

PRIVATE
IMS_SINT32 SubscriberState::TranslateMessage(IN CONST ISIPMessage *piSIPMsg)
{
    IMS_SINT32 nMsgType = piSIPMsg->GetType();
    const SIPMethod &objMethod = piSIPMsg->GetMethod();

    //---------------------------------------------------------------------------------------------

    if (nMsgType == ISIPMessage::TYPE_REQUEST)
    {
        if (objMethod.Equals(SIPMethod::SUBSCRIBE))
        {
            if (GetState() == STATE_INIT)
                return MESSAGE_SUBSCRIBE;
            else
                return MESSAGE_RESUBSCRIBE;
        }
        else if (objMethod.Equals(SIPMethod::NOTIFY))
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
    else if (nMsgType == ISIPMessage::TYPE_RESPONSE)
    {
        IMS_SINT32 nStatusCode = piSIPMsg->GetStatusCode();

        if (objMethod.Equals(SIPMethod::SUBSCRIBE))
        {
            IMS_SINT32 nState = GetState();

            if (SIPStatusCode::Is1XX(nStatusCode))
            {
                if (nState == STATE_SUBSCRIBING)
                    return MESSAGE_SUBSCRIBE_1XX;
                else if ((nState == STATE_ACTIVE) || (nState == STATE_PENDING))
                    return MESSAGE_RESUBSCRIBE_1XX;
            }
            else if (nStatusCode == SIPStatusCode::SC_200)
            {
                if (nState == STATE_SUBSCRIBING)
                    return MESSAGE_SUBSCRIBE_200;
                else if ((nState == STATE_ACTIVE) || (nState == STATE_PENDING))
                    return MESSAGE_RESUBSCRIBE_200;
            }
            else if (SIPStatusCode::IsFinalSuccess(nStatusCode))
            {
                if (nState == STATE_SUBSCRIBING)
                    return MESSAGE_SUBSCRIBE_202;
                else if ((nState == STATE_ACTIVE) || (nState == STATE_PENDING))
                    return MESSAGE_RESUBSCRIBE_202;
            }
            else
            {
                if (nStatusCode == SIPStatusCode::SC_481)
                {
                    if (nState == STATE_SUBSCRIBING)
                        return MESSAGE_SUBSCRIBE_481;
                    else if ((nState == STATE_ACTIVE) || (nState == STATE_PENDING))
                        return MESSAGE_RESUBSCRIBE_481;
                }
                else
                {
                    if (nState == STATE_SUBSCRIBING)
                        return MESSAGE_SUBSCRIBE_NON2XX;
                    else if ((nState == STATE_ACTIVE) || (nState == STATE_PENDING))
                        return MESSAGE_RESUBSCRIBE_NON2XX;
                }
            }
        }
        else if (objMethod.Equals(SIPMethod::NOTIFY))
        {
            if (SIPStatusCode::Is1XX(nStatusCode))
            {
                return MESSAGE_NOTIFY_1XX;
            }
            else if (SIPStatusCode::IsFinalSuccess(nStatusCode))
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
IMS_BOOL SubscriberState::UpdateOnNOTIFYRequest(IN CONST ISIPMessage *piSIPMsg)
{
    AString strHeader;
    ISIPHeader *piHeader;
    EventPackage *pEventPackage = GetEventPackage();

    //---------------------------------------------------------------------------------------------

    // A forked SUBSCRIBE can result in a NOTIFYs from different Notifiers each of which
    // could result in creation of a subscription.
    // In such a case (of a subscription-creating NOTIFY), Event header needs to be updated.
    if (pEventPackage->GetEventHeader() == IMS_NULL)
    {
        if (!piSIPMsg->IsHeaderPresent(ISIPHeader::EVENT))
        {
            IMS_TRACE_E(0, "Mandatory header missing : Event header", 0, 0, 0);
            return IMS_FALSE;
        }

        strHeader = piSIPMsg->GetHeader(ISIPHeader::EVENT);
        piHeader = SIPParsingHelper::CreateHeader(ISIPHeader::EVENT, strHeader);

        pEventPackage->SetEventHeader(piHeader);
    }

    // Extracts a subs-state & "expires" parameter from Subscription-State header
    strHeader = piSIPMsg->GetHeader(ISIPHeader::SUBSCRIPTION_STATE);
    piHeader = SIPParsingHelper::CreateHeader(ISIPHeader::SUBSCRIPTION_STATE, strHeader);

    if (piHeader == IMS_NULL)
    {
        IMS_TRACE_E(0, "Mandatory header missing : Subscription-State header", 0, 0, 0);
        return IMS_FALSE;
    }

    IMS_SINT32 nSubStateValue = ExtractSubStateValue(piHeader);

    if ((nSubStateValue == SUB_STATE_ACTIVE)
            || (nSubStateValue == SUB_STATE_PENDING))
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

                    SetDuration(nSubscriptionDuration);
                }
                else
                {
                    // Do nothing regarding to the refresh timer
                    // Keep the previous expires timer
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

                SetDuration(nSubscriptionDuration);
            }
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
IMS_BOOL SubscriberState::UpdateOnNOTIFYResponse(IN CONST ISIPMessage *piSIPMsg)
{
    IMS_SINT32 nStatusCode = piSIPMsg->GetStatusCode();

    //---------------------------------------------------------------------------------------------

    if (SIPStatusCode::Is1XX(nStatusCode))
    {
        // Do nothing ...
        return IMS_TRUE;
    }
    else if (SIPStatusCode::IsFinalSuccess(nStatusCode))
    {
    }
    else if ((nStatusCode == SIPStatusCode::SC_401)
            || (nStatusCode == SIPStatusCode::SC_407))
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

        if (nStatusCode == SIPStatusCode::SC_481)
        {
            // Transit the state to TERMINATED
            SetState(piSIPMsg, STATE_TERMINATED);
        }
        else
        {
            // If no "Retry-After" header ...
            if (!piSIPMsg->IsHeaderPresent(ISIPHeader::RETRY_AFTER_ANY))
            {
                // Transit the state to TERMINATED
                SetState(piSIPMsg, STATE_TERMINATED);
            }
        }
    }

    return IMS_TRUE;
}

PRIVATE
IMS_BOOL SubscriberState::UpdateOnSUBSCRIBERequest(IN CONST ISIPMessage *piSIPMsg)
{
    AString strHeader;
    ISIPHeader *piHeader;
    EventPackage *pEventPackage = GetEventPackage();

    //---------------------------------------------------------------------------------------------

    // Extracts a duartion of subscription from Expires header
    if (piSIPMsg->IsHeaderPresent(ISIPHeader::EXPIRES_ANY))
    {
        strHeader = piSIPMsg->GetHeader(ISIPHeader::EXPIRES_ANY);
        piHeader = SIPParsingHelper::CreateHeader(ISIPHeader::EXPIRES_ANY, strHeader);

        if (piHeader != IMS_NULL)
        {
            IMS_SINT32 nSubscriptionDuration = piHeader->GetValueInt();

            // Stores the Expires value in the initial SUBSCRIBE request...
            if (GetState() == STATE_INIT)
            {
                pEventPackage->SetDuration(nSubscriptionDuration);

                // Instant subscription: it's for fetching the event state at once.
                if (nSubscriptionDuration == 0)
                {
                    SetInstantSubscription(IMS_TRUE);
                }
            }

            SetDuration(nSubscriptionDuration);

            piHeader->Destroy();
        }
    }

    // Extracts an Event header
    if (GetState() == STATE_INIT)
    {
        if (!piSIPMsg->IsHeaderPresent(ISIPHeader::EVENT))
        {
            IMS_TRACE_E(0, "Mandatory header missing : Event header", 0, 0, 0);
            return IMS_FALSE;
        }

        strHeader = piSIPMsg->GetHeader(ISIPHeader::EVENT);
        piHeader = SIPParsingHelper::CreateHeader(ISIPHeader::EVENT, strHeader);

        pEventPackage->SetEventHeader(piHeader);
    }

    // Stores an initial/refresh SUBSCRIBE request for refresh/removal operation
    if ((GetOperation() == OPERATION_CREATE)
            || (GetOperation() == OPERATION_REFRESH))
    {
        StoreMessage(piSIPMsg);
    }

    return IMS_TRUE;
}


PRIVATE
IMS_BOOL SubscriberState::UpdateOnSUBSCRIBEResponse(IN CONST ISIPMessage *piSIPMsg)
{
    //---------------------------------------------------------------------------------------------

    if (GetState() == STATE_TERMINATED)
    {
        IMS_TRACE_D("Subscription is already in TERMINATED state...", 0, 0, 0);
        return IMS_TRUE;
    }

    IMS_SINT32 nStatusCode = piSIPMsg->GetStatusCode();

    if (SIPStatusCode::Is1XX(nStatusCode))
    {
        // Do nothing ...
        return IMS_TRUE;
    }
    else if (SIPStatusCode::IsFinalSuccess(nStatusCode))
    {
        // Extracts a duartion of subscription from Expires header
        if (piSIPMsg->IsHeaderPresent(ISIPHeader::EXPIRES_ANY))
        {
            AString strHeader = piSIPMsg->GetHeader(ISIPHeader::EXPIRES_ANY);
            ISIPHeader *piHeader
                    = SIPParsingHelper::CreateHeader(ISIPHeader::EXPIRES_ANY, strHeader);

            if (piHeader != IMS_NULL)
            {
                IMS_SINT32 nSubscriptionDuration = piHeader->GetValueInt();

                if (nSubscriptionDuration < 0)
                {
                    nSubscriptionDuration = 0;

                    // Transit the state to TERMINATED
                    SetState(piSIPMsg, STATE_TERMINATED);

                    // Start or Stop the refresh timer
                    SetDurationUpdated(IMS_TRUE);
                }
                else
                {
                    if (!IsInstantSubscription())
                    {
                        // Start or Stop the refresh timer
                        SetDurationUpdated(IMS_TRUE);
                    }
                }

                SetDuration(nSubscriptionDuration);

                piHeader->Destroy();
            }
        }
        else if (IsConfigurationSet(CONFIG_USE_INITIAL_EXPIRES_ON_NO_EXPIRES_IN_200_OK))
        {
            if (GetState() == STATE_SUBSCRIBING)
            {
                EventPackage* pEventPackage = GetEventPackage();

                if (pEventPackage->GetDuration() <= 0)
                {
                    // Transit the state to TERMINATED
                    SetState(piSIPMsg, STATE_TERMINATED);
                }
                else
                {
                    // Start or Stop the refresh timer
                    SetDurationUpdated(IMS_TRUE);
                    SetDuration(pEventPackage->GetDuration());
                }
            }
            else
            {
                // Keep the previous refresh timer (initial one or expires in NOTIFY request)
            }
        }
        else
        {
            // Transit the state to TERMINATED
            SetState(piSIPMsg, STATE_TERMINATED);
        }
    }
    else if ((nStatusCode == SIPStatusCode::SC_401)
            || (nStatusCode == SIPStatusCode::SC_407))
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

PRIVATE GLOBAL
void SubscriberState::InitializeStateTable()
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

    // SUBSCRIBE
    STATE[STATE_INIT][MESSAGE_SUBSCRIBE] = STATE_SUBSCRIBING;

    // refresh SUBSCRIBE

    // NOTIFY

    ////////////////////////////////////////
    // SUBSCRIBER : STATE_SUBSCRIBING

    // SUBSCRIBE
    STATE[STATE_SUBSCRIBING][MESSAGE_SUBSCRIBE_200] = STATE_ACTIVE;
    STATE[STATE_SUBSCRIBING][MESSAGE_SUBSCRIBE_202] = STATE_PENDING;
    STATE[STATE_SUBSCRIBING][MESSAGE_SUBSCRIBE_481] = STATE_INIT;
    STATE[STATE_SUBSCRIBING][MESSAGE_SUBSCRIBE_NON2XX] = STATE_INIT;

    // refresh SUBSCRIBE

    // NOTIFY
    STATE[STATE_SUBSCRIBING][MESSAGE_NOTIFY_ACTIVE] = STATE_ACTIVE;
    STATE[STATE_SUBSCRIBING][MESSAGE_NOTIFY_PENDING] = STATE_PENDING;
    STATE[STATE_SUBSCRIBING][MESSAGE_NOTIFY_TERMINATED] = STATE_TERMINATED;

    /////////////////////////////////
    // SUBSCRIBER : STATE_PENDING

    // SUBSCRIBE
    STATE[STATE_PENDING][MESSAGE_SUBSCRIBE_481] = STATE_TERMINATED;

    // refresh SUBSCRIBE
    STATE[STATE_PENDING][MESSAGE_RESUBSCRIBE_200] = STATE_ACTIVE;
    STATE[STATE_PENDING][MESSAGE_RESUBSCRIBE_481] = STATE_TERMINATED;

    // NOTIFY
    STATE[STATE_PENDING][MESSAGE_NOTIFY_ACTIVE] = STATE_ACTIVE;
    STATE[STATE_PENDING][MESSAGE_NOTIFY_XXX_TERMINATED] = STATE_TERMINATED;
    STATE[STATE_PENDING][MESSAGE_NOTIFY_NON2XX] = STATE_TERMINATED;

    ////////////////////////////////
    // SUBSCRIBER : STATE_ACTIVE

    // SUBSCRIBE

    // refresh SUBSCRIBE
    STATE[STATE_ACTIVE][MESSAGE_RESUBSCRIBE_202] = STATE_PENDING;
    STATE[STATE_ACTIVE][MESSAGE_RESUBSCRIBE_481] = STATE_TERMINATED;

    // NOTIFY
    STATE[STATE_ACTIVE][MESSAGE_NOTIFY_PENDING] = STATE_PENDING;
    STATE[STATE_ACTIVE][MESSAGE_NOTIFY_XXX_TERMINATED] = STATE_TERMINATED;
    STATE[STATE_ACTIVE][MESSAGE_NOTIFY_NON2XX] = STATE_TERMINATED;

    ////////////////////////////////////
    // SUBSCRIBER : STATE_TERMINATED

    // SUBSCRIBE

    // refresh SUBSCRIBE

    // NOTIFY

    bInitialized = IMS_TRUE;
}
