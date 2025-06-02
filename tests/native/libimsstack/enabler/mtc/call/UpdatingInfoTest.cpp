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

#include "MockISession.h"
#include "MtcDef.h"
#include "call/IMtcCall.h"
#include "call/MockIMtcCallContext.h"
#include "call/MockIMtcSession.h"
#include "call/UpdatingInfo.h"
#include "media/MockIMtcMediaManager.h"
#include <gtest/gtest.h>

using ::testing::Return;
using ::testing::ReturnRef;

class UpdatingInfoTest : public ::testing::Test
{
public:
    MockIMtcCallContext objCallContext;
    MockIMtcSession objSession;
    MockIMtcMediaManager objMediaManager;
    MockISession objISession;
    UpdatingInfo* pUpdatingInfo;

protected:
    virtual void SetUp() override
    {
        ON_CALL(objCallContext, GetSession()).WillByDefault(Return(&objSession));
        ON_CALL(objSession, GetISession()).WillByDefault(ReturnRef(objISession));
        ON_CALL(objCallContext, GetMediaManager()).WillByDefault(ReturnRef(objMediaManager));
    }

    virtual void TearDown() override { delete pUpdatingInfo; }
};

TEST_F(UpdatingInfoTest, GetTargetCallTypeReturnsUnknownInitially)
{
    pUpdatingInfo = new UpdatingInfo(objCallContext);
    EXPECT_EQ(CallType::UNKNOWN, pUpdatingInfo->GetTargetCallType());
}

TEST_F(UpdatingInfoTest, GetTargetCallTypeReturnsSetValue)
{
    pUpdatingInfo = new UpdatingInfo(objCallContext);
    CallType eCallType = CallType::VOIP;

    pUpdatingInfo->SetTargetCallType(eCallType);

    EXPECT_EQ(eCallType, pUpdatingInfo->GetTargetCallType());
}

TEST_F(UpdatingInfoTest, GetModifyingInfoReturnsNotNull)
{
    pUpdatingInfo = new UpdatingInfo(objCallContext);
    EXPECT_NE(nullptr, &pUpdatingInfo->GetModifyingInfo());
}

TEST_F(UpdatingInfoTest, GetAlertingInfoReturnsNotNull)
{
    pUpdatingInfo = new UpdatingInfo(objCallContext);
    EXPECT_NE(nullptr, &pUpdatingInfo->GetAlertingInfo());
}

TEST_F(UpdatingInfoTest, GetModifiedInfoReturnsNotNull)
{
    pUpdatingInfo = new UpdatingInfo(objCallContext);
    EXPECT_NE(nullptr, &pUpdatingInfo->GetModifiedInfo());
}

TEST_F(UpdatingInfoTest, IsModifierReturnsFalseInitially)
{
    pUpdatingInfo = new UpdatingInfo(objCallContext);
    EXPECT_EQ(IMS_FALSE, pUpdatingInfo->IsModifier());
}

TEST_F(UpdatingInfoTest, IsModifierReturnsTrueAfterSet)
{
    pUpdatingInfo = new UpdatingInfo(objCallContext);
    pUpdatingInfo->SetModifier();
    EXPECT_EQ(IMS_TRUE, pUpdatingInfo->IsModifier());
}

TEST_F(UpdatingInfoTest, IsAlertedReturnsFalseInitially)
{
    pUpdatingInfo = new UpdatingInfo(objCallContext);
    EXPECT_EQ(IMS_FALSE, pUpdatingInfo->IsAlerted());
}

TEST_F(UpdatingInfoTest, IsAlertedReturnsTrueAfterSet)
{
    pUpdatingInfo = new UpdatingInfo(objCallContext);
    pUpdatingInfo->SetAlerted();
    EXPECT_EQ(IMS_TRUE, pUpdatingInfo->IsAlerted());
}

TEST_F(UpdatingInfoTest, GetModifiedMediaInfoWithOriginalAudioDirReturnsCorrectInfo)
{
    pUpdatingInfo = new UpdatingInfo(objCallContext);
    MediaInfo& objOriginalInfo = pUpdatingInfo->GetOriginalInfo();
    MediaInfo& objModifiedInfo = pUpdatingInfo->GetModifiedInfo();

    objOriginalInfo.eAudioDirection = DIRECTION_SEND;
    objModifiedInfo.eAudioDirection = DIRECTION_SEND_RECEIVE;

    EXPECT_EQ(DIRECTION_SEND,
            pUpdatingInfo->GetModifiedMediaInfoWithOriginalAudioDir().eAudioDirection);
}

TEST_F(UpdatingInfoTest, ReturnHoldResumeStatusIfModifyingSendReceiveToHeld)
{
    pUpdatingInfo = new UpdatingInfo(objCallContext);
    MediaInfo& objOriginalInfo = pUpdatingInfo->GetOriginalInfo();
    MediaInfo& objModifyingInfo = pUpdatingInfo->GetModifyingInfo();

    objOriginalInfo.eAudioDirection = DIRECTION_SEND_RECEIVE;
    objModifyingInfo.eAudioDirection = DIRECTION_SEND;

    EXPECT_EQ(IMS_TRUE, pUpdatingInfo->IsHeld());
    EXPECT_EQ(IMS_FALSE, pUpdatingInfo->IsHeldBy());
    EXPECT_EQ(IMS_FALSE, pUpdatingInfo->IsResumed());
    EXPECT_EQ(IMS_FALSE, pUpdatingInfo->IsResumedBy());
}

TEST_F(UpdatingInfoTest, ReturnHoldResumeStatusIfModifyingSendReceiveToInactive)
{
    pUpdatingInfo = new UpdatingInfo(objCallContext);
    MediaInfo& objOriginalInfo = pUpdatingInfo->GetOriginalInfo();
    MediaInfo& objModifyingInfo = pUpdatingInfo->GetModifyingInfo();

    objOriginalInfo.eAudioDirection = DIRECTION_SEND_RECEIVE;
    objModifyingInfo.eAudioDirection = DIRECTION_INACTIVE;

    EXPECT_EQ(IMS_TRUE, pUpdatingInfo->IsHeld());
    EXPECT_EQ(IMS_FALSE, pUpdatingInfo->IsHeldBy());
    EXPECT_EQ(IMS_FALSE, pUpdatingInfo->IsResumed());
    EXPECT_EQ(IMS_FALSE, pUpdatingInfo->IsResumedBy());
}

TEST_F(UpdatingInfoTest, ReturnHoldResumeStatusIfModifyingReceiveToInactive)
{
    pUpdatingInfo = new UpdatingInfo(objCallContext);
    MediaInfo& objOriginalInfo = pUpdatingInfo->GetOriginalInfo();
    MediaInfo& objModifyingInfo = pUpdatingInfo->GetModifyingInfo();

    objOriginalInfo.eAudioDirection = DIRECTION_RECEIVE;
    objModifyingInfo.eAudioDirection = DIRECTION_INACTIVE;

    EXPECT_EQ(IMS_TRUE, pUpdatingInfo->IsHeld());
    EXPECT_EQ(IMS_FALSE, pUpdatingInfo->IsHeldBy());
    EXPECT_EQ(IMS_FALSE, pUpdatingInfo->IsResumed());
    EXPECT_EQ(IMS_FALSE, pUpdatingInfo->IsResumedBy());
}

TEST_F(UpdatingInfoTest, ReturnHoldResumeStatusIfModifiedSendReceiveToReceive)
{
    pUpdatingInfo = new UpdatingInfo(objCallContext);
    MediaInfo& objOriginalInfo = pUpdatingInfo->GetOriginalInfo();
    MediaInfo& objModifiedInfo = pUpdatingInfo->GetModifiedInfo();

    objOriginalInfo.eAudioDirection = DIRECTION_SEND_RECEIVE;
    objModifiedInfo.eAudioDirection = DIRECTION_RECEIVE;

    EXPECT_EQ(IMS_FALSE, pUpdatingInfo->IsHeld());
    EXPECT_EQ(IMS_TRUE, pUpdatingInfo->IsHeldBy());
    EXPECT_EQ(IMS_FALSE, pUpdatingInfo->IsResumed());
    EXPECT_EQ(IMS_FALSE, pUpdatingInfo->IsResumedBy());

    EXPECT_EQ(IMS_FALSE, pUpdatingInfo->IsNeedToAlert());
}

TEST_F(UpdatingInfoTest, ReturnHoldResumeStatusIfModifiedSendReceiveToInactive)
{
    pUpdatingInfo = new UpdatingInfo(objCallContext);
    MediaInfo& objOriginalInfo = pUpdatingInfo->GetOriginalInfo();
    MediaInfo& objModifiedInfo = pUpdatingInfo->GetModifiedInfo();
    MediaInfo& objModifyingInfo = pUpdatingInfo->GetModifyingInfo();
    objModifyingInfo.eAudioDirection = DIRECTION_INVALID;

    objOriginalInfo.eAudioDirection = DIRECTION_SEND_RECEIVE;
    objModifiedInfo.eAudioDirection = DIRECTION_INACTIVE;

    EXPECT_EQ(IMS_FALSE, pUpdatingInfo->IsHeld());
    EXPECT_EQ(IMS_TRUE, pUpdatingInfo->IsHeldBy());
    EXPECT_EQ(IMS_FALSE, pUpdatingInfo->IsResumed());
    EXPECT_EQ(IMS_FALSE, pUpdatingInfo->IsResumedBy());

    EXPECT_EQ(IMS_FALSE, pUpdatingInfo->IsNeedToAlert());
}

TEST_F(UpdatingInfoTest, ReturnHoldResumeStatusIfModifiedSendToInactive)
{
    pUpdatingInfo = new UpdatingInfo(objCallContext);
    MediaInfo& objOriginalInfo = pUpdatingInfo->GetOriginalInfo();
    MediaInfo& objModifiedInfo = pUpdatingInfo->GetModifiedInfo();

    objOriginalInfo.eAudioDirection = DIRECTION_SEND;
    objModifiedInfo.eAudioDirection = DIRECTION_INACTIVE;

    EXPECT_EQ(IMS_FALSE, pUpdatingInfo->IsHeld());
    EXPECT_EQ(IMS_TRUE, pUpdatingInfo->IsHeldBy());
    EXPECT_EQ(IMS_FALSE, pUpdatingInfo->IsResumed());
    EXPECT_EQ(IMS_FALSE, pUpdatingInfo->IsResumedBy());

    EXPECT_EQ(IMS_FALSE, pUpdatingInfo->IsNeedToAlert());
}

TEST_F(UpdatingInfoTest, ReturnHoldResumeStatusIfAlertingSendReceiveToReceive)
{
    pUpdatingInfo = new UpdatingInfo(objCallContext);
    MediaInfo& objOriginalInfo = pUpdatingInfo->GetOriginalInfo();
    MediaInfo& objAlertingInfo = pUpdatingInfo->GetAlertingInfo();
    MediaInfo& objModifiedInfo = pUpdatingInfo->GetModifiedInfo();
    objModifiedInfo.eAudioDirection = DIRECTION_INVALID;

    objOriginalInfo.eAudioDirection = DIRECTION_SEND_RECEIVE;
    objAlertingInfo.eAudioDirection = DIRECTION_RECEIVE;

    EXPECT_EQ(IMS_FALSE, pUpdatingInfo->IsHeld());
    EXPECT_EQ(IMS_TRUE, pUpdatingInfo->IsHeldBy());
    EXPECT_EQ(IMS_FALSE, pUpdatingInfo->IsResumed());
    EXPECT_EQ(IMS_FALSE, pUpdatingInfo->IsResumedBy());

    EXPECT_EQ(IMS_FALSE, pUpdatingInfo->IsNeedToAlert());
}

TEST_F(UpdatingInfoTest, ReturnHoldResumeStatusIfAlertingSendReceiveToInactive)
{
    pUpdatingInfo = new UpdatingInfo(objCallContext);
    MediaInfo& objOriginalInfo = pUpdatingInfo->GetOriginalInfo();
    MediaInfo& objAlertingInfo = pUpdatingInfo->GetAlertingInfo();
    MediaInfo& objModifiedInfo = pUpdatingInfo->GetModifiedInfo();
    MediaInfo& objModifyingInfo = pUpdatingInfo->GetModifyingInfo();
    objModifiedInfo.eAudioDirection = DIRECTION_INVALID;
    objModifyingInfo.eAudioDirection = DIRECTION_INVALID;

    objOriginalInfo.eAudioDirection = DIRECTION_SEND_RECEIVE;
    objAlertingInfo.eAudioDirection = DIRECTION_INACTIVE;

    EXPECT_EQ(IMS_FALSE, pUpdatingInfo->IsHeld());
    EXPECT_EQ(IMS_TRUE, pUpdatingInfo->IsHeldBy());
    EXPECT_EQ(IMS_FALSE, pUpdatingInfo->IsResumed());
    EXPECT_EQ(IMS_FALSE, pUpdatingInfo->IsResumedBy());

    EXPECT_EQ(IMS_FALSE, pUpdatingInfo->IsNeedToAlert());
}

TEST_F(UpdatingInfoTest, ReturnHoldResumeStatusIfAlertingSendToInactive)
{
    pUpdatingInfo = new UpdatingInfo(objCallContext);
    MediaInfo& objOriginalInfo = pUpdatingInfo->GetOriginalInfo();
    MediaInfo& objAlertingInfo = pUpdatingInfo->GetAlertingInfo();
    MediaInfo& objModifiedInfo = pUpdatingInfo->GetModifiedInfo();
    objModifiedInfo.eAudioDirection = DIRECTION_INVALID;

    objOriginalInfo.eAudioDirection = DIRECTION_SEND;
    objAlertingInfo.eAudioDirection = DIRECTION_INACTIVE;

    EXPECT_EQ(IMS_FALSE, pUpdatingInfo->IsHeld());
    EXPECT_EQ(IMS_TRUE, pUpdatingInfo->IsHeldBy());
    EXPECT_EQ(IMS_FALSE, pUpdatingInfo->IsResumed());
    EXPECT_EQ(IMS_FALSE, pUpdatingInfo->IsResumedBy());

    EXPECT_EQ(IMS_FALSE, pUpdatingInfo->IsNeedToAlert());
}

TEST_F(UpdatingInfoTest, ReturnHoldResumeStatusIfModifyingSendToSendReceive)
{
    pUpdatingInfo = new UpdatingInfo(objCallContext);
    MediaInfo& objOriginalInfo = pUpdatingInfo->GetOriginalInfo();
    MediaInfo& objModifyingInfo = pUpdatingInfo->GetModifyingInfo();

    objOriginalInfo.eAudioDirection = DIRECTION_SEND;
    objModifyingInfo.eAudioDirection = DIRECTION_SEND_RECEIVE;

    EXPECT_EQ(IMS_FALSE, pUpdatingInfo->IsHeld());
    EXPECT_EQ(IMS_FALSE, pUpdatingInfo->IsHeldBy());
    EXPECT_EQ(IMS_TRUE, pUpdatingInfo->IsResumed());
    EXPECT_EQ(IMS_FALSE, pUpdatingInfo->IsResumedBy());
}

TEST_F(UpdatingInfoTest, ReturnHoldResumeStatusIfModifyingInactiveToReceive)
{
    pUpdatingInfo = new UpdatingInfo(objCallContext);
    MediaInfo& objOriginalInfo = pUpdatingInfo->GetOriginalInfo();
    MediaInfo& objModifyingInfo = pUpdatingInfo->GetModifyingInfo();

    objOriginalInfo.eAudioDirection = DIRECTION_INACTIVE;
    objModifyingInfo.eAudioDirection = DIRECTION_RECEIVE;

    EXPECT_EQ(IMS_FALSE, pUpdatingInfo->IsHeld());
    EXPECT_EQ(IMS_FALSE, pUpdatingInfo->IsHeldBy());
    EXPECT_EQ(IMS_TRUE, pUpdatingInfo->IsResumed());
    EXPECT_EQ(IMS_FALSE, pUpdatingInfo->IsResumedBy());
}

TEST_F(UpdatingInfoTest, ReturnHoldResumeStatusIfModifyingInactiveToSendReceive)
{
    pUpdatingInfo = new UpdatingInfo(objCallContext);
    MediaInfo& objOriginalInfo = pUpdatingInfo->GetOriginalInfo();
    MediaInfo& objModifyingInfo = pUpdatingInfo->GetModifyingInfo();

    objOriginalInfo.eAudioDirection = DIRECTION_INACTIVE;
    objModifyingInfo.eAudioDirection = DIRECTION_SEND_RECEIVE;

    EXPECT_EQ(IMS_FALSE, pUpdatingInfo->IsHeld());
    EXPECT_EQ(IMS_FALSE, pUpdatingInfo->IsHeldBy());
    EXPECT_EQ(IMS_TRUE, pUpdatingInfo->IsResumed());
    EXPECT_EQ(IMS_FALSE, pUpdatingInfo->IsResumedBy());
}

TEST_F(UpdatingInfoTest, ReturnHoldResumeStatusIfModifiedReceiveToSendReceive)
{
    pUpdatingInfo = new UpdatingInfo(objCallContext);
    MediaInfo& objOriginalInfo = pUpdatingInfo->GetOriginalInfo();
    MediaInfo& objModifiedInfo = pUpdatingInfo->GetModifiedInfo();

    objModifiedInfo.eAudioDirection = DIRECTION_SEND_RECEIVE;
    objOriginalInfo.eAudioDirection = DIRECTION_RECEIVE;

    EXPECT_EQ(IMS_FALSE, pUpdatingInfo->IsHeld());
    EXPECT_EQ(IMS_FALSE, pUpdatingInfo->IsHeldBy());
    EXPECT_EQ(IMS_FALSE, pUpdatingInfo->IsResumed());
    EXPECT_EQ(IMS_TRUE, pUpdatingInfo->IsResumedBy());

    EXPECT_EQ(IMS_FALSE, pUpdatingInfo->IsNeedToAlert());
}

TEST_F(UpdatingInfoTest, ReturnHoldResumeStatusIfModifiedInactiveToSend)
{
    pUpdatingInfo = new UpdatingInfo(objCallContext);
    MediaInfo& objOriginalInfo = pUpdatingInfo->GetOriginalInfo();
    MediaInfo& objModifiedInfo = pUpdatingInfo->GetModifiedInfo();

    objOriginalInfo.eAudioDirection = DIRECTION_INACTIVE;
    objModifiedInfo.eAudioDirection = DIRECTION_SEND;

    EXPECT_EQ(IMS_FALSE, pUpdatingInfo->IsHeld());
    EXPECT_EQ(IMS_FALSE, pUpdatingInfo->IsHeldBy());
    EXPECT_EQ(IMS_FALSE, pUpdatingInfo->IsResumed());
    EXPECT_EQ(IMS_TRUE, pUpdatingInfo->IsResumedBy());

    EXPECT_EQ(IMS_FALSE, pUpdatingInfo->IsNeedToAlert());
}

TEST_F(UpdatingInfoTest, ReturnHoldResumeStatusIfModifiedInactiveToSendReceive)
{
    pUpdatingInfo = new UpdatingInfo(objCallContext);
    MediaInfo& objOriginalInfo = pUpdatingInfo->GetOriginalInfo();
    MediaInfo& objModifiedInfo = pUpdatingInfo->GetModifiedInfo();
    MediaInfo& objModifyingInfo = pUpdatingInfo->GetModifyingInfo();
    objModifyingInfo.eAudioDirection = DIRECTION_INVALID;

    objOriginalInfo.eAudioDirection = DIRECTION_INACTIVE;
    objModifiedInfo.eAudioDirection = DIRECTION_SEND_RECEIVE;

    EXPECT_EQ(IMS_FALSE, pUpdatingInfo->IsHeld());
    EXPECT_EQ(IMS_FALSE, pUpdatingInfo->IsHeldBy());
    EXPECT_EQ(IMS_FALSE, pUpdatingInfo->IsResumed());
    EXPECT_EQ(IMS_TRUE, pUpdatingInfo->IsResumedBy());

    EXPECT_EQ(IMS_FALSE, pUpdatingInfo->IsNeedToAlert());
}

TEST_F(UpdatingInfoTest, ReturnHoldResumeStatusIfAlertingReceiveToSendReceive)
{
    pUpdatingInfo = new UpdatingInfo(objCallContext);
    MediaInfo& objOriginalInfo = pUpdatingInfo->GetOriginalInfo();
    MediaInfo& objAlertingInfo = pUpdatingInfo->GetAlertingInfo();
    MediaInfo& objModifiedInfo = pUpdatingInfo->GetModifiedInfo();
    objModifiedInfo.eAudioDirection = DIRECTION_INVALID;

    objOriginalInfo.eAudioDirection = DIRECTION_RECEIVE;
    objAlertingInfo.eAudioDirection = DIRECTION_SEND_RECEIVE;

    EXPECT_EQ(IMS_FALSE, pUpdatingInfo->IsHeld());
    EXPECT_EQ(IMS_FALSE, pUpdatingInfo->IsHeldBy());
    EXPECT_EQ(IMS_FALSE, pUpdatingInfo->IsResumed());
    EXPECT_EQ(IMS_TRUE, pUpdatingInfo->IsResumedBy());

    EXPECT_EQ(IMS_FALSE, pUpdatingInfo->IsNeedToAlert());
}

TEST_F(UpdatingInfoTest, ReturnHoldResumeStatusIfAlertingInactiveToSend)
{
    pUpdatingInfo = new UpdatingInfo(objCallContext);
    MediaInfo& objOriginalInfo = pUpdatingInfo->GetOriginalInfo();
    MediaInfo& objAlertingInfo = pUpdatingInfo->GetAlertingInfo();
    MediaInfo& objModifiedInfo = pUpdatingInfo->GetModifiedInfo();
    objModifiedInfo.eAudioDirection = DIRECTION_INVALID;

    objOriginalInfo.eAudioDirection = DIRECTION_INACTIVE;
    objAlertingInfo.eAudioDirection = DIRECTION_SEND;

    EXPECT_EQ(IMS_FALSE, pUpdatingInfo->IsHeld());
    EXPECT_EQ(IMS_FALSE, pUpdatingInfo->IsHeldBy());
    EXPECT_EQ(IMS_FALSE, pUpdatingInfo->IsResumed());
    EXPECT_EQ(IMS_TRUE, pUpdatingInfo->IsResumedBy());

    EXPECT_EQ(IMS_FALSE, pUpdatingInfo->IsNeedToAlert());
}

TEST_F(UpdatingInfoTest, ReturnHoldResumeStatusIfAlertingInactiveToSendReceive)
{
    pUpdatingInfo = new UpdatingInfo(objCallContext);
    MediaInfo& objOriginalInfo = pUpdatingInfo->GetOriginalInfo();
    MediaInfo& objAlertingInfo = pUpdatingInfo->GetAlertingInfo();
    MediaInfo& objModifiedInfo = pUpdatingInfo->GetModifiedInfo();
    MediaInfo& objModifyingInfo = pUpdatingInfo->GetModifyingInfo();
    objModifiedInfo.eAudioDirection = DIRECTION_INVALID;
    objModifyingInfo.eAudioDirection = DIRECTION_INVALID;

    objOriginalInfo.eAudioDirection = DIRECTION_INACTIVE;
    objAlertingInfo.eAudioDirection = DIRECTION_SEND_RECEIVE;

    EXPECT_EQ(IMS_FALSE, pUpdatingInfo->IsHeld());
    EXPECT_EQ(IMS_FALSE, pUpdatingInfo->IsHeldBy());
    EXPECT_EQ(IMS_FALSE, pUpdatingInfo->IsResumed());
    EXPECT_EQ(IMS_TRUE, pUpdatingInfo->IsResumedBy());

    EXPECT_EQ(IMS_FALSE, pUpdatingInfo->IsNeedToAlert());
}

TEST_F(UpdatingInfoTest, IsNeedToAlertReturnsTrueIfVideoDirectionIsChanging)
{
    ON_CALL(objSession, GetCallType()).WillByDefault(Return(CallType::VT));
    pUpdatingInfo = new UpdatingInfo(objCallContext);
    ON_CALL(objMediaManager, GetNegotiatedCallType(&objISession))
            .WillByDefault(Return(CallType::VT));

    MediaInfo& objOriginalInfo = pUpdatingInfo->GetOriginalInfo();
    MediaInfo& objModifyingInfo = pUpdatingInfo->GetModifyingInfo();
    MediaInfo& objAlertingInfo = pUpdatingInfo->GetAlertingInfo();

    // Not (IsHeldBy() || IsResumedBy())
    objOriginalInfo.eAudioDirection = DIRECTION_INACTIVE;
    objModifyingInfo.eAudioDirection = DIRECTION_SEND_RECEIVE;

    objOriginalInfo.eVideoDirection = DIRECTION_SEND;
    objAlertingInfo.eVideoDirection = DIRECTION_SEND_RECEIVE;

    EXPECT_EQ(IMS_TRUE, pUpdatingInfo->IsNeedToAlert());

    ON_CALL(objSession, GetCallType()).WillByDefault(Return(CallType::VIDEO_RTT));
    ON_CALL(objMediaManager, GetNegotiatedCallType(&objISession))
            .WillByDefault(Return(CallType::VIDEO_RTT));
    EXPECT_EQ(IMS_TRUE, pUpdatingInfo->IsNeedToAlert());
}

TEST_F(UpdatingInfoTest,
        IsNeedToAlertReturnsFalseIfVideoDirectionIsChangingButCallTypeKeepsBeingVoip)
{
    ON_CALL(objSession, GetCallType()).WillByDefault(Return(CallType::VOIP));
    pUpdatingInfo = new UpdatingInfo(objCallContext);
    ON_CALL(objMediaManager, GetNegotiatedCallType(&objISession))
            .WillByDefault(Return(CallType::VOIP));

    MediaInfo& objOriginalInfo = pUpdatingInfo->GetOriginalInfo();
    MediaInfo& objModifyingInfo = pUpdatingInfo->GetModifyingInfo();
    MediaInfo& objAlertingInfo = pUpdatingInfo->GetAlertingInfo();

    // Not (IsHeldBy() || IsResumedBy())
    objOriginalInfo.eAudioDirection = DIRECTION_INACTIVE;
    objModifyingInfo.eAudioDirection = DIRECTION_SEND_RECEIVE;

    objOriginalInfo.eVideoDirection = DIRECTION_SEND;
    objAlertingInfo.eVideoDirection = DIRECTION_SEND_RECEIVE;

    EXPECT_EQ(IMS_FALSE, pUpdatingInfo->IsNeedToAlert());
}

TEST_F(UpdatingInfoTest, IsNeedToAlertReturnsFalseIfVideoDirectionIsNotChanging)
{
    pUpdatingInfo = new UpdatingInfo(objCallContext);
    MediaInfo& objOriginalInfo = pUpdatingInfo->GetOriginalInfo();
    MediaInfo& objModifyingInfo = pUpdatingInfo->GetModifyingInfo();
    MediaInfo& objAlertingInfo = pUpdatingInfo->GetAlertingInfo();

    // Not (IsHeldBy() || IsResumedBy())
    objOriginalInfo.eAudioDirection = DIRECTION_INACTIVE;
    objModifyingInfo.eAudioDirection = DIRECTION_SEND_RECEIVE;

    objOriginalInfo.eVideoDirection = objAlertingInfo.eVideoDirection = DIRECTION_SEND;

    EXPECT_EQ(IMS_FALSE, pUpdatingInfo->IsNeedToAlert());
}

TEST_F(UpdatingInfoTest, IsNeedToAlertReturnsTrueIfCallTypeIsChanged)
{
    ON_CALL(objSession, GetCallType()).WillByDefault(Return(CallType::VT));
    pUpdatingInfo = new UpdatingInfo(objCallContext);
    MediaInfo& objOriginalInfo = pUpdatingInfo->GetOriginalInfo();
    MediaInfo& objModifyingInfo = pUpdatingInfo->GetModifyingInfo();
    MediaInfo& objAlertingInfo = pUpdatingInfo->GetAlertingInfo();

    // Not (IsHeldBy() || IsResumedBy())
    objOriginalInfo.eAudioDirection = DIRECTION_INACTIVE;
    objModifyingInfo.eAudioDirection = DIRECTION_SEND_RECEIVE;

    objOriginalInfo.eVideoDirection = objAlertingInfo.eVideoDirection = DIRECTION_SEND;

    ON_CALL(objMediaManager, GetNegotiatedCallType(&objISession))
            .WillByDefault(Return(CallType::VOIP));

    EXPECT_EQ(IMS_TRUE, pUpdatingInfo->IsNeedToAlert());
}

TEST_F(UpdatingInfoTest, IsNeedToAlertReturnsTrueIfCallTypeIsChangedAndResumedBySimultaneously)
{
    ON_CALL(objSession, GetCallType()).WillByDefault(Return(CallType::VT));
    pUpdatingInfo = new UpdatingInfo(objCallContext);
    MediaInfo& objOriginalInfo = pUpdatingInfo->GetOriginalInfo();
    MediaInfo& objAlertingInfo = pUpdatingInfo->GetAlertingInfo();
    MediaInfo& objModifiedInfo = pUpdatingInfo->GetModifiedInfo();
    MediaInfo& objModifyingInfo = pUpdatingInfo->GetModifyingInfo();
    objModifiedInfo.eAudioDirection = DIRECTION_INVALID;
    objModifyingInfo.eAudioDirection = DIRECTION_INVALID;

    objOriginalInfo.eAudioDirection = DIRECTION_INACTIVE;
    objAlertingInfo.eAudioDirection = DIRECTION_SEND_RECEIVE;

    EXPECT_EQ(IMS_FALSE, pUpdatingInfo->IsHeld());
    EXPECT_EQ(IMS_FALSE, pUpdatingInfo->IsHeldBy());
    EXPECT_EQ(IMS_FALSE, pUpdatingInfo->IsResumed());
    EXPECT_EQ(IMS_TRUE, pUpdatingInfo->IsResumedBy());

    ON_CALL(objMediaManager, GetNegotiatedCallType(&objISession))
            .WillByDefault(Return(CallType::VOIP));

    EXPECT_EQ(IMS_TRUE, pUpdatingInfo->IsNeedToAlert());
}

TEST_F(UpdatingInfoTest, IsRequestedHoldResumeReturnsTrueIfAudioDirectionIsChanging)
{
    pUpdatingInfo = new UpdatingInfo(objCallContext);
    MediaInfo& objOriginalInfo = pUpdatingInfo->GetOriginalInfo();
    MediaInfo& objModifyingInfo = pUpdatingInfo->GetModifyingInfo();

    objOriginalInfo.eAudioDirection = DIRECTION_SEND;
    objModifyingInfo.eAudioDirection = DIRECTION_SEND_RECEIVE;

    EXPECT_EQ(IMS_TRUE, pUpdatingInfo->IsRequestedHoldResume());
}

TEST_F(UpdatingInfoTest, IsRequestedHoldResumeReturnsTrueIfAudioDirectionIsNotChanging)
{
    pUpdatingInfo = new UpdatingInfo(objCallContext);
    MediaInfo& objOriginalInfo = pUpdatingInfo->GetOriginalInfo();
    MediaInfo& objModifyingInfo = pUpdatingInfo->GetModifyingInfo();

    objOriginalInfo.eAudioDirection = objModifyingInfo.eAudioDirection = DIRECTION_SEND;

    EXPECT_EQ(IMS_FALSE, pUpdatingInfo->IsRequestedHoldResume());
}

TEST_F(UpdatingInfoTest, IsRequestedHoldResumeReturnsTrueIfAudioDirectionIsInvalid)
{
    pUpdatingInfo = new UpdatingInfo(objCallContext);
    MediaInfo& objOriginalInfo = pUpdatingInfo->GetOriginalInfo();
    objOriginalInfo.eAudioDirection = DIRECTION_INVALID;

    EXPECT_EQ(IMS_FALSE, pUpdatingInfo->IsRequestedHoldResume());
}

TEST_F(UpdatingInfoTest, IsRequestedModifyingReturnsFalseIfModifyingInfoIsInvalid)
{
    pUpdatingInfo = new UpdatingInfo(objCallContext);
    MediaInfo& objModifyingInfo = pUpdatingInfo->GetModifyingInfo();
    objModifyingInfo.eAudioDirection = DIRECTION_INVALID;

    EXPECT_EQ(IMS_FALSE, pUpdatingInfo->IsRequestedModifying());
}

TEST_F(UpdatingInfoTest, IsRequestedModifyingReturnsFalseIfAudioDirectionIsChanging)
{
    pUpdatingInfo = new UpdatingInfo(objCallContext);
    MediaInfo& objOriginalInfo = pUpdatingInfo->GetOriginalInfo();
    MediaInfo& objModifyingInfo = pUpdatingInfo->GetModifyingInfo();

    objOriginalInfo.eAudioDirection = DIRECTION_SEND;
    objModifyingInfo.eAudioDirection = DIRECTION_SEND_RECEIVE;

    EXPECT_EQ(IMS_FALSE, pUpdatingInfo->IsRequestedModifying());
}

TEST_F(UpdatingInfoTest, IsRequestedModifyingReturnsTrueIfVideoDirectionIsChanging)
{
    pUpdatingInfo = new UpdatingInfo(objCallContext);
    MediaInfo& objOriginalInfo = pUpdatingInfo->GetOriginalInfo();
    MediaInfo& objModifyingInfo = pUpdatingInfo->GetModifyingInfo();
    objModifyingInfo.eAudioDirection = objOriginalInfo.eAudioDirection = DIRECTION_SEND;

    objOriginalInfo.eVideoDirection = DIRECTION_SEND;
    objModifyingInfo.eVideoDirection = DIRECTION_SEND_RECEIVE;

    EXPECT_EQ(IMS_TRUE, pUpdatingInfo->IsRequestedModifying());
}

TEST_F(UpdatingInfoTest, IsRequestedModifyingReturnsFalseIfVideoDirectionIsNotChanging)
{
    pUpdatingInfo = new UpdatingInfo(objCallContext);
    MediaInfo& objOriginalInfo = pUpdatingInfo->GetOriginalInfo();
    MediaInfo& objModifyingInfo = pUpdatingInfo->GetModifyingInfo();
    objOriginalInfo.eAudioDirection = objModifyingInfo.eAudioDirection = DIRECTION_SEND;

    objOriginalInfo.eVideoDirection = objModifyingInfo.eVideoDirection = DIRECTION_SEND;

    EXPECT_EQ(IMS_FALSE, pUpdatingInfo->IsRequestedModifying());
}

TEST_F(UpdatingInfoTest, IsRequestedModifyingReturnsTrueIfCallTypeIsChanged)
{
    ON_CALL(objSession, GetCallType()).WillByDefault(Return(CallType::VT));
    pUpdatingInfo = new UpdatingInfo(objCallContext);
    MediaInfo& objOriginalInfo = pUpdatingInfo->GetOriginalInfo();
    MediaInfo& objModifyingInfo = pUpdatingInfo->GetModifyingInfo();
    objOriginalInfo.eAudioDirection = objModifyingInfo.eAudioDirection = DIRECTION_SEND;

    objOriginalInfo.eVideoDirection = objModifyingInfo.eVideoDirection = DIRECTION_SEND;

    pUpdatingInfo->SetTargetCallType(CallType::VOIP);

    EXPECT_EQ(IMS_TRUE, pUpdatingInfo->IsRequestedModifying());
}

TEST_F(UpdatingInfoTest, IsModifiedReturnsTrueIfCallTypeIsChanged)
{
    ON_CALL(objSession, GetCallType()).WillByDefault(Return(CallType::VT));
    pUpdatingInfo = new UpdatingInfo(objCallContext);
    ON_CALL(objMediaManager, GetNegotiatedCallType(&objISession))
            .WillByDefault(Return(CallType::VOIP));

    EXPECT_EQ(IMS_TRUE, pUpdatingInfo->IsModified());
}

TEST_F(UpdatingInfoTest, IsModifiedReturnsFalseIfCallTypeIsNotChanged)
{
    ON_CALL(objSession, GetCallType()).WillByDefault(Return(CallType::VT));
    pUpdatingInfo = new UpdatingInfo(objCallContext);
    ON_CALL(objMediaManager, GetNegotiatedCallType(&objISession))
            .WillByDefault(Return(CallType::VT));

    EXPECT_EQ(IMS_FALSE, pUpdatingInfo->IsModified());
}

TEST_F(UpdatingInfoTest, IsDowngradedReturnsTrueIfCallTypeIsDowngraded)
{
    // VT -> VOIP
    ON_CALL(objSession, GetCallType()).WillByDefault(Return(CallType::VT));
    pUpdatingInfo = new UpdatingInfo(objCallContext);
    ON_CALL(objMediaManager, GetNegotiatedCallType(&objISession))
            .WillByDefault(Return(CallType::VOIP));

    EXPECT_EQ(IMS_TRUE, pUpdatingInfo->IsDowngraded());
    delete pUpdatingInfo;

    // RTT -> VOIP
    ON_CALL(objSession, GetCallType()).WillByDefault(Return(CallType::RTT));
    pUpdatingInfo = new UpdatingInfo(objCallContext);
    ON_CALL(objMediaManager, GetNegotiatedCallType(&objISession))
            .WillByDefault(Return(CallType::VOIP));

    EXPECT_EQ(IMS_TRUE, pUpdatingInfo->IsDowngraded());
    delete pUpdatingInfo;

    // VIDEO_RTT -> RTT
    ON_CALL(objSession, GetCallType()).WillByDefault(Return(CallType::VIDEO_RTT));
    pUpdatingInfo = new UpdatingInfo(objCallContext);
    ON_CALL(objMediaManager, GetNegotiatedCallType(&objISession))
            .WillByDefault(Return(CallType::RTT));

    EXPECT_EQ(IMS_TRUE, pUpdatingInfo->IsDowngraded());
    delete pUpdatingInfo;

    // VIDEO_RTT -> VT
    ON_CALL(objSession, GetCallType()).WillByDefault(Return(CallType::VIDEO_RTT));
    pUpdatingInfo = new UpdatingInfo(objCallContext);
    ON_CALL(objMediaManager, GetNegotiatedCallType(&objISession))
            .WillByDefault(Return(CallType::VT));

    EXPECT_EQ(IMS_TRUE, pUpdatingInfo->IsDowngraded());
    delete pUpdatingInfo;

    // VIDEO_RTT -> VOIP
    ON_CALL(objSession, GetCallType()).WillByDefault(Return(CallType::VIDEO_RTT));
    pUpdatingInfo = new UpdatingInfo(objCallContext);
    ON_CALL(objMediaManager, GetNegotiatedCallType(&objISession))
            .WillByDefault(Return(CallType::VT));

    EXPECT_EQ(IMS_TRUE, pUpdatingInfo->IsDowngraded());
}

TEST_F(UpdatingInfoTest, IsDowngradedReturnsFalseIfCallTypeIsNotModified)
{
    pUpdatingInfo = new UpdatingInfo(objCallContext);
    ON_CALL(objSession, GetCallType()).WillByDefault(Return(CallType::VT));
    ON_CALL(objMediaManager, GetNegotiatedCallType(&objISession))
            .WillByDefault(Return(CallType::VT));

    EXPECT_EQ(IMS_FALSE, pUpdatingInfo->IsDowngraded());
}

TEST_F(UpdatingInfoTest, IsDowngradedReturnsFalseIfCallTypeIsUpgraded)
{
    // VOIP -> VT
    ON_CALL(objSession, GetCallType()).WillByDefault(Return(CallType::VOIP));
    pUpdatingInfo = new UpdatingInfo(objCallContext);
    ON_CALL(objMediaManager, GetNegotiatedCallType(&objISession))
            .WillByDefault(Return(CallType::VT));
    EXPECT_EQ(IMS_FALSE, pUpdatingInfo->IsDowngraded());
    delete pUpdatingInfo;

    // VOIP -> RTT
    ON_CALL(objSession, GetCallType()).WillByDefault(Return(CallType::VOIP));
    pUpdatingInfo = new UpdatingInfo(objCallContext);
    ON_CALL(objMediaManager, GetNegotiatedCallType(&objISession))
            .WillByDefault(Return(CallType::RTT));
    EXPECT_EQ(IMS_FALSE, pUpdatingInfo->IsDowngraded());
    delete pUpdatingInfo;

    // VOIP -> VIDEO_RTT
    ON_CALL(objSession, GetCallType()).WillByDefault(Return(CallType::VOIP));
    pUpdatingInfo = new UpdatingInfo(objCallContext);
    ON_CALL(objMediaManager, GetNegotiatedCallType(&objISession))
            .WillByDefault(Return(CallType::VIDEO_RTT));
    EXPECT_EQ(IMS_FALSE, pUpdatingInfo->IsDowngraded());
    delete pUpdatingInfo;

    // VT -> VIDEO_RTT
    ON_CALL(objSession, GetCallType()).WillByDefault(Return(CallType::VT));
    pUpdatingInfo = new UpdatingInfo(objCallContext);
    ON_CALL(objMediaManager, GetNegotiatedCallType(&objISession))
            .WillByDefault(Return(CallType::VIDEO_RTT));
    EXPECT_EQ(IMS_FALSE, pUpdatingInfo->IsDowngraded());
    delete pUpdatingInfo;

    // RTT -> VIDEO_RTT
    ON_CALL(objSession, GetCallType()).WillByDefault(Return(CallType::RTT));
    pUpdatingInfo = new UpdatingInfo(objCallContext);
    ON_CALL(objMediaManager, GetNegotiatedCallType(&objISession))
            .WillByDefault(Return(CallType::VIDEO_RTT));
    EXPECT_EQ(IMS_FALSE, pUpdatingInfo->IsDowngraded());
    delete pUpdatingInfo;

    // Invalid : VT -> RTT
    ON_CALL(objSession, GetCallType()).WillByDefault(Return(CallType::VT));
    pUpdatingInfo = new UpdatingInfo(objCallContext);
    ON_CALL(objMediaManager, GetNegotiatedCallType(&objISession))
            .WillByDefault(Return(CallType::RTT));
    EXPECT_EQ(IMS_FALSE, pUpdatingInfo->IsDowngraded());
    delete pUpdatingInfo;

    // Invalid : RTT -> VT
    ON_CALL(objSession, GetCallType()).WillByDefault(Return(CallType::RTT));
    pUpdatingInfo = new UpdatingInfo(objCallContext);
    ON_CALL(objMediaManager, GetNegotiatedCallType(&objISession))
            .WillByDefault(Return(CallType::VT));
    EXPECT_EQ(IMS_FALSE, pUpdatingInfo->IsDowngraded());
}
