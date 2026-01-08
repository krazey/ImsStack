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

#include "MockISession.h"
#include "call/MockIMtcCallContext.h"
#include "call/MockIMtcSession.h"
#include "call/MtcPendingOperationHolder.h"
#include "call/state/IMtcCallState.h"
#include "call/state/MockIMtcCallState.h"
#include <gtest/gtest.h>
#include <functional>

using ::testing::Return;
using ::testing::ReturnRef;

LOCAL IMS_UINT32 nAnyIpcan = 1;

class MtcPendingOperationHolderTest : public ::testing::Test
{
public:
    MockIMtcCallContext objMockContext;
    MockIMtcSession objMockMtcSession;
    MockISession objMockISession;
    MtcPendingOperationHolder* pMtcPendingOperationHolder;
    MockIMtcCallState* pMockIMtcCallState;
    std::function<IMtcCall::State(IMtcCallState*)> objPendingOperation;

protected:
    virtual void SetUp() override
    {
        ON_CALL(objMockContext, GetSession()).WillByDefault(Return(&objMockMtcSession));
        ON_CALL(objMockMtcSession, GetISession).WillByDefault(ReturnRef(objMockISession));

        pMockIMtcCallState = new MockIMtcCallState();

        objPendingOperation = [](IMtcCallState* pState)
        {
            return pState->OnIpcanChanged(nAnyIpcan);
        };

        pMtcPendingOperationHolder = new MtcPendingOperationHolder();
    }

    virtual void TearDown() override
    {
        delete pMtcPendingOperationHolder;
        delete pMockIMtcCallState;
    }
};

TEST_F(MtcPendingOperationHolderTest, HasPendingOperationReturnsFalseIfNoOperation)
{
    EXPECT_FALSE(pMtcPendingOperationHolder->HasPendingOperation());
}

TEST_F(MtcPendingOperationHolderTest, HasPendingOperationReturnsTrueIfHasOperation)
{
    pMtcPendingOperationHolder->PushPendingOperation(objPendingOperation);

    EXPECT_TRUE(pMtcPendingOperationHolder->HasPendingOperation());
}

TEST_F(MtcPendingOperationHolderTest, HasPendingOperationReturnsFalseAfterOperationPopped)
{
    pMtcPendingOperationHolder->PushPendingOperation(objPendingOperation);
    pMtcPendingOperationHolder->PopPendingOperation();

    EXPECT_FALSE(pMtcPendingOperationHolder->HasPendingOperation());
}

TEST_F(MtcPendingOperationHolderTest, PopPendingOperationReturnsOperation)
{
    pMtcPendingOperationHolder->PushPendingOperation(objPendingOperation);
    pMtcPendingOperationHolder->PushPendingOperation(objPendingOperation);

    EXPECT_CALL(*pMockIMtcCallState, OnIpcanChanged(nAnyIpcan)).Times(2);

    pMtcPendingOperationHolder->PopPendingOperation()(pMockIMtcCallState);
    pMtcPendingOperationHolder->PopPendingOperation()(pMockIMtcCallState);
}
