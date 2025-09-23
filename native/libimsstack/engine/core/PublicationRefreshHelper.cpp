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

#include "PubState.h"
#include "PublicationRefreshHelper.h"
#include "SipStatusCode.h"

__IMS_TRACE_TAG_IMS_CORE__;

PUBLIC
PublicationRefreshHelper::PublicationRefreshHelper(
        IN IRefreshable* piRefreshable, IN const PubState* pPubState) :
        RefreshHelper(piRefreshable, IMS_FALSE),
        m_pPubState(pPubState)
{
}

PUBLIC VIRTUAL PublicationRefreshHelper::~PublicationRefreshHelper()
{
    IMS_TRACE_D("Destructor :: PublicationRefreshHelper", 0, 0, 0);
}

PUBLIC VIRTUAL IMS_RESULT PublicationRefreshHelper::SendRefreshRequest(
        IN ISipClientConnection* piScc)
{
    if (piScc == IMS_NULL)
    {
        return IMS_FAILURE;
    }

    if (!AddSpecificHeader(piScc))
    {
        IMS_TRACE_E(
                0, "Adding the specific headers for a publication refresh request failed", 0, 0, 0);
        return IMS_FAILURE;
    }

    if (RefreshHelper::SendRefreshRequest(piScc) != IMS_SUCCESS)
    {
        IMS_TRACE_E(0, "Sending a publication refresh request failed", 0, 0, 0);
        return IMS_FAILURE;
    }

    return IMS_SUCCESS;
}

PUBLIC VIRTUAL IMS_RESULT PublicationRefreshHelper::UpdateOnMessageReceived(
        IN const ISipConnection* piSc)
{
    const ISipMessage* piSipMsg = piSc->GetMessage();

    if (piSipMsg == IMS_NULL)
    {
        return IMS_FAILURE;
    }

    // On PUBLISH response received ...
    if (piSipMsg->GetMethod().Equals(SipMethod::PUBLISH))
    {
        // If the publication state is in TERMINATED state & the refresh timer is active,
        // then stop the refresh timer...
        if (m_pPubState->IsTerminated())
        {
            StopRefresh();

            return IMS_SUCCESS;
        }

        IMS_SINT32 nStatusCode = piSipMsg->GetStatusCode();

        if (SipStatusCode::IsFinalSuccess(nStatusCode))
        {
            if (m_pPubState->GetDuration() <= 0)
            {
                StopRefresh();
            }
            else
            {
                SetDuration(m_pPubState->GetDuration());

                StopRefresh();

                if (!StartRefresh())
                {
                    return IMS_FAILURE;
                }
            }
        }
        else
        {
            // Start the timer for the remained time interval
            ConsumeRemainedTime();
        }
    }

    return IMS_SUCCESS;
}

PROTECTED VIRTUAL void PublicationRefreshHelper::RefreshCompleted(
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

PROTECTED VIRTUAL void PublicationRefreshHelper::RefreshStarted()
{
    if (Refreshable_RefreshStarted() != IMS_TRUE)
    {
        // Clean up the refresh timer related resources

        Refreshable_RefreshTerminated();
    }
}
