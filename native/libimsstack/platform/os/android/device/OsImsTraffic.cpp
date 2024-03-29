/*
 * Copyright (C) 2023 The Android Open Source Project
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
#include "IImsRadio.h"
#include "ImsMessageDef.h"
#include "PlatformContext.h"
#include "ServiceMessage.h"
#include "ServiceMutex.h"
#include "ServiceTrace.h"
#include "SystemConfig.h"
#include "device/OsImsTraffic.h"
#include "system-intf/SystemConstants.h"

__IMS_TRACE_TAG_ADAPT__;

PUBLIC
OsImsTraffic::OsImsTraffic() :
        m_piLock(IMS_NULL),
        m_objTraffics(ImsMap<IMS_SINT32, Traffic*>()),
        m_objThreadListeners(ImsList<TrafficListeners*>())
{
    m_piLock = MutexService::GetMutexService()->CreateMutex();

    for (IMS_SINT32 i = 0; i < SystemConfig::GetSupportedSimCount(); i++)
    {
        m_objTraffics.Add(i, new Traffic(i, this));
    }
}

PUBLIC VIRTUAL OsImsTraffic::~OsImsTraffic()
{
    for (IMS_UINT32 i = 0; i < m_objTraffics.GetSize(); i++)
    {
        Traffic* pTraffic = m_objTraffics.GetValueAt(i);
        if (pTraffic != IMS_NULL)
        {
            delete pTraffic;
        }
    }

    m_objTraffics.Clear();

    MutexService::GetMutexService()->DestroyMutex(m_piLock);
}

PUBLIC VIRTUAL void OsImsTraffic::Disable(IN IMS_SINT32 nSlotId)
{
    LockGuard objLock(m_piLock);
    Traffic* pTraffic = GetTraffic(nSlotId);

    if (pTraffic != IMS_NULL)
    {
        pTraffic->Enable(IMS_FALSE);
        pTraffic->ClearTraffics();
        pTraffic->StopAllTimers();
        pTraffic->SetWlan(IMS_FALSE);
        pTraffic->SetTopPriorityTraffic(TRAFFIC_PRIORITY_NONE);
    }
}

PUBLIC VIRTUAL IMS_BOOL OsImsTraffic::IsAllowed(IN IMS_SINT32 nSlotId, IN IMS_UINT32 nTrafficType)
{
    if (IsSimultaneousCallingSupported(nSlotId))
    {
        return IMS_TRUE;
    }

    IMS_UINT32 nPriorityType = GetPriorityType(nTrafficType);

    if (nPriorityType == TRAFFIC_PRIORITY_EMERGENCY ||
            nPriorityType == TRAFFIC_PRIORITY_EMERGENCY_SMS)
    {
        return IMS_TRUE;
    }

    SetEnabled(nSlotId);

    if (IsEmergency(nSlotId))
    {
        IMS_TRACE_D("IsAllowed :: [%d] same slot is emergency", nSlotId, 0, 0);
        return IMS_TRUE;
    }

    if (IsEmergencyInOtherSlot(nSlotId))
    {
        IMS_TRACE_D("IsAllowed :: [%d] other slot is emergency", nSlotId, 0, 0);
        return IMS_FALSE;
    }

    if (IsWlan(nSlotId) || IsWlanInOtherSlot(nSlotId))
    {
        IMS_TRACE_D("IsAllowed :: [%d] WLAN is connected", nSlotId, 0, 0);
        return IMS_TRUE;
    }

    if (IsIdle() || (GetTopPrioritizedSlot() == nSlotId))
    {
        return IMS_TRUE;
    }

    if (HasHighPriorityInOtherSlot(nSlotId, nPriorityType))
    {
        return IMS_FALSE;
    }

    return IMS_TRUE;
}

PUBLIC VIRTUAL void OsImsTraffic::Start(IN IMS_SINT32 nSlotId, IN IMS_UINT32 nTrafficType)
{
    IMS_UINT32 nPriorityType = GetPriorityType(nTrafficType);

    if (nPriorityType == TRAFFIC_PRIORITY_NONE)
    {
        return;
    }

    IMS_TRACE_I("Start :: [%d][%s]", nSlotId, PriorityTypeToString(nPriorityType), 0);

    LockGuard objLock(m_piLock);
    Traffic* pTraffic = GetTraffic(nSlotId);

    if (pTraffic == IMS_NULL)
    {
        return;
    }

    if (pTraffic->IsStarted(nPriorityType))
    {
        return;
    }

    pTraffic->Start(nPriorityType);

    if (pTraffic->m_nTopPriorityTraffic < nPriorityType)
    {
        pTraffic->SetTopPriorityTraffic(nPriorityType);

        IMS_TRACE_I(
                "Start :: [%d][%s] top priority", nSlotId, PriorityTypeToString(nPriorityType), 0);

        PlatformContext::GetInstance()->GetSystem()->SetTrafficPriority(nPriorityType, nSlotId);

        PostMessage();
    }

    SetTimer(nPriorityType, pTraffic, IMS_TRUE);
}

PUBLIC VIRTUAL void OsImsTraffic::Stop(IN IMS_SINT32 nSlotId, IN IMS_UINT32 nTrafficType)
{
    IMS_UINT32 nPriorityType = GetPriorityType(nTrafficType);

    if (nPriorityType == TRAFFIC_PRIORITY_NONE)
    {
        return;
    }

    IMS_TRACE_I("Stop :: [%d][%s]", nSlotId, PriorityTypeToString(nPriorityType), 0);

    LockGuard objLock(m_piLock);
    Traffic* pTraffic = GetTraffic(nSlotId);

    if (pTraffic == IMS_NULL)
    {
        return;
    }

    if (!pTraffic->IsStarted(nPriorityType))
    {
        return;
    }

    SetTimer(nPriorityType, pTraffic, IMS_FALSE);
    pTraffic->Stop(nPriorityType);

    if (pTraffic->m_nTopPriorityTraffic == TRAFFIC_PRIORITY_NONE ||
            pTraffic->m_nTopPriorityTraffic < nPriorityType)
    {
        return;
    }

    if (pTraffic->m_nTopPriorityTraffic == nPriorityType)
    {
        if (pTraffic->m_nTraffics == TRAFFIC_PRIORITY_NONE)
        {
            pTraffic->SetTopPriorityTraffic(TRAFFIC_PRIORITY_NONE);
            PlatformContext::GetInstance()->GetSystem()->SetTrafficPriority(
                    TRAFFIC_PRIORITY_NONE, nSlotId);
        }
        else
        {
            IMS_UINT32 nHighPriorityType = GetHighPriorityType(pTraffic->m_nTraffics);

            pTraffic->SetTopPriorityTraffic(nHighPriorityType);
            PlatformContext::GetInstance()->GetSystem()->SetTrafficPriority(
                    nHighPriorityType, nSlotId);
        }

        PostMessage();
    }
}

PUBLIC VIRTUAL void OsImsTraffic::SetSimultaneousCallingSupported(
        IN IMS_SINT32 nSlotId, IN IMS_BOOL bSupported)
{
    LockGuard objLock(m_piLock);
    Traffic* pTraffic = GetTraffic(nSlotId);

    if (pTraffic == IMS_NULL)
    {
        return;
    }

    if (pTraffic->IsSimultaneousCallingSupported() == bSupported)
    {
        return;
    }

    IMS_TRACE_D("SetSimultaneousCallingSupported :: [%d] simultaneous calling support (%s)",
            nSlotId, _TRACE_B_(bSupported), 0);

    pTraffic->SetSimultaneousCallingSupported(bSupported);

    if (bSupported)
    {
        PostMessage();
    }
}

PUBLIC VIRTUAL void OsImsTraffic::SetWlan(IN IMS_SINT32 nSlotId, IN IMS_BOOL bEnabled)
{
    LockGuard objLock(m_piLock);
    Traffic* pTraffic = GetTraffic(nSlotId);

    if (pTraffic == IMS_NULL)
    {
        return;
    }

    if (pTraffic->IsWlan() == bEnabled)
    {
        return;
    }

    IMS_TRACE_D("SetWlan :: [%d] is WLAN (%d)", nSlotId, bEnabled, 0);

    pTraffic->SetWlan(bEnabled);
    PostMessage();
}

PUBLIC VIRTUAL void OsImsTraffic::AddListener(IN IImsTrafficListener* piListener)
{
    if (piListener == IMS_NULL)
    {
        return;
    }

    IThread* piThread = ThreadService::GetThreadService()->GetCurrentThread();

    for (IMS_UINT32 i = 0; i < m_objThreadListeners.GetSize(); i++)
    {
        TrafficListeners* pTrfficListeners = m_objThreadListeners.GetAt(i);

        if (pTrfficListeners == IMS_NULL)
        {
            continue;
        }

        if (*pTrfficListeners == piThread)
        {
            pTrfficListeners->objListeners.Append(piListener);
            return;
        }
    }

    m_objThreadListeners.Append(new TrafficListeners(piListener));
}

PUBLIC VIRTUAL void OsImsTraffic::RemoveListener(IN IImsTrafficListener* piListener)
{
    if (piListener == IMS_NULL)
    {
        return;
    }

    IThread* piThread = ThreadService::GetThreadService()->GetCurrentThread();

    for (IMS_UINT32 i = 0; i < m_objThreadListeners.GetSize(); i++)
    {
        TrafficListeners* pTrfficListeners = m_objThreadListeners.GetAt(i);

        if (pTrfficListeners == IMS_NULL)
        {
            continue;
        }

        if (*pTrfficListeners == piThread)
        {
            for (IMS_UINT32 j = 0; j < pTrfficListeners->objListeners.GetSize(); j++)
            {
                IImsTrafficListener* piTrafficListener = pTrfficListeners->objListeners.GetAt(j);

                if (piListener == piTrafficListener)
                {
                    pTrfficListeners->objListeners.RemoveAt(j);
                    break;
                }
            }
            break;
        }
    }
}

PUBLIC VIRTUAL void OsImsTraffic::ImsTrafficTimer_Expired(
        IN IMS_UINT32 nSlotId, IN IMS_UINT32 nType)
{
    IMS_TRACE_D("ImsTrafficTimer_Expired :: [%d][%s]", nSlotId, PriorityTypeToString(nType), 0);

    Stop(nSlotId, GetTrafficType(nType));
}

PUBLIC VIRTUAL void OsImsTraffic::DispatchServiceMessage(
        IN IMS_UINTP /* nWparam */, IN IMS_UINTP /* nLparam */)
{
    IThread* piThread = ThreadService::GetThreadService()->GetCurrentThread();

    for (IMS_UINT32 i = 0; i < m_objThreadListeners.GetSize(); i++)
    {
        TrafficListeners* pTrfficListeners = m_objThreadListeners.GetAt(i);

        if (pTrfficListeners == IMS_NULL)
        {
            continue;
        }

        if (*pTrfficListeners == piThread)
        {
            for (IMS_UINT32 j = 0; j < pTrfficListeners->objListeners.GetSize(); j++)
            {
                IImsTrafficListener* piTrafficListener = pTrfficListeners->objListeners.GetAt(j);

                if (piTrafficListener != IMS_NULL)
                {
                    piTrafficListener->ImsTraffic_OnPriorityChanged();
                }
            }

            break;
        }
    }
}

PRIVATE IMS_SINT32 OsImsTraffic::GetTopPrioritizedSlot()
{
    LockGuard objLock(m_piLock);
    IMS_SINT32 nTopSlot = 0;

    for (IMS_SINT32 i = 1; i < m_objTraffics.GetSize(); i++)
    {
        Traffic* pTopSlotTraffic = m_objTraffics.GetValue(nTopSlot);
        Traffic* pTraffic = m_objTraffics.GetValue(i);

        if (pTopSlotTraffic == IMS_NULL || pTraffic == IMS_NULL)
        {
            continue;
        }

        if (pTopSlotTraffic->IsWlan() || pTopSlotTraffic->m_nTraffics < pTraffic->m_nTraffics)
        {
            nTopSlot = i;
        }
    }

    return nTopSlot;
}

PRIVATE Traffic* OsImsTraffic::GetTraffic(IN IMS_SINT32 nSlotId) const
{
    IMS_SLONG nIndex = m_objTraffics.GetIndexOfKey(nSlotId);

    if (nIndex < 0)
    {
        return IMS_NULL;
    }

    return m_objTraffics.GetValueAt(nIndex);
}

PRIVATE IMS_BOOL OsImsTraffic::HasHighPriorityInOtherSlot(
        IN IMS_SINT32 nSlotId, IN IMS_UINT32 nType) const
{
    LockGuard objLock(m_piLock);

    for (IMS_UINT32 i = 0; i < m_objTraffics.GetSize(); i++)
    {
        if (i == nSlotId)
        {
            continue;
        }

        Traffic* pTraffic = m_objTraffics.GetValue(i);

        if (pTraffic == IMS_NULL)
        {
            continue;
        }

        if (pTraffic->IsWlan())
        {
            continue;
        }

        if (pTraffic->m_nTopPriorityTraffic >= nType)
        {
            return IMS_TRUE;
        }
    }

    return IMS_FALSE;
}

PRIVATE IMS_BOOL OsImsTraffic::IsEmergency(IN IMS_SINT32 nSlotId) const
{
    LockGuard objLock(m_piLock);

    Traffic* pTraffic = GetTraffic(nSlotId);
    if (pTraffic == IMS_NULL)
    {
        return IMS_FALSE;
    }

    return pTraffic->IsEmergency();
}

PRIVATE IMS_BOOL OsImsTraffic::IsEmergencyInOtherSlot(IN IMS_SINT32 nSlotId) const
{
    LockGuard objLock(m_piLock);

    for (IMS_UINT32 i = 0; i < m_objTraffics.GetSize(); i++)
    {
        if (i == nSlotId)
        {
            continue;
        }

        Traffic* pTraffic = m_objTraffics.GetValue(i);

        if (pTraffic == IMS_NULL)
        {
            continue;
        }

        if (pTraffic->IsEmergency())
        {
            return IMS_TRUE;
        }
    }

    return IMS_FALSE;
}

PRIVATE IMS_BOOL OsImsTraffic::IsEnabled(IN IMS_SINT32 nSlotId) const
{
    LockGuard objLock(m_piLock);

    Traffic* pTraffic = GetTraffic(nSlotId);
    if (pTraffic == IMS_NULL)
    {
        return IMS_FALSE;
    }

    return pTraffic->IsEnabled();
}

PRIVATE IMS_BOOL OsImsTraffic::IsIdle() const
{
    LockGuard objLock(m_piLock);

    for (IMS_UINT32 i = 0; i < m_objTraffics.GetSize(); i++)
    {
        Traffic* pTraffic = m_objTraffics.GetValue(i);

        if (pTraffic == IMS_NULL)
        {
            continue;
        }

        if (pTraffic->m_nTraffics != IImsTraffic::TRAFFIC_PRIORITY_NONE)
        {
            return IMS_FALSE;
        }
    }

    return IMS_TRUE;
}

PRIVATE IMS_BOOL OsImsTraffic::IsSimultaneousCallingSupported(IN IMS_SINT32 nSlotId) const
{
    LockGuard objLock(m_piLock);
    Traffic* pTraffic = GetTraffic(nSlotId);

    if (pTraffic == IMS_NULL)
    {
        return IMS_FALSE;
    }

    return pTraffic->IsSimultaneousCallingSupported();
}

PRIVATE IMS_BOOL OsImsTraffic::IsWlan(IN IMS_SINT32 nSlotId) const
{
    LockGuard objLock(m_piLock);
    Traffic* pTraffic = GetTraffic(nSlotId);

    if (pTraffic == IMS_NULL)
    {
        return IMS_FALSE;
    }

    return pTraffic->IsWlan();
}

PRIVATE IMS_BOOL OsImsTraffic::IsWlanInOtherSlot(IN IMS_SINT32 nSlotId) const
{
    if (m_objTraffics.GetSize() == 1)
    {
        return IMS_FALSE;
    }

    for (IMS_UINT32 i = 0; i < m_objTraffics.GetSize(); i++)
    {
        if (i == nSlotId)
        {
            continue;
        }

        Traffic* pTraffic = m_objTraffics.GetValue(i);

        if (pTraffic == IMS_NULL)
        {
            return IMS_FALSE;
        }

        if (!pTraffic->IsWlan())
        {
            return IMS_FALSE;
        }
    }

    return IMS_TRUE;
}

PRIVATE void OsImsTraffic::PostMessage()
{
    IMS_UINT32 nEnabledCount = 0;

    for (IMS_UINT32 i = 0; i < m_objTraffics.GetSize(); i++)
    {
        Traffic* pTraffic = m_objTraffics.GetValue(i);

        if (pTraffic == IMS_NULL)
        {
            continue;
        }

        if (pTraffic->IsEnabled())
        {
            nEnabledCount++;
        }
    }

    if (nEnabledCount < 2)
    {
        return;
    }

    for (IMS_UINT32 i = 0; i < m_objThreadListeners.GetSize(); i++)
    {
        TrafficListeners* pTrfficListeners = m_objThreadListeners.GetAt(i);

        if (pTrfficListeners == IMS_NULL)
        {
            continue;
        }

        IMS_MSG_CreateNPostThreadMessage(pTrfficListeners->piOwnerThread, IMS_MSG_TRAFFIC, 0, 0);
    }
}

PRIVATE void OsImsTraffic::SetEnabled(IN IMS_SINT32 nSlotId)
{
    LockGuard objLock(m_piLock);

    Traffic* pTraffic = GetTraffic(nSlotId);
    if (pTraffic != IMS_NULL)
    {
        pTraffic->Enable(IMS_TRUE);
    }
}

PRIVATE void OsImsTraffic::SetTimer(IN IMS_SINT32 nType, Traffic* pTraffic, IN IMS_BOOL bStart)
{
    if (nType == TRAFFIC_PRIORITY_SMS || nType == TRAFFIC_PRIORITY_REGISTRATION)
    {
        pTraffic->SetTimer(nType, bStart);
    }
}

PRIVATE GLOBAL IMS_UINT32 OsImsTraffic::GetHighPriorityType(IN IMS_UINT32 nTraffics)
{
    if ((nTraffics & TRAFFIC_PRIORITY_EMERGENCY) > 0)
    {
        return TRAFFIC_PRIORITY_EMERGENCY;
    }

    if ((nTraffics & TRAFFIC_PRIORITY_EMERGENCY_SMS) > 0)
    {
        return TRAFFIC_PRIORITY_EMERGENCY_SMS;
    }

    if ((nTraffics & TRAFFIC_PRIORITY_VOICE) > 0)
    {
        return TRAFFIC_PRIORITY_VOICE;
    }

    if ((nTraffics & TRAFFIC_PRIORITY_VIDEO) > 0)
    {
        return TRAFFIC_PRIORITY_VIDEO;
    }

    if ((nTraffics & TRAFFIC_PRIORITY_SMS) > 0)
    {
        return TRAFFIC_PRIORITY_SMS;
    }

    if ((nTraffics & TRAFFIC_PRIORITY_REGISTRATION) > 0)
    {
        return TRAFFIC_PRIORITY_REGISTRATION;
    }

    return TRAFFIC_PRIORITY_NONE;
}

PRIVATE GLOBAL IMS_UINT32 OsImsTraffic::GetPriorityType(IN IMS_UINT32 nTrafficType)
{
    switch (nTrafficType)
    {
        case IImsRadio::TRAFFIC_TYPE_EMERGENCY:
            return TRAFFIC_PRIORITY_EMERGENCY;

        case IImsRadio::TRAFFIC_TYPE_EMERGENCY_SMS:
            return TRAFFIC_PRIORITY_EMERGENCY_SMS;

        case IImsRadio::TRAFFIC_TYPE_VOICE:
            return TRAFFIC_PRIORITY_VOICE;

        case IImsRadio::TRAFFIC_TYPE_VIDEO:
            return TRAFFIC_PRIORITY_VIDEO;

        case IImsRadio::TRAFFIC_TYPE_SMS:
            return TRAFFIC_PRIORITY_SMS;

        case IImsRadio::TRAFFIC_TYPE_REGISTRATION:
            return TRAFFIC_PRIORITY_REGISTRATION;

        default:
            return TRAFFIC_PRIORITY_NONE;
    }
}

PRIVATE GLOBAL IMS_UINT32 OsImsTraffic::GetTrafficType(IN IMS_UINT32 nPriorityType)
{
    switch (nPriorityType)
    {
        case TRAFFIC_PRIORITY_EMERGENCY:
            return IImsRadio::TRAFFIC_TYPE_EMERGENCY;

        case TRAFFIC_PRIORITY_EMERGENCY_SMS:
            return IImsRadio::TRAFFIC_TYPE_EMERGENCY_SMS;

        case TRAFFIC_PRIORITY_VOICE:
            return IImsRadio::TRAFFIC_TYPE_VOICE;

        case TRAFFIC_PRIORITY_VIDEO:
            return IImsRadio::TRAFFIC_TYPE_VIDEO;

        case TRAFFIC_PRIORITY_SMS:
            return IImsRadio::TRAFFIC_TYPE_SMS;

        case TRAFFIC_PRIORITY_REGISTRATION:
            return IImsRadio::TRAFFIC_TYPE_REGISTRATION;

        default:
            return 0;
    }
}

PRIVATE GLOBAL const IMS_CHAR* OsImsTraffic::PriorityTypeToString(IN IMS_UINT32 nType)
{
    switch (nType)
    {
        case TRAFFIC_PRIORITY_EMERGENCY:
            return "EMERGENCY";

        case TRAFFIC_PRIORITY_EMERGENCY_SMS:
            return "EMERGENCY_SMS";

        case TRAFFIC_PRIORITY_VOICE:
            return "VOICE";

        case TRAFFIC_PRIORITY_VIDEO:
            return "VIDEO";

        case TRAFFIC_PRIORITY_SMS:
            return "SMS";

        case TRAFFIC_PRIORITY_REGISTRATION:
            return "REGISTRATION";

        default:
            return "NONE";
    }
}
