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
#include "ServiceTrace.h"
#include "device/OsPhoneInfoCall.h"
#include "system-intf/System.h"

__IMS_TRACE_TAG_ADAPT__;

PUBLIC
OsPhoneInfoCall::OsPhoneInfoCall(IN IMS_SINT32 nSlotId)
    : ImsSlot(nSlotId)
{
}

PUBLIC VIRTUAL
OsPhoneInfoCall::~OsPhoneInfoCall()
{
}

PUBLIC VIRTUAL
IMS_BOOL OsPhoneInfoCall::IsEmergencyNumber(IN const AString& strNumber) const
{
    return System::GetInstance()->IsEmergencyNumber(strNumber, GetSlotId());
}

PUBLIC VIRTUAL
IMS_UINT32 OsPhoneInfoCall::GetTtyMode() const
{
    return System::GetInstance()->GetTtyMode(GetSlotId());
}

PUBLIC VIRTUAL
IMS_UINT32 OsPhoneInfoCall::GetRttMode() const
{
    return System::GetInstance()->GetRttMode(GetSlotId());
}

PUBLIC VIRTUAL
IMS_BOOL OsPhoneInfoCall::IsWifiCallingEnabled()
{
    return System::GetInstance()->IsWifiCallingEnabled(GetSlotId());
}

PUBLIC VIRTUAL
IMS_UINT32 OsPhoneInfoCall::GetWifiCallingPreferences()
{
    return System::GetInstance()->GetWifiCallingPreferences(GetSlotId());
}

PUBLIC VIRTUAL
IMS_BOOL OsPhoneInfoCall::IsWifiCallingProvisioned()
{
    return System::GetInstance()->IsWifiCallingProvisioned(GetSlotId());
}

PUBLIC VIRTUAL
AString OsPhoneInfoCall::GetWifiCallingAddressId()
{
    return System::GetInstance()->GetWifiCallingAddressId(GetSlotId());
}

PUBLIC VIRTUAL
IMS_SINT32 OsPhoneInfoCall::GetCsCallStateInOtherSlot() const
{
    return System::GetInstance()->GetCallStateInOtherSlot(GetSlotId());
}
