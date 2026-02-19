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
#include "ImsNetworkConnectionState.h"
#include "ImsSocketState.h"
#include "OsTimerService.h"
#include "OsUtil.h"
#include "PlatformContext.h"
#include "PlatformProperty.h"
#include "ServiceMemory.h"
#include "network/OsSocket.h"

PUBLIC GLOBAL IMS_BOOL PlatformProperty::Initialize()
{
    // Initialize PlatformContext
    PlatformContext::GetInstance();

    // Initialize the network connection state & socket state
    ImsNetworkConnectionState_InitInstance();
    ImsSocketState_InitInstance();

    return IMS_TRUE;
}

PUBLIC GLOBAL void PlatformProperty::Start()
{
    // Start up the timer thread
    OsTimerService::StartUp();

    // Start up the socket thread
    OsSocket::StartUp();
}

PUBLIC GLOBAL void PlatformProperty::Stop()
{
    // Clean up the socket thread
    OsSocket::CleanUp();

    // Clean up timer service
    OsTimerService::CleanUp();
}

PUBLIC GLOBAL void PlatformProperty::Uninitialize()
{
    // Uninitialize the other module
    ImsSocketState_ExitInstance();
    ImsNetworkConnectionState_ExitInstance();

    // Initialize PlatformContext
    PlatformContext::DestroyInstance();
}
