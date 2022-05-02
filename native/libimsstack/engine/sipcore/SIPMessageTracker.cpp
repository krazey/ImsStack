/*
    Author
    <table>
    date      author                    description
    --------  --------------            ----------
    20120204  hwangoo.park@             Created
    </table>

    Description

*/

#include "ServiceMemory.h"
#include "ISipMessageTrackerListener.h"
#include "SIPMessageTracker.h"



PRIVATE
SIPMessageTracker::SIPMessageTracker()
    : piListener(IMS_NULL)
{
}

PRIVATE VIRTUAL
SIPMessageTracker::~SIPMessageTracker()
{
}

/*

Remarks

*/
PUBLIC
IMS_BOOL SIPMessageTracker::IsMessageTrackerEnabled() const
{
    return (piListener != IMS_NULL);
}

/*

Remarks

*/
PUBLIC
void SIPMessageTracker::NotifyMessageReceived(IN CONST SIPMethod &objMethod,
        IN IMS_SINT32 nStatusCode, IN CONST AString &strCallId)
{
    if (piListener == IMS_NULL)
    {
        return;
    }

    if (objIncomingFilters.IsEmpty())
    {
        piListener->MessageTracker_NotifyMessageReceived(objMethod, nStatusCode, strCallId);
    }
    else
    {
        for (IMS_UINT32 i = 0; i < objIncomingFilters.GetSize(); ++i)
        {
            MessageFilter *pFilter = objIncomingFilters.GetAt(i);

            if (pFilter != IMS_NULL)
            {
                if (pFilter->Equals(objMethod, nStatusCode))
                {
                    piListener->MessageTracker_NotifyMessageReceived(
                            objMethod, nStatusCode, strCallId);
                    break;
                }
            }
        }
    }
}

/*

Remarks

*/
PUBLIC
void SIPMessageTracker::NotifyMessageSent(IN CONST SIPMethod &objMethod, IN IMS_SINT32 nStatusCode,
        IN CONST AString &strCallId, IN IMS_SINT32 nErrorCode /* = 0 */)
{
    if (piListener == IMS_NULL)
    {
        return;
    }

    if (objOutgoingFilters.IsEmpty())
    {
        if (nErrorCode == 0)
        {
            piListener->MessageTracker_NotifyMessageSent(objMethod, nStatusCode, strCallId);
        }
        else
        {
            piListener->MessageTracker_NotifyMessageSentFailed(objMethod, nStatusCode, strCallId);
        }
    }
    else
    {
        for (IMS_UINT32 i = 0; i < objOutgoingFilters.GetSize(); ++i)
        {
            MessageFilter *pFilter = objOutgoingFilters.GetAt(i);

            if (pFilter != IMS_NULL)
            {
                if (pFilter->Equals(objMethod, nStatusCode))
                {
                    if (nErrorCode == 0)
                    {
                        piListener->MessageTracker_NotifyMessageSent(
                                objMethod, nStatusCode, strCallId);
                    }
                    else
                    {
                        piListener->MessageTracker_NotifyMessageSentFailed(
                                objMethod, nStatusCode, strCallId);
                    }
                    break;
                }
            }
        }
    }
}

/*

Remarks

*/
PRIVATE VIRTUAL
IMS_BOOL SIPMessageTracker::AddFilter(IN CONST SIPMethod &objMethod, IN IMS_SINT32 nStatusCode,
        IN IMS_BOOL bOutgoing)
{
    MessageFilter *pFilter = new MessageFilter(objMethod, nStatusCode);

    if (pFilter == IMS_NULL)
    {
        return IMS_FALSE;
    }

    if (bOutgoing)
    {
        if (!objOutgoingFilters.Append(pFilter))
        {
            delete pFilter;
            return IMS_FALSE;
        }
    }
    else
    {
        if (!objIncomingFilters.Append(pFilter))
        {
            delete pFilter;
            return IMS_FALSE;
        }
    }

    return IMS_TRUE;
}

/*

Remarks

*/
PRIVATE VIRTUAL
void SIPMessageTracker::RemoveFilter(IN CONST SIPMethod &objMethod)
{
    for (IMS_UINT32 i = 0; i < objOutgoingFilters.GetSize(); )
    {
        MessageFilter *pFilter = objOutgoingFilters.GetAt(i);

        if (pFilter == IMS_NULL)
        {
            ++i;
            continue;
        }

        if (pFilter->GetMethod().Equals(objMethod))
        {
            delete pFilter;

            objOutgoingFilters.RemoveAt(i);
            continue;
        }

        ++i;
    }

    for (IMS_UINT32 i = 0; i < objIncomingFilters.GetSize(); )
    {
        MessageFilter *pFilter = objIncomingFilters.GetAt(i);

        if (pFilter == IMS_NULL)
        {
            ++i;
            continue;
        }

        if (pFilter->GetMethod().Equals(objMethod))
        {
            delete pFilter;

            objIncomingFilters.RemoveAt(i);
            continue;
        }

        ++i;
    }
}

/*

Remarks

*/
PRIVATE VIRTUAL
void SIPMessageTracker::RemoveFilter(IN CONST SIPMethod &objMethod, IN IMS_SINT32 nStatusCode,
        IN IMS_BOOL bOutgoing)
{
    if (bOutgoing)
    {
        for (IMS_UINT32 i = 0; i < objOutgoingFilters.GetSize(); )
        {
            MessageFilter *pFilter = objOutgoingFilters.GetAt(i);

            if (pFilter == IMS_NULL)
            {
                ++i;
                continue;
            }

            if (pFilter->Equals(objMethod, nStatusCode))
            {
                delete pFilter;

                objOutgoingFilters.RemoveAt(i);
                continue;
            }

            ++i;
        }
    }
    else
    {
        for (IMS_UINT32 i = 0; i < objIncomingFilters.GetSize(); )
        {
            MessageFilter *pFilter = objIncomingFilters.GetAt(i);

            if (pFilter == IMS_NULL)
            {
                ++i;
                continue;
            }

            if (pFilter->Equals(objMethod, nStatusCode))
            {
                delete pFilter;

                objIncomingFilters.RemoveAt(i);
                continue;
            }

            ++i;
        }
    }
}

/*

Remarks

*/
PRIVATE VIRTUAL
void SIPMessageTracker::RemoveAllFilters()
{
    for (IMS_UINT32 i = 0; i < objIncomingFilters.GetSize(); ++i)
    {
        MessageFilter *pFilter = objIncomingFilters.GetAt(i);

        if (pFilter != IMS_NULL)
        {
            delete pFilter;
        }
    }

    objIncomingFilters.Clear();

    for (IMS_UINT32 i = 0; i < objOutgoingFilters.GetSize(); ++i)
    {
        MessageFilter *pFilter = objOutgoingFilters.GetAt(i);

        if (pFilter != IMS_NULL)
        {
            delete pFilter;
        }
    }

    objOutgoingFilters.Clear();
}

/*

Remarks

*/
PRIVATE VIRTUAL
void SIPMessageTracker::SetListener(IN ISIPMessageTrackerListener *piListener)
{
    this->piListener = piListener;
}
