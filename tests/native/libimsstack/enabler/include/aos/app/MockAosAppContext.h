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

#ifndef MOCK_AOS_APP_CONTEXT_H_
#define MOCK_AOS_APP_CONTEXT_H_

#include <gmock/gmock.h>

#include "interface/IAosAppContext.h"
#include "app/AosAppContext.h"

class MockAosAppContext : public AosAppContext {
public:
    explicit MockAosAppContext(IN AosStaticProfile* pProfile) :
            AosAppContext(pProfile)
    {
    }
    ~MockAosAppContext() override {}

    MOCK_METHOD(IMS_SINT32, GetSlotId, (), (const, override));
    MOCK_METHOD(const AString&, GetProfileId, (), (const, override));
    MOCK_METHOD(IAosHandle*, GetHandle, (IN const AString& strSrvId), (const, override));
    MOCK_METHOD(IAosHandle*, GetHandle, (IN IMS_UINT32 nServiceType), (override));
    MOCK_METHOD((ImsMap<AString, IAosHandle*>&), GetHandles, (), (override));
    MOCK_METHOD(IAosApplication*, GetApp, (), (const, override));
    MOCK_METHOD(IAosConnection*, GetConnection, (), (const, override));
    MOCK_METHOD(IAosRegistration*, GetRegistration, (), (const, override));
    MOCK_METHOD(IAosNetTracker*, GetNetTracker, (), (const, override));
    MOCK_METHOD(IAosBlock*, GetBlock, (), (const, override));
    MOCK_METHOD(IAosSubscriber*, GetSubscriber, (), (const, override));
    MOCK_METHOD(IAosPcscf*, GetPcscf, (), (const, override));
    MOCK_METHOD(AosStaticProfile*, GetStaticProfile, (), (const, override));
    MOCK_METHOD(void, SetSlotId, (IN IMS_SINT32 nSlotId), (override));
    MOCK_METHOD(void, AddHandle, (IN const AString& strSrvId, IN IAosHandle* piHandle), (override));
    MOCK_METHOD(void, SetApp, (IN IAosApplication* piApp), (override));
    MOCK_METHOD(void, SetConnection, (IN IAosConnection* piConnection), (override));
    MOCK_METHOD(void, SetRegistration, (IN IAosRegistration* piRegistration), (override));
    MOCK_METHOD(void, SetNetTracker, (IN IAosNetTracker* piNetTracker), (override));
    MOCK_METHOD(void, SetBlock, (IN IAosBlock* piBlock), (override));
    MOCK_METHOD(void, SetSubscriber, (IN IAosSubscriber* piSubscriber), (override));
    MOCK_METHOD(void, SetPcscf, (IN IAosPcscf* piPcscf), (override));
};

#endif // MOCK_AOS_APP_CONTEXT_H_
