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

#include "ISipMessage.h"
#include "SipStatusCode.h"
#include "SubState.h"
#include "SubscriberRefreshHelper.h"

__IMS_TRACE_TAG_IMS_CORE__;

PUBLIC
SubscriberRefreshHelper::SubscriberRefreshHelper(
        IN IRefreshable* piRefreshable, IN const SubState* pSubState) :
        RefreshHelper(piRefreshable, IMS_FALSE),
        m_pSubState(pSubState)
{
}

PUBLIC VIRTUAL SubscriberRefreshHelper::~SubscriberRefreshHelper()
{
    IMS_TRACE_D("Destructor :: SubscriberRefreshHelper", 0, 0, 0);
}

PUBLIC VIRTUAL IMS_RESULT SubscriberRefreshHelper::SendRefreshRequest(
        IN ISipClientConnection* piScc)
{
    if (piScc == IMS_NULL)
    {
        return IMS_FAILURE;
    }

    if (!AddSpecificHeader(piScc))
    {
        IMS_TRACE_E(0, "Adding the specific headers for a subscription refresh request failed", 0,
                0, 0);
        return IMS_FAILURE;
    }

    if (RefreshHelper::SendRefreshRequest(piScc) != IMS_SUCCESS)
    {
        IMS_TRACE_E(0, "Sending a subscription refresh request failed", 0, 0, 0);
        return IMS_FAILURE;
    }

    return IMS_SUCCESS;
}

PUBLIC VIRTUAL IMS_RESULT SubscriberRefreshHelper::UpdateOnMessageReceived(
        IN const ISipConnection* piSc)
{
    const ISipMessage* piSipMsg = piSc->GetMessage();

    if (piSipMsg == IMS_NULL)
    {
        return IMS_FAILURE;
    }

    const SipMethod& objMethod = piSipMsg->GetMethod();

    // Case 1) SUBSCRIBE response received ...
    if (objMethod.Equals(SipMethod::SUBSCRIBE))
    {
        // If the subscription is in TERMINATED state & the refresh timer is active,
        // then stop the refresh timer...
        if (m_pSubState->IsTerminated())
        {
            StopRefresh();
            return IMS_SUCCESS;
        }

        IMS_SINT32 nStatusCode = piSipMsg->GetStatusCode();

        if (SipStatusCode::IsFinalSuccess(nStatusCode))
        {
            if (m_pSubState->IsSubscriptionDurationUpdated())
            {
                if (m_pSubState->GetDuration() <= 0)
                {
                    StopRefresh();
                }
                else
                {
                    SetDuration(m_pSubState->GetDuration());

                    StopRefresh();

                    if (!StartRefresh())
                    {
                        return IMS_FAILURE;
                    }
                }
            }
        }
        else
        {
            // Start the timer for the remained time interval
            ConsumeRemainedTime();
        }
    }
    // Case 2) NOTIFY request received ...
    else
    {
        // No actions after receiving NOTIFY request ...
    }

    return IMS_SUCCESS;
}

PUBLIC VIRTUAL IMS_RESULT SubscriberRefreshHelper::UpdateOnMessageSent(
        IN const ISipConnection* piSc)
{
    const ISipMessage* piSipMsg = piSc->GetMessage();

    if (piSipMsg == IMS_NULL)
    {
        return IMS_FAILURE;
    }

    // Case 1) SUBSCRIBE request sent ...
    if (piSipMsg->GetMethod().Equals(SipMethod::SUBSCRIBE))
    {
        // No actions after sending SUBSCRIBE request ...
    }
    // Case 2) NOTIFY response sent ...
    else
    {
        // If the subscription is in TERMINATED state & the refresh timer is active,
        // then stop the refresh timer...
        if (m_pSubState->IsTerminated())
        {
            StopRefresh();
            return IMS_SUCCESS;
        }

        IMS_SINT32 nStatusCode = piSipMsg->GetStatusCode();

        if (m_pSubState->IsSubscriptionDurationUpdated())
        {
            StopRefresh();
        }

        if (SipStatusCode::IsFinalSuccess(nStatusCode))
        {
            if (m_pSubState->IsSubscriptionDurationUpdated() && (m_pSubState->GetDuration() > 0))
            {
                SetDuration(m_pSubState->GetDuration());

                if (!StartRefresh())
                {
                    return IMS_FAILURE;
                }
            }
        }
        else
        {
            // 401/407: a special handling required if new code is written.
            // A failure final response to NOTIFY request results
            // in the removal of subscription from the Subscriber.
        }
    }

    return IMS_SUCCESS;
}

PROTECTED VIRTUAL void SubscriberRefreshHelper::RefreshCompleted(
        IN ISipClientConnection* piScc, IN IMS_SINT32 nCode /*= 0*/)
{
    Refreshable_RefreshCompleted(piScc, nCode);

    // do something ...
    if (nCode == 0)
    {
        IMS_SINT32 nStatusCode = piScc->GetStatusCode();

        if (SipStatusCode::IsFinalSuccess(nStatusCode))
        {
            StopRefresh();
        }

        UpdateOnMessageReceived(piScc);
    }
    else if (nCode == TRANSACTION_TIMEOUT)
    {
        // Behaves as if the 408 request timeout response received
        StopRefresh();
    }
}

PROTECTED VIRTUAL void SubscriberRefreshHelper::RefreshStarted()
{
    if (Refreshable_RefreshStarted() != IMS_TRUE)
    {
        // Clean up the refresh timer related resources

        Refreshable_RefreshTerminated();
    }
}
