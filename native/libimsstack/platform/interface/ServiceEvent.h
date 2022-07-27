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
#ifndef SERVICE_EVENT_H_
#define SERVICE_EVENT_H_

#include "IEventListener.h"
#include "ImsEventDef.h"
#include "PlatformService.h"

class EventServicePrivate;

// Event Service class
class EventService : public PlatformService
{
public:
    EventService();
    EventService(IN const EventService&) = delete;
    EventService& operator=(IN const EventService&) = delete;

protected:
    virtual ~EventService();

public:
    virtual void AddListener(
            IN IMS_SINT32 nEvent, IN IEventListener* piListener, IN IMS_SINT32 nSlotId);
    virtual void RemoveListener(
            IN IMS_SINT32 nEvent, IN IEventListener* piListener, IN IMS_SINT32 nSlotId);
    virtual void SendEvent(IN IMS_SINT32 nEvent, IN IMS_UINT32 nWParam, IN IMS_UINT32 nLParam,
            IN IMS_SINT32 nSlotId);

    void SetUnregisteredEvents(IN IMS_SINT32 nSlotId);

    static EventService* GetEventService();

private:
    EventServicePrivate* m_pPrivate;
};

#define IMS_EVENT_AddListenerForSlotId(nEvent, piListener, nSlotId) \
    EventService::GetEventService()->AddListener(nEvent, piListener, nSlotId)

#define IMS_EVENT_RemoveListenerForSlotId(nEvent, piListener, nSlotId) \
    EventService::GetEventService()->RemoveListener(nEvent, piListener, nSlotId)

#define IMS_EVENT_SendEventForSlotId(nEvent, nWParam, nLParam, nSlotId) \
    EventService::GetEventService()->SendEvent(nEvent, nWParam, nLParam, nSlotId)

#endif
