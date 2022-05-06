/*
    Author
    <table>
    date      author                    description
    --------  --------------            ----------
    20100424  hwangoo.park@             Created
    </table>

    Description

*/

#include "ServiceMemory.h"
#include "ServiceTrace.h"
#include "SipStatusCode.h"
#include "PubState.h"
#include "PublicationRefreshHelper.h"

__IMS_TRACE_TAG_IMS_CORE__;

PUBLIC
PublicationRefreshHelper::PublicationRefreshHelper(
        IN IRefreshable* piRefreshable_, IN CONST PubState* pPubState_) :
        RefreshHelper(piRefreshable_, IMS_FALSE),
        pPubState(pPubState_)
{
}

PUBLIC VIRTUAL PublicationRefreshHelper::~PublicationRefreshHelper()
{
    //---------------------------------------------------------------------------------------------

    IMS_TRACE_D("Destructor :: PublicationRefreshHelper", 0, 0, 0);
}

PUBLIC VIRTUAL IMS_BOOL PublicationRefreshHelper::AddSpecificHeader(IN ISipConnection* piSC)
{
    //---------------------------------------------------------------------------------------------

    (void)piSC;

    return IMS_TRUE;
}

PUBLIC VIRTUAL IMS_RESULT PublicationRefreshHelper::SendRefreshRequest(
        IN ISipClientConnection* piSCC)
{
    //---------------------------------------------------------------------------------------------

    if (piSCC == IMS_NULL)
    {
        return IMS_FAILURE;
    }

    if (!AddSpecificHeader(piSCC))
    {
        IMS_TRACE_E(
                0, "Adding the specific headers for a publication refresh request failed", 0, 0, 0);
        return IMS_FAILURE;
    }

    if (RefreshHelper::SendRefreshRequest(piSCC) != IMS_SUCCESS)
    {
        IMS_TRACE_E(0, "Sending a publication refresh request failed", 0, 0, 0);
        return IMS_FAILURE;
    }

    return IMS_SUCCESS;
}

PUBLIC VIRTUAL IMS_RESULT PublicationRefreshHelper::UpdateOnMessageReceived(
        IN CONST ISipConnection* piSC)
{
    ISipMessage* piSIPMsg = piSC->GetMessage();

    //---------------------------------------------------------------------------------------------

    if (piSIPMsg == IMS_NULL)
    {
        return IMS_FAILURE;
    }

    // On PUBLISH response received ...
    if (piSIPMsg->GetMethod().Equals(SipMethod::PUBLISH))
    {
        // If the publication state is in TERMINATED state & the refresh timer is active,
        // then stop the refresh timer...
        if (pPubState->IsTerminated())
        {
            StopRefresh();

            return IMS_SUCCESS;
        }

        IMS_SINT32 nStatusCode = piSIPMsg->GetStatusCode();

        if (SipStatusCode::IsFinalSuccess(nStatusCode))
        {
            if (pPubState->GetDuration() <= 0)
                StopRefresh();
            else
            {
                SetDuration(pPubState->GetDuration());

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

PUBLIC VIRTUAL IMS_RESULT PublicationRefreshHelper::UpdateOnMessageSent(
        IN CONST ISipConnection* piSC)
{
    //---------------------------------------------------------------------------------------------

    (void)piSC;

    return IMS_SUCCESS;
}

PROTECTED VIRTUAL void PublicationRefreshHelper::RefreshCompleted(
        IN ISipClientConnection* piSCC, IN IMS_SINT32 nCode /* = 0 */)
{
    //---------------------------------------------------------------------------------------------

    Refreshable_RefreshCompleted(piSCC, nCode);

    // do something ...
    if (nCode == 0)
    {
        IMS_SINT32 nStatusCode = piSCC->GetStatusCode();

        if (SipStatusCode::IsFinalSuccess(nStatusCode))
        {
            StopRefresh();
        }

        UpdateOnMessageReceived(piSCC);
    }
    else if (nCode == TRANSACTION_TIMEOUT)
    {
        // Behaves as if the 408 request timeout response received
        StopRefresh();
    }
}

PROTECTED VIRTUAL void PublicationRefreshHelper::RefreshStarted()
{
    //---------------------------------------------------------------------------------------------

    if (Refreshable_RefreshStarted() != IMS_TRUE)
    {
        // Clean up the refresh timer related resources

        Refreshable_RefreshTerminated();
    }
}

PROTECTED VIRTUAL void PublicationRefreshHelper::RefreshTerminated()
{
    //---------------------------------------------------------------------------------------------

    Refreshable_RefreshTerminated();
}
