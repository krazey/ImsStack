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

#include "MockIOsFactory.h"
#include "PlatformContext.h"
#include "core/MockICoreService.h"
#include "core/MockIMessage.h"
#include "core/MockIReference.h"
#include "core/MockISession.h"
#include "helper/sipinterfaceholder/MockIInterfaceHolderListener.h"
#include "helper/sipinterfaceholder/ReferenceInterfaceHolder.h"
#include "sipcore/MockISipServerConnection.h"
#include <gtest/gtest.h>

using ::testing::_;
using ::testing::Return;

namespace android
{

class TestImsTimerForRiHolder : public ImsTimer
{
public:
    inline TestImsTimerForRiHolder() {}
    inline ~TestImsTimerForRiHolder() {}

public:
    IMS_BOOL Equals(IN const ITimer* piTimer) const override
    {
        ImsTimer* pTimer = DYNAMIC_CAST(ImsTimer*, piTimer);
        return pTimer->GetTimerId() == reinterpret_cast<IMS_UINTP>(this);
    }

    IMS_UINTP SetTimer(IN IMS_UINT32 /*nDuration*/, IN ITimerListener* /*piListener*/) override
    {
        return reinterpret_cast<IMS_UINTP>(this);
    }

    void KillTimer() override {}

    IMS_UINTP GetTimerId() const override { return reinterpret_cast<IMS_UINTP>(this); }

    void DispatchServiceMessage(IN IMS_UINTP /*nWparam*/, IN IMS_UINTP /*nLparam*/) override {}
};

class ReferenceInterfaceHolderTest : public ::testing::Test
{
public:
    MockIOsFactory objMockIOsFactory;
    IOsFactory* piOldOsFactory;
    TestImsTimerForRiHolder* pTestImsTimerForRiHolder;
    ReferenceInterfaceHolder* pHolder;
    MockIInterfaceHolderListener objListener;
    MockIReference objMockIReference;
    MockISession objMockISession;

protected:
    virtual void SetUp() override
    {
        ON_CALL(objMockISession, CreateReference(_, _)).WillByDefault(Return(&objMockIReference));
        pHolder = new ReferenceInterfaceHolder(objListener);
    }

    virtual void TearDown() override { delete pHolder; }
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
    piOldOsFactory = PlatformContext::GetInstance()->SetOsFactory(&objMockIOsFactory);

    // will be deleted in the TimerService::DestroyTimer.
    pTestImsTimerForRiHolder = new TestImsTimerForRiHolder();
    EXPECT_CALL(objMockIOsFactory, CreateTimer())
            .Times(1)
            .WillOnce(Return(pTestImsTimerForRiHolder));

    ON_CALL(objMockIReference, GetState).WillByDefault(Return(IReference::STATE_REFERRING));

    EXPECT_EQ(pHolder->GetReferenceCount(), 0);

    pHolder->GetIReference(&objMockISession, "sip:referToUri", "referMethod");
    EXPECT_FALSE(pHolder->IsTimerExist(&objMockIReference));
    EXPECT_EQ(pHolder->GetReferenceCount(), 1);

    pHolder->ReleaseIReference(&objMockIReference, IMS_FALSE);
    EXPECT_TRUE(pHolder->IsTimerExist(&objMockIReference));

    pHolder->Timer_TimerExpired(pTestImsTimerForRiHolder);

    EXPECT_FALSE(pHolder->IsTimerExist(&objMockIReference));
    EXPECT_EQ(pHolder->GetReferenceCount(), 0);

    PlatformContext::GetInstance()->SetOsFactory(piOldOsFactory);
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
