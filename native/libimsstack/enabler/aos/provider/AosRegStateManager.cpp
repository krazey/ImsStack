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
#include "ServiceTrace.h"
#include "ServiceEvent.h"
#include "IUIMS.h"
#include "ImsEventDef.h"
#include "SipStatusCode.h"
#include "ImsAosParameter.h"
#include "provider/AosRegStateManager.h"

__IMS_TRACE_TAG_USER_DECL__("AOS");

#define AOSTAG m_strTag.GetStr()

PUBLIC
AosRegStateManager::AosRegStateManager() :
        m_nSlotId(IMS_SLOT_0),
        m_nRegState(IMS_REG_OFF),
        m_nERegState(IMS_REG_OFF),
        m_nRegServices(ImsAosService::NONE),
        m_nReportedRegServices(IMS_REGISTRATION_SERVICE_NONE),
        m_nRegDetailState(IMS_REGISTRATION_INVALID),
        m_nReportedRegDetailState(IMS_REGISTRATION_INVALID),
        m_nRegReason(0),
        m_nRegRespCode(SipStatusCode::SC_INVALID),
        m_bLimitedMode(IMS_FALSE)
{
    IMS_TRACE_MEM("AOS_MEM", "AOS_M : AosRegStateManager = %" PFLS_u "/%" PFLS_x,
            sizeof(AosRegStateManager), this, 0);
}

PUBLIC VIRTUAL AosRegStateManager::~AosRegStateManager()
{
    IMS_TRACE_MEM("AOS_MEM", "AOS_F : AosRegStateManager = %" PFLS_u "/%" PFLS_x,
            sizeof(AosRegStateManager), this, 0);
}

PUBLIC VIRTUAL IMS_SINT32 AosRegStateManager::GetSlotId() const
{
    return m_nSlotId;
}

PUBLIC VIRTUAL void AosRegStateManager::SetSlotId(IN IMS_SINT32 nSlotId)
{
    m_nSlotId = nSlotId;

    m_strTag.Sprintf("%d", m_nSlotId);
}

PUBLIC VIRTUAL void AosRegStateManager::SetImsRegState(IN IMS_UINT32 nState, IN IMS_BOOL bLimited)
{
    if (m_nRegState != nState)
    {
        m_nRegState = nState;
    }

    if (nState == IMS_REG_ON)
    {
        m_bLimitedMode = bLimited;
    }
}

PUBLIC VIRTUAL IMS_SINT32 AosRegStateManager::GetImsRegState()
{
    return m_nRegState;
}

PUBLIC VIRTUAL void AosRegStateManager::SetEImsRegState(IN IMS_UINT32 nState)
{
    m_nERegState = nState;
}

PUBLIC VIRTUAL void AosRegStateManager::SetRegState(
        IN IMS_UINT32 nServiceType, IN IMS_UINT32 nState)
{
    if (ConvertServiceType(nServiceType) == IUIMS::APP_UNKNOWN)
    {
        return;
    }

    if (nState == IMS_REG_ON)
    {
        if (IsRegService(nServiceType))
        {
            return;
        }

        AddRegService(nServiceType);
    }
    else
    {
        if (!IsRegService(nServiceType))
        {
            return;
        }

        RemoveRegService(nServiceType);
    }
}

PUBLIC VIRTUAL IMS_SINT32 AosRegStateManager::ConvertServiceType(IMS_UINT32 nServiceType)
{
    switch (nServiceType)
    {
        case ImsAosService::MTS:
            return IUIMS::APP_MTS;

        case ImsAosService::MTC:
            return IUIMS::APP_MTC;

        default:
            return IUIMS::APP_UNKNOWN;
    }
}

PUBLIC VIRTUAL void AosRegStateManager::SetDetailState(IN IMS_SINT32 nState)
{
    m_nRegDetailState = nState;
}

PUBLIC VIRTUAL IMS_SINT32 AosRegStateManager::GetDetailState()
{
    return m_nRegDetailState;
}

PUBLIC VIRTUAL void AosRegStateManager::SetReason(IN IMS_UINT32 nReason)
{
    m_nRegReason = nReason;
}

PUBLIC VIRTUAL void AosRegStateManager::EnforceUpdateRegistration()
{
    // RIL_INTEGRATION
    m_nReportedRegServices = GetConvertedRegServices();
    m_nReportedRegDetailState = (m_nRegDetailState == IMS_REGISTRATION_STOP)
            ? IMS_REGISTRATION_OFFLINE
            : m_nRegDetailState;

    IMS_EVENT_SendEventForSlotId(IMS_EVENT_REGISTRATION,
            IMS_MAKEPARAM(
                    ((IsRegistered(m_nReportedRegDetailState)) ? 1 : 0), m_nReportedRegServices),
            IMS_MAKEPARAM(m_nRegReason, m_nReportedRegDetailState), GetSlotId());

    m_nRegReason = 0;
}

PUBLIC VIRTUAL void AosRegStateManager::UpdateRegistration()
{
    // RIL_INTEGRATION
    IMS_UINT32 nService = GetConvertedRegServices();
    IMS_SINT32 nUpdatedRegDetailState = m_nRegDetailState;
    IMS_BOOL bIsReportRequired = IMS_FALSE;

    if (nService != m_nReportedRegServices)
    {
        bIsReportRequired = IMS_TRUE;
        m_nReportedRegServices = nService;
    }

    if (nUpdatedRegDetailState == IMS_REGISTRATION_STOP)
    {
        nUpdatedRegDetailState = IMS_REGISTRATION_OFFLINE;
    }

    if (nUpdatedRegDetailState != m_nReportedRegDetailState)
    {
        bIsReportRequired = IMS_TRUE;
        m_nReportedRegDetailState = nUpdatedRegDetailState;
    }

    if (bIsReportRequired)
    {
        IMS_UINT32 nReason = m_nRegReason;
        m_nRegReason = 0;
        if (nReason == IMS_REGISTRATION_REASON_BLOCK_NOTIFICATION)
        {
            return;
        }

        IMS_EVENT_SendEventForSlotId(IMS_EVENT_REGISTRATION,
                IMS_MAKEPARAM(((IsRegistered(m_nReportedRegDetailState)) ? 1 : 0),
                        m_nReportedRegServices),
                IMS_MAKEPARAM(nReason, m_nReportedRegDetailState), GetSlotId());
    }
}

PUBLIC VIRTUAL void AosRegStateManager::ClearRegServices()
{
    m_nRegServices = ImsAosService::NONE;
}

PUBLIC VIRTUAL IMS_UINT32 AosRegStateManager::GetRegServices() const
{
    return m_nRegServices;
}

PUBLIC VIRTUAL void AosRegStateManager::UpdateRegServices(
        IN IMS_BOOL bUpdateCurrState /* = IMS_FALSE */)
{
    if (bUpdateCurrState)
    {
        EnforceUpdateRegistration();
    }
    else
    {
        UpdateRegistration();
    }
}

PUBLIC VIRTUAL void AosRegStateManager::SetRegRespCode(IN IMS_SINT32 nRespCode)
{
    m_nRegRespCode = nRespCode;
}

PUBLIC VIRTUAL IMS_BOOL AosRegStateManager::IsLimitedMode() const
{
    return m_bLimitedMode;
}

PROTECTED
void AosRegStateManager::AddRegService(IN IMS_UINT32 nType)
{
    m_nRegServices |= nType;
}

PROTECTED
void AosRegStateManager::RemoveRegService(IN IMS_UINT32 nType)
{
    m_nRegServices &= ~(nType);
}

PROTECTED
IMS_BOOL AosRegStateManager::IsRegService(IN IMS_UINT32 nType)
{
    return (m_nRegServices & nType);
}

PROTECTED
IMS_UINT32 AosRegStateManager::GetConvertedRegServices()
{
    IMS_UINT32 nServices = IMS_REGISTRATION_SERVICE_NONE;

    if (IsRegService(ImsAosService::MTC))
    {
        nServices |= ImsAosService::MTC;
    }

    if (IsRegService(ImsAosService::MTS))
    {
        nServices |= ImsAosService::MTS;
    }

    A_IMS_TRACE_I(AOSTAG, "GetConvertedRegServices:: services (%d)", nServices, 0, 0);

    return nServices;
}

PROTECTED
IMS_BOOL AosRegStateManager::IsRegistered(IN IMS_UINT32 nDetailState) const
{
    if (nDetailState == IMS_REGISTRATION_REGISTERED ||
            nDetailState == IMS_REGISTRATION_REREGISTERING)
    {
        return IMS_TRUE;
    }

    return IMS_FALSE;
}
