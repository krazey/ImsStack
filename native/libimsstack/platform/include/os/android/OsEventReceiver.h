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
#ifndef OS_EVENT_RECEIVER_H_
#define OS_EVENT_RECEIVER_H_

#include "IEventReceiver.h"
#include "ISystemListener.h"
#include "ImsSlot.h"

class OsEventReceiver : public ImsSlot, public IEventReceiver, public ISystemListener
{
public:
    OsEventReceiver(IN IMS_SINT32 nSlotId);
    virtual ~OsEventReceiver();

private:
    // IEventReceiver class
    void ResetEvent(IN IMS_SINT32 nEvent) override;
    void SetEvent(IN IMS_SINT32 nEvent) override;
    void SetListener(IN IEventReceiverListener* piListener) override;

    // ISystemListener class
    void System_NotifyEvent(
            IN IMS_UINT32 nEvent, IN IMS_UINTP nWParam, IN IMS_UINTP nLParam) override;

private:
    IEventReceiverListener* m_piListener;
};

#endif
