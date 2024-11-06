/*
 * Copyright (C) 2024 The Android Open Source Project
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
#ifndef MOCK_I_SERVICE_MANAGER_H_
#define MOCK_I_SERVICE_MANAGER_H_

#include <gmock/gmock.h>

#include "IServiceManager.h"

class MockIServiceManager : public IServiceManager
{
public:
    MockIServiceManager() = default;
    ~MockIServiceManager() override = default;

    MOCK_METHOD(void, ServiceClosed, (IN Service * pService), (override));
    MOCK_METHOD(IMS_BOOL, AttachService, (IN Service * pService), (override));
    MOCK_METHOD(void, DetachService, (IN Service * pService), (override));
    MOCK_METHOD(Service*, GetService,
            (IN IMS_SINT32 nSlotId, IN const AString& strAppId, IN const AString& strServiceId),
            (const, override));
    MOCK_METHOD(const ImsList<Service*>&, GetServices, (), (const, override));
    MOCK_METHOD(ImsList<Service*>, GetServices, (IN IMS_SINT32 nSlotId), (const, override));
};

#endif
