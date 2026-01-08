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

#include "CallReasonInfo.h"
#include "IImsAosInfo.h"
#include "MockIMtcService.h"
#include "MockISession.h"
#include "MtcDef.h"
#include "PlatformContext.h"
#include "TestImsRadioService.h"
#include "TestTimerService.h"
#include "call/EpsFallbackTrigger.h"
#include "call/IMtcCall.h"
#include "call/MockIMtcCall.h"
#include "call/MockIMtcCallContext.h"
#include "call/MockIMtcSession.h"
#include "configuration/MockMtcConfigurationProxy.h"
#include "configuration/MtcConfigurationProxy.h"
#include "helper/MockIMtcAosConnector.h"
#include "precondition/MockIMtcPreconditionManager.h"
#include "precondition/QosDef.h"
#include <gtest/gtest.h>

using ::testing::_;
using ::testing::Return;
using ::testing::ReturnRef;

class EpsFallbackTriggerTest : public ::testing::Test
{
public:
    inline EpsFallbackTriggerTest() :
            pConfigurationProxy(IMS_NULL),
            objOtherCalls(),
            objTimerService(),
            objTimer(objTimerService.GetMockTimer()),
            pEpsFbTrigger(IMS_NULL)
    {
    }

    MockIMtcCallContext objContext;
    MockIMtcService objService;
    MockIMtcAosConnector objAosConnector;
    MockMtcConfigurationProxy* pConfigurationProxy;
    ImsList<IMtcCall*> objOtherCalls;
    TestImsRadioService objImsRadioService;
    TestTimerService objTimerService;
    CallInfo objCallInfo;
    MockITimer& objTimer;
    EpsFallbackTrigger* pEpsFbTrigger;

protected:
    virtual void SetUp() override
    {
        PlatformContext::GetInstance()->SetService(
                PlatformContext::SERVICE_TIMER, &objTimerService);
        PlatformContext::GetInstance()->SetService(
                PlatformContext::SERVICE_RADIO, &objImsRadioService);

        ON_CALL(objService, IsNr).WillByDefault(Return(IMS_TRUE));
        ON_CALL(objService, GetAosConnector).WillByDefault(Return(&objAosConnector));

        ON_CALL(objContext, GetService).WillByDefault(ReturnRef(objService));
        pConfigurationProxy = new MockMtcConfigurationProxy();
        ON_CALL(objContext, GetConfigurationProxy).WillByDefault(ReturnRef(*pConfigurationProxy));
        ON_CALL(objContext, GetCallInfo).WillByDefault(ReturnRef(objCallInfo));
        ON_CALL(objContext, GetOtherCalls).WillByDefault(Return(objOtherCalls));

        pEpsFbTrigger = new EpsFallbackTrigger(objContext);
    }

    virtual void TearDown() override
    {
        delete pConfigurationProxy;
        delete pEpsFbTrigger;

        PlatformContext::GetInstance()->SetService(PlatformContext::SERVICE_TIMER, IMS_NULL);
        PlatformContext::GetInstance()->SetService(PlatformContext::SERVICE_RADIO, IMS_NULL);
    }
};

TEST_F(EpsFallbackTriggerTest, ShouldTriggerByReasonInfoReturnsFalseIfMt)
{
    objCallInfo.ePeerType = PeerType::MT;
    ON_CALL(objService, IsNr).WillByDefault(Return(IMS_TRUE));
    ON_CALL(*pConfigurationProxy,
            GetBoolean(ConfigVoice::KEY_EPS_FALLBACK_TRIGGER_BY_AC_BARRING_BOOL))
            .WillByDefault(Return(IMS_TRUE));

    EXPECT_FALSE(EpsFallbackTrigger::ShouldTriggerByReasonInfo(
            objContext, CallReasonInfo(CODE_ACCESS_CLASS_BLOCKED)));
}

TEST_F(EpsFallbackTriggerTest, ShouldTriggerByReasonInfoReturnsFalseIfNotInNr)
{
    objCallInfo.ePeerType = PeerType::MO;
    ON_CALL(objService, IsNr).WillByDefault(Return(IMS_FALSE));
    ON_CALL(*pConfigurationProxy,
            GetBoolean(ConfigVoice::KEY_EPS_FALLBACK_TRIGGER_BY_AC_BARRING_BOOL))
            .WillByDefault(Return(IMS_TRUE));

    EXPECT_FALSE(EpsFallbackTrigger::ShouldTriggerByReasonInfo(
            objContext, CallReasonInfo(CODE_ACCESS_CLASS_BLOCKED)));
}

TEST_F(EpsFallbackTriggerTest, ShouldTriggerByReasonInfoReturnsFalseIfEpsFbNotAvailable)
{
    objCallInfo.ePeerType = PeerType::MO;
    ON_CALL(objService, IsNr).WillByDefault(Return(IMS_TRUE));
    MockIMtcCall objOtherCall;
    ON_CALL(objOtherCall, GetState()).WillByDefault(Return(IMtcCall::State::ESTABLISHED));
    objOtherCalls.Append(&objOtherCall);
    ON_CALL(objContext, GetOtherCalls).WillByDefault(Return(objOtherCalls));
    ON_CALL(*pConfigurationProxy,
            GetBoolean(ConfigVoice::KEY_EPS_FALLBACK_TRIGGER_BY_AC_BARRING_BOOL))
            .WillByDefault(Return(IMS_TRUE));

    EXPECT_FALSE(EpsFallbackTrigger::ShouldTriggerByReasonInfo(
            objContext, CallReasonInfo(CODE_ACCESS_CLASS_BLOCKED)));
}

TEST_F(EpsFallbackTriggerTest, ShouldTriggerByReasonInfoReturnsFalseForAcBlockedIfConfigOff)
{
    objCallInfo.ePeerType = PeerType::MO;
    ON_CALL(objService, IsNr).WillByDefault(Return(IMS_TRUE));
    ON_CALL(*pConfigurationProxy,
            GetBoolean(ConfigVoice::KEY_EPS_FALLBACK_TRIGGER_BY_AC_BARRING_BOOL))
            .WillByDefault(Return(IMS_FALSE));

    EXPECT_FALSE(EpsFallbackTrigger::ShouldTriggerByReasonInfo(
            objContext, CallReasonInfo(CODE_ACCESS_CLASS_BLOCKED)));
}

TEST_F(EpsFallbackTriggerTest, ShouldTriggerByReasonInfoReturnsTrueForAcBlocked)
{
    objCallInfo.ePeerType = PeerType::MO;
    ON_CALL(objService, IsNr).WillByDefault(Return(IMS_TRUE));
    ON_CALL(*pConfigurationProxy,
            GetBoolean(ConfigVoice::KEY_EPS_FALLBACK_TRIGGER_BY_AC_BARRING_BOOL))
            .WillByDefault(Return(IMS_TRUE));

    EXPECT_TRUE(EpsFallbackTrigger::ShouldTriggerByReasonInfo(
            objContext, CallReasonInfo(CODE_ACCESS_CLASS_BLOCKED)));
}

TEST_F(EpsFallbackTriggerTest,
        ShouldTriggerByReasonInfoReturnsFalseForRrcRejectIfRejectTimeIsLessThanMoRequestTimeout)
{
    objCallInfo.ePeerType = PeerType::MO;
    ON_CALL(objService, IsNr).WillByDefault(Return(IMS_TRUE));
    ON_CALL(*pConfigurationProxy,
            GetBoolean(ConfigVoice::KEY_EPS_FALLBACK_TRIGGER_BY_RRC_REJECT_WAIT_TIME_BOOL))
            .WillByDefault(Return(IMS_FALSE));
    ON_CALL(*pConfigurationProxy, GetInt(ConfigVoice::KEY_MO_CALL_REQUEST_TIMEOUT_MILLIS_INT))
            .WillByDefault(Return(1000));

    EXPECT_FALSE(EpsFallbackTrigger::ShouldTriggerByReasonInfo(
            objContext, CallReasonInfo(CODE_INTERNAL_RRC_REJECT, 2000)));
}

TEST_F(EpsFallbackTriggerTest, ShouldTriggerByReasonInfoReturnsFalseForRrcRejectIfConfigOff)
{
    objCallInfo.ePeerType = PeerType::MO;
    ON_CALL(objService, IsNr).WillByDefault(Return(IMS_TRUE));
    ON_CALL(*pConfigurationProxy,
            GetBoolean(ConfigVoice::KEY_EPS_FALLBACK_TRIGGER_BY_RRC_REJECT_WAIT_TIME_BOOL))
            .WillByDefault(Return(IMS_FALSE));
    ON_CALL(*pConfigurationProxy, GetInt(ConfigVoice::KEY_MO_CALL_REQUEST_TIMEOUT_MILLIS_INT))
            .WillByDefault(Return(2000));

    EXPECT_FALSE(EpsFallbackTrigger::ShouldTriggerByReasonInfo(
            objContext, CallReasonInfo(CODE_INTERNAL_RRC_REJECT, 2000)));
}

TEST_F(EpsFallbackTriggerTest, ShouldTriggerByReasonInfoReturnsTrueForRrcReject)
{
    objCallInfo.ePeerType = PeerType::MO;
    ON_CALL(objService, IsNr).WillByDefault(Return(IMS_TRUE));
    ON_CALL(*pConfigurationProxy,
            GetBoolean(ConfigVoice::KEY_EPS_FALLBACK_TRIGGER_BY_RRC_REJECT_WAIT_TIME_BOOL))
            .WillByDefault(Return(IMS_TRUE));
    ON_CALL(*pConfigurationProxy, GetInt(ConfigVoice::KEY_MO_CALL_REQUEST_TIMEOUT_MILLIS_INT))
            .WillByDefault(Return(2000));

    EXPECT_TRUE(EpsFallbackTrigger::ShouldTriggerByReasonInfo(
            objContext, CallReasonInfo(CODE_INTERNAL_RRC_REJECT, 2000)));
}

TEST_F(EpsFallbackTriggerTest, ShouldTriggerByReasonInfoReturnsFalseForOtherReasons)
{
    objCallInfo.ePeerType = PeerType::MO;
    ON_CALL(objService, IsNr).WillByDefault(Return(IMS_TRUE));

    EXPECT_FALSE(EpsFallbackTrigger::ShouldTriggerByReasonInfo(
            objContext, CallReasonInfo(CODE_RADIO_INTERNAL_ERROR)));
    EXPECT_FALSE(EpsFallbackTrigger::ShouldTriggerByReasonInfo(
            objContext, CallReasonInfo(CODE_LOCAL_NETWORK_NO_SERVICE)));
}

TEST_F(EpsFallbackTriggerTest, ShouldTriggerByWatchdogTimerReturnsFalseIfNotInNr)
{
    ON_CALL(objService, IsNr).WillByDefault(Return(IMS_FALSE));
    ON_CALL(*pConfigurationProxy, GetInt(ConfigVoice::KEY_EPS_FALLBACK_WATCHDOG_TIME_MILLIS_INT))
            .WillByDefault(Return(1000));
    EXPECT_FALSE(pEpsFbTrigger->ShouldTriggerByWatchdogTimer(objContext));
}

TEST_F(EpsFallbackTriggerTest, ShouldTriggerByWatchdogTimerReturnsFalseIfInNrAndConfigIsZeroOrBelow)
{
    ON_CALL(objService, IsNr).WillByDefault(Return(IMS_TRUE));

    ON_CALL(*pConfigurationProxy, GetInt(ConfigVoice::KEY_EPS_FALLBACK_WATCHDOG_TIME_MILLIS_INT))
            .WillByDefault(Return(0));
    EXPECT_FALSE(pEpsFbTrigger->ShouldTriggerByWatchdogTimer(objContext));

    ON_CALL(*pConfigurationProxy, GetInt(ConfigVoice::KEY_EPS_FALLBACK_WATCHDOG_TIME_MILLIS_INT))
            .WillByDefault(Return(-1));
    EXPECT_FALSE(pEpsFbTrigger->ShouldTriggerByWatchdogTimer(objContext));
}

TEST_F(EpsFallbackTriggerTest, ShouldTriggerByWatchdogTimerReturnsFalseIfEpsFbNotAvailable)
{
    ON_CALL(objService, IsNr).WillByDefault(Return(IMS_TRUE));
    MockIMtcCall objOtherCall;
    ON_CALL(objOtherCall, GetState()).WillByDefault(Return(IMtcCall::State::ESTABLISHED));
    objOtherCalls.Append(&objOtherCall);
    ON_CALL(objContext, GetOtherCalls).WillByDefault(Return(objOtherCalls));
    ON_CALL(*pConfigurationProxy, GetInt(ConfigVoice::KEY_EPS_FALLBACK_WATCHDOG_TIME_MILLIS_INT))
            .WillByDefault(Return(1000));

    EXPECT_FALSE(pEpsFbTrigger->ShouldTriggerByWatchdogTimer(objContext));
}

TEST_F(EpsFallbackTriggerTest, ShouldTriggerByWatchdogTimerReturnsTrue)
{
    ON_CALL(objService, IsNr).WillByDefault(Return(IMS_TRUE));

    ON_CALL(*pConfigurationProxy, GetInt(ConfigVoice::KEY_EPS_FALLBACK_WATCHDOG_TIME_MILLIS_INT))
            .WillByDefault(Return(1000));
    EXPECT_TRUE(pEpsFbTrigger->ShouldTriggerByWatchdogTimer(objContext));
}

TEST_F(EpsFallbackTriggerTest, ShouldTriggerByMoRequestTimeoutReturnsFalseIfNotInNr)
{
    ON_CALL(objService, IsNr).WillByDefault(Return(IMS_FALSE));
    ON_CALL(*pConfigurationProxy,
            GetInt(ConfigVoice::KEY_MO_CALL_REQUEST_TIMEOUT_FOR_EPS_FALLBACK_TRIGGER_MILLIS_INT))
            .WillByDefault(Return(1000));
    EXPECT_FALSE(pEpsFbTrigger->ShouldTriggerByMoRequestTimeout(objContext));
}

TEST_F(EpsFallbackTriggerTest, ShouldTriggerByMoRequestTimeoutReturnsFalseIfEpsFbNotAvailable)
{
    ON_CALL(objService, IsNr).WillByDefault(Return(IMS_TRUE));
    MockIMtcCall objOtherCall;
    ON_CALL(objOtherCall, GetState()).WillByDefault(Return(IMtcCall::State::ESTABLISHED));
    objOtherCalls.Append(&objOtherCall);
    ON_CALL(objContext, GetOtherCalls).WillByDefault(Return(objOtherCalls));
    ON_CALL(*pConfigurationProxy,
            GetInt(ConfigVoice::KEY_MO_CALL_REQUEST_TIMEOUT_FOR_EPS_FALLBACK_TRIGGER_MILLIS_INT))
            .WillByDefault(Return(1000));

    EXPECT_FALSE(pEpsFbTrigger->ShouldTriggerByMoRequestTimeout(objContext));
}

TEST_F(EpsFallbackTriggerTest, ShouldTriggerByMoRequestTimeoutReturnsFalseIfInNrAndConfigIsNegative)
{
    ON_CALL(objService, IsNr).WillByDefault(Return(IMS_TRUE));

    ON_CALL(*pConfigurationProxy,
            GetInt(ConfigVoice::KEY_MO_CALL_REQUEST_TIMEOUT_FOR_EPS_FALLBACK_TRIGGER_MILLIS_INT))
            .WillByDefault(Return(-1));
    EXPECT_FALSE(pEpsFbTrigger->ShouldTriggerByMoRequestTimeout(objContext));
}

TEST_F(EpsFallbackTriggerTest, ShouldTriggerByMoRequestTimeoutReturnsTrue)
{
    ON_CALL(objService, IsNr).WillByDefault(Return(IMS_TRUE));

    ON_CALL(*pConfigurationProxy,
            GetInt(ConfigVoice::KEY_MO_CALL_REQUEST_TIMEOUT_FOR_EPS_FALLBACK_TRIGGER_MILLIS_INT))
            .WillByDefault(Return(1000));
    EXPECT_TRUE(pEpsFbTrigger->ShouldTriggerByMoRequestTimeout(objContext));
}

TEST_F(EpsFallbackTriggerTest, IsEpsFbAvailableReturnsFalseIfEstablishedCallExists)
{
    MockIMtcCall objOtherCall;
    ON_CALL(objOtherCall, GetState()).WillByDefault(Return(IMtcCall::State::ESTABLISHED));
    objOtherCalls.Append(&objOtherCall);
    ON_CALL(objContext, GetOtherCalls).WillByDefault(Return(objOtherCalls));

    EXPECT_FALSE(EpsFallbackTrigger::IsEpsFbAvailable(objContext));
}

TEST_F(EpsFallbackTriggerTest, IsEpsFbAvailableReturnsFalseIfUpdatingCallExists)
{
    MockIMtcCall objOtherCall;
    ON_CALL(objOtherCall, GetState()).WillByDefault(Return(IMtcCall::State::UPDATING));
    objOtherCalls.Append(&objOtherCall);
    ON_CALL(objContext, GetOtherCalls).WillByDefault(Return(objOtherCalls));

    EXPECT_FALSE(EpsFallbackTrigger::IsEpsFbAvailable(objContext));
}

TEST_F(EpsFallbackTriggerTest, IsEpsFbAvailableReturnsTrueIfNoOtherCalls)
{
    // objOtherCalls is empty

    EXPECT_TRUE(EpsFallbackTrigger::IsEpsFbAvailable(objContext));
}

TEST_F(EpsFallbackTriggerTest, IsEpsFbAvailableReturnsTrueIfOtherCallsAreNotInBlockingStates)
{
    ON_CALL(objContext, GetOtherCalls).WillByDefault(Return(objOtherCalls));
    MockIMtcCall objOtherCall1, objOtherCall2;
    ON_CALL(objOtherCall2, GetState()).WillByDefault(Return(IMtcCall::State::TERMINATING));
    objOtherCalls.Append(&objOtherCall1);
    objOtherCalls.Append(&objOtherCall2);

    EXPECT_TRUE(EpsFallbackTrigger::IsEpsFbAvailable(objContext));
}

TEST_F(EpsFallbackTriggerTest, StartWatchdogSetsTimer)
{
    IMS_SINT32 nAnyWatchdogTime = 6000;
    ON_CALL(*pConfigurationProxy, GetInt(ConfigVoice::KEY_EPS_FALLBACK_WATCHDOG_TIME_MILLIS_INT))
            .WillByDefault(Return(nAnyWatchdogTime));

    EXPECT_CALL(objTimer, SetTimer(nAnyWatchdogTime, pEpsFbTrigger));
    pEpsFbTrigger->StartWatchdog();
}

TEST_F(EpsFallbackTriggerTest, StartWatchdogResetsExistingTimer)
{
    IMS_SINT32 nAnyWatchdogTime = 6000;
    ON_CALL(*pConfigurationProxy, GetInt(ConfigVoice::KEY_EPS_FALLBACK_WATCHDOG_TIME_MILLIS_INT))
            .WillByDefault(Return(nAnyWatchdogTime));
    pEpsFbTrigger->StartWatchdog();

    EXPECT_CALL(objTimer, KillTimer);
    EXPECT_CALL(objTimer, SetTimer(nAnyWatchdogTime, pEpsFbTrigger));
    pEpsFbTrigger->StartWatchdog();

    EXPECT_CALL(objTimer, KillTimer);  // By the destructor
}

TEST_F(EpsFallbackTriggerTest, StartWatchdogAndTimerExpiredNotTriggersEpsFallbackIfQosAndNotInNr)
{
    IMS_SINT32 nAnyWatchdogTime = 6000;
    ON_CALL(*pConfigurationProxy, GetInt(ConfigVoice::KEY_EPS_FALLBACK_WATCHDOG_TIME_MILLIS_INT))
            .WillByDefault(Return(nAnyWatchdogTime));
    pEpsFbTrigger->StartWatchdog();

    MockIMtcSession objMtcSession;
    MockISession objSession;
    ON_CALL(objContext, GetSession()).WillByDefault(Return(&objMtcSession));
    ON_CALL(objMtcSession, GetISession).WillByDefault(ReturnRef(objSession));

    MockIMtcPreconditionManager objPreconditionManager;
    ON_CALL(objContext, GetPreconditionManager).WillByDefault(ReturnRef(objPreconditionManager));

    ON_CALL(objPreconditionManager, IsDedicatedBearerAllocated(&objSession, MEDIATYPE_AUDIO))
            .WillByDefault(Return(IMS_TRUE));
    ON_CALL(objService, IsNr).WillByDefault(Return(IMS_FALSE));

    EXPECT_CALL(objImsRadioService.GetMockImsRadio(), TriggerEpsFallback(_)).Times(0);
    pEpsFbTrigger->Timer_TimerExpired(&objTimer);
}

TEST_F(EpsFallbackTriggerTest, StartWatchdogAndTimerExpiredNotTriggersEpsFallbackIfQosAndInNr)
{
    IMS_SINT32 nAnyWatchdogTime = 6000;
    ON_CALL(*pConfigurationProxy, GetInt(ConfigVoice::KEY_EPS_FALLBACK_WATCHDOG_TIME_MILLIS_INT))
            .WillByDefault(Return(nAnyWatchdogTime));
    pEpsFbTrigger->StartWatchdog();

    MockIMtcSession objMtcSession;
    MockISession objSession;
    ON_CALL(objContext, GetSession()).WillByDefault(Return(&objMtcSession));
    ON_CALL(objMtcSession, GetISession).WillByDefault(ReturnRef(objSession));

    MockIMtcPreconditionManager objPreconditionManager;
    ON_CALL(objContext, GetPreconditionManager).WillByDefault(ReturnRef(objPreconditionManager));

    ON_CALL(objPreconditionManager, IsDedicatedBearerAllocated(&objSession, MEDIATYPE_AUDIO))
            .WillByDefault(Return(IMS_TRUE));
    ON_CALL(objService, IsNr).WillByDefault(Return(IMS_TRUE));

    EXPECT_CALL(objImsRadioService.GetMockImsRadio(), TriggerEpsFallback(_)).Times(0);
    pEpsFbTrigger->Timer_TimerExpired(&objTimer);
}

TEST_F(EpsFallbackTriggerTest, StartWatchdogAndTimerExpiredNotTriggersEpsFallbackIfNoQosAndNotInNr)
{
    IMS_SINT32 nAnyWatchdogTime = 6000;
    ON_CALL(*pConfigurationProxy, GetInt(ConfigVoice::KEY_EPS_FALLBACK_WATCHDOG_TIME_MILLIS_INT))
            .WillByDefault(Return(nAnyWatchdogTime));
    pEpsFbTrigger->StartWatchdog();

    MockIMtcSession objMtcSession;
    MockISession objSession;
    ON_CALL(objContext, GetSession()).WillByDefault(Return(&objMtcSession));
    ON_CALL(objMtcSession, GetISession).WillByDefault(ReturnRef(objSession));

    MockIMtcPreconditionManager objPreconditionManager;
    ON_CALL(objContext, GetPreconditionManager).WillByDefault(ReturnRef(objPreconditionManager));

    ON_CALL(objPreconditionManager, IsDedicatedBearerAllocated(&objSession, MEDIATYPE_AUDIO))
            .WillByDefault(Return(IMS_FALSE));
    ON_CALL(objService, IsNr).WillByDefault(Return(IMS_FALSE));

    EXPECT_CALL(objImsRadioService.GetMockImsRadio(), TriggerEpsFallback(_)).Times(0);
    pEpsFbTrigger->Timer_TimerExpired(&objTimer);
}

TEST_F(EpsFallbackTriggerTest, StartWatchdogAndTimerExpiredTriggersEpsFallbackIfNoQosAndInNr)
{
    IMS_SINT32 nAnyWatchdogTime = 6000;
    ON_CALL(*pConfigurationProxy, GetInt(ConfigVoice::KEY_EPS_FALLBACK_WATCHDOG_TIME_MILLIS_INT))
            .WillByDefault(Return(nAnyWatchdogTime));
    pEpsFbTrigger->StartWatchdog();

    MockIMtcSession objMtcSession;
    MockISession objSession;
    ON_CALL(objContext, GetSession()).WillByDefault(Return(&objMtcSession));
    ON_CALL(objMtcSession, GetISession).WillByDefault(ReturnRef(objSession));

    MockIMtcPreconditionManager objPreconditionManager;
    ON_CALL(objContext, GetPreconditionManager).WillByDefault(ReturnRef(objPreconditionManager));

    ON_CALL(objPreconditionManager, IsDedicatedBearerAllocated(&objSession, MEDIATYPE_AUDIO))
            .WillByDefault(Return(IMS_FALSE));
    ON_CALL(objService, IsNr).WillByDefault(Return(IMS_TRUE));

    EXPECT_CALL(objImsRadioService.GetMockImsRadio(),
            TriggerEpsFallback(IImsRadio::EPSFB_REASON_NO_NETWORK_TRIGGER))
            .Times(1);
    pEpsFbTrigger->Timer_TimerExpired(&objTimer);
}

TEST_F(EpsFallbackTriggerTest, TimerExpiredWithoutStartDoesNotTriggerEpsFallback)
{
    EXPECT_CALL(objImsRadioService.GetMockImsRadio(), TriggerEpsFallback(_)).Times(0);
    pEpsFbTrigger->Timer_TimerExpired(&objTimer);
}

TEST_F(EpsFallbackTriggerTest, InvalidTimerExpiredDoesNotTriggerEpsFallback)
{
    EXPECT_CALL(objImsRadioService.GetMockImsRadio(), TriggerEpsFallback(_)).Times(0);
    pEpsFbTrigger->Timer_TimerExpired(IMS_NULL);

    MockITimer objDifferentTimer;
    pEpsFbTrigger->Timer_TimerExpired(&objDifferentTimer);
}

TEST_F(EpsFallbackTriggerTest, TriggerNoResponseEpsFallbackTriggersEpsFallbackWithoutReg)
{
    EXPECT_CALL(objAosConnector, NotifyEpsfbCallState(IImsAosInfo::EPSFB_CALL_START)).Times(0);
    EXPECT_CALL(objImsRadioService.GetMockImsRadio(),
            TriggerEpsFallback(IImsRadio::EPSFB_REASON_NO_NETWORK_RESPONSE));

    pEpsFbTrigger->TriggerEpsFallback(EpsFallbackReason::NO_NETWORK_RESPONSE);
    EXPECT_TRUE(pEpsFbTrigger->IsWaitingEpsFallback());
    EXPECT_FALSE(pEpsFbTrigger->IsWaitingRegistration());

    // call terminated without OnEpsFallbackCompleted() case.
    EXPECT_CALL(objAosConnector, NotifyEpsfbCallState(IImsAosInfo::EPSFB_CALL_FAILED)).Times(0);
}

TEST_F(EpsFallbackTriggerTest, TriggerNoResponseRequiringRegEpsFallbackTriggersEpsFallbackAndReg)
{
    EXPECT_CALL(objAosConnector, NotifyEpsfbCallState(IImsAosInfo::EPSFB_CALL_START));
    EXPECT_CALL(objImsRadioService.GetMockImsRadio(),
            TriggerEpsFallback(IImsRadio::EPSFB_REASON_NO_NETWORK_RESPONSE));

    pEpsFbTrigger->TriggerEpsFallback(EpsFallbackReason::NO_NETWORK_RESPONSE_REQUIRING_REG);
    EXPECT_FALSE(pEpsFbTrigger->IsWaitingEpsFallback());
    EXPECT_TRUE(pEpsFbTrigger->IsWaitingRegistration());

    // call terminated without OnEpsFallbackCompleted() case.
    EXPECT_CALL(objAosConnector, NotifyEpsfbCallState(IImsAosInfo::EPSFB_CALL_FAILED));
}

TEST_F(EpsFallbackTriggerTest, TriggerRadioCheckBlockEpsFallbackTriggersEpsFallbackAndReg)
{
    EXPECT_CALL(objAosConnector, NotifyEpsfbCallState(IImsAosInfo::EPSFB_CALL_START));
    EXPECT_CALL(objImsRadioService.GetMockImsRadio(),
            TriggerEpsFallback(IImsRadio::EPSFB_REASON_NO_NETWORK_RESPONSE));

    pEpsFbTrigger->TriggerEpsFallback(EpsFallbackReason::RADIO_CHECK_BLOCK);
    EXPECT_FALSE(pEpsFbTrigger->IsWaitingEpsFallback());
    EXPECT_TRUE(pEpsFbTrigger->IsWaitingRegistration());
}

TEST_F(EpsFallbackTriggerTest, TriggerNoTriggerEpsFallbackTriggersEpsFallback)
{
    EXPECT_CALL(objAosConnector, NotifyEpsfbCallState(IImsAosInfo::EPSFB_CALL_START)).Times(0);
    EXPECT_CALL(objImsRadioService.GetMockImsRadio(),
            TriggerEpsFallback(IImsRadio::EPSFB_REASON_NO_NETWORK_TRIGGER));

    pEpsFbTrigger->TriggerEpsFallback(EpsFallbackReason::NO_NETWORK_TRIGGER);
    EXPECT_FALSE(pEpsFbTrigger->IsWaitingEpsFallback());
    EXPECT_FALSE(pEpsFbTrigger->IsWaitingRegistration());

    // call terminated without OnEpsFallbackCompleted() case.
    EXPECT_CALL(objAosConnector, NotifyEpsfbCallState(IImsAosInfo::EPSFB_CALL_FAILED)).Times(0);
}

TEST_F(EpsFallbackTriggerTest, TriggerFailureResponseEpsFallbackTriggersEpsFallback)
{
    EXPECT_CALL(objAosConnector, NotifyEpsfbCallState(IImsAosInfo::EPSFB_CALL_START)).Times(0);
    EXPECT_CALL(objImsRadioService.GetMockImsRadio(),
            TriggerEpsFallback(IImsRadio::EPSFB_REASON_NO_NETWORK_TRIGGER));

    pEpsFbTrigger->TriggerEpsFallback(EpsFallbackReason::FAILURE_RESPONSE);
    EXPECT_TRUE(pEpsFbTrigger->IsWaitingEpsFallback());
    EXPECT_FALSE(pEpsFbTrigger->IsWaitingRegistration());

    // call terminated without OnEpsFallbackCompleted() case.
    EXPECT_CALL(objAosConnector, NotifyEpsfbCallState(IImsAosInfo::EPSFB_CALL_FAILED)).Times(0);
}

TEST_F(EpsFallbackTriggerTest, OnEpsFallbackCompletedAfterTriggerEpsfbWithRegStopsTimer)
{
    pEpsFbTrigger->TriggerEpsFallback(EpsFallbackReason::NO_NETWORK_RESPONSE_REQUIRING_REG);

    EXPECT_CALL(objTimer, KillTimer);

    pEpsFbTrigger->OnEpsFallbackCompleted();

    EXPECT_FALSE(pEpsFbTrigger->IsWaitingRegistration());
}

TEST_F(EpsFallbackTriggerTest,
        OnEpsFallbackCompletedAfterTriggerByNoNetworkTriggerEpsfbDoesNotStopTimer)
{
    pEpsFbTrigger->TriggerEpsFallback(EpsFallbackReason::NO_NETWORK_TRIGGER);

    EXPECT_CALL(objTimer, KillTimer).Times(0);

    pEpsFbTrigger->OnEpsFallbackCompleted();

    EXPECT_FALSE(pEpsFbTrigger->IsWaitingRegistration());
}

TEST_F(EpsFallbackTriggerTest, TriggerNoResponseEpsFallbackAndTimerExpiredTerminatesCall)
{
    pEpsFbTrigger->TriggerEpsFallback(EpsFallbackReason::NO_NETWORK_RESPONSE);

    MockIMtcCall objCall;
    ON_CALL(objContext, GetCall).WillByDefault(ReturnRef(objCall));

    EXPECT_CALL(objCall,
            Terminate(CallReasonInfo(
                    CODE_LOCAL_CALL_CS_RETRY_REQUIRED, EXTRA_CODE_CALL_RETRY_SILENT_REDIAL)));

    EXPECT_CALL(objAosConnector, NotifyEpsfbCallState(IImsAosInfo::EPSFB_CALL_FAILED)).Times(0);

    pEpsFbTrigger->Timer_TimerExpired(&objTimer);
}

TEST_F(EpsFallbackTriggerTest, TriggerEpsFallbackWithNoNetworkResponseSetsTimer)
{
    EXPECT_CALL(objTimer, SetTimer(20000, pEpsFbTrigger));
    pEpsFbTrigger->TriggerEpsFallback(EpsFallbackReason::NO_NETWORK_RESPONSE);
    EXPECT_TRUE(pEpsFbTrigger->IsWaitingEpsFallback());

    pEpsFbTrigger->OnEpsFallbackCompleted();
    EXPECT_FALSE(pEpsFbTrigger->IsWaitingEpsFallback());
}

TEST_F(EpsFallbackTriggerTest, TriggerEpsFallbackWithNoNetworkTriggerDoesNotSetTimer)
{
    EXPECT_CALL(objTimer, SetTimer(_, pEpsFbTrigger)).Times(0);
    pEpsFbTrigger->TriggerEpsFallback(EpsFallbackReason::NO_NETWORK_TRIGGER);
    EXPECT_FALSE(pEpsFbTrigger->IsWaitingEpsFallback());

    pEpsFbTrigger->OnEpsFallbackCompleted();
    EXPECT_FALSE(pEpsFbTrigger->IsWaitingEpsFallback());
}

TEST_F(EpsFallbackTriggerTest, TriggerEpsFallbackDoesNothingIfAlreadyTriggeredByNoNetworkResponse)
{
    EXPECT_CALL(objImsRadioService.GetMockImsRadio(), TriggerEpsFallback(_)).Times(1);

    pEpsFbTrigger->TriggerEpsFallback(EpsFallbackReason::NO_NETWORK_RESPONSE);
    pEpsFbTrigger->TriggerEpsFallback(EpsFallbackReason::NO_NETWORK_RESPONSE);
}

TEST_F(EpsFallbackTriggerTest, TriggerEpsFallbackDoesNothingIfAlreadyTriggeredByNoNetworkTrigger)
{
    EXPECT_CALL(objImsRadioService.GetMockImsRadio(), TriggerEpsFallback(_)).Times(1);

    pEpsFbTrigger->TriggerEpsFallback(EpsFallbackReason::NO_NETWORK_TRIGGER);
    pEpsFbTrigger->TriggerEpsFallback(EpsFallbackReason::NO_NETWORK_TRIGGER);
}
