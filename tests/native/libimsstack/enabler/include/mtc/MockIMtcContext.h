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
#include <memory>

class CallConnectionIdManager;
class ICallStateProxy;
class IConferenceManager;
class IEctManager;
class ILastComeFirstServedHelper;
class IMtcAosConnector;
class IMtcCallController;
class IMtcCallManager;
class IMtcDialingPlan;
class IMtcEmergencyServiceManager;
class IMtcImsEventReceiver;
class IMtcRadioChecker;
class IMtcService;
class IMtcSipInterfaceFactory;
class IMultiEndpointManager;
class IPassiveTimerHolder;
class MessageUtils;
class MtcConfigurationProxy;
class MtcTimerWrapper;
class OperationAsyncRunner;

class MockIMtcContext : public IMtcContext
{
public:
    virtual ~MockIMtcContext() {}

    MOCK_METHOD(IMS_SINT32, GetSlotId, (), (const, override));
    MOCK_METHOD(const ISubscriberConfig*, GetSubscriberConfig, (), (const, override));
    MOCK_METHOD(IMtcService*, GetServiceByType, (IN ServiceType), (override));
    MOCK_METHOD(IMtcDialingPlan&, GetDialingPlan, (), (override));
    MOCK_METHOD(IMtcCallController&, GetCallController, (), (override));
    MOCK_METHOD(IMtcCallManager&, GetCallManager, (), (override));
    MOCK_METHOD(IMtcRadioChecker&, GetRadioChecker, (), (override));
    MOCK_METHOD(MtcConfigurationProxy&, GetConfigurationProxy, (), (override));
    MOCK_METHOD(ICallStateProxy&, GetCallStateProxy, (), (override));
    MOCK_METHOD(IMtcImsEventReceiver&, GetImsEventReceiver, (), (override));
    MOCK_METHOD(IMtcAosConnector*, GetAosConnector, (IN ServiceType), (override));
    MOCK_METHOD(IMtcSipInterfaceFactory&, GetSipInterfaceFactory, (), (override));
    MOCK_METHOD(IConferenceManager&, GetConferenceManager, (), (override));
    MOCK_METHOD(IEctManager&, GetEctManager, (), (override));
    MOCK_METHOD(IMtcEmergencyServiceManager&, GetEmergencyServiceManager, (), (override));
    MOCK_METHOD(void, RunAsyncOperation, (IN void*, IN std::function<void()>), (override));
    MOCK_METHOD(void, ReleaseAsyncOperation, (IN void*), (override));
    MOCK_METHOD(IMessageUtils&, GetMessageUtils, (), (override));
    MOCK_METHOD(std::unique_ptr<MtcTimerWrapper>, CreateTimer, (), (override));
    MOCK_METHOD(IPassiveTimerHolder&, GetPassiveTimerHolder, (), (override));
    MOCK_METHOD(IMultiEndpointManager*, GetMultiEndpointManager, (), (override));
    MOCK_METHOD(ILastComeFirstServedHelper&, GetLastComeFirstServedHelper, (), (override));
    MOCK_METHOD(CallConnectionIdManager&, GetCallConnectionIdManager, (), (override));
    MOCK_METHOD(MtcLocationRefresher&, GetLocationRefresher, (), (override));
    MOCK_METHOD(void, CreateRttAutoUpgrader, (), (override));
    MOCK_METHOD(void, DestroyRttAutoUpgrader, (), (override));
    MOCK_METHOD(IMS_BOOL, IsWifiTestMode, (), (override));
};

#endif
