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

#include "AosReason.h"
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
#include "provider/AosString.h"
#include "provider/AosUtil.h"

#include "interface/MockIAosAppContext.h"
#include "interface/MockIAosApplication.h"
#include "interface/MockIAosCallTracker.h"
#include "interface/MockIAosConnection.h"
#include "interface/MockIAosNConfiguration.h"
#include "interface/MockIAosNetTracker.h"
#include "interface/MockIAosRegistration.h"
#include "interface/MockIAosRegStateManager.h"

#include "../../interface/aos/MockIImsAosInfo.h"
#include "../../interface/aos/MockIImsAosListener.h"
#include "../../interface/aos/MockIImsAosMonitor.h"
#include "../../interface/aos/MockIAosService.h"
#include "../../../platform/interface/MockIWifiWatcher.h"

using ::testing::_;
using ::testing::AnyNumber;
using ::testing::Return;
using ::testing::ReturnRef;

#define DECLARE_USING(Base)                        \
    using Base::RemoveBlock;                       \
    using Base::SetSuspendedReason;                \
    using Base::SetHandleState;                    \
    using Base::Init;                              \
    using Base::CleanUp;                           \
    using Base::IsHandleBlocked;                   \
    using Base::SetReason;                         \
    using Base::ClearSuspendedReason;              \
    using Base::GetAppState;                       \
    using Base::GetImsAosReason;                   \
    using Base::GetImsAosReasonForSuspend;         \
    using Base::IsEpdgEnabled;                     \
    using Base::IsEqualNetworkType;                \
    using Base::IsCapabilityExisted;               \
    using Base::IsCapabilityExistedForNetworkType; \
    using Base::IsNetworkTypeMatchedToRat;         \
    using Base::GetNetworkType;                    \
    using Base::GetMobileNetworkType;              \
    using Base::GetMobileChangingNetworkType;      \
    using Base::GetAosFeature;                     \
    using Base::ReevaluateBlocks;                  \
    using Base::UpdateIpcan;                       \
    using Base::PreProcessBlock;                   \
    using Base::ProcessBlock;                      \
    using Base::ProcessFeatureBlock;               \
    using Base::ProcessCheckBlock;                 \
    using Base::ProcessUnavailableFeature;         \
    using Base::ProcessUnavailableFeatureChanged;  \
    using Base::BackupAllBlocks;                   \
    using Base::HoldBlockForInvalidNetwork;        \
    using Base::IsBlockForMobile;                  \
    using Base::IsBlockForWifi;                    \
    using Base::InitializeHoldingBlocksPolicy;     \
    using Base::InitializeFeatureTags;             \
    using Base::ProcessImsSuspended;               \
    using Base::ProcessImsResumed;                 \
    using Base::CheckSuspended;                    \
    using Base::ResetSuspendedReason;              \
    using Base::ReportRegState;                    \
    using Base::ProcessCapabilitiesChanged;        \
    using Base::ProcessNetworkChanged;             \
    using Base::ProcessVopsStateChanged;           \
    using Base::ProcessPsRoamingStateChanged;      \
    using Base::IsSupportedNetworkType;            \
    using Base::IsSupportedNetworkTypeForCellular; \
    using Base::StateToString;                     \
    using Base::MsgToString;                       \
    using Base::RadioTypeToString;                 \
    using Base::ServiceTypeToString;               \
    using Base::ReevaluateUnavailableFeature;      \
    using Base::Is3G;                              \
    using Base::IsEmergencyService;                \
    using Base::IsRoaming;                         \
    using Base::StateConnecting;                   \
    using Base::StateConnected;                    \
    using Base::StateDisconnecting;

class TestAosHandle : public AosHandle
{
public:
    DECLARE_USING(AosHandle)

    inline TestAosHandle(IN IAosAppContext* piAppContext, IN const AString& strAppId,
            IN const AString& strServiceId, IN const IMS_UINT32 nServiceType) :
            AosHandle(piAppContext, strAppId, strServiceId, nServiceType)
    {
    }

    IMS_BOOL IsHoldingBlockForMobile(IN IMS_UINT32 nBlock)
    {
        return IsHandleBlocked(m_nHoldingBlocksForMobile, nBlock);
    }

    FRIEND_TEST(AosHandleTest, Constructor);
    FRIEND_TEST(AosHandleTest, NetTracker_StatusChanged_Test9);
    FRIEND_TEST(AosHandleTest, IsEmergencyService_Test);
    FRIEND_TEST(AosHandleTest, ProcessImsSuspended_Test4);
    FRIEND_TEST(AosHandleTest, ProcessImsResumed_Test6);

    FRIEND_TEST(AosHandleTest, AddBlock_IsHandleBlocked_Normal);
    FRIEND_TEST(AosHandleTest, AddBlock_Duplicated);
    FRIEND_TEST(AosHandleTest, IsHandleBlocked_ForEach);
    FRIEND_TEST(AosHandleTest, IsHandleBlocked_ForAll);
    FRIEND_TEST(AosHandleTest, NetTracker_StatusChanged_Test2);
    FRIEND_TEST(AosHandleTest, NetTracker_StatusChanged_Test3);
    FRIEND_TEST(AosHandleTest, NetTracker_StatusChanged_Test7);
    FRIEND_TEST(AosHandleTest, BackupAllBlocks_Test1);
    FRIEND_TEST(AosHandleTest, BackupAllBlocks_Test2);
    FRIEND_TEST(AosHandleTest, BackupAllBlocks_Test3);
    FRIEND_TEST(AosHandleTest, BackupAllBlocks_Test4);
    FRIEND_TEST(AosHandleTest, BackupBlocks_Test);
    FRIEND_TEST(AosHandleTest, HoldBlockForInvalidNetwork_Test2);
    FRIEND_TEST(AosHandleTest, HoldBlockForInvalidNetwork_Test5);
    FRIEND_TEST(AosHandleTest, HoldBlockForInvalidNetwork_Test6);
    FRIEND_TEST(AosHandleTest, ReevaluateBlocks_Test1);
    FRIEND_TEST(AosHandleTest, ReevaluateBlocks_Test5);
    FRIEND_TEST(AosHandleTest, ProcessBlock_Test3);
    FRIEND_TEST(AosHandleTest, ProcessBlock_Test5);
    FRIEND_TEST(AosHandleTest, ProcessBlock_Test7);
    FRIEND_TEST(AosHandleTest, ProcessBlock_Test9);
    FRIEND_TEST(AosHandleTest, ProcessBlock_Test11);
    FRIEND_TEST(AosHandleTest, ProcessBlock_Test13);
    FRIEND_TEST(AosHandleTest, ProcessCheckBlock_Test1);
    FRIEND_TEST(AosHandleTest, ProcessCheckBlock_Test3);

    FRIEND_TEST(AosHandleTest, RequestAppToRegisterWithNextPcscf);

public:
    void SetState(IN IMS_UINT32 nState) { AosHandle::SetState(nState); }
    IMS_UINT32 GetState() { return AosHandle::GetState(); }
};

class AosHandleTest : public ::testing::Test
{
public:
    TestAosHandle* m_pAosHandle;

    MockIAosAppContext m_objMockIAosAppContext;
    MockIAosApplication m_objMockIAosApplication;
    MockIAosNetTracker m_objMockIAosNetTracker;
    MockIImsAosListener m_objMockIImsAosListener;
    MockIAosConnection m_objMockIAosConnection;
    MockIWifiWatcher m_objMockIWifiWatcher;

    IAosNConfiguration* m_piAosNConfiguration;
    MockIAosNConfiguration m_objMockIAosNConfiguration;

    IAosService* m_piAosService;
    MockIAosService m_objMockIAosService;

    IAosRegStateManager* m_piAosRegStateManager;
    MockIAosRegStateManager m_objMockIAosRegStateManager;

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
        AosProvider::GetInstance()->SetNConfiguration(&m_objMockIAosNConfiguration);

        m_piAosService = AosProvider::GetInstance()->GetService();
        AosProvider::GetInstance()->SetService(&m_objMockIAosService);

        m_piAosRegStateManager = AosProvider::GetInstance()->GetRegStateManager();
        AosProvider::GetInstance()->SetRegStateManager(&m_objMockIAosRegStateManager);

        m_pAosHandle = new TestAosHandle(
                &m_objMockIAosAppContext, m_strAppId, m_strServiceId, m_nServiceType);

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

    void AddHoldingBlockForMobile(IN IMS_UINT32 nBlock)
    {
        m_pAosHandle->AddBlock(nBlock, m_pAosHandle->m_nHoldingBlocksForMobile);
    }

    void AddHoldingBlockForWifi(IN IMS_UINT32 nBlock)
    {
        m_pAosHandle->AddBlock(nBlock, m_pAosHandle->m_nHoldingBlocksForWifi);
    }

    void ClearBlocks() { m_pAosHandle->m_nBlocks = 0; }

    void ClearHoldingBlocksForMobile() { m_pAosHandle->m_nHoldingBlocksForMobile = 0; }

    void ClearHoldingBlocksForWifi() { m_pAosHandle->m_nHoldingBlocksForWifi = 0; }

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

    void SetNetSrvIn(IN IMS_BOOL bNetSrvIn) { m_pAosHandle->m_bNetSrvIn = bNetSrvIn; }

    IMS_BOOL GetNetSrvIn() { return m_pAosHandle->m_bNetSrvIn; }

    IImsAosInfo* GetAosInfo() { return m_pAosHandle->m_piInfo; }

    IMS_UINT32 GetReason() { return m_pAosHandle->m_nReason; }

    void SetEpdgEnabled(IN IMS_BOOL bEnabled) { m_pAosHandle->m_bEpdgEnabled = bEnabled; }

    void SetRegFeatureTagRequired(IN IMS_BOOL bRequired)
    {
        m_pAosHandle->m_bRegFeatureTagRequired = bRequired;
    }

    void SetFeatureTagList(IN const AosFeatureTagList& objFeatureTagList)
    {
        m_pAosHandle->m_objFeatureTagList = objFeatureTagList;
    }

    void SetBindedFeatureTagList(IN const AosFeatureTagList& objBindedFeatureTagList)
    {
        m_pAosHandle->m_objBindedFeatureTagList = objBindedFeatureTagList;
    }

    ImsList<IMS_UINT32> GetHoldingBlocksPolicyForMobile()
    {
        return m_pAosHandle->m_objHoldingBlocksPolicyForMobile;
    }

    void SetHoldingBlocksPolicyForMobile()
    {
        m_pAosHandle->m_objHoldingBlocksPolicyForMobile.Append(AosHandle::BLOCK_VOLTE_CAPABILITY);
        m_pAosHandle->m_objHoldingBlocksPolicyForMobile.Append(AosHandle::BLOCK_VILTE_CAPABILITY);
        m_pAosHandle->m_objHoldingBlocksPolicyForMobile.Append(AosHandle::BLOCK_VOPS);
        m_pAosHandle->m_objHoldingBlocksPolicyForMobile.Append(AosHandle::BLOCK_NETWORK);
    }

    ImsList<IMS_UINT32> GetHoldingBlocksPolicyForWifi()
    {
        return m_pAosHandle->m_objHoldingBlocksPolicyForWifi;
    }

    void SetHoldingBlocksPolicyForWifi()
    {
        m_pAosHandle->m_objHoldingBlocksPolicyForWifi.Append(AosHandle::BLOCK_VOWIFI_CAPABILITY);
        m_pAosHandle->m_objHoldingBlocksPolicyForWifi.Append(AosHandle::BLOCK_VIWIFI_CAPABILITY);
    }

    void SetHoldingBlocksPolicyForTest()
    {
        m_pAosHandle->m_objHoldingBlocksPolicyForMobile.Append(AosHandle::BLOCK_SMS_CAPABILITY);
        m_pAosHandle->m_objHoldingBlocksPolicyForWifi.Append(AosHandle::BLOCK_SMS_CAPABILITY);
    }

    void ClearHoldingBlocksPolicyForMobile()
    {
        m_pAosHandle->m_objHoldingBlocksPolicyForMobile.Clear();
    }

    void ClearHoldingBlocksPolicyForWifi()
    {
        m_pAosHandle->m_objHoldingBlocksPolicyForWifi.Clear();
    }

    void SetBlocked(IMS_BOOL bBlocked) { m_pAosHandle->m_bBlocked = bBlocked; }

    IMS_UINT32 GetUnavailableFeatures()
    {
        return m_pAosHandle->m_objFeatureTagList.GetUnavailableFeatures();
    }

    IMS_UINT32 GetBindedUnavailableFeatures()
    {
        return m_pAosHandle->m_objBindedFeatureTagList.GetUnavailableFeatures();
    }

    void BackupBlocksForMobile()
    {
        m_pAosHandle->BackupBlocks(m_pAosHandle->m_objHoldingBlocksPolicyForMobile,
                m_pAosHandle->m_nHoldingBlocksForMobile);
    }

    void BackupBlocksForWifi()
    {
        m_pAosHandle->BackupBlocks(m_pAosHandle->m_objHoldingBlocksPolicyForWifi,
                m_pAosHandle->m_nHoldingBlocksForWifi);
    }

    void RestoreBlocksForMobile()
    {
        m_pAosHandle->RestoreBlocks(m_pAosHandle->m_objHoldingBlocksPolicyForMobile,
                m_pAosHandle->m_nHoldingBlocksForMobile);
    }

    void RestoreBlocksForWifi()
    {
        m_pAosHandle->RestoreBlocks(m_pAosHandle->m_objHoldingBlocksPolicyForWifi,
                m_pAosHandle->m_nHoldingBlocksForWifi);
    }

    IMS_BOOL IsBlocked() const { return m_pAosHandle->m_bBlocked; }

    IMS_BOOL HasFeatureTag(IN const AString& strName, IN const AString& strValue) const
    {
        return m_pAosHandle->m_objFeatureTagList.HasFeatureTag(strName, strValue);
    }

    IMS_BOOL AddFeatureTag(IN const AString& strName)
    {
        return m_pAosHandle->m_objFeatureTagList.AddFeatureTag(strName);
    }

    void SetSuspendedReasonForTest(IN IMS_UINT32 nReason)
    {
        m_pAosHandle->m_nSuspendedReason = nReason;
    }

    void SetServiceType(IN IMS_UINT32 nType) { m_pAosHandle->m_nServiceType = nType; }

    void SetDataConnected(IN IMS_BOOL bConnected) { m_pAosHandle->m_bDataConnected = bConnected; }

    void SetWifiWatcher(IN IWifiWatcher* piWifiWatcher)
    {
        m_pAosHandle->m_piWifiWatcher = piWifiWatcher;
    }

    ImsMap<IMS_UINT32, IMS_UINT32> GetCapabilities() { return m_pAosHandle->m_objCapabilities; }

    void SetCapabilities(IN const ImsMap<IMS_UINT32, IMS_UINT32>& objNewCapabilities)
    {
        m_pAosHandle->m_objCapabilities = objNewCapabilities;
    }

    void SetRoamingState(IN IMS_UINT32 nState) { m_pAosHandle->m_nRoamingState = nState; }

    IMS_BOOL IsCsVoiceAvailable() { return m_pAosHandle->m_bCsVoiceAvailable; }
};

TEST_F(AosHandleTest, Constructor)
{
    MockIAosAppContext objMockIAosAppContext;
    MockIAosNConfiguration objMockIAosNConfiguration;

    EXPECT_CALL(objMockIAosAppContext, GetSlotId()).Times(AnyNumber()).WillRepeatedly(Return(0));

    const AString strValue = AString("test");
    EXPECT_CALL(objMockIAosAppContext, GetProfileId())
            .Times(AnyNumber())
            .WillRepeatedly(ReturnRef(strValue));

    IAosNConfiguration* piAosNConfiguration = AosProvider::GetInstance()->GetNConfiguration();
    AosProvider::GetInstance()->SetNConfiguration(
            static_cast<IAosNConfiguration*>(&objMockIAosNConfiguration));

    EXPECT_CALL(objMockIAosNConfiguration, SetListener(_)).Times(1);

    TestAosHandle* pTestAosHandle =
            new TestAosHandle(static_cast<IAosAppContext*>(&objMockIAosAppContext), m_strAppId,
                    m_strServiceId, m_nServiceType);

    ASSERT_TRUE(pTestAosHandle != nullptr);

    EXPECT_EQ(pTestAosHandle->GetState(), AosHandle::STATE_DISCONNECTED);

    delete pTestAosHandle;

    AosProvider::GetInstance()->SetNConfiguration(piAosNConfiguration);
}

TEST_F(AosHandleTest, Destructor)
{
    EXPECT_CALL(m_objMockIAosNConfiguration, RemoveListener(m_pAosHandle)).Times(1);
}

TEST_F(AosHandleTest, AddBlock_IsHandleBlocked_Normal)
{
    IMS_UINT32 nTestBlocks = AosHandle::BLOCK_NONE;

    EXPECT_FALSE(m_pAosHandle->IsHandleBlocked(nTestBlocks, AosHandle::BLOCK_VOLTE_CAPABILITY));
    EXPECT_FALSE(m_pAosHandle->IsHandleBlocked(nTestBlocks, AosHandle::BLOCK_VILTE_CAPABILITY));

    m_pAosHandle->AddBlock(AosHandle::BLOCK_VOLTE_CAPABILITY, nTestBlocks);
    EXPECT_EQ(nTestBlocks, AosHandle::BLOCK_VOLTE_CAPABILITY);
    EXPECT_TRUE(m_pAosHandle->IsHandleBlocked(nTestBlocks, AosHandle::BLOCK_VOLTE_CAPABILITY));
    EXPECT_FALSE(m_pAosHandle->IsHandleBlocked(nTestBlocks, AosHandle::BLOCK_VILTE_CAPABILITY));

    m_pAosHandle->AddBlock(AosHandle::BLOCK_VILTE_CAPABILITY, nTestBlocks);
    EXPECT_EQ(nTestBlocks, (AosHandle::BLOCK_VOLTE_CAPABILITY | AosHandle::BLOCK_VILTE_CAPABILITY));
    EXPECT_TRUE(m_pAosHandle->IsHandleBlocked(nTestBlocks, AosHandle::BLOCK_VOLTE_CAPABILITY));
    EXPECT_TRUE(m_pAosHandle->IsHandleBlocked(nTestBlocks, AosHandle::BLOCK_VILTE_CAPABILITY));
}

TEST_F(AosHandleTest, AddBlock_Duplicated)
{
    IMS_UINT32 nTestBlocks = AosHandle::BLOCK_NONE;

    m_pAosHandle->AddBlock(AosHandle::BLOCK_VOLTE_CAPABILITY, nTestBlocks);
    m_pAosHandle->AddBlock(AosHandle::BLOCK_VOLTE_CAPABILITY, nTestBlocks);
    m_pAosHandle->AddBlock(AosHandle::BLOCK_VOLTE_CAPABILITY, nTestBlocks);

    EXPECT_EQ(nTestBlocks, AosHandle::BLOCK_VOLTE_CAPABILITY);
}

TEST_F(AosHandleTest, RemoveBlock_IsHandleBlocked_Normal)
{
    IMS_UINT32 nTestBlocks =
            (AosHandle::BLOCK_VOLTE_CAPABILITY | AosHandle::BLOCK_VILTE_CAPABILITY);

    EXPECT_TRUE(m_pAosHandle->IsHandleBlocked(nTestBlocks, AosHandle::BLOCK_VOLTE_CAPABILITY));
    EXPECT_TRUE(m_pAosHandle->IsHandleBlocked(nTestBlocks, AosHandle::BLOCK_VILTE_CAPABILITY));

    m_pAosHandle->RemoveBlock(AosHandle::BLOCK_VOLTE_CAPABILITY, nTestBlocks);
    EXPECT_EQ(nTestBlocks, AosHandle::BLOCK_VILTE_CAPABILITY);
    EXPECT_FALSE(m_pAosHandle->IsHandleBlocked(nTestBlocks, AosHandle::BLOCK_VOLTE_CAPABILITY));
    EXPECT_TRUE(m_pAosHandle->IsHandleBlocked(nTestBlocks, AosHandle::BLOCK_VILTE_CAPABILITY));

    m_pAosHandle->RemoveBlock(AosHandle::BLOCK_VILTE_CAPABILITY, nTestBlocks);
    EXPECT_EQ(nTestBlocks, AosHandle::BLOCK_NONE);
    EXPECT_FALSE(m_pAosHandle->IsHandleBlocked(nTestBlocks, AosHandle::BLOCK_VOLTE_CAPABILITY));
    EXPECT_FALSE(m_pAosHandle->IsHandleBlocked(nTestBlocks, AosHandle::BLOCK_VILTE_CAPABILITY));
}

TEST_F(AosHandleTest, RemoveBlock_NotExisted)
{
    IMS_UINT32 nTestBlocks =
            (AosHandle::BLOCK_VOLTE_CAPABILITY | AosHandle::BLOCK_VILTE_CAPABILITY);

    m_pAosHandle->RemoveBlock(AosHandle::BLOCK_VOLTE_CAPABILITY, nTestBlocks);
    m_pAosHandle->RemoveBlock(AosHandle::BLOCK_VOLTE_CAPABILITY, nTestBlocks);
    m_pAosHandle->RemoveBlock(AosHandle::BLOCK_VOLTE_CAPABILITY, nTestBlocks);

    EXPECT_EQ(nTestBlocks, AosHandle::BLOCK_VILTE_CAPABILITY);
}

TEST_F(AosHandleTest, IsHandleBlocked_ForEach)
{
    ClearBlocks();
    m_pAosHandle->AddBlock(AosHandle::BLOCK_VOLTE_CAPABILITY, m_pAosHandle->m_nBlocks);
    m_pAosHandle->AddBlock(AosHandle::BLOCK_VILTE_CAPABILITY, m_pAosHandle->m_nBlocks);
    m_pAosHandle->AddBlock(AosHandle::BLOCK_SMS_CAPABILITY, m_pAosHandle->m_nBlocks);
    m_pAosHandle->AddBlock(AosHandle::BLOCK_VOPS, m_pAosHandle->m_nBlocks);
    m_pAosHandle->AddBlock(
            AosHandle::BLOCK_SMS_OVER_IP_NETWORK_INDICATION, m_pAosHandle->m_nBlocks);
    m_pAosHandle->AddBlock(AosHandle::BLOCK_VOWIFI_CAPABILITY, m_pAosHandle->m_nBlocks);
    m_pAosHandle->AddBlock(AosHandle::BLOCK_VIWIFI_CAPABILITY, m_pAosHandle->m_nBlocks);
    EXPECT_TRUE(m_pAosHandle->IsHandleBlocked(AosHandle::BLOCK_VOLTE_CAPABILITY));
    EXPECT_TRUE(m_pAosHandle->IsHandleBlocked(AosHandle::BLOCK_VILTE_CAPABILITY));
    EXPECT_TRUE(m_pAosHandle->IsHandleBlocked(AosHandle::BLOCK_SMS_CAPABILITY));
    EXPECT_TRUE(m_pAosHandle->IsHandleBlocked(AosHandle::BLOCK_VOPS));
    EXPECT_TRUE(m_pAosHandle->IsHandleBlocked(AosHandle::BLOCK_SMS_OVER_IP_NETWORK_INDICATION));
    EXPECT_TRUE(m_pAosHandle->IsHandleBlocked(AosHandle::BLOCK_VOWIFI_CAPABILITY));
    EXPECT_TRUE(m_pAosHandle->IsHandleBlocked(AosHandle::BLOCK_VIWIFI_CAPABILITY));

    ClearBlocks();
    EXPECT_FALSE(m_pAosHandle->IsHandleBlocked(AosHandle::BLOCK_VOLTE_CAPABILITY));
    EXPECT_FALSE(m_pAosHandle->IsHandleBlocked(AosHandle::BLOCK_VILTE_CAPABILITY));
    EXPECT_FALSE(m_pAosHandle->IsHandleBlocked(AosHandle::BLOCK_SMS_CAPABILITY));
    EXPECT_FALSE(m_pAosHandle->IsHandleBlocked(AosHandle::BLOCK_VOPS));
    EXPECT_FALSE(m_pAosHandle->IsHandleBlocked(AosHandle::BLOCK_SMS_OVER_IP_NETWORK_INDICATION));
    EXPECT_FALSE(m_pAosHandle->IsHandleBlocked(AosHandle::BLOCK_VOWIFI_CAPABILITY));
    EXPECT_FALSE(m_pAosHandle->IsHandleBlocked(AosHandle::BLOCK_VIWIFI_CAPABILITY));
}

TEST_F(AosHandleTest, IsHandleBlocked_ForAll)
{
    ClearBlocks();
    m_pAosHandle->AddBlock(AosHandle::BLOCK_VOLTE_CAPABILITY, m_pAosHandle->m_nBlocks);
    EXPECT_TRUE(m_pAosHandle->IsHandleBlocked());

    ClearBlocks();
    EXPECT_FALSE(m_pAosHandle->IsHandleBlocked());
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
    m_pAosHandle->SetState(AosHandle::STATE_INVALID);

    m_pAosHandle->ProcessFeatureTagChange();
}

TEST_F(AosHandleTest, ProcessFeatureTagChange_STATE_DISCONNECTED)
{
    EXPECT_CALL(m_objMockIAosApplication, Reconfig()).Times(0);

    ClearFeatureTagList();
    ClearBindedFeatureTagList();
    AddFeature(ImsAosFeature::MMTEL);
    m_pAosHandle->SetState(AosHandle::STATE_DISCONNECTED);

    m_pAosHandle->ProcessFeatureTagChange();
}

TEST_F(AosHandleTest, ProcessFeatureTagChange_STATE_DISCONNECTING)
{
    EXPECT_CALL(m_objMockIAosApplication, Reconfig()).Times(0);

    ClearFeatureTagList();
    ClearBindedFeatureTagList();
    AddFeature(ImsAosFeature::MMTEL);
    m_pAosHandle->SetState(AosHandle::STATE_DISCONNECTING);

    m_pAosHandle->ProcessFeatureTagChange();
}

TEST_F(AosHandleTest, ProcessFeatureTagChange_STATE_CONNECTING)
{
    EXPECT_CALL(m_objMockIAosApplication, Reconfig()).Times(1);

    ClearFeatureTagList();
    ClearBindedFeatureTagList();
    AddFeature(ImsAosFeature::MMTEL);
    m_pAosHandle->SetState(AosHandle::STATE_CONNECTING);

    m_pAosHandle->ProcessFeatureTagChange();
}

TEST_F(AosHandleTest, ProcessFeatureTagChange_STATE_CONNECTED)
{
    EXPECT_CALL(m_objMockIAosApplication, Reconfig()).Times(1);

    ClearFeatureTagList();
    ClearBindedFeatureTagList();
    AddFeature(ImsAosFeature::MMTEL);
    m_pAosHandle->SetState(AosHandle::STATE_CONNECTED);

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

    m_pAosHandle->SetState(AosHandle::STATE_DISCONNECTED);
    EXPECT_CALL(m_objMockIImsAosListener, ImsAos_Disconnected(_)).Times(0);
    EXPECT_FALSE(m_pAosHandle->App_Notify());

    m_pAosHandle->SetState(AosHandle::STATE_CONNECTING);
    EXPECT_FALSE(m_pAosHandle->App_Notify());

    m_pAosHandle->SetState(AosHandle::STATE_CONNECTED);
    EXPECT_CALL(m_objMockIImsAosListener, ImsAos_Connected(_, _)).Times(0);
    EXPECT_FALSE(m_pAosHandle->App_Notify());

    m_pAosHandle->SetState(AosHandle::STATE_DISCONNECTING);
    EXPECT_CALL(m_objMockIImsAosListener, ImsAos_Disconnecting(_)).Times(0);
    EXPECT_FALSE(m_pAosHandle->App_Notify());
}

TEST_F(AosHandleTest, App_Notify_No_Notify)
{
    m_pAosHandle->SetListener(static_cast<IImsAosListener*>(&m_objMockIImsAosListener));
    ASSERT_NE(GetListener(), nullptr);

    SetNotify(IMS_FALSE);
    ASSERT_FALSE(GetNotify());

    m_pAosHandle->SetState(AosHandle::STATE_DISCONNECTED);
    EXPECT_CALL(m_objMockIImsAosListener, ImsAos_Disconnected(_)).Times(0);
    EXPECT_FALSE(m_pAosHandle->App_Notify());

    m_pAosHandle->SetState(AosHandle::STATE_CONNECTING);
    EXPECT_FALSE(m_pAosHandle->App_Notify());

    m_pAosHandle->SetState(AosHandle::STATE_CONNECTED);
    EXPECT_CALL(m_objMockIImsAosListener, ImsAos_Connected(_, _)).Times(0);
    EXPECT_FALSE(m_pAosHandle->App_Notify());

    m_pAosHandle->SetState(AosHandle::STATE_DISCONNECTING);
    EXPECT_CALL(m_objMockIImsAosListener, ImsAos_Disconnecting(_)).Times(0);
    EXPECT_FALSE(m_pAosHandle->App_Notify());
}

TEST_F(AosHandleTest, App_Notify_STATE_INVALID)
{
    m_pAosHandle->SetListener(static_cast<IImsAosListener*>(&m_objMockIImsAosListener));
    ASSERT_NE(GetListener(), nullptr);

    SetNotify(IMS_TRUE);
    ASSERT_TRUE(GetNotify());

    m_pAosHandle->SetState(AosHandle::STATE_INVALID);
    EXPECT_FALSE(m_pAosHandle->App_Notify());
}

TEST_F(AosHandleTest, App_Notify_STATE_DISCONNECTED)
{
    m_pAosHandle->SetListener(static_cast<IImsAosListener*>(&m_objMockIImsAosListener));
    ASSERT_NE(GetListener(), nullptr);

    SetNotify(IMS_TRUE);
    ASSERT_TRUE(GetNotify());

    m_pAosHandle->SetState(AosHandle::STATE_DISCONNECTED);
    EXPECT_CALL(m_objMockIImsAosListener, ImsAos_Disconnected(_)).Times(1);
    EXPECT_TRUE(m_pAosHandle->App_Notify());
}

TEST_F(AosHandleTest, App_Notify_STATE_CONNECTING)
{
    m_pAosHandle->SetListener(static_cast<IImsAosListener*>(&m_objMockIImsAosListener));
    ASSERT_NE(GetListener(), nullptr);

    SetNotify(IMS_TRUE);
    ASSERT_TRUE(GetNotify());

    m_pAosHandle->SetState(AosHandle::STATE_CONNECTING);
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

    m_pAosHandle->SetState(AosHandle::STATE_CONNECTED);
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

    m_pAosHandle->SetState(AosHandle::STATE_DISCONNECTING);
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
    m_pAosHandle->SetState(AosHandle::STATE_DISCONNECTED);
    EXPECT_EQ(m_pAosHandle->GetFeatures(), ImsAosFeature::NONE);
}

TEST_F(AosHandleTest, GetFeatures_Mmtel_Video_Binded_And_No_Unavailable)
{
    m_pAosHandle->SetState(AosHandle::STATE_CONNECTED);

    AddBindedFeature(ImsAosFeature::MMTEL);
    AddBindedFeature(ImsAosFeature::VIDEO);

    EXPECT_EQ(m_pAosHandle->GetFeatures(), (ImsAosFeature::MMTEL | ImsAosFeature::VIDEO));
}

TEST_F(AosHandleTest, GetFeatures_Mmtel_Video_Binded_And_Mmtel_Unavailable)
{
    m_pAosHandle->SetState(AosHandle::STATE_CONNECTED);

    AddBindedFeature(ImsAosFeature::MMTEL);
    AddBindedFeature(ImsAosFeature::VIDEO);
    AddUnavailableFeature(ImsAosFeature::MMTEL);

    EXPECT_EQ(m_pAosHandle->GetFeatures(), ImsAosFeature::VIDEO);
}

TEST_F(AosHandleTest, GetFeatures_Mmtel_Binded_And_Mmtel_Unavailable)
{
    m_pAosHandle->SetState(AosHandle::STATE_CONNECTED);

    AddBindedFeature(ImsAosFeature::MMTEL);
    AddUnavailableFeature(ImsAosFeature::MMTEL);

    EXPECT_EQ(m_pAosHandle->GetFeatures(), ImsAosFeature::NONE);
}

TEST_F(AosHandleTest, SetSuspendedReason_GetSuspendedReason)
{
    m_pAosHandle->SetSuspendedReason(AosReason::SUSPEND_CS_CALL);
    EXPECT_EQ(m_pAosHandle->GetSuspendedReason(), AosReason::SUSPEND_NONE);

    m_pAosHandle->SetSuspendedReason(AosReason::SUSPEND_LOW_BATTERY);
    EXPECT_EQ(m_pAosHandle->GetSuspendedReason(), AosReason::SUSPEND_NONE);

    m_pAosHandle->SetSuspendedReason(AosReason::SUSPEND_NO_LTE_COVERAGE);
    EXPECT_EQ(m_pAosHandle->GetSuspendedReason(), AosReason::SUSPEND_NONE);

    m_pAosHandle->SetSuspendedReason(AosReason::SUSPEND_INSTANTANEOUS_OFFLINE);
    EXPECT_EQ(m_pAosHandle->GetSuspendedReason(), AosReason::SUSPEND_NONE);

    m_pAosHandle->SetSuspendedReason(AosReason::SUSPEND_NO_SERVICE);
    EXPECT_EQ(m_pAosHandle->GetSuspendedReason(), AosReason::SUSPEND_NO_SERVICE);

    m_pAosHandle->SetSuspendedReason(AosReason::SUSPEND_CS_CALL);
    EXPECT_EQ(m_pAosHandle->GetSuspendedReason(), AosReason::SUSPEND_NO_SERVICE);

    m_pAosHandle->SetSuspendedReason(AosReason::SUSPEND_LOW_BATTERY);
    EXPECT_EQ(m_pAosHandle->GetSuspendedReason(), AosReason::SUSPEND_NO_SERVICE);

    m_pAosHandle->SetSuspendedReason(AosReason::SUSPEND_NO_LTE_COVERAGE);
    EXPECT_EQ(m_pAosHandle->GetSuspendedReason(), AosReason::SUSPEND_NO_SERVICE);

    m_pAosHandle->SetSuspendedReason(AosReason::SUSPEND_INSTANTANEOUS_OFFLINE);
    EXPECT_EQ(m_pAosHandle->GetSuspendedReason(), AosReason::SUSPEND_NO_SERVICE);
}

TEST_F(AosHandleTest, IsFeatureConnected_Test)
{
    m_pAosHandle->SetState(AosHandle::STATE_CONNECTED);
    AddBindedFeature(ImsAosFeature::MMTEL);

    EXPECT_TRUE(m_pAosHandle->IsFeatureConnected(ImsAosFeature::MMTEL));
    EXPECT_FALSE(m_pAosHandle->IsFeatureConnected(ImsAosFeature::VIDEO));
}

TEST_F(AosHandleTest, IsImsConnected_Test)
{
    m_pAosHandle->SetState(AosHandle::STATE_DISCONNECTED);
    EXPECT_FALSE(m_pAosHandle->IsImsConnected());

    m_pAosHandle->SetState(AosHandle::STATE_CONNECTING);
    EXPECT_FALSE(m_pAosHandle->IsImsConnected());

    m_pAosHandle->SetState(AosHandle::STATE_CONNECTED);
    EXPECT_TRUE(m_pAosHandle->IsImsConnected());

    m_pAosHandle->SetState(AosHandle::STATE_DISCONNECTING);
    EXPECT_FALSE(m_pAosHandle->IsImsConnected());
}

TEST_F(AosHandleTest, IsImsSuspended_Test)
{
    EXPECT_FALSE(m_pAosHandle->IsImsSuspended());

    m_pAosHandle->SetSuspendedReason(AosReason::SUSPEND_NO_SERVICE);
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

TEST_F(AosHandleTest, NetTracker_StatusChanged_Test1)
{
    // Test1: Srv OUT->IN->OUT / Data and epdg connection not changed
    // Expectation: ImsSuspended false if Srv IN, ImsSuspended true if Srv OUT

    m_pAosHandle->SetState(AosHandle::STATE_CONNECTED);

    EXPECT_CALL(m_objMockIAosNetTracker, IsSuspended())
            .Times(2)
            .WillOnce(Return(IMS_FALSE))
            .WillOnce(Return(IMS_TRUE));

    EXPECT_CALL(m_objMockIAosConnection, IsEpdgEnabled())
            .Times(AnyNumber())
            .WillRepeatedly(Return(IMS_FALSE));

    EXPECT_CALL(m_objMockIAosConnection, GetState())
            .Times(AnyNumber())
            .WillRepeatedly(Return(IAosConnection::STATE_IDLE));

    m_pAosHandle->NetTracker_StatusChanged();
    EXPECT_FALSE(m_pAosHandle->IsImsSuspended());

    m_pAosHandle->NetTracker_StatusChanged();
    EXPECT_TRUE(m_pAosHandle->IsImsSuspended());
}

TEST_F(AosHandleTest, NetTracker_StatusChanged_Test2)
{
    // Test2: Data disconnected -> connected -> disconnected
    //        Wfc available / Epdg not enabled / invalid network
    // Expectation: Wifi locks are backed up if data is changed to connected.
    //              HoldingBlocksForMobile are reevaluated if data is changed to connected.
    //              All blocks are backed up if data is changed to disconnected.

    m_pAosHandle->SetState(AosHandle::STATE_CONNECTED);
    SetNetSrvIn(IMS_TRUE);

    SetHoldingBlocksPolicyForMobile();
    SetHoldingBlocksPolicyForWifi();

    EXPECT_CALL(m_objMockIAosNConfiguration, IsWfcImsAvailable())
            .Times(AnyNumber())
            .WillRepeatedly(Return(IMS_TRUE));

    SetWifiWatcher(static_cast<IWifiWatcher*>(&m_objMockIWifiWatcher));
    EXPECT_CALL(m_objMockIWifiWatcher, GetState())
            .Times(AnyNumber())
            .WillRepeatedly(Return(IWifiWatcher::STATE_DISCONNECTED));

    EXPECT_CALL(m_objMockIAosNetTracker, GetMobileNetworkType())
            .Times(AnyNumber())
            .WillRepeatedly(Return(NW_REPORT_RADIO_INVALID));

    EXPECT_CALL(m_objMockIAosNetTracker, IsSuspended())
            .Times(AnyNumber())
            .WillRepeatedly(Return(IMS_FALSE));

    EXPECT_CALL(m_objMockIAosConnection, IsEpdgEnabled())
            .Times(AnyNumber())
            .WillRepeatedly(Return(IMS_FALSE));

    EXPECT_CALL(m_objMockIAosConnection, GetState())
            .Times(2)
            .WillOnce(Return(IAosConnection::STATE_ACTIVE))
            .WillOnce(Return(IAosConnection::STATE_IDLE));

    AddHoldingBlockForMobile(AosHandle::BLOCK_VOPS);
    m_pAosHandle->AddBlock(AosHandle::BLOCK_VOWIFI_CAPABILITY, m_pAosHandle->m_nBlocks);

    m_pAosHandle->NetTracker_StatusChanged();
    EXPECT_FALSE(m_pAosHandle->IsHoldingBlockForMobile(AosHandle::BLOCK_VOPS));
    EXPECT_TRUE(m_pAosHandle->IsHandleBlocked(AosHandle::BLOCK_VOPS));
    EXPECT_TRUE(IsHoldingBlockForWifi(AosHandle::BLOCK_VOWIFI_CAPABILITY));
    EXPECT_FALSE(m_pAosHandle->IsHandleBlocked(AosHandle::BLOCK_VOWIFI_CAPABILITY));

    m_pAosHandle->NetTracker_StatusChanged();
    EXPECT_TRUE(m_pAosHandle->IsHoldingBlockForMobile(AosHandle::BLOCK_VOPS));
    EXPECT_TRUE(IsHoldingBlockForWifi(AosHandle::BLOCK_VOWIFI_CAPABILITY));
    EXPECT_FALSE(m_pAosHandle->IsHandleBlocked(AosHandle::BLOCK_VOPS));
    EXPECT_FALSE(m_pAosHandle->IsHandleBlocked(AosHandle::BLOCK_VOWIFI_CAPABILITY));
}

TEST_F(AosHandleTest, NetTracker_StatusChanged_Test3)
{
    // Test3: Epdg not enabled -> enabled -> not enabled
    //        Wfc available / Valid network
    // Expectation: Mobile blocks are holt if epdg is enabled.
    //              Holding wifi blocks are moved to main blocks.

    m_pAosHandle->SetState(AosHandle::STATE_CONNECTED);
    SetNetSrvIn(IMS_TRUE);
    SetDataConnected(IMS_TRUE);

    SetHoldingBlocksPolicyForMobile();
    SetHoldingBlocksPolicyForWifi();

    EXPECT_CALL(m_objMockIAosNConfiguration, IsWfcImsAvailable())
            .Times(AnyNumber())
            .WillRepeatedly(Return(IMS_TRUE));

    SetWifiWatcher(static_cast<IWifiWatcher*>(&m_objMockIWifiWatcher));
    EXPECT_CALL(m_objMockIWifiWatcher, GetState())
            .Times(AnyNumber())
            .WillRepeatedly(Return(IWifiWatcher::STATE_CONNECTED));

    EXPECT_CALL(m_objMockIAosNetTracker, GetMobileNetworkType())
            .Times(AnyNumber())
            .WillRepeatedly(Return(NW_REPORT_RADIO_LTE));

    EXPECT_CALL(m_objMockIAosNetTracker, IsSuspended())
            .Times(AnyNumber())
            .WillRepeatedly(Return(IMS_FALSE));

    EXPECT_CALL(m_objMockIAosConnection, IsEpdgEnabled())
            .Times(2)
            .WillOnce(Return(IMS_TRUE))
            .WillOnce(Return(IMS_FALSE));

    EXPECT_CALL(m_objMockIAosConnection, GetState())
            .Times(AnyNumber())
            .WillRepeatedly(Return(IAosConnection::STATE_ACTIVE));

    AddHoldingBlockForWifi(AosHandle::BLOCK_VOWIFI_CAPABILITY);
    m_pAosHandle->AddBlock(AosHandle::BLOCK_VOPS, m_pAosHandle->m_nBlocks);

    m_pAosHandle->NetTracker_StatusChanged();
    EXPECT_TRUE(m_pAosHandle->IsHoldingBlockForMobile(AosHandle::BLOCK_VOPS));
    EXPECT_FALSE(m_pAosHandle->IsHandleBlocked(AosHandle::BLOCK_VOPS));
    EXPECT_FALSE(IsHoldingBlockForWifi(AosHandle::BLOCK_VOWIFI_CAPABILITY));
    EXPECT_TRUE(m_pAosHandle->IsHandleBlocked(AosHandle::BLOCK_VOWIFI_CAPABILITY));

    m_pAosHandle->NetTracker_StatusChanged();
    EXPECT_FALSE(m_pAosHandle->IsHoldingBlockForMobile(AosHandle::BLOCK_VOPS));
    EXPECT_TRUE(m_pAosHandle->IsHandleBlocked(AosHandle::BLOCK_VOPS));
    EXPECT_TRUE(IsHoldingBlockForWifi(AosHandle::BLOCK_VOWIFI_CAPABILITY));
    EXPECT_FALSE(m_pAosHandle->IsHandleBlocked(AosHandle::BLOCK_VOWIFI_CAPABILITY));
}

TEST_F(AosHandleTest, NetTracker_StatusChanged_Test4)
{
    // Test4: BLOCK_3G test.
    //        IsDeregOn3gNetwork() returns false.
    // Expectation: BLOCK_3G is not blocked

    EXPECT_CALL(m_objMockIAosNConfiguration, IsDeregOn3gNetwork())
            .Times(1)
            .WillOnce(Return(IMS_FALSE));

    m_pAosHandle->NetTracker_StatusChanged();
    EXPECT_FALSE(m_pAosHandle->IsHandleBlocked(AosHandle::BLOCK_3G));
}

TEST_F(AosHandleTest, NetTracker_StatusChanged_Test5)
{
    // Test5: BLOCK_3G test.
    //        IsDeregOn3gNetwork() returns true.
    //        Srv is not In.
    // Expectation: BLOCK_3G is not blocked

    EXPECT_CALL(m_objMockIAosNConfiguration, IsDeregOn3gNetwork())
            .Times(1)
            .WillOnce(Return(IMS_TRUE));

    EXPECT_CALL(m_objMockIAosNetTracker, IsSuspended()).Times(1).WillOnce(Return(IMS_TRUE));

    m_pAosHandle->NetTracker_StatusChanged();
    EXPECT_FALSE(m_pAosHandle->IsHandleBlocked(AosHandle::BLOCK_3G));
}

TEST_F(AosHandleTest, NetTracker_StatusChanged_Test6)
{
    // Test6: BLOCK_3G test.
    //        IsDeregOn3gNetwork() returns true.
    //        Srv is In. Network type is 2G (Not supported, Not 3g).
    // Expectation: BLOCK_3G is not blocked

    EXPECT_CALL(m_objMockIAosNConfiguration, IsDeregOn3gNetwork())
            .Times(1)
            .WillOnce(Return(IMS_TRUE));

    EXPECT_CALL(m_objMockIAosNetTracker, IsSuspended()).Times(1).WillOnce(Return(IMS_FALSE));

    EXPECT_CALL(m_objMockIAosNetTracker, GetNetworkType())
            .Times(AnyNumber())
            .WillRepeatedly(Return(NW_REPORT_RADIO_GSM));

    m_pAosHandle->NetTracker_StatusChanged();
    EXPECT_FALSE(m_pAosHandle->IsHandleBlocked(AosHandle::BLOCK_3G));
}

TEST_F(AosHandleTest, NetTracker_StatusChanged_Test7)
{
    // Test7: BLOCK_3G test.
    //        IsDeregOn3gNetwork() returns true.
    //        Srv is In. Network type is supported one(LTE)
    // Expectation: BLOCK_3G is not blocked regardless of previous state.

    EXPECT_CALL(m_objMockIAosNConfiguration, IsDeregOn3gNetwork())
            .Times(AnyNumber())
            .WillRepeatedly(Return(IMS_TRUE));

    EXPECT_CALL(m_objMockIAosNetTracker, IsSuspended())
            .Times(AnyNumber())
            .WillRepeatedly(Return(IMS_FALSE));

    EXPECT_CALL(m_objMockIAosNetTracker, GetNetworkType())
            .Times(AnyNumber())
            .WillRepeatedly(Return(NW_REPORT_RADIO_LTE));

    EXPECT_FALSE(m_pAosHandle->IsHandleBlocked(AosHandle::BLOCK_3G));
    m_pAosHandle->NetTracker_StatusChanged();
    EXPECT_FALSE(m_pAosHandle->IsHandleBlocked(AosHandle::BLOCK_3G));

    m_pAosHandle->AddBlock(AosHandle::BLOCK_3G, m_pAosHandle->m_nBlocks);
    EXPECT_TRUE(m_pAosHandle->IsHandleBlocked(AosHandle::BLOCK_3G));
    m_pAosHandle->NetTracker_StatusChanged();
    EXPECT_FALSE(m_pAosHandle->IsHandleBlocked(AosHandle::BLOCK_3G));
}

TEST_F(AosHandleTest, NetTracker_StatusChanged_Test8)
{
    // Test8: BLOCK_3G test.
    //        IsDeregOn3gNetwork() returns true.
    //        Srv is In. Network type is 3G(WCDMA)
    // Expectation: BLOCK_3G is blocked.

    EXPECT_CALL(m_objMockIAosNConfiguration, IsDeregOn3gNetwork())
            .Times(AnyNumber())
            .WillRepeatedly(Return(IMS_TRUE));

    EXPECT_CALL(m_objMockIAosNetTracker, IsSuspended())
            .Times(AnyNumber())
            .WillRepeatedly(Return(IMS_FALSE));

    EXPECT_CALL(m_objMockIAosNetTracker, GetNetworkType())
            .Times(AnyNumber())
            .WillRepeatedly(Return(NW_REPORT_RADIO_WCDMA));

    SetDataConnected(IMS_TRUE);

    EXPECT_FALSE(m_pAosHandle->IsHandleBlocked(AosHandle::BLOCK_3G));
    m_pAosHandle->NetTracker_StatusChanged();
    EXPECT_TRUE(m_pAosHandle->IsHandleBlocked(AosHandle::BLOCK_3G));
}

TEST_F(AosHandleTest, NetTracker_StatusChanged_Test9)
{
    // Test9: No process for emergency service
    // Expectation: BLOCK_3G is not blocked.

    MockIAosAppContext objMockIAosAppContext;

    EXPECT_CALL(objMockIAosAppContext, GetSlotId()).Times(AnyNumber()).WillRepeatedly(Return(0));

    const AString strValue = AString("test");
    EXPECT_CALL(objMockIAosAppContext, GetProfileId())
            .Times(AnyNumber())
            .WillRepeatedly(ReturnRef(strValue));

    TestAosHandle* pTestAosHandleEmergencyMtc =
            new TestAosHandle(static_cast<IAosAppContext*>(&objMockIAosAppContext), m_strAppId,
                    m_strServiceId, ImsAosService::EMERGENCY_MTC);

    ASSERT_TRUE(pTestAosHandleEmergencyMtc != nullptr);

    pTestAosHandleEmergencyMtc->NetTracker_StatusChanged();

    EXPECT_FALSE(pTestAosHandleEmergencyMtc->AosHandle::IsHandleBlocked(AosHandle::BLOCK_3G));

    delete pTestAosHandleEmergencyMtc;
}

TEST_F(AosHandleTest, Init_CleanUp)
{
    EXPECT_CALL(m_objMockIAosNetTracker, SetListener(_)).Times(1);
    EXPECT_CALL(m_objMockIAosNetTracker, RemoveListener(_)).Times(1);

    EXPECT_CALL(m_objMockIAosService,
            AddListener(DYNAMIC_CAST(IAosRegistrationControlListener*, m_pAosHandle)))
            .Times(1);
    EXPECT_CALL(m_objMockIAosService,
            AddListener(DYNAMIC_CAST(IAosServiceSettingListener*, m_pAosHandle)))
            .Times(1);
    EXPECT_CALL(m_objMockIAosService,
            RemoveListener(DYNAMIC_CAST(IAosRegistrationControlListener*, m_pAosHandle)))
            .Times(1);
    EXPECT_CALL(m_objMockIAosService,
            RemoveListener(DYNAMIC_CAST(IAosServiceSettingListener*, m_pAosHandle)))
            .Times(1);

    EXPECT_TRUE(GetAosInfo() == nullptr);

    EXPECT_CALL(m_objMockIAosNConfiguration, IsCdmalessFeatureTagRequired())
            .Times(1)
            .WillRepeatedly(Return(IMS_TRUE));

    EXPECT_CALL(m_objMockIAosNConfiguration, IsDeregOn3gNetwork())
            .Times(1)
            .WillRepeatedly(Return(IMS_TRUE));

    m_pAosHandle->Init();

    EXPECT_TRUE(GetAosInfo() != nullptr);
    EXPECT_TRUE(HasFeatureTag(FeatureTags::CDMALESS, AString::ConstNull()));
    EXPECT_TRUE(m_pAosHandle->IsBlockForMobile(AosHandle::BLOCK_3G));
    EXPECT_FALSE(m_pAosHandle->IsBlockForWifi(AosHandle::BLOCK_3G));

    m_pAosHandle->CleanUp();
}

TEST_F(AosHandleTest, SetHandleState_Test)
{
    m_pAosHandle->SetSuspendedReason(AosReason::SUSPEND_NO_SERVICE);
    EXPECT_TRUE(m_pAosHandle->IsImsSuspended());

    m_pAosHandle->SetHandleState(AosHandle::STATE_DISCONNECTED);
    EXPECT_EQ(m_pAosHandle->GetState(), AosHandle::STATE_DISCONNECTED);
    EXPECT_FALSE(m_pAosHandle->IsImsSuspended());

    m_pAosHandle->SetSuspendedReason(AosReason::SUSPEND_NO_SERVICE);
    EXPECT_TRUE(m_pAosHandle->IsImsSuspended());

    m_pAosHandle->SetHandleState(AosHandle::STATE_DISCONNECTING);
    EXPECT_EQ(m_pAosHandle->GetState(), AosHandle::STATE_DISCONNECTING);
    EXPECT_FALSE(m_pAosHandle->IsImsSuspended());

    m_pAosHandle->SetSuspendedReason(AosReason::SUSPEND_NO_SERVICE);
    EXPECT_TRUE(m_pAosHandle->IsImsSuspended());

    m_pAosHandle->SetHandleState(AosHandle::STATE_CONNECTING);
    EXPECT_EQ(m_pAosHandle->GetState(), AosHandle::STATE_CONNECTING);
    EXPECT_TRUE(m_pAosHandle->IsImsSuspended());

    m_pAosHandle->SetHandleState(AosHandle::STATE_CONNECTED);
    EXPECT_EQ(m_pAosHandle->GetState(), AosHandle::STATE_CONNECTED);
    EXPECT_TRUE(m_pAosHandle->IsImsSuspended());
}

TEST_F(AosHandleTest, SetReason_Test)
{
    m_pAosHandle->SetReason(AosReason::NONE);
    EXPECT_EQ(GetReason(), AosReason::NONE);

    m_pAosHandle->SetReason(AosReason::SRV_OUT);
    EXPECT_EQ(GetReason(), AosReason::SRV_OUT);

    m_pAosHandle->SetReason(AosReason::POWER_OFF);
    EXPECT_EQ(GetReason(), AosReason::POWER_OFF);

    m_pAosHandle->SetReason(AosReason::BAD_BATTERY);
    EXPECT_EQ(GetReason(), AosReason::BAD_BATTERY);

    m_pAosHandle->SetReason(AosReason::AIRPLANE_MODE);
    EXPECT_EQ(GetReason(), AosReason::AIRPLANE_MODE);

    m_pAosHandle->SetReason(AosReason::NO_LTE_COVERAGE);
    EXPECT_EQ(GetReason(), AosReason::NO_LTE_COVERAGE);

    m_pAosHandle->SetReason(AosReason::SERVICE_POLICY);
    EXPECT_EQ(GetReason(), AosReason::SERVICE_POLICY);

    m_pAosHandle->SetReason(AosReason::SERVICE_BLOCKED);
    EXPECT_EQ(GetReason(), AosReason::SERVICE_BLOCKED);

    m_pAosHandle->SetReason(AosReason::IMS_DISABLED);
    EXPECT_EQ(GetReason(), AosReason::IMS_DISABLED);

    m_pAosHandle->SetReason(AosReason::TTYMODEON);
    EXPECT_EQ(GetReason(), AosReason::TTYMODEON);

    m_pAosHandle->SetReason(AosReason::NOT_SPECIFIED);
    EXPECT_EQ(GetReason(), AosReason::NOT_SPECIFIED);

    m_pAosHandle->SetReason(AosReason::IP_CHANGED);
    EXPECT_EQ(GetReason(), AosReason::IP_CHANGED);

    m_pAosHandle->SetReason(AosReason::DATA_DISCONNECTED);
    EXPECT_EQ(GetReason(), AosReason::DATA_DISCONNECTED);

    m_pAosHandle->SetReason(AosReason::DATA_CONNECTION_MAINTAIN);
    EXPECT_EQ(GetReason(), AosReason::DATA_CONNECTION_MAINTAIN);

    m_pAosHandle->SetReason(AosReason::DATA_PERMANENTLY_FAILED);
    EXPECT_EQ(GetReason(), AosReason::DATA_PERMANENTLY_FAILED);

    m_pAosHandle->SetReason(AosReason::REG_FAILURE);
    EXPECT_EQ(GetReason(), AosReason::REG_FAILURE);

    m_pAosHandle->SetReason(AosReason::REG_TERMINATED);
    EXPECT_EQ(GetReason(), AosReason::REG_TERMINATED);

    m_pAosHandle->SetReason(AosReason::INITIAL_REG_REQUESTED);
    EXPECT_EQ(GetReason(), AosReason::INITIAL_REG_REQUESTED);

    m_pAosHandle->SetReason(AosReason::REG_TERMINATING);
    EXPECT_EQ(GetReason(), AosReason::REG_TERMINATING);

    m_pAosHandle->SetReason(AosReason::PCSCF_DISCOVERY_FAILED);
    EXPECT_EQ(GetReason(), AosReason::PCSCF_DISCOVERY_FAILED);

    m_pAosHandle->SetReason(AosReason::UNKNOWN);
    EXPECT_EQ(GetReason(), AosReason::UNKNOWN);
}

TEST_F(AosHandleTest, ClearSuspendedReason_Test)
{
    m_pAosHandle->SetSuspendedReason(AosReason::SUSPEND_NO_SERVICE);
    EXPECT_TRUE(m_pAosHandle->IsImsSuspended());

    m_pAosHandle->ClearSuspendedReason();
    EXPECT_FALSE(m_pAosHandle->IsImsSuspended());
}

TEST_F(AosHandleTest, GetAppState_Test)
{
    m_pAosHandle->SetState(AosHandle::STATE_CONNECTED);
    EXPECT_EQ(m_pAosHandle->GetAppState(), AosHandle::APP_STATE_CONNECTED);

    m_pAosHandle->SetState(AosHandle::STATE_DISCONNECTING);
    EXPECT_EQ(m_pAosHandle->GetAppState(), AosHandle::APP_STATE_DISCONNECTING);

    m_pAosHandle->SetState(AosHandle::STATE_CONNECTING);
    EXPECT_EQ(m_pAosHandle->GetAppState(), AosHandle::APP_STATE_DISCONNECTED);

    m_pAosHandle->SetState(AosHandle::STATE_DISCONNECTED);
    EXPECT_EQ(m_pAosHandle->GetAppState(), AosHandle::APP_STATE_DISCONNECTED);
}

TEST_F(AosHandleTest, GetImsAosReason_Test)
{
    EXPECT_EQ(m_pAosHandle->GetImsAosReason(AosReason::NONE), ImsAosReason::NOT_SPECIFIED);
    EXPECT_EQ(m_pAosHandle->GetImsAosReason(AosReason::BAD_BATTERY), ImsAosReason::POWER_OFF);
    EXPECT_EQ(m_pAosHandle->GetImsAosReason(AosReason::POWER_OFF), ImsAosReason::POWER_OFF);
    EXPECT_EQ(m_pAosHandle->GetImsAosReason(AosReason::AIRPLANE_MODE),
            ImsAosReason::DATA_DISCONNECTED);
    EXPECT_EQ(m_pAosHandle->GetImsAosReason(AosReason::DATA_DISCONNECTED),
            ImsAosReason::DATA_DISCONNECTED);
    EXPECT_EQ(m_pAosHandle->GetImsAosReason(AosReason::NO_LTE_COVERAGE),
            ImsAosReason::NO_RAT_COVERAGE);
    EXPECT_EQ(
            m_pAosHandle->GetImsAosReason(AosReason::SERVICE_POLICY), ImsAosReason::SERVICE_POLICY);
    EXPECT_EQ(m_pAosHandle->GetImsAosReason(AosReason::SERVICE_BLOCKED),
            ImsAosReason::SERVICE_BLOCKED);
    EXPECT_EQ(m_pAosHandle->GetImsAosReason(AosReason::SRV_OUT), ImsAosReason::OUT_OF_SERVICE);
    EXPECT_EQ(
            m_pAosHandle->GetImsAosReason(AosReason::REG_TERMINATED), ImsAosReason::REG_TERMINATED);
    EXPECT_EQ(m_pAosHandle->GetImsAosReason(AosReason::INITIAL_REG_REQUESTED),
            ImsAosReason::REG_NEW_REQUIRED);
    EXPECT_EQ(m_pAosHandle->GetImsAosReason(AosReason::REG_TERMINATING),
            ImsAosReason::REG_TERMINATING);
    EXPECT_EQ(m_pAosHandle->GetImsAosReason(AosReason::IMS_DISABLED), ImsAosReason::NOT_SPECIFIED);
    EXPECT_EQ(m_pAosHandle->GetImsAosReason(AosReason::TTYMODEON), ImsAosReason::NOT_SPECIFIED);
}

TEST_F(AosHandleTest, GetImsAosReasonForSuspend_Test)
{
    EXPECT_EQ(m_pAosHandle->GetImsAosReasonForSuspend(AosReason::SUSPEND_NONE),
            ImsAosReason::SUSPEND_NONE);
    EXPECT_EQ(m_pAosHandle->GetImsAosReasonForSuspend(AosReason::SUSPEND_NO_SERVICE),
            ImsAosReason::SUSPEND_OUT_OF_SERVICE);
    EXPECT_EQ(m_pAosHandle->GetImsAosReasonForSuspend(AosReason::SUSPEND_NO_LTE_COVERAGE),
            ImsAosReason::SUSPEND_NO_RAT_COVERAGE);

    EXPECT_EQ(m_pAosHandle->GetImsAosReasonForSuspend(AosReason::SUSPEND_CS_CALL),
            ImsAosReason::SUSPEND_NONE);
    EXPECT_EQ(m_pAosHandle->GetImsAosReasonForSuspend(AosReason::SUSPEND_LOW_BATTERY),
            ImsAosReason::SUSPEND_NONE);
    EXPECT_EQ(m_pAosHandle->GetImsAosReasonForSuspend(AosReason::SUSPEND_INSTANTANEOUS_OFFLINE),
            ImsAosReason::SUSPEND_NONE);
}

TEST_F(AosHandleTest, IsEpdgEnabled_Test)
{
    SetEpdgEnabled(IMS_TRUE);
    EXPECT_TRUE(m_pAosHandle->IsEpdgEnabled());

    SetEpdgEnabled(IMS_FALSE);
    EXPECT_FALSE(m_pAosHandle->IsEpdgEnabled());
}

TEST_F(AosHandleTest, IsEqualNetworkType_Test)
{
    EXPECT_TRUE(m_pAosHandle->IsEqualNetworkType(-1, AosNetworkType::NONE));
    EXPECT_TRUE(m_pAosHandle->IsEqualNetworkType(0, AosNetworkType::LTE));
    EXPECT_TRUE(m_pAosHandle->IsEqualNetworkType(1, AosNetworkType::IWLAN));
    EXPECT_TRUE(m_pAosHandle->IsEqualNetworkType(2, AosNetworkType::CROSS_SIM));
    EXPECT_TRUE(m_pAosHandle->IsEqualNetworkType(3, AosNetworkType::NR));
    EXPECT_TRUE(m_pAosHandle->IsEqualNetworkType(4, AosNetworkType::UTRAN));

    EXPECT_FALSE(m_pAosHandle->IsEqualNetworkType(-1, AosNetworkType::LTE));
    EXPECT_FALSE(m_pAosHandle->IsEqualNetworkType(-1, AosNetworkType::IWLAN));
    EXPECT_FALSE(m_pAosHandle->IsEqualNetworkType(-1, AosNetworkType::CROSS_SIM));
    EXPECT_FALSE(m_pAosHandle->IsEqualNetworkType(-1, AosNetworkType::NR));
    EXPECT_FALSE(m_pAosHandle->IsEqualNetworkType(-1, AosNetworkType::UTRAN));

    EXPECT_FALSE(m_pAosHandle->IsEqualNetworkType(0, AosNetworkType::NONE));
    EXPECT_FALSE(m_pAosHandle->IsEqualNetworkType(0, AosNetworkType::IWLAN));
    EXPECT_FALSE(m_pAosHandle->IsEqualNetworkType(0, AosNetworkType::CROSS_SIM));
    EXPECT_FALSE(m_pAosHandle->IsEqualNetworkType(0, AosNetworkType::NR));
    EXPECT_FALSE(m_pAosHandle->IsEqualNetworkType(0, AosNetworkType::UTRAN));

    EXPECT_FALSE(m_pAosHandle->IsEqualNetworkType(1, AosNetworkType::NONE));
    EXPECT_FALSE(m_pAosHandle->IsEqualNetworkType(1, AosNetworkType::LTE));
    EXPECT_FALSE(m_pAosHandle->IsEqualNetworkType(1, AosNetworkType::CROSS_SIM));
    EXPECT_FALSE(m_pAosHandle->IsEqualNetworkType(1, AosNetworkType::NR));
    EXPECT_FALSE(m_pAosHandle->IsEqualNetworkType(1, AosNetworkType::UTRAN));

    EXPECT_FALSE(m_pAosHandle->IsEqualNetworkType(2, AosNetworkType::NONE));
    EXPECT_FALSE(m_pAosHandle->IsEqualNetworkType(2, AosNetworkType::LTE));
    EXPECT_FALSE(m_pAosHandle->IsEqualNetworkType(2, AosNetworkType::IWLAN));
    EXPECT_FALSE(m_pAosHandle->IsEqualNetworkType(2, AosNetworkType::NR));
    EXPECT_FALSE(m_pAosHandle->IsEqualNetworkType(2, AosNetworkType::UTRAN));

    EXPECT_FALSE(m_pAosHandle->IsEqualNetworkType(3, AosNetworkType::NONE));
    EXPECT_FALSE(m_pAosHandle->IsEqualNetworkType(3, AosNetworkType::LTE));
    EXPECT_FALSE(m_pAosHandle->IsEqualNetworkType(3, AosNetworkType::IWLAN));
    EXPECT_FALSE(m_pAosHandle->IsEqualNetworkType(3, AosNetworkType::CROSS_SIM));
    EXPECT_FALSE(m_pAosHandle->IsEqualNetworkType(3, AosNetworkType::UTRAN));

    EXPECT_FALSE(m_pAosHandle->IsEqualNetworkType(4, AosNetworkType::NONE));
    EXPECT_FALSE(m_pAosHandle->IsEqualNetworkType(4, AosNetworkType::LTE));
    EXPECT_FALSE(m_pAosHandle->IsEqualNetworkType(4, AosNetworkType::IWLAN));
    EXPECT_FALSE(m_pAosHandle->IsEqualNetworkType(4, AosNetworkType::CROSS_SIM));
    EXPECT_FALSE(m_pAosHandle->IsEqualNetworkType(4, AosNetworkType::NR));
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
    EXPECT_TRUE(m_pAosHandle->IsCapabilityExisted(nCapabilities, AosCapability::VOICE));
    EXPECT_TRUE(m_pAosHandle->IsCapabilityExisted(nCapabilities, AosCapability::VIDEO));
    EXPECT_TRUE(m_pAosHandle->IsCapabilityExisted(nCapabilities, AosCapability::UT));
    EXPECT_TRUE(m_pAosHandle->IsCapabilityExisted(nCapabilities, AosCapability::SMS));
    EXPECT_TRUE(m_pAosHandle->IsCapabilityExisted(nCapabilities, AosCapability::CALL_COMPOSER));
    EXPECT_TRUE(m_pAosHandle->IsCapabilityExisted(nCapabilities, AosCapability::OPTIONS_UCE));
    EXPECT_TRUE(m_pAosHandle->IsCapabilityExisted(nCapabilities, AosCapability::PRESENCE_UCE));

    nCapabilities = static_cast<IMS_UINT32>(AosCapability::VOICE);
    EXPECT_TRUE(m_pAosHandle->IsCapabilityExisted(nCapabilities, AosCapability::VOICE));
    EXPECT_FALSE(m_pAosHandle->IsCapabilityExisted(nCapabilities, AosCapability::VIDEO));
    EXPECT_FALSE(m_pAosHandle->IsCapabilityExisted(nCapabilities, AosCapability::UT));
    EXPECT_FALSE(m_pAosHandle->IsCapabilityExisted(nCapabilities, AosCapability::SMS));
    EXPECT_FALSE(m_pAosHandle->IsCapabilityExisted(nCapabilities, AosCapability::CALL_COMPOSER));
    EXPECT_FALSE(m_pAosHandle->IsCapabilityExisted(nCapabilities, AosCapability::OPTIONS_UCE));
    EXPECT_FALSE(m_pAosHandle->IsCapabilityExisted(nCapabilities, AosCapability::PRESENCE_UCE));

    nCapabilities = static_cast<IMS_UINT32>(AosCapability::VIDEO);
    EXPECT_FALSE(m_pAosHandle->IsCapabilityExisted(nCapabilities, AosCapability::VOICE));
    EXPECT_TRUE(m_pAosHandle->IsCapabilityExisted(nCapabilities, AosCapability::VIDEO));
    EXPECT_FALSE(m_pAosHandle->IsCapabilityExisted(nCapabilities, AosCapability::UT));
    EXPECT_FALSE(m_pAosHandle->IsCapabilityExisted(nCapabilities, AosCapability::SMS));
    EXPECT_FALSE(m_pAosHandle->IsCapabilityExisted(nCapabilities, AosCapability::CALL_COMPOSER));
    EXPECT_FALSE(m_pAosHandle->IsCapabilityExisted(nCapabilities, AosCapability::OPTIONS_UCE));
    EXPECT_FALSE(m_pAosHandle->IsCapabilityExisted(nCapabilities, AosCapability::PRESENCE_UCE));
}

TEST_F(AosHandleTest, IsCapabilityExistedForNetworkType_Test)
{
    ImsMap<IMS_UINT32, IMS_UINT32> objCapabilities;
    objCapabilities.Add(static_cast<IMS_UINT32>(AosNetworkType::LTE),
            static_cast<IMS_UINT32>(AosCapability::VOICE) |
                    static_cast<IMS_UINT32>(AosCapability::VIDEO));
    objCapabilities.Add(static_cast<IMS_UINT32>(AosNetworkType::IWLAN),
            static_cast<IMS_UINT32>(AosCapability::VIDEO));
    objCapabilities.Add(static_cast<IMS_UINT32>(AosNetworkType::NR),
            static_cast<IMS_UINT32>(AosCapability::NONE));

    SetCapabilities(objCapabilities);

    EXPECT_TRUE(m_pAosHandle->IsCapabilityExistedForNetworkType(
            NW_REPORT_RADIO_LTE, AosCapability::VOICE));
    EXPECT_TRUE(m_pAosHandle->IsCapabilityExistedForNetworkType(
            NW_REPORT_RADIO_LTE, AosCapability::VIDEO));
    EXPECT_FALSE(m_pAosHandle->IsCapabilityExistedForNetworkType(
            NW_REPORT_RADIO_WLAN, AosCapability::VOICE));
    EXPECT_TRUE(m_pAosHandle->IsCapabilityExistedForNetworkType(
            NW_REPORT_RADIO_WLAN, AosCapability::VIDEO));
    EXPECT_FALSE(m_pAosHandle->IsCapabilityExistedForNetworkType(
            NW_REPORT_RADIO_NR, AosCapability::VOICE));
    EXPECT_FALSE(m_pAosHandle->IsCapabilityExistedForNetworkType(
            NW_REPORT_RADIO_NR, AosCapability::VIDEO));
    EXPECT_FALSE(m_pAosHandle->IsCapabilityExistedForNetworkType(
            NW_REPORT_RADIO_GSM, AosCapability::VOICE));
    EXPECT_FALSE(m_pAosHandle->IsCapabilityExistedForNetworkType(
            NW_REPORT_RADIO_GSM, AosCapability::VIDEO));
}

TEST_F(AosHandleTest, IsNetworkTypeMatchedToRat_Test)
{
    EXPECT_TRUE(m_pAosHandle->IsNetworkTypeMatchedToRat(0, NW_REPORT_RADIO_LTE));
    EXPECT_FALSE(m_pAosHandle->IsNetworkTypeMatchedToRat(0, NW_REPORT_RADIO_WLAN));
    EXPECT_FALSE(m_pAosHandle->IsNetworkTypeMatchedToRat(0, NW_REPORT_RADIO_NR));

    EXPECT_FALSE(m_pAosHandle->IsNetworkTypeMatchedToRat(1, NW_REPORT_RADIO_LTE));
    EXPECT_TRUE(m_pAosHandle->IsNetworkTypeMatchedToRat(1, NW_REPORT_RADIO_WLAN));
    EXPECT_FALSE(m_pAosHandle->IsNetworkTypeMatchedToRat(1, NW_REPORT_RADIO_NR));

    EXPECT_FALSE(m_pAosHandle->IsNetworkTypeMatchedToRat(3, NW_REPORT_RADIO_LTE));
    EXPECT_FALSE(m_pAosHandle->IsNetworkTypeMatchedToRat(3, NW_REPORT_RADIO_WLAN));
    EXPECT_TRUE(m_pAosHandle->IsNetworkTypeMatchedToRat(3, NW_REPORT_RADIO_NR));

    EXPECT_FALSE(m_pAosHandle->IsNetworkTypeMatchedToRat(0, NW_REPORT_RADIO_NOSRV));
    EXPECT_FALSE(m_pAosHandle->IsNetworkTypeMatchedToRat(0, NW_REPORT_RADIO_CDMA));
    EXPECT_FALSE(m_pAosHandle->IsNetworkTypeMatchedToRat(0, NW_REPORT_RADIO_EHRPD));
    EXPECT_FALSE(m_pAosHandle->IsNetworkTypeMatchedToRat(0, NW_REPORT_RADIO_GSM));
    EXPECT_FALSE(m_pAosHandle->IsNetworkTypeMatchedToRat(0, NW_REPORT_RADIO_WCDMA));
}

TEST_F(AosHandleTest, IsEmergencyService_Test)
{
    // Expectation: return true if service id is emergency.

    MockIAosAppContext objMockIAosAppContext;
    MockIAosNConfiguration objMockIAosNConfiguration;

    EXPECT_CALL(objMockIAosAppContext, GetSlotId()).Times(AnyNumber()).WillRepeatedly(Return(0));

    const AString strValue = AString("test");
    EXPECT_CALL(objMockIAosAppContext, GetProfileId())
            .Times(AnyNumber())
            .WillRepeatedly(ReturnRef(strValue));

    IAosNConfiguration* piAosNConfiguration = AosProvider::GetInstance()->GetNConfiguration();
    AosProvider::GetInstance()->SetNConfiguration(
            static_cast<IAosNConfiguration*>(&objMockIAosNConfiguration));

    EXPECT_CALL(objMockIAosNConfiguration, IsDeregOn3gNetwork())
            .Times(AnyNumber())
            .WillRepeatedly(Return(IMS_TRUE));

    TestAosHandle* pTestAosHandleEmergencyMtc =
            new TestAosHandle(static_cast<IAosAppContext*>(&objMockIAosAppContext), m_strAppId,
                    m_strServiceId, ImsAosService::EMERGENCY_MTC);

    TestAosHandle* pTestAosHandleEmergencyMts =
            new TestAosHandle(static_cast<IAosAppContext*>(&objMockIAosAppContext), m_strAppId,
                    m_strServiceId, ImsAosService::EMERGENCY_MTS);

    ASSERT_TRUE(pTestAosHandleEmergencyMtc != nullptr);
    ASSERT_TRUE(pTestAosHandleEmergencyMts != nullptr);

    EXPECT_FALSE(m_pAosHandle->IsEmergencyService());
    EXPECT_TRUE(pTestAosHandleEmergencyMtc->AosHandle::IsEmergencyService());
    EXPECT_TRUE(pTestAosHandleEmergencyMts->AosHandle::IsEmergencyService());

    delete pTestAosHandleEmergencyMtc;
    delete pTestAosHandleEmergencyMts;

    AosProvider::GetInstance()->SetNConfiguration(piAosNConfiguration);
}

TEST_F(AosHandleTest, IsRoaming_Test)
{
    SetRoamingState(IMS_ROAMING_STATE_OFF);
    EXPECT_EQ(m_pAosHandle->IsRoaming(), IMS_ROAMING_STATE_OFF);

    SetRoamingState(IMS_ROAMING_STATE_ON);
    EXPECT_EQ(m_pAosHandle->IsRoaming(), IMS_ROAMING_STATE_ON);
}

TEST_F(AosHandleTest, GetNetworkType_Test)
{
    EXPECT_CALL(m_objMockIAosNetTracker, GetNetworkType()).Times(1);
    m_pAosHandle->GetNetworkType();
}

TEST_F(AosHandleTest, GetMobileNetworkType_Test)
{
    EXPECT_CALL(m_objMockIAosNetTracker, GetMobileNetworkType()).Times(1);
    m_pAosHandle->GetMobileNetworkType();
}

TEST_F(AosHandleTest, GetMobileChangingNetworkType_Test)
{
    EXPECT_CALL(m_objMockIAosNetTracker, GetMobileChangingNetworkType()).Times(1);
    m_pAosHandle->GetMobileChangingNetworkType();
}

TEST_F(AosHandleTest, GetAosFeature_Test)
{
    EXPECT_EQ(m_pAosHandle->GetAosFeature(AosHandle::BLOCK_VOLTE_CAPABILITY), ImsAosFeature::MMTEL);
    EXPECT_EQ(
            m_pAosHandle->GetAosFeature(AosHandle::BLOCK_VOWIFI_CAPABILITY), ImsAosFeature::MMTEL);
    EXPECT_EQ(m_pAosHandle->GetAosFeature(AosHandle::BLOCK_VOPS), ImsAosFeature::MMTEL);

    EXPECT_EQ(m_pAosHandle->GetAosFeature(AosHandle::BLOCK_VILTE_CAPABILITY), ImsAosFeature::VIDEO);
    EXPECT_EQ(
            m_pAosHandle->GetAosFeature(AosHandle::BLOCK_VIWIFI_CAPABILITY), ImsAosFeature::VIDEO);

    EXPECT_EQ(m_pAosHandle->GetAosFeature(AosHandle::BLOCK_SMS_CAPABILITY), ImsAosFeature::SMSIP);
    EXPECT_EQ(m_pAosHandle->GetAosFeature(AosHandle::BLOCK_SMS_OVER_IP_NETWORK_INDICATION),
            ImsAosFeature::SMSIP);

    EXPECT_EQ(m_pAosHandle->GetAosFeature(AosHandle::BLOCK_CALL_COMPOSER_CAPABILITY),
            ImsAosFeature::CALL_COMPOSER_VIA_TELEPHONY);

    EXPECT_EQ(m_pAosHandle->GetAosFeature(AosHandle::BLOCK_NONE), ImsAosFeature::NONE);
}

TEST_F(AosHandleTest, BackupAllBlocks_Test1)
{
    // Test1: Valid cellular, wfc unavailable
    // Expectation: No backup

    SetHoldingBlocksPolicyForMobile();
    SetHoldingBlocksPolicyForWifi();

    m_pAosHandle->AddBlock(AosHandle::BLOCK_VOPS, m_pAosHandle->m_nBlocks);
    m_pAosHandle->AddBlock(AosHandle::BLOCK_VILTE_CAPABILITY, m_pAosHandle->m_nBlocks);
    m_pAosHandle->AddBlock(AosHandle::BLOCK_VOWIFI_CAPABILITY, m_pAosHandle->m_nBlocks);
    m_pAosHandle->AddBlock(AosHandle::BLOCK_VIWIFI_CAPABILITY, m_pAosHandle->m_nBlocks);

    EXPECT_CALL(m_objMockIAosNetTracker, GetMobileNetworkType())
            .Times(AnyNumber())
            .WillRepeatedly(Return(NW_REPORT_RADIO_LTE));

    EXPECT_CALL(m_objMockIAosNConfiguration, IsWfcImsAvailable())
            .Times(AnyNumber())
            .WillOnce(Return(IMS_FALSE));

    m_pAosHandle->BackupAllBlocks();

    EXPECT_TRUE(m_pAosHandle->IsHandleBlocked(AosHandle::BLOCK_VOPS));
    EXPECT_TRUE(m_pAosHandle->IsHandleBlocked(AosHandle::BLOCK_VILTE_CAPABILITY));
    EXPECT_TRUE(m_pAosHandle->IsHandleBlocked(AosHandle::BLOCK_VOWIFI_CAPABILITY));
    EXPECT_TRUE(m_pAosHandle->IsHandleBlocked(AosHandle::BLOCK_VIWIFI_CAPABILITY));

    EXPECT_FALSE(m_pAosHandle->IsHoldingBlockForMobile(AosHandle::BLOCK_VOPS));
    EXPECT_FALSE(m_pAosHandle->IsHoldingBlockForMobile(AosHandle::BLOCK_VILTE_CAPABILITY));
    EXPECT_FALSE(IsHoldingBlockForWifi(AosHandle::BLOCK_VOWIFI_CAPABILITY));
    EXPECT_FALSE(IsHoldingBlockForWifi(AosHandle::BLOCK_VIWIFI_CAPABILITY));
}

TEST_F(AosHandleTest, BackupAllBlocks_Test2)
{
    // Test2: Invalid cellular network, wifi connected
    // Expectation: Backup mobile only

    SetHoldingBlocksPolicyForMobile();
    SetHoldingBlocksPolicyForWifi();

    m_pAosHandle->AddBlock(AosHandle::BLOCK_VOPS, m_pAosHandle->m_nBlocks);
    m_pAosHandle->AddBlock(AosHandle::BLOCK_VILTE_CAPABILITY, m_pAosHandle->m_nBlocks);
    m_pAosHandle->AddBlock(AosHandle::BLOCK_VOWIFI_CAPABILITY, m_pAosHandle->m_nBlocks);
    m_pAosHandle->AddBlock(AosHandle::BLOCK_VIWIFI_CAPABILITY, m_pAosHandle->m_nBlocks);

    EXPECT_CALL(m_objMockIAosNetTracker, GetMobileNetworkType())
            .Times(AnyNumber())
            .WillRepeatedly(Return(NW_REPORT_RADIO_INVALID));

    EXPECT_CALL(m_objMockIAosNConfiguration, IsWfcImsAvailable())
            .Times(AnyNumber())
            .WillOnce(Return(IMS_TRUE));

    SetWifiWatcher(static_cast<IWifiWatcher*>(&m_objMockIWifiWatcher));
    EXPECT_CALL(m_objMockIWifiWatcher, GetState())
            .Times(AnyNumber())
            .WillRepeatedly(Return(IWifiWatcher::STATE_CONNECTED));

    m_pAosHandle->BackupAllBlocks();

    EXPECT_FALSE(m_pAosHandle->IsHandleBlocked(AosHandle::BLOCK_VOPS));
    EXPECT_FALSE(m_pAosHandle->IsHandleBlocked(AosHandle::BLOCK_VILTE_CAPABILITY));
    EXPECT_TRUE(m_pAosHandle->IsHoldingBlockForMobile(AosHandle::BLOCK_VOPS));
    EXPECT_TRUE(m_pAosHandle->IsHoldingBlockForMobile(AosHandle::BLOCK_VILTE_CAPABILITY));

    EXPECT_TRUE(m_pAosHandle->IsHandleBlocked(AosHandle::BLOCK_VOWIFI_CAPABILITY));
    EXPECT_TRUE(m_pAosHandle->IsHandleBlocked(AosHandle::BLOCK_VIWIFI_CAPABILITY));
    EXPECT_FALSE(IsHoldingBlockForWifi(AosHandle::BLOCK_VOWIFI_CAPABILITY));
    EXPECT_FALSE(IsHoldingBlockForWifi(AosHandle::BLOCK_VIWIFI_CAPABILITY));
}

TEST_F(AosHandleTest, BackupAllBlocks_Test3)
{
    // Test3: Valid cellular network, wfc available & wifi disconnected
    // Expectation: Backup wifi only

    SetHoldingBlocksPolicyForMobile();
    SetHoldingBlocksPolicyForWifi();

    m_pAosHandle->AddBlock(AosHandle::BLOCK_VOPS, m_pAosHandle->m_nBlocks);
    m_pAosHandle->AddBlock(AosHandle::BLOCK_VILTE_CAPABILITY, m_pAosHandle->m_nBlocks);
    m_pAosHandle->AddBlock(AosHandle::BLOCK_VOWIFI_CAPABILITY, m_pAosHandle->m_nBlocks);
    m_pAosHandle->AddBlock(AosHandle::BLOCK_VIWIFI_CAPABILITY, m_pAosHandle->m_nBlocks);

    EXPECT_CALL(m_objMockIAosNetTracker, GetMobileNetworkType())
            .Times(AnyNumber())
            .WillRepeatedly(Return(NW_REPORT_RADIO_LTE));

    EXPECT_CALL(m_objMockIAosNConfiguration, IsWfcImsAvailable())
            .Times(AnyNumber())
            .WillOnce(Return(IMS_TRUE));

    SetWifiWatcher(static_cast<IWifiWatcher*>(&m_objMockIWifiWatcher));
    EXPECT_CALL(m_objMockIWifiWatcher, GetState())
            .Times(AnyNumber())
            .WillRepeatedly(Return(IWifiWatcher::STATE_DISCONNECTED));

    m_pAosHandle->BackupAllBlocks();

    EXPECT_TRUE(m_pAosHandle->IsHandleBlocked(AosHandle::BLOCK_VOPS));
    EXPECT_TRUE(m_pAosHandle->IsHandleBlocked(AosHandle::BLOCK_VILTE_CAPABILITY));
    EXPECT_FALSE(m_pAosHandle->IsHoldingBlockForMobile(AosHandle::BLOCK_VOPS));
    EXPECT_FALSE(m_pAosHandle->IsHoldingBlockForMobile(AosHandle::BLOCK_VILTE_CAPABILITY));

    EXPECT_FALSE(m_pAosHandle->IsHandleBlocked(AosHandle::BLOCK_VOWIFI_CAPABILITY));
    EXPECT_FALSE(m_pAosHandle->IsHandleBlocked(AosHandle::BLOCK_VIWIFI_CAPABILITY));
    EXPECT_TRUE(IsHoldingBlockForWifi(AosHandle::BLOCK_VOWIFI_CAPABILITY));
    EXPECT_TRUE(IsHoldingBlockForWifi(AosHandle::BLOCK_VIWIFI_CAPABILITY));
}

TEST_F(AosHandleTest, BackupAllBlocks_Test4)
{
    // Test4: Invalid cellular network, wfc available & wifi disconnected
    // Expectation: Backup both mobile and wifi

    SetHoldingBlocksPolicyForMobile();
    SetHoldingBlocksPolicyForWifi();

    m_pAosHandle->AddBlock(AosHandle::BLOCK_VOPS, m_pAosHandle->m_nBlocks);
    m_pAosHandle->AddBlock(AosHandle::BLOCK_VILTE_CAPABILITY, m_pAosHandle->m_nBlocks);
    m_pAosHandle->AddBlock(AosHandle::BLOCK_VOWIFI_CAPABILITY, m_pAosHandle->m_nBlocks);
    m_pAosHandle->AddBlock(AosHandle::BLOCK_VIWIFI_CAPABILITY, m_pAosHandle->m_nBlocks);

    EXPECT_CALL(m_objMockIAosNetTracker, GetMobileNetworkType())
            .Times(AnyNumber())
            .WillRepeatedly(Return(NW_REPORT_RADIO_INVALID));

    EXPECT_CALL(m_objMockIAosNConfiguration, IsWfcImsAvailable())
            .Times(AnyNumber())
            .WillOnce(Return(IMS_TRUE));

    SetWifiWatcher(static_cast<IWifiWatcher*>(&m_objMockIWifiWatcher));
    EXPECT_CALL(m_objMockIWifiWatcher, GetState())
            .Times(AnyNumber())
            .WillRepeatedly(Return(IWifiWatcher::STATE_DISCONNECTED));

    m_pAosHandle->BackupAllBlocks();

    EXPECT_FALSE(m_pAosHandle->IsHandleBlocked(AosHandle::BLOCK_VOPS));
    EXPECT_FALSE(m_pAosHandle->IsHandleBlocked(AosHandle::BLOCK_VILTE_CAPABILITY));
    EXPECT_TRUE(m_pAosHandle->IsHoldingBlockForMobile(AosHandle::BLOCK_VOPS));
    EXPECT_TRUE(m_pAosHandle->IsHoldingBlockForMobile(AosHandle::BLOCK_VILTE_CAPABILITY));

    EXPECT_FALSE(m_pAosHandle->IsHandleBlocked(AosHandle::BLOCK_VOWIFI_CAPABILITY));
    EXPECT_FALSE(m_pAosHandle->IsHandleBlocked(AosHandle::BLOCK_VIWIFI_CAPABILITY));
    EXPECT_TRUE(IsHoldingBlockForWifi(AosHandle::BLOCK_VOWIFI_CAPABILITY));
    EXPECT_TRUE(IsHoldingBlockForWifi(AosHandle::BLOCK_VIWIFI_CAPABILITY));
}

TEST_F(AosHandleTest, BackupBlocks_Test)
{
    SetHoldingBlocksPolicyForMobile();
    SetHoldingBlocksPolicyForWifi();

    m_pAosHandle->AddBlock(AosHandle::BLOCK_VOPS, m_pAosHandle->m_nBlocks);
    m_pAosHandle->AddBlock(AosHandle::BLOCK_VILTE_CAPABILITY, m_pAosHandle->m_nBlocks);
    m_pAosHandle->AddBlock(AosHandle::BLOCK_VOWIFI_CAPABILITY, m_pAosHandle->m_nBlocks);
    m_pAosHandle->AddBlock(AosHandle::BLOCK_VIWIFI_CAPABILITY, m_pAosHandle->m_nBlocks);

    BackupBlocksForMobile();

    EXPECT_FALSE(m_pAosHandle->IsHandleBlocked(AosHandle::BLOCK_VOPS));
    EXPECT_FALSE(m_pAosHandle->IsHandleBlocked(AosHandle::BLOCK_VILTE_CAPABILITY));
    EXPECT_TRUE(m_pAosHandle->IsHoldingBlockForMobile(AosHandle::BLOCK_VOPS));
    EXPECT_TRUE(m_pAosHandle->IsHoldingBlockForMobile(AosHandle::BLOCK_VILTE_CAPABILITY));

    BackupBlocksForWifi();

    EXPECT_FALSE(m_pAosHandle->IsHandleBlocked(AosHandle::BLOCK_VOWIFI_CAPABILITY));
    EXPECT_FALSE(m_pAosHandle->IsHandleBlocked(AosHandle::BLOCK_VIWIFI_CAPABILITY));
    EXPECT_TRUE(IsHoldingBlockForWifi(AosHandle::BLOCK_VOWIFI_CAPABILITY));
    EXPECT_TRUE(IsHoldingBlockForWifi(AosHandle::BLOCK_VIWIFI_CAPABILITY));
}

TEST_F(AosHandleTest, RestoreBlocks_Test)
{
    SetHoldingBlocksPolicyForMobile();
    SetHoldingBlocksPolicyForWifi();

    AddHoldingBlockForMobile(AosHandle::BLOCK_VOPS);
    AddHoldingBlockForMobile(AosHandle::BLOCK_VILTE_CAPABILITY);
    AddHoldingBlockForWifi(AosHandle::BLOCK_VOWIFI_CAPABILITY);
    AddHoldingBlockForWifi(AosHandle::BLOCK_VIWIFI_CAPABILITY);

    RestoreBlocksForMobile();

    EXPECT_TRUE(m_pAosHandle->IsHandleBlocked(AosHandle::BLOCK_VOPS));
    EXPECT_TRUE(m_pAosHandle->IsHandleBlocked(AosHandle::BLOCK_VILTE_CAPABILITY));
    EXPECT_FALSE(m_pAosHandle->IsHoldingBlockForMobile(AosHandle::BLOCK_VOPS));
    EXPECT_FALSE(m_pAosHandle->IsHoldingBlockForMobile(AosHandle::BLOCK_VILTE_CAPABILITY));

    RestoreBlocksForWifi();

    EXPECT_TRUE(m_pAosHandle->IsHandleBlocked(AosHandle::BLOCK_VOWIFI_CAPABILITY));
    EXPECT_TRUE(m_pAosHandle->IsHandleBlocked(AosHandle::BLOCK_VIWIFI_CAPABILITY));
    EXPECT_FALSE(IsHoldingBlockForWifi(AosHandle::BLOCK_VOWIFI_CAPABILITY));
    EXPECT_FALSE(IsHoldingBlockForWifi(AosHandle::BLOCK_VIWIFI_CAPABILITY));
}

TEST_F(AosHandleTest, HoldBlockForInvalidNetwork_Test1)
{
    // Test1: Block for mobile, valid network(LTE)
    // Expectation: return false, no change in holding block

    SetHoldingBlocksPolicyForMobile();
    SetHoldingBlocksPolicyForWifi();

    EXPECT_CALL(m_objMockIAosNetTracker, GetMobileNetworkType())
            .Times(AnyNumber())
            .WillRepeatedly(Return(NW_REPORT_RADIO_LTE));

    AddHoldingBlockForMobile(AosHandle::BLOCK_VILTE_CAPABILITY);

    EXPECT_FALSE(m_pAosHandle->HoldBlockForInvalidNetwork(AosHandle::BLOCK_VOPS, IMS_TRUE));
    EXPECT_FALSE(m_pAosHandle->IsHoldingBlockForMobile(AosHandle::BLOCK_VOPS));

    EXPECT_FALSE(
            m_pAosHandle->HoldBlockForInvalidNetwork(AosHandle::BLOCK_VILTE_CAPABILITY, IMS_FALSE));
    EXPECT_TRUE(m_pAosHandle->IsHoldingBlockForMobile(AosHandle::BLOCK_VILTE_CAPABILITY));
}

TEST_F(AosHandleTest, HoldBlockForInvalidNetwork_Test2)
{
    // Test2: Block for mobile, Invalid network
    // Expectation: return false if the block reason has been blocked, otherwise return true

    SetHoldingBlocksPolicyForMobile();
    SetHoldingBlocksPolicyForWifi();

    EXPECT_CALL(m_objMockIAosNetTracker, GetMobileNetworkType())
            .Times(AnyNumber())
            .WillRepeatedly(Return(NW_REPORT_RADIO_INVALID));

    AddHoldingBlockForMobile(AosHandle::BLOCK_VOPS);
    m_pAosHandle->AddBlock(AosHandle::BLOCK_VOPS, m_pAosHandle->m_nBlocks);

    EXPECT_FALSE(m_pAosHandle->HoldBlockForInvalidNetwork(AosHandle::BLOCK_VOPS, IMS_FALSE));
    EXPECT_FALSE(m_pAosHandle->IsHoldingBlockForMobile(AosHandle::BLOCK_VOPS));

    EXPECT_TRUE(
            m_pAosHandle->HoldBlockForInvalidNetwork(AosHandle::BLOCK_VILTE_CAPABILITY, IMS_TRUE));
    EXPECT_TRUE(m_pAosHandle->IsHoldingBlockForMobile(AosHandle::BLOCK_VILTE_CAPABILITY));
}

TEST_F(AosHandleTest, HoldBlockForInvalidNetwork_Test3)
{
    // Test3: Block for Wifi, Wfc not available
    // Expectation: return false

    SetHoldingBlocksPolicyForMobile();
    SetHoldingBlocksPolicyForWifi();

    EXPECT_CALL(m_objMockIAosNConfiguration, IsWfcImsAvailable())
            .Times(AnyNumber())
            .WillRepeatedly(Return(IMS_FALSE));

    EXPECT_FALSE(
            m_pAosHandle->HoldBlockForInvalidNetwork(AosHandle::BLOCK_VIWIFI_CAPABILITY, IMS_TRUE));
    EXPECT_FALSE(IsHoldingBlockForWifi(AosHandle::BLOCK_VIWIFI_CAPABILITY));
}

TEST_F(AosHandleTest, HoldBlockForInvalidNetwork_Test4)
{
    // Test4: Block for Wifi, Wfc available, Wifi connected
    // Expectation: return false

    SetHoldingBlocksPolicyForMobile();
    SetHoldingBlocksPolicyForWifi();

    EXPECT_CALL(m_objMockIAosNConfiguration, IsWfcImsAvailable())
            .Times(AnyNumber())
            .WillRepeatedly(Return(IMS_TRUE));

    SetWifiWatcher(static_cast<IWifiWatcher*>(&m_objMockIWifiWatcher));
    EXPECT_CALL(m_objMockIWifiWatcher, GetState())
            .Times(AnyNumber())
            .WillRepeatedly(Return(IWifiWatcher::STATE_CONNECTED));

    EXPECT_FALSE(
            m_pAosHandle->HoldBlockForInvalidNetwork(AosHandle::BLOCK_VIWIFI_CAPABILITY, IMS_TRUE));
    EXPECT_FALSE(IsHoldingBlockForWifi(AosHandle::BLOCK_VIWIFI_CAPABILITY));
}

TEST_F(AosHandleTest, HoldBlockForInvalidNetwork_Test5)
{
    // Test5: Block for Wifi, Wfc available, Wifi disconnected
    // Expectation: return false if the block reason has been blocked, otherwise return true

    SetHoldingBlocksPolicyForMobile();
    SetHoldingBlocksPolicyForWifi();

    EXPECT_CALL(m_objMockIAosNConfiguration, IsWfcImsAvailable())
            .Times(AnyNumber())
            .WillRepeatedly(Return(IMS_TRUE));

    SetWifiWatcher(static_cast<IWifiWatcher*>(&m_objMockIWifiWatcher));
    EXPECT_CALL(m_objMockIWifiWatcher, GetState())
            .Times(AnyNumber())
            .WillRepeatedly(Return(IWifiWatcher::STATE_DISCONNECTED));

    AddHoldingBlockForWifi(AosHandle::BLOCK_VOWIFI_CAPABILITY);
    AddHoldingBlockForWifi(AosHandle::BLOCK_VIWIFI_CAPABILITY);
    m_pAosHandle->AddBlock(AosHandle::BLOCK_VIWIFI_CAPABILITY, m_pAosHandle->m_nBlocks);

    EXPECT_FALSE(m_pAosHandle->HoldBlockForInvalidNetwork(
            AosHandle::BLOCK_VIWIFI_CAPABILITY, IMS_FALSE));
    EXPECT_FALSE(IsHoldingBlockForWifi(AosHandle::BLOCK_VIWIFI_CAPABILITY));

    EXPECT_TRUE(
            m_pAosHandle->HoldBlockForInvalidNetwork(AosHandle::BLOCK_VOWIFI_CAPABILITY, IMS_TRUE));
    EXPECT_TRUE(IsHoldingBlockForWifi(AosHandle::BLOCK_VOWIFI_CAPABILITY));
}

TEST_F(AosHandleTest, HoldBlockForInvalidNetwork_Test6)
{
    // Test6: Block for nither mobile nor wifi
    // Expectation: return false

    SetHoldingBlocksPolicyForTest();

    EXPECT_FALSE(
            m_pAosHandle->HoldBlockForInvalidNetwork(AosHandle::BLOCK_SMS_CAPABILITY, IMS_TRUE));
    EXPECT_FALSE(
            m_pAosHandle->HoldBlockForInvalidNetwork(AosHandle::BLOCK_SMS_CAPABILITY, IMS_FALSE));
}

TEST_F(AosHandleTest, ReevaluateBlocks_Test1)
{
    // Test1: Epdg enabled, mobile block existed
    ClearBlocks();
    ClearHoldingBlocksForMobile();

    ClearHoldingBlocksPolicyForMobile();
    SetHoldingBlocksPolicyForMobile();

    SetEpdgEnabled(IMS_TRUE);
    m_pAosHandle->AddBlock(AosHandle::BLOCK_VOLTE_CAPABILITY, m_pAosHandle->m_nBlocks);
    m_pAosHandle->AddBlock(AosHandle::BLOCK_VILTE_CAPABILITY, m_pAosHandle->m_nBlocks);
    m_pAosHandle->AddBlock(AosHandle::BLOCK_VOPS, m_pAosHandle->m_nBlocks);

    EXPECT_TRUE(m_pAosHandle->IsHandleBlocked(AosHandle::BLOCK_VOLTE_CAPABILITY));
    EXPECT_TRUE(m_pAosHandle->IsHandleBlocked(AosHandle::BLOCK_VILTE_CAPABILITY));
    EXPECT_TRUE(m_pAosHandle->IsHandleBlocked(AosHandle::BLOCK_VOPS));
    EXPECT_FALSE(m_pAosHandle->IsHoldingBlockForMobile(AosHandle::BLOCK_VOLTE_CAPABILITY));
    EXPECT_FALSE(m_pAosHandle->IsHoldingBlockForMobile(AosHandle::BLOCK_VILTE_CAPABILITY));
    EXPECT_FALSE(m_pAosHandle->IsHoldingBlockForMobile(AosHandle::BLOCK_VOPS));

    m_pAosHandle->ReevaluateBlocks();

    EXPECT_FALSE(m_pAosHandle->IsHandleBlocked(AosHandle::BLOCK_VOLTE_CAPABILITY));
    EXPECT_FALSE(m_pAosHandle->IsHandleBlocked(AosHandle::BLOCK_VILTE_CAPABILITY));
    EXPECT_FALSE(m_pAosHandle->IsHandleBlocked(AosHandle::BLOCK_VOPS));
    EXPECT_TRUE(m_pAosHandle->IsHoldingBlockForMobile(AosHandle::BLOCK_VOLTE_CAPABILITY));
    EXPECT_TRUE(m_pAosHandle->IsHoldingBlockForMobile(AosHandle::BLOCK_VILTE_CAPABILITY));
    EXPECT_TRUE(m_pAosHandle->IsHoldingBlockForMobile(AosHandle::BLOCK_VOPS));

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

    EXPECT_FALSE(m_pAosHandle->IsHandleBlocked(AosHandle::BLOCK_VOLTE_CAPABILITY));
    EXPECT_FALSE(m_pAosHandle->IsHandleBlocked(AosHandle::BLOCK_VILTE_CAPABILITY));
    EXPECT_FALSE(m_pAosHandle->IsHandleBlocked(AosHandle::BLOCK_VOPS));
    EXPECT_FALSE(m_pAosHandle->IsHoldingBlockForMobile(AosHandle::BLOCK_VOLTE_CAPABILITY));
    EXPECT_FALSE(m_pAosHandle->IsHoldingBlockForMobile(AosHandle::BLOCK_VILTE_CAPABILITY));
    EXPECT_FALSE(m_pAosHandle->IsHoldingBlockForMobile(AosHandle::BLOCK_VOPS));

    m_pAosHandle->ReevaluateBlocks();

    EXPECT_FALSE(m_pAosHandle->IsHandleBlocked(AosHandle::BLOCK_VOLTE_CAPABILITY));
    EXPECT_FALSE(m_pAosHandle->IsHandleBlocked(AosHandle::BLOCK_VILTE_CAPABILITY));
    EXPECT_FALSE(m_pAosHandle->IsHandleBlocked(AosHandle::BLOCK_VOPS));
    EXPECT_FALSE(m_pAosHandle->IsHoldingBlockForMobile(AosHandle::BLOCK_VOLTE_CAPABILITY));
    EXPECT_FALSE(m_pAosHandle->IsHoldingBlockForMobile(AosHandle::BLOCK_VILTE_CAPABILITY));
    EXPECT_FALSE(m_pAosHandle->IsHoldingBlockForMobile(AosHandle::BLOCK_VOPS));

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

    EXPECT_FALSE(m_pAosHandle->IsHandleBlocked(AosHandle::BLOCK_VOWIFI_CAPABILITY));
    EXPECT_FALSE(m_pAosHandle->IsHandleBlocked(AosHandle::BLOCK_VIWIFI_CAPABILITY));
    EXPECT_TRUE(IsHoldingBlockForWifi(AosHandle::BLOCK_VOWIFI_CAPABILITY));
    EXPECT_TRUE(IsHoldingBlockForWifi(AosHandle::BLOCK_VIWIFI_CAPABILITY));

    m_pAosHandle->ReevaluateBlocks();

    EXPECT_TRUE(m_pAosHandle->IsHandleBlocked(AosHandle::BLOCK_VOWIFI_CAPABILITY));
    EXPECT_TRUE(m_pAosHandle->IsHandleBlocked(AosHandle::BLOCK_VIWIFI_CAPABILITY));
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

    EXPECT_FALSE(m_pAosHandle->IsHandleBlocked(AosHandle::BLOCK_VOWIFI_CAPABILITY));
    EXPECT_FALSE(m_pAosHandle->IsHandleBlocked(AosHandle::BLOCK_VIWIFI_CAPABILITY));
    EXPECT_FALSE(IsHoldingBlockForWifi(AosHandle::BLOCK_VOWIFI_CAPABILITY));
    EXPECT_FALSE(IsHoldingBlockForWifi(AosHandle::BLOCK_VIWIFI_CAPABILITY));

    m_pAosHandle->ReevaluateBlocks();

    EXPECT_FALSE(m_pAosHandle->IsHandleBlocked(AosHandle::BLOCK_VOWIFI_CAPABILITY));
    EXPECT_FALSE(m_pAosHandle->IsHandleBlocked(AosHandle::BLOCK_VIWIFI_CAPABILITY));
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
    m_pAosHandle->AddBlock(AosHandle::BLOCK_VOWIFI_CAPABILITY, m_pAosHandle->m_nBlocks);
    m_pAosHandle->AddBlock(AosHandle::BLOCK_VIWIFI_CAPABILITY, m_pAosHandle->m_nBlocks);

    EXPECT_TRUE(m_pAosHandle->IsHandleBlocked(AosHandle::BLOCK_VOWIFI_CAPABILITY));
    EXPECT_TRUE(m_pAosHandle->IsHandleBlocked(AosHandle::BLOCK_VIWIFI_CAPABILITY));
    EXPECT_FALSE(IsHoldingBlockForWifi(AosHandle::BLOCK_VOWIFI_CAPABILITY));
    EXPECT_FALSE(IsHoldingBlockForWifi(AosHandle::BLOCK_VIWIFI_CAPABILITY));

    m_pAosHandle->ReevaluateBlocks();

    EXPECT_FALSE(m_pAosHandle->IsHandleBlocked(AosHandle::BLOCK_VOWIFI_CAPABILITY));
    EXPECT_FALSE(m_pAosHandle->IsHandleBlocked(AosHandle::BLOCK_VIWIFI_CAPABILITY));
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

    EXPECT_FALSE(m_pAosHandle->IsHandleBlocked(AosHandle::BLOCK_VOWIFI_CAPABILITY));
    EXPECT_FALSE(m_pAosHandle->IsHandleBlocked(AosHandle::BLOCK_VIWIFI_CAPABILITY));
    EXPECT_FALSE(IsHoldingBlockForWifi(AosHandle::BLOCK_VOWIFI_CAPABILITY));
    EXPECT_FALSE(IsHoldingBlockForWifi(AosHandle::BLOCK_VIWIFI_CAPABILITY));

    m_pAosHandle->ReevaluateBlocks();

    EXPECT_FALSE(m_pAosHandle->IsHandleBlocked(AosHandle::BLOCK_VOWIFI_CAPABILITY));
    EXPECT_FALSE(m_pAosHandle->IsHandleBlocked(AosHandle::BLOCK_VIWIFI_CAPABILITY));
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

    EXPECT_FALSE(m_pAosHandle->IsHandleBlocked(AosHandle::BLOCK_VOLTE_CAPABILITY));
    EXPECT_FALSE(m_pAosHandle->IsHandleBlocked(AosHandle::BLOCK_VILTE_CAPABILITY));
    EXPECT_FALSE(m_pAosHandle->IsHandleBlocked(AosHandle::BLOCK_VOPS));
    EXPECT_TRUE(m_pAosHandle->IsHoldingBlockForMobile(AosHandle::BLOCK_VOLTE_CAPABILITY));
    EXPECT_TRUE(m_pAosHandle->IsHoldingBlockForMobile(AosHandle::BLOCK_VILTE_CAPABILITY));
    EXPECT_TRUE(m_pAosHandle->IsHoldingBlockForMobile(AosHandle::BLOCK_VOPS));

    m_pAosHandle->ReevaluateBlocks();

    EXPECT_TRUE(m_pAosHandle->IsHandleBlocked(AosHandle::BLOCK_VOLTE_CAPABILITY));
    EXPECT_TRUE(m_pAosHandle->IsHandleBlocked(AosHandle::BLOCK_VILTE_CAPABILITY));
    EXPECT_TRUE(m_pAosHandle->IsHandleBlocked(AosHandle::BLOCK_VOPS));
    EXPECT_FALSE(m_pAosHandle->IsHoldingBlockForMobile(AosHandle::BLOCK_VOLTE_CAPABILITY));
    EXPECT_FALSE(m_pAosHandle->IsHoldingBlockForMobile(AosHandle::BLOCK_VILTE_CAPABILITY));
    EXPECT_FALSE(m_pAosHandle->IsHoldingBlockForMobile(AosHandle::BLOCK_VOPS));

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

    EXPECT_FALSE(m_pAosHandle->IsHandleBlocked(AosHandle::BLOCK_VOLTE_CAPABILITY));
    EXPECT_FALSE(m_pAosHandle->IsHandleBlocked(AosHandle::BLOCK_VILTE_CAPABILITY));
    EXPECT_FALSE(m_pAosHandle->IsHandleBlocked(AosHandle::BLOCK_VOPS));
    EXPECT_FALSE(m_pAosHandle->IsHoldingBlockForMobile(AosHandle::BLOCK_VOLTE_CAPABILITY));
    EXPECT_FALSE(m_pAosHandle->IsHoldingBlockForMobile(AosHandle::BLOCK_VILTE_CAPABILITY));
    EXPECT_FALSE(m_pAosHandle->IsHoldingBlockForMobile(AosHandle::BLOCK_VOPS));

    m_pAosHandle->ReevaluateBlocks();

    EXPECT_FALSE(m_pAosHandle->IsHandleBlocked(AosHandle::BLOCK_VOLTE_CAPABILITY));
    EXPECT_FALSE(m_pAosHandle->IsHandleBlocked(AosHandle::BLOCK_VILTE_CAPABILITY));
    EXPECT_FALSE(m_pAosHandle->IsHandleBlocked(AosHandle::BLOCK_VOPS));
    EXPECT_FALSE(m_pAosHandle->IsHoldingBlockForMobile(AosHandle::BLOCK_VOLTE_CAPABILITY));
    EXPECT_FALSE(m_pAosHandle->IsHoldingBlockForMobile(AosHandle::BLOCK_VILTE_CAPABILITY));
    EXPECT_FALSE(m_pAosHandle->IsHoldingBlockForMobile(AosHandle::BLOCK_VOPS));

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

    EXPECT_TRUE(m_pAosHandle->UpdateIpcan());
    EXPECT_FALSE(m_pAosHandle->UpdateIpcan());
    EXPECT_TRUE(m_pAosHandle->UpdateIpcan());
    EXPECT_FALSE(m_pAosHandle->UpdateIpcan());
}

TEST_F(AosHandleTest, PreProcessBlock_Test1)
{
    // Test1: Epdg enabled, Block for wifi
    // Expectation: The block should not be held/released for mobile

    ClearHoldingBlocksForMobile();
    ClearHoldingBlocksPolicyForMobile();
    SetHoldingBlocksPolicyForMobile();

    SetEpdgEnabled(IMS_TRUE);

    EXPECT_FALSE(m_pAosHandle->IsHoldingBlockForMobile(AosHandle::BLOCK_VOWIFI_CAPABILITY));
    EXPECT_FALSE(m_pAosHandle->IsHoldingBlockForMobile(AosHandle::BLOCK_VIWIFI_CAPABILITY));

    EXPECT_FALSE(m_pAosHandle->PreProcessBlock(AosHandle::BLOCK_VOWIFI_CAPABILITY, IMS_TRUE));
    EXPECT_FALSE(m_pAosHandle->PreProcessBlock(AosHandle::BLOCK_VIWIFI_CAPABILITY, IMS_TRUE));

    EXPECT_FALSE(m_pAosHandle->IsHoldingBlockForMobile(AosHandle::BLOCK_VOWIFI_CAPABILITY));
    EXPECT_FALSE(m_pAosHandle->IsHoldingBlockForMobile(AosHandle::BLOCK_VIWIFI_CAPABILITY));

    EXPECT_FALSE(m_pAosHandle->PreProcessBlock(AosHandle::BLOCK_VOWIFI_CAPABILITY, IMS_FALSE));
    EXPECT_FALSE(m_pAosHandle->PreProcessBlock(AosHandle::BLOCK_VIWIFI_CAPABILITY, IMS_FALSE));

    EXPECT_FALSE(m_pAosHandle->IsHoldingBlockForMobile(AosHandle::BLOCK_VOWIFI_CAPABILITY));
    EXPECT_FALSE(m_pAosHandle->IsHoldingBlockForMobile(AosHandle::BLOCK_VIWIFI_CAPABILITY));

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

    EXPECT_FALSE(m_pAosHandle->IsHoldingBlockForMobile(AosHandle::BLOCK_VOLTE_CAPABILITY));
    EXPECT_FALSE(m_pAosHandle->IsHoldingBlockForMobile(AosHandle::BLOCK_VILTE_CAPABILITY));
    EXPECT_FALSE(m_pAosHandle->IsHoldingBlockForMobile(AosHandle::BLOCK_VOPS));

    EXPECT_TRUE(m_pAosHandle->PreProcessBlock(AosHandle::BLOCK_VOLTE_CAPABILITY, IMS_TRUE));
    EXPECT_TRUE(m_pAosHandle->PreProcessBlock(AosHandle::BLOCK_VILTE_CAPABILITY, IMS_TRUE));
    EXPECT_TRUE(m_pAosHandle->PreProcessBlock(AosHandle::BLOCK_VOPS, IMS_TRUE));

    EXPECT_TRUE(m_pAosHandle->IsHoldingBlockForMobile(AosHandle::BLOCK_VOLTE_CAPABILITY));
    EXPECT_TRUE(m_pAosHandle->IsHoldingBlockForMobile(AosHandle::BLOCK_VILTE_CAPABILITY));
    EXPECT_TRUE(m_pAosHandle->IsHoldingBlockForMobile(AosHandle::BLOCK_VOPS));

    EXPECT_TRUE(m_pAosHandle->PreProcessBlock(AosHandle::BLOCK_VOLTE_CAPABILITY, IMS_FALSE));
    EXPECT_TRUE(m_pAosHandle->PreProcessBlock(AosHandle::BLOCK_VILTE_CAPABILITY, IMS_FALSE));
    EXPECT_TRUE(m_pAosHandle->PreProcessBlock(AosHandle::BLOCK_VOPS, IMS_FALSE));

    EXPECT_FALSE(m_pAosHandle->IsHoldingBlockForMobile(AosHandle::BLOCK_VOLTE_CAPABILITY));
    EXPECT_FALSE(m_pAosHandle->IsHoldingBlockForMobile(AosHandle::BLOCK_VILTE_CAPABILITY));
    EXPECT_FALSE(m_pAosHandle->IsHoldingBlockForMobile(AosHandle::BLOCK_VOPS));

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

    EXPECT_FALSE(m_pAosHandle->PreProcessBlock(AosHandle::BLOCK_VOLTE_CAPABILITY, IMS_TRUE));
    EXPECT_FALSE(m_pAosHandle->PreProcessBlock(AosHandle::BLOCK_VILTE_CAPABILITY, IMS_TRUE));
    EXPECT_FALSE(m_pAosHandle->PreProcessBlock(AosHandle::BLOCK_VOPS, IMS_TRUE));

    EXPECT_FALSE(IsHoldingBlockForWifi(AosHandle::BLOCK_VOLTE_CAPABILITY));
    EXPECT_FALSE(IsHoldingBlockForWifi(AosHandle::BLOCK_VILTE_CAPABILITY));
    EXPECT_FALSE(IsHoldingBlockForWifi(AosHandle::BLOCK_VOPS));

    EXPECT_FALSE(m_pAosHandle->PreProcessBlock(AosHandle::BLOCK_VOLTE_CAPABILITY, IMS_FALSE));
    EXPECT_FALSE(m_pAosHandle->PreProcessBlock(AosHandle::BLOCK_VILTE_CAPABILITY, IMS_FALSE));
    EXPECT_FALSE(m_pAosHandle->PreProcessBlock(AosHandle::BLOCK_VOPS, IMS_FALSE));

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

    EXPECT_TRUE(m_pAosHandle->PreProcessBlock(AosHandle::BLOCK_VOWIFI_CAPABILITY, IMS_TRUE));
    EXPECT_TRUE(m_pAosHandle->PreProcessBlock(AosHandle::BLOCK_VIWIFI_CAPABILITY, IMS_TRUE));

    EXPECT_TRUE(IsHoldingBlockForWifi(AosHandle::BLOCK_VOWIFI_CAPABILITY));
    EXPECT_TRUE(IsHoldingBlockForWifi(AosHandle::BLOCK_VIWIFI_CAPABILITY));

    EXPECT_TRUE(m_pAosHandle->PreProcessBlock(AosHandle::BLOCK_VOWIFI_CAPABILITY, IMS_FALSE));
    EXPECT_TRUE(m_pAosHandle->PreProcessBlock(AosHandle::BLOCK_VIWIFI_CAPABILITY, IMS_FALSE));

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

    EXPECT_FALSE(m_pAosHandle->IsHandleBlocked());
    EXPECT_EQ(m_pAosHandle->GetFeatureTagList().GetFeatures(),
            (ImsAosFeature::MMTEL | ImsAosFeature::VIDEO | ImsAosFeature::SMSIP));

    // Set
    m_pAosHandle->ProcessBlock(AosHandle::BLOCK_VOLTE_CAPABILITY, IMS_TRUE, IMS_FALSE);
    m_pAosHandle->ProcessBlock(AosHandle::BLOCK_VOWIFI_CAPABILITY, IMS_TRUE, IMS_FALSE);
    m_pAosHandle->ProcessBlock(AosHandle::BLOCK_VOPS, IMS_TRUE, IMS_FALSE);

    EXPECT_TRUE(m_pAosHandle->IsHandleBlocked(AosHandle::BLOCK_VOLTE_CAPABILITY));
    EXPECT_TRUE(m_pAosHandle->IsHandleBlocked(AosHandle::BLOCK_VOWIFI_CAPABILITY));
    EXPECT_TRUE(m_pAosHandle->IsHandleBlocked(AosHandle::BLOCK_VOPS));
    EXPECT_EQ(m_pAosHandle->GetFeatureTagList().GetFeatures(),
            (ImsAosFeature::VIDEO | ImsAosFeature::SMSIP));

    m_pAosHandle->ProcessBlock(AosHandle::BLOCK_VILTE_CAPABILITY, IMS_TRUE, IMS_FALSE);
    m_pAosHandle->ProcessBlock(AosHandle::BLOCK_VIWIFI_CAPABILITY, IMS_TRUE, IMS_FALSE);

    EXPECT_TRUE(m_pAosHandle->IsHandleBlocked(AosHandle::BLOCK_VILTE_CAPABILITY));
    EXPECT_TRUE(m_pAosHandle->IsHandleBlocked(AosHandle::BLOCK_VIWIFI_CAPABILITY));
    EXPECT_EQ(m_pAosHandle->GetFeatureTagList().GetFeatures(), ImsAosFeature::SMSIP);

    m_pAosHandle->ProcessBlock(AosHandle::BLOCK_SMS_CAPABILITY, IMS_TRUE, IMS_FALSE);
    m_pAosHandle->ProcessBlock(
            AosHandle::BLOCK_SMS_OVER_IP_NETWORK_INDICATION, IMS_TRUE, IMS_FALSE);

    EXPECT_TRUE(m_pAosHandle->IsHandleBlocked(AosHandle::BLOCK_SMS_CAPABILITY));
    EXPECT_TRUE(m_pAosHandle->IsHandleBlocked(AosHandle::BLOCK_SMS_OVER_IP_NETWORK_INDICATION));
    EXPECT_EQ(m_pAosHandle->GetFeatureTagList().GetFeatures(), ImsAosFeature::NONE);

    // Reset
    m_pAosHandle->ProcessBlock(AosHandle::BLOCK_VOLTE_CAPABILITY, IMS_FALSE, IMS_FALSE);
    m_pAosHandle->ProcessBlock(AosHandle::BLOCK_VOWIFI_CAPABILITY, IMS_FALSE, IMS_FALSE);
    m_pAosHandle->ProcessBlock(AosHandle::BLOCK_VOPS, IMS_FALSE, IMS_FALSE);

    EXPECT_FALSE(m_pAosHandle->IsHandleBlocked(AosHandle::BLOCK_VOLTE_CAPABILITY));
    EXPECT_FALSE(m_pAosHandle->IsHandleBlocked(AosHandle::BLOCK_VOWIFI_CAPABILITY));
    EXPECT_FALSE(m_pAosHandle->IsHandleBlocked(AosHandle::BLOCK_VOPS));
    EXPECT_EQ(m_pAosHandle->GetFeatureTagList().GetFeatures(), ImsAosFeature::MMTEL);

    m_pAosHandle->ProcessBlock(AosHandle::BLOCK_VILTE_CAPABILITY, IMS_FALSE, IMS_FALSE);
    m_pAosHandle->ProcessBlock(AosHandle::BLOCK_VIWIFI_CAPABILITY, IMS_FALSE, IMS_FALSE);

    EXPECT_FALSE(m_pAosHandle->IsHandleBlocked(AosHandle::BLOCK_VILTE_CAPABILITY));
    EXPECT_FALSE(m_pAosHandle->IsHandleBlocked(AosHandle::BLOCK_VIWIFI_CAPABILITY));
    EXPECT_EQ(m_pAosHandle->GetFeatureTagList().GetFeatures(),
            (ImsAosFeature::MMTEL | ImsAosFeature::VIDEO));

    m_pAosHandle->ProcessBlock(AosHandle::BLOCK_SMS_CAPABILITY, IMS_FALSE, IMS_FALSE);
    m_pAosHandle->ProcessBlock(
            AosHandle::BLOCK_SMS_OVER_IP_NETWORK_INDICATION, IMS_FALSE, IMS_FALSE);

    EXPECT_FALSE(m_pAosHandle->IsHandleBlocked(AosHandle::BLOCK_SMS_CAPABILITY));
    EXPECT_FALSE(m_pAosHandle->IsHandleBlocked(AosHandle::BLOCK_SMS_OVER_IP_NETWORK_INDICATION));
    EXPECT_EQ(m_pAosHandle->GetFeatureTagList().GetFeatures(),
            (ImsAosFeature::MMTEL | ImsAosFeature::VIDEO | ImsAosFeature::SMSIP));

    EXPECT_FALSE(m_pAosHandle->IsHandleBlocked());

    ClearBlocks();
    ClearFeatureTagList();
}

TEST_F(AosHandleTest, ProcessBlock_Test2)
{
    // Test2: Adding block / PreProcess / Wfc not available / Data Disconnected / Invalid network
    // Expectation: Mobile blocks are added to holding blocks.
    //              Wifi blocks are added to main blocks.

    // Initialization
    SetHoldingBlocksPolicyForMobile();
    SetHoldingBlocksPolicyForWifi();
    SetDataConnected(IMS_FALSE);

    AddFeature(ImsAosFeature::MMTEL);
    AddFeature(ImsAosFeature::VIDEO);
    AddFeature(ImsAosFeature::SMSIP);

    EXPECT_CALL(m_objMockIAosNConfiguration, IsWfcImsAvailable())
            .Times(AnyNumber())
            .WillRepeatedly(Return(IMS_FALSE));

    EXPECT_CALL(m_objMockIAosNetTracker, GetMobileNetworkType())
            .Times(AnyNumber())
            .WillRepeatedly(Return(NW_REPORT_RADIO_INVALID));

    EXPECT_FALSE(m_pAosHandle->IsHandleBlocked());
    EXPECT_EQ(m_pAosHandle->GetFeatureTagList().GetFeatures(),
            (ImsAosFeature::MMTEL | ImsAosFeature::VIDEO | ImsAosFeature::SMSIP));

    // Execution
    m_pAosHandle->ProcessBlock(AosHandle::BLOCK_VOLTE_CAPABILITY, IMS_TRUE, IMS_TRUE);
    m_pAosHandle->ProcessBlock(AosHandle::BLOCK_VILTE_CAPABILITY, IMS_TRUE, IMS_TRUE);
    m_pAosHandle->ProcessBlock(AosHandle::BLOCK_VOPS, IMS_TRUE, IMS_TRUE);
    m_pAosHandle->ProcessBlock(AosHandle::BLOCK_SMS_CAPABILITY, IMS_TRUE, IMS_TRUE);
    m_pAosHandle->ProcessBlock(AosHandle::BLOCK_SMS_OVER_IP_NETWORK_INDICATION, IMS_TRUE, IMS_TRUE);
    m_pAosHandle->ProcessBlock(AosHandle::BLOCK_VOWIFI_CAPABILITY, IMS_TRUE, IMS_TRUE);
    m_pAosHandle->ProcessBlock(AosHandle::BLOCK_VIWIFI_CAPABILITY, IMS_TRUE, IMS_TRUE);

    // Checking result
    EXPECT_FALSE(m_pAosHandle->IsHandleBlocked(AosHandle::BLOCK_VOLTE_CAPABILITY));
    EXPECT_FALSE(m_pAosHandle->IsHandleBlocked(AosHandle::BLOCK_VILTE_CAPABILITY));
    EXPECT_FALSE(m_pAosHandle->IsHandleBlocked(AosHandle::BLOCK_VOPS));
    EXPECT_FALSE(m_pAosHandle->IsHandleBlocked(AosHandle::BLOCK_SMS_CAPABILITY));
    EXPECT_FALSE(m_pAosHandle->IsHandleBlocked(AosHandle::BLOCK_SMS_OVER_IP_NETWORK_INDICATION));
    EXPECT_TRUE(m_pAosHandle->IsHandleBlocked(AosHandle::BLOCK_VOWIFI_CAPABILITY));
    EXPECT_TRUE(m_pAosHandle->IsHandleBlocked(AosHandle::BLOCK_VIWIFI_CAPABILITY));

    EXPECT_TRUE(m_pAosHandle->IsHoldingBlockForMobile(AosHandle::BLOCK_VOLTE_CAPABILITY));
    EXPECT_TRUE(m_pAosHandle->IsHoldingBlockForMobile(AosHandle::BLOCK_VILTE_CAPABILITY));
    EXPECT_TRUE(m_pAosHandle->IsHoldingBlockForMobile(AosHandle::BLOCK_VOPS));
    EXPECT_TRUE(m_pAosHandle->IsHoldingBlockForMobile(AosHandle::BLOCK_SMS_CAPABILITY));
    EXPECT_TRUE(
            m_pAosHandle->IsHoldingBlockForMobile(AosHandle::BLOCK_SMS_OVER_IP_NETWORK_INDICATION));
    EXPECT_FALSE(m_pAosHandle->IsHoldingBlockForMobile(AosHandle::BLOCK_VOWIFI_CAPABILITY));
    EXPECT_FALSE(m_pAosHandle->IsHoldingBlockForMobile(AosHandle::BLOCK_VIWIFI_CAPABILITY));

    EXPECT_FALSE(IsHoldingBlockForWifi(AosHandle::BLOCK_VOLTE_CAPABILITY));
    EXPECT_FALSE(IsHoldingBlockForWifi(AosHandle::BLOCK_VILTE_CAPABILITY));
    EXPECT_FALSE(IsHoldingBlockForWifi(AosHandle::BLOCK_VOPS));
    EXPECT_FALSE(IsHoldingBlockForWifi(AosHandle::BLOCK_SMS_CAPABILITY));
    EXPECT_FALSE(IsHoldingBlockForWifi(AosHandle::BLOCK_SMS_OVER_IP_NETWORK_INDICATION));
    EXPECT_FALSE(IsHoldingBlockForWifi(AosHandle::BLOCK_VOWIFI_CAPABILITY));
    EXPECT_FALSE(IsHoldingBlockForWifi(AosHandle::BLOCK_VIWIFI_CAPABILITY));

    EXPECT_EQ(m_pAosHandle->GetFeatureTagList().GetFeatures(), ImsAosFeature::SMSIP);
}

TEST_F(AosHandleTest, ProcessBlock_Test3)
{
    // Test3: Removing block / PreProcess / Wfc not available / Data Disconnected / Invalid network
    // Expectation: All blocks are removed from both main and holding block.

    // Initialization
    SetHoldingBlocksPolicyForMobile();
    SetHoldingBlocksPolicyForWifi();

    AddHoldingBlockForMobile(AosHandle::BLOCK_VOLTE_CAPABILITY | AosHandle::BLOCK_VILTE_CAPABILITY |
            AosHandle::BLOCK_VOPS);
    AddHoldingBlockForWifi(AosHandle::BLOCK_VOWIFI_CAPABILITY | AosHandle::BLOCK_VIWIFI_CAPABILITY);

    m_pAosHandle->AddBlock(AosHandle::BLOCK_VOLTE_CAPABILITY | AosHandle::BLOCK_VILTE_CAPABILITY |
                    AosHandle::BLOCK_VOPS | AosHandle::BLOCK_SMS_CAPABILITY |
                    AosHandle::BLOCK_SMS_OVER_IP_NETWORK_INDICATION |
                    AosHandle::BLOCK_VOWIFI_CAPABILITY | AosHandle::BLOCK_VIWIFI_CAPABILITY,
            m_pAosHandle->m_nBlocks);

    SetDataConnected(IMS_FALSE);

    EXPECT_CALL(m_objMockIAosNConfiguration, IsWfcImsAvailable())
            .Times(AnyNumber())
            .WillRepeatedly(Return(IMS_FALSE));

    EXPECT_CALL(m_objMockIAosNetTracker, GetMobileNetworkType())
            .Times(AnyNumber())
            .WillRepeatedly(Return(NW_REPORT_RADIO_INVALID));

    // Execution
    m_pAosHandle->ProcessBlock(AosHandle::BLOCK_VOLTE_CAPABILITY, IMS_FALSE, IMS_TRUE);
    m_pAosHandle->ProcessBlock(AosHandle::BLOCK_VILTE_CAPABILITY, IMS_FALSE, IMS_TRUE);
    m_pAosHandle->ProcessBlock(AosHandle::BLOCK_VOPS, IMS_FALSE, IMS_TRUE);
    m_pAosHandle->ProcessBlock(AosHandle::BLOCK_SMS_CAPABILITY, IMS_FALSE, IMS_TRUE);
    m_pAosHandle->ProcessBlock(
            AosHandle::BLOCK_SMS_OVER_IP_NETWORK_INDICATION, IMS_FALSE, IMS_TRUE);
    m_pAosHandle->ProcessBlock(AosHandle::BLOCK_VOWIFI_CAPABILITY, IMS_FALSE, IMS_TRUE);
    m_pAosHandle->ProcessBlock(AosHandle::BLOCK_VIWIFI_CAPABILITY, IMS_FALSE, IMS_TRUE);

    // Checking result
    EXPECT_FALSE(m_pAosHandle->IsHandleBlocked(AosHandle::BLOCK_VOLTE_CAPABILITY));
    EXPECT_FALSE(m_pAosHandle->IsHandleBlocked(AosHandle::BLOCK_VILTE_CAPABILITY));
    EXPECT_FALSE(m_pAosHandle->IsHandleBlocked(AosHandle::BLOCK_VOPS));
    EXPECT_FALSE(m_pAosHandle->IsHandleBlocked(AosHandle::BLOCK_SMS_CAPABILITY));
    EXPECT_FALSE(m_pAosHandle->IsHandleBlocked(AosHandle::BLOCK_SMS_OVER_IP_NETWORK_INDICATION));
    EXPECT_FALSE(m_pAosHandle->IsHandleBlocked(AosHandle::BLOCK_VOWIFI_CAPABILITY));
    EXPECT_FALSE(m_pAosHandle->IsHandleBlocked(AosHandle::BLOCK_VIWIFI_CAPABILITY));

    EXPECT_FALSE(m_pAosHandle->IsHoldingBlockForMobile(AosHandle::BLOCK_VOLTE_CAPABILITY));
    EXPECT_FALSE(m_pAosHandle->IsHoldingBlockForMobile(AosHandle::BLOCK_VILTE_CAPABILITY));
    EXPECT_FALSE(m_pAosHandle->IsHoldingBlockForMobile(AosHandle::BLOCK_VOPS));
    EXPECT_FALSE(m_pAosHandle->IsHoldingBlockForMobile(AosHandle::BLOCK_SMS_CAPABILITY));
    EXPECT_FALSE(
            m_pAosHandle->IsHoldingBlockForMobile(AosHandle::BLOCK_SMS_OVER_IP_NETWORK_INDICATION));
    EXPECT_FALSE(m_pAosHandle->IsHoldingBlockForMobile(AosHandle::BLOCK_VOWIFI_CAPABILITY));
    EXPECT_FALSE(m_pAosHandle->IsHoldingBlockForMobile(AosHandle::BLOCK_VIWIFI_CAPABILITY));

    EXPECT_FALSE(IsHoldingBlockForWifi(AosHandle::BLOCK_VOLTE_CAPABILITY));
    EXPECT_FALSE(IsHoldingBlockForWifi(AosHandle::BLOCK_VILTE_CAPABILITY));
    EXPECT_FALSE(IsHoldingBlockForWifi(AosHandle::BLOCK_VOPS));
    EXPECT_FALSE(IsHoldingBlockForWifi(AosHandle::BLOCK_SMS_CAPABILITY));
    EXPECT_FALSE(IsHoldingBlockForWifi(AosHandle::BLOCK_SMS_OVER_IP_NETWORK_INDICATION));
    EXPECT_FALSE(IsHoldingBlockForWifi(AosHandle::BLOCK_VOWIFI_CAPABILITY));
    EXPECT_FALSE(IsHoldingBlockForWifi(AosHandle::BLOCK_VIWIFI_CAPABILITY));

    EXPECT_EQ(m_pAosHandle->GetFeatureTagList().GetFeatures(),
            (ImsAosFeature::MMTEL | ImsAosFeature::VIDEO | ImsAosFeature::SMSIP));
}

TEST_F(AosHandleTest, ProcessBlock_Test4)
{
    // Test4: Adding block / PreProcess / Wfc available / Data Disconnected / Invalid network
    // Expectation: All blocks are added to holding blocks.

    // Initialization
    SetHoldingBlocksPolicyForMobile();
    SetHoldingBlocksPolicyForWifi();
    SetDataConnected(IMS_FALSE);

    AddFeature(ImsAosFeature::MMTEL);
    AddFeature(ImsAosFeature::VIDEO);
    AddFeature(ImsAosFeature::SMSIP);

    EXPECT_CALL(m_objMockIAosNConfiguration, IsWfcImsAvailable())
            .Times(AnyNumber())
            .WillRepeatedly(Return(IMS_TRUE));

    SetWifiWatcher(static_cast<IWifiWatcher*>(&m_objMockIWifiWatcher));
    EXPECT_CALL(m_objMockIWifiWatcher, GetState())
            .Times(AnyNumber())
            .WillRepeatedly(Return(IWifiWatcher::STATE_DISCONNECTED));

    EXPECT_CALL(m_objMockIAosNetTracker, GetMobileNetworkType())
            .Times(AnyNumber())
            .WillRepeatedly(Return(NW_REPORT_RADIO_INVALID));

    EXPECT_FALSE(m_pAosHandle->IsHandleBlocked());
    EXPECT_EQ(m_pAosHandle->GetFeatureTagList().GetFeatures(),
            (ImsAosFeature::MMTEL | ImsAosFeature::VIDEO | ImsAosFeature::SMSIP));

    // Execution
    m_pAosHandle->ProcessBlock(AosHandle::BLOCK_VOLTE_CAPABILITY, IMS_TRUE, IMS_TRUE);
    m_pAosHandle->ProcessBlock(AosHandle::BLOCK_VILTE_CAPABILITY, IMS_TRUE, IMS_TRUE);
    m_pAosHandle->ProcessBlock(AosHandle::BLOCK_VOPS, IMS_TRUE, IMS_TRUE);
    m_pAosHandle->ProcessBlock(AosHandle::BLOCK_SMS_CAPABILITY, IMS_TRUE, IMS_TRUE);
    m_pAosHandle->ProcessBlock(AosHandle::BLOCK_SMS_OVER_IP_NETWORK_INDICATION, IMS_TRUE, IMS_TRUE);
    m_pAosHandle->ProcessBlock(AosHandle::BLOCK_VOWIFI_CAPABILITY, IMS_TRUE, IMS_TRUE);
    m_pAosHandle->ProcessBlock(AosHandle::BLOCK_VIWIFI_CAPABILITY, IMS_TRUE, IMS_TRUE);

    // Checking result
    EXPECT_FALSE(m_pAosHandle->IsHandleBlocked(AosHandle::BLOCK_VOLTE_CAPABILITY));
    EXPECT_FALSE(m_pAosHandle->IsHandleBlocked(AosHandle::BLOCK_VILTE_CAPABILITY));
    EXPECT_FALSE(m_pAosHandle->IsHandleBlocked(AosHandle::BLOCK_VOPS));
    EXPECT_FALSE(m_pAosHandle->IsHandleBlocked(AosHandle::BLOCK_SMS_CAPABILITY));
    EXPECT_FALSE(m_pAosHandle->IsHandleBlocked(AosHandle::BLOCK_SMS_OVER_IP_NETWORK_INDICATION));
    EXPECT_FALSE(m_pAosHandle->IsHandleBlocked(AosHandle::BLOCK_VOWIFI_CAPABILITY));
    EXPECT_FALSE(m_pAosHandle->IsHandleBlocked(AosHandle::BLOCK_VIWIFI_CAPABILITY));

    EXPECT_TRUE(m_pAosHandle->IsHoldingBlockForMobile(AosHandle::BLOCK_VOLTE_CAPABILITY));
    EXPECT_TRUE(m_pAosHandle->IsHoldingBlockForMobile(AosHandle::BLOCK_VILTE_CAPABILITY));
    EXPECT_TRUE(m_pAosHandle->IsHoldingBlockForMobile(AosHandle::BLOCK_VOPS));
    EXPECT_TRUE(m_pAosHandle->IsHoldingBlockForMobile(AosHandle::BLOCK_SMS_CAPABILITY));
    EXPECT_TRUE(
            m_pAosHandle->IsHoldingBlockForMobile(AosHandle::BLOCK_SMS_OVER_IP_NETWORK_INDICATION));
    EXPECT_FALSE(m_pAosHandle->IsHoldingBlockForMobile(AosHandle::BLOCK_VOWIFI_CAPABILITY));
    EXPECT_FALSE(m_pAosHandle->IsHoldingBlockForMobile(AosHandle::BLOCK_VIWIFI_CAPABILITY));

    EXPECT_FALSE(IsHoldingBlockForWifi(AosHandle::BLOCK_VOLTE_CAPABILITY));
    EXPECT_FALSE(IsHoldingBlockForWifi(AosHandle::BLOCK_VILTE_CAPABILITY));
    EXPECT_FALSE(IsHoldingBlockForWifi(AosHandle::BLOCK_VOPS));
    EXPECT_FALSE(IsHoldingBlockForWifi(AosHandle::BLOCK_SMS_CAPABILITY));
    EXPECT_FALSE(IsHoldingBlockForWifi(AosHandle::BLOCK_SMS_OVER_IP_NETWORK_INDICATION));
    EXPECT_TRUE(IsHoldingBlockForWifi(AosHandle::BLOCK_VOWIFI_CAPABILITY));
    EXPECT_TRUE(IsHoldingBlockForWifi(AosHandle::BLOCK_VIWIFI_CAPABILITY));

    EXPECT_EQ(m_pAosHandle->GetFeatureTagList().GetFeatures(),
            (ImsAosFeature::MMTEL | ImsAosFeature::VIDEO | ImsAosFeature::SMSIP));
}

TEST_F(AosHandleTest, ProcessBlock_Test5)
{
    // Test5: Removing block / PreProcess / Wfc available / Data Disconnected / Invalid network
    // Expectation: All blocks are removed from both main and holding block.

    // Initialization
    SetHoldingBlocksPolicyForMobile();
    SetHoldingBlocksPolicyForWifi();

    AddHoldingBlockForMobile(AosHandle::BLOCK_VOLTE_CAPABILITY | AosHandle::BLOCK_VILTE_CAPABILITY |
            AosHandle::BLOCK_VOPS);
    AddHoldingBlockForWifi(AosHandle::BLOCK_VOWIFI_CAPABILITY | AosHandle::BLOCK_VIWIFI_CAPABILITY);

    m_pAosHandle->AddBlock(AosHandle::BLOCK_VOLTE_CAPABILITY | AosHandle::BLOCK_VILTE_CAPABILITY |
                    AosHandle::BLOCK_VOPS | AosHandle::BLOCK_SMS_CAPABILITY |
                    AosHandle::BLOCK_SMS_OVER_IP_NETWORK_INDICATION |
                    AosHandle::BLOCK_VOWIFI_CAPABILITY | AosHandle::BLOCK_VIWIFI_CAPABILITY,
            m_pAosHandle->m_nBlocks);

    SetDataConnected(IMS_FALSE);

    EXPECT_CALL(m_objMockIAosNConfiguration, IsWfcImsAvailable())
            .Times(AnyNumber())
            .WillRepeatedly(Return(IMS_TRUE));

    SetWifiWatcher(static_cast<IWifiWatcher*>(&m_objMockIWifiWatcher));
    EXPECT_CALL(m_objMockIWifiWatcher, GetState())
            .Times(AnyNumber())
            .WillRepeatedly(Return(IWifiWatcher::STATE_DISCONNECTED));

    EXPECT_CALL(m_objMockIAosNetTracker, GetMobileNetworkType())
            .Times(AnyNumber())
            .WillRepeatedly(Return(NW_REPORT_RADIO_INVALID));

    // Execution
    m_pAosHandle->ProcessBlock(AosHandle::BLOCK_VOLTE_CAPABILITY, IMS_FALSE, IMS_TRUE);
    m_pAosHandle->ProcessBlock(AosHandle::BLOCK_VILTE_CAPABILITY, IMS_FALSE, IMS_TRUE);
    m_pAosHandle->ProcessBlock(AosHandle::BLOCK_VOPS, IMS_FALSE, IMS_TRUE);
    m_pAosHandle->ProcessBlock(AosHandle::BLOCK_SMS_CAPABILITY, IMS_FALSE, IMS_TRUE);
    m_pAosHandle->ProcessBlock(
            AosHandle::BLOCK_SMS_OVER_IP_NETWORK_INDICATION, IMS_FALSE, IMS_TRUE);
    m_pAosHandle->ProcessBlock(AosHandle::BLOCK_VOWIFI_CAPABILITY, IMS_FALSE, IMS_TRUE);
    m_pAosHandle->ProcessBlock(AosHandle::BLOCK_VIWIFI_CAPABILITY, IMS_FALSE, IMS_TRUE);

    // Checking result
    EXPECT_FALSE(m_pAosHandle->IsHandleBlocked(AosHandle::BLOCK_VOLTE_CAPABILITY));
    EXPECT_FALSE(m_pAosHandle->IsHandleBlocked(AosHandle::BLOCK_VILTE_CAPABILITY));
    EXPECT_FALSE(m_pAosHandle->IsHandleBlocked(AosHandle::BLOCK_VOPS));
    EXPECT_FALSE(m_pAosHandle->IsHandleBlocked(AosHandle::BLOCK_SMS_CAPABILITY));
    EXPECT_FALSE(m_pAosHandle->IsHandleBlocked(AosHandle::BLOCK_SMS_OVER_IP_NETWORK_INDICATION));
    EXPECT_FALSE(m_pAosHandle->IsHandleBlocked(AosHandle::BLOCK_VOWIFI_CAPABILITY));
    EXPECT_FALSE(m_pAosHandle->IsHandleBlocked(AosHandle::BLOCK_VIWIFI_CAPABILITY));

    EXPECT_FALSE(m_pAosHandle->IsHoldingBlockForMobile(AosHandle::BLOCK_VOLTE_CAPABILITY));
    EXPECT_FALSE(m_pAosHandle->IsHoldingBlockForMobile(AosHandle::BLOCK_VILTE_CAPABILITY));
    EXPECT_FALSE(m_pAosHandle->IsHoldingBlockForMobile(AosHandle::BLOCK_VOPS));
    EXPECT_FALSE(m_pAosHandle->IsHoldingBlockForMobile(AosHandle::BLOCK_SMS_CAPABILITY));
    EXPECT_FALSE(
            m_pAosHandle->IsHoldingBlockForMobile(AosHandle::BLOCK_SMS_OVER_IP_NETWORK_INDICATION));
    EXPECT_FALSE(m_pAosHandle->IsHoldingBlockForMobile(AosHandle::BLOCK_VOWIFI_CAPABILITY));
    EXPECT_FALSE(m_pAosHandle->IsHoldingBlockForMobile(AosHandle::BLOCK_VIWIFI_CAPABILITY));

    EXPECT_FALSE(IsHoldingBlockForWifi(AosHandle::BLOCK_VOLTE_CAPABILITY));
    EXPECT_FALSE(IsHoldingBlockForWifi(AosHandle::BLOCK_VILTE_CAPABILITY));
    EXPECT_FALSE(IsHoldingBlockForWifi(AosHandle::BLOCK_VOPS));
    EXPECT_FALSE(IsHoldingBlockForWifi(AosHandle::BLOCK_SMS_CAPABILITY));
    EXPECT_FALSE(IsHoldingBlockForWifi(AosHandle::BLOCK_SMS_OVER_IP_NETWORK_INDICATION));
    EXPECT_FALSE(IsHoldingBlockForWifi(AosHandle::BLOCK_VOWIFI_CAPABILITY));
    EXPECT_FALSE(IsHoldingBlockForWifi(AosHandle::BLOCK_VIWIFI_CAPABILITY));

    EXPECT_EQ(m_pAosHandle->GetFeatureTagList().GetFeatures(),
            (ImsAosFeature::MMTEL | ImsAosFeature::VIDEO | ImsAosFeature::SMSIP));
}

TEST_F(AosHandleTest, ProcessBlock_Test6)
{
    // Test6: Adding block / PreProcess / Wfc available / Data Disconnected / Valid network
    // Expectation: Mobile blocks are added to main blocks.
    //              Wifi blocks are added to holding blocks.

    // Initialization
    SetHoldingBlocksPolicyForMobile();
    SetHoldingBlocksPolicyForWifi();
    SetDataConnected(IMS_FALSE);
    SetEpdgEnabled(IMS_FALSE);

    AddFeature(ImsAosFeature::MMTEL);
    AddFeature(ImsAosFeature::VIDEO);
    AddFeature(ImsAosFeature::SMSIP);

    EXPECT_CALL(m_objMockIAosNConfiguration, IsWfcImsAvailable())
            .Times(AnyNumber())
            .WillRepeatedly(Return(IMS_TRUE));

    SetWifiWatcher(static_cast<IWifiWatcher*>(&m_objMockIWifiWatcher));
    EXPECT_CALL(m_objMockIWifiWatcher, GetState())
            .Times(AnyNumber())
            .WillRepeatedly(Return(IWifiWatcher::STATE_CONNECTED));

    EXPECT_CALL(m_objMockIAosNetTracker, GetMobileNetworkType())
            .Times(AnyNumber())
            .WillRepeatedly(Return(NW_REPORT_RADIO_LTE));

    EXPECT_FALSE(m_pAosHandle->IsHandleBlocked());
    EXPECT_EQ(m_pAosHandle->GetFeatureTagList().GetFeatures(),
            (ImsAosFeature::MMTEL | ImsAosFeature::VIDEO | ImsAosFeature::SMSIP));

    // Execution
    m_pAosHandle->ProcessBlock(AosHandle::BLOCK_VOLTE_CAPABILITY, IMS_TRUE, IMS_TRUE);
    m_pAosHandle->ProcessBlock(AosHandle::BLOCK_VILTE_CAPABILITY, IMS_TRUE, IMS_TRUE);
    m_pAosHandle->ProcessBlock(AosHandle::BLOCK_VOPS, IMS_TRUE, IMS_TRUE);
    m_pAosHandle->ProcessBlock(AosHandle::BLOCK_SMS_CAPABILITY, IMS_TRUE, IMS_TRUE);
    m_pAosHandle->ProcessBlock(AosHandle::BLOCK_SMS_OVER_IP_NETWORK_INDICATION, IMS_TRUE, IMS_TRUE);
    m_pAosHandle->ProcessBlock(AosHandle::BLOCK_VOWIFI_CAPABILITY, IMS_TRUE, IMS_TRUE);
    m_pAosHandle->ProcessBlock(AosHandle::BLOCK_VIWIFI_CAPABILITY, IMS_TRUE, IMS_TRUE);

    // Checking result
    EXPECT_TRUE(m_pAosHandle->IsHandleBlocked(AosHandle::BLOCK_VOLTE_CAPABILITY));
    EXPECT_TRUE(m_pAosHandle->IsHandleBlocked(AosHandle::BLOCK_VILTE_CAPABILITY));
    EXPECT_TRUE(m_pAosHandle->IsHandleBlocked(AosHandle::BLOCK_VOPS));
    EXPECT_TRUE(m_pAosHandle->IsHandleBlocked(AosHandle::BLOCK_SMS_CAPABILITY));
    EXPECT_TRUE(m_pAosHandle->IsHandleBlocked(AosHandle::BLOCK_SMS_OVER_IP_NETWORK_INDICATION));
    EXPECT_FALSE(m_pAosHandle->IsHandleBlocked(AosHandle::BLOCK_VOWIFI_CAPABILITY));
    EXPECT_FALSE(m_pAosHandle->IsHandleBlocked(AosHandle::BLOCK_VIWIFI_CAPABILITY));

    EXPECT_FALSE(m_pAosHandle->IsHoldingBlockForMobile(AosHandle::BLOCK_VOLTE_CAPABILITY));
    EXPECT_FALSE(m_pAosHandle->IsHoldingBlockForMobile(AosHandle::BLOCK_VILTE_CAPABILITY));
    EXPECT_FALSE(m_pAosHandle->IsHoldingBlockForMobile(AosHandle::BLOCK_VOPS));
    EXPECT_FALSE(m_pAosHandle->IsHoldingBlockForMobile(AosHandle::BLOCK_SMS_CAPABILITY));
    EXPECT_FALSE(
            m_pAosHandle->IsHoldingBlockForMobile(AosHandle::BLOCK_SMS_OVER_IP_NETWORK_INDICATION));
    EXPECT_FALSE(m_pAosHandle->IsHoldingBlockForMobile(AosHandle::BLOCK_VOWIFI_CAPABILITY));
    EXPECT_FALSE(m_pAosHandle->IsHoldingBlockForMobile(AosHandle::BLOCK_VIWIFI_CAPABILITY));

    EXPECT_FALSE(IsHoldingBlockForWifi(AosHandle::BLOCK_VOLTE_CAPABILITY));
    EXPECT_FALSE(IsHoldingBlockForWifi(AosHandle::BLOCK_VILTE_CAPABILITY));
    EXPECT_FALSE(IsHoldingBlockForWifi(AosHandle::BLOCK_VOPS));
    EXPECT_FALSE(IsHoldingBlockForWifi(AosHandle::BLOCK_SMS_CAPABILITY));
    EXPECT_FALSE(IsHoldingBlockForWifi(AosHandle::BLOCK_SMS_OVER_IP_NETWORK_INDICATION));
    EXPECT_TRUE(IsHoldingBlockForWifi(AosHandle::BLOCK_VOWIFI_CAPABILITY));
    EXPECT_TRUE(IsHoldingBlockForWifi(AosHandle::BLOCK_VIWIFI_CAPABILITY));

    EXPECT_EQ(m_pAosHandle->GetFeatureTagList().GetFeatures(), ImsAosFeature::NONE);
}

TEST_F(AosHandleTest, ProcessBlock_Test7)
{
    // Test7: Removing block / PreProcess / Wfc available / Data Disconnected / Valid network
    // Expectation: All blocks are removed from both main and holding block.

    // Initialization
    SetHoldingBlocksPolicyForMobile();
    SetHoldingBlocksPolicyForWifi();

    AddHoldingBlockForMobile(AosHandle::BLOCK_VOLTE_CAPABILITY | AosHandle::BLOCK_VILTE_CAPABILITY |
            AosHandle::BLOCK_VOPS);
    AddHoldingBlockForWifi(AosHandle::BLOCK_VOWIFI_CAPABILITY | AosHandle::BLOCK_VIWIFI_CAPABILITY);

    m_pAosHandle->AddBlock(AosHandle::BLOCK_VOLTE_CAPABILITY | AosHandle::BLOCK_VILTE_CAPABILITY |
                    AosHandle::BLOCK_VOPS | AosHandle::BLOCK_SMS_CAPABILITY |
                    AosHandle::BLOCK_SMS_OVER_IP_NETWORK_INDICATION |
                    AosHandle::BLOCK_VOWIFI_CAPABILITY | AosHandle::BLOCK_VIWIFI_CAPABILITY,
            m_pAosHandle->m_nBlocks);

    SetDataConnected(IMS_FALSE);
    SetEpdgEnabled(IMS_FALSE);

    EXPECT_CALL(m_objMockIAosNConfiguration, IsWfcImsAvailable())
            .Times(AnyNumber())
            .WillRepeatedly(Return(IMS_FALSE));

    SetWifiWatcher(static_cast<IWifiWatcher*>(&m_objMockIWifiWatcher));
    EXPECT_CALL(m_objMockIWifiWatcher, GetState())
            .Times(AnyNumber())
            .WillRepeatedly(Return(IWifiWatcher::STATE_CONNECTED));

    EXPECT_CALL(m_objMockIAosNetTracker, GetMobileNetworkType())
            .Times(AnyNumber())
            .WillRepeatedly(Return(NW_REPORT_RADIO_LTE));

    // Execution
    m_pAosHandle->ProcessBlock(AosHandle::BLOCK_VOLTE_CAPABILITY, IMS_FALSE, IMS_TRUE);
    m_pAosHandle->ProcessBlock(AosHandle::BLOCK_VILTE_CAPABILITY, IMS_FALSE, IMS_TRUE);
    m_pAosHandle->ProcessBlock(AosHandle::BLOCK_VOPS, IMS_FALSE, IMS_TRUE);
    m_pAosHandle->ProcessBlock(AosHandle::BLOCK_SMS_CAPABILITY, IMS_FALSE, IMS_TRUE);
    m_pAosHandle->ProcessBlock(
            AosHandle::BLOCK_SMS_OVER_IP_NETWORK_INDICATION, IMS_FALSE, IMS_TRUE);
    m_pAosHandle->ProcessBlock(AosHandle::BLOCK_VOWIFI_CAPABILITY, IMS_FALSE, IMS_TRUE);
    m_pAosHandle->ProcessBlock(AosHandle::BLOCK_VIWIFI_CAPABILITY, IMS_FALSE, IMS_TRUE);

    // Checking result
    EXPECT_FALSE(m_pAosHandle->IsHandleBlocked(AosHandle::BLOCK_VOLTE_CAPABILITY));
    EXPECT_FALSE(m_pAosHandle->IsHandleBlocked(AosHandle::BLOCK_VILTE_CAPABILITY));
    EXPECT_FALSE(m_pAosHandle->IsHandleBlocked(AosHandle::BLOCK_VOPS));
    EXPECT_FALSE(m_pAosHandle->IsHandleBlocked(AosHandle::BLOCK_SMS_CAPABILITY));
    EXPECT_FALSE(m_pAosHandle->IsHandleBlocked(AosHandle::BLOCK_SMS_OVER_IP_NETWORK_INDICATION));
    EXPECT_FALSE(m_pAosHandle->IsHandleBlocked(AosHandle::BLOCK_VOWIFI_CAPABILITY));
    EXPECT_FALSE(m_pAosHandle->IsHandleBlocked(AosHandle::BLOCK_VIWIFI_CAPABILITY));

    EXPECT_FALSE(m_pAosHandle->IsHoldingBlockForMobile(AosHandle::BLOCK_VOLTE_CAPABILITY));
    EXPECT_FALSE(m_pAosHandle->IsHoldingBlockForMobile(AosHandle::BLOCK_VILTE_CAPABILITY));
    EXPECT_FALSE(m_pAosHandle->IsHoldingBlockForMobile(AosHandle::BLOCK_VOPS));
    EXPECT_FALSE(m_pAosHandle->IsHoldingBlockForMobile(AosHandle::BLOCK_SMS_CAPABILITY));
    EXPECT_FALSE(
            m_pAosHandle->IsHoldingBlockForMobile(AosHandle::BLOCK_SMS_OVER_IP_NETWORK_INDICATION));
    EXPECT_FALSE(m_pAosHandle->IsHoldingBlockForMobile(AosHandle::BLOCK_VOWIFI_CAPABILITY));
    EXPECT_FALSE(m_pAosHandle->IsHoldingBlockForMobile(AosHandle::BLOCK_VIWIFI_CAPABILITY));

    EXPECT_FALSE(IsHoldingBlockForWifi(AosHandle::BLOCK_VOLTE_CAPABILITY));
    EXPECT_FALSE(IsHoldingBlockForWifi(AosHandle::BLOCK_VILTE_CAPABILITY));
    EXPECT_FALSE(IsHoldingBlockForWifi(AosHandle::BLOCK_VOPS));
    EXPECT_FALSE(IsHoldingBlockForWifi(AosHandle::BLOCK_SMS_CAPABILITY));
    EXPECT_FALSE(IsHoldingBlockForWifi(AosHandle::BLOCK_SMS_OVER_IP_NETWORK_INDICATION));
    EXPECT_FALSE(IsHoldingBlockForWifi(AosHandle::BLOCK_VOWIFI_CAPABILITY));
    EXPECT_FALSE(IsHoldingBlockForWifi(AosHandle::BLOCK_VIWIFI_CAPABILITY));

    EXPECT_EQ(m_pAosHandle->GetFeatureTagList().GetFeatures(),
            (ImsAosFeature::MMTEL | ImsAosFeature::VIDEO | ImsAosFeature::SMSIP));
}

TEST_F(AosHandleTest, ProcessBlock_Test8)
{
    // Test8: Adding block / PreProcess / Wfc unavailable / Data connected
    // Expectation: All blocks are added to main blocks.

    // Initialization
    SetHoldingBlocksPolicyForMobile();
    SetHoldingBlocksPolicyForWifi();
    SetDataConnected(IMS_TRUE);

    AddFeature(ImsAosFeature::MMTEL);
    AddFeature(ImsAosFeature::VIDEO);
    AddFeature(ImsAosFeature::SMSIP);

    EXPECT_CALL(m_objMockIAosNConfiguration, IsWfcImsAvailable())
            .Times(AnyNumber())
            .WillRepeatedly(Return(IMS_FALSE));

    EXPECT_FALSE(m_pAosHandle->IsHandleBlocked());
    EXPECT_EQ(m_pAosHandle->GetFeatureTagList().GetFeatures(),
            (ImsAosFeature::MMTEL | ImsAosFeature::VIDEO | ImsAosFeature::SMSIP));

    // Execution
    m_pAosHandle->ProcessBlock(AosHandle::BLOCK_VOLTE_CAPABILITY, IMS_TRUE, IMS_TRUE);
    m_pAosHandle->ProcessBlock(AosHandle::BLOCK_VILTE_CAPABILITY, IMS_TRUE, IMS_TRUE);
    m_pAosHandle->ProcessBlock(AosHandle::BLOCK_VOPS, IMS_TRUE, IMS_TRUE);
    m_pAosHandle->ProcessBlock(AosHandle::BLOCK_SMS_CAPABILITY, IMS_TRUE, IMS_TRUE);
    m_pAosHandle->ProcessBlock(AosHandle::BLOCK_SMS_OVER_IP_NETWORK_INDICATION, IMS_TRUE, IMS_TRUE);
    m_pAosHandle->ProcessBlock(AosHandle::BLOCK_VOWIFI_CAPABILITY, IMS_TRUE, IMS_TRUE);
    m_pAosHandle->ProcessBlock(AosHandle::BLOCK_VIWIFI_CAPABILITY, IMS_TRUE, IMS_TRUE);

    // Checking result
    EXPECT_TRUE(m_pAosHandle->IsHandleBlocked(AosHandle::BLOCK_VOLTE_CAPABILITY));
    EXPECT_TRUE(m_pAosHandle->IsHandleBlocked(AosHandle::BLOCK_VILTE_CAPABILITY));
    EXPECT_TRUE(m_pAosHandle->IsHandleBlocked(AosHandle::BLOCK_VOPS));
    EXPECT_TRUE(m_pAosHandle->IsHandleBlocked(AosHandle::BLOCK_SMS_CAPABILITY));
    EXPECT_TRUE(m_pAosHandle->IsHandleBlocked(AosHandle::BLOCK_SMS_OVER_IP_NETWORK_INDICATION));
    EXPECT_TRUE(m_pAosHandle->IsHandleBlocked(AosHandle::BLOCK_VOWIFI_CAPABILITY));
    EXPECT_TRUE(m_pAosHandle->IsHandleBlocked(AosHandle::BLOCK_VIWIFI_CAPABILITY));

    EXPECT_FALSE(m_pAosHandle->IsHoldingBlockForMobile(AosHandle::BLOCK_VOLTE_CAPABILITY));
    EXPECT_FALSE(m_pAosHandle->IsHoldingBlockForMobile(AosHandle::BLOCK_VILTE_CAPABILITY));
    EXPECT_FALSE(m_pAosHandle->IsHoldingBlockForMobile(AosHandle::BLOCK_VOPS));
    EXPECT_FALSE(m_pAosHandle->IsHoldingBlockForMobile(AosHandle::BLOCK_SMS_CAPABILITY));
    EXPECT_FALSE(
            m_pAosHandle->IsHoldingBlockForMobile(AosHandle::BLOCK_SMS_OVER_IP_NETWORK_INDICATION));
    EXPECT_FALSE(m_pAosHandle->IsHoldingBlockForMobile(AosHandle::BLOCK_VOWIFI_CAPABILITY));
    EXPECT_FALSE(m_pAosHandle->IsHoldingBlockForMobile(AosHandle::BLOCK_VIWIFI_CAPABILITY));

    EXPECT_FALSE(IsHoldingBlockForWifi(AosHandle::BLOCK_VOLTE_CAPABILITY));
    EXPECT_FALSE(IsHoldingBlockForWifi(AosHandle::BLOCK_VILTE_CAPABILITY));
    EXPECT_FALSE(IsHoldingBlockForWifi(AosHandle::BLOCK_VOPS));
    EXPECT_FALSE(IsHoldingBlockForWifi(AosHandle::BLOCK_SMS_CAPABILITY));
    EXPECT_FALSE(IsHoldingBlockForWifi(AosHandle::BLOCK_SMS_OVER_IP_NETWORK_INDICATION));
    EXPECT_FALSE(IsHoldingBlockForWifi(AosHandle::BLOCK_VOWIFI_CAPABILITY));
    EXPECT_FALSE(IsHoldingBlockForWifi(AosHandle::BLOCK_VIWIFI_CAPABILITY));

    EXPECT_EQ(m_pAosHandle->GetFeatureTagList().GetFeatures(), ImsAosFeature::NONE);
}

TEST_F(AosHandleTest, ProcessBlock_Test9)
{
    // Test9: Removing block / PreProcess / Wfc unavailable / Data connected
    // Expectation: All blocks are removed from both main and holding block.

    // Initialization
    SetHoldingBlocksPolicyForMobile();
    SetHoldingBlocksPolicyForWifi();

    AddHoldingBlockForMobile(AosHandle::BLOCK_VOLTE_CAPABILITY | AosHandle::BLOCK_VILTE_CAPABILITY |
            AosHandle::BLOCK_VOPS);
    AddHoldingBlockForWifi(AosHandle::BLOCK_VOWIFI_CAPABILITY | AosHandle::BLOCK_VIWIFI_CAPABILITY);

    m_pAosHandle->AddBlock(AosHandle::BLOCK_VOLTE_CAPABILITY | AosHandle::BLOCK_VILTE_CAPABILITY |
                    AosHandle::BLOCK_VOPS | AosHandle::BLOCK_SMS_CAPABILITY |
                    AosHandle::BLOCK_SMS_OVER_IP_NETWORK_INDICATION |
                    AosHandle::BLOCK_VOWIFI_CAPABILITY | AosHandle::BLOCK_VIWIFI_CAPABILITY,
            m_pAosHandle->m_nBlocks);

    SetDataConnected(IMS_TRUE);

    EXPECT_CALL(m_objMockIAosNConfiguration, IsWfcImsAvailable())
            .Times(AnyNumber())
            .WillRepeatedly(Return(IMS_FALSE));

    // Execution
    m_pAosHandle->ProcessBlock(AosHandle::BLOCK_VOLTE_CAPABILITY, IMS_FALSE, IMS_TRUE);
    m_pAosHandle->ProcessBlock(AosHandle::BLOCK_VILTE_CAPABILITY, IMS_FALSE, IMS_TRUE);
    m_pAosHandle->ProcessBlock(AosHandle::BLOCK_VOPS, IMS_FALSE, IMS_TRUE);
    m_pAosHandle->ProcessBlock(AosHandle::BLOCK_SMS_CAPABILITY, IMS_FALSE, IMS_TRUE);
    m_pAosHandle->ProcessBlock(
            AosHandle::BLOCK_SMS_OVER_IP_NETWORK_INDICATION, IMS_FALSE, IMS_TRUE);
    m_pAosHandle->ProcessBlock(AosHandle::BLOCK_VOWIFI_CAPABILITY, IMS_FALSE, IMS_TRUE);
    m_pAosHandle->ProcessBlock(AosHandle::BLOCK_VIWIFI_CAPABILITY, IMS_FALSE, IMS_TRUE);

    // Checking result
    EXPECT_FALSE(m_pAosHandle->IsHandleBlocked(AosHandle::BLOCK_VOLTE_CAPABILITY));
    EXPECT_FALSE(m_pAosHandle->IsHandleBlocked(AosHandle::BLOCK_VILTE_CAPABILITY));
    EXPECT_FALSE(m_pAosHandle->IsHandleBlocked(AosHandle::BLOCK_VOPS));
    EXPECT_FALSE(m_pAosHandle->IsHandleBlocked(AosHandle::BLOCK_SMS_CAPABILITY));
    EXPECT_FALSE(m_pAosHandle->IsHandleBlocked(AosHandle::BLOCK_SMS_OVER_IP_NETWORK_INDICATION));
    EXPECT_FALSE(m_pAosHandle->IsHandleBlocked(AosHandle::BLOCK_VOWIFI_CAPABILITY));
    EXPECT_FALSE(m_pAosHandle->IsHandleBlocked(AosHandle::BLOCK_VIWIFI_CAPABILITY));

    EXPECT_FALSE(m_pAosHandle->IsHoldingBlockForMobile(AosHandle::BLOCK_VOLTE_CAPABILITY));
    EXPECT_FALSE(m_pAosHandle->IsHoldingBlockForMobile(AosHandle::BLOCK_VILTE_CAPABILITY));
    EXPECT_FALSE(m_pAosHandle->IsHoldingBlockForMobile(AosHandle::BLOCK_VOPS));
    EXPECT_FALSE(m_pAosHandle->IsHoldingBlockForMobile(AosHandle::BLOCK_SMS_CAPABILITY));
    EXPECT_FALSE(
            m_pAosHandle->IsHoldingBlockForMobile(AosHandle::BLOCK_SMS_OVER_IP_NETWORK_INDICATION));
    EXPECT_FALSE(m_pAosHandle->IsHoldingBlockForMobile(AosHandle::BLOCK_VOWIFI_CAPABILITY));
    EXPECT_FALSE(m_pAosHandle->IsHoldingBlockForMobile(AosHandle::BLOCK_VIWIFI_CAPABILITY));

    EXPECT_FALSE(IsHoldingBlockForWifi(AosHandle::BLOCK_VOLTE_CAPABILITY));
    EXPECT_FALSE(IsHoldingBlockForWifi(AosHandle::BLOCK_VILTE_CAPABILITY));
    EXPECT_FALSE(IsHoldingBlockForWifi(AosHandle::BLOCK_VOPS));
    EXPECT_FALSE(IsHoldingBlockForWifi(AosHandle::BLOCK_SMS_CAPABILITY));
    EXPECT_FALSE(IsHoldingBlockForWifi(AosHandle::BLOCK_SMS_OVER_IP_NETWORK_INDICATION));
    EXPECT_FALSE(IsHoldingBlockForWifi(AosHandle::BLOCK_VOWIFI_CAPABILITY));
    EXPECT_FALSE(IsHoldingBlockForWifi(AosHandle::BLOCK_VIWIFI_CAPABILITY));

    EXPECT_EQ(m_pAosHandle->GetFeatureTagList().GetFeatures(),
            (ImsAosFeature::MMTEL | ImsAosFeature::VIDEO | ImsAosFeature::SMSIP));
}

TEST_F(AosHandleTest, ProcessBlock_Test10)
{
    // Test10: Adding block / PreProcess / Wfc available / Data connected / Epdg not enabled
    // Expectation: Mobile blocks are added to main blocks.
    //              Wifi blocks are added to holding blocks.

    // Initialization
    SetHoldingBlocksPolicyForMobile();
    SetHoldingBlocksPolicyForWifi();
    SetDataConnected(IMS_TRUE);
    SetEpdgEnabled(IMS_FALSE);

    AddFeature(ImsAosFeature::MMTEL);
    AddFeature(ImsAosFeature::VIDEO);
    AddFeature(ImsAosFeature::SMSIP);

    EXPECT_CALL(m_objMockIAosNConfiguration, IsWfcImsAvailable())
            .Times(AnyNumber())
            .WillRepeatedly(Return(IMS_TRUE));

    EXPECT_FALSE(m_pAosHandle->IsHandleBlocked());
    EXPECT_EQ(m_pAosHandle->GetFeatureTagList().GetFeatures(),
            (ImsAosFeature::MMTEL | ImsAosFeature::VIDEO | ImsAosFeature::SMSIP));

    // Execution
    m_pAosHandle->ProcessBlock(AosHandle::BLOCK_VOLTE_CAPABILITY, IMS_TRUE, IMS_TRUE);
    m_pAosHandle->ProcessBlock(AosHandle::BLOCK_VILTE_CAPABILITY, IMS_TRUE, IMS_TRUE);
    m_pAosHandle->ProcessBlock(AosHandle::BLOCK_VOPS, IMS_TRUE, IMS_TRUE);
    m_pAosHandle->ProcessBlock(AosHandle::BLOCK_SMS_CAPABILITY, IMS_TRUE, IMS_TRUE);
    m_pAosHandle->ProcessBlock(AosHandle::BLOCK_SMS_OVER_IP_NETWORK_INDICATION, IMS_TRUE, IMS_TRUE);
    m_pAosHandle->ProcessBlock(AosHandle::BLOCK_VOWIFI_CAPABILITY, IMS_TRUE, IMS_TRUE);
    m_pAosHandle->ProcessBlock(AosHandle::BLOCK_VIWIFI_CAPABILITY, IMS_TRUE, IMS_TRUE);

    // Checking result
    EXPECT_TRUE(m_pAosHandle->IsHandleBlocked(AosHandle::BLOCK_VOLTE_CAPABILITY));
    EXPECT_TRUE(m_pAosHandle->IsHandleBlocked(AosHandle::BLOCK_VILTE_CAPABILITY));
    EXPECT_TRUE(m_pAosHandle->IsHandleBlocked(AosHandle::BLOCK_VOPS));
    EXPECT_TRUE(m_pAosHandle->IsHandleBlocked(AosHandle::BLOCK_SMS_CAPABILITY));
    EXPECT_TRUE(m_pAosHandle->IsHandleBlocked(AosHandle::BLOCK_SMS_OVER_IP_NETWORK_INDICATION));
    EXPECT_FALSE(m_pAosHandle->IsHandleBlocked(AosHandle::BLOCK_VOWIFI_CAPABILITY));
    EXPECT_FALSE(m_pAosHandle->IsHandleBlocked(AosHandle::BLOCK_VIWIFI_CAPABILITY));

    EXPECT_FALSE(m_pAosHandle->IsHoldingBlockForMobile(AosHandle::BLOCK_VOLTE_CAPABILITY));
    EXPECT_FALSE(m_pAosHandle->IsHoldingBlockForMobile(AosHandle::BLOCK_VILTE_CAPABILITY));
    EXPECT_FALSE(m_pAosHandle->IsHoldingBlockForMobile(AosHandle::BLOCK_VOPS));
    EXPECT_FALSE(m_pAosHandle->IsHoldingBlockForMobile(AosHandle::BLOCK_SMS_CAPABILITY));
    EXPECT_FALSE(
            m_pAosHandle->IsHoldingBlockForMobile(AosHandle::BLOCK_SMS_OVER_IP_NETWORK_INDICATION));
    EXPECT_FALSE(m_pAosHandle->IsHoldingBlockForMobile(AosHandle::BLOCK_VOWIFI_CAPABILITY));
    EXPECT_FALSE(m_pAosHandle->IsHoldingBlockForMobile(AosHandle::BLOCK_VIWIFI_CAPABILITY));

    EXPECT_FALSE(IsHoldingBlockForWifi(AosHandle::BLOCK_VOLTE_CAPABILITY));
    EXPECT_FALSE(IsHoldingBlockForWifi(AosHandle::BLOCK_VILTE_CAPABILITY));
    EXPECT_FALSE(IsHoldingBlockForWifi(AosHandle::BLOCK_VOPS));
    EXPECT_FALSE(IsHoldingBlockForWifi(AosHandle::BLOCK_SMS_CAPABILITY));
    EXPECT_FALSE(IsHoldingBlockForWifi(AosHandle::BLOCK_SMS_OVER_IP_NETWORK_INDICATION));
    EXPECT_TRUE(IsHoldingBlockForWifi(AosHandle::BLOCK_VOWIFI_CAPABILITY));
    EXPECT_TRUE(IsHoldingBlockForWifi(AosHandle::BLOCK_VIWIFI_CAPABILITY));

    EXPECT_EQ(m_pAosHandle->GetFeatureTagList().GetFeatures(), ImsAosFeature::NONE);
}

TEST_F(AosHandleTest, ProcessBlock_Test11)
{
    // Test11: Removing block / PreProcess / Wfc available / Data connected / Epdg not enabled
    // Expectation: All blocks are removed from both main and holding block.

    // Initialization
    SetHoldingBlocksPolicyForMobile();
    SetHoldingBlocksPolicyForWifi();

    AddHoldingBlockForMobile(AosHandle::BLOCK_VOLTE_CAPABILITY | AosHandle::BLOCK_VILTE_CAPABILITY |
            AosHandle::BLOCK_VOPS);
    AddHoldingBlockForWifi(AosHandle::BLOCK_VOWIFI_CAPABILITY | AosHandle::BLOCK_VIWIFI_CAPABILITY);

    m_pAosHandle->AddBlock(AosHandle::BLOCK_VOLTE_CAPABILITY | AosHandle::BLOCK_VILTE_CAPABILITY |
                    AosHandle::BLOCK_VOPS | AosHandle::BLOCK_SMS_CAPABILITY |
                    AosHandle::BLOCK_SMS_OVER_IP_NETWORK_INDICATION |
                    AosHandle::BLOCK_VOWIFI_CAPABILITY | AosHandle::BLOCK_VIWIFI_CAPABILITY,
            m_pAosHandle->m_nBlocks);

    SetDataConnected(IMS_TRUE);
    SetEpdgEnabled(IMS_FALSE);

    EXPECT_CALL(m_objMockIAosNConfiguration, IsWfcImsAvailable())
            .Times(AnyNumber())
            .WillRepeatedly(Return(IMS_TRUE));

    // Execution
    m_pAosHandle->ProcessBlock(AosHandle::BLOCK_VOLTE_CAPABILITY, IMS_FALSE, IMS_TRUE);
    m_pAosHandle->ProcessBlock(AosHandle::BLOCK_VILTE_CAPABILITY, IMS_FALSE, IMS_TRUE);
    m_pAosHandle->ProcessBlock(AosHandle::BLOCK_VOPS, IMS_FALSE, IMS_TRUE);
    m_pAosHandle->ProcessBlock(AosHandle::BLOCK_SMS_CAPABILITY, IMS_FALSE, IMS_TRUE);
    m_pAosHandle->ProcessBlock(
            AosHandle::BLOCK_SMS_OVER_IP_NETWORK_INDICATION, IMS_FALSE, IMS_TRUE);
    m_pAosHandle->ProcessBlock(AosHandle::BLOCK_VOWIFI_CAPABILITY, IMS_FALSE, IMS_TRUE);
    m_pAosHandle->ProcessBlock(AosHandle::BLOCK_VIWIFI_CAPABILITY, IMS_FALSE, IMS_TRUE);

    // Checking result
    EXPECT_FALSE(m_pAosHandle->IsHandleBlocked(AosHandle::BLOCK_VOLTE_CAPABILITY));
    EXPECT_FALSE(m_pAosHandle->IsHandleBlocked(AosHandle::BLOCK_VILTE_CAPABILITY));
    EXPECT_FALSE(m_pAosHandle->IsHandleBlocked(AosHandle::BLOCK_VOPS));
    EXPECT_FALSE(m_pAosHandle->IsHandleBlocked(AosHandle::BLOCK_SMS_CAPABILITY));
    EXPECT_FALSE(m_pAosHandle->IsHandleBlocked(AosHandle::BLOCK_SMS_OVER_IP_NETWORK_INDICATION));
    EXPECT_FALSE(m_pAosHandle->IsHandleBlocked(AosHandle::BLOCK_VOWIFI_CAPABILITY));
    EXPECT_FALSE(m_pAosHandle->IsHandleBlocked(AosHandle::BLOCK_VIWIFI_CAPABILITY));

    EXPECT_FALSE(m_pAosHandle->IsHoldingBlockForMobile(AosHandle::BLOCK_VOLTE_CAPABILITY));
    EXPECT_FALSE(m_pAosHandle->IsHoldingBlockForMobile(AosHandle::BLOCK_VILTE_CAPABILITY));
    EXPECT_FALSE(m_pAosHandle->IsHoldingBlockForMobile(AosHandle::BLOCK_VOPS));
    EXPECT_FALSE(m_pAosHandle->IsHoldingBlockForMobile(AosHandle::BLOCK_SMS_CAPABILITY));
    EXPECT_FALSE(
            m_pAosHandle->IsHoldingBlockForMobile(AosHandle::BLOCK_SMS_OVER_IP_NETWORK_INDICATION));
    EXPECT_FALSE(m_pAosHandle->IsHoldingBlockForMobile(AosHandle::BLOCK_VOWIFI_CAPABILITY));
    EXPECT_FALSE(m_pAosHandle->IsHoldingBlockForMobile(AosHandle::BLOCK_VIWIFI_CAPABILITY));

    EXPECT_FALSE(IsHoldingBlockForWifi(AosHandle::BLOCK_VOLTE_CAPABILITY));
    EXPECT_FALSE(IsHoldingBlockForWifi(AosHandle::BLOCK_VILTE_CAPABILITY));
    EXPECT_FALSE(IsHoldingBlockForWifi(AosHandle::BLOCK_VOPS));
    EXPECT_FALSE(IsHoldingBlockForWifi(AosHandle::BLOCK_SMS_CAPABILITY));
    EXPECT_FALSE(IsHoldingBlockForWifi(AosHandle::BLOCK_SMS_OVER_IP_NETWORK_INDICATION));
    EXPECT_FALSE(IsHoldingBlockForWifi(AosHandle::BLOCK_VOWIFI_CAPABILITY));
    EXPECT_FALSE(IsHoldingBlockForWifi(AosHandle::BLOCK_VIWIFI_CAPABILITY));

    EXPECT_EQ(m_pAosHandle->GetFeatureTagList().GetFeatures(),
            (ImsAosFeature::MMTEL | ImsAosFeature::VIDEO | ImsAosFeature::SMSIP));
}

TEST_F(AosHandleTest, ProcessBlock_Test12)
{
    // Test12: Adding block / PreProcess / Wfc available / Data connected / Epdg enabled
    // Expectation: Mobile blocks are added to holding blocks.
    //              Wifi blocks are added to main blocks.

    // Initialization
    SetHoldingBlocksPolicyForMobile();
    SetHoldingBlocksPolicyForWifi();
    SetDataConnected(IMS_TRUE);
    SetEpdgEnabled(IMS_TRUE);

    AddFeature(ImsAosFeature::MMTEL);
    AddFeature(ImsAosFeature::VIDEO);
    AddFeature(ImsAosFeature::SMSIP);

    EXPECT_CALL(m_objMockIAosNConfiguration, IsWfcImsAvailable())
            .Times(AnyNumber())
            .WillRepeatedly(Return(IMS_TRUE));

    EXPECT_FALSE(m_pAosHandle->IsHandleBlocked());
    EXPECT_EQ(m_pAosHandle->GetFeatureTagList().GetFeatures(),
            (ImsAosFeature::MMTEL | ImsAosFeature::VIDEO | ImsAosFeature::SMSIP));

    // Execution
    m_pAosHandle->ProcessBlock(AosHandle::BLOCK_VOLTE_CAPABILITY, IMS_TRUE, IMS_TRUE);
    m_pAosHandle->ProcessBlock(AosHandle::BLOCK_VILTE_CAPABILITY, IMS_TRUE, IMS_TRUE);
    m_pAosHandle->ProcessBlock(AosHandle::BLOCK_VOPS, IMS_TRUE, IMS_TRUE);
    m_pAosHandle->ProcessBlock(AosHandle::BLOCK_SMS_CAPABILITY, IMS_TRUE, IMS_TRUE);
    m_pAosHandle->ProcessBlock(AosHandle::BLOCK_SMS_OVER_IP_NETWORK_INDICATION, IMS_TRUE, IMS_TRUE);
    m_pAosHandle->ProcessBlock(AosHandle::BLOCK_VOWIFI_CAPABILITY, IMS_TRUE, IMS_TRUE);
    m_pAosHandle->ProcessBlock(AosHandle::BLOCK_VIWIFI_CAPABILITY, IMS_TRUE, IMS_TRUE);

    // Checking result
    EXPECT_FALSE(m_pAosHandle->IsHandleBlocked(AosHandle::BLOCK_VOLTE_CAPABILITY));
    EXPECT_FALSE(m_pAosHandle->IsHandleBlocked(AosHandle::BLOCK_VILTE_CAPABILITY));
    EXPECT_FALSE(m_pAosHandle->IsHandleBlocked(AosHandle::BLOCK_VOPS));
    EXPECT_TRUE(m_pAosHandle->IsHandleBlocked(AosHandle::BLOCK_SMS_CAPABILITY));
    EXPECT_TRUE(m_pAosHandle->IsHandleBlocked(AosHandle::BLOCK_SMS_OVER_IP_NETWORK_INDICATION));
    EXPECT_TRUE(m_pAosHandle->IsHandleBlocked(AosHandle::BLOCK_VOWIFI_CAPABILITY));
    EXPECT_TRUE(m_pAosHandle->IsHandleBlocked(AosHandle::BLOCK_VIWIFI_CAPABILITY));

    EXPECT_TRUE(m_pAosHandle->IsHoldingBlockForMobile(AosHandle::BLOCK_VOLTE_CAPABILITY));
    EXPECT_TRUE(m_pAosHandle->IsHoldingBlockForMobile(AosHandle::BLOCK_VILTE_CAPABILITY));
    EXPECT_TRUE(m_pAosHandle->IsHoldingBlockForMobile(AosHandle::BLOCK_VOPS));
    EXPECT_FALSE(m_pAosHandle->IsHoldingBlockForMobile(AosHandle::BLOCK_SMS_CAPABILITY));
    EXPECT_FALSE(
            m_pAosHandle->IsHoldingBlockForMobile(AosHandle::BLOCK_SMS_OVER_IP_NETWORK_INDICATION));
    EXPECT_FALSE(m_pAosHandle->IsHoldingBlockForMobile(AosHandle::BLOCK_VOWIFI_CAPABILITY));
    EXPECT_FALSE(m_pAosHandle->IsHoldingBlockForMobile(AosHandle::BLOCK_VIWIFI_CAPABILITY));

    EXPECT_FALSE(IsHoldingBlockForWifi(AosHandle::BLOCK_VOLTE_CAPABILITY));
    EXPECT_FALSE(IsHoldingBlockForWifi(AosHandle::BLOCK_VILTE_CAPABILITY));
    EXPECT_FALSE(IsHoldingBlockForWifi(AosHandle::BLOCK_VOPS));
    EXPECT_FALSE(IsHoldingBlockForWifi(AosHandle::BLOCK_SMS_CAPABILITY));
    EXPECT_FALSE(IsHoldingBlockForWifi(AosHandle::BLOCK_SMS_OVER_IP_NETWORK_INDICATION));
    EXPECT_FALSE(IsHoldingBlockForWifi(AosHandle::BLOCK_VOWIFI_CAPABILITY));
    EXPECT_FALSE(IsHoldingBlockForWifi(AosHandle::BLOCK_VIWIFI_CAPABILITY));

    EXPECT_EQ(m_pAosHandle->GetFeatureTagList().GetFeatures(), ImsAosFeature::NONE);
}

TEST_F(AosHandleTest, ProcessBlock_Test13)
{
    // Test13: Removing block / PreProcess / Wfc available / Data connected / Epdg enabled
    // Expectation: All blocks are removed from both main and holding block.

    // Initialization
    SetHoldingBlocksPolicyForMobile();
    SetHoldingBlocksPolicyForWifi();

    AddHoldingBlockForMobile(AosHandle::BLOCK_VOLTE_CAPABILITY | AosHandle::BLOCK_VILTE_CAPABILITY |
            AosHandle::BLOCK_VOPS);
    AddHoldingBlockForWifi(AosHandle::BLOCK_VOWIFI_CAPABILITY | AosHandle::BLOCK_VIWIFI_CAPABILITY);

    m_pAosHandle->AddBlock(AosHandle::BLOCK_VOLTE_CAPABILITY | AosHandle::BLOCK_VILTE_CAPABILITY |
                    AosHandle::BLOCK_VOPS | AosHandle::BLOCK_SMS_CAPABILITY |
                    AosHandle::BLOCK_SMS_OVER_IP_NETWORK_INDICATION |
                    AosHandle::BLOCK_VOWIFI_CAPABILITY | AosHandle::BLOCK_VIWIFI_CAPABILITY,
            m_pAosHandle->m_nBlocks);

    SetDataConnected(IMS_TRUE);
    SetEpdgEnabled(IMS_TRUE);

    EXPECT_CALL(m_objMockIAosNConfiguration, IsWfcImsAvailable())
            .Times(AnyNumber())
            .WillRepeatedly(Return(IMS_TRUE));

    // Execution
    m_pAosHandle->ProcessBlock(AosHandle::BLOCK_VOLTE_CAPABILITY, IMS_FALSE, IMS_TRUE);
    m_pAosHandle->ProcessBlock(AosHandle::BLOCK_VILTE_CAPABILITY, IMS_FALSE, IMS_TRUE);
    m_pAosHandle->ProcessBlock(AosHandle::BLOCK_VOPS, IMS_FALSE, IMS_TRUE);
    m_pAosHandle->ProcessBlock(AosHandle::BLOCK_SMS_CAPABILITY, IMS_FALSE, IMS_TRUE);
    m_pAosHandle->ProcessBlock(
            AosHandle::BLOCK_SMS_OVER_IP_NETWORK_INDICATION, IMS_FALSE, IMS_TRUE);
    m_pAosHandle->ProcessBlock(AosHandle::BLOCK_VOWIFI_CAPABILITY, IMS_FALSE, IMS_TRUE);
    m_pAosHandle->ProcessBlock(AosHandle::BLOCK_VIWIFI_CAPABILITY, IMS_FALSE, IMS_TRUE);

    // Checking result
    EXPECT_FALSE(m_pAosHandle->IsHandleBlocked(AosHandle::BLOCK_VOLTE_CAPABILITY));
    EXPECT_FALSE(m_pAosHandle->IsHandleBlocked(AosHandle::BLOCK_VILTE_CAPABILITY));
    EXPECT_FALSE(m_pAosHandle->IsHandleBlocked(AosHandle::BLOCK_VOPS));
    EXPECT_FALSE(m_pAosHandle->IsHandleBlocked(AosHandle::BLOCK_SMS_CAPABILITY));
    EXPECT_FALSE(m_pAosHandle->IsHandleBlocked(AosHandle::BLOCK_SMS_OVER_IP_NETWORK_INDICATION));
    EXPECT_FALSE(m_pAosHandle->IsHandleBlocked(AosHandle::BLOCK_VOWIFI_CAPABILITY));
    EXPECT_FALSE(m_pAosHandle->IsHandleBlocked(AosHandle::BLOCK_VIWIFI_CAPABILITY));

    EXPECT_FALSE(m_pAosHandle->IsHoldingBlockForMobile(AosHandle::BLOCK_VOLTE_CAPABILITY));
    EXPECT_FALSE(m_pAosHandle->IsHoldingBlockForMobile(AosHandle::BLOCK_VILTE_CAPABILITY));
    EXPECT_FALSE(m_pAosHandle->IsHoldingBlockForMobile(AosHandle::BLOCK_VOPS));
    EXPECT_FALSE(m_pAosHandle->IsHoldingBlockForMobile(AosHandle::BLOCK_SMS_CAPABILITY));
    EXPECT_FALSE(
            m_pAosHandle->IsHoldingBlockForMobile(AosHandle::BLOCK_SMS_OVER_IP_NETWORK_INDICATION));
    EXPECT_FALSE(m_pAosHandle->IsHoldingBlockForMobile(AosHandle::BLOCK_VOWIFI_CAPABILITY));
    EXPECT_FALSE(m_pAosHandle->IsHoldingBlockForMobile(AosHandle::BLOCK_VIWIFI_CAPABILITY));

    EXPECT_FALSE(IsHoldingBlockForWifi(AosHandle::BLOCK_VOLTE_CAPABILITY));
    EXPECT_FALSE(IsHoldingBlockForWifi(AosHandle::BLOCK_VILTE_CAPABILITY));
    EXPECT_FALSE(IsHoldingBlockForWifi(AosHandle::BLOCK_VOPS));
    EXPECT_FALSE(IsHoldingBlockForWifi(AosHandle::BLOCK_SMS_CAPABILITY));
    EXPECT_FALSE(IsHoldingBlockForWifi(AosHandle::BLOCK_SMS_OVER_IP_NETWORK_INDICATION));
    EXPECT_FALSE(IsHoldingBlockForWifi(AosHandle::BLOCK_VOWIFI_CAPABILITY));
    EXPECT_FALSE(IsHoldingBlockForWifi(AosHandle::BLOCK_VIWIFI_CAPABILITY));

    EXPECT_EQ(m_pAosHandle->GetFeatureTagList().GetFeatures(),
            (ImsAosFeature::MMTEL | ImsAosFeature::VIDEO | ImsAosFeature::SMSIP));
}

TEST_F(AosHandleTest, ProcessFeatureBlock_Test)
{
    // Expectation: Add/Remove the feature

    ClearFeatureTagList();

    EXPECT_EQ(m_pAosHandle->GetFeatureTagList().GetFeatures(), ImsAosFeature::NONE);

    m_pAosHandle->ProcessFeatureBlock(ImsAosFeature::MMTEL, IMS_FALSE);
    EXPECT_EQ(m_pAosHandle->GetFeatureTagList().GetFeatures(), ImsAosFeature::MMTEL);

    m_pAosHandle->ProcessFeatureBlock(ImsAosFeature::MMTEL, IMS_TRUE);
    EXPECT_EQ(m_pAosHandle->GetFeatureTagList().GetFeatures(), ImsAosFeature::NONE);

    ClearFeatureTagList();
}

TEST_F(AosHandleTest, ProcessCheckBlock_Test1)
{
    // Test1: Block not changed, blocked.
    // Expectation: Do nothing. return false

    m_pAosHandle->AddBlock(AosHandle::BLOCK_VOPS, m_pAosHandle->m_nBlocks);
    SetBlocked(IMS_TRUE);

    EXPECT_FALSE(m_pAosHandle->ProcessCheckBlock(IMS_TRUE));
}

TEST_F(AosHandleTest, ProcessCheckBlock_Test2)
{
    // Test2: Block not changed, not blocked. state connected.
    // Expectation: Call ProcessFeatureChange, return true

    ClearBlocks();
    ClearFeatureTagList();
    SetBlocked(IMS_FALSE);
    m_pAosHandle->SetHandleState(AosHandle::STATE_CONNECTED);
    AddFeature(ImsAosFeature::MMTEL);

    EXPECT_CALL(m_objMockIAosApplication, Reconfig()).Times(1);
    EXPECT_TRUE(m_pAosHandle->ProcessCheckBlock(IMS_TRUE));

    ClearFeatureTagList();
}

TEST_F(AosHandleTest, ProcessCheckBlock_Test3)
{
    // Test3: Block changed to blocked. state connected.
    // Expectation: Call StateConnected via state machine, return true

    m_pAosHandle->AddBlock(AosHandle::BLOCK_VOPS, m_pAosHandle->m_nBlocks);
    SetBlocked(IMS_FALSE);
    m_pAosHandle->SetHandleState(AosHandle::STATE_CONNECTED);

    EXPECT_CALL(m_objMockIAosApplication, Reconfig()).Times(1);
    EXPECT_TRUE(m_pAosHandle->ProcessCheckBlock(IMS_TRUE));
    EXPECT_EQ(m_pAosHandle->GetState(), AosHandle::STATE_DISCONNECTING);

    ClearBlocks();
    m_pAosHandle->SetHandleState(AosHandle::STATE_DISCONNECTED);
}

TEST_F(AosHandleTest, ProcessCheckBlock_Test4)
{
    // Test4: Block changed to not blocked. state disconnected.
    // Expectation: Call StateDisconnected via state machine, return true

    ClearBlocks();
    SetBlocked(IMS_TRUE);
    m_pAosHandle->SetHandleState(AosHandle::STATE_DISCONNECTED);

    EXPECT_CALL(m_objMockIAosApplication, Reconfig()).Times(1);
    EXPECT_TRUE(m_pAosHandle->ProcessCheckBlock(IMS_TRUE));
    EXPECT_EQ(m_pAosHandle->GetState(), AosHandle::STATE_CONNECTING);

    m_pAosHandle->SetHandleState(AosHandle::STATE_DISCONNECTED);
}

TEST_F(AosHandleTest, ProcessUnavailableFeature_Test)
{
    // Expectation: Add/Remove unavailable feature

    ClearFeatureTagList();

    EXPECT_EQ(GetUnavailableFeatures(), ImsAosFeature::NONE);

    m_pAosHandle->ProcessUnavailableFeature(ImsAosFeature::MMTEL, IMS_TRUE);
    EXPECT_EQ(GetUnavailableFeatures(), ImsAosFeature::MMTEL);
    EXPECT_EQ(GetBindedUnavailableFeatures(), ImsAosFeature::MMTEL);

    m_pAosHandle->ProcessUnavailableFeature(ImsAosFeature::MMTEL, IMS_FALSE);
    EXPECT_EQ(GetUnavailableFeatures(), ImsAosFeature::NONE);
    EXPECT_EQ(GetBindedUnavailableFeatures(), ImsAosFeature::NONE);
}

TEST_F(AosHandleTest, ProcessUnavailableFeatureChanged_Test1)
{
    // Test1: state not connected
    // Expectation: Call AosRegistration::RequestCmd() with CMD_UNAVAILABLE_FEATURE_TAG param.

    m_pAosHandle->SetHandleState(AosHandle::STATE_DISCONNECTED);

    MockIAosRegistration objMockIAosRegistration;
    EXPECT_CALL(m_objMockIAosAppContext, GetRegistration())
            .Times(AnyNumber())
            .WillRepeatedly(Return(static_cast<IAosRegistration*>(&objMockIAosRegistration)));

    EXPECT_CALL(
            objMockIAosRegistration, RequestCmd(IAosRegistration::CMD_UNAVAILABLE_FEATURE_TAG, 0))
            .Times(1);
    EXPECT_CALL(m_objMockIImsAosListener, ImsAos_Connected(_, _)).Times(0);

    m_pAosHandle->ProcessUnavailableFeatureChanged();
}

TEST_F(AosHandleTest, ProcessUnavailableFeatureChanged_Test2)
{
    // Test2: state connected, listener is null
    // Expectation: Call AosRegistration::RequestCmd() with CMD_UNAVAILABLE_FEATURE_TAG param.

    m_pAosHandle->SetHandleState(AosHandle::STATE_CONNECTED);
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

    m_pAosHandle->ProcessUnavailableFeatureChanged();

    m_pAosHandle->SetHandleState(AosHandle::STATE_DISCONNECTED);
}

TEST_F(AosHandleTest, ProcessUnavailableFeatureChanged_Test3)
{
    // Test 3: state connected, listener is not null
    // Expectation: Call AosRegistration::RequestCmd() with CMD_UNAVAILABLE_FEATURE_TAG param.
    // + Call ImsAos_Connected of the listener

    m_pAosHandle->SetHandleState(AosHandle::STATE_CONNECTED);
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

    m_pAosHandle->ProcessUnavailableFeatureChanged();

    m_pAosHandle->SetListener(IMS_NULL);
    m_pAosHandle->SetHandleState(AosHandle::STATE_DISCONNECTED);
}

TEST_F(AosHandleTest, IsBlockForMobile_Test)
{
    // Expectation: return false if the block is in wifi block policy, else true

    ClearHoldingBlocksPolicyForWifi();
    SetHoldingBlocksPolicyForWifi();

    EXPECT_TRUE(m_pAosHandle->IsBlockForMobile(AosHandle::BLOCK_VOLTE_CAPABILITY));
    EXPECT_TRUE(m_pAosHandle->IsBlockForMobile(AosHandle::BLOCK_VILTE_CAPABILITY));
    EXPECT_TRUE(m_pAosHandle->IsBlockForMobile(AosHandle::BLOCK_VOPS));

    EXPECT_FALSE(m_pAosHandle->IsBlockForMobile(AosHandle::BLOCK_VOWIFI_CAPABILITY));
    EXPECT_FALSE(m_pAosHandle->IsBlockForMobile(AosHandle::BLOCK_VIWIFI_CAPABILITY));

    EXPECT_TRUE(m_pAosHandle->IsBlockForMobile(AosHandle::BLOCK_SMS_CAPABILITY));
    EXPECT_TRUE(m_pAosHandle->IsBlockForMobile(AosHandle::BLOCK_SMS_OVER_IP_NETWORK_INDICATION));

    ClearHoldingBlocksPolicyForWifi();
}

TEST_F(AosHandleTest, IsBlockForWifi_Test)
{
    // Expectation: return false if the block is in mobile block policy, else true

    ClearHoldingBlocksPolicyForMobile();
    SetHoldingBlocksPolicyForMobile();

    EXPECT_FALSE(m_pAosHandle->IsBlockForWifi(AosHandle::BLOCK_VOLTE_CAPABILITY));
    EXPECT_FALSE(m_pAosHandle->IsBlockForWifi(AosHandle::BLOCK_VILTE_CAPABILITY));
    EXPECT_FALSE(m_pAosHandle->IsBlockForWifi(AosHandle::BLOCK_VOPS));

    EXPECT_TRUE(m_pAosHandle->IsBlockForWifi(AosHandle::BLOCK_VOWIFI_CAPABILITY));
    EXPECT_TRUE(m_pAosHandle->IsBlockForWifi(AosHandle::BLOCK_VIWIFI_CAPABILITY));

    EXPECT_TRUE(m_pAosHandle->IsBlockForWifi(AosHandle::BLOCK_SMS_CAPABILITY));
    EXPECT_TRUE(m_pAosHandle->IsBlockForWifi(AosHandle::BLOCK_SMS_OVER_IP_NETWORK_INDICATION));

    ClearHoldingBlocksPolicyForMobile();
}

TEST_F(AosHandleTest, StateDisconnected_Test1)
{
    // Test1: HANDLE_MSG_BLOCK_STATUS, Handle not blocked
    // Expectation: state-connecting, request type-attach, call reconfig(), need to notify.

    m_pAosHandle->SetHandleState(AosHandle::STATE_DISCONNECTED);
    SetBlocked(IMS_FALSE);
    SetNotify(IMS_FALSE);

    EXPECT_CALL(m_objMockIAosApplication, Reconfig()).Times(1);

    IMSMSG objMSG(0 /*AosHandle::HANDLE_MSG_BLOCK_STATUS*/, 0, 0);
    m_pAosHandle->OnStateMessage(objMSG);

    EXPECT_EQ(m_pAosHandle->GetState(), AosHandle::STATE_CONNECTING);
    EXPECT_EQ(m_pAosHandle->GetRequestType(), IAosHandle::ATTACH);
    EXPECT_TRUE(GetNotify());
}

TEST_F(AosHandleTest, StateDisconnected_Test2)
{
    // Test2: HANDLE_MSG_BLOCK_STATUS, Handle blocked
    // Expectation: state-disconnected, request type-detach, no call reconfig(), no need to notify.

    m_pAosHandle->SetHandleState(AosHandle::STATE_DISCONNECTED);
    SetBlocked(IMS_TRUE);
    SetNotify(IMS_FALSE);

    EXPECT_CALL(m_objMockIAosApplication, Reconfig()).Times(0);

    IMSMSG objMSG(0 /*AosHandle::HANDLE_MSG_BLOCK_STATUS*/, 0, 0);
    m_pAosHandle->OnStateMessage(objMSG);

    EXPECT_EQ(m_pAosHandle->GetState(), AosHandle::STATE_DISCONNECTED);
    EXPECT_EQ(m_pAosHandle->GetRequestType(), IAosHandle::DETACH);
    EXPECT_FALSE(GetNotify());
}

TEST_F(AosHandleTest, StateDisconnected_Test3)
{
    // Test3: HANDLE_MSG_APP_STATUS, Handle blocked
    // Expectation: state-disconnected, request type-detach, no call reconfig(), no need to notify.

    m_pAosHandle->SetHandleState(AosHandle::STATE_DISCONNECTED);
    SetBlocked(IMS_TRUE);
    SetNotify(IMS_FALSE);

    EXPECT_CALL(m_objMockIAosApplication, Reconfig()).Times(0);

    IMSMSG objMSG(1 /*AosHandle::HANDLE_MSG_APP_STATUS*/, 0, 0);
    m_pAosHandle->OnStateMessage(objMSG);

    EXPECT_EQ(m_pAosHandle->GetState(), AosHandle::STATE_DISCONNECTED);
    EXPECT_EQ(m_pAosHandle->GetRequestType(), IAosHandle::DETACH);
    EXPECT_FALSE(GetNotify());
}

TEST_F(AosHandleTest, StateDisconnected_Test4)
{
    // Test4: Invalid msg
    // Expectation: state-disconnected, no need to notify.

    m_pAosHandle->SetHandleState(AosHandle::STATE_DISCONNECTED);
    SetNotify(IMS_FALSE);

    IMSMSG objMSG(2 /*AosHandle::HANDLE_MSG_INVALID*/, 0, 0);
    m_pAosHandle->OnStateMessage(objMSG);

    EXPECT_EQ(m_pAosHandle->GetState(), AosHandle::STATE_DISCONNECTED);
    EXPECT_FALSE(GetNotify());
}

TEST_F(AosHandleTest, StateConnecting_Test1)
{
    // Test1: HANDLE_MSG_BLOCK_STATUS, Handle blocked
    // Expectation: state-disconnected, request type-detach, call reconfig(), need to notify.

    m_pAosHandle->SetHandleState(AosHandle::STATE_CONNECTING);
    SetBlocked(IMS_TRUE);
    SetNotify(IMS_FALSE);

    EXPECT_CALL(m_objMockIAosApplication, Reconfig()).Times(1);

    IMSMSG objMSG(0 /*HANDLE_MSG_BLOCK_STATUS*/, 0, 0);
    m_pAosHandle->OnStateMessage(objMSG);

    EXPECT_EQ(m_pAosHandle->GetState(), AosHandle::STATE_DISCONNECTED);
    EXPECT_EQ(m_pAosHandle->GetRequestType(), IAosHandle::DETACH);
    EXPECT_TRUE(GetNotify());
}

TEST_F(AosHandleTest, StateConnecting_Test2)
{
    // Test2: HANDLE_MSG_BLOCK_STATUS, Handle not blocked, feature tag not changed
    // Expectation: state-connecting, request type-attach, no call reconfig(), no need to notify.

    m_pAosHandle->SetHandleState(AosHandle::STATE_CONNECTING);
    m_pAosHandle->SetRequestType(IAosHandle::ATTACH);
    SetBlocked(IMS_FALSE);
    SetNotify(IMS_FALSE);

    EXPECT_CALL(m_objMockIAosApplication, Reconfig()).Times(0);

    IMSMSG objMSG(0 /*HANDLE_MSG_BLOCK_STATUS*/, 0, 0);
    m_pAosHandle->OnStateMessage(objMSG);

    EXPECT_EQ(m_pAosHandle->GetState(), AosHandle::STATE_CONNECTING);
    EXPECT_EQ(m_pAosHandle->GetRequestType(), IAosHandle::ATTACH);
    EXPECT_FALSE(GetNotify());
}

TEST_F(AosHandleTest, StateConnecting_Test3)
{
    // Test3: HANDLE_MSG_BLOCK_STATUS, Handle not blocked, feature tag changed
    // Expectation: state-connecting, request type-attach, call reconfig(), no need to notify.

    m_pAosHandle->SetHandleState(AosHandle::STATE_CONNECTING);
    m_pAosHandle->SetRequestType(IAosHandle::ATTACH);
    SetBlocked(IMS_FALSE);
    SetNotify(IMS_FALSE);

    AosFeatureTagList objTestFeatureTagList, objTestBindedFeatureTagList;
    objTestFeatureTagList.AddFeature(
            ImsAosFeature::MMTEL | ImsAosFeature::VIDEO | ImsAosFeature::TEXT);
    objTestBindedFeatureTagList.AddFeature(ImsAosFeature::MMTEL | ImsAosFeature::VIDEO);

    SetFeatureTagList(objTestFeatureTagList);
    SetBindedFeatureTagList(objTestBindedFeatureTagList);

    EXPECT_CALL(m_objMockIAosApplication, Reconfig()).Times(1);

    IMSMSG objMSG(0 /*HANDLE_MSG_BLOCK_STATUS*/, 0, 0);
    m_pAosHandle->OnStateMessage(objMSG);

    EXPECT_EQ(m_pAosHandle->GetState(), AosHandle::STATE_CONNECTING);
    EXPECT_EQ(m_pAosHandle->GetRequestType(), IAosHandle::ATTACH);
    EXPECT_FALSE(GetNotify());
}

TEST_F(AosHandleTest, StateConnecting_Test4)
{
    // Test4: HANDLE_MSG_APP_STATUS, APP_CONNECTED, Reg binded
    // Expectation: clear suspended reason, state-connected, need to notify

    m_pAosHandle->SetHandleState(AosHandle::STATE_CONNECTING);
    m_pAosHandle->SetRegBinded(IMS_TRUE);
    SetNotify(IMS_FALSE);
    m_pAosHandle->SetSuspendedReason(AosReason::SUSPEND_NO_SERVICE);
    EXPECT_EQ(m_pAosHandle->GetSuspendedReason(), AosReason::SUSPEND_NO_SERVICE);

    EXPECT_CALL(m_objMockIAosNetTracker, IsSuspended()).Times(1).WillOnce(Return(IMS_FALSE));

    m_pAosHandle->App_StateChanged(IAosApplication::APP_CONNECTED, 0);

    EXPECT_EQ(m_pAosHandle->GetSuspendedReason(), AosReason::SUSPEND_NONE);
    EXPECT_EQ(m_pAosHandle->GetState(), AosHandle::STATE_CONNECTED);
    EXPECT_TRUE(GetNotify());
}

TEST_F(AosHandleTest, StateConnecting_Test5)
{
    // Test5: HANDLE_MSG_APP_STATUS, APP_DISCONNECTED
    // Expectation: need to notify

    m_pAosHandle->SetHandleState(AosHandle::STATE_CONNECTING);
    SetNotify(IMS_FALSE);

    m_pAosHandle->App_StateChanged(IAosApplication::APP_DISCONNECTED, 0);

    EXPECT_TRUE(GetNotify());
}

TEST_F(AosHandleTest, StateConnecting_Test6)
{
    // Test6: HANDLE_MSG_APP_STATUS, APP_DISCONNECTING
    // Expectation: no need to notify

    m_pAosHandle->SetHandleState(AosHandle::STATE_CONNECTING);
    SetNotify(IMS_FALSE);

    m_pAosHandle->App_StateChanged(IAosApplication::APP_DISCONNECTING, 0);

    EXPECT_FALSE(GetNotify());
}

TEST_F(AosHandleTest, StateConnecting_Test7)
{
    // Test7: Invalid msg
    // Expectation: no need to notify

    m_pAosHandle->SetHandleState(AosHandle::STATE_CONNECTING);
    SetNotify(IMS_FALSE);

    IMSMSG objMSG(2 /*AosHandle::HANDLE_MSG_INVALID*/, 0, 0);
    EXPECT_TRUE(m_pAosHandle->StateConnecting(objMSG));
    EXPECT_FALSE(GetNotify());
}

TEST_F(AosHandleTest, StateConnected_Test1)
{
    // Test1: HANDLE_MSG_BLOCK_STATUS, Handle blocked, reason == 0
    // Expectation: state-disconnecting, request type-detach, call reconfig(), need to notify.

    m_pAosHandle->SetHandleState(AosHandle::STATE_CONNECTED);
    m_pAosHandle->SetRequestType(IAosHandle::ATTACH);
    SetBlocked(IMS_TRUE);
    SetNotify(IMS_FALSE);

    EXPECT_CALL(m_objMockIAosApplication, Reconfig()).Times(1);

    IMSMSG objMSG(0 /*AosHandle::HANDLE_MSG_BLOCK_STATUS*/, 0, 0);
    m_pAosHandle->OnStateMessage(objMSG);

    EXPECT_EQ(m_pAosHandle->GetState(), AosHandle::STATE_DISCONNECTING);
    EXPECT_EQ(m_pAosHandle->GetRequestType(), IAosHandle::DETACH);
    EXPECT_TRUE(GetNotify());
}

TEST_F(AosHandleTest, StateConnected_Test2)
{
    // Test2: HANDLE_MSG_BLOCK_STATUS, Handle blocked, reason > 0
    // Expectation: state-disconnecting, request type-detach, call reconfig(), need to notify,
    //              set reason

    m_pAosHandle->SetHandleState(AosHandle::STATE_CONNECTED);
    m_pAosHandle->SetRequestType(IAosHandle::ATTACH);
    SetBlocked(IMS_TRUE);
    SetNotify(IMS_FALSE);

    EXPECT_CALL(m_objMockIAosApplication, Reconfig()).Times(1);

    IMSMSG objMSG(0 /*AosHandle::HANDLE_MSG_BLOCK_STATUS*/, 1, 0);
    m_pAosHandle->OnStateMessage(objMSG);

    EXPECT_EQ(m_pAosHandle->GetState(), AosHandle::STATE_DISCONNECTING);
    EXPECT_EQ(m_pAosHandle->GetRequestType(), IAosHandle::DETACH);
    EXPECT_TRUE(GetNotify());
    EXPECT_EQ(GetReason(), 1);
}

TEST_F(AosHandleTest, StateConnected_Test3)
{
    // Test3: HANDLE_MSG_BLOCK_STATUS, Handle not blocked, feature tag not changed
    // Expectation: state-connected, request type-attach, no call reconfig(), no need to notify.

    m_pAosHandle->SetHandleState(AosHandle::STATE_CONNECTED);
    m_pAosHandle->SetRequestType(IAosHandle::ATTACH);
    SetBlocked(IMS_FALSE);
    SetNotify(IMS_FALSE);

    EXPECT_CALL(m_objMockIAosApplication, Reconfig()).Times(0);

    IMSMSG objMSG(0 /*AosHandle::HANDLE_MSG_BLOCK_STATUS*/, 0, 0);
    m_pAosHandle->OnStateMessage(objMSG);

    EXPECT_EQ(m_pAosHandle->GetState(), AosHandle::STATE_CONNECTED);
    EXPECT_EQ(m_pAosHandle->GetRequestType(), IAosHandle::ATTACH);
    EXPECT_FALSE(GetNotify());
}

TEST_F(AosHandleTest, StateConnected_Test4)
{
    // Test4: HANDLE_MSG_BLOCK_STATUS, Handle not blocked, feature tag changed
    // Expectation: state-connecting, request type-attach, call reconfig(), no need to notify.

    m_pAosHandle->SetHandleState(AosHandle::STATE_CONNECTING);
    m_pAosHandle->SetRequestType(IAosHandle::ATTACH);
    SetBlocked(IMS_FALSE);
    SetNotify(IMS_FALSE);

    AosFeatureTagList objTestFeatureTagList, objTestBindedFeatureTagList;
    objTestFeatureTagList.AddFeature(
            ImsAosFeature::MMTEL | ImsAosFeature::VIDEO | ImsAosFeature::TEXT);
    objTestBindedFeatureTagList.AddFeature(ImsAosFeature::MMTEL | ImsAosFeature::VIDEO);

    SetFeatureTagList(objTestFeatureTagList);
    SetBindedFeatureTagList(objTestBindedFeatureTagList);

    EXPECT_CALL(m_objMockIAosApplication, Reconfig()).Times(1);

    IMSMSG objMSG(0 /*HANDLE_MSG_BLOCK_STATUS*/, 0, 0);
    m_pAosHandle->OnStateMessage(objMSG);

    EXPECT_EQ(m_pAosHandle->GetState(), AosHandle::STATE_CONNECTING);
    EXPECT_EQ(m_pAosHandle->GetRequestType(), IAosHandle::ATTACH);
    EXPECT_FALSE(GetNotify());
}

TEST_F(AosHandleTest, StateConnected_Test5)
{
    // Test5: HANDLE_MSG_APP_STATUS, APP_DISCONNECTED
    // Expectation: state-connecting, need to notify

    m_pAosHandle->SetHandleState(AosHandle::STATE_CONNECTED);
    SetNotify(IMS_FALSE);

    m_pAosHandle->App_StateChanged(IAosApplication::APP_DISCONNECTED, 0);

    EXPECT_EQ(m_pAosHandle->GetState(), AosHandle::STATE_CONNECTING);
    EXPECT_TRUE(GetNotify());
}

TEST_F(AosHandleTest, StateConnected_Test6)
{
    // Test6: HANDLE_MSG_APP_STATUS, APP_CONNECTED, Reg not binded, handle blocked
    // Expectation: state-disconnected, need to notify

    m_pAosHandle->SetHandleState(AosHandle::STATE_CONNECTED);
    SetBlocked(IMS_TRUE);
    SetNotify(IMS_FALSE);
    m_pAosHandle->SetRegBinded(IMS_FALSE);

    m_pAosHandle->App_StateChanged(IAosApplication::APP_CONNECTED, 0);

    EXPECT_EQ(m_pAosHandle->GetState(), AosHandle::STATE_DISCONNECTED);
    EXPECT_TRUE(GetNotify());
}

TEST_F(AosHandleTest, StateConnected_Test7)
{
    // Test7: HANDLE_MSG_APP_STATUS, APP_CONNECTED, Reg not binded, handle not blocked
    // Expectation: state-connecting, need to notify

    m_pAosHandle->SetHandleState(AosHandle::STATE_CONNECTED);
    SetBlocked(IMS_FALSE);
    SetNotify(IMS_FALSE);
    m_pAosHandle->SetRegBinded(IMS_FALSE);

    m_pAosHandle->App_StateChanged(IAosApplication::APP_CONNECTED, 0);

    EXPECT_EQ(m_pAosHandle->GetState(), AosHandle::STATE_CONNECTING);
    EXPECT_TRUE(GetNotify());
}

TEST_F(AosHandleTest, StateConnected_Test8)
{
    // Test8: HANDLE_MSG_APP_STATUS, APP_DISCONNECTING
    // Expectation: state-disconnecting, need to notify

    m_pAosHandle->SetHandleState(AosHandle::STATE_CONNECTED);

    m_pAosHandle->App_StateChanged(IAosApplication::APP_DISCONNECTING, 0);

    EXPECT_EQ(m_pAosHandle->GetState(), AosHandle::STATE_DISCONNECTING);
    EXPECT_TRUE(GetNotify());
}

TEST_F(AosHandleTest, StateConnected_Test9)
{
    // Test9: HANDLE_MSG_APP_STATUS, APP_UPDATING
    // Expectation: no need to notify

    m_pAosHandle->SetHandleState(AosHandle::STATE_CONNECTED);

    m_pAosHandle->App_StateChanged(IAosApplication::APP_UPDATING, 0);

    EXPECT_FALSE(GetNotify());
}

TEST_F(AosHandleTest, StateConnected_Test10)
{
    // Test10: Invalid msg
    // Expectation: no need to notify

    m_pAosHandle->SetHandleState(AosHandle::STATE_CONNECTED);

    IMSMSG objMSG(2 /*AosHandle::HANDLE_MSG_INVALID*/, 0, 0);
    EXPECT_TRUE(m_pAosHandle->StateConnected(objMSG));
    EXPECT_FALSE(GetNotify());
}

TEST_F(AosHandleTest, StateDisconnecting_Test1)
{
    // Test1: HANDLE_MSG_BLOCK_STATUS, Handle blocked
    // Expectation: request type-detach, call reconfig(), no need to notify.

    m_pAosHandle->SetHandleState(AosHandle::STATE_DISCONNECTING);
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

    m_pAosHandle->SetHandleState(AosHandle::STATE_DISCONNECTING);
    SetBlocked(IMS_FALSE);
    SetNotify(IMS_FALSE);

    EXPECT_CALL(m_objMockIAosApplication, Reconfig()).Times(1);

    IMSMSG objMSG(0 /*HANDLE_MSG_BLOCK_STATUS*/, 0, 0);
    m_pAosHandle->OnStateMessage(objMSG);

    EXPECT_EQ(m_pAosHandle->GetRequestType(), IAosHandle::ATTACH);
    EXPECT_EQ(m_pAosHandle->GetState(), AosHandle::STATE_CONNECTING);
    EXPECT_TRUE(GetNotify());
}

TEST_F(AosHandleTest, StateDisconnecting_Test3)
{
    // Test3: HANDLE_MSG_APP_STATUS, APP_DISCONNECTED, handle blocked
    // Expectation: state-disconnected, need to notify

    m_pAosHandle->SetHandleState(AosHandle::STATE_DISCONNECTING);
    SetBlocked(IMS_TRUE);
    SetNotify(IMS_FALSE);

    m_pAosHandle->App_StateChanged(IAosApplication::APP_DISCONNECTED, 0);

    EXPECT_EQ(m_pAosHandle->GetState(), AosHandle::STATE_DISCONNECTED);
    EXPECT_TRUE(GetNotify());
}

TEST_F(AosHandleTest, StateDisconnecting_Test4)
{
    // Test4: HANDLE_MSG_APP_STATUS, APP_DISCONNECTED, handle not blocked
    // Expectation: state-connecting, need to notify

    m_pAosHandle->SetHandleState(AosHandle::STATE_DISCONNECTING);
    SetBlocked(IMS_FALSE);
    SetNotify(IMS_FALSE);

    m_pAosHandle->App_StateChanged(IAosApplication::APP_DISCONNECTED, 0);

    EXPECT_EQ(m_pAosHandle->GetState(), AosHandle::STATE_CONNECTING);
    EXPECT_TRUE(GetNotify());
}

TEST_F(AosHandleTest, StateDisconnecting_Test5)
{
    // Test5: HANDLE_MSG_APP_STATUS, APP_CONNECTED, Reg binded, handle not blocked
    // Expectation: state-connected, need to notify

    m_pAosHandle->SetHandleState(AosHandle::STATE_DISCONNECTING);
    m_pAosHandle->SetRegBinded(IMS_TRUE);
    SetBlocked(IMS_FALSE);
    SetNotify(IMS_FALSE);

    m_pAosHandle->App_StateChanged(IAosApplication::APP_CONNECTED, 0);

    EXPECT_EQ(m_pAosHandle->GetState(), AosHandle::STATE_CONNECTED);
    EXPECT_TRUE(GetNotify());
}

TEST_F(AosHandleTest, StateDisconnecting_Test6)
{
    // Test6: HANDLE_MSG_APP_STATUS, APP_CONNECTED, Reg not binded, handle blocked
    // Expectation: state-disconnected, need to notify

    m_pAosHandle->SetHandleState(AosHandle::STATE_DISCONNECTING);
    m_pAosHandle->SetRegBinded(IMS_FALSE);
    SetBlocked(IMS_TRUE);
    SetNotify(IMS_FALSE);

    m_pAosHandle->App_StateChanged(IAosApplication::APP_CONNECTED, 0);

    EXPECT_EQ(m_pAosHandle->GetState(), AosHandle::STATE_DISCONNECTED);
    EXPECT_TRUE(GetNotify());
}

TEST_F(AosHandleTest, StateDisconnecting_Test7)
{
    // Test7: HANDLE_MSG_APP_STATUS, APP_CONNECTED, Reg not binded, handle not blocked
    // Expectation: state-connecting, need to notify

    m_pAosHandle->SetHandleState(AosHandle::STATE_DISCONNECTING);
    m_pAosHandle->SetRegBinded(IMS_FALSE);
    SetBlocked(IMS_FALSE);
    SetNotify(IMS_FALSE);

    m_pAosHandle->App_StateChanged(IAosApplication::APP_CONNECTED, 0);

    EXPECT_EQ(m_pAosHandle->GetState(), AosHandle::STATE_CONNECTING);
    EXPECT_TRUE(GetNotify());
}

TEST_F(AosHandleTest, StateDisconnecting_Test8)
{
    // Test8: HANDLE_MSG_APP_STATUS, APP_UPDATING, Reg binded, handle not blocked
    // Expectation: state-connecting, need to notify

    m_pAosHandle->SetHandleState(AosHandle::STATE_DISCONNECTING);
    m_pAosHandle->SetRegBinded(IMS_TRUE);
    SetBlocked(IMS_FALSE);
    SetNotify(IMS_FALSE);

    m_pAosHandle->App_StateChanged(IAosApplication::APP_UPDATING, 0);

    EXPECT_EQ(m_pAosHandle->GetState(), AosHandle::STATE_CONNECTING);
    EXPECT_TRUE(GetNotify());
}

TEST_F(AosHandleTest, StateDisconnecting_Test9)
{
    // Test9: HANDLE_MSG_APP_STATUS, APP_UPDATING, Reg not binded, handle blocked
    // Expectation: state-disconnected, need to notify

    m_pAosHandle->SetHandleState(AosHandle::STATE_DISCONNECTING);
    m_pAosHandle->SetRegBinded(IMS_FALSE);
    SetBlocked(IMS_TRUE);
    SetNotify(IMS_FALSE);

    m_pAosHandle->App_StateChanged(IAosApplication::APP_UPDATING, 0);

    EXPECT_EQ(m_pAosHandle->GetState(), AosHandle::STATE_DISCONNECTED);
    EXPECT_TRUE(GetNotify());
}

TEST_F(AosHandleTest, StateDisconnecting_Test10)
{
    // Test10: HANDLE_MSG_APP_STATUS, APP_UPDATING, Reg not binded, handle not blocked
    // Expectation: state-connecting, need to notify

    m_pAosHandle->SetHandleState(AosHandle::STATE_DISCONNECTING);
    m_pAosHandle->SetRegBinded(IMS_FALSE);
    SetBlocked(IMS_FALSE);
    SetNotify(IMS_FALSE);

    m_pAosHandle->App_StateChanged(IAosApplication::APP_UPDATING, 0);

    EXPECT_EQ(m_pAosHandle->GetState(), AosHandle::STATE_CONNECTING);
    EXPECT_TRUE(GetNotify());
}

TEST_F(AosHandleTest, StateDisconnecting_Test11)
{
    // Test10: HANDLE_MSG_APP_STATUS, APP_DISCONNECTING
    // Expectation: need to notify

    m_pAosHandle->SetHandleState(AosHandle::STATE_DISCONNECTING);
    m_pAosHandle->SetRegBinded(IMS_FALSE);
    SetBlocked(IMS_FALSE);
    SetNotify(IMS_FALSE);

    m_pAosHandle->App_StateChanged(IAosApplication::APP_DISCONNECTING, 0);

    EXPECT_TRUE(GetNotify());
}

TEST_F(AosHandleTest, StateDisconnecting_Test12)
{
    // Test10: HANDLE_MSG_APP_STATUS, invalid app status
    // Expectation: no need to notify

    m_pAosHandle->SetHandleState(AosHandle::STATE_DISCONNECTING);
    m_pAosHandle->SetRegBinded(IMS_FALSE);
    SetBlocked(IMS_FALSE);
    SetNotify(IMS_FALSE);

    IMSMSG objMSG(1 /*AosHandle::HANDLE_MSG_APP_STATUS*/, 4 /*Invalid app status*/, 0);
    m_pAosHandle->OnStateMessage(objMSG);

    EXPECT_FALSE(GetNotify());
}

TEST_F(AosHandleTest, StateDisconnecting_Test13)
{
    // Test10: Invalid msg
    // Expectation: no need to notify

    m_pAosHandle->SetHandleState(AosHandle::STATE_DISCONNECTING);
    m_pAosHandle->SetRegBinded(IMS_FALSE);
    SetBlocked(IMS_FALSE);
    SetNotify(IMS_FALSE);

    IMSMSG objMSG(2 /*AosHandle::HANDLE_MSG_APP_INVALID*/, 0, 0);
    EXPECT_TRUE(m_pAosHandle->StateDisconnecting(objMSG));
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

TEST_F(AosHandleTest, InitializeHoldingBlocksPolicy_Test1)
{
    // Test1: IsDeregOn3gNetwork is false
    // Expectation: empty list

    SetHoldingBlocksPolicyForMobile();
    SetHoldingBlocksPolicyForWifi();

    EXPECT_TRUE(GetHoldingBlocksPolicyForMobile().GetSize() > 0);
    EXPECT_TRUE(GetHoldingBlocksPolicyForWifi().GetSize() > 0);

    EXPECT_CALL(m_objMockIAosNConfiguration, IsDeregOn3gNetwork())
            .Times(1)
            .WillOnce(Return(IMS_FALSE));

    m_pAosHandle->InitializeHoldingBlocksPolicy();

    EXPECT_FALSE(GetHoldingBlocksPolicyForMobile().GetSize() > 0);
    EXPECT_FALSE(GetHoldingBlocksPolicyForWifi().GetSize() > 0);
}

TEST_F(AosHandleTest, InitializeHoldingBlocksPolicy_Test2)
{
    // Test2: IsDeregOn3gNetwork is true
    // Expectation: BLOCK_3G is in mobile polity

    SetHoldingBlocksPolicyForMobile();
    SetHoldingBlocksPolicyForWifi();

    EXPECT_TRUE(GetHoldingBlocksPolicyForMobile().GetSize() > 0);
    EXPECT_TRUE(GetHoldingBlocksPolicyForWifi().GetSize() > 0);

    EXPECT_CALL(m_objMockIAosNConfiguration, IsDeregOn3gNetwork())
            .Times(1)
            .WillOnce(Return(IMS_TRUE));

    m_pAosHandle->InitializeHoldingBlocksPolicy();

    ASSERT_TRUE(GetHoldingBlocksPolicyForMobile().GetSize() == 1);
    EXPECT_EQ(GetHoldingBlocksPolicyForMobile().GetAt(0), AosHandle::BLOCK_3G);
    EXPECT_FALSE(GetHoldingBlocksPolicyForWifi().GetSize() > 0);
}

TEST_F(AosHandleTest, InitializeFeatureTags_Test)
{
    // Expectation: +cdmaless if the config is set

    EXPECT_CALL(m_objMockIAosNConfiguration, IsCdmalessFeatureTagRequired())
            .Times(2)
            .WillOnce(Return(IMS_TRUE))
            .WillOnce(Return(IMS_FALSE));

    m_pAosHandle->InitializeFeatureTags();
    EXPECT_TRUE(HasFeatureTag(FeatureTags::CDMALESS, AString::ConstNull()));

    m_pAosHandle->InitializeFeatureTags();
    EXPECT_FALSE(HasFeatureTag(FeatureTags::CDMALESS, AString::ConstNull()));
}

TEST_F(AosHandleTest, ProcessImsSuspended_Test1)
{
    // Test1: ims not connected
    // Expectation: no suspended reason is set, return false

    m_pAosHandle->SetHandleState(AosHandle::STATE_DISCONNECTED);
    m_pAosHandle->ClearSuspendedReason();
    EXPECT_FALSE(m_pAosHandle->ProcessImsSuspended(0));
    EXPECT_EQ(m_pAosHandle->GetSuspendedReason(), AosReason::SUSPEND_NONE);
}

TEST_F(AosHandleTest, ProcessImsSuspended_Test2)
{
    // Test2: ims connected, suspended reason-SUSPEND_NO_SERVICE, listener is not null
    // Expectation: Call ImsAos_Suspended, return true

    m_pAosHandle->SetHandleState(AosHandle::STATE_CONNECTED);
    IImsAosListener* piListener = static_cast<IImsAosListener*>(&m_objMockIImsAosListener);
    m_pAosHandle->SetListener(piListener);
    EXPECT_CALL(m_objMockIImsAosListener, ImsAos_Suspended(_)).Times(1);

    EXPECT_TRUE(m_pAosHandle->ProcessImsSuspended(AosReason::SUSPEND_NO_SERVICE));
}

TEST_F(AosHandleTest, ProcessImsSuspended_Test3)
{
    // Test3: ims connected, suspended reason-SUSPEND_NO_SERVICE, listener is null
    // Expectation: suspended reason is set, No call ImsAos_Suspended, return false

    m_pAosHandle->SetHandleState(AosHandle::STATE_CONNECTED);
    m_pAosHandle->ClearSuspendedReason();
    m_pAosHandle->SetListener(IMS_NULL);
    EXPECT_CALL(m_objMockIImsAosListener, ImsAos_Suspended(_)).Times(0);

    EXPECT_FALSE(m_pAosHandle->ProcessImsSuspended(AosReason::SUSPEND_NO_SERVICE));
    EXPECT_EQ(m_pAosHandle->GetSuspendedReason(), AosReason::SUSPEND_NO_SERVICE);
}

TEST_F(AosHandleTest, ProcessImsSuspended_Test4)
{
    // Test4: IsEmergencyService true
    // Expectation: return false

    MockIAosAppContext objMockIAosAppContext;
    MockIAosNConfiguration objMockIAosNConfiguration;

    EXPECT_CALL(objMockIAosAppContext, GetSlotId()).Times(AnyNumber()).WillRepeatedly(Return(0));

    const AString strValue = AString("test");
    EXPECT_CALL(objMockIAosAppContext, GetProfileId())
            .Times(AnyNumber())
            .WillRepeatedly(ReturnRef(strValue));

    IAosNConfiguration* piAosNConfiguration = AosProvider::GetInstance()->GetNConfiguration();
    AosProvider::GetInstance()->SetNConfiguration(
            static_cast<IAosNConfiguration*>(&objMockIAosNConfiguration));

    TestAosHandle* pTestAosHandleEmergencyMtc =
            new TestAosHandle(static_cast<IAosAppContext*>(&objMockIAosAppContext), m_strAppId,
                    m_strServiceId, ImsAosService::EMERGENCY_MTC);

    ASSERT_TRUE(pTestAosHandleEmergencyMtc != nullptr);

    EXPECT_FALSE(pTestAosHandleEmergencyMtc->ProcessImsSuspended());

    delete pTestAosHandleEmergencyMtc;

    AosProvider::GetInstance()->SetNConfiguration(piAosNConfiguration);
}

TEST_F(AosHandleTest, ProcessImsResumed_Test1)
{
    // Test1: ims not connected
    // Expectation: suspended reason is not reset, return false

    m_pAosHandle->SetHandleState(AosHandle::STATE_DISCONNECTED);
    m_pAosHandle->SetSuspendedReason(AosReason::SUSPEND_NO_SERVICE);

    EXPECT_FALSE(m_pAosHandle->ProcessImsResumed(AosReason::SUSPEND_NO_SERVICE));
    EXPECT_EQ(m_pAosHandle->GetSuspendedReason(), AosReason::SUSPEND_NO_SERVICE);
}

TEST_F(AosHandleTest, ProcessImsResumed_Test2)
{
    // Test2: ims connected, ims not suspended
    // Expectation: return false

    m_pAosHandle->SetHandleState(AosHandle::STATE_CONNECTED);
    m_pAosHandle->ClearSuspendedReason();

    EXPECT_FALSE(m_pAosHandle->ProcessImsResumed(AosReason::SUSPEND_NO_SERVICE));
}

TEST_F(AosHandleTest, ProcessImsResumed_Test3)
{
    // Test3: ims connected, ims suspended, still suspended after reset the reason.
    // Expectation: reset the reason from suspended reason, return false

    m_pAosHandle->SetHandleState(AosHandle::STATE_CONNECTED);
    m_pAosHandle->ClearSuspendedReason();
    SetSuspendedReasonForTest((AosReason::SUSPEND_NO_SERVICE | AosReason::SUSPEND_NO_LTE_COVERAGE));

    EXPECT_FALSE(m_pAosHandle->ProcessImsResumed(AosReason::SUSPEND_NO_SERVICE));
    EXPECT_EQ(m_pAosHandle->GetSuspendedReason(), AosReason::SUSPEND_NO_LTE_COVERAGE);
}

TEST_F(AosHandleTest, ProcessImsResumed_Test4)
{
    // Test4: ims connected, ims suspended, no suspended reason after reset, no listener.
    // Expectation: reset the reason from suspended reason, set reason, return false

    m_pAosHandle->SetHandleState(AosHandle::STATE_CONNECTED);
    m_pAosHandle->ClearSuspendedReason();
    m_pAosHandle->SetSuspendedReason(AosReason::SUSPEND_NO_SERVICE);
    m_pAosHandle->SetListener(IMS_NULL);
    m_pAosHandle->SetReason(AosReason::SUSPEND_NONE);

    EXPECT_FALSE(m_pAosHandle->ProcessImsResumed(AosReason::SUSPEND_NO_SERVICE));
    EXPECT_EQ(m_pAosHandle->GetSuspendedReason(), AosReason::SUSPEND_NONE);
    EXPECT_EQ(GetReason(), AosReason::SUSPEND_NO_SERVICE);
}

TEST_F(AosHandleTest, ProcessImsResumed_Test5)
{
    // Test5: ims connected, ims suspended, no suspended reason after reset, listener existed.
    // Expectation: no suspended reason, set reason, call ImsAos_Resumed(), return true

    m_pAosHandle->SetHandleState(AosHandle::STATE_CONNECTED);
    m_pAosHandle->ClearSuspendedReason();
    m_pAosHandle->SetSuspendedReason(AosReason::SUSPEND_NO_SERVICE);
    IImsAosListener* piListener = static_cast<IImsAosListener*>(&m_objMockIImsAosListener);
    m_pAosHandle->SetListener(piListener);
    m_pAosHandle->SetReason(AosReason::SUSPEND_NONE);

    EXPECT_CALL(m_objMockIImsAosListener, ImsAos_Resumed()).Times(1);
    EXPECT_TRUE(m_pAosHandle->ProcessImsResumed(AosReason::SUSPEND_NO_SERVICE));
    EXPECT_EQ(m_pAosHandle->GetSuspendedReason(), AosReason::SUSPEND_NONE);
    EXPECT_EQ(GetReason(), AosReason::SUSPEND_NO_SERVICE);

    // Clean up
    m_pAosHandle->SetHandleState(AosHandle::STATE_DISCONNECTED);
    m_pAosHandle->ClearSuspendedReason();
    m_pAosHandle->SetListener(IMS_NULL);
    m_pAosHandle->SetReason(AosReason::SUSPEND_NONE);
}

TEST_F(AosHandleTest, ProcessImsResumed_Test6)
{
    // Test6: Emergency service.
    // Expectation: return false

    MockIAosAppContext objMockIAosAppContext;
    MockIAosNConfiguration objMockIAosNConfiguration;

    EXPECT_CALL(objMockIAosAppContext, GetSlotId()).Times(AnyNumber()).WillRepeatedly(Return(0));

    const AString strValue = AString("test");
    EXPECT_CALL(objMockIAosAppContext, GetProfileId())
            .Times(AnyNumber())
            .WillRepeatedly(ReturnRef(strValue));

    IAosNConfiguration* piAosNConfiguration = AosProvider::GetInstance()->GetNConfiguration();
    AosProvider::GetInstance()->SetNConfiguration(
            static_cast<IAosNConfiguration*>(&objMockIAosNConfiguration));

    TestAosHandle* pTestAosHandleEmergencyMtc =
            new TestAosHandle(static_cast<IAosAppContext*>(&objMockIAosAppContext), m_strAppId,
                    m_strServiceId, ImsAosService::EMERGENCY_MTC);

    ASSERT_TRUE(pTestAosHandleEmergencyMtc != nullptr);

    EXPECT_FALSE(pTestAosHandleEmergencyMtc->ProcessImsResumed());

    delete pTestAosHandleEmergencyMtc;

    AosProvider::GetInstance()->SetNConfiguration(piAosNConfiguration);
}

TEST_F(AosHandleTest, CheckSuspended_Test1)
{
    // Test1: Suspended.
    // Expectation: Suspended reason-SUSPEND_NO_SERVICE, m_bNetSrvIn is false.

    m_pAosHandle->ClearSuspendedReason();

    EXPECT_CALL(m_objMockIAosNetTracker, IsSuspended()).Times(1).WillOnce(Return(IMS_TRUE));

    m_pAosHandle->CheckSuspended();

    EXPECT_EQ(m_pAosHandle->GetSuspendedReason(), AosReason::SUSPEND_NO_SERVICE);
    EXPECT_FALSE(GetNetSrvIn());
}

TEST_F(AosHandleTest, CheckSuspended_Test2)
{
    // Test2: Not suspended.
    // Expectation: Suspended reason-SUSPEND_NONE, m_bNetSrvIn is true.

    m_pAosHandle->ClearSuspendedReason();

    EXPECT_CALL(m_objMockIAosNetTracker, IsSuspended()).Times(1).WillOnce(Return(IMS_FALSE));

    m_pAosHandle->CheckSuspended();

    EXPECT_EQ(m_pAosHandle->GetSuspendedReason(), AosReason::SUSPEND_NONE);
    EXPECT_TRUE(GetNetSrvIn());
}

TEST_F(AosHandleTest, ResetSuspendedReason_Test1)
{
    // Test1: reason-SUSPEND_NO_SERVICE
    // Expectation: reset the reason from suspended reason

    m_pAosHandle->ClearSuspendedReason();
    m_pAosHandle->SetSuspendedReason(AosReason::SUSPEND_NO_SERVICE);
    EXPECT_EQ(m_pAosHandle->GetSuspendedReason(), AosReason::SUSPEND_NO_SERVICE);

    m_pAosHandle->ResetSuspendedReason(AosReason::SUSPEND_NO_SERVICE);

    EXPECT_EQ(m_pAosHandle->GetSuspendedReason(), AosReason::SUSPEND_NONE);
}

TEST_F(AosHandleTest, ResetSuspendedReason_Test2)
{
    // Test2: reason-other than SUSPEND_NO_SERVICE
    // Expectation: no reset the reason from suspended reason

    m_pAosHandle->ClearSuspendedReason();
    SetSuspendedReasonForTest(AosReason::SUSPEND_NO_LTE_COVERAGE);
    EXPECT_EQ(m_pAosHandle->GetSuspendedReason(), AosReason::SUSPEND_NO_LTE_COVERAGE);

    m_pAosHandle->ResetSuspendedReason(AosReason::SUSPEND_NO_LTE_COVERAGE);

    EXPECT_EQ(m_pAosHandle->GetSuspendedReason(), AosReason::SUSPEND_NO_LTE_COVERAGE);

    m_pAosHandle->ClearSuspendedReason();
}

TEST_F(AosHandleTest, ReportRegState_Test1)
{
    // Test1: RegStateManager is null
    // Expectation: no call SetRegState with any parameteres.

    AosProvider::GetInstance()->SetRegStateManager(IMS_NULL);

    EXPECT_CALL(m_objMockIAosRegStateManager, SetRegState(_, _)).Times(0);

    m_pAosHandle->ReportRegState();
}

TEST_F(AosHandleTest, ReportRegState_Test2)
{
    // Test2: RegStateManager is not null, state connected
    // Expectation: call SetRegState with IMS_REG_ON.

    AosProvider::GetInstance()->SetRegStateManager(
            static_cast<IAosRegStateManager*>(&m_objMockIAosRegStateManager));
    m_pAosHandle->SetHandleState(AosHandle::STATE_CONNECTED);

    EXPECT_CALL(m_objMockIAosRegStateManager, SetRegState(_, IMS_REG_ON)).Times(1);

    m_pAosHandle->ReportRegState();
}

TEST_F(AosHandleTest, ReportRegState_Test3)
{
    // Test3: RegStateManager is not null, state other than connected
    // Expectation: call SetRegState with IMS_REG_OFF.

    EXPECT_CALL(m_objMockIAosRegStateManager, SetRegState(_, IMS_REG_OFF)).Times(3);

    m_pAosHandle->SetHandleState(AosHandle::STATE_DISCONNECTED);
    m_pAosHandle->ReportRegState();

    m_pAosHandle->SetHandleState(AosHandle::STATE_DISCONNECTING);
    m_pAosHandle->ReportRegState();

    m_pAosHandle->SetHandleState(AosHandle::STATE_CONNECTING);
    m_pAosHandle->ReportRegState();
}

TEST_F(AosHandleTest, ReportRegState_Test4)
{
    // Test4: RegStateManager is not null, invalid state
    // Expectation: no call SetRegState with any parameteres.

    AosProvider::GetInstance()->SetRegStateManager(IMS_NULL);
    m_pAosHandle->SetHandleState(AosHandle::STATE_INVALID);

    EXPECT_CALL(m_objMockIAosRegStateManager, SetRegState(_, _)).Times(0);

    m_pAosHandle->ReportRegState();
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

TEST_F(AosHandleTest, UpdateFeature_for_ImsAosFeatureTag_Test)
{
    ImsList<ImsAosFeatureTag*> objFeatureTag;
    m_pAosHandle->UpdateFeature(objFeatureTag);
}

TEST_F(AosHandleTest, RequestAppToRegisterWithNextPcscf)
{
    // GIVEN
    EXPECT_CALL(m_objMockIAosApplication, RequestCmd(ImsAosControl::PCSCF_NEXT_WITH_DISCOVERY, _))
            .Times(2);

    // WHEN
    m_pAosHandle->RegisterWithNextPcscf(30);
    m_pAosHandle->RegisterWithNextPcscf(0);

    // THEN: The GIVEN condition should be met.
}

TEST_F(AosHandleTest, CallTracker_StateChanged_Test)
{
    m_pAosHandle->CallTracker_StateChanged(IAosCallTracker::TYPE_NORMAL, CallState::IDLE);
}

TEST_F(AosHandleTest, ReevaluateUnavailableFeature_Test)
{
    m_pAosHandle->ReevaluateUnavailableFeature();
}

TEST_F(AosHandleTest, ProcessCapabilitiesChanged_Test)
{
    m_pAosHandle->ProcessCapabilitiesChanged(ImsMap<IMS_UINT32, IMS_UINT32>());
}

TEST_F(AosHandleTest, ProcessNetworkChanged_Test)
{
    m_pAosHandle->ProcessNetworkChanged();
}

TEST_F(AosHandleTest, ProcessVopsStateChanged_Test)
{
    m_pAosHandle->ProcessVopsStateChanged(0);
}

TEST_F(AosHandleTest, ProcessPsRoamingStateChanged_Test)
{
    m_pAosHandle->ProcessPsRoamingStateChanged(IMS_ROAMING_STATE_OFF);
    EXPECT_FALSE(m_pAosHandle->IsRoaming());

    m_pAosHandle->ProcessPsRoamingStateChanged(IMS_ROAMING_STATE_ON);
    EXPECT_TRUE(m_pAosHandle->IsRoaming());
}

TEST_F(AosHandleTest, IsSupportedNetworkType_Test)
{
    EXPECT_TRUE(m_pAosHandle->IsSupportedNetworkType(NW_REPORT_RADIO_LTE));
    EXPECT_TRUE(m_pAosHandle->IsSupportedNetworkType(NW_REPORT_RADIO_NR));
    EXPECT_TRUE(m_pAosHandle->IsSupportedNetworkType(NW_REPORT_RADIO_WLAN));
    EXPECT_FALSE(m_pAosHandle->IsSupportedNetworkType(NW_REPORT_RADIO_CDMA));
}

TEST_F(AosHandleTest, IsSupportedNetworkTypeForCellular_Test)
{
    EXPECT_TRUE(m_pAosHandle->IsSupportedNetworkTypeForCellular(NW_REPORT_RADIO_LTE));
    EXPECT_TRUE(m_pAosHandle->IsSupportedNetworkTypeForCellular(NW_REPORT_RADIO_NR));
    EXPECT_FALSE(m_pAosHandle->IsSupportedNetworkTypeForCellular(NW_REPORT_RADIO_WLAN));
    EXPECT_FALSE(m_pAosHandle->IsSupportedNetworkTypeForCellular(NW_REPORT_RADIO_CDMA));
}

TEST_F(AosHandleTest, Is3G_Test)
{
    EXPECT_TRUE(m_pAosHandle->Is3G(NW_REPORT_RADIO_WCDMA));
    EXPECT_TRUE(m_pAosHandle->Is3G(NW_REPORT_RADIO_HSPA));
    EXPECT_FALSE(m_pAosHandle->Is3G(NW_REPORT_RADIO_GSM));
    EXPECT_FALSE(m_pAosHandle->Is3G(NW_REPORT_RADIO_EDGE));
}

TEST_F(AosHandleTest, Event_NotifyEvent_Test)
{
    IMS_BOOL bIsWifiTest = AosUtil::GetInstance()->IsWifiTest();
    AosUtil::GetInstance()->SetWifiTest(IMS_TRUE);
    m_pAosHandle->Event_NotifyEvent(IMS_EVENT_IMS_VOICE_OVER_PS_STATE, 1, 0);
    AosUtil::GetInstance()->SetWifiTest(bIsWifiTest);
    m_pAosHandle->Event_NotifyEvent(IMS_EVENT_IMS_VOICE_OVER_PS_STATE, 1, 0);

    m_pAosHandle->Event_NotifyEvent(IMS_EVENT_ROAMING_STATE, 1, 0);

    m_pAosHandle->Event_NotifyEvent(
            IMS_EVENT_LTE_INFO, IMS_LTE_INFO_EPS_ONLY_ATTACHED, IMS_LTE_INFO_EXTRA_NONE);
    EXPECT_FALSE(IsCsVoiceAvailable());
    m_pAosHandle->Event_NotifyEvent(
            IMS_EVENT_LTE_INFO, IMS_LTE_INFO_COMBINED_ATTACHED, IMS_LTE_INFO_EXTRA_NONE);
    EXPECT_TRUE(IsCsVoiceAvailable());
    m_pAosHandle->Event_NotifyEvent(IMS_EVENT_LTE_INFO, IMS_LTE_INFO_COMBINED_ATTACHED,
            IMS_LTE_INFO_EXTRA_CSFB_NOT_PREFERRED);
    EXPECT_FALSE(IsCsVoiceAvailable());
    m_pAosHandle->Event_NotifyEvent(
            IMS_EVENT_LTE_INFO, IMS_LTE_INFO_COMBINED_ATTACHED, IMS_LTE_INFO_EXTRA_SMS_ONLY);
    EXPECT_FALSE(IsCsVoiceAvailable());
    m_pAosHandle->Event_NotifyEvent(IMS_EVENT_LTE_INFO, IMS_LTE_INFO_COMBINED_ATTACHED,
            (IMS_LTE_INFO_EXTRA_CSFB_NOT_PREFERRED | IMS_LTE_INFO_EXTRA_SMS_ONLY));
    EXPECT_FALSE(IsCsVoiceAvailable());
}

TEST_F(AosHandleTest, Event_NotifyEvent_InvalidEvent)
{
    m_pAosHandle->Event_NotifyEvent(IMS_EVENT_VOLTE_SETTING, IMS_VOLTE_SETTING_OFF, 0);
}

TEST_F(AosHandleTest, RegistrationControl_NotifyCapabilitiesChanged_Test)
{
    IMS_BOOL bIsWifiTest = AosUtil::GetInstance()->IsWifiTest();
    AosUtil::GetInstance()->SetWifiTest(IMS_TRUE);
    m_pAosHandle->RegistrationControl_NotifyCapabilitiesChanged(ImsMap<IMS_UINT32, IMS_UINT32>());
    AosUtil::GetInstance()->SetWifiTest(bIsWifiTest);
    m_pAosHandle->RegistrationControl_NotifyCapabilitiesChanged(ImsMap<IMS_UINT32, IMS_UINT32>());
}

TEST_F(AosHandleTest, ServiceSetting_RoamingPreferredVoiceNetworkChanged_Test)
{
    m_pAosHandle->ServiceSetting_RoamingPreferredVoiceNetworkChanged(
            RoamingPreferredVoiceNetwork::CELLULAR);
}

TEST_F(AosHandleTest, StateToString_Test)
{
    EXPECT_STREQ(m_pAosHandle->StateToString(0 /*STATE_DISCONNECTED*/), "STATE_DISCONNECTED");
    EXPECT_STREQ(m_pAosHandle->StateToString(1 /*STATE_CONNECTING*/), "STATE_CONNECTING");
    EXPECT_STREQ(m_pAosHandle->StateToString(2 /*STATE_CONNECTED*/), "STATE_CONNECTED");
    EXPECT_STREQ(m_pAosHandle->StateToString(3 /*STATE_DISCONNECTING*/), "STATE_DISCONNECTING");
    EXPECT_STREQ(m_pAosHandle->StateToString(4 /*invalid*/), "__INVALID__");
}

TEST_F(AosHandleTest, MsgToString_Test)
{
    EXPECT_STREQ(
            m_pAosHandle->MsgToString(0 /*HANDLE_MSG_BLOCK_STATUS*/), "HANDLE_MSG_BLOCK_STATUS");
    EXPECT_STREQ(m_pAosHandle->MsgToString(1 /*HANDLE_MSG_APP_STATUS*/), "HANDLE_MSG_APP_STATUS");
    EXPECT_STREQ(m_pAosHandle->MsgToString(2 /*HANDLE_MSG_INVALID*/), "__INVALID__");
}

TEST_F(AosHandleTest, RadioTypeToString_Test)
{
    EXPECT_STREQ(m_pAosHandle->RadioTypeToString(NW_REPORT_RADIO_WLAN), "WLAN");
    EXPECT_STREQ(m_pAosHandle->RadioTypeToString(NW_REPORT_RADIO_LTE), "LTE");
    EXPECT_STREQ(m_pAosHandle->RadioTypeToString(NW_REPORT_RADIO_NR), "NR");
    EXPECT_STREQ(m_pAosHandle->RadioTypeToString(NW_REPORT_RADIO_WCDMA), "3G");
    EXPECT_STREQ(m_pAosHandle->RadioTypeToString(NW_REPORT_RADIO_HSPA), "3G");
    EXPECT_STREQ(m_pAosHandle->RadioTypeToString(NW_REPORT_RADIO_CDMA), "__INVALID__");
}

TEST_F(AosHandleTest, ServiceTypeToString_Test)
{
    SetServiceType(ImsAosService::MTC);
    EXPECT_STREQ(m_pAosHandle->ServiceTypeToString(), "mtc");

    SetServiceType(ImsAosService::MTS);
    EXPECT_STREQ(m_pAosHandle->ServiceTypeToString(), "mts");

    SetServiceType(ImsAosService::EMERGENCY_MTC);
    EXPECT_STREQ(m_pAosHandle->ServiceTypeToString(), "mtc");

    SetServiceType(ImsAosService::EMERGENCY_MTS);
    EXPECT_STREQ(m_pAosHandle->ServiceTypeToString(), "mts");

    SetServiceType(ImsAosService::UCE);
    EXPECT_STREQ(m_pAosHandle->ServiceTypeToString(), "uce");

    SetServiceType(ImsAosService::SIP_CONTROLLER);
    EXPECT_STREQ(m_pAosHandle->ServiceTypeToString(), "sip_controller");

    SetServiceType(ImsAosService::NONE);
    EXPECT_STREQ(m_pAosHandle->ServiceTypeToString(), "invalid");
}
