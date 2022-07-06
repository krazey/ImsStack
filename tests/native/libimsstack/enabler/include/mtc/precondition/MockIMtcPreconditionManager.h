/*
 * Copyright (C) 2022 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except compliance with the License.
 * You may obtaa copy of the License at
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

#include <gmock/gmock.h>

#include "ImsTypeDef.h"
#include "precondition/IMtcPreconditionManager.h"
#include "precondition/QosDef.h"

class ISession;
class IMessage;
class IMtcPreconditionListener;

class MockIMtcPreconditionManager : public IMtcPreconditionManager
{
public:
    ~MockIMtcPreconditionManager() {}
    MOCK_METHOD(void, CreateQos, (IN ISession * piSession), (override));
    MOCK_METHOD(void, DestroyQos, (IN ISession * piSession), (override));
    MOCK_METHOD(void, SetListener, (IN IMtcPreconditionListener * pListener), (override));
    MOCK_METHOD(IMS_BOOL, IsResourceReserved, (IN ISession * piSession, IN QosCheckType eType),
            (override));
    MOCK_METHOD(void, StartQosTimer, (IN ISession * piSession, IN QosTimerType eType), (override));
    MOCK_METHOD(void, StopQosTimer, (IN ISession * piSession, IN QosTimerType eType), (override));
    MOCK_METHOD(void, StopAllQosTimer, (IN ISession * piSession), (override));
    MOCK_METHOD(void, UpdatePreconditionCapability,
            (IN ISession * piSession, IN IMS_BOOL bCapability), (override));
    MOCK_METHOD(IMS_BOOL, HasPreconditionCapability, (IN ISession * piSession), (override));
    MOCK_METHOD(IMS_BOOL, IsPreconditionSupportedInLocal, (), (override));
    MOCK_METHOD(void, UpdateQosAttributesFromSdp, (IN ISession * piSession), (override));
    MOCK_METHOD(
            void, FormPreconditionSdp, (IN ISession * piSession, IN IMS_BOOL bFailure), (override));
    MOCK_METHOD(void, RemovePreconditionSdp, (IN ISession * piSession), (override));
    MOCK_METHOD(IMS_UINT32, SetLocalResourceAvailable, (IN ISession * piSession), (override));
    MOCK_METHOD(void, SetRemoteResourceAvailable, (IN ISession * piSession), (override));
};

#endif
