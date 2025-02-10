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
#ifndef OS_PHONE_INFO_CALL_H_
#define OS_PHONE_INFO_CALL_H_

#include "ImsSlot.h"
#include "IPhoneInfoCall.h"

class OsPhoneInfoCall : public ImsSlot, public ICallInfo
{
public:
    explicit OsPhoneInfoCall(IN IMS_SINT32 nSlotId);
    virtual ~OsPhoneInfoCall();

    OsPhoneInfoCall(IN const OsPhoneInfoCall&) = delete;
    OsPhoneInfoCall& operator=(IN const OsPhoneInfoCall&) = delete;

public:
    IMS_BOOL IsEmergencyNumber(IN const AString& strNumber) const override;
    IMS_UINT32 GetTtyMode() const override;
    IMS_UINT32 GetRttMode() const override;

    IMS_BOOL IsWifiCallingEnabled() override;
    IMS_UINT32 GetWifiCallingPreferences() override;
    IMS_BOOL IsWifiCallingProvisioned() override;
    AString GetWifiCallingAddressId() override;

    IMS_SINT32 GetCsCallStateInOtherSlot() const override;
    IMS_BOOL IsCrossSimRedialingAvailable() const override;
};

#endif
