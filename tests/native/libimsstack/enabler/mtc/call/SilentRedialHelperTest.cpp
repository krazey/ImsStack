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
#include "CarrierConfig.h"
#include "ImsAosParameter.h"
#include "MockIMtcCallController.h"
#include "MockIMessage.h"
#include "MockIMtcService.h"
#include "MockISession.h"
#include "MockITimer.h"
#include "MtcDef.h"
#include "PlatformContext.h"
#include "TestTimerService.h"
#include "SipStatusCode.h"
#include "call/MockIMtcCall.h"
#include "call/MockIMtcCallContext.h"
#include "call/MockIMtcCallManager.h"
#include "call/MockIMtcSession.h"
#include "call/ParticipantInfo.h"
#include "call/SilentRedialHelper.h"
#include "configuration/MockMtcConfigurationProxy.h"
#include "configuration/MtcConfigurationProxy.h"
#include "helper/MockICallStateProxy.h"
#include "helper/MockIMtcAosConnector.h"
#include "helper/MockMtcTimerWrapper.h"
#include "helper/MtcSupplementaryService.h"
#include "media/MockIMtcMediaManager.h"
#include "precondition/MockIMtcPreconditionManager.h"
#include "utility/MockIMessageUtils.h"
#include <gtest/gtest.h>
#include <vector>

using ::testing::_;
using ::testing::Ref;
using ::testing::Return;
using ::testing::ReturnRef;

LOCAL const CallKey ANY_CALL_KEY = 100;
LOCAL const IMS_SINT32 MAX_DURATION = 10000;
LOCAL const IMS_SINT32 INTERVAL_OVER_MAX = 11000;

class SilentRedialHelperTest : public ::testing::Test
{
public:
    inline SilentRedialHelperTest() :
            pRedialHelper(IMS_NULL),
            pTimerService(new TestTimerService()),
            objTimer(pTimerService->GetMockTimer()),
            pConfigurationProxy(IMS_NULL),
            pSupplementaryService(IMS_NULL)
    {
        PlatformContext::GetInstance()->SetService(PlatformContext::SERVICE_TIMER, pTimerService);
    }
    inline virtual ~SilentRedialHelperTest()
    {
        delete pRedialHelper;
        delete pTimerService;
        PlatformContext::GetInstance()->SetService(PlatformContext::SERVICE_TIMER, IMS_NULL);
    }

    SilentRedialHelper* pRedialHelper;
    MockIMtcCallContext objContext;
    MockICallStateProxy objCallStateProxy;
    MockIMtcCallController objController;
    MockIMtcCall objMtcCall;
    MockIMtcCallManager objMtcCallManager;
    MockIMessageUtils objMessageUtils;
    MockIMtcPreconditionManager objPreconditionManager;
    MockIMtcMediaManager objMediaManager;
    MockIMtcSession objMtcSession;
    MockISession objSession;
    MockIMessage objPreviousResponse;
    TestTimerService* pTimerService;
    MockITimer& objTimer;
    MockMtcConfigurationProxy* pConfigurationProxy;
    MtcSupplementaryService* pSupplementaryService;
    MediaInfo objMediaInfo;
    MockMtcTimerWrapper objTimerWrapper;

protected:
    virtual void SetUp() override
    {
        ON_CALL(objContext, GetCallKey).WillByDefault(Return(ANY_CALL_KEY));
        ON_CALL(objContext, GetSession()).WillByDefault(Return(&objMtcSession));
        ON_CALL(objContext, GetPreconditionManager)
                .WillByDefault(ReturnRef(objPreconditionManager));
        ON_CALL(objContext, GetMediaManager).WillByDefault(ReturnRef(objMediaManager));
        ON_CALL(objMtcSession, GetISession).WillByDefault(ReturnRef(objSession));
        ON_CALL(objMediaManager, GetMediaInfo(&objSession)).WillByDefault(ReturnRef(objMediaInfo));
        ON_CALL(objContext, GetCallStateProxy).WillByDefault(ReturnRef(objCallStateProxy));
        ON_CALL(objContext, GetCallController).WillByDefault(ReturnRef(objController));
        ON_CALL(objContext, GetCallManager).WillByDefault(ReturnRef(objMtcCallManager));
        ON_CALL(objContext, GetMessageUtils).WillByDefault(ReturnRef(objMessageUtils));
        ON_CALL(objMessageUtils, GetPreviousResponse(_, _, _))
                .WillByDefault(Return(&objPreviousResponse));
        ON_CALL(objMessageUtils, GetCauseFromReasonHeader).WillByDefault(Return(-1));
        ON_CALL(objMtcCallManager, GetCallByCallKey(ANY_CALL_KEY))
                .WillByDefault(Return(&objMtcCall));

        pConfigurationProxy = new MockMtcConfigurationProxy();
        ON_CALL(objContext, GetConfigurationProxy).WillByDefault(ReturnRef(*pConfigurationProxy));
        pSupplementaryService = new MtcSupplementaryService(objContext, *pConfigurationProxy);
        ON_CALL(objContext, GetSupplementaryService)
                .WillByDefault(ReturnRef(*pSupplementaryService));
        ON_CALL(objContext, GetTimer()).WillByDefault(ReturnRef(objTimerWrapper));
        ON_CALL(*pConfigurationProxy, GetInt(ConfigVoice::KEY_SILENT_REDIAL_MAX_RETRY_COUNT_INT))
                .WillByDefault(Return(1));
        ON_CALL(*pConfigurationProxy,
                GetInt(ConfigVoice::KEY_SILENT_REDIAL_MAX_DURATION_MILLIS_INT))
                .WillByDefault(Return(MAX_DURATION));
        ON_CALL(*pConfigurationProxy,
                GetInt(ConfigVoice::KEY_SILENT_REDIAL_ULTIMATE_FAILURE_ACTION_INT))
                .WillByDefault(Return(ConfigVoice::SILENT_REDIAL_FAILURE_ACTION_TERMINATE));
    }

    virtual void TearDown() override
    {
        delete pConfigurationProxy;
        delete pSupplementaryService;
    }
};

TEST_F(SilentRedialHelperTest, CreateHelperWithRetryAfterType)
{
    EXPECT_CALL(
            *pConfigurationProxy, GetInt(ConfigVoice::KEY_SILENT_REDIAL_MAX_DURATION_MILLIS_INT))
            .WillOnce(Return(20000));
    EXPECT_CALL(*pConfigurationProxy, GetInt(ConfigVoice::KEY_SILENT_REDIAL_INTERVAL_MILLIS_INT))
            .Times(0);
    EXPECT_CALL(*pConfigurationProxy, GetInt(ConfigVoice::KEY_SILENT_REDIAL_MAX_RETRY_COUNT_INT))
            .WillOnce(Return(3));
    const CallReasonInfo objReason(CODE_INTERNAL_REDIAL, EXTRA_CODE_REDIAL_BY_RETRY_AFTER, "1000");
    pRedialHelper = new SilentRedialHelper(objContext, objReason);
    EXPECT_EQ(pRedialHelper->GetType(), EXTRA_CODE_REDIAL_BY_RETRY_AFTER);
}

TEST_F(SilentRedialHelperTest, CreateHelperWithRequestTimeoutType)
{
    EXPECT_CALL(
            *pConfigurationProxy, GetInt(ConfigVoice::KEY_SILENT_REDIAL_MAX_DURATION_MILLIS_INT))
            .WillOnce(Return(20000));
    EXPECT_CALL(*pConfigurationProxy, GetInt(ConfigVoice::KEY_SILENT_REDIAL_INTERVAL_MILLIS_INT))
            .WillOnce(Return(2000));
    EXPECT_CALL(*pConfigurationProxy, GetInt(ConfigVoice::KEY_SILENT_REDIAL_MAX_RETRY_COUNT_INT))
            .WillOnce(Return(3));

    const CallReasonInfo objReason(CODE_INTERNAL_REDIAL, EXTRA_CODE_REDIAL_BY_REQUEST_TIMEOUT);
    pRedialHelper = new SilentRedialHelper(objContext, objReason);
    EXPECT_EQ(pRedialHelper->GetType(), EXTRA_CODE_REDIAL_BY_REQUEST_TIMEOUT);
}

TEST_F(SilentRedialHelperTest, CreateHelperWithErrorResponseType)
{
    EXPECT_CALL(
            *pConfigurationProxy, GetInt(ConfigVoice::KEY_SILENT_REDIAL_MAX_DURATION_MILLIS_INT))
            .WillOnce(Return(20000));
    EXPECT_CALL(*pConfigurationProxy, GetInt(ConfigVoice::KEY_SILENT_REDIAL_INTERVAL_MILLIS_INT))
            .WillOnce(Return(2000));
    EXPECT_CALL(*pConfigurationProxy, GetInt(ConfigVoice::KEY_SILENT_REDIAL_MAX_RETRY_COUNT_INT))
            .WillOnce(Return(3));

    const CallReasonInfo objReason(CODE_INTERNAL_REDIAL, EXTRA_CODE_REDIAL_BY_ERROR_RESPONSE);
    pRedialHelper = new SilentRedialHelper(objContext, objReason);
    EXPECT_EQ(pRedialHelper->GetType(), EXTRA_CODE_REDIAL_BY_ERROR_RESPONSE);
}

TEST_F(SilentRedialHelperTest, CreateHelperWithRedirectionType)
{
    EXPECT_CALL(
            *pConfigurationProxy, GetInt(ConfigVoice::KEY_SILENT_REDIAL_MAX_DURATION_MILLIS_INT))
            .Times(0);
    EXPECT_CALL(*pConfigurationProxy, GetInt(ConfigVoice::KEY_SILENT_REDIAL_INTERVAL_MILLIS_INT))
            .Times(0);
    EXPECT_CALL(*pConfigurationProxy, GetInt(ConfigVoice::KEY_SILENT_REDIAL_MAX_RETRY_COUNT_INT))
            .Times(0);

    const CallReasonInfo objReason(CODE_INTERNAL_REDIAL, EXTRA_CODE_REDIAL_FOR_REDIRECTION);
    pRedialHelper = new SilentRedialHelper(objContext, objReason);
    EXPECT_EQ(pRedialHelper->GetType(), EXTRA_CODE_REDIAL_FOR_REDIRECTION);
}

TEST_F(SilentRedialHelperTest, CreateHelperWithSdpChangeType)
{
    EXPECT_CALL(
            *pConfigurationProxy, GetInt(ConfigVoice::KEY_SILENT_REDIAL_MAX_DURATION_MILLIS_INT))
            .Times(0);
    EXPECT_CALL(*pConfigurationProxy, GetInt(ConfigVoice::KEY_SILENT_REDIAL_INTERVAL_MILLIS_INT))
            .Times(0);
    EXPECT_CALL(*pConfigurationProxy, GetInt(ConfigVoice::KEY_SILENT_REDIAL_MAX_RETRY_COUNT_INT))
            .Times(0);

    const CallReasonInfo objReason(CODE_INTERNAL_REDIAL, EXTRA_CODE_REDIAL_FOR_SDP_CHANGE);
    pRedialHelper = new SilentRedialHelper(objContext, objReason);
    EXPECT_EQ(pRedialHelper->GetType(), EXTRA_CODE_REDIAL_FOR_SDP_CHANGE);
}

TEST_F(SilentRedialHelperTest, CreateHelperWithByEpsfbType)
{
    EXPECT_CALL(
            *pConfigurationProxy, GetInt(ConfigVoice::KEY_SILENT_REDIAL_MAX_DURATION_MILLIS_INT))
            .Times(0);
    EXPECT_CALL(*pConfigurationProxy, GetInt(ConfigVoice::KEY_SILENT_REDIAL_INTERVAL_MILLIS_INT))
            .Times(0);
    EXPECT_CALL(*pConfigurationProxy, GetInt(ConfigVoice::KEY_SILENT_REDIAL_MAX_RETRY_COUNT_INT))
            .Times(0);

    const CallReasonInfo objReason(CODE_INTERNAL_REDIAL, EXTRA_CODE_REDIAL_BY_EPS_FALLBACK);
    pRedialHelper = new SilentRedialHelper(objContext, objReason);
    EXPECT_EQ(pRedialHelper->GetType(), EXTRA_CODE_REDIAL_BY_EPS_FALLBACK);
}

TEST_F(SilentRedialHelperTest, CreateHelperWithByEpsfbAfterRegType)
{
    EXPECT_CALL(
            *pConfigurationProxy, GetInt(ConfigVoice::KEY_SILENT_REDIAL_MAX_DURATION_MILLIS_INT))
            .Times(0);
    EXPECT_CALL(*pConfigurationProxy, GetInt(ConfigVoice::KEY_SILENT_REDIAL_INTERVAL_MILLIS_INT))
            .Times(0);
    EXPECT_CALL(*pConfigurationProxy, GetInt(ConfigVoice::KEY_SILENT_REDIAL_MAX_RETRY_COUNT_INT))
            .Times(0);

    const CallReasonInfo objReason(
            CODE_INTERNAL_REDIAL, EXTRA_CODE_REDIAL_BY_EPS_FALLBACK_WITH_REG);
    pRedialHelper = new SilentRedialHelper(objContext, objReason);
    EXPECT_EQ(pRedialHelper->GetType(), EXTRA_CODE_REDIAL_BY_EPS_FALLBACK_WITH_REG);
}

TEST_F(SilentRedialHelperTest, CreateHelperWithRedialNormalType)
{
    EXPECT_CALL(
            *pConfigurationProxy, GetInt(ConfigVoice::KEY_SILENT_REDIAL_MAX_DURATION_MILLIS_INT))
            .Times(0);
    EXPECT_CALL(*pConfigurationProxy, GetInt(ConfigVoice::KEY_SILENT_REDIAL_INTERVAL_MILLIS_INT))
            .Times(0);
    EXPECT_CALL(*pConfigurationProxy, GetInt(ConfigVoice::KEY_SILENT_REDIAL_MAX_RETRY_COUNT_INT))
            .Times(0);

    const CallReasonInfo objReason(CODE_INTERNAL_REDIAL, EXTRA_CODE_REDIAL_WITH_NEXT_PCSCF);
    pRedialHelper = new SilentRedialHelper(objContext, objReason);
    EXPECT_EQ(pRedialHelper->GetType(), EXTRA_CODE_REDIAL_WITH_NEXT_PCSCF);
}

TEST_F(SilentRedialHelperTest, CreateHelperWithRedialEmergencyType)
{
    EXPECT_CALL(
            *pConfigurationProxy, GetInt(ConfigVoice::KEY_SILENT_REDIAL_MAX_DURATION_MILLIS_INT))
            .Times(0);
    EXPECT_CALL(*pConfigurationProxy, GetInt(ConfigVoice::KEY_SILENT_REDIAL_INTERVAL_MILLIS_INT))
            .Times(0);
    EXPECT_CALL(*pConfigurationProxy, GetInt(ConfigVoice::KEY_SILENT_REDIAL_MAX_RETRY_COUNT_INT))
            .Times(0);

    const CallReasonInfo objReason(
            CODE_INTERNAL_REDIAL, EXTRA_CODE_REDIAL_EMERGENCY_WITH_NEXT_PCSCF);
    pRedialHelper = new SilentRedialHelper(objContext, objReason);
    EXPECT_EQ(pRedialHelper->GetType(), EXTRA_CODE_REDIAL_EMERGENCY_WITH_NEXT_PCSCF);
}

TEST_F(SilentRedialHelperTest, CreateHelperWithRedialByRttEmergencyRejection)
{
    EXPECT_CALL(
            *pConfigurationProxy, GetInt(ConfigVoice::KEY_SILENT_REDIAL_MAX_DURATION_MILLIS_INT))
            .Times(0);
    EXPECT_CALL(*pConfigurationProxy, GetInt(ConfigVoice::KEY_SILENT_REDIAL_INTERVAL_MILLIS_INT))
            .Times(0);
    EXPECT_CALL(*pConfigurationProxy, GetInt(ConfigVoice::KEY_SILENT_REDIAL_MAX_RETRY_COUNT_INT))
            .Times(0);

    const CallReasonInfo objReason(
            CODE_INTERNAL_REDIAL, EXTRA_CODE_REDIAL_BY_RTT_EMERGENCY_REJECTION);
    pRedialHelper = new SilentRedialHelper(objContext, objReason);
    EXPECT_EQ(pRedialHelper->GetType(), EXTRA_CODE_REDIAL_BY_RTT_EMERGENCY_REJECTION);
}

TEST_F(SilentRedialHelperTest, IsSameRedialTypeReturnsComparisonResult)
{
    const CallReasonInfo objAnyReason(CODE_INTERNAL_REDIAL, EXTRA_CODE_REDIAL_FOR_REDIRECTION);
    pRedialHelper = new SilentRedialHelper(objContext, objAnyReason);

    EXPECT_TRUE(pRedialHelper->IsSameRedialType(objAnyReason));
    const CallReasonInfo objDifferentReason(CODE_INTERNAL_REDIAL, EXTRA_CODE_REDIAL_FOR_SDP_CHANGE);
    EXPECT_FALSE(pRedialHelper->IsSameRedialType(objDifferentReason));
}

TEST_F(SilentRedialHelperTest, CallStateChangedToTerminatingInvokesReleaseRedialHelper)
{
    const CallReasonInfo objAnyReason(CODE_INTERNAL_REDIAL, EXTRA_CODE_REDIAL_FOR_REDIRECTION);
    pRedialHelper = new SilentRedialHelper(objContext, objAnyReason);

    EXPECT_CALL(objController, ReleaseRedialHelper);

    pRedialHelper->OnCallStateChanged(
            ANY_CALL_KEY, IMtcCall::State::ESTABLISHED, CallType::VOIP, IMS_FALSE, 0);
}

TEST_F(SilentRedialHelperTest, CallStateChangedToEstablishedInvokesReleaseRedialHelper)
{
    const CallReasonInfo objAnyReason(CODE_INTERNAL_REDIAL, EXTRA_CODE_REDIAL_FOR_REDIRECTION);
    pRedialHelper = new SilentRedialHelper(objContext, objAnyReason);

    EXPECT_CALL(objController, ReleaseRedialHelper);

    pRedialHelper->OnCallStateChanged(
            ANY_CALL_KEY, IMtcCall::State::TERMINATING, CallType::VOIP, IMS_FALSE, 0);
}

TEST_F(SilentRedialHelperTest, OnTotalCallStateChangedDoesNothing)
{
    pRedialHelper = new SilentRedialHelper(
            objContext, CallReasonInfo(CODE_INTERNAL_REDIAL, EXTRA_CODE_REDIAL_FOR_REDIRECTION));

    std::vector<IMtcCall::State> objCallStates{IMtcCall::State::IDLE, IMtcCall::State::OUTGOING,
            IMtcCall::State::INCOMING, IMtcCall::State::ALERTING, IMtcCall::State::ESTABLISHED,
            IMtcCall::State::UPDATING, IMtcCall::State::TERMINATING};

    for (IMtcCall::State eCallState : objCallStates)
    {
        pRedialHelper->OnTotalCallStateChanged(eCallState);
    }
}

TEST_F(SilentRedialHelperTest, IsSynchronousCallRequiredReturnsTrue)
{
    const CallReasonInfo objAnyReason(CODE_INTERNAL_REDIAL, EXTRA_CODE_REDIAL_FOR_REDIRECTION);
    pRedialHelper = new SilentRedialHelper(objContext, objAnyReason);
    EXPECT_TRUE(pRedialHelper->IsSynchronousCallRequired());
}

TEST_F(SilentRedialHelperTest, RedialReleasesSessionResources)
{
    EXPECT_CALL(objPreconditionManager, InitializeMobileRatInformation());
    EXPECT_CALL(objMediaManager, DestroyMediaSession());
    EXPECT_CALL(objContext, RemoveAllSessions());
    EXPECT_CALL(objTimerWrapper, StopAll());

    const CallReasonInfo objAnyReason(CODE_INTERNAL_REDIAL, EXTRA_CODE_REDIAL_FOR_REDIRECTION);
    pRedialHelper = new SilentRedialHelper(objContext, objAnyReason);
    EXPECT_EQ(CallReasonInfo(CODE_NONE), pRedialHelper->Redial(ISilentRedialHelper::INTERVAL_BY_TYPE));
}

TEST_F(SilentRedialHelperTest, RedialReleaseSessionResourceIfRedialEmergency)
{
    EXPECT_CALL(objPreconditionManager, InitializeMobileRatInformation());
    EXPECT_CALL(objMediaManager, DestroyMediaSession());
    EXPECT_CALL(objContext, RemoveAllSessions());
    EXPECT_CALL(objTimerWrapper, StopAll).Times(0);

    const CallReasonInfo objAnyReason(
            CODE_INTERNAL_REDIAL, EXTRA_CODE_REDIAL_EMERGENCY_WITH_NEXT_PCSCF);
    pRedialHelper = new SilentRedialHelper(objContext, objAnyReason);
    EXPECT_EQ(CallReasonInfo(CODE_NONE), pRedialHelper->Redial(ISilentRedialHelper::INTERVAL_BY_TYPE));
}

TEST_F(SilentRedialHelperTest, RedialStartsTimer)
{
    const CallReasonInfo objAnyReason(CODE_INTERNAL_REDIAL, EXTRA_CODE_REDIAL_FOR_REDIRECTION);
    IMS_SINT32 nRedirectionInterval = 0;
    pRedialHelper = new SilentRedialHelper(objContext, objAnyReason);
    EXPECT_CALL(objTimer, SetTimer(nRedirectionInterval, pRedialHelper));
    EXPECT_EQ(CallReasonInfo(CODE_NONE), pRedialHelper->Redial(ISilentRedialHelper::INTERVAL_BY_TYPE));
}

TEST_F(SilentRedialHelperTest, RedialReturnsFailureIfCountExceedsMaxRedialCount)
{
    ON_CALL(objPreviousResponse, GetStatusCode).WillByDefault(Return(SipStatusCode::SC_488));
    const CallReasonInfo objAnyReason(CODE_INTERNAL_REDIAL, EXTRA_CODE_REDIAL_FOR_REDIRECTION);
    pRedialHelper = new SilentRedialHelper(objContext, objAnyReason);
    pRedialHelper->Redial(ISilentRedialHelper::INTERVAL_BY_TYPE);
    pRedialHelper->Redial(ISilentRedialHelper::INTERVAL_BY_TYPE);
    EXPECT_EQ(CallReasonInfo(CODE_NONE), pRedialHelper->Redial(ISilentRedialHelper::INTERVAL_BY_TYPE));
    EXPECT_EQ(CallReasonInfo(CODE_SIP_NOT_ACCEPTABLE, SipStatusCode::SC_488),
            pRedialHelper->Redial(ISilentRedialHelper::INTERVAL_BY_TYPE));
}

TEST_F(SilentRedialHelperTest, RedialReturnsFailureIfMaxDurationExpires)
{
    ON_CALL(objPreviousResponse, GetStatusCode).WillByDefault(Return(SipStatusCode::SC_488));
    const CallReasonInfo objAnyReason(CODE_INTERNAL_REDIAL, EXTRA_CODE_REDIAL_BY_REQUEST_TIMEOUT);
    ON_CALL(*pConfigurationProxy, GetInt(ConfigVoice::KEY_SILENT_REDIAL_INTERVAL_MILLIS_INT))
            .WillByDefault(Return(INTERVAL_OVER_MAX));
    ON_CALL(*pConfigurationProxy, GetInt(ConfigVoice::KEY_SILENT_REDIAL_INTERVAL_MILLIS_INT))
            .WillByDefault(Return(INTERVAL_OVER_MAX));
    pRedialHelper = new SilentRedialHelper(objContext, objAnyReason);
    EXPECT_EQ(CallReasonInfo(CODE_SIP_NOT_ACCEPTABLE, SipStatusCode::SC_488),
            pRedialHelper->Redial(ISilentRedialHelper::INTERVAL_BY_TYPE));
}

TEST_F(SilentRedialHelperTest, RedialIfSessionIsNull)
{
    IMS_UINT32 nMaxCount = 0;
    ON_CALL(*pConfigurationProxy, GetInt(ConfigVoice::KEY_SILENT_REDIAL_MAX_DURATION_MILLIS_INT))
            .WillByDefault(Return(MAX_DURATION));
    ON_CALL(*pConfigurationProxy, GetInt(ConfigVoice::KEY_SILENT_REDIAL_MAX_RETRY_COUNT_INT))
            .WillByDefault(Return(nMaxCount));
    const CallReasonInfo objAnyReason(CODE_INTERNAL_REDIAL, EXTRA_CODE_REDIAL_BY_RETRY_AFTER);
    pRedialHelper = new SilentRedialHelper(objContext, objAnyReason);

    EXPECT_CALL(objContext, GetSession()).WillOnce(Return(IMS_NULL));
    EXPECT_EQ(CallReasonInfo(CODE_NONE),
            pRedialHelper->Redial(ISilentRedialHelper::INTERVAL_BY_TYPE));
}

TEST_F(SilentRedialHelperTest, HandleFailureTriggersInitialRegistrationByConfig)
{
    ON_CALL(*pConfigurationProxy,
            GetInt(ConfigVoice::KEY_SILENT_REDIAL_ULTIMATE_FAILURE_ACTION_INT))
            .WillByDefault(Return(ConfigVoice::SILENT_REDIAL_FAILURE_ACTION_REGISTRATION));

    MockIMtcService objService;
    ON_CALL(objContext, GetService).WillByDefault(ReturnRef(objService));

    MockIMtcAosConnector objAosConnector;
    ON_CALL(objService, GetAosConnector).WillByDefault(Return(&objAosConnector));
    EXPECT_CALL(objAosConnector, Control(ImsAosControl::REGISTER_REINITIATE));

    const CallReasonInfo objAnyReason(CODE_INTERNAL_REDIAL, EXTRA_CODE_REDIAL_BY_REQUEST_TIMEOUT);
    ON_CALL(*pConfigurationProxy, GetInt(ConfigVoice::KEY_SILENT_REDIAL_INTERVAL_MILLIS_INT))
            .WillByDefault(Return(INTERVAL_OVER_MAX));
    pRedialHelper = new SilentRedialHelper(objContext, objAnyReason);

    ON_CALL(objPreviousResponse, GetStatusCode).WillByDefault(Return(SipStatusCode::SC_488));
    EXPECT_EQ(CallReasonInfo(CODE_SIP_NOT_ACCEPTABLE, SipStatusCode::SC_488),
            pRedialHelper->Redial(ISilentRedialHelper::INTERVAL_BY_TYPE));
}

TEST_F(SilentRedialHelperTest, HandleFailureTriggersCsfbByConfig)
{
    ON_CALL(*pConfigurationProxy,
            GetInt(ConfigVoice::KEY_SILENT_REDIAL_ULTIMATE_FAILURE_ACTION_INT))
            .WillByDefault(Return(ConfigVoice::SILENT_REDIAL_FAILURE_ACTION_CSFB));
    EXPECT_CALL(objContext, IsCsfbAvailable).WillOnce(Return(IMS_TRUE)).WillOnce(Return(IMS_FALSE));

    const CallReasonInfo objAnyReason(CODE_INTERNAL_REDIAL, EXTRA_CODE_REDIAL_BY_REQUEST_TIMEOUT);
    ON_CALL(*pConfigurationProxy, GetInt(ConfigVoice::KEY_SILENT_REDIAL_INTERVAL_MILLIS_INT))
            .WillByDefault(Return(INTERVAL_OVER_MAX));
    pRedialHelper = new SilentRedialHelper(objContext, objAnyReason);

    EXPECT_EQ(
            CallReasonInfo(CODE_LOCAL_CALL_CS_RETRY_REQUIRED, EXTRA_CODE_CALL_RETRY_SILENT_REDIAL),
            pRedialHelper->Redial(ISilentRedialHelper::INTERVAL_BY_TYPE));

    ON_CALL(objPreviousResponse, GetStatusCode).WillByDefault(Return(SipStatusCode::SC_488));
    EXPECT_EQ(CallReasonInfo(CODE_SIP_NOT_ACCEPTABLE, SipStatusCode::SC_488),
            pRedialHelper->Redial(ISilentRedialHelper::INTERVAL_BY_TYPE));
}

TEST_F(SilentRedialHelperTest, TimerExpiresDoesNothingIfTimerIsNull)
{
    const CallReasonInfo objAnyReason(CODE_INTERNAL_REDIAL, EXTRA_CODE_REDIAL_FOR_REDIRECTION);
    pRedialHelper = new SilentRedialHelper(objContext, objAnyReason);
    MockITimer objDifferentTimer;
    pRedialHelper->Timer_TimerExpired(&objDifferentTimer);
    // nothing to check
}

TEST_F(SilentRedialHelperTest, TimerExpiresInvokesReStartWithRetryAfterType)
{
    IMS_SINT32 nRetryAfterInSeconds = 1;
    AString strRetryAfterInMillis;
    strRetryAfterInMillis.SetNumber(nRetryAfterInSeconds * 1000);
    const CallReasonInfo objAnyReason(
            CODE_INTERNAL_REDIAL, EXTRA_CODE_REDIAL_BY_RETRY_AFTER, strRetryAfterInMillis);
    pRedialHelper = new SilentRedialHelper(objContext, objAnyReason);
    pRedialHelper->Redial(nRetryAfterInSeconds * 1000);

    CallInfo objCallInfo;
    ON_CALL(objContext, GetCallInfo).WillByDefault(ReturnRef(objCallInfo));  // initial type is VOIP
    ParticipantInfo objParticipantInfo(objContext);
    ON_CALL(objContext, GetParticipantInfo)
            .WillByDefault(ReturnRef(objParticipantInfo));  // empty remote target.

    AString strEmptyNumber;
    EXPECT_CALL(objMtcCall, Start(CallType::VOIP, strEmptyNumber, _, _));

    pRedialHelper->Timer_TimerExpired(&objTimer);
}

TEST_F(SilentRedialHelperTest, TimerExpiresInvokesReStartWithRequestTimeoutType)
{
    IMS_SINT32 nInterval = 2000;
    IMS_UINT32 nMaxCount = 2;
    ON_CALL(*pConfigurationProxy, GetInt(ConfigVoice::KEY_SILENT_REDIAL_INTERVAL_MILLIS_INT))
            .WillByDefault(Return(nInterval));
    ON_CALL(*pConfigurationProxy, GetInt(ConfigVoice::KEY_SILENT_REDIAL_MAX_RETRY_COUNT_INT))
            .WillByDefault(Return(nMaxCount));

    const CallReasonInfo objAnyReason(CODE_INTERNAL_REDIAL, EXTRA_CODE_REDIAL_BY_REQUEST_TIMEOUT);
    pRedialHelper = new SilentRedialHelper(objContext, objAnyReason);
    pRedialHelper->Redial(ISilentRedialHelper::INTERVAL_BY_TYPE);

    CallInfo objCallInfo;
    ON_CALL(objContext, GetCallInfo).WillByDefault(ReturnRef(objCallInfo));  // initial type is VOIP
    ParticipantInfo objParticipantInfo(objContext);
    ON_CALL(objContext, GetParticipantInfo)
            .WillByDefault(ReturnRef(objParticipantInfo));  // empty remote target.

    AString strEmptyNumber;
    EXPECT_CALL(objMtcCall, Start(CallType::VOIP, strEmptyNumber, _, _)).Times(2);

    pRedialHelper->Timer_TimerExpired(&objTimer);

    EXPECT_EQ(CallReasonInfo(CODE_NONE), pRedialHelper->Redial(ISilentRedialHelper::INTERVAL_BY_TYPE));

    ON_CALL(objMessageUtils, GetPreviousResponse(_, _, _)).WillByDefault(Return(nullptr));
    pRedialHelper->Timer_TimerExpired(&objTimer);
    EXPECT_EQ(CallReasonInfo(CODE_NETWORK_RESP_TIMEOUT, EXTRA_CODE_METHOD_INVITE),
            pRedialHelper->Redial(ISilentRedialHelper::INTERVAL_BY_TYPE));  // by nMaxCount
}

TEST_F(SilentRedialHelperTest, TimerExpiresInvokesReStartWithRedirectionType)
{
    AString strContactUri("sip:AnyContact@ims.google.com");
    const CallReasonInfo objAnyReason(
            CODE_INTERNAL_REDIAL, EXTRA_CODE_REDIAL_FOR_REDIRECTION, strContactUri);
    pRedialHelper = new SilentRedialHelper(objContext, objAnyReason);
    pRedialHelper->Redial(ISilentRedialHelper::INTERVAL_BY_TYPE);

    CallInfo objCallInfo;
    ON_CALL(objContext, GetCallInfo).WillByDefault(ReturnRef(objCallInfo));  // initial type is VOIP

    EXPECT_CALL(objMtcCall, Start(CallType::VOIP, strContactUri, _, _));

    pRedialHelper->Timer_TimerExpired(&objTimer);
}

TEST_F(SilentRedialHelperTest, TimerExpiresInvokesReStartWithVideoSdpChangeType)
{
    AString strMediaTypes("audiovideo");
    const CallReasonInfo objAnyReason(
            CODE_INTERNAL_REDIAL, EXTRA_CODE_REDIAL_FOR_SDP_CHANGE, strMediaTypes);
    pRedialHelper = new SilentRedialHelper(objContext, objAnyReason);
    pRedialHelper->Redial(ISilentRedialHelper::INTERVAL_BY_TYPE);

    ParticipantInfo objParticipantInfo(objContext);
    ON_CALL(objContext, GetParticipantInfo)
            .WillByDefault(ReturnRef(objParticipantInfo));  // empty remote target.

    AString strEmptyNumber;
    EXPECT_CALL(objMtcCall, Start(CallType::VT, strEmptyNumber, _, _));

    pRedialHelper->Timer_TimerExpired(&objTimer);
}

TEST_F(SilentRedialHelperTest, TimerExpiresInvokesReStartWithAudioSdpChangeType)
{
    AString strMediaTypes("audio");
    const CallReasonInfo objAnyReason(
            CODE_INTERNAL_REDIAL, EXTRA_CODE_REDIAL_FOR_SDP_CHANGE, strMediaTypes);
    pRedialHelper = new SilentRedialHelper(objContext, objAnyReason);
    pRedialHelper->Redial(ISilentRedialHelper::INTERVAL_BY_TYPE);

    ParticipantInfo objParticipantInfo(objContext);
    ON_CALL(objContext, GetParticipantInfo)
            .WillByDefault(ReturnRef(objParticipantInfo));  // empty remote target.

    AString strEmptyNumber;
    EXPECT_CALL(objMtcCall, Start(CallType::VOIP, strEmptyNumber, _, _));

    pRedialHelper->Timer_TimerExpired(&objTimer);
}

TEST_F(SilentRedialHelperTest, TimerExpiresInvokesReStartWithRedialNormalWithNextPcscf)
{
    const CallReasonInfo objAnyReason(CODE_INTERNAL_REDIAL, EXTRA_CODE_REDIAL_WITH_NEXT_PCSCF);
    pRedialHelper = new SilentRedialHelper(objContext, objAnyReason);
    pRedialHelper->Redial(ISilentRedialHelper::INTERVAL_BY_TYPE);

    CallInfo objCallInfo;
    ON_CALL(objContext, GetCallInfo).WillByDefault(ReturnRef(objCallInfo));  // initial type is VOIP

    ParticipantInfo objParticipantInfo(objContext);
    ON_CALL(objContext, GetParticipantInfo)
            .WillByDefault(ReturnRef(objParticipantInfo));  // empty remote target.

    AString strEmptyNumber;
    EXPECT_CALL(objMtcCall, Start(CallType::VOIP, strEmptyNumber, _, _));

    pRedialHelper->Timer_TimerExpired(&objTimer);
}

TEST_F(SilentRedialHelperTest, TimerExpiresInvokesReStartWithRedialEmergencyWithNextPcscf)
{
    const CallReasonInfo objAnyReason(
            CODE_INTERNAL_REDIAL, EXTRA_CODE_REDIAL_EMERGENCY_WITH_NEXT_PCSCF);
    pRedialHelper = new SilentRedialHelper(objContext, objAnyReason);
    pRedialHelper->Redial(ISilentRedialHelper::INTERVAL_BY_TYPE);

    CallInfo objCallInfo;
    ON_CALL(objContext, GetCallInfo).WillByDefault(ReturnRef(objCallInfo));  // initial type is VOIP

    ParticipantInfo objParticipantInfo(objContext);
    ON_CALL(objContext, GetParticipantInfo)
            .WillByDefault(ReturnRef(objParticipantInfo));  // empty remote target.

    AString strEmptyNumber;
    EXPECT_CALL(objMtcCall, Start(CallType::VOIP, strEmptyNumber, _, _));

    pRedialHelper->Timer_TimerExpired(&objTimer);
}

TEST_F(SilentRedialHelperTest, TimerExpiresInvokesReStartWithRedialByRttEmergencyRejection)
{
    const CallReasonInfo objAnyReason(
            CODE_INTERNAL_REDIAL, EXTRA_CODE_REDIAL_BY_RTT_EMERGENCY_REJECTION);
    pRedialHelper = new SilentRedialHelper(objContext, objAnyReason);
    pRedialHelper->Redial(ISilentRedialHelper::INTERVAL_BY_TYPE);

    CallInfo objCallInfo;
    objCallInfo.eInitialCallType = CallType::RTT;
    ON_CALL(objContext, GetCallInfo).WillByDefault(ReturnRef(objCallInfo));  // initial type is RTT

    ParticipantInfo objParticipantInfo(objContext);
    ON_CALL(objContext, GetParticipantInfo)
            .WillByDefault(ReturnRef(objParticipantInfo));  // empty remote target.

    AString strEmptyNumber;
    EXPECT_CALL(objMtcCall, Start(CallType::VOIP, strEmptyNumber, _, _));

    pRedialHelper->Timer_TimerExpired(&objTimer);
}
