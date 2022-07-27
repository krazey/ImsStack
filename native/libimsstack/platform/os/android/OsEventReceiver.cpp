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
#include "OsEventReceiver.h"
#include "PlatformContext.h"
#include "ServiceMemory.h"
#include "system-intf/SystemConstants.h"

PUBLIC
OsEventReceiver::OsEventReceiver(IN IMS_SINT32 nSlotId) :
        ImsSlot(nSlotId),
        m_piListener(IMS_NULL)
{
}

PUBLIC VIRTUAL OsEventReceiver::~OsEventReceiver() {}

PRIVATE VIRTUAL void OsEventReceiver::ResetEvent(IN IMS_SINT32 nEvent)
{
    ISystem* piSystem = PlatformContext::GetInstance()->GetSystem();

    if (piSystem != IMS_NULL)
    {
        piSystem->ResetEvent(nEvent, GetSlotId());
    }
}

PRIVATE VIRTUAL void OsEventReceiver::SetEvent(IN IMS_SINT32 nEvent)
{
    ISystem* piSystem = PlatformContext::GetInstance()->GetSystem();

    if (piSystem != IMS_NULL)
    {
        piSystem->SetEvent(nEvent, GetSlotId());
    }
}

PRIVATE VIRTUAL void OsEventReceiver::SetListener(IN IEventReceiverListener* piListener)
{
    m_piListener = piListener;

    ISystem* piSystem = PlatformContext::GetInstance()->GetSystem();

    if (m_piListener != IMS_NULL && piSystem != IMS_NULL)
    {
        piSystem->AddListener(SystemConstants::CATEGORY_EVENT, this, GetSlotId());
    }
}

PRIVATE VIRTUAL void OsEventReceiver::System_NotifyEvent(
        IN IMS_UINT32 nEvent, IN IMS_UINTP nWParam, IN IMS_UINTP nLParam)
{
    if (m_piListener != IMS_NULL)
    {
        m_piListener->EventReceiver_NotifyEvent(
                static_cast<IMS_SINT32>(nEvent), LONG_TO_INT(nWParam), LONG_TO_INT(nLParam));
    }
}
