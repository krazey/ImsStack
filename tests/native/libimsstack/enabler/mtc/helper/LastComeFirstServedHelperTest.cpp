/*
 * Copyright (C) 2023 The Android Open Source Project
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
#include "IMessage.h"
#include "ImsTypeDef.h"
#include "MockIMessage.h"
#include "MockIMtcContext.h"
#include "MockIMtcService.h"
#include "MockISession.h"
#include "PlatformContext.h"
#include "call/IMtcCall.h"
#include "call/MockIMtcCall.h"
#include "call/MockIMtcCallContext.h"
#include "call/MockIMtcCallManager.h"
#include "call/MockIMtcSession.h"
#include "configuration/MockIMtcConfigurationManager.h"
#include "configuration/MtcConfigurationProxy.h"
#include "helper/ILastComeFirstServedHelper.h"
#include "helper/LastComeFirstServedHelper.h"
#include "helper/MockIPassiveTimerHolder.h"
#include "precondition/MockIMtcPreconditionManager.h"
#include <gtest/gtest.h>

using ::testing::_;
using ::testing::Return;
using ::testing::ReturnRef;

LOCAL const CallKey FIRST_INCOMING_KEY = 100;
LOCAL const CallKey LAST_INCOMING_KEY = 101;

LOCAL const IMS_SINT32 VALID_TIME = 7000;
LOCAL const IMS_SINT32 INVALID_TIME = 0;

class LastComeFirstServedHelperTest : public ::testing::Test
{
public:
    inline LastComeFirstServedHelperTest() :
            objContext(),
            objPassiveTimerHolder(),
            pConfigurationManager(new MockIMtcConfigurationManager),
            objConfigurationProxy(pConfigurationManager),
            objCallManager(),
            objLastIncomingCall(),
            objFirstIncomingCall(),
            objCallContext(),
            objCallInfo(),
            objService(),
            objPreconditionManager(),
            objSession(),
            objISession(),
            objIncomingCalls()
    {
    }

protected:
    MockIMtcContext objContext;
    MockIPassiveTimerHolder objPassiveTimerHolder;
    MockIMtcConfigurationManager* pConfigurationManager;
    MtcConfigurationProxy objConfigurationProxy;
    MockIMtcCallManager objCallManager;
    MockIMtcCall objLastIncomingCall;
    MockIMtcCall objFirstIncomingCall;
    MockIMtcCallContext objCallContext;
    CallInfo objCallInfo;
    MockIMtcService objService;
    MockIMtcPreconditionManager objPreconditionManager;
    MockIMtcSession objSession;
    MockISession objISession;

    ImsList<IMtcCall*> objIncomingCalls;

protected:
    virtual void SetUp() override
    {
        ON_CALL(objContext, GetPassiveTimerHolder())
                .WillByDefault(ReturnRef(objPassiveTimerHolder));
        ON_CALL(objContext, GetConfigurationProxy())
                .WillByDefault(ReturnRef(objConfigurationProxy));
        ON_CALL(objContext, GetCallManager()).WillByDefault(ReturnRef(objCallManager));
        ON_CALL(objCallManager, GetCallByCallKey(FIRST_INCOMING_KEY))
                .WillByDefault(Return(&objFirstIncomingCall));
        ON_CALL(objCallManager, GetCallByCallKey(LAST_INCOMING_KEY))
                .WillByDefault(Return(&objLastIncomingCall));

        ON_CALL(objLastIncomingCall, GetCallContext()).WillByDefault(ReturnRef(objCallContext));
        ON_CALL(objFirstIncomingCall, GetCallContext()).WillByDefault(ReturnRef(objCallContext));
        ON_CALL(objLastIncomingCall, GetKey()).WillByDefault(Return(LAST_INCOMING_KEY));
        ON_CALL(objFirstIncomingCall, GetKey()).WillByDefault(Return(FIRST_INCOMING_KEY));

        ON_CALL(objCallContext, GetCallInfo()).WillByDefault(ReturnRef(objCallInfo));
        ON_CALL(objCallContext, GetService()).WillByDefault(ReturnRef(objService));
        ON_CALL(objCallContext, GetPreconditionManager())
                .WillByDefault(ReturnRef(objPreconditionManager));
        ON_CALL(objCallContext, GetSession()).WillByDefault(Return(&objSession));
        ON_CALL(objSession, GetISession()).WillByDefault(ReturnRef(objISession));
    }

    virtual void TearDown() override {}

    void SetUpForNetworkInfo(IN IMS_BOOL bNr, IN IMS_BOOL bWlan = IMS_FALSE)
    {
        ON_CALL(objService, IsNr()).WillByDefault(Return(bNr));
        ON_CALL(objService, IsWlanIpCanType()).WillByDefault(Return(bWlan));
    }
};

TEST_F(LastComeFirstServedHelperTest, IsSupportedReturnsTrueIfPreAlertingTimerIsValid)
{
    LastComeFirstServedHelper objLastComeFirstServedHelper(objContext);
    ON_CALL(*pConfigurationManager, GetPreAlertingTimer()).WillByDefault(Return(VALID_TIME));
    EXPECT_TRUE(objLastComeFirstServedHelper.IsSupported(objConfigurationProxy));
}

TEST_F(LastComeFirstServedHelperTest, IsSupportedReturnsFalseIfPreAlertingTimerIsInvalid)
{
    LastComeFirstServedHelper objLastComeFirstServedHelper(objContext);
    ON_CALL(*pConfigurationManager, GetPreAlertingTimer()).WillByDefault(Return(INVALID_TIME));
    EXPECT_FALSE(objLastComeFirstServedHelper.IsSupported(objConfigurationProxy));
}

TEST_F(LastComeFirstServedHelperTest, OnCallReceivedDoesNothingInEmergencyCall)
{
    objCallInfo.eEmergencyType = EmergencyType::EMERGENCY_ROUTING;
    LastComeFirstServedHelper objLastComeFirstServedHelper(objContext);

    objLastComeFirstServedHelper.OnCallReceived(LAST_INCOMING_KEY);
}

TEST_F(LastComeFirstServedHelperTest, OnCallReceivedDoesNothingInUssi)
{
    objCallInfo.bUssi = IMS_TRUE;
    LastComeFirstServedHelper objLastComeFirstServedHelper(objContext);

    objLastComeFirstServedHelper.OnCallReceived(LAST_INCOMING_KEY);
}

TEST_F(LastComeFirstServedHelperTest, OnCallReceivedStartsTimerIfNoPreExistingIncomingCallAndOnNr)
{
    LastComeFirstServedHelper objLastComeFirstServedHelper(objContext);

    SetUpForNetworkInfo(IMS_TRUE);
    ON_CALL(*pConfigurationManager, GetPreAlertingTimer()).WillByDefault(Return(VALID_TIME));
    EXPECT_CALL(objPassiveTimerHolder,
            AddTimer(IPassiveTimerHolder::Type::PRE_ALERTING_GUARD, VALID_TIME, IMS_TRUE));

    objLastComeFirstServedHelper.OnCallReceived(LAST_INCOMING_KEY);
}

TEST_F(LastComeFirstServedHelperTest,
        OnCallReceivedDoesNothingIfNoPreExistingIncomingCallAndNotOnNr)
{
    LastComeFirstServedHelper objLastComeFirstServedHelper(objContext);

    SetUpForNetworkInfo(IMS_FALSE, IMS_TRUE);
    EXPECT_CALL(
            objPassiveTimerHolder, AddTimer(IPassiveTimerHolder::Type::PRE_ALERTING_GUARD, _, _))
            .Times(0);

    objLastComeFirstServedHelper.OnCallReceived(LAST_INCOMING_KEY);
}

TEST_F(LastComeFirstServedHelperTest,
        OnCallReceivedDoesNothingIfPreExistingIncomingCallAndTimerIsActive)
{
    LastComeFirstServedHelper objLastComeFirstServedHelper(objContext);

    SetUpForNetworkInfo(IMS_TRUE);
    objLastComeFirstServedHelper.OnCallReceived(FIRST_INCOMING_KEY);
    objIncomingCalls.Append(&objFirstIncomingCall);

    SetUpForNetworkInfo(IMS_FALSE);
    ON_CALL(objPassiveTimerHolder, IsActive(IPassiveTimerHolder::Type::PRE_ALERTING_GUARD))
            .WillByDefault((Return(IMS_TRUE)));

    objLastComeFirstServedHelper.OnCallReceived(LAST_INCOMING_KEY);
}

TEST_F(LastComeFirstServedHelperTest,
        OnCallReceivedDoesNothingIfPreExistingIncomingCallAndTimerNotStarted)
{
    LastComeFirstServedHelper objLastComeFirstServedHelper(objContext);

    SetUpForNetworkInfo(IMS_TRUE);
    objLastComeFirstServedHelper.OnCallReceived(FIRST_INCOMING_KEY);
    objIncomingCalls.Append(&objFirstIncomingCall);

    EXPECT_CALL(objPassiveTimerHolder, IsActive(IPassiveTimerHolder::Type::PRE_ALERTING_GUARD))
            .Times(0);

    objLastComeFirstServedHelper.OnCallReceived(LAST_INCOMING_KEY);
}

TEST_F(LastComeFirstServedHelperTest,
        OnCallReceivedRejectsPreExistingIncomingCallWith486IfPrackReceived)
{
    LastComeFirstServedHelper objLastComeFirstServedHelper(objContext);

    SetUpForNetworkInfo(IMS_TRUE);
    objLastComeFirstServedHelper.OnCallReceived(FIRST_INCOMING_KEY);
    objIncomingCalls.Append(&objFirstIncomingCall);

    ON_CALL(objCallManager, GetCallsByState(IMtcCall::State::INCOMING))
            .WillByDefault(Return(objIncomingCalls));

    ON_CALL(objPassiveTimerHolder, IsActive(IPassiveTimerHolder::Type::PRE_ALERTING_GUARD))
            .WillByDefault((Return(IMS_FALSE)));

    MockIMessage objPrackMessage;
    ON_CALL(objISession, GetPreviousRequest(IMessage::SESSION_PRACK))
            .WillByDefault(Return(&objPrackMessage));
    EXPECT_CALL(objFirstIncomingCall, Reject(CallReasonInfo(CODE_REJECT_ONGOING_CALL_SETUP)));

    SetUpForNetworkInfo(IMS_FALSE);
    EXPECT_CALL(
            objPassiveTimerHolder, AddTimer(IPassiveTimerHolder::Type::PRE_ALERTING_GUARD, _, _))
            .Times(0);
    objLastComeFirstServedHelper.OnCallReceived(LAST_INCOMING_KEY);
}

TEST_F(LastComeFirstServedHelperTest,
        OnCallReceivedRejectsPreExistingIncomingCallWith486IfDedicatedBearerEstablished)
{
    LastComeFirstServedHelper objLastComeFirstServedHelper(objContext);

    SetUpForNetworkInfo(IMS_TRUE);
    objLastComeFirstServedHelper.OnCallReceived(FIRST_INCOMING_KEY);
    objIncomingCalls.Append(&objFirstIncomingCall);

    ON_CALL(objCallManager, GetCallsByState(IMtcCall::State::INCOMING))
            .WillByDefault(Return(objIncomingCalls));

    ON_CALL(objPassiveTimerHolder, IsActive(IPassiveTimerHolder::Type::PRE_ALERTING_GUARD))
            .WillByDefault((Return(IMS_FALSE)));

    ON_CALL(objISession, GetPreviousRequest(IMessage::SESSION_PRACK))
            .WillByDefault(Return(nullptr));
    ON_CALL(objPreconditionManager, IsDedicatedBearerAllocated(&objISession, MEDIATYPE_AUDIO))
            .WillByDefault(Return(IMS_TRUE));
    EXPECT_CALL(objFirstIncomingCall, Reject(CallReasonInfo(CODE_REJECT_ONGOING_CALL_SETUP)));

    SetUpForNetworkInfo(IMS_FALSE);
    EXPECT_CALL(
            objPassiveTimerHolder, AddTimer(IPassiveTimerHolder::Type::PRE_ALERTING_GUARD, _, _))
            .Times(0);
    objLastComeFirstServedHelper.OnCallReceived(LAST_INCOMING_KEY);
}

TEST_F(LastComeFirstServedHelperTest,
        OnCallReceivedRejectsPreExistingIncomingCallWith580IfNoPrackAndDedicatedBearer)
{
    LastComeFirstServedHelper objLastComeFirstServedHelper(objContext);

    SetUpForNetworkInfo(IMS_TRUE);
    objLastComeFirstServedHelper.OnCallReceived(FIRST_INCOMING_KEY);
    objIncomingCalls.Append(&objFirstIncomingCall);

    ON_CALL(objCallManager, GetCallsByState(IMtcCall::State::INCOMING))
            .WillByDefault(Return(objIncomingCalls));

    ON_CALL(objPassiveTimerHolder, IsActive(IPassiveTimerHolder::Type::PRE_ALERTING_GUARD))
            .WillByDefault((Return(IMS_FALSE)));

    ON_CALL(objISession, GetPreviousRequest(IMessage::SESSION_PRACK))
            .WillByDefault(Return(nullptr));
    ON_CALL(objPreconditionManager, IsDedicatedBearerAllocated(&objISession, MEDIATYPE_AUDIO))
            .WillByDefault(Return(IMS_FALSE));
    EXPECT_CALL(objFirstIncomingCall, Reject(CallReasonInfo(CODE_REJECT_QOS_FAILURE)));

    SetUpForNetworkInfo(IMS_FALSE);
    EXPECT_CALL(
            objPassiveTimerHolder, AddTimer(IPassiveTimerHolder::Type::PRE_ALERTING_GUARD, _, _))
            .Times(0);
    objLastComeFirstServedHelper.OnCallReceived(LAST_INCOMING_KEY);
}

TEST_F(LastComeFirstServedHelperTest,
        OnCallReceivedRejectsPreExistingIncomingCallAndStartsTimerOnNr)
{
    LastComeFirstServedHelper objLastComeFirstServedHelper(objContext);

    SetUpForNetworkInfo(IMS_TRUE);
    objLastComeFirstServedHelper.OnCallReceived(FIRST_INCOMING_KEY);
    objIncomingCalls.Append(&objFirstIncomingCall);

    ON_CALL(objCallManager, GetCallsByState(IMtcCall::State::INCOMING))
            .WillByDefault(Return(objIncomingCalls));

    ON_CALL(objPassiveTimerHolder, IsActive(IPassiveTimerHolder::Type::PRE_ALERTING_GUARD))
            .WillByDefault((Return(IMS_FALSE)));

    ON_CALL(objISession, GetPreviousRequest(IMessage::SESSION_PRACK))
            .WillByDefault(Return(nullptr));
    ON_CALL(objPreconditionManager, IsDedicatedBearerAllocated(&objISession, MEDIATYPE_AUDIO))
            .WillByDefault(Return(IMS_FALSE));
    EXPECT_CALL(objFirstIncomingCall, Reject(CallReasonInfo(CODE_REJECT_QOS_FAILURE)));

    ON_CALL(*pConfigurationManager, GetPreAlertingTimer()).WillByDefault(Return(VALID_TIME));
    EXPECT_CALL(objPassiveTimerHolder,
            AddTimer(IPassiveTimerHolder::Type::PRE_ALERTING_GUARD, VALID_TIME, IMS_TRUE));
    objLastComeFirstServedHelper.OnCallReceived(LAST_INCOMING_KEY);
}

TEST_F(LastComeFirstServedHelperTest, OnCallReceivedDoesNothingIfAllIncomingCallsNotOnNr)
{
    LastComeFirstServedHelper objLastComeFirstServedHelper(objContext);

    SetUpForNetworkInfo(IMS_FALSE);
    objLastComeFirstServedHelper.OnCallReceived(FIRST_INCOMING_KEY);
    objIncomingCalls.Append(&objFirstIncomingCall);

    ON_CALL(objCallManager, GetCallsByState(IMtcCall::State::INCOMING))
            .WillByDefault(Return(objIncomingCalls));

    objLastComeFirstServedHelper.OnCallReceived(LAST_INCOMING_KEY);
}

TEST_F(LastComeFirstServedHelperTest,
        OnCallReceivedDoesNothingIfPreExistingIncomingCallIsEmergencyOnNr)
{
    LastComeFirstServedHelper objLastComeFirstServedHelper(objContext);

    MockIMtcCall objEmergencyCall;
    objIncomingCalls.Append(&objEmergencyCall);
    EXPECT_CALL(objCallManager, GetCallsByState(IMtcCall::State::INCOMING))
            .WillOnce(Return(objIncomingCalls));

    const CallKey EMERGENCY_CALL_KEY = 102;
    ON_CALL(objEmergencyCall, GetKey()).WillByDefault(Return(EMERGENCY_CALL_KEY));
    ON_CALL(objCallManager, GetCallByCallKey(EMERGENCY_CALL_KEY))
            .WillByDefault(Return(&objEmergencyCall));
    MockIMtcCallContext objEmergencyCallContext;
    ON_CALL(objEmergencyCall, GetCallContext()).WillByDefault(ReturnRef(objEmergencyCallContext));
    CallInfo objEmergencyCallInfo;
    objEmergencyCallInfo.eEmergencyType = EmergencyType::EMERGENCY_ROUTING;
    ON_CALL(objEmergencyCallContext, GetCallInfo()).WillByDefault(ReturnRef(objEmergencyCallInfo));

    objLastComeFirstServedHelper.OnCallReceived(LAST_INCOMING_KEY);
}

TEST_F(LastComeFirstServedHelperTest, OnCallReceivedDoesNothingIfPreExistingIncomingCallIsUssiOnNr)
{
    LastComeFirstServedHelper objLastComeFirstServedHelper(objContext);

    MockIMtcCall objUssiCall;
    objIncomingCalls.Append(&objUssiCall);
    EXPECT_CALL(objCallManager, GetCallsByState(IMtcCall::State::INCOMING))
            .WillOnce(Return(objIncomingCalls));

    const CallKey USSI_CALL_KEY = 102;
    ON_CALL(objUssiCall, GetKey()).WillByDefault(Return(USSI_CALL_KEY));
    ON_CALL(objCallManager, GetCallByCallKey(USSI_CALL_KEY)).WillByDefault(Return(&objUssiCall));
    MockIMtcCallContext objUssiCallContext;
    ON_CALL(objUssiCall, GetCallContext()).WillByDefault(ReturnRef(objUssiCallContext));
    CallInfo objUssiCallInfo;
    objUssiCallInfo.bUssi = IMS_TRUE;
    ON_CALL(objUssiCallContext, GetCallInfo()).WillByDefault(ReturnRef(objUssiCallInfo));

    objLastComeFirstServedHelper.OnCallReceived(LAST_INCOMING_KEY);
}
