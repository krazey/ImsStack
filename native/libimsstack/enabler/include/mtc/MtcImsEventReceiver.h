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

#ifndef MTC_IMS_EVENT_RECEIVER_H_
#define MTC_IMS_EVENT_RECEIVER_H_

#include "IEventListener.h"
#include "IMtcImsEventReceiver.h"
#include "ImsList.h"
#include "ImsMap.h"
#include "ImsTypeDef.h"

/*
 * This class receives event from Java layer defined in ImsEventDef.h. The users can read
 * cached values or receive notification when the event occurs.
 * For supported events, see `RegisterSupportedEvents()`.
 */
class MtcImsEventReceiver final : public IEventListener, public IMtcImsEventReceiver
{
public:
    explicit MtcImsEventReceiver(IN IMS_SINT32 nSlotId);
    virtual ~MtcImsEventReceiver();
    MtcImsEventReceiver(const MtcImsEventReceiver&) = delete;
    MtcImsEventReceiver& operator=(const MtcImsEventReceiver&) = delete;

    IMS_UINT32 GetWParam(IN ImsEvent nEvent) override;
    IMS_UINT32 GetLParam(IN ImsEvent nEvent) override;
    void AddListener(IN IMtcImsEventListener* pListener, IN ImsEvent nEvent) override;
    void RemoveListener(IN IMtcImsEventListener* pListener, IN ImsEvent nEvent) override;

    void Event_NotifyEvent(
            IN ImsEvent nEvent, IN IMS_UINT32 nWParam, IN IMS_UINT32 nLParam) override;

private:
    struct EventEntry
    {
        IMSList<IMtcImsEventListener*> lstListeners;
        IMS_UINT32 nWParam = UNKNOWN_VALUE;
        IMS_UINT32 nLParam = UNKNOWN_VALUE;
    };

    void RegisterSupportedEvents();
    void RegisterEvent(IN ImsEvent nEvent);
    void DeregisterEvent(IN ImsEvent nEvent);

    EventEntry* GetEntry(IN ImsEvent nEvent);

    IMS_SINT32 m_nSlotId;
    IMSMap<ImsEvent, EventEntry*> m_objEvents;
};

#endif
