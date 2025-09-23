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

#include "ISipHeader.h"
#include "Sip.h"
#include "SipHeaderName.h"
#include "SipHeaderProperty.h"
#include "SipParsingHelper.h"
#include "SipStatusCode.h"
#include "SubscriberState.h"

__IMS_TRACE_TAG_IMS_CORE__;

// clang-format off
PUBLIC GLOBAL
IMS_SINT32 SubscriberState::s_nState[SubState::STATE_MAX][SubscriberState::MESSAGE_MAX] =
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

// 4 send it to "Util" directory ?
PUBLIC GLOBAL
const SipHeaderProperty SubscriberState::RESTRICTED_HEADER_PROPERTIES[] =
{
        // Header name, Header type, Is single header ?
        {IMS_NULL,                      ISipHeader::ACCEPT_CONTACT,        IMS_FALSE},
        {IMS_NULL,                      ISipHeader::AUTHORIZATION,         IMS_FALSE},
        {IMS_NULL,                      ISipHeader::ALLOW,                 IMS_FALSE},
        {IMS_NULL,                      ISipHeader::CALL_ID,               IMS_TRUE },
        {IMS_NULL,                      ISipHeader::CONTACT_ANY,           IMS_FALSE},
        {IMS_NULL,                      ISipHeader::CONTENT_LENGTH,        IMS_TRUE },
        {IMS_NULL,                      ISipHeader::CSEQ,                  IMS_TRUE },
        {IMS_NULL,                      ISipHeader::FROM,                  IMS_TRUE },
        {IMS_NULL,                      ISipHeader::MAX_FORWARDS,          IMS_TRUE },
        {IMS_NULL,                      ISipHeader::MIN_EXPIRES,           IMS_TRUE },
        {IMS_NULL,                      ISipHeader::P_ACCESS_NETWORK_INFO, IMS_TRUE },
        {IMS_NULL,                      ISipHeader::P_ASSERTED_IDENTITY,   IMS_FALSE},
        {IMS_NULL,                      ISipHeader::P_PREFERRED_IDENTITY,  IMS_FALSE},
        {IMS_NULL,                      ISipHeader::PROXY_AUTHORIZATION,   IMS_FALSE},
        {IMS_NULL,                      ISipHeader::ROUTE,                 IMS_FALSE},
        {IMS_NULL,                      ISipHeader::TO,                    IMS_TRUE },
        {IMS_NULL,                      ISipHeader::SECURITY_CLIENT,       IMS_FALSE},
        {IMS_NULL,                      ISipHeader::SECURITY_VERIFY,       IMS_FALSE},
        {IMS_NULL,                      ISipHeader::VIA,                   IMS_FALSE},
        // SIP: Content-Length header is handled as unknown header
        {SipHeaderName::CONTENT_LENGTH, ISipHeader::UNKNOWN,               IMS_TRUE },
        {"l",                           ISipHeader::UNKNOWN,               IMS_TRUE }
};
// clang-format on

PUBLIC
SubscriberState::SubscriberState() :
        SubState()
{
    InitializeStateTable();
}

PUBLIC VIRTUAL IMS_BOOL SubscriberState::UpdateState(IN const ISipMessage* piSipMsg)
{
    if (piSipMsg == IMS_NULL)
    {
        return IMS_FALSE;
    }

    const SipMethod& objMethod = piSipMsg->GetMethod();

    // Update the subscription state information...
    if (objMethod.Equals(SipMethod::SUBSCRIBE))
    {
        // On SUBSCRIBE request sent ...
        if (piSipMsg->GetType() == ISipMessage::TYPE_REQUEST)
        {
            if (!UpdateOnSubscribeRequest(piSipMsg))
            {
                IMS_TRACE_E(
                        0, "Updating the subscriber state on SUBSCRIBE request failed", 0, 0, 0);
                return IMS_FALSE;
            }
        }
        // On SUBSCRIBE response received ...
        else
        {
            // Reset the flag for subscription duration changed
            SetDurationUpdated(IMS_FALSE);

            if (!UpdateOnSubscribeResponse(piSipMsg))
            {
                IMS_TRACE_E(
                        0, "Updating the subscriber state on SUBSCRIBE response failed", 0, 0, 0);
                return IMS_FALSE;
            }
        }
    }
    else if (objMethod.Equals(SipMethod::NOTIFY))
    {
        // On NOTIFY request received ...
        if (piSipMsg->GetType() == ISipMessage::TYPE_REQUEST)
        {
            // Reset the flag for subscription duration changed
            SetDurationUpdated(IMS_FALSE);

            if (!UpdateOnNotifyRequest(piSipMsg))
            {
                IMS_TRACE_E(0, "Updating the subscriber state on NOTIFY request failed", 0, 0, 0);
                return IMS_FALSE;
            }
        }
        // On NOTIFY response sent ...
        else
        {
            if (!UpdateOnNotifyResponse(piSipMsg))
            {
                IMS_TRACE_E(0, "Updating the subscriber state on NOTIFY response failed", 0, 0, 0);
                return IMS_FALSE;
            }
        }
    }

    IMS_SINT32 nSipMsg = TranslateMessage(piSipMsg);

    if (nSipMsg == MESSAGE_INVALID)
    {
        IMS_TRACE_I(
                "SUBS_STATE - NO TRANSITION (%s)", piSipMsg->GetMethod().ToString().GetStr(), 0, 0);
        return IMS_TRUE;
    }

    // If the subscription is an outgoing and the current state map to a valid state transition,
    // then change the state.
    IMS_SINT32 nState = GetState();

    if (s_nState[nState][nSipMsg] != STATE_INVALID)
    {
        SetState(piSipMsg, s_nState[nState][nSipMsg]);
    }

    return IMS_TRUE;
}

PROTECTED VIRTUAL const SipHeaderProperty* SubscriberState::GetRestrictedHeaders(
        OUT IMS_UINT32& nCount) const
{
    nCount = sizeof(RESTRICTED_HEADER_PROPERTIES) / sizeof(RESTRICTED_HEADER_PROPERTIES[0]);

    return &(RESTRICTED_HEADER_PROPERTIES[0]);
}

PRIVATE
IMS_SINT32 SubscriberState::TranslateMessage(IN const ISipMessage* piSipMsg) const
{
    IMS_SINT32 nMsgType = piSipMsg->GetType();
    const SipMethod& objMethod = piSipMsg->GetMethod();

    if (nMsgType == ISipMessage::TYPE_REQUEST)
    {
        if (objMethod.Equals(SipMethod::SUBSCRIBE))
        {
            if (GetState() == STATE_INIT)
            {
                return MESSAGE_SUBSCRIBE;
            }
            else
            {
                return MESSAGE_RESUBSCRIBE;
            }
        }
        else if (objMethod.Equals(SipMethod::NOTIFY))
        {
            IMS_SINT32 nSubStateValue = GetSubState();

            if (nSubStateValue == SUB_STATE_ACTIVE)
            {
                return MESSAGE_NOTIFY_ACTIVE;
            }
            else if (nSubStateValue == SUB_STATE_PENDING)
            {
                return MESSAGE_NOTIFY_PENDING;
            }
            else if (nSubStateValue == SUB_STATE_TERMINATED)
            {
                return MESSAGE_NOTIFY_TERMINATED;
            }
        }
    }
    else if (nMsgType == ISipMessage::TYPE_RESPONSE)
    {
        IMS_SINT32 nStatusCode = piSipMsg->GetStatusCode();

        if (objMethod.Equals(SipMethod::SUBSCRIBE))
        {
            IMS_SINT32 nState = GetState();

            if (SipStatusCode::Is1XX(nStatusCode))
            {
                if (nState == STATE_SUBSCRIBING)
                {
                    return MESSAGE_SUBSCRIBE_1XX;
                }
                else if ((nState == STATE_ACTIVE) || (nState == STATE_PENDING))
                {
                    return MESSAGE_RESUBSCRIBE_1XX;
                }
            }
            else if (nStatusCode == SipStatusCode::SC_200)
            {
                if (nState == STATE_SUBSCRIBING)
                {
                    return MESSAGE_SUBSCRIBE_200;
                }
                else if ((nState == STATE_ACTIVE) || (nState == STATE_PENDING))
                {
                    return MESSAGE_RESUBSCRIBE_200;
                }
            }
            else if (SipStatusCode::IsFinalSuccess(nStatusCode))
            {
                if (nState == STATE_SUBSCRIBING)
                {
                    return MESSAGE_SUBSCRIBE_202;
                }
                else if ((nState == STATE_ACTIVE) || (nState == STATE_PENDING))
                {
                    return MESSAGE_RESUBSCRIBE_202;
                }
            }
            else
            {
                if (nStatusCode == SipStatusCode::SC_481)
                {
                    if (nState == STATE_SUBSCRIBING)
                    {
                        return MESSAGE_SUBSCRIBE_481;
                    }
                    else if ((nState == STATE_ACTIVE) || (nState == STATE_PENDING))
                    {
                        return MESSAGE_RESUBSCRIBE_481;
                    }
                }
                else
                {
                    if (nState == STATE_SUBSCRIBING)
                    {
                        return MESSAGE_SUBSCRIBE_NON2XX;
                    }
                    else if ((nState == STATE_ACTIVE) || (nState == STATE_PENDING))
                    {
                        return MESSAGE_RESUBSCRIBE_NON2XX;
                    }
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
                {
                    return MESSAGE_NOTIFY_XXX_TERMINATED;
                }
                else
                {
                    return MESSAGE_NOTIFY_2XX;
                }
            }
            else
            {
                if (GetSubState() == SUB_STATE_TERMINATED)
                {
                    return MESSAGE_NOTIFY_XXX_TERMINATED;
                }
                else
                {
                    return MESSAGE_NOTIFY_NON2XX;
                }
            }
        }
    }

    return MESSAGE_INVALID;
}

PRIVATE
IMS_BOOL SubscriberState::UpdateOnNotifyRequest(IN const ISipMessage* piSipMsg)
{
    AString strHeader;
    ISipHeader* piHeader;
    EventPackage* pEventPackage = GetEventPackage();

    // A forked SUBSCRIBE can result in a NOTIFYs from different Notifiers each of which
    // could result in creation of a subscription.
    // In such a case (of a subscription-creating NOTIFY), Event header needs to be updated.
    if (pEventPackage->GetEventHeader() == IMS_NULL)
    {
        if (!piSipMsg->IsHeaderPresent(ISipHeader::EVENT))
        {
            IMS_TRACE_E(0, "Mandatory header missing : Event header", 0, 0, 0);
            return IMS_FALSE;
        }

        strHeader = piSipMsg->GetHeader(ISipHeader::EVENT);
        piHeader = SipParsingHelper::CreateHeader(ISipHeader::EVENT, strHeader);

        pEventPackage->SetEventHeader(piHeader);
    }

    // Extracts a subs-state & "expires" parameter from Subscription-State header
    strHeader = piSipMsg->GetHeader(ISipHeader::SUBSCRIPTION_STATE);
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
IMS_BOOL SubscriberState::UpdateOnNotifyResponse(IN const ISipMessage* piSipMsg)
{
    IMS_SINT32 nStatusCode = piSipMsg->GetStatusCode();

    if (SipStatusCode::Is1XX(nStatusCode) || SipStatusCode::IsFinalSuccess(nStatusCode) ||
            (nStatusCode == SipStatusCode::SC_401) || (nStatusCode == SipStatusCode::SC_407))
    {
        // Do nothing ...
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
            SetState(piSipMsg, STATE_TERMINATED);
        }
        else
        {
            // If no "Retry-After" header ...
            if (!piSipMsg->IsHeaderPresent(ISipHeader::RETRY_AFTER_ANY))
            {
                // Transit the state to TERMINATED
                SetState(piSipMsg, STATE_TERMINATED);
            }
        }
    }

    return IMS_TRUE;
}

PRIVATE
IMS_BOOL SubscriberState::UpdateOnSubscribeRequest(IN const ISipMessage* piSipMsg)
{
    AString strHeader;
    ISipHeader* piHeader;
    EventPackage* pEventPackage = GetEventPackage();

    // Extracts a duartion of subscription from Expires header
    if (piSipMsg->IsHeaderPresent(ISipHeader::EXPIRES_ANY))
    {
        strHeader = piSipMsg->GetHeader(ISipHeader::EXPIRES_ANY);
        piHeader = SipParsingHelper::CreateHeader(ISipHeader::EXPIRES_ANY, strHeader);

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
        if (!piSipMsg->IsHeaderPresent(ISipHeader::EVENT))
        {
            IMS_TRACE_E(0, "Mandatory header missing : Event header", 0, 0, 0);
            return IMS_FALSE;
        }

        strHeader = piSipMsg->GetHeader(ISipHeader::EVENT);
        piHeader = SipParsingHelper::CreateHeader(ISipHeader::EVENT, strHeader);

        pEventPackage->SetEventHeader(piHeader);
    }

    // Stores an initial/refresh SUBSCRIBE request for refresh/removal operation
    if ((GetOperation() == OPERATION_CREATE) || (GetOperation() == OPERATION_REFRESH))
    {
        StoreMessage(piSipMsg);
    }

    return IMS_TRUE;
}

PRIVATE
IMS_BOOL SubscriberState::UpdateOnSubscribeResponse(IN const ISipMessage* piSipMsg)
{
    if (GetState() == STATE_TERMINATED)
    {
        IMS_TRACE_D("Subscription is already in TERMINATED state...", 0, 0, 0);
        return IMS_TRUE;
    }

    IMS_SINT32 nStatusCode = piSipMsg->GetStatusCode();

    if (SipStatusCode::Is1XX(nStatusCode))
    {
        // Do nothing ...
        return IMS_TRUE;
    }
    else if (SipStatusCode::IsFinalSuccess(nStatusCode))
    {
        // Extracts a duartion of subscription from Expires header
        if (piSipMsg->IsHeaderPresent(ISipHeader::EXPIRES_ANY))
        {
            AString strHeader = piSipMsg->GetHeader(ISipHeader::EXPIRES_ANY);
            ISipHeader* piHeader =
                    SipParsingHelper::CreateHeader(ISipHeader::EXPIRES_ANY, strHeader);

            if (piHeader != IMS_NULL)
            {
                IMS_SINT32 nSubscriptionDuration = piHeader->GetValueInt();

                if (nSubscriptionDuration < 0)
                {
                    nSubscriptionDuration = 0;

                    // Transit the state to TERMINATED
                    SetState(piSipMsg, STATE_TERMINATED);

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
                const EventPackage* pEventPackage = GetEventPackage();

                if (pEventPackage->GetDuration() <= 0)
                {
                    // Transit the state to TERMINATED
                    SetState(piSipMsg, STATE_TERMINATED);
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
            SetState(piSipMsg, STATE_TERMINATED);
        }
    }
    else if ((nStatusCode == SipStatusCode::SC_401) || (nStatusCode == SipStatusCode::SC_407))
    {
        if (GetOperation() == OPERATION_REMOVE)
        {
            // Transit the state to TERMINATED
            SetState(piSipMsg, STATE_TERMINATED);
        }
        else
        {
            // In case of an initial SUBSCRIBE request sent ...
            if (GetState() == STATE_TERMINATED)
            {
                SetState(piSipMsg, STATE_INIT);
            }
        }

        return IMS_TRUE;
    }
    else
    {
        if (GetState() == STATE_SUBSCRIBING)
        {
            // Transit the state to TERMINATED
            SetState(piSipMsg, STATE_TERMINATED);
        }
    }

    return IMS_TRUE;
}

PRIVATE GLOBAL void SubscriberState::InitializeStateTable()
{
    static IMS_BOOL bInitialized = IMS_FALSE;
    IMS_SINT32 i;
    IMS_SINT32 j;

    if (bInitialized)
    {
        return;
    }

    // ONLY OUTGOING SUBSCRIPTION WILL BE CONCERNED ...
    for (i = 0; i < STATE_MAX; ++i)
    {
        for (j = 0; j < MESSAGE_MAX; ++j)
        {
            s_nState[i][j] = STATE_INVALID;
        }
    }

    ///////////////////////////////////////////////////////////////////////
    //// Initialize the OUTGOING SUBSCRIPTION's state machine for message sent/received

    /////////////////////////////
    // SUBSCRIBER : STATE_INIT

    // SUBSCRIBE
    s_nState[STATE_INIT][MESSAGE_SUBSCRIBE] = STATE_SUBSCRIBING;

    // refresh SUBSCRIBE

    // NOTIFY

    ////////////////////////////////////////
    // SUBSCRIBER : STATE_SUBSCRIBING

    // SUBSCRIBE
    s_nState[STATE_SUBSCRIBING][MESSAGE_SUBSCRIBE_200] = STATE_ACTIVE;
    s_nState[STATE_SUBSCRIBING][MESSAGE_SUBSCRIBE_202] = STATE_PENDING;
    s_nState[STATE_SUBSCRIBING][MESSAGE_SUBSCRIBE_481] = STATE_INIT;
    s_nState[STATE_SUBSCRIBING][MESSAGE_SUBSCRIBE_NON2XX] = STATE_INIT;

    // refresh SUBSCRIBE

    // NOTIFY
    s_nState[STATE_SUBSCRIBING][MESSAGE_NOTIFY_ACTIVE] = STATE_ACTIVE;
    s_nState[STATE_SUBSCRIBING][MESSAGE_NOTIFY_PENDING] = STATE_PENDING;
    s_nState[STATE_SUBSCRIBING][MESSAGE_NOTIFY_TERMINATED] = STATE_TERMINATED;

    /////////////////////////////////
    // SUBSCRIBER : STATE_PENDING

    // SUBSCRIBE
    s_nState[STATE_PENDING][MESSAGE_SUBSCRIBE_481] = STATE_TERMINATED;

    // refresh SUBSCRIBE
    s_nState[STATE_PENDING][MESSAGE_RESUBSCRIBE_200] = STATE_ACTIVE;
    s_nState[STATE_PENDING][MESSAGE_RESUBSCRIBE_481] = STATE_TERMINATED;

    // NOTIFY
    s_nState[STATE_PENDING][MESSAGE_NOTIFY_ACTIVE] = STATE_ACTIVE;
    s_nState[STATE_PENDING][MESSAGE_NOTIFY_XXX_TERMINATED] = STATE_TERMINATED;
    s_nState[STATE_PENDING][MESSAGE_NOTIFY_NON2XX] = STATE_TERMINATED;

    ////////////////////////////////
    // SUBSCRIBER : STATE_ACTIVE

    // SUBSCRIBE

    // refresh SUBSCRIBE
    s_nState[STATE_ACTIVE][MESSAGE_RESUBSCRIBE_202] = STATE_PENDING;
    s_nState[STATE_ACTIVE][MESSAGE_RESUBSCRIBE_481] = STATE_TERMINATED;

    // NOTIFY
    s_nState[STATE_ACTIVE][MESSAGE_NOTIFY_PENDING] = STATE_PENDING;
    s_nState[STATE_ACTIVE][MESSAGE_NOTIFY_XXX_TERMINATED] = STATE_TERMINATED;
    s_nState[STATE_ACTIVE][MESSAGE_NOTIFY_NON2XX] = STATE_TERMINATED;

    ////////////////////////////////////
    // SUBSCRIBER : STATE_TERMINATED

    // SUBSCRIBE

    // refresh SUBSCRIBE

    // NOTIFY

    bInitialized = IMS_TRUE;
}
