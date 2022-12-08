/*
 * Copyright (C) 2022 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef MOCK_I_MTC_PRECONDITION_MANAGER_H_
#define MOCK_I_MTC_PRECONDITION_MANAGER_H_

#include "ImsTypeDef.h"
#include "precondition/IMtcPreconditionManager.h"
#include "precondition/QosDef.h"
#include <gmock/gmock.h>

class ISession;
class IMtcPreconditionListener;

class MockIMtcPreconditionManager : public IMtcPreconditionManager
{
public:
    ~MockIMtcPreconditionManager() {}
    MOCK_METHOD(void, CreateQos, (IN ISession* piSession), (override));
    MOCK_METHOD(void, DestroyQos, (IN ISession* piSession), (override));
    MOCK_METHOD(void, SetListener, (IN IMtcPreconditionListener* pListener), (override));
    MOCK_METHOD(void, StartQosTimer, (IN ISession* piSession, IN QosTimerType eType), (override));
    MOCK_METHOD(IMS_BOOL, IsPreconditionSupportedInLocal, (), (const, override));
    MOCK_METHOD(void, UpdateSupportingPrecondition,
            (IN ISession* piSession, IN IMS_BOOL bRemoteSupported), (override));
    MOCK_METHOD(IMS_BOOL, IsPreconditionSupported, (IN ISession* piSession), (const, override));
    MOCK_METHOD(IMS_BOOL, IsResourceReserved,
            (IN ISession* piSession, IN QosCheckType eType, IN IMS_BOOL bAtLeastOneReserved),
            (const, override));
    MOCK_METHOD(IMS_BOOL, IsDedicatedBearerAllocated,
            (IN ISession* piSession, IN IMS_UINT32 eMediaType), (const, override));
    MOCK_METHOD(void, UpdateQosAttributesFromRemoteSdp, (IN ISession* piSession), (override));
    MOCK_METHOD(void, SetRemoteResourceAvailable, (IN ISession* piSession), (override));
    MOCK_METHOD(
            void, FormPreconditionSdp, (IN ISession* piSession, IN IMS_BOOL bFailure), (override));
    MOCK_METHOD(void, HandleQosOnIpcanChanged, (), (override));
    MOCK_METHOD(void, CheckLocalResourceAvailableOnCallEstablished,
            (IN ISession* piSession, IN IMS_BOOL bCallModified), (override));
};

#endif
