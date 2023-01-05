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

#include "IMtsApp.h"
#include "MtsApp.h"
#include "MtsFactory.h"
#include "ServiceMutex.h"
#include "ServiceTrace.h"

__IMS_TRACE_TAG_COM_MTS__;

PUBLIC
MtsFactory::MtsFactory() :
        m_objMtsApps(ImsMap<IMS_SINT32, IMtsApp*>())
{
    IMS_TRACE_D("+MtsFactory", 0, 0, 0);
    m_piLock = MutexService::GetMutexService()->CreateMutex();
}

PUBLIC
MtsFactory::~MtsFactory()
{
    IMS_TRACE_D("~MtsFactory", 0, 0, 0);

    MutexService::GetMutexService()->DestroyMutex(m_piLock);

    for (IMS_UINT32 index = 0; index < m_objMtsApps.GetSize(); index++)
    {
        IMtsApp* pApp = m_objMtsApps.GetValueAt(index);
        delete pApp;
    }
    m_objMtsApps.Clear();
}

PUBLIC GLOBAL MtsFactory* MtsFactory::GetInstance()
{
    static MtsFactory* s_pFactory = IMS_NULL;

    if (s_pFactory == IMS_NULL)
    {
        s_pFactory = new MtsFactory();
    }

    return s_pFactory;
}

PUBLIC
void MtsFactory::Start(IN IMS_SINT32 nSlotId)
{
    IMS_TRACE_D("Start[slot_%d]", nSlotId, 0, 0);

    LockGuard objLock(m_piLock);

    IMS_SLONG nIndex = m_objMtsApps.GetIndexOfKey(nSlotId);
    if (nIndex >= 0)
    {
        IMS_TRACE_E(0, "Start : an App in the slot is already running", 0, 0, 0);
        return;
    }

    IMtsApp* piMtsApp = new MtsApp(nSlotId);
    m_objMtsApps.Add(nSlotId, piMtsApp);
    piMtsApp->Start();
}

PUBLIC
void MtsFactory::Stop(IN IMS_SINT32 nSlotId)
{
    IMS_TRACE_D("Stop[slot_%d]", nSlotId, 0, 0);

    LockGuard objLock(m_piLock);

    IMS_SLONG nIndex = m_objMtsApps.GetIndexOfKey(nSlotId);
    if (nIndex < 0)
    {
        IMS_TRACE_E(0, "Stop : no App in the slot", 0, 0, 0);
        return;
    }

    IMtsApp* piMtsApp = m_objMtsApps.GetValueAt(nIndex);
    piMtsApp->Stop();
    delete piMtsApp;
    m_objMtsApps.RemoveAt(nIndex);
}
