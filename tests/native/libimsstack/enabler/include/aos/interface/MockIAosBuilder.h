/*
 * Copyright (C) 2026 The Android Open Source Project
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
#ifndef MOCK_I_AOS_BUILDER_H_
#define MOCK_I_AOS_BUILDER_H_

#include <gmock/gmock.h>

#include "interface/IAosBuilder.h"

class AString;
class IAosAppContext;
class IAosApplication;
class IAosBlock;
class IAosCallTracker;
class IAosConnection;
class IAosHandle;
class IAosNConfiguration;
class IAosNetTracker;
class IAosPcscf;
class IAosRegStateManager;
class IAosRegistration;
class IAosRetryRepository;
class IAosService;
class IAosSubscriber;
class IAosSubscriberManager;
class IAosTracer;
class IAosTransaction;
class AosStaticProfile;

class MockIAosBuilder : public IAosBuilder
{
public:
    virtual ~MockIAosBuilder() = default;

    // AosAppContext Components
    MOCK_METHOD(IAosAppContext*, BuildAppContext, (IN AosStaticProfile * pProfile), (override));
    MOCK_METHOD(IAosApplication*, BuildApp, (IN IAosAppContext * piAppContext), (override));
    MOCK_METHOD(IAosHandle*, BuildHandle,
            (IN IAosAppContext * piAppContext, IN const AString& strAppId,
                    IN const AString& strSrvId),
            (override));
    MOCK_METHOD(
            IAosRegistration*, BuildRegistration, (IN IAosAppContext * piAppContext), (override));
    MOCK_METHOD(IAosSubscriber*, BuildSubscriber, (IN IAosAppContext * piAppContext), (override));
    MOCK_METHOD(IAosPcscf*, BuildPcscf, (IN IAosAppContext * piAppContext), (override));
    MOCK_METHOD(IAosBlock*, BuildBlock, (IN IAosAppContext * piAppContext), (override));
    MOCK_METHOD(IAosConnection*, BuildConnection, (IN IAosAppContext * piAppContext), (override));
    MOCK_METHOD(IAosNetTracker*, BuildNetTracker, (IN IAosAppContext * piAppContext), (override));

    // AosProvider Services
    MOCK_METHOD(IAosCallTracker*, BuildCallTracker, (IN IMS_SINT32 nSlotId), (override));
    MOCK_METHOD(IAosRegStateManager*, BuildRegStateManager, (), (override));
    MOCK_METHOD(IAosService*, BuildService, (IN IMS_SINT32 nSlotId), (override));
    MOCK_METHOD(
            IAosSubscriberManager*, BuildSubscriberManager, (IN IMS_SINT32 nSlotId), (override));
    MOCK_METHOD(IAosRetryRepository*, BuildRetryRepository, (IN IMS_SINT32 nSlotId), (override));
    MOCK_METHOD(IAosNConfiguration*, BuildNConfiguration, (), (override));
    MOCK_METHOD(IAosTracer*, BuildTracer, (IN IMS_SINT32 nSlotId), (override));
    MOCK_METHOD(IAosTransaction*, BuildTransaction, (IN IMS_SINT32 nSlotId), (override));
};

#endif  // MOCK_I_AOS_BUILDER_H_
