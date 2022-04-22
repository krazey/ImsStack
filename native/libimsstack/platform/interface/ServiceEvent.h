/*
    Author
    <table>
    date      author                    description
    --------  --------------            ----------
    20101006  hwangoo.park@             Created
    </table>

    Description

*/

#ifndef _SERVICE_IMS_EVENT_H_
#define _SERVICE_IMS_EVENT_H_

#include "ImsEventDef.h"
#include "IEventListener.h"

class EventServicePrivate;

// Event Service class
class EventService
{
private:
    EventService();
    ~EventService();

    EventService(IN const EventService& objRHS);
    EventService& operator=(IN const EventService& objRHS);

public:
    void AddListener(IN IMS_SINT32 nEvent, IN IEventListener *piListener,
            IN IMS_SINT32 nSlotId);

    void RemoveListener(IN IMS_SINT32 nEvent, IN IEventListener *piListener,
            IN IMS_SINT32 nSlotId);

    void SendEvent(IN IMS_SINT32 nEvent, IN IMS_UINT32 nWParam, IN IMS_UINT32 nLParam,
            IN IMS_SINT32 nSlotId);

    void SetUnregisteredEvents(IN IMS_SINT32 nSlotId);

    static EventService* GetEventService();

private:
    EventServicePrivate *pPrivate;
};

#define IMS_EVENT_AddListenerForSlotId(nEvent, piListener, nSlotId) \
        EventService::GetEventService()->AddListener(nEvent, piListener, nSlotId)

#define IMS_EVENT_RemoveListenerForSlotId(nEvent, piListener, nSlotId) \
        EventService::GetEventService()->RemoveListener(nEvent, piListener, nSlotId)

#define IMS_EVENT_SendEventForSlotId(nEvent, nWParam, nLParam, nSlotId) \
        EventService::GetEventService()->SendEvent(nEvent, nWParam, nLParam, nSlotId)

#endif // _SERVICE_IMS_EVENT_H_
