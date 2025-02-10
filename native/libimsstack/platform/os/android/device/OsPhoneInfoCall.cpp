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
#include "PlatformContext.h"
#include "ServiceMemory.h"
#include "ServiceTrace.h"
#include "device/OsPhoneInfoCall.h"

__IMS_TRACE_TAG_IPL__;

PUBLIC
OsPhoneInfoCall::OsPhoneInfoCall(IN IMS_SINT32 nSlotId) :
        ImsSlot(nSlotId)
{
}

PUBLIC VIRTUAL OsPhoneInfoCall::~OsPhoneInfoCall() {}

PUBLIC VIRTUAL IMS_BOOL OsPhoneInfoCall::IsEmergencyNumber(IN const AString& strNumber) const
{
    return PlatformContext::GetInstance()->GetSystem()->IsEmergencyNumber(strNumber, GetSlotId());
}

PUBLIC VIRTUAL IMS_UINT32 OsPhoneInfoCall::GetTtyMode() const
{
    return PlatformContext::GetInstance()->GetSystem()->GetTtyMode(GetSlotId());
}

PUBLIC VIRTUAL IMS_UINT32 OsPhoneInfoCall::GetRttMode() const
{
    return PlatformContext::GetInstance()->GetSystem()->GetRttMode(GetSlotId());
}

PUBLIC VIRTUAL IMS_BOOL OsPhoneInfoCall::IsWifiCallingEnabled()
{
    return PlatformContext::GetInstance()->GetSystem()->IsWifiCallingEnabled(GetSlotId());
}

PUBLIC VIRTUAL IMS_UINT32 OsPhoneInfoCall::GetWifiCallingPreferences()
{
    return PlatformContext::GetInstance()->GetSystem()->GetWifiCallingPreferences(GetSlotId());
}

PUBLIC VIRTUAL IMS_BOOL OsPhoneInfoCall::IsWifiCallingProvisioned()
{
    return PlatformContext::GetInstance()->GetSystem()->IsWifiCallingProvisioned(GetSlotId());
}

PUBLIC VIRTUAL AString OsPhoneInfoCall::GetWifiCallingAddressId()
{
    return PlatformContext::GetInstance()->GetSystem()->GetWifiCallingAddressId(GetSlotId());
}

PUBLIC VIRTUAL IMS_SINT32 OsPhoneInfoCall::GetCsCallStateInOtherSlot() const
{
    return PlatformContext::GetInstance()->GetSystem()->GetCsCallStateInOtherSlot(GetSlotId());
}

PUBLIC VIRTUAL IMS_BOOL OsPhoneInfoCall::IsCrossSimRedialingAvailable() const
{
    return PlatformContext::GetInstance()->GetSystem()->IsCrossSimRedialingAvailable(GetSlotId());
}
