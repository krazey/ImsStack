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
#include "call/state/UpdatingState.h"
#include "call/MockIMtcCallContext.h"
#include "call/UpdatingInfo.h"
#include "call/MockMtcSession.h"
#include "configuration/MockIMtcConfigurationManager.h"
#include "configuration/MtcConfigurationProxy.h"
#include "core/MockIMessage.h"
#include "core/MockISession.h"
#include "sipcore/SipMethod.h"

using ::testing::_;
using ::testing::Return;
using ::testing::ReturnRef;

class UpdatingStateTest : public ::testing::Test
{
public:
    MockIMtcCallContext objContext;
    MockIMtcConfigurationManager* pConfigurationManager;
    MtcConfigurationProxy* pConfigurationProxy;
    UpdatingInfo* pUpdatingInfo;
    UpdatingState* pUpdatingState;
    CallInfo objCallInfo;

protected:
    virtual void SetUp() override
    {
        pConfigurationManager = new MockIMtcConfigurationManager();
        pConfigurationProxy = new MtcConfigurationProxy(pConfigurationManager);
        ON_CALL(objContext, GetConfigurationProxy)
                .WillByDefault(ReturnRef(*pConfigurationProxy));

        ON_CALL(objContext, GetCallInfo)
                .WillByDefault(ReturnRef(objCallInfo));

        pUpdatingInfo = new UpdatingInfo();
        ON_CALL(objContext, GetUpdatingInfo).WillByDefault(ReturnRef(*pUpdatingInfo));

        pUpdatingState = new UpdatingState(objContext);
    }

    virtual void TearDown() override
    {
        delete pUpdatingState;
        delete pUpdatingInfo;
    }
};

TEST_F(UpdatingStateTest, OnExitDoesntSendUpdateIfUpdatingInfoDoesntHavePendingUpdate)
{
    MockIMessage objMessage;
    MockISession objSession;
    ON_CALL(objSession, GetNextRequest())
                .WillByDefault(Return(&objMessage));
    MockMtcSession objMtcSession(objContext, objSession, CallType::VOIP);
    ON_CALL(objContext, GetSession())
                .WillByDefault(Return(&objMtcSession));

    EXPECT_CALL(objSession, UpdateEx(_, _))
            .Times(0);

    pUpdatingInfo->SetPendingUpdate(IMS_FALSE);
    pUpdatingState->OnExit();
}

TEST_F(UpdatingStateTest, OnExitSendsUpdateIfUpdatingInfoHasPendingUpdate)
{
    MockIMessage objMessage;
    MockISession objSession;
    ON_CALL(objSession, GetNextRequest())
                .WillByDefault(Return(&objMessage));
    MockMtcSession objMtcSession(objContext, objSession, CallType::VOIP);
    ON_CALL(objContext, GetSession())
                .WillByDefault(Return(&objMtcSession));

    EXPECT_CALL(objSession, UpdateEx(SipMethod::INVALID, IMS_TRUE))
            .Times(1);

    pUpdatingInfo->SetPendingUpdate(IMS_TRUE);
    pUpdatingState->OnExit();
}
