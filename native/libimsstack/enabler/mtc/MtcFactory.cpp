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

#include "IMtcApp.h"
#include "MtcApp.h"
#include "MtcFactory.h"
#include "IMutex.h"
#include "ServiceMutex.h"
#include "ServiceTrace.h"

__IMS_TRACE_TAG_COM_MTC__;

PUBLIC
MtcFactory::MtcFactory() :
        m_objMtcApps(ImsMap<IMS_UINT32, IMtcApp*>())
{
    IMS_TRACE_I("+MtcFactory", 0, 0, 0);
    m_piLock = MutexService::GetMutexService()->CreateMutex();
}

PUBLIC VIRTUAL MtcFactory::~MtcFactory()
{
    IMS_TRACE_I("~MtcFactory", 0, 0, 0);

    MutexService::GetMutexService()->DestroyMutex(m_piLock);

    for (IMS_UINT32 index = 0; index < m_objMtcApps.GetSize(); index++)
    {
        IMtcApp* pApp = m_objMtcApps.GetValueAt(index);
        delete pApp;
    }
    m_objMtcApps.Clear();
}

PUBLIC GLOBAL MtcFactory* MtcFactory::GetInstance()
{
    static MtcFactory* pFactory = IMS_NULL;

    if (pFactory == IMS_NULL)
    {
        pFactory = new MtcFactory();
    }

    return pFactory;
}

PUBLIC
void MtcFactory::Start(IN IMS_SINT32 nSlotId)
{
    IMS_TRACE_D("Start[slot_%d]", nSlotId, 0, 0);

    LockGuard objLock(m_piLock);

    IMS_SLONG nIndex = m_objMtcApps.GetIndexOfKey(nSlotId);
    if (nIndex >= 0)
    {
        IMS_TRACE_E(0, "Start : an App in the slot is already running", 0, 0, 0);
        return;
    }

    IMtcApp* piMtcApp = new MtcApp(nSlotId);
    m_objMtcApps.Add(nSlotId, piMtcApp);
    piMtcApp->Start();
}

PUBLIC
void MtcFactory::Stop(IN IMS_SINT32 nSlotId)
{
    IMS_TRACE_D("Stop[slot_%d]", nSlotId, 0, 0);

    LockGuard objLock(m_piLock);

    IMS_SLONG nIndex = m_objMtcApps.GetIndexOfKey(nSlotId);
    if (nIndex < 0)
    {
        IMS_TRACE_E(0, "Stop : no App in the slot", 0, 0, 0);
        return;
    }

    IMtcApp* piMtcApp = m_objMtcApps.GetValueAt(nIndex);
    piMtcApp->Stop();
    delete piMtcApp;
    m_objMtcApps.RemoveAt(nIndex);
}
