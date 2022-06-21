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
#include "SystemConfigManager.h"

#include "EnablerFactory.h"
#include "EnablerLoader.h"
#include "EnablerThread.h"
#include "EnablerUtils.h"
#include "GeolocationHelper.h"

__IMS_TRACE_TAG_USER_DECL__("EnablerLoader");

PRIVATE GLOBAL EnablerLoader* EnablerLoader::s_pEnablerLoader = IMS_NULL;

PRIVATE
EnablerLoader::EnablerLoader() :
        m_pEnablerFactory(IMS_NULL),
        m_objEnablerThreads(IMSMap<IMS_SINT32, EnablerThread*>())
{
    m_pEnablerFactory = new EnablerFactory();

    SystemConfigManager::GetInstance()->AddListener(this);
}

PRIVATE
EnablerLoader::~EnablerLoader()
{
    SystemConfigManager::GetInstance()->RemoveListener(this);

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

    for (IMS_SINT32 i = IMS_SLOT_1; i < SystemConfig::GetMaxSimSlot(); ++i)
    {
        CreateAndAddThread(i);
    }
}

PUBLIC GLOBAL
void EnablerLoader::CreateInstance()
{
    if (s_pEnablerLoader == IMS_NULL)
    {
        s_pEnablerLoader = new EnablerLoader();
    }
}

PUBLIC GLOBAL
void EnablerLoader::DestroyInstance()
{
    if (s_pEnablerLoader != IMS_NULL)
    {
        delete s_pEnablerLoader;
        s_pEnablerLoader = IMS_NULL;
    }
}

PUBLIC GLOBAL
EnablerLoader* EnablerLoader::GetInstance()
{
    if (s_pEnablerLoader == IMS_NULL)
    {
        CreateInstance();
    }

    return s_pEnablerLoader;
}

PROTECTED VIRTUAL void EnablerLoader::SystemConfig_ConfigurationChanged(
        IN IMS_SINT32 nEvent, IN IMS_SINT32 nSlotId /*= IMS_SLOT_ANY*/)
{
    IMS_TRACE_I("SystemConfig :: event=%d, slotId=%d", nEvent, nSlotId, 0);

    if (nEvent == SystemConfig::EVENT_ON_BOOT)
    {
        // Ignore it on boot-up
    }
    else
    {
        ControlEnablers(nSlotId);
    }
}

PRIVATE
void EnablerLoader::CreateAndAddThread(IN IMS_SINT32 nSlotId)
{
    AString strThreadName = EnablerUtils::GetEnablerThreadName(nSlotId);
    EnablerThreadParam objParam(m_pEnablerFactory, nSlotId);
    ImsProcess* pProcess = ImsProcess::GetInstance();

    pProcess->LoadAppThreadWithParam(strThreadName,
            EnablerLoader::CreateThread, reinterpret_cast<void*>(&objParam), nSlotId);

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

PRIVATE
void EnablerLoader::ControlEnablers(IN IMS_SINT32 nSlotId)
{
    SystemConfigManager* pScm = SystemConfigManager::GetInstance();
    const SystemConfig* pOldConfig = pScm->GetOldConfig(nSlotId);
    const SystemConfig* pNewConfig = pScm->GetConfig(nSlotId);
    IMS_SINT32 nCtrlFlags = EnablerThread::CONTROL_NONE;

    if (SystemConfig::IsOperatorChanged(pOldConfig, pNewConfig))
    {
        nCtrlFlags = EnablerThread::CONTROL_ALL;

        if (pNewConfig != IMS_NULL)
        {
            if (pNewConfig->GetOperator().GetLength() == 0)
            {
                nCtrlFlags = EnablerThread::CONTROL_STOP | EnablerThread::CONTROL_DESTROY;
            }
            else if (SystemConfig::IsMultiSimEnabled() && !pNewConfig->IsDds())
            {
                nCtrlFlags = EnablerThread::CONTROL_STOP | EnablerThread::CONTROL_DESTROY;
            }
            else if (pNewConfig->GetServiceFeatures() == 0)
            {
                nCtrlFlags = EnablerThread::CONTROL_STOP | EnablerThread::CONTROL_DESTROY;
            }
        }
    }
    else if (SystemConfig::IsDdsChanged(pOldConfig, pNewConfig))
    {
        if (pNewConfig != IMS_NULL)
        {
            if (pNewConfig->GetOperator().GetLength() == 0)
            {
                // Operator: old/new empty - do not control enabler threads
            }
            else if (pNewConfig->IsDds())
            {
                if (pNewConfig->GetServiceFeatures() == 0)
                {
                    // No services - do not start enabler thread
                }
                else
                {
                    nCtrlFlags = EnablerThread::CONTROL_ALL;
                }
            }
            else
            {
                nCtrlFlags = EnablerThread::CONTROL_STOP | EnablerThread::CONTROL_DESTROY;
            }
        }
    }
    else if (SystemConfig::IsServiceFeatureChanged(pOldConfig, pNewConfig))
    {
        nCtrlFlags = EnablerThread::CONTROL_ALL;

        // If service features are changed and it's non-DDS slot, then stop the enablers.
        // SIM1: empty, SIM2: SIM_LOCKED (newly added) : SIM2 is non-DDS when it's in LOADED
        if (SystemConfig::IsMultiSimEnabled() && !SystemConfig::IsMultiImsEnabled() &&
                !SystemConfig::IsMultiImsEnabledOnDssv())
        {
            if ((pNewConfig != IMS_NULL) && !pNewConfig->IsDds())
            {
                nCtrlFlags = EnablerThread::CONTROL_STOP | EnablerThread::CONTROL_DESTROY;
            }
        }
    }
    else if (SystemConfig::IsSimMobilityChanged(pOldConfig, pNewConfig))
    {
        nCtrlFlags = EnablerThread::CONTROL_ALL;
    }
    /** Don't stop enablers. It's stopped and started when SIM is inserted.
    else
    {
        bOldNoUicc = (pOldConfig != IMS_NULL) ? pOldConfig->IsNoUicc() : IMS_FALSE;
        bNewNoUicc = (pNewConfig != IMS_NULL) ? pNewConfig->IsNoUicc() : IMS_TRUE;

        if ((!bOldNoUicc && bNewNoUicc) || (bOldNoUicc && !bNewNoUicc))
        {
            nCtrlFlags = EnablerThread::CONTROL_STOP | EnablerThread::CONTROL_START;
        }
    }
    */
    IMS_BOOL bOldNoUicc = (pOldConfig != IMS_NULL) ? pOldConfig->IsNoUicc() : IMS_FALSE;
    IMS_BOOL bNewNoUicc = (pNewConfig != IMS_NULL) ? pNewConfig->IsNoUicc() : IMS_TRUE;

    IMS_TRACE_I("ControlEnablers :: flags=%08X, oldNoUicc=%s, newNoUicc=%s",
            nCtrlFlags, _TRACE_B_(bOldNoUicc), _TRACE_B_(bNewNoUicc));

    if (nCtrlFlags != EnablerThread::CONTROL_NONE)
    {
        EnablerThread* pThread = GetEnablerThread(nSlotId);

        if (pThread != IMS_NULL)
        {
            pThread->ControlEnablers(nCtrlFlags);
        }
    }
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
