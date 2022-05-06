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
#include "ServiceEvent.h"
#include "ServiceTrace.h"
#include "device/OsSrvcc.h"
#include "system-intf/System.h"

__IMS_TRACE_TAG_ADAPT__;

PUBLIC
OsSrvcc::OsSrvcc(IN IMS_SINT32 nSlotId) :
        ImsSlot(nSlotId),
        m_bSrvccEventRegistered(IMS_FALSE),
        m_objSrvccListeners(IMSList<ISrvccListener*>())
{
}

PUBLIC VIRTUAL OsSrvcc::~OsSrvcc() {}

PUBLIC VIRTUAL void OsSrvcc::SubscribeSrvccListener(IN ISrvccListener* piListener)
{
    if (piListener == IMS_NULL)
    {
        return;
    }

    for (IMS_UINT32 i = 0; i < m_objSrvccListeners.GetSize(); i++)
    {
        ISrvccListener* piTmpListener = m_objSrvccListeners.GetAt(i);

        if (piTmpListener == IMS_NULL)
        {
            continue;
        }

        if (piTmpListener == piListener)
        {
            // Same listener already exists
            return;
        }
    }

    m_objSrvccListeners.Append(piListener);

    if (m_objSrvccListeners.GetSize() > 0)
    {
        ListenSrvccEvent();
    }
}

PUBLIC VIRTUAL void OsSrvcc::UnsubscribeSrvccListener(IN ISrvccListener* piListener)
{
    if (piListener == IMS_NULL)
    {
        return;
    }

    for (IMS_UINT32 i = 0; i < m_objSrvccListeners.GetSize(); i++)
    {
        ISrvccListener* piTmpListener = m_objSrvccListeners.GetAt(i);

        if (piTmpListener == IMS_NULL)
        {
            continue;
        }

        if (piTmpListener == piListener)
        {
            m_objSrvccListeners.RemoveAt(i);
            break;
        }
    }

    if (m_objSrvccListeners.GetSize() == 0)
    {
        UnlistenSrvccEvent();
    }
}

PROTECTED VIRTUAL void OsSrvcc::Event_NotifyEvent(
        IN IMS_SINT32 nEvent, IN IMS_UINT32 nWParam, IN IMS_UINT32 /*nLParam*/)
{
    switch (nEvent)
    {
        case IMS_EVENT_SRVCC_NOTIFICATION:
            NotifySrvccEvent(nWParam);
            break;

        default:
            break;
    }
}

PRIVATE
void OsSrvcc::ListenSrvccEvent()
{
    if (m_bSrvccEventRegistered)
    {
        return;
    }

    System::GetInstance()->ListenSrvccEvent(GetSlotId());
    m_bSrvccEventRegistered = IMS_TRUE;

    IMS_EVENT_AddListenerForSlotId(IMS_EVENT_SRVCC_NOTIFICATION, this, GetSlotId());
}

PRIVATE
void OsSrvcc::UnlistenSrvccEvent()
{
    if (!m_bSrvccEventRegistered)
    {
        return;
    }

    System::GetInstance()->UnlistenSrvccEvent(GetSlotId());
    m_bSrvccEventRegistered = IMS_FALSE;

    IMS_EVENT_RemoveListenerForSlotId(IMS_EVENT_SRVCC_NOTIFICATION, this, GetSlotId());
}

PRIVATE
void OsSrvcc::NotifySrvccEvent(IN IMS_UINT32 nEvent)
{
    for (IMS_UINT32 i = 0; i < m_objSrvccListeners.GetSize(); i++)
    {
        ISrvccListener* piListener = m_objSrvccListeners.GetAt(i);

        if (piListener != IMS_NULL)
        {
            piListener->Srvcc_NotifyEventChanged(static_cast<IMS_SINT32>(nEvent));
        }
    }
}
