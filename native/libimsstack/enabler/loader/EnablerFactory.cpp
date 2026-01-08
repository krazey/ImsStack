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
#include "ServiceMemory.h"
#include "ServiceMutex.h"
#include "SystemConfig.h"

#include "ConfigEnabler.h"
#include "EnablerFactory.h"
#include "service/AosEnabler.h"
#include "service/MtcEnabler.h"
#include "service/MtsEnabler.h"
#include "service/UceEnabler.h"

PUBLIC
EnablerFactory::EnablerFactory() :
        m_piLock(IMS_NULL)
{
    IMS_SINT32 nSimCount = SystemConfig::GetSupportedSimCount();

    for (IMS_SINT32 i = 0; i < nSimCount; ++i)
    {
        m_objImsEnablers.Add(i, new ImsList<IEnabler*>());
    }

    if (nSimCount > 1)
    {
        m_piLock = MutexService::GetMutexService()->CreateMutex();
    }
}

PUBLIC
EnablerFactory::~EnablerFactory()
{
    {
        LockGuard objLock(m_piLock);

        IMS_SINT32 nSimCount = SystemConfig::GetSupportedSimCount();

        for (IMS_SINT32 i = 0; i < nSimCount; ++i)
        {
            DestroyEnablers(i);

            ImsList<IEnabler*>* pEnablers = m_objImsEnablers.GetValueAt(i);

            if (pEnablers != IMS_NULL)
            {
                delete pEnablers;
            }
        }

        m_objImsEnablers.Clear();
    }

    MutexService::GetMutexService()->DestroyMutex(m_piLock);
}

PUBLIC
void EnablerFactory::CreateEnablers(IN IMS_SINT32 nSlotId)
{
    LockGuard objLock(m_piLock);

    IMS_SLONG nIndex = m_objImsEnablers.GetIndexOfKey(nSlotId);

    if (nIndex < 0)
    {
        return;
    }

    ImsList<IEnabler*>* pEnablers = m_objImsEnablers.GetValueAt(nIndex);

    if ((pEnablers != IMS_NULL) && pEnablers->IsEmpty())
    {
        CreateEnablers(nSlotId, pEnablers);
    }
}

PUBLIC
void EnablerFactory::DestroyEnablers(IN IMS_SINT32 nSlotId)
{
    LockGuard objLock(m_piLock);

    IMS_SLONG nIndex = m_objImsEnablers.GetIndexOfKey(nSlotId);

    if (nIndex < 0)
    {
        return;
    }

    ImsList<IEnabler*>* pEnablers = m_objImsEnablers.GetValueAt(nIndex);

    if ((pEnablers != IMS_NULL) && !pEnablers->IsEmpty())
    {
        IMS_SINT32 i = static_cast<IMS_SINT32>(pEnablers->GetSize() - 1);

        for (; i >= 0; i--)
        {
            Enabler* pEnabler = DYNAMIC_CAST(Enabler*, pEnablers->GetAt(i));

            if (pEnabler != IMS_NULL)
            {
                pEnabler->Destroy();
            }
        }

        pEnablers->Clear();
    }
}

PUBLIC
const ImsList<IEnabler*>* EnablerFactory::GetEnablers(IN IMS_SINT32 nSlotId) const
{
    LockGuard objLock(m_piLock);

    IMS_SLONG nIndex = m_objImsEnablers.GetIndexOfKey(nSlotId);

    if (nIndex < 0)
    {
        return IMS_NULL;
    }

    return m_objImsEnablers.GetValueAt(nIndex);
}

PUBLIC
IMS_BOOL EnablerFactory::HasEnablers(IN IMS_SINT32 nSlotId) const
{
    LockGuard objLock(m_piLock);

    IMS_SLONG nIndex = m_objImsEnablers.GetIndexOfKey(nSlotId);

    if (nIndex < 0)
    {
        return IMS_FALSE;
    }

    const ImsList<IEnabler*>* pEnablers = m_objImsEnablers.GetValueAt(nIndex);
    return pEnablers != IMS_NULL && !pEnablers->IsEmpty();
}

PRIVATE
void EnablerFactory::CreateEnablers(IN IMS_SINT32 nSlotId, OUT ImsList<IEnabler*>*& pEnablers)
{
    pEnablers->Append(new ConfigEnabler(nSlotId));
    pEnablers->Append(new AosEnabler(nSlotId));
    pEnablers->Append(new MtcEnabler(nSlotId));
    pEnablers->Append(new MtsEnabler(nSlotId));
    pEnablers->Append(new UceEnabler(nSlotId));
}
