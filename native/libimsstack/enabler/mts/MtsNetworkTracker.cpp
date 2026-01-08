/*
 * Copyright (C) 2025 The Android Open Source Project
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

#include "IImsAos.h"
#include "IImsAosInfo.h"
#include "ImsEventDef.h"
#include "IMtsContext.h"
#include "IMtsService.h"
#include "INetworkWatcher.h"
#include "MtsDef.h"
#include "MtsNetworkTracker.h"
#include "ServiceEvent.h"
#include "ServicePhoneInfo.h"
#include "ServiceTrace.h"

__IMS_TRACE_TAG_COM_MTS__;

MtsNetworkTracker::MtsNetworkTracker(IN IMtsContext& objContext) :
        m_piNetWatcherInfo(
                PhoneInfoService::GetPhoneInfoService()->GetNetworkWatcher(objContext.GetSlotId())),
        m_objContext(objContext),
        m_bDataRoaming(IMS_FALSE),
        m_nLteAttachState(IMS_LTE_INFO_UNKNOWN)
{
    IMS_TRACE_I("+MtsNetworkTracker [slot_%d]", m_objContext.GetSlotId(), 0, 0);

    IMS_EVENT_AddListenerForSlotId(IMS_EVENT_ROAMING_STATE, this, m_objContext.GetSlotId());
    IMS_EVENT_AddListenerForSlotId(IMS_EVENT_LTE_INFO, this, m_objContext.GetSlotId());
}

PUBLIC
MtsNetworkTracker::~MtsNetworkTracker()
{
    IMS_TRACE_I("~MtsNetworkTracker [slot_%d]", m_objContext.GetSlotId(), 0, 0);

    IMS_EVENT_RemoveListenerForSlotId(IMS_EVENT_ROAMING_STATE, this, m_objContext.GetSlotId());
    IMS_EVENT_RemoveListenerForSlotId(IMS_EVENT_LTE_INFO, this, m_objContext.GetSlotId());
}

PUBLIC VIRTUAL void MtsNetworkTracker::Event_NotifyEvent(
        IN IMS_SINT32 nEvent, IN IMS_UINT32 nWParam, IN IMS_UINT32 nLParam)
{
    IMS_TRACE_D("Event_NotifyEvent : Event[%x]/WParam[%d]/LParam[%d]", nEvent, nWParam, nLParam);
    switch (nEvent)
    {
        case IMS_EVENT_ROAMING_STATE:
        {
            m_bDataRoaming = (nWParam == IMS_ROAMING_STATE_ON);
            break;
        }

        case IMS_EVENT_LTE_INFO:
        {
            m_nLteAttachState = nWParam;
            break;
        }

        default:
            break;
    }
}

PUBLIC VIRTUAL IMS_SINT32 MtsNetworkTracker::GetCellularServiceState() const
{
    if (m_piNetWatcherInfo != IMS_NULL)
    {
        return m_piNetWatcherInfo->GetCellularServiceState();
    }

    return INetworkWatcher::STATE_INVALID;
}

PUBLIC VIRTUAL IMS_SINT32 MtsNetworkTracker::GetNetworkType() const
{
    if (m_piNetWatcherInfo != IMS_NULL)
    {
        return m_piNetWatcherInfo->GetNetworkType();
    }

    return INetworkWatcher::RADIOTECH_TYPE_INVALID;
}
