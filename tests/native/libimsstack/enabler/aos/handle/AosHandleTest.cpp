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
#include <gmock/gmock.h>

#include "ImsAosParameter.h"

#include "handle/AosHandle.h"
#include "interface/IAosAppContext.h"
#include "interface/MockIAosAppContext.h"

using ::testing::AnyNumber;
using ::testing::Return;
using ::testing::ReturnRef;

class AosHandleTest : public ::testing::Test
{
public:
    AosHandle* m_pAosHandle;
    MockIAosAppContext objMockIAosAppContext;

protected:
    virtual void SetUp() override
    {
        EXPECT_CALL(objMockIAosAppContext, GetSlotId())
                .Times(AnyNumber())
                .WillRepeatedly(Return(0));

        const AString strValue = AString("test");
        EXPECT_CALL(objMockIAosAppContext, GetProfileId())
                .Times(AnyNumber())
                .WillRepeatedly(ReturnRef(strValue));

        const AString strAppId = AString("ims.app.mtc");
        const AString strServiceId = AString("ims.service.mtc");
        const IMS_UINT32 nServiceType = ImsAosService::MTC;

        m_pAosHandle = new AosHandle(static_cast<IAosAppContext*>(&objMockIAosAppContext), strAppId,
                strServiceId, nServiceType);

        ASSERT_TRUE(m_pAosHandle != nullptr);
    }

    virtual void TearDown() override
    {
        if (m_pAosHandle)
        {
            delete m_pAosHandle;
        }
    }

    IMS_UINT32 GetState() { return m_pAosHandle->GetState(); }

    void AddBlock(IN IMS_UINT32 nBlock) { m_pAosHandle->AddBlock(nBlock, m_pAosHandle->m_nBlocks); }

    void AddBlock(IN IMS_UINT32 nBlock, IN_OUT IMS_UINT32& nBlocks)
    {
        m_pAosHandle->AddBlock(nBlock, nBlocks);
    }

    void RemoveBlock(IN IMS_UINT32 nBlock, IN_OUT IMS_UINT32& nBlocks)
    {
        m_pAosHandle->RemoveBlock(nBlock, nBlocks);
    }

    void ClearBlocks() { m_pAosHandle->m_nBlocks = 0; }

    IMS_UINT32 GetBlocks() { return m_pAosHandle->m_nBlocks; }

    IMS_BOOL IsHandleBlocked() { return m_pAosHandle->IsHandleBlocked(); }

    IMS_BOOL IsHandleBlocked(IN IMS_UINT32 nType) { return m_pAosHandle->IsHandleBlocked(nType); }
};

TEST_F(AosHandleTest, Constructor)
{
    EXPECT_EQ(GetState(), AosHandle::STATE_DISCONNECTED);
}

TEST_F(AosHandleTest, AddBlock_Normal)
{
    IMS_UINT32 nTestBlocks = AosHandle::BLOCK_NONE;

    AddBlock(AosHandle::BLOCK_VOLTE_CAPABILITY, nTestBlocks);
    EXPECT_EQ(nTestBlocks, AosHandle::BLOCK_VOLTE_CAPABILITY);

    AddBlock(AosHandle::BLOCK_VILTE_CAPABILITY, nTestBlocks);
    EXPECT_EQ(nTestBlocks, (AosHandle::BLOCK_VOLTE_CAPABILITY | AosHandle::BLOCK_VILTE_CAPABILITY));
}

TEST_F(AosHandleTest, AddBlock_Duplicated)
{
    IMS_UINT32 nTestBlocks = AosHandle::BLOCK_NONE;

    AddBlock(AosHandle::BLOCK_VOLTE_CAPABILITY, nTestBlocks);
    AddBlock(AosHandle::BLOCK_VOLTE_CAPABILITY, nTestBlocks);
    AddBlock(AosHandle::BLOCK_VOLTE_CAPABILITY, nTestBlocks);

    EXPECT_EQ(nTestBlocks, AosHandle::BLOCK_VOLTE_CAPABILITY);
}

TEST_F(AosHandleTest, RemoveBlock_Normal)
{
    IMS_UINT32 nTestBlocks =
            (AosHandle::BLOCK_VOLTE_CAPABILITY | AosHandle::BLOCK_VILTE_CAPABILITY);

    RemoveBlock(AosHandle::BLOCK_VOLTE_CAPABILITY, nTestBlocks);
    EXPECT_EQ(nTestBlocks, AosHandle::BLOCK_VILTE_CAPABILITY);

    RemoveBlock(AosHandle::BLOCK_VILTE_CAPABILITY, nTestBlocks);
    EXPECT_EQ(nTestBlocks, AosHandle::BLOCK_NONE);
}

TEST_F(AosHandleTest, RemoveBlock_NotExisted)
{
    IMS_UINT32 nTestBlocks =
            (AosHandle::BLOCK_VOLTE_CAPABILITY | AosHandle::BLOCK_VILTE_CAPABILITY);

    RemoveBlock(AosHandle::BLOCK_VOLTE_CAPABILITY, nTestBlocks);
    RemoveBlock(AosHandle::BLOCK_VOLTE_CAPABILITY, nTestBlocks);
    RemoveBlock(AosHandle::BLOCK_VOLTE_CAPABILITY, nTestBlocks);

    EXPECT_EQ(nTestBlocks, AosHandle::BLOCK_VILTE_CAPABILITY);
}

TEST_F(AosHandleTest, IsHandleBlocked_ForEach)
{
    ClearBlocks();
    AddBlock(AosHandle::BLOCK_VOLTE_CAPABILITY);
    AddBlock(AosHandle::BLOCK_VILTE_CAPABILITY);
    AddBlock(AosHandle::BLOCK_SMS_CAPABILITY);
    AddBlock(AosHandle::BLOCK_NETWORK);
    AddBlock(AosHandle::BLOCK_VOPS);
    AddBlock(AosHandle::BLOCK_SMS_OVER_IP_NETWORK_INDICATION);
    AddBlock(AosHandle::BLOCK_LIMITED_REGISTRATION);
    AddBlock(AosHandle::BLOCK_VOWIFI_CAPABILITY);
    AddBlock(AosHandle::BLOCK_VIWIFI_CAPABILITY);
    EXPECT_TRUE(IsHandleBlocked(AosHandle::BLOCK_VOLTE_CAPABILITY));
    EXPECT_TRUE(IsHandleBlocked(AosHandle::BLOCK_VILTE_CAPABILITY));
    EXPECT_TRUE(IsHandleBlocked(AosHandle::BLOCK_SMS_CAPABILITY));
    EXPECT_TRUE(IsHandleBlocked(AosHandle::BLOCK_NETWORK));
    EXPECT_TRUE(IsHandleBlocked(AosHandle::BLOCK_VOPS));
    EXPECT_TRUE(IsHandleBlocked(AosHandle::BLOCK_SMS_OVER_IP_NETWORK_INDICATION));
    EXPECT_TRUE(IsHandleBlocked(AosHandle::BLOCK_LIMITED_REGISTRATION));
    EXPECT_TRUE(IsHandleBlocked(AosHandle::BLOCK_VOWIFI_CAPABILITY));
    EXPECT_TRUE(IsHandleBlocked(AosHandle::BLOCK_VIWIFI_CAPABILITY));

    ClearBlocks();
    EXPECT_FALSE(IsHandleBlocked(AosHandle::BLOCK_VOLTE_CAPABILITY));
    EXPECT_FALSE(IsHandleBlocked(AosHandle::BLOCK_VILTE_CAPABILITY));
    EXPECT_FALSE(IsHandleBlocked(AosHandle::BLOCK_SMS_CAPABILITY));
    EXPECT_FALSE(IsHandleBlocked(AosHandle::BLOCK_NETWORK));
    EXPECT_FALSE(IsHandleBlocked(AosHandle::BLOCK_VOPS));
    EXPECT_FALSE(IsHandleBlocked(AosHandle::BLOCK_SMS_OVER_IP_NETWORK_INDICATION));
    EXPECT_FALSE(IsHandleBlocked(AosHandle::BLOCK_LIMITED_REGISTRATION));
    EXPECT_FALSE(IsHandleBlocked(AosHandle::BLOCK_VOWIFI_CAPABILITY));
    EXPECT_FALSE(IsHandleBlocked(AosHandle::BLOCK_VIWIFI_CAPABILITY));
}

TEST_F(AosHandleTest, IsHandleBlocked_ForAll)
{
    ClearBlocks();
    AddBlock(AosHandle::BLOCK_VOLTE_CAPABILITY);
    EXPECT_TRUE(IsHandleBlocked());

    ClearBlocks();
    EXPECT_FALSE(IsHandleBlocked());
}