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
#ifndef OS_EVENT_SENDER_H_
#define OS_EVENT_SENDER_H_

#include "IEventSender.h"

class OsEventSender : public IEventSender
{
public:
    OsEventSender();
    virtual ~OsEventSender();

public:
    void SendEvent(IN IMS_SINT32 nEvent, IN IMS_UINT32 nWParam, IN IMS_UINT32 nLParam,
            IN IMS_SINT32 nSlotId) override;
};

#endif
