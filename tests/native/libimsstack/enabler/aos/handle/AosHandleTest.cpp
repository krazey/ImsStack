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
#include "IIpcan.h"
#include "INetworkWatcher.h"

#include "handle/AosHandle.h"
#include "handle/AosFeatureTag.h"
#include "interface/IAosAppContext.h"
#include "interface/IAosHandle.h"
#include "interface/IAosNConfiguration.h"
#include "interface/IAosRegistration.h"
#include "interface/IAosRegStateManager.h"
#include "provider/AosProvider.h"

#include "interface/MockIAosAppContext.h"
#include "interface/MockIAosApplication.h"
#include "interface/MockIAosCallTracker.h"
#include "interface/MockIAosConnection.h"
#include "interface/MockIAosNConfiguration.h"
#include "interface/MockIAosNetTracker.h"
#include "interface/MockIAosRegistration.h"
#include "interface/MockIAosRegStateManager.h"
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

    IAosNConfiguration* m_piAosNConfiguration;
    MockIAosNConfiguration m_objMockIAosNConfiguration;

    IAosService* m_piAosService;
    MockIAosService m_objMockIAosService;

    IAosRegStateManager* m_piAosRegStateManager;
    MockIAosRegStateManager m_objMockIAosRegStateManager;

    MockIAosConnection m_objMockIAosConnection;

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

        EXPECT_CALL(m_objMockIAosAppContext, GetConnection())
                .Times(AnyNumber())
                .WillRepeatedly(Return(&m_objMockIAosConnection));

        m_piAosNConfiguration = AosProvider::GetInstance()->GetNConfiguration();
        AosProvider::GetInstance()->SetNConfiguration(
                static_cast<IAosNConfiguration*>(&m_objMockIAosNConfiguration));

        m_piAosService = AosProvider::GetInstance()->GetService();
        AosProvider::GetInstance()->SetService(static_cast<IAosService*>(&m_objMockIAosService));

        m_piAosRegStateManager = AosProvider::GetInstance()->GetRegStateManager();
        AosProvider::GetInstance()->SetRegStateManager(
                static_cast<IAosRegStateManager*>(&m_objMockIAosRegStateManager));

        EXPECT_CALL(m_objMockIAosNConfiguration, SetListener(_)).Times(1);

        m_pAosHandle = new AosHandle(static_cast<IAosAppContext*>(&m_objMockIAosAppContext),
                m_strAppId, m_strServiceId, m_nServiceType);

        ASSERT_TRUE(m_pAosHandle != nullptr);
    }

    virtual void TearDown() override
    {
        if (m_pAosHandle != nullptr)
        {
            delete m_pAosHandle;
            m_pAosHandle = nullptr;
        }

        AosProvider::GetInstance()->SetNConfiguration(m_piAosNConfiguration);
        AosProvider::GetInstance()->SetService(m_piAosService);
        AosProvider::GetInstance()->SetRegStateManager(m_piAosRegStateManager);
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

    void ClearFeatureTagList() { m_pAosHandle->m_objFeatureTagList.Clear(); }

    void ClearBindedFeatureTagList() { m_pAosHandle->m_objBindedFeatureTagList.Clear(); }

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

    IMS_BOOL GetNetSrvIn() { return m_pAosHandle->m_bNetSrvIn; }

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

    IMS_BOOL PreProcessBlock(IN IMS_UINT32 nBlock, IN IMS_BOOL bAdded)
    {
        return m_pAosHandle->PreProcessBlock(nBlock, bAdded);
    }

    void ProcessBlock(IN IMS_UINT32 nBlock, IN IMS_BOOL bAdded, IN IMS_BOOL bPreProcess)
    {
        m_pAosHandle->ProcessBlock(nBlock, bAdded, bPreProcess);
    }

    void ProcessFeatureBlock(IN IMS_UINT32 nFeature, IN IMS_BOOL bBlocked)
    {
        m_pAosHandle->ProcessFeatureBlock(nFeature, bBlocked);
    }

    IMS_BOOL ProcessCheckBlock(IN IMS_BOOL bRunStateMachine)
    {
        return m_pAosHandle->ProcessCheckBlock(bRunStateMachine);
    }

    void SetBlocked(IMS_BOOL bBlocked) { m_pAosHandle->m_bBlocked = bBlocked; }

    void ProcessUnavailableFeature(IN IMS_UINT32 nFeature, IN IMS_BOOL bAdd)
    {
        m_pAosHandle->ProcessUnavailableFeature(nFeature, bAdd);
    }

    IMS_UINT32 GetUnavailableFeatures()
    {
        return m_pAosHandle->m_objFeatureTagList.GetUnavailableFeatures();
    }

    void ProcessUnavailableFeatureChanged() { m_pAosHandle->ProcessUnavailableFeatureChanged(); }

    IMS_BOOL IsBlockForMobile(IN IMS_UINT32 nBlock) const
    {
        return m_pAosHandle->IsBlockForMobile(nBlock);
    }

    IMS_BOOL IsBlockForWifi(IN IMS_UINT32 nBlock) const
    {
        return m_pAosHandle->IsBlockForWifi(nBlock);
    }

    IMS_BOOL IsUnavailableFeature(IN IMS_SINT32 nConfigFeature) const
    {
        return m_pAosHandle->IsUnavailableFeature(nConfigFeature);
    }

    IMS_BOOL IsUnavailableFeaturePolicy(IN IMS_SINT32 nPolicy) const
    {
        return m_pAosHandle->IsUnavailableFeaturePolicy(nPolicy);
    }

    IMS_BOOL IsBlocked() const { return m_pAosHandle->m_bBlocked; }

    void InitializeFeatureTags() { m_pAosHandle->InitializeFeatureTags(); }

    IMS_BOOL HasFeatureTag(IN const AString& strName, IN const AString& strValue) const
    {
        return m_pAosHandle->m_objFeatureTagList.HasFeatureTag(strName, strValue);
    }

    void UpdateFeatureTags() { m_pAosHandle->UpdateFeatureTags(); }

    IMS_BOOL AddFeatureTag(IN const AString& strName)
    {
        return m_pAosHandle->m_objFeatureTagList.AddFeatureTag(strName);
    }

    IMS_BOOL ProcessImsSuspended(IN IMS_UINT32 nReason)
    {
        return m_pAosHandle->ProcessImsSuspended(nReason);
    }

    IMS_BOOL ProcessImsResumed(IN IMS_UINT32 nReason)
    {
        return m_pAosHandle->ProcessImsResumed(nReason);
    }

    void SetSuspendedReasonForTest(IN IMS_UINT32 nReason)
    {
        m_pAosHandle->m_nSuspendedReason = nReason;
    }

    void CheckSuspended() { m_pAosHandle->CheckSuspended(); }

    void ResetSuspendedReason(IN IMS_UINT32 nReason)
    {
        m_pAosHandle->ResetSuspendedReason(nReason);
    }

    void ReportRegState() { m_pAosHandle->ReportRegState(); }

    void ProcessCapabilitiesChanged(IN const IMSMap<IMS_UINT32, IMS_UINT32>& objNewCapabilities)
    {
        m_pAosHandle->ProcessCapabilitiesChanged(objNewCapabilities);
    }

    void ProcessNetworkChanged() { m_pAosHandle->ProcessNetworkChanged(); }

    void ProcessVopsStateChanged(IN IMS_UINT32 nState)
    {
        m_pAosHandle->ProcessVopsStateChanged(nState);
    }

    IMS_BOOL ProcessUnavailableFeatureForVops(IN IMS_UINT32 nState)
    {
        return m_pAosHandle->ProcessUnavailableFeatureForVops(nState);
    }

    IMS_BOOL IsSupportedNetworkType(IN IMS_UINT32 nType) const
    {
        return m_pAosHandle->IsSupportedNetworkType(nType);
    }

    IMS_BOOL IsSupportedNetworkTypeForCellular(IN IMS_UINT32 nType) const
    {
        return m_pAosHandle->IsSupportedNetworkTypeForCellular(nType);
    }

    const IMS_CHAR* MsgToString(IN IMS_UINT32 nMsg) { return m_pAosHandle->MsgToString(nMsg); }

    const IMS_CHAR* RadioTypeToString(IN IMS_UINT32 nType)
    {
        return m_pAosHandle->RadioTypeToString(nType);
    }

    const IMS_CHAR* ServiceTypeToString() { return m_pAosHandle->ServiceTypeToString(); }

    void SetServiceType(IN IMS_UINT32 nType) { m_pAosHandle->m_nServiceType = nType; }
};

TEST_F(AosHandleTest, Constructor)
{
    EXPECT_EQ(GetState(), AosHandle::STATE_DISCONNECTED);
}

TEST_F(AosHandleTest, Destructor)
{
    EXPECT_CALL(m_objMockIAosNConfiguration, RemoveListener(m_pAosHandle)).Times(1);
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

TEST_F(AosHandleTest, GetAppId_Test)
{
    EXPECT_STREQ(m_pAosHandle->GetAppId().GetStr(), m_strAppId.GetStr());
}

TEST_F(AosHandleTest, GetServiceId_Test)
{
    EXPECT_STREQ(m_pAosHandle->GetServiceId().GetStr(), m_strServiceId.GetStr());
}

TEST_F(AosHandleTest, GetServiceType_Test)
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

TEST_F(AosHandleTest, IsRegFeatureTagRequired_Test)
{
    IMS_BOOL bRegFeatureTagRequired = m_pAosHandle->IsRegFeatureTagRequired();

    SetRegFeatureTagRequired(IMS_TRUE);
    EXPECT_TRUE(m_pAosHandle->IsRegFeatureTagRequired());

    SetRegFeatureTagRequired(IMS_FALSE);
    EXPECT_FALSE(m_pAosHandle->IsRegFeatureTagRequired());

    SetRegFeatureTagRequired(bRegFeatureTagRequired);
}

TEST_F(AosHandleTest, GetFeatureTagList_Test)
{
    AosFeatureTagList& objFeatureTagList = m_pAosHandle->GetFeatureTagList();
    AosFeatureTagList objTestFeatureTagList;

    SetFeatureTagList(objTestFeatureTagList);
    EXPECT_TRUE(m_pAosHandle->GetFeatureTagList().Equals(objTestFeatureTagList));

    SetFeatureTagList(objFeatureTagList);
}

TEST_F(AosHandleTest, GetBindedFeatureTagList_Test)
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

    ClearFeatureTagList();
    ClearBindedFeatureTagList();
    AddFeature(ImsAosFeature::MMTEL);
    AddBindedFeature(ImsAosFeature::MMTEL);

    m_pAosHandle->ProcessFeatureTagChange();
}

TEST_F(AosHandleTest, ProcessFeatureTagChange_STATE_INVALID)
{
    EXPECT_CALL(m_objMockIAosApplication, Reconfig()).Times(0);

    ClearFeatureTagList();
    ClearBindedFeatureTagList();
    AddFeature(ImsAosFeature::MMTEL);
    SetState(AosHandle::STATE_INVALID);

    m_pAosHandle->ProcessFeatureTagChange();
}

TEST_F(AosHandleTest, ProcessFeatureTagChange_STATE_DISCONNECTED)
{
    EXPECT_CALL(m_objMockIAosApplication, Reconfig()).Times(0);

    ClearFeatureTagList();
    ClearBindedFeatureTagList();
    AddFeature(ImsAosFeature::MMTEL);
    SetState(AosHandle::STATE_DISCONNECTED);

    m_pAosHandle->ProcessFeatureTagChange();
}

TEST_F(AosHandleTest, ProcessFeatureTagChange_STATE_DISCONNECTING)
{
    EXPECT_CALL(m_objMockIAosApplication, Reconfig()).Times(0);

    ClearFeatureTagList();
    ClearBindedFeatureTagList();
    AddFeature(ImsAosFeature::MMTEL);
    SetState(AosHandle::STATE_DISCONNECTING);

    m_pAosHandle->ProcessFeatureTagChange();
}

TEST_F(AosHandleTest, ProcessFeatureTagChange_STATE_CONNECTING)
{
    EXPECT_CALL(m_objMockIAosApplication, Reconfig()).Times(1);

    ClearFeatureTagList();
    ClearBindedFeatureTagList();
    AddFeature(ImsAosFeature::MMTEL);
    SetState(AosHandle::STATE_CONNECTING);

    m_pAosHandle->ProcessFeatureTagChange();
}

TEST_F(AosHandleTest, ProcessFeatureTagChange_STATE_CONNECTED)
{
    EXPECT_CALL(m_objMockIAosApplication, Reconfig()).Times(1);

    ClearFeatureTagList();
    ClearBindedFeatureTagList();
    AddFeature(ImsAosFeature::MMTEL);
    SetState(AosHandle::STATE_CONNECTED);

    m_pAosHandle->ProcessFeatureTagChange();
}

TEST_F(AosHandleTest, App_StateChanged_Test)
{
    EXPECT_TRUE(m_pAosHandle->App_StateChanged(IAosApplication::APP_DISCONNECTED, 0));
    EXPECT_TRUE(m_pAosHandle->App_StateChanged(IAosApplication::APP_CONNECTED, 0));
    EXPECT_TRUE(m_pAosHandle->App_StateChanged(IAosApplication::APP_UPDATING, 0));
    EXPECT_TRUE(m_pAosHandle->App_StateChanged(IAosApplication::APP_DISCONNECTING, 0));
    EXPECT_FALSE(m_pAosHandle->App_StateChanged(-1, 0));
    EXPECT_FALSE(m_pAosHandle->App_StateChanged(4, 0));
}

TEST_F(AosHandleTest, SetListener_Test)
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

    EXPECT_CALL(m_objMockIAosConnection, GetIpcanCategory())
            .Times(2)
            .WillOnce(Return(IIpcan::CATEGORY_MOBILE))
            .WillOnce(Return(IIpcan::CATEGORY_WLAN));

    SetState(AosHandle::STATE_CONNECTED);
    EXPECT_CALL(m_objMockIImsAosListener, ImsAos_Connected(_, IIpcan::CATEGORY_MOBILE)).Times(1);
    EXPECT_CALL(m_objMockIImsAosListener, ImsAos_Connected(_, IIpcan::CATEGORY_WLAN)).Times(1);
    EXPECT_TRUE(m_pAosHandle->App_Notify());
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

TEST_F(AosHandleTest, Control_Test)
{
    EXPECT_CALL(m_objMockIAosApplication, RequestCmd(_, _)).Times(1);
    m_pAosHandle->Control(0);
}

TEST_F(AosHandleTest, GetAosInfo_Test)
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

TEST_F(AosHandleTest, IsFeatureConnected_Test)
{
    SetState(AosHandle::STATE_CONNECTED);
    AddBindedFeature(ImsAosFeature::MMTEL);

    EXPECT_TRUE(m_pAosHandle->IsFeatureConnected(ImsAosFeature::MMTEL));
    EXPECT_FALSE(m_pAosHandle->IsFeatureConnected(ImsAosFeature::VIDEO));
}

TEST_F(AosHandleTest, IsImsConnected_Test)
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

TEST_F(AosHandleTest, IsImsSuspended_Test)
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

TEST_F(AosHandleTest, NetTracker_StatusChanged_Test)
{
    SetState(AosHandle::STATE_CONNECTED);

    EXPECT_CALL(m_objMockIAosNetTracker, IsSuspended())
            .Times(2)
            .WillOnce(Return(IMS_FALSE))
            .WillOnce(Return(IMS_TRUE));

    EXPECT_CALL(m_objMockIAosConnection, IsEpdgEnabled())
            .Times(AnyNumber())
            .WillRepeatedly(Return(IMS_TRUE));

    m_pAosHandle->NetTracker_StatusChanged();
    EXPECT_FALSE(m_pAosHandle->IsImsSuspended());

    m_pAosHandle->NetTracker_StatusChanged();
    EXPECT_TRUE(m_pAosHandle->IsImsSuspended());
}

TEST_F(AosHandleTest, Init_CleanUp)
{
    EXPECT_CALL(m_objMockIAosNetTracker, SetListener(_)).Times(1);
    EXPECT_CALL(m_objMockIAosNetTracker, RemoveListener(_)).Times(1);

    EXPECT_CALL(m_objMockIAosService,
            AddListener(DYNAMIC_CAST(IAosRegistrationControlListener*, m_pAosHandle)))
            .Times(1);
    EXPECT_CALL(m_objMockIAosService,
            AddListener(DYNAMIC_CAST(IAosServicePhoneListener*, m_pAosHandle)))
            .Times(1);
    EXPECT_CALL(m_objMockIAosService,
            AddListener(DYNAMIC_CAST(IAosServiceSettingListener*, m_pAosHandle)))
            .Times(1);
    EXPECT_CALL(m_objMockIAosService,
            RemoveListener(DYNAMIC_CAST(IAosRegistrationControlListener*, m_pAosHandle)))
            .Times(1);
    EXPECT_CALL(m_objMockIAosService,
            RemoveListener(DYNAMIC_CAST(IAosServicePhoneListener*, m_pAosHandle)))
            .Times(1);
    EXPECT_CALL(m_objMockIAosService,
            RemoveListener(DYNAMIC_CAST(IAosServiceSettingListener*, m_pAosHandle)))
            .Times(1);

    EXPECT_TRUE(GetAosInfo() == nullptr);

    EXPECT_CALL(m_objMockIAosNConfiguration, IsCdmalessFeatureTagRequired())
            .Times(1)
            .WillRepeatedly(Return(IMS_TRUE));

    Init();

    EXPECT_TRUE(GetAosInfo() != nullptr);

    CleanUp();

    EXPECT_TRUE(GetAosInfo() == nullptr);
}

TEST_F(AosHandleTest, SetHandleState_Test)
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

TEST_F(AosHandleTest, SetReason_Test)
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

TEST_F(AosHandleTest, ClearSuspendedReason_Test)
{
    SetSuspendedReason(AoSReason::SUSPEND_NO_SERVICE);
    EXPECT_TRUE(m_pAosHandle->IsImsSuspended());

    ClearSuspendedReason();
    EXPECT_FALSE(m_pAosHandle->IsImsSuspended());
}

TEST_F(AosHandleTest, GetAppState_Test)
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

TEST_F(AosHandleTest, GetImsAosReason_Test)
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

TEST_F(AosHandleTest, GetImsAosReasonForSuspend_Test)
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

TEST_F(AosHandleTest, IsEpdgEnabled_Test)
{
    SetEpdgEnabled(IMS_TRUE);
    EXPECT_TRUE(IsEpdgEnabled());

    SetEpdgEnabled(IMS_FALSE);
    EXPECT_FALSE(IsEpdgEnabled());
}

TEST_F(AosHandleTest, IsEqualNetworkType_Test)
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

TEST_F(AosHandleTest, IsCapabilityExisted_Test)
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

TEST_F(AosHandleTest, IsNetworkTypeMatchedToRat_Test)
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

TEST_F(AosHandleTest, IsServiceFeature_Test)
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

TEST_F(AosHandleTest, GetNetworkType_Test)
{
    EXPECT_CALL(m_objMockIAosNetTracker, GetNetworkType()).Times(1);
    GetNetworkType();
}

TEST_F(AosHandleTest, GetMobileNetworkType_Test)
{
    EXPECT_CALL(m_objMockIAosNetTracker, GetMobileNetworkType()).Times(1);
    GetMobileNetworkType();
}

TEST_F(AosHandleTest, GetBlock_Test)
{
    EXPECT_EQ(GetBlock(IMS_EVENT_IMS_VOICE_OVER_PS_STATE), AosHandle::BLOCK_VOPS);

    EXPECT_EQ(GetBlock(IMS_EVENT_ROAMING_STATE), AosHandle::BLOCK_NONE);
    EXPECT_EQ(GetBlock(IMS_EVENT_VOLTE_SETTING), AosHandle::BLOCK_NONE);
    EXPECT_EQ(GetBlock(IMS_EVENT_VIDEO_SETTING), AosHandle::BLOCK_NONE);
    EXPECT_EQ(GetBlock(IMS_EVENT_WFC_SETTING_CHANGED), AosHandle::BLOCK_NONE);
    EXPECT_EQ(GetBlock(IMS_EVENT_MOBILE_DATA_SETTING), AosHandle::BLOCK_NONE);
}

TEST_F(AosHandleTest, GetAosFeature_Test)
{
    EXPECT_EQ(GetAosFeature(AosHandle::BLOCK_VOLTE_CAPABILITY), ImsAosFeature::MMTEL);
    EXPECT_EQ(GetAosFeature(AosHandle::BLOCK_VOWIFI_CAPABILITY), ImsAosFeature::MMTEL);
    EXPECT_EQ(GetAosFeature(AosHandle::BLOCK_VOPS), ImsAosFeature::MMTEL);

    EXPECT_EQ(GetAosFeature(AosHandle::BLOCK_VILTE_CAPABILITY), ImsAosFeature::VIDEO);
    EXPECT_EQ(GetAosFeature(AosHandle::BLOCK_VIWIFI_CAPABILITY), ImsAosFeature::VIDEO);

    EXPECT_EQ(GetAosFeature(AosHandle::BLOCK_SMS_CAPABILITY), ImsAosFeature::SMSIP);
    EXPECT_EQ(GetAosFeature(AosHandle::BLOCK_SMS_OVER_IP_NETWORK_INDICATION), ImsAosFeature::SMSIP);

    EXPECT_EQ(GetAosFeature(AosHandle::BLOCK_NONE), ImsAosFeature::NONE);
}

TEST_F(AosHandleTest, ConvertToAosFeature_Test)
{
    EXPECT_EQ(ConvertToAosFeature(CarrierConfig::Assets::UNAVAILABLE_FEATURE_TYPE_MMTEL),
            ImsAosFeature::MMTEL);
    EXPECT_EQ(ConvertToAosFeature(CarrierConfig::Assets::UNAVAILABLE_FEATURE_TYPE_VIDEO),
            ImsAosFeature::VIDEO);
    EXPECT_EQ(ConvertToAosFeature(CarrierConfig::Assets::UNAVAILABLE_FEATURE_TYPE_SMS),
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

    ClearBlocks();
    ClearHoldingBlocksForMobile();
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

    ClearBlocks();
    ClearHoldingBlocksForMobile();
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

    ClearBlocks();
    ClearHoldingBlocksForWifi();
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

    ClearBlocks();
    ClearHoldingBlocksForWifi();
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

    ClearBlocks();
    ClearHoldingBlocksForWifi();
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

    ClearBlocks();
    ClearHoldingBlocksForWifi();
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

    ClearBlocks();
    ClearHoldingBlocksForMobile();
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

    ClearBlocks();
    ClearHoldingBlocksForMobile();
}

TEST_F(AosHandleTest, UpdateIpcan_Test)
{
    EXPECT_CALL(m_objMockIAosConnection, IsEpdgEnabled())
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

TEST_F(AosHandleTest, PreProcessBlock_Test1)
{
    // Test1: Epdg enabled, Block for wifi
    // Expectation: The block should not be held/released for mobile

    ClearHoldingBlocksForMobile();
    ClearHoldingBlocksPolicyForMobile();
    SetHoldingBlocksPolicyForMobile();

    SetEpdgEnabled(IMS_TRUE);

    EXPECT_FALSE(IsHoldingBlockForMobile(AosHandle::BLOCK_VOWIFI_CAPABILITY));
    EXPECT_FALSE(IsHoldingBlockForMobile(AosHandle::BLOCK_VIWIFI_CAPABILITY));

    EXPECT_FALSE(PreProcessBlock(AosHandle::BLOCK_VOWIFI_CAPABILITY, IMS_TRUE));
    EXPECT_FALSE(PreProcessBlock(AosHandle::BLOCK_VIWIFI_CAPABILITY, IMS_TRUE));

    EXPECT_FALSE(IsHoldingBlockForMobile(AosHandle::BLOCK_VOWIFI_CAPABILITY));
    EXPECT_FALSE(IsHoldingBlockForMobile(AosHandle::BLOCK_VIWIFI_CAPABILITY));

    EXPECT_FALSE(PreProcessBlock(AosHandle::BLOCK_VOWIFI_CAPABILITY, IMS_FALSE));
    EXPECT_FALSE(PreProcessBlock(AosHandle::BLOCK_VIWIFI_CAPABILITY, IMS_FALSE));

    EXPECT_FALSE(IsHoldingBlockForMobile(AosHandle::BLOCK_VOWIFI_CAPABILITY));
    EXPECT_FALSE(IsHoldingBlockForMobile(AosHandle::BLOCK_VIWIFI_CAPABILITY));

    ClearHoldingBlocksForMobile();
    ClearHoldingBlocksPolicyForMobile();
}

TEST_F(AosHandleTest, PreProcessBlock_Test2)
{
    // Test2: Epdg enabled, No block for wifi
    // Expectation: The block should be held/released for mobile

    ClearHoldingBlocksForMobile();
    ClearHoldingBlocksPolicyForMobile();
    SetHoldingBlocksPolicyForMobile();

    SetEpdgEnabled(IMS_TRUE);

    EXPECT_FALSE(IsHoldingBlockForMobile(AosHandle::BLOCK_VOLTE_CAPABILITY));
    EXPECT_FALSE(IsHoldingBlockForMobile(AosHandle::BLOCK_VILTE_CAPABILITY));
    EXPECT_FALSE(IsHoldingBlockForMobile(AosHandle::BLOCK_VOPS));

    EXPECT_TRUE(PreProcessBlock(AosHandle::BLOCK_VOLTE_CAPABILITY, IMS_TRUE));
    EXPECT_TRUE(PreProcessBlock(AosHandle::BLOCK_VILTE_CAPABILITY, IMS_TRUE));
    EXPECT_TRUE(PreProcessBlock(AosHandle::BLOCK_VOPS, IMS_TRUE));

    EXPECT_TRUE(IsHoldingBlockForMobile(AosHandle::BLOCK_VOLTE_CAPABILITY));
    EXPECT_TRUE(IsHoldingBlockForMobile(AosHandle::BLOCK_VILTE_CAPABILITY));
    EXPECT_TRUE(IsHoldingBlockForMobile(AosHandle::BLOCK_VOPS));

    EXPECT_TRUE(PreProcessBlock(AosHandle::BLOCK_VOLTE_CAPABILITY, IMS_FALSE));
    EXPECT_TRUE(PreProcessBlock(AosHandle::BLOCK_VILTE_CAPABILITY, IMS_FALSE));
    EXPECT_TRUE(PreProcessBlock(AosHandle::BLOCK_VOPS, IMS_FALSE));

    EXPECT_FALSE(IsHoldingBlockForMobile(AosHandle::BLOCK_VOLTE_CAPABILITY));
    EXPECT_FALSE(IsHoldingBlockForMobile(AosHandle::BLOCK_VILTE_CAPABILITY));
    EXPECT_FALSE(IsHoldingBlockForMobile(AosHandle::BLOCK_VOPS));

    ClearHoldingBlocksForMobile();
    ClearHoldingBlocksPolicyForMobile();
}

TEST_F(AosHandleTest, PreProcessBlock_Test3)
{
    // Test3: Epdg not enabled, Block for mobile
    // Expectation: The block should not be held/released for wifi

    ClearHoldingBlocksForWifi();
    ClearHoldingBlocksPolicyForWifi();
    SetHoldingBlocksPolicyForWifi();

    SetEpdgEnabled(IMS_FALSE);

    EXPECT_FALSE(IsHoldingBlockForWifi(AosHandle::BLOCK_VOLTE_CAPABILITY));
    EXPECT_FALSE(IsHoldingBlockForWifi(AosHandle::BLOCK_VILTE_CAPABILITY));
    EXPECT_FALSE(IsHoldingBlockForWifi(AosHandle::BLOCK_VOPS));

    EXPECT_FALSE(PreProcessBlock(AosHandle::BLOCK_VOLTE_CAPABILITY, IMS_TRUE));
    EXPECT_FALSE(PreProcessBlock(AosHandle::BLOCK_VILTE_CAPABILITY, IMS_TRUE));
    EXPECT_FALSE(PreProcessBlock(AosHandle::BLOCK_VOPS, IMS_TRUE));

    EXPECT_FALSE(IsHoldingBlockForWifi(AosHandle::BLOCK_VOLTE_CAPABILITY));
    EXPECT_FALSE(IsHoldingBlockForWifi(AosHandle::BLOCK_VILTE_CAPABILITY));
    EXPECT_FALSE(IsHoldingBlockForWifi(AosHandle::BLOCK_VOPS));

    EXPECT_FALSE(PreProcessBlock(AosHandle::BLOCK_VOLTE_CAPABILITY, IMS_FALSE));
    EXPECT_FALSE(PreProcessBlock(AosHandle::BLOCK_VILTE_CAPABILITY, IMS_FALSE));
    EXPECT_FALSE(PreProcessBlock(AosHandle::BLOCK_VOPS, IMS_FALSE));

    EXPECT_FALSE(IsHoldingBlockForWifi(AosHandle::BLOCK_VOLTE_CAPABILITY));
    EXPECT_FALSE(IsHoldingBlockForWifi(AosHandle::BLOCK_VILTE_CAPABILITY));
    EXPECT_FALSE(IsHoldingBlockForWifi(AosHandle::BLOCK_VOPS));

    ClearHoldingBlocksForWifi();
    ClearHoldingBlocksPolicyForWifi();
}

TEST_F(AosHandleTest, PreProcessBlock_Test4)
{
    // Test4: Epdg not enabled, No block for mobile
    // Expectation: The block should be held/released for wifi

    ClearHoldingBlocksForWifi();
    ClearHoldingBlocksPolicyForWifi();
    SetHoldingBlocksPolicyForWifi();

    SetEpdgEnabled(IMS_FALSE);

    EXPECT_FALSE(IsHoldingBlockForWifi(AosHandle::BLOCK_VOWIFI_CAPABILITY));
    EXPECT_FALSE(IsHoldingBlockForWifi(AosHandle::BLOCK_VIWIFI_CAPABILITY));

    EXPECT_TRUE(PreProcessBlock(AosHandle::BLOCK_VOWIFI_CAPABILITY, IMS_TRUE));
    EXPECT_TRUE(PreProcessBlock(AosHandle::BLOCK_VIWIFI_CAPABILITY, IMS_TRUE));

    EXPECT_TRUE(IsHoldingBlockForWifi(AosHandle::BLOCK_VOWIFI_CAPABILITY));
    EXPECT_TRUE(IsHoldingBlockForWifi(AosHandle::BLOCK_VIWIFI_CAPABILITY));

    EXPECT_TRUE(PreProcessBlock(AosHandle::BLOCK_VOWIFI_CAPABILITY, IMS_FALSE));
    EXPECT_TRUE(PreProcessBlock(AosHandle::BLOCK_VIWIFI_CAPABILITY, IMS_FALSE));

    EXPECT_FALSE(IsHoldingBlockForWifi(AosHandle::BLOCK_VOWIFI_CAPABILITY));
    EXPECT_FALSE(IsHoldingBlockForWifi(AosHandle::BLOCK_VIWIFI_CAPABILITY));

    ClearHoldingBlocksForWifi();
    ClearHoldingBlocksPolicyForWifi();
}

TEST_F(AosHandleTest, ProcessBlock_Test1)
{
    // Test1: No PreProcess
    // Expectation: Set/reset the block and the matched feature

    // Initialization
    ClearBlocks();
    ClearFeatureTagList();

    AddFeature(ImsAosFeature::MMTEL);
    AddFeature(ImsAosFeature::VIDEO);
    AddFeature(ImsAosFeature::SMSIP);

    EXPECT_FALSE(IsHandleBlocked());
    EXPECT_EQ(m_pAosHandle->GetFeatureTagList().GetFeatures(),
            (ImsAosFeature::MMTEL | ImsAosFeature::VIDEO | ImsAosFeature::SMSIP));

    // Set
    ProcessBlock(AosHandle::BLOCK_VOLTE_CAPABILITY, IMS_TRUE, IMS_FALSE);
    ProcessBlock(AosHandle::BLOCK_VOWIFI_CAPABILITY, IMS_TRUE, IMS_FALSE);
    ProcessBlock(AosHandle::BLOCK_VOPS, IMS_TRUE, IMS_FALSE);

    EXPECT_TRUE(IsHandleBlocked(AosHandle::BLOCK_VOLTE_CAPABILITY));
    EXPECT_TRUE(IsHandleBlocked(AosHandle::BLOCK_VOWIFI_CAPABILITY));
    EXPECT_TRUE(IsHandleBlocked(AosHandle::BLOCK_VOPS));
    EXPECT_EQ(m_pAosHandle->GetFeatureTagList().GetFeatures(),
            (ImsAosFeature::VIDEO | ImsAosFeature::SMSIP));

    ProcessBlock(AosHandle::BLOCK_VILTE_CAPABILITY, IMS_TRUE, IMS_FALSE);
    ProcessBlock(AosHandle::BLOCK_VIWIFI_CAPABILITY, IMS_TRUE, IMS_FALSE);

    EXPECT_TRUE(IsHandleBlocked(AosHandle::BLOCK_VILTE_CAPABILITY));
    EXPECT_TRUE(IsHandleBlocked(AosHandle::BLOCK_VIWIFI_CAPABILITY));
    EXPECT_EQ(m_pAosHandle->GetFeatureTagList().GetFeatures(), ImsAosFeature::SMSIP);

    ProcessBlock(AosHandle::BLOCK_SMS_CAPABILITY, IMS_TRUE, IMS_FALSE);
    ProcessBlock(AosHandle::BLOCK_SMS_OVER_IP_NETWORK_INDICATION, IMS_TRUE, IMS_FALSE);

    EXPECT_TRUE(IsHandleBlocked(AosHandle::BLOCK_SMS_CAPABILITY));
    EXPECT_TRUE(IsHandleBlocked(AosHandle::BLOCK_SMS_OVER_IP_NETWORK_INDICATION));
    EXPECT_EQ(m_pAosHandle->GetFeatureTagList().GetFeatures(), ImsAosFeature::NONE);

    // Reset
    ProcessBlock(AosHandle::BLOCK_VOLTE_CAPABILITY, IMS_FALSE, IMS_FALSE);
    ProcessBlock(AosHandle::BLOCK_VOWIFI_CAPABILITY, IMS_FALSE, IMS_FALSE);
    ProcessBlock(AosHandle::BLOCK_VOPS, IMS_FALSE, IMS_FALSE);

    EXPECT_FALSE(IsHandleBlocked(AosHandle::BLOCK_VOLTE_CAPABILITY));
    EXPECT_FALSE(IsHandleBlocked(AosHandle::BLOCK_VOWIFI_CAPABILITY));
    EXPECT_FALSE(IsHandleBlocked(AosHandle::BLOCK_VOPS));
    EXPECT_EQ(m_pAosHandle->GetFeatureTagList().GetFeatures(), ImsAosFeature::MMTEL);

    ProcessBlock(AosHandle::BLOCK_VILTE_CAPABILITY, IMS_FALSE, IMS_FALSE);
    ProcessBlock(AosHandle::BLOCK_VIWIFI_CAPABILITY, IMS_FALSE, IMS_FALSE);

    EXPECT_FALSE(IsHandleBlocked(AosHandle::BLOCK_VILTE_CAPABILITY));
    EXPECT_FALSE(IsHandleBlocked(AosHandle::BLOCK_VIWIFI_CAPABILITY));
    EXPECT_EQ(m_pAosHandle->GetFeatureTagList().GetFeatures(),
            (ImsAosFeature::MMTEL | ImsAosFeature::VIDEO));

    ProcessBlock(AosHandle::BLOCK_SMS_CAPABILITY, IMS_FALSE, IMS_FALSE);
    ProcessBlock(AosHandle::BLOCK_SMS_OVER_IP_NETWORK_INDICATION, IMS_FALSE, IMS_FALSE);

    EXPECT_FALSE(IsHandleBlocked(AosHandle::BLOCK_SMS_CAPABILITY));
    EXPECT_FALSE(IsHandleBlocked(AosHandle::BLOCK_SMS_OVER_IP_NETWORK_INDICATION));
    EXPECT_EQ(m_pAosHandle->GetFeatureTagList().GetFeatures(),
            (ImsAosFeature::MMTEL | ImsAosFeature::VIDEO | ImsAosFeature::SMSIP));

    EXPECT_FALSE(IsHandleBlocked());

    ClearBlocks();
    ClearFeatureTagList();
}

/*
TEST_F(AosHandleTest, ProcessBlock_Test2)
{
    // Test2: PreProcess / Wfc not available
    // Expectation: Set/reset the block and the matched feature

    // Initialization
    ClearBlocks();
    ClearFeatureTagList();

    AddFeature(ImsAosFeature::MMTEL);
    AddFeature(ImsAosFeature::VIDEO);
    AddFeature(ImsAosFeature::SMSIP);

    EXPECT_CALL(m_objMockIAosNConfiguration, IsWfcImsAvailable())
            .Times(AnyNumber())
            .WillRepeatedly(Return(IMS_FALSE));

    EXPECT_FALSE(IsHandleBlocked());
    EXPECT_EQ(m_pAosHandle->GetFeatureTagList().GetFeatures(),
            (ImsAosFeature::MMTEL | ImsAosFeature::VIDEO | ImsAosFeature::SMSIP));

    // Set
    ProcessBlock(AosHandle::BLOCK_VOLTE_CAPABILITY, IMS_TRUE, IMS_TRUE);
    ProcessBlock(AosHandle::BLOCK_VOWIFI_CAPABILITY, IMS_TRUE, IMS_TRUE);
    ProcessBlock(AosHandle::BLOCK_VOPS, IMS_TRUE, IMS_TRUE);

    EXPECT_TRUE(IsHandleBlocked(AosHandle::BLOCK_VOLTE_CAPABILITY));
    EXPECT_TRUE(IsHandleBlocked(AosHandle::BLOCK_VOWIFI_CAPABILITY));
    EXPECT_TRUE(IsHandleBlocked(AosHandle::BLOCK_VOPS));
    EXPECT_EQ(m_pAosHandle->GetFeatureTagList().GetFeatures(),
            (ImsAosFeature::VIDEO | ImsAosFeature::SMSIP));

    ProcessBlock(AosHandle::BLOCK_VILTE_CAPABILITY, IMS_TRUE, IMS_TRUE);
    ProcessBlock(AosHandle::BLOCK_VIWIFI_CAPABILITY, IMS_TRUE, IMS_TRUE);

    EXPECT_TRUE(IsHandleBlocked(AosHandle::BLOCK_VILTE_CAPABILITY));
    EXPECT_TRUE(IsHandleBlocked(AosHandle::BLOCK_VIWIFI_CAPABILITY));
    EXPECT_EQ(m_pAosHandle->GetFeatureTagList().GetFeatures(), ImsAosFeature::SMSIP);

    ProcessBlock(AosHandle::BLOCK_SMS_CAPABILITY, IMS_TRUE, IMS_TRUE);
    ProcessBlock(AosHandle::BLOCK_SMS_OVER_IP_NETWORK_INDICATION, IMS_TRUE, IMS_TRUE);

    EXPECT_TRUE(IsHandleBlocked(AosHandle::BLOCK_SMS_CAPABILITY));
    EXPECT_TRUE(IsHandleBlocked(AosHandle::BLOCK_SMS_OVER_IP_NETWORK_INDICATION));
    EXPECT_EQ(m_pAosHandle->GetFeatureTagList().GetFeatures(), ImsAosFeature::NONE);

    // Reset
    ProcessBlock(AosHandle::BLOCK_VOLTE_CAPABILITY, IMS_FALSE, IMS_TRUE);
    ProcessBlock(AosHandle::BLOCK_VOWIFI_CAPABILITY, IMS_FALSE, IMS_TRUE);
    ProcessBlock(AosHandle::BLOCK_VOPS, IMS_FALSE, IMS_TRUE);

    EXPECT_FALSE(IsHandleBlocked(AosHandle::BLOCK_VOLTE_CAPABILITY));
    EXPECT_FALSE(IsHandleBlocked(AosHandle::BLOCK_VOWIFI_CAPABILITY));
    EXPECT_FALSE(IsHandleBlocked(AosHandle::BLOCK_VOPS));
    EXPECT_EQ(m_pAosHandle->GetFeatureTagList().GetFeatures(), ImsAosFeature::MMTEL);

    ProcessBlock(AosHandle::BLOCK_VILTE_CAPABILITY, IMS_FALSE, IMS_TRUE);
    ProcessBlock(AosHandle::BLOCK_VIWIFI_CAPABILITY, IMS_FALSE, IMS_TRUE);

    EXPECT_FALSE(IsHandleBlocked(AosHandle::BLOCK_VILTE_CAPABILITY));
    EXPECT_FALSE(IsHandleBlocked(AosHandle::BLOCK_VIWIFI_CAPABILITY));
    EXPECT_EQ(m_pAosHandle->GetFeatureTagList().GetFeatures(),
            (ImsAosFeature::MMTEL | ImsAosFeature::VIDEO));

    ProcessBlock(AosHandle::BLOCK_SMS_CAPABILITY, IMS_FALSE, IMS_TRUE);
    ProcessBlock(AosHandle::BLOCK_SMS_OVER_IP_NETWORK_INDICATION, IMS_FALSE, IMS_TRUE);

    EXPECT_FALSE(IsHandleBlocked(AosHandle::BLOCK_SMS_CAPABILITY));
    EXPECT_FALSE(IsHandleBlocked(AosHandle::BLOCK_SMS_OVER_IP_NETWORK_INDICATION));
    EXPECT_EQ(m_pAosHandle->GetFeatureTagList().GetFeatures(),
            (ImsAosFeature::MMTEL | ImsAosFeature::VIDEO | ImsAosFeature::SMSIP));

    EXPECT_FALSE(IsHandleBlocked());

    ClearBlocks();
    ClearFeatureTagList();
}

TEST_F(AosHandleTest, ProcessBlock_Test3)
{
    // Test3: PreProcess / Wfc available / PreProcessBlock result false
    // Expectation: Set/reset the block and the matched feature

    // Initialization
    ClearBlocks();
    ClearFeatureTagList();

    AddFeature(ImsAosFeature::MMTEL);
    AddFeature(ImsAosFeature::VIDEO);
    AddFeature(ImsAosFeature::SMSIP);

    EXPECT_CALL(m_objMockIAosNConfiguration, IsWfcImsAvailable())
            .Times(AnyNumber())
            .WillRepeatedly(Return(IMS_TRUE));

    EXPECT_FALSE(IsHandleBlocked());
    EXPECT_EQ(m_pAosHandle->GetFeatureTagList().GetFeatures(),
            (ImsAosFeature::MMTEL | ImsAosFeature::VIDEO | ImsAosFeature::SMSIP));

    SetEpdgEnabled(IMS_FALSE);

    // Set
    ProcessBlock(AosHandle::BLOCK_VOLTE_CAPABILITY, IMS_TRUE, IMS_TRUE);
    ProcessBlock(AosHandle::BLOCK_VOPS, IMS_TRUE, IMS_TRUE);

    EXPECT_TRUE(IsHandleBlocked(AosHandle::BLOCK_VOLTE_CAPABILITY));
    EXPECT_TRUE(IsHandleBlocked(AosHandle::BLOCK_VOPS));
    EXPECT_EQ(m_pAosHandle->GetFeatureTagList().GetFeatures(),
            (ImsAosFeature::VIDEO | ImsAosFeature::SMSIP));

    ProcessBlock(AosHandle::BLOCK_VILTE_CAPABILITY, IMS_TRUE, IMS_TRUE);

    EXPECT_TRUE(IsHandleBlocked(AosHandle::BLOCK_VILTE_CAPABILITY));
    EXPECT_EQ(m_pAosHandle->GetFeatureTagList().GetFeatures(), ImsAosFeature::SMSIP);

    ProcessBlock(AosHandle::BLOCK_SMS_CAPABILITY, IMS_TRUE, IMS_TRUE);
    ProcessBlock(AosHandle::BLOCK_SMS_OVER_IP_NETWORK_INDICATION, IMS_TRUE, IMS_TRUE);

    EXPECT_TRUE(IsHandleBlocked(AosHandle::BLOCK_SMS_CAPABILITY));
    EXPECT_TRUE(IsHandleBlocked(AosHandle::BLOCK_SMS_OVER_IP_NETWORK_INDICATION));
    EXPECT_EQ(m_pAosHandle->GetFeatureTagList().GetFeatures(), ImsAosFeature::NONE);

    // Reset
    ProcessBlock(AosHandle::BLOCK_VOLTE_CAPABILITY, IMS_FALSE, IMS_TRUE);
    ProcessBlock(AosHandle::BLOCK_VOPS, IMS_FALSE, IMS_TRUE);

    EXPECT_FALSE(IsHandleBlocked(AosHandle::BLOCK_VOLTE_CAPABILITY));
    EXPECT_FALSE(IsHandleBlocked(AosHandle::BLOCK_VOPS));
    EXPECT_EQ(m_pAosHandle->GetFeatureTagList().GetFeatures(), ImsAosFeature::MMTEL);

    ProcessBlock(AosHandle::BLOCK_VILTE_CAPABILITY, IMS_FALSE, IMS_TRUE);

    EXPECT_FALSE(IsHandleBlocked(AosHandle::BLOCK_VILTE_CAPABILITY));
    EXPECT_EQ(m_pAosHandle->GetFeatureTagList().GetFeatures(),
            (ImsAosFeature::MMTEL | ImsAosFeature::VIDEO));

    ProcessBlock(AosHandle::BLOCK_SMS_CAPABILITY, IMS_FALSE, IMS_TRUE);
    ProcessBlock(AosHandle::BLOCK_SMS_OVER_IP_NETWORK_INDICATION, IMS_FALSE, IMS_TRUE);

    EXPECT_FALSE(IsHandleBlocked(AosHandle::BLOCK_SMS_CAPABILITY));
    EXPECT_FALSE(IsHandleBlocked(AosHandle::BLOCK_SMS_OVER_IP_NETWORK_INDICATION));
    EXPECT_EQ(m_pAosHandle->GetFeatureTagList().GetFeatures(),
            (ImsAosFeature::MMTEL | ImsAosFeature::VIDEO | ImsAosFeature::SMSIP));

    EXPECT_FALSE(IsHandleBlocked());

    ClearBlocks();
    ClearFeatureTagList();
}
*/

TEST_F(AosHandleTest, ProcessBlock_Test4)
{
    // Test3: PreProcess / Wfc available / PreProcessBlock result true
    // Expectation: No change of the blocks and the feature

    // Initialization
    ClearBlocks();
    ClearFeatureTagList();
    ClearHoldingBlocksPolicyForMobile();
    SetHoldingBlocksPolicyForMobile();

    AddFeature(ImsAosFeature::MMTEL);
    AddFeature(ImsAosFeature::VIDEO);

    EXPECT_CALL(m_objMockIAosNConfiguration, IsWfcImsAvailable())
            .Times(AnyNumber())
            .WillRepeatedly(Return(IMS_TRUE));

    EXPECT_FALSE(IsHandleBlocked());
    EXPECT_EQ(m_pAosHandle->GetFeatureTagList().GetFeatures(),
            (ImsAosFeature::MMTEL | ImsAosFeature::VIDEO));

    SetEpdgEnabled(IMS_TRUE);

    // Set
    ProcessBlock(AosHandle::BLOCK_VOLTE_CAPABILITY, IMS_TRUE, IMS_TRUE);
    ProcessBlock(AosHandle::BLOCK_VILTE_CAPABILITY, IMS_TRUE, IMS_TRUE);
    ProcessBlock(AosHandle::BLOCK_VOPS, IMS_TRUE, IMS_TRUE);

    EXPECT_FALSE(IsHandleBlocked());
    EXPECT_EQ(m_pAosHandle->GetFeatureTagList().GetFeatures(),
            (ImsAosFeature::MMTEL | ImsAosFeature::VIDEO));

    // Clean up
    ClearBlocks();
    ClearFeatureTagList();
    ClearHoldingBlocksForMobile();
    ClearHoldingBlocksPolicyForMobile();
    SetEpdgEnabled(IMS_FALSE);
}

TEST_F(AosHandleTest, ProcessFeatureBlock_Test)
{
    // Expectation: Add/Remove the feature

    ClearFeatureTagList();

    EXPECT_EQ(m_pAosHandle->GetFeatureTagList().GetFeatures(), ImsAosFeature::NONE);

    ProcessFeatureBlock(ImsAosFeature::MMTEL, IMS_FALSE);
    EXPECT_EQ(m_pAosHandle->GetFeatureTagList().GetFeatures(), ImsAosFeature::MMTEL);

    ProcessFeatureBlock(ImsAosFeature::MMTEL, IMS_TRUE);
    EXPECT_EQ(m_pAosHandle->GetFeatureTagList().GetFeatures(), ImsAosFeature::NONE);

    ClearFeatureTagList();
}

TEST_F(AosHandleTest, ProcessCheckBlock_Test1)
{
    // Test1: Block not changed, blocked.
    // Expectation: Do nothing. return false

    AddBlock(AosHandle::BLOCK_VOPS);
    SetBlocked(IMS_TRUE);

    EXPECT_FALSE(ProcessCheckBlock(IMS_TRUE));
}

TEST_F(AosHandleTest, ProcessCheckBlock_Test2)
{
    // Test2: Block not changed, not blocked. state connected.
    // Expectation: Call ProcessFeatureChange, return true

    ClearBlocks();
    ClearFeatureTagList();
    SetBlocked(IMS_FALSE);
    SetHandleState(AosHandle::STATE_CONNECTED);
    AddFeature(ImsAosFeature::MMTEL);

    EXPECT_CALL(m_objMockIAosApplication, Reconfig()).Times(1);
    EXPECT_TRUE(ProcessCheckBlock(IMS_TRUE));

    ClearFeatureTagList();
}

TEST_F(AosHandleTest, ProcessCheckBlock_Test3)
{
    // Test3: Block changed to blocked. state connected.
    // Expectation: Call StateConnected via state machine, return true

    AddBlock(AosHandle::BLOCK_VOPS);
    SetBlocked(IMS_FALSE);
    SetHandleState(AosHandle::STATE_CONNECTED);

    EXPECT_CALL(m_objMockIAosApplication, Reconfig()).Times(1);
    EXPECT_TRUE(ProcessCheckBlock(IMS_TRUE));
    EXPECT_EQ(GetState(), AosHandle::STATE_DISCONNECTING);

    ClearBlocks();
    SetHandleState(AosHandle::STATE_DISCONNECTED);
}

TEST_F(AosHandleTest, ProcessCheckBlock_Test4)
{
    // Test4: Block changed to not blocked. state disconnected.
    // Expectation: Call StateDisconnected via state machine, return true

    ClearBlocks();
    SetBlocked(IMS_TRUE);
    SetHandleState(AosHandle::STATE_DISCONNECTED);

    EXPECT_CALL(m_objMockIAosApplication, Reconfig()).Times(1);
    EXPECT_TRUE(ProcessCheckBlock(IMS_TRUE));
    EXPECT_EQ(GetState(), AosHandle::STATE_CONNECTING);

    SetHandleState(AosHandle::STATE_DISCONNECTED);
}

TEST_F(AosHandleTest, ProcessUnavailableFeature_Test)
{
    // Expectation: Add/Remove unavailable feature

    ClearFeatureTagList();

    EXPECT_EQ(GetUnavailableFeatures(), ImsAosFeature::NONE);

    ProcessUnavailableFeature(ImsAosFeature::MMTEL, IMS_TRUE);
    EXPECT_EQ(GetUnavailableFeatures(), ImsAosFeature::MMTEL);

    ProcessUnavailableFeature(ImsAosFeature::MMTEL, IMS_FALSE);
    EXPECT_EQ(GetUnavailableFeatures(), ImsAosFeature::NONE);
}

TEST_F(AosHandleTest, ProcessUnavailableFeatureChanged_Test1)
{
    // Test1: state not connected
    // Expectation: Call AosRegistration::RequestCmd() with CMD_UNAVAILABLE_FEATURE_TAG param.

    SetHandleState(AosHandle::STATE_DISCONNECTED);

    MockIAosRegistration objMockIAosRegistration;
    EXPECT_CALL(m_objMockIAosAppContext, GetRegistration())
            .Times(AnyNumber())
            .WillRepeatedly(Return(static_cast<IAosRegistration*>(&objMockIAosRegistration)));

    EXPECT_CALL(
            objMockIAosRegistration, RequestCmd(IAosRegistration::CMD_UNAVAILABLE_FEATURE_TAG, 0))
            .Times(1);
    EXPECT_CALL(m_objMockIImsAosListener, ImsAos_Connected(_, _)).Times(0);

    ProcessUnavailableFeatureChanged();
}

TEST_F(AosHandleTest, ProcessUnavailableFeatureChanged_Test2)
{
    // Test2: state connected, listener is null
    // Expectation: Call AosRegistration::RequestCmd() with CMD_UNAVAILABLE_FEATURE_TAG param.

    SetHandleState(AosHandle::STATE_CONNECTED);
    m_pAosHandle->SetListener(IMS_NULL);
    ASSERT_EQ(GetListener(), nullptr);

    MockIAosRegistration objMockIAosRegistration;
    EXPECT_CALL(m_objMockIAosAppContext, GetRegistration())
            .Times(AnyNumber())
            .WillRepeatedly(Return(static_cast<IAosRegistration*>(&objMockIAosRegistration)));

    EXPECT_CALL(
            objMockIAosRegistration, RequestCmd(IAosRegistration::CMD_UNAVAILABLE_FEATURE_TAG, 0))
            .Times(1);
    EXPECT_CALL(m_objMockIImsAosListener, ImsAos_Connected(_, _)).Times(0);

    ProcessUnavailableFeatureChanged();

    SetHandleState(AosHandle::STATE_DISCONNECTED);
}

TEST_F(AosHandleTest, ProcessUnavailableFeatureChanged_Test3)
{
    // Test 3: state connected, listener is not null
    // Expectation: Call AosRegistration::RequestCmd() with CMD_UNAVAILABLE_FEATURE_TAG param.
    // + Call ImsAos_Connected of the listener

    SetHandleState(AosHandle::STATE_CONNECTED);
    m_pAosHandle->SetListener(static_cast<IImsAosListener*>(&m_objMockIImsAosListener));
    ASSERT_NE(GetListener(), nullptr);

    MockIAosRegistration objMockIAosRegistration;
    EXPECT_CALL(m_objMockIAosAppContext, GetRegistration())
            .Times(AnyNumber())
            .WillRepeatedly(Return(static_cast<IAosRegistration*>(&objMockIAosRegistration)));

    EXPECT_CALL(
            objMockIAosRegistration, RequestCmd(IAosRegistration::CMD_UNAVAILABLE_FEATURE_TAG, 0))
            .Times(1);

    EXPECT_CALL(m_objMockIAosConnection, GetIpcanCategory())
            .Times(AnyNumber())
            .WillRepeatedly(Return(IIpcan::CATEGORY_MOBILE));

    EXPECT_CALL(m_objMockIImsAosListener, ImsAos_Connected(_, _)).Times(1);

    ProcessUnavailableFeatureChanged();

    m_pAosHandle->SetListener(IMS_NULL);
    SetHandleState(AosHandle::STATE_DISCONNECTED);
}

TEST_F(AosHandleTest, IsBlockForMobile_Test)
{
    // Expectation: return false if the block is in wifi block policy, else true

    ClearHoldingBlocksPolicyForWifi();
    SetHoldingBlocksPolicyForWifi();

    EXPECT_TRUE(IsBlockForMobile(AosHandle::BLOCK_VOLTE_CAPABILITY));
    EXPECT_TRUE(IsBlockForMobile(AosHandle::BLOCK_VILTE_CAPABILITY));
    EXPECT_TRUE(IsBlockForMobile(AosHandle::BLOCK_VOPS));

    EXPECT_FALSE(IsBlockForMobile(AosHandle::BLOCK_VOWIFI_CAPABILITY));
    EXPECT_FALSE(IsBlockForMobile(AosHandle::BLOCK_VIWIFI_CAPABILITY));

    EXPECT_TRUE(IsBlockForMobile(AosHandle::BLOCK_SMS_CAPABILITY));
    EXPECT_TRUE(IsBlockForMobile(AosHandle::BLOCK_SMS_OVER_IP_NETWORK_INDICATION));

    ClearHoldingBlocksPolicyForWifi();
}

TEST_F(AosHandleTest, IsBlockForWifi_Test)
{
    // Expectation: return false if the block is in mobile block policy, else true

    ClearHoldingBlocksPolicyForMobile();
    SetHoldingBlocksPolicyForMobile();

    EXPECT_FALSE(IsBlockForWifi(AosHandle::BLOCK_VOLTE_CAPABILITY));
    EXPECT_FALSE(IsBlockForWifi(AosHandle::BLOCK_VILTE_CAPABILITY));
    EXPECT_FALSE(IsBlockForWifi(AosHandle::BLOCK_VOPS));

    EXPECT_TRUE(IsBlockForWifi(AosHandle::BLOCK_VOWIFI_CAPABILITY));
    EXPECT_TRUE(IsBlockForWifi(AosHandle::BLOCK_VIWIFI_CAPABILITY));

    EXPECT_TRUE(IsBlockForWifi(AosHandle::BLOCK_SMS_CAPABILITY));
    EXPECT_TRUE(IsBlockForWifi(AosHandle::BLOCK_SMS_OVER_IP_NETWORK_INDICATION));

    ClearHoldingBlocksPolicyForMobile();
}

TEST_F(AosHandleTest, IsUnavailableFeature_Test)
{
    // Expectation: Return true if the config feature is existed in unavailable feature list.
    // Else return false.

    IMSVector<IMS_SINT32> objRegistrationWithFeatureTagUnavailable;
    objRegistrationWithFeatureTagUnavailable.Add(
            CarrierConfig::Assets::UNAVAILABLE_FEATURE_TYPE_MMTEL);

    EXPECT_CALL(m_objMockIAosNConfiguration, GetRegWithFeatureTagUnavailable())
            .Times(AnyNumber())
            .WillRepeatedly(ReturnRef(objRegistrationWithFeatureTagUnavailable));

    EXPECT_TRUE(IsUnavailableFeature(CarrierConfig::Assets::UNAVAILABLE_FEATURE_TYPE_MMTEL));
    EXPECT_FALSE(IsUnavailableFeature(CarrierConfig::Assets::UNAVAILABLE_FEATURE_TYPE_VIDEO));
}

TEST_F(AosHandleTest, IsUnavailableFeaturePolicy_Test)
{
    // Expectation: Return true if the policy is existed in unavailable feature policy list.
    // Else return false.

    IMSVector<IMS_SINT32> objRegistrationWithFeatureTagUnavailablePolicy;
    objRegistrationWithFeatureTagUnavailablePolicy.Add(
            CarrierConfig::Assets::UNAVAILABLE_FEATURE_POLICY_VOPS);

    EXPECT_CALL(m_objMockIAosNConfiguration, GetRegWithFeatureTagUnavailablePolicy())
            .Times(AnyNumber())
            .WillRepeatedly(ReturnRef(objRegistrationWithFeatureTagUnavailablePolicy));

    EXPECT_TRUE(IsUnavailableFeaturePolicy(CarrierConfig::Assets::UNAVAILABLE_FEATURE_POLICY_VOPS));
    EXPECT_FALSE(IsUnavailableFeaturePolicy(
            CarrierConfig::Assets::UNAVAILABLE_FEATURE_POLICY_CAPABILITY));
}

TEST_F(AosHandleTest, StateDisconnected_Test1)
{
    // Test1: HANDLE_MSG_BLOCK_STATUS, Handle not blocked
    // Expectation: state-connecting, request type-attach, call reconfig(), need to notify.

    SetHandleState(AosHandle::STATE_DISCONNECTED);
    SetBlocked(IMS_FALSE);
    SetNotify(IMS_FALSE);

    EXPECT_CALL(m_objMockIAosApplication, Reconfig()).Times(1);

    IMSMSG objMSG(0 /*AosHandle::HANDLE_MSG_BLOCK_STATUS*/, 0, 0);
    m_pAosHandle->OnStateMessage(objMSG);

    EXPECT_EQ(GetState(), AosHandle::STATE_CONNECTING);
    EXPECT_EQ(m_pAosHandle->GetRequestType(), IAosHandle::ATTACH);
    EXPECT_TRUE(GetNotify());
}

TEST_F(AosHandleTest, StateDisconnected_Test2)
{
    // Test2: HANDLE_MSG_BLOCK_STATUS, Handle blocked
    // Expectation: state-disconnected, request type-detach, no call reconfig(), no need to notify.

    SetHandleState(AosHandle::STATE_DISCONNECTED);
    SetBlocked(IMS_TRUE);
    SetNotify(IMS_FALSE);

    EXPECT_CALL(m_objMockIAosApplication, Reconfig()).Times(0);

    IMSMSG objMSG(0 /*AosHandle::HANDLE_MSG_BLOCK_STATUS*/, 0, 0);
    m_pAosHandle->OnStateMessage(objMSG);

    EXPECT_EQ(GetState(), AosHandle::STATE_DISCONNECTED);
    EXPECT_EQ(m_pAosHandle->GetRequestType(), IAosHandle::DETACH);
    EXPECT_FALSE(GetNotify());
}

TEST_F(AosHandleTest, StateDisconnected_Test3)
{
    // Test3: HANDLE_MSG_APP_STATUS, Handle blocked
    // Expectation: state-disconnected, request type-detach, no call reconfig(), no need to notify.

    SetHandleState(AosHandle::STATE_DISCONNECTED);
    SetBlocked(IMS_TRUE);
    SetNotify(IMS_FALSE);

    EXPECT_CALL(m_objMockIAosApplication, Reconfig()).Times(0);

    IMSMSG objMSG(1 /*AosHandle::HANDLE_MSG_APP_STATUS*/, 0, 0);
    m_pAosHandle->OnStateMessage(objMSG);

    EXPECT_EQ(GetState(), AosHandle::STATE_DISCONNECTED);
    EXPECT_EQ(m_pAosHandle->GetRequestType(), IAosHandle::DETACH);
    EXPECT_FALSE(GetNotify());
}

TEST_F(AosHandleTest, StateDisconnected_Test4)
{
    // Test4: HANDLE_MSG_INVALID
    // Expectation: state-disconnected, request type-detach, no call reconfig(), no need to notify.

    SetHandleState(AosHandle::STATE_DISCONNECTED);
    SetNotify(IMS_FALSE);

    EXPECT_CALL(m_objMockIAosApplication, Reconfig()).Times(0);

    IMSMSG objMSG(2 /*AosHandle::HANDLE_MSG_INVALID*/, 0, 0);
    m_pAosHandle->OnStateMessage(objMSG);

    EXPECT_EQ(GetState(), AosHandle::STATE_DISCONNECTED);
    EXPECT_EQ(m_pAosHandle->GetRequestType(), IAosHandle::DETACH);
    EXPECT_FALSE(GetNotify());
}

TEST_F(AosHandleTest, StateConnecting_Test1)
{
    // Test1: HANDLE_MSG_BLOCK_STATUS, Handle blocked
    // Expectation: state-disconnected, request type-detach, call reconfig(), need to notify.

    SetHandleState(AosHandle::STATE_CONNECTING);
    SetBlocked(IMS_TRUE);
    SetNotify(IMS_FALSE);

    EXPECT_CALL(m_objMockIAosApplication, Reconfig()).Times(1);

    IMSMSG objMSG(0 /*HANDLE_MSG_BLOCK_STATUS*/, 0, 0);
    m_pAosHandle->OnStateMessage(objMSG);

    EXPECT_EQ(GetState(), AosHandle::STATE_DISCONNECTED);
    EXPECT_EQ(m_pAosHandle->GetRequestType(), IAosHandle::DETACH);
    EXPECT_TRUE(GetNotify());
}

TEST_F(AosHandleTest, StateConnecting_Test2)
{
    // Test2: HANDLE_MSG_BLOCK_STATUS, Handle not blocked
    // Expectation: state-connecting, request type-attach, no call reconfig(), no need to notify.

    SetHandleState(AosHandle::STATE_CONNECTING);
    m_pAosHandle->SetRequestType(IAosHandle::ATTACH);
    SetBlocked(IMS_FALSE);
    SetNotify(IMS_FALSE);

    EXPECT_CALL(m_objMockIAosApplication, Reconfig()).Times(0);

    IMSMSG objMSG(0 /*HANDLE_MSG_BLOCK_STATUS*/, 0, 0);
    m_pAosHandle->OnStateMessage(objMSG);

    EXPECT_EQ(GetState(), AosHandle::STATE_CONNECTING);
    EXPECT_EQ(m_pAosHandle->GetRequestType(), IAosHandle::ATTACH);
    EXPECT_FALSE(GetNotify());
}

TEST_F(AosHandleTest, StateConnecting_Test3)
{
    // Test3: HANDLE_MSG_APP_STATUS, APP_CONNECTED, Reg binded
    // Expectation: clear suspended reason, state-connected, need to notify

    SetHandleState(AosHandle::STATE_CONNECTING);
    m_pAosHandle->SetRegBinded(IMS_TRUE);
    SetNotify(IMS_FALSE);
    SetSuspendedReason(AoSReason::SUSPEND_NO_SERVICE);
    EXPECT_EQ(m_pAosHandle->GetSuspendedReason(), AoSReason::SUSPEND_NO_SERVICE);

    EXPECT_CALL(m_objMockIAosNetTracker, IsSuspended()).Times(1).WillOnce(Return(IMS_FALSE));

    m_pAosHandle->App_StateChanged(IAosApplication::APP_CONNECTED, 0);

    EXPECT_EQ(m_pAosHandle->GetSuspendedReason(), AoSReason::SUSPEND_NONE);
    EXPECT_EQ(GetState(), AosHandle::STATE_CONNECTED);
    EXPECT_TRUE(GetNotify());
}

TEST_F(AosHandleTest, StateConnecting_Test4)
{
    // Test4: HANDLE_MSG_APP_STATUS, APP_DISCONNECTED
    // Expectation: need to notify

    SetHandleState(AosHandle::STATE_CONNECTING);
    SetNotify(IMS_FALSE);

    m_pAosHandle->App_StateChanged(IAosApplication::APP_DISCONNECTED, 0);

    EXPECT_TRUE(GetNotify());
}

TEST_F(AosHandleTest, StateConnecting_Test5)
{
    // Test5: HANDLE_MSG_APP_STATUS, APP_DISCONNECTING
    // Expectation: no need to notify

    SetHandleState(AosHandle::STATE_CONNECTING);
    SetNotify(IMS_FALSE);

    m_pAosHandle->App_StateChanged(IAosApplication::APP_DISCONNECTING, 0);

    EXPECT_FALSE(GetNotify());
}

TEST_F(AosHandleTest, StateConnecting_Test6)
{
    // Test6: HANDLE_MSG_INVALID
    // Expectation: state-connecting, request type-attach, no call reconfig(), no need to notify.

    SetHandleState(AosHandle::STATE_CONNECTING);
    m_pAosHandle->SetRequestType(IAosHandle::ATTACH);
    SetNotify(IMS_FALSE);

    EXPECT_CALL(m_objMockIAosApplication, Reconfig()).Times(0);

    IMSMSG objMSG(2 /*HANDLE_MSG_INVALID*/, 0, 0);
    m_pAosHandle->OnStateMessage(objMSG);

    EXPECT_EQ(GetState(), AosHandle::STATE_CONNECTING);
    EXPECT_EQ(m_pAosHandle->GetRequestType(), IAosHandle::ATTACH);
    EXPECT_FALSE(GetNotify());
}

TEST_F(AosHandleTest, StateConnected_Test1)
{
    // Test1: HANDLE_MSG_BLOCK_STATUS, Handle blocked, reason == 0
    // Expectation: state-disconnecting, request type-detach, call reconfig(), need to notify.

    SetHandleState(AosHandle::STATE_CONNECTED);
    m_pAosHandle->SetRequestType(IAosHandle::ATTACH);
    SetBlocked(IMS_TRUE);
    SetNotify(IMS_FALSE);

    EXPECT_CALL(m_objMockIAosApplication, Reconfig()).Times(1);

    IMSMSG objMSG(0 /*AosHandle::HANDLE_MSG_BLOCK_STATUS*/, 0, 0);
    m_pAosHandle->OnStateMessage(objMSG);

    EXPECT_EQ(GetState(), AosHandle::STATE_DISCONNECTING);
    EXPECT_EQ(m_pAosHandle->GetRequestType(), IAosHandle::DETACH);
    EXPECT_TRUE(GetNotify());
}

TEST_F(AosHandleTest, StateConnected_Test2)
{
    // Test2: HANDLE_MSG_BLOCK_STATUS, Handle blocked, reason > 0
    // Expectation: state-disconnecting, request type-detach, call reconfig(), need to notify,
    //              set reason

    SetHandleState(AosHandle::STATE_CONNECTED);
    m_pAosHandle->SetRequestType(IAosHandle::ATTACH);
    SetBlocked(IMS_TRUE);
    SetNotify(IMS_FALSE);

    EXPECT_CALL(m_objMockIAosApplication, Reconfig()).Times(1);

    IMSMSG objMSG(0 /*AosHandle::HANDLE_MSG_BLOCK_STATUS*/, 1, 0);
    m_pAosHandle->OnStateMessage(objMSG);

    EXPECT_EQ(GetState(), AosHandle::STATE_DISCONNECTING);
    EXPECT_EQ(m_pAosHandle->GetRequestType(), IAosHandle::DETACH);
    EXPECT_TRUE(GetNotify());
    EXPECT_EQ(GetReason(), 1);
}

TEST_F(AosHandleTest, StateConnected_Test3)
{
    // Test3: HANDLE_MSG_BLOCK_STATUS, Handle not blocked
    // Expectation: state-connected, request type-attach, no call reconfig(), no need to notify.

    SetHandleState(AosHandle::STATE_CONNECTED);
    m_pAosHandle->SetRequestType(IAosHandle::ATTACH);
    SetBlocked(IMS_FALSE);
    SetNotify(IMS_FALSE);

    EXPECT_CALL(m_objMockIAosApplication, Reconfig()).Times(0);

    IMSMSG objMSG(0 /*AosHandle::HANDLE_MSG_BLOCK_STATUS*/, 0, 0);
    m_pAosHandle->OnStateMessage(objMSG);

    EXPECT_EQ(GetState(), AosHandle::STATE_CONNECTED);
    EXPECT_EQ(m_pAosHandle->GetRequestType(), IAosHandle::ATTACH);
    EXPECT_FALSE(GetNotify());
}

TEST_F(AosHandleTest, StateConnected_Test4)
{
    // Test4: HANDLE_MSG_APP_STATUS, APP_DISCONNECTED
    // Expectation: state-connecting, need to notify

    SetHandleState(AosHandle::STATE_CONNECTED);
    SetNotify(IMS_FALSE);

    m_pAosHandle->App_StateChanged(IAosApplication::APP_DISCONNECTED, 0);

    EXPECT_EQ(GetState(), AosHandle::STATE_CONNECTING);
    EXPECT_TRUE(GetNotify());
}

TEST_F(AosHandleTest, StateConnected_Test5)
{
    // Test5: HANDLE_MSG_APP_STATUS, APP_CONNECTED, Reg not binded, handle blocked
    // Expectation: state-disconnected, need to notify

    SetHandleState(AosHandle::STATE_CONNECTED);
    SetBlocked(IMS_TRUE);
    SetNotify(IMS_FALSE);
    m_pAosHandle->SetRegBinded(IMS_FALSE);

    m_pAosHandle->App_StateChanged(IAosApplication::APP_CONNECTED, 0);

    EXPECT_EQ(GetState(), AosHandle::STATE_DISCONNECTED);
    EXPECT_TRUE(GetNotify());
}

TEST_F(AosHandleTest, StateConnected_Test6)
{
    // Test6: HANDLE_MSG_APP_STATUS, APP_CONNECTED, Reg not binded, handle not blocked
    // Expectation: state-connecting, need to notify

    SetHandleState(AosHandle::STATE_CONNECTED);
    SetBlocked(IMS_FALSE);
    SetNotify(IMS_FALSE);
    m_pAosHandle->SetRegBinded(IMS_FALSE);

    m_pAosHandle->App_StateChanged(IAosApplication::APP_CONNECTED, 0);

    EXPECT_EQ(GetState(), AosHandle::STATE_CONNECTING);
    EXPECT_TRUE(GetNotify());
}

TEST_F(AosHandleTest, StateConnected_Test7)
{
    // Test7: HANDLE_MSG_APP_STATUS, APP_DISCONNECTING
    // Expectation: state-disconnecting, need to notify

    SetHandleState(AosHandle::STATE_CONNECTED);

    m_pAosHandle->App_StateChanged(IAosApplication::APP_DISCONNECTING, 0);

    EXPECT_EQ(GetState(), AosHandle::STATE_DISCONNECTING);
    EXPECT_TRUE(GetNotify());
}

TEST_F(AosHandleTest, StateConnected_Test8)
{
    // Test8: HANDLE_MSG_APP_STATUS, APP_UPDATING
    // Expectation: no need to notify

    SetHandleState(AosHandle::STATE_CONNECTED);

    m_pAosHandle->App_StateChanged(IAosApplication::APP_UPDATING, 0);

    EXPECT_FALSE(GetNotify());
}

TEST_F(AosHandleTest, StateConnected_Test9)
{
    // Test9: HANDLE_MSG_INVALID
    // Expectation: no need to notify

    SetHandleState(AosHandle::STATE_CONNECTED);

    IMSMSG objMSG(2 /*HANDLE_MSG_INVALID*/, 0, 0);
    m_pAosHandle->OnStateMessage(objMSG);

    EXPECT_FALSE(GetNotify());
}

TEST_F(AosHandleTest, StateDisconnecting_Test1)
{
    // Test1: HANDLE_MSG_BLOCK_STATUS, Handle blocked
    // Expectation: request type-detach, call reconfig(), no need to notify.

    SetHandleState(AosHandle::STATE_DISCONNECTING);
    SetBlocked(IMS_TRUE);
    SetNotify(IMS_FALSE);

    EXPECT_CALL(m_objMockIAosApplication, Reconfig()).Times(1);

    IMSMSG objMSG(0 /*HANDLE_MSG_BLOCK_STATUS*/, 0, 0);
    m_pAosHandle->OnStateMessage(objMSG);

    EXPECT_EQ(m_pAosHandle->GetRequestType(), IAosHandle::DETACH);
    EXPECT_FALSE(GetNotify());
}

TEST_F(AosHandleTest, StateDisconnecting_Test2)
{
    // Test2: HANDLE_MSG_BLOCK_STATUS, Handle not blocked
    // Expectation: request type-attach, state-connecting, call reconfig(), need to notify.

    SetHandleState(AosHandle::STATE_DISCONNECTING);
    SetBlocked(IMS_FALSE);
    SetNotify(IMS_FALSE);

    EXPECT_CALL(m_objMockIAosApplication, Reconfig()).Times(1);

    IMSMSG objMSG(0 /*HANDLE_MSG_BLOCK_STATUS*/, 0, 0);
    m_pAosHandle->OnStateMessage(objMSG);

    EXPECT_EQ(m_pAosHandle->GetRequestType(), IAosHandle::ATTACH);
    EXPECT_EQ(GetState(), AosHandle::STATE_CONNECTING);
    EXPECT_TRUE(GetNotify());
}

TEST_F(AosHandleTest, StateDisconnecting_Test3)
{
    // Test3: HANDLE_MSG_APP_STATUS, APP_DISCONNECTED, handle blocked
    // Expectation: state-disconnected, need to notify

    SetHandleState(AosHandle::STATE_DISCONNECTING);
    SetBlocked(IMS_TRUE);
    SetNotify(IMS_FALSE);

    m_pAosHandle->App_StateChanged(IAosApplication::APP_DISCONNECTED, 0);

    EXPECT_EQ(GetState(), AosHandle::STATE_DISCONNECTED);
    EXPECT_TRUE(GetNotify());
}

TEST_F(AosHandleTest, StateDisconnecting_Test4)
{
    // Test4: HANDLE_MSG_APP_STATUS, APP_DISCONNECTED, handle not blocked
    // Expectation: state-connecting, need to notify

    SetHandleState(AosHandle::STATE_DISCONNECTING);
    SetBlocked(IMS_FALSE);
    SetNotify(IMS_FALSE);

    m_pAosHandle->App_StateChanged(IAosApplication::APP_DISCONNECTED, 0);

    EXPECT_EQ(GetState(), AosHandle::STATE_CONNECTING);
    EXPECT_TRUE(GetNotify());
}

TEST_F(AosHandleTest, StateDisconnecting_Test5)
{
    // Test5: HANDLE_MSG_APP_STATUS, APP_CONNECTED, Reg binded, handle not blocked
    // Expectation: state-connected, need to notify

    SetHandleState(AosHandle::STATE_DISCONNECTING);
    m_pAosHandle->SetRegBinded(IMS_TRUE);
    SetBlocked(IMS_FALSE);
    SetNotify(IMS_FALSE);

    m_pAosHandle->App_StateChanged(IAosApplication::APP_CONNECTED, 0);

    EXPECT_EQ(GetState(), AosHandle::STATE_CONNECTED);
    EXPECT_TRUE(GetNotify());
}

TEST_F(AosHandleTest, StateDisconnecting_Test6)
{
    // Test6: HANDLE_MSG_APP_STATUS, APP_CONNECTED, Reg not binded, handle blocked
    // Expectation: state-disconnected, need to notify

    SetHandleState(AosHandle::STATE_DISCONNECTING);
    m_pAosHandle->SetRegBinded(IMS_FALSE);
    SetBlocked(IMS_TRUE);
    SetNotify(IMS_FALSE);

    m_pAosHandle->App_StateChanged(IAosApplication::APP_CONNECTED, 0);

    EXPECT_EQ(GetState(), AosHandle::STATE_DISCONNECTED);
    EXPECT_TRUE(GetNotify());
}

TEST_F(AosHandleTest, StateDisconnecting_Test7)
{
    // Test7: HANDLE_MSG_APP_STATUS, APP_CONNECTED, Reg not binded, handle not blocked
    // Expectation: state-connecting, need to notify

    SetHandleState(AosHandle::STATE_DISCONNECTING);
    m_pAosHandle->SetRegBinded(IMS_FALSE);
    SetBlocked(IMS_FALSE);
    SetNotify(IMS_FALSE);

    m_pAosHandle->App_StateChanged(IAosApplication::APP_CONNECTED, 0);

    EXPECT_EQ(GetState(), AosHandle::STATE_CONNECTING);
    EXPECT_TRUE(GetNotify());
}

TEST_F(AosHandleTest, StateDisconnecting_Test8)
{
    // Test8: HANDLE_MSG_APP_STATUS, APP_UPDATING, Reg binded, handle not blocked
    // Expectation: state-connecting, need to notify

    SetHandleState(AosHandle::STATE_DISCONNECTING);
    m_pAosHandle->SetRegBinded(IMS_TRUE);
    SetBlocked(IMS_FALSE);
    SetNotify(IMS_FALSE);

    m_pAosHandle->App_StateChanged(IAosApplication::APP_UPDATING, 0);

    EXPECT_EQ(GetState(), AosHandle::STATE_CONNECTING);
    EXPECT_TRUE(GetNotify());
}

TEST_F(AosHandleTest, StateDisconnecting_Test9)
{
    // Test9: HANDLE_MSG_APP_STATUS, APP_UPDATING, Reg not binded, handle blocked
    // Expectation: state-disconnected, need to notify

    SetHandleState(AosHandle::STATE_DISCONNECTING);
    m_pAosHandle->SetRegBinded(IMS_FALSE);
    SetBlocked(IMS_TRUE);
    SetNotify(IMS_FALSE);

    m_pAosHandle->App_StateChanged(IAosApplication::APP_UPDATING, 0);

    EXPECT_EQ(GetState(), AosHandle::STATE_DISCONNECTED);
    EXPECT_TRUE(GetNotify());
}

TEST_F(AosHandleTest, StateDisconnecting_Test10)
{
    // Test10: HANDLE_MSG_APP_STATUS, APP_UPDATING, Reg not binded, handle not blocked
    // Expectation: state-connecting, need to notify

    SetHandleState(AosHandle::STATE_DISCONNECTING);
    m_pAosHandle->SetRegBinded(IMS_FALSE);
    SetBlocked(IMS_FALSE);
    SetNotify(IMS_FALSE);

    m_pAosHandle->App_StateChanged(IAosApplication::APP_UPDATING, 0);

    EXPECT_EQ(GetState(), AosHandle::STATE_CONNECTING);
    EXPECT_TRUE(GetNotify());
}

TEST_F(AosHandleTest, StateDisconnecting_Test11)
{
    // Test10: HANDLE_MSG_APP_STATUS, APP_DISCONNECTING
    // Expectation: need to notify

    SetHandleState(AosHandle::STATE_DISCONNECTING);
    m_pAosHandle->SetRegBinded(IMS_FALSE);
    SetBlocked(IMS_FALSE);
    SetNotify(IMS_FALSE);

    m_pAosHandle->App_StateChanged(IAosApplication::APP_DISCONNECTING, 0);

    EXPECT_TRUE(GetNotify());
}

TEST_F(AosHandleTest, StateDisconnecting_Test12)
{
    // Test12: HANDLE_MSG_APP_STATUS, invalid app state
    // Expectation: no need to notify

    SetHandleState(AosHandle::STATE_DISCONNECTING);
    SetNotify(IMS_FALSE);

    m_pAosHandle->App_StateChanged(4, 0);

    EXPECT_FALSE(GetNotify());
}

TEST_F(AosHandleTest, StateDisconnecting_Test13)
{
    // Test13: HANDLE_MSG_INVALID
    // Expectation: no need to notify

    SetHandleState(AosHandle::STATE_DISCONNECTING);
    SetNotify(IMS_FALSE);

    IMSMSG objMSG(2 /*HANDLE_MSG_INVALID*/, 0, 0);
    m_pAosHandle->OnStateMessage(objMSG);

    EXPECT_FALSE(GetNotify());
}

TEST_F(AosHandleTest, IsBlocked_Test)
{
    // Expectation: return m_bBlocked value as set

    SetBlocked(IMS_TRUE);
    EXPECT_TRUE(IsBlocked());

    SetBlocked(IMS_FALSE);
    EXPECT_FALSE(IsBlocked());
}

TEST_F(AosHandleTest, InitializeFeatureTags_Test)
{
    // Expectation: +cdmaless if the config is set

    EXPECT_CALL(m_objMockIAosNConfiguration, IsCdmalessFeatureTagRequired())
            .Times(2)
            .WillOnce(Return(IMS_TRUE))
            .WillOnce(Return(IMS_FALSE));

    InitializeFeatureTags();
    EXPECT_TRUE(HasFeatureTag("+cdmaless", AString::ConstNull()));

    InitializeFeatureTags();
    EXPECT_FALSE(HasFeatureTag("+cdmaless", AString::ConstNull()));
}

TEST_F(AosHandleTest, UpdateFeatureTags_Test)
{
    // Expectation: Clear feature tags if no features
    ClearFeatureTagList();

    AddFeatureTag("+cdmaless");
    EXPECT_TRUE(HasFeatureTag("+cdmaless", AString::ConstNull()));

    UpdateFeatureTags();
    EXPECT_FALSE(HasFeatureTag("+cdmaless", AString::ConstNull()));

    ClearFeatureTagList();
}

TEST_F(AosHandleTest, ProcessImsSuspended_Test1)
{
    // Test1: ims not connected
    // Expectation: no suspended reason is set, return false

    SetHandleState(AosHandle::STATE_DISCONNECTED);
    ClearSuspendedReason();
    EXPECT_FALSE(ProcessImsSuspended(0));
    EXPECT_EQ(m_pAosHandle->GetSuspendedReason(), AoSReason::SUSPEND_NONE);
}

TEST_F(AosHandleTest, ProcessImsSuspended_Test2)
{
    // Test2: ims connected, suspended reason-SUSPEND_NO_SERVICE, listener is not null
    // Expectation: Call ImsAos_Suspended, return true

    SetHandleState(AosHandle::STATE_CONNECTED);
    IImsAosListener* piListener = static_cast<IImsAosListener*>(&m_objMockIImsAosListener);
    m_pAosHandle->SetListener(piListener);
    EXPECT_CALL(m_objMockIImsAosListener, ImsAos_Suspended(_)).Times(1);

    EXPECT_TRUE(ProcessImsSuspended(AoSReason::SUSPEND_NO_SERVICE));
}

TEST_F(AosHandleTest, ProcessImsSuspended_Test3)
{
    // Test3: ims connected, suspended reason-SUSPEND_NO_SERVICE, listener is null
    // Expectation: suspended reason is set, No call ImsAos_Suspended, return false

    SetHandleState(AosHandle::STATE_CONNECTED);
    ClearSuspendedReason();
    m_pAosHandle->SetListener(IMS_NULL);
    EXPECT_CALL(m_objMockIImsAosListener, ImsAos_Suspended(_)).Times(0);

    EXPECT_FALSE(ProcessImsSuspended(AoSReason::SUSPEND_NO_SERVICE));
    EXPECT_EQ(m_pAosHandle->GetSuspendedReason(), AoSReason::SUSPEND_NO_SERVICE);
}

TEST_F(AosHandleTest, ProcessImsResumed_Test1)
{
    // Test1: ims not connected
    // Expectation: suspended reason is not reset, return false

    SetHandleState(AosHandle::STATE_DISCONNECTED);
    SetSuspendedReason(AoSReason::SUSPEND_NO_SERVICE);

    EXPECT_FALSE(ProcessImsResumed(AoSReason::SUSPEND_NO_SERVICE));
    EXPECT_EQ(m_pAosHandle->GetSuspendedReason(), AoSReason::SUSPEND_NO_SERVICE);
}

TEST_F(AosHandleTest, ProcessImsResumed_Test2)
{
    // Test2: ims connected, ims not suspended
    // Expectation: return false

    SetHandleState(AosHandle::STATE_CONNECTED);
    ClearSuspendedReason();

    EXPECT_FALSE(ProcessImsResumed(AoSReason::SUSPEND_NO_SERVICE));
}

TEST_F(AosHandleTest, ProcessImsResumed_Test3)
{
    // Test3: ims connected, ims suspended, still suspended after reset the reason.
    // Expectation: reset the reason from suspended reason, return false

    SetHandleState(AosHandle::STATE_CONNECTED);
    ClearSuspendedReason();
    SetSuspendedReasonForTest((AoSReason::SUSPEND_NO_SERVICE | AoSReason::SUSPEND_NO_LTE_COVERAGE));

    EXPECT_FALSE(ProcessImsResumed(AoSReason::SUSPEND_NO_SERVICE));
    EXPECT_EQ(m_pAosHandle->GetSuspendedReason(), AoSReason::SUSPEND_NO_LTE_COVERAGE);
}

TEST_F(AosHandleTest, ProcessImsResumed_Test4)
{
    // Test4: ims connected, ims suspended, no suspended reason after reset, no listener.
    // Expectation: reset the reason from suspended reason, set reason, return false

    SetHandleState(AosHandle::STATE_CONNECTED);
    ClearSuspendedReason();
    SetSuspendedReason(AoSReason::SUSPEND_NO_SERVICE);
    m_pAosHandle->SetListener(IMS_NULL);
    SetReason(AoSReason::SUSPEND_NONE);

    EXPECT_FALSE(ProcessImsResumed(AoSReason::SUSPEND_NO_SERVICE));
    EXPECT_EQ(m_pAosHandle->GetSuspendedReason(), AoSReason::SUSPEND_NONE);
    EXPECT_EQ(GetReason(), AoSReason::SUSPEND_NO_SERVICE);
}

TEST_F(AosHandleTest, ProcessImsResumed_Test5)
{
    // Test5: ims connected, ims suspended, no suspended reason after reset, listener existed.
    // Expectation: no suspended reason, set reason, call ImsAos_Resumed(), return true

    SetHandleState(AosHandle::STATE_CONNECTED);
    ClearSuspendedReason();
    SetSuspendedReason(AoSReason::SUSPEND_NO_SERVICE);
    IImsAosListener* piListener = static_cast<IImsAosListener*>(&m_objMockIImsAosListener);
    m_pAosHandle->SetListener(piListener);
    SetReason(AoSReason::SUSPEND_NONE);

    EXPECT_CALL(m_objMockIImsAosListener, ImsAos_Resumed()).Times(1);
    EXPECT_TRUE(ProcessImsResumed(AoSReason::SUSPEND_NO_SERVICE));
    EXPECT_EQ(m_pAosHandle->GetSuspendedReason(), AoSReason::SUSPEND_NONE);
    EXPECT_EQ(GetReason(), AoSReason::SUSPEND_NO_SERVICE);

    // Clean up
    SetHandleState(AosHandle::STATE_DISCONNECTED);
    ClearSuspendedReason();
    m_pAosHandle->SetListener(IMS_NULL);
    SetReason(AoSReason::SUSPEND_NONE);
}

TEST_F(AosHandleTest, CheckSuspended_Test1)
{
    // Test1: Suspended.
    // Expectation: Suspended reason-SUSPEND_NO_SERVICE, m_bNetSrvIn is false.

    ClearSuspendedReason();

    EXPECT_CALL(m_objMockIAosNetTracker, IsSuspended()).Times(1).WillOnce(Return(IMS_TRUE));

    CheckSuspended();

    EXPECT_EQ(m_pAosHandle->GetSuspendedReason(), AoSReason::SUSPEND_NO_SERVICE);
    EXPECT_FALSE(GetNetSrvIn());
}

TEST_F(AosHandleTest, CheckSuspended_Test2)
{
    // Test2: Not suspended.
    // Expectation: Suspended reason-SUSPEND_NONE, m_bNetSrvIn is true.

    ClearSuspendedReason();

    EXPECT_CALL(m_objMockIAosNetTracker, IsSuspended()).Times(1).WillOnce(Return(IMS_FALSE));

    CheckSuspended();

    EXPECT_EQ(m_pAosHandle->GetSuspendedReason(), AoSReason::SUSPEND_NONE);
    EXPECT_TRUE(GetNetSrvIn());
}

TEST_F(AosHandleTest, ResetSuspendedReason_Test1)
{
    // Test1: reason-SUSPEND_NO_SERVICE
    // Expectation: reset the reason from suspended reason

    ClearSuspendedReason();
    SetSuspendedReason(AoSReason::SUSPEND_NO_SERVICE);
    EXPECT_EQ(m_pAosHandle->GetSuspendedReason(), AoSReason::SUSPEND_NO_SERVICE);

    ResetSuspendedReason(AoSReason::SUSPEND_NO_SERVICE);

    EXPECT_EQ(m_pAosHandle->GetSuspendedReason(), AoSReason::SUSPEND_NONE);
}

TEST_F(AosHandleTest, ResetSuspendedReason_Test2)
{
    // Test2: reason-other than SUSPEND_NO_SERVICE
    // Expectation: no reset the reason from suspended reason

    ClearSuspendedReason();
    SetSuspendedReasonForTest(AoSReason::SUSPEND_NO_LTE_COVERAGE);
    EXPECT_EQ(m_pAosHandle->GetSuspendedReason(), AoSReason::SUSPEND_NO_LTE_COVERAGE);

    ResetSuspendedReason(AoSReason::SUSPEND_NO_LTE_COVERAGE);

    EXPECT_EQ(m_pAosHandle->GetSuspendedReason(), AoSReason::SUSPEND_NO_LTE_COVERAGE);

    ClearSuspendedReason();
}

TEST_F(AosHandleTest, ReportRegState_Test1)
{
    // Test1: RegStateManager is null
    // Expectation: no call SetRegState with any parameteres.

    AosProvider::GetInstance()->SetRegStateManager(IMS_NULL);

    EXPECT_CALL(m_objMockIAosRegStateManager, SetRegState(_, _)).Times(0);

    ReportRegState();
}

TEST_F(AosHandleTest, ReportRegState_Test2)
{
    // Test2: RegStateManager is not null, state connected
    // Expectation: call SetRegState with IMS_REG_ON.

    AosProvider::GetInstance()->SetRegStateManager(
            static_cast<IAosRegStateManager*>(&m_objMockIAosRegStateManager));
    SetHandleState(AosHandle::STATE_CONNECTED);

    EXPECT_CALL(m_objMockIAosRegStateManager, SetRegState(_, IMS_REG_ON)).Times(1);

    ReportRegState();
}

TEST_F(AosHandleTest, ReportRegState_Test3)
{
    // Test3: RegStateManager is not null, state other than connected
    // Expectation: call SetRegState with IMS_REG_OFF.

    EXPECT_CALL(m_objMockIAosRegStateManager, SetRegState(_, IMS_REG_OFF)).Times(3);

    SetHandleState(AosHandle::STATE_DISCONNECTED);
    ReportRegState();

    SetHandleState(AosHandle::STATE_DISCONNECTING);
    ReportRegState();

    SetHandleState(AosHandle::STATE_CONNECTING);
    ReportRegState();
}

TEST_F(AosHandleTest, ReportRegState_Test4)
{
    // Test4: RegStateManager is not null, invalid state
    // Expectation: no call SetRegState with any parameteres.

    AosProvider::GetInstance()->SetRegStateManager(IMS_NULL);
    SetHandleState(AosHandle::STATE_INVALID);

    EXPECT_CALL(m_objMockIAosRegStateManager, SetRegState(_, _)).Times(0);

    ReportRegState();
}

TEST_F(AosHandleTest, NConfiguration_NotifyConfigChanged_Test1)
{
    // Test1: NConfig is null
    // Expectation: do nothing

    AosProvider::GetInstance()->SetNConfiguration(IMS_NULL);

    EXPECT_CALL(m_objMockIAosNConfiguration, IsCdmalessFeatureTagRequired()).Times(0);

    m_pAosHandle->NConfiguration_NotifyConfigChanged();
}

TEST_F(AosHandleTest, NConfiguration_NotifyConfigChanged_Test2)
{
    // Test2: NConfig is not null
    // Expectation: initialize handle

    AosProvider::GetInstance()->SetNConfiguration(
            static_cast<IAosNConfiguration*>(&m_objMockIAosNConfiguration));

    EXPECT_CALL(m_objMockIAosNConfiguration, IsCdmalessFeatureTagRequired()).Times(1);

    m_pAosHandle->NConfiguration_NotifyConfigChanged();
}

TEST_F(AosHandleTest, Request_Test)
{
    m_pAosHandle->Request(0);
}

TEST_F(AosHandleTest, Handle_Notify_Test)
{
    m_pAosHandle->Handle_Notify(0, IMS_FALSE);
}

TEST_F(AosHandleTest, UpdateFeature_Test)
{
    m_pAosHandle->UpdateFeature(0);
}

TEST_F(AosHandleTest, UpdateFeature_2_Test)
{
    m_pAosHandle->UpdateFeature(IMS_NULL);
}

TEST_F(AosHandleTest, CallTracker_StateChanged_Test)
{
    m_pAosHandle->CallTracker_StateChanged(IAosCallTracker::TYPE_NORMAL, CallState::IDLE);
}

TEST_F(AosHandleTest, ProcessCapabilitiesChanged_Test)
{
    ProcessCapabilitiesChanged(IMSMap<IMS_UINT32, IMS_UINT32>());
}

TEST_F(AosHandleTest, ProcessNetworkChanged_Test)
{
    ProcessNetworkChanged();
}

TEST_F(AosHandleTest, ProcessVopsStateChanged_Test)
{
    ProcessVopsStateChanged(00);
}

TEST_F(AosHandleTest, ProcessUnavailableFeatureForVops_Test)
{
    ProcessUnavailableFeatureForVops(0);
}

TEST_F(AosHandleTest, IsSupportedNetworkType_Test)
{
    EXPECT_TRUE(IsSupportedNetworkType(NW_REPORT_RADIO_LTE));
    EXPECT_TRUE(IsSupportedNetworkType(NW_REPORT_RADIO_NR));
    EXPECT_TRUE(IsSupportedNetworkType(NW_REPORT_RADIO_WLAN));
    EXPECT_FALSE(IsSupportedNetworkType(NW_REPORT_RADIO_CDMA));
}

TEST_F(AosHandleTest, IsSupportedNetworkTypeForCellular_Test)
{
    EXPECT_TRUE(IsSupportedNetworkTypeForCellular(NW_REPORT_RADIO_LTE));
    EXPECT_TRUE(IsSupportedNetworkTypeForCellular(NW_REPORT_RADIO_NR));
    EXPECT_FALSE(IsSupportedNetworkTypeForCellular(NW_REPORT_RADIO_WLAN));
    EXPECT_FALSE(IsSupportedNetworkTypeForCellular(NW_REPORT_RADIO_CDMA));
}

TEST_F(AosHandleTest, Event_NotifyEvent_Test)
{
    m_pAosHandle->Event_NotifyEvent(IMS_EVENT_IMS_VOICE_OVER_PS_STATE, 1, 0);
    m_pAosHandle->Event_NotifyEvent(IMS_EVENT_OMADM_UPDATED, 1, 0);
    m_pAosHandle->Event_NotifyEvent(IMS_EVENT_ROAMING_STATE, 1, 0);
    m_pAosHandle->Event_NotifyEvent(IMS_EVENT_ROAMING_PREFERRED_VOICE_CALL_NETWORK, 1, 0);
    m_pAosHandle->Event_NotifyEvent(IMS_EVENT_CONFIG_UPDATE, 1, 0);
    m_pAosHandle->Event_NotifyEvent(IMS_EVENT_PHONE_RESTARTED, 1, 0);
}

TEST_F(AosHandleTest, RegistrationControl_NotifyCapabilitiesChanged_Test)
{
    m_pAosHandle->RegistrationControl_NotifyCapabilitiesChanged(IMSMap<IMS_UINT32, IMS_UINT32>());
}

TEST_F(AosHandleTest, ServiceSetting_RoamingPreferredVoiceNetworkChanged_Test)
{
    m_pAosHandle->ServiceSetting_RoamingPreferredVoiceNetworkChanged(
            RoamingPreferredVoiceNetwork::CELLULAR);
}

TEST_F(AosHandleTest, MsgToString_Test)
{
    EXPECT_STREQ(MsgToString(0 /*HANDLE_MSG_BLOCK_STATUS*/), "HANDLE_MSG_BLOCK_STATUS");
    EXPECT_STREQ(MsgToString(1 /*HANDLE_MSG_APP_STATUS*/), "HANDLE_MSG_APP_STATUS");
    EXPECT_STREQ(MsgToString(2 /*HANDLE_MSG_INVALID*/), "__INVALID__");
}

TEST_F(AosHandleTest, RadioTypeToString_Test)
{
    EXPECT_STREQ(RadioTypeToString(NW_REPORT_RADIO_WLAN), "WLAN");
    EXPECT_STREQ(RadioTypeToString(NW_REPORT_RADIO_LTE), "LTE");
    EXPECT_STREQ(RadioTypeToString(NW_REPORT_RADIO_NR), "NR");
    EXPECT_STREQ(RadioTypeToString(NW_REPORT_RADIO_CDMA), "__INVALID__");
}

TEST_F(AosHandleTest, ServiceTypeToString_Test)
{
    SetServiceType(ImsAosService::MTC);
    EXPECT_STREQ(ServiceTypeToString(), "mtc");

    SetServiceType(ImsAosService::MTS);
    EXPECT_STREQ(ServiceTypeToString(), "mts");

    SetServiceType(ImsAosService::EMERGENCY_MTC);
    EXPECT_STREQ(ServiceTypeToString(), "mtc");

    SetServiceType(ImsAosService::EMERGENCY_MTS);
    EXPECT_STREQ(ServiceTypeToString(), "mts");

    SetServiceType(ImsAosService::UCE);
    EXPECT_STREQ(ServiceTypeToString(), "uce");

    SetServiceType(ImsAosService::SIP_CONTROLLER);
    EXPECT_STREQ(ServiceTypeToString(), "sip_controller");

    SetServiceType(ImsAosService::NONE);
    EXPECT_STREQ(ServiceTypeToString(), "invalid");
}