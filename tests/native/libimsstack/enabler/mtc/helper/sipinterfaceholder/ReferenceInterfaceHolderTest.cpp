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

#include "MockICoreService.h"
#include "MockIMessage.h"
#include "MockIReference.h"
#include "MockISession.h"
#include "MockISipServerConnection.h"
#include "MockITimer.h"
#include "PlatformContext.h"
#include "TestTimerService.h"
#include "helper/sipinterfaceholder/MockIInterfaceHolderListener.h"
#include "helper/sipinterfaceholder/ReferenceInterfaceHolder.h"
#include <gtest/gtest.h>

using ::testing::_;
using ::testing::Return;

namespace android
{

class ReferenceInterfaceHolderTest : public ::testing::Test
{
public:
    ReferenceInterfaceHolder* pHolder;
    MockIInterfaceHolderListener objListener;
    MockIReference objMockIReference;
    MockISession objMockISession;
    TestTimerService objTimerService;
    MockITimer objMockITimer;

protected:
    virtual void SetUp() override
    {
        objTimerService.SetTimer(&objMockITimer);
        PlatformContext::GetInstance()->SetService(
                PlatformContext::SERVICE_TIMER, &objTimerService);
        ON_CALL(objMockISession, CreateReference(_, _)).WillByDefault(Return(&objMockIReference));
        pHolder = new ReferenceInterfaceHolder(objListener);
    }

    virtual void TearDown() override
    {
        delete pHolder;
        PlatformContext::GetInstance()->SetService(PlatformContext::SERVICE_TIMER, IMS_NULL);
    }
};

TEST_F(ReferenceInterfaceHolderTest, HolderDoesNothingForReferenceListenerExceptTerminated)
{
    MockIMessage objIMessage;

    pHolder->ReferenceDelivered(&objMockIReference);
    pHolder->ReferenceDeliveryFailed(&objMockIReference);
    pHolder->ReferenceNotify(&objMockIReference, &objIMessage);

    EXPECT_FALSE(pHolder->IsTimerExist(&objMockIReference));
}

TEST_F(ReferenceInterfaceHolderTest, StopsTimerIfReferenceTerminated)
{
    EXPECT_CALL(objMockIReference, Destroy()).Times(1);

    pHolder->GetIReference(&objMockISession, "sip:referToUri", "referMethod");
    pHolder->ReferenceTerminated(&objMockIReference);

    EXPECT_FALSE(pHolder->IsTimerExist(&objMockIReference));
    EXPECT_EQ(pHolder->GetReferenceCount(), 0);
}

TEST_F(ReferenceInterfaceHolderTest, AddAndReleaseStartsTimer)
{
    ON_CALL(objMockIReference, GetState).WillByDefault(Return(IReference::STATE_REFERRING));

    EXPECT_EQ(pHolder->GetReferenceCount(), 0);

    pHolder->GetIReference(&objMockISession, "sip:referToUri", "referMethod");
    EXPECT_FALSE(pHolder->IsTimerExist(&objMockIReference));
    EXPECT_EQ(pHolder->GetReferenceCount(), 1);

    pHolder->ReleaseIReference(&objMockIReference, IMS_FALSE);
    EXPECT_TRUE(pHolder->IsTimerExist(&objMockIReference));
}

TEST_F(ReferenceInterfaceHolderTest, AddAndReleaseWithTimerExpiredStopsTimer)
{
    ON_CALL(objMockIReference, GetState).WillByDefault(Return(IReference::STATE_REFERRING));

    EXPECT_EQ(pHolder->GetReferenceCount(), 0);

    pHolder->GetIReference(&objMockISession, "sip:referToUri", "referMethod");
    EXPECT_FALSE(pHolder->IsTimerExist(&objMockIReference));
    EXPECT_EQ(pHolder->GetReferenceCount(), 1);

    pHolder->ReleaseIReference(&objMockIReference, IMS_FALSE);
    EXPECT_TRUE(pHolder->IsTimerExist(&objMockIReference));

    pHolder->Timer_TimerExpired(&objMockITimer);

    EXPECT_FALSE(pHolder->IsTimerExist(&objMockIReference));
    EXPECT_EQ(pHolder->GetReferenceCount(), 0);
}

TEST_F(ReferenceInterfaceHolderTest, AddAndReleaseWithTerminatedStopsTimer)
{
    ON_CALL(objMockIReference, GetState).WillByDefault(Return(IReference::STATE_REFERRING));
    EXPECT_CALL(objMockIReference, Destroy()).Times(1);

    EXPECT_EQ(pHolder->GetReferenceCount(), 0);

    pHolder->GetIReference(&objMockISession, "sip:referToUri", "referMethod");
    EXPECT_FALSE(pHolder->IsTimerExist(&objMockIReference));
    EXPECT_EQ(pHolder->GetReferenceCount(), 1);

    pHolder->ReleaseIReference(&objMockIReference, IMS_FALSE);
    EXPECT_TRUE(pHolder->IsTimerExist(&objMockIReference));

    pHolder->ReleaseIReference(&objMockIReference, IMS_TRUE);
    EXPECT_FALSE(pHolder->IsTimerExist(&objMockIReference));
    EXPECT_EQ(pHolder->GetReferenceCount(), 0);
}

}  // namespace android
