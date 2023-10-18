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

#include "MtcDef.h"
#include "call/IMtcCall.h"
#include "call/MockIMtcCallContext.h"
#include "call/MockIMtcSession.h"
#include "call/UpdatingInfo.h"
#include "core/MockISession.h"
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
        pUpdatingInfo = new UpdatingInfo(objCallContext);
    }

    virtual void TearDown() override { delete pUpdatingInfo; }
};

TEST_F(UpdatingInfoTest, GetTargetCallTypeReturnsUnknownInitially)
{
    EXPECT_EQ(CallType::UNKNOWN, pUpdatingInfo->GetTargetCallType());
}

TEST_F(UpdatingInfoTest, GetTargetCallTypeReturnsSetValue)
{
    CallType eCallType = CallType::VOIP;

    pUpdatingInfo->SetTargetCallType(eCallType);

    EXPECT_EQ(eCallType, pUpdatingInfo->GetTargetCallType());
}

TEST_F(UpdatingInfoTest, GetModifyingInfoReturnsNotNull)
{
    EXPECT_NE(nullptr, &pUpdatingInfo->GetModifyingInfo());
}

TEST_F(UpdatingInfoTest, GetAlertingInfoReturnsNotNull)
{
    EXPECT_NE(nullptr, &pUpdatingInfo->GetAlertingInfo());
}

TEST_F(UpdatingInfoTest, GetModifiedInfoReturnsNotNull)
{
    EXPECT_NE(nullptr, &pUpdatingInfo->GetModifiedInfo());
}

TEST_F(UpdatingInfoTest, IsModifierReturnsFalseInitially)
{
    EXPECT_EQ(IMS_FALSE, pUpdatingInfo->IsModifier());
}

TEST_F(UpdatingInfoTest, IsModifierReturnsTrueAfterSet)
{
    pUpdatingInfo->SetModifier();
    EXPECT_EQ(IMS_TRUE, pUpdatingInfo->IsModifier());
}

TEST_F(UpdatingInfoTest, IsAlertedReturnsFalseInitially)
{
    EXPECT_EQ(IMS_FALSE, pUpdatingInfo->IsAlerted());
}

TEST_F(UpdatingInfoTest, IsAlertedReturnsTrueAfterSet)
{
    pUpdatingInfo->SetAlerted();
    EXPECT_EQ(IMS_TRUE, pUpdatingInfo->IsAlerted());
}

TEST_F(UpdatingInfoTest, ReturnHoldResumeStatusIfModifyingSendReceiveToHeld)
{
    MediaInfo& objNegotiatedInfo = pUpdatingInfo->GetNegotiatedInfo();
    MediaInfo& objModifyingInfo = pUpdatingInfo->GetModifyingInfo();

    objNegotiatedInfo.eAudioDirection = DIRECTION_SEND_RECEIVE;
    objModifyingInfo.eAudioDirection = DIRECTION_SEND;

    EXPECT_EQ(IMS_TRUE, pUpdatingInfo->IsHeld());
    EXPECT_EQ(IMS_FALSE, pUpdatingInfo->IsHeldBy());
    EXPECT_EQ(IMS_FALSE, pUpdatingInfo->IsResumed());
    EXPECT_EQ(IMS_FALSE, pUpdatingInfo->IsResumedBy());
}

TEST_F(UpdatingInfoTest, ReturnHoldResumeStatusIfModifyingSendReceiveToInactive)
{
    MediaInfo& objNegotiatedInfo = pUpdatingInfo->GetNegotiatedInfo();
    MediaInfo& objModifyingInfo = pUpdatingInfo->GetModifyingInfo();

    objNegotiatedInfo.eAudioDirection = DIRECTION_SEND_RECEIVE;
    objModifyingInfo.eAudioDirection = DIRECTION_INACTIVE;

    EXPECT_EQ(IMS_TRUE, pUpdatingInfo->IsHeld());
    EXPECT_EQ(IMS_FALSE, pUpdatingInfo->IsHeldBy());
    EXPECT_EQ(IMS_FALSE, pUpdatingInfo->IsResumed());
    EXPECT_EQ(IMS_FALSE, pUpdatingInfo->IsResumedBy());
}

TEST_F(UpdatingInfoTest, ReturnHoldResumeStatusIfModifyingReceiveToInactive)
{
    MediaInfo& objNegotiatedInfo = pUpdatingInfo->GetNegotiatedInfo();
    MediaInfo& objModifyingInfo = pUpdatingInfo->GetModifyingInfo();

    objNegotiatedInfo.eAudioDirection = DIRECTION_RECEIVE;
    objModifyingInfo.eAudioDirection = DIRECTION_INACTIVE;

    EXPECT_EQ(IMS_TRUE, pUpdatingInfo->IsHeld());
    EXPECT_EQ(IMS_FALSE, pUpdatingInfo->IsHeldBy());
    EXPECT_EQ(IMS_FALSE, pUpdatingInfo->IsResumed());
    EXPECT_EQ(IMS_FALSE, pUpdatingInfo->IsResumedBy());
}

TEST_F(UpdatingInfoTest, ReturnHoldResumeStatusIfModifiedSendReceiveToReceive)
{
    MediaInfo& objNegotiatedInfo = pUpdatingInfo->GetNegotiatedInfo();
    MediaInfo& objModifiedInfo = pUpdatingInfo->GetModifiedInfo();

    objNegotiatedInfo.eAudioDirection = DIRECTION_SEND_RECEIVE;
    objModifiedInfo.eAudioDirection = DIRECTION_RECEIVE;

    EXPECT_EQ(IMS_FALSE, pUpdatingInfo->IsHeld());
    EXPECT_EQ(IMS_TRUE, pUpdatingInfo->IsHeldBy());
    EXPECT_EQ(IMS_FALSE, pUpdatingInfo->IsResumed());
    EXPECT_EQ(IMS_FALSE, pUpdatingInfo->IsResumedBy());

    EXPECT_EQ(IMS_FALSE, pUpdatingInfo->IsNeedToAlert());
}

TEST_F(UpdatingInfoTest, ReturnHoldResumeStatusIfModifiedSendReceiveToInactive)
{
    MediaInfo& objNegotiatedInfo = pUpdatingInfo->GetNegotiatedInfo();
    MediaInfo& objModifiedInfo = pUpdatingInfo->GetModifiedInfo();
    MediaInfo& objModifyingInfo = pUpdatingInfo->GetModifyingInfo();
    objModifyingInfo.eAudioDirection = DIRECTION_INVALID;

    objNegotiatedInfo.eAudioDirection = DIRECTION_SEND_RECEIVE;
    objModifiedInfo.eAudioDirection = DIRECTION_INACTIVE;

    EXPECT_EQ(IMS_FALSE, pUpdatingInfo->IsHeld());
    EXPECT_EQ(IMS_TRUE, pUpdatingInfo->IsHeldBy());
    EXPECT_EQ(IMS_FALSE, pUpdatingInfo->IsResumed());
    EXPECT_EQ(IMS_FALSE, pUpdatingInfo->IsResumedBy());

    EXPECT_EQ(IMS_FALSE, pUpdatingInfo->IsNeedToAlert());
}

TEST_F(UpdatingInfoTest, ReturnHoldResumeStatusIfModifiedSendToInactive)
{
    MediaInfo& objNegotiatedInfo = pUpdatingInfo->GetNegotiatedInfo();
    MediaInfo& objModifiedInfo = pUpdatingInfo->GetModifiedInfo();

    objNegotiatedInfo.eAudioDirection = DIRECTION_SEND;
    objModifiedInfo.eAudioDirection = DIRECTION_INACTIVE;

    EXPECT_EQ(IMS_FALSE, pUpdatingInfo->IsHeld());
    EXPECT_EQ(IMS_TRUE, pUpdatingInfo->IsHeldBy());
    EXPECT_EQ(IMS_FALSE, pUpdatingInfo->IsResumed());
    EXPECT_EQ(IMS_FALSE, pUpdatingInfo->IsResumedBy());

    EXPECT_EQ(IMS_FALSE, pUpdatingInfo->IsNeedToAlert());
}

TEST_F(UpdatingInfoTest, ReturnHoldResumeStatusIfAlertingSendReceiveToReceive)
{
    MediaInfo& objNegotiatedInfo = pUpdatingInfo->GetNegotiatedInfo();
    MediaInfo& objAlertingInfo = pUpdatingInfo->GetAlertingInfo();
    MediaInfo& objModifiedInfo = pUpdatingInfo->GetModifiedInfo();
    objModifiedInfo.eAudioDirection = DIRECTION_INVALID;

    objNegotiatedInfo.eAudioDirection = DIRECTION_SEND_RECEIVE;
    objAlertingInfo.eAudioDirection = DIRECTION_RECEIVE;

    EXPECT_EQ(IMS_FALSE, pUpdatingInfo->IsHeld());
    EXPECT_EQ(IMS_TRUE, pUpdatingInfo->IsHeldBy());
    EXPECT_EQ(IMS_FALSE, pUpdatingInfo->IsResumed());
    EXPECT_EQ(IMS_FALSE, pUpdatingInfo->IsResumedBy());

    EXPECT_EQ(IMS_FALSE, pUpdatingInfo->IsNeedToAlert());
}

TEST_F(UpdatingInfoTest, ReturnHoldResumeStatusIfAlertingSendReceiveToInactive)
{
    MediaInfo& objNegotiatedInfo = pUpdatingInfo->GetNegotiatedInfo();
    MediaInfo& objAlertingInfo = pUpdatingInfo->GetAlertingInfo();
    MediaInfo& objModifiedInfo = pUpdatingInfo->GetModifiedInfo();
    MediaInfo& objModifyingInfo = pUpdatingInfo->GetModifyingInfo();
    objModifiedInfo.eAudioDirection = DIRECTION_INVALID;
    objModifyingInfo.eAudioDirection = DIRECTION_INVALID;

    objNegotiatedInfo.eAudioDirection = DIRECTION_SEND_RECEIVE;
    objAlertingInfo.eAudioDirection = DIRECTION_INACTIVE;

    EXPECT_EQ(IMS_FALSE, pUpdatingInfo->IsHeld());
    EXPECT_EQ(IMS_TRUE, pUpdatingInfo->IsHeldBy());
    EXPECT_EQ(IMS_FALSE, pUpdatingInfo->IsResumed());
    EXPECT_EQ(IMS_FALSE, pUpdatingInfo->IsResumedBy());

    EXPECT_EQ(IMS_FALSE, pUpdatingInfo->IsNeedToAlert());
}

TEST_F(UpdatingInfoTest, ReturnHoldResumeStatusIfAlertingSendToInactive)
{
    MediaInfo& objNegotiatedInfo = pUpdatingInfo->GetNegotiatedInfo();
    MediaInfo& objAlertingInfo = pUpdatingInfo->GetAlertingInfo();
    MediaInfo& objModifiedInfo = pUpdatingInfo->GetModifiedInfo();
    objModifiedInfo.eAudioDirection = DIRECTION_INVALID;

    objNegotiatedInfo.eAudioDirection = DIRECTION_SEND;
    objAlertingInfo.eAudioDirection = DIRECTION_INACTIVE;

    EXPECT_EQ(IMS_FALSE, pUpdatingInfo->IsHeld());
    EXPECT_EQ(IMS_TRUE, pUpdatingInfo->IsHeldBy());
    EXPECT_EQ(IMS_FALSE, pUpdatingInfo->IsResumed());
    EXPECT_EQ(IMS_FALSE, pUpdatingInfo->IsResumedBy());

    EXPECT_EQ(IMS_FALSE, pUpdatingInfo->IsNeedToAlert());
}

TEST_F(UpdatingInfoTest, ReturnHoldResumeStatusIfModifyingSendToSendReceive)
{
    MediaInfo& objNegotiatedInfo = pUpdatingInfo->GetNegotiatedInfo();
    MediaInfo& objModifyingInfo = pUpdatingInfo->GetModifyingInfo();

    objNegotiatedInfo.eAudioDirection = DIRECTION_SEND;
    objModifyingInfo.eAudioDirection = DIRECTION_SEND_RECEIVE;

    EXPECT_EQ(IMS_FALSE, pUpdatingInfo->IsHeld());
    EXPECT_EQ(IMS_FALSE, pUpdatingInfo->IsHeldBy());
    EXPECT_EQ(IMS_TRUE, pUpdatingInfo->IsResumed());
    EXPECT_EQ(IMS_FALSE, pUpdatingInfo->IsResumedBy());
}

TEST_F(UpdatingInfoTest, ReturnHoldResumeStatusIfModifyingInactiveToReceive)
{
    MediaInfo& objNegotiatedInfo = pUpdatingInfo->GetNegotiatedInfo();
    MediaInfo& objModifyingInfo = pUpdatingInfo->GetModifyingInfo();

    objNegotiatedInfo.eAudioDirection = DIRECTION_INACTIVE;
    objModifyingInfo.eAudioDirection = DIRECTION_RECEIVE;

    EXPECT_EQ(IMS_FALSE, pUpdatingInfo->IsHeld());
    EXPECT_EQ(IMS_FALSE, pUpdatingInfo->IsHeldBy());
    EXPECT_EQ(IMS_TRUE, pUpdatingInfo->IsResumed());
    EXPECT_EQ(IMS_FALSE, pUpdatingInfo->IsResumedBy());
}

TEST_F(UpdatingInfoTest, ReturnHoldResumeStatusIfModifyingInactiveToSendReceive)
{
    MediaInfo& objNegotiatedInfo = pUpdatingInfo->GetNegotiatedInfo();
    MediaInfo& objModifyingInfo = pUpdatingInfo->GetModifyingInfo();

    objNegotiatedInfo.eAudioDirection = DIRECTION_INACTIVE;
    objModifyingInfo.eAudioDirection = DIRECTION_SEND_RECEIVE;

    EXPECT_EQ(IMS_FALSE, pUpdatingInfo->IsHeld());
    EXPECT_EQ(IMS_FALSE, pUpdatingInfo->IsHeldBy());
    EXPECT_EQ(IMS_TRUE, pUpdatingInfo->IsResumed());
    EXPECT_EQ(IMS_FALSE, pUpdatingInfo->IsResumedBy());
}

TEST_F(UpdatingInfoTest, ReturnHoldResumeStatusIfModifiedReceiveToSendReceive)
{
    MediaInfo& objNegotiatedInfo = pUpdatingInfo->GetNegotiatedInfo();
    MediaInfo& objModifiedInfo = pUpdatingInfo->GetModifiedInfo();

    objModifiedInfo.eAudioDirection = DIRECTION_SEND_RECEIVE;
    objNegotiatedInfo.eAudioDirection = DIRECTION_RECEIVE;

    EXPECT_EQ(IMS_FALSE, pUpdatingInfo->IsHeld());
    EXPECT_EQ(IMS_FALSE, pUpdatingInfo->IsHeldBy());
    EXPECT_EQ(IMS_FALSE, pUpdatingInfo->IsResumed());
    EXPECT_EQ(IMS_TRUE, pUpdatingInfo->IsResumedBy());

    EXPECT_EQ(IMS_FALSE, pUpdatingInfo->IsNeedToAlert());
}

TEST_F(UpdatingInfoTest, ReturnHoldResumeStatusIfModifiedInactiveToSend)
{
    MediaInfo& objNegotiatedInfo = pUpdatingInfo->GetNegotiatedInfo();
    MediaInfo& objModifiedInfo = pUpdatingInfo->GetModifiedInfo();

    objNegotiatedInfo.eAudioDirection = DIRECTION_INACTIVE;
    objModifiedInfo.eAudioDirection = DIRECTION_SEND;

    EXPECT_EQ(IMS_FALSE, pUpdatingInfo->IsHeld());
    EXPECT_EQ(IMS_FALSE, pUpdatingInfo->IsHeldBy());
    EXPECT_EQ(IMS_FALSE, pUpdatingInfo->IsResumed());
    EXPECT_EQ(IMS_TRUE, pUpdatingInfo->IsResumedBy());

    EXPECT_EQ(IMS_FALSE, pUpdatingInfo->IsNeedToAlert());
}

TEST_F(UpdatingInfoTest, ReturnHoldResumeStatusIfModifiedInactiveToSendReceive)
{
    MediaInfo& objNegotiatedInfo = pUpdatingInfo->GetNegotiatedInfo();
    MediaInfo& objModifiedInfo = pUpdatingInfo->GetModifiedInfo();
    MediaInfo& objModifyingInfo = pUpdatingInfo->GetModifyingInfo();
    objModifyingInfo.eAudioDirection = DIRECTION_INVALID;

    objNegotiatedInfo.eAudioDirection = DIRECTION_INACTIVE;
    objModifiedInfo.eAudioDirection = DIRECTION_SEND_RECEIVE;

    EXPECT_EQ(IMS_FALSE, pUpdatingInfo->IsHeld());
    EXPECT_EQ(IMS_FALSE, pUpdatingInfo->IsHeldBy());
    EXPECT_EQ(IMS_FALSE, pUpdatingInfo->IsResumed());
    EXPECT_EQ(IMS_TRUE, pUpdatingInfo->IsResumedBy());

    EXPECT_EQ(IMS_FALSE, pUpdatingInfo->IsNeedToAlert());
}

TEST_F(UpdatingInfoTest, ReturnHoldResumeStatusIfAlertingReceiveToSendReceive)
{
    MediaInfo& objNegotiatedInfo = pUpdatingInfo->GetNegotiatedInfo();
    MediaInfo& objAlertingInfo = pUpdatingInfo->GetAlertingInfo();
    MediaInfo& objModifiedInfo = pUpdatingInfo->GetModifiedInfo();
    objModifiedInfo.eAudioDirection = DIRECTION_INVALID;

    objNegotiatedInfo.eAudioDirection = DIRECTION_RECEIVE;
    objAlertingInfo.eAudioDirection = DIRECTION_SEND_RECEIVE;

    EXPECT_EQ(IMS_FALSE, pUpdatingInfo->IsHeld());
    EXPECT_EQ(IMS_FALSE, pUpdatingInfo->IsHeldBy());
    EXPECT_EQ(IMS_FALSE, pUpdatingInfo->IsResumed());
    EXPECT_EQ(IMS_TRUE, pUpdatingInfo->IsResumedBy());

    EXPECT_EQ(IMS_FALSE, pUpdatingInfo->IsNeedToAlert());
}

TEST_F(UpdatingInfoTest, ReturnHoldResumeStatusIfAlertingInactiveToSend)
{
    MediaInfo& objNegotiatedInfo = pUpdatingInfo->GetNegotiatedInfo();
    MediaInfo& objAlertingInfo = pUpdatingInfo->GetAlertingInfo();
    MediaInfo& objModifiedInfo = pUpdatingInfo->GetModifiedInfo();
    objModifiedInfo.eAudioDirection = DIRECTION_INVALID;

    objNegotiatedInfo.eAudioDirection = DIRECTION_INACTIVE;
    objAlertingInfo.eAudioDirection = DIRECTION_SEND;

    EXPECT_EQ(IMS_FALSE, pUpdatingInfo->IsHeld());
    EXPECT_EQ(IMS_FALSE, pUpdatingInfo->IsHeldBy());
    EXPECT_EQ(IMS_FALSE, pUpdatingInfo->IsResumed());
    EXPECT_EQ(IMS_TRUE, pUpdatingInfo->IsResumedBy());

    EXPECT_EQ(IMS_FALSE, pUpdatingInfo->IsNeedToAlert());
}

TEST_F(UpdatingInfoTest, ReturnHoldResumeStatusIfAlertingInactiveToSendReceive)
{
    MediaInfo& objNegotiatedInfo = pUpdatingInfo->GetNegotiatedInfo();
    MediaInfo& objAlertingInfo = pUpdatingInfo->GetAlertingInfo();
    MediaInfo& objModifiedInfo = pUpdatingInfo->GetModifiedInfo();
    MediaInfo& objModifyingInfo = pUpdatingInfo->GetModifyingInfo();
    objModifiedInfo.eAudioDirection = DIRECTION_INVALID;
    objModifyingInfo.eAudioDirection = DIRECTION_INVALID;

    objNegotiatedInfo.eAudioDirection = DIRECTION_INACTIVE;
    objAlertingInfo.eAudioDirection = DIRECTION_SEND_RECEIVE;

    EXPECT_EQ(IMS_FALSE, pUpdatingInfo->IsHeld());
    EXPECT_EQ(IMS_FALSE, pUpdatingInfo->IsHeldBy());
    EXPECT_EQ(IMS_FALSE, pUpdatingInfo->IsResumed());
    EXPECT_EQ(IMS_TRUE, pUpdatingInfo->IsResumedBy());

    EXPECT_EQ(IMS_FALSE, pUpdatingInfo->IsNeedToAlert());
}

TEST_F(UpdatingInfoTest, IsNeedToAlertReturnsTrueIfVideoDirectionIsChanging)
{
    ON_CALL(objSession, GetPreviousCallType()).WillByDefault(Return(CallType::VT));
    ON_CALL(objMediaManager, GetNegotiatedCallType(&objISession))
            .WillByDefault(Return(CallType::VT));

    MediaInfo& objNegotiatedInfo = pUpdatingInfo->GetNegotiatedInfo();
    MediaInfo& objModifyingInfo = pUpdatingInfo->GetModifyingInfo();
    MediaInfo& objAlertingInfo = pUpdatingInfo->GetAlertingInfo();

    // Not (IsHeldBy() || IsResumedBy())
    objNegotiatedInfo.eAudioDirection = DIRECTION_INACTIVE;
    objModifyingInfo.eAudioDirection = DIRECTION_SEND_RECEIVE;

    objNegotiatedInfo.eVideoDirection = DIRECTION_SEND;
    objAlertingInfo.eVideoDirection = DIRECTION_SEND_RECEIVE;

    EXPECT_EQ(IMS_TRUE, pUpdatingInfo->IsNeedToAlert());

    ON_CALL(objSession, GetPreviousCallType()).WillByDefault(Return(CallType::VIDEO_RTT));
    ON_CALL(objMediaManager, GetNegotiatedCallType(&objISession))
            .WillByDefault(Return(CallType::VIDEO_RTT));
    EXPECT_EQ(IMS_TRUE, pUpdatingInfo->IsNeedToAlert());
}

TEST_F(UpdatingInfoTest, IsNeedToAlertReturnsFalseIfVideoDirectionIsChangingButCallTypeIsVoip)
{
    ON_CALL(objSession, GetPreviousCallType()).WillByDefault(Return(CallType::VOIP));
    ON_CALL(objMediaManager, GetNegotiatedCallType(&objISession))
            .WillByDefault(Return(CallType::VOIP));

    MediaInfo& objNegotiatedInfo = pUpdatingInfo->GetNegotiatedInfo();
    MediaInfo& objModifyingInfo = pUpdatingInfo->GetModifyingInfo();
    MediaInfo& objAlertingInfo = pUpdatingInfo->GetAlertingInfo();

    // Not (IsHeldBy() || IsResumedBy())
    objNegotiatedInfo.eAudioDirection = DIRECTION_INACTIVE;
    objModifyingInfo.eAudioDirection = DIRECTION_SEND_RECEIVE;

    objNegotiatedInfo.eVideoDirection = DIRECTION_SEND;
    objAlertingInfo.eVideoDirection = DIRECTION_SEND_RECEIVE;

    EXPECT_EQ(IMS_FALSE, pUpdatingInfo->IsNeedToAlert());
}

TEST_F(UpdatingInfoTest, IsNeedToAlertReturnsFalseIfVideoDirectionIsNotChanging)
{
    MediaInfo& objNegotiatedInfo = pUpdatingInfo->GetNegotiatedInfo();
    MediaInfo& objModifyingInfo = pUpdatingInfo->GetModifyingInfo();
    MediaInfo& objAlertingInfo = pUpdatingInfo->GetAlertingInfo();

    // Not (IsHeldBy() || IsResumedBy())
    objNegotiatedInfo.eAudioDirection = DIRECTION_INACTIVE;
    objModifyingInfo.eAudioDirection = DIRECTION_SEND_RECEIVE;

    objNegotiatedInfo.eVideoDirection = objAlertingInfo.eVideoDirection = DIRECTION_SEND;

    EXPECT_EQ(IMS_FALSE, pUpdatingInfo->IsNeedToAlert());
}

TEST_F(UpdatingInfoTest, IsNeedToAlertReturnsTrueIfCallTypeIsChanged)
{
    MediaInfo& objNegotiatedInfo = pUpdatingInfo->GetNegotiatedInfo();
    MediaInfo& objModifyingInfo = pUpdatingInfo->GetModifyingInfo();
    MediaInfo& objAlertingInfo = pUpdatingInfo->GetAlertingInfo();

    // Not (IsHeldBy() || IsResumedBy())
    objNegotiatedInfo.eAudioDirection = DIRECTION_INACTIVE;
    objModifyingInfo.eAudioDirection = DIRECTION_SEND_RECEIVE;

    objNegotiatedInfo.eVideoDirection = objAlertingInfo.eVideoDirection = DIRECTION_SEND;

    ON_CALL(objSession, GetPreviousCallType()).WillByDefault(Return(CallType::VT));
    ON_CALL(objMediaManager, GetNegotiatedCallType(&objISession))
            .WillByDefault(Return(CallType::VOIP));

    EXPECT_EQ(IMS_TRUE, pUpdatingInfo->IsNeedToAlert());
}

TEST_F(UpdatingInfoTest, IsNeedToAlertReturnsTrueIfCallTypeIsChangedAndResumedBySimultaneously)
{
    MediaInfo& objNegotiatedInfo = pUpdatingInfo->GetNegotiatedInfo();
    MediaInfo& objAlertingInfo = pUpdatingInfo->GetAlertingInfo();
    MediaInfo& objModifiedInfo = pUpdatingInfo->GetModifiedInfo();
    MediaInfo& objModifyingInfo = pUpdatingInfo->GetModifyingInfo();
    objModifiedInfo.eAudioDirection = DIRECTION_INVALID;
    objModifyingInfo.eAudioDirection = DIRECTION_INVALID;

    objNegotiatedInfo.eAudioDirection = DIRECTION_INACTIVE;
    objAlertingInfo.eAudioDirection = DIRECTION_SEND_RECEIVE;

    EXPECT_EQ(IMS_FALSE, pUpdatingInfo->IsHeld());
    EXPECT_EQ(IMS_FALSE, pUpdatingInfo->IsHeldBy());
    EXPECT_EQ(IMS_FALSE, pUpdatingInfo->IsResumed());
    EXPECT_EQ(IMS_TRUE, pUpdatingInfo->IsResumedBy());

    ON_CALL(objSession, GetPreviousCallType()).WillByDefault(Return(CallType::VT));
    ON_CALL(objMediaManager, GetNegotiatedCallType(&objISession))
            .WillByDefault(Return(CallType::VOIP));

    EXPECT_EQ(IMS_TRUE, pUpdatingInfo->IsNeedToAlert());
}

TEST_F(UpdatingInfoTest, IsRequestedHoldResumeReturnsTrueIfAudioDirectionIsChanging)
{
    MediaInfo& objNegotiatedInfo = pUpdatingInfo->GetNegotiatedInfo();
    MediaInfo& objModifyingInfo = pUpdatingInfo->GetModifyingInfo();

    objNegotiatedInfo.eAudioDirection = DIRECTION_SEND;
    objModifyingInfo.eAudioDirection = DIRECTION_SEND_RECEIVE;

    EXPECT_EQ(IMS_TRUE, pUpdatingInfo->IsRequestedHoldResume());
}

TEST_F(UpdatingInfoTest, IsRequestedHoldResumeReturnsTrueIfAudioDirectionIsNotChanging)
{
    MediaInfo& objNegotiatedInfo = pUpdatingInfo->GetNegotiatedInfo();
    MediaInfo& objModifyingInfo = pUpdatingInfo->GetModifyingInfo();

    objNegotiatedInfo.eAudioDirection = objModifyingInfo.eAudioDirection = DIRECTION_SEND;

    EXPECT_EQ(IMS_FALSE, pUpdatingInfo->IsRequestedHoldResume());
}

TEST_F(UpdatingInfoTest, IsRequestedHoldResumeReturnsTrueIfAudioDirectionIsInvalid)
{
    MediaInfo& objNegotiatedInfo = pUpdatingInfo->GetNegotiatedInfo();
    objNegotiatedInfo.eAudioDirection = DIRECTION_INVALID;

    EXPECT_EQ(IMS_FALSE, pUpdatingInfo->IsRequestedHoldResume());
}

TEST_F(UpdatingInfoTest, IsRequestedModifyingReturnsFalseIfModifyingInfoIsInvalid)
{
    MediaInfo& objModifyingInfo = pUpdatingInfo->GetModifyingInfo();
    objModifyingInfo.eAudioDirection = DIRECTION_INVALID;

    EXPECT_EQ(IMS_FALSE, pUpdatingInfo->IsRequestedModifying());
}

TEST_F(UpdatingInfoTest, IsRequestedModifyingReturnsFalseIfAudioDirectionIsChanging)
{
    MediaInfo& objNegotiatedInfo = pUpdatingInfo->GetNegotiatedInfo();
    MediaInfo& objModifyingInfo = pUpdatingInfo->GetModifyingInfo();

    objNegotiatedInfo.eAudioDirection = DIRECTION_SEND;
    objModifyingInfo.eAudioDirection = DIRECTION_SEND_RECEIVE;

    EXPECT_EQ(IMS_FALSE, pUpdatingInfo->IsRequestedModifying());
}

TEST_F(UpdatingInfoTest, IsRequestedModifyingReturnsTrueIfVideoDirectionIsChanging)
{
    MediaInfo& objNegotiatedInfo = pUpdatingInfo->GetNegotiatedInfo();
    MediaInfo& objModifyingInfo = pUpdatingInfo->GetModifyingInfo();
    objModifyingInfo.eAudioDirection = objNegotiatedInfo.eAudioDirection = DIRECTION_SEND;

    objNegotiatedInfo.eVideoDirection = DIRECTION_SEND;
    objModifyingInfo.eVideoDirection = DIRECTION_SEND_RECEIVE;

    EXPECT_EQ(IMS_TRUE, pUpdatingInfo->IsRequestedModifying());
}

TEST_F(UpdatingInfoTest, IsRequestedModifyingReturnsFalseIfVideoDirectionIsNotChanging)
{
    MediaInfo& objNegotiatedInfo = pUpdatingInfo->GetNegotiatedInfo();
    MediaInfo& objModifyingInfo = pUpdatingInfo->GetModifyingInfo();
    objNegotiatedInfo.eAudioDirection = objModifyingInfo.eAudioDirection = DIRECTION_SEND;

    objNegotiatedInfo.eVideoDirection = objModifyingInfo.eVideoDirection = DIRECTION_SEND;

    EXPECT_EQ(IMS_FALSE, pUpdatingInfo->IsRequestedModifying());
}

TEST_F(UpdatingInfoTest, IsRequestedModifyingReturnsTrueIfCallTypeIsChanged)
{
    MediaInfo& objNegotiatedInfo = pUpdatingInfo->GetNegotiatedInfo();
    MediaInfo& objModifyingInfo = pUpdatingInfo->GetModifyingInfo();
    objNegotiatedInfo.eAudioDirection = objModifyingInfo.eAudioDirection = DIRECTION_SEND;

    objNegotiatedInfo.eVideoDirection = objModifyingInfo.eVideoDirection = DIRECTION_SEND;

    pUpdatingInfo->SetTargetCallType(CallType::VOIP);
    ON_CALL(objSession, GetPreviousCallType()).WillByDefault(Return(CallType::VT));

    EXPECT_EQ(IMS_TRUE, pUpdatingInfo->IsRequestedModifying());
}

TEST_F(UpdatingInfoTest, IsModifiedReturnsTrueIfCallTypeIsChanged)
{
    ON_CALL(objSession, GetPreviousCallType()).WillByDefault(Return(CallType::VT));
    ON_CALL(objMediaManager, GetNegotiatedCallType(&objISession))
            .WillByDefault(Return(CallType::VOIP));

    EXPECT_EQ(IMS_TRUE, pUpdatingInfo->IsModified());
}

TEST_F(UpdatingInfoTest, IsModifiedReturnsFalseIfCallTypeIsNotChanged)
{
    ON_CALL(objSession, GetPreviousCallType()).WillByDefault(Return(CallType::VT));
    ON_CALL(objMediaManager, GetNegotiatedCallType(&objISession))
            .WillByDefault(Return(CallType::VT));

    EXPECT_EQ(IMS_FALSE, pUpdatingInfo->IsModified());
}

TEST_F(UpdatingInfoTest, IsDowngradedReturnsTrueIfCallTypeIsDowngraded)
{
    // VT -> VOIP
    ON_CALL(objSession, GetPreviousCallType()).WillByDefault(Return(CallType::VT));
    ON_CALL(objMediaManager, GetNegotiatedCallType(&objISession))
            .WillByDefault(Return(CallType::VOIP));

    EXPECT_EQ(IMS_TRUE, pUpdatingInfo->IsDowngraded());

    // RTT -> VOIP
    ON_CALL(objSession, GetPreviousCallType()).WillByDefault(Return(CallType::RTT));
    ON_CALL(objMediaManager, GetNegotiatedCallType(&objISession))
            .WillByDefault(Return(CallType::VOIP));

    EXPECT_EQ(IMS_TRUE, pUpdatingInfo->IsDowngraded());

    // VIDEO_RTT -> RTT
    ON_CALL(objSession, GetPreviousCallType()).WillByDefault(Return(CallType::VIDEO_RTT));
    ON_CALL(objMediaManager, GetNegotiatedCallType(&objISession))
            .WillByDefault(Return(CallType::RTT));

    EXPECT_EQ(IMS_TRUE, pUpdatingInfo->IsDowngraded());

    // VIDEO_RTT -> VT
    ON_CALL(objSession, GetPreviousCallType()).WillByDefault(Return(CallType::VIDEO_RTT));
    ON_CALL(objMediaManager, GetNegotiatedCallType(&objISession))
            .WillByDefault(Return(CallType::VT));

    EXPECT_EQ(IMS_TRUE, pUpdatingInfo->IsDowngraded());

    // VIDEO_RTT -> VOIP
    ON_CALL(objSession, GetPreviousCallType()).WillByDefault(Return(CallType::VIDEO_RTT));
    ON_CALL(objMediaManager, GetNegotiatedCallType(&objISession))
            .WillByDefault(Return(CallType::VT));

    EXPECT_EQ(IMS_TRUE, pUpdatingInfo->IsDowngraded());
}

TEST_F(UpdatingInfoTest, IsDowngradedReturnsFalseIfCallTypeIsNotModified)
{
    ON_CALL(objSession, GetPreviousCallType()).WillByDefault(Return(CallType::VT));
    ON_CALL(objMediaManager, GetNegotiatedCallType(&objISession))
            .WillByDefault(Return(CallType::VT));

    EXPECT_EQ(IMS_FALSE, pUpdatingInfo->IsDowngraded());
}

TEST_F(UpdatingInfoTest, IsDowngradedReturnsFalseIfCallTypeIsUpgraded)
{
    // VOIP -> VT
    ON_CALL(objSession, GetPreviousCallType()).WillByDefault(Return(CallType::VOIP));
    ON_CALL(objMediaManager, GetNegotiatedCallType(&objISession))
            .WillByDefault(Return(CallType::VT));
    EXPECT_EQ(IMS_FALSE, pUpdatingInfo->IsDowngraded());

    // VOIP -> RTT
    ON_CALL(objSession, GetPreviousCallType()).WillByDefault(Return(CallType::VOIP));
    ON_CALL(objMediaManager, GetNegotiatedCallType(&objISession))
            .WillByDefault(Return(CallType::RTT));
    EXPECT_EQ(IMS_FALSE, pUpdatingInfo->IsDowngraded());

    // VOIP -> VIDEO_RTT
    ON_CALL(objSession, GetPreviousCallType()).WillByDefault(Return(CallType::VOIP));
    ON_CALL(objMediaManager, GetNegotiatedCallType(&objISession))
            .WillByDefault(Return(CallType::VIDEO_RTT));
    EXPECT_EQ(IMS_FALSE, pUpdatingInfo->IsDowngraded());

    // VT -> VIDEO_RTT
    ON_CALL(objSession, GetPreviousCallType()).WillByDefault(Return(CallType::VT));
    ON_CALL(objMediaManager, GetNegotiatedCallType(&objISession))
            .WillByDefault(Return(CallType::VIDEO_RTT));
    EXPECT_EQ(IMS_FALSE, pUpdatingInfo->IsDowngraded());

    // RTT -> VIDEO_RTT
    ON_CALL(objSession, GetPreviousCallType()).WillByDefault(Return(CallType::RTT));
    ON_CALL(objMediaManager, GetNegotiatedCallType(&objISession))
            .WillByDefault(Return(CallType::VIDEO_RTT));
    EXPECT_EQ(IMS_FALSE, pUpdatingInfo->IsDowngraded());

    // Invalid : VT -> RTT
    ON_CALL(objSession, GetPreviousCallType()).WillByDefault(Return(CallType::VT));
    ON_CALL(objMediaManager, GetNegotiatedCallType(&objISession))
            .WillByDefault(Return(CallType::RTT));
    EXPECT_EQ(IMS_FALSE, pUpdatingInfo->IsDowngraded());

    // Invalid : RTT -> VT
    ON_CALL(objSession, GetPreviousCallType()).WillByDefault(Return(CallType::RTT));
    ON_CALL(objMediaManager, GetNegotiatedCallType(&objISession))
            .WillByDefault(Return(CallType::VT));

    EXPECT_EQ(IMS_FALSE, pUpdatingInfo->IsDowngraded());
}
