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
#include "ImsProcess.h"
#include "ServiceMemory.h"
#include "ServiceTrace.h"
#include "SystemConfig.h"

#include "EnablerFactory.h"
#include "EnablerLoader.h"
#include "EnablerThread.h"
#include "EnablerUtils.h"
#include "GeolocationHelper.h"

__IMS_TRACE_TAG_BASE__;

PRIVATE GLOBAL EnablerLoader* EnablerLoader::s_pEnablerLoader = IMS_NULL;

PRIVATE
EnablerLoader::EnablerLoader() :
        m_pEnablerFactory(IMS_NULL),
        m_objEnablerThreads(ImsMap<IMS_SINT32, EnablerThread*>())
{
    m_pEnablerFactory = new EnablerFactory();
}

PRIVATE
EnablerLoader::~EnablerLoader()
{
    if (m_pEnablerFactory != IMS_NULL)
    {
        delete m_pEnablerFactory;
        m_pEnablerFactory = IMS_NULL;
    }
}

PUBLIC
void EnablerLoader::Init()
{
    GeolocationHelper::GetInstance();

    // As default, the enabler thread for slot0 is always created.
    CreateAndAddThread(IMS_SLOT_0);

    for (IMS_SINT32 i = IMS_SLOT_1; i < SystemConfig::GetSupportedSimCount(); ++i)
    {
        CreateAndAddThread(i);
    }
}

PUBLIC VIRTUAL void EnablerLoader::StartEnabler(IN IMS_SINT32 nSlotId)
{
    IMS_TRACE_I("StartEnabler: slot%d", nSlotId, 0, 0);
    const IMS_SINT32 nCtrlFlags = EnablerThread::CONTROL_CREATE | EnablerThread::CONTROL_START;
    EnablerThread* pThread = GetEnablerThread(nSlotId);

    if (pThread != IMS_NULL)
    {
        pThread->ControlEnablers(nCtrlFlags);
    }
    else
    {
        IMS_TRACE_I("EnablerThread(%d) not found", nSlotId, 0, 0);
    }
}

PUBLIC VIRTUAL void EnablerLoader::StopEnabler(IN IMS_SINT32 nSlotId)
{
    IMS_TRACE_I("StopEnabler: slot%d", nSlotId, 0, 0);
    const IMS_SINT32 nCtrlFlags = EnablerThread::CONTROL_STOP | EnablerThread::CONTROL_DESTROY;
    EnablerThread* pThread = GetEnablerThread(nSlotId);

    if (pThread != IMS_NULL)
    {
        pThread->ControlEnablers(nCtrlFlags);
    }
    else
    {
        IMS_TRACE_I("EnablerThread(%d) not found", nSlotId, 0, 0);
    }
}

PUBLIC GLOBAL void EnablerLoader::CreateInstance()
{
    if (s_pEnablerLoader == IMS_NULL)
    {
        s_pEnablerLoader = new EnablerLoader();
    }
}

PUBLIC GLOBAL void EnablerLoader::DestroyInstance()
{
    if (s_pEnablerLoader != IMS_NULL)
    {
        delete s_pEnablerLoader;
        s_pEnablerLoader = IMS_NULL;
    }
}

PUBLIC GLOBAL EnablerLoader* EnablerLoader::GetInstance()
{
    if (s_pEnablerLoader == IMS_NULL)
    {
        CreateInstance();
    }

    return s_pEnablerLoader;
}

PRIVATE
void EnablerLoader::CreateAndAddThread(IN IMS_SINT32 nSlotId)
{
    AString strThreadName = EnablerUtils::GetEnablerThreadName(nSlotId);
    EnablerThreadParam objParam(m_pEnablerFactory, nSlotId);
    ImsProcess* pProcess = ImsProcess::GetInstance();

    pProcess->LoadAppThreadWithParam(strThreadName, EnablerLoader::CreateThread,
            reinterpret_cast<void*>(&objParam), nSlotId);

    EnablerThread* pThread =
            DYNAMIC_CAST(EnablerThread*, pProcess->GetApplicationThread(strThreadName));

    if (pThread != IMS_NULL)
    {
        IMS_TRACE_I("EnablerThread created - %s", strThreadName.GetStr(), 0, 0);

        m_objEnablerThreads.Add(nSlotId, pThread);
    }
}

PRIVATE
EnablerThread* EnablerLoader::GetEnablerThread(IN IMS_SINT32 nSlotId) const
{
    IMS_SLONG nIndex = m_objEnablerThreads.GetIndexOfKey(nSlotId);

    if (nIndex < 0)
    {
        return IMS_NULL;
    }

    return m_objEnablerThreads.GetValueAt(nIndex);
}

PRIVATE GLOBAL ImsAppThread* EnablerLoader::CreateThread(IN void* pvParam)
{
    EnablerThreadParam* pParam = reinterpret_cast<EnablerThreadParam*>(pvParam);

    if (pParam == IMS_NULL)
    {
        return IMS_NULL;
    }

    return new EnablerThread(pParam->m_pEnablerFactory, pParam->m_nSlotId);
}
