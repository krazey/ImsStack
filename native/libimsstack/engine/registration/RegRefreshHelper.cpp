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

#include "RegRefreshHelper.h"
#include "SipStatusCode.h"

__IMS_TRACE_TAG_REG__;

PUBLIC
RegRefreshHelper::RegRefreshHelper(IN IRefreshable* piRefreshable) :
        RefreshHelper(piRefreshable, IMS_TRUE)
{
}

PUBLIC VIRTUAL RegRefreshHelper::~RegRefreshHelper()
{
    IMS_TRACE_D("Destructor :: RegRefreshHelper", 0, 0, 0);
}

PUBLIC VIRTUAL IMS_RESULT RegRefreshHelper::SendRefreshRequest(IN ISipClientConnection* piScc)
{
    if (piScc == IMS_NULL)
    {
        return IMS_FAILURE;
    }

    if (!AddSpecificHeader(piScc))
    {
        IMS_TRACE_E(0, "Adding the specific headers for a re-registration request failed", 0, 0, 0);
        return IMS_FAILURE;
    }

    if (RefreshHelper::SendRefreshRequest(piScc) != IMS_SUCCESS)
    {
        return IMS_FAILURE;
    }

    UpdateOnMessageSent(piScc);

    return IMS_SUCCESS;
}

PUBLIC
IMS_BOOL RegRefreshHelper::UpdateRefreshTimer(IN IMS_SINT32 nDuration)
{
    // Stop the refresh timer if it is running...
    if (IsTimerActive())
    {
        StopRefresh();
    }

    if (nDuration <= 0)
    {
        SetDuration(0);
        return IMS_TRUE;
    }

    SetDuration(nDuration);

    if (!StartRefresh())
    {
        IMS_TRACE_E(0, "Starting a registration refresh timer failed", 0, 0, 0);
        return IMS_FALSE;
    }

    return IMS_TRUE;
}

PROTECTED VIRTUAL void RegRefreshHelper::RefreshCompleted(
        IN ISipClientConnection* piScc, IN IMS_SINT32 nCode /*= 0*/)
{
    // do something ...
    if (nCode == 0)
    {
        IMS_SINT32 nStatusCode = piScc->GetStatusCode();

        if ((nStatusCode >= SipStatusCode::SC_200) && (nStatusCode < SipStatusCode::SC_300))
        {
            // Update the refresh timer
            if (UpdateOnMessageReceived(piScc) != IMS_SUCCESS)
            {
                Refreshable_RefreshCompleted(piScc, nCode);
                return;
            }

            // If the refresh timer is not running ..... ????
        }
    }
    else if (nCode == TRANSACTION_TIMEOUT)
    {
        // Behaves as if the 408 request timeout response received
        // hwangoo.park, 130529, do not stop registration refresh timer
        // even though the transaction timer is expired.
        // StopRefresh();
    }

    Refreshable_RefreshCompleted(piScc, nCode);
}

PROTECTED VIRTUAL void RegRefreshHelper::RefreshStarted()
{
    if (Refreshable_RefreshStarted() != IMS_TRUE)
    {
        // Clean up the refresh timer related resources

        Refreshable_RefreshTerminated();
    }
}

PROTECTED VIRTUAL void RegRefreshHelper::RefreshTerminated()
{
    Refreshable_RefreshTerminated();
}
