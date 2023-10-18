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

#ifndef MOCK_I_AOS_APPLICATION_H_
#define MOCK_I_AOS_APPLICATION_H_

#include <gmock/gmock.h>

#include "ImsTypeDef.h"

#include "interface/IAosApplication.h"

class MockIAosApplication : public IAosApplication
{
public:
    MOCK_METHOD(void, Reconfig, (), (override));
    MOCK_METHOD(IMS_BOOL, RequestCmd, (IN IMS_UINT32 nCmdType, IN IMS_UINT32 nReason), (override));
    MOCK_METHOD(const AString&, GetActivityName, (), (override));
    MOCK_METHOD(void, GetProperty,
            (IN IMS_UINT32 nType, OUT IMS_UINT32& nValue, OUT AString& strValue), (override));
    MOCK_METHOD(IMS_UINT32, GetAppState, (), (override));
    MOCK_METHOD(IMS_UINT32, GetOffReason, (), (override));
    MOCK_METHOD(IMS_BOOL, IsActivated, (), (override));
    MOCK_METHOD(IMS_BOOL, IsOn, (), (override));
    MOCK_METHOD(void, SetActivation, (IN IMS_BOOL bActivation), (override));
    MOCK_METHOD(void, NotifyEpsFallbackCallState, (IN IMS_UINT32 nState), (override));
    MOCK_METHOD(void, NotifyPublishState, (IN IMS_BOOL bStart), (override));
    MOCK_METHOD(void, Init, (), (override));
    MOCK_METHOD(void, CleanUp, (), (override));
};

#endif  // MOCK_I_AOS_APPLICATION_H_
