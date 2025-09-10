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
    MOCK_METHOD(void, CreateQos, (IN ISession* piSession), (override));
    MOCK_METHOD(void, DestroyQos, (IN ISession* piSession), (override));
    MOCK_METHOD(void, SetListener, (IN IMtcPreconditionListener* pListener), (override));
    MOCK_METHOD(void, InitializeMobileRatInformation, (), (override));
    MOCK_METHOD(IMS_BOOL, IsPreconditionSupportedInLocal, (), (const, override));
    MOCK_METHOD(IMS_BOOL, IsDedicatedBearerAllocated,
            (IN ISession* piSession, IN IMS_UINT32 eMediaType), (const, override));
    MOCK_METHOD(IMS_BOOL, IsCheckingResourcesRequiredToAlertUser, (), (const, override));
    MOCK_METHOD(IMS_BOOL, IsAvailableToAlertUser, (IN ISession* piSession), (const, override));
    MOCK_METHOD(IMS_BOOL, IsLocalResourceConfirmationRequired, (IN ISession * piSession),
            (const, override));
    MOCK_METHOD(IMS_BOOL, IsAvailableToSendLocalResourceConfirmation, (IN ISession * piSession),
            (const, override));
    MOCK_METHOD(
            IMS_BOOL, IsPreconditionIncludedInSdp, (IN ISession * piSession), (const, override));
    MOCK_METHOD(
            void, FormPreconditionSdp, (IN ISession* piSession, IN IMS_BOOL bFailure), (override));
    MOCK_METHOD(void, OnSdpReceived, (IN ISession * piSession), (override));
    MOCK_METHOD(void, OnSdpSent, (IN ISession * piSession, IN IMS_BOOL bInitialInvite), (override));
    MOCK_METHOD(
            void, OnMessageReceived, (IN ISession* piSession, IN IMessage* piMessage), (override));
    MOCK_METHOD(void, OnCallEstablished, (IN ISession* piSession), (override));
    MOCK_METHOD(void, OnCallModified, (IN ISession* piSession), (override));
    MOCK_METHOD(void, OnRatChanged, (IN IMS_SINT32 eRatType), (override));
};

#endif
