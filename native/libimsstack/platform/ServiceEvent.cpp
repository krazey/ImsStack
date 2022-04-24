/*
    Author
    <table>
    date      author                    description
    --------  --------------            ----------
    20101006  hwangoo.park@             Created
    </table>

    Description

*/

#include "ServiceMemory.h"
#include "ServiceTrace.h"
#include "ServiceMutex.h"
#include "IMSMap.h"
#include "ImsMessageDef.h"
#include "IEventReceiver.h"
#include "IEventSender.h"
#include "IMSActivity.h"
#include "SystemConfig.h"
#include "PlatformFactory.h"
#include "ServiceEvent.h"

__IMS_TRACE_TAG_BASE__;

// EventActivity class
class EventActivity
    : public IMSActivity
{
public:
    EventActivity(IN IMS_SINT32 nEvent_, IN IEventListener *piListener_);
    virtual ~EventActivity();

public:
    IMS_BOOL IsSameListener(IN IEventListener *piListener);
    void NotifyEvent(IN IMS_UINT32 nWParam, IN IMS_UINT32 nLParam);

private:
    // IMSActivity class
    virtual IIMSActivityControl* GetController();
    virtual IMS_BOOL DispatchMessage(IN ImsMessage &objMSG);

private:
    IMS_SINT32 nEvent;
    IEventListener *piListener;
};

PUBLIC
EventActivity::EventActivity(IN IMS_SINT32 nEvent_, IN IEventListener *piListener_)
    : IMSActivity()
    , nEvent(nEvent_)
    , piListener(piListener_)
{
}

PUBLIC VIRTUAL
EventActivity::~EventActivity()
{
}

PUBLIC
IMS_BOOL EventActivity::IsSameListener(IN IEventListener *piListener)
{
    if (this->piListener == piListener)
    {
        return IMS_TRUE;
    }

    return IMS_FALSE;
}

PUBLIC
void EventActivity::NotifyEvent(IN IMS_UINT32 nWParam, IN IMS_UINT32 nLParam)
{
    if (piListener == IMS_NULL)
    {
        return;
    }

    PostMessage(IMS_MSG_USER + nEvent, nWParam, nLParam);
}

PRIVATE VIRTUAL
IIMSActivityControl* EventActivity::GetController()
{
    return IMS_NULL;
}

PRIVATE VIRTUAL
IMS_BOOL EventActivity::DispatchMessage(IN ImsMessage &objMSG)
{
    if (objMSG.GetName() == (IMS_MSG_USER + nEvent))
    {
        if (piListener != IMS_NULL)
        {
            piListener->Event_NotifyEvent(nEvent,
                    LONG_TO_INT(objMSG.nWparam), LONG_TO_INT(objMSG.nLparam));
        }
    }

    return IMS_TRUE;
}



class EventHolder
    : public IEventReceiverListener
{
public:
    EventHolder(IN IMS_SINT32 nSlotId_);
    virtual ~EventHolder();

private:
    EventHolder(IN const EventHolder& objRHS);
    EventHolder& operator=(IN const EventHolder& objRHS);

public:
    IMS_BOOL AddListener(IN IMS_SINT32 nEvent, IN IEventListener *piListener);
    void RemoveListener(IN IMS_SINT32 nEvent, IN IEventListener *piListener);
    void SetUnregisteredEvents();

private:
    // IEventReceiverListener class
    virtual IMS_BOOL EventReceiver_NotifyEvent(IN IMS_SINT32 nEvent,
            IN IMS_UINT32 nWParam, IN IMS_UINT32 nLParam);

private:
    IMutex *piLock;
    IEventReceiver *piReceiver;
    // <Event, EventActivity>
    IMSMap<IMS_SINT32, IMSList<EventActivity*> > objEventMap;
};

PUBLIC
EventHolder::EventHolder(IN IMS_SINT32 nSlotId_)
    : piLock(IMS_NULL)
    , piReceiver(PlatformFactory::CreateEventReceiver(nSlotId_))
    , objEventMap(IMSMap<IMS_SINT32, IMSList<EventActivity*> >())
{
    piLock = MutexService::GetMutexService()->CreateMutex();

    piReceiver->SetListener(this);
}

PUBLIC VIRTUAL
EventHolder::~EventHolder()
{
    {
        LockGuard objLock(piLock);

        for (IMS_UINT32 i = 0; i < objEventMap.GetSize(); ++i)
        {
            IMSList<EventActivity*> &objActivities = objEventMap.GetValueAt(i);

            for (IMS_UINT32 j = 0; j < objActivities.GetSize(); ++j)
            {
                EventActivity *pActivity = objActivities.GetAt(j);

                if (pActivity != IMS_NULL)
                {
                    delete pActivity;
                }
            }
        }
    }

    MutexService::GetMutexService()->DestroyMutex(piLock);

    piReceiver->SetListener(IMS_NULL);

    PlatformFactory::DestroyEventReceiver(piReceiver);
}

PUBLIC
IMS_BOOL EventHolder::AddListener(IN IMS_SINT32 nEvent, IN IEventListener *piListener)
{
    LockGuard objLock(piLock);

    IMS_SLONG nIndex = objEventMap.GetIndexOfKey(nEvent);

    if (nIndex < 0)
    {
        IMSList<EventActivity*> objActivities;
        EventActivity *pActivity = new EventActivity(nEvent, piListener);

        if (pActivity == IMS_NULL)
        {
            return IMS_FALSE;
        }

        if (!objActivities.Append(pActivity))
        {
            delete pActivity;
            return IMS_FALSE;
        }

        objEventMap.Add(nEvent, objActivities);

        // Set the event
        piReceiver->SetEvent(nEvent);

        return IMS_TRUE;
    }

    IMSList<EventActivity*> &objActivities = objEventMap.GetValueAt(nIndex);

    for (IMS_UINT32 i = 0; i < objActivities.GetSize(); ++i)
    {
        EventActivity *pActivity = objActivities.GetAt(i);

        if (pActivity == IMS_NULL)
        {
            continue;
        }

        if (pActivity->IsSameListener(piListener))
        {
            IMS_TRACE_D("Event (%d) is already registered", nEvent, 0, 0);
            return IMS_TRUE;
        }
    }

    // Not found; so add a new listener
    EventActivity *pActivity = new EventActivity(nEvent, piListener);

    if (pActivity == IMS_NULL)
    {
        return IMS_FALSE;
    }

    if (!objActivities.Append(pActivity))
    {
        delete pActivity;
        return IMS_FALSE;
    }

    return IMS_TRUE;
}

PUBLIC
void EventHolder::RemoveListener(IN IMS_SINT32 nEvent, IN IEventListener *piListener)
{
    LockGuard objLock(piLock);

    IMS_SLONG nIndex = objEventMap.GetIndexOfKey(nEvent);

    if (nIndex < 0)
    {
        return;
    }

    IMSList<EventActivity*> &objActivities = objEventMap.GetValueAt(nIndex);

    for (IMS_UINT32 i = 0; i < objActivities.GetSize(); ++i)
    {
        EventActivity *pActivity = objActivities.GetAt(i);

        if (pActivity == IMS_NULL)
        {
            continue;
        }

        if (pActivity->IsSameListener(piListener))
        {
            delete pActivity;
            objActivities.RemoveAt(i);
            break;
        }
    }

    if (objActivities.IsEmpty())
    {
        objEventMap.RemoveAt(nIndex);

        // Set the event
        piReceiver->ResetEvent(nEvent);
    }
}

PUBLIC
void EventHolder::SetUnregisteredEvents()
{
    if (!objEventMap.IsEmpty())
    {
        for (IMS_UINT32 i = 0; i < objEventMap.GetSize(); i++)
        {
            IMS_SINT32 nEvent = objEventMap.GetKeyAt(i);
            const IMSList<EventActivity*> &objActivities = objEventMap.GetValueAt(i);

            piReceiver->SetEvent(nEvent);

            IMS_TRACE_I("UnregisteredEvents :: event=%08X, listeners=%d",
                    nEvent, objActivities.GetSize(), 0);
        }
    }
    else
    {
        IMS_TRACE_D("UnregisteredEvents :: no events", 0, 0, 0);
    }
}

PRIVATE VIRTUAL
IMS_BOOL EventHolder::EventReceiver_NotifyEvent(IN IMS_SINT32 nEvent,
        IN IMS_UINT32 nWParam, IN IMS_UINT32 nLParam)
{
    LockGuard objLock(piLock);

    IMS_SLONG nIndex = objEventMap.GetIndexOfKey(nEvent);

    if (nIndex < 0)
    {
        IMS_TRACE_D("Event (%d : %d, %d) is not handled", nEvent, nWParam, nLParam);
        return IMS_FALSE;
    }

    IMS_TRACE_I("EventReceiver :: E (%d), W (%d), L (%d)", nEvent, nWParam, nLParam);

    IMSList<EventActivity*> &objActivities = objEventMap.GetValueAt(nIndex);

    if (objActivities.IsEmpty())
    {
        IMS_TRACE_D("There are no event receivers", 0, 0, 0);
        return IMS_FALSE;
    }

    for (IMS_UINT32 i = 0; i < objActivities.GetSize(); ++i)
    {
        EventActivity *pActivity = objActivities.GetAt(i);

        if (pActivity != IMS_NULL)
        {
            pActivity->NotifyEvent(nWParam, nLParam);
        }
    }

    return IMS_TRUE;
}



// EventServicePrivate class
class EventServicePrivate
{
public:
    EventServicePrivate();
    ~EventServicePrivate();

private:
    EventServicePrivate(IN const EventServicePrivate& objRHS);
    EventServicePrivate& operator=(IN const EventServicePrivate& objRHS);

public:
    inline EventHolder* GetHolder(IN IMS_SINT32 nSlotId)
    {
        if ((nSlotId < IMS_SLOT_0) || (nSlotId >= SystemConfig::GetMaxSimSlot()))
        {
            nSlotId = IMS_SLOT_0;
        }

        return ppHolder[nSlotId];
    }

    inline IEventSender* GetSender() const
    { return piSender; }

private:
    IEventSender *piSender;

    EventHolder **ppHolder;
};

PUBLIC
EventServicePrivate::EventServicePrivate()
    : piSender(PlatformFactory::CreateEventSender())
    , ppHolder(IMS_NULL)
{
    IMS_SINT32 nSimCount = SystemConfig::GetMaxSimSlot();

    ppHolder = new EventHolder*[nSimCount];

    for (IMS_SINT32 i = 0; i < nSimCount; ++i)
    {
        ppHolder[i] = new EventHolder(i);
    }
}

PUBLIC VIRTUAL
EventServicePrivate::~EventServicePrivate()
{
    if (ppHolder != IMS_NULL)
    {
        IMS_SINT32 nSimCount = SystemConfig::GetMaxSimSlot();

        for (IMS_SINT32 i = 0; i < nSimCount; ++i)
        {
            if (ppHolder[i] != IMS_NULL)
            {
                delete ppHolder[i];
            }
        }

        delete[] ppHolder;
    }

    PlatformFactory::DestroyEventSender(piSender);
}



// EventService class
PRIVATE
EventService::EventService()
    : pPrivate(new EventServicePrivate())
{
}

PRIVATE
EventService::~EventService()
{
    if (pPrivate != IMS_NULL)
    {
        delete pPrivate;
    }
}

PUBLIC
void EventService::AddListener(IN IMS_SINT32 nEvent, IN IEventListener *piListener,
        IN IMS_SINT32 nSlotId)
{
    EventHolder *pHolder = pPrivate->GetHolder(nSlotId);

    pHolder->AddListener(nEvent, piListener);
}

PUBLIC
void EventService::RemoveListener(IN IMS_SINT32 nEvent, IN IEventListener *piListener,
        IN IMS_SINT32 nSlotId)
{
    EventHolder *pHolder = pPrivate->GetHolder(nSlotId);

    pHolder->RemoveListener(nEvent, piListener);
}

PUBLIC
void EventService::SendEvent(IN IMS_SINT32 nEvent,
        IN IMS_UINT32 nWParam, IN IMS_UINT32 nLParam,
        IN IMS_SINT32 nSlotId)
{
    pPrivate->GetSender()->SendEvent(nEvent, nWParam, nLParam, nSlotId);
}

PUBLIC
void EventService::SetUnregisteredEvents(IN IMS_SINT32 nSlotId)
{
    EventHolder *pHolder = pPrivate->GetHolder(nSlotId);
    pHolder->SetUnregisteredEvents();
}

PUBLIC GLOBAL
EventService* EventService::GetEventService()
{
    static EventService *pEventService = IMS_NULL;

    if (pEventService == IMS_NULL)
    {
        pEventService = new EventService();
    }

    return pEventService;
}
