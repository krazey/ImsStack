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
#include "PlatformProperty.h"
#include "ServiceTrace.h"
#include "SystemConfigManager.h"

#include "private/ConfigurationManager.h"

#include "EnablerLoader.h"
#include "EngineState.h"
#include "ImsMain.h"

__IMS_TRACE_TAG_USER_DECL__("ImsMain")

PUBLIC GLOBAL void ImsMain::SetConfiguration(
        IN IMS_SINT32 nEvent, IN IMS_SINT32 nCount, IN const __SystemConfig* pSysConfig)
{
    SystemConfigManager* pScm = SystemConfigManager::GetInstance();
    pScm->UpdateSystemConfig(nEvent, nCount, pSysConfig);

    if ((nEvent == SystemConfig::EVENT_FEATURE_CHANGED) ||
            (nEvent == SystemConfig::EVENT_ALL_CONFIGURATION_CHANGED))
    {
        // no-op
    }
}

PUBLIC GLOBAL void ImsMain::SetDeviceConfig(IN const __DeviceConfig& objConfig)
{
    DeviceConfig::SetConfig(objConfig);
}

PUBLIC GLOBAL void ImsMain::Initialize()
{
    PlatformProperty::Initialize();
}

PUBLIC GLOBAL void ImsMain::Uninitialize()
{
    PlatformProperty::Uninitialize();
}

PUBLIC GLOBAL void ImsMain::Start()
{
    InitializeConfigurationManager();

    // Starts the platform-specific threads
    PlatformProperty::Start();

    ImsProcess* pProcess = ImsProcess::GetInstance();

    // Starts the main worker thread
    if (!pProcess->Initialize())
    {
        return;
    }

    EngineState::Initialize();
    EnablerLoader::GetInstance()->Init();

    IMS_TRACE_I("ImsMain: start - %s", DeviceConfig::ToString().GetStr(), 0, 0);
}

PUBLIC GLOBAL void ImsMain::Stop()
{
    IMS_TRACE_I("ImsMain: stop", 0, 0, 0);

    EngineState::Uninitialize();

    ImsProcess::GetInstance()->Uninitialize();

    PlatformProperty::Stop();
}

PRIVATE GLOBAL void ImsMain::InitializeConfigurationManager()
{
    if (!ConfigurationManager::GetInstance()->Initialize())
    {
        IMS_TRACE_E(0, "Initializing ConfigurationManager failed", 0, 0, 0);
        return;
    }

    IMS_TRACE_I(">>> ConfigurationManager: Initialized <<<", 0, 0, 0);
}
