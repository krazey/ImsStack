/*
    Author
    <table>
    date      author                    description
    --------  --------------            ----------
    20100330  hwangoo.park@             Created
    </table>

    Description

*/

#include "ServiceMemory.h"
#include "ServiceTrace.h"
#include "ISipMessage.h"
#include "SipStatusCode.h"
#include "SubState.h"
#include "SubscriberRefreshHelper.h"

__IMS_TRACE_TAG_IMS_CORE__;



PUBLIC
SubscriberRefreshHelper::SubscriberRefreshHelper(IN IRefreshable *piRefreshable_,
        IN CONST SubState *pSubState_)
    : RefreshHelper(piRefreshable_, IMS_FALSE)
    , pSubState(pSubState_)
{
}

PUBLIC VIRTUAL
SubscriberRefreshHelper::~SubscriberRefreshHelper()
{
    //---------------------------------------------------------------------------------------------

    IMS_TRACE_D("Destructor :: SubscriberRefreshHelper", 0, 0, 0);
}

PUBLIC VIRTUAL
IMS_BOOL SubscriberRefreshHelper::AddSpecificHeader(IN ISIPConnection *piSC)
{
    //---------------------------------------------------------------------------------------------

    (void) piSC;

    return IMS_TRUE;
}

PUBLIC VIRTUAL
IMS_RESULT SubscriberRefreshHelper::SendRefreshRequest(IN ISIPClientConnection *piSCC)
{
    //---------------------------------------------------------------------------------------------

    if (piSCC == IMS_NULL)
    {
        return IMS_FAILURE;
    }

    if (!AddSpecificHeader(piSCC))
    {
        IMS_TRACE_E(0, "Adding the specific headers for a subscription refresh request failed",
                0, 0, 0);
        return IMS_FAILURE;
    }

    if (RefreshHelper::SendRefreshRequest(piSCC) != IMS_SUCCESS)
    {
        IMS_TRACE_E(0, "Sending a subscription refresh request failed", 0, 0, 0);
        return IMS_FAILURE;
    }

    return IMS_SUCCESS;
}

PUBLIC VIRTUAL
IMS_RESULT SubscriberRefreshHelper::UpdateOnMessageReceived(IN CONST ISIPConnection *piSC)
{
    ISIPMessage *piSIPMsg = piSC->GetMessage();

    //---------------------------------------------------------------------------------------------

    if (piSIPMsg == IMS_NULL)
    {
        return IMS_FAILURE;
    }

    const SIPMethod &objMethod = piSIPMsg->GetMethod();

    // Case 1) SUBSCRIBE response received ...
    if (objMethod.Equals(SIPMethod::SUBSCRIBE))
    {
        // If the subscription is in TERMINATED state & the refresh timer is active,
        // then stop the refresh timer...
        if (pSubState->IsTerminated())
        {
            StopRefresh();

            return IMS_SUCCESS;
        }

        IMS_SINT32 nStatusCode = piSIPMsg->GetStatusCode();

        if (SIPStatusCode::IsFinalSuccess(nStatusCode))
        {
            if (pSubState->IsSubscriptionDurationUpdated())
            {
                if (pSubState->GetDuration() <= 0)
                    StopRefresh();
                else
                {
                    SetDuration(pSubState->GetDuration());

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

PUBLIC VIRTUAL
IMS_RESULT SubscriberRefreshHelper::UpdateOnMessageSent(IN CONST ISIPConnection *piSC)
{
    ISIPMessage *piSIPMsg = piSC->GetMessage();

    //---------------------------------------------------------------------------------------------

    if (piSIPMsg == IMS_NULL)
    {
        return IMS_FAILURE;
    }

    // Case 1) SUBSCRIBE request sent ...
    if (piSIPMsg->GetMethod().Equals(SIPMethod::SUBSCRIBE))
    {
        // No actions after sending SUBSCRIBE request ...
    }
    // Case 2) NOTIFY response sent ...
    else
    {
        // If the subscription is in TERMINATED state & the refresh timer is active,
        // then stop the refresh timer...
        if (pSubState->IsTerminated())
        {
            StopRefresh();

            return IMS_SUCCESS;
        }

        IMS_SINT32 nStatusCode = piSIPMsg->GetStatusCode();

        if (pSubState->IsSubscriptionDurationUpdated())
        {
            StopRefresh();
        }

        if (SIPStatusCode::IsFinalSuccess(nStatusCode))
        {
            if (pSubState->IsSubscriptionDurationUpdated() && (pSubState->GetDuration() > 0))
            {
                SetDuration(pSubState->GetDuration());

                if (!StartRefresh())
                {
                    return IMS_FAILURE;
                }
            }
        }
        else if ((nStatusCode == SIPStatusCode::SC_401)
            || (nStatusCode == SIPStatusCode::SC_407))
        {
        }
        else
        {
            // A failure final response to NOTIFY request results
            // in the removal of subscription from the Subscriber.
        }
    }

    return IMS_SUCCESS;
}

PROTECTED VIRTUAL
void SubscriberRefreshHelper::RefreshCompleted(IN ISIPClientConnection *piSCC,
        IN IMS_SINT32 nCode /* = 0 */)
{
    //---------------------------------------------------------------------------------------------

    Refreshable_RefreshCompleted(piSCC, nCode);

    // do something ...
    if (nCode == 0)
    {
        IMS_SINT32 nStatusCode = piSCC->GetStatusCode();

        if (SIPStatusCode::IsFinalSuccess(nStatusCode))
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

PROTECTED VIRTUAL
void SubscriberRefreshHelper::RefreshStarted()
{
    //---------------------------------------------------------------------------------------------

    if (Refreshable_RefreshStarted() != IMS_TRUE)
    {
        // Clean up the refresh timer related resources

        Refreshable_RefreshTerminated();
    }
}

PROTECTED VIRTUAL
void SubscriberRefreshHelper::RefreshTerminated()
{
    //---------------------------------------------------------------------------------------------

    Refreshable_RefreshTerminated();
}
