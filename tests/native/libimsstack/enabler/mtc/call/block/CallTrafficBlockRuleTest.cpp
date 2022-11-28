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
#include "call/block/CallTrafficBlockRule.h"
#include "call/block/MockIMtcBlockRule.h"
#include "call/traffic/IMtcCallTrafficChecker.h"
#include "call/traffic/MockIMtcCallTrafficChecker.h"
#include <gmock/gmock.h>

using ::testing::_;
using ::testing::Eq;
using ::testing::Return;
using ::testing::ReturnRef;
using Result = IMtcBlockRule::Result;

namespace android
{

class CallTrafficBlockRuleTest : public ::testing::Test
{
public:
    MockIMtcCallContext m_objContext;
    MockIMtcCallTrafficChecker m_objMockIMtcCallTrafficChecker;
    CallInfo m_objCallInfo;
    MockIMtcService m_objMtcService;
    MockIMtcBlockRuleCheckListener m_BlockRuleCheckListener;
    CallTrafficBlockRule* m_pCallTrafficBlockRule;

protected:
    virtual void SetUp() override
    {
        ON_CALL(m_objContext, GetCallTrafficChecker)
                .WillByDefault(ReturnRef(m_objMockIMtcCallTrafficChecker));
        ON_CALL(m_objContext, GetCallInfo).WillByDefault(ReturnRef(m_objCallInfo));
        ON_CALL(m_objContext, GetService).WillByDefault(ReturnRef(m_objMtcService));

        ON_CALL(m_objMtcService, IsWlanIpCanType).WillByDefault(Return(IMS_FALSE));
    }

    virtual void TearDown() override { delete m_pCallTrafficBlockRule; }

    void CreateCallTrafficBlockRuleWithGivenValue(
            IN CallType eCallType, IN PeerType ePeerTypeIn, IN IMS_BOOL bEmergencyIn)
    {
        m_objCallInfo.ePeerType = ePeerTypeIn;
        m_objCallInfo.bEmergency = bEmergencyIn;

        m_pCallTrafficBlockRule = new CallTrafficBlockRule(m_objContext, eCallType);
    }
};

TEST_F(CallTrafficBlockRuleTest, Check)
{
    CreateCallTrafficBlockRuleWithGivenValue(CallType::VOIP, PeerType::MT, IMS_FALSE);

    EXPECT_CALL(m_objMockIMtcCallTrafficChecker, SetTrafficCheckerListener(_)).Times(4);

    EXPECT_CALL(m_objMockIMtcCallTrafficChecker,
            Check(CallType::VOIP, IMS_FALSE, PeerType::MT, IMS_FALSE))
            .Times(3)
            .WillOnce(Return(CheckResult::UNBLOCKED))
            .WillOnce(Return(CheckResult::PENDING))
            .WillOnce(Return(CheckResult::BLOCKED));

    EXPECT_EQ(Result(Result::Status::UNBLOCKED),
            m_pCallTrafficBlockRule->Check(m_BlockRuleCheckListener));
    EXPECT_EQ(Result(Result::Status::PENDING),
            m_pCallTrafficBlockRule->Check(m_BlockRuleCheckListener));
    EXPECT_EQ(Result(Result::Status::BLOCKED, CallReasonInfo(CODE_LOCAL_NETWORK_NO_SERVICE)),
            m_pCallTrafficBlockRule->Check(m_BlockRuleCheckListener));
}

TEST_F(CallTrafficBlockRuleTest, IMtcCallTrafficCheckerListener)
{
    // Assume that Check() done.
    CreateCallTrafficBlockRuleWithGivenValue(CallType::VT, PeerType::MO, IMS_TRUE);
    m_pCallTrafficBlockRule->Check(m_BlockRuleCheckListener);

    EXPECT_CALL(m_BlockRuleCheckListener,
            OnBlockRuleChecked(Result(static_cast<Result::Status>(CheckResult::UNBLOCKED))))
            .Times(1);

    m_pCallTrafficBlockRule->OnConnectionSetupPrepared();

    EXPECT_CALL(m_BlockRuleCheckListener,
            OnBlockRuleChecked(
                    Result(Result::Status::BLOCKED, CallReasonInfo(CODE_LOCAL_NETWORK_NO_SERVICE))))
            .Times(1);

    m_pCallTrafficBlockRule->OnConnectionFailed();
}

}  // namespace android
