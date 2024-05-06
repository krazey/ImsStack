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

#ifndef MOCK_I_MTC_CALL_CONTEXT_H_
#define MOCK_I_MTC_CALL_CONTEXT_H_

#include "ImsList.h"
#include "ImsTypeDef.h"
#include "JniCallInfo.h"
#include "call/IMtcCallContext.h"
#include <gmock/gmock.h>
#include <functional>
#include <memory>

class CallConnectionIdManager;
class CurrentLocationDiscoveryController;
class EpsFallbackTrigger;
class ILastComeFirstServedHelper;
class IMtcBlockChecker;
class IMtcBlockRule;
class IMtcMediaManager;
class IMtcPreconditionManager;
class IMtcRadioChecker;
class IMtcService;
class IMtcSession;
class IMtcUiNotifier;
class IMultiEndpointManager;
class IPassiveTimerHolder;
class ISession;
class ISipClientConnection;
class MessageUtils;
class MtcPendingOperationHolder;
class MtcSupplementaryService;
class MtcTimerWrapper;
class OperationAsyncRunner;
class ParticipantInfo;
class UdpKeepAliveSender;
class UpdatingInfo;
class UssiController;
struct CallInfo;

class MockIMtcCallContext : public IMtcCallContext
{
public:
    virtual ~MockIMtcCallContext() {}

    MOCK_METHOD(IMS_UINTP, GetCallKey, (), (const, override));
    MOCK_METHOD(IMS_BOOL, IsHeldByMe, (), (const, override));
    MOCK_METHOD(IMS_BOOL, IsUssi, (), (const, override));
    MOCK_METHOD(CallInfo&, GetCallInfo, (), (override));
    MOCK_METHOD(ParticipantInfo&, GetParticipantInfo, (), (override));
    MOCK_METHOD(IMtcSession*, GetSession, (IN const ISession* piSession), (const, override));
    MOCK_METHOD(IMtcSession*, GetSession, (), (const, override));
    MOCK_METHOD(const ImsList<IMtcSession*>&, GetSessions, (), (const, override));
    MOCK_METHOD(IMtcService&, GetService, (), (override));
    MOCK_METHOD(IMtcUiNotifier&, GetUiNotifier, (), (override));
    MOCK_METHOD(IMtcMediaManager&, GetMediaManager, (), (override));
    MOCK_METHOD(IMtcPreconditionManager&, GetPreconditionManager, (), (override));
    MOCK_METHOD(MtcTimerWrapper&, GetTimer, (), (override));
    MOCK_METHOD(MtcSupplementaryService&, GetSupplementaryService, (), (override));
    MOCK_METHOD(UpdatingInfo&, GetUpdatingInfo, (), (override));
    MOCK_METHOD(EpsFallbackTrigger&, GetEpsFallbackTrigger, (), (override));
    MOCK_METHOD(UdpKeepAliveSender&, GetUdpKeepAliveSender, (), (override));
    MOCK_METHOD(CurrentLocationDiscoveryController&, GetCurrentLocationDiscoveryController, (),
            (override));
    MOCK_METHOD(UssiController*, GetUssiController, (), (override));
    MOCK_METHOD(MtcPendingOperationHolder&, GetPendingOperationHolder, (), (override));
    MOCK_METHOD(IMtcCall&, GetCall, (), (override));
    MOCK_METHOD(ImsList<IMtcCall*>, GetOtherCalls, (), (override));
    MOCK_METHOD(void, SetHeldByMe, (IMS_BOOL), (override));
    MOCK_METHOD(IMtcSession*, CreateSession, (IN ISession* piSession), (override));
    MOCK_METHOD(IMtcSession*, CreateSession, (), (override));
    MOCK_METHOD(IMtcBlockChecker*, CreateBlockChecker, (IN const ImsList<IMtcBlockRule*>& lstRules),
            (override));
    MOCK_METHOD(JniCallInfo, CreateJniCallInfo, (), (override));
    MOCK_METHOD(ISipClientConnection*, CreateClientConnection, (IN SipMethod eMethod), (override));
    MOCK_METHOD(void, RemoveSession, (IN const ISession* piSession), (override));
    MOCK_METHOD(void, RemoveInactiveSessions, (IN const ISession* piActiveSession), (override));
    MOCK_METHOD(void, DeleteUpdatingInfo, (), (override));
    MOCK_METHOD(void, RunPendingOperationIfPossible, (), (override));

    // IMtcContext
    MOCK_METHOD(IMS_SINT32, GetSlotId, (), (const, override));
    MOCK_METHOD(const ISubscriberConfig*, GetSubscriberConfig, (), (const, override));
    MOCK_METHOD(IMtcService*, GetServiceByType, (IN ServiceType eServiceType), (override));
    MOCK_METHOD(IMtcDialingPlan&, GetDialingPlan, (), (override));
    MOCK_METHOD(IMtcCallController&, GetCallController, (), (override));
    MOCK_METHOD(IMtcCallManager&, GetCallManager, (), (override));
    MOCK_METHOD(IMtcRadioChecker&, GetRadioChecker, (), (override));
    MOCK_METHOD(MtcConfigurationProxy&, GetConfigurationProxy, (), (override));
    MOCK_METHOD(ICallStateProxy&, GetCallStateProxy, (), (override));
    MOCK_METHOD(IMtcImsEventReceiver&, GetImsEventReceiver, (), (override));
    MOCK_METHOD(IMtcAosConnector*, GetAosConnector, (IN ServiceType eServiceType), (override));
    MOCK_METHOD(IMtcSipInterfaceFactory&, GetSipInterfaceFactory, (), (override));
    MOCK_METHOD(IConferenceManager&, GetConferenceManager, (), (override));
    MOCK_METHOD(IEctManager&, GetEctManager, (), (override));
    MOCK_METHOD(IMtcEmergencyServiceManager&, GetEmergencyServiceManager, (), (override));
    MOCK_METHOD(OperationAsyncRunner*, GetAsyncRunner, (IN std::function<void()>), (override));
    MOCK_METHOD(IMessageUtils&, GetMessageUtils, (), (override));
    MOCK_METHOD(std::unique_ptr<MtcTimerWrapper>, CreateTimer, (), (override));
    MOCK_METHOD(IPassiveTimerHolder&, GetPassiveTimerHolder, (), (override));
    MOCK_METHOD(IMultiEndpointManager*, GetMultiEndpointManager, (), (override));
    MOCK_METHOD(ILastComeFirstServedHelper&, GetLastComeFirstServedHelper, (), (override));
    MOCK_METHOD(CallConnectionIdManager&, GetCallConnectionIdManager, (), (override));
    MOCK_METHOD(IMS_BOOL, IsWifiTestMode, (), (override));
};

#endif
