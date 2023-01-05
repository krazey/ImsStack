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
#include "IUIMS.h"
#include "ImsAosParameter.h"
#include "provider/AosRegStateManager.h"

PUBLIC
AosRegStateManager::AosRegStateManager() :
        m_nSlotId(IMS_SLOT_0),
        m_nRegState(IMS_REG_OFF),
        m_nERegState(IMS_REG_OFF),
        m_nRegServices(ImsAosService::NONE)
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

PUBLIC VIRTUAL void AosRegStateManager::SetImsRegState(
        IN IMS_UINT32 nState, IN IMS_BOOL /*bLimited*/)
{
    m_nRegState = nState;
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

PROTECTED
IMS_SINT32 AosRegStateManager::ConvertServiceType(IMS_UINT32 nServiceType)
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
IMS_BOOL AosRegStateManager::IsRegService(IN IMS_UINT32 nType) const
{
    return (m_nRegServices & nType);
}
