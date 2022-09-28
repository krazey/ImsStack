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

#include <gmock/gmock.h>
#include "call/IMtcCall.h"
#include "ImsTypeDef.h"
#include "call/MockIMtcCallContext.h"
#include "MockIMtcService.h"
#include "call/block/CallTrafficBlockRule.h"
#include "call/traffic/MockIMtcCallTrafficChecker.h"
#include "call/block/MockIMtcBlockRule.h"

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

TEST_F(CallTrafficBlockRuleTest, CheckMt)
{
    CreateCallTrafficBlockRuleWithGivenValue(CallType::VOIP, PeerType::MT, IMS_FALSE);

    EXPECT_CALL(m_objMockIMtcCallTrafficChecker,
            StartTrafficChecking(CallType::VOIP, IMS_FALSE, IMS_FALSE))
            .Times(1);

    EXPECT_EQ(Result(Result::Status::UNBLOCKED),
            m_pCallTrafficBlockRule->Check(m_BlockRuleCheckListener));
}

TEST_F(CallTrafficBlockRuleTest, CheckMoTrafficPrepared)
{
    CreateCallTrafficBlockRuleWithGivenValue(CallType::VOIP, PeerType::MO, IMS_FALSE);

    EXPECT_CALL(m_objMockIMtcCallTrafficChecker, IsTrafficPrepared(_, _))
            .WillRepeatedly(Return(IMS_TRUE));

    EXPECT_EQ(Result(Result::Status::UNBLOCKED),
            m_pCallTrafficBlockRule->Check(m_BlockRuleCheckListener));
}

TEST_F(CallTrafficBlockRuleTest, CheckMoTrafficAllowed)
{
    CreateCallTrafficBlockRuleWithGivenValue(CallType::VOIP, PeerType::MO, IMS_FALSE);

    EXPECT_CALL(m_objMockIMtcCallTrafficChecker, IsTrafficAllowed(_, _))
            .WillRepeatedly(Return(IMS_FALSE));

    EXPECT_EQ(Result(Result::Status::BLOCKED, CODE_LOCAL_NETWORK_NO_SERVICE),
            m_pCallTrafficBlockRule->Check(m_BlockRuleCheckListener));
}

TEST_F(CallTrafficBlockRuleTest, CheckMoStartTrafficChecking)
{
    EXPECT_CALL(m_objMtcService, IsWlanIpCanType).Times(1).WillOnce(Return(IMS_TRUE));

    CreateCallTrafficBlockRuleWithGivenValue(CallType::VT, PeerType::MO, IMS_TRUE);

    EXPECT_CALL(m_objMockIMtcCallTrafficChecker, IsTrafficPrepared(_, _))
            .WillRepeatedly(Return(IMS_FALSE));
    EXPECT_CALL(m_objMockIMtcCallTrafficChecker, IsTrafficAllowed(_, _))
            .WillRepeatedly(Return(IMS_TRUE));

    EXPECT_CALL(m_objMockIMtcCallTrafficChecker, SetTrafficCheckerListener(m_pCallTrafficBlockRule))
            .Times(1);
    EXPECT_CALL(
            m_objMockIMtcCallTrafficChecker, StartTrafficChecking(CallType::VT, IMS_TRUE, IMS_TRUE))
            .Times(1);

    EXPECT_EQ(Result(Result::Status::PENDING),
            m_pCallTrafficBlockRule->Check(m_BlockRuleCheckListener));
}

TEST_F(CallTrafficBlockRuleTest, IMtcCallTrafficCheckerListener)
{
    // Assume that CheckMoStartTrafficChecking passed.
    CreateCallTrafficBlockRuleWithGivenValue(CallType::VT, PeerType::MO, IMS_TRUE);
    EXPECT_CALL(m_objMockIMtcCallTrafficChecker, IsTrafficPrepared(_, _))
            .WillRepeatedly(Return(IMS_FALSE));
    EXPECT_CALL(m_objMockIMtcCallTrafficChecker, IsTrafficAllowed(_, _))
            .WillRepeatedly(Return(IMS_TRUE));
    EXPECT_EQ(Result(Result::Status::PENDING),
            m_pCallTrafficBlockRule->Check(m_BlockRuleCheckListener));

    EXPECT_CALL(m_BlockRuleCheckListener, OnBlockRuleChecked(Result(Result::Status::UNBLOCKED)))
            .Times(1);

    m_pCallTrafficBlockRule->OnConnectionSetupPrepared();

    EXPECT_CALL(m_BlockRuleCheckListener,
            OnBlockRuleChecked(Result(Result::Status::BLOCKED, CODE_LOCAL_NETWORK_NO_SERVICE)))
            .Times(1);

    m_pCallTrafficBlockRule->OnConnectionFailed();
}

}  // namespace android
