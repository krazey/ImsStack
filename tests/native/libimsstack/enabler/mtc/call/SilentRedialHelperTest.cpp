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
#include "MockIMtcCallController.h"
#include "MockITimer.h"
#include "MtcDef.h"
#include "PlatformContext.h"
#include "TestTimerService.h"
#include "call/MockIMtcCall.h"
#include "call/MockIMtcCallContext.h"
#include "call/MockIMtcCallManager.h"
#include "call/MockIMtcSession.h"
#include "call/ParticipantInfo.h"
#include "call/SilentRedialHelper.h"
#include "configuration/MockIMtcConfigurationManager.h"
#include "configuration/MtcConfigurationProxy.h"
#include "core/MockISession.h"
#include "helper/MockICallStateProxy.h"
#include "helper/MockMtcTimerWrapper.h"
#include "helper/MtcSupplementaryService.h"
#include "media/MockIMtcMediaManager.h"
#include <gtest/gtest.h>
#include <vector>

using ::testing::_;
using ::testing::Return;
using ::testing::ReturnRef;

LOCAL CallKey ANY_CALL_KEY = 100;

class SilentRedialHelperTest : public ::testing::Test
{
public:
    inline SilentRedialHelperTest() :
            pRedialHelper(IMS_NULL),
            pTimerService(new TestTimerService()),
            objTimer(pTimerService->GetMockTimer()),
            pMockConfigurationManager(IMS_NULL),
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
    MockIMtcMediaManager objMediaManager;
    MockIMtcSession objMtcSession;
    MockISession objSession;
    TestTimerService* pTimerService;
    MockITimer& objTimer;
    MockIMtcConfigurationManager* pMockConfigurationManager;
    MtcConfigurationProxy* pConfigurationProxy;
    MtcSupplementaryService* pSupplementaryService;
    MediaInfo objMediaInfo;
    MockMtcTimerWrapper objTimerWrapper;

protected:
    virtual void SetUp() override
    {
        ON_CALL(objContext, GetCallKey).WillByDefault(Return(ANY_CALL_KEY));
        ON_CALL(objContext, GetSession()).WillByDefault(Return(&objMtcSession));
        ON_CALL(objContext, GetMediaManager).WillByDefault(ReturnRef(objMediaManager));
        ON_CALL(objMediaManager, GetMediaInfo).WillByDefault(ReturnRef(objMediaInfo));
        ON_CALL(objMtcSession, GetISession).WillByDefault(ReturnRef(objSession));
        ON_CALL(objContext, GetCallStateProxy).WillByDefault(ReturnRef(objCallStateProxy));
        ON_CALL(objContext, GetCallController).WillByDefault(ReturnRef(objController));
        ON_CALL(objContext, GetCallManager).WillByDefault(ReturnRef(objMtcCallManager));
        ON_CALL(objMtcCallManager, GetCallByCallKey(ANY_CALL_KEY))
                .WillByDefault(Return(&objMtcCall));

        pMockConfigurationManager = new MockIMtcConfigurationManager();
        pConfigurationProxy = new MtcConfigurationProxy(pMockConfigurationManager);
        ON_CALL(objContext, GetConfigurationProxy).WillByDefault(ReturnRef(*pConfigurationProxy));
        pSupplementaryService = new MtcSupplementaryService(objContext, *pConfigurationProxy);
        ON_CALL(objContext, GetSupplementaryService)
                .WillByDefault(ReturnRef(*pSupplementaryService));
        ON_CALL(objContext, GetTimer()).WillByDefault(ReturnRef(objTimerWrapper));
    }

    virtual void TearDown() override
    {
        delete pConfigurationProxy;
        delete pSupplementaryService;
    }
};

TEST_F(SilentRedialHelperTest, CreateHelperWithRetryAfterType)
{
    const CallReasonInfo objReason(CODE_INTERNAL_REDIAL, EXTRA_CODE_REDIAL_BY_RETRY_AFTER, "1000");
    pRedialHelper = new SilentRedialHelper(objContext, objReason);
    EXPECT_EQ(pRedialHelper->GetType(), EXTRA_CODE_REDIAL_BY_RETRY_AFTER);
}

TEST_F(SilentRedialHelperTest, CreateHelperWithRequestTimeoutType)
{
    ON_CALL(*pMockConfigurationManager, GetSilentRedialInterval).WillByDefault(Return(2000));
    ON_CALL(*pMockConfigurationManager, GetSilentRedialMaxRetryCount).WillByDefault(Return(3));

    const CallReasonInfo objReason(CODE_INTERNAL_REDIAL, EXTRA_CODE_REDIAL_BY_REQUEST_TIMEOUT);
    pRedialHelper = new SilentRedialHelper(objContext, objReason);
    EXPECT_EQ(pRedialHelper->GetType(), EXTRA_CODE_REDIAL_BY_REQUEST_TIMEOUT);
}

TEST_F(SilentRedialHelperTest, CreateHelperWithRedirectionType)
{
    const CallReasonInfo objReason(CODE_INTERNAL_REDIAL, EXTRA_CODE_REDIAL_FOR_REDIRECTION);
    pRedialHelper = new SilentRedialHelper(objContext, objReason);
    EXPECT_EQ(pRedialHelper->GetType(), EXTRA_CODE_REDIAL_FOR_REDIRECTION);
}

TEST_F(SilentRedialHelperTest, CreateHelperWithSdpChangeType)
{
    const CallReasonInfo objReason(CODE_INTERNAL_REDIAL, EXTRA_CODE_REDIAL_FOR_SDP_CHANGE);
    pRedialHelper = new SilentRedialHelper(objContext, objReason);
    EXPECT_EQ(pRedialHelper->GetType(), EXTRA_CODE_REDIAL_FOR_SDP_CHANGE);
}

TEST_F(SilentRedialHelperTest, CreateHelperWithRedialEmergencyType)
{
    const CallReasonInfo objReason(
            CODE_INTERNAL_REDIAL, EXTRA_CODE_REDIAL_EMERGENCY_WITH_NEXT_PCSCF);
    pRedialHelper = new SilentRedialHelper(objContext, objReason);
    EXPECT_EQ(pRedialHelper->GetType(), EXTRA_CODE_REDIAL_EMERGENCY_WITH_NEXT_PCSCF);
}

TEST_F(SilentRedialHelperTest, IsSameRedialTypeReturnsComparisonResult)
{
    const CallReasonInfo objAnyReason(CODE_INTERNAL_REDIAL, EXTRA_CODE_REDIAL_FOR_REDIRECTION);
    pRedialHelper = new SilentRedialHelper(objContext, objAnyReason);

    EXPECT_TRUE(pRedialHelper->IsSameRedialType(objAnyReason));
    const CallReasonInfo objDifferentReason(CODE_INTERNAL_REDIAL, EXTRA_CODE_REDIAL_FOR_SDP_CHANGE);
    EXPECT_FALSE(pRedialHelper->IsSameRedialType(objDifferentReason));
}

TEST_F(SilentRedialHelperTest, CallStateChangedToTerminatingInvokesReleaseRedialHelepr)
{
    const CallReasonInfo objAnyReason(CODE_INTERNAL_REDIAL, EXTRA_CODE_REDIAL_FOR_REDIRECTION);
    pRedialHelper = new SilentRedialHelper(objContext, objAnyReason);

    EXPECT_CALL(objController, ReleaseRedialHelper);

    pRedialHelper->OnCallStateChanged(
            ANY_CALL_KEY, IMtcCall::State::ESTABLISHED, CallType::VOIP, IMS_FALSE, 0);
}

TEST_F(SilentRedialHelperTest, CallStateChangedToEstablishedInvokesReleaseRedialHelepr)
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
    EXPECT_CALL(objMediaManager, DestroyMediaSession());
    EXPECT_CALL(objContext, RemoveSession(&objSession));
    EXPECT_CALL(objTimerWrapper, StopAll());

    const CallReasonInfo objAnyReason(CODE_INTERNAL_REDIAL, EXTRA_CODE_REDIAL_FOR_REDIRECTION);
    pRedialHelper = new SilentRedialHelper(objContext, objAnyReason);
    EXPECT_EQ(IMS_SUCCESS, pRedialHelper->Redial());
}

TEST_F(SilentRedialHelperTest, RedialReleaseSessionResourceIfRedialEmergency)
{
    EXPECT_CALL(objMediaManager, DestroyMediaSession());
    EXPECT_CALL(objContext, RemoveSession(&objSession));
    EXPECT_CALL(objTimerWrapper, Stop(_));

    const CallReasonInfo objAnyReason(
            CODE_INTERNAL_REDIAL, EXTRA_CODE_REDIAL_EMERGENCY_WITH_NEXT_PCSCF);
    pRedialHelper = new SilentRedialHelper(objContext, objAnyReason);
    EXPECT_EQ(IMS_SUCCESS, pRedialHelper->Redial());
}

TEST_F(SilentRedialHelperTest, RedialStartsTimer)
{
    const CallReasonInfo objAnyReason(CODE_INTERNAL_REDIAL, EXTRA_CODE_REDIAL_FOR_REDIRECTION);
    IMS_SINT32 nRedirectionInterval = 0;
    pRedialHelper = new SilentRedialHelper(objContext, objAnyReason);
    EXPECT_CALL(objTimer, SetTimer(nRedirectionInterval, pRedialHelper));
    EXPECT_EQ(IMS_SUCCESS, pRedialHelper->Redial());
}

TEST_F(SilentRedialHelperTest, RedialReturnsFailureIfCountExcceedsMaxRedialCount)
{
    const CallReasonInfo objAnyReason(CODE_INTERNAL_REDIAL, EXTRA_CODE_REDIAL_FOR_REDIRECTION);
    pRedialHelper = new SilentRedialHelper(objContext, objAnyReason);
    pRedialHelper->Redial();
    EXPECT_EQ(IMS_FAILURE, pRedialHelper->Redial());
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
    const CallReasonInfo objAnyReason(
            CODE_INTERNAL_REDIAL, EXTRA_CODE_REDIAL_BY_RETRY_AFTER, "1000");
    pRedialHelper = new SilentRedialHelper(objContext, objAnyReason);
    pRedialHelper->Redial();

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
    ON_CALL(*pMockConfigurationManager, GetSilentRedialInterval).WillByDefault(Return(nInterval));
    ON_CALL(*pMockConfigurationManager, GetSilentRedialMaxRetryCount)
            .WillByDefault(Return(nMaxCount));

    const CallReasonInfo objAnyReason(CODE_INTERNAL_REDIAL, EXTRA_CODE_REDIAL_BY_REQUEST_TIMEOUT);
    pRedialHelper = new SilentRedialHelper(objContext, objAnyReason);
    pRedialHelper->Redial();

    CallInfo objCallInfo;
    ON_CALL(objContext, GetCallInfo).WillByDefault(ReturnRef(objCallInfo));  // initial type is VOIP
    ParticipantInfo objParticipantInfo(objContext);
    ON_CALL(objContext, GetParticipantInfo)
            .WillByDefault(ReturnRef(objParticipantInfo));  // empty remote target.

    AString strEmptyNumber;
    EXPECT_CALL(objMtcCall, Start(CallType::VOIP, strEmptyNumber, _, _)).Times(2);

    pRedialHelper->Timer_TimerExpired(&objTimer);

    EXPECT_EQ(IMS_SUCCESS, pRedialHelper->Redial());
    pRedialHelper->Timer_TimerExpired(&objTimer);
    EXPECT_EQ(IMS_FAILURE, pRedialHelper->Redial());  // by nMaxCount
}

TEST_F(SilentRedialHelperTest, TimerExpiresInvokesReStartWithRedirectionType)
{
    AString strContactUri("sip:AnyContact@ims.google.com");
    const CallReasonInfo objAnyReason(
            CODE_INTERNAL_REDIAL, EXTRA_CODE_REDIAL_FOR_REDIRECTION, strContactUri);
    pRedialHelper = new SilentRedialHelper(objContext, objAnyReason);
    pRedialHelper->Redial();

    CallInfo objCallInfo;
    ON_CALL(objContext, GetCallInfo).WillByDefault(ReturnRef(objCallInfo));  // initial type is VOIP

    EXPECT_CALL(objMtcCall, Start(CallType::VOIP, strContactUri, _, _));

    pRedialHelper->Timer_TimerExpired(&objTimer);
}

TEST_F(SilentRedialHelperTest, TimerExpiresInvokesReStartWithVideoSdpChangeType)
{
    AString strSdpBody("m=audio 1234.... m=video 5678");
    const CallReasonInfo objAnyReason(
            CODE_INTERNAL_REDIAL, EXTRA_CODE_REDIAL_FOR_SDP_CHANGE, strSdpBody);
    pRedialHelper = new SilentRedialHelper(objContext, objAnyReason);
    pRedialHelper->Redial();

    ParticipantInfo objParticipantInfo(objContext);
    ON_CALL(objContext, GetParticipantInfo)
            .WillByDefault(ReturnRef(objParticipantInfo));  // empty remote target.

    AString strEmptyNumber;
    EXPECT_CALL(objMtcCall, Start(CallType::VT, strEmptyNumber, _, _));

    pRedialHelper->Timer_TimerExpired(&objTimer);
}

TEST_F(SilentRedialHelperTest, TimerExpiresInvokesReStartWithAudioSdpChangeType)
{
    AString strSdpBody("m=audio 1234....");
    const CallReasonInfo objAnyReason(
            CODE_INTERNAL_REDIAL, EXTRA_CODE_REDIAL_FOR_SDP_CHANGE, strSdpBody);
    pRedialHelper = new SilentRedialHelper(objContext, objAnyReason);
    pRedialHelper->Redial();

    ParticipantInfo objParticipantInfo(objContext);
    ON_CALL(objContext, GetParticipantInfo)
            .WillByDefault(ReturnRef(objParticipantInfo));  // empty remote target.

    AString strEmptyNumber;
    EXPECT_CALL(objMtcCall, Start(CallType::VOIP, strEmptyNumber, _, _));

    pRedialHelper->Timer_TimerExpired(&objTimer);
}

TEST_F(SilentRedialHelperTest, TimerExpiresInvokesReStartWithDowngradeAudioSdpChangeType)
{
    AString strSdpBody("m=audio 1234.... m=video 0");
    const CallReasonInfo objAnyReason(
            CODE_INTERNAL_REDIAL, EXTRA_CODE_REDIAL_FOR_SDP_CHANGE, strSdpBody);
    pRedialHelper = new SilentRedialHelper(objContext, objAnyReason);
    pRedialHelper->Redial();

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
    pRedialHelper->Redial();

    CallInfo objCallInfo;
    ON_CALL(objContext, GetCallInfo).WillByDefault(ReturnRef(objCallInfo));  // initial type is VOIP

    ParticipantInfo objParticipantInfo(objContext);
    ON_CALL(objContext, GetParticipantInfo)
            .WillByDefault(ReturnRef(objParticipantInfo));  // empty remote target.

    AString strEmptyNumber;
    EXPECT_CALL(objMtcCall, Start(CallType::VOIP, strEmptyNumber, _, _));

    pRedialHelper->Timer_TimerExpired(&objTimer);
}
