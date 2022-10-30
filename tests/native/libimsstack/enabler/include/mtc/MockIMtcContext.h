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

#ifndef MOCK_I_MTC_CONTEXT_H_
#define MOCK_I_MTC_CONTEXT_H_

#include "IMtcContext.h"
#include "ImsTypeDef.h"
#include <gmock/gmock.h>
#include <functional>

class IMtcDialingPlan;
class IMtcCallController;
class IMtcCallManager;
class MtcConfigurationProxy;
class ICallStateProxy;
class IMtcAosConnector;
class IMtcImsEventReceiver;
class IMtcSipInterfaceFactory;
class IMtcService;
class IConferenceManager;
class IEctManager;
class MtcEmergencyServiceManager;
class OperationAsyncRunner;
class MessageUtils;
class IMtcCallTrafficChecker;

class MockIMtcContext : public IMtcContext
{
public:
    virtual ~MockIMtcContext() {}

    MOCK_METHOD(IMS_SINT32, GetSlotId, (), (override));
    MOCK_METHOD(IMtcService*, GetServiceByType, (IN ServiceType), (override));
    MOCK_METHOD(IMtcDialingPlan&, GetDialingPlan, (), (override));
    MOCK_METHOD(IMtcCallController&, GetCallController, (), (override));
    MOCK_METHOD(IMtcCallManager&, GetCallManager, (), (override));
    MOCK_METHOD(IMtcCallTrafficChecker&, GetCallTrafficChecker, (), (override));
    MOCK_METHOD(MtcConfigurationProxy&, GetConfigurationProxy, (), (override));
    MOCK_METHOD(ICallStateProxy&, GetCallStateProxy, (), (override));
    MOCK_METHOD(IMtcImsEventReceiver&, GetImsEventReceiver, (), (override));
    MOCK_METHOD(IMtcAosConnector*, GetAosConnector, (IN ServiceType), (override));
    MOCK_METHOD(IMtcSipInterfaceFactory&, GetSipInterfaceFactory, (), (override));
    MOCK_METHOD(IConferenceManager&, GetConferenceManager, (), (override));
    MOCK_METHOD(IEctManager*, GetEctManager, (), (override));
    MOCK_METHOD(MtcEmergencyServiceManager*, GetEmergencyServiceManager, (), (override));
    MOCK_METHOD(OperationAsyncRunner*, GetAsyncRunner, (IN std::function<void()>), (override));
    MOCK_METHOD(IMessageUtils&, GetMessageUtils, (), (override));
    MOCK_METHOD(IMS_BOOL, IsWifiTestMode, (), (override));
};

#endif
