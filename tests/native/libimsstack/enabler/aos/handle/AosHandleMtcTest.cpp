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

    void SetCapabilities(IN IMSMap<IMS_UINT32, IMS_UINT32>& objNewCapabilities)
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

    void UpdateFeatureTags() { m_pAosHandleMtc->UpdateFeatureTags(); }

    IMS_BOOL ProcessImsSuspended(IN IMS_UINT32 nReason)
    {
        return m_pAosHandleMtc->ProcessImsSuspended(nReason);
    };

    IMS_BOOL ProcessImsResumed(IN IMS_UINT32 nReason)
    {
        return m_pAosHandleMtc->ProcessImsResumed(nReason);
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

    IMS_BOOL ProcessUnavailableFeatureForVops(IN IMS_UINT32 nState)
    {
        return m_pAosHandleMtc->ProcessUnavailableFeatureForVops(nState);
    }

    void SetDataConnected(IN IMS_BOOL bConnected)
    {
        m_pAosHandleMtc->m_bDataConnected = bConnected;
    }
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

    m_pAosHandleMtc->CallTracker_StateChanged(IAosCallTracker::TYPE_CS, CallState::RINGING);
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

    IMSVector<IMS_SINT32> objTestVector;
    EXPECT_CALL(m_objMockIAosNConfiguration, GetRegWithFeatureTagUnavailablePolicy())
            .Times(AnyNumber())
            .WillRepeatedly(ReturnRef(objTestVector));

    EXPECT_CALL(m_objMockIAosNConfiguration, GetRegWithFeatureTagUnavailable())
            .Times(AnyNumber())
            .WillRepeatedly(ReturnRef(objTestVector));

    EXPECT_CALL(m_objMockIAosNConfiguration,
            IsGGsmaRcsTelephonyFeatureTagUsedAsAvailableVoiceCallType())
            .Times(AnyNumber())
            .WillRepeatedly(Return(IMS_FALSE));

    EXPECT_CALL(m_objMockIAosNetTracker, GetMobileNetworkType())
            .Times(AnyNumber())
            .WillRepeatedly(Return(NW_REPORT_RADIO_LTE));

    EXPECT_CALL(m_objMockIAosNConfiguration, IsWfcImsAvailable())
            .Times(AnyNumber())
            .WillRepeatedly(Return(IMS_TRUE));

    SetHoldingVopsState(IMS_VOICE_OVER_PS_NOT_SUPPORTED);

    m_pAosHandleMtc->CallTracker_StateChanged(IAosCallTracker::TYPE_NORMAL, CallState::IDLE);

    EXPECT_TRUE(IsHandleBlocked(AosHandle::BLOCK_VOPS));
}

TEST_F(AosHandleMtcTest, CallTracker_StateChanged_Test5)
{
    // Test5: Holding vops for call test 2.
    //        call type is normal / call state is idle / vops holt for call
    //        / unavailable feature policy for vops / no unavailable feature
    // Expectation: Add vops block to main blocks

    EXPECT_CALL(m_objMockIAosNConfiguration, IsVopsIgnoredForVolteEnabled())
            .Times(AnyNumber())
            .WillRepeatedly(Return(IMS_FALSE));

    IMSVector<IMS_SINT32> objPolicy;
    objPolicy.Add(CarrierConfig::Assets::UNAVAILABLE_FEATURE_POLICY_VOPS);
    EXPECT_CALL(m_objMockIAosNConfiguration, GetRegWithFeatureTagUnavailablePolicy())
            .Times(AnyNumber())
            .WillRepeatedly(ReturnRef(objPolicy));

    IMSVector<IMS_SINT32> objFeature;
    EXPECT_CALL(m_objMockIAosNConfiguration, GetRegWithFeatureTagUnavailable())
            .Times(AnyNumber())
            .WillRepeatedly(ReturnRef(objFeature));

    EXPECT_CALL(m_objMockIAosNConfiguration,
            IsGGsmaRcsTelephonyFeatureTagUsedAsAvailableVoiceCallType())
            .Times(AnyNumber())
            .WillRepeatedly(Return(IMS_FALSE));

    EXPECT_CALL(m_objMockIAosNetTracker, GetMobileNetworkType())
            .Times(AnyNumber())
            .WillRepeatedly(Return(NW_REPORT_RADIO_LTE));

    EXPECT_CALL(m_objMockIAosNConfiguration, IsWfcImsAvailable())
            .Times(AnyNumber())
            .WillRepeatedly(Return(IMS_TRUE));

    SetHoldingVopsState(IMS_VOICE_OVER_PS_NOT_SUPPORTED);

    m_pAosHandleMtc->CallTracker_StateChanged(IAosCallTracker::TYPE_NORMAL, CallState::IDLE);

    EXPECT_TRUE(IsHandleBlocked(AosHandle::BLOCK_VOPS));
}

TEST_F(AosHandleMtcTest, CallTracker_StateChanged_Test6)
{
    // Test6: Holding vops for call test 3.
    //        call type is normal / call state is idle / vops holt for call
    //        / unavailable feature policy for vops / unavailable feature mmtel/video
    // Expectation: No add vops block to main blocks

    EXPECT_CALL(m_objMockIAosNConfiguration, IsVopsIgnoredForVolteEnabled())
            .Times(AnyNumber())
            .WillRepeatedly(Return(IMS_FALSE));

    IMSVector<IMS_SINT32> objPolicy;
    objPolicy.Add(CarrierConfig::Assets::UNAVAILABLE_FEATURE_POLICY_VOPS);
    EXPECT_CALL(m_objMockIAosNConfiguration, GetRegWithFeatureTagUnavailablePolicy())
            .Times(AnyNumber())
            .WillRepeatedly(ReturnRef(objPolicy));

    IMSVector<IMS_SINT32> objFeature;
    objFeature.Add(CarrierConfig::Assets::UNAVAILABLE_FEATURE_TYPE_MMTEL);
    objFeature.Add(CarrierConfig::Assets::UNAVAILABLE_FEATURE_TYPE_VIDEO);
    EXPECT_CALL(m_objMockIAosNConfiguration, GetRegWithFeatureTagUnavailable())
            .Times(AnyNumber())
            .WillRepeatedly(ReturnRef(objFeature));

    EXPECT_CALL(m_objMockIAosNConfiguration,
            IsGGsmaRcsTelephonyFeatureTagUsedAsAvailableVoiceCallType())
            .Times(AnyNumber())
            .WillRepeatedly(Return(IMS_FALSE));

    EXPECT_CALL(m_objMockIAosNetTracker, GetMobileNetworkType())
            .Times(AnyNumber())
            .WillRepeatedly(Return(NW_REPORT_RADIO_LTE));

    EXPECT_CALL(m_objMockIAosNConfiguration, IsWfcImsAvailable())
            .Times(AnyNumber())
            .WillRepeatedly(Return(IMS_TRUE));

    SetHoldingVopsState(IMS_VOICE_OVER_PS_NOT_SUPPORTED);

    m_pAosHandleMtc->CallTracker_StateChanged(IAosCallTracker::TYPE_NORMAL, CallState::IDLE);

    EXPECT_FALSE(IsHandleBlocked(AosHandle::BLOCK_VOPS));
}

TEST_F(AosHandleMtcTest, CallTracker_StateChanged_Test7)
{
    // Test7: vzw vowifi feature tag test. Idle -> Offhook -> Idle
    // Expectation: "cs" in idle, "cs,volte" in offhook

    AddFeature(ImsAosFeature::VIDEO);
    AddFeatureTag("+g.gsma.rcs.telephony", "\"cs\"");
    AddBindedFeature(ImsAosFeature::VIDEO);
    AddBindedFeatureTag("+g.gsma.rcs.telephony", "\"cs\"");

    SetEpdgEnabled(IMS_TRUE);

    AosFeatureTagList objExpectedFeatureTagListIdle;
    objExpectedFeatureTagListIdle.AddFeature(ImsAosFeature::VIDEO);
    objExpectedFeatureTagListIdle.AddFeatureTag("+g.gsma.rcs.telephony", "\"cs\"");

    AosFeatureTagList objExpectedFeatureTagListOffhook;
    objExpectedFeatureTagListOffhook.AddFeature(ImsAosFeature::VIDEO);
    objExpectedFeatureTagListOffhook.AddFeatureTag("+g.gsma.rcs.telephony", "cs");
    objExpectedFeatureTagListOffhook.AddFeatureTag("+g.gsma.rcs.telephony", "volte");

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

    IMSVector<IMS_SINT32> objTestVector;
    EXPECT_CALL(m_objMockIAosNConfiguration, GetRegWithFeatureTagUnavailablePolicy())
            .Times(AnyNumber())
            .WillRepeatedly(ReturnRef(objTestVector));

    EXPECT_CALL(m_objMockIAosNConfiguration, GetRegWithFeatureTagUnavailable())
            .Times(AnyNumber())
            .WillRepeatedly(ReturnRef(objTestVector));

    m_pAosHandleMtc->NetTracker_StatusChanged();

    EXPECT_EQ(m_pAosHandleMtc->GetSuspendedReason(), AosReason::SUSPEND_NONE);
    EXPECT_FALSE(IsHandleBlocked(AosHandle::BLOCK_VILTE_CAPABILITY));
}

TEST_F(AosHandleMtcTest, NetTracker_StatusChanged_Test2)
{
    // Test2: Srv In, Network changed to GSM
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

    IMSVector<IMS_SINT32> objPolicy;
    EXPECT_CALL(m_objMockIAosNConfiguration, GetRegWithFeatureTagUnavailablePolicy())
            .Times(AnyNumber())
            .WillRepeatedly(ReturnRef(objPolicy));

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

TEST_F(AosHandleMtcTest, InitializeServiceBlock_Test)
{
    // Expectation: Set m_bBlocked depending on AosHandleMtc::IsHandleBlocked()

    IMSVector<IMS_SINT32> objTestVector;
    EXPECT_CALL(m_objMockIAosNConfiguration, GetRegWithFeatureTagUnavailable())
            .Times(AnyNumber())
            .WillRepeatedly(ReturnRef(objTestVector));

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

    IMSVector<IMS_SINT32> objTestVector;
    EXPECT_CALL(m_objMockIAosNConfiguration, GetRegWithFeatureTagUnavailable())
            .Times(AnyNumber())
            .WillRepeatedly(ReturnRef(objTestVector));

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

    IMSVector<IMS_SINT32> objTestVector;
    EXPECT_CALL(m_objMockIAosNConfiguration, GetRegWithFeatureTagUnavailable())
            .Times(AnyNumber())
            .WillRepeatedly(ReturnRef(objTestVector));

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
    objExpectedFeatureTagList.AddFeatureTag("+g.gsma.rcs.telephony", "cs");
    objExpectedFeatureTagList.AddFeatureTag("+g.gsma.rcs.telephony", "volte");

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

TEST_F(AosHandleMtcTest, UpdateFeatureTags_Test1)
{
    // Test1: No use +g.gsma.rcs.telephony as available voice call type
    // Expectation: Do nothing. No change in feature tag list.

    AddFeature(ImsAosFeature::MMTEL);
    AddFeatureTag("+g.gsma.rcs.telephony", "cs");
    AddFeatureTag("+g.gsma.rcs.telephony", "volte");

    AosFeatureTagList objExpectedFeatureTagList;
    objExpectedFeatureTagList.AddFeature(ImsAosFeature::MMTEL);
    objExpectedFeatureTagList.AddFeatureTag("+g.gsma.rcs.telephony", "cs");
    objExpectedFeatureTagList.AddFeatureTag("+g.gsma.rcs.telephony", "volte");

    EXPECT_CALL(m_objMockIAosNConfiguration,
            IsGGsmaRcsTelephonyFeatureTagUsedAsAvailableVoiceCallType())
            .Times(AnyNumber())
            .WillRepeatedly(Return(IMS_FALSE));

    UpdateFeatureTags();

    EXPECT_TRUE(m_pAosHandleMtc->GetFeatureTagList().Equals(objExpectedFeatureTagList));
}

TEST_F(AosHandleMtcTest, UpdateFeatureTags_Test2)
{
    // Test2: vzw vowifi "cs" feature tag for call test. Idle -> Offhook -> Idle
    // Expectation: "cs" in idle, "cs,volte" in offhook

    AddFeature(ImsAosFeature::VIDEO);
    AddFeatureTag("+g.gsma.rcs.telephony", "\"cs\"");
    AddBindedFeature(ImsAosFeature::VIDEO);
    AddBindedFeatureTag("+g.gsma.rcs.telephony", "\"cs\"");

    SetEpdgEnabled(IMS_TRUE);

    AosFeatureTagList objExpectedFeatureTagListIdle;
    objExpectedFeatureTagListIdle.AddFeature(ImsAosFeature::VIDEO);
    objExpectedFeatureTagListIdle.AddFeatureTag("+g.gsma.rcs.telephony", "\"cs\"");

    AosFeatureTagList objExpectedFeatureTagListOffhook;
    objExpectedFeatureTagListOffhook.AddFeature(ImsAosFeature::VIDEO);
    objExpectedFeatureTagListOffhook.AddFeatureTag("+g.gsma.rcs.telephony", "cs");
    objExpectedFeatureTagListOffhook.AddFeatureTag("+g.gsma.rcs.telephony", "volte");

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

    UpdateFeatureTags();
    EXPECT_TRUE(m_pAosHandleMtc->GetFeatureTagList().Equals(objExpectedFeatureTagListOffhook));

    UpdateFeatureTags();
    EXPECT_TRUE(m_pAosHandleMtc->GetFeatureTagList().Equals(objExpectedFeatureTagListIdle));
}

TEST_F(AosHandleMtcTest, UpdateFeatureTags_Test3)
{
    // Test3: VZW VoWiFi IMS Registration Table Test - Wifi Only.
    // Expectation: "cs,volte" if mmtel / "cs" if no mmtel but video / none if no mmtel and video

    AddFeature(ImsAosFeature::MMTEL);
    AddFeature(ImsAosFeature::VIDEO);
    AddFeatureTag("+g.gsma.rcs.telephony", "cs");
    AddFeatureTag("+g.gsma.rcs.telephony", "volte");

    SetEpdgEnabled(IMS_TRUE);

    AosFeatureTagList objExpectedFeatureTagListMmtel;
    objExpectedFeatureTagListMmtel.AddFeature(ImsAosFeature::MMTEL);
    objExpectedFeatureTagListMmtel.AddFeature(ImsAosFeature::VIDEO);
    objExpectedFeatureTagListMmtel.AddFeatureTag("+g.gsma.rcs.telephony", "cs");
    objExpectedFeatureTagListMmtel.AddFeatureTag("+g.gsma.rcs.telephony", "volte");

    AosFeatureTagList objExpectedFeatureTagListVideo;
    objExpectedFeatureTagListVideo.AddFeature(ImsAosFeature::VIDEO);
    objExpectedFeatureTagListVideo.AddFeatureTag("+g.gsma.rcs.telephony", "\"cs\"");

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
    UpdateFeatureTags();
    EXPECT_TRUE(m_pAosHandleMtc->GetFeatureTagList().Equals(objExpectedFeatureTagListVideo));

    // Mmtel
    AddFeature(ImsAosFeature::MMTEL);
    UpdateFeatureTags();
    EXPECT_TRUE(m_pAosHandleMtc->GetFeatureTagList().Equals(objExpectedFeatureTagListMmtel));

    // No mmtel and video
    RemoveFeature(ImsAosFeature::MMTEL);
    RemoveFeature(ImsAosFeature::VIDEO);
    UpdateFeatureTags();
    EXPECT_TRUE(m_pAosHandleMtc->GetFeatureTagList().Equals(objExpectedFeatureTagListNoFeature));
}

TEST_F(AosHandleMtcTest, UpdateFeatureTags_Test4)
{
    // Test4: VZW VoWiFi IMS Registration Table Test - CS Roam + Wifi
    // Expectation: "cs,volte" if mmtel / "cs" if no mmtel but video / none if no mmtel and video

    AddFeature(ImsAosFeature::MMTEL);
    AddFeature(ImsAosFeature::VIDEO);
    AddFeatureTag("+g.gsma.rcs.telephony", "cs");
    AddFeatureTag("+g.gsma.rcs.telephony", "volte");

    SetEpdgEnabled(IMS_TRUE);

    AosFeatureTagList objExpectedFeatureTagListMmtel;
    objExpectedFeatureTagListMmtel.AddFeature(ImsAosFeature::MMTEL);
    objExpectedFeatureTagListMmtel.AddFeature(ImsAosFeature::VIDEO);
    objExpectedFeatureTagListMmtel.AddFeatureTag("+g.gsma.rcs.telephony", "cs");
    objExpectedFeatureTagListMmtel.AddFeatureTag("+g.gsma.rcs.telephony", "volte");

    AosFeatureTagList objExpectedFeatureTagListVideo;
    objExpectedFeatureTagListVideo.AddFeature(ImsAosFeature::VIDEO);
    objExpectedFeatureTagListVideo.AddFeatureTag("+g.gsma.rcs.telephony", "\"cs\"");

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
    UpdateFeatureTags();
    EXPECT_TRUE(m_pAosHandleMtc->GetFeatureTagList().Equals(objExpectedFeatureTagListVideo));

    // Mmtel
    AddFeature(ImsAosFeature::MMTEL);
    UpdateFeatureTags();
    EXPECT_TRUE(m_pAosHandleMtc->GetFeatureTagList().Equals(objExpectedFeatureTagListMmtel));

    // No mmtel and video
    RemoveFeature(ImsAosFeature::MMTEL);
    RemoveFeature(ImsAosFeature::VIDEO);
    UpdateFeatureTags();
    EXPECT_TRUE(m_pAosHandleMtc->GetFeatureTagList().Equals(objExpectedFeatureTagListNoFeature));
}

TEST_F(AosHandleMtcTest, ProcessImsSuspended_Test1)
{
    // Test1: ims not connected
    // Expectation: no suspended reason is set, return false

    SetHandleState(AosHandle::STATE_DISCONNECTED);
    EXPECT_FALSE(ProcessImsSuspended(0));
    EXPECT_EQ(m_pAosHandleMtc->GetSuspendedReason(), AosReason::SUSPEND_NONE);
}

TEST_F(AosHandleMtcTest, ProcessImsSuspended_Test2)
{
    // Test2: ims connected, invalid reason
    // Expectation: no suspended reason is set, return false

    SetHandleState(AosHandle::STATE_CONNECTED);
    EXPECT_FALSE(ProcessImsSuspended(0));
    EXPECT_EQ(m_pAosHandleMtc->GetSuspendedReason(), AosReason::SUSPEND_NONE);
}

TEST_F(AosHandleMtcTest, ProcessImsSuspended_Test3)
{
    // Test3: ims connected, valid reason, no listener
    // Expectation: The suspended reason is set, no callback, return false

    SetHandleState(AosHandle::STATE_CONNECTED);

    m_pAosHandleMtc->SetListener(IMS_NULL);
    EXPECT_CALL(m_objMockIImsAosListener, ImsAos_Suspended(_)).Times(0);

    EXPECT_FALSE(ProcessImsSuspended(AosReason::SUSPEND_NO_SERVICE));
    EXPECT_EQ(m_pAosHandleMtc->GetSuspendedReason(), AosReason::SUSPEND_NO_SERVICE);

    EXPECT_FALSE(ProcessImsSuspended(AosReason::SUSPEND_NO_LTE_COVERAGE));
    EXPECT_EQ(m_pAosHandleMtc->GetSuspendedReason(),
            (AosReason::SUSPEND_NO_SERVICE | AosReason::SUSPEND_NO_LTE_COVERAGE));
}

TEST_F(AosHandleMtcTest, ProcessImsSuspended_Test4)
{
    // Test4: ims connected, valid reason, valid listener
    // Expectation: The suspended reason is set, callback, return true

    SetHandleState(AosHandle::STATE_CONNECTED);

    m_pAosHandleMtc->SetListener(static_cast<IImsAosListener*>(&m_objMockIImsAosListener));
    EXPECT_CALL(m_objMockIImsAosListener, ImsAos_Suspended(_)).Times(2);

    EXPECT_TRUE(ProcessImsSuspended(AosReason::SUSPEND_NO_SERVICE));
    EXPECT_EQ(m_pAosHandleMtc->GetSuspendedReason(), AosReason::SUSPEND_NO_SERVICE);

    EXPECT_TRUE(ProcessImsSuspended(AosReason::SUSPEND_NO_LTE_COVERAGE));
    EXPECT_EQ(m_pAosHandleMtc->GetSuspendedReason(),
            (AosReason::SUSPEND_NO_SERVICE | AosReason::SUSPEND_NO_LTE_COVERAGE));
}

TEST_F(AosHandleMtcTest, ProcessImsResumed_Test1)
{
    // Test1: ims not connected
    // Expectation: suspended reason is not reset, return false

    SetHandleState(AosHandle::STATE_DISCONNECTED);
    SetSuspendedReason(AosReason::SUSPEND_NO_SERVICE);

    EXPECT_FALSE(ProcessImsResumed(AosReason::SUSPEND_NO_SERVICE));
    EXPECT_EQ(m_pAosHandleMtc->GetSuspendedReason(), AosReason::SUSPEND_NO_SERVICE);
}

TEST_F(AosHandleMtcTest, ProcessImsResumed_Test2)
{
    // Test2: ims connected, ims not suspended
    // Expectation: return false

    SetHandleState(AosHandle::STATE_CONNECTED);

    EXPECT_FALSE(ProcessImsResumed(AosReason::SUSPEND_NO_SERVICE));
}

TEST_F(AosHandleMtcTest, ProcessImsResumed_Test3)
{
    // Test3: ims connected, ims suspended, call with invalid reason
    // Expectation: still suspended, return false

    SetHandleState(AosHandle::STATE_CONNECTED);
    SetSuspendedReason(AosReason::SUSPEND_NO_LTE_COVERAGE);

    EXPECT_FALSE(ProcessImsResumed(AosReason::SUSPEND_CS_CALL));
    EXPECT_EQ(m_pAosHandleMtc->GetSuspendedReason(), AosReason::SUSPEND_NO_LTE_COVERAGE);
}

TEST_F(AosHandleMtcTest, ProcessImsResumed_Test4)
{
    // Test4: ims connected, ims suspended, multiple reasons set, call with valid reason
    // Expectation: still suspended, no callback, return false

    SetHandleState(AosHandle::STATE_CONNECTED);
    SetSuspendedReason(AosReason::SUSPEND_NO_SERVICE);
    SetSuspendedReason(AosReason::SUSPEND_NO_LTE_COVERAGE);

    m_pAosHandleMtc->SetListener(static_cast<IImsAosListener*>(&m_objMockIImsAosListener));
    EXPECT_CALL(m_objMockIImsAosListener, ImsAos_Resumed()).Times(0);

    EXPECT_FALSE(ProcessImsResumed(AosReason::SUSPEND_NO_SERVICE));
    EXPECT_EQ(m_pAosHandleMtc->GetSuspendedReason(), AosReason::SUSPEND_NO_LTE_COVERAGE);
}

TEST_F(AosHandleMtcTest, ProcessImsResumed_Test5)
{
    // Test5: ims connected, ims suspended, no listener, call with SUSPEND_NO_LTE_COVERAGE
    // Expectation: no suspended reason, no callback, return false

    SetHandleState(AosHandle::STATE_CONNECTED);
    SetSuspendedReason(AosReason::SUSPEND_NO_LTE_COVERAGE);

    EXPECT_CALL(m_objMockIImsAosListener, ImsAos_Resumed()).Times(0);

    EXPECT_FALSE(ProcessImsResumed(AosReason::SUSPEND_NO_LTE_COVERAGE));
    EXPECT_EQ(m_pAosHandleMtc->GetSuspendedReason(), AosReason::NONE);
}

TEST_F(AosHandleMtcTest, ProcessImsResumed_Test6)
{
    // Test6: ims connected, ims suspended, valid listener, call with SUSPEND_NO_SERVICE, LTE
    // Expectation: no suspended reason, callback, return true

    SetHandleState(AosHandle::STATE_CONNECTED);
    SetSuspendedReason(AosReason::SUSPEND_NO_SERVICE);

    EXPECT_CALL(m_objMockIAosNetTracker, GetNetworkType())
            .Times(AnyNumber())
            .WillRepeatedly(Return(NW_REPORT_RADIO_LTE));

    m_pAosHandleMtc->SetListener(static_cast<IImsAosListener*>(&m_objMockIImsAosListener));
    EXPECT_CALL(m_objMockIImsAosListener, ImsAos_Resumed()).Times(1);

    EXPECT_TRUE(ProcessImsResumed(AosReason::SUSPEND_NO_SERVICE));
    EXPECT_EQ(m_pAosHandleMtc->GetSuspendedReason(), AosReason::NONE);
}

TEST_F(AosHandleMtcTest, ProcessImsResumed_Test7)
{
    // Test7: ims connected, ims suspended, valid listener, call with SUSPEND_NO_SERVICE, GSM
    // Expectation: suspended reason changed to SUSPEND_NO_LTE_COVERAGE, no callback, return false

    SetHandleState(AosHandle::STATE_CONNECTED);
    SetSuspendedReason(AosReason::SUSPEND_NO_SERVICE);

    EXPECT_CALL(m_objMockIAosNetTracker, GetNetworkType())
            .Times(AnyNumber())
            .WillRepeatedly(Return(NW_REPORT_RADIO_GSM));

    m_pAosHandleMtc->SetListener(static_cast<IImsAosListener*>(&m_objMockIImsAosListener));
    EXPECT_CALL(m_objMockIImsAosListener, ImsAos_Resumed()).Times(0);

    EXPECT_FALSE(ProcessImsResumed(AosReason::SUSPEND_NO_SERVICE));
    EXPECT_EQ(m_pAosHandleMtc->GetSuspendedReason(), AosReason::SUSPEND_NO_LTE_COVERAGE);
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

    IMSVector<IMS_SINT32> objTestVector;
    EXPECT_CALL(m_objMockIAosNConfiguration, GetRegWithFeatureTagUnavailable())
            .Times(AnyNumber())
            .WillRepeatedly(ReturnRef(objTestVector));

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
    // Test5: Epdg not enabled, BLOCK_NONE, BLOCK_VOPS or BLOCK_VOLTE_CAPABILITY
    //        no unavailable feature tag
    // Expectation: return true if BLOCK_VOPS or BLOCK_VOLTE_CAPABILITY else return false

    IMSVector<IMS_SINT32> objTestVector;
    EXPECT_CALL(m_objMockIAosNConfiguration, GetRegWithFeatureTagUnavailable())
            .Times(AnyNumber())
            .WillRepeatedly(ReturnRef(objTestVector));

    AddBlock(AosHandle::BLOCK_VOPS);
    EXPECT_TRUE(IsHandleBlocked());

    RemoveBlock(AosHandle::BLOCK_VOPS);
    EXPECT_FALSE(IsHandleBlocked());

    AddBlock(AosHandle::BLOCK_VOLTE_CAPABILITY);
    EXPECT_TRUE(IsHandleBlocked());
}

TEST_F(AosHandleMtcTest, IsHandleBlocked_Test6)
{
    // Test6: Epdg not enabled, BLOCK_VOPS & BLOCK_VOLTE_CAPABILITY,
    //        No unavailable feature tag,
    // Expectation: return true

    AddBlock(AosHandle::BLOCK_VOPS);
    AddBlock(AosHandle::BLOCK_VOLTE_CAPABILITY);

    IMSVector<IMS_SINT32> objFeature;
    EXPECT_CALL(m_objMockIAosNConfiguration, GetRegWithFeatureTagUnavailable())
            .Times(AnyNumber())
            .WillRepeatedly(ReturnRef(objFeature));

    EXPECT_TRUE(IsHandleBlocked());
}

TEST_F(AosHandleMtcTest, IsHandleBlocked_Test7)
{
    // Test7: Epdg not enabled, BLOCK_VOPS & BLOCK_VOLTE_CAPABILITY,
    //        unavailable feature tag = MMTEL,
    //        unavailable feature tag policy = other than vops and capability
    // Expectation: return true

    AddBlock(AosHandle::BLOCK_VOPS);
    AddBlock(AosHandle::BLOCK_VOLTE_CAPABILITY);

    IMSVector<IMS_SINT32> objPolicy;
    objPolicy.Add(CarrierConfig::Assets::UNAVAILABLE_FEATURE_POLICY_3G);

    EXPECT_CALL(m_objMockIAosNConfiguration, GetRegWithFeatureTagUnavailablePolicy())
            .Times(AnyNumber())
            .WillRepeatedly(ReturnRef(objPolicy));

    IMSVector<IMS_SINT32> objFeature;
    objFeature.Add(CarrierConfig::Assets::UNAVAILABLE_FEATURE_TYPE_MMTEL);
    EXPECT_CALL(m_objMockIAosNConfiguration, GetRegWithFeatureTagUnavailable())
            .Times(AnyNumber())
            .WillRepeatedly(ReturnRef(objFeature));

    EXPECT_TRUE(IsHandleBlocked());
}

TEST_F(AosHandleMtcTest, IsHandleBlocked_Test8)
{
    // Test8: Epdg not enabled, BLOCK_VOPS
    //        unavailable feature tag = MMTEL,
    //        unavailable feature tag policy = vops
    // Expectation: return false

    AddBlock(AosHandle::BLOCK_VOPS);

    IMSVector<IMS_SINT32> objPolicy;
    objPolicy.Add(CarrierConfig::Assets::UNAVAILABLE_FEATURE_POLICY_VOPS);

    EXPECT_CALL(m_objMockIAosNConfiguration, GetRegWithFeatureTagUnavailablePolicy())
            .Times(AnyNumber())
            .WillRepeatedly(ReturnRef(objPolicy));

    IMSVector<IMS_SINT32> objFeature;
    objFeature.Add(CarrierConfig::Assets::UNAVAILABLE_FEATURE_TYPE_MMTEL);
    EXPECT_CALL(m_objMockIAosNConfiguration, GetRegWithFeatureTagUnavailable())
            .Times(AnyNumber())
            .WillRepeatedly(ReturnRef(objFeature));

    EXPECT_FALSE(IsHandleBlocked());
}

TEST_F(AosHandleMtcTest, IsHandleBlocked_Test9)
{
    // Test9: Epdg not enabled, BLOCK_VOLTE_CAPABILITY
    //        unavailable feature tag = MMTEL,
    //        unavailable feature tag policy = capability
    // Expectation: return false

    AddBlock(AosHandle::BLOCK_VOLTE_CAPABILITY);

    IMSVector<IMS_SINT32> objPolicy;
    objPolicy.Add(CarrierConfig::Assets::UNAVAILABLE_FEATURE_POLICY_CAPABILITY);

    EXPECT_CALL(m_objMockIAosNConfiguration, GetRegWithFeatureTagUnavailablePolicy())
            .Times(AnyNumber())
            .WillRepeatedly(ReturnRef(objPolicy));

    IMSVector<IMS_SINT32> objFeature;
    objFeature.Add(CarrierConfig::Assets::UNAVAILABLE_FEATURE_TYPE_MMTEL);
    EXPECT_CALL(m_objMockIAosNConfiguration, GetRegWithFeatureTagUnavailable())
            .Times(AnyNumber())
            .WillRepeatedly(ReturnRef(objFeature));

    EXPECT_FALSE(IsHandleBlocked());
}

TEST_F(AosHandleMtcTest, IsHandleBlocked_Test10)
{
    // Test10: Epdg not enabled, BLOCK_VOPS & BLOCK_VOLTE_CAPABILITY,
    //        unavailable feature tag = MMTEL,
    //        unavailable feature tag policy = vops
    // Expectation: return true

    AddBlock(AosHandle::BLOCK_VOPS);
    AddBlock(AosHandle::BLOCK_VOLTE_CAPABILITY);

    IMSVector<IMS_SINT32> objPolicy;
    objPolicy.Add(CarrierConfig::Assets::UNAVAILABLE_FEATURE_POLICY_VOPS);

    EXPECT_CALL(m_objMockIAosNConfiguration, GetRegWithFeatureTagUnavailablePolicy())
            .Times(AnyNumber())
            .WillRepeatedly(ReturnRef(objPolicy));

    IMSVector<IMS_SINT32> objFeature;
    objFeature.Add(CarrierConfig::Assets::UNAVAILABLE_FEATURE_TYPE_MMTEL);
    EXPECT_CALL(m_objMockIAosNConfiguration, GetRegWithFeatureTagUnavailable())
            .Times(AnyNumber())
            .WillRepeatedly(ReturnRef(objFeature));

    EXPECT_TRUE(IsHandleBlocked());
}

TEST_F(AosHandleMtcTest, IsHandleBlocked_Test11)
{
    // Test11: Epdg not enabled, BLOCK_VOPS & BLOCK_VOLTE_CAPABILITY,
    //        unavailable feature tag = MMTEL,
    //        unavailable feature tag policy = capability
    // Expectation: return true

    AddBlock(AosHandle::BLOCK_VOPS);
    AddBlock(AosHandle::BLOCK_VOLTE_CAPABILITY);

    IMSVector<IMS_SINT32> objPolicy;
    objPolicy.Add(CarrierConfig::Assets::UNAVAILABLE_FEATURE_POLICY_CAPABILITY);

    EXPECT_CALL(m_objMockIAosNConfiguration, GetRegWithFeatureTagUnavailablePolicy())
            .Times(AnyNumber())
            .WillRepeatedly(ReturnRef(objPolicy));

    IMSVector<IMS_SINT32> objFeature;
    objFeature.Add(CarrierConfig::Assets::UNAVAILABLE_FEATURE_TYPE_MMTEL);
    EXPECT_CALL(m_objMockIAosNConfiguration, GetRegWithFeatureTagUnavailable())
            .Times(AnyNumber())
            .WillRepeatedly(ReturnRef(objFeature));

    EXPECT_TRUE(IsHandleBlocked());
}

TEST_F(AosHandleMtcTest, IsHandleBlocked_Test12)
{
    // Test12: Epdg not enabled, BLOCK_VOPS & BLOCK_VOLTE_CAPABILITY,
    //        unavailable feature tag = MMTEL,
    //        unavailable feature tag policy = vops & capability
    // Expectation: return false

    AddBlock(AosHandle::BLOCK_VOPS);
    AddBlock(AosHandle::BLOCK_VOLTE_CAPABILITY);

    IMSVector<IMS_SINT32> objPolicy;
    objPolicy.Add(CarrierConfig::Assets::UNAVAILABLE_FEATURE_POLICY_VOPS);
    objPolicy.Add(CarrierConfig::Assets::UNAVAILABLE_FEATURE_POLICY_CAPABILITY);

    EXPECT_CALL(m_objMockIAosNConfiguration, GetRegWithFeatureTagUnavailablePolicy())
            .Times(AnyNumber())
            .WillRepeatedly(ReturnRef(objPolicy));

    IMSVector<IMS_SINT32> objFeature;
    objFeature.Add(CarrierConfig::Assets::UNAVAILABLE_FEATURE_TYPE_MMTEL);
    EXPECT_CALL(m_objMockIAosNConfiguration, GetRegWithFeatureTagUnavailable())
            .Times(AnyNumber())
            .WillRepeatedly(ReturnRef(objFeature));

    EXPECT_FALSE(IsHandleBlocked());
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
    // Test1: no capability(LTE,IWLAN,NR), Current network=GSM
    // Expectation: set none capa for the network. No block.

    IMSMap<IMS_UINT32, IMS_UINT32> objNewCapabilities, objExpectedCapabilities;

    objExpectedCapabilities.Add(static_cast<IMS_UINT32>(AosNetworkType::LTE),
            static_cast<IMS_UINT32>(AosCapability::NONE));
    objExpectedCapabilities.Add(static_cast<IMS_UINT32>(AosNetworkType::IWLAN),
            static_cast<IMS_UINT32>(AosCapability::NONE));
    objExpectedCapabilities.Add(static_cast<IMS_UINT32>(AosNetworkType::NR),
            static_cast<IMS_UINT32>(AosCapability::NONE));

    EXPECT_CALL(m_objMockIAosNetTracker, GetNetworkType())
            .Times(AnyNumber())
            .WillRepeatedly(Return(NW_REPORT_RADIO_GSM));

    ProcessCapabilitiesChanged(objNewCapabilities);

    EXPECT_FALSE(IsHandleBlockedBase());
    EXPECT_TRUE(IsEqualCapabilities(GetCapabilities(), objExpectedCapabilities));
}

TEST_F(AosHandleMtcTest, ProcessCapabilitiesChanged_Test2)
{
    // Test2: no capability(LTE,IWLAN,NR), Current network=LTE, no unavailable policy for capa.
    // Expectation: block BLOCK_VOLTE_CAPABILITY, BLOCK_VILTE_CAPABILITY,
    //              set none capa for the network.

    IMSMap<IMS_UINT32, IMS_UINT32> objNewCapabilities, objExpectedCapabilities;

    objExpectedCapabilities.Add(static_cast<IMS_UINT32>(AosNetworkType::LTE),
            static_cast<IMS_UINT32>(AosCapability::NONE));
    objExpectedCapabilities.Add(static_cast<IMS_UINT32>(AosNetworkType::IWLAN),
            static_cast<IMS_UINT32>(AosCapability::NONE));
    objExpectedCapabilities.Add(static_cast<IMS_UINT32>(AosNetworkType::NR),
            static_cast<IMS_UINT32>(AosCapability::NONE));

    EXPECT_CALL(m_objMockIAosNetTracker, GetNetworkType())
            .Times(AnyNumber())
            .WillRepeatedly(Return(NW_REPORT_RADIO_LTE));

    IMSVector<IMS_SINT32> objPolicy;
    objPolicy.Add(CarrierConfig::Assets::UNAVAILABLE_FEATURE_POLICY_3G);

    EXPECT_CALL(m_objMockIAosNConfiguration, GetRegWithFeatureTagUnavailablePolicy())
            .Times(AnyNumber())
            .WillRepeatedly(ReturnRef(objPolicy));

    IMSVector<IMS_SINT32> objFeature;
    objFeature.Add(CarrierConfig::Assets::UNAVAILABLE_FEATURE_TYPE_SMS);
    EXPECT_CALL(m_objMockIAosNConfiguration, GetRegWithFeatureTagUnavailable())
            .Times(AnyNumber())
            .WillRepeatedly(ReturnRef(objFeature));

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
}

TEST_F(AosHandleMtcTest, ProcessCapabilitiesChanged_Test3)
{
    // Test3: no capability(LTE,IWLAN,NR), Current network=WLAN, no unavailable policy for capa.
    // Expectation: block BLOCK_VOLTE_CAPABILITY, BLOCK_VILTE_CAPABILITY,
    //              set none capa for the network.

    IMSMap<IMS_UINT32, IMS_UINT32> objNewCapabilities, objExpectedCapabilities;

    objExpectedCapabilities.Add(static_cast<IMS_UINT32>(AosNetworkType::LTE),
            static_cast<IMS_UINT32>(AosCapability::NONE));
    objExpectedCapabilities.Add(static_cast<IMS_UINT32>(AosNetworkType::IWLAN),
            static_cast<IMS_UINT32>(AosCapability::NONE));
    objExpectedCapabilities.Add(static_cast<IMS_UINT32>(AosNetworkType::NR),
            static_cast<IMS_UINT32>(AosCapability::NONE));

    EXPECT_CALL(m_objMockIAosNetTracker, GetNetworkType())
            .Times(AnyNumber())
            .WillRepeatedly(Return(NW_REPORT_RADIO_WLAN));

    IMSVector<IMS_SINT32> objPolicy;
    objPolicy.Add(CarrierConfig::Assets::UNAVAILABLE_FEATURE_POLICY_3G);

    EXPECT_CALL(m_objMockIAosNConfiguration, GetRegWithFeatureTagUnavailablePolicy())
            .Times(AnyNumber())
            .WillRepeatedly(ReturnRef(objPolicy));

    IMSVector<IMS_SINT32> objFeature;
    objFeature.Add(CarrierConfig::Assets::UNAVAILABLE_FEATURE_TYPE_SMS);
    EXPECT_CALL(m_objMockIAosNConfiguration, GetRegWithFeatureTagUnavailable())
            .Times(AnyNumber())
            .WillRepeatedly(ReturnRef(objFeature));

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
}

TEST_F(AosHandleMtcTest, ProcessCapabilitiesChanged_Test4)
{
    // Test4: no capability(LTE), Current network=LTE,
    //        unavailable policy for capa, unavailable feature voice/video capa.
    // Expectation: block none, set new capa for the network.

    IMSMap<IMS_UINT32, IMS_UINT32> objNewCapabilities;
    objNewCapabilities.Add(static_cast<IMS_UINT32>(AosNetworkType::IWLAN),
            static_cast<IMS_UINT32>(AosCapability::VOICE) |
                    static_cast<IMS_UINT32>(AosCapability::VIDEO));
    objNewCapabilities.Add(static_cast<IMS_UINT32>(AosNetworkType::NR),
            static_cast<IMS_UINT32>(AosCapability::VOICE) |
                    static_cast<IMS_UINT32>(AosCapability::VIDEO));

    IMSMap<IMS_UINT32, IMS_UINT32> objExpectedCapabilities;
    objExpectedCapabilities.Add(static_cast<IMS_UINT32>(AosNetworkType::LTE),
            static_cast<IMS_UINT32>(AosCapability::NONE));
    objExpectedCapabilities.Add(static_cast<IMS_UINT32>(AosNetworkType::IWLAN),
            static_cast<IMS_UINT32>(AosCapability::VOICE) |
                    static_cast<IMS_UINT32>(AosCapability::VIDEO));
    objExpectedCapabilities.Add(static_cast<IMS_UINT32>(AosNetworkType::NR),
            static_cast<IMS_UINT32>(AosCapability::VOICE) |
                    static_cast<IMS_UINT32>(AosCapability::VIDEO));

    EXPECT_CALL(m_objMockIAosNetTracker, GetNetworkType())
            .Times(AnyNumber())
            .WillRepeatedly(Return(NW_REPORT_RADIO_LTE));

    IMSVector<IMS_SINT32> objPolicy;
    objPolicy.Add(CarrierConfig::Assets::UNAVAILABLE_FEATURE_POLICY_CAPABILITY);

    EXPECT_CALL(m_objMockIAosNConfiguration, GetRegWithFeatureTagUnavailablePolicy())
            .Times(AnyNumber())
            .WillRepeatedly(ReturnRef(objPolicy));

    IMSVector<IMS_SINT32> objFeature;
    objFeature.Add(CarrierConfig::Assets::UNAVAILABLE_FEATURE_TYPE_MMTEL);
    objFeature.Add(CarrierConfig::Assets::UNAVAILABLE_FEATURE_TYPE_VIDEO);
    EXPECT_CALL(m_objMockIAosNConfiguration, GetRegWithFeatureTagUnavailable())
            .Times(AnyNumber())
            .WillRepeatedly(ReturnRef(objFeature));

    EXPECT_CALL(m_objMockIAosNConfiguration, IsWfcImsAvailable())
            .Times(AnyNumber())
            .WillRepeatedly(Return(IMS_TRUE));

    EXPECT_CALL(m_objMockIAosNConfiguration,
            IsGGsmaRcsTelephonyFeatureTagUsedAsAvailableVoiceCallType())
            .Times(AnyNumber())
            .WillRepeatedly(Return(IMS_FALSE));

    ProcessCapabilitiesChanged(objNewCapabilities);

    EXPECT_FALSE(IsHandleBlockedBase());
    EXPECT_TRUE(IsEqualCapabilities(GetCapabilities(), objExpectedCapabilities));
}

TEST_F(AosHandleMtcTest, ProcessCapabilitiesChanged_Test5)
{
    // Test5: no capability(LTE), Current network=LTE,
    //        unavailable policy for capa, no unavailable feature.
    // Expectation: block BLOCK_VOLTE_CAPABILITY, BLOCK_VILTE_CAPABILITY,
    //              set new capa for the network.

    IMSMap<IMS_UINT32, IMS_UINT32> objNewCapabilities;
    objNewCapabilities.Add(static_cast<IMS_UINT32>(AosNetworkType::IWLAN),
            static_cast<IMS_UINT32>(AosCapability::VOICE) |
                    static_cast<IMS_UINT32>(AosCapability::VIDEO));
    objNewCapabilities.Add(static_cast<IMS_UINT32>(AosNetworkType::NR),
            static_cast<IMS_UINT32>(AosCapability::VOICE) |
                    static_cast<IMS_UINT32>(AosCapability::VIDEO));

    IMSMap<IMS_UINT32, IMS_UINT32> objExpectedCapabilities;
    objExpectedCapabilities.Add(static_cast<IMS_UINT32>(AosNetworkType::LTE),
            static_cast<IMS_UINT32>(AosCapability::NONE));
    objExpectedCapabilities.Add(static_cast<IMS_UINT32>(AosNetworkType::IWLAN),
            static_cast<IMS_UINT32>(AosCapability::VOICE) |
                    static_cast<IMS_UINT32>(AosCapability::VIDEO));
    objExpectedCapabilities.Add(static_cast<IMS_UINT32>(AosNetworkType::NR),
            static_cast<IMS_UINT32>(AosCapability::VOICE) |
                    static_cast<IMS_UINT32>(AosCapability::VIDEO));

    EXPECT_CALL(m_objMockIAosNetTracker, GetNetworkType())
            .Times(AnyNumber())
            .WillRepeatedly(Return(NW_REPORT_RADIO_LTE));

    IMSVector<IMS_SINT32> objPolicy;
    objPolicy.Add(CarrierConfig::Assets::UNAVAILABLE_FEATURE_POLICY_CAPABILITY);

    EXPECT_CALL(m_objMockIAosNConfiguration, GetRegWithFeatureTagUnavailablePolicy())
            .Times(AnyNumber())
            .WillRepeatedly(ReturnRef(objPolicy));

    IMSVector<IMS_SINT32> objFeature;
    objFeature.Add(CarrierConfig::Assets::UNAVAILABLE_FEATURE_TYPE_SMS);
    EXPECT_CALL(m_objMockIAosNConfiguration, GetRegWithFeatureTagUnavailable())
            .Times(AnyNumber())
            .WillRepeatedly(ReturnRef(objFeature));

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
}

TEST_F(AosHandleMtcTest, ProcessCapabilitiesChanged_Test6)
{
    // Test6: no capability(IWLAN), Current network=WLAN,
    //        unavailable policy for capa, no unavailable feature.
    // Expectation: block BLOCK_VOWIFI_CAPABILITY, BLOCK_VIWIFI_CAPABILITY,
    //              set new capa for the network.

    IMSMap<IMS_UINT32, IMS_UINT32> objNewCapabilities;
    objNewCapabilities.Add(static_cast<IMS_UINT32>(AosNetworkType::LTE),
            static_cast<IMS_UINT32>(AosCapability::VOICE) |
                    static_cast<IMS_UINT32>(AosCapability::VIDEO));
    objNewCapabilities.Add(static_cast<IMS_UINT32>(AosNetworkType::NR),
            static_cast<IMS_UINT32>(AosCapability::VOICE) |
                    static_cast<IMS_UINT32>(AosCapability::VIDEO));

    IMSMap<IMS_UINT32, IMS_UINT32> objExpectedCapabilities;
    objExpectedCapabilities.Add(static_cast<IMS_UINT32>(AosNetworkType::IWLAN),
            static_cast<IMS_UINT32>(AosCapability::NONE));
    objExpectedCapabilities.Add(static_cast<IMS_UINT32>(AosNetworkType::LTE),
            static_cast<IMS_UINT32>(AosCapability::VOICE) |
                    static_cast<IMS_UINT32>(AosCapability::VIDEO));
    objExpectedCapabilities.Add(static_cast<IMS_UINT32>(AosNetworkType::NR),
            static_cast<IMS_UINT32>(AosCapability::VOICE) |
                    static_cast<IMS_UINT32>(AosCapability::VIDEO));

    SetEpdgEnabled(IMS_TRUE);
    EXPECT_CALL(m_objMockIAosNetTracker, GetNetworkType())
            .Times(AnyNumber())
            .WillRepeatedly(Return(NW_REPORT_RADIO_WLAN));

    IMSVector<IMS_SINT32> objPolicy;
    objPolicy.Add(CarrierConfig::Assets::UNAVAILABLE_FEATURE_POLICY_CAPABILITY);

    EXPECT_CALL(m_objMockIAosNConfiguration, GetRegWithFeatureTagUnavailablePolicy())
            .Times(AnyNumber())
            .WillRepeatedly(ReturnRef(objPolicy));

    IMSVector<IMS_SINT32> objFeature;
    objFeature.Add(CarrierConfig::Assets::UNAVAILABLE_FEATURE_TYPE_SMS);
    EXPECT_CALL(m_objMockIAosNConfiguration, GetRegWithFeatureTagUnavailable())
            .Times(AnyNumber())
            .WillRepeatedly(ReturnRef(objFeature));

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
}

TEST_F(AosHandleMtcTest, ProcessCapabilitiesChanged_Test7)
{
    // Test7: capability change to sms only(LTE), Current network=LTE, no unavailable policy.
    // Expectation: block BLOCK_VOLTE_CAPABILITY, BLOCK_VILTE_CAPABILITY,
    //              set new capa for the network.

    IMSMap<IMS_UINT32, IMS_UINT32> objNewCapabilities;
    objNewCapabilities.Add(static_cast<IMS_UINT32>(AosNetworkType::LTE),
            static_cast<IMS_UINT32>(AosCapability::SMS));
    objNewCapabilities.Add(static_cast<IMS_UINT32>(AosNetworkType::IWLAN),
            static_cast<IMS_UINT32>(AosCapability::VOICE) |
                    static_cast<IMS_UINT32>(AosCapability::VIDEO));
    objNewCapabilities.Add(static_cast<IMS_UINT32>(AosNetworkType::NR),
            static_cast<IMS_UINT32>(AosCapability::VOICE) |
                    static_cast<IMS_UINT32>(AosCapability::VIDEO));

    EXPECT_CALL(m_objMockIAosNetTracker, GetNetworkType())
            .Times(AnyNumber())
            .WillRepeatedly(Return(NW_REPORT_RADIO_LTE));

    IMSVector<IMS_SINT32> objPolicy;
    EXPECT_CALL(m_objMockIAosNConfiguration, GetRegWithFeatureTagUnavailablePolicy())
            .Times(AnyNumber())
            .WillRepeatedly(ReturnRef(objPolicy));

    IMSVector<IMS_SINT32> objFeature;
    EXPECT_CALL(m_objMockIAosNConfiguration, GetRegWithFeatureTagUnavailable())
            .Times(AnyNumber())
            .WillRepeatedly(ReturnRef(objFeature));

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
    EXPECT_TRUE(IsEqualCapabilities(GetCapabilities(), objNewCapabilities));
}

TEST_F(AosHandleMtcTest, ProcessCapabilitiesChanged_Test8)
{
    // Test8: capability change to sms only(IWLAN), Current network=WLAN, no unavailable policy.
    // Expectation: block BLOCK_VOWIFI_CAPABILITY, BLOCK_VIWIFI_CAPABILITY,
    //              set new capa for the network.

    IMSMap<IMS_UINT32, IMS_UINT32> objNewCapabilities;
    objNewCapabilities.Add(static_cast<IMS_UINT32>(AosNetworkType::IWLAN),
            static_cast<IMS_UINT32>(AosCapability::SMS));
    objNewCapabilities.Add(static_cast<IMS_UINT32>(AosNetworkType::LTE),
            static_cast<IMS_UINT32>(AosCapability::VOICE) |
                    static_cast<IMS_UINT32>(AosCapability::VIDEO));
    objNewCapabilities.Add(static_cast<IMS_UINT32>(AosNetworkType::NR),
            static_cast<IMS_UINT32>(AosCapability::VOICE) |
                    static_cast<IMS_UINT32>(AosCapability::VIDEO));

    SetEpdgEnabled(IMS_TRUE);
    EXPECT_CALL(m_objMockIAosNetTracker, GetNetworkType())
            .Times(AnyNumber())
            .WillRepeatedly(Return(NW_REPORT_RADIO_WLAN));

    IMSVector<IMS_SINT32> objPolicy;
    EXPECT_CALL(m_objMockIAosNConfiguration, GetRegWithFeatureTagUnavailablePolicy())
            .Times(AnyNumber())
            .WillRepeatedly(ReturnRef(objPolicy));

    IMSVector<IMS_SINT32> objFeature;
    EXPECT_CALL(m_objMockIAosNConfiguration, GetRegWithFeatureTagUnavailable())
            .Times(AnyNumber())
            .WillRepeatedly(ReturnRef(objFeature));

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
    EXPECT_TRUE(IsEqualCapabilities(GetCapabilities(), objNewCapabilities));
}

TEST_F(AosHandleMtcTest, ProcessCapabilitiesChanged_Test9)
{
    // Test9: capability change to sms only(LTE), Current network=LTE,
    //        unavailable policy for capa, no unavailable feature.
    // Expectation: block BLOCK_VOLTE_CAPABILITY, BLOCK_VILTE_CAPABILITY,
    //              set new capa for the network.

    IMSMap<IMS_UINT32, IMS_UINT32> objNewCapabilities;
    objNewCapabilities.Add(static_cast<IMS_UINT32>(AosNetworkType::LTE),
            static_cast<IMS_UINT32>(AosCapability::SMS));
    objNewCapabilities.Add(static_cast<IMS_UINT32>(AosNetworkType::IWLAN),
            static_cast<IMS_UINT32>(AosCapability::VOICE) |
                    static_cast<IMS_UINT32>(AosCapability::VIDEO));
    objNewCapabilities.Add(static_cast<IMS_UINT32>(AosNetworkType::NR),
            static_cast<IMS_UINT32>(AosCapability::VOICE) |
                    static_cast<IMS_UINT32>(AosCapability::VIDEO));

    EXPECT_CALL(m_objMockIAosNetTracker, GetNetworkType())
            .Times(AnyNumber())
            .WillRepeatedly(Return(NW_REPORT_RADIO_LTE));

    IMSVector<IMS_SINT32> objPolicy;
    objPolicy.Add(CarrierConfig::Assets::UNAVAILABLE_FEATURE_POLICY_CAPABILITY);

    EXPECT_CALL(m_objMockIAosNConfiguration, GetRegWithFeatureTagUnavailablePolicy())
            .Times(AnyNumber())
            .WillRepeatedly(ReturnRef(objPolicy));

    IMSVector<IMS_SINT32> objFeature;
    objFeature.Add(CarrierConfig::Assets::UNAVAILABLE_FEATURE_TYPE_SMS);
    EXPECT_CALL(m_objMockIAosNConfiguration, GetRegWithFeatureTagUnavailable())
            .Times(AnyNumber())
            .WillRepeatedly(ReturnRef(objFeature));

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
    EXPECT_TRUE(IsEqualCapabilities(GetCapabilities(), objNewCapabilities));
}

TEST_F(AosHandleMtcTest, ProcessCapabilitiesChanged_Test10)
{
    // Test10: capability change to sms only(IWLAN), Current network=WLAN,
    //        unavailable policy for capa, no unavailable feature.
    // Expectation: block BLOCK_VOWIFI_CAPABILITY, BLOCK_VIWIFI_CAPABILITY,
    //              set new capa for the network.

    IMSMap<IMS_UINT32, IMS_UINT32> objNewCapabilities;
    objNewCapabilities.Add(static_cast<IMS_UINT32>(AosNetworkType::IWLAN),
            static_cast<IMS_UINT32>(AosCapability::SMS));
    objNewCapabilities.Add(static_cast<IMS_UINT32>(AosNetworkType::LTE),
            static_cast<IMS_UINT32>(AosCapability::VOICE) |
                    static_cast<IMS_UINT32>(AosCapability::VIDEO));
    objNewCapabilities.Add(static_cast<IMS_UINT32>(AosNetworkType::NR),
            static_cast<IMS_UINT32>(AosCapability::VOICE) |
                    static_cast<IMS_UINT32>(AosCapability::VIDEO));

    SetEpdgEnabled(IMS_TRUE);
    EXPECT_CALL(m_objMockIAosNetTracker, GetNetworkType())
            .Times(AnyNumber())
            .WillRepeatedly(Return(NW_REPORT_RADIO_WLAN));

    IMSVector<IMS_SINT32> objPolicy;
    objPolicy.Add(CarrierConfig::Assets::UNAVAILABLE_FEATURE_POLICY_CAPABILITY);

    EXPECT_CALL(m_objMockIAosNConfiguration, GetRegWithFeatureTagUnavailablePolicy())
            .Times(AnyNumber())
            .WillRepeatedly(ReturnRef(objPolicy));

    IMSVector<IMS_SINT32> objFeature;
    objFeature.Add(CarrierConfig::Assets::UNAVAILABLE_FEATURE_TYPE_SMS);
    EXPECT_CALL(m_objMockIAosNConfiguration, GetRegWithFeatureTagUnavailable())
            .Times(AnyNumber())
            .WillRepeatedly(ReturnRef(objFeature));

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
    EXPECT_TRUE(IsEqualCapabilities(GetCapabilities(), objNewCapabilities));
}

TEST_F(AosHandleMtcTest, ProcessCapabilitiesChanged_Test11)
{
    // Test11: capability change to sms only(LTE), Current network=LTE,
    //        unavailable policy for capa, unavailable feature for voice/video.
    // Expectation: block none, set new capa for the network.

    IMSMap<IMS_UINT32, IMS_UINT32> objNewCapabilities;
    objNewCapabilities.Add(static_cast<IMS_UINT32>(AosNetworkType::LTE),
            static_cast<IMS_UINT32>(AosCapability::SMS));
    objNewCapabilities.Add(static_cast<IMS_UINT32>(AosNetworkType::IWLAN),
            static_cast<IMS_UINT32>(AosCapability::VOICE) |
                    static_cast<IMS_UINT32>(AosCapability::VIDEO));
    objNewCapabilities.Add(static_cast<IMS_UINT32>(AosNetworkType::NR),
            static_cast<IMS_UINT32>(AosCapability::VOICE) |
                    static_cast<IMS_UINT32>(AosCapability::VIDEO));

    EXPECT_CALL(m_objMockIAosNetTracker, GetNetworkType())
            .Times(AnyNumber())
            .WillRepeatedly(Return(NW_REPORT_RADIO_LTE));

    IMSVector<IMS_SINT32> objPolicy;
    objPolicy.Add(CarrierConfig::Assets::UNAVAILABLE_FEATURE_POLICY_CAPABILITY);

    EXPECT_CALL(m_objMockIAosNConfiguration, GetRegWithFeatureTagUnavailablePolicy())
            .Times(AnyNumber())
            .WillRepeatedly(ReturnRef(objPolicy));

    IMSVector<IMS_SINT32> objFeature;
    objFeature.Add(CarrierConfig::Assets::UNAVAILABLE_FEATURE_TYPE_MMTEL);
    objFeature.Add(CarrierConfig::Assets::UNAVAILABLE_FEATURE_TYPE_VIDEO);
    EXPECT_CALL(m_objMockIAosNConfiguration, GetRegWithFeatureTagUnavailable())
            .Times(AnyNumber())
            .WillRepeatedly(ReturnRef(objFeature));

    EXPECT_CALL(m_objMockIAosNConfiguration, IsWfcImsAvailable())
            .Times(AnyNumber())
            .WillRepeatedly(Return(IMS_TRUE));

    EXPECT_CALL(m_objMockIAosNConfiguration,
            IsGGsmaRcsTelephonyFeatureTagUsedAsAvailableVoiceCallType())
            .Times(AnyNumber())
            .WillRepeatedly(Return(IMS_FALSE));

    ProcessCapabilitiesChanged(objNewCapabilities);

    EXPECT_FALSE(IsHandleBlockedBase());
    EXPECT_TRUE(IsEqualCapabilities(GetCapabilities(), objNewCapabilities));
}

TEST_F(AosHandleMtcTest, ProcessCapabilitiesChanged_Test12)
{
    // Test12: capability change to sms only(IWLAN), Current network=WLAN,
    //        unavailable policy for capa, unavailable feature for voice/video.
    // Expectation: block none, set new capa for the network.

    IMSMap<IMS_UINT32, IMS_UINT32> objNewCapabilities;
    objNewCapabilities.Add(static_cast<IMS_UINT32>(AosNetworkType::IWLAN),
            static_cast<IMS_UINT32>(AosCapability::SMS));
    objNewCapabilities.Add(static_cast<IMS_UINT32>(AosNetworkType::LTE),
            static_cast<IMS_UINT32>(AosCapability::VOICE) |
                    static_cast<IMS_UINT32>(AosCapability::VIDEO));
    objNewCapabilities.Add(static_cast<IMS_UINT32>(AosNetworkType::NR),
            static_cast<IMS_UINT32>(AosCapability::VOICE) |
                    static_cast<IMS_UINT32>(AosCapability::VIDEO));

    SetEpdgEnabled(IMS_TRUE);
    EXPECT_CALL(m_objMockIAosNetTracker, GetNetworkType())
            .Times(AnyNumber())
            .WillRepeatedly(Return(NW_REPORT_RADIO_WLAN));

    IMSVector<IMS_SINT32> objPolicy;
    objPolicy.Add(CarrierConfig::Assets::UNAVAILABLE_FEATURE_POLICY_CAPABILITY);

    EXPECT_CALL(m_objMockIAosNConfiguration, GetRegWithFeatureTagUnavailablePolicy())
            .Times(AnyNumber())
            .WillRepeatedly(ReturnRef(objPolicy));

    IMSVector<IMS_SINT32> objFeature;
    objFeature.Add(CarrierConfig::Assets::UNAVAILABLE_FEATURE_TYPE_MMTEL);
    objFeature.Add(CarrierConfig::Assets::UNAVAILABLE_FEATURE_TYPE_VIDEO);
    EXPECT_CALL(m_objMockIAosNConfiguration, GetRegWithFeatureTagUnavailable())
            .Times(AnyNumber())
            .WillRepeatedly(ReturnRef(objFeature));

    EXPECT_CALL(m_objMockIAosNConfiguration, IsWfcImsAvailable())
            .Times(AnyNumber())
            .WillRepeatedly(Return(IMS_TRUE));

    EXPECT_CALL(m_objMockIAosNConfiguration,
            IsGGsmaRcsTelephonyFeatureTagUsedAsAvailableVoiceCallType())
            .Times(AnyNumber())
            .WillRepeatedly(Return(IMS_FALSE));

    ProcessCapabilitiesChanged(objNewCapabilities);

    EXPECT_FALSE(IsHandleBlockedBase());
    EXPECT_TRUE(IsEqualCapabilities(GetCapabilities(), objNewCapabilities));
}

TEST_F(AosHandleMtcTest, ProcessNetworkChanged_Test1)
{
    // Test1: Capa=(LTE:voice,video / IWLAN:video / NR:none), no unavailable policy, network=LTE
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

    EXPECT_CALL(m_objMockIAosNetTracker, GetNetworkType())
            .Times(AnyNumber())
            .WillRepeatedly(Return(NW_REPORT_RADIO_LTE));

    EXPECT_CALL(m_objMockIAosNConfiguration, IsWfcImsAvailable())
            .Times(AnyNumber())
            .WillRepeatedly(Return(IMS_TRUE));

    EXPECT_CALL(m_objMockIAosNConfiguration,
            IsGGsmaRcsTelephonyFeatureTagUsedAsAvailableVoiceCallType())
            .Times(AnyNumber())
            .WillRepeatedly(Return(IMS_FALSE));

    IMSVector<IMS_SINT32> objPolicy;
    EXPECT_CALL(m_objMockIAosNConfiguration, GetRegWithFeatureTagUnavailablePolicy())
            .Times(AnyNumber())
            .WillRepeatedly(ReturnRef(objPolicy));

    IMSVector<IMS_SINT32> objFeature;
    EXPECT_CALL(m_objMockIAosNConfiguration, GetRegWithFeatureTagUnavailable())
            .Times(AnyNumber())
            .WillRepeatedly(ReturnRef(objFeature));

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
    EXPECT_CALL(m_objMockIAosNetTracker, GetNetworkType())
            .Times(AnyNumber())
            .WillRepeatedly(Return(NW_REPORT_RADIO_WLAN));

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

    IMSVector<IMS_SINT32> objPolicy;
    EXPECT_CALL(m_objMockIAosNConfiguration, GetRegWithFeatureTagUnavailablePolicy())
            .Times(AnyNumber())
            .WillRepeatedly(ReturnRef(objPolicy));

    IMSVector<IMS_SINT32> objFeature;
    EXPECT_CALL(m_objMockIAosNConfiguration, GetRegWithFeatureTagUnavailable())
            .Times(AnyNumber())
            .WillRepeatedly(ReturnRef(objFeature));

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

    EXPECT_CALL(m_objMockIAosNetTracker, GetNetworkType())
            .Times(AnyNumber())
            .WillRepeatedly(Return(NW_REPORT_RADIO_NR));

    EXPECT_CALL(m_objMockIAosNConfiguration, IsWfcImsAvailable())
            .Times(AnyNumber())
            .WillRepeatedly(Return(IMS_TRUE));

    EXPECT_CALL(m_objMockIAosNConfiguration,
            IsGGsmaRcsTelephonyFeatureTagUsedAsAvailableVoiceCallType())
            .Times(AnyNumber())
            .WillRepeatedly(Return(IMS_FALSE));

    IMSVector<IMS_SINT32> objPolicy;
    EXPECT_CALL(m_objMockIAosNConfiguration, GetRegWithFeatureTagUnavailablePolicy())
            .Times(AnyNumber())
            .WillRepeatedly(ReturnRef(objPolicy));

    IMSVector<IMS_SINT32> objFeature;
    EXPECT_CALL(m_objMockIAosNConfiguration, GetRegWithFeatureTagUnavailable())
            .Times(AnyNumber())
            .WillRepeatedly(ReturnRef(objFeature));

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
    //        unavailable policy for capa, unavailable feature for sms
    // Expectation: block BLOCK_VOLTE_CAPABILITY, BLOCK_VILTE_CAPABILITY

    IMSMap<IMS_UINT32, IMS_UINT32> objCapabilities;
    objCapabilities.Add(static_cast<IMS_UINT32>(AosNetworkType::LTE),
            static_cast<IMS_UINT32>(AosCapability::VOICE) |
                    static_cast<IMS_UINT32>(AosCapability::VIDEO));
    objCapabilities.Add(static_cast<IMS_UINT32>(AosNetworkType::IWLAN),
            static_cast<IMS_UINT32>(AosCapability::NONE));
    objCapabilities.Add(static_cast<IMS_UINT32>(AosNetworkType::NR),
            static_cast<IMS_UINT32>(AosCapability::NONE));

    SetCapabilities(objCapabilities);

    EXPECT_CALL(m_objMockIAosNetTracker, GetNetworkType())
            .Times(AnyNumber())
            .WillRepeatedly(Return(NW_REPORT_RADIO_NR));

    EXPECT_CALL(m_objMockIAosNConfiguration, IsWfcImsAvailable())
            .Times(AnyNumber())
            .WillRepeatedly(Return(IMS_TRUE));

    EXPECT_CALL(m_objMockIAosNConfiguration,
            IsGGsmaRcsTelephonyFeatureTagUsedAsAvailableVoiceCallType())
            .Times(AnyNumber())
            .WillRepeatedly(Return(IMS_FALSE));

    IMSVector<IMS_SINT32> objPolicy;
    objPolicy.Add(CarrierConfig::Assets::UNAVAILABLE_FEATURE_POLICY_CAPABILITY);
    EXPECT_CALL(m_objMockIAosNConfiguration, GetRegWithFeatureTagUnavailablePolicy())
            .Times(AnyNumber())
            .WillRepeatedly(ReturnRef(objPolicy));

    IMSVector<IMS_SINT32> objFeature;
    objFeature.Add(CarrierConfig::Assets::UNAVAILABLE_FEATURE_TYPE_SMS);
    EXPECT_CALL(m_objMockIAosNConfiguration, GetRegWithFeatureTagUnavailable())
            .Times(AnyNumber())
            .WillRepeatedly(ReturnRef(objFeature));

    ProcessNetworkChanged();
    EXPECT_TRUE(IsHandleBlocked(AosHandle::BLOCK_VOLTE_CAPABILITY));
    EXPECT_TRUE(IsHandleBlocked(AosHandle::BLOCK_VILTE_CAPABILITY));
}

TEST_F(AosHandleMtcTest, ProcessNetworkChanged_Test5)
{
    // Test5: Capa=(LTE:voice,video / IWLAN:none / NR:none), network=WLAN
    //        unavailable policy for capa, unavailable feature for sms
    // Expectation: block BLOCK_VOWIFI_CAPABILITY, BLOCK_VIWIFI_CAPABILITY

    IMSMap<IMS_UINT32, IMS_UINT32> objCapabilities;
    objCapabilities.Add(static_cast<IMS_UINT32>(AosNetworkType::LTE),
            static_cast<IMS_UINT32>(AosCapability::VOICE) |
                    static_cast<IMS_UINT32>(AosCapability::VIDEO));
    objCapabilities.Add(static_cast<IMS_UINT32>(AosNetworkType::IWLAN),
            static_cast<IMS_UINT32>(AosCapability::NONE));
    objCapabilities.Add(static_cast<IMS_UINT32>(AosNetworkType::NR),
            static_cast<IMS_UINT32>(AosCapability::NONE));

    SetCapabilities(objCapabilities);

    EXPECT_CALL(m_objMockIAosNetTracker, GetNetworkType())
            .Times(AnyNumber())
            .WillRepeatedly(Return(NW_REPORT_RADIO_WLAN));

    SetEpdgEnabled(IMS_TRUE);
    EXPECT_CALL(m_objMockIAosNConfiguration, IsWfcImsAvailable())
            .Times(AnyNumber())
            .WillRepeatedly(Return(IMS_TRUE));

    EXPECT_CALL(m_objMockIAosNConfiguration,
            IsGGsmaRcsTelephonyFeatureTagUsedAsAvailableVoiceCallType())
            .Times(AnyNumber())
            .WillRepeatedly(Return(IMS_FALSE));

    IMSVector<IMS_SINT32> objPolicy;
    objPolicy.Add(CarrierConfig::Assets::UNAVAILABLE_FEATURE_POLICY_CAPABILITY);
    EXPECT_CALL(m_objMockIAosNConfiguration, GetRegWithFeatureTagUnavailablePolicy())
            .Times(AnyNumber())
            .WillRepeatedly(ReturnRef(objPolicy));

    IMSVector<IMS_SINT32> objFeature;
    objFeature.Add(CarrierConfig::Assets::UNAVAILABLE_FEATURE_TYPE_SMS);
    EXPECT_CALL(m_objMockIAosNConfiguration, GetRegWithFeatureTagUnavailable())
            .Times(AnyNumber())
            .WillRepeatedly(ReturnRef(objFeature));

    ProcessNetworkChanged();
    EXPECT_TRUE(IsHandleBlocked(AosHandle::BLOCK_VOWIFI_CAPABILITY));
    EXPECT_TRUE(IsHandleBlocked(AosHandle::BLOCK_VIWIFI_CAPABILITY));
}

TEST_F(AosHandleMtcTest, ProcessNetworkChanged_Test6)
{
    // Test6: Capa=(LTE:voice,video / IWLAN:none / NR:none), network=NR
    //        unavailable policy for capa, unavailable feature for mmtel/video
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

    EXPECT_CALL(m_objMockIAosNetTracker, GetNetworkType())
            .Times(AnyNumber())
            .WillRepeatedly(Return(NW_REPORT_RADIO_NR));

    EXPECT_CALL(m_objMockIAosNConfiguration, IsWfcImsAvailable())
            .Times(AnyNumber())
            .WillRepeatedly(Return(IMS_TRUE));

    EXPECT_CALL(m_objMockIAosNConfiguration,
            IsGGsmaRcsTelephonyFeatureTagUsedAsAvailableVoiceCallType())
            .Times(AnyNumber())
            .WillRepeatedly(Return(IMS_FALSE));

    IMSVector<IMS_SINT32> objPolicy;
    objPolicy.Add(CarrierConfig::Assets::UNAVAILABLE_FEATURE_POLICY_CAPABILITY);
    EXPECT_CALL(m_objMockIAosNConfiguration, GetRegWithFeatureTagUnavailablePolicy())
            .Times(AnyNumber())
            .WillRepeatedly(ReturnRef(objPolicy));

    IMSVector<IMS_SINT32> objFeature;
    objFeature.Add(CarrierConfig::Assets::UNAVAILABLE_FEATURE_TYPE_MMTEL);
    objFeature.Add(CarrierConfig::Assets::UNAVAILABLE_FEATURE_TYPE_VIDEO);
    EXPECT_CALL(m_objMockIAosNConfiguration, GetRegWithFeatureTagUnavailable())
            .Times(AnyNumber())
            .WillRepeatedly(ReturnRef(objFeature));

    ProcessNetworkChanged();
    EXPECT_FALSE(IsHandleBlockedBase());
}

TEST_F(AosHandleMtcTest, ProcessVopsStateChanged_Test1)
{
    // Test1: call active
    // Expectation: set/reset HoldingVopsState as the state

    EXPECT_CALL(m_objMockIAosCallTracker, IsNormalCallActive())
            .Times(AnyNumber())
            .WillRepeatedly(Return(IMS_TRUE));

    ProcessVopsStateChanged(IMS_VOICE_OVER_PS_NOT_SUPPORTED);
    EXPECT_EQ(GetHoldingVopsState(), IMS_VOICE_OVER_PS_NOT_SUPPORTED);

    ProcessVopsStateChanged(IMS_VOICE_OVER_PS_SUPPORTED);
    EXPECT_EQ(GetHoldingVopsState(), IMS_VOICE_OVER_PS_SUPPORTED);
}

TEST_F(AosHandleMtcTest, ProcessVopsStateChanged_Test2)
{
    // Test2: call idle, no unavailalble policy
    // Expectation: set/reset block BLOCK_VOPS as the state

    EXPECT_CALL(m_objMockIAosCallTracker, IsNormalCallActive())
            .Times(AnyNumber())
            .WillRepeatedly(Return(IMS_FALSE));

    IMSVector<IMS_SINT32> objPolicy;
    EXPECT_CALL(m_objMockIAosNConfiguration, GetRegWithFeatureTagUnavailablePolicy())
            .Times(AnyNumber())
            .WillRepeatedly(ReturnRef(objPolicy));

    IMSVector<IMS_SINT32> objFeature;
    EXPECT_CALL(m_objMockIAosNConfiguration, GetRegWithFeatureTagUnavailable())
            .Times(AnyNumber())
            .WillRepeatedly(ReturnRef(objFeature));

    EXPECT_CALL(m_objMockIAosNConfiguration, IsWfcImsAvailable())
            .Times(AnyNumber())
            .WillRepeatedly(Return(IMS_TRUE));

    EXPECT_CALL(m_objMockIAosNConfiguration,
            IsGGsmaRcsTelephonyFeatureTagUsedAsAvailableVoiceCallType())
            .Times(AnyNumber())
            .WillRepeatedly(Return(IMS_FALSE));

    ProcessVopsStateChanged(IMS_VOICE_OVER_PS_NOT_SUPPORTED);
    EXPECT_TRUE(IsHandleBlocked(AosHandle::BLOCK_VOPS));

    ProcessVopsStateChanged(IMS_VOICE_OVER_PS_SUPPORTED);
    EXPECT_FALSE(IsHandleBlocked(AosHandle::BLOCK_VOPS));
}

TEST_F(AosHandleMtcTest, ProcessVopsStateChanged_Test3)
{
    // Test3: call idle, unavailalble policy for vops, no unavailable feature.
    // Expectation: set/reset block BLOCK_VOPS as the state

    EXPECT_CALL(m_objMockIAosCallTracker, IsNormalCallActive())
            .Times(AnyNumber())
            .WillRepeatedly(Return(IMS_FALSE));

    IMSVector<IMS_SINT32> objPolicy;
    objPolicy.Add(CarrierConfig::Assets::UNAVAILABLE_FEATURE_POLICY_VOPS);
    EXPECT_CALL(m_objMockIAosNConfiguration, GetRegWithFeatureTagUnavailablePolicy())
            .Times(AnyNumber())
            .WillRepeatedly(ReturnRef(objPolicy));

    IMSVector<IMS_SINT32> objFeature;
    EXPECT_CALL(m_objMockIAosNConfiguration, GetRegWithFeatureTagUnavailable())
            .Times(AnyNumber())
            .WillRepeatedly(ReturnRef(objFeature));

    EXPECT_CALL(m_objMockIAosNConfiguration, IsWfcImsAvailable())
            .Times(AnyNumber())
            .WillRepeatedly(Return(IMS_TRUE));

    EXPECT_CALL(m_objMockIAosNConfiguration,
            IsGGsmaRcsTelephonyFeatureTagUsedAsAvailableVoiceCallType())
            .Times(AnyNumber())
            .WillRepeatedly(Return(IMS_FALSE));

    ProcessVopsStateChanged(IMS_VOICE_OVER_PS_NOT_SUPPORTED);
    EXPECT_TRUE(IsHandleBlocked(AosHandle::BLOCK_VOPS));

    ProcessVopsStateChanged(IMS_VOICE_OVER_PS_SUPPORTED);
    EXPECT_FALSE(IsHandleBlocked(AosHandle::BLOCK_VOPS));
}

TEST_F(AosHandleMtcTest, ProcessVopsStateChanged_Test4)
{
    // Test4: call idle, unavailalble policy for vops, no unavailable feature.
    // Expectation: no block BLOCK_VOPS

    EXPECT_CALL(m_objMockIAosCallTracker, IsNormalCallActive())
            .Times(AnyNumber())
            .WillRepeatedly(Return(IMS_FALSE));

    IMSVector<IMS_SINT32> objPolicy;
    objPolicy.Add(CarrierConfig::Assets::UNAVAILABLE_FEATURE_POLICY_VOPS);
    EXPECT_CALL(m_objMockIAosNConfiguration, GetRegWithFeatureTagUnavailablePolicy())
            .Times(AnyNumber())
            .WillRepeatedly(ReturnRef(objPolicy));

    IMSVector<IMS_SINT32> objFeature;
    objFeature.Add(CarrierConfig::Assets::UNAVAILABLE_FEATURE_TYPE_MMTEL);
    objFeature.Add(CarrierConfig::Assets::UNAVAILABLE_FEATURE_TYPE_VIDEO);
    EXPECT_CALL(m_objMockIAosNConfiguration, GetRegWithFeatureTagUnavailable())
            .Times(AnyNumber())
            .WillRepeatedly(ReturnRef(objFeature));

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

TEST_F(AosHandleMtcTest, ProcessUnavailableFeatureForVops_Test1)
{
    // Test1: no unavailable feature
    // Expectation: return false regardless of the arg.

    IMSVector<IMS_SINT32> objFeature;
    EXPECT_CALL(m_objMockIAosNConfiguration, GetRegWithFeatureTagUnavailable())
            .Times(AnyNumber())
            .WillRepeatedly(ReturnRef(objFeature));

    EXPECT_FALSE(ProcessUnavailableFeatureForVops(IMS_VOICE_OVER_PS_NOT_SUPPORTED));
    EXPECT_FALSE(ProcessUnavailableFeatureForVops(IMS_VOICE_OVER_PS_SUPPORTED));
}

TEST_F(AosHandleMtcTest, ProcessUnavailableFeatureForVops_Test2)
{
    // Test2: unavailable feature for invalid
    // Expectation: return false regardless of the arg.

    IMSVector<IMS_SINT32> objFeature;
    objFeature.Add(0);
    EXPECT_CALL(m_objMockIAosNConfiguration, GetRegWithFeatureTagUnavailable())
            .Times(AnyNumber())
            .WillRepeatedly(ReturnRef(objFeature));

    EXPECT_FALSE(ProcessUnavailableFeatureForVops(IMS_VOICE_OVER_PS_NOT_SUPPORTED));
    EXPECT_FALSE(ProcessUnavailableFeatureForVops(IMS_VOICE_OVER_PS_SUPPORTED));
}

TEST_F(AosHandleMtcTest, ProcessUnavailableFeatureForVops_Test3)
{
    // Test3: unavailable feature for mmtel/video
    // Expectation: return true regardless of the arg.

    IMSVector<IMS_SINT32> objFeature;
    objFeature.Add(CarrierConfig::Assets::UNAVAILABLE_FEATURE_TYPE_MMTEL);
    objFeature.Add(CarrierConfig::Assets::UNAVAILABLE_FEATURE_TYPE_VIDEO);
    EXPECT_CALL(m_objMockIAosNConfiguration, GetRegWithFeatureTagUnavailable())
            .Times(AnyNumber())
            .WillRepeatedly(ReturnRef(objFeature));

    EXPECT_TRUE(ProcessUnavailableFeatureForVops(IMS_VOICE_OVER_PS_NOT_SUPPORTED));
    EXPECT_TRUE(ProcessUnavailableFeatureForVops(IMS_VOICE_OVER_PS_SUPPORTED));
}