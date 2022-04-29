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
#include "IMSProcess.h"
#include "PlatformProperty.h"
#include "ServiceTrace.h"
#include "SystemConfigManager.h"

#include "EnablerLoader.h"
#include "EngineState.h"
#include "ImsMain.h"
#include "ImsProvisioning.h"

__IMS_TRACE_TAG_USER_DECL__("ImsMain")

PUBLIC GLOBAL
void ImsMain::SetConfiguration(IN IMS_SINT32 nEvent,
        IN IMS_SINT32 nCount, IN const __SystemConfig* pSysConfig)
{
    SystemConfigManager* pScm = SystemConfigManager::GetInstance();
    pScm->UpdateSystemConfig(nEvent, nCount, pSysConfig);

    if ((nEvent == SystemConfig::EVENT_FEATURE_CHANGED)
            || (nEvent == SystemConfig::EVENT_ALL_CONFIGURATION_CHANGED))
    {
        // no-op
    }
}

PUBLIC GLOBAL
void ImsMain::SetDeviceConfig(IN const __DeviceConfig& objConfig)
{
    DeviceConfig::SetConfig(objConfig);
}

PUBLIC GLOBAL
void ImsMain::Initialize()
{
    PlatformProperty::Initialize();
}

PUBLIC GLOBAL
void ImsMain::Uninitialize()
{
    PlatformProperty::Uninitialize();
}

PUBLIC GLOBAL
void ImsMain::Start()
{
    ImsProvisioning::Initialize();

    // Starts the platform-specific threads
    PlatformProperty::Start();

    IMSProcess* pProcess = IMSProcess::GetInstance();

    // Starts the main worker thread
    if (!pProcess->Initialize())
    {
        return;
    }

    EngineState::Initialize();
    EnablerLoader::GetInstance()->Init();

    IMS_TRACE_I("ImsMain: start - %s", DeviceConfig::ToString().GetStr(), 0, 0);
}

PUBLIC GLOBAL
void ImsMain::Stop()
{
    IMS_TRACE_I("ImsMain: stop", 0, 0, 0);

    EngineState::Uninitialize();

    IMSProcess::GetInstance()->Uninitialize();

    PlatformProperty::Stop();
}
