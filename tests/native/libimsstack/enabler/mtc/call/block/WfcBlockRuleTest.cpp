/*
 * Copyright (C) 2024 The Android Open Source Project
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

#include "IMtcImsEventReceiver.h"
#include "ImsEventDef.h"
#include "MockIMtcImsEventReceiver.h"
#include "MockIMtcService.h"
#include "PlatformContext.h"
#include "TestImsRadioService.h"
#include "call/IMtcCall.h"
#include "call/MockIMtcCall.h"
#include "call/MockIMtcCallContext.h"
#include "call/MockIMtcCallManager.h"
#include "call/block/MockIMtcBlockRule.h"
#include "call/block/WfcBlockRule.h"
#include "helper/MockIPassiveTimerHolder.h"
#include <gmock/gmock.h>
#include <gtest/gtest.h>

using ::testing::Return;
using ::testing::ReturnRef;
using Result = IMtcBlockRule::Result;

class WfcBlockRuleTest : public ::testing::Test
{
public:
    MockIMtcService objService;
    MockIMtcImsEventReceiver objImsEventReceiver;
    MockIPassiveTimerHolder objPassiveTimerHolder;
    TestImsRadioService objImsRadioService;
    SsacInfo objSsacInfo;
    MockIMtcCallContext objContext;
    // cppcheck-suppress unusedStructMember
    MockIMtcBlockRuleCheckListener objListener;
    MockIMtcCallManager objCallManager;

    ImsList<IMtcCall*> lstOtherCalls;

protected:
    virtual void SetUp() override
    {
        PlatformContext::GetInstance()->SetService(
                PlatformContext::SERVICE_RADIO, &objImsRadioService);

        ON_CALL(objContext, GetService).WillByDefault(ReturnRef(objService));
        ON_CALL(objContext, GetImsEventReceiver).WillByDefault(ReturnRef(objImsEventReceiver));
        ON_CALL(objContext, GetPassiveTimerHolder())
                .WillByDefault(ReturnRef(objPassiveTimerHolder));
        ON_CALL(objContext, GetCallManager).WillByDefault(ReturnRef(objCallManager));

        ON_CALL(objImsRadioService.GetMockImsRadio(), GetSsacInfo())
                .WillByDefault(ReturnRef(objSsacInfo));
    }

    virtual void TearDown() override
    {
        PlatformContext::GetInstance()->SetService(PlatformContext::SERVICE_RADIO, IMS_NULL);

        for (IMS_UINT32 nIndex = 0; nIndex < lstOtherCalls.GetSize(); nIndex++)
        {
            delete lstOtherCalls.GetAt(nIndex);
        }
        lstOtherCalls.Clear();
    }

    MockIMtcCall* CreateMockIMtcCall(CallType eCallType)
    {
        MockIMtcCall* pCall = new MockIMtcCall();

        ON_CALL(*pCall, GetCallType).WillByDefault(Return(eCallType));

        return pCall;
    }

    void AssertResultForCallType(IN const CallType eCallType, IN const Result& objExpectedResult)
    {
        Result objResult = WfcBlockRule(objContext, eCallType).Check(objListener);

        EXPECT_EQ(objExpectedResult.eStatus, objResult.eStatus);
        EXPECT_EQ(objExpectedResult.objReason, objResult.objReason);
    }
};

// Conceptually WfcBlockRule doesn't care about cellular registration cases
TEST_F(WfcBlockRuleTest, CheckReturnsUnblockedIfNotRegisteredOnWifi)
{
    ON_CALL(objService, IsWlanIpCanType).WillByDefault(Return(IMS_FALSE));
    ON_CALL(objImsEventReceiver, GetWParam(IMS_EVENT_WFC_SETTING_CHANGED))
            .WillByDefault(Return(IMS_WFC_ON));

    ON_CALL(objImsEventReceiver, GetWParam(IMS_EVENT_IMS_VOICE_OVER_PS_STATE))
            .WillByDefault(Return(IMS_VOICE_OVER_PS_NOT_SUPPORTED));
    ON_CALL(objPassiveTimerHolder, IsActive(IPassiveTimerHolder::Type::SSAC_VOICE_BARRING))
            .WillByDefault(Return(IMS_TRUE));
    objSsacInfo.nBarringFactorForVoice = 0;

    AssertResultForCallType(CallType::VOIP, Result(Result::Status::UNBLOCKED));
    AssertResultForCallType(CallType::RTT, Result(Result::Status::UNBLOCKED));
    AssertResultForCallType(CallType::UNKNOWN, Result(Result::Status::UNBLOCKED));
}

TEST_F(WfcBlockRuleTest, CheckReturnsUnblockedIfWfcAvailable)
{
    ON_CALL(objService, IsWlanIpCanType).WillByDefault(Return(IMS_TRUE));
    ON_CALL(objImsEventReceiver, GetWParam(IMS_EVENT_WFC_SETTING_CHANGED))
            .WillByDefault(Return(IMS_WFC_ON));

    ON_CALL(objImsEventReceiver, GetWParam(IMS_EVENT_IMS_VOICE_OVER_PS_STATE))
            .WillByDefault(Return(IMS_VOICE_OVER_PS_NOT_SUPPORTED));
    ON_CALL(objPassiveTimerHolder, IsActive(IPassiveTimerHolder::Type::SSAC_VOICE_BARRING))
            .WillByDefault(Return(IMS_TRUE));
    objSsacInfo.nBarringFactorForVoice = 0;

    AssertResultForCallType(CallType::VOIP, Result(Result::Status::UNBLOCKED));
    AssertResultForCallType(CallType::RTT, Result(Result::Status::UNBLOCKED));
    AssertResultForCallType(CallType::UNKNOWN, Result(Result::Status::UNBLOCKED));
}

TEST_F(WfcBlockRuleTest, CheckReturnsUnblockedIfVoiceCallsAreAvailableOnCellular)
{
    ON_CALL(objService, IsWlanIpCanType).WillByDefault(Return(IMS_TRUE));
    ON_CALL(objImsEventReceiver, GetWParam(IMS_EVENT_WFC_SETTING_CHANGED))
            .WillByDefault(Return(IMS_WFC_OFF));

    ON_CALL(objImsEventReceiver, GetWParam(IMS_EVENT_IMS_VOICE_OVER_PS_STATE))
            .WillByDefault(Return(IMS_VOICE_OVER_PS_SUPPORTED));
    ON_CALL(objPassiveTimerHolder, IsActive(IPassiveTimerHolder::Type::SSAC_VOICE_BARRING))
            .WillByDefault(Return(IMS_FALSE));
    objSsacInfo.nBarringFactorForVoice = 100;

    AssertResultForCallType(CallType::VOIP, Result(Result::Status::UNBLOCKED));
    AssertResultForCallType(CallType::RTT, Result(Result::Status::UNBLOCKED));
    AssertResultForCallType(CallType::UNKNOWN, Result(Result::Status::UNBLOCKED));
}

TEST_F(WfcBlockRuleTest, CheckReturnsUnblockedForVideoCalls)
{
    ON_CALL(objService, IsWlanIpCanType).WillByDefault(Return(IMS_TRUE));
    ON_CALL(objImsEventReceiver, GetWParam(IMS_EVENT_WFC_SETTING_CHANGED))
            .WillByDefault(Return(IMS_WFC_OFF));

    ON_CALL(objImsEventReceiver, GetWParam(IMS_EVENT_IMS_VOICE_OVER_PS_STATE))
            .WillByDefault(Return(IMS_VOICE_OVER_PS_NOT_SUPPORTED));
    ON_CALL(objPassiveTimerHolder, IsActive(IPassiveTimerHolder::Type::SSAC_VOICE_BARRING))
            .WillByDefault(Return(IMS_TRUE));
    objSsacInfo.nBarringFactorForVoice = 0;

    lstOtherCalls.Append(CreateMockIMtcCall(CallType::VOIP));
    ON_CALL(objContext, GetOtherCalls).WillByDefault(Return(lstOtherCalls));

    AssertResultForCallType(CallType::VT, Result(Result::Status::UNBLOCKED));
    AssertResultForCallType(CallType::VIDEO_RTT, Result(Result::Status::UNBLOCKED));
}

TEST_F(WfcBlockRuleTest, CheckReturnsBlockedForVideoCallsIfAlreadyHasVideoCall)
{
    ON_CALL(objService, IsWlanIpCanType).WillByDefault(Return(IMS_TRUE));
    ON_CALL(objImsEventReceiver, GetWParam(IMS_EVENT_WFC_SETTING_CHANGED))
            .WillByDefault(Return(IMS_WFC_OFF));

    ON_CALL(objImsEventReceiver, GetWParam(IMS_EVENT_IMS_VOICE_OVER_PS_STATE))
            .WillByDefault(Return(IMS_VOICE_OVER_PS_NOT_SUPPORTED));
    ON_CALL(objPassiveTimerHolder, IsActive(IPassiveTimerHolder::Type::SSAC_VOICE_BARRING))
            .WillByDefault(Return(IMS_TRUE));
    objSsacInfo.nBarringFactorForVoice = 0;

    lstOtherCalls.Append(CreateMockIMtcCall(CallType::VT));
    ON_CALL(objContext, GetOtherCalls).WillByDefault(Return(lstOtherCalls));

    AssertResultForCallType(CallType::VT,
            Result(Result::Status::BLOCKED,
                    CallReasonInfo(CODE_LOCAL_CALL_BUSY, EXTRA_CODE_VOWIFI_OFF)));
    AssertResultForCallType(CallType::VIDEO_RTT,
            Result(Result::Status::BLOCKED,
                    CallReasonInfo(CODE_LOCAL_CALL_BUSY, EXTRA_CODE_VOWIFI_OFF)));
}

TEST_F(WfcBlockRuleTest, CheckReturnsBlockedIfVoiceCallsAreUnavailableByVops)
{
    ON_CALL(objService, IsWlanIpCanType).WillByDefault(Return(IMS_TRUE));
    ON_CALL(objImsEventReceiver, GetWParam(IMS_EVENT_WFC_SETTING_CHANGED))
            .WillByDefault(Return(IMS_WFC_OFF));

    ON_CALL(objImsEventReceiver, GetWParam(IMS_EVENT_IMS_VOICE_OVER_PS_STATE))
            .WillByDefault(Return(IMS_VOICE_OVER_PS_NOT_SUPPORTED));
    ON_CALL(objPassiveTimerHolder, IsActive(IPassiveTimerHolder::Type::SSAC_VOICE_BARRING))
            .WillByDefault(Return(IMS_FALSE));
    objSsacInfo.nBarringFactorForVoice = 100;

    AssertResultForCallType(CallType::VOIP,
            Result(Result::Status::BLOCKED,
                    CallReasonInfo(CODE_LOCAL_CALL_BUSY, EXTRA_CODE_VOWIFI_OFF)));
    AssertResultForCallType(CallType::RTT,
            Result(Result::Status::BLOCKED,
                    CallReasonInfo(CODE_LOCAL_CALL_BUSY, EXTRA_CODE_VOWIFI_OFF)));
    AssertResultForCallType(CallType::UNKNOWN,
            Result(Result::Status::BLOCKED,
                    CallReasonInfo(CODE_LOCAL_CALL_BUSY, EXTRA_CODE_VOWIFI_OFF)));
}

TEST_F(WfcBlockRuleTest, CheckReturnsBlockedIfVoiceCallsAreUnavailableBySsacP00)
{
    ON_CALL(objService, IsWlanIpCanType).WillByDefault(Return(IMS_TRUE));
    ON_CALL(objImsEventReceiver, GetWParam(IMS_EVENT_WFC_SETTING_CHANGED))
            .WillByDefault(Return(IMS_WFC_OFF));

    ON_CALL(objImsEventReceiver, GetWParam(IMS_EVENT_IMS_VOICE_OVER_PS_STATE))
            .WillByDefault(Return(IMS_VOICE_OVER_PS_SUPPORTED));
    ON_CALL(objPassiveTimerHolder, IsActive(IPassiveTimerHolder::Type::SSAC_VOICE_BARRING))
            .WillByDefault(Return(IMS_FALSE));
    objSsacInfo.nBarringFactorForVoice = 0;

    AssertResultForCallType(CallType::VOIP,
            Result(Result::Status::BLOCKED,
                    CallReasonInfo(CODE_LOCAL_CALL_BUSY, EXTRA_CODE_VOWIFI_OFF)));
    AssertResultForCallType(CallType::RTT,
            Result(Result::Status::BLOCKED,
                    CallReasonInfo(CODE_LOCAL_CALL_BUSY, EXTRA_CODE_VOWIFI_OFF)));
    AssertResultForCallType(CallType::UNKNOWN,
            Result(Result::Status::BLOCKED,
                    CallReasonInfo(CODE_LOCAL_CALL_BUSY, EXTRA_CODE_VOWIFI_OFF)));
}

TEST_F(WfcBlockRuleTest, CheckReturnsBlockedIfVoiceCallsAreUnavailableBySsacTimer)
{
    ON_CALL(objService, IsWlanIpCanType).WillByDefault(Return(IMS_TRUE));
    ON_CALL(objImsEventReceiver, GetWParam(IMS_EVENT_WFC_SETTING_CHANGED))
            .WillByDefault(Return(IMS_WFC_OFF));

    ON_CALL(objImsEventReceiver, GetWParam(IMS_EVENT_IMS_VOICE_OVER_PS_STATE))
            .WillByDefault(Return(IMS_VOICE_OVER_PS_SUPPORTED));
    objSsacInfo.nBarringFactorForVoice = 60;
    ON_CALL(objPassiveTimerHolder, IsActive(IPassiveTimerHolder::Type::SSAC_VOICE_BARRING))
            .WillByDefault(Return(IMS_TRUE));

    AssertResultForCallType(CallType::VOIP,
            Result(Result::Status::BLOCKED,
                    CallReasonInfo(CODE_LOCAL_CALL_BUSY, EXTRA_CODE_VOWIFI_OFF)));
    AssertResultForCallType(CallType::RTT,
            Result(Result::Status::BLOCKED,
                    CallReasonInfo(CODE_LOCAL_CALL_BUSY, EXTRA_CODE_VOWIFI_OFF)));
    AssertResultForCallType(CallType::UNKNOWN,
            Result(Result::Status::BLOCKED,
                    CallReasonInfo(CODE_LOCAL_CALL_BUSY, EXTRA_CODE_VOWIFI_OFF)));
}
