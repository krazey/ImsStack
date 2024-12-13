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
#include "DeviceConfig.h"
#include "ImsProcess.h"
#include "PlatformProperty.h"
#include "ServiceTrace.h"

#include "private/ConfigurationManager.h"

#include "EnablerLoader.h"
#include "EngineState.h"
#include "ImsMain.h"
#include "NativeCommandsHandler.h"

__IMS_TRACE_TAG_BASE__;

static NativeCommandsHandler s_objCommandsHandler;

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
    s_objCommandsHandler.SetEnablerLoader(EnablerLoader::GetInstance());

    IMS_TRACE_I("ImsMain: start - %s", DeviceConfig::ToString().GetStr(), 0, 0);
}

PUBLIC GLOBAL void ImsMain::Stop()
{
    IMS_TRACE_I("ImsMain: stop", 0, 0, 0);

    s_objCommandsHandler.SetEnablerLoader(IMS_NULL);

    EngineState::Uninitialize();
    ImsProcess::GetInstance()->Uninitialize();
    PlatformProperty::Stop();
}

PUBLIC GLOBAL void ImsMain::SendCommand(
        IN IMS_SINT32 nCmd, IN IMS_SINT32 nSlotId, IN IMS_UINTP pnParam)
{
    s_objCommandsHandler.OnCommand(nCmd, nSlotId, pnParam);
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
