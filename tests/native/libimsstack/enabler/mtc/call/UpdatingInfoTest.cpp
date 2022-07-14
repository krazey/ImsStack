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

#include <gtest/gtest.h>
#include "MtcDef.h"
#include "call/IMtcCall.h"
#include "call/UpdatingInfo.h"

class UpdatingInfoTest : public ::testing::Test
{
public:
    UpdatingInfo objUpdatingInfo;

protected:
    virtual void SetUp() override {}
    virtual void TearDown() override {}
};

TEST_F(UpdatingInfoTest, GetTargetCallTypeReturnsUnknownInitially)
{
    EXPECT_EQ(CallType::UNKNOWN, objUpdatingInfo.GetTargetCallType());
}

TEST_F(UpdatingInfoTest, GetTargetCallTypeReturnsSetValue)
{
    CallType eCallType = CallType::VOIP;

    objUpdatingInfo.SetTargetCallType(eCallType);

    EXPECT_EQ(eCallType, objUpdatingInfo.GetTargetCallType());
}

TEST_F(UpdatingInfoTest, GetModifyingInfoReturnsNotNull)
{
    EXPECT_NE(nullptr, &objUpdatingInfo.GetModifyingInfo());
}

TEST_F(UpdatingInfoTest, GetAlertingInfoReturnsNotNull)
{
    EXPECT_NE(nullptr, &objUpdatingInfo.GetAlertingInfo());
}

TEST_F(UpdatingInfoTest, GetModifiedInfoReturnsNotNull)
{
    EXPECT_NE(nullptr, &objUpdatingInfo.GetModifiedInfo());
}

TEST_F(UpdatingInfoTest, IsModifierReturnsFalseInitially)
{
    EXPECT_EQ(IMS_FALSE, objUpdatingInfo.IsModifier());
}

TEST_F(UpdatingInfoTest, IsModifierReturnsTrueAfterSet)
{
    objUpdatingInfo.SetModifier();
    EXPECT_EQ(IMS_TRUE, objUpdatingInfo.IsModifier());
}

TEST_F(UpdatingInfoTest, IsAlertedReturnsFalseInitially)
{
    EXPECT_EQ(IMS_FALSE, objUpdatingInfo.IsAlerted());
}

TEST_F(UpdatingInfoTest, IsAlertedReturnsTrueAfterSet)
{
    objUpdatingInfo.SetAlerted();
    EXPECT_EQ(IMS_TRUE, objUpdatingInfo.IsAlerted());
}

TEST_F(UpdatingInfoTest, ReturnHoldResumeStatusIfModifyingSendReceiveToHeld)
{
    MediaInfo& objNegotiatedInfo = objUpdatingInfo.GetNegotiatedInfo();
    MediaInfo& objModifyingInfo = objUpdatingInfo.GetModifyingInfo();

    objNegotiatedInfo.eADir = DIRECTION_SEND_RECEIVE;
    objModifyingInfo.eADir = DIRECTION_SEND;

    EXPECT_EQ(IMS_TRUE, objUpdatingInfo.IsHeld());
    EXPECT_EQ(IMS_FALSE, objUpdatingInfo.IsHeldBy());
    EXPECT_EQ(IMS_FALSE, objUpdatingInfo.IsResumed());
    EXPECT_EQ(IMS_FALSE, objUpdatingInfo.IsResumedBy());
}

TEST_F(UpdatingInfoTest, ReturnHoldResumeStatusIfModifyingSendReceiveToInactive)
{
    MediaInfo& objNegotiatedInfo = objUpdatingInfo.GetNegotiatedInfo();
    MediaInfo& objModifyingInfo = objUpdatingInfo.GetModifyingInfo();

    objNegotiatedInfo.eADir = DIRECTION_SEND_RECEIVE;
    objModifyingInfo.eADir = DIRECTION_INACTIVE;

    EXPECT_EQ(IMS_TRUE, objUpdatingInfo.IsHeld());
    EXPECT_EQ(IMS_FALSE, objUpdatingInfo.IsHeldBy());
    EXPECT_EQ(IMS_FALSE, objUpdatingInfo.IsResumed());
    EXPECT_EQ(IMS_FALSE, objUpdatingInfo.IsResumedBy());
}

TEST_F(UpdatingInfoTest, ReturnHoldResumeStatusIfModifyingReceiveToInactive)
{
    MediaInfo& objNegotiatedInfo = objUpdatingInfo.GetNegotiatedInfo();
    MediaInfo& objModifyingInfo = objUpdatingInfo.GetModifyingInfo();

    objNegotiatedInfo.eADir = DIRECTION_RECEIVE;
    objModifyingInfo.eADir = DIRECTION_INACTIVE;

    EXPECT_EQ(IMS_TRUE, objUpdatingInfo.IsHeld());
    EXPECT_EQ(IMS_FALSE, objUpdatingInfo.IsHeldBy());
    EXPECT_EQ(IMS_FALSE, objUpdatingInfo.IsResumed());
    EXPECT_EQ(IMS_FALSE, objUpdatingInfo.IsResumedBy());
}

TEST_F(UpdatingInfoTest, ReturnHoldResumeStatusIfModifiedSendReceiveToReceive)
{
    MediaInfo& objNegotiatedInfo = objUpdatingInfo.GetNegotiatedInfo();
    MediaInfo& objModifiedInfo = objUpdatingInfo.GetModifiedInfo();

    objNegotiatedInfo.eADir = DIRECTION_SEND_RECEIVE;
    objModifiedInfo.eADir = DIRECTION_RECEIVE;

    EXPECT_EQ(IMS_FALSE, objUpdatingInfo.IsHeld());
    EXPECT_EQ(IMS_TRUE, objUpdatingInfo.IsHeldBy());
    EXPECT_EQ(IMS_FALSE, objUpdatingInfo.IsResumed());
    EXPECT_EQ(IMS_FALSE, objUpdatingInfo.IsResumedBy());

    EXPECT_EQ(IMS_FALSE, objUpdatingInfo.IsNeedToAlert());
}

TEST_F(UpdatingInfoTest, ReturnHoldResumeStatusIfModifiedSendReceiveToInactive)
{
    MediaInfo& objNegotiatedInfo = objUpdatingInfo.GetNegotiatedInfo();
    MediaInfo& objModifiedInfo = objUpdatingInfo.GetModifiedInfo();
    MediaInfo& objModifyingInfo = objUpdatingInfo.GetModifyingInfo();
    objModifyingInfo.eADir = DIRECTION_INVALID;

    objNegotiatedInfo.eADir = DIRECTION_SEND_RECEIVE;
    objModifiedInfo.eADir = DIRECTION_INACTIVE;

    EXPECT_EQ(IMS_FALSE, objUpdatingInfo.IsHeld());
    EXPECT_EQ(IMS_TRUE, objUpdatingInfo.IsHeldBy());
    EXPECT_EQ(IMS_FALSE, objUpdatingInfo.IsResumed());
    EXPECT_EQ(IMS_FALSE, objUpdatingInfo.IsResumedBy());

    EXPECT_EQ(IMS_FALSE, objUpdatingInfo.IsNeedToAlert());
}

TEST_F(UpdatingInfoTest, ReturnHoldResumeStatusIfModifiedSendToInactive)
{
    MediaInfo& objNegotiatedInfo = objUpdatingInfo.GetNegotiatedInfo();
    MediaInfo& objModifiedInfo = objUpdatingInfo.GetModifiedInfo();

    objNegotiatedInfo.eADir = DIRECTION_SEND;
    objModifiedInfo.eADir = DIRECTION_INACTIVE;

    EXPECT_EQ(IMS_FALSE, objUpdatingInfo.IsHeld());
    EXPECT_EQ(IMS_TRUE, objUpdatingInfo.IsHeldBy());
    EXPECT_EQ(IMS_FALSE, objUpdatingInfo.IsResumed());
    EXPECT_EQ(IMS_FALSE, objUpdatingInfo.IsResumedBy());

    EXPECT_EQ(IMS_FALSE, objUpdatingInfo.IsNeedToAlert());
}

TEST_F(UpdatingInfoTest, ReturnHoldResumeStatusIfAlertingSendReceiveToReceive)
{
    MediaInfo& objNegotiatedInfo = objUpdatingInfo.GetNegotiatedInfo();
    MediaInfo& objAlertingInfo = objUpdatingInfo.GetAlertingInfo();
    MediaInfo& objModifiedInfo = objUpdatingInfo.GetModifiedInfo();
    objModifiedInfo.eADir = DIRECTION_INVALID;

    objNegotiatedInfo.eADir = DIRECTION_SEND_RECEIVE;
    objAlertingInfo.eADir = DIRECTION_RECEIVE;

    EXPECT_EQ(IMS_FALSE, objUpdatingInfo.IsHeld());
    EXPECT_EQ(IMS_TRUE, objUpdatingInfo.IsHeldBy());
    EXPECT_EQ(IMS_FALSE, objUpdatingInfo.IsResumed());
    EXPECT_EQ(IMS_FALSE, objUpdatingInfo.IsResumedBy());

    EXPECT_EQ(IMS_FALSE, objUpdatingInfo.IsNeedToAlert());
}

TEST_F(UpdatingInfoTest, ReturnHoldResumeStatusIfAlertingSendReceiveToInactive)
{
    MediaInfo& objNegotiatedInfo = objUpdatingInfo.GetNegotiatedInfo();
    MediaInfo& objAlertingInfo = objUpdatingInfo.GetAlertingInfo();
    MediaInfo& objModifiedInfo = objUpdatingInfo.GetModifiedInfo();
    MediaInfo& objModifyingInfo = objUpdatingInfo.GetModifyingInfo();
    objModifiedInfo.eADir = DIRECTION_INVALID;
    objModifyingInfo.eADir = DIRECTION_INVALID;

    objNegotiatedInfo.eADir = DIRECTION_SEND_RECEIVE;
    objAlertingInfo.eADir = DIRECTION_INACTIVE;

    EXPECT_EQ(IMS_FALSE, objUpdatingInfo.IsHeld());
    EXPECT_EQ(IMS_TRUE, objUpdatingInfo.IsHeldBy());
    EXPECT_EQ(IMS_FALSE, objUpdatingInfo.IsResumed());
    EXPECT_EQ(IMS_FALSE, objUpdatingInfo.IsResumedBy());

    EXPECT_EQ(IMS_FALSE, objUpdatingInfo.IsNeedToAlert());
}

TEST_F(UpdatingInfoTest, ReturnHoldResumeStatusIfAlertingSendToInactive)
{
    MediaInfo& objNegotiatedInfo = objUpdatingInfo.GetNegotiatedInfo();
    MediaInfo& objAlertingInfo = objUpdatingInfo.GetAlertingInfo();
    MediaInfo& objModifiedInfo = objUpdatingInfo.GetModifiedInfo();
    objModifiedInfo.eADir = DIRECTION_INVALID;

    objNegotiatedInfo.eADir = DIRECTION_SEND;
    objAlertingInfo.eADir = DIRECTION_INACTIVE;

    EXPECT_EQ(IMS_FALSE, objUpdatingInfo.IsHeld());
    EXPECT_EQ(IMS_TRUE, objUpdatingInfo.IsHeldBy());
    EXPECT_EQ(IMS_FALSE, objUpdatingInfo.IsResumed());
    EXPECT_EQ(IMS_FALSE, objUpdatingInfo.IsResumedBy());

    EXPECT_EQ(IMS_FALSE, objUpdatingInfo.IsNeedToAlert());
}

TEST_F(UpdatingInfoTest, ReturnHoldResumeStatusIfModifyingSendToSendReceive)
{
    MediaInfo& objNegotiatedInfo = objUpdatingInfo.GetNegotiatedInfo();
    MediaInfo& objModifyingInfo = objUpdatingInfo.GetModifyingInfo();

    objNegotiatedInfo.eADir = DIRECTION_SEND;
    objModifyingInfo.eADir = DIRECTION_SEND_RECEIVE;

    EXPECT_EQ(IMS_FALSE, objUpdatingInfo.IsHeld());
    EXPECT_EQ(IMS_FALSE, objUpdatingInfo.IsHeldBy());
    EXPECT_EQ(IMS_TRUE, objUpdatingInfo.IsResumed());
    EXPECT_EQ(IMS_FALSE, objUpdatingInfo.IsResumedBy());
}

TEST_F(UpdatingInfoTest, ReturnHoldResumeStatusIfModifyingInactiveToReceive)
{
    MediaInfo& objNegotiatedInfo = objUpdatingInfo.GetNegotiatedInfo();
    MediaInfo& objModifyingInfo = objUpdatingInfo.GetModifyingInfo();

    objNegotiatedInfo.eADir = DIRECTION_INACTIVE;
    objModifyingInfo.eADir = DIRECTION_RECEIVE;

    EXPECT_EQ(IMS_FALSE, objUpdatingInfo.IsHeld());
    EXPECT_EQ(IMS_FALSE, objUpdatingInfo.IsHeldBy());
    EXPECT_EQ(IMS_TRUE, objUpdatingInfo.IsResumed());
    EXPECT_EQ(IMS_FALSE, objUpdatingInfo.IsResumedBy());
}

TEST_F(UpdatingInfoTest, ReturnHoldResumeStatusIfModifyingInactiveToSendReceive)
{
    MediaInfo& objNegotiatedInfo = objUpdatingInfo.GetNegotiatedInfo();
    MediaInfo& objModifyingInfo = objUpdatingInfo.GetModifyingInfo();

    objNegotiatedInfo.eADir = DIRECTION_INACTIVE;
    objModifyingInfo.eADir = DIRECTION_SEND_RECEIVE;

    EXPECT_EQ(IMS_FALSE, objUpdatingInfo.IsHeld());
    EXPECT_EQ(IMS_FALSE, objUpdatingInfo.IsHeldBy());
    EXPECT_EQ(IMS_TRUE, objUpdatingInfo.IsResumed());
    EXPECT_EQ(IMS_FALSE, objUpdatingInfo.IsResumedBy());
}

TEST_F(UpdatingInfoTest, ReturnHoldResumeStatusIfModifiedReceiveToSendReceive)
{
    MediaInfo& objNegotiatedInfo = objUpdatingInfo.GetNegotiatedInfo();
    MediaInfo& objModifiedInfo = objUpdatingInfo.GetModifiedInfo();

    objModifiedInfo.eADir = DIRECTION_SEND_RECEIVE;
    objNegotiatedInfo.eADir = DIRECTION_RECEIVE;

    EXPECT_EQ(IMS_FALSE, objUpdatingInfo.IsHeld());
    EXPECT_EQ(IMS_FALSE, objUpdatingInfo.IsHeldBy());
    EXPECT_EQ(IMS_FALSE, objUpdatingInfo.IsResumed());
    EXPECT_EQ(IMS_TRUE, objUpdatingInfo.IsResumedBy());

    EXPECT_EQ(IMS_FALSE, objUpdatingInfo.IsNeedToAlert());
}

TEST_F(UpdatingInfoTest, ReturnHoldResumeStatusIfModifiedInactiveToSend)
{
    MediaInfo& objNegotiatedInfo = objUpdatingInfo.GetNegotiatedInfo();
    MediaInfo& objModifiedInfo = objUpdatingInfo.GetModifiedInfo();

    objNegotiatedInfo.eADir = DIRECTION_INACTIVE;
    objModifiedInfo.eADir = DIRECTION_SEND;

    EXPECT_EQ(IMS_FALSE, objUpdatingInfo.IsHeld());
    EXPECT_EQ(IMS_FALSE, objUpdatingInfo.IsHeldBy());
    EXPECT_EQ(IMS_FALSE, objUpdatingInfo.IsResumed());
    EXPECT_EQ(IMS_TRUE, objUpdatingInfo.IsResumedBy());

    EXPECT_EQ(IMS_FALSE, objUpdatingInfo.IsNeedToAlert());
}

TEST_F(UpdatingInfoTest, ReturnHoldResumeStatusIfModifiedInactiveToSendReceive)
{
    MediaInfo& objNegotiatedInfo = objUpdatingInfo.GetNegotiatedInfo();
    MediaInfo& objModifiedInfo = objUpdatingInfo.GetModifiedInfo();
    MediaInfo& objModifyingInfo = objUpdatingInfo.GetModifyingInfo();
    objModifyingInfo.eADir = DIRECTION_INVALID;

    objNegotiatedInfo.eADir = DIRECTION_INACTIVE;
    objModifiedInfo.eADir = DIRECTION_SEND_RECEIVE;

    EXPECT_EQ(IMS_FALSE, objUpdatingInfo.IsHeld());
    EXPECT_EQ(IMS_FALSE, objUpdatingInfo.IsHeldBy());
    EXPECT_EQ(IMS_FALSE, objUpdatingInfo.IsResumed());
    EXPECT_EQ(IMS_TRUE, objUpdatingInfo.IsResumedBy());

    EXPECT_EQ(IMS_FALSE, objUpdatingInfo.IsNeedToAlert());
}

TEST_F(UpdatingInfoTest, ReturnHoldResumeStatusIfAlertingReceiveToSendReceive)
{
    MediaInfo& objNegotiatedInfo = objUpdatingInfo.GetNegotiatedInfo();
    MediaInfo& objAlertingInfo = objUpdatingInfo.GetAlertingInfo();
    MediaInfo& objModifiedInfo = objUpdatingInfo.GetModifiedInfo();
    objModifiedInfo.eADir = DIRECTION_INVALID;

    objNegotiatedInfo.eADir = DIRECTION_RECEIVE;
    objAlertingInfo.eADir = DIRECTION_SEND_RECEIVE;

    EXPECT_EQ(IMS_FALSE, objUpdatingInfo.IsHeld());
    EXPECT_EQ(IMS_FALSE, objUpdatingInfo.IsHeldBy());
    EXPECT_EQ(IMS_FALSE, objUpdatingInfo.IsResumed());
    EXPECT_EQ(IMS_TRUE, objUpdatingInfo.IsResumedBy());

    EXPECT_EQ(IMS_FALSE, objUpdatingInfo.IsNeedToAlert());
}

TEST_F(UpdatingInfoTest, ReturnHoldResumeStatusIfAlertingInactiveToSend)
{
    MediaInfo& objNegotiatedInfo = objUpdatingInfo.GetNegotiatedInfo();
    MediaInfo& objAlertingInfo = objUpdatingInfo.GetAlertingInfo();
    MediaInfo& objModifiedInfo = objUpdatingInfo.GetModifiedInfo();
    objModifiedInfo.eADir = DIRECTION_INVALID;

    objNegotiatedInfo.eADir = DIRECTION_INACTIVE;
    objAlertingInfo.eADir = DIRECTION_SEND;

    EXPECT_EQ(IMS_FALSE, objUpdatingInfo.IsHeld());
    EXPECT_EQ(IMS_FALSE, objUpdatingInfo.IsHeldBy());
    EXPECT_EQ(IMS_FALSE, objUpdatingInfo.IsResumed());
    EXPECT_EQ(IMS_TRUE, objUpdatingInfo.IsResumedBy());

    EXPECT_EQ(IMS_FALSE, objUpdatingInfo.IsNeedToAlert());
}

TEST_F(UpdatingInfoTest, ReturnHoldResumeStatusIfAlertingInactiveToSendReceive)
{
    MediaInfo& objNegotiatedInfo = objUpdatingInfo.GetNegotiatedInfo();
    MediaInfo& objAlertingInfo = objUpdatingInfo.GetAlertingInfo();
    MediaInfo& objModifiedInfo = objUpdatingInfo.GetModifiedInfo();
    MediaInfo& objModifyingInfo = objUpdatingInfo.GetModifyingInfo();
    objModifiedInfo.eADir = DIRECTION_INVALID;
    objModifyingInfo.eADir = DIRECTION_INVALID;

    objNegotiatedInfo.eADir = DIRECTION_INACTIVE;
    objAlertingInfo.eADir = DIRECTION_SEND_RECEIVE;

    EXPECT_EQ(IMS_FALSE, objUpdatingInfo.IsHeld());
    EXPECT_EQ(IMS_FALSE, objUpdatingInfo.IsHeldBy());
    EXPECT_EQ(IMS_FALSE, objUpdatingInfo.IsResumed());
    EXPECT_EQ(IMS_TRUE, objUpdatingInfo.IsResumedBy());

    EXPECT_EQ(IMS_FALSE, objUpdatingInfo.IsNeedToAlert());
}

TEST_F(UpdatingInfoTest, IsNeedToAlertReturnsTrueIfVideoDirectionIsChanging)
{
    MediaInfo& objNegotiatedInfo = objUpdatingInfo.GetNegotiatedInfo();
    MediaInfo& objModifyingInfo = objUpdatingInfo.GetModifyingInfo();
    MediaInfo& objAlertingInfo = objUpdatingInfo.GetAlertingInfo();

    // Not (IsHeldBy() || IsResumedBy())
    objNegotiatedInfo.eADir = DIRECTION_INACTIVE;
    objModifyingInfo.eADir = DIRECTION_SEND_RECEIVE;

    objNegotiatedInfo.eVDir = DIRECTION_SEND;
    objAlertingInfo.eVDir = DIRECTION_SEND_RECEIVE;

    EXPECT_EQ(IMS_TRUE, objUpdatingInfo.IsNeedToAlert());
}

TEST_F(UpdatingInfoTest, IsNeedToAlertReturnsTrueIfTextDirectionIsChanging)
{
    MediaInfo& objNegotiatedInfo = objUpdatingInfo.GetNegotiatedInfo();
    MediaInfo& objModifyingInfo = objUpdatingInfo.GetModifyingInfo();
    MediaInfo& objAlertingInfo = objUpdatingInfo.GetAlertingInfo();

    // Not (IsHeldBy() || IsResumedBy())
    objNegotiatedInfo.eADir = DIRECTION_INACTIVE;
    objModifyingInfo.eADir = DIRECTION_SEND_RECEIVE;

    objNegotiatedInfo.eTDir = DIRECTION_SEND;
    objAlertingInfo.eTDir = DIRECTION_SEND_RECEIVE;

    EXPECT_EQ(IMS_TRUE, objUpdatingInfo.IsNeedToAlert());
}

TEST_F(UpdatingInfoTest, IsNeedToAlertReturnsFalseIfVideoTextDirectionIsNotChanging)
{
    MediaInfo& objNegotiatedInfo = objUpdatingInfo.GetNegotiatedInfo();
    MediaInfo& objModifyingInfo = objUpdatingInfo.GetModifyingInfo();
    MediaInfo& objAlertingInfo = objUpdatingInfo.GetAlertingInfo();

    // Not (IsHeldBy() || IsResumedBy())
    objNegotiatedInfo.eADir = DIRECTION_INACTIVE;
    objModifyingInfo.eADir = DIRECTION_SEND_RECEIVE;

    objNegotiatedInfo.eVDir = objAlertingInfo.eVDir = DIRECTION_SEND;
    objNegotiatedInfo.eTDir = objAlertingInfo.eTDir = DIRECTION_SEND;

    EXPECT_EQ(IMS_FALSE, objUpdatingInfo.IsNeedToAlert());
}

TEST_F(UpdatingInfoTest, IsRequestedHoldResumeReturnsTrueIfAudioDirectionIsChanging)
{
    MediaInfo& objNegotiatedInfo = objUpdatingInfo.GetNegotiatedInfo();
    MediaInfo& objModifyingInfo = objUpdatingInfo.GetModifyingInfo();

    objNegotiatedInfo.eADir = DIRECTION_SEND;
    objModifyingInfo.eADir = DIRECTION_SEND_RECEIVE;

    EXPECT_EQ(IMS_TRUE, objUpdatingInfo.IsRequestedHoldResume());
}

TEST_F(UpdatingInfoTest, IsRequestedHoldResumeReturnsTrueIfAudioDirectionIsNotChanging)
{
    MediaInfo& objNegotiatedInfo = objUpdatingInfo.GetNegotiatedInfo();
    MediaInfo& objModifyingInfo = objUpdatingInfo.GetModifyingInfo();

    objNegotiatedInfo.eADir = objModifyingInfo.eADir = DIRECTION_SEND;

    EXPECT_EQ(IMS_FALSE, objUpdatingInfo.IsRequestedHoldResume());
}

TEST_F(UpdatingInfoTest, IsRequestedHoldResumeReturnsTrueIfAudioDirectionIsInvalid)
{
    MediaInfo& objNegotiatedInfo = objUpdatingInfo.GetNegotiatedInfo();
    objNegotiatedInfo.eADir = DIRECTION_INVALID;

    EXPECT_EQ(IMS_FALSE, objUpdatingInfo.IsRequestedHoldResume());
}

TEST_F(UpdatingInfoTest, IsRequestedModifyingReturnsFalseIfModifyingInfoIsInvalid)
{
    MediaInfo& objModifyingInfo = objUpdatingInfo.GetModifyingInfo();
    objModifyingInfo.eADir = DIRECTION_INVALID;

    EXPECT_EQ(IMS_FALSE, objUpdatingInfo.IsRequestedModifying());
}

TEST_F(UpdatingInfoTest, IsRequestedModifyingReturnsFalseIfAudioDirectionIsChanging)
{
    MediaInfo& objNegotiatedInfo = objUpdatingInfo.GetNegotiatedInfo();
    MediaInfo& objModifyingInfo = objUpdatingInfo.GetModifyingInfo();

    objNegotiatedInfo.eADir = DIRECTION_SEND;
    objModifyingInfo.eADir = DIRECTION_SEND_RECEIVE;

    EXPECT_EQ(IMS_FALSE, objUpdatingInfo.IsRequestedModifying());
}

TEST_F(UpdatingInfoTest, IsRequestedModifyingReturnsTrueIfVideoDirectionIsChanging)
{
    MediaInfo& objNegotiatedInfo = objUpdatingInfo.GetNegotiatedInfo();
    MediaInfo& objModifyingInfo = objUpdatingInfo.GetModifyingInfo();
    objModifyingInfo.eADir = objNegotiatedInfo.eADir = DIRECTION_SEND;

    objNegotiatedInfo.eVDir = DIRECTION_SEND;
    objModifyingInfo.eVDir = DIRECTION_SEND_RECEIVE;

    EXPECT_EQ(IMS_TRUE, objUpdatingInfo.IsRequestedModifying());
}

TEST_F(UpdatingInfoTest, IsRequestedModifyingReturnsTrueIfTextDirectionIsChanging)
{
    MediaInfo& objNegotiatedInfo = objUpdatingInfo.GetNegotiatedInfo();
    MediaInfo& objModifyingInfo = objUpdatingInfo.GetModifyingInfo();
    objNegotiatedInfo.eADir = DIRECTION_SEND;
    objModifyingInfo.eADir = objNegotiatedInfo.eADir;

    objNegotiatedInfo.eTDir = DIRECTION_SEND;
    objModifyingInfo.eTDir = DIRECTION_SEND_RECEIVE;

    EXPECT_EQ(IMS_TRUE, objUpdatingInfo.IsRequestedModifying());
}

TEST_F(UpdatingInfoTest, IsRequestedModifyingReturnsFalseIfVideoTextDirectionIsNotChanging)
{
    MediaInfo& objNegotiatedInfo = objUpdatingInfo.GetNegotiatedInfo();
    MediaInfo& objModifyingInfo = objUpdatingInfo.GetModifyingInfo();
    objNegotiatedInfo.eADir = objModifyingInfo.eADir = DIRECTION_SEND;

    objNegotiatedInfo.eVDir = objModifyingInfo.eVDir = DIRECTION_SEND;
    objNegotiatedInfo.eTDir = objModifyingInfo.eTDir = DIRECTION_SEND;

    EXPECT_EQ(IMS_FALSE, objUpdatingInfo.IsRequestedModifying());
}

TEST_F(UpdatingInfoTest, IsModifiedReturnsTrueIfVideoDirectionIsChangingToInvalid)
{
    MediaInfo& objNegotiatedInfo = objUpdatingInfo.GetNegotiatedInfo();
    MediaInfo& objModifiedInfo = objUpdatingInfo.GetModifiedInfo();
    objNegotiatedInfo.eTDir = objModifiedInfo.eTDir = DIRECTION_SEND;

    objNegotiatedInfo.eVDir = DIRECTION_SEND;
    objModifiedInfo.eVDir = DIRECTION_INVALID;

    EXPECT_EQ(IMS_TRUE, objUpdatingInfo.IsModified());
}

TEST_F(UpdatingInfoTest, IsModifiedReturnsTrueIfVideoDirectionIsChangingFromInvalid)
{
    MediaInfo& objNegotiatedInfo = objUpdatingInfo.GetNegotiatedInfo();
    MediaInfo& objModifiedInfo = objUpdatingInfo.GetModifiedInfo();
    objNegotiatedInfo.eTDir = objModifiedInfo.eTDir = DIRECTION_SEND;

    objNegotiatedInfo.eVDir = DIRECTION_INVALID;
    objModifiedInfo.eVDir = DIRECTION_SEND;

    EXPECT_EQ(IMS_TRUE, objUpdatingInfo.IsModified());
}

TEST_F(UpdatingInfoTest, IsModifiedReturnsFalseIfVideoDirectionIsChangingNotInvalid)
{
    MediaInfo& objNegotiatedInfo = objUpdatingInfo.GetNegotiatedInfo();
    MediaInfo& objModifiedInfo = objUpdatingInfo.GetModifiedInfo();
    objNegotiatedInfo.eTDir = objModifiedInfo.eTDir = DIRECTION_SEND;

    objNegotiatedInfo.eVDir = DIRECTION_SEND;
    objModifiedInfo.eVDir = DIRECTION_SEND_RECEIVE;

    EXPECT_EQ(IMS_FALSE, objUpdatingInfo.IsModified());
}

TEST_F(UpdatingInfoTest, IsModifiedReturnsTrueIfTextDirectionIsChangingToInvalid)
{
    MediaInfo& objNegotiatedInfo = objUpdatingInfo.GetNegotiatedInfo();
    MediaInfo& objModifiedInfo = objUpdatingInfo.GetModifiedInfo();
    objNegotiatedInfo.eVDir = objModifiedInfo.eVDir = DIRECTION_SEND;

    objNegotiatedInfo.eTDir = DIRECTION_SEND;
    objModifiedInfo.eTDir = DIRECTION_INVALID;

    EXPECT_EQ(IMS_TRUE, objUpdatingInfo.IsModified());
}

TEST_F(UpdatingInfoTest, IsModifiedReturnsTrueIfTextDirectionIsChangingFromInvalid)
{
    MediaInfo& objNegotiatedInfo = objUpdatingInfo.GetNegotiatedInfo();
    MediaInfo& objModifiedInfo = objUpdatingInfo.GetModifiedInfo();
    objNegotiatedInfo.eVDir = objModifiedInfo.eVDir = DIRECTION_SEND;

    objNegotiatedInfo.eTDir = DIRECTION_INVALID;
    objModifiedInfo.eTDir = DIRECTION_SEND;

    EXPECT_EQ(IMS_TRUE, objUpdatingInfo.IsModified());
}

TEST_F(UpdatingInfoTest, IsModifiedReturnsFalseIfTextDirectionIsChangingNotInvalid)
{
    MediaInfo& objNegotiatedInfo = objUpdatingInfo.GetNegotiatedInfo();
    MediaInfo& objModifiedInfo = objUpdatingInfo.GetModifiedInfo();
    objNegotiatedInfo.eVDir = objModifiedInfo.eVDir = DIRECTION_SEND;

    objNegotiatedInfo.eTDir = DIRECTION_SEND;
    objModifiedInfo.eTDir = DIRECTION_SEND_RECEIVE;

    EXPECT_EQ(IMS_FALSE, objUpdatingInfo.IsModified());
}

TEST_F(UpdatingInfoTest, IsModifiedReturnsTrueIfVideoTextDirectionIsNotChanging)
{
    MediaInfo& objNegotiatedInfo = objUpdatingInfo.GetNegotiatedInfo();
    MediaInfo& objModifiedInfo = objUpdatingInfo.GetModifiedInfo();

    objNegotiatedInfo.eVDir = objModifiedInfo.eVDir = DIRECTION_SEND;
    objNegotiatedInfo.eTDir = objModifiedInfo.eTDir = DIRECTION_SEND;

    EXPECT_EQ(IMS_FALSE, objUpdatingInfo.IsModified());
}
