/*
    Author
    <table>
    date      author                    description
    --------  --------------            ----------
    20090904  toastops@                 Created
    </table>

    Description

*/

#include "ServiceMemory.h"
#include "ServiceTrace.h"
#include "SipStatusCode.h"
#include "RegRefreshHelper.h"

__IMS_TRACE_TAG_REG__;



PUBLIC
RegRefreshHelper::RegRefreshHelper(IN IRefreshable *piRefreshable_)
    : RefreshHelper(piRefreshable_, IMS_TRUE)
{
}

PUBLIC VIRTUAL
RegRefreshHelper::~RegRefreshHelper()
{
    //---------------------------------------------------------------------------------------------

    IMS_TRACE_D("Destructor :: RegRefreshHelper", 0, 0, 0);
}

PUBLIC VIRTUAL
IMS_BOOL RegRefreshHelper::AddSpecificHeader(IN ISIPConnection *piSC)
{
    //---------------------------------------------------------------------------------------------

    (void) piSC;

    return IMS_TRUE;
}

PUBLIC VIRTUAL
IMS_RESULT RegRefreshHelper::SendRefreshRequest(IN ISIPClientConnection *piSCC)
{
    //---------------------------------------------------------------------------------------------

    if (piSCC == IMS_NULL)
    {
        return IMS_FAILURE;
    }

    if (!AddSpecificHeader(piSCC))
    {
        IMS_TRACE_E(0, "Adding the specific headers for a re-registration request failed",
                0, 0, 0);
        return IMS_FAILURE;
    }

    if (RefreshHelper::SendRefreshRequest(piSCC) != IMS_SUCCESS)
    {
        return IMS_FAILURE;
    }

    UpdateOnMessageSent(piSCC);

    return IMS_SUCCESS;
}

PUBLIC VIRTUAL
IMS_RESULT RegRefreshHelper::UpdateOnMessageReceived(IN CONST ISIPConnection *piSC)
{
    //---------------------------------------------------------------------------------------------

    (void) piSC;

    return IMS_SUCCESS;
}

PUBLIC VIRTUAL
IMS_RESULT RegRefreshHelper::UpdateOnMessageSent(IN CONST ISIPConnection *piSC)
{
    //---------------------------------------------------------------------------------------------

    (void) piSC;

    return IMS_SUCCESS;
}

PUBLIC
const SIPAddress& RegRefreshHelper::GetContactAddress() const
{
    //---------------------------------------------------------------------------------------------

    return objContactAddress;
}

PUBLIC
void RegRefreshHelper::SetContactAddress(IN CONST SIPAddress &objContactAddress)
{
    //---------------------------------------------------------------------------------------------

    this->objContactAddress = objContactAddress;
}

PUBLIC
IMS_BOOL RegRefreshHelper::UpdateRefreshTimer(IN IMS_SINT32 nDuration)
{
    //---------------------------------------------------------------------------------------------

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

PROTECTED VIRTUAL
void RegRefreshHelper::RefreshCompleted(IN ISIPClientConnection *piSCC,
        IN IMS_SINT32 nCode /* = 0 */)
{
    //---------------------------------------------------------------------------------------------

    // do something ...
    if (nCode == 0)
    {
        IMS_SINT32 nStatusCode = piSCC->GetStatusCode();

        if ((nStatusCode >= SIPStatusCode::SC_200)
                && (nStatusCode < SIPStatusCode::SC_300))
        {
            // Update the refresh timer
            if (UpdateOnMessageReceived(piSCC) != IMS_SUCCESS)
            {
                // Internal error ???
                Refreshable_RefreshCompleted(piSCC, nCode);
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

    Refreshable_RefreshCompleted(piSCC, nCode);
}

PROTECTED VIRTUAL
void RegRefreshHelper::RefreshStarted()
{
    //---------------------------------------------------------------------------------------------

    if (Refreshable_RefreshStarted() != IMS_TRUE)
    {
        // Clean up the refresh timer related resources

        Refreshable_RefreshTerminated();
    }
}

PROTECTED VIRTUAL
void RegRefreshHelper::RefreshTerminated()
{
    //---------------------------------------------------------------------------------------------

    Refreshable_RefreshTerminated();
}
