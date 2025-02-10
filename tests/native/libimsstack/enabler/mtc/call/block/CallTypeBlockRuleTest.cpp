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

#include "CarrierConfig.h"
#include "MockISession.h"
#include "call/IMtcCall.h"
#include "call/MockIMtcCall.h"
#include "call/MockIMtcCallContext.h"
#include "call/MockIMtcCallManager.h"
#include "call/MockIMtcSession.h"
#include "call/UpdatingInfo.h"
#include "call/block/CallTypeBlockRule.h"
#include "call/block/MockIMtcBlockRule.h"
#include "configuration/MockMtcConfigurationProxy.h"
#include "configuration/MtcConfigurationProxy.h"
#include "utility/MockIMessageUtils.h"
#include <gmock/gmock.h>
#include <gtest/gtest.h>

using ::testing::_;
using ::testing::Return;
using ::testing::ReturnRef;
using Result = IMtcBlockRule::Result;

class CallTypeBlockRuleTest : public ::testing::Test
{
public:
    MockIMtcCallContext objContext;
    MockIMtcBlockRuleCheckListener objListener;
    CallInfo objCallInfo;
    ImsList<IMtcCall*> lstCalls;
    ImsList<IMtcCall*> lstOtherCalls;
    MockMtcConfigurationProxy* pConfigurationProxy;
    MockIMessageUtils objMessageUtils;
    MockIMtcSession objMtcSession;
    UpdatingInfo* pUpdatingInfo;
    MockISession objSession;
    MockIMtcCall objMtcCall;
    MockIMtcCallManager objCallManager;

protected:
    virtual void SetUp() override
    {
        pConfigurationProxy = new MockMtcConfigurationProxy();
        pUpdatingInfo = new UpdatingInfo(objContext);

        ON_CALL(objContext, GetConfigurationProxy).WillByDefault(ReturnRef(*pConfigurationProxy));
        ON_CALL(objContext, GetCallInfo).WillByDefault(ReturnRef(objCallInfo));
        ON_CALL(objContext, GetMessageUtils).WillByDefault(ReturnRef(objMessageUtils));
        ON_CALL(objContext, GetSession()).WillByDefault(Return(&objMtcSession));
        ON_CALL(objContext, GetUpdatingInfo).WillByDefault(ReturnRef(*pUpdatingInfo));
        ON_CALL(objContext, GetCall).WillByDefault(ReturnRef(objMtcCall));
        ON_CALL(objContext, GetCallManager).WillByDefault(ReturnRef(objCallManager));

        ON_CALL(objMtcSession, GetISession()).WillByDefault(ReturnRef(objSession));
    }

    virtual void TearDown() override
    {
        delete pConfigurationProxy;
        delete pUpdatingInfo;

        for (IMS_UINT32 nIndex = 0; nIndex < lstOtherCalls.GetSize(); nIndex++)
        {
            delete lstOtherCalls.GetAt(nIndex);
        }
        lstOtherCalls.Clear();

        for (IMS_UINT32 nIndex = 0; nIndex < lstCalls.GetSize(); nIndex++)
        {
            delete lstCalls.GetAt(nIndex);
        }
        lstCalls.Clear();
    }

    MockIMtcCall* CreateMockIMtcCall(CallType eCallType)
    {
        MockIMtcCall* pCall = new MockIMtcCall();

        ON_CALL(*pCall, GetCallType).WillByDefault(Return(eCallType));

        return pCall;
    }

    CallTypeBlockRule GetRuleByTargetCallType(IN CallType eTargetCallType)
    {
        ON_CALL(objMessageUtils, GetCallTypeFromSdp(_, _, _, _))
                .WillByDefault(Return(eTargetCallType));
        ON_CALL(objMtcSession, GetCallType).WillByDefault(Return(eTargetCallType));
        pUpdatingInfo->SetTargetCallType(eTargetCallType);
        return CallTypeBlockRule(objContext);
    }
};

TEST_F(CallTypeBlockRuleTest, CheckReturnsBlockedForVideoRttIfNotAllowed)
{
    ON_CALL(*pConfigurationProxy, GetInt(ConfigVt::KEY_POLICY_FOR_TEXT_WITH_VIDEO_INT))
            .WillByDefault(Return(ConfigVt::TEXT_VIDEO_NOT_ALLOWED));

    CallTypeBlockRule objBlockRule(GetRuleByTargetCallType(CallType::VIDEO_RTT));

    {
        ON_CALL(objMtcCall, GetState).WillByDefault(Return(IMtcCall::State::ESTABLISHED));
        Result objResult = objBlockRule.Check(objListener);

        EXPECT_EQ(Result::Status::BLOCKED, objResult.eStatus);
        EXPECT_EQ(CallReasonInfo(CODE_SIP_NOT_ACCEPTABLE, EXTRA_CODE_NOT_ACCEPTABLE_BY_CALL_TYPE),
                objResult.objReason);
    }
    {
        ON_CALL(objMtcCall, GetState).WillByDefault(Return(IMtcCall::State::IDLE));
        Result objResult = objBlockRule.Check(objListener);

        EXPECT_EQ(Result::Status::BLOCKED, objResult.eStatus);
        EXPECT_EQ(CallReasonInfo(CODE_SIP_NOT_ACCEPTABLE, EXTRA_CODE_NOT_ACCEPTABLE_BY_CALL_TYPE),
                objResult.objReason);
    }
}

TEST_F(CallTypeBlockRuleTest, CheckReturnsBlockedVideoPortZeroAndTextIfConfigIsNotAllowed)
{
    ON_CALL(objMtcCall, GetState).WillByDefault(Return(IMtcCall::State::ESTABLISHED));
    ON_CALL(*pConfigurationProxy, GetInt(ConfigVt::KEY_POLICY_FOR_TEXT_WITH_VIDEO_INT))
            .WillByDefault(Return(ConfigVt::TEXT_VIDEO_NOT_ALLOWED));

    CallTypeBlockRule objBlockRule(GetRuleByTargetCallType(CallType::VIDEO_RTT));

    // video port = 0
    pUpdatingInfo->SetTargetCallType(CallType::RTT);

    Result objResult = objBlockRule.Check(objListener);

    EXPECT_EQ(Result::Status::BLOCKED, objResult.eStatus);
    EXPECT_EQ(CallReasonInfo(CODE_SIP_NOT_ACCEPTABLE, EXTRA_CODE_NOT_ACCEPTABLE_BY_CALL_TYPE),
            objResult.objReason);
}

TEST_F(CallTypeBlockRuleTest, CheckReturnsUnblockedVideoPortZeroAndTextIfConfigIsNotAllowedIfActive)
{
    ON_CALL(objMtcCall, GetState).WillByDefault(Return(IMtcCall::State::ESTABLISHED));
    ON_CALL(*pConfigurationProxy, GetInt(ConfigVt::KEY_POLICY_FOR_TEXT_WITH_VIDEO_INT))
            .WillByDefault(Return(ConfigVt::TEXT_VIDEO_NOT_ALLOWED_IF_ACTIVE));
    ON_CALL(*pConfigurationProxy,
            GetBoolean(ConfigVoice::KEY_ALLOW_MULTIPLE_CALL_INCLUDING_VIDEO_CALL_BOOL))
            .WillByDefault(Return(IMS_TRUE));

    CallTypeBlockRule objBlockRule(GetRuleByTargetCallType(CallType::VIDEO_RTT));

    // video port = 0
    pUpdatingInfo->SetTargetCallType(CallType::RTT);

    Result objResult = objBlockRule.Check(objListener);

    EXPECT_EQ(Result::Status::UNBLOCKED, objResult.eStatus);
}

TEST_F(CallTypeBlockRuleTest, CheckReturnsUnblockedIfAllowed)
{
    ON_CALL(*pConfigurationProxy, GetInt(ConfigVt::KEY_POLICY_FOR_TEXT_WITH_VIDEO_INT))
            .WillByDefault(Return(ConfigVt::TEXT_VIDEO_ALLOWED));
    ON_CALL(*pConfigurationProxy,
            GetBoolean(ConfigVoice::KEY_ALLOW_MULTIPLE_CALL_INCLUDING_VIDEO_CALL_BOOL))
            .WillByDefault(Return(IMS_TRUE));

    EXPECT_CALL(objContext, GetOtherCalls).Times(0);

    CallTypeBlockRule objBlockRuleForVoip(GetRuleByTargetCallType(CallType::VOIP));
    EXPECT_EQ(Result::Status::UNBLOCKED, objBlockRuleForVoip.Check(objListener).eStatus);

    CallTypeBlockRule objBlockRuleForVt(GetRuleByTargetCallType(CallType::VT));
    EXPECT_EQ(Result::Status::UNBLOCKED, objBlockRuleForVt.Check(objListener).eStatus);

    CallTypeBlockRule objBlockRuleForRtt(GetRuleByTargetCallType(CallType::RTT));
    EXPECT_EQ(Result::Status::UNBLOCKED, objBlockRuleForRtt.Check(objListener).eStatus);

    CallTypeBlockRule objBlockRuleForVideoRtt(GetRuleByTargetCallType(CallType::VIDEO_RTT));
    EXPECT_EQ(Result::Status::UNBLOCKED, objBlockRuleForVideoRtt.Check(objListener).eStatus);
}

TEST_F(CallTypeBlockRuleTest, CheckReturnsUnblockedIfNoOtherCallExists)
{
    ON_CALL(*pConfigurationProxy, GetInt(ConfigVt::KEY_POLICY_FOR_TEXT_WITH_VIDEO_INT))
            .WillByDefault(Return(ConfigVt::TEXT_VIDEO_ALLOWED));
    ON_CALL(*pConfigurationProxy,
            GetBoolean(ConfigVoice::KEY_ALLOW_MULTIPLE_CALL_INCLUDING_VIDEO_CALL_BOOL))
            .WillByDefault(Return(IMS_FALSE));

    // no other call
    ON_CALL(objContext, GetOtherCalls).WillByDefault(Return(lstOtherCalls));

    CallTypeBlockRule objBlockRuleForVoip(GetRuleByTargetCallType(CallType::VOIP));
    EXPECT_EQ(Result::Status::UNBLOCKED, objBlockRuleForVoip.Check(objListener).eStatus);

    CallTypeBlockRule objBlockRuleForVt(GetRuleByTargetCallType(CallType::VT));
    EXPECT_EQ(Result::Status::UNBLOCKED, objBlockRuleForVt.Check(objListener).eStatus);

    CallTypeBlockRule objBlockRuleForRtt(GetRuleByTargetCallType(CallType::RTT));
    EXPECT_EQ(Result::Status::UNBLOCKED, objBlockRuleForRtt.Check(objListener).eStatus);

    CallTypeBlockRule objBlockRuleForVideoRtt(GetRuleByTargetCallType(CallType::VIDEO_RTT));
    EXPECT_EQ(Result::Status::UNBLOCKED, objBlockRuleForVideoRtt.Check(objListener).eStatus);
}

TEST_F(CallTypeBlockRuleTest, CheckReturnsBlockedForVideoCallIfVoipExists)
{
    lstOtherCalls.Append(CreateMockIMtcCall(CallType::VOIP));
    ON_CALL(objContext, GetOtherCalls).WillByDefault(Return(lstOtherCalls));

    ON_CALL(*pConfigurationProxy, GetInt(ConfigVt::KEY_POLICY_FOR_TEXT_WITH_VIDEO_INT))
            .WillByDefault(Return(ConfigVt::TEXT_VIDEO_ALLOWED));
    ON_CALL(*pConfigurationProxy,
            GetBoolean(ConfigVoice::KEY_ALLOW_MULTIPLE_CALL_INCLUDING_VIDEO_CALL_BOOL))
            .WillByDefault(Return(IMS_FALSE));

    ON_CALL(objMtcCall, GetState).WillByDefault(Return(IMtcCall::State::ESTABLISHED));
    {
        objCallInfo.ePeerType = PeerType::MO;

        CallTypeBlockRule objBlockRuleForVt(GetRuleByTargetCallType(CallType::VT));
        Result objResultForVt = objBlockRuleForVt.Check(objListener);
        EXPECT_EQ(Result::Status::BLOCKED, objResultForVt.eStatus);
        EXPECT_EQ(CallReasonInfo(CODE_LOCAL_CALL_EXCEEDED), objResultForVt.objReason);

        CallTypeBlockRule objBlockRuleForVideoRtt(GetRuleByTargetCallType(CallType::VIDEO_RTT));
        Result objResultForVideoRtt = objBlockRuleForVideoRtt.Check(objListener);
        EXPECT_EQ(Result::Status::BLOCKED, objResultForVideoRtt.eStatus);
        EXPECT_EQ(CallReasonInfo(CODE_LOCAL_CALL_EXCEEDED), objResultForVideoRtt.objReason);
    }
    {
        objCallInfo.ePeerType = PeerType::MT;

        CallTypeBlockRule objBlockRuleForVt(GetRuleByTargetCallType(CallType::VT));
        Result objResultForVt = objBlockRuleForVt.Check(objListener);
        EXPECT_EQ(Result::Status::BLOCKED, objResultForVt.eStatus);
        EXPECT_EQ(CallReasonInfo(CODE_REJECT_MAX_CALL_LIMIT_REACHED), objResultForVt.objReason);

        CallTypeBlockRule objBlockRuleForVideoRtt(GetRuleByTargetCallType(CallType::VIDEO_RTT));
        Result objResultForVideoRtt = objBlockRuleForVideoRtt.Check(objListener);
        EXPECT_EQ(Result::Status::BLOCKED, objResultForVideoRtt.eStatus);
        EXPECT_EQ(
                CallReasonInfo(CODE_REJECT_MAX_CALL_LIMIT_REACHED), objResultForVideoRtt.objReason);
    }
}

TEST_F(CallTypeBlockRuleTest, CheckReturnsUnblockedForVoipCallIfVoipExists)
{
    lstOtherCalls.Append(CreateMockIMtcCall(CallType::VOIP));
    ON_CALL(objContext, GetOtherCalls).WillByDefault(Return(lstOtherCalls));

    ON_CALL(*pConfigurationProxy, GetInt(ConfigVt::KEY_POLICY_FOR_TEXT_WITH_VIDEO_INT))
            .WillByDefault(Return(ConfigVt::TEXT_VIDEO_ALLOWED));
    ON_CALL(*pConfigurationProxy,
            GetBoolean(ConfigVoice::KEY_ALLOW_MULTIPLE_CALL_INCLUDING_VIDEO_CALL_BOOL))
            .WillByDefault(Return(IMS_FALSE));

    {
        ON_CALL(objMtcCall, GetState).WillByDefault(Return(IMtcCall::State::IDLE));
        CallTypeBlockRule objBlockRuleForVt(GetRuleByTargetCallType(CallType::VOIP));
        Result objResultForVt = objBlockRuleForVt.Check(objListener);
        EXPECT_EQ(Result::Status::UNBLOCKED, objResultForVt.eStatus);
    }
    {
        ON_CALL(objMtcCall, GetState).WillByDefault(Return(IMtcCall::State::ESTABLISHED));
        CallTypeBlockRule objBlockRuleForVt(GetRuleByTargetCallType(CallType::VOIP));
        Result objResultForVt = objBlockRuleForVt.Check(objListener);
        EXPECT_EQ(Result::Status::UNBLOCKED, objResultForVt.eStatus);
    }
}

TEST_F(CallTypeBlockRuleTest, CheckReturnsBlockedForVoipIfVtExists)
{
    lstOtherCalls.Append(CreateMockIMtcCall(CallType::VT));
    ON_CALL(objContext, GetOtherCalls).WillByDefault(Return(lstOtherCalls));

    ON_CALL(*pConfigurationProxy, GetInt(ConfigVt::KEY_POLICY_FOR_TEXT_WITH_VIDEO_INT))
            .WillByDefault(Return(ConfigVt::TEXT_VIDEO_ALLOWED));
    ON_CALL(*pConfigurationProxy,
            GetBoolean(ConfigVoice::KEY_ALLOW_MULTIPLE_CALL_INCLUDING_VIDEO_CALL_BOOL))
            .WillByDefault(Return(IMS_FALSE));

    {
        objCallInfo.ePeerType = PeerType::MO;

        CallTypeBlockRule objBlockRule(GetRuleByTargetCallType(CallType::VOIP));
        Result objResultForVt = objBlockRule.Check(objListener);
        EXPECT_EQ(Result::Status::BLOCKED, objResultForVt.eStatus);
        EXPECT_EQ(CallReasonInfo(CODE_LOCAL_CALL_EXCEEDED), objResultForVt.objReason);
    }
    {
        objCallInfo.ePeerType = PeerType::MT;

        CallTypeBlockRule objBlockRule(GetRuleByTargetCallType(CallType::VOIP));
        Result objResultForVt = objBlockRule.Check(objListener);
        EXPECT_EQ(Result::Status::BLOCKED, objResultForVt.eStatus);
        EXPECT_EQ(CallReasonInfo(CODE_REJECT_MAX_CALL_LIMIT_REACHED), objResultForVt.objReason);
    }
}

TEST_F(CallTypeBlockRuleTest, CheckReturnsBlockedForVoipIfVideoRttExists)
{
    lstOtherCalls.Append(CreateMockIMtcCall(CallType::VIDEO_RTT));
    ON_CALL(objContext, GetOtherCalls).WillByDefault(Return(lstOtherCalls));

    ON_CALL(*pConfigurationProxy, GetInt(ConfigVt::KEY_POLICY_FOR_TEXT_WITH_VIDEO_INT))
            .WillByDefault(Return(ConfigVt::TEXT_VIDEO_ALLOWED));
    ON_CALL(*pConfigurationProxy,
            GetBoolean(ConfigVoice::KEY_ALLOW_MULTIPLE_CALL_INCLUDING_VIDEO_CALL_BOOL))
            .WillByDefault(Return(IMS_FALSE));

    {
        objCallInfo.ePeerType = PeerType::MO;

        CallTypeBlockRule objBlockRule(GetRuleByTargetCallType(CallType::VOIP));
        Result objResultForVt = objBlockRule.Check(objListener);
        EXPECT_EQ(Result::Status::BLOCKED, objResultForVt.eStatus);
        EXPECT_EQ(CallReasonInfo(CODE_LOCAL_CALL_EXCEEDED), objResultForVt.objReason);
    }
    {
        objCallInfo.ePeerType = PeerType::MT;

        CallTypeBlockRule objBlockRule(GetRuleByTargetCallType(CallType::VOIP));
        Result objResultForVt = objBlockRule.Check(objListener);
        EXPECT_EQ(Result::Status::BLOCKED, objResultForVt.eStatus);
        EXPECT_EQ(CallReasonInfo(CODE_REJECT_MAX_CALL_LIMIT_REACHED), objResultForVt.objReason);
    }
}

TEST_F(CallTypeBlockRuleTest, CheckReturnsBlockedForConferenceDuringRtt)
{
    lstCalls.Append(CreateMockIMtcCall(CallType::RTT));
    ON_CALL(objCallManager, GetCalls).WillByDefault(Return(lstCalls));

    ON_CALL(objMtcCall, GetState).WillByDefault(Return(IMtcCall::State::IDLE));
    objCallInfo.bConference = IMS_TRUE;

    CallTypeBlockRule objBlockRule(GetRuleByTargetCallType(CallType::VOIP));
    Result objResult = objBlockRule.Check(objListener);
    EXPECT_EQ(Result::Status::BLOCKED, objResult.eStatus);
    EXPECT_EQ(CallReasonInfo(CODE_LOCAL_CALL_BUSY, EXTRA_CODE_RTT_ON), objResult.objReason);
}
