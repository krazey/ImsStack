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

#ifndef MOCK_I_AOS_REGISTRATION_H_
#define MOCK_I_AOS_REGISTRATION_H_

#include <gmock/gmock.h>

#include "ImsTypeDef.h"
#include "interface/IAosRegistration.h"

class MockIAosRegistration : public IAosRegistration {
public:
    MockIAosRegistration()
    {
        ON_CALL(*this, GetProperty)
                .WillByDefault(
                        [this](IN IMS_UINT32 nType, OUT IMS_UINT32& nValue, OUT AString& strValue)
                        {
                            return GetPropertyInternal(nType, &nValue, &strValue);
                        });
    }

    MOCK_METHOD(void, Start, (), (override));
    MOCK_METHOD(void, Stop, (), (override));
    MOCK_METHOD(void, Update, (IN IMS_BOOL bIgnoreRetryTimer, IN IMS_BOOL bExplicitUpdate),
            (override));
    MOCK_METHOD(void, Reconfig, (), (override));
    MOCK_METHOD(void, Destroy, (), (override));
    MOCK_METHOD(void, SetListener, (IN IAosRegistrationListener* piRegListener), (override));
    MOCK_METHOD(void, RequestCmd, (IN IMS_UINT32 nCmdType, IN IMS_UINT32 nReason), (override));
    MOCK_METHOD(IMS_UINT32, GetMode, (), (override));
    MOCK_METHOD(IMS_UINT32, GetProperty, (IN IMS_UINT32 nType, OUT IMS_UINT32& nValue,
            OUT AString& strValue), (override));
    MOCK_METHOD(IMS_UINT32, GetState, (), (override));
    MOCK_METHOD(AosRegistrationType, GetRegType, (), (override));
    MOCK_METHOD(IMS_BOOL, IsRegistered, (), (override));
    MOCK_METHOD(IMS_BOOL, IsRefreshing, (), (override));
    MOCK_METHOD(IMS_BOOL, IsRetryTimer, (), (override));
    MOCK_METHOD(IMS_BOOL, IsRetryHeld, (), (override));
    MOCK_METHOD(IMS_BOOL, IsTerminated, (), (override));
    MOCK_METHOD(void, SetAppReady, (IN IMS_BOOL bReady), (override));

    MOCK_METHOD(void, Init, (), (override));
    MOCK_METHOD(void, CleanUp, (), (override));

    // Add mock method that can set OUT parameter
    MOCK_METHOD(IMS_UINT32, GetPropertyInternal,
            (IN IMS_UINT32 nType, OUT IMS_UINT32* nValue, OUT AString* strValue));
};

#endif // MOCK_I_AOS_REGISTRATION_H_
