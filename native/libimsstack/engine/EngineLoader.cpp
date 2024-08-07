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
#include "EngineLoader.h"
#include "ImsCoreContext.h"
#include "StaticSip.h"
#include "base/Ims.h"
#include "base/SubscriberTracker.h"
#include "util/ISipConnectionNotifierManager.h"

// It will be called by EnablerThread to load a proper component for each slot.

PUBLIC GLOBAL void EngineLoader::Initialize(IN IMS_SINT32 nSlotId)
{
    // Service
    SubscriberTracker::GetInstance()->InitForSlot(nSlotId);

    // sipcore
    StaticSip::InitializeForSlot(nSlotId);

    // core
    Ims::Init(nSlotId);
    ImsCoreContext::GetInstance()->GetSipConnectionNotifierManager()->Init(nSlotId);
}

PUBLIC GLOBAL void EngineLoader::Uninitialize(IN IMS_SINT32 nSlotId)
{
    // sipcore
    StaticSip::UninitializeForSlot(nSlotId);
}
