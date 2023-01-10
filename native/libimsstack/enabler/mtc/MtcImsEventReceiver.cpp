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

#include "IMtcImsEventReceiver.h"
#include "MtcImsEventReceiver.h"
#include "ServiceEvent.h"
#include "ServiceTrace.h"

__IMS_TRACE_TAG_COM_MTC__;

// Matched to undefined values in IMSEventDef
const IMS_UINT32 IMtcImsEventReceiver::UNKNOWN_VALUE = -1;

PUBLIC
MtcImsEventReceiver::MtcImsEventReceiver(IN IMS_SINT32 nSlotId) :
        m_nSlotId(nSlotId),
        m_objEvents(ImsMap<ImsEvent, EventEntry*>())
{
    RegisterSupportedEvents();
}

PUBLIC
MtcImsEventReceiver::~MtcImsEventReceiver()
{
    while (m_objEvents.GetSize() > 0)
    {
        DeregisterEvent(m_objEvents.GetKeyAt(0));
    }
}

PUBLIC
IMS_UINT32 MtcImsEventReceiver::GetWParam(IN ImsEvent nEvent)
{
    EventEntry* pEntry = GetEntry(nEvent);
    return pEntry ? pEntry->nWParam : UNKNOWN_VALUE;
}

PUBLIC
IMS_UINT32 MtcImsEventReceiver::GetLParam(IN ImsEvent nEvent)
{
    EventEntry* pEntry = GetEntry(nEvent);
    return pEntry ? pEntry->nLParam : UNKNOWN_VALUE;
}

PUBLIC
void MtcImsEventReceiver::AddListener(IN IMtcImsEventListener* pListener, IN ImsEvent nEvent)
{
    EventEntry* pEntry = GetEntry(nEvent);
    if (pEntry == IMS_NULL)
    {
        return;
    }

    for (IMS_UINT32 nIndex = 0; nIndex < pEntry->lstListeners.GetSize(); nIndex++)
    {
        if (pEntry->lstListeners.GetAt(nIndex) == pListener)
        {
            IMS_TRACE_E(0, "AddListener : Already added for the event [%d]", nEvent, 0, 0);
            return;
        }
    }

    pEntry->lstListeners.Append(pListener);
}

PUBLIC
void MtcImsEventReceiver::RemoveListener(IN IMtcImsEventListener* pListener, IN ImsEvent nEvent)
{
    EventEntry* pEntry = GetEntry(nEvent);
    if (pEntry == IMS_NULL)
    {
        return;
    }

    for (IMS_UINT32 nIndex = 0; nIndex < pEntry->lstListeners.GetSize(); nIndex++)
    {
        if (pEntry->lstListeners.GetAt(nIndex) == pListener)
        {
            pEntry->lstListeners.RemoveAt(nIndex);
            break;
        }
    }
}

PUBLIC VIRTUAL void MtcImsEventReceiver::Event_NotifyEvent(
        IN ImsEvent nEvent, IN IMS_UINT32 nWParam, IN IMS_UINT32 nLParam)
{
    IMS_TRACE_D("Event_NotifyEvent : event[%x] param1[%d] param2[%d]", nEvent, nWParam, nLParam);

    EventEntry* pEntry = GetEntry(nEvent);
    if (pEntry == IMS_NULL)
    {
        return;
    }

    if (pEntry->nWParam == nWParam && pEntry->nLParam == nLParam)
    {
        return;
    }
    pEntry->nWParam = nWParam;
    pEntry->nLParam = nLParam;

    for (IMS_UINT32 nIndex = 0; nIndex < pEntry->lstListeners.GetSize(); nIndex++)
    {
        IMtcImsEventListener* pListener = pEntry->lstListeners.GetAt(nIndex);
        pListener->OnImsEventNotified(nEvent, nWParam, nLParam);
    }
}

PRIVATE
void MtcImsEventReceiver::RegisterSupportedEvents()
{
    RegisterEvent(IMS_EVENT_AC_BARRING_STATE);
    RegisterEvent(IMS_EVENT_CSCALL_STATE);
    RegisterEvent(IMS_EVENT_IMS_VOICE_OVER_PS_STATE);
    RegisterEvent(IMS_EVENT_LTE_INFO);
    RegisterEvent(IMS_EVENT_LTE_STATE);
    RegisterEvent(IMS_EVENT_POWER_LOW_BATTERY);
    RegisterEvent(IMS_EVENT_REG_CONTROL);
    RegisterEvent(IMS_EVENT_ROAMING_STATE);
    RegisterEvent(IMS_EVENT_RTT_SETTING);
    RegisterEvent(IMS_EVENT_VOLTE_SETTING);
    RegisterEvent(IMS_EVENT_WFC_SETTING_CHANGED);
}

PRIVATE
void MtcImsEventReceiver::RegisterEvent(IN ImsEvent nEvent)
{
    IMS_EVENT_AddListenerForSlotId(nEvent, this, m_nSlotId);

    m_objEvents.Add(nEvent, new EventEntry());
}

PRIVATE
void MtcImsEventReceiver::DeregisterEvent(IN ImsEvent nEvent)
{
    IMS_EVENT_RemoveListenerForSlotId(nEvent, this, m_nSlotId);

    IMS_SLONG nIndex = m_objEvents.GetIndexOfKey(nEvent);
    EventEntry* pEntry = m_objEvents.GetValueAt(nIndex);
    m_objEvents.RemoveAt(nIndex);
    delete pEntry;
}

PRIVATE
MtcImsEventReceiver::EventEntry* MtcImsEventReceiver::GetEntry(IN ImsEvent nEvent)
{
    IMS_SLONG nIndex = m_objEvents.GetIndexOfKey(nEvent);
    IMS_ASSERT(nIndex >= 0);
    if (nIndex < 0)
    {
        return IMS_NULL;
    }

    return m_objEvents.GetValueAt(nIndex);
}
