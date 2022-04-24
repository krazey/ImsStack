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
#include "ImsEventDef.h"
#include "ServiceMutex.h"
#include "ServiceTrace.h"
#include "SystemConfig.h"
#include "device/OsTrm.h"
#include "system-intf/System.h"
#include "system-intf/SystemConstants.h"

__IMS_TRACE_TAG_ADAPT__;

PUBLIC
OsTrm::OsTrm()
    : m_piLock(IMS_NULL)
    , m_objTrms(IMSMap<IMS_UINT32, TrmInfo*>())
{
    IMS_TRACE_I("OsTrm", 0, 0, 0);

    if (!IsTrmSupported())
    {
        return;
    }

    IMS_TRACE_D("TRM supported", 0, 0, 0);

    System::GetInstance()->AddListener(
            SystemConstants::CATEGORY_TRM, this, IMS_SLOT_0);

    m_piLock = MutexService::GetMutexService()->CreateMutex();

    for (IMS_SINT32 i = 0; i < SystemConfig::GetMaxSimSlot(); ++i)
    {
        m_objTrms.Add(i, new TrmInfo(static_cast<IMS_UINT32>(i), this));
    }
}

PUBLIC VIRTUAL
OsTrm::~OsTrm()
{
    for (IMS_UINT32 i = 0; i < m_objTrms.GetSize(); ++i)
    {
        TrmInfo* pTrmInfo = m_objTrms.GetValueAt(i);

        if (pTrmInfo != IMS_NULL)
        {
            delete pTrmInfo;
        }
    }

    m_objTrms.Clear();

    MutexService::GetMutexService()->DestroyMutex(m_piLock);

    System::GetInstance()->RemoveListener(
            SystemConstants::CATEGORY_TRM, this, IMS_SLOT_0);
}

PUBLIC VIRTUAL
void OsTrm::Enable(IN IMS_UINT32 nSlotId)
{
    LockGuard objLock(m_piLock);

    TrmInfo* pTrmInfo = GetTrmInfo(nSlotId);
    if (pTrmInfo != IMS_NULL)
    {
        pTrmInfo->Enable(IMS_TRUE);
        pTrmInfo->ClearEmergency();
        pTrmInfo->ClearServices();
        pTrmInfo->SetUpdatedService(SERVICE_NONE);
        SetTrmInfo(pTrmInfo, SERVICE_NONE, nSlotId);
    }
}

PUBLIC VIRTUAL
void OsTrm::Disable(IN IMS_UINT32 nSlotId)
{
    LockGuard objLock(m_piLock);

    TrmInfo* pTrmInfo = GetTrmInfo(nSlotId);
    if (pTrmInfo != IMS_NULL)
    {
        pTrmInfo->Enable(IMS_FALSE);
        pTrmInfo->ClearEmergency();
        pTrmInfo->ClearServices();
        pTrmInfo->SetIpcan(IIpcan::CATEGORY_MOBILE);
        pTrmInfo->SetUpdatedService(SERVICE_NONE);
        SetTrmInfo(pTrmInfo, SERVICE_NONE, nSlotId);
    }
}

PUBLIC VIRTUAL
IMS_BOOL OsTrm::IsServiceAvailable(IN IMS_UINT32 nSlotId, IN IMS_UINT32 nType)
{
    if (!IsEnabled(nSlotId))
    {
        IMS_TRACE_I("IsServiceAvailable :: [%d] TRM is not enabled", nSlotId, 0, 0);
        return IMS_FALSE;
    }

    if (IsEmergency(nSlotId))
    {
        IMS_TRACE_I("IsServiceAvailable :: [%d] same slot is emergency", nSlotId, 0, 0);
        return IMS_TRUE;
    }

    if (IsEmergencyInOtherSlot(nSlotId))
    {
        IMS_TRACE_I("IsServiceAvailable :: [%d] other slot is emergency", nSlotId, 0, 0);
        return IMS_FALSE;
    }

    if (IsWlan(nSlotId))
    {
        IMS_TRACE_I("IsServiceAvailable :: [%d] WLAN is connected", nSlotId, 0, 0);
        return IMS_TRUE;
    }

    if (IsIdle() || (GetUpdatedTopPrioritySlot() == nSlotId))
    {
        return IMS_TRUE;
    }

    if (IsHighPriortyExistInOtherSlot(nSlotId, nType))
    {
        return IMS_FALSE;
    }

    return IMS_TRUE;
}

PUBLIC VIRTUAL
IMS_BOOL OsTrm::IsTrmSupported()
{
    return SystemConfig::IsMultiLteEnabled();
}

PUBLIC VIRTUAL
void OsTrm::SetEmergencyService(IN IMS_UINT32 nSlotId, IN IMS_UINT32 nType,
        IN IMS_UINT32 nMode)
{
    IMS_TRACE_I("SetEmergencyService :: send TRM [%d][%s][mode=%d]", nSlotId,
        ServiceToString(nType), nMode);

    LockGuard objLock(m_piLock);

    TrmInfo* pTrmInfo = GetTrmInfo(nSlotId);
    if (pTrmInfo == IMS_NULL)
    {
        return;
    }

    if (IsSameERequest(pTrmInfo, nType, nMode))
    {
        return;
    }

    if (nMode == MODE_START)
    {
        SetEmergencyService_Start(pTrmInfo, nType, nSlotId);
    }
    else
    {
        SetEmergencyService_Stop(pTrmInfo, nType, nSlotId);
    }
}

PUBLIC VIRTUAL
void OsTrm::SetIpcan(IN IMS_UINT32 nSlotId, IN IMS_UINT32 nCategory)
{
    LockGuard objLock(m_piLock);

    TrmInfo* pTrmInfo = GetTrmInfo(nSlotId);
    if (pTrmInfo == IMS_NULL)
    {
        return;
    }

    if (pTrmInfo->m_nIpcanCategory == nCategory)
    {
        return;
    }

    pTrmInfo->SetIpcan(nCategory);

    if (IsEmergency())
    {
        IMS_TRACE_I("SetIpcan :: [%d][cat=%d] emergency service is going", nSlotId, nCategory, 0);
        return;
    }

    if (nCategory == IIpcan::CATEGORY_WLAN)
    {
        SetIpcan_Wlan(nSlotId);
    }
    else // IIpcan::CATEGORY_MOBILE
    {
        SetIpcan_Mobile(pTrmInfo, nSlotId);
    }
}

PUBLIC VIRTUAL
IMS_BOOL OsTrm::SetService(IN IMS_UINT32 nSlotId, IN IMS_UINT32 nType, IN IMS_UINT32 nMode)
{
    IMS_TRACE_I("SetService :: send TRM [%d][%s][mode=%d]", nSlotId, ServiceToString(nType)
        , nMode);

    if (nMode == MODE_START)
    {
        if(!IsServiceAvailable(nSlotId, nType))
        {
            IMS_TRACE_I("SetService :: not ready [%d][%s][START]", nSlotId,
                ServiceToString(nType), 0);
            return IMS_FALSE;
        }
    }

    LockGuard objLock(m_piLock);

    TrmInfo* pTrmInfo = GetTrmInfo(nSlotId);
    if (pTrmInfo == IMS_NULL)
    {
        return IMS_FALSE;
    }

    if (IsSameRequest(pTrmInfo, nType, nMode))
    {
        return IMS_TRUE;
    }

    if (IsNoTrmReport(pTrmInfo, nType, nMode))
    {
        return IMS_TRUE;
    }

    if (nMode == MODE_START)
    {
        SetService_Start(pTrmInfo, nType, nSlotId);
    }
    else
    {
        SetService_Stop(pTrmInfo, nType, nSlotId);
    }

    return IMS_TRUE;
}

PUBLIC VIRTUAL
void OsTrm::TrmTimer_TimerExpired(IN IMS_UINT32 nSlotId, IN IMS_UINT32 nType)
{
    IMS_TRACE_D("TrmTimer_TimerExpired :: [%d] type=%s", nSlotId, ServiceToString(nType), 0);

    SetService(nSlotId, nType, MODE_END);
}

PUBLIC VIRTUAL
void OsTrm::System_NotifyEvent(IN IMS_UINT32 nEvent,
        IN IMS_UINTP nWParam, IN IMS_UINTP nLParam)
{
    (void)nLParam;

    IMS_TRACE_D("System_NotifyEvent :: event=%d, wp=%" PFLS_d ", lp=%" PFLS_d,
            nEvent, nWParam, nLParam);

    switch (nEvent)
    {
        case IMS_SYSTEM_TRM_SERVICE_CHANGED:
            UpdateServiceStateChanged(IMS_HIWORD(LONG_TO_INT(nWParam)),
                    IMS_LOWORD(LONG_TO_INT(nWParam)), LONG_TO_INT(nLParam));
            break;
        default:
            break;
    }
}

PRIVATE
IMS_UINT32 OsTrm::GetHighPriorityService(IN IMS_UINT32 nServices)
{
    IMS_UINT32 nHighService = SERVICE_NONE;

    if ((nServices & SERVICE_VOLTE) > 0)
    {
        nHighService = SERVICE_VOLTE;
    }
    else if ((nServices & SERVICE_SMS) > 0)
    {
        nHighService = SERVICE_SMS;
    }
    else if ((nServices & SERVICE_REG) > 0)
    {
        nHighService = SERVICE_REG;
    }
    else if ((nServices & SERVICE_UT) > 0)
    {
        nHighService = SERVICE_UT;
    }

    return nHighService;
}

PRIVATE
IMS_UINT32 OsTrm::GetTopPrioritySlot()
{
    IMS_UINT32 nTopSlot = 0;

    for (IMS_UINT32 i = 1; i < m_objTrms.GetSize(); i++)
    {
        TrmInfo* pTopSlotTrmInfo = m_objTrms.GetValue(nTopSlot);
        TrmInfo* pCurrentTrmInfo = m_objTrms.GetValue(i);
        if (pTopSlotTrmInfo == IMS_NULL || pCurrentTrmInfo == IMS_NULL)
        {
            continue;
        }

        if (pTopSlotTrmInfo->m_nServices < pCurrentTrmInfo->m_nServices)
        {
            nTopSlot = i;
        }
    }

    return nTopSlot;
}

PRIVATE
IMS_UINT32 OsTrm::GetUpdatedTopPrioritySlot()
{
    // FIXME: check equal case
    IMS_UINT32 nTopSlot = 0;

    LockGuard objLock(m_piLock);

    for (IMS_UINT32 i = 1; i < m_objTrms.GetSize(); i++)
    {
        TrmInfo* pTopSlotTrmInfo = m_objTrms.GetValue(nTopSlot);
        TrmInfo* pCurrentTrmInfo = m_objTrms.GetValue(i);
        if (pTopSlotTrmInfo == IMS_NULL || pCurrentTrmInfo == IMS_NULL)
        {
            continue;
        }

        if (pTopSlotTrmInfo->m_nUpdatedService < pCurrentTrmInfo->m_nUpdatedService) {
            nTopSlot = i;
        }
    }

    return nTopSlot;
}

PRIVATE
TrmInfo* OsTrm::GetTrmInfo(IN IMS_UINT32 nSlotId)
{
    IMS_SLONG nIndex = m_objTrms.GetIndexOfKey(nSlotId);

    if (nIndex < 0)
    {
        return IMS_NULL;
    }

    return m_objTrms.GetValueAt(nIndex);
}

PRIVATE
IMS_UINT32 OsTrm::GetTrmType(IN IMS_UINT32 nType)
{
    if (nType == SERVICE_VOLTE)
    {
        return TRM_TYPE_VOLTE;
    }
    else if (nType == SERVICE_SMS)
    {
        return TRM_TYPE_SMS;
    }
    else if (nType == SERVICE_REG)
    {
        return TRM_TYPE_REGISTRATION;
    }
    else if (nType == SERVICE_UT)
    {
        return TRM_TYPE_UT;
    }

    return TRM_TYPE_NONE;
}

PRIVATE
IMS_BOOL OsTrm::IsEnabled(IN IMS_UINT32 nSlotId)
{
    LockGuard objLock(m_piLock);

    TrmInfo* pTrmInfo = GetTrmInfo(nSlotId);
    if (pTrmInfo == IMS_NULL)
    {
        return IMS_FALSE;
    }

    return pTrmInfo->m_bStarted;
}

PRIVATE
IMS_BOOL OsTrm::IsEmergency()
{
    for (IMS_UINT32 i = 0; i < m_objTrms.GetSize(); i++) {
        TrmInfo* pTrmInfo = m_objTrms.GetValue(i);
        if (pTrmInfo == IMS_NULL)
        {
            continue;
        }

        if (pTrmInfo->m_nUpdatedEmergencyService != SERVICE_NONE)
        {
            return IMS_TRUE;
        }
    }

    return IMS_FALSE;
}

PRIVATE
IMS_BOOL OsTrm::IsEmergency(IN IMS_UINT32 nSlotId)
{
    LockGuard objLock(m_piLock);

    TrmInfo* pTrmInfo = GetTrmInfo(nSlotId);
    if (pTrmInfo == IMS_NULL)
    {
        return IMS_FALSE;
    }

    return (pTrmInfo->m_nUpdatedEmergencyService != SERVICE_NONE);
}

PRIVATE
IMS_BOOL OsTrm::IsEmergencyInOtherSlot(IN IMS_UINT32 nSlotId)
{
    LockGuard objLock(m_piLock);

    for (IMS_UINT32 i = 0; i < m_objTrms.GetSize(); i++) {
        if (i == nSlotId)
        {
            continue;
        }

        TrmInfo* pTrmInfo = m_objTrms.GetValue(i);
        if (pTrmInfo == IMS_NULL)
        {
            continue;
        }

        if (pTrmInfo->m_nUpdatedEmergencyService != SERVICE_NONE) {
            return IMS_TRUE;
        }
    }

    return IMS_FALSE;
}

PRIVATE
IMS_BOOL OsTrm::IsHighPriortyExistInOtherSlot(IN IMS_UINT32 nSlotId, IN IMS_UINT32 nType)
{
    LockGuard objLock(m_piLock);

    for (IMS_UINT32 i = 0; i < m_objTrms.GetSize(); i++) {
        TrmInfo* pTrmInfo = m_objTrms.GetValue(i);
        if (pTrmInfo == IMS_NULL || i == nSlotId)
        {
            continue;
        }

        if (pTrmInfo->m_nUpdatedService >= nType) {
            return IMS_TRUE;
        }
    }

    return IMS_FALSE;
}

PRIVATE
IMS_BOOL OsTrm::IsIdle()
{
    LockGuard objLock(m_piLock);

    for (IMS_UINT32 i = 0; i < m_objTrms.GetSize(); i++) {
        TrmInfo* pTrmInfo = m_objTrms.GetValue(i);
        if (pTrmInfo == IMS_NULL)
        {
            continue;
        }

        if (pTrmInfo->m_nUpdatedService != SERVICE_NONE) {
            return IMS_FALSE;
        }
    }

    return IMS_TRUE;
}

PRIVATE
IMS_BOOL OsTrm::IsNoTrmReport(IN TrmInfo* pTrmInfo, IN IMS_UINT32 nType, IN IMS_UINT32 nMode)
{
    if (pTrmInfo->m_nUpdatedEmergencyService != SERVICE_NONE || pTrmInfo->IsWlan())
    {
        if (nMode == MODE_START)
        {
            pTrmInfo->Start(nType);
        }
        else
        {
            pTrmInfo->Stop(nType);
        }

        return IMS_TRUE;
    }

    return IMS_FALSE;
}

PRIVATE
IMS_BOOL OsTrm::IsSameRequest(IN TrmInfo* pTrmInfo, IN IMS_UINT32 nType, IN IMS_UINT32 nMode)
{
    if (nMode == MODE_START)
    {
        return (pTrmInfo->IsStarted(nType));
    }
    else
    {
        return (!pTrmInfo->IsStarted(nType));
    }
}

PRIVATE
IMS_BOOL OsTrm::IsSameERequest(IN TrmInfo* pTrmInfo, IN IMS_UINT32 nType, IN IMS_UINT32 nMode)
{
    if (nMode == MODE_START)
    {
        return (pTrmInfo->IsEStarted(nType));
    }
    else
    {
        return (!pTrmInfo->IsEStarted(nType));
    }
}

PRIVATE
IMS_BOOL OsTrm::IsWlan(IN IMS_UINT32 nSlotId)
{
    LockGuard objLock(m_piLock);

    TrmInfo* pTrmInfo = GetTrmInfo(nSlotId);
    if (pTrmInfo == IMS_NULL)
    {
        return IMS_FALSE;
    }

    return (pTrmInfo->m_nIpcanCategory == IIpcan::CATEGORY_WLAN);
}

PRIVATE
IMS_BOOL OsTrm::IsWlanInOtherSlot(IN IMS_UINT32 nSlotId)
{
    for (IMS_UINT32 i = 0; i < m_objTrms.GetSize(); i++)
    {
        if (i == nSlotId)
        {
            continue;
        }

        TrmInfo* pTrmInfo = m_objTrms.GetValue(i);
        if (pTrmInfo == IMS_NULL)
        {
            continue;
        }

        if (pTrmInfo->m_nIpcanCategory == IIpcan::CATEGORY_WLAN)
        {
            return IMS_TRUE;
        }
    }

    return IMS_FALSE;
}

PRIVATE
IMS_BOOL OsTrm::ResetService(IN IMS_UINT32 nSlotId)
{
    TrmInfo* pTrmInfo = GetTrmInfo(nSlotId);
    if (pTrmInfo == IMS_NULL)
    {
        return IMS_FALSE;
    }

    if (pTrmInfo->m_nUpdatedService > SERVICE_NONE) {
        pTrmInfo->SetUpdatedService(SERVICE_NONE);
        SetTrmInfo(pTrmInfo, SERVICE_NONE, nSlotId);
        return IMS_TRUE;
    }

    return IMS_FALSE;
}

PRIVATE
void OsTrm::ResetServiceInOtherSlot(IN IMS_UINT32 nSlotId)
{
    for (IMS_UINT32 i = 0; i < m_objTrms.GetSize(); i++)
    {
        TrmInfo* pTrmInfo = m_objTrms.GetValue(i);
        if (pTrmInfo == IMS_NULL || i == nSlotId)
        {
            continue;
        }

        pTrmInfo->ClearServices();
        if (pTrmInfo->m_nUpdatedService > SERVICE_NONE)
        {
            pTrmInfo->SetUpdatedService(SERVICE_NONE);
            SetTrmInfo(pTrmInfo, SERVICE_NONE, i);
        }
    }
}

PRIVATE
void OsTrm::SetEmergencyService_Start(IN TrmInfo* pTrmInfo, IN IMS_UINT32 nType,
    IN IMS_UINT32 nSlotId)
{
    pTrmInfo->StartEmergency(nType);
    if (pTrmInfo->m_nUpdatedEmergencyService < nType)
    {
        IMS_TRACE_I("SetEmergencyService_START :: send TRM [%d][%s]", nSlotId,
            ServiceToString(nType), 0);
        pTrmInfo->SetUpdatedEmergencyService(nType);

        ResetServiceInOtherSlot(nSlotId);
        SetTrmInfo(pTrmInfo, nType, nSlotId, IMS_FALSE);

        PostMsgRegisteredThread();
    }
}

PRIVATE
void OsTrm::SetEmergencyService_Stop(IN TrmInfo* pTrmInfo, IN IMS_UINT32 nType,
    IN IMS_UINT32 nSlotId)
{
    pTrmInfo->StopEmergency(nType);
    if (pTrmInfo->m_nUpdatedEmergencyService == SERVICE_NONE)
    {
        return;
    }

    if (pTrmInfo->m_nUpdatedEmergencyService < nType)
    {
        IMS_TRACE_D("SetEmergencyService_STOP :: invalid call type [%d][%s]", nSlotId,
            ServiceToString(nType), 0);
        return;
    }

    if (pTrmInfo->m_nUpdatedEmergencyService == nType)
    {
        IMS_UINT32 nUpdateService = SERVICE_NONE;
        if (pTrmInfo->m_nEmergencyServices == SERVICE_NONE)
        {
            pTrmInfo->SetUpdatedEmergencyService(nUpdateService);

            IMS_UINT32 nTopSlot = GetTopPrioritySlot();
            if (nTopSlot == nSlotId)
            {
                if (pTrmInfo->m_nServices == SERVICE_NONE)
                {
                    SetTrmInfo(pTrmInfo, SERVICE_NONE, nSlotId);
                }
                else
                {
                    if (pTrmInfo->IsWlan())
                    {
                        SetTrmInfo(pTrmInfo, SERVICE_NONE, nSlotId);
                        IMS_TRACE_I("SetEmergencyService_STOP :: WLAN reset [%d][%s]",
                            nSlotId, ServiceToString(nUpdateService), 0);
                        PostMsgRegisteredThread();
                        return;
                    }
                    else
                    {
                        nUpdateService = GetHighPriorityService(pTrmInfo->m_nServices);
                        pTrmInfo->SetUpdatedService(nUpdateService);
                        SetTrmInfo(pTrmInfo, nUpdateService, nSlotId);
                    }
                }
            }
            else
            {
                TrmInfo* pTopTrmInfo = GetTrmInfo(nTopSlot);
                if (pTopTrmInfo == IMS_NULL)
                {
                    return;
                }

                if (pTopTrmInfo->IsWlan())
                {
                    if (pTrmInfo->m_nServices != SERVICE_NONE)
                    {
                        nUpdateService = GetHighPriorityService(pTrmInfo->m_nServices);
                        pTrmInfo->SetUpdatedService(nUpdateService);
                        SetTrmInfo(pTrmInfo, nUpdateService, nSlotId);
                    }
                    else
                    {
                        nUpdateService = SERVICE_NONE;
                        pTrmInfo->SetUpdatedService(nUpdateService);
                        SetTrmInfo(pTrmInfo, SERVICE_NONE, nSlotId);
                    }
                }
                else
                {
                    nUpdateService = SERVICE_NONE;
                    pTrmInfo->ClearServices();
                    pTrmInfo->SetUpdatedService(nUpdateService);
                    SetTrmInfo(pTrmInfo, SERVICE_NONE, nSlotId);

                    if (pTopTrmInfo->m_nServices != SERVICE_NONE)
                    {
                        nUpdateService = GetHighPriorityService(pTopTrmInfo->m_nServices);
                        pTopTrmInfo->SetUpdatedService(nUpdateService);
                        SetTrmInfo(pTopTrmInfo, nUpdateService, nTopSlot);
                    }
                }
            }
        }
        else
        {
            nUpdateService = GetHighPriorityService(pTrmInfo->m_nEmergencyServices);
            pTrmInfo->SetUpdatedEmergencyService(nUpdateService);
            SetTrmInfo(pTrmInfo, nUpdateService, nSlotId, IMS_FALSE);
        }

        IMS_TRACE_I("SetEmergencyService_STOP :: send TRM [%d][%s]", nSlotId,
            ServiceToString(nUpdateService), 0);

        PostMsgRegisteredThread();
    }

}

PRIVATE
void OsTrm::SetIpcan_Mobile(IN TrmInfo* pTrmInfo, IN IMS_UINT32 nSlotId)
{
    IMS_TRACE_I("SetIPCAN_MOBILE :: [%d]", nSlotId, 0, 0);

    IMS_UINT32 nTopSlot = GetTopPrioritySlot();
    IMS_UINT32 nUpdateService = SERVICE_NONE;
    IMS_BOOL bIsWlanInOtherSlot = IsWlanInOtherSlot(nSlotId);

    if (bIsWlanInOtherSlot || (nTopSlot == nSlotId))
    {
        if (pTrmInfo->m_nUpdatedService == SERVICE_NONE && pTrmInfo->m_nServices == SERVICE_NONE)
        {
            return;
        }

        if (!bIsWlanInOtherSlot)
        {
            ResetServiceInOtherSlot(nSlotId);
        }

        nUpdateService = GetHighPriorityService(pTrmInfo->m_nServices);
        pTrmInfo->SetUpdatedService(nUpdateService);
        SetTrmInfo(pTrmInfo, nUpdateService, nSlotId);
    }
    else
    {
        TrmInfo* pTopTrmInfo = GetTrmInfo(nTopSlot);
        if (pTopTrmInfo == IMS_NULL)
        {
            return;
        }

        ResetServiceInOtherSlot(nTopSlot);

        nUpdateService = GetHighPriorityService(pTopTrmInfo->m_nServices);
        if (pTopTrmInfo->m_nUpdatedService != nUpdateService)
        {
            pTopTrmInfo->SetUpdatedService(nUpdateService);
            SetTrmInfo(pTopTrmInfo, nUpdateService, nTopSlot);
        }
    }

    PostMsgRegisteredThread();
}

PRIVATE
void OsTrm::SetIpcan_Wlan(IN IMS_UINT32 nSlotId)
{
    IMS_TRACE_I("SetIPCAN_WLAN :: [%d]", nSlotId, 0, 0);

    if (ResetService(nSlotId))
    {
        IMS_TRACE_I("SetIPCAN_WLAN :: [%d] trm is reset in WLAN", nSlotId, 0, 0);
        PostMsgRegisteredThread();
    }
}

PRIVATE
void OsTrm::SetService_Start(IN TrmInfo* pTrmInfo, IN IMS_UINT32 nType, IN IMS_UINT32 nSlotId)
{
    pTrmInfo->Start(nType);
    if (pTrmInfo->m_nUpdatedService < nType)
    {
        IMS_TRACE_I("SetService_START :: send TRM [%d][%s]", nSlotId, ServiceToString(nType), 0);
        pTrmInfo->SetUpdatedService(nType);
        SetTrmInfo(pTrmInfo, nType, nSlotId);
        PostMsgRegisteredThread();
    }
}

PRIVATE
void OsTrm::SetService_Stop(IN TrmInfo* pTrmInfo, IN IMS_UINT32 nType, IN IMS_UINT32 nSlotId)
{
    pTrmInfo->Stop(nType);

    if (pTrmInfo->m_nUpdatedService == SERVICE_NONE)
    {
        IMS_TRACE_D("SetService_STOP :: no service is updated [%d][%s]", nSlotId,
            ServiceToString(nType), 0);
        return;
    }

    if (pTrmInfo->m_nUpdatedService < nType)
    {
        IMS_TRACE_D("SetService_STOP , invalid call type [%d][%s]", nSlotId,
            ServiceToString(nType), 0);
        return;
    }

    if (pTrmInfo->m_nUpdatedService == nType)
    {
        IMS_UINT32 nUpdateService = SERVICE_NONE;

        if (pTrmInfo->m_nServices == SERVICE_NONE)
        {
            pTrmInfo->SetUpdatedService(nUpdateService);
            SetTrmInfo(pTrmInfo, nUpdateService, nSlotId);
        }
        else
        {
            nUpdateService = GetHighPriorityService(pTrmInfo->m_nServices);

            if (IsWlanInOtherSlot(nSlotId))
            {
                pTrmInfo->SetUpdatedService(nUpdateService);
                SetTrmInfo(pTrmInfo, nUpdateService, nSlotId);
            }
            else
            {
                if (!UpdateHighPrioritySlotAndResetOtherSlot(nSlotId, nUpdateService))
                {
                    ResetServiceInOtherSlot(nSlotId);
                    pTrmInfo->SetUpdatedService(nUpdateService);
                    SetTrmInfo(pTrmInfo, nUpdateService, nSlotId);
                }
            }
        }

        PostMsgRegisteredThread();
    }
}

PRIVATE
void OsTrm::SetTrmInfo(IN TrmInfo* pTrmInfo, IN IMS_UINT32 nType, IN IMS_UINT32 nSlotId,
        IN IMS_BOOL bIsTimerRequired /*= IMS_TRUE*/)
{
    if (nType == SERVICE_NONE || nType == SERVICE_VOLTE)
    {
        pTrmInfo->StopAllTimers();
    }
    else
    {
        if (bIsTimerRequired)
        {
            pTrmInfo->StartTimerAndStopRestTimers(nType);
        }
    }

    IMS_TRACE_D("SetTRM :: send TRM [%d][trm type = %d][%s]", nSlotId, GetTrmType(nType),
        ServiceToString(nType));

    System::GetInstance()->SetTrm(GetTrmType(nType), nSlotId);
}

PRIVATE
IMS_BOOL OsTrm::UpdateHighPrioritySlotAndResetOtherSlot(IN IMS_UINT32 nSlotId,
    IN IMS_UINT32 nType)
{
    IMS_UINT32 nTopSlot = 0;
    TrmInfo* pTopSlotTrmInfo = IMS_NULL;

    for (IMS_UINT32 i = 0; i < m_objTrms.GetSize(); i++) {
        TrmInfo* pTrmInfo = m_objTrms.GetValue(i);
        if (pTrmInfo == IMS_NULL || i == nSlotId)
        {
            continue;
        }

        if (GetHighPriorityService(pTrmInfo->m_nServices) > nType)
        {
            nTopSlot = i;
            pTopSlotTrmInfo = pTrmInfo;
            break;
        }
    }

    if (pTopSlotTrmInfo != IMS_NULL)
    {
        ResetServiceInOtherSlot(nTopSlot);
        IMS_UINT32 nUpdateService = GetHighPriorityService(pTopSlotTrmInfo->m_nServices);
        pTopSlotTrmInfo->SetUpdatedService(nUpdateService);
        SetTrmInfo(pTopSlotTrmInfo, nUpdateService, nTopSlot);
        return IMS_TRUE;
    }

    return IMS_FALSE;
}

PRIVATE
void OsTrm::UpdateServiceStateChanged(IN IMS_UINT32 nServiceType, IN IMS_UINT32 nMode,
        IN IMS_UINT32 nSlotId)
{
    IMS_TRACE_D("UpdateServiceStateChanged :: type=%s, mode=%d, slot=%d",
        ServiceToString(nServiceType), nMode, nSlotId);

    TrmStateParam* pParam = new TrmStateParam();
    pParam->nSlotId = nSlotId;
    pParam->nServiceType = nServiceType;
    pParam->nMode = nMode;
    PostMsgEnablerThread(pParam);
}

PUBLIC GLOBAL
const IMS_CHAR* OsTrm::ServiceToString(IN IMS_SINT32 nType)
{
    switch (nType)
    {
        case SERVICE_UT:
            return "SERVICE_UT";

        case SERVICE_REG:
            return "SERVICE_REG";

        case SERVICE_SMS:
            return "SERVICE_SMS";

        case SERVICE_VOLTE:
            return "SERVICE_VOLTE";

        case SERVICE_NONE:
            return "SERVICE_NONE";

        default:
            return "__INVALID__";
    }
}
