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
#include "interface/IAosHandle.h"
#include "interface/MockIAosAppContext.h"
#include "interface/MockIAosApplication.h"
#include "../../interface/aos/MockIImsAosListener.h"

using ::testing::_;
using ::testing::AnyNumber;
using ::testing::Return;
using ::testing::ReturnRef;

class AosHandleTest : public ::testing::Test
{
public:
    AosHandle* m_pAosHandle;
    MockIAosAppContext m_objMockIAosAppContext;
    MockIAosApplication m_objMockIAosApplication;
    MockIImsAosListener m_objMockIImsAosListener;

    const AString m_strAppId = AString("ims.app.test");
    const AString m_strServiceId = AString("ims.service.test");
    const IMS_UINT32 m_nServiceType = -1;

protected:
    virtual void SetUp() override
    {
        EXPECT_CALL(m_objMockIAosAppContext, GetSlotId())
                .Times(AnyNumber())
                .WillRepeatedly(Return(0));

        const AString strValue = AString("test");
        EXPECT_CALL(m_objMockIAosAppContext, GetProfileId())
                .Times(AnyNumber())
                .WillRepeatedly(ReturnRef(strValue));

        EXPECT_CALL(m_objMockIAosAppContext, GetApp())
                .Times(AnyNumber())
                .WillRepeatedly(Return(&m_objMockIAosApplication));

        m_pAosHandle = new AosHandle(static_cast<IAosAppContext*>(&m_objMockIAosAppContext),
                m_strAppId, m_strServiceId, m_nServiceType);

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

    void ClearFeature() { m_pAosHandle->m_objFeatureTagList.Clear(); }

    void ClearBindedFeature() { m_pAosHandle->m_objBindedFeatureTagList.Clear(); }

    void AddFeature(IN IMS_UINT32 nFeature)
    {
        m_pAosHandle->m_objFeatureTagList.AddFeature(nFeature);
    }

    void AddBindedFeature(IN IMS_UINT32 nFeature)
    {
        m_pAosHandle->m_objBindedFeatureTagList.AddFeature(nFeature);
    }

    void SetHandleState(IN IMS_UINT32 nState) { m_pAosHandle->SetHandleState(nState); }

    IImsAosListener* GetListener() { return m_pAosHandle->m_piListener; }

    void SetNotify(IN IMS_BOOL bNotify) { m_pAosHandle->m_bNotify = bNotify; }

    IMS_BOOL GetNotify() { return m_pAosHandle->m_bNotify; }
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

TEST_F(AosHandleTest, GetAppId_)
{
    EXPECT_STREQ(m_pAosHandle->GetAppId().GetStr(), m_strAppId.GetStr());
}

TEST_F(AosHandleTest, GetServiceId_)
{
    EXPECT_STREQ(m_pAosHandle->GetServiceId().GetStr(), m_strServiceId.GetStr());
}

TEST_F(AosHandleTest, GetServiceType_)
{
    EXPECT_EQ(m_pAosHandle->GetServiceType(), m_nServiceType);
}

TEST_F(AosHandleTest, SetRequestType_GetRequestType)
{
    m_pAosHandle->SetRequestType(IAosHandle::ATTACH);
    EXPECT_EQ(m_pAosHandle->GetRequestType(), IAosHandle::ATTACH);
    m_pAosHandle->SetRequestType(IAosHandle::DETACH);
    EXPECT_EQ(m_pAosHandle->GetRequestType(), IAosHandle::DETACH);
}

TEST_F(AosHandleTest, SetRegBinded_IsRegBinded)
{
    m_pAosHandle->SetRegBinded(IMS_TRUE);
    EXPECT_TRUE(m_pAosHandle->IsRegBinded());
    m_pAosHandle->SetRegBinded(IMS_FALSE);
    EXPECT_FALSE(m_pAosHandle->IsRegBinded());
}

TEST_F(AosHandleTest, SetNetworkRegBinded_IsNetworkRegBinded)
{
    m_pAosHandle->SetNetworkRegBinded(IMS_TRUE);
    EXPECT_TRUE(m_pAosHandle->IsNetworkRegBinded());
    m_pAosHandle->SetNetworkRegBinded(IMS_FALSE);
    EXPECT_FALSE(m_pAosHandle->IsNetworkRegBinded());
}

TEST_F(AosHandleTest, ProcessFeatureTagChange_NoChange)
{
    EXPECT_CALL(m_objMockIAosApplication, Reconfig()).Times(0);

    ClearFeature();
    ClearBindedFeature();
    AddFeature(ImsAosFeature::MMTEL);
    AddBindedFeature(ImsAosFeature::MMTEL);

    m_pAosHandle->ProcessFeatureTagChange();
}

TEST_F(AosHandleTest, ProcessFeatureTagChange_STATE_DISCONNECTED)
{
    EXPECT_CALL(m_objMockIAosApplication, Reconfig()).Times(0);

    ClearFeature();
    ClearBindedFeature();
    AddFeature(ImsAosFeature::MMTEL);
    SetHandleState(AosHandle::STATE_DISCONNECTED);

    m_pAosHandle->ProcessFeatureTagChange();
}

TEST_F(AosHandleTest, ProcessFeatureTagChange_STATE_DISCONNECTING)
{
    EXPECT_CALL(m_objMockIAosApplication, Reconfig()).Times(0);

    ClearFeature();
    ClearBindedFeature();
    AddFeature(ImsAosFeature::MMTEL);
    SetHandleState(AosHandle::STATE_DISCONNECTING);

    m_pAosHandle->ProcessFeatureTagChange();
}

TEST_F(AosHandleTest, ProcessFeatureTagChange_STATE_CONNECTING)
{
    EXPECT_CALL(m_objMockIAosApplication, Reconfig()).Times(1);

    ClearFeature();
    ClearBindedFeature();
    AddFeature(ImsAosFeature::MMTEL);
    SetHandleState(AosHandle::STATE_CONNECTING);

    m_pAosHandle->ProcessFeatureTagChange();
}

TEST_F(AosHandleTest, ProcessFeatureTagChange_STATE_CONNECTED)
{
    EXPECT_CALL(m_objMockIAosApplication, Reconfig()).Times(1);

    ClearFeature();
    ClearBindedFeature();
    AddFeature(ImsAosFeature::MMTEL);
    SetHandleState(AosHandle::STATE_CONNECTED);

    m_pAosHandle->ProcessFeatureTagChange();
}

TEST_F(AosHandleTest, SetListener_)
{
    IImsAosListener* piListener = static_cast<IImsAosListener*>(&m_objMockIImsAosListener);
    m_pAosHandle->SetListener(piListener);
    EXPECT_EQ(GetListener(), piListener);
}

TEST_F(AosHandleTest, App_Notify_Null_Listener)
{
    m_pAosHandle->SetListener(IMS_NULL);
    ASSERT_EQ(GetListener(), nullptr);

    SetHandleState(AosHandle::STATE_DISCONNECTED);
    EXPECT_CALL(m_objMockIImsAosListener, ImsAos_Disconnected(_)).Times(0);
    EXPECT_FALSE(m_pAosHandle->App_Notify());

    SetHandleState(AosHandle::STATE_CONNECTING);
    EXPECT_FALSE(m_pAosHandle->App_Notify());

    SetHandleState(AosHandle::STATE_CONNECTED);
    EXPECT_CALL(m_objMockIImsAosListener, ImsAos_Connected(_, _)).Times(0);
    EXPECT_FALSE(m_pAosHandle->App_Notify());

    SetHandleState(AosHandle::STATE_DISCONNECTING);
    EXPECT_CALL(m_objMockIImsAosListener, ImsAos_Disconnecting(_)).Times(0);
    EXPECT_FALSE(m_pAosHandle->App_Notify());
}

TEST_F(AosHandleTest, App_Notify_No_Notify)
{
    m_pAosHandle->SetListener(static_cast<IImsAosListener*>(&m_objMockIImsAosListener));
    ASSERT_NE(GetListener(), nullptr);

    SetNotify(IMS_FALSE);
    ASSERT_FALSE(GetNotify());

    SetHandleState(AosHandle::STATE_DISCONNECTED);
    EXPECT_CALL(m_objMockIImsAosListener, ImsAos_Disconnected(_)).Times(0);
    EXPECT_FALSE(m_pAosHandle->App_Notify());

    SetHandleState(AosHandle::STATE_CONNECTING);
    EXPECT_FALSE(m_pAosHandle->App_Notify());

    SetHandleState(AosHandle::STATE_CONNECTED);
    EXPECT_CALL(m_objMockIImsAosListener, ImsAos_Connected(_, _)).Times(0);
    EXPECT_FALSE(m_pAosHandle->App_Notify());

    SetHandleState(AosHandle::STATE_DISCONNECTING);
    EXPECT_CALL(m_objMockIImsAosListener, ImsAos_Disconnecting(_)).Times(0);
    EXPECT_FALSE(m_pAosHandle->App_Notify());
}

TEST_F(AosHandleTest, App_Notify_STATE_DISCONNECTED)
{
    m_pAosHandle->SetListener(static_cast<IImsAosListener*>(&m_objMockIImsAosListener));
    ASSERT_NE(GetListener(), nullptr);

    SetNotify(IMS_TRUE);
    ASSERT_TRUE(GetNotify());

    SetHandleState(AosHandle::STATE_DISCONNECTED);
    EXPECT_CALL(m_objMockIImsAosListener, ImsAos_Disconnected(_)).Times(1);
    EXPECT_TRUE(m_pAosHandle->App_Notify());
}

TEST_F(AosHandleTest, App_Notify_STATE_CONNECTING)
{
    m_pAosHandle->SetListener(static_cast<IImsAosListener*>(&m_objMockIImsAosListener));
    ASSERT_NE(GetListener(), nullptr);

    SetNotify(IMS_TRUE);
    ASSERT_TRUE(GetNotify());

    SetHandleState(AosHandle::STATE_CONNECTING);
    EXPECT_CALL(m_objMockIImsAosListener, ImsAos_Disconnected(_)).Times(1);
    EXPECT_TRUE(m_pAosHandle->App_Notify());
}

TEST_F(AosHandleTest, App_Notify_STATE_CONNECTED)
{
    m_pAosHandle->SetListener(static_cast<IImsAosListener*>(&m_objMockIImsAosListener));
    ASSERT_NE(GetListener(), nullptr);

    SetNotify(IMS_TRUE);
    ASSERT_TRUE(GetNotify());

    SetHandleState(AosHandle::STATE_CONNECTED);
    EXPECT_CALL(m_objMockIImsAosListener, ImsAos_Connected(_, _)).Times(1);
    EXPECT_TRUE(m_pAosHandle->App_Notify());
}

TEST_F(AosHandleTest, App_Notify_STATE_DISCONNECTING)
{
    m_pAosHandle->SetListener(static_cast<IImsAosListener*>(&m_objMockIImsAosListener));
    ASSERT_NE(GetListener(), nullptr);

    SetNotify(IMS_TRUE);
    ASSERT_TRUE(GetNotify());

    SetHandleState(AosHandle::STATE_DISCONNECTING);
    EXPECT_CALL(m_objMockIImsAosListener, ImsAos_Disconnecting(_)).Times(1);
    EXPECT_TRUE(m_pAosHandle->App_Notify());
}