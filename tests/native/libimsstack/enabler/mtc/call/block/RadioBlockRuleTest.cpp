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
#include "ImsTypeDef.h"
#include "MockIMtcService.h"
#include "call/IMtcCall.h"
#include "call/MockIMtcCallContext.h"
#include "call/block/RadioBlockRule.h"
#include "call/block/MockIMtcBlockRule.h"
#include "call/radio/IMtcRadioChecker.h"
#include "call/radio/MockIMtcRadioChecker.h"
#include <gmock/gmock.h>

using ::testing::_;
using ::testing::Return;
using ::testing::ReturnRef;
using Result = IMtcBlockRule::Result;

LOCAL const CallKey CALL_KEY = 123;

namespace android
{

class RadioBlockRuleTest : public ::testing::Test
{
public:
    MockIMtcCallContext objContext;
    MockIMtcRadioChecker objMockIMtcRadioChecker;
    CallInfo objCallInfo;
    MockIMtcService objMtcService;
    MockIMtcBlockRuleCheckListener BlockRuleCheckListener;

    RadioBlockRule* pRadioBlockRule;

protected:
    virtual void SetUp() override
    {
        ON_CALL(objContext, GetRadioChecker).WillByDefault(ReturnRef(objMockIMtcRadioChecker));
        ON_CALL(objContext, GetCallInfo).WillByDefault(ReturnRef(objCallInfo));
        ON_CALL(objContext, GetService).WillByDefault(ReturnRef(objMtcService));
        ON_CALL(objContext, GetCallKey).WillByDefault(Return(CALL_KEY));

        ON_CALL(objMtcService, IsWlanIpCanType).WillByDefault(Return(IMS_FALSE));
    }

    virtual void TearDown() override { delete pRadioBlockRule; }

    void CreateRadioBlockRuleWithGivenValue(
            IN CallType eCallType, IN PeerType ePeerTypeIn, IN EmergencyType eEmergencyTypeIn)
    {
        objCallInfo.ePeerType = ePeerTypeIn;
        objCallInfo.eEmergencyType = eEmergencyTypeIn;

        pRadioBlockRule = new RadioBlockRule(objContext, eCallType);
    }
};

TEST_F(RadioBlockRuleTest, Check)
{
    CreateRadioBlockRuleWithGivenValue(CallType::VOIP, PeerType::MT, EmergencyType::NONE);

    EXPECT_CALL(objMockIMtcRadioChecker, SetTrafficCheckerListener(_)).Times(4);
    EXPECT_CALL(objMockIMtcRadioChecker,
            Check(CallType::VOIP, IMS_FALSE, PeerType::MT, IMS_FALSE, IMS_FALSE, CALL_KEY))
            .Times(3)
            .WillOnce(Return(CheckResult::UNBLOCKED))
            .WillOnce(Return(CheckResult::PENDING))
            .WillOnce(Return(CheckResult::BLOCKED));

    EXPECT_EQ(Result(Result::Status::UNBLOCKED), pRadioBlockRule->Check(BlockRuleCheckListener));
    EXPECT_EQ(Result(Result::Status::PENDING), pRadioBlockRule->Check(BlockRuleCheckListener));
    EXPECT_EQ(Result(Result::Status::BLOCKED, CallReasonInfo(CODE_LOCAL_NETWORK_NO_SERVICE)),
            pRadioBlockRule->Check(BlockRuleCheckListener));
}

TEST_F(RadioBlockRuleTest, OnConnectionSetupPreparedNotifiesUnblocked)
{
    CreateRadioBlockRuleWithGivenValue(CallType::VOIP, PeerType::MT, EmergencyType::NONE);

    ON_CALL(objMockIMtcRadioChecker, Check(_, _, _, _, _, _))
            .WillByDefault(Return(CheckResult::PENDING));
    pRadioBlockRule->Check(BlockRuleCheckListener);

    EXPECT_CALL(BlockRuleCheckListener, OnBlockRuleChecked(Result(Result::Status::UNBLOCKED)))
            .Times(1);
    pRadioBlockRule->OnConnectionSetupPrepared();
}

TEST_F(RadioBlockRuleTest, OnConnectionFailedNotifiesNoService)
{
    CreateRadioBlockRuleWithGivenValue(
            CallType::VT, PeerType::MO, EmergencyType::EMERGENCY_ROUTING);

    ON_CALL(objMockIMtcRadioChecker, Check(_, _, _, _, _, _))
            .WillByDefault(Return(CheckResult::PENDING));
    pRadioBlockRule->Check(BlockRuleCheckListener);

    const IMS_UINT32 nRejectWaitTimeMillis = 2;
    const IMS_UINT32 nFailureReason = IImsRadio::REASON_NAS_FAILURE;

    EXPECT_CALL(BlockRuleCheckListener,
            OnBlockRuleChecked(
                    Result(Result::Status::BLOCKED, CallReasonInfo(CODE_LOCAL_NETWORK_NO_SERVICE))))
            .Times(1);
    pRadioBlockRule->OnConnectionFailed(nFailureReason, nRejectWaitTimeMillis);
}

TEST_F(RadioBlockRuleTest, OnConnectionFailedNotifiesAcBlockedWhenAccessDenied)
{
    CreateRadioBlockRuleWithGivenValue(
            CallType::VT, PeerType::MO, EmergencyType::EMERGENCY_ROUTING);

    ON_CALL(objMockIMtcRadioChecker, Check(_, _, _, _, _, _))
            .WillByDefault(Return(CheckResult::PENDING));
    pRadioBlockRule->Check(BlockRuleCheckListener);

    const IMS_UINT32 nRejectWaitTimeMillis = 2;
    const IMS_UINT32 nFailureReason = IImsRadio::REASON_ACCESS_DENIED;

    EXPECT_CALL(BlockRuleCheckListener,
            OnBlockRuleChecked(
                    Result(Result::Status::BLOCKED, CallReasonInfo(CODE_ACCESS_CLASS_BLOCKED))))
            .Times(1);
    pRadioBlockRule->OnConnectionFailed(nFailureReason, nRejectWaitTimeMillis);
}

TEST_F(RadioBlockRuleTest, OnConnectionFailedNotifiesRrcRejectWhenRrcReject)
{
    CreateRadioBlockRuleWithGivenValue(
            CallType::VT, PeerType::MO, EmergencyType::EMERGENCY_ROUTING);

    ON_CALL(objMockIMtcRadioChecker, Check(_, _, _, _, _, _))
            .WillByDefault(Return(CheckResult::PENDING));
    pRadioBlockRule->Check(BlockRuleCheckListener);

    const IMS_UINT32 nRejectWaitTimeMillis = 2;
    const IMS_UINT32 nFailureReason = IImsRadio::REASON_RRC_REJECT;

    EXPECT_CALL(BlockRuleCheckListener,
            OnBlockRuleChecked(
                    Result(Result::Status::BLOCKED, CallReasonInfo(CODE_INTERNAL_RRC_REJECT, 2))))
            .Times(1);
    pRadioBlockRule->OnConnectionFailed(nFailureReason, nRejectWaitTimeMillis);
}

TEST_F(RadioBlockRuleTest, OnConnectionFailedNotifiesRadioInternalErrorWhenInternalError)
{
    CreateRadioBlockRuleWithGivenValue(
            CallType::VT, PeerType::MO, EmergencyType::EMERGENCY_ROUTING);

    ON_CALL(objMockIMtcRadioChecker, Check(_, _, _, _, _, _))
            .WillByDefault(Return(CheckResult::PENDING));
    pRadioBlockRule->Check(BlockRuleCheckListener);

    const IMS_UINT32 nRejectWaitTimeMillis = 2;
    const IMS_UINT32 nFailureReason = IImsRadio::REASON_INTERNAL_ERROR;

    EXPECT_CALL(BlockRuleCheckListener,
            OnBlockRuleChecked(
                    Result(Result::Status::BLOCKED, CallReasonInfo(CODE_RADIO_INTERNAL_ERROR))))
            .Times(1);
    pRadioBlockRule->OnConnectionFailed(nFailureReason, nRejectWaitTimeMillis);
}

}  // namespace android
