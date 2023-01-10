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

#include "ImsList.h"
#include "call/block/MockIMtcBlockChecker.h"
#include "call/block/MockIMtcBlockRule.h"
#include "call/block/MtcBlockChecker.h"
#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <utility>

using ::testing::_;
using ::testing::Return;
using Result = IMtcBlockChecker::Result;

const CallReasonInfo objDefaultReason(CODE_NONE);
const CallReasonInfo objReason(CODE_LOCAL_SERVICE_UNAVAILABLE);

MockIMtcBlockRule* CreateMockIMtcBlockRule(Result objResult)
{
    MockIMtcBlockRule* pRule = new MockIMtcBlockRule();

    ON_CALL(*pRule, Check).WillByDefault(Return(std::move(objResult)));

    return pRule;
}

TEST(MtcBlockCheckerTest, CheckReturnsUnblockedForEmptyRule)
{
    ImsList<IMtcBlockRule*> lstRules;

    MtcBlockChecker objChecker(lstRules, nullptr);
    Result objResult = objChecker.Check();

    EXPECT_EQ(Result::Status::UNBLOCKED, objResult.eStatus);
}

TEST(MtcBlockCheckerTest, CheckReturnsUnblockedIfAllRulesUnblocked)
{
    ImsList<IMtcBlockRule*> lstRules;
    lstRules.Append(CreateMockIMtcBlockRule(Result(Result::Status::UNBLOCKED)));
    lstRules.Append(CreateMockIMtcBlockRule(Result(Result::Status::UNBLOCKED)));

    MtcBlockChecker objChecker(lstRules, nullptr);
    Result objResult = objChecker.Check();

    EXPECT_EQ(Result::Status::UNBLOCKED, objResult.eStatus);
}

TEST(MtcBlockCheckerTest, CheckReturnsBlockedIfSomeRulesBlocked)
{
    ImsList<IMtcBlockRule*> lstRules;
    lstRules.Append(CreateMockIMtcBlockRule(Result(Result::Status::UNBLOCKED)));
    lstRules.Append(CreateMockIMtcBlockRule(Result(Result::Status::PENDING)));
    lstRules.Append(CreateMockIMtcBlockRule(Result(Result::Status::BLOCKED, objReason)));

    MtcBlockChecker objChecker(lstRules, nullptr);
    Result objResult = objChecker.Check();

    EXPECT_EQ(Result::Status::BLOCKED, objResult.eStatus);
}

TEST(MtcBlockCheckerTest, CheckReturnsPendingIfSomeRulesPending)
{
    ImsList<IMtcBlockRule*> lstRules;
    lstRules.Append(CreateMockIMtcBlockRule(Result(Result::Status::UNBLOCKED)));
    lstRules.Append(CreateMockIMtcBlockRule(Result(Result::Status::PENDING)));

    MtcBlockChecker objChecker(lstRules, nullptr);
    Result objResult = objChecker.Check();

    EXPECT_EQ(Result::Status::PENDING, objResult.eStatus);
}

TEST(MtcBlockCheckerTest, ListenerNotifiedWhenAllPendingRulesUnblocked)
{
    ImsList<IMtcBlockRule*> lstRules;
    lstRules.Append(CreateMockIMtcBlockRule(Result(Result::Status::PENDING)));
    lstRules.Append(CreateMockIMtcBlockRule(Result(Result::Status::PENDING)));

    MockIMtcBlockCheckListener objListener;
    EXPECT_CALL(objListener, OnBlockChecked(Result(Result::Status::UNBLOCKED, objDefaultReason)))
            .Times(1);

    MtcBlockChecker objChecker(lstRules, &objListener);
    objChecker.Check();

    objChecker.OnBlockRuleChecked(Result(Result::Status::UNBLOCKED, objDefaultReason));
    objChecker.OnBlockRuleChecked(Result(Result::Status::UNBLOCKED, objDefaultReason));
}

TEST(MtcBlockCheckerTest, ListenerNotNotifiedWhenSomePendingRulesUnblocked)
{
    ImsList<IMtcBlockRule*> lstRules;
    lstRules.Append(CreateMockIMtcBlockRule(Result(Result::Status::PENDING)));
    lstRules.Append(CreateMockIMtcBlockRule(Result(Result::Status::PENDING)));

    MockIMtcBlockCheckListener objListener;
    EXPECT_CALL(objListener, OnBlockChecked(_)).Times(0);

    MtcBlockChecker objChecker(lstRules, &objListener);
    objChecker.Check();
    objChecker.OnBlockRuleChecked(Result(Result::Status::UNBLOCKED, objDefaultReason));
}

TEST(MtcBlockCheckerTest, ListenerNotifiedWhenSomePendingRulesBlocked)
{
    ImsList<IMtcBlockRule*> lstRules;
    lstRules.Append(CreateMockIMtcBlockRule(Result(Result::Status::PENDING)));
    lstRules.Append(CreateMockIMtcBlockRule(Result(Result::Status::PENDING)));

    MockIMtcBlockCheckListener objListener;
    EXPECT_CALL(objListener, OnBlockChecked(Result(Result::Status::BLOCKED, objReason))).Times(1);

    MtcBlockChecker objChecker(lstRules, &objListener);
    objChecker.Check();
    objChecker.OnBlockRuleChecked(Result(Result::Status::BLOCKED, objReason));
}

TEST(MtcBlockCheckerTest, ListenerNotNotifiedAfterNotifiedOnce)
{
    ImsList<IMtcBlockRule*> lstRules;
    lstRules.Append(CreateMockIMtcBlockRule(Result(Result::Status::PENDING)));
    lstRules.Append(CreateMockIMtcBlockRule(Result(Result::Status::PENDING)));

    MockIMtcBlockCheckListener objListener;
    EXPECT_CALL(objListener, OnBlockChecked(_)).Times(0);
    EXPECT_CALL(objListener, OnBlockChecked(Result(Result::Status::BLOCKED, objReason))).Times(1);

    MtcBlockChecker objChecker(lstRules, &objListener);
    objChecker.Check();
    objChecker.OnBlockRuleChecked(Result(Result::Status::BLOCKED, objReason));
    objChecker.OnBlockRuleChecked(Result(Result::Status::UNBLOCKED, objDefaultReason));
}
