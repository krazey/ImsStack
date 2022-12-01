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

#include "MockIMtcImsEventReceiver.h"
#include "MockIMtcService.h"
#include "call/MockIMtcCallContext.h"
#include "call/MockIMtcCallManager.h"
#include "call/ParticipantInfo.h"
#include "call/block/MockIMtcBlockChecker.h"
#include "call/radio/MockIMtcRadioChecker.h"
#include "call/state/IdleState.h"
#include "configuration/MockIMtcConfigurationManager.h"
#include "configuration/MtcConfigurationProxy.h"
#include "dialingplan/MockIMtcDialingPlan.h"
#include "helper/MtcSupplementaryService.h"
#include "media/MockIMtcMediaManager.h"
#include <gtest/gtest.h>

using ::testing::_;
using ::testing::Return;
using ::testing::ReturnRef;

class IdleStateTest : public ::testing::Test
{
public:
    IdleState* pIdleState;
    MockIMtcConfigurationManager* pConfigurationManager;
    MtcConfigurationProxy* pConfigurationProxy;
    MtcSupplementaryService* pSupplementaryService;
    ParticipantInfo* pParticipantInfo;
    MockIMtcCallContext objCallContext;
    MockIMtcDialingPlan objDialingPlan;
    MockIMtcMediaManager objMediaManager;
    MockIMtcService objService;
    MockIMtcImsEventReceiver objImsEventReceiver;
    MockIMtcCallManager objCallManager;
    CallInfo objCallInfo;
    MockIMtcRadioChecker objMockIMtcRadioChecker;

protected:
    virtual void SetUp() override
    {
        pConfigurationManager = new MockIMtcConfigurationManager();
        pConfigurationProxy = new MtcConfigurationProxy(pConfigurationManager);
        ON_CALL(objCallContext, GetConfigurationProxy)
                .WillByDefault(ReturnRef(*pConfigurationProxy));

        pSupplementaryService = new MtcSupplementaryService(*pConfigurationProxy);
        ON_CALL(objCallContext, GetSupplementaryService)
                .WillByDefault(ReturnRef(*pSupplementaryService));

        pParticipantInfo = new ParticipantInfo(objCallContext);
        ON_CALL(objCallContext, GetParticipantInfo).WillByDefault(ReturnRef(*pParticipantInfo));

        ON_CALL(objCallContext, GetDialingPlan).WillByDefault(ReturnRef(objDialingPlan));
        ON_CALL(objCallContext, GetCallInfo).WillByDefault(ReturnRef(objCallInfo));
        ON_CALL(objCallContext, GetService).WillByDefault(ReturnRef(objService));
        ON_CALL(objCallContext, GetMediaManager).WillByDefault(ReturnRef(objMediaManager));
        ON_CALL(objCallContext, GetImsEventReceiver).WillByDefault(ReturnRef(objImsEventReceiver));
        ON_CALL(objCallContext, GetCallManager).WillByDefault(ReturnRef(objCallManager));
        ON_CALL(objCallContext, GetRadioChecker).WillByDefault(ReturnRef(objMockIMtcRadioChecker));

        pIdleState = new IdleState(objCallContext);
    }

    virtual void TearDown() override
    {
        delete pIdleState;
        delete pConfigurationProxy;
        delete pSupplementaryService;
        delete pParticipantInfo;
    }
};

TEST_F(IdleStateTest, StartSetsUpCallInfo)
{
    CallType eCallType = CallType::VOIP;
    AString strTarget("some_target");
    MediaInfo objMediaInfo;
    MockIMtcBlockChecker* pBlockChecker = new MockIMtcBlockChecker();
    ImsMap<SuppType, SuppService*> objSuppServices;
    IMS_BOOL bUssi = IMS_FALSE;

    ON_CALL(objCallContext, IsUssi).WillByDefault(Return(bUssi));
    ON_CALL(objCallContext, CreateBlockChecker).WillByDefault(Return(pBlockChecker));
    ON_CALL(*pBlockChecker, Check)
            .WillByDefault(
                    Return(IMtcBlockChecker::Result(IMtcBlockChecker::Result::Status::PENDING)));
    ON_CALL(objDialingPlan, GetToUri(_, _, _)).WillByDefault(Return(strTarget));

    pIdleState->Start(eCallType, strTarget, objMediaInfo, objSuppServices);

    EXPECT_EQ(eCallType, objCallInfo.eInitialCallType);
    EXPECT_EQ(PeerType::MO, objCallInfo.ePeerType);
    EXPECT_EQ(bUssi, objCallInfo.bUssi);
    EXPECT_EQ(IMS_FALSE, objCallInfo.bConference);
}

TEST_F(IdleStateTest, StartSetsUpParticipantInfo)
{
    CallType eCallType = CallType::VOIP;
    AString strTarget("some_target");
    MediaInfo objMediaInfo;
    MockIMtcBlockChecker* pBlockChecker = new MockIMtcBlockChecker();
    ImsMap<SuppType, SuppService*> objSuppServices;

    ON_CALL(objCallContext, IsUssi).WillByDefault(Return(IMS_FALSE));
    ON_CALL(objCallContext, CreateBlockChecker).WillByDefault(Return(pBlockChecker));
    ON_CALL(*pBlockChecker, Check)
            .WillByDefault(
                    Return(IMtcBlockChecker::Result(IMtcBlockChecker::Result::Status::PENDING)));
    ON_CALL(objDialingPlan, GetToUri(_, _, _)).WillByDefault(Return(strTarget));

    pIdleState->Start(eCallType, strTarget, objMediaInfo, objSuppServices);

    EXPECT_EQ(strTarget, pParticipantInfo->GetRemoteNumber());
}

TEST_F(IdleStateTest, StartSetsUpSupplementaryService)
{
    CallType eCallType = CallType::VOIP;
    AString strTarget("some_target");
    MediaInfo objMediaInfo;
    MockIMtcBlockChecker* pBlockChecker = new MockIMtcBlockChecker();

    SuppType eSuppType = SuppType::GEOLOCATION;
    ImsMap<SuppType, SuppService*> objSuppServices;
    objSuppServices.Add(eSuppType, new SuppService());

    ON_CALL(objCallContext, IsUssi).WillByDefault(Return(IMS_FALSE));
    ON_CALL(objCallContext, CreateBlockChecker).WillByDefault(Return(pBlockChecker));
    ON_CALL(*pBlockChecker, Check)
            .WillByDefault(
                    Return(IMtcBlockChecker::Result(IMtcBlockChecker::Result::Status::PENDING)));
    ON_CALL(objDialingPlan, GetToUri(_, _, _)).WillByDefault(Return(strTarget));

    pIdleState->Start(eCallType, strTarget, objMediaInfo, objSuppServices);

    EXPECT_NE(nullptr, pSupplementaryService->Get(eSuppType));
}

TEST_F(IdleStateTest, StartSetsUpMediaManager)
{
    CallType eCallType = CallType::VOIP;
    AString strTarget("some_target");
    MediaInfo objMediaInfo;
    MockIMtcBlockChecker* pBlockChecker = new MockIMtcBlockChecker();
    ImsMap<SuppType, SuppService*> objSuppServices;

    ON_CALL(objCallContext, IsUssi).WillByDefault(Return(IMS_FALSE));
    ON_CALL(objCallContext, CreateBlockChecker).WillByDefault(Return(pBlockChecker));
    ON_CALL(*pBlockChecker, Check)
            .WillByDefault(
                    Return(IMtcBlockChecker::Result(IMtcBlockChecker::Result::Status::PENDING)));
    ON_CALL(objDialingPlan, GetToUri(_, _, _)).WillByDefault(Return(strTarget));

    EXPECT_CALL(objMediaManager, SetMediaInfo(objMediaInfo)).Times(1);

    pIdleState->Start(eCallType, strTarget, objMediaInfo, objSuppServices);
}
