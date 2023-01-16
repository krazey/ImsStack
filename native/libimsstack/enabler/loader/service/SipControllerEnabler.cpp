/*
 * Copyright (C) 2023 The Android Open Source Project
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
#include "ServiceTrace.h"

#include "SipControllerFactory.h"
#include "service/SipControllerEnabler.h"

__IMS_TRACE_TAG_USER_DECL__("IMS_SNC");

PUBLIC
SipControllerEnabler::SipControllerEnabler(IN IMS_SINT32 nSlotId) :
        Enabler(nSlotId)
{
}

PUBLIC VIRTUAL SipControllerEnabler::~SipControllerEnabler() {}

PUBLIC VIRTUAL void SipControllerEnabler::Start()
{
    IMS_TRACE_D("Start", 0, 0, 0);
    SipControllerFactory::Start(GetSlotId());
}

PUBLIC VIRTUAL void SipControllerEnabler::Stop()
{
    IMS_TRACE_D("Stop", 0, 0, 0);
    SipControllerFactory::Stop(GetSlotId());
}
