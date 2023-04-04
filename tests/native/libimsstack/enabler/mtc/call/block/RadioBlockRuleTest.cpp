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
#include "call/MockEpsFallbackTrigger.h"
#include "call/MockIMtcCallContext.h"
#include "call/block/RadioBlockRule.h"
#include "call/block/MockIMtcBlockRule.h"
#include "call/radio/IMtcRadioChecker.h"
#include "call/radio/MockIMtcRadioChecker.h"
#include "configuration/MockIMtcConfigurationManager.h"
#include "configuration/MtcConfigurationProxy.h"
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
    MockEpsFallbackTrigger* pEpsFbTrigger;
    MockIMtcBlockRuleCheckListener BlockRuleCheckListener;
    MockIMtcConfigurationManager* pConfigurationManager;
    MtcConfigurationProxy* pConfigurationProxy;

    RadioBlockRule* pRadioBlockRule;

protected:
    virtual void SetUp() override
    {
        pEpsFbTrigger = new MockEpsFallbackTrigger(objContext);
        pConfigurationManager = new MockIMtcConfigurationManager();
        pConfigurationProxy = new MtcConfigurationProxy(pConfigurationManager);

        ON_CALL(objContext, GetRadioChecker).WillByDefault(ReturnRef(objMockIMtcRadioChecker));
        ON_CALL(objContext, GetCallInfo).WillByDefault(ReturnRef(objCallInfo));
        ON_CALL(objContext, GetService).WillByDefault(ReturnRef(objMtcService));
        ON_CALL(objContext, GetCallKey).WillByDefault(Return(CALL_KEY));
        ON_CALL(objContext, GetEpsFallbackTrigger).WillByDefault(ReturnRef(*pEpsFbTrigger));
        ON_CALL(objContext, GetConfigurationProxy).WillByDefault(ReturnRef(*pConfigurationProxy));

        ON_CALL(objMtcService, IsWlanIpCanType).WillByDefault(Return(IMS_FALSE));
    }

    virtual void TearDown() override
    {
        delete pRadioBlockRule;
        delete pConfigurationProxy;
    }

    void CreateRadioBlockRuleWithGivenValue(
            IN CallType eCallType, IN PeerType ePeerTypeIn, IN IMS_BOOL bEmergencyIn)
    {
        objCallInfo.ePeerType = ePeerTypeIn;
        objCallInfo.bEmergency = bEmergencyIn;

        pRadioBlockRule = new RadioBlockRule(objContext, eCallType);
    }

    void SetEpsFallbackRequiredByConfig(IN IMS_BOOL bRequired)
    {
        ON_CALL(*pConfigurationManager, GetEpsFallbackWatchdogTime)
                .WillByDefault(Return(bRequired ? 1 : 0));
    }
};

TEST_F(RadioBlockRuleTest, Check)
{
    CreateRadioBlockRuleWithGivenValue(CallType::VOIP, PeerType::MT, IMS_FALSE);

    EXPECT_CALL(objMockIMtcRadioChecker, SetTrafficCheckerListener(_)).Times(4);
    EXPECT_CALL(objMockIMtcRadioChecker,
            Check(CallType::VOIP, IMS_FALSE, PeerType::MT, IMS_FALSE, CALL_KEY))
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
    CreateRadioBlockRuleWithGivenValue(CallType::VOIP, PeerType::MT, IMS_FALSE);

    ON_CALL(objMockIMtcRadioChecker, Check(_, _, _, _, _))
            .WillByDefault(Return(CheckResult::PENDING));
    pRadioBlockRule->Check(BlockRuleCheckListener);

    EXPECT_CALL(BlockRuleCheckListener, OnBlockRuleChecked(Result(Result::Status::UNBLOCKED)))
            .Times(1);
    pRadioBlockRule->OnConnectionSetupPrepared();
}

TEST_F(RadioBlockRuleTest, OnConnectionFailedNotifiesNoServiceWhenNotInVoNr)
{
    CreateRadioBlockRuleWithGivenValue(CallType::VT, PeerType::MO, IMS_TRUE);

    ON_CALL(objMockIMtcRadioChecker, Check(_, _, _, _, _))
            .WillByDefault(Return(CheckResult::PENDING));
    pRadioBlockRule->Check(BlockRuleCheckListener);

    const IMS_UINT32 nTimerVzw = 2;
    const IMS_UINT32 nRejectWaitTimeMillis = 2;
    const IMS_UINT32 nFailureReason = IImsRadio::REASON_RRC_REJECT;
    ON_CALL(*pConfigurationManager, GetMoCallRequestTimeout).WillByDefault(Return(nTimerVzw));
    ON_CALL(*pEpsFbTrigger, IsVoNr).WillByDefault(Return(IMS_FALSE));
    SetEpsFallbackRequiredByConfig(IMS_TRUE);

    EXPECT_CALL(BlockRuleCheckListener,
            OnBlockRuleChecked(
                    Result(Result::Status::BLOCKED, CallReasonInfo(CODE_LOCAL_NETWORK_NO_SERVICE))))
            .Times(1);
    pRadioBlockRule->OnConnectionFailed(nFailureReason, nRejectWaitTimeMillis);
}

TEST_F(RadioBlockRuleTest, OnConnectionFailedNotifiesNoServiceWhenNotRrcReject)
{
    CreateRadioBlockRuleWithGivenValue(CallType::VT, PeerType::MO, IMS_TRUE);

    ON_CALL(objMockIMtcRadioChecker, Check(_, _, _, _, _))
            .WillByDefault(Return(CheckResult::PENDING));
    pRadioBlockRule->Check(BlockRuleCheckListener);

    const IMS_UINT32 nTimerVzw = 2;
    const IMS_UINT32 nRejectWaitTimeMillis = 2;
    const IMS_UINT32 nFailureReason = IImsRadio::REASON_ACCESS_DENIED;
    ON_CALL(*pConfigurationManager, GetMoCallRequestTimeout).WillByDefault(Return(nTimerVzw));
    ON_CALL(*pEpsFbTrigger, IsVoNr).WillByDefault(Return(IMS_TRUE));
    SetEpsFallbackRequiredByConfig(IMS_TRUE);

    EXPECT_CALL(BlockRuleCheckListener,
            OnBlockRuleChecked(
                    Result(Result::Status::BLOCKED, CallReasonInfo(CODE_LOCAL_NETWORK_NO_SERVICE))))
            .Times(1);
    pRadioBlockRule->OnConnectionFailed(nFailureReason, nRejectWaitTimeMillis);
}

TEST_F(RadioBlockRuleTest, OnConnectionFailedNotifiesNoServiceWhenRejectTimeIsLessThanTimerVzw)
{
    CreateRadioBlockRuleWithGivenValue(CallType::VT, PeerType::MO, IMS_TRUE);

    ON_CALL(objMockIMtcRadioChecker, Check(_, _, _, _, _))
            .WillByDefault(Return(CheckResult::PENDING));
    pRadioBlockRule->Check(BlockRuleCheckListener);

    const IMS_UINT32 nTimerVzw = 2;
    const IMS_UINT32 nRejectWaitTimeMillis = 1;
    const IMS_UINT32 nFailureReason = IImsRadio::REASON_RRC_REJECT;
    ON_CALL(*pConfigurationManager, GetMoCallRequestTimeout).WillByDefault(Return(nTimerVzw));
    ON_CALL(*pEpsFbTrigger, IsVoNr).WillByDefault(Return(IMS_TRUE));
    SetEpsFallbackRequiredByConfig(IMS_TRUE);

    EXPECT_CALL(BlockRuleCheckListener,
            OnBlockRuleChecked(
                    Result(Result::Status::BLOCKED, CallReasonInfo(CODE_LOCAL_NETWORK_NO_SERVICE))))
            .Times(1);
    pRadioBlockRule->OnConnectionFailed(nFailureReason, nRejectWaitTimeMillis);
}

TEST_F(RadioBlockRuleTest, OnConnectionFailedNotifiesEpsFallbackWhenConditionsAreMet)
{
    CreateRadioBlockRuleWithGivenValue(CallType::VT, PeerType::MO, IMS_TRUE);

    ON_CALL(objMockIMtcRadioChecker, Check(_, _, _, _, _))
            .WillByDefault(Return(CheckResult::PENDING));
    pRadioBlockRule->Check(BlockRuleCheckListener);

    const IMS_UINT32 nTimerVzw = 2;
    const IMS_UINT32 nRejectWaitTimeMillis = 2;
    const IMS_UINT32 nFailureReason = IImsRadio::REASON_RRC_REJECT;
    ON_CALL(*pConfigurationManager, GetMoCallRequestTimeout).WillByDefault(Return(nTimerVzw));
    ON_CALL(*pEpsFbTrigger, IsVoNr).WillByDefault(Return(IMS_TRUE));
    SetEpsFallbackRequiredByConfig(IMS_TRUE);

    EXPECT_CALL(BlockRuleCheckListener,
            OnBlockRuleChecked(Result(Result::Status::BLOCKED,
                    CallReasonInfo(CODE_INTERNAL_REDIAL, EXTRA_CODE_REDIAL_AFTER_EPS_FALLBACK))))
            .Times(1);
    pRadioBlockRule->OnConnectionFailed(nFailureReason, nRejectWaitTimeMillis);
}

}  // namespace android
