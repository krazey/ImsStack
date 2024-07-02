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

#include "CoreContext.h"
#include "EngineState.h"
#include "ImsCore.h"
#include "ProtocolPermission.h"
#include "ServiceProtocol.h"
#include "Sip.h"
#include "SipCoreContext.h"
#include "StaticSip.h"
#include "base/Ims.h"

// Initialization / Uninitialization for Engine
PUBLIC GLOBAL IMS_BOOL EngineState::Initialize()
{
    ProtocolPermission::RegisterProtocol(
            ImsCore::CONNECTION_SCHEME, CoreContext::GetInstance()->GetImsCoreProtocol());
    ProtocolPermission::RegisterProtocol(
            Sip::CONNECTION_SCHEME_SIP, SipCoreContext::GetInstance()->GetSipProtocol());

    // Initialize a SipManager
    IMS_BOOL bResult = StaticSip::Initialize();

    // Initialize another function blocks
    Ims::Init();

    return bResult;
}

PUBLIC GLOBAL void EngineState::Uninitialize()
{
    // Releases all the resources in the reverse order of initialization ...

    // Uninitialize a SipManager
    StaticSip::Uninitialize();

    ProtocolPermission::UnregisterAllProtocols();
}
