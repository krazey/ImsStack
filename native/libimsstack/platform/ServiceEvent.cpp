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
#include "IEventReceiver.h"
#include "IEventSender.h"
#include "IMSActivity.h"
#include "IMSMap.h"
#include "ImsMessageDef.h"
#include "PlatformFactory.h"
#include "ServiceEvent.h"
#include "ServiceMemory.h"
#include "ServiceMutex.h"
#include "ServiceTrace.h"
#include "SystemConfig.h"

__IMS_TRACE_TAG_BASE__;

// EventActivity class
class EventActivity : public IMSActivity
{
public:
    EventActivity(IN IMS_SINT32 nEvent, IN IEventListener* piListener);
    virtual ~EventActivity();

    EventActivity(IN const EventActivity&) = delete;
    EventActivity& operator=(IN const EventActivity&) = delete;

public:
    IMS_BOOL IsSameListener(IN IEventListener* piListener);
    void NotifyEvent(IN IMS_UINT32 nWParam, IN IMS_UINT32 nLParam);

private:
    // IMSActivity class
    virtual IIMSActivityControl* GetController();
    virtual IMS_BOOL DispatchMessage(IN ImsMessage& objMsg);

private:
    IMS_SINT32 m_nEvent;
    IEventListener* m_piListener;
};

PUBLIC
EventActivity::EventActivity(IN IMS_SINT32 nEvent, IN IEventListener* piListener) :
        IMSActivity(),
        m_nEvent(nEvent),
        m_piListener(piListener)
{
}

PUBLIC VIRTUAL EventActivity::~EventActivity() {}

PUBLIC
IMS_BOOL EventActivity::IsSameListener(IN IEventListener* piListener)
{
    if (m_piListener == piListener)
    {
        return IMS_TRUE;
    }

    return IMS_FALSE;
}

PUBLIC
void EventActivity::NotifyEvent(IN IMS_UINT32 nWParam, IN IMS_UINT32 nLParam)
{
    if (m_piListener == IMS_NULL)
    {
        return;
    }

    PostMessage(IMS_MSG_USER + m_nEvent, nWParam, nLParam);
}

PRIVATE VIRTUAL IIMSActivityControl* EventActivity::GetController()
{
    return IMS_NULL;
}

PRIVATE VIRTUAL IMS_BOOL EventActivity::DispatchMessage(IN ImsMessage& objMsg)
{
    if (objMsg.GetName() == (IMS_MSG_USER + m_nEvent))
    {
        if (m_piListener != IMS_NULL)
        {
            m_piListener->Event_NotifyEvent(
                    m_nEvent, LONG_TO_INT(objMsg.nWparam), LONG_TO_INT(objMsg.nLparam));
        }
    }

    return IMS_TRUE;
}

class EventHolder : public IEventReceiverListener
{
public:
    EventHolder(IN IMS_SINT32 nSlotId);
    virtual ~EventHolder();

    EventHolder(IN const EventHolder&) = delete;
    EventHolder& operator=(IN const EventHolder&) = delete;

public:
    IMS_BOOL AddListener(IN IMS_SINT32 nEvent, IN IEventListener* piListener);
    void RemoveListener(IN IMS_SINT32 nEvent, IN IEventListener* piListener);
    void SetUnregisteredEvents();

private:
    // IEventReceiverListener class
    virtual IMS_BOOL EventReceiver_NotifyEvent(
            IN IMS_SINT32 nEvent, IN IMS_UINT32 nWParam, IN IMS_UINT32 nLParam);

private:
    IMutex* m_piLock;
    IEventReceiver* m_piReceiver;
    // <Event, EventActivity>
    IMSMap<IMS_SINT32, IMSList<EventActivity*>> m_objEventMap;
};

PUBLIC
EventHolder::EventHolder(IN IMS_SINT32 nSlotId) :
        m_piLock(IMS_NULL),
        m_piReceiver(PlatformFactory::CreateEventReceiver(nSlotId)),
        m_objEventMap(IMSMap<IMS_SINT32, IMSList<EventActivity*>>())
{
    m_piLock = MutexService::GetMutexService()->CreateMutex();

    m_piReceiver->SetListener(this);
}

PUBLIC VIRTUAL EventHolder::~EventHolder()
{
    {
        LockGuard objLock(m_piLock);

        for (IMS_UINT32 i = 0; i < m_objEventMap.GetSize(); ++i)
        {
            IMSList<EventActivity*>& objActivities = m_objEventMap.GetValueAt(i);

            for (IMS_UINT32 j = 0; j < objActivities.GetSize(); ++j)
            {
                EventActivity* pActivity = objActivities.GetAt(j);

                if (pActivity != IMS_NULL)
                {
                    delete pActivity;
                }
            }
        }
    }

    MutexService::GetMutexService()->DestroyMutex(m_piLock);

    m_piReceiver->SetListener(IMS_NULL);

    PlatformFactory::DestroyEventReceiver(m_piReceiver);
}

PUBLIC
IMS_BOOL EventHolder::AddListener(IN IMS_SINT32 nEvent, IN IEventListener* piListener)
{
    LockGuard objLock(m_piLock);

    IMS_SLONG nIndex = m_objEventMap.GetIndexOfKey(nEvent);

    if (nIndex < 0)
    {
        IMSList<EventActivity*> objActivities;
        EventActivity* pActivity = new EventActivity(nEvent, piListener);

        if (pActivity == IMS_NULL)
        {
            return IMS_FALSE;
        }

        if (!objActivities.Append(pActivity))
        {
            delete pActivity;
            return IMS_FALSE;
        }

        m_objEventMap.Add(nEvent, objActivities);

        // Set the event
        m_piReceiver->SetEvent(nEvent);

        return IMS_TRUE;
    }

    IMSList<EventActivity*>& objActivities = m_objEventMap.GetValueAt(nIndex);

    for (IMS_UINT32 i = 0; i < objActivities.GetSize(); ++i)
    {
        EventActivity* pActivity = objActivities.GetAt(i);

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
    EventActivity* pActivity = new EventActivity(nEvent, piListener);

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
void EventHolder::RemoveListener(IN IMS_SINT32 nEvent, IN IEventListener* piListener)
{
    LockGuard objLock(m_piLock);

    IMS_SLONG nIndex = m_objEventMap.GetIndexOfKey(nEvent);

    if (nIndex < 0)
    {
        return;
    }

    IMSList<EventActivity*>& objActivities = m_objEventMap.GetValueAt(nIndex);

    for (IMS_UINT32 i = 0; i < objActivities.GetSize(); ++i)
    {
        EventActivity* pActivity = objActivities.GetAt(i);

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
        m_objEventMap.RemoveAt(nIndex);

        // Set the event
        m_piReceiver->ResetEvent(nEvent);
    }
}

PUBLIC
void EventHolder::SetUnregisteredEvents()
{
    if (!m_objEventMap.IsEmpty())
    {
        for (IMS_UINT32 i = 0; i < m_objEventMap.GetSize(); i++)
        {
            IMS_SINT32 nEvent = m_objEventMap.GetKeyAt(i);
            const IMSList<EventActivity*>& objActivities = m_objEventMap.GetValueAt(i);

            m_piReceiver->SetEvent(nEvent);

            IMS_TRACE_I("UnregisteredEvents :: event=%08X, listeners=%d", nEvent,
                    objActivities.GetSize(), 0);
        }
    }
    else
    {
        IMS_TRACE_D("UnregisteredEvents :: no events", 0, 0, 0);
    }
}

PRIVATE VIRTUAL IMS_BOOL EventHolder::EventReceiver_NotifyEvent(
        IN IMS_SINT32 nEvent, IN IMS_UINT32 nWParam, IN IMS_UINT32 nLParam)
{
    LockGuard objLock(m_piLock);

    IMS_SLONG nIndex = m_objEventMap.GetIndexOfKey(nEvent);

    if (nIndex < 0)
    {
        IMS_TRACE_D("Event (%d : %d, %d) is not handled", nEvent, nWParam, nLParam);
        return IMS_FALSE;
    }

    IMS_TRACE_I("EventReceiver :: E (%d), W (%d), L (%d)", nEvent, nWParam, nLParam);

    IMSList<EventActivity*>& objActivities = m_objEventMap.GetValueAt(nIndex);

    if (objActivities.IsEmpty())
    {
        IMS_TRACE_D("There are no event receivers", 0, 0, 0);
        return IMS_FALSE;
    }

    for (IMS_UINT32 i = 0; i < objActivities.GetSize(); ++i)
    {
        EventActivity* pActivity = objActivities.GetAt(i);

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

    EventServicePrivate(IN const EventServicePrivate&) = delete;
    EventServicePrivate& operator=(IN const EventServicePrivate&) = delete;

public:
    inline EventHolder* GetHolder(IN IMS_SINT32 nSlotId)
    {
        if ((nSlotId < IMS_SLOT_0) || (nSlotId >= SystemConfig::GetMaxSimSlot()))
        {
            nSlotId = IMS_SLOT_0;
        }

        return m_ppHolder[nSlotId];
    }

    inline IEventSender* GetSender() const { return m_piSender; }

private:
    IEventSender* m_piSender;

    EventHolder** m_ppHolder;
};

PUBLIC
EventServicePrivate::EventServicePrivate() :
        m_piSender(PlatformFactory::CreateEventSender()),
        m_ppHolder(IMS_NULL)
{
    IMS_SINT32 nSimCount = SystemConfig::GetMaxSimSlot();

    m_ppHolder = new EventHolder*[nSimCount];

    for (IMS_SINT32 i = 0; i < nSimCount; ++i)
    {
        m_ppHolder[i] = new EventHolder(i);
    }
}

PUBLIC VIRTUAL EventServicePrivate::~EventServicePrivate()
{
    if (m_ppHolder != IMS_NULL)
    {
        IMS_SINT32 nSimCount = SystemConfig::GetMaxSimSlot();

        for (IMS_SINT32 i = 0; i < nSimCount; ++i)
        {
            if (m_ppHolder[i] != IMS_NULL)
            {
                delete m_ppHolder[i];
            }
        }

        delete[] m_ppHolder;
    }

    PlatformFactory::DestroyEventSender(m_piSender);
}

// EventService class
PRIVATE
EventService::EventService() :
        m_pPrivate(new EventServicePrivate())
{
}

PRIVATE
EventService::~EventService()
{
    if (m_pPrivate != IMS_NULL)
    {
        delete m_pPrivate;
    }
}

PUBLIC
void EventService::AddListener(
        IN IMS_SINT32 nEvent, IN IEventListener* piListener, IN IMS_SINT32 nSlotId)
{
    EventHolder* pHolder = m_pPrivate->GetHolder(nSlotId);

    pHolder->AddListener(nEvent, piListener);
}

PUBLIC
void EventService::RemoveListener(
        IN IMS_SINT32 nEvent, IN IEventListener* piListener, IN IMS_SINT32 nSlotId)
{
    EventHolder* m_pHolder = m_pPrivate->GetHolder(nSlotId);

    m_pHolder->RemoveListener(nEvent, piListener);
}

PUBLIC
void EventService::SendEvent(
        IN IMS_SINT32 nEvent, IN IMS_UINT32 nWParam, IN IMS_UINT32 nLParam, IN IMS_SINT32 nSlotId)
{
    m_pPrivate->GetSender()->SendEvent(nEvent, nWParam, nLParam, nSlotId);
}

PUBLIC
void EventService::SetUnregisteredEvents(IN IMS_SINT32 nSlotId)
{
    EventHolder* pHolder = m_pPrivate->GetHolder(nSlotId);
    pHolder->SetUnregisteredEvents();
}

PUBLIC GLOBAL EventService* EventService::GetEventService()
{
    static EventService* s_pEventService = IMS_NULL;

    if (s_pEventService == IMS_NULL)
    {
        s_pEventService = new EventService();
    }

    return s_pEventService;
}
