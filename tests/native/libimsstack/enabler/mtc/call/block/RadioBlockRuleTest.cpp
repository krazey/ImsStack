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

namespace android
{

class RadioBlockRuleTest : public ::testing::Test
{
public:
    MockIMtcCallContext m_objContext;
    MockIMtcRadioChecker m_objMockIMtcRadioChecker;
    CallInfo m_objCallInfo;
    MockIMtcService m_objMtcService;
    MockIMtcBlockRuleCheckListener m_BlockRuleCheckListener;
    RadioBlockRule* m_pRadioBlockRule;

protected:
    virtual void SetUp() override
    {
        ON_CALL(m_objContext, GetRadioChecker).WillByDefault(ReturnRef(m_objMockIMtcRadioChecker));
        ON_CALL(m_objContext, GetCallInfo).WillByDefault(ReturnRef(m_objCallInfo));
        ON_CALL(m_objContext, GetService).WillByDefault(ReturnRef(m_objMtcService));

        ON_CALL(m_objMtcService, IsWlanIpCanType).WillByDefault(Return(IMS_FALSE));
    }

    virtual void TearDown() override { delete m_pRadioBlockRule; }

    void CreateRadioBlockRuleWithGivenValue(
            IN CallType eCallType, IN PeerType ePeerTypeIn, IN IMS_BOOL bEmergencyIn)
    {
        m_objCallInfo.ePeerType = ePeerTypeIn;
        m_objCallInfo.bEmergency = bEmergencyIn;

        m_pRadioBlockRule = new RadioBlockRule(m_objContext, eCallType);
    }
};

TEST_F(RadioBlockRuleTest, Check)
{
    CreateRadioBlockRuleWithGivenValue(CallType::VOIP, PeerType::MT, IMS_FALSE);

    EXPECT_CALL(m_objMockIMtcRadioChecker, SetTrafficCheckerListener(_)).Times(4);

    EXPECT_CALL(
            m_objMockIMtcRadioChecker, Check(CallType::VOIP, IMS_FALSE, PeerType::MT, IMS_FALSE))
            .Times(3)
            .WillOnce(Return(CheckResult::UNBLOCKED))
            .WillOnce(Return(CheckResult::PENDING))
            .WillOnce(Return(CheckResult::BLOCKED));

    EXPECT_EQ(
            Result(Result::Status::UNBLOCKED), m_pRadioBlockRule->Check(m_BlockRuleCheckListener));
    EXPECT_EQ(Result(Result::Status::PENDING), m_pRadioBlockRule->Check(m_BlockRuleCheckListener));
    EXPECT_EQ(Result(Result::Status::BLOCKED, CallReasonInfo(CODE_LOCAL_NETWORK_NO_SERVICE)),
            m_pRadioBlockRule->Check(m_BlockRuleCheckListener));
}

TEST_F(RadioBlockRuleTest, IMtcRadioCheckerListener)
{
    // Assume that Check() done.
    CreateRadioBlockRuleWithGivenValue(CallType::VT, PeerType::MO, IMS_TRUE);
    m_pRadioBlockRule->Check(m_BlockRuleCheckListener);

    EXPECT_CALL(m_BlockRuleCheckListener, OnBlockRuleChecked(Result(Result::Status::UNBLOCKED)))
            .Times(1);

    m_pRadioBlockRule->OnConnectionSetupPrepared();

    EXPECT_CALL(m_BlockRuleCheckListener,
            OnBlockRuleChecked(
                    Result(Result::Status::BLOCKED, CallReasonInfo(CODE_LOCAL_NETWORK_NO_SERVICE))))
            .Times(1);

    m_pRadioBlockRule->OnConnectionFailed();
}

}  // namespace android
