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

#include "JniEnablerConnector.h"
#include "interface/MockIJniEnabler.h"
#include "interface/MockINativeEnabler.h"

namespace android
{

LOCAL IMS_SINT32 SLOT_ID = 0;

class JniEnablerConnectorTest : public ::testing::Test
{
public:
    MockIJniEnabler* pMockJniEnabler;
    MockINativeEnabler* pMockNativeEnabler;

    JniEnablerConnector* pConnector;

protected:
    virtual void SetUp() override
    {
        pMockJniEnabler = new MockIJniEnabler();
        pMockNativeEnabler = new MockINativeEnabler();

        pConnector = &JniEnablerConnector::GetInstance();
    }
    virtual void TearDown() override
    {
        delete pMockJniEnabler;
        delete pMockNativeEnabler;
        delete pConnector;
    }
};

TEST_F(JniEnablerConnectorTest, GetInstanceReturnsSameInstance)
{
    const JniEnablerConnector* pOtherConnector = &JniEnablerConnector::GetInstance();

    EXPECT_EQ(pConnector, pOtherConnector);
}

TEST_F(JniEnablerConnectorTest, SetNGetNativeEnablerWithoutKey)
{
    pConnector->SetNativeEnabler(SLOT_ID, EnablerType::MTC_SERVICE, pMockNativeEnabler);
    EXPECT_EQ(pMockNativeEnabler, pConnector->GetNativeEnabler(SLOT_ID, EnablerType::MTC_SERVICE));
}

TEST_F(JniEnablerConnectorTest, SetNGetJniEnablerWithoutKey)
{
    pConnector->SetJniEnabler(SLOT_ID, EnablerType::MTC_SERVICE, pMockJniEnabler);
    EXPECT_EQ(pMockJniEnabler, pConnector->GetJniEnabler(SLOT_ID, EnablerType::MTC_SERVICE));
}

TEST_F(JniEnablerConnectorTest, SetNGetJniEnablerWithKey)
{
    IMS_ULONG nKey = 1;
    pConnector->SetJniEnabler(SLOT_ID, EnablerType::MTC_SERVICE, pMockJniEnabler, nKey);
    EXPECT_EQ(nullptr, pConnector->GetJniEnabler(SLOT_ID, EnablerType::MTC_SERVICE));
    EXPECT_EQ(pMockJniEnabler, pConnector->GetJniEnabler(SLOT_ID, EnablerType::MTC_SERVICE, nKey));
}

TEST_F(JniEnablerConnectorTest, SetNullJniEnablerRemovesConnection)
{
    IMS_ULONG nKey = 1;
    pConnector->SetJniEnabler(SLOT_ID, EnablerType::MTC_SERVICE, pMockJniEnabler, nKey);
    EXPECT_EQ(pConnector->GetConnectionSize(), 1);
    pConnector->SetJniEnabler(SLOT_ID, EnablerType::MTC_SERVICE, IMS_NULL, nKey);
    EXPECT_EQ(pConnector->GetConnectionSize(), 0);
}

TEST_F(JniEnablerConnectorTest, SetNullNativeEnablerRemovesConnection)
{
    IMS_ULONG nKey = 1;
    pConnector->SetJniEnabler(SLOT_ID, EnablerType::MTC_SERVICE, pMockJniEnabler, nKey);
    pConnector->SetNativeEnabler(SLOT_ID, EnablerType::MTC_SERVICE, pMockNativeEnabler);
    EXPECT_EQ(pConnector->GetConnectionSize(), 1);
    pConnector->SetJniEnabler(SLOT_ID, EnablerType::MTC_SERVICE, IMS_NULL, nKey);
    EXPECT_EQ(pConnector->GetConnectionSize(), 1);
    pConnector->SetNativeEnabler(SLOT_ID, EnablerType::MTC_SERVICE, IMS_NULL);
    EXPECT_EQ(pConnector->GetConnectionSize(), 0);
}

TEST_F(JniEnablerConnectorTest, SetNullJniEnablerNotRemovesConnectionIfNativeEnablerExists)
{
    IMS_ULONG nKey = 1;
    pConnector->SetNativeEnabler(SLOT_ID, EnablerType::MTC_SERVICE, pMockNativeEnabler);
    pConnector->SetJniEnabler(SLOT_ID, EnablerType::MTC_SERVICE, pMockJniEnabler, nKey);
    pConnector->SetJniEnabler(SLOT_ID, EnablerType::MTC_SERVICE, IMS_NULL, nKey);
    EXPECT_EQ(pConnector->GetConnectionSize(), 1);
}

TEST_F(JniEnablerConnectorTest, SetJniEnablerInvokesNotifyJniEnablerSet)
{
    EXPECT_CALL(*pMockNativeEnabler, NotifyJniEnablerSet).Times(1);

    pConnector->SetNativeEnabler(SLOT_ID, EnablerType::MTC_SERVICE, pMockNativeEnabler);
    pConnector->SetJniEnabler(SLOT_ID, EnablerType::MTC_SERVICE, pMockJniEnabler);
}

TEST_F(JniEnablerConnectorTest, SetNativeEnablerInvokesNotifyNativeEnablerSet)
{
    EXPECT_CALL(*pMockJniEnabler, NotifyNativeEnablerSet).Times(1);

    pConnector->SetJniEnabler(SLOT_ID, EnablerType::MTC_SERVICE, pMockJniEnabler);
    pConnector->SetNativeEnabler(SLOT_ID, EnablerType::MTC_SERVICE, pMockNativeEnabler);
}

TEST_F(JniEnablerConnectorTest, CreateOnlyOneConnectionForSameType)
{
    pConnector->SetJniEnabler(SLOT_ID, EnablerType::MTC_SERVICE, pMockJniEnabler, 0);
    pConnector->SetJniEnabler(SLOT_ID, EnablerType::MTC_SERVICE, pMockJniEnabler, 1);
    pConnector->SetJniEnabler(SLOT_ID, EnablerType::MTC_SERVICE, pMockJniEnabler, 2);
    pConnector->SetNativeEnabler(SLOT_ID, EnablerType::MTC_SERVICE, pMockNativeEnabler);
    EXPECT_EQ(pConnector->GetConnectionSize(), 1);
}

TEST_F(JniEnablerConnectorTest, AllJniEnablerIsNotifiedWhenNativeEnablerSet)
{
    EXPECT_CALL(*pMockJniEnabler, NotifyNativeEnablerSet).Times(3);

    pConnector->SetJniEnabler(SLOT_ID, EnablerType::MTC_SERVICE, pMockJniEnabler, 0);
    pConnector->SetJniEnabler(SLOT_ID, EnablerType::MTC_SERVICE, pMockJniEnabler, 1);
    pConnector->SetJniEnabler(SLOT_ID, EnablerType::MTC_SERVICE, pMockJniEnabler, 2);
    pConnector->SetNativeEnabler(SLOT_ID, EnablerType::MTC_SERVICE, pMockNativeEnabler);
}

}  // namespace android
