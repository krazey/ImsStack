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
#ifndef MOCK_I_PHONE_INFO_CALL_H_
#define MOCK_I_PHONE_INFO_CALL_H_

#include <gmock/gmock.h>

#include "IPhoneInfoCall.h"

class MockICallInfo : public ICallInfo
{
public:
    inline MockICallInfo() {}
    inline virtual ~MockICallInfo() {}

    MOCK_METHOD(IMS_BOOL, IsEmergencyNumber, (IN const AString& strNumber), (const, override));
    MOCK_METHOD(IMS_UINT32, GetTtyMode, (), (const, override));
    MOCK_METHOD(IMS_UINT32, GetRttMode, (), (const, override));
    MOCK_METHOD(IMS_BOOL, IsWifiCallingEnabled, (), (override));
    MOCK_METHOD(IMS_UINT32, GetWifiCallingPreferences, (), (override));
    MOCK_METHOD(IMS_BOOL, IsWifiCallingProvisioned, (), (override));
    MOCK_METHOD(AString, GetWifiCallingAddressId, (), (override));
    MOCK_METHOD(IMS_SINT32, GetCsCallStateInOtherSlot, (), (const, override));
    MOCK_METHOD(IMS_BOOL, IsCrossSimRedialingAvailable, (), (const, override));
};

#endif
