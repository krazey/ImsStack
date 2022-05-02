/*
    Author
    <table>
    date      author                    description
    --------  --------------            ----------
    20130518  hwangoo.park@             Created
    </table>

    Description

*/

#include "ServiceMemory.h"
#include "ISipRoutingRejectListener.h"
#include "SipRoutingRejectNotifier.h"

PUBLIC
SIPRoutingRejectNotifier::SIPRoutingRejectNotifier()
{
}

PUBLIC VIRTUAL
SIPRoutingRejectNotifier::~SIPRoutingRejectNotifier()
{
}

/*

Remarks

*/
PUBLIC
IMS_BOOL SIPRoutingRejectNotifier::IsNotificationRequired() const
{
    return !objListeners.IsEmpty();
}

/*

Remarks

*/
PUBLIC
void SIPRoutingRejectNotifier::NotifyRequestReject(IN ISIPMessage *piSIPMsg,
        IN_OUT SIPStatusCode &objStatusCode)
{
    for (IMS_UINT32 i = 0; i < objListeners.GetSize(); ++i)
    {
        ISIPRoutingRejectListener *piListener = objListeners.GetAt(i);

        if (piListener->RoutingReject_NotifyRequest(piSIPMsg, objStatusCode))
        {
            return;
        }
    }
}

/*

Remarks

*/
PUBLIC
void SIPRoutingRejectNotifier::NotifyRequestReject(IN ISIPServerConnection *piSSC,
        IN_OUT SIPStatusCode &objStatusCode)
{
    for (IMS_UINT32 i = 0; i < objListeners.GetSize(); ++i)
    {
        ISIPRoutingRejectListener *piListener = objListeners.GetAt(i);

        if (piListener->RoutingReject_NotifyRequest(piSSC, objStatusCode))
        {
            return;
        }
    }
}

/*

Remarks

*/
PRIVATE VIRTUAL
void SIPRoutingRejectNotifier::AddListener(IN ISIPRoutingRejectListener *piListener)
{
    if (piListener == IMS_NULL)
    {
        return;
    }

    for (IMS_UINT32 i = 0; i < objListeners.GetSize(); ++i)
    {
        ISIPRoutingRejectListener *piTmpListener = objListeners.GetAt(i);

        if (piTmpListener == piListener)
        {
            return;
        }
    }

    objListeners.Append(piListener);
}

/*

Remarks

*/
PRIVATE VIRTUAL
void SIPRoutingRejectNotifier::RemoveListener(IN ISIPRoutingRejectListener *piListener)
{
    if (piListener == IMS_NULL)
    {
        return;
    }

    for (IMS_UINT32 i = 0; i < objListeners.GetSize(); ++i)
    {
        ISIPRoutingRejectListener *piTmpListener = objListeners.GetAt(i);

        if (piTmpListener == piListener)
        {
            objListeners.RemoveAt(i);
            return;
        }
    }
}
