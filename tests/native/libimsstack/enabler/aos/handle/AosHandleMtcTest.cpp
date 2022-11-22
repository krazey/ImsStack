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
#include "CarrierConfig.h"
#include "ImsAosParameter.h"
#include "ImsAosReason.h"
#include "ImsEventDef.h"
#include "INetworkWatcher.h"

#include "handle/AosHandle.h"
#include "handle/AosHandleMtc.h"
#include "interface/IAosAppContext.h"
#include "interface/IAosNConfiguration.h"
#include "interface/IAosRegistration.h"
#include "provider/AosProvider.h"
#include "provider/AosString.h"

#include "interface/MockIAosAppContext.h"
#include "interface/MockIAosApplication.h"
#include "interface/MockIAosCallTracker.h"
#include "interface/MockIAosConnection.h"
#include "interface/MockIAosHandle.h"
#include "interface/MockIAosNConfiguration.h"
#include "interface/MockIAosNetTracker.h"
#include "interface/MockIAosRegistration.h"

#include "../../interface/aos/MockIImsAosListener.h"

using ::testing::_;
using ::testing::AnyNumber;
using ::testing::Return;
using ::testing::ReturnRef;

class AosHandleMtcTest : public ::testing::Test
{
public:
    AosHandleMtc* m_pAosHandleMtc;

    MockIAosAppContext m_objMockIAosAppContext;
    MockIAosApplication m_objMockIAosApplication;
    MockIAosConnection m_objMockIAosConnection;
    MockIAosHandle m_objMockIAosHandle;
    MockIAosNetTracker m_objMockIAosNetTracker;
    MockIAosRegistration m_objMockIAosRegistration;
    MockIImsAosListener m_objMockIImsAosListener;

    IAosNConfiguration* m_piAosNConfiguration;
    MockIAosNConfiguration m_objMockIAosNConfiguration;

    IAosCallTracker* m_piAosCallTracker;
    MockIAosCallTracker m_objMockIAosCallTracker;

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

        const AString strAppId = AString("ims.app.mtc.test");
        const AString strServiceId = AString("ims.service.mtc.test");
        const IMS_UINT32 nServiceType = -1;

        m_piAosNConfiguration = AosProvider::GetInstance()->GetNConfiguration();
        AosProvider::GetInstance()->SetNConfiguration(
                static_cast<IAosNConfiguration*>(&m_objMockIAosNConfiguration));

        EXPECT_CALL(m_objMockIAosAppContext, GetApp())
                .Times(AnyNumber())
                .WillRepeatedly(Return(&m_objMockIAosApplication));

        EXPECT_CALL(m_objMockIAosAppContext, GetNetTracker())
                .Times(AnyNumber())
                .WillRepeatedly(Return(&m_objMockIAosNetTracker));

        m_piAosCallTracker = AosProvider::GetInstance()->GetCallTracker();
        AosProvider::GetInstance()->SetCallTracker(
                static_cast<IAosCallTracker*>(&m_objMockIAosCallTracker), 0);

        EXPECT_CALL(m_objMockIAosNConfiguration, SetListener(_)).Times(1);

        EXPECT_CALL(m_objMockIAosAppContext, GetRegistration())
                .Times(AnyNumber())
                .WillRepeatedly(Return(static_cast<IAosRegistration*>(&m_objMockIAosRegistration)));

        EXPECT_CALL(m_objMockIAosAppContext, GetConnection())
                .Times(AnyNumber())
                .WillRepeatedly(Return(&m_objMockIAosConnection));

        EXPECT_CALL(m_objMockIAosRegistration, RequestCmd(_, _)).Times(AnyNumber());

        EXPECT_CALL(m_objMockIAosNConfiguration, IsVopsIgnoredForVolteEnabled())
                .Times(AnyNumber())
                .WillRepeatedly(Return(IMS_TRUE));

        EXPECT_CALL(m_objMockIAosNConfiguration, IsWfcImsAvailable())
                .Times(AnyNumber())
                .WillRepeatedly(Return(IMS_FALSE));

        EXPECT_CALL(m_objMockIAosNConfiguration,
                IsGGsmaRcsTelephonyFeatureTagUsedAsAvailableVoiceCallType())
                .Times(AnyNumber())
                .WillRepeatedly(Return(IMS_FALSE));

        m_pAosHandleMtc = new AosHandleMtc(static_cast<IAosAppContext*>(&m_objMockIAosAppContext),
                strAppId, strServiceId, nServiceType);

        ASSERT_TRUE(m_pAosHandleMtc != nullptr);

        m_pAosHandleMtc->m_bDataConnected = IMS_TRUE;
    }

    virtual void TearDown() override
    {
        if (m_pAosHandleMtc != nullptr)
        {
            delete m_pAosHandleMtc;
            m_pAosHandleMtc = nullptr;
        }

        AosProvider::GetInstance()->SetNConfiguration(m_piAosNConfiguration);
        AosProvider::GetInstance()->SetCallTracker(
                static_cast<IAosCallTracker*>(m_piAosCallTracker), 0);
    }

    IMSMap<IMS_UINT32, IMS_UINT32> GetCapabilities() { return m_pAosHandleMtc->m_objCapabilities; }

    void SetCapabilities(IN const IMSMap<IMS_UINT32, IMS_UINT32>& objNewCapabilities)
    {
        m_pAosHandleMtc->m_objCapabilities = objNewCapabilities;
    }

    IMS_BOOL IsServiceFeature(IN IMS_UINT32 nFeature)
    {
        return m_pAosHandleMtc->IsServiceFeature(nFeature);
    }

    IMS_BOOL IsBlockForMobile(IN IMS_UINT32 nBlock)
    {
        return m_pAosHandleMtc->IsBlockForMobile(nBlock);
    }

    IMS_BOOL IsBlockForWifi(IN IMS_UINT32 nBlock)
    {
        return m_pAosHandleMtc->IsBlockForWifi(nBlock);
    }

    void SetHoldingVopsState(IN IMS_UINT32 nState)
    {
        m_pAosHandleMtc->m_nHoldingVopsState = nState;
    }

    IMS_UINT32 GetHoldingVopsState() { return m_pAosHandleMtc->m_nHoldingVopsState; }

    IMS_UINT32 GetVopsState() { return m_pAosHandleMtc->m_nVopsState; }

    IMS_BOOL IsHandleBlocked(IN IMS_UINT32 nBlock)
    {
        return m_pAosHandleMtc->AosHandle::IsHandleBlocked(nBlock);
    }

    IMS_BOOL IsHandleBlocked() { return m_pAosHandleMtc->IsHandleBlocked(); }

    IMS_BOOL IsHandleBlockedBase() { return m_pAosHandleMtc->AosHandle::IsHandleBlocked(); }

    void AddFeature(IN IMS_UINT32 nFeature)
    {
        m_pAosHandleMtc->m_objFeatureTagList.AddFeature(nFeature);
    }

    void RemoveFeature(IN IMS_UINT32 nFeature)
    {
        m_pAosHandleMtc->m_objFeatureTagList.RemoveFeature(nFeature);
    }

    void AddBindedFeature(IN IMS_UINT32 nFeature)
    {
        m_pAosHandleMtc->m_objBindedFeatureTagList.AddFeature(nFeature);
    }

    IMS_BOOL AddFeatureTag(IN const AString& strName, IN const AString& strValue)
    {
        return m_pAosHandleMtc->m_objFeatureTagList.AddFeatureTag(strName, strValue);
    }

    IMS_BOOL AddBindedFeatureTag(IN const AString& strName, IN const AString& strValue)
    {
        return m_pAosHandleMtc->m_objBindedFeatureTagList.AddFeatureTag(strName, strValue);
    }

    void SetEpdgEnabled(IN IMS_BOOL bEnabled) { m_pAosHandleMtc->m_bEpdgEnabled = bEnabled; }

    void SetBlocked(IMS_BOOL bBlocked) { m_pAosHandleMtc->m_bBlocked = bBlocked; }

    IMS_BOOL IsBlocked() { return m_pAosHandleMtc->IsBlocked(); }

    void AddBlock(IN IMS_UINT32 nBlock)
    {
        m_pAosHandleMtc->AddBlock(nBlock, m_pAosHandleMtc->m_nBlocks);
    }

    void RemoveBlock(IN IMS_UINT32 nBlock)
    {
        m_pAosHandleMtc->RemoveBlock(nBlock, m_pAosHandleMtc->m_nBlocks);
    }

    void InitializeServiceBlock() { m_pAosHandleMtc->InitializeServiceBlock(); }

    void InitializeServiceFeature() { m_pAosHandleMtc->InitializeServiceFeature(); }

    void InitializeFeatureTags() { m_pAosHandleMtc->InitializeFeatureTags(); }

    void UpdateGGsmaRcsTelephonyFeatureTag()
    {
        m_pAosHandleMtc->UpdateGGsmaRcsTelephonyFeatureTag();
    }

    void SetHandleState(IN IMS_UINT32 nState) { m_pAosHandleMtc->SetHandleState(nState); }

    void SetSuspendedReason(IN IMS_UINT32 nReason) { m_pAosHandleMtc->SetSuspendedReason(nReason); }

    void ResetSuspendedReason(IN IMS_UINT32 nReason)
    {
        m_pAosHandleMtc->ResetSuspendedReason(nReason);
    }

    void CheckSuspended() { m_pAosHandleMtc->CheckSuspended(); }

    IMS_BOOL GetNetSrvIn() { return m_pAosHandleMtc->m_bNetSrvIn; }

    void SetNetSrvIn(IMS_BOOL bNetSrvIn) { m_pAosHandleMtc->m_bNetSrvIn = bNetSrvIn; }

    IMS_UINT32 GetNetworkType() { return m_pAosHandleMtc->m_nNetworkType; }

    void SetNetworkType(IMS_UINT32 nNetworkType) { m_pAosHandleMtc->m_nNetworkType = nNetworkType; }

    void Init() { m_pAosHandleMtc->Init(); }

    void CleanUp() { m_pAosHandleMtc->CleanUp(); }

    void ProcessBlockChanged() { m_pAosHandleMtc->ProcessBlockChanged(); }

    void ProcessCapabilitiesChanged(IN const IMSMap<IMS_UINT32, IMS_UINT32>& objNewCapabilities)
    {
        m_pAosHandleMtc->ProcessCapabilitiesChanged(objNewCapabilities);
    }

    IMS_BOOL IsEqualCapabilities(IN const IMSMap<IMS_UINT32, IMS_UINT32>& objSrcCapabilities,
            IN IMSMap<IMS_UINT32, IMS_UINT32>& objDestCapabilities)
    {
        if (objSrcCapabilities.GetSize() != objDestCapabilities.GetSize())
        {
            return IMS_FALSE;
        }

        for (IMS_UINT32 i = 0; i < objSrcCapabilities.GetSize(); i++)
        {
            IMS_UINT32 nNetworkType = objSrcCapabilities.GetKeyAt(i);

            if (objDestCapabilities.GetIndexOfKey(nNetworkType) < 0)
            {
                return IMS_FALSE;
            }

            if (objSrcCapabilities.GetValue(nNetworkType) !=
                    objDestCapabilities.GetValue(nNetworkType))
            {
                return IMS_FALSE;
            }
        }

        return IMS_TRUE;
    }

    void ProcessNetworkChanged() { m_pAosHandleMtc->ProcessNetworkChanged(); }

    void ProcessVopsStateChanged(IN IMS_UINT32 nState)
    {
        m_pAosHandleMtc->ProcessVopsStateChanged(nState);
    }

    void SetDataConnected(IN IMS_BOOL bConnected)
    {
        m_pAosHandleMtc->m_bDataConnected = bConnected;
    }

    void ReevaluateUnavailableFeature() { m_pAosHandleMtc->ReevaluateUnavailableFeature(); }

    void ProcessFeatureBlock(IN IMS_UINT32 nFeature, IN IMS_BOOL bBlocked)
    {
        m_pAosHandleMtc->ProcessFeatureBlock(nFeature, bBlocked);
    }

    IMS_UINT32 GetVoiceBlockReasonForIpcan()
    {
        return m_pAosHandleMtc->GetVoiceBlockReasonForIpcan();
    }

    IMS_UINT32 GetVideoBlockReasonForIpcan()
    {
        return m_pAosHandleMtc->GetVideoBlockReasonForIpcan();
    }

    IMS_BOOL IsHoldingBlockForMobile(IN IMS_UINT32 nBlock)
    {
        return m_pAosHandleMtc->AosHandle::IsHandleBlocked(
                m_pAosHandleMtc->m_nHoldingBlocksForMobile, nBlock);
    }

    IMS_BOOL IsHoldingBlockForWifi(IN IMS_UINT32 nBlock)
    {
        return m_pAosHandleMtc->AosHandle::IsHandleBlocked(
                m_pAosHandleMtc->m_nHoldingBlocksForWifi, nBlock);
    }

    void AddHoldingBlockForMobile(IN IMS_UINT32 nBlock)
    {
        m_pAosHandleMtc->AddBlock(nBlock, m_pAosHandleMtc->m_nHoldingBlocksForMobile);
    }

    void AddHoldingBlockForWifi(IN IMS_UINT32 nBlock)
    {
        m_pAosHandleMtc->AddBlock(nBlock, m_pAosHandleMtc->m_nHoldingBlocksForWifi);
    }

    IMS_BOOL IsCsFeatureTagRequired() { return m_pAosHandleMtc->IsCsFeatureTagRequired(); }

    IMS_BOOL IsInvalidMobileNetwork() { return m_pAosHandleMtc->IsInvalidMobileNetwork(); }
};

TEST_F(AosHandleMtcTest, Constructor)
{
    ASSERT_FALSE(GetCapabilities().GetIndexOfKey(static_cast<IMS_UINT32>(AosNetworkType::LTE)) < 0);
    ASSERT_FALSE(
            GetCapabilities().GetIndexOfKey(static_cast<IMS_UINT32>(AosNetworkType::IWLAN)) < 0);
    ASSERT_FALSE(GetCapabilities().GetIndexOfKey(static_cast<IMS_UINT32>(AosNetworkType::NR)) < 0);

    EXPECT_TRUE((GetCapabilities().GetValue(static_cast<IMS_UINT32>(AosNetworkType::LTE)) &
                        static_cast<IMS_UINT32>(AosCapability::VOICE)) > 0);
    EXPECT_TRUE((GetCapabilities().GetValue(static_cast<IMS_UINT32>(AosNetworkType::LTE)) &
                        static_cast<IMS_UINT32>(AosCapability::VIDEO)) > 0);
    EXPECT_TRUE((GetCapabilities().GetValue(static_cast<IMS_UINT32>(AosNetworkType::IWLAN)) &
                        static_cast<IMS_UINT32>(AosCapability::VOICE)) > 0);
    EXPECT_TRUE((GetCapabilities().GetValue(static_cast<IMS_UINT32>(AosNetworkType::IWLAN)) &
                        static_cast<IMS_UINT32>(AosCapability::VIDEO)) > 0);
    EXPECT_TRUE((GetCapabilities().GetValue(static_cast<IMS_UINT32>(AosNetworkType::NR)) &
                        static_cast<IMS_UINT32>(AosCapability::VOICE)) > 0);
    EXPECT_TRUE((GetCapabilities().GetValue(static_cast<IMS_UINT32>(AosNetworkType::NR)) &
                        static_cast<IMS_UINT32>(AosCapability::VIDEO)) > 0);

    EXPECT_TRUE(IsServiceFeature(ImsAosFeature::MMTEL));
    EXPECT_TRUE(IsServiceFeature(ImsAosFeature::VIDEO));
    EXPECT_TRUE(IsServiceFeature(ImsAosFeature::TEXT));
    EXPECT_TRUE(IsServiceFeature(ImsAosFeature::USSI));
    EXPECT_TRUE(IsServiceFeature(ImsAosFeature::VERSTAT));

    EXPECT_TRUE(IsBlockForMobile(AosHandle::BLOCK_VOLTE_CAPABILITY));
    EXPECT_TRUE(IsBlockForMobile(AosHandle::BLOCK_VILTE_CAPABILITY));
    EXPECT_TRUE(IsBlockForMobile(AosHandle::BLOCK_VOPS));
    EXPECT_TRUE(IsBlockForWifi(AosHandle::BLOCK_VOWIFI_CAPABILITY));
    EXPECT_TRUE(IsBlockForWifi(AosHandle::BLOCK_VIWIFI_CAPABILITY));
}

TEST_F(AosHandleMtcTest, Destructor)
{
    EXPECT_CALL(m_objMockIAosNConfiguration, RemoveListener(_)).Times(1);
}

TEST_F(AosHandleMtcTest, App_Notify_Test)
{
    m_pAosHandleMtc->App_Notify();
}

TEST_F(AosHandleMtcTest, CallTracker_StateChanged_Test1)
{
    // Test1: call type is not normal
    // Expectation: do nothing

    m_pAosHandleMtc->CallTracker_StateChanged(IAosCallTracker::TYPE_CS, CallState::RINGING);
}

TEST_F(AosHandleMtcTest, CallTracker_StateChanged_Test2)
{
    // Test2: call type is normal / call state is neither idle nor offhook
    // Expectation: do nothing

    m_pAosHandleMtc->CallTracker_StateChanged(IAosCallTracker::TYPE_NORMAL, CallState::RINGING);
}

TEST_F(AosHandleMtcTest, CallTracker_StateChanged_Test3)
{
    // Test3: call type is normal / call state is offhook
    // Expectation: do nothing

    m_pAosHandleMtc->CallTracker_StateChanged(IAosCallTracker::TYPE_NORMAL, CallState::OFFHOOK);
}

TEST_F(AosHandleMtcTest, CallTracker_StateChanged_Test4)
{
    // Test4: Holding vops for call test.
    //        call type is normal / call state is idle / vops holt for call
    //        / no unavailable feature policy for vops
    // Expectation: Add vops block to main blocks

    EXPECT_CALL(m_objMockIAosNConfiguration, IsVopsIgnoredForVolteEnabled())
            .Times(AnyNumber())
            .WillRepeatedly(Return(IMS_FALSE));

    EXPECT_CALL(m_objMockIAosNConfiguration, IsRegWithFeatureTagUnavailableSupported())
            .Times(AnyNumber())
            .WillRepeatedly(Return(IMS_FALSE));

    EXPECT_CALL(m_objMockIAosNConfiguration,
            IsGGsmaRcsTelephonyFeatureTagUsedAsAvailableVoiceCallType())
            .Times(AnyNumber())
            .WillRepeatedly(Return(IMS_FALSE));

    SetNetworkType(NW_REPORT_RADIO_LTE);

    EXPECT_CALL(m_objMockIAosNConfiguration, IsWfcImsAvailable())
            .Times(AnyNumber())
            .WillRepeatedly(Return(IMS_TRUE));

    SetHoldingVopsState(IMS_VOICE_OVER_PS_NOT_SUPPORTED);

    m_pAosHandleMtc->CallTracker_StateChanged(IAosCallTracker::TYPE_NORMAL, CallState::IDLE);

    EXPECT_FALSE(m_pAosHandleMtc->GetFeatureTagList().HasUnavailableFeature(ImsAosFeature::MMTEL));
    EXPECT_FALSE(m_pAosHandleMtc->GetFeatureTagList().HasUnavailableFeature(ImsAosFeature::VIDEO));
    EXPECT_TRUE(IsHandleBlocked(AosHandle::BLOCK_VOPS));
}

TEST_F(AosHandleMtcTest, CallTracker_StateChanged_Test5)
{
    // Test5: Holding vops for call test 2.
    //        call type is normal / call state is idle / vops holt for call
    //        / unavailable feature policy for vops
    // Expectation: No add vops block to main blocks

    EXPECT_CALL(m_objMockIAosNConfiguration, IsVopsIgnoredForVolteEnabled())
            .Times(AnyNumber())
            .WillRepeatedly(Return(IMS_FALSE));

    EXPECT_CALL(m_objMockIAosNConfiguration, IsRegWithFeatureTagUnavailableSupported())
            .Times(AnyNumber())
            .WillRepeatedly(Return(IMS_TRUE));

    EXPECT_CALL(m_objMockIAosNConfiguration,
            IsGGsmaRcsTelephonyFeatureTagUsedAsAvailableVoiceCallType())
            .Times(AnyNumber())
            .WillRepeatedly(Return(IMS_FALSE));

    SetNetworkType(NW_REPORT_RADIO_LTE);

    EXPECT_CALL(m_objMockIAosNConfiguration, IsWfcImsAvailable())
            .Times(AnyNumber())
            .WillRepeatedly(Return(IMS_TRUE));

    SetHoldingVopsState(IMS_VOICE_OVER_PS_NOT_SUPPORTED);

    m_pAosHandleMtc->CallTracker_StateChanged(IAosCallTracker::TYPE_NORMAL, CallState::IDLE);

    EXPECT_TRUE(m_pAosHandleMtc->GetFeatureTagList().HasUnavailableFeature(ImsAosFeature::MMTEL));
    EXPECT_TRUE(m_pAosHandleMtc->GetFeatureTagList().HasUnavailableFeature(ImsAosFeature::VIDEO));
    EXPECT_FALSE(IsHandleBlocked(AosHandle::BLOCK_VOPS));
}

TEST_F(AosHandleMtcTest, CallTracker_StateChanged_Test6)
{
    // Test6: vzw vowifi feature tag test. Idle -> Offhook -> Idle
    // Expectation: "cs" in idle, "cs,volte" in offhook

    AddFeature(ImsAosFeature::VIDEO);
    AddFeatureTag(FeatureTags::RCS_TELEPHONY, AosString::STR_CS_WITH_DQ);
    AddBindedFeature(ImsAosFeature::VIDEO);
    AddBindedFeatureTag(FeatureTags::RCS_TELEPHONY, AosString::STR_CS_WITH_DQ);

    SetEpdgEnabled(IMS_TRUE);

    AosFeatureTagList objExpectedFeatureTagListIdle;
    objExpectedFeatureTagListIdle.AddFeature(ImsAosFeature::VIDEO);
    objExpectedFeatureTagListIdle.AddFeatureTag(
            FeatureTags::RCS_TELEPHONY, AosString::STR_CS_WITH_DQ);

    AosFeatureTagList objExpectedFeatureTagListOffhook;
    objExpectedFeatureTagListOffhook.AddFeature(ImsAosFeature::VIDEO);
    objExpectedFeatureTagListOffhook.AddFeatureTag(FeatureTags::RCS_TELEPHONY, AosString::STR_CS);
    objExpectedFeatureTagListOffhook.AddFeatureTag(
            FeatureTags::RCS_TELEPHONY, AosString::STR_VOLTE);

    EXPECT_CALL(m_objMockIAosNConfiguration, IsVopsIgnoredForVolteEnabled())
            .Times(AnyNumber())
            .WillRepeatedly(Return(IMS_TRUE));

    EXPECT_CALL(m_objMockIAosNConfiguration,
            IsGGsmaRcsTelephonyFeatureTagUsedAsAvailableVoiceCallType())
            .Times(AnyNumber())
            .WillRepeatedly(Return(IMS_TRUE));

    EXPECT_CALL(m_objMockIAosCallTracker, IsNormalCallActive())
            .Times(2)
            .WillOnce(Return(IMS_TRUE))
            .WillOnce(Return(IMS_FALSE));

    EXPECT_CALL(m_objMockIAosNConfiguration, IsVideoOverWifiSupportedWithoutVoice())
            .Times(AnyNumber())
            .WillRepeatedly(Return(IMS_TRUE));

    EXPECT_CALL(m_objMockIAosNetTracker, GetMobileNetworkType())
            .Times(AnyNumber())
            .WillRepeatedly(Return(NW_REPORT_RADIO_GSM));

    m_pAosHandleMtc->CallTracker_StateChanged(IAosCallTracker::TYPE_NORMAL, CallState::OFFHOOK);
    EXPECT_TRUE(m_pAosHandleMtc->GetFeatureTagList().Equals(objExpectedFeatureTagListOffhook));

    m_pAosHandleMtc->CallTracker_StateChanged(IAosCallTracker::TYPE_NORMAL, CallState::IDLE);
    EXPECT_TRUE(m_pAosHandleMtc->GetFeatureTagList().Equals(objExpectedFeatureTagListIdle));
}

TEST_F(AosHandleMtcTest, NetTracker_StatusChanged_Test1)
{
    // Test1: Srv In, Network changed to LTE, Suspended with SUSPEND_NO_LTE_COVERAGE
    //        Blocked BLOCK_VILTE_CAPABILITY
    // Expectation: call ImsAos_Resumed(), unblock BLOCK_VILTE_CAPABILITY

    SetNetSrvIn(IMS_TRUE);
    SetHandleState(AosHandle::STATE_CONNECTED);
    SetSuspendedReason(AosReason::SUSPEND_NO_LTE_COVERAGE);
    SetDataConnected(IMS_TRUE);
    AddBlock(AosHandle::BLOCK_VILTE_CAPABILITY);

    EXPECT_CALL(m_objMockIAosNetTracker, IsSuspended())
            .Times(AnyNumber())
            .WillRepeatedly(Return(IMS_FALSE));

    EXPECT_CALL(m_objMockIAosNetTracker, GetNetworkType())
            .Times(AnyNumber())
            .WillRepeatedly(Return(NW_REPORT_RADIO_LTE));

    EXPECT_CALL(m_objMockIAosConnection, GetState())
            .Times(AnyNumber())
            .WillRepeatedly(Return(IAosConnection::STATE_ACTIVE));

    EXPECT_CALL(m_objMockIAosConnection, IsEpdgEnabled())
            .Times(AnyNumber())
            .WillRepeatedly(Return(IMS_FALSE));

    EXPECT_CALL(m_objMockIAosNConfiguration, IsWfcImsAvailable())
            .Times(AnyNumber())
            .WillRepeatedly(Return(IMS_FALSE));

    EXPECT_CALL(m_objMockIAosNConfiguration,
            IsGGsmaRcsTelephonyFeatureTagUsedAsAvailableVoiceCallType())
            .Times(AnyNumber())
            .WillRepeatedly(Return(IMS_FALSE));

    m_pAosHandleMtc->SetListener(static_cast<IImsAosListener*>(&m_objMockIImsAosListener));
    EXPECT_CALL(m_objMockIImsAosListener, ImsAos_Resumed()).Times(1);

    EXPECT_CALL(m_objMockIAosNConfiguration, IsRegWithFeatureTagUnavailableSupported())
            .Times(AnyNumber())
            .WillRepeatedly(Return(IMS_FALSE));

    m_pAosHandleMtc->NetTracker_StatusChanged();

    EXPECT_EQ(m_pAosHandleMtc->GetSuspendedReason(), AosReason::SUSPEND_NONE);
    EXPECT_FALSE(IsHandleBlocked(AosHandle::BLOCK_VILTE_CAPABILITY));
}

TEST_F(AosHandleMtcTest, NetTracker_StatusChanged_Test2)
{
    // Test2: Srv In, Network changed to 2G(GSM)
    // Expectation: Set SUSPEND_NO_LTE_COVERAGE, call ImsAos_Suspended()

    SetNetSrvIn(IMS_TRUE);
    SetHandleState(AosHandle::STATE_CONNECTED);
    SetDataConnected(IMS_TRUE);

    EXPECT_CALL(m_objMockIAosNetTracker, IsSuspended())
            .Times(AnyNumber())
            .WillRepeatedly(Return(IMS_FALSE));

    EXPECT_CALL(m_objMockIAosNetTracker, GetNetworkType())
            .Times(AnyNumber())
            .WillRepeatedly(Return(NW_REPORT_RADIO_GSM));

    EXPECT_CALL(m_objMockIAosConnection, GetState())
            .Times(AnyNumber())
            .WillRepeatedly(Return(IAosConnection::STATE_ACTIVE));

    EXPECT_CALL(m_objMockIAosConnection, IsEpdgEnabled())
            .Times(AnyNumber())
            .WillRepeatedly(Return(IMS_FALSE));

    m_pAosHandleMtc->SetListener(static_cast<IImsAosListener*>(&m_objMockIImsAosListener));
    EXPECT_CALL(m_objMockIImsAosListener, ImsAos_Suspended(ImsAosReason::SUSPEND_NO_RAT_COVERAGE))
            .Times(1);

    EXPECT_CALL(m_objMockIAosNConfiguration, IsRegWithFeatureTagUnavailableSupported())
            .Times(AnyNumber())
            .WillRepeatedly(Return(IMS_FALSE));

    m_pAosHandleMtc->NetTracker_StatusChanged();

    EXPECT_EQ(m_pAosHandleMtc->GetSuspendedReason(), AosReason::SUSPEND_NO_LTE_COVERAGE);
}

TEST_F(AosHandleMtcTest, NetTracker_StatusChanged_Test3)
{
    // Test3: Srv In, Network not changed (LTE)
    // Expectation: No suspended reason, no call ImsAos_Suspended() or ImsAos_Resumed()

    SetNetSrvIn(IMS_TRUE);
    SetNetworkType(NW_REPORT_RADIO_LTE);
    SetHandleState(AosHandle::STATE_CONNECTED);
    SetDataConnected(IMS_TRUE);

    EXPECT_CALL(m_objMockIAosNetTracker, IsSuspended())
            .Times(AnyNumber())
            .WillRepeatedly(Return(IMS_FALSE));

    EXPECT_CALL(m_objMockIAosNetTracker, GetNetworkType())
            .Times(AnyNumber())
            .WillRepeatedly(Return(NW_REPORT_RADIO_LTE));

    EXPECT_CALL(m_objMockIAosConnection, GetState())
            .Times(AnyNumber())
            .WillRepeatedly(Return(IAosConnection::STATE_ACTIVE));

    EXPECT_CALL(m_objMockIAosConnection, IsEpdgEnabled())
            .Times(AnyNumber())
            .WillRepeatedly(Return(IMS_FALSE));

    m_pAosHandleMtc->SetListener(static_cast<IImsAosListener*>(&m_objMockIImsAosListener));
    EXPECT_CALL(m_objMockIImsAosListener, ImsAos_Suspended(_)).Times(0);
    EXPECT_CALL(m_objMockIImsAosListener, ImsAos_Resumed()).Times(0);

    m_pAosHandleMtc->NetTracker_StatusChanged();

    EXPECT_EQ(m_pAosHandleMtc->GetSuspendedReason(), AosReason::SUSPEND_NONE);
}

TEST_F(AosHandleMtcTest, NetTracker_StatusChanged_Test4)
{
    // Test4: Srv In, Network changed to 3G(WCDMA) / Unavailable feature policy is existed
    // Expectation: Set SUSPEND_NO_LTE_COVERAGE, call ImsAos_Suspended()

    SetNetSrvIn(IMS_TRUE);
    SetHandleState(AosHandle::STATE_CONNECTED);
    SetDataConnected(IMS_TRUE);

    EXPECT_CALL(m_objMockIAosNetTracker, IsSuspended())
            .Times(AnyNumber())
            .WillRepeatedly(Return(IMS_FALSE));

    EXPECT_CALL(m_objMockIAosNetTracker, GetNetworkType())
            .Times(AnyNumber())
            .WillRepeatedly(Return(NW_REPORT_RADIO_WCDMA));

    EXPECT_CALL(m_objMockIAosNConfiguration, IsRegWithFeatureTagUnavailableSupported())
            .Times(AnyNumber())
            .WillRepeatedly(Return(IMS_TRUE));

    EXPECT_CALL(m_objMockIAosConnection, GetState())
            .Times(AnyNumber())
            .WillRepeatedly(Return(IAosConnection::STATE_ACTIVE));

    EXPECT_CALL(m_objMockIAosConnection, IsEpdgEnabled())
            .Times(AnyNumber())
            .WillRepeatedly(Return(IMS_FALSE));

    m_pAosHandleMtc->SetListener(static_cast<IImsAosListener*>(&m_objMockIImsAosListener));
    EXPECT_CALL(m_objMockIImsAosListener, ImsAos_Suspended(ImsAosReason::SUSPEND_NO_RAT_COVERAGE))
            .Times(1);

    m_pAosHandleMtc->NetTracker_StatusChanged();

    EXPECT_EQ(m_pAosHandleMtc->GetSuspendedReason(), AosReason::SUSPEND_NO_LTE_COVERAGE);
}

TEST_F(AosHandleMtcTest, NetTracker_StatusChanged_Test5)
{
    // Test5: Srv In, Network changed to 3G(WCDMA) / No unavailable feature policy
    // Expectation: Suspended reason SUSPEND_NONE (cleared when detach), call ImsAos_Suspended()

    SetNetSrvIn(IMS_TRUE);
    SetHandleState(AosHandle::STATE_CONNECTED);
    SetDataConnected(IMS_TRUE);

    EXPECT_CALL(m_objMockIAosNetTracker, IsSuspended())
            .Times(AnyNumber())
            .WillRepeatedly(Return(IMS_FALSE));

    EXPECT_CALL(m_objMockIAosNetTracker, GetNetworkType())
            .Times(AnyNumber())
            .WillRepeatedly(Return(NW_REPORT_RADIO_WCDMA));

    EXPECT_CALL(m_objMockIAosNConfiguration, IsRegWithFeatureTagUnavailableSupported())
            .Times(AnyNumber())
            .WillRepeatedly(Return(IMS_FALSE));

    EXPECT_CALL(m_objMockIAosConnection, GetState())
            .Times(AnyNumber())
            .WillRepeatedly(Return(IAosConnection::STATE_ACTIVE));

    EXPECT_CALL(m_objMockIAosConnection, IsEpdgEnabled())
            .Times(AnyNumber())
            .WillRepeatedly(Return(IMS_FALSE));

    m_pAosHandleMtc->SetListener(static_cast<IImsAosListener*>(&m_objMockIImsAosListener));
    EXPECT_CALL(m_objMockIImsAosListener, ImsAos_Suspended(ImsAosReason::SUSPEND_NO_RAT_COVERAGE))
            .Times(1);

    m_pAosHandleMtc->NetTracker_StatusChanged();

    EXPECT_TRUE(IsHandleBlocked(AosHandle::BLOCK_NETWORK));
    EXPECT_EQ(m_pAosHandleMtc->GetSuspendedReason(), AosReason::SUSPEND_NONE);
}

TEST_F(AosHandleMtcTest, NetTracker_StatusChanged_Test6)
{
    // Test6: Srv Out, Network changed to 3G(WCDMA) / No unavailable feature policy
    // Expectation: Suspended reason SUSPEND_NONE (cleared when detach), Blocked BLOCK_NETWORK

    SetNetSrvIn(IMS_TRUE);
    SetHandleState(AosHandle::STATE_CONNECTING);
    SetDataConnected(IMS_TRUE);

    EXPECT_CALL(m_objMockIAosNetTracker, IsSuspended())
            .Times(AnyNumber())
            .WillRepeatedly(Return(IMS_TRUE));

    EXPECT_CALL(m_objMockIAosNetTracker, GetNetworkType())
            .Times(AnyNumber())
            .WillRepeatedly(Return(NW_REPORT_RADIO_WCDMA));

    EXPECT_CALL(m_objMockIAosNConfiguration, IsRegWithFeatureTagUnavailableSupported())
            .Times(AnyNumber())
            .WillRepeatedly(Return(IMS_FALSE));

    EXPECT_CALL(m_objMockIAosConnection, GetState())
            .Times(AnyNumber())
            .WillRepeatedly(Return(IAosConnection::STATE_ACTIVE));

    EXPECT_CALL(m_objMockIAosConnection, IsEpdgEnabled())
            .Times(AnyNumber())
            .WillRepeatedly(Return(IMS_FALSE));

    m_pAosHandleMtc->NetTracker_StatusChanged();

    EXPECT_TRUE(IsHandleBlocked(AosHandle::BLOCK_NETWORK));
    EXPECT_EQ(m_pAosHandleMtc->GetSuspendedReason(), AosReason::SUSPEND_NONE);
}

TEST_F(AosHandleMtcTest, InitializeServiceBlock_Test)
{
    // Expectation: Set m_bBlocked depending on AosHandleMtc::IsHandleBlocked()

    AddBlock(AosHandle::BLOCK_VOPS);
    InitializeServiceBlock();
    EXPECT_TRUE(IsHandleBlockedBase());

    RemoveBlock(AosHandle::BLOCK_VOPS);
    InitializeServiceBlock();
    EXPECT_FALSE(IsHandleBlockedBase());
}

TEST_F(AosHandleMtcTest, InitializeServiceFeature_Test1)
{
    // Test1: All support
    // Expectation: MMTEL, VIDEO, TEXT, VERSTAT, USSI

    EXPECT_CALL(m_objMockIAosNConfiguration, IsRttSupported())
            .Times(AnyNumber())
            .WillRepeatedly(Return(IMS_TRUE));

    EXPECT_CALL(m_objMockIAosNConfiguration, IsVerstatForRegistrationSupported())
            .Times(AnyNumber())
            .WillRepeatedly(Return(IMS_TRUE));

    EXPECT_CALL(m_objMockIAosNConfiguration, GetUssdMethod())
            .Times(AnyNumber())
            .WillRepeatedly(Return(CarrierConfig::USSD_OVER_IMS_PREFERRED));

    InitializeServiceFeature();

    EXPECT_TRUE(m_pAosHandleMtc->GetFeatureTagList().HasFeature(ImsAosFeature::MMTEL));
    EXPECT_TRUE(m_pAosHandleMtc->GetFeatureTagList().HasFeature(ImsAosFeature::VIDEO));
    EXPECT_TRUE(m_pAosHandleMtc->GetFeatureTagList().HasFeature(ImsAosFeature::TEXT));
    EXPECT_TRUE(m_pAosHandleMtc->GetFeatureTagList().HasFeature(ImsAosFeature::VERSTAT));
    EXPECT_TRUE(m_pAosHandleMtc->GetFeatureTagList().HasFeature(ImsAosFeature::USSI));
}

TEST_F(AosHandleMtcTest, InitializeServiceFeature_Test2)
{
    // Test2: No support
    // Expectation: no features

    AddBlock(AosHandle::BLOCK_VOLTE_CAPABILITY);
    AddBlock(AosHandle::BLOCK_VILTE_CAPABILITY);

    EXPECT_CALL(m_objMockIAosNConfiguration, IsRttSupported())
            .Times(AnyNumber())
            .WillRepeatedly(Return(IMS_FALSE));

    EXPECT_CALL(m_objMockIAosNConfiguration, IsVerstatForRegistrationSupported())
            .Times(AnyNumber())
            .WillRepeatedly(Return(IMS_FALSE));

    EXPECT_CALL(m_objMockIAosNConfiguration, GetUssdMethod())
            .Times(AnyNumber())
            .WillRepeatedly(Return(CarrierConfig::USSD_OVER_CS_ONLY));

    InitializeServiceFeature();

    EXPECT_FALSE(m_pAosHandleMtc->GetFeatureTagList().HasFeature(ImsAosFeature::MMTEL));
    EXPECT_FALSE(m_pAosHandleMtc->GetFeatureTagList().HasFeature(ImsAosFeature::VIDEO));
    EXPECT_FALSE(m_pAosHandleMtc->GetFeatureTagList().HasFeature(ImsAosFeature::TEXT));
    EXPECT_FALSE(m_pAosHandleMtc->GetFeatureTagList().HasFeature(ImsAosFeature::VERSTAT));
    EXPECT_FALSE(m_pAosHandleMtc->GetFeatureTagList().HasFeature(ImsAosFeature::USSI));
}

TEST_F(AosHandleMtcTest, InitializeFeatureTags_Test)
{
    // Expectation: Include +cdmaless, "cs,volte" feature tags

    AddFeature(ImsAosFeature::MMTEL);

    AosFeatureTagList objExpectedFeatureTagList;
    objExpectedFeatureTagList.AddFeature(ImsAosFeature::MMTEL);
    objExpectedFeatureTagList.AddFeatureTag("+cdmaless");
    objExpectedFeatureTagList.AddFeatureTag(FeatureTags::RCS_TELEPHONY, AosString::STR_CS);
    objExpectedFeatureTagList.AddFeatureTag(FeatureTags::RCS_TELEPHONY, AosString::STR_VOLTE);

    EXPECT_CALL(m_objMockIAosNConfiguration, IsCdmalessFeatureTagRequired())
            .Times(1)
            .WillOnce(Return(IMS_TRUE));

    EXPECT_CALL(m_objMockIAosNConfiguration,
            IsGGsmaRcsTelephonyFeatureTagUsedAsAvailableVoiceCallType())
            .Times(1)
            .WillOnce(Return(IMS_TRUE));

    EXPECT_CALL(m_objMockIAosCallTracker, IsNormalCallActive())
            .Times(1)
            .WillOnce(Return(IMS_FALSE));

    InitializeFeatureTags();

    EXPECT_TRUE(m_pAosHandleMtc->GetFeatureTagList().Equals(objExpectedFeatureTagList));
}

TEST_F(AosHandleMtcTest, UpdateGGsmaRcsTelephonyFeatureTag_Test1)
{
    // Test1: vzw vowifi "cs" feature tag for call test. Idle -> Offhook -> Idle
    // Expectation: "cs" in idle, "cs,volte" in offhook

    AddFeature(ImsAosFeature::VIDEO);
    AddFeatureTag(FeatureTags::RCS_TELEPHONY, AosString::STR_CS_WITH_DQ);
    AddBindedFeature(ImsAosFeature::VIDEO);
    AddBindedFeatureTag(FeatureTags::RCS_TELEPHONY, AosString::STR_CS_WITH_DQ);

    SetEpdgEnabled(IMS_TRUE);

    AosFeatureTagList objExpectedFeatureTagListIdle;
    objExpectedFeatureTagListIdle.AddFeature(ImsAosFeature::VIDEO);
    objExpectedFeatureTagListIdle.AddFeatureTag(
            FeatureTags::RCS_TELEPHONY, AosString::STR_CS_WITH_DQ);

    AosFeatureTagList objExpectedFeatureTagListOffhook;
    objExpectedFeatureTagListOffhook.AddFeature(ImsAosFeature::VIDEO);
    objExpectedFeatureTagListOffhook.AddFeatureTag(FeatureTags::RCS_TELEPHONY, AosString::STR_CS);
    objExpectedFeatureTagListOffhook.AddFeatureTag(
            FeatureTags::RCS_TELEPHONY, AosString::STR_VOLTE);

    EXPECT_CALL(m_objMockIAosNConfiguration,
            IsGGsmaRcsTelephonyFeatureTagUsedAsAvailableVoiceCallType())
            .Times(AnyNumber())
            .WillRepeatedly(Return(IMS_TRUE));

    EXPECT_CALL(m_objMockIAosCallTracker, IsNormalCallActive())
            .Times(2)
            .WillOnce(Return(IMS_TRUE))
            .WillOnce(Return(IMS_FALSE));

    EXPECT_CALL(m_objMockIAosNConfiguration, IsVideoOverWifiSupportedWithoutVoice())
            .Times(AnyNumber())
            .WillRepeatedly(Return(IMS_TRUE));

    EXPECT_CALL(m_objMockIAosNetTracker, GetMobileNetworkType())
            .Times(AnyNumber())
            .WillRepeatedly(Return(NW_REPORT_RADIO_GSM));

    UpdateGGsmaRcsTelephonyFeatureTag();
    EXPECT_TRUE(m_pAosHandleMtc->GetFeatureTagList().Equals(objExpectedFeatureTagListOffhook));

    UpdateGGsmaRcsTelephonyFeatureTag();
    EXPECT_TRUE(m_pAosHandleMtc->GetFeatureTagList().Equals(objExpectedFeatureTagListIdle));
}

TEST_F(AosHandleMtcTest, UpdateGGsmaRcsTelephonyFeatureTag_Test2)
{
    // Test2: VZW VoWiFi IMS Registration Table Test - Wifi Only.
    // Expectation: "cs,volte" if mmtel / "cs" if no mmtel but video / none if no mmtel and video

    AddFeature(ImsAosFeature::MMTEL);
    AddFeature(ImsAosFeature::VIDEO);
    AddFeatureTag(FeatureTags::RCS_TELEPHONY, AosString::STR_CS);
    AddFeatureTag(FeatureTags::RCS_TELEPHONY, AosString::STR_VOLTE);

    SetEpdgEnabled(IMS_TRUE);

    AosFeatureTagList objExpectedFeatureTagListMmtel;
    objExpectedFeatureTagListMmtel.AddFeature(ImsAosFeature::MMTEL);
    objExpectedFeatureTagListMmtel.AddFeature(ImsAosFeature::VIDEO);
    objExpectedFeatureTagListMmtel.AddFeatureTag(FeatureTags::RCS_TELEPHONY, AosString::STR_CS);
    objExpectedFeatureTagListMmtel.AddFeatureTag(FeatureTags::RCS_TELEPHONY, AosString::STR_VOLTE);

    AosFeatureTagList objExpectedFeatureTagListVideo;
    objExpectedFeatureTagListVideo.AddFeature(ImsAosFeature::VIDEO);
    objExpectedFeatureTagListVideo.AddFeatureTag(
            FeatureTags::RCS_TELEPHONY, AosString::STR_CS_WITH_DQ);

    AosFeatureTagList objExpectedFeatureTagListNoFeature;
    objExpectedFeatureTagListNoFeature.Clear();

    EXPECT_CALL(m_objMockIAosNConfiguration,
            IsGGsmaRcsTelephonyFeatureTagUsedAsAvailableVoiceCallType())
            .Times(AnyNumber())
            .WillRepeatedly(Return(IMS_TRUE));

    EXPECT_CALL(m_objMockIAosCallTracker, IsNormalCallActive())
            .Times(AnyNumber())
            .WillRepeatedly(Return(IMS_FALSE));

    EXPECT_CALL(m_objMockIAosNConfiguration, IsVideoOverWifiSupportedWithoutVoice())
            .Times(AnyNumber())
            .WillRepeatedly(Return(IMS_TRUE));

    EXPECT_CALL(m_objMockIAosNetTracker, GetMobileNetworkType())
            .Times(AnyNumber())
            .WillRepeatedly(Return(NW_REPORT_RADIO_INVALID));

    // Video
    RemoveFeature(ImsAosFeature::MMTEL);
    UpdateGGsmaRcsTelephonyFeatureTag();
    EXPECT_TRUE(m_pAosHandleMtc->GetFeatureTagList().Equals(objExpectedFeatureTagListVideo));

    // Mmtel
    AddFeature(ImsAosFeature::MMTEL);
    UpdateGGsmaRcsTelephonyFeatureTag();
    EXPECT_TRUE(m_pAosHandleMtc->GetFeatureTagList().Equals(objExpectedFeatureTagListMmtel));

    // No mmtel and video
    RemoveFeature(ImsAosFeature::MMTEL);
    RemoveFeature(ImsAosFeature::VIDEO);
    UpdateGGsmaRcsTelephonyFeatureTag();
    EXPECT_TRUE(m_pAosHandleMtc->GetFeatureTagList().Equals(objExpectedFeatureTagListNoFeature));
}

TEST_F(AosHandleMtcTest, UpdateGGsmaRcsTelephonyFeatureTag_Test3)
{
    // Test3: VZW VoWiFi IMS Registration Table Test - CS Roam + Wifi
    // Expectation: "cs,volte" if mmtel / "cs" if no mmtel but video / none if no mmtel and video

    AddFeature(ImsAosFeature::MMTEL);
    AddFeature(ImsAosFeature::VIDEO);
    AddFeatureTag(FeatureTags::RCS_TELEPHONY, AosString::STR_CS);
    AddFeatureTag(FeatureTags::RCS_TELEPHONY, AosString::STR_VOLTE);

    SetEpdgEnabled(IMS_TRUE);

    AosFeatureTagList objExpectedFeatureTagListMmtel;
    objExpectedFeatureTagListMmtel.AddFeature(ImsAosFeature::MMTEL);
    objExpectedFeatureTagListMmtel.AddFeature(ImsAosFeature::VIDEO);
    objExpectedFeatureTagListMmtel.AddFeatureTag(FeatureTags::RCS_TELEPHONY, AosString::STR_CS);
    objExpectedFeatureTagListMmtel.AddFeatureTag(FeatureTags::RCS_TELEPHONY, AosString::STR_VOLTE);

    AosFeatureTagList objExpectedFeatureTagListVideo;
    objExpectedFeatureTagListVideo.AddFeature(ImsAosFeature::VIDEO);
    objExpectedFeatureTagListVideo.AddFeatureTag(
            FeatureTags::RCS_TELEPHONY, AosString::STR_CS_WITH_DQ);

    AosFeatureTagList objExpectedFeatureTagListNoFeature;
    objExpectedFeatureTagListNoFeature.Clear();

    EXPECT_CALL(m_objMockIAosNConfiguration,
            IsGGsmaRcsTelephonyFeatureTagUsedAsAvailableVoiceCallType())
            .Times(AnyNumber())
            .WillRepeatedly(Return(IMS_TRUE));

    EXPECT_CALL(m_objMockIAosCallTracker, IsNormalCallActive())
            .Times(AnyNumber())
            .WillRepeatedly(Return(IMS_FALSE));

    EXPECT_CALL(m_objMockIAosNConfiguration, IsVideoOverWifiSupportedWithoutVoice())
            .Times(AnyNumber())
            .WillRepeatedly(Return(IMS_TRUE));

    EXPECT_CALL(m_objMockIAosNetTracker, GetMobileNetworkType())
            .Times(AnyNumber())
            .WillRepeatedly(Return(NW_REPORT_RADIO_GSM));

    // Video
    RemoveFeature(ImsAosFeature::MMTEL);
    UpdateGGsmaRcsTelephonyFeatureTag();
    EXPECT_TRUE(m_pAosHandleMtc->GetFeatureTagList().Equals(objExpectedFeatureTagListVideo));

    // Mmtel
    AddFeature(ImsAosFeature::MMTEL);
    UpdateGGsmaRcsTelephonyFeatureTag();
    EXPECT_TRUE(m_pAosHandleMtc->GetFeatureTagList().Equals(objExpectedFeatureTagListMmtel));

    // No mmtel and video
    RemoveFeature(ImsAosFeature::MMTEL);
    RemoveFeature(ImsAosFeature::VIDEO);
    UpdateGGsmaRcsTelephonyFeatureTag();
    EXPECT_TRUE(m_pAosHandleMtc->GetFeatureTagList().Equals(objExpectedFeatureTagListNoFeature));
}

TEST_F(AosHandleMtcTest, UpdateGGsmaRcsTelephonyFeatureTag_Test4)
{
    // Test4: VZW VoWiFi IMS Registration Table Test - Changing to Wifi Only.
    // Expectation: "cs,volte" if mmtel / "cs" if no mmtel but video / none if no mmtel and video

    AddFeature(ImsAosFeature::MMTEL);
    AddFeature(ImsAosFeature::VIDEO);
    AddFeatureTag(FeatureTags::RCS_TELEPHONY, AosString::STR_CS);
    AddFeatureTag(FeatureTags::RCS_TELEPHONY, AosString::STR_VOLTE);

    SetEpdgEnabled(IMS_TRUE);

    AosFeatureTagList objExpectedFeatureTagListMmtel;
    objExpectedFeatureTagListMmtel.AddFeature(ImsAosFeature::MMTEL);
    objExpectedFeatureTagListMmtel.AddFeature(ImsAosFeature::VIDEO);
    objExpectedFeatureTagListMmtel.AddFeatureTag(FeatureTags::RCS_TELEPHONY, AosString::STR_CS);
    objExpectedFeatureTagListMmtel.AddFeatureTag(FeatureTags::RCS_TELEPHONY, AosString::STR_VOLTE);

    AosFeatureTagList objExpectedFeatureTagListVideo;
    objExpectedFeatureTagListVideo.AddFeature(ImsAosFeature::VIDEO);
    objExpectedFeatureTagListVideo.AddFeatureTag(
            FeatureTags::RCS_TELEPHONY, AosString::STR_CS_WITH_DQ);

    AosFeatureTagList objExpectedFeatureTagListNoFeature;
    objExpectedFeatureTagListNoFeature.Clear();

    EXPECT_CALL(m_objMockIAosNConfiguration,
            IsGGsmaRcsTelephonyFeatureTagUsedAsAvailableVoiceCallType())
            .Times(AnyNumber())
            .WillRepeatedly(Return(IMS_TRUE));

    EXPECT_CALL(m_objMockIAosCallTracker, IsNormalCallActive())
            .Times(AnyNumber())
            .WillRepeatedly(Return(IMS_FALSE));

    EXPECT_CALL(m_objMockIAosNConfiguration, IsVideoOverWifiSupportedWithoutVoice())
            .Times(AnyNumber())
            .WillRepeatedly(Return(IMS_TRUE));

    EXPECT_CALL(m_objMockIAosNetTracker, GetMobileNetworkType())
            .Times(AnyNumber())
            .WillRepeatedly(Return(NW_REPORT_RADIO_LTE));

    EXPECT_CALL(m_objMockIAosNetTracker, GetMobileChangingNetworkType())
            .Times(AnyNumber())
            .WillRepeatedly(Return(NW_REPORT_RADIO_INVALID));

    // Video
    RemoveFeature(ImsAosFeature::MMTEL);
    UpdateGGsmaRcsTelephonyFeatureTag();
    EXPECT_TRUE(m_pAosHandleMtc->GetFeatureTagList().Equals(objExpectedFeatureTagListVideo));

    // Mmtel
    AddFeature(ImsAosFeature::MMTEL);
    UpdateGGsmaRcsTelephonyFeatureTag();
    EXPECT_TRUE(m_pAosHandleMtc->GetFeatureTagList().Equals(objExpectedFeatureTagListMmtel));

    // No mmtel and video
    RemoveFeature(ImsAosFeature::MMTEL);
    RemoveFeature(ImsAosFeature::VIDEO);
    UpdateGGsmaRcsTelephonyFeatureTag();
    EXPECT_TRUE(m_pAosHandleMtc->GetFeatureTagList().Equals(objExpectedFeatureTagListNoFeature));
}

TEST_F(AosHandleMtcTest, CheckSuspended_Test1)
{
    // Test1: suspended, valid network(LTE)
    // Expectation: set SUSPEND_NO_SERVICE

    EXPECT_CALL(m_objMockIAosAppContext, GetNetTracker())
            .Times(AnyNumber())
            .WillRepeatedly(Return(&m_objMockIAosNetTracker));
    EXPECT_CALL(m_objMockIAosNetTracker, IsSuspended())
            .Times(AnyNumber())
            .WillRepeatedly(Return(IMS_TRUE));
    EXPECT_CALL(m_objMockIAosNetTracker, GetNetworkType())
            .Times(AnyNumber())
            .WillRepeatedly(Return(NW_REPORT_RADIO_LTE));

    CheckSuspended();
    EXPECT_EQ(m_pAosHandleMtc->GetSuspendedReason(), AosReason::SUSPEND_NO_SERVICE);

    EXPECT_FALSE(GetNetSrvIn());
    EXPECT_EQ(GetNetworkType(), NW_REPORT_RADIO_LTE);
}

TEST_F(AosHandleMtcTest, CheckSuspended_Test2)
{
    // Test2: suspended, invalid network(GSM)
    // Expectation: set SUSPEND_NO_SERVICE

    EXPECT_CALL(m_objMockIAosAppContext, GetNetTracker())
            .Times(AnyNumber())
            .WillRepeatedly(Return(&m_objMockIAosNetTracker));
    EXPECT_CALL(m_objMockIAosNetTracker, IsSuspended())
            .Times(AnyNumber())
            .WillRepeatedly(Return(IMS_TRUE));
    EXPECT_CALL(m_objMockIAosNetTracker, GetNetworkType())
            .Times(AnyNumber())
            .WillRepeatedly(Return(NW_REPORT_RADIO_GSM));

    CheckSuspended();
    EXPECT_EQ(m_pAosHandleMtc->GetSuspendedReason(),
            (AosReason::SUSPEND_NO_SERVICE | AosReason::SUSPEND_NO_LTE_COVERAGE));

    EXPECT_FALSE(GetNetSrvIn());
    EXPECT_EQ(GetNetworkType(), NW_REPORT_RADIO_GSM);
}

TEST_F(AosHandleMtcTest, CheckSuspended_Test3)
{
    // Test3: not suspended, valid network(LTE)
    // Expectation: no suspended reason

    EXPECT_CALL(m_objMockIAosAppContext, GetNetTracker())
            .Times(AnyNumber())
            .WillRepeatedly(Return(&m_objMockIAosNetTracker));
    EXPECT_CALL(m_objMockIAosNetTracker, IsSuspended())
            .Times(AnyNumber())
            .WillRepeatedly(Return(IMS_FALSE));
    EXPECT_CALL(m_objMockIAosNetTracker, GetNetworkType())
            .Times(AnyNumber())
            .WillRepeatedly(Return(NW_REPORT_RADIO_LTE));

    CheckSuspended();
    EXPECT_EQ(m_pAosHandleMtc->GetSuspendedReason(), AosReason::NONE);

    EXPECT_TRUE(GetNetSrvIn());
    EXPECT_EQ(GetNetworkType(), NW_REPORT_RADIO_LTE);
}

TEST_F(AosHandleMtcTest, SetSuspendedReason_ResetSuspendedReason_Test)
{
    // Expectation: Only SUSPEND_NO_SERVICE and SUSPEND_NO_LTE_COVERAGE are allowed to be set/reset.

    SetSuspendedReason(AosReason::SUSPEND_NO_SERVICE);
    EXPECT_EQ(m_pAosHandleMtc->GetSuspendedReason(), AosReason::SUSPEND_NO_SERVICE);

    SetSuspendedReason(AosReason::SUSPEND_NO_LTE_COVERAGE);
    EXPECT_EQ(m_pAosHandleMtc->GetSuspendedReason(),
            (AosReason::SUSPEND_NO_SERVICE | AosReason::SUSPEND_NO_LTE_COVERAGE));

    SetSuspendedReason(AosReason::SUSPEND_CS_CALL);
    EXPECT_EQ(m_pAosHandleMtc->GetSuspendedReason(),
            (AosReason::SUSPEND_NO_SERVICE | AosReason::SUSPEND_NO_LTE_COVERAGE));

    ResetSuspendedReason(AosReason::SUSPEND_NO_SERVICE);
    EXPECT_EQ(m_pAosHandleMtc->GetSuspendedReason(), AosReason::SUSPEND_NO_LTE_COVERAGE);

    ResetSuspendedReason(AosReason::SUSPEND_CS_CALL);
    EXPECT_EQ(m_pAosHandleMtc->GetSuspendedReason(), AosReason::SUSPEND_NO_LTE_COVERAGE);

    ResetSuspendedReason(AosReason::SUSPEND_NO_LTE_COVERAGE);
    EXPECT_EQ(m_pAosHandleMtc->GetSuspendedReason(), AosReason::NONE);
}

TEST_F(AosHandleMtcTest, Init_CleanUp_Test)
{
    // Expectation: Set/Remove AosCallTracker listneners.

    // Base
    EXPECT_CALL(m_objMockIAosNetTracker, SetListener(_)).Times(1);
    EXPECT_CALL(m_objMockIAosNetTracker, RemoveListener(_)).Times(1);
    EXPECT_CALL(m_objMockIAosNConfiguration, IsCdmalessFeatureTagRequired())
            .Times(1)
            .WillRepeatedly(Return(IMS_TRUE));

    // Mtc
    EXPECT_CALL(m_objMockIAosCallTracker, SetListener(_)).Times(1);
    EXPECT_CALL(m_objMockIAosCallTracker, RemoveListener(_)).Times(1);

    EXPECT_CALL(m_objMockIAosNConfiguration, IsRttSupported())
            .Times(AnyNumber())
            .WillRepeatedly(Return(IMS_TRUE));

    EXPECT_CALL(m_objMockIAosNConfiguration, IsVerstatForRegistrationSupported())
            .Times(AnyNumber())
            .WillRepeatedly(Return(IMS_FALSE));

    EXPECT_CALL(m_objMockIAosNConfiguration, GetUssdMethod())
            .Times(AnyNumber())
            .WillRepeatedly(Return(CarrierConfig::USSD_OVER_CS_ONLY));

    EXPECT_CALL(m_objMockIAosNConfiguration,
            IsGGsmaRcsTelephonyFeatureTagUsedAsAvailableVoiceCallType())
            .Times(AnyNumber())
            .WillRepeatedly(Return(IMS_FALSE));

    Init();
    CleanUp();
}

TEST_F(AosHandleMtcTest, IsHandleBlocked_Test1)
{
    // Test1: Epdg enabled, BLOCK_VOWIFI_CAPABILITY, ViWiFi without Voice NOT supported
    // Expectation: return true

    SetEpdgEnabled(IMS_TRUE);
    AddBlock(AosHandle::BLOCK_VOWIFI_CAPABILITY);

    EXPECT_CALL(m_objMockIAosNConfiguration, IsVideoOverWifiSupportedWithoutVoice())
            .Times(AnyNumber())
            .WillRepeatedly(Return(IMS_FALSE));

    EXPECT_TRUE(IsHandleBlocked());
}

TEST_F(AosHandleMtcTest, IsHandleBlocked_Test2)
{
    // Test2: Epdg enabled, BLOCK_VOWIFI_CAPABILITY, BLOCK_VIWIFI_CAPABILITY
    //        ViWiFi without Voice supported
    // Expectation: return true

    SetEpdgEnabled(IMS_TRUE);
    AddBlock(AosHandle::BLOCK_VOWIFI_CAPABILITY);
    AddBlock(AosHandle::BLOCK_VIWIFI_CAPABILITY);

    EXPECT_CALL(m_objMockIAosNConfiguration, IsVideoOverWifiSupportedWithoutVoice())
            .Times(AnyNumber())
            .WillRepeatedly(Return(IMS_TRUE));

    EXPECT_TRUE(IsHandleBlocked());
}

TEST_F(AosHandleMtcTest, IsHandleBlocked_Test3)
{
    // Test3: Epdg enabled, BLOCK_VOWIFI_CAPABILITY
    //        ViWiFi without Voice supported, valid network
    // Expectation: return true

    SetEpdgEnabled(IMS_TRUE);
    AddBlock(AosHandle::BLOCK_VOWIFI_CAPABILITY);

    EXPECT_CALL(m_objMockIAosNConfiguration, IsVideoOverWifiSupportedWithoutVoice())
            .Times(AnyNumber())
            .WillRepeatedly(Return(IMS_TRUE));

    EXPECT_CALL(m_objMockIAosNetTracker, GetMobileNetworkType())
            .Times(AnyNumber())
            .WillRepeatedly(Return(NW_REPORT_RADIO_LTE));

    EXPECT_CALL(m_objMockIAosNetTracker, GetMobileChangingNetworkType())
            .Times(AnyNumber())
            .WillRepeatedly(Return(NW_REPORT_RADIO_LTE));

    EXPECT_TRUE(IsHandleBlocked());
}

TEST_F(AosHandleMtcTest, IsHandleBlocked_Test4)
{
    // Test4: Epdg enabled, BLOCK_VOWIFI_CAPABILITY
    //        ViWiFi without Voice supported, invalid network
    // Expectation: return true

    SetEpdgEnabled(IMS_TRUE);
    AddBlock(AosHandle::BLOCK_VOWIFI_CAPABILITY);

    EXPECT_CALL(m_objMockIAosNConfiguration, IsVideoOverWifiSupportedWithoutVoice())
            .Times(AnyNumber())
            .WillRepeatedly(Return(IMS_TRUE));

    EXPECT_CALL(m_objMockIAosNetTracker, GetMobileNetworkType())
            .Times(AnyNumber())
            .WillRepeatedly(Return(NW_REPORT_RADIO_GSM));

    EXPECT_FALSE(IsHandleBlocked());
}

TEST_F(AosHandleMtcTest, IsHandleBlocked_Test5)
{
    // Test5: Epdg not enabled
    // Expectation: return true if BLOCK_VOPS or BLOCK_VOLTE_CAPABILITY or BLOCK_NETWORK or BLOCK_3G
    //              else return false

    AddBlock(AosHandle::BLOCK_VOPS);
    AddBlock(AosHandle::BLOCK_VOLTE_CAPABILITY);
    AddBlock(AosHandle::BLOCK_NETWORK);
    AddBlock(AosHandle::BLOCK_3G);
    EXPECT_TRUE(IsHandleBlocked());

    RemoveBlock(AosHandle::BLOCK_VOPS);
    EXPECT_TRUE(IsHandleBlocked());

    RemoveBlock(AosHandle::BLOCK_VOLTE_CAPABILITY);
    EXPECT_TRUE(IsHandleBlocked());

    RemoveBlock(AosHandle::BLOCK_NETWORK);
    EXPECT_TRUE(IsHandleBlocked());

    RemoveBlock(AosHandle::BLOCK_3G);
    EXPECT_FALSE(IsHandleBlocked());
}

TEST_F(AosHandleMtcTest, IsHandleBlocked_Test6)
{
    // Test6: Epdg enabled, BLOCK_VOWIFI_CAPABILITY
    //        ViWiFi without Voice supported, Changing to invalid network
    // Expectation: return true

    SetEpdgEnabled(IMS_TRUE);
    AddBlock(AosHandle::BLOCK_VOWIFI_CAPABILITY);

    EXPECT_CALL(m_objMockIAosNConfiguration, IsVideoOverWifiSupportedWithoutVoice())
            .Times(AnyNumber())
            .WillRepeatedly(Return(IMS_TRUE));

    EXPECT_CALL(m_objMockIAosNetTracker, GetMobileNetworkType())
            .Times(AnyNumber())
            .WillRepeatedly(Return(NW_REPORT_RADIO_LTE));

    EXPECT_CALL(m_objMockIAosNetTracker, GetMobileChangingNetworkType())
            .Times(AnyNumber())
            .WillRepeatedly(Return(NW_REPORT_RADIO_INVALID));

    EXPECT_FALSE(IsHandleBlocked());
}

TEST_F(AosHandleMtcTest, ProcessFeatureBlock_Test)
{
    EXPECT_CALL(m_objMockIAosNConfiguration,
            IsGGsmaRcsTelephonyFeatureTagUsedAsAvailableVoiceCallType())
            .Times(AnyNumber())
            .WillRepeatedly(Return(IMS_TRUE));

    EXPECT_CALL(m_objMockIAosNConfiguration, IsVideoOverWifiSupportedWithoutVoice())
            .Times(AnyNumber())
            .WillRepeatedly(Return(IMS_FALSE));

    AosFeatureTagList objExpectedFeatureTagListNoMmtel;
    AosFeatureTagList objExpectedFeatureTagListMmtel;
    objExpectedFeatureTagListMmtel.AddFeature(ImsAosFeature::MMTEL);
    objExpectedFeatureTagListMmtel.AddFeatureTag(FeatureTags::RCS_TELEPHONY, AosString::STR_CS);
    objExpectedFeatureTagListMmtel.AddFeatureTag(FeatureTags::RCS_TELEPHONY, AosString::STR_VOLTE);

    EXPECT_EQ(m_pAosHandleMtc->GetFeatureTagList().GetFeatures(), ImsAosFeature::NONE);
    EXPECT_TRUE(m_pAosHandleMtc->GetFeatureTagList().Equals(objExpectedFeatureTagListNoMmtel));

    ProcessFeatureBlock(ImsAosFeature::MMTEL, IMS_FALSE);
    EXPECT_TRUE(m_pAosHandleMtc->GetFeatureTagList().Equals(objExpectedFeatureTagListMmtel));

    ProcessFeatureBlock(ImsAosFeature::MMTEL, IMS_TRUE);
    EXPECT_EQ(m_pAosHandleMtc->GetFeatureTagList().GetFeatures(), ImsAosFeature::NONE);
    EXPECT_TRUE(m_pAosHandleMtc->GetFeatureTagList().Equals(objExpectedFeatureTagListNoMmtel));
}

TEST_F(AosHandleMtcTest, ProcessBlockChanged_Test1)
{
    // Test1: sms over ims is available without voice capability
    // Expectation: do nothing

    EXPECT_CALL(m_objMockIAosNConfiguration, IsSmsOverImsAvailableWithoutVoiceCapability())
            .Times(AnyNumber())
            .WillRepeatedly(Return(IMS_TRUE));

    EXPECT_CALL(m_objMockIAosHandle, Handle_Notify(ImsAosService::MTC, IsBlocked())).Times(0);

    ProcessBlockChanged();
}

TEST_F(AosHandleMtcTest, ProcessBlockChanged_Test2)
{
    // Test2: sms over ims is NOT available without voice capability. HandleMts is null.
    // Expectation: do nothing

    EXPECT_CALL(m_objMockIAosNConfiguration, IsSmsOverImsAvailableWithoutVoiceCapability())
            .Times(AnyNumber())
            .WillRepeatedly(Return(IMS_FALSE));

    EXPECT_CALL(m_objMockIAosAppContext, GetHandle(ImsAosService::MTS))
            .Times(1)
            .WillOnce(Return(static_cast<IAosHandle*>(IMS_NULL)));

    EXPECT_CALL(m_objMockIAosHandle, Handle_Notify(ImsAosService::MTC, IsBlocked())).Times(0);

    ProcessBlockChanged();
}

TEST_F(AosHandleMtcTest, ProcessBlockChanged_Test3)
{
    // Test3: sms over ims is NOT available without voice capability. HandleMts is existed.
    // Expectation: call Handle_Notify

    EXPECT_CALL(m_objMockIAosNConfiguration, IsSmsOverImsAvailableWithoutVoiceCapability())
            .Times(AnyNumber())
            .WillRepeatedly(Return(IMS_FALSE));

    EXPECT_CALL(m_objMockIAosAppContext, GetHandle(ImsAosService::MTS))
            .Times(1)
            .WillOnce(Return(static_cast<IAosHandle*>(&m_objMockIAosHandle)));

    EXPECT_CALL(m_objMockIAosHandle, Handle_Notify(ImsAosService::MTC, IsBlocked())).Times(1);

    ProcessBlockChanged();
}

TEST_F(AosHandleMtcTest, ProcessCapabilitiesChanged_Test1)
{
    // Test1: No capability(LTE,IWLAN,NR), Unsupported network(GSM)
    // Expectation: Set none capa for all networks. No block.

    IMSMap<IMS_UINT32, IMS_UINT32> objNewCapabilities, objExpectedCapabilities;

    objExpectedCapabilities.Add(static_cast<IMS_UINT32>(AosNetworkType::LTE),
            static_cast<IMS_UINT32>(AosCapability::NONE));
    objExpectedCapabilities.Add(static_cast<IMS_UINT32>(AosNetworkType::IWLAN),
            static_cast<IMS_UINT32>(AosCapability::NONE));
    objExpectedCapabilities.Add(static_cast<IMS_UINT32>(AosNetworkType::NR),
            static_cast<IMS_UINT32>(AosCapability::NONE));

    SetNetworkType(NW_REPORT_RADIO_GSM);

    ProcessCapabilitiesChanged(objNewCapabilities);

    EXPECT_FALSE(IsHandleBlockedBase());
    EXPECT_TRUE(IsEqualCapabilities(GetCapabilities(), objExpectedCapabilities));
    EXPECT_EQ(m_pAosHandleMtc->GetFeatureTagList().GetUnavailableFeatures(), ImsAosFeature::NONE);
}

TEST_F(AosHandleMtcTest, ProcessCapabilitiesChanged_Test2)
{
    // Test2: No capability(LTE,IWLAN,NR), Current network=LTE
    // Expectation: block BLOCK_VOLTE_CAPABILITY, BLOCK_VILTE_CAPABILITY
    //              set none capa for the network.

    IMSMap<IMS_UINT32, IMS_UINT32> objNewCapabilities, objExpectedCapabilities;

    objExpectedCapabilities.Add(static_cast<IMS_UINT32>(AosNetworkType::LTE),
            static_cast<IMS_UINT32>(AosCapability::NONE));
    objExpectedCapabilities.Add(static_cast<IMS_UINT32>(AosNetworkType::IWLAN),
            static_cast<IMS_UINT32>(AosCapability::NONE));
    objExpectedCapabilities.Add(static_cast<IMS_UINT32>(AosNetworkType::NR),
            static_cast<IMS_UINT32>(AosCapability::NONE));

    SetNetworkType(NW_REPORT_RADIO_LTE);

    EXPECT_CALL(m_objMockIAosNConfiguration, IsWfcImsAvailable())
            .Times(AnyNumber())
            .WillRepeatedly(Return(IMS_TRUE));

    EXPECT_CALL(m_objMockIAosNConfiguration,
            IsGGsmaRcsTelephonyFeatureTagUsedAsAvailableVoiceCallType())
            .Times(AnyNumber())
            .WillRepeatedly(Return(IMS_FALSE));

    ProcessCapabilitiesChanged(objNewCapabilities);

    EXPECT_TRUE(IsHandleBlocked(AosHandle::BLOCK_VOLTE_CAPABILITY));
    EXPECT_TRUE(IsHandleBlocked(AosHandle::BLOCK_VILTE_CAPABILITY));
    EXPECT_TRUE(IsEqualCapabilities(GetCapabilities(), objExpectedCapabilities));
    EXPECT_EQ(m_pAosHandleMtc->GetFeatureTagList().GetUnavailableFeatures(), ImsAosFeature::NONE);
}

TEST_F(AosHandleMtcTest, ProcessCapabilitiesChanged_Test3)
{
    // Test3: no capability(LTE,IWLAN,NR), Current network=WLAN
    // Expectation: block BLOCK_VOWIFI_CAPABILITY, BLOCK_VIWIFI_CAPABILITY,
    //              set none capa for the network.

    IMSMap<IMS_UINT32, IMS_UINT32> objNewCapabilities, objExpectedCapabilities;

    objExpectedCapabilities.Add(static_cast<IMS_UINT32>(AosNetworkType::LTE),
            static_cast<IMS_UINT32>(AosCapability::NONE));
    objExpectedCapabilities.Add(static_cast<IMS_UINT32>(AosNetworkType::IWLAN),
            static_cast<IMS_UINT32>(AosCapability::NONE));
    objExpectedCapabilities.Add(static_cast<IMS_UINT32>(AosNetworkType::NR),
            static_cast<IMS_UINT32>(AosCapability::NONE));

    SetNetworkType(NW_REPORT_RADIO_WLAN);

    SetEpdgEnabled(IMS_TRUE);
    EXPECT_CALL(m_objMockIAosNConfiguration, IsWfcImsAvailable())
            .Times(AnyNumber())
            .WillRepeatedly(Return(IMS_TRUE));

    EXPECT_CALL(m_objMockIAosNConfiguration,
            IsGGsmaRcsTelephonyFeatureTagUsedAsAvailableVoiceCallType())
            .Times(AnyNumber())
            .WillRepeatedly(Return(IMS_FALSE));

    ProcessCapabilitiesChanged(objNewCapabilities);

    EXPECT_TRUE(IsHandleBlocked(AosHandle::BLOCK_VOWIFI_CAPABILITY));
    EXPECT_TRUE(IsHandleBlocked(AosHandle::BLOCK_VIWIFI_CAPABILITY));
    EXPECT_TRUE(IsEqualCapabilities(GetCapabilities(), objExpectedCapabilities));
    EXPECT_EQ(m_pAosHandleMtc->GetFeatureTagList().GetUnavailableFeatures(), ImsAosFeature::NONE);
}

TEST_F(AosHandleMtcTest, ProcessCapabilitiesChanged_Test4)
{
    // Test4: WFC OFF on WiFi->ON on LTE (Voice capa changed for IWLAN),  Current network=LTE
    // Expectation: Remove BLOCK_VOWIFI_CAPABILITY from holding block for wifi

    IMSMap<IMS_UINT32, IMS_UINT32> objNewCapabilities;

    objNewCapabilities.Add(static_cast<IMS_UINT32>(AosNetworkType::LTE),
            static_cast<IMS_UINT32>(AosCapability::VOICE) |
                    static_cast<IMS_UINT32>(AosCapability::VIDEO));
    objNewCapabilities.Add(static_cast<IMS_UINT32>(AosNetworkType::IWLAN),
            static_cast<IMS_UINT32>(AosCapability::VOICE) |
                    static_cast<IMS_UINT32>(AosCapability::VIDEO));
    objNewCapabilities.Add(static_cast<IMS_UINT32>(AosNetworkType::NR),
            static_cast<IMS_UINT32>(AosCapability::VOICE) |
                    static_cast<IMS_UINT32>(AosCapability::VIDEO));

    SetNetworkType(NW_REPORT_RADIO_LTE);
    SetDataConnected(IMS_TRUE);
    SetEpdgEnabled(IMS_FALSE);
    AddHoldingBlockForWifi(AosHandle::BLOCK_VOWIFI_CAPABILITY);

    EXPECT_CALL(m_objMockIAosNConfiguration, IsWfcImsAvailable())
            .Times(AnyNumber())
            .WillRepeatedly(Return(IMS_TRUE));

    EXPECT_TRUE(IsHoldingBlockForWifi(AosHandle::BLOCK_VOWIFI_CAPABILITY));

    ProcessCapabilitiesChanged(objNewCapabilities);

    EXPECT_FALSE(IsHoldingBlockForWifi(AosHandle::BLOCK_VOWIFI_CAPABILITY));
    EXPECT_TRUE(IsEqualCapabilities(GetCapabilities(), objNewCapabilities));
}

TEST_F(AosHandleMtcTest, ProcessCapabilitiesChanged_Test5)
{
    // Test5: Video OFF on WiFi->ON on LTE (Video capa changed for IWLAN),  Current network=LTE
    // Expectation: Remove BLOCK_VIWIFI_CAPABILITY from holding block for wifi

    IMSMap<IMS_UINT32, IMS_UINT32> objNewCapabilities;

    objNewCapabilities.Add(static_cast<IMS_UINT32>(AosNetworkType::LTE),
            static_cast<IMS_UINT32>(AosCapability::VOICE) |
                    static_cast<IMS_UINT32>(AosCapability::VIDEO));
    objNewCapabilities.Add(static_cast<IMS_UINT32>(AosNetworkType::IWLAN),
            static_cast<IMS_UINT32>(AosCapability::VOICE) |
                    static_cast<IMS_UINT32>(AosCapability::VIDEO));
    objNewCapabilities.Add(static_cast<IMS_UINT32>(AosNetworkType::NR),
            static_cast<IMS_UINT32>(AosCapability::VOICE) |
                    static_cast<IMS_UINT32>(AosCapability::VIDEO));

    SetNetworkType(NW_REPORT_RADIO_LTE);
    SetDataConnected(IMS_TRUE);
    SetEpdgEnabled(IMS_FALSE);
    AddHoldingBlockForWifi(AosHandle::BLOCK_VIWIFI_CAPABILITY);

    EXPECT_CALL(m_objMockIAosNConfiguration, IsWfcImsAvailable())
            .Times(AnyNumber())
            .WillRepeatedly(Return(IMS_TRUE));

    EXPECT_TRUE(IsHoldingBlockForWifi(AosHandle::BLOCK_VIWIFI_CAPABILITY));

    ProcessCapabilitiesChanged(objNewCapabilities);

    EXPECT_FALSE(IsHoldingBlockForWifi(AosHandle::BLOCK_VIWIFI_CAPABILITY));
    EXPECT_TRUE(IsEqualCapabilities(GetCapabilities(), objNewCapabilities));
}

TEST_F(AosHandleMtcTest, ProcessCapabilitiesChanged_Test6)
{
    // Test6: Video OFF on LTE->ON on WiFi (Video capa changed for LTE)
    // Current mobile network=LTE, Current network=WiFi
    // Expectation: Remove BLOCK_VILTE_CAPABILITY from holding block for mobile

    IMSMap<IMS_UINT32, IMS_UINT32> objNewCapabilities;

    objNewCapabilities.Add(static_cast<IMS_UINT32>(AosNetworkType::LTE),
            static_cast<IMS_UINT32>(AosCapability::VOICE) |
                    static_cast<IMS_UINT32>(AosCapability::VIDEO));
    objNewCapabilities.Add(static_cast<IMS_UINT32>(AosNetworkType::IWLAN),
            static_cast<IMS_UINT32>(AosCapability::VOICE) |
                    static_cast<IMS_UINT32>(AosCapability::VIDEO));
    objNewCapabilities.Add(static_cast<IMS_UINT32>(AosNetworkType::NR),
            static_cast<IMS_UINT32>(AosCapability::VOICE) |
                    static_cast<IMS_UINT32>(AosCapability::VIDEO));

    SetNetworkType(NW_REPORT_RADIO_WLAN);
    SetDataConnected(IMS_TRUE);
    SetEpdgEnabled(IMS_TRUE);
    AddHoldingBlockForMobile(AosHandle::BLOCK_VILTE_CAPABILITY);

    EXPECT_CALL(m_objMockIAosNetTracker, GetMobileNetworkType())
            .Times(AnyNumber())
            .WillRepeatedly(Return(NW_REPORT_RADIO_LTE));

    EXPECT_CALL(m_objMockIAosNConfiguration, IsWfcImsAvailable())
            .Times(AnyNumber())
            .WillRepeatedly(Return(IMS_TRUE));

    EXPECT_TRUE(IsHoldingBlockForMobile(AosHandle::BLOCK_VILTE_CAPABILITY));

    ProcessCapabilitiesChanged(objNewCapabilities);

    EXPECT_FALSE(IsHoldingBlockForMobile(AosHandle::BLOCK_VILTE_CAPABILITY));
    EXPECT_TRUE(IsEqualCapabilities(GetCapabilities(), objNewCapabilities));
}

TEST_F(AosHandleMtcTest, ProcessCapabilitiesChanged_Test7)
{
    // Test7: Video OFF on NR->ON on WiFi (Video capa changed for NR)
    // Current mobile network=NR, Current network=WiFi
    // Expectation: Remove BLOCK_VILTE_CAPABILITY from holding block for mobile

    IMSMap<IMS_UINT32, IMS_UINT32> objNewCapabilities;

    objNewCapabilities.Add(static_cast<IMS_UINT32>(AosNetworkType::LTE),
            static_cast<IMS_UINT32>(AosCapability::VOICE) |
                    static_cast<IMS_UINT32>(AosCapability::VIDEO));
    objNewCapabilities.Add(static_cast<IMS_UINT32>(AosNetworkType::IWLAN),
            static_cast<IMS_UINT32>(AosCapability::VOICE) |
                    static_cast<IMS_UINT32>(AosCapability::VIDEO));
    objNewCapabilities.Add(static_cast<IMS_UINT32>(AosNetworkType::NR),
            static_cast<IMS_UINT32>(AosCapability::VOICE) |
                    static_cast<IMS_UINT32>(AosCapability::VIDEO));

    SetNetworkType(NW_REPORT_RADIO_WLAN);
    SetDataConnected(IMS_TRUE);
    SetEpdgEnabled(IMS_TRUE);
    AddHoldingBlockForMobile(AosHandle::BLOCK_VILTE_CAPABILITY);

    EXPECT_CALL(m_objMockIAosNetTracker, GetMobileNetworkType())
            .Times(AnyNumber())
            .WillRepeatedly(Return(NW_REPORT_RADIO_NR));

    EXPECT_CALL(m_objMockIAosNConfiguration, IsWfcImsAvailable())
            .Times(AnyNumber())
            .WillRepeatedly(Return(IMS_TRUE));

    EXPECT_TRUE(IsHoldingBlockForMobile(AosHandle::BLOCK_VILTE_CAPABILITY));

    ProcessCapabilitiesChanged(objNewCapabilities);

    EXPECT_FALSE(IsHoldingBlockForMobile(AosHandle::BLOCK_VILTE_CAPABILITY));
    EXPECT_TRUE(IsEqualCapabilities(GetCapabilities(), objNewCapabilities));
}

TEST_F(AosHandleMtcTest, ProcessCapabilitiesChanged_Test8)
{
    // Test7: Video OFF on cellular->ON on WiFi (Video capa changed for LTE/NR)
    // Current mobile network=Not supported type(GSM), Current network=WiFi
    // Expectation: BLOCK_VILTE_CAPABILITY is still in holding block for mobile

    IMSMap<IMS_UINT32, IMS_UINT32> objNewCapabilities;

    objNewCapabilities.Add(static_cast<IMS_UINT32>(AosNetworkType::LTE),
            static_cast<IMS_UINT32>(AosCapability::VOICE) |
                    static_cast<IMS_UINT32>(AosCapability::VIDEO));
    objNewCapabilities.Add(static_cast<IMS_UINT32>(AosNetworkType::IWLAN),
            static_cast<IMS_UINT32>(AosCapability::VOICE) |
                    static_cast<IMS_UINT32>(AosCapability::VIDEO));
    objNewCapabilities.Add(static_cast<IMS_UINT32>(AosNetworkType::NR),
            static_cast<IMS_UINT32>(AosCapability::VOICE) |
                    static_cast<IMS_UINT32>(AosCapability::VIDEO));

    SetNetworkType(NW_REPORT_RADIO_WLAN);
    SetDataConnected(IMS_TRUE);
    SetEpdgEnabled(IMS_TRUE);
    AddHoldingBlockForMobile(AosHandle::BLOCK_VILTE_CAPABILITY);

    EXPECT_CALL(m_objMockIAosNetTracker, GetMobileNetworkType())
            .Times(AnyNumber())
            .WillRepeatedly(Return(NW_REPORT_RADIO_GSM));

    EXPECT_CALL(m_objMockIAosNConfiguration, IsWfcImsAvailable())
            .Times(AnyNumber())
            .WillRepeatedly(Return(IMS_TRUE));

    EXPECT_TRUE(IsHoldingBlockForMobile(AosHandle::BLOCK_VILTE_CAPABILITY));

    ProcessCapabilitiesChanged(objNewCapabilities);

    EXPECT_TRUE(IsHoldingBlockForMobile(AosHandle::BLOCK_VILTE_CAPABILITY));
    EXPECT_TRUE(IsEqualCapabilities(GetCapabilities(), objNewCapabilities));
}

TEST_F(AosHandleMtcTest, ProcessNetworkChanged_Test1)
{
    // Test1: Capa=(LTE:voice,video / IWLAN:video / NR:none), no unavailable policy, network=LTE
    // Blocked BLOCK_NETWORK
    // Expectation: block none

    IMSMap<IMS_UINT32, IMS_UINT32> objCapabilities;
    objCapabilities.Add(static_cast<IMS_UINT32>(AosNetworkType::LTE),
            static_cast<IMS_UINT32>(AosCapability::VOICE) |
                    static_cast<IMS_UINT32>(AosCapability::VIDEO));
    objCapabilities.Add(static_cast<IMS_UINT32>(AosNetworkType::IWLAN),
            static_cast<IMS_UINT32>(AosCapability::VIDEO));
    objCapabilities.Add(static_cast<IMS_UINT32>(AosNetworkType::NR),
            static_cast<IMS_UINT32>(AosCapability::NONE));

    SetCapabilities(objCapabilities);
    SetNetworkType(NW_REPORT_RADIO_LTE);
    AddBlock(AosHandle::BLOCK_NETWORK);

    EXPECT_CALL(m_objMockIAosNConfiguration, IsWfcImsAvailable())
            .Times(AnyNumber())
            .WillRepeatedly(Return(IMS_TRUE));

    EXPECT_CALL(m_objMockIAosNConfiguration,
            IsGGsmaRcsTelephonyFeatureTagUsedAsAvailableVoiceCallType())
            .Times(AnyNumber())
            .WillRepeatedly(Return(IMS_FALSE));

    EXPECT_CALL(m_objMockIAosNConfiguration, IsRegWithFeatureTagUnavailableSupported())
            .Times(AnyNumber())
            .WillRepeatedly(Return(IMS_FALSE));

    ProcessNetworkChanged();
    EXPECT_FALSE(IsHandleBlockedBase());
}

TEST_F(AosHandleMtcTest, ProcessNetworkChanged_Test2)
{
    // Test2: Capa=(LTE:voice,video / IWLAN:video / NR:none), no unavailable policy, network=WLAN
    // Expectation: block BLOCK_VOWIFI_CAPABILITY

    IMSMap<IMS_UINT32, IMS_UINT32> objCapabilities;
    objCapabilities.Add(static_cast<IMS_UINT32>(AosNetworkType::LTE),
            static_cast<IMS_UINT32>(AosCapability::VOICE) |
                    static_cast<IMS_UINT32>(AosCapability::VIDEO));
    objCapabilities.Add(static_cast<IMS_UINT32>(AosNetworkType::IWLAN),
            static_cast<IMS_UINT32>(AosCapability::VIDEO));
    objCapabilities.Add(static_cast<IMS_UINT32>(AosNetworkType::NR),
            static_cast<IMS_UINT32>(AosCapability::NONE));

    SetCapabilities(objCapabilities);
    SetEpdgEnabled(IMS_TRUE);
    SetNetworkType(NW_REPORT_RADIO_WLAN);

    EXPECT_CALL(m_objMockIAosNetTracker, GetMobileNetworkType())
            .Times(AnyNumber())
            .WillRepeatedly(Return(NW_REPORT_RADIO_GSM));

    EXPECT_CALL(m_objMockIAosNConfiguration, IsWfcImsAvailable())
            .Times(AnyNumber())
            .WillRepeatedly(Return(IMS_TRUE));

    EXPECT_CALL(m_objMockIAosNConfiguration,
            IsGGsmaRcsTelephonyFeatureTagUsedAsAvailableVoiceCallType())
            .Times(AnyNumber())
            .WillRepeatedly(Return(IMS_FALSE));

    EXPECT_CALL(m_objMockIAosNConfiguration, IsVideoOverWifiSupportedWithoutVoice())
            .Times(AnyNumber())
            .WillRepeatedly(Return(IMS_TRUE));

    EXPECT_CALL(m_objMockIAosNConfiguration, IsRegWithFeatureTagUnavailableSupported())
            .Times(AnyNumber())
            .WillRepeatedly(Return(IMS_FALSE));
    ProcessNetworkChanged();
    EXPECT_TRUE(IsHandleBlocked(AosHandle::BLOCK_VOWIFI_CAPABILITY));
}

TEST_F(AosHandleMtcTest, ProcessNetworkChanged_Test3)
{
    // Test3: Capa=(LTE:voice,video / IWLAN:video / NR:none), no unavailable policy, network=NR
    // Expectation: block BLOCK_VOLTE_CAPABILITY, BLOCK_VILTE_CAPABILITY

    IMSMap<IMS_UINT32, IMS_UINT32> objCapabilities;
    objCapabilities.Add(static_cast<IMS_UINT32>(AosNetworkType::LTE),
            static_cast<IMS_UINT32>(AosCapability::VOICE) |
                    static_cast<IMS_UINT32>(AosCapability::VIDEO));
    objCapabilities.Add(static_cast<IMS_UINT32>(AosNetworkType::IWLAN),
            static_cast<IMS_UINT32>(AosCapability::VIDEO));
    objCapabilities.Add(static_cast<IMS_UINT32>(AosNetworkType::NR),
            static_cast<IMS_UINT32>(AosCapability::NONE));

    SetCapabilities(objCapabilities);
    SetNetworkType(NW_REPORT_RADIO_NR);

    EXPECT_CALL(m_objMockIAosNConfiguration, IsWfcImsAvailable())
            .Times(AnyNumber())
            .WillRepeatedly(Return(IMS_TRUE));

    EXPECT_CALL(m_objMockIAosNConfiguration,
            IsGGsmaRcsTelephonyFeatureTagUsedAsAvailableVoiceCallType())
            .Times(AnyNumber())
            .WillRepeatedly(Return(IMS_FALSE));

    EXPECT_CALL(m_objMockIAosNConfiguration, IsRegWithFeatureTagUnavailableSupported())
            .Times(AnyNumber())
            .WillRepeatedly(Return(IMS_FALSE));

    EXPECT_CALL(m_objMockIAosNConfiguration, IsSmsOverImsAvailableWithoutVoiceCapability())
            .Times(AnyNumber())
            .WillRepeatedly(Return(IMS_TRUE));

    ProcessNetworkChanged();
    EXPECT_TRUE(IsHandleBlocked(AosHandle::BLOCK_VOLTE_CAPABILITY));
    EXPECT_TRUE(IsHandleBlocked(AosHandle::BLOCK_VILTE_CAPABILITY));
}

TEST_F(AosHandleMtcTest, ProcessNetworkChanged_Test4)
{
    // Test4: Capa=(LTE:voice,video / IWLAN:none / NR:none), network=NR
    //        unavailable feature policy is existed.
    // Expectation: block none

    IMSMap<IMS_UINT32, IMS_UINT32> objCapabilities;
    objCapabilities.Add(static_cast<IMS_UINT32>(AosNetworkType::LTE),
            static_cast<IMS_UINT32>(AosCapability::VOICE) |
                    static_cast<IMS_UINT32>(AosCapability::VIDEO));
    objCapabilities.Add(static_cast<IMS_UINT32>(AosNetworkType::IWLAN),
            static_cast<IMS_UINT32>(AosCapability::NONE));
    objCapabilities.Add(static_cast<IMS_UINT32>(AosNetworkType::NR),
            static_cast<IMS_UINT32>(AosCapability::NONE));

    SetCapabilities(objCapabilities);
    SetNetworkType(NW_REPORT_RADIO_NR);

    EXPECT_CALL(m_objMockIAosNConfiguration, IsWfcImsAvailable())
            .Times(AnyNumber())
            .WillRepeatedly(Return(IMS_TRUE));

    EXPECT_CALL(m_objMockIAosNConfiguration,
            IsGGsmaRcsTelephonyFeatureTagUsedAsAvailableVoiceCallType())
            .Times(AnyNumber())
            .WillRepeatedly(Return(IMS_FALSE));

    EXPECT_CALL(m_objMockIAosNConfiguration, IsRegWithFeatureTagUnavailableSupported())
            .Times(AnyNumber())
            .WillRepeatedly(Return(IMS_TRUE));

    ProcessNetworkChanged();

    EXPECT_TRUE(IsHandleBlocked(AosHandle::BLOCK_VILTE_CAPABILITY));
}

TEST_F(AosHandleMtcTest, ProcessNetworkChanged_Test5)
{
    // Test5: Video mismatch check / LTE->3G->LTE->3G->LTE
    //        Video changed while on 3G
    //        unavailable feature policy is existed.
    // Expectation: Video blocked after network changed to LTE again.

    IMSMap<IMS_UINT32, IMS_UINT32> objCapabilitiesVoiceVideo;
    objCapabilitiesVoiceVideo.Add(static_cast<IMS_UINT32>(AosNetworkType::LTE),
            static_cast<IMS_UINT32>(AosCapability::VOICE) |
                    static_cast<IMS_UINT32>(AosCapability::VIDEO));

    IMSMap<IMS_UINT32, IMS_UINT32> objCapabilitiesVoice;
    objCapabilitiesVoice.Add(static_cast<IMS_UINT32>(AosNetworkType::LTE),
            static_cast<IMS_UINT32>(AosCapability::VOICE));

    EXPECT_CALL(m_objMockIAosNConfiguration, IsWfcImsAvailable())
            .Times(AnyNumber())
            .WillRepeatedly(Return(IMS_TRUE));

    EXPECT_CALL(m_objMockIAosNConfiguration,
            IsGGsmaRcsTelephonyFeatureTagUsedAsAvailableVoiceCallType())
            .Times(AnyNumber())
            .WillRepeatedly(Return(IMS_FALSE));

    EXPECT_CALL(m_objMockIAosNConfiguration, IsRegWithFeatureTagUnavailableSupported())
            .Times(AnyNumber())
            .WillRepeatedly(Return(IMS_TRUE));

    SetNetworkType(NW_REPORT_RADIO_WCDMA);
    ProcessNetworkChanged();
    EXPECT_FALSE(IsHandleBlockedBase());

    ProcessCapabilitiesChanged(objCapabilitiesVoice);
    EXPECT_FALSE(IsHandleBlocked(AosHandle::BLOCK_VILTE_CAPABILITY));

    SetNetworkType(NW_REPORT_RADIO_LTE);
    ProcessNetworkChanged();
    EXPECT_TRUE(IsHandleBlocked(AosHandle::BLOCK_VILTE_CAPABILITY));

    SetNetworkType(NW_REPORT_RADIO_WCDMA);
    ProcessNetworkChanged();
    EXPECT_TRUE(IsHandleBlocked(AosHandle::BLOCK_VILTE_CAPABILITY));

    ProcessCapabilitiesChanged(objCapabilitiesVoiceVideo);
    EXPECT_TRUE(IsHandleBlocked(AosHandle::BLOCK_VILTE_CAPABILITY));

    SetNetworkType(NW_REPORT_RADIO_LTE);
    ProcessNetworkChanged();
    EXPECT_FALSE(IsHandleBlocked(AosHandle::BLOCK_VILTE_CAPABILITY));
}

TEST_F(AosHandleMtcTest, ProcessNetworkChanged_Test6)
{
    // Test6: Capa=(LTE:voice,video / IWLAN:video / NR:none), no unavailable policy, network=2G
    // Expectation: No block network

    IMSMap<IMS_UINT32, IMS_UINT32> objCapabilities;
    objCapabilities.Add(static_cast<IMS_UINT32>(AosNetworkType::LTE),
            static_cast<IMS_UINT32>(AosCapability::VOICE) |
                    static_cast<IMS_UINT32>(AosCapability::VIDEO));
    objCapabilities.Add(static_cast<IMS_UINT32>(AosNetworkType::IWLAN),
            static_cast<IMS_UINT32>(AosCapability::VIDEO));
    objCapabilities.Add(static_cast<IMS_UINT32>(AosNetworkType::NR),
            static_cast<IMS_UINT32>(AosCapability::NONE));

    SetCapabilities(objCapabilities);
    SetNetworkType(NW_REPORT_RADIO_GSM);

    EXPECT_CALL(m_objMockIAosNConfiguration, IsWfcImsAvailable())
            .Times(AnyNumber())
            .WillRepeatedly(Return(IMS_TRUE));

    EXPECT_CALL(m_objMockIAosNConfiguration,
            IsGGsmaRcsTelephonyFeatureTagUsedAsAvailableVoiceCallType())
            .Times(AnyNumber())
            .WillRepeatedly(Return(IMS_FALSE));

    EXPECT_CALL(m_objMockIAosNConfiguration, IsRegWithFeatureTagUnavailableSupported())
            .Times(AnyNumber())
            .WillRepeatedly(Return(IMS_FALSE));

    EXPECT_CALL(m_objMockIAosNConfiguration, IsSmsOverImsAvailableWithoutVoiceCapability())
            .Times(AnyNumber())
            .WillRepeatedly(Return(IMS_TRUE));

    ProcessNetworkChanged();
    EXPECT_FALSE(IsHandleBlockedBase());
}

TEST_F(AosHandleMtcTest, ProcessNetworkChanged_Test7)
{
    // Test7: Capa=(LTE:voice,video / IWLAN:video / NR:none), unavailable policy, network=2G
    // Expectation: No block network

    IMSMap<IMS_UINT32, IMS_UINT32> objCapabilities;
    objCapabilities.Add(static_cast<IMS_UINT32>(AosNetworkType::LTE),
            static_cast<IMS_UINT32>(AosCapability::VOICE) |
                    static_cast<IMS_UINT32>(AosCapability::VIDEO));
    objCapabilities.Add(static_cast<IMS_UINT32>(AosNetworkType::IWLAN),
            static_cast<IMS_UINT32>(AosCapability::VIDEO));
    objCapabilities.Add(static_cast<IMS_UINT32>(AosNetworkType::NR),
            static_cast<IMS_UINT32>(AosCapability::NONE));

    SetCapabilities(objCapabilities);
    SetNetworkType(NW_REPORT_RADIO_GSM);

    EXPECT_CALL(m_objMockIAosNConfiguration, IsWfcImsAvailable())
            .Times(AnyNumber())
            .WillRepeatedly(Return(IMS_TRUE));

    EXPECT_CALL(m_objMockIAosNConfiguration,
            IsGGsmaRcsTelephonyFeatureTagUsedAsAvailableVoiceCallType())
            .Times(AnyNumber())
            .WillRepeatedly(Return(IMS_FALSE));

    EXPECT_CALL(m_objMockIAosNConfiguration, IsRegWithFeatureTagUnavailableSupported())
            .Times(AnyNumber())
            .WillRepeatedly(Return(IMS_TRUE));

    EXPECT_CALL(m_objMockIAosNConfiguration, IsSmsOverImsAvailableWithoutVoiceCapability())
            .Times(AnyNumber())
            .WillRepeatedly(Return(IMS_TRUE));

    ProcessNetworkChanged();
    EXPECT_FALSE(IsHandleBlockedBase());
}

TEST_F(AosHandleMtcTest, ProcessNetworkChanged_Test8)
{
    // Test8: Capa=(LTE:voice,video / IWLAN:video / NR:none), no unavailable policy, network=3G
    // Expectation: Block network on 3G but LTE

    IMSMap<IMS_UINT32, IMS_UINT32> objCapabilities;
    objCapabilities.Add(static_cast<IMS_UINT32>(AosNetworkType::LTE),
            static_cast<IMS_UINT32>(AosCapability::VOICE) |
                    static_cast<IMS_UINT32>(AosCapability::VIDEO));
    objCapabilities.Add(static_cast<IMS_UINT32>(AosNetworkType::IWLAN),
            static_cast<IMS_UINT32>(AosCapability::VIDEO));
    objCapabilities.Add(static_cast<IMS_UINT32>(AosNetworkType::NR),
            static_cast<IMS_UINT32>(AosCapability::NONE));

    SetCapabilities(objCapabilities);
    SetNetworkType(NW_REPORT_RADIO_WCDMA);

    EXPECT_CALL(m_objMockIAosNConfiguration, IsWfcImsAvailable())
            .Times(AnyNumber())
            .WillRepeatedly(Return(IMS_TRUE));

    EXPECT_CALL(m_objMockIAosNConfiguration,
            IsGGsmaRcsTelephonyFeatureTagUsedAsAvailableVoiceCallType())
            .Times(AnyNumber())
            .WillRepeatedly(Return(IMS_FALSE));

    EXPECT_CALL(m_objMockIAosNConfiguration, IsRegWithFeatureTagUnavailableSupported())
            .Times(AnyNumber())
            .WillRepeatedly(Return(IMS_FALSE));

    EXPECT_CALL(m_objMockIAosNConfiguration, IsSmsOverImsAvailableWithoutVoiceCapability())
            .Times(AnyNumber())
            .WillRepeatedly(Return(IMS_TRUE));

    ProcessNetworkChanged();
    EXPECT_TRUE(IsHandleBlocked(AosHandle::BLOCK_NETWORK));

    SetNetworkType(NW_REPORT_RADIO_LTE);
    ProcessNetworkChanged();
    EXPECT_FALSE(IsHandleBlocked(AosHandle::BLOCK_NETWORK));
}

TEST_F(AosHandleMtcTest, ProcessNetworkChanged_Test9)
{
    // Test9: Capa=(LTE:voice,video / IWLAN:video / NR:none), no unavailable policy, network=3G
    // Data not connected / epdg not enabled
    // Expectation: Hold block network on 3G but LTE

    IMSMap<IMS_UINT32, IMS_UINT32> objCapabilities;
    objCapabilities.Add(static_cast<IMS_UINT32>(AosNetworkType::LTE),
            static_cast<IMS_UINT32>(AosCapability::VOICE) |
                    static_cast<IMS_UINT32>(AosCapability::VIDEO));
    objCapabilities.Add(static_cast<IMS_UINT32>(AosNetworkType::IWLAN),
            static_cast<IMS_UINT32>(AosCapability::VIDEO));
    objCapabilities.Add(static_cast<IMS_UINT32>(AosNetworkType::NR),
            static_cast<IMS_UINT32>(AosCapability::NONE));

    SetCapabilities(objCapabilities);
    SetNetworkType(NW_REPORT_RADIO_WCDMA);
    SetDataConnected(IMS_FALSE);
    SetEpdgEnabled(IMS_FALSE);

    EXPECT_CALL(m_objMockIAosNConfiguration, IsWfcImsAvailable())
            .Times(AnyNumber())
            .WillRepeatedly(Return(IMS_TRUE));

    EXPECT_CALL(m_objMockIAosNConfiguration,
            IsGGsmaRcsTelephonyFeatureTagUsedAsAvailableVoiceCallType())
            .Times(AnyNumber())
            .WillRepeatedly(Return(IMS_FALSE));

    EXPECT_CALL(m_objMockIAosNConfiguration, IsRegWithFeatureTagUnavailableSupported())
            .Times(AnyNumber())
            .WillRepeatedly(Return(IMS_FALSE));

    EXPECT_CALL(m_objMockIAosNConfiguration, IsSmsOverImsAvailableWithoutVoiceCapability())
            .Times(AnyNumber())
            .WillRepeatedly(Return(IMS_TRUE));

    ProcessNetworkChanged();
    EXPECT_FALSE(IsHandleBlocked(AosHandle::BLOCK_NETWORK));
    EXPECT_TRUE(IsHoldingBlockForMobile(AosHandle::BLOCK_NETWORK));

    SetNetworkType(NW_REPORT_RADIO_LTE);
    ProcessNetworkChanged();
    EXPECT_FALSE(IsHandleBlocked(AosHandle::BLOCK_NETWORK));
    EXPECT_FALSE(IsHoldingBlockForMobile(AosHandle::BLOCK_NETWORK));
}

TEST_F(AosHandleMtcTest, ProcessNetworkChanged_Test10)
{
    // Test10: Capa=(LTE:voice,video / IWLAN:video / NR:none), no unavailable policy, network=WLAN
    // Epdg enabled
    // Expectation: Holding block network is maintained on WLAN

    IMSMap<IMS_UINT32, IMS_UINT32> objCapabilities;
    objCapabilities.Add(static_cast<IMS_UINT32>(AosNetworkType::LTE),
            static_cast<IMS_UINT32>(AosCapability::VOICE) |
                    static_cast<IMS_UINT32>(AosCapability::VIDEO));
    objCapabilities.Add(static_cast<IMS_UINT32>(AosNetworkType::IWLAN),
            static_cast<IMS_UINT32>(AosCapability::VIDEO));
    objCapabilities.Add(static_cast<IMS_UINT32>(AosNetworkType::NR),
            static_cast<IMS_UINT32>(AosCapability::NONE));

    SetCapabilities(objCapabilities);
    SetNetworkType(NW_REPORT_RADIO_WLAN);
    SetEpdgEnabled(IMS_TRUE);
    AddHoldingBlockForMobile(AosHandle::BLOCK_NETWORK);

    EXPECT_CALL(m_objMockIAosNConfiguration, IsWfcImsAvailable())
            .Times(AnyNumber())
            .WillRepeatedly(Return(IMS_TRUE));

    EXPECT_CALL(m_objMockIAosNConfiguration,
            IsGGsmaRcsTelephonyFeatureTagUsedAsAvailableVoiceCallType())
            .Times(AnyNumber())
            .WillRepeatedly(Return(IMS_FALSE));

    EXPECT_CALL(m_objMockIAosNConfiguration, IsRegWithFeatureTagUnavailableSupported())
            .Times(AnyNumber())
            .WillRepeatedly(Return(IMS_FALSE));

    EXPECT_CALL(m_objMockIAosNConfiguration, IsSmsOverImsAvailableWithoutVoiceCapability())
            .Times(AnyNumber())
            .WillRepeatedly(Return(IMS_TRUE));

    ProcessNetworkChanged();
    EXPECT_TRUE(IsHoldingBlockForMobile(AosHandle::BLOCK_NETWORK));
}

TEST_F(AosHandleMtcTest, ProcessVopsStateChanged_Test1)
{
    // Test1: call active
    // Expectation: set/reset m_nHoldingVopsState as the state
    //              No change m_nVopsState

    EXPECT_CALL(m_objMockIAosCallTracker, IsNormalCallActive())
            .Times(AnyNumber())
            .WillRepeatedly(Return(IMS_TRUE));

    ProcessVopsStateChanged(IMS_VOICE_OVER_PS_NOT_SUPPORTED);
    EXPECT_EQ(GetHoldingVopsState(), IMS_VOICE_OVER_PS_NOT_SUPPORTED);
    EXPECT_EQ(GetVopsState(), IMS_VOICE_OVER_PS_SUPPORTED);
    EXPECT_FALSE(IsHandleBlocked(AosHandle::BLOCK_VOPS));

    ProcessVopsStateChanged(IMS_VOICE_OVER_PS_SUPPORTED);
    EXPECT_EQ(GetHoldingVopsState(), IMS_VOICE_OVER_PS_SUPPORTED);
    EXPECT_EQ(GetVopsState(), IMS_VOICE_OVER_PS_SUPPORTED);
    EXPECT_FALSE(IsHandleBlocked(AosHandle::BLOCK_VOPS));
}

TEST_F(AosHandleMtcTest, ProcessVopsStateChanged_Test2)
{
    // Test2: call idle, no unavailalble policy, network=LTE
    // Expectation: set/reset block BLOCK_VOPS as the state
    //              set/reset m_nVopsState as the state
    //              No change m_nHoldingVopsState

    EXPECT_CALL(m_objMockIAosCallTracker, IsNormalCallActive())
            .Times(AnyNumber())
            .WillRepeatedly(Return(IMS_FALSE));

    SetNetworkType(NW_REPORT_RADIO_LTE);

    EXPECT_CALL(m_objMockIAosNConfiguration, IsRegWithFeatureTagUnavailableSupported())
            .Times(AnyNumber())
            .WillRepeatedly(Return(IMS_FALSE));

    EXPECT_CALL(m_objMockIAosNConfiguration, IsWfcImsAvailable())
            .Times(AnyNumber())
            .WillRepeatedly(Return(IMS_TRUE));

    EXPECT_CALL(m_objMockIAosNConfiguration,
            IsGGsmaRcsTelephonyFeatureTagUsedAsAvailableVoiceCallType())
            .Times(AnyNumber())
            .WillRepeatedly(Return(IMS_FALSE));

    ProcessVopsStateChanged(IMS_VOICE_OVER_PS_NOT_SUPPORTED);
    EXPECT_EQ(GetHoldingVopsState(), IMS_VOICE_OVER_PS_SUPPORTED);
    EXPECT_EQ(GetVopsState(), IMS_VOICE_OVER_PS_NOT_SUPPORTED);
    EXPECT_TRUE(IsHandleBlocked(AosHandle::BLOCK_VOPS));

    ProcessVopsStateChanged(IMS_VOICE_OVER_PS_SUPPORTED);
    EXPECT_EQ(GetHoldingVopsState(), IMS_VOICE_OVER_PS_SUPPORTED);
    EXPECT_EQ(GetVopsState(), IMS_VOICE_OVER_PS_SUPPORTED);
    EXPECT_FALSE(IsHandleBlocked(AosHandle::BLOCK_VOPS));
}

TEST_F(AosHandleMtcTest, ProcessVopsStateChanged_Test3)
{
    // Test3: call idle, unavailalble feature policy is existed
    // Expectation: no block BLOCK_VOPS

    EXPECT_CALL(m_objMockIAosCallTracker, IsNormalCallActive())
            .Times(AnyNumber())
            .WillRepeatedly(Return(IMS_FALSE));

    EXPECT_CALL(m_objMockIAosNConfiguration, IsRegWithFeatureTagUnavailableSupported())
            .Times(AnyNumber())
            .WillRepeatedly(Return(IMS_TRUE));

    EXPECT_CALL(m_objMockIAosNConfiguration, IsWfcImsAvailable())
            .Times(AnyNumber())
            .WillRepeatedly(Return(IMS_TRUE));

    EXPECT_CALL(m_objMockIAosNConfiguration,
            IsGGsmaRcsTelephonyFeatureTagUsedAsAvailableVoiceCallType())
            .Times(AnyNumber())
            .WillRepeatedly(Return(IMS_FALSE));

    ProcessVopsStateChanged(IMS_VOICE_OVER_PS_NOT_SUPPORTED);
    EXPECT_FALSE(IsHandleBlocked(AosHandle::BLOCK_VOPS));

    ProcessVopsStateChanged(IMS_VOICE_OVER_PS_SUPPORTED);
    EXPECT_FALSE(IsHandleBlocked(AosHandle::BLOCK_VOPS));
}

TEST_F(AosHandleMtcTest, ReevaluateUnavailableFeature_Test1)
{
    // Test1: Vops change / Video Capable
    // Expectation: Add/Remove mmtel, video to/from unavailable feature / No block vops
    //              Notify to AosRegistration if unavailable feature is changed

    EXPECT_CALL(m_objMockIAosNConfiguration, IsRegWithFeatureTagUnavailableSupported())
            .Times(AnyNumber())
            .WillRepeatedly(Return(IMS_TRUE));

    SetNetworkType(NW_REPORT_RADIO_LTE);

    EXPECT_CALL(m_objMockIAosNConfiguration, IsVopsIgnoredForVolteEnabled())
            .Times(AnyNumber())
            .WillRepeatedly(Return(IMS_FALSE));

    EXPECT_CALL(
            m_objMockIAosRegistration, RequestCmd(IAosRegistration::CMD_UNAVAILABLE_FEATURE_TAG, 0))
            .Times(2);

    ProcessVopsStateChanged(IMS_VOICE_OVER_PS_NOT_SUPPORTED);
    EXPECT_TRUE(m_pAosHandleMtc->GetFeatureTagList().HasUnavailableFeature(ImsAosFeature::MMTEL));
    EXPECT_TRUE(m_pAosHandleMtc->GetFeatureTagList().HasUnavailableFeature(ImsAosFeature::VIDEO));
    EXPECT_FALSE(IsHandleBlocked(AosHandle::BLOCK_VOPS));

    ProcessVopsStateChanged(IMS_VOICE_OVER_PS_SUPPORTED);
    EXPECT_FALSE(m_pAosHandleMtc->GetFeatureTagList().HasUnavailableFeature(ImsAosFeature::MMTEL));
    EXPECT_FALSE(m_pAosHandleMtc->GetFeatureTagList().HasUnavailableFeature(ImsAosFeature::VIDEO));
    EXPECT_FALSE(IsHandleBlocked(AosHandle::BLOCK_VOPS));
}

TEST_F(AosHandleMtcTest, ReevaluateUnavailableFeature_Test2)
{
    // Test2: Vops change / Video NOT capable
    // Expectation: Add/Remove mmtel to/from unavailable feature / No block vops
    //              Notify to AosRegistration if unavailable feature is changed

    EXPECT_CALL(m_objMockIAosNConfiguration, IsRegWithFeatureTagUnavailableSupported())
            .Times(AnyNumber())
            .WillRepeatedly(Return(IMS_TRUE));

    SetNetworkType(NW_REPORT_RADIO_LTE);

    IMSMap<IMS_UINT32, IMS_UINT32> objCapabilitiesVoice;
    objCapabilitiesVoice.Add(static_cast<IMS_UINT32>(AosNetworkType::LTE),
            static_cast<IMS_UINT32>(AosCapability::VOICE));

    EXPECT_CALL(m_objMockIAosNConfiguration, IsVopsIgnoredForVolteEnabled())
            .Times(AnyNumber())
            .WillRepeatedly(Return(IMS_FALSE));

    EXPECT_CALL(
            m_objMockIAosRegistration, RequestCmd(IAosRegistration::CMD_UNAVAILABLE_FEATURE_TAG, 0))
            .Times(2);

    ProcessCapabilitiesChanged(objCapabilitiesVoice);

    ProcessVopsStateChanged(IMS_VOICE_OVER_PS_NOT_SUPPORTED);
    EXPECT_TRUE(m_pAosHandleMtc->GetFeatureTagList().HasUnavailableFeature(ImsAosFeature::MMTEL));
    EXPECT_FALSE(m_pAosHandleMtc->GetFeatureTagList().HasUnavailableFeature(ImsAosFeature::VIDEO));
    EXPECT_FALSE(IsHandleBlocked(AosHandle::BLOCK_VOPS));

    ProcessVopsStateChanged(IMS_VOICE_OVER_PS_SUPPORTED);
    EXPECT_FALSE(m_pAosHandleMtc->GetFeatureTagList().HasUnavailableFeature(ImsAosFeature::MMTEL));
    EXPECT_FALSE(m_pAosHandleMtc->GetFeatureTagList().HasUnavailableFeature(ImsAosFeature::VIDEO));
    EXPECT_FALSE(IsHandleBlocked(AosHandle::BLOCK_VOPS));
}

TEST_F(AosHandleMtcTest, ReevaluateUnavailableFeature_Test3)
{
    // Test3: Network change
    // Expectation: Unavailable feature on 3G but LTE / No block network on 3G
    //              Notify to AosRegistration if unavailable feature is changed

    EXPECT_CALL(m_objMockIAosNConfiguration, IsRegWithFeatureTagUnavailableSupported())
            .Times(AnyNumber())
            .WillRepeatedly(Return(IMS_TRUE));

    SetNetworkType(NW_REPORT_RADIO_HSPA);

    EXPECT_CALL(
            m_objMockIAosRegistration, RequestCmd(IAosRegistration::CMD_UNAVAILABLE_FEATURE_TAG, 0))
            .Times(2);

    ProcessNetworkChanged();
    EXPECT_TRUE(m_pAosHandleMtc->GetFeatureTagList().HasUnavailableFeature(ImsAosFeature::MMTEL));
    EXPECT_TRUE(m_pAosHandleMtc->GetFeatureTagList().HasUnavailableFeature(ImsAosFeature::VIDEO));
    EXPECT_FALSE(IsHandleBlocked(AosHandle::BLOCK_NETWORK));

    SetNetworkType(NW_REPORT_RADIO_LTE);

    ProcessNetworkChanged();
    EXPECT_FALSE(m_pAosHandleMtc->GetFeatureTagList().HasUnavailableFeature(ImsAosFeature::MMTEL));
    EXPECT_FALSE(m_pAosHandleMtc->GetFeatureTagList().HasUnavailableFeature(ImsAosFeature::VIDEO));
    EXPECT_FALSE(IsHandleBlocked(AosHandle::BLOCK_NETWORK));
}

TEST_F(AosHandleMtcTest, GetVoiceBlockReasonForIpcan_Test)
{
    SetNetworkType(NW_REPORT_RADIO_LTE);
    EXPECT_EQ(GetVoiceBlockReasonForIpcan(), AosHandle::BLOCK_VOLTE_CAPABILITY);

    SetNetworkType(NW_REPORT_RADIO_WLAN);
    EXPECT_EQ(GetVoiceBlockReasonForIpcan(), AosHandle::BLOCK_VOWIFI_CAPABILITY);
}

TEST_F(AosHandleMtcTest, GetVideoBlockReasonForIpcan_Test)
{
    SetNetworkType(NW_REPORT_RADIO_LTE);
    EXPECT_EQ(GetVideoBlockReasonForIpcan(), AosHandle::BLOCK_VILTE_CAPABILITY);

    SetNetworkType(NW_REPORT_RADIO_WLAN);
    EXPECT_EQ(GetVideoBlockReasonForIpcan(), AosHandle::BLOCK_VIWIFI_CAPABILITY);
}

TEST_F(AosHandleMtcTest, IsCsFeatureTagRequired_Test)
{
    EXPECT_CALL(m_objMockIAosNConfiguration, IsVideoOverWifiSupportedWithoutVoice())
            .Times(AnyNumber())
            .WillRepeatedly(Return(IMS_FALSE));
    EXPECT_FALSE(IsCsFeatureTagRequired());

    EXPECT_CALL(m_objMockIAosNConfiguration, IsVideoOverWifiSupportedWithoutVoice())
            .Times(AnyNumber())
            .WillRepeatedly(Return(IMS_TRUE));
    RemoveFeature(ImsAosFeature::VIDEO);
    EXPECT_FALSE(IsCsFeatureTagRequired());

    AddFeature(ImsAosFeature::VIDEO);
    SetEpdgEnabled(IMS_FALSE);
    EXPECT_FALSE(IsCsFeatureTagRequired());

    SetEpdgEnabled(IMS_TRUE);
    EXPECT_CALL(m_objMockIAosNetTracker, GetMobileNetworkType())
            .Times(AnyNumber())
            .WillRepeatedly(Return(NW_REPORT_RADIO_LTE));
    EXPECT_CALL(m_objMockIAosNetTracker, GetMobileChangingNetworkType())
            .Times(2)
            .WillOnce(Return(NW_REPORT_RADIO_LTE))
            .WillOnce(Return(NW_REPORT_RADIO_GSM));
    EXPECT_FALSE(IsCsFeatureTagRequired());
    EXPECT_TRUE(IsCsFeatureTagRequired());
}

TEST_F(AosHandleMtcTest, IsInvalidMobileNetwork_Test)
{
    EXPECT_CALL(m_objMockIAosNetTracker, GetMobileNetworkType())
            .Times(4)
            .WillOnce(Return(NW_REPORT_RADIO_LTE))
            .WillOnce(Return(NW_REPORT_RADIO_LTE))
            .WillOnce(Return(NW_REPORT_RADIO_GSM))
            .WillOnce(Return(NW_REPORT_RADIO_GSM));
    EXPECT_CALL(m_objMockIAosNetTracker, GetMobileChangingNetworkType())
            .Times(2)
            .WillOnce(Return(NW_REPORT_RADIO_LTE))
            .WillOnce(Return(NW_REPORT_RADIO_GSM));

    EXPECT_FALSE(IsInvalidMobileNetwork());
    EXPECT_TRUE(IsInvalidMobileNetwork());
    EXPECT_TRUE(IsInvalidMobileNetwork());
    EXPECT_TRUE(IsInvalidMobileNetwork());
}