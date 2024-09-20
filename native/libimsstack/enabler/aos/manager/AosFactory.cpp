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
#include "ServiceTrace.h"

#include "manager/ImsAosManager.h"
#include "manager/AosMngrAdaptor.h"
#include "manager/AosFactory.h"

__IMS_TRACE_TAG_AOS__;

PRIVATE GLOBAL AosFactory* AosFactory::m_gpAosFactory = IMS_NULL;

PRIVATE GLOBAL IMutex* AosFactory::m_gpiLock = IMS_NULL;

PRIVATE GLOBAL ImsMap<IMS_SINT32, ImsAosManager*> AosFactory::m_objManagers =
        ImsMap<IMS_SINT32, ImsAosManager*>();

PUBLIC
AosFactory::AosFactory()
{
    IMS_TRACE_D("AosFactory", 0, 0, 0);

    m_gpiLock = MutexService::GetMutexService()->CreateMutex();
}

PUBLIC VIRTUAL AosFactory::~AosFactory()
{
    MutexService::GetMutexService()->DestroyMutex(m_gpiLock);
}

PUBLIC GLOBAL ImsAosManager* AosFactory::GetManager(IN IMS_SINT32 nSlotId /* = IMS_SLOT_0 */)
{
    if (m_gpiLock == IMS_NULL)
    {
        IMS_TRACE_D("m_gpiLock is null", 0, 0, 0);
        return IMS_NULL;
    }

    LockGuard objLock(m_gpiLock);

    IMS_SLONG nIndex = m_objManagers.GetIndexOfKey(nSlotId);

    if (nIndex < 0)
    {
        return IMS_NULL;
    }

    return m_objManagers.GetValueAt(nIndex);
}

PUBLIC GLOBAL void AosFactory::Start(IN IMS_SINT32 nSlotId /* = IMS_SLOT_0 */)
{
    LockGuard objLock(m_gpiLock);

    if (m_gpAosFactory == IMS_NULL)
    {
        m_gpAosFactory = new AosFactory();
    }

    IMS_TRACE_D("Start :: slot(%d)", nSlotId, 0, 0);

    if (GetManager(nSlotId) != IMS_NULL)
    {
        return;
    }

    AString strName;
    strName.Sprintf("ImsAosManager%d", nSlotId);

    ImsAosManager* pAoSMngr = new AosMngrAdaptor(strName, nSlotId);
    m_objManagers.Add(nSlotId, pAoSMngr);
}

PUBLIC GLOBAL void AosFactory::Stop(IN IMS_SINT32 nSlotId /* = IMS_SLOT_0 */)
{
    LockGuard objLock(m_gpiLock);

    if (m_gpAosFactory == IMS_NULL)
    {
        m_gpAosFactory = new AosFactory();
    }

    IMS_TRACE_D("Stop :: slot(%d)", nSlotId, 0, 0);

    ImsAosManager* pAoSMngr = GetManager(nSlotId);
    if (pAoSMngr != IMS_NULL)
    {
        delete pAoSMngr;
        m_objManagers.Remove(nSlotId);
    }
}
