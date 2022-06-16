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

#include "AoSReason.h"
#include "CarrierConfig.h"
#include "ImsAosParameter.h"
#include "ImsAosReason.h"
#include "ImsEventDef.h"
#include "INetworkWatcher.h"

#include "handle/AosHandle.h"
#include "handle/AosFeatureTag.h"
#include "interface/IAosAppContext.h"
#include "interface/IAosHandle.h"
#include "interface/IAosNConfiguration.h"
#include "provider/AosProvider.h"

#include "interface/MockIAosAppContext.h"
#include "interface/MockIAosApplication.h"
#include "interface/MockIAosCallTracker.h"
#include "interface/MockIAosConnection.h"
#include "interface/MockIAosNConfiguration.h"
#include "interface/MockIAosNetTracker.h"
#include "interface/MockIAosService.h"
#include "../../interface/aos/MockIImsAosInfo.h"
#include "../../interface/aos/MockIImsAosListener.h"
#include "../../interface/aos/MockIImsAosMonitor.h"

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
    MockIAosNetTracker m_objMockIAosNetTracker;
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

        EXPECT_CALL(m_objMockIAosAppContext, GetNetTracker())
                .Times(AnyNumber())
                .WillRepeatedly(Return(&m_objMockIAosNetTracker));

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

    void SetState(IN IMS_UINT32 nState) { m_pAosHandle->SetState(nState); }

    IMS_UINT32 GetState() { return m_pAosHandle->GetState(); }

    void AddBlock(IN IMS_UINT32 nBlock) { m_pAosHandle->AddBlock(nBlock, m_pAosHandle->m_nBlocks); }

    void AddBlock(IN IMS_UINT32 nBlock, IN_OUT IMS_UINT32& nBlocks)
    {
        m_pAosHandle->AddBlock(nBlock, nBlocks);
    }

    void AddHoldingBlockForMobile(IN IMS_UINT32 nBlock)
    {
        m_pAosHandle->AddBlock(nBlock, m_pAosHandle->m_nHoldingBlocksForMobile);
    }

    void AddHoldingBlockForWifi(IN IMS_UINT32 nBlock)
    {
        m_pAosHandle->AddBlock(nBlock, m_pAosHandle->m_nHoldingBlocksForWifi);
    }

    void RemoveBlock(IN IMS_UINT32 nBlock, IN_OUT IMS_UINT32& nBlocks)
    {
        m_pAosHandle->RemoveBlock(nBlock, nBlocks);
    }

    void ClearBlocks() { m_pAosHandle->m_nBlocks = 0; }

    void ClearHoldingBlocksForMobile() { m_pAosHandle->m_nHoldingBlocksForMobile = 0; }

    void ClearHoldingBlocksForWifi() { m_pAosHandle->m_nHoldingBlocksForWifi = 0; }

    IMS_UINT32 GetBlocks() { return m_pAosHandle->m_nBlocks; }

    IMS_BOOL IsHandleBlocked() { return m_pAosHandle->IsHandleBlocked(); }

    IMS_BOOL IsHandleBlocked(IN IMS_UINT32 nBlock) { return m_pAosHandle->IsHandleBlocked(nBlock); }

    IMS_BOOL IsHandleBlocked(IN IMS_UINT32& nBlocks, IN IMS_UINT32 nBlock) const
    {
        return m_pAosHandle->IsHandleBlocked(nBlocks, nBlock);
    }

    IMS_BOOL IsHoldingBlockForMobile(IN IMS_UINT32 nBlock)
    {
        return m_pAosHandle->IsHandleBlocked(m_pAosHandle->m_nHoldingBlocksForMobile, nBlock);
    }

    IMS_BOOL IsHoldingBlockForWifi(IN IMS_UINT32 nBlock)
    {
        return m_pAosHandle->IsHandleBlocked(m_pAosHandle->m_nHoldingBlocksForWifi, nBlock);
    }

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

    void AddUnavailableFeature(IN IMS_UINT32 nFeature)
    {
        m_pAosHandle->m_objBindedFeatureTagList.AddUnavailableFeature(nFeature);
    }

    IImsAosListener* GetListener() { return m_pAosHandle->m_piListener; }

    void SetNotify(IN IMS_BOOL bNotify) { m_pAosHandle->m_bNotify = bNotify; }

    IMS_BOOL GetNotify() { return m_pAosHandle->m_bNotify; }

    void SetAosInfo(IN IImsAosInfo* piAosInfo) { m_pAosHandle->m_piInfo = piAosInfo; }

    void SetSuspendedReason(IN IMS_UINT32 nReason) { m_pAosHandle->SetSuspendedReason(nReason); }

    void SetNetSrvIn(IN IMS_BOOL bNetSrvIn) { m_pAosHandle->m_bNetSrvIn = bNetSrvIn; }

    IImsAosInfo* GetAosInfo() { return m_pAosHandle->m_piInfo; }

    void Init() { m_pAosHandle->Init(); }

    void CleanUp() { m_pAosHandle->CleanUp(); }

    void SetHandleState(IN IMS_UINT32 nState) { m_pAosHandle->SetHandleState(nState); }

    void SetReason(IN IMS_UINT32 nReason) { m_pAosHandle->SetReason(nReason); }

    IMS_UINT32 GetReason() { return m_pAosHandle->m_nReason; }

    void ClearSuspendedReason() { m_pAosHandle->ClearSuspendedReason(); }

    IMS_UINT32 GetAppState() { return m_pAosHandle->GetAppState(); }

    IMS_UINT32 GetImsAosReason(IN IMS_UINT32 nAosReason)
    {
        return m_pAosHandle->GetImsAosReason(nAosReason);
    }

    IMS_UINT32 GetImsAosReasonForSuspend(IN IMS_UINT32 nAosReason)
    {
        return m_pAosHandle->GetImsAosReasonForSuspend(nAosReason);
    }

    void SetEpdgEnabled(IN IMS_BOOL bEnabled) { m_pAosHandle->m_bEpdgEnabled = bEnabled; }

    IMS_BOOL IsEpdgEnabled() const { return m_pAosHandle->IsEpdgEnabled(); }

    IMS_BOOL IsEqualNetworkType(IN IMS_UINT32 nType, IN AosNetworkType eType) const
    {
        return m_pAosHandle->IsEqualNetworkType(nType, eType);
    }

    IMS_BOOL IsCapabilityExisted(IN IMS_UINT32 nCapabilities, IN AosCapability eCapability) const
    {
        return m_pAosHandle->IsCapabilityExisted(nCapabilities, eCapability);
    }

    IMS_BOOL IsNetworkTypeMatchedToRat(IMS_UINT32 nNetworkType, IMS_UINT32 nRat) const
    {
        return m_pAosHandle->IsNetworkTypeMatchedToRat(nNetworkType, nRat);
    }

    IMS_BOOL IsServiceFeature(IN IMS_UINT32 nFeature) const
    {
        return m_pAosHandle->IsServiceFeature(nFeature);
    }

    void AddServiceFeature(IMS_UINT32 nAosServiceFeature)
    {
        m_pAosHandle->m_objServiceFeatures.Append(nAosServiceFeature);
    }

    IMS_UINT32 GetNetworkType() const { return m_pAosHandle->GetNetworkType(); }

    IMS_UINT32 GetMobileNetworkType() const { return m_pAosHandle->GetMobileNetworkType(); }

    IMS_UINT32 GetBlock(IN IMS_UINT32 nEvent) { return m_pAosHandle->GetBlock(nEvent); }

    IMS_UINT32 GetAosFeature(IN IMS_UINT32 nBlock) { return m_pAosHandle->GetAosFeature(nBlock); }

    IMS_UINT32 ConvertToAosFeature(IN IMS_UINT32 nConfigFeature)
    {
        return m_pAosHandle->ConvertToAosFeature(nConfigFeature);
    }

    void SetRegFeatureTagRequired(IN IMS_BOOL bRequired)
    {
        m_pAosHandle->m_bRegFeatureTagRequired = bRequired;
    }

    void SetFeatureTagList(IN AosFeatureTagList& objFeatureTagList)
    {
        m_pAosHandle->m_objFeatureTagList = objFeatureTagList;
    }

    void SetBindedFeatureTagList(IN AosFeatureTagList& objBindedFeatureTagList)
    {
        m_pAosHandle->m_objBindedFeatureTagList = objBindedFeatureTagList;
    }

    void ReevaluateBlocks() { m_pAosHandle->ReevaluateBlocks(); }

    IMS_BOOL UpdateIpcan() { return m_pAosHandle->UpdateIpcan(); }

    void SetHoldingBlocksPolicyForMobile()
    {
        m_pAosHandle->m_objHoldingBlocksPolicyForMobile.Append(AosHandle::BLOCK_VOLTE_CAPABILITY);
        m_pAosHandle->m_objHoldingBlocksPolicyForMobile.Append(AosHandle::BLOCK_VILTE_CAPABILITY);
        m_pAosHandle->m_objHoldingBlocksPolicyForMobile.Append(AosHandle::BLOCK_VOPS);
    }

    void SetHoldingBlocksPolicyForWifi()
    {
        m_pAosHandle->m_objHoldingBlocksPolicyForWifi.Append(AosHandle::BLOCK_VOWIFI_CAPABILITY);
        m_pAosHandle->m_objHoldingBlocksPolicyForWifi.Append(AosHandle::BLOCK_VIWIFI_CAPABILITY);
    }

    void ClearHoldingBlocksPolicyForMobile()
    {
        m_pAosHandle->m_objHoldingBlocksPolicyForMobile.Clear();
    }

    void ClearHoldingBlocksPolicyForWifi()
    {
        m_pAosHandle->m_objHoldingBlocksPolicyForWifi.Clear();
    }
};

TEST_F(AosHandleTest, Constructor)
{
    EXPECT_EQ(GetState(), AosHandle::STATE_DISCONNECTED);
}

TEST_F(AosHandleTest, AddBlock_IsHandleBlocked_Normal)
{
    IMS_UINT32 nTestBlocks = AosHandle::BLOCK_NONE;

    EXPECT_FALSE(IsHandleBlocked(nTestBlocks, AosHandle::BLOCK_VOLTE_CAPABILITY));
    EXPECT_FALSE(IsHandleBlocked(nTestBlocks, AosHandle::BLOCK_VILTE_CAPABILITY));

    AddBlock(AosHandle::BLOCK_VOLTE_CAPABILITY, nTestBlocks);
    EXPECT_EQ(nTestBlocks, AosHandle::BLOCK_VOLTE_CAPABILITY);
    EXPECT_TRUE(IsHandleBlocked(nTestBlocks, AosHandle::BLOCK_VOLTE_CAPABILITY));
    EXPECT_FALSE(IsHandleBlocked(nTestBlocks, AosHandle::BLOCK_VILTE_CAPABILITY));

    AddBlock(AosHandle::BLOCK_VILTE_CAPABILITY, nTestBlocks);
    EXPECT_EQ(nTestBlocks, (AosHandle::BLOCK_VOLTE_CAPABILITY | AosHandle::BLOCK_VILTE_CAPABILITY));
    EXPECT_TRUE(IsHandleBlocked(nTestBlocks, AosHandle::BLOCK_VOLTE_CAPABILITY));
    EXPECT_TRUE(IsHandleBlocked(nTestBlocks, AosHandle::BLOCK_VILTE_CAPABILITY));
}

TEST_F(AosHandleTest, AddBlock_Duplicated)
{
    IMS_UINT32 nTestBlocks = AosHandle::BLOCK_NONE;

    AddBlock(AosHandle::BLOCK_VOLTE_CAPABILITY, nTestBlocks);
    AddBlock(AosHandle::BLOCK_VOLTE_CAPABILITY, nTestBlocks);
    AddBlock(AosHandle::BLOCK_VOLTE_CAPABILITY, nTestBlocks);

    EXPECT_EQ(nTestBlocks, AosHandle::BLOCK_VOLTE_CAPABILITY);
}

TEST_F(AosHandleTest, RemoveBlock_IsHandleBlocked_Normal)
{
    IMS_UINT32 nTestBlocks =
            (AosHandle::BLOCK_VOLTE_CAPABILITY | AosHandle::BLOCK_VILTE_CAPABILITY);

    EXPECT_TRUE(IsHandleBlocked(nTestBlocks, AosHandle::BLOCK_VOLTE_CAPABILITY));
    EXPECT_TRUE(IsHandleBlocked(nTestBlocks, AosHandle::BLOCK_VILTE_CAPABILITY));

    RemoveBlock(AosHandle::BLOCK_VOLTE_CAPABILITY, nTestBlocks);
    EXPECT_EQ(nTestBlocks, AosHandle::BLOCK_VILTE_CAPABILITY);
    EXPECT_FALSE(IsHandleBlocked(nTestBlocks, AosHandle::BLOCK_VOLTE_CAPABILITY));
    EXPECT_TRUE(IsHandleBlocked(nTestBlocks, AosHandle::BLOCK_VILTE_CAPABILITY));

    RemoveBlock(AosHandle::BLOCK_VILTE_CAPABILITY, nTestBlocks);
    EXPECT_EQ(nTestBlocks, AosHandle::BLOCK_NONE);
    EXPECT_FALSE(IsHandleBlocked(nTestBlocks, AosHandle::BLOCK_VOLTE_CAPABILITY));
    EXPECT_FALSE(IsHandleBlocked(nTestBlocks, AosHandle::BLOCK_VILTE_CAPABILITY));
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
    AddBlock(AosHandle::BLOCK_VOPS);
    AddBlock(AosHandle::BLOCK_SMS_OVER_IP_NETWORK_INDICATION);
    AddBlock(AosHandle::BLOCK_VOWIFI_CAPABILITY);
    AddBlock(AosHandle::BLOCK_VIWIFI_CAPABILITY);
    EXPECT_TRUE(IsHandleBlocked(AosHandle::BLOCK_VOLTE_CAPABILITY));
    EXPECT_TRUE(IsHandleBlocked(AosHandle::BLOCK_VILTE_CAPABILITY));
    EXPECT_TRUE(IsHandleBlocked(AosHandle::BLOCK_SMS_CAPABILITY));
    EXPECT_TRUE(IsHandleBlocked(AosHandle::BLOCK_VOPS));
    EXPECT_TRUE(IsHandleBlocked(AosHandle::BLOCK_SMS_OVER_IP_NETWORK_INDICATION));
    EXPECT_TRUE(IsHandleBlocked(AosHandle::BLOCK_VOWIFI_CAPABILITY));
    EXPECT_TRUE(IsHandleBlocked(AosHandle::BLOCK_VIWIFI_CAPABILITY));

    ClearBlocks();
    EXPECT_FALSE(IsHandleBlocked(AosHandle::BLOCK_VOLTE_CAPABILITY));
    EXPECT_FALSE(IsHandleBlocked(AosHandle::BLOCK_VILTE_CAPABILITY));
    EXPECT_FALSE(IsHandleBlocked(AosHandle::BLOCK_SMS_CAPABILITY));
    EXPECT_FALSE(IsHandleBlocked(AosHandle::BLOCK_VOPS));
    EXPECT_FALSE(IsHandleBlocked(AosHandle::BLOCK_SMS_OVER_IP_NETWORK_INDICATION));
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

TEST_F(AosHandleTest, IsRegFeatureTagRequired_)
{
    IMS_BOOL bRegFeatureTagRequired = m_pAosHandle->IsRegFeatureTagRequired();

    SetRegFeatureTagRequired(IMS_TRUE);
    EXPECT_TRUE(m_pAosHandle->IsRegFeatureTagRequired());

    SetRegFeatureTagRequired(IMS_FALSE);
    EXPECT_FALSE(m_pAosHandle->IsRegFeatureTagRequired());

    SetRegFeatureTagRequired(bRegFeatureTagRequired);
}

TEST_F(AosHandleTest, GetFeatureTagList_)
{
    AosFeatureTagList& objFeatureTagList = m_pAosHandle->GetFeatureTagList();
    AosFeatureTagList objTestFeatureTagList;

    SetFeatureTagList(objTestFeatureTagList);
    EXPECT_TRUE(m_pAosHandle->GetFeatureTagList().Equals(objTestFeatureTagList));

    SetFeatureTagList(objFeatureTagList);
}

TEST_F(AosHandleTest, GetBindedFeatureTagList_)
{
    AosFeatureTagList& objBindedFeatureTagList = m_pAosHandle->GetBindedFeatureTagList();
    AosFeatureTagList objTestBindedFeatureTagList;

    SetBindedFeatureTagList(objTestBindedFeatureTagList);
    EXPECT_TRUE(m_pAosHandle->GetBindedFeatureTagList().Equals(objTestBindedFeatureTagList));

    SetBindedFeatureTagList(objBindedFeatureTagList);
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

TEST_F(AosHandleTest, ProcessFeatureTagChange_STATE_INVALID)
{
    EXPECT_CALL(m_objMockIAosApplication, Reconfig()).Times(0);

    ClearFeature();
    ClearBindedFeature();
    AddFeature(ImsAosFeature::MMTEL);
    SetState(AosHandle::STATE_INVALID);

    m_pAosHandle->ProcessFeatureTagChange();
}

TEST_F(AosHandleTest, ProcessFeatureTagChange_STATE_DISCONNECTED)
{
    EXPECT_CALL(m_objMockIAosApplication, Reconfig()).Times(0);

    ClearFeature();
    ClearBindedFeature();
    AddFeature(ImsAosFeature::MMTEL);
    SetState(AosHandle::STATE_DISCONNECTED);

    m_pAosHandle->ProcessFeatureTagChange();
}

TEST_F(AosHandleTest, ProcessFeatureTagChange_STATE_DISCONNECTING)
{
    EXPECT_CALL(m_objMockIAosApplication, Reconfig()).Times(0);

    ClearFeature();
    ClearBindedFeature();
    AddFeature(ImsAosFeature::MMTEL);
    SetState(AosHandle::STATE_DISCONNECTING);

    m_pAosHandle->ProcessFeatureTagChange();
}

TEST_F(AosHandleTest, ProcessFeatureTagChange_STATE_CONNECTING)
{
    EXPECT_CALL(m_objMockIAosApplication, Reconfig()).Times(1);

    ClearFeature();
    ClearBindedFeature();
    AddFeature(ImsAosFeature::MMTEL);
    SetState(AosHandle::STATE_CONNECTING);

    m_pAosHandle->ProcessFeatureTagChange();
}

TEST_F(AosHandleTest, ProcessFeatureTagChange_STATE_CONNECTED)
{
    EXPECT_CALL(m_objMockIAosApplication, Reconfig()).Times(1);

    ClearFeature();
    ClearBindedFeature();
    AddFeature(ImsAosFeature::MMTEL);
    SetState(AosHandle::STATE_CONNECTED);

    m_pAosHandle->ProcessFeatureTagChange();
}

TEST_F(AosHandleTest, App_StateChanged_)
{
    EXPECT_TRUE(m_pAosHandle->App_StateChanged(IAosApplication::APP_DISCONNECTED, 0));
    EXPECT_TRUE(m_pAosHandle->App_StateChanged(IAosApplication::APP_CONNECTED, 0));
    EXPECT_TRUE(m_pAosHandle->App_StateChanged(IAosApplication::APP_UPDATING, 0));
    EXPECT_TRUE(m_pAosHandle->App_StateChanged(IAosApplication::APP_DISCONNECTING, 0));
    EXPECT_FALSE(m_pAosHandle->App_StateChanged(-1, 0));
    EXPECT_FALSE(m_pAosHandle->App_StateChanged(4, 0));
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

    SetState(AosHandle::STATE_DISCONNECTED);
    EXPECT_CALL(m_objMockIImsAosListener, ImsAos_Disconnected(_)).Times(0);
    EXPECT_FALSE(m_pAosHandle->App_Notify());

    SetState(AosHandle::STATE_CONNECTING);
    EXPECT_FALSE(m_pAosHandle->App_Notify());

    SetState(AosHandle::STATE_CONNECTED);
    EXPECT_CALL(m_objMockIImsAosListener, ImsAos_Connected(_, _)).Times(0);
    EXPECT_FALSE(m_pAosHandle->App_Notify());

    SetState(AosHandle::STATE_DISCONNECTING);
    EXPECT_CALL(m_objMockIImsAosListener, ImsAos_Disconnecting(_)).Times(0);
    EXPECT_FALSE(m_pAosHandle->App_Notify());
}

TEST_F(AosHandleTest, App_Notify_No_Notify)
{
    m_pAosHandle->SetListener(static_cast<IImsAosListener*>(&m_objMockIImsAosListener));
    ASSERT_NE(GetListener(), nullptr);

    SetNotify(IMS_FALSE);
    ASSERT_FALSE(GetNotify());

    SetState(AosHandle::STATE_DISCONNECTED);
    EXPECT_CALL(m_objMockIImsAosListener, ImsAos_Disconnected(_)).Times(0);
    EXPECT_FALSE(m_pAosHandle->App_Notify());

    SetState(AosHandle::STATE_CONNECTING);
    EXPECT_FALSE(m_pAosHandle->App_Notify());

    SetState(AosHandle::STATE_CONNECTED);
    EXPECT_CALL(m_objMockIImsAosListener, ImsAos_Connected(_, _)).Times(0);
    EXPECT_FALSE(m_pAosHandle->App_Notify());

    SetState(AosHandle::STATE_DISCONNECTING);
    EXPECT_CALL(m_objMockIImsAosListener, ImsAos_Disconnecting(_)).Times(0);
    EXPECT_FALSE(m_pAosHandle->App_Notify());
}

TEST_F(AosHandleTest, App_Notify_STATE_INVALID)
{
    m_pAosHandle->SetListener(static_cast<IImsAosListener*>(&m_objMockIImsAosListener));
    ASSERT_NE(GetListener(), nullptr);

    SetNotify(IMS_TRUE);
    ASSERT_TRUE(GetNotify());

    SetState(AosHandle::STATE_INVALID);
    EXPECT_FALSE(m_pAosHandle->App_Notify());
}

TEST_F(AosHandleTest, App_Notify_STATE_DISCONNECTED)
{
    m_pAosHandle->SetListener(static_cast<IImsAosListener*>(&m_objMockIImsAosListener));
    ASSERT_NE(GetListener(), nullptr);

    SetNotify(IMS_TRUE);
    ASSERT_TRUE(GetNotify());

    SetState(AosHandle::STATE_DISCONNECTED);
    EXPECT_CALL(m_objMockIImsAosListener, ImsAos_Disconnected(_)).Times(1);
    EXPECT_TRUE(m_pAosHandle->App_Notify());
}

TEST_F(AosHandleTest, App_Notify_STATE_CONNECTING)
{
    m_pAosHandle->SetListener(static_cast<IImsAosListener*>(&m_objMockIImsAosListener));
    ASSERT_NE(GetListener(), nullptr);

    SetNotify(IMS_TRUE);
    ASSERT_TRUE(GetNotify());

    SetState(AosHandle::STATE_CONNECTING);
    EXPECT_CALL(m_objMockIImsAosListener, ImsAos_Disconnected(_)).Times(1);
    EXPECT_TRUE(m_pAosHandle->App_Notify());
}

TEST_F(AosHandleTest, App_Notify_STATE_CONNECTED)
{
    m_pAosHandle->SetListener(static_cast<IImsAosListener*>(&m_objMockIImsAosListener));
    ASSERT_NE(GetListener(), nullptr);

    SetNotify(IMS_TRUE);
    ASSERT_TRUE(GetNotify());

    SetState(AosHandle::STATE_CONNECTED);
    EXPECT_CALL(m_objMockIImsAosListener, ImsAos_Connected(_, _)).Times(1);
    EXPECT_TRUE(m_pAosHandle->App_Notify());
}

TEST_F(AosHandleTest, App_Notify_STATE_DISCONNECTING)
{
    m_pAosHandle->SetListener(static_cast<IImsAosListener*>(&m_objMockIImsAosListener));
    ASSERT_NE(GetListener(), nullptr);

    SetNotify(IMS_TRUE);
    ASSERT_TRUE(GetNotify());

    SetState(AosHandle::STATE_DISCONNECTING);
    EXPECT_CALL(m_objMockIImsAosListener, ImsAos_Disconnecting(_)).Times(1);
    EXPECT_TRUE(m_pAosHandle->App_Notify());
}

TEST_F(AosHandleTest, Control_)
{
    EXPECT_CALL(m_objMockIAosApplication, RequestCmd(_, _)).Times(1);
    m_pAosHandle->Control(0);
}

TEST_F(AosHandleTest, GetAosInfo_)
{
    MockIImsAosInfo objMockIImsAosInfo;
    IImsAosInfo* piAosInfo = static_cast<IImsAosInfo*>(&objMockIImsAosInfo);
    SetAosInfo(piAosInfo);
    EXPECT_EQ(m_pAosHandle->GetAosInfo(), piAosInfo);
}

TEST_F(AosHandleTest, GetFeatures_IsImsConnected_False)
{
    SetState(AosHandle::STATE_DISCONNECTED);
    EXPECT_EQ(m_pAosHandle->GetFeatures(), ImsAosFeature::NONE);
}

TEST_F(AosHandleTest, GetFeatures_Mmtel_Video_Binded_And_No_Unavailable)
{
    SetState(AosHandle::STATE_CONNECTED);

    AddBindedFeature(ImsAosFeature::MMTEL);
    AddBindedFeature(ImsAosFeature::VIDEO);

    EXPECT_EQ(m_pAosHandle->GetFeatures(), (ImsAosFeature::MMTEL | ImsAosFeature::VIDEO));
}

TEST_F(AosHandleTest, GetFeatures_Mmtel_Video_Binded_And_Mmtel_Unavailable)
{
    SetState(AosHandle::STATE_CONNECTED);

    AddBindedFeature(ImsAosFeature::MMTEL);
    AddBindedFeature(ImsAosFeature::VIDEO);
    AddUnavailableFeature(ImsAosFeature::MMTEL);

    EXPECT_EQ(m_pAosHandle->GetFeatures(), ImsAosFeature::VIDEO);
}

TEST_F(AosHandleTest, GetFeatures_Mmtel_Binded_And_Mmtel_Unavailable)
{
    SetState(AosHandle::STATE_CONNECTED);

    AddBindedFeature(ImsAosFeature::MMTEL);
    AddUnavailableFeature(ImsAosFeature::MMTEL);

    EXPECT_EQ(m_pAosHandle->GetFeatures(), ImsAosFeature::NONE);
}

TEST_F(AosHandleTest, SetSuspendedReason_GetSuspendedReason)
{
    SetSuspendedReason(AoSReason::SUSPEND_CS_CALL);
    EXPECT_EQ(m_pAosHandle->GetSuspendedReason(), AoSReason::SUSPEND_NONE);

    SetSuspendedReason(AoSReason::SUSPEND_LOW_BATTERY);
    EXPECT_EQ(m_pAosHandle->GetSuspendedReason(), AoSReason::SUSPEND_NONE);

    SetSuspendedReason(AoSReason::SUSPEND_NO_LTE_COVERAGE);
    EXPECT_EQ(m_pAosHandle->GetSuspendedReason(), AoSReason::SUSPEND_NONE);

    SetSuspendedReason(AoSReason::SUSPEND_INSTANTANEOUS_OFFLINE);
    EXPECT_EQ(m_pAosHandle->GetSuspendedReason(), AoSReason::SUSPEND_NONE);

    SetSuspendedReason(AoSReason::SUSPEND_NO_SERVICE);
    EXPECT_EQ(m_pAosHandle->GetSuspendedReason(), AoSReason::SUSPEND_NO_SERVICE);

    SetSuspendedReason(AoSReason::SUSPEND_CS_CALL);
    EXPECT_EQ(m_pAosHandle->GetSuspendedReason(), AoSReason::SUSPEND_NO_SERVICE);

    SetSuspendedReason(AoSReason::SUSPEND_LOW_BATTERY);
    EXPECT_EQ(m_pAosHandle->GetSuspendedReason(), AoSReason::SUSPEND_NO_SERVICE);

    SetSuspendedReason(AoSReason::SUSPEND_NO_LTE_COVERAGE);
    EXPECT_EQ(m_pAosHandle->GetSuspendedReason(), AoSReason::SUSPEND_NO_SERVICE);

    SetSuspendedReason(AoSReason::SUSPEND_INSTANTANEOUS_OFFLINE);
    EXPECT_EQ(m_pAosHandle->GetSuspendedReason(), AoSReason::SUSPEND_NO_SERVICE);
}

TEST_F(AosHandleTest, IsFeatureConnected_)
{
    SetState(AosHandle::STATE_CONNECTED);
    AddBindedFeature(ImsAosFeature::MMTEL);

    EXPECT_TRUE(m_pAosHandle->IsFeatureConnected(ImsAosFeature::MMTEL));
    EXPECT_FALSE(m_pAosHandle->IsFeatureConnected(ImsAosFeature::VIDEO));
}

TEST_F(AosHandleTest, IsImsConnected_)
{
    SetState(AosHandle::STATE_DISCONNECTED);
    EXPECT_FALSE(m_pAosHandle->IsImsConnected());

    SetState(AosHandle::STATE_CONNECTING);
    EXPECT_FALSE(m_pAosHandle->IsImsConnected());

    SetState(AosHandle::STATE_CONNECTED);
    EXPECT_TRUE(m_pAosHandle->IsImsConnected());

    SetState(AosHandle::STATE_DISCONNECTING);
    EXPECT_FALSE(m_pAosHandle->IsImsConnected());
}

TEST_F(AosHandleTest, IsImsSuspended_)
{
    EXPECT_FALSE(m_pAosHandle->IsImsSuspended());

    SetSuspendedReason(AoSReason::SUSPEND_NO_SERVICE);
    EXPECT_TRUE(m_pAosHandle->IsImsSuspended());
}

TEST_F(AosHandleTest, SetMonitor_GetMonitor)
{
    IImsAosMonitor* piMonitor = m_pAosHandle->GetMonitor();

    MockIImsAosMonitor objMockIImsAosMonitor;
    IImsAosMonitor* piTestMonitor = static_cast<IImsAosMonitor*>(&objMockIImsAosMonitor);
    m_pAosHandle->SetMonitor(piTestMonitor);
    EXPECT_EQ(m_pAosHandle->GetMonitor(), piTestMonitor);

    m_pAosHandle->SetMonitor(piMonitor);
}

TEST_F(AosHandleTest, SetReady_Not_Mtc)
{
    EXPECT_FALSE(m_pAosHandle->SetReady(IMS_TRUE, ImsAosService::MTS));
    EXPECT_FALSE(m_pAosHandle->SetReady(IMS_TRUE, ImsAosService::EMERGENCY_MTC));
    EXPECT_FALSE(m_pAosHandle->SetReady(IMS_TRUE, ImsAosService::EMERGENCY_MTS));
    EXPECT_FALSE(m_pAosHandle->SetReady(IMS_TRUE, ImsAosService::UCE));
    EXPECT_FALSE(m_pAosHandle->SetReady(IMS_TRUE, ImsAosService::SIP_CONTROLLER));
}

TEST_F(AosHandleTest, SetReady_Null_CallTracker)
{
    IAosCallTracker* piCallTracker = AosProvider::GetInstance()->GetCallTracker();

    AosProvider::GetInstance()->SetCallTracker(IMS_NULL, 0);
    EXPECT_FALSE(m_pAosHandle->SetReady(IMS_TRUE, ImsAosService::MTC));
    EXPECT_FALSE(m_pAosHandle->SetReady(IMS_FALSE, ImsAosService::MTC));

    AosProvider::GetInstance()->SetCallTracker(piCallTracker);
}

TEST_F(AosHandleTest, SetReady_Mtc)
{
    IAosCallTracker* piCallTracker = AosProvider::GetInstance()->GetCallTracker();

    MockIAosCallTracker objMockIAosCallTracker;
    AosProvider::GetInstance()->SetCallTracker(
            static_cast<IAosCallTracker*>(&objMockIAosCallTracker), 0);
    EXPECT_TRUE(m_pAosHandle->SetReady(IMS_TRUE, ImsAosService::MTC));
    EXPECT_FALSE(m_pAosHandle->SetReady(IMS_FALSE, ImsAosService::MTC));

    AosProvider::GetInstance()->SetCallTracker(piCallTracker);
}

TEST_F(AosHandleTest, NetTracker_StatusChanged_)
{
    SetState(AosHandle::STATE_CONNECTED);

    EXPECT_CALL(m_objMockIAosNetTracker, IsSuspended())
            .Times(2)
            .WillOnce(Return(IMS_FALSE))
            .WillOnce(Return(IMS_TRUE));

    MockIAosConnection objMockIAosConnection;
    EXPECT_CALL(m_objMockIAosAppContext, GetConnection())
            .Times(AnyNumber())
            .WillRepeatedly(Return(static_cast<IAosConnection*>(&objMockIAosConnection)));
    EXPECT_CALL(objMockIAosConnection, IsEpdgEnabled())
            .Times(AnyNumber())
            .WillRepeatedly(Return(IMS_FALSE));

    m_pAosHandle->NetTracker_StatusChanged();
    EXPECT_FALSE(m_pAosHandle->IsImsSuspended());

    m_pAosHandle->NetTracker_StatusChanged();
    EXPECT_TRUE(m_pAosHandle->IsImsSuspended());
}

TEST_F(AosHandleTest, Init_CleanUp)
{
    EXPECT_CALL(m_objMockIAosNetTracker, SetListener(_)).Times(1);
    EXPECT_CALL(m_objMockIAosNetTracker, RemoveListener(_)).Times(1);

    IAosNConfiguration* piAosNConfiguration = AosProvider::GetInstance()->GetNConfiguration();
    IAosService* piAosService = AosProvider::GetInstance()->GetService();

    MockIAosService objMockIAosService;
    AosProvider::GetInstance()->SetService(static_cast<IAosService*>(&objMockIAosService), 0);
    EXPECT_CALL(objMockIAosService,
            AddListener(DYNAMIC_CAST(IAosRegistrationControlListener*, m_pAosHandle)))
            .Times(1);
    EXPECT_CALL(
            objMockIAosService, AddListener(DYNAMIC_CAST(IAosServicePhoneListener*, m_pAosHandle)))
            .Times(1);
    EXPECT_CALL(objMockIAosService,
            AddListener(DYNAMIC_CAST(IAosServiceSettingListener*, m_pAosHandle)))
            .Times(1);
    EXPECT_CALL(objMockIAosService,
            RemoveListener(DYNAMIC_CAST(IAosRegistrationControlListener*, m_pAosHandle)))
            .Times(1);
    EXPECT_CALL(objMockIAosService,
            RemoveListener(DYNAMIC_CAST(IAosServicePhoneListener*, m_pAosHandle)))
            .Times(1);
    EXPECT_CALL(objMockIAosService,
            RemoveListener(DYNAMIC_CAST(IAosServiceSettingListener*, m_pAosHandle)))
            .Times(1);

    EXPECT_TRUE(GetAosInfo() == nullptr);

    MockIAosNConfiguration objMockIAosNConfiguration;
    AosProvider::GetInstance()->SetNConfiguration(
            static_cast<IAosNConfiguration*>(&objMockIAosNConfiguration), 0);
    EXPECT_CALL(objMockIAosNConfiguration, IsCdmalessFeatureTagRequired())
            .Times(1)
            .WillRepeatedly(Return(IMS_TRUE));

    Init();

    EXPECT_TRUE(GetAosInfo() != nullptr);

    CleanUp();

    EXPECT_TRUE(GetAosInfo() == nullptr);

    AosProvider::GetInstance()->SetNConfiguration(piAosNConfiguration);
    AosProvider::GetInstance()->SetService(piAosService);
}

TEST_F(AosHandleTest, SetHandleState_)
{
    SetSuspendedReason(AoSReason::SUSPEND_NO_SERVICE);
    EXPECT_TRUE(m_pAosHandle->IsImsSuspended());

    SetHandleState(AosHandle::STATE_DISCONNECTED);
    EXPECT_EQ(GetState(), AosHandle::STATE_DISCONNECTED);
    EXPECT_FALSE(m_pAosHandle->IsImsSuspended());

    SetSuspendedReason(AoSReason::SUSPEND_NO_SERVICE);
    EXPECT_TRUE(m_pAosHandle->IsImsSuspended());

    SetHandleState(AosHandle::STATE_DISCONNECTING);
    EXPECT_EQ(GetState(), AosHandle::STATE_DISCONNECTING);
    EXPECT_FALSE(m_pAosHandle->IsImsSuspended());

    SetSuspendedReason(AoSReason::SUSPEND_NO_SERVICE);
    EXPECT_TRUE(m_pAosHandle->IsImsSuspended());

    SetHandleState(AosHandle::STATE_CONNECTING);
    EXPECT_EQ(GetState(), AosHandle::STATE_CONNECTING);
    EXPECT_TRUE(m_pAosHandle->IsImsSuspended());

    SetHandleState(AosHandle::STATE_CONNECTED);
    EXPECT_EQ(GetState(), AosHandle::STATE_CONNECTED);
    EXPECT_TRUE(m_pAosHandle->IsImsSuspended());
}

TEST_F(AosHandleTest, SetReason_)
{
    SetReason(AoSReason::NONE);
    EXPECT_EQ(GetReason(), AoSReason::NONE);

    SetReason(AoSReason::SRV_OUT);
    EXPECT_EQ(GetReason(), AoSReason::SRV_OUT);

    SetReason(AoSReason::CS_CONNECTED);
    EXPECT_EQ(GetReason(), AoSReason::CS_CONNECTED);

    SetReason(AoSReason::DATA_OFF);
    EXPECT_EQ(GetReason(), AoSReason::DATA_OFF);

    SetReason(AoSReason::POWER_OFF);
    EXPECT_EQ(GetReason(), AoSReason::POWER_OFF);

    SetReason(AoSReason::BAD_BATTERY);
    EXPECT_EQ(GetReason(), AoSReason::BAD_BATTERY);

    SetReason(AoSReason::AIRPLANE_MODE);
    EXPECT_EQ(GetReason(), AoSReason::AIRPLANE_MODE);

    SetReason(AoSReason::NO_LTE_COVERAGE);
    EXPECT_EQ(GetReason(), AoSReason::NO_LTE_COVERAGE);

    SetReason(AoSReason::SERVICE_POLICY);
    EXPECT_EQ(GetReason(), AoSReason::SERVICE_POLICY);

    SetReason(AoSReason::SERVICE_BLOCKED);
    EXPECT_EQ(GetReason(), AoSReason::SERVICE_BLOCKED);

    SetReason(AoSReason::LTE_SUSPENDED);
    EXPECT_EQ(GetReason(), AoSReason::LTE_SUSPENDED);

    SetReason(AoSReason::IMS_DISABLED);
    EXPECT_EQ(GetReason(), AoSReason::IMS_DISABLED);

    SetReason(AoSReason::TTYMODEON);
    EXPECT_EQ(GetReason(), AoSReason::TTYMODEON);

    SetReason(AoSReason::INSTANTANEOUS_OFFLINE);
    EXPECT_EQ(GetReason(), AoSReason::INSTANTANEOUS_OFFLINE);

    SetReason(AoSReason::NOT_SPECIFIED);
    EXPECT_EQ(GetReason(), AoSReason::NOT_SPECIFIED);

    SetReason(AoSReason::IP_CHANGED);
    EXPECT_EQ(GetReason(), AoSReason::IP_CHANGED);

    SetReason(AoSReason::DATA_DISCONNECTED);
    EXPECT_EQ(GetReason(), AoSReason::DATA_DISCONNECTED);

    SetReason(AoSReason::DATA_CONNECTION_MAINTAIN);
    EXPECT_EQ(GetReason(), AoSReason::DATA_CONNECTION_MAINTAIN);

    SetReason(AoSReason::DATA_PERMANENTLY_FAILED);
    EXPECT_EQ(GetReason(), AoSReason::DATA_PERMANENTLY_FAILED);

    SetReason(AoSReason::REG_FAILURE);
    EXPECT_EQ(GetReason(), AoSReason::REG_FAILURE);

    SetReason(AoSReason::REG_FAILED_LIMITED_SERVICE);
    EXPECT_EQ(GetReason(), AoSReason::REG_FAILED_LIMITED_SERVICE);

    SetReason(AoSReason::REG_REFRESH_FORBIDDEN);
    EXPECT_EQ(GetReason(), AoSReason::REG_REFRESH_FORBIDDEN);

    SetReason(AoSReason::REG_FORBIDDEN);
    EXPECT_EQ(GetReason(), AoSReason::REG_FORBIDDEN);

    SetReason(AoSReason::REG_BANNED);
    EXPECT_EQ(GetReason(), AoSReason::REG_BANNED);

    SetReason(AoSReason::REG_AUTH_FAIL);
    EXPECT_EQ(GetReason(), AoSReason::REG_AUTH_FAIL);

    SetReason(AoSReason::REG_TERMINATED);
    EXPECT_EQ(GetReason(), AoSReason::REG_TERMINATED);

    SetReason(AoSReason::REG_TERMINATED_EXPIRE);
    EXPECT_EQ(GetReason(), AoSReason::REG_TERMINATED_EXPIRE);

    SetReason(AoSReason::INITIAL_REG_REQUESTED);
    EXPECT_EQ(GetReason(), AoSReason::INITIAL_REG_REQUESTED);

    SetReason(AoSReason::PCSCF_DISCOVERY_FAILED);
    EXPECT_EQ(GetReason(), AoSReason::PCSCF_DISCOVERY_FAILED);

    SetReason(AoSReason::REG_FAILED_INTERNAL_ERROR);
    EXPECT_EQ(GetReason(), AoSReason::REG_FAILED_INTERNAL_ERROR);

    SetReason(AoSReason::UNKNOWN);
    EXPECT_EQ(GetReason(), AoSReason::UNKNOWN);

    SetReason(AoSReason::OPERATOR);
    EXPECT_EQ(GetReason(), AoSReason::OPERATOR);
}

TEST_F(AosHandleTest, ClearSuspendedReason_)
{
    SetSuspendedReason(AoSReason::SUSPEND_NO_SERVICE);
    EXPECT_TRUE(m_pAosHandle->IsImsSuspended());

    ClearSuspendedReason();
    EXPECT_FALSE(m_pAosHandle->IsImsSuspended());
}

TEST_F(AosHandleTest, GetAppState_)
{
    SetState(AosHandle::STATE_CONNECTED);
    EXPECT_EQ(GetAppState(), AosHandle::APP_STATE_CONNECTED);

    SetState(AosHandle::STATE_DISCONNECTING);
    EXPECT_EQ(GetAppState(), AosHandle::APP_STATE_DISCONNECTING);

    SetState(AosHandle::STATE_CONNECTING);
    EXPECT_EQ(GetAppState(), AosHandle::APP_STATE_DISCONNECTED);

    SetState(AosHandle::STATE_DISCONNECTED);
    EXPECT_EQ(GetAppState(), AosHandle::APP_STATE_DISCONNECTED);
}

TEST_F(AosHandleTest, GetImsAosReason_)
{
    EXPECT_EQ(GetImsAosReason(AoSReason::NONE), ImsAosReason::NONE);
    EXPECT_EQ(GetImsAosReason(AoSReason::BAD_BATTERY), ImsAosReason::POWER_OFF);
    EXPECT_EQ(GetImsAosReason(AoSReason::POWER_OFF), ImsAosReason::POWER_OFF);
    EXPECT_EQ(GetImsAosReason(AoSReason::AIRPLANE_MODE), ImsAosReason::DATA_DISCONNECTED);
    EXPECT_EQ(GetImsAosReason(AoSReason::DATA_DISCONNECTED), ImsAosReason::DATA_DISCONNECTED);
    EXPECT_EQ(GetImsAosReason(AoSReason::NO_LTE_COVERAGE), ImsAosReason::NO_RAT_COVERAGE);
    EXPECT_EQ(GetImsAosReason(AoSReason::SERVICE_POLICY), ImsAosReason::SERVICE_POLICY);
    EXPECT_EQ(GetImsAosReason(AoSReason::SERVICE_BLOCKED), ImsAosReason::SERVICE_BLOCKED);
    EXPECT_EQ(GetImsAosReason(AoSReason::SRV_OUT), ImsAosReason::OUT_OF_SERVICE);
    EXPECT_EQ(GetImsAosReason(AoSReason::REG_REFRESH_FORBIDDEN), ImsAosReason::REG_TERMINATED);
    EXPECT_EQ(GetImsAosReason(AoSReason::REG_TERMINATED_EXPIRE), ImsAosReason::REG_TERMINATED);
    EXPECT_EQ(GetImsAosReason(AoSReason::REG_TERMINATED), ImsAosReason::REG_TERMINATED);
    EXPECT_EQ(GetImsAosReason(AoSReason::INITIAL_REG_REQUESTED), ImsAosReason::REG_NEW_REQUIRED);

    EXPECT_EQ(GetImsAosReason(AoSReason::CS_CONNECTED), ImsAosReason::NOT_SPECIFIED);
    EXPECT_EQ(GetImsAosReason(AoSReason::DATA_OFF), ImsAosReason::NOT_SPECIFIED);
    EXPECT_EQ(GetImsAosReason(AoSReason::LTE_SUSPENDED), ImsAosReason::NOT_SPECIFIED);
    EXPECT_EQ(GetImsAosReason(AoSReason::IMS_DISABLED), ImsAosReason::NOT_SPECIFIED);
    EXPECT_EQ(GetImsAosReason(AoSReason::TTYMODEON), ImsAosReason::NOT_SPECIFIED);
    EXPECT_EQ(GetImsAosReason(AoSReason::INSTANTANEOUS_OFFLINE), ImsAosReason::NOT_SPECIFIED);
}

TEST_F(AosHandleTest, GetImsAosReasonForSuspend_)
{
    EXPECT_EQ(GetImsAosReasonForSuspend(AoSReason::SUSPEND_NONE), ImsAosReason::SUSPEND_NONE);
    EXPECT_EQ(GetImsAosReasonForSuspend(AoSReason::SUSPEND_NO_SERVICE),
            ImsAosReason::SUSPEND_OUT_OF_SERVICE);
    EXPECT_EQ(GetImsAosReasonForSuspend(AoSReason::SUSPEND_NO_LTE_COVERAGE),
            ImsAosReason::SUSPEND_NO_RAT_COVERAGE);

    EXPECT_EQ(GetImsAosReasonForSuspend(AoSReason::SUSPEND_CS_CALL), ImsAosReason::SUSPEND_NONE);
    EXPECT_EQ(
            GetImsAosReasonForSuspend(AoSReason::SUSPEND_LOW_BATTERY), ImsAosReason::SUSPEND_NONE);
    EXPECT_EQ(GetImsAosReasonForSuspend(AoSReason::SUSPEND_INSTANTANEOUS_OFFLINE),
            ImsAosReason::SUSPEND_NONE);
}

TEST_F(AosHandleTest, IsEpdgEnabled_)
{
    SetEpdgEnabled(IMS_TRUE);
    EXPECT_TRUE(IsEpdgEnabled());

    SetEpdgEnabled(IMS_FALSE);
    EXPECT_FALSE(IsEpdgEnabled());
}

TEST_F(AosHandleTest, IsEqualNetworkType_)
{
    EXPECT_TRUE(IsEqualNetworkType(-1, AosNetworkType::NONE));
    EXPECT_TRUE(IsEqualNetworkType(0, AosNetworkType::LTE));
    EXPECT_TRUE(IsEqualNetworkType(1, AosNetworkType::IWLAN));
    EXPECT_TRUE(IsEqualNetworkType(2, AosNetworkType::CROSS_SIM));
    EXPECT_TRUE(IsEqualNetworkType(3, AosNetworkType::NR));
    EXPECT_TRUE(IsEqualNetworkType(4, AosNetworkType::UTRAN));

    EXPECT_FALSE(IsEqualNetworkType(-1, AosNetworkType::LTE));
    EXPECT_FALSE(IsEqualNetworkType(-1, AosNetworkType::IWLAN));
    EXPECT_FALSE(IsEqualNetworkType(-1, AosNetworkType::CROSS_SIM));
    EXPECT_FALSE(IsEqualNetworkType(-1, AosNetworkType::NR));
    EXPECT_FALSE(IsEqualNetworkType(-1, AosNetworkType::UTRAN));

    EXPECT_FALSE(IsEqualNetworkType(0, AosNetworkType::NONE));
    EXPECT_FALSE(IsEqualNetworkType(0, AosNetworkType::IWLAN));
    EXPECT_FALSE(IsEqualNetworkType(0, AosNetworkType::CROSS_SIM));
    EXPECT_FALSE(IsEqualNetworkType(0, AosNetworkType::NR));
    EXPECT_FALSE(IsEqualNetworkType(0, AosNetworkType::UTRAN));

    EXPECT_FALSE(IsEqualNetworkType(1, AosNetworkType::NONE));
    EXPECT_FALSE(IsEqualNetworkType(1, AosNetworkType::LTE));
    EXPECT_FALSE(IsEqualNetworkType(1, AosNetworkType::CROSS_SIM));
    EXPECT_FALSE(IsEqualNetworkType(1, AosNetworkType::NR));
    EXPECT_FALSE(IsEqualNetworkType(1, AosNetworkType::UTRAN));

    EXPECT_FALSE(IsEqualNetworkType(2, AosNetworkType::NONE));
    EXPECT_FALSE(IsEqualNetworkType(2, AosNetworkType::LTE));
    EXPECT_FALSE(IsEqualNetworkType(2, AosNetworkType::IWLAN));
    EXPECT_FALSE(IsEqualNetworkType(2, AosNetworkType::NR));
    EXPECT_FALSE(IsEqualNetworkType(2, AosNetworkType::UTRAN));

    EXPECT_FALSE(IsEqualNetworkType(3, AosNetworkType::NONE));
    EXPECT_FALSE(IsEqualNetworkType(3, AosNetworkType::LTE));
    EXPECT_FALSE(IsEqualNetworkType(3, AosNetworkType::IWLAN));
    EXPECT_FALSE(IsEqualNetworkType(3, AosNetworkType::CROSS_SIM));
    EXPECT_FALSE(IsEqualNetworkType(3, AosNetworkType::UTRAN));

    EXPECT_FALSE(IsEqualNetworkType(4, AosNetworkType::NONE));
    EXPECT_FALSE(IsEqualNetworkType(4, AosNetworkType::LTE));
    EXPECT_FALSE(IsEqualNetworkType(4, AosNetworkType::IWLAN));
    EXPECT_FALSE(IsEqualNetworkType(4, AosNetworkType::CROSS_SIM));
    EXPECT_FALSE(IsEqualNetworkType(4, AosNetworkType::NR));
}

TEST_F(AosHandleTest, IsCapabilityExisted_)
{
    IMS_UINT32 nCapabilities = (static_cast<IMS_UINT32>(AosCapability::VOICE) |
            static_cast<IMS_UINT32>(AosCapability::VIDEO) |
            static_cast<IMS_UINT32>(AosCapability::UT) |
            static_cast<IMS_UINT32>(AosCapability::SMS) |
            static_cast<IMS_UINT32>(AosCapability::CALL_COMPOSER) |
            static_cast<IMS_UINT32>(AosCapability::OPTIONS_UCE) |
            static_cast<IMS_UINT32>(AosCapability::PRESENCE_UCE));
    EXPECT_TRUE(IsCapabilityExisted(nCapabilities, AosCapability::VOICE));
    EXPECT_TRUE(IsCapabilityExisted(nCapabilities, AosCapability::VIDEO));
    EXPECT_TRUE(IsCapabilityExisted(nCapabilities, AosCapability::UT));
    EXPECT_TRUE(IsCapabilityExisted(nCapabilities, AosCapability::SMS));
    EXPECT_TRUE(IsCapabilityExisted(nCapabilities, AosCapability::CALL_COMPOSER));
    EXPECT_TRUE(IsCapabilityExisted(nCapabilities, AosCapability::OPTIONS_UCE));
    EXPECT_TRUE(IsCapabilityExisted(nCapabilities, AosCapability::PRESENCE_UCE));

    nCapabilities = static_cast<IMS_UINT32>(AosCapability::VOICE);
    EXPECT_TRUE(IsCapabilityExisted(nCapabilities, AosCapability::VOICE));
    EXPECT_FALSE(IsCapabilityExisted(nCapabilities, AosCapability::VIDEO));
    EXPECT_FALSE(IsCapabilityExisted(nCapabilities, AosCapability::UT));
    EXPECT_FALSE(IsCapabilityExisted(nCapabilities, AosCapability::SMS));
    EXPECT_FALSE(IsCapabilityExisted(nCapabilities, AosCapability::CALL_COMPOSER));
    EXPECT_FALSE(IsCapabilityExisted(nCapabilities, AosCapability::OPTIONS_UCE));
    EXPECT_FALSE(IsCapabilityExisted(nCapabilities, AosCapability::PRESENCE_UCE));

    nCapabilities = static_cast<IMS_UINT32>(AosCapability::VIDEO);
    EXPECT_FALSE(IsCapabilityExisted(nCapabilities, AosCapability::VOICE));
    EXPECT_TRUE(IsCapabilityExisted(nCapabilities, AosCapability::VIDEO));
    EXPECT_FALSE(IsCapabilityExisted(nCapabilities, AosCapability::UT));
    EXPECT_FALSE(IsCapabilityExisted(nCapabilities, AosCapability::SMS));
    EXPECT_FALSE(IsCapabilityExisted(nCapabilities, AosCapability::CALL_COMPOSER));
    EXPECT_FALSE(IsCapabilityExisted(nCapabilities, AosCapability::OPTIONS_UCE));
    EXPECT_FALSE(IsCapabilityExisted(nCapabilities, AosCapability::PRESENCE_UCE));
}

TEST_F(AosHandleTest, IsNetworkTypeMatchedToRat_)
{
    EXPECT_TRUE(IsNetworkTypeMatchedToRat(0, NW_REPORT_RADIO_LTE));
    EXPECT_FALSE(IsNetworkTypeMatchedToRat(0, NW_REPORT_RADIO_WLAN));
    EXPECT_FALSE(IsNetworkTypeMatchedToRat(0, NW_REPORT_RADIO_NR));

    EXPECT_FALSE(IsNetworkTypeMatchedToRat(1, NW_REPORT_RADIO_LTE));
    EXPECT_TRUE(IsNetworkTypeMatchedToRat(1, NW_REPORT_RADIO_WLAN));
    EXPECT_FALSE(IsNetworkTypeMatchedToRat(1, NW_REPORT_RADIO_NR));

    EXPECT_FALSE(IsNetworkTypeMatchedToRat(3, NW_REPORT_RADIO_LTE));
    EXPECT_FALSE(IsNetworkTypeMatchedToRat(3, NW_REPORT_RADIO_WLAN));
    EXPECT_TRUE(IsNetworkTypeMatchedToRat(3, NW_REPORT_RADIO_NR));

    EXPECT_FALSE(IsNetworkTypeMatchedToRat(0, NW_REPORT_RADIO_NOSRV));
    EXPECT_FALSE(IsNetworkTypeMatchedToRat(0, NW_REPORT_RADIO_CDMA));
    EXPECT_FALSE(IsNetworkTypeMatchedToRat(0, NW_REPORT_RADIO_EHRPD));
    EXPECT_FALSE(IsNetworkTypeMatchedToRat(0, NW_REPORT_RADIO_GSM));
    EXPECT_FALSE(IsNetworkTypeMatchedToRat(0, NW_REPORT_RADIO_WCDMA));
}

TEST_F(AosHandleTest, IsServiceFeature_)
{
    EXPECT_FALSE(IsServiceFeature(ImsAosFeature::MMTEL));
    EXPECT_FALSE(IsServiceFeature(ImsAosFeature::VIDEO));
    EXPECT_FALSE(IsServiceFeature(ImsAosFeature::TEXT));
    EXPECT_FALSE(IsServiceFeature(ImsAosFeature::USSI));
    EXPECT_FALSE(IsServiceFeature(ImsAosFeature::VERSTAT));

    AddServiceFeature(ImsAosFeature::MMTEL);
    EXPECT_TRUE(IsServiceFeature(ImsAosFeature::MMTEL));
    EXPECT_FALSE(IsServiceFeature(ImsAosFeature::VIDEO));
    EXPECT_FALSE(IsServiceFeature(ImsAosFeature::TEXT));
    EXPECT_FALSE(IsServiceFeature(ImsAosFeature::USSI));
    EXPECT_FALSE(IsServiceFeature(ImsAosFeature::VERSTAT));

    AddServiceFeature(ImsAosFeature::VIDEO);
    EXPECT_TRUE(IsServiceFeature(ImsAosFeature::MMTEL));
    EXPECT_TRUE(IsServiceFeature(ImsAosFeature::VIDEO));
    EXPECT_FALSE(IsServiceFeature(ImsAosFeature::TEXT));
    EXPECT_FALSE(IsServiceFeature(ImsAosFeature::USSI));
    EXPECT_FALSE(IsServiceFeature(ImsAosFeature::VERSTAT));

    AddServiceFeature(ImsAosFeature::TEXT);
    EXPECT_TRUE(IsServiceFeature(ImsAosFeature::MMTEL));
    EXPECT_TRUE(IsServiceFeature(ImsAosFeature::VIDEO));
    EXPECT_TRUE(IsServiceFeature(ImsAosFeature::TEXT));
    EXPECT_FALSE(IsServiceFeature(ImsAosFeature::USSI));
    EXPECT_FALSE(IsServiceFeature(ImsAosFeature::VERSTAT));

    AddServiceFeature(ImsAosFeature::USSI);
    EXPECT_TRUE(IsServiceFeature(ImsAosFeature::MMTEL));
    EXPECT_TRUE(IsServiceFeature(ImsAosFeature::VIDEO));
    EXPECT_TRUE(IsServiceFeature(ImsAosFeature::TEXT));
    EXPECT_TRUE(IsServiceFeature(ImsAosFeature::USSI));
    EXPECT_FALSE(IsServiceFeature(ImsAosFeature::VERSTAT));

    AddServiceFeature(ImsAosFeature::VERSTAT);
    EXPECT_TRUE(IsServiceFeature(ImsAosFeature::MMTEL));
    EXPECT_TRUE(IsServiceFeature(ImsAosFeature::VIDEO));
    EXPECT_TRUE(IsServiceFeature(ImsAosFeature::TEXT));
    EXPECT_TRUE(IsServiceFeature(ImsAosFeature::USSI));
    EXPECT_TRUE(IsServiceFeature(ImsAosFeature::VERSTAT));
}

TEST_F(AosHandleTest, GetNetworkType_)
{
    EXPECT_CALL(m_objMockIAosNetTracker, GetNetworkType()).Times(1);
    GetNetworkType();
}

TEST_F(AosHandleTest, GetMobileNetworkType_)
{
    EXPECT_CALL(m_objMockIAosNetTracker, GetMobileNetworkType()).Times(1);
    GetMobileNetworkType();
}

TEST_F(AosHandleTest, GetBlock_)
{
    EXPECT_EQ(GetBlock(IMS_EVENT_IMS_VOICE_OVER_PS_STATE), AosHandle::BLOCK_VOPS);

    EXPECT_EQ(GetBlock(IMS_EVENT_ROAMING_STATE), AosHandle::BLOCK_NONE);
    EXPECT_EQ(GetBlock(IMS_EVENT_VOLTE_SETTING), AosHandle::BLOCK_NONE);
    EXPECT_EQ(GetBlock(IMS_EVENT_VIDEO_SETTING), AosHandle::BLOCK_NONE);
    EXPECT_EQ(GetBlock(IMS_EVENT_WFC_SETTING_CHANGED), AosHandle::BLOCK_NONE);
    EXPECT_EQ(GetBlock(IMS_EVENT_MOBILE_DATA_SETTING), AosHandle::BLOCK_NONE);
}

TEST_F(AosHandleTest, GetAosFeature_)
{
    EXPECT_EQ(GetAosFeature(AosHandle::BLOCK_VOLTE_CAPABILITY), ImsAosFeature::MMTEL);
    EXPECT_EQ(GetAosFeature(AosHandle::BLOCK_VOWIFI_CAPABILITY), ImsAosFeature::MMTEL);
    EXPECT_EQ(GetAosFeature(AosHandle::BLOCK_VOPS), ImsAosFeature::MMTEL);

    EXPECT_EQ(GetAosFeature(AosHandle::BLOCK_VILTE_CAPABILITY), ImsAosFeature::VIDEO);
    EXPECT_EQ(GetAosFeature(AosHandle::BLOCK_VILTE_CAPABILITY), ImsAosFeature::VIDEO);

    EXPECT_EQ(GetAosFeature(AosHandle::BLOCK_SMS_CAPABILITY), ImsAosFeature::SMSIP);
    EXPECT_EQ(GetAosFeature(AosHandle::BLOCK_SMS_OVER_IP_NETWORK_INDICATION), ImsAosFeature::SMSIP);

    EXPECT_EQ(GetAosFeature(AosHandle::BLOCK_NONE), ImsAosFeature::NONE);
}

TEST_F(AosHandleTest, ConvertToAosFeature_)
{
    EXPECT_EQ(ConvertToAosFeature(CarrierConfig::Ims::UNAVAILABLE_FEATURE_TYPE_MMTEL),
            ImsAosFeature::MMTEL);
    EXPECT_EQ(ConvertToAosFeature(CarrierConfig::Ims::UNAVAILABLE_FEATURE_TYPE_VIDEO),
            ImsAosFeature::VIDEO);
    EXPECT_EQ(ConvertToAosFeature(CarrierConfig::Ims::UNAVAILABLE_FEATURE_TYPE_SMS),
            ImsAosFeature::SMSIP);
    EXPECT_EQ(ConvertToAosFeature(0), ImsAosFeature::NONE);
}

TEST_F(AosHandleTest, ReevaluateBlocks_Test1)
{
    // Test1: Epdg enabled, mobile block existed
    ClearBlocks();
    ClearHoldingBlocksForMobile();

    ClearHoldingBlocksPolicyForMobile();
    SetHoldingBlocksPolicyForMobile();

    SetEpdgEnabled(IMS_TRUE);
    AddBlock(AosHandle::BLOCK_VOLTE_CAPABILITY);
    AddBlock(AosHandle::BLOCK_VILTE_CAPABILITY);
    AddBlock(AosHandle::BLOCK_VOPS);

    EXPECT_TRUE(IsHandleBlocked(AosHandle::BLOCK_VOLTE_CAPABILITY));
    EXPECT_TRUE(IsHandleBlocked(AosHandle::BLOCK_VILTE_CAPABILITY));
    EXPECT_TRUE(IsHandleBlocked(AosHandle::BLOCK_VOPS));
    EXPECT_FALSE(IsHoldingBlockForMobile(AosHandle::BLOCK_VOLTE_CAPABILITY));
    EXPECT_FALSE(IsHoldingBlockForMobile(AosHandle::BLOCK_VILTE_CAPABILITY));
    EXPECT_FALSE(IsHoldingBlockForMobile(AosHandle::BLOCK_VOPS));

    ReevaluateBlocks();

    EXPECT_FALSE(IsHandleBlocked(AosHandle::BLOCK_VOLTE_CAPABILITY));
    EXPECT_FALSE(IsHandleBlocked(AosHandle::BLOCK_VILTE_CAPABILITY));
    EXPECT_FALSE(IsHandleBlocked(AosHandle::BLOCK_VOPS));
    EXPECT_TRUE(IsHoldingBlockForMobile(AosHandle::BLOCK_VOLTE_CAPABILITY));
    EXPECT_TRUE(IsHoldingBlockForMobile(AosHandle::BLOCK_VILTE_CAPABILITY));
    EXPECT_TRUE(IsHoldingBlockForMobile(AosHandle::BLOCK_VOPS));
}

TEST_F(AosHandleTest, ReevaluateBlocks_Test2)
{
    // Test2: Epdg enabled, mobile block not existed.
    ClearBlocks();
    ClearHoldingBlocksForMobile();

    ClearHoldingBlocksPolicyForMobile();
    SetHoldingBlocksPolicyForMobile();

    SetEpdgEnabled(IMS_TRUE);

    EXPECT_FALSE(IsHandleBlocked(AosHandle::BLOCK_VOLTE_CAPABILITY));
    EXPECT_FALSE(IsHandleBlocked(AosHandle::BLOCK_VILTE_CAPABILITY));
    EXPECT_FALSE(IsHandleBlocked(AosHandle::BLOCK_VOPS));
    EXPECT_FALSE(IsHoldingBlockForMobile(AosHandle::BLOCK_VOLTE_CAPABILITY));
    EXPECT_FALSE(IsHoldingBlockForMobile(AosHandle::BLOCK_VILTE_CAPABILITY));
    EXPECT_FALSE(IsHoldingBlockForMobile(AosHandle::BLOCK_VOPS));

    ReevaluateBlocks();

    EXPECT_FALSE(IsHandleBlocked(AosHandle::BLOCK_VOLTE_CAPABILITY));
    EXPECT_FALSE(IsHandleBlocked(AosHandle::BLOCK_VILTE_CAPABILITY));
    EXPECT_FALSE(IsHandleBlocked(AosHandle::BLOCK_VOPS));
    EXPECT_FALSE(IsHoldingBlockForMobile(AosHandle::BLOCK_VOLTE_CAPABILITY));
    EXPECT_FALSE(IsHoldingBlockForMobile(AosHandle::BLOCK_VILTE_CAPABILITY));
    EXPECT_FALSE(IsHoldingBlockForMobile(AosHandle::BLOCK_VOPS));
}

TEST_F(AosHandleTest, ReevaluateBlocks_Test3)
{
    // Test3: Epdg enabled, Holding wifi block existed.
    ClearBlocks();
    ClearHoldingBlocksForWifi();

    ClearHoldingBlocksPolicyForWifi();
    SetHoldingBlocksPolicyForWifi();

    SetEpdgEnabled(IMS_TRUE);
    AddHoldingBlockForWifi(AosHandle::BLOCK_VOWIFI_CAPABILITY);
    AddHoldingBlockForWifi(AosHandle::BLOCK_VIWIFI_CAPABILITY);

    EXPECT_FALSE(IsHandleBlocked(AosHandle::BLOCK_VOWIFI_CAPABILITY));
    EXPECT_FALSE(IsHandleBlocked(AosHandle::BLOCK_VIWIFI_CAPABILITY));
    EXPECT_TRUE(IsHoldingBlockForWifi(AosHandle::BLOCK_VOWIFI_CAPABILITY));
    EXPECT_TRUE(IsHoldingBlockForWifi(AosHandle::BLOCK_VIWIFI_CAPABILITY));

    ReevaluateBlocks();

    EXPECT_TRUE(IsHandleBlocked(AosHandle::BLOCK_VOWIFI_CAPABILITY));
    EXPECT_TRUE(IsHandleBlocked(AosHandle::BLOCK_VIWIFI_CAPABILITY));
    EXPECT_FALSE(IsHoldingBlockForWifi(AosHandle::BLOCK_VOWIFI_CAPABILITY));
    EXPECT_FALSE(IsHoldingBlockForWifi(AosHandle::BLOCK_VIWIFI_CAPABILITY));
}

TEST_F(AosHandleTest, ReevaluateBlocks_Test4)
{
    // Test4: Epdg enabled, Holding wifi block not existed.
    ClearBlocks();
    ClearHoldingBlocksForWifi();

    ClearHoldingBlocksPolicyForWifi();
    SetHoldingBlocksPolicyForWifi();

    SetEpdgEnabled(IMS_TRUE);

    EXPECT_FALSE(IsHandleBlocked(AosHandle::BLOCK_VOWIFI_CAPABILITY));
    EXPECT_FALSE(IsHandleBlocked(AosHandle::BLOCK_VIWIFI_CAPABILITY));
    EXPECT_FALSE(IsHoldingBlockForWifi(AosHandle::BLOCK_VOWIFI_CAPABILITY));
    EXPECT_FALSE(IsHoldingBlockForWifi(AosHandle::BLOCK_VIWIFI_CAPABILITY));

    ReevaluateBlocks();

    EXPECT_FALSE(IsHandleBlocked(AosHandle::BLOCK_VOWIFI_CAPABILITY));
    EXPECT_FALSE(IsHandleBlocked(AosHandle::BLOCK_VIWIFI_CAPABILITY));
    EXPECT_FALSE(IsHoldingBlockForWifi(AosHandle::BLOCK_VOWIFI_CAPABILITY));
    EXPECT_FALSE(IsHoldingBlockForWifi(AosHandle::BLOCK_VIWIFI_CAPABILITY));
}

TEST_F(AosHandleTest, ReevaluateBlocks_Test5)
{
    // Test5: Epdg not enabled, wifi block existed
    ClearBlocks();
    ClearHoldingBlocksForWifi();

    ClearHoldingBlocksPolicyForWifi();
    SetHoldingBlocksPolicyForWifi();

    SetEpdgEnabled(IMS_FALSE);
    AddBlock(AosHandle::BLOCK_VOWIFI_CAPABILITY);
    AddBlock(AosHandle::BLOCK_VIWIFI_CAPABILITY);

    EXPECT_TRUE(IsHandleBlocked(AosHandle::BLOCK_VOWIFI_CAPABILITY));
    EXPECT_TRUE(IsHandleBlocked(AosHandle::BLOCK_VIWIFI_CAPABILITY));
    EXPECT_FALSE(IsHoldingBlockForWifi(AosHandle::BLOCK_VOWIFI_CAPABILITY));
    EXPECT_FALSE(IsHoldingBlockForWifi(AosHandle::BLOCK_VIWIFI_CAPABILITY));

    ReevaluateBlocks();

    EXPECT_FALSE(IsHandleBlocked(AosHandle::BLOCK_VOWIFI_CAPABILITY));
    EXPECT_FALSE(IsHandleBlocked(AosHandle::BLOCK_VIWIFI_CAPABILITY));
    EXPECT_TRUE(IsHoldingBlockForWifi(AosHandle::BLOCK_VOWIFI_CAPABILITY));
    EXPECT_TRUE(IsHoldingBlockForWifi(AosHandle::BLOCK_VIWIFI_CAPABILITY));
}

TEST_F(AosHandleTest, ReevaluateBlocks_Test6)
{
    // Test5: Epdg not enabled, wifi block not existed
    ClearBlocks();
    ClearHoldingBlocksForWifi();

    ClearHoldingBlocksPolicyForWifi();
    SetHoldingBlocksPolicyForWifi();

    SetEpdgEnabled(IMS_FALSE);

    EXPECT_FALSE(IsHandleBlocked(AosHandle::BLOCK_VOWIFI_CAPABILITY));
    EXPECT_FALSE(IsHandleBlocked(AosHandle::BLOCK_VIWIFI_CAPABILITY));
    EXPECT_FALSE(IsHoldingBlockForWifi(AosHandle::BLOCK_VOWIFI_CAPABILITY));
    EXPECT_FALSE(IsHoldingBlockForWifi(AosHandle::BLOCK_VIWIFI_CAPABILITY));

    ReevaluateBlocks();

    EXPECT_FALSE(IsHandleBlocked(AosHandle::BLOCK_VOWIFI_CAPABILITY));
    EXPECT_FALSE(IsHandleBlocked(AosHandle::BLOCK_VIWIFI_CAPABILITY));
    EXPECT_FALSE(IsHoldingBlockForWifi(AosHandle::BLOCK_VOWIFI_CAPABILITY));
    EXPECT_FALSE(IsHoldingBlockForWifi(AosHandle::BLOCK_VIWIFI_CAPABILITY));
}

TEST_F(AosHandleTest, ReevaluateBlocks_Test7)
{
    // Test7: Epdg not enabled, Holding mobile block existed.
    ClearBlocks();
    ClearHoldingBlocksForMobile();

    ClearHoldingBlocksPolicyForMobile();
    SetHoldingBlocksPolicyForMobile();

    SetEpdgEnabled(IMS_FALSE);
    AddHoldingBlockForMobile(AosHandle::BLOCK_VOLTE_CAPABILITY);
    AddHoldingBlockForMobile(AosHandle::BLOCK_VILTE_CAPABILITY);
    AddHoldingBlockForMobile(AosHandle::BLOCK_VOPS);

    EXPECT_FALSE(IsHandleBlocked(AosHandle::BLOCK_VOLTE_CAPABILITY));
    EXPECT_FALSE(IsHandleBlocked(AosHandle::BLOCK_VILTE_CAPABILITY));
    EXPECT_FALSE(IsHandleBlocked(AosHandle::BLOCK_VOPS));
    EXPECT_TRUE(IsHoldingBlockForMobile(AosHandle::BLOCK_VOLTE_CAPABILITY));
    EXPECT_TRUE(IsHoldingBlockForMobile(AosHandle::BLOCK_VILTE_CAPABILITY));
    EXPECT_TRUE(IsHoldingBlockForMobile(AosHandle::BLOCK_VOPS));

    ReevaluateBlocks();

    EXPECT_TRUE(IsHandleBlocked(AosHandle::BLOCK_VOLTE_CAPABILITY));
    EXPECT_TRUE(IsHandleBlocked(AosHandle::BLOCK_VILTE_CAPABILITY));
    EXPECT_TRUE(IsHandleBlocked(AosHandle::BLOCK_VOPS));
    EXPECT_FALSE(IsHoldingBlockForMobile(AosHandle::BLOCK_VOLTE_CAPABILITY));
    EXPECT_FALSE(IsHoldingBlockForMobile(AosHandle::BLOCK_VILTE_CAPABILITY));
    EXPECT_FALSE(IsHoldingBlockForMobile(AosHandle::BLOCK_VOPS));
}

TEST_F(AosHandleTest, ReevaluateBlocks_Test8)
{
    // Test8: Epdg not enabled, Holding mobile block not existed.
    ClearBlocks();
    ClearHoldingBlocksForMobile();

    ClearHoldingBlocksPolicyForMobile();
    SetHoldingBlocksPolicyForMobile();

    SetEpdgEnabled(IMS_FALSE);

    EXPECT_FALSE(IsHandleBlocked(AosHandle::BLOCK_VOLTE_CAPABILITY));
    EXPECT_FALSE(IsHandleBlocked(AosHandle::BLOCK_VILTE_CAPABILITY));
    EXPECT_FALSE(IsHandleBlocked(AosHandle::BLOCK_VOPS));
    EXPECT_FALSE(IsHoldingBlockForMobile(AosHandle::BLOCK_VOLTE_CAPABILITY));
    EXPECT_FALSE(IsHoldingBlockForMobile(AosHandle::BLOCK_VILTE_CAPABILITY));
    EXPECT_FALSE(IsHoldingBlockForMobile(AosHandle::BLOCK_VOPS));

    ReevaluateBlocks();

    EXPECT_FALSE(IsHandleBlocked(AosHandle::BLOCK_VOLTE_CAPABILITY));
    EXPECT_FALSE(IsHandleBlocked(AosHandle::BLOCK_VILTE_CAPABILITY));
    EXPECT_FALSE(IsHandleBlocked(AosHandle::BLOCK_VOPS));
    EXPECT_FALSE(IsHoldingBlockForMobile(AosHandle::BLOCK_VOLTE_CAPABILITY));
    EXPECT_FALSE(IsHoldingBlockForMobile(AosHandle::BLOCK_VILTE_CAPABILITY));
    EXPECT_FALSE(IsHoldingBlockForMobile(AosHandle::BLOCK_VOPS));
}

TEST_F(AosHandleTest, UpdateIpcan_)
{
    MockIAosConnection objMockIAosConnection;
    EXPECT_CALL(m_objMockIAosAppContext, GetConnection())
            .Times(AnyNumber())
            .WillRepeatedly(Return(static_cast<IAosConnection*>(&objMockIAosConnection)));
    EXPECT_CALL(objMockIAosConnection, IsEpdgEnabled())
            .Times(4)
            .WillOnce(Return(IMS_TRUE))
            .WillOnce(Return(IMS_TRUE))
            .WillOnce(Return(IMS_FALSE))
            .WillOnce(Return(IMS_FALSE));

    EXPECT_TRUE(UpdateIpcan());
    EXPECT_FALSE(UpdateIpcan());
    EXPECT_TRUE(UpdateIpcan());
    EXPECT_FALSE(UpdateIpcan());
}