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
#include "IImsRadio.h"
#include "INetworkWatcher.h"
#include "ITimer.h"
#include "PlatformContext.h"

#include "handle/AosHandle.h"
#include "handle/AosHandleMtc.h"
#include "interface/IAosAppContext.h"
#include "interface/IAosNConfiguration.h"
#include "interface/IAosRegistration.h"
#include "provider/AosProvider.h"
#include "provider/AosString.h"
#include "provider/AosUtil.h"

#include "interface/MockIAosAppContext.h"
#include "interface/MockIAosApplication.h"
#include "interface/MockIAosCallTracker.h"
#include "interface/MockIAosConnection.h"
#include "interface/MockIAosHandle.h"
#include "interface/MockIAosNConfiguration.h"
#include "interface/MockIAosNetTracker.h"
#include "interface/MockIAosRegistration.h"

#include "../../interface/aos/MockIAosService.h"
#include "../../interface/aos/MockIImsAosListener.h"
#include "../../../platform/interface/TestImsRadioService.h"

using ::testing::_;
using ::testing::AnyNumber;
using ::testing::Return;
using ::testing::ReturnRef;

#define DECLARE_USING(Base)       \
    using Base::IsBlockForMobile; \
    using Base::IsBlockForWifi;   \
    using Base::Request;

class TestAosHandleMtc : public AosHandleMtc
{
public:
    DECLARE_USING(AosHandleMtc)

    inline TestAosHandleMtc(IN IAosAppContext* piAppContext, IN const AString& strAppId,
            IN const AString& strServiceId, IN const IMS_SINT32 nServiceType) :
            AosHandleMtc(piAppContext, strAppId, strServiceId, nServiceType)
    {
    }
};

class AosHandleMtcTest : public ::testing::Test
{
public:
    TestAosHandleMtc* m_pAosHandleMtc;

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

    IAosService* m_piAosService;
    MockIAosService m_objMockIAosService;

    PlatformService* m_pPlatformService;
    TestImsRadioService m_objTestImsRadioService;

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
                .WillRepeatedly(Return(IMS_FALSE));

        EXPECT_CALL(m_objMockIAosNConfiguration, IsWfcImsAvailable())
                .Times(AnyNumber())
                .WillRepeatedly(Return(IMS_FALSE));

        EXPECT_CALL(m_objMockIAosNConfiguration,
                IsGGsmaRcsTelephonyFeatureTagUsedAsAvailableVoiceCallType())
                .Times(AnyNumber())
                .WillRepeatedly(Return(IMS_FALSE));

        m_piAosService = AosProvider::GetInstance()->GetService();
        AosProvider::GetInstance()->SetService(static_cast<IAosService*>(&m_objMockIAosService));

        m_pPlatformService =
                PlatformContext::GetInstance()->GetService(PlatformContext::SERVICE_RADIO);
        PlatformContext::GetInstance()->SetService(
                PlatformContext::SERVICE_RADIO, &m_objTestImsRadioService);

        m_pAosHandleMtc = new TestAosHandleMtc(
                &m_objMockIAosAppContext, strAppId, strServiceId, nServiceType);

        ASSERT_TRUE(m_pAosHandleMtc != nullptr);

        m_pAosHandleMtc->m_bDataConnected = IMS_TRUE;
        m_pAosHandleMtc->m_bVopsIgnoredForVolteEnabled = IMS_FALSE;
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
        AosProvider::GetInstance()->SetService(m_piAosService);
        PlatformContext::GetInstance()->SetService(
                PlatformContext::SERVICE_RADIO, m_pPlatformService);
    }

    ImsMap<IMS_UINT32, IMS_UINT32> GetCapabilities() { return m_pAosHandleMtc->m_objCapabilities; }

    void SetCapabilities(IN const ImsMap<IMS_UINT32, IMS_UINT32>& objNewCapabilities)
    {
        m_pAosHandleMtc->m_objCapabilities = objNewCapabilities;
    }

    void SetHoldingVopsState(IN IMS_UINT32 nState)
    {
        m_pAosHandleMtc->m_nHoldingVopsState = nState;
    }

    IMS_UINT32 GetHoldingVopsState() { return m_pAosHandleMtc->m_nHoldingVopsState; }

    IMS_UINT32 GetVopsState() { return m_pAosHandleMtc->m_nVopsState; }

    void SetVopsState(IN IMS_UINT32 nState) { m_pAosHandleMtc->m_nVopsState = nState; }

    void SetVopsIgnoredForVolteEnabled(IN IMS_BOOL bIgnored)
    {
        m_pAosHandleMtc->m_bVopsIgnoredForVolteEnabled = bIgnored;
    }

    IMS_BOOL IsVopsIgnoredForVolteEnabled()
    {
        return m_pAosHandleMtc->m_bVopsIgnoredForVolteEnabled;
    }

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

    void InitializeHoldingBlocksPolicy() { m_pAosHandleMtc->InitializeHoldingBlocksPolicy(); }

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

    void ProcessCapabilitiesChanged(IN const ImsMap<IMS_UINT32, IMS_UINT32>& objNewCapabilities)
    {
        m_pAosHandleMtc->ProcessCapabilitiesChanged(objNewCapabilities);
    }

    IMS_BOOL IsEqualCapabilities(IN const ImsMap<IMS_UINT32, IMS_UINT32>& objSrcCapabilities,
            IN const ImsMap<IMS_UINT32, IMS_UINT32>& objDestCapabilities)
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

    void ProcessVopsStateChanged(IN IMS_UINT32 nState, IN IMS_BOOL bUpdateState = IMS_TRUE)
    {
        m_pAosHandleMtc->ProcessVopsStateChanged(nState, bUpdateState);
    }

    void SetDataConnected(IN IMS_BOOL bConnected)
    {
        m_pAosHandleMtc->m_bDataConnected = bConnected;
    }

    IMS_BOOL IsDataConnected() { return m_pAosHandleMtc->m_bDataConnected; }

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

    ImsList<IMS_UINT32> GetHoldingBlocksPolicyForWifi()
    {
        return m_pAosHandleMtc->m_objHoldingBlocksPolicyForWifi;
    }

    void NConfiguration_NotifyConfigChanged()
    {
        m_pAosHandleMtc->NConfiguration_NotifyConfigChanged();
    }

    void ImsRadio_OnSsacChanged(IN const SsacInfo& objSsacInfo)
    {
        m_pAosHandleMtc->ImsRadio_OnSsacChanged(objSsacInfo);
    }

    IMS_BOOL IsPlmnBlockCondition() { return m_pAosHandleMtc->IsPlmnBlockCondition(); }

    void SetRoamingState(IN IMS_UINT32 nState) { m_pAosHandleMtc->m_nRoamingState = nState; }

    void SetCsVoiceAvailable(IN IMS_BOOL bIsCsVoiceAvailable)
    {
        m_pAosHandleMtc->m_bCsVoiceAvailable = bIsCsVoiceAvailable;
    }

    void SetSsacBarred(IN IMS_BOOL bIsSsacBarred)
    {
        m_pAosHandleMtc->m_bSsacBarred = bIsSsacBarred;
    }

    void SetSsacHeld(IN IMS_BOOL bIsSsacHeld) { m_pAosHandleMtc->m_bSsacHeld = bIsSsacHeld; }

    IMS_BOOL IsSsacBarred() { return m_pAosHandleMtc->m_bSsacBarred; }

    IMS_BOOL IsSsacHeld() { return m_pAosHandleMtc->m_bSsacHeld; }

    IMS_BOOL ProcessHoldingVopsState(IN IMS_UINT32 nState)
    {
        return m_pAosHandleMtc->ProcessHoldingVopsState(nState);
    }

    IMS_BOOL ProcessHoldingSsacState(IN IMS_SINT32 nBarringFactorForVoice)
    {
        return m_pAosHandleMtc->ProcessHoldingSsacState(nBarringFactorForVoice);
    }

    void ProcessVolteHysTimerExpired() { m_pAosHandleMtc->ProcessVolteHysTimerExpired(); }

    IMS_BOOL StartVolteHysTimer(IN IMS_UINT32 nDuration)
    {
        return m_pAosHandleMtc->StartVolteHysTimer(nDuration);
    }

    void StopVolteHysTimer() { m_pAosHandleMtc->StopVolteHysTimer(); }

    IMS_BOOL IsVolteHysTimerRunning() { return m_pAosHandleMtc->IsVolteHysTimerRunning(); }

    void ServicePhone_PlmnChanged() { m_pAosHandleMtc->ServicePhone_PlmnChanged(); }

    ITimer* GetVolteHysTimer() { return m_pAosHandleMtc->m_piVolteHysTimer; }

    void Timer_TimerExpired(IN ITimer* piTimer) { m_pAosHandleMtc->Timer_TimerExpired(piTimer); }

    inline void SetServiceType(IN IMS_UINT32 nServiceType)
    {
        m_pAosHandleMtc->m_nServiceType = nServiceType;
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
    EXPECT_TRUE((GetCapabilities().GetValue(static_cast<IMS_UINT32>(AosNetworkType::LTE)) &
                        static_cast<IMS_UINT32>(AosCapability::CALL_COMPOSER)) > 0);
    EXPECT_TRUE((GetCapabilities().GetValue(static_cast<IMS_UINT32>(AosNetworkType::IWLAN)) &
                        static_cast<IMS_UINT32>(AosCapability::VOICE)) > 0);
    EXPECT_TRUE((GetCapabilities().GetValue(static_cast<IMS_UINT32>(AosNetworkType::IWLAN)) &
                        static_cast<IMS_UINT32>(AosCapability::VIDEO)) > 0);
    EXPECT_TRUE((GetCapabilities().GetValue(static_cast<IMS_UINT32>(AosNetworkType::NR)) &
                        static_cast<IMS_UINT32>(AosCapability::VOICE)) > 0);
    EXPECT_TRUE((GetCapabilities().GetValue(static_cast<IMS_UINT32>(AosNetworkType::NR)) &
                        static_cast<IMS_UINT32>(AosCapability::VIDEO)) > 0);
    EXPECT_TRUE((GetCapabilities().GetValue(static_cast<IMS_UINT32>(AosNetworkType::NR)) &
                        static_cast<IMS_UINT32>(AosCapability::CALL_COMPOSER)) > 0);
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
    // Test4: Holding vops and ssac for call test.
    //        call type is normal / call state is idle / vops and ssac held for call
    //        / no unavailable feature policy
    // Expectation: Plmn block if the condition is met. Add vops and ssac block to main blocks.

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
    SetSsacHeld(IMS_TRUE);

    EXPECT_CALL(m_objMockIAosNConfiguration, IsPlmnBlockWithTimeoutOnVoiceCallUnavailable())
            .Times(AnyNumber())
            .WillRepeatedly(Return(IMS_TRUE));

    EXPECT_CALL(m_objMockIAosApplication, RequestCmd(ImsAosControl::PLMN_BLOCK_WITH_TIMEOUT, 0))
            .Times(1);

    m_pAosHandleMtc->CallTracker_StateChanged(IAosCallTracker::TYPE_NORMAL, CallState::IDLE);

    EXPECT_FALSE(m_pAosHandleMtc->GetFeatureTagList().HasUnavailableFeature(ImsAosFeature::MMTEL));
    EXPECT_FALSE(m_pAosHandleMtc->GetFeatureTagList().HasUnavailableFeature(ImsAosFeature::VIDEO));
    EXPECT_EQ(GetVopsState(), IMS_VOICE_OVER_PS_NOT_SUPPORTED);
    EXPECT_EQ(GetHoldingVopsState(), IMS_VOICE_OVER_PS_SUPPORTED);
    EXPECT_TRUE(IsHandleBlocked(AosHandle::BLOCK_VOPS));
    EXPECT_TRUE(IsSsacBarred());
    EXPECT_FALSE(IsSsacHeld());
    EXPECT_TRUE(IsHandleBlocked(AosHandle::BLOCK_SSAC));

    // reset condiitons for plmn block
    SetVopsState(IMS_VOICE_OVER_PS_SUPPORTED);
    SetHoldingVopsState(IMS_VOICE_OVER_PS_NOT_SUPPORTED);
    SetSsacBarred(IMS_FALSE);
    SetSsacHeld(IMS_TRUE);
    SetRoamingState(IMS_ROAMING_STATE_ON);
    SetCsVoiceAvailable(IMS_TRUE);

    m_pAosHandleMtc->CallTracker_StateChanged(IAosCallTracker::TYPE_NORMAL, CallState::IDLE);

    EXPECT_FALSE(m_pAosHandleMtc->GetFeatureTagList().HasUnavailableFeature(ImsAosFeature::MMTEL));
    EXPECT_FALSE(m_pAosHandleMtc->GetFeatureTagList().HasUnavailableFeature(ImsAosFeature::VIDEO));
    EXPECT_EQ(GetVopsState(), IMS_VOICE_OVER_PS_NOT_SUPPORTED);
    EXPECT_EQ(GetHoldingVopsState(), IMS_VOICE_OVER_PS_SUPPORTED);
    EXPECT_TRUE(IsHandleBlocked(AosHandle::BLOCK_VOPS));
    EXPECT_TRUE(IsSsacBarred());
    EXPECT_FALSE(IsSsacHeld());
    EXPECT_TRUE(IsHandleBlocked(AosHandle::BLOCK_SSAC));
}

TEST_F(AosHandleMtcTest, CallTracker_StateChanged_Test5)
{
    // Test5: Holding vops and ssac for call test 2.
    //        call type is normal / call state is idle / vops and ssac held for call
    //        / unavailable feature policy
    // Expectation: No add vops and ssac block to main blocks. No Plmn block.

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
    SetSsacHeld(IMS_TRUE);

    EXPECT_CALL(m_objMockIAosApplication, RequestCmd(ImsAosControl::PLMN_BLOCK_WITH_TIMEOUT, 0))
            .Times(0);

    m_pAosHandleMtc->CallTracker_StateChanged(IAosCallTracker::TYPE_NORMAL, CallState::IDLE);

    EXPECT_TRUE(m_pAosHandleMtc->GetFeatureTagList().HasUnavailableFeature(ImsAosFeature::MMTEL));
    EXPECT_TRUE(m_pAosHandleMtc->GetFeatureTagList().HasUnavailableFeature(ImsAosFeature::VIDEO));
    EXPECT_EQ(GetVopsState(), IMS_VOICE_OVER_PS_NOT_SUPPORTED);
    EXPECT_EQ(GetHoldingVopsState(), IMS_VOICE_OVER_PS_SUPPORTED);
    EXPECT_FALSE(IsHandleBlocked(AosHandle::BLOCK_VOPS));
    EXPECT_TRUE(IsSsacBarred());
    EXPECT_FALSE(IsSsacHeld());
    EXPECT_FALSE(IsHandleBlocked(AosHandle::BLOCK_SSAC));
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

TEST_F(AosHandleMtcTest, NetTracker_StatusChanged_WifiTest)
{
    IMS_BOOL bIsWifiTest = AosUtil::GetInstance()->IsWifiTest();
    AosUtil::GetInstance()->SetWifiTest(IMS_TRUE);

    EXPECT_CALL(m_objMockIAosNetTracker, IsSuspended()).Times(0);
    EXPECT_CALL(m_objMockIAosNetTracker, GetNetworkType()).Times(0);

    m_pAosHandleMtc->NetTracker_StatusChanged();

    AosUtil::GetInstance()->SetWifiTest(bIsWifiTest);
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

TEST_F(AosHandleMtcTest, InitializeHoldingBlocksPolicy_Test)
{
    EXPECT_CALL(m_objMockIAosNConfiguration, IsDeregOn3gNetwork())
            .Times(AnyNumber())
            .WillRepeatedly(Return(IMS_TRUE));

    InitializeHoldingBlocksPolicy();

    EXPECT_TRUE(m_pAosHandleMtc->IsBlockForMobile(AosHandle::BLOCK_VOLTE_CAPABILITY));
    EXPECT_TRUE(m_pAosHandleMtc->IsBlockForMobile(AosHandle::BLOCK_VILTE_CAPABILITY));
    EXPECT_TRUE(m_pAosHandleMtc->IsBlockForMobile(AosHandle::BLOCK_VOPS));
    EXPECT_TRUE(m_pAosHandleMtc->IsBlockForMobile(AosHandle::BLOCK_SSAC));
    EXPECT_TRUE(m_pAosHandleMtc->IsBlockForMobile(AosHandle::BLOCK_3G));
    EXPECT_TRUE(m_pAosHandleMtc->IsBlockForWifi(AosHandle::BLOCK_VOWIFI_CAPABILITY));
    EXPECT_TRUE(m_pAosHandleMtc->IsBlockForWifi(AosHandle::BLOCK_VIWIFI_CAPABILITY));

    EXPECT_FALSE(m_pAosHandleMtc->IsBlockForWifi(AosHandle::BLOCK_VOLTE_CAPABILITY));
    EXPECT_FALSE(m_pAosHandleMtc->IsBlockForWifi(AosHandle::BLOCK_VILTE_CAPABILITY));
    EXPECT_FALSE(m_pAosHandleMtc->IsBlockForWifi(AosHandle::BLOCK_VOPS));
    EXPECT_FALSE(m_pAosHandleMtc->IsBlockForWifi(AosHandle::BLOCK_SSAC));
    EXPECT_FALSE(m_pAosHandleMtc->IsBlockForWifi(AosHandle::BLOCK_3G));
    EXPECT_FALSE(m_pAosHandleMtc->IsBlockForMobile(AosHandle::BLOCK_VOWIFI_CAPABILITY));
    EXPECT_FALSE(m_pAosHandleMtc->IsBlockForMobile(AosHandle::BLOCK_VIWIFI_CAPABILITY));
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
    EXPECT_TRUE(m_pAosHandleMtc->GetFeatureTagList().HasFeature(
            ImsAosFeature::CALL_COMPOSER_VIA_TELEPHONY));
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
    AddBlock(AosHandle::BLOCK_CALL_COMPOSER_CAPABILITY);

    EXPECT_CALL(m_objMockIAosNConfiguration, IsVerstatForRegistrationSupported())
            .Times(AnyNumber())
            .WillRepeatedly(Return(IMS_FALSE));

    EXPECT_CALL(m_objMockIAosNConfiguration, GetUssdMethod())
            .Times(AnyNumber())
            .WillRepeatedly(Return(CarrierConfig::USSD_OVER_CS_ONLY));

    InitializeServiceFeature();

    EXPECT_FALSE(m_pAosHandleMtc->GetFeatureTagList().HasFeature(ImsAosFeature::MMTEL));
    EXPECT_FALSE(m_pAosHandleMtc->GetFeatureTagList().HasFeature(ImsAosFeature::VIDEO));
    EXPECT_FALSE(m_pAosHandleMtc->GetFeatureTagList().HasFeature(
            ImsAosFeature::CALL_COMPOSER_VIA_TELEPHONY));
    EXPECT_FALSE(m_pAosHandleMtc->GetFeatureTagList().HasFeature(ImsAosFeature::VERSTAT));
    EXPECT_FALSE(m_pAosHandleMtc->GetFeatureTagList().HasFeature(ImsAosFeature::USSI));
}

TEST_F(AosHandleMtcTest,
        TextFeatureShouldBeAddedToTheListIfTextCapabilityIsNotBlockedWhenInitializeServiceFeature)
{
    // WHEN
    InitializeServiceFeature();

    // THEN
    EXPECT_TRUE(m_pAosHandleMtc->GetFeatureTagList().HasFeature(ImsAosFeature::TEXT));
}

TEST_F(AosHandleMtcTest,
        TextFeatureShouldNotBeAddedToTheListIfTextCapabilityIsBlockedWhenInitializeServiceFeature)
{
    // GIVEN
    AddBlock(AosHandle::BLOCK_TEXT_CAPABILITY);

    // WHEN
    InitializeServiceFeature();

    // THEN
    EXPECT_FALSE(m_pAosHandleMtc->GetFeatureTagList().HasFeature(ImsAosFeature::TEXT));
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

TEST_F(AosHandleMtcTest,
        ShouldBlockMmtelFeatureIfRegModeChangedToLimitedWhileMmtelIsUnavailableInLimitedReg)
{
    // GIVEN
    ImsVector<IMS_SINT32> objUnavailableFeatures;
    objUnavailableFeatures.Add(CarrierConfig::Assets::REG_FEATURE_MMTEL);
    ON_CALL(m_objMockIAosNConfiguration, GetUnavailableFeaturesInLimitedReg())
            .WillByDefault(ReturnRef(objUnavailableFeatures));

    // WHEN
    m_pAosHandleMtc->Request(IAosHandle::TYPE_LIMITED_MODE, IAosHandle::STATE_ADD);

    // THEN
    EXPECT_TRUE(IsHandleBlocked(AosHandle::BLOCK_LIMITED_MMTEL));
}

TEST_F(AosHandleMtcTest,
        ShouldUnblockMmtelFeatureIfRegModeChangedToNormalWhileMmtelIsUnavailableInLimitedReg)
{
    // GIVEN
    ImsVector<IMS_SINT32> objUnavailableFeatures;
    objUnavailableFeatures.Add(CarrierConfig::Assets::REG_FEATURE_MMTEL);
    ON_CALL(m_objMockIAosNConfiguration, GetUnavailableFeaturesInLimitedReg())
            .WillByDefault(ReturnRef(objUnavailableFeatures));
    AddBlock(AosHandle::BLOCK_LIMITED_MMTEL);

    // WHEN
    m_pAosHandleMtc->Request(IAosHandle::TYPE_LIMITED_MODE, IAosHandle::STATE_REMOVE);

    // THEN
    EXPECT_FALSE(IsHandleBlocked(AosHandle::BLOCK_LIMITED_MMTEL));
}

TEST_F(AosHandleMtcTest,
        ShouldBlockVideoFeatureIfRegModeChangedToLimitedWhileVideoIsUnavailableInLimitedReg)
{
    // GIVEN
    ImsVector<IMS_SINT32> objUnavailableFeatures;
    objUnavailableFeatures.Add(CarrierConfig::Assets::REG_FEATURE_VIDEO);
    ON_CALL(m_objMockIAosNConfiguration, GetUnavailableFeaturesInLimitedReg())
            .WillByDefault(ReturnRef(objUnavailableFeatures));

    // WHEN
    m_pAosHandleMtc->Request(IAosHandle::TYPE_LIMITED_MODE, IAosHandle::STATE_ADD);

    // THEN
    EXPECT_TRUE(IsHandleBlocked(AosHandle::BLOCK_LIMITED_VIDEO));
}

TEST_F(AosHandleMtcTest,
        ShouldUnblockVideoFeatureIfRegModeChangedToNormalWhileVideoIsUnavailableInLimitedReg)
{
    // GIVEN
    ImsVector<IMS_SINT32> objUnavailableFeatures;
    objUnavailableFeatures.Add(CarrierConfig::Assets::REG_FEATURE_VIDEO);
    ON_CALL(m_objMockIAosNConfiguration, GetUnavailableFeaturesInLimitedReg())
            .WillByDefault(ReturnRef(objUnavailableFeatures));
    AddBlock(AosHandle::BLOCK_LIMITED_VIDEO);

    // WHEN
    m_pAosHandleMtc->Request(IAosHandle::TYPE_LIMITED_MODE, IAosHandle::STATE_REMOVE);

    // THEN
    EXPECT_FALSE(IsHandleBlocked(AosHandle::BLOCK_LIMITED_VIDEO));
}

TEST_F(AosHandleMtcTest,
        ShouldBlockTextFeatureIfRegModeChangedToLimitedWhileTextIsUnavailableInLimitedReg)
{
    // GIVEN
    ImsVector<IMS_SINT32> objUnavailableFeatures;
    objUnavailableFeatures.Add(CarrierConfig::Assets::REG_FEATURE_TEXT);
    ON_CALL(m_objMockIAosNConfiguration, GetUnavailableFeaturesInLimitedReg())
            .WillByDefault(ReturnRef(objUnavailableFeatures));

    // WHEN
    m_pAosHandleMtc->Request(IAosHandle::TYPE_LIMITED_MODE, IAosHandle::STATE_ADD);

    // THEN
    EXPECT_TRUE(IsHandleBlocked(AosHandle::BLOCK_LIMITED_TEXT));
}

TEST_F(AosHandleMtcTest,
        ShouldUnblockTextFeatureIfRegModeChangedToNormalWhileTextIsUnavailableInLimitedReg)
{
    // GIVEN
    ImsVector<IMS_SINT32> objUnavailableFeatures;
    objUnavailableFeatures.Add(CarrierConfig::Assets::REG_FEATURE_TEXT);
    ON_CALL(m_objMockIAosNConfiguration, GetUnavailableFeaturesInLimitedReg())
            .WillByDefault(ReturnRef(objUnavailableFeatures));
    AddBlock(AosHandle::BLOCK_LIMITED_TEXT);

    // WHEN
    m_pAosHandleMtc->Request(IAosHandle::TYPE_LIMITED_MODE, IAosHandle::STATE_REMOVE);

    // THEN
    EXPECT_FALSE(IsHandleBlocked(AosHandle::BLOCK_LIMITED_TEXT));
}

TEST_F(AosHandleMtcTest, CheckSuspended_WifiTest)
{
    IMS_BOOL bIsWifiTest = AosUtil::GetInstance()->IsWifiTest();
    AosUtil::GetInstance()->SetWifiTest(IMS_TRUE);

    EXPECT_CALL(m_objMockIAosAppContext, GetNetTracker()).Times(0);
    EXPECT_CALL(m_objMockIAosNetTracker, IsSuspended()).Times(0);
    EXPECT_CALL(m_objMockIAosNetTracker, GetNetworkType()).Times(0);

    CheckSuspended();

    AosUtil::GetInstance()->SetWifiTest(bIsWifiTest);
}

TEST_F(AosHandleMtcTest, CheckSuspended_Test1)
{
    // Test1: suspended, valid network(LTE)
    // Expectation: set SUSPEND_NO_SERVICE

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
            .WillRepeatedly(Return(IMS_TRUE));

    EXPECT_CALL(m_objTestImsRadioService.GetMockImsRadio(),
            AddListenerForSsac(DYNAMIC_CAST(IImsRadioSsacListener*, m_pAosHandleMtc)))
            .Times(1);

    EXPECT_CALL(m_objMockIAosService,
            AddListener(DYNAMIC_CAST(IAosServicePhoneListener*, m_pAosHandleMtc)))
            .Times(1);

    EXPECT_CALL(m_objTestImsRadioService.GetMockImsRadio(),
            RemoveListenerForSsac(DYNAMIC_CAST(IImsRadioSsacListener*, m_pAosHandleMtc)))
            .Times(1);

    EXPECT_CALL(m_objMockIAosService,
            RemoveListener(DYNAMIC_CAST(IAosServicePhoneListener*, m_pAosHandleMtc)))
            .Times(1);

    Init();

    EXPECT_EQ(m_pAosHandleMtc->GetFeatureTagList().GetFeatures(),
            (ImsAosFeature::MMTEL | ImsAosFeature::VIDEO | ImsAosFeature::TEXT |
                    ImsAosFeature::CALL_COMPOSER_VIA_TELEPHONY));

    EXPECT_TRUE(m_pAosHandleMtc->GetFeatureTagList().HasFeatureTag(FeatureTags::CDMALESS));
    EXPECT_TRUE(m_pAosHandleMtc->GetFeatureTagList().HasFeatureTag(
            FeatureTags::RCS_TELEPHONY, AosString::STR_CS));
    EXPECT_TRUE(m_pAosHandleMtc->GetFeatureTagList().HasFeatureTag(
            FeatureTags::RCS_TELEPHONY, AosString::STR_VOLTE));

    EXPECT_TRUE(m_pAosHandleMtc->IsBlockForMobile(AosHandle::BLOCK_VOLTE_CAPABILITY));
    EXPECT_TRUE(m_pAosHandleMtc->IsBlockForMobile(AosHandle::BLOCK_VILTE_CAPABILITY));
    EXPECT_TRUE(m_pAosHandleMtc->IsBlockForMobile(AosHandle::BLOCK_VOPS));
    EXPECT_TRUE(m_pAosHandleMtc->IsBlockForWifi(AosHandle::BLOCK_VOWIFI_CAPABILITY));
    EXPECT_TRUE(m_pAosHandleMtc->IsBlockForWifi(AosHandle::BLOCK_VIWIFI_CAPABILITY));

    EXPECT_FALSE(m_pAosHandleMtc->IsBlockForWifi(AosHandle::BLOCK_VOLTE_CAPABILITY));
    EXPECT_FALSE(m_pAosHandleMtc->IsBlockForWifi(AosHandle::BLOCK_VILTE_CAPABILITY));
    EXPECT_FALSE(m_pAosHandleMtc->IsBlockForWifi(AosHandle::BLOCK_VOPS));
    EXPECT_FALSE(m_pAosHandleMtc->IsBlockForMobile(AosHandle::BLOCK_VOWIFI_CAPABILITY));
    EXPECT_FALSE(m_pAosHandleMtc->IsBlockForMobile(AosHandle::BLOCK_VIWIFI_CAPABILITY));

    EXPECT_FALSE(IsVopsIgnoredForVolteEnabled());

    StartVolteHysTimer(60);
    ASSERT_TRUE(IsVolteHysTimerRunning());
    CleanUp();
    EXPECT_FALSE(IsVolteHysTimerRunning());
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
    //        ViWiFi without Voice supported
    // Expectation: return false

    SetEpdgEnabled(IMS_TRUE);
    AddBlock(AosHandle::BLOCK_VOWIFI_CAPABILITY);

    EXPECT_CALL(m_objMockIAosNConfiguration, IsVideoOverWifiSupportedWithoutVoice())
            .Times(AnyNumber())
            .WillRepeatedly(Return(IMS_TRUE));

    EXPECT_FALSE(IsHandleBlocked());
}

TEST_F(AosHandleMtcTest, IsHandleBlocked_Test4)
{
    // Test4: Epdg not enabled
    // Expectation: return true if BLOCK_VOPS or BLOCK_VOLTE_CAPABILITY or BLOCK_NETWORK or BLOCK_3G
    //              or BLOCK_SSAC
    //              else return false

    AddBlock(AosHandle::BLOCK_VOPS);
    AddBlock(AosHandle::BLOCK_VOLTE_CAPABILITY);
    AddBlock(AosHandle::BLOCK_NETWORK);
    AddBlock(AosHandle::BLOCK_3G);
    AddBlock(AosHandle::BLOCK_SSAC);
    EXPECT_TRUE(IsHandleBlocked());

    RemoveBlock(AosHandle::BLOCK_VOPS);
    EXPECT_TRUE(IsHandleBlocked());

    RemoveBlock(AosHandle::BLOCK_VOLTE_CAPABILITY);
    EXPECT_TRUE(IsHandleBlocked());

    RemoveBlock(AosHandle::BLOCK_NETWORK);
    EXPECT_TRUE(IsHandleBlocked());

    RemoveBlock(AosHandle::BLOCK_3G);
    EXPECT_TRUE(IsHandleBlocked());

    RemoveBlock(AosHandle::BLOCK_SSAC);
    EXPECT_FALSE(IsHandleBlocked());
}

TEST_F(AosHandleMtcTest, IsHandleBlocked_Test5)
{
    // Test5: Epdg enabled, BLOCK_VOWIFI_CAPABILITY
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

TEST_F(AosHandleMtcTest, DoNothingIfEmergencyServiceWhenCapabilityChanged)
{
    // GIVEN
    SetServiceType(ImsAosService::EMERGENCY_MTC);

    ImsMap<IMS_UINT32, IMS_UINT32> objNewCapabilities, objExpectedCapabilities;

    objNewCapabilities.Add(static_cast<IMS_UINT32>(AosNetworkType::LTE),
            static_cast<IMS_UINT32>(AosCapability::NONE));
    objNewCapabilities.Add(static_cast<IMS_UINT32>(AosNetworkType::IWLAN),
            static_cast<IMS_UINT32>(AosCapability::NONE));
    objNewCapabilities.Add(static_cast<IMS_UINT32>(AosNetworkType::NR),
            static_cast<IMS_UINT32>(AosCapability::NONE));
    objNewCapabilities.Add(static_cast<IMS_UINT32>(AosNetworkType::UTRAN),
            static_cast<IMS_UINT32>(AosCapability::NONE));

    objExpectedCapabilities.Add(static_cast<IMS_UINT32>(AosNetworkType::LTE),
            static_cast<IMS_UINT32>(AosCapability::VOICE) |
                    static_cast<IMS_UINT32>(AosCapability::VIDEO) |
                    static_cast<IMS_UINT32>(AosCapability::CALL_COMPOSER));
    objExpectedCapabilities.Add(static_cast<IMS_UINT32>(AosNetworkType::IWLAN),
            static_cast<IMS_UINT32>(AosCapability::VOICE) |
                    static_cast<IMS_UINT32>(AosCapability::VIDEO));
    objExpectedCapabilities.Add(static_cast<IMS_UINT32>(AosNetworkType::NR),
            static_cast<IMS_UINT32>(AosCapability::VOICE) |
                    static_cast<IMS_UINT32>(AosCapability::VIDEO) |
                    static_cast<IMS_UINT32>(AosCapability::CALL_COMPOSER));
    objExpectedCapabilities.Add(static_cast<IMS_UINT32>(AosNetworkType::UTRAN),
            static_cast<IMS_UINT32>(AosCapability::NONE));

    // WHEN
    ProcessCapabilitiesChanged(objNewCapabilities);

    // THEN
    EXPECT_TRUE(IsEqualCapabilities(GetCapabilities(), objExpectedCapabilities));
}

TEST_F(AosHandleMtcTest,
        BlockCallComposerCapabilityIfCallComposerBusinessOnlyIsRemovedWhileCallComposerIsNotExisted)
{
    // GIVEN
    SetNetworkType(NW_REPORT_RADIO_LTE);

    ImsMap<IMS_UINT32, IMS_UINT32> objCapabilities;
    objCapabilities.Add(static_cast<IMS_UINT32>(AosNetworkType::LTE),
            static_cast<IMS_UINT32>(AosCapability::VOICE) |
                    static_cast<IMS_UINT32>(AosCapability::VIDEO) |
                    static_cast<IMS_UINT32>(AosCapability::CALL_COMPOSER_BUSINESS_ONLY));
    SetCapabilities(objCapabilities);

    EXPECT_FALSE(IsHandleBlocked(AosHandle::BLOCK_CALL_COMPOSER_CAPABILITY));

    // WHEN
    objCapabilities.SetValue(static_cast<IMS_UINT32>(AosNetworkType::LTE),
            static_cast<IMS_UINT32>(AosCapability::VOICE) |
                    static_cast<IMS_UINT32>(AosCapability::VIDEO));
    ProcessCapabilitiesChanged(objCapabilities);

    // THEN
    EXPECT_TRUE(IsHandleBlocked(AosHandle::BLOCK_CALL_COMPOSER_CAPABILITY));
}

TEST_F(AosHandleMtcTest,
        UnblockCallComposerCapabilityIfCallComposerBusinessOnlyIsAddedWhileCallComposerIsNotExisted)
{
    // GIVEN
    SetNetworkType(NW_REPORT_RADIO_LTE);
    AddBlock(AosHandle::BLOCK_CALL_COMPOSER_CAPABILITY);

    ImsMap<IMS_UINT32, IMS_UINT32> objCapabilities;
    objCapabilities.Add(static_cast<IMS_UINT32>(AosNetworkType::LTE),
            static_cast<IMS_UINT32>(AosCapability::VOICE) |
                    static_cast<IMS_UINT32>(AosCapability::VIDEO));
    SetCapabilities(objCapabilities);

    // WHEN
    objCapabilities.SetValue(static_cast<IMS_UINT32>(AosNetworkType::LTE),
            static_cast<IMS_UINT32>(AosCapability::VOICE) |
                    static_cast<IMS_UINT32>(AosCapability::VIDEO) |
                    static_cast<IMS_UINT32>(AosCapability::CALL_COMPOSER_BUSINESS_ONLY));
    ProcessCapabilitiesChanged(objCapabilities);

    // THEN
    EXPECT_FALSE(IsHandleBlocked(AosHandle::BLOCK_CALL_COMPOSER_CAPABILITY));
}

TEST_F(AosHandleMtcTest,
        BlockCallComposerCapabilityIfBothCallComposerAndCallComposerBusinessOnlyAreRemoved)
{
    // GIVEN
    SetNetworkType(NW_REPORT_RADIO_LTE);

    ImsMap<IMS_UINT32, IMS_UINT32> objCapabilities;
    objCapabilities.Add(static_cast<IMS_UINT32>(AosNetworkType::LTE),
            static_cast<IMS_UINT32>(AosCapability::VOICE) |
                    static_cast<IMS_UINT32>(AosCapability::VIDEO) |
                    static_cast<IMS_UINT32>(AosCapability::CALL_COMPOSER) |
                    static_cast<IMS_UINT32>(AosCapability::CALL_COMPOSER_BUSINESS_ONLY));
    SetCapabilities(objCapabilities);

    EXPECT_FALSE(IsHandleBlocked(AosHandle::BLOCK_CALL_COMPOSER_CAPABILITY));

    // WHEN
    objCapabilities.SetValue(static_cast<IMS_UINT32>(AosNetworkType::LTE),
            static_cast<IMS_UINT32>(AosCapability::VOICE) |
                    static_cast<IMS_UINT32>(AosCapability::VIDEO));
    ProcessCapabilitiesChanged(objCapabilities);

    // THEN
    EXPECT_TRUE(IsHandleBlocked(AosHandle::BLOCK_CALL_COMPOSER_CAPABILITY));
}

TEST_F(AosHandleMtcTest,
        UnblockCallComposerCapabilityIfBothCallComposerAndCallComposerBusinessOnlyAreAdded)
{
    // GIVEN
    SetNetworkType(NW_REPORT_RADIO_LTE);
    AddBlock(AosHandle::BLOCK_CALL_COMPOSER_CAPABILITY);

    ImsMap<IMS_UINT32, IMS_UINT32> objCapabilities;
    objCapabilities.Add(static_cast<IMS_UINT32>(AosNetworkType::LTE),
            static_cast<IMS_UINT32>(AosCapability::VOICE) |
                    static_cast<IMS_UINT32>(AosCapability::VIDEO));
    SetCapabilities(objCapabilities);

    // WHEN
    objCapabilities.SetValue(static_cast<IMS_UINT32>(AosNetworkType::LTE),
            static_cast<IMS_UINT32>(AosCapability::VOICE) |
                    static_cast<IMS_UINT32>(AosCapability::VIDEO) |
                    static_cast<IMS_UINT32>(AosCapability::CALL_COMPOSER) |
                    static_cast<IMS_UINT32>(AosCapability::CALL_COMPOSER_BUSINESS_ONLY));
    ProcessCapabilitiesChanged(objCapabilities);

    // THEN
    EXPECT_FALSE(IsHandleBlocked(AosHandle::BLOCK_CALL_COMPOSER_CAPABILITY));
}

TEST_F(AosHandleMtcTest,
        NoChangeCallComposerCapabilityIfCallComposerBusinessOnlyIsRemovedWhileCallComposerIsExisted)
{
    // GIVEN
    SetNetworkType(NW_REPORT_RADIO_LTE);

    ImsMap<IMS_UINT32, IMS_UINT32> objCapabilities;
    objCapabilities.Add(static_cast<IMS_UINT32>(AosNetworkType::LTE),
            static_cast<IMS_UINT32>(AosCapability::VOICE) |
                    static_cast<IMS_UINT32>(AosCapability::VIDEO) |
                    static_cast<IMS_UINT32>(AosCapability::CALL_COMPOSER) |
                    static_cast<IMS_UINT32>(AosCapability::CALL_COMPOSER_BUSINESS_ONLY));
    SetCapabilities(objCapabilities);

    EXPECT_FALSE(IsHandleBlocked(AosHandle::BLOCK_CALL_COMPOSER_CAPABILITY));

    // WHEN
    objCapabilities.SetValue(static_cast<IMS_UINT32>(AosNetworkType::LTE),
            static_cast<IMS_UINT32>(AosCapability::VOICE) |
                    static_cast<IMS_UINT32>(AosCapability::VIDEO) |
                    static_cast<IMS_UINT32>(AosCapability::CALL_COMPOSER));
    ProcessCapabilitiesChanged(objCapabilities);

    // THEN
    EXPECT_FALSE(IsHandleBlocked(AosHandle::BLOCK_CALL_COMPOSER_CAPABILITY));
}

TEST_F(AosHandleMtcTest,
        NoChangeCallComposerCapabilityIfCallComposerIsRemovedWhileCallComposerBusinessOnlyIsExisted)
{
    // GIVEN
    SetNetworkType(NW_REPORT_RADIO_LTE);

    ImsMap<IMS_UINT32, IMS_UINT32> objCapabilities;
    objCapabilities.Add(static_cast<IMS_UINT32>(AosNetworkType::LTE),
            static_cast<IMS_UINT32>(AosCapability::VOICE) |
                    static_cast<IMS_UINT32>(AosCapability::VIDEO) |
                    static_cast<IMS_UINT32>(AosCapability::CALL_COMPOSER) |
                    static_cast<IMS_UINT32>(AosCapability::CALL_COMPOSER_BUSINESS_ONLY));
    SetCapabilities(objCapabilities);

    EXPECT_FALSE(IsHandleBlocked(AosHandle::BLOCK_CALL_COMPOSER_CAPABILITY));

    // WHEN
    objCapabilities.SetValue(static_cast<IMS_UINT32>(AosNetworkType::LTE),
            static_cast<IMS_UINT32>(AosCapability::VOICE) |
                    static_cast<IMS_UINT32>(AosCapability::VIDEO) |
                    static_cast<IMS_UINT32>(AosCapability::CALL_COMPOSER_BUSINESS_ONLY));
    ProcessCapabilitiesChanged(objCapabilities);

    // THEN
    EXPECT_FALSE(IsHandleBlocked(AosHandle::BLOCK_CALL_COMPOSER_CAPABILITY));
}

TEST_F(AosHandleMtcTest, ShouldBlockTextCapabilityIfItIsNotInTheListWhenCapabilitiesChanged)
{
    // GIVEN
    SetNetworkType(NW_REPORT_RADIO_LTE);

    ImsMap<IMS_UINT32, IMS_UINT32> objCapabilities;
    objCapabilities.Add(static_cast<IMS_UINT32>(AosNetworkType::LTE),
            static_cast<IMS_UINT32>(AosCapability::VOICE) |
                    static_cast<IMS_UINT32>(AosCapability::VIDEO) |
                    static_cast<IMS_UINT32>(AosCapability::TEXT));
    SetCapabilities(objCapabilities);

    // WHEN
    objCapabilities.SetValue(static_cast<IMS_UINT32>(AosNetworkType::LTE),
            static_cast<IMS_UINT32>(AosCapability::VOICE) |
                    static_cast<IMS_UINT32>(AosCapability::VIDEO));
    ProcessCapabilitiesChanged(objCapabilities);

    // THEN
    EXPECT_TRUE(IsHandleBlocked(AosHandle::BLOCK_TEXT_CAPABILITY));
}

TEST_F(AosHandleMtcTest, ShouldUnblockTextCapabilityIfItIsInTheListWhenCapabilitiesChanged)
{
    // GIVEN
    SetNetworkType(NW_REPORT_RADIO_LTE);

    ImsMap<IMS_UINT32, IMS_UINT32> objCapabilities;
    objCapabilities.Add(static_cast<IMS_UINT32>(AosNetworkType::LTE),
            static_cast<IMS_UINT32>(AosCapability::VOICE) |
                    static_cast<IMS_UINT32>(AosCapability::VIDEO));
    SetCapabilities(objCapabilities);
    AddBlock(AosHandle::BLOCK_TEXT_CAPABILITY);

    // WHEN
    objCapabilities.SetValue(static_cast<IMS_UINT32>(AosNetworkType::LTE),
            static_cast<IMS_UINT32>(AosCapability::VOICE) |
                    static_cast<IMS_UINT32>(AosCapability::VIDEO) |
                    static_cast<IMS_UINT32>(AosCapability::TEXT));
    ProcessCapabilitiesChanged(objCapabilities);

    // THEN
    EXPECT_FALSE(IsHandleBlocked(AosHandle::BLOCK_TEXT_CAPABILITY));
}

TEST_F(AosHandleMtcTest, ProcessCapabilitiesChanged_Test2)
{
    // Test2: No capability(LTE,IWLAN,NR), Current network=LTE, Wfc available
    // Expectation: block BLOCK_VOLTE_CAPABILITY, BLOCK_VILTE_CAPABILITY
    //              No block but holding block BLOCK_VOWIFI_CAPABILITY, BLOCK_VIWIFI_CAPABILITY
    //              set none capa for the network.

    Init();

    ImsMap<IMS_UINT32, IMS_UINT32> objNewCapabilities, objExpectedCapabilities;

    objExpectedCapabilities.Add(static_cast<IMS_UINT32>(AosNetworkType::LTE),
            static_cast<IMS_UINT32>(AosCapability::NONE));
    objExpectedCapabilities.Add(static_cast<IMS_UINT32>(AosNetworkType::IWLAN),
            static_cast<IMS_UINT32>(AosCapability::NONE));
    objExpectedCapabilities.Add(static_cast<IMS_UINT32>(AosNetworkType::NR),
            static_cast<IMS_UINT32>(AosCapability::NONE));
    objExpectedCapabilities.Add(static_cast<IMS_UINT32>(AosNetworkType::UTRAN),
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
    EXPECT_FALSE(IsHandleBlocked(AosHandle::BLOCK_VOWIFI_CAPABILITY));
    EXPECT_FALSE(IsHandleBlocked(AosHandle::BLOCK_VIWIFI_CAPABILITY));
    EXPECT_TRUE(IsHoldingBlockForWifi(AosHandle::BLOCK_VOWIFI_CAPABILITY));
    EXPECT_TRUE(IsHoldingBlockForWifi(AosHandle::BLOCK_VIWIFI_CAPABILITY));
    EXPECT_TRUE(IsEqualCapabilities(GetCapabilities(), objExpectedCapabilities));
    EXPECT_EQ(m_pAosHandleMtc->GetFeatureTagList().GetUnavailableFeatures(), ImsAosFeature::NONE);
}

TEST_F(AosHandleMtcTest, ProcessCapabilitiesChanged_Test3)
{
    // Test3: no capability(LTE,IWLAN,NR), Current network=WLAN & LTE
    // Expectation: block BLOCK_VOWIFI_CAPABILITY, BLOCK_VIWIFI_CAPABILITY,
    //              No block but holding block BLOCK_VOLTE_CAPABILITY, BLOCK_VILTE_CAPABILITY
    //              set none capa for the network.

    Init();

    ImsMap<IMS_UINT32, IMS_UINT32> objNewCapabilities, objExpectedCapabilities;

    objExpectedCapabilities.Add(static_cast<IMS_UINT32>(AosNetworkType::LTE),
            static_cast<IMS_UINT32>(AosCapability::NONE));
    objExpectedCapabilities.Add(static_cast<IMS_UINT32>(AosNetworkType::IWLAN),
            static_cast<IMS_UINT32>(AosCapability::NONE));
    objExpectedCapabilities.Add(static_cast<IMS_UINT32>(AosNetworkType::NR),
            static_cast<IMS_UINT32>(AosCapability::NONE));
    objExpectedCapabilities.Add(static_cast<IMS_UINT32>(AosNetworkType::UTRAN),
            static_cast<IMS_UINT32>(AosCapability::NONE));

    SetNetworkType(NW_REPORT_RADIO_WLAN);
    SetEpdgEnabled(IMS_TRUE);

    EXPECT_CALL(m_objMockIAosNetTracker, GetMobileNetworkType())
            .Times(AnyNumber())
            .WillRepeatedly(Return(NW_REPORT_RADIO_LTE));

    EXPECT_CALL(m_objMockIAosNConfiguration, IsWfcImsAvailable())
            .Times(AnyNumber())
            .WillRepeatedly(Return(IMS_TRUE));

    EXPECT_CALL(m_objMockIAosNConfiguration,
            IsGGsmaRcsTelephonyFeatureTagUsedAsAvailableVoiceCallType())
            .Times(AnyNumber())
            .WillRepeatedly(Return(IMS_FALSE));

    ProcessCapabilitiesChanged(objNewCapabilities);

    EXPECT_FALSE(IsHandleBlocked(AosHandle::BLOCK_VOLTE_CAPABILITY));
    EXPECT_FALSE(IsHandleBlocked(AosHandle::BLOCK_VILTE_CAPABILITY));
    EXPECT_TRUE(IsHoldingBlockForMobile(AosHandle::BLOCK_VOLTE_CAPABILITY));
    EXPECT_TRUE(IsHoldingBlockForMobile(AosHandle::BLOCK_VILTE_CAPABILITY));
    EXPECT_TRUE(IsHandleBlocked(AosHandle::BLOCK_VOWIFI_CAPABILITY));
    EXPECT_TRUE(IsHandleBlocked(AosHandle::BLOCK_VIWIFI_CAPABILITY));
    EXPECT_TRUE(IsEqualCapabilities(GetCapabilities(), objExpectedCapabilities));
    EXPECT_EQ(m_pAosHandleMtc->GetFeatureTagList().GetUnavailableFeatures(), ImsAosFeature::NONE);
}

TEST_F(AosHandleMtcTest, ProcessCapabilitiesChanged_Test4)
{
    // Test4: WFC OFF on WiFi->ON on LTE (Voice capa changed for IWLAN),  Current network=LTE
    // Expectation: Remove BLOCK_VOWIFI_CAPABILITY from holding block for wifi

    ImsMap<IMS_UINT32, IMS_UINT32> objNewCapabilities;

    objNewCapabilities.Add(static_cast<IMS_UINT32>(AosNetworkType::LTE),
            static_cast<IMS_UINT32>(AosCapability::VOICE) |
                    static_cast<IMS_UINT32>(AosCapability::VIDEO));
    objNewCapabilities.Add(static_cast<IMS_UINT32>(AosNetworkType::IWLAN),
            static_cast<IMS_UINT32>(AosCapability::VOICE) |
                    static_cast<IMS_UINT32>(AosCapability::VIDEO));
    objNewCapabilities.Add(static_cast<IMS_UINT32>(AosNetworkType::NR),
            static_cast<IMS_UINT32>(AosCapability::VOICE) |
                    static_cast<IMS_UINT32>(AosCapability::VIDEO));
    objNewCapabilities.Add(static_cast<IMS_UINT32>(AosNetworkType::UTRAN),
            static_cast<IMS_UINT32>(AosCapability::NONE));

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

    ImsMap<IMS_UINT32, IMS_UINT32> objNewCapabilities;

    objNewCapabilities.Add(static_cast<IMS_UINT32>(AosNetworkType::LTE),
            static_cast<IMS_UINT32>(AosCapability::VOICE) |
                    static_cast<IMS_UINT32>(AosCapability::VIDEO));
    objNewCapabilities.Add(static_cast<IMS_UINT32>(AosNetworkType::IWLAN),
            static_cast<IMS_UINT32>(AosCapability::VOICE) |
                    static_cast<IMS_UINT32>(AosCapability::VIDEO));
    objNewCapabilities.Add(static_cast<IMS_UINT32>(AosNetworkType::NR),
            static_cast<IMS_UINT32>(AosCapability::VOICE) |
                    static_cast<IMS_UINT32>(AosCapability::VIDEO));
    objNewCapabilities.Add(static_cast<IMS_UINT32>(AosNetworkType::UTRAN),
            static_cast<IMS_UINT32>(AosCapability::NONE));

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

    ImsMap<IMS_UINT32, IMS_UINT32> objNewCapabilities;

    objNewCapabilities.Add(static_cast<IMS_UINT32>(AosNetworkType::LTE),
            static_cast<IMS_UINT32>(AosCapability::VOICE) |
                    static_cast<IMS_UINT32>(AosCapability::VIDEO));
    objNewCapabilities.Add(static_cast<IMS_UINT32>(AosNetworkType::IWLAN),
            static_cast<IMS_UINT32>(AosCapability::VOICE) |
                    static_cast<IMS_UINT32>(AosCapability::VIDEO));
    objNewCapabilities.Add(static_cast<IMS_UINT32>(AosNetworkType::NR),
            static_cast<IMS_UINT32>(AosCapability::VOICE) |
                    static_cast<IMS_UINT32>(AosCapability::VIDEO));
    objNewCapabilities.Add(static_cast<IMS_UINT32>(AosNetworkType::UTRAN),
            static_cast<IMS_UINT32>(AosCapability::NONE));

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

    ImsMap<IMS_UINT32, IMS_UINT32> objNewCapabilities;

    objNewCapabilities.Add(static_cast<IMS_UINT32>(AosNetworkType::LTE),
            static_cast<IMS_UINT32>(AosCapability::VOICE) |
                    static_cast<IMS_UINT32>(AosCapability::VIDEO));
    objNewCapabilities.Add(static_cast<IMS_UINT32>(AosNetworkType::IWLAN),
            static_cast<IMS_UINT32>(AosCapability::VOICE) |
                    static_cast<IMS_UINT32>(AosCapability::VIDEO));
    objNewCapabilities.Add(static_cast<IMS_UINT32>(AosNetworkType::NR),
            static_cast<IMS_UINT32>(AosCapability::VOICE) |
                    static_cast<IMS_UINT32>(AosCapability::VIDEO));
    objNewCapabilities.Add(static_cast<IMS_UINT32>(AosNetworkType::UTRAN),
            static_cast<IMS_UINT32>(AosCapability::NONE));

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

    ImsMap<IMS_UINT32, IMS_UINT32> objNewCapabilities;

    objNewCapabilities.Add(static_cast<IMS_UINT32>(AosNetworkType::LTE),
            static_cast<IMS_UINT32>(AosCapability::VOICE) |
                    static_cast<IMS_UINT32>(AosCapability::VIDEO));
    objNewCapabilities.Add(static_cast<IMS_UINT32>(AosNetworkType::IWLAN),
            static_cast<IMS_UINT32>(AosCapability::VOICE) |
                    static_cast<IMS_UINT32>(AosCapability::VIDEO));
    objNewCapabilities.Add(static_cast<IMS_UINT32>(AosNetworkType::NR),
            static_cast<IMS_UINT32>(AosCapability::VOICE) |
                    static_cast<IMS_UINT32>(AosCapability::VIDEO));
    objNewCapabilities.Add(static_cast<IMS_UINT32>(AosNetworkType::UTRAN),
            static_cast<IMS_UINT32>(AosCapability::NONE));

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

TEST_F(AosHandleMtcTest, ProcessCapabilitiesChanged_Test9)
{
    // Test9: No capability(LTE,IWLAN,NR), Current network=LTE, Wfc unavailable
    // Expectation: block BLOCK_VOLTE_CAPABILITY, BLOCK_VILTE_CAPABILITY
    //              No block/holding block BLOCK_VOWIFI_CAPABILITY, BLOCK_VIWIFI_CAPABILITY
    //              set none capa for the network.

    Init();

    ImsMap<IMS_UINT32, IMS_UINT32> objNewCapabilities, objExpectedCapabilities;

    objExpectedCapabilities.Add(static_cast<IMS_UINT32>(AosNetworkType::LTE),
            static_cast<IMS_UINT32>(AosCapability::NONE));
    objExpectedCapabilities.Add(static_cast<IMS_UINT32>(AosNetworkType::IWLAN),
            static_cast<IMS_UINT32>(AosCapability::NONE));
    objExpectedCapabilities.Add(static_cast<IMS_UINT32>(AosNetworkType::NR),
            static_cast<IMS_UINT32>(AosCapability::NONE));
    objExpectedCapabilities.Add(static_cast<IMS_UINT32>(AosNetworkType::UTRAN),
            static_cast<IMS_UINT32>(AosCapability::NONE));

    SetNetworkType(NW_REPORT_RADIO_LTE);

    EXPECT_CALL(m_objMockIAosNConfiguration, IsWfcImsAvailable())
            .Times(AnyNumber())
            .WillRepeatedly(Return(IMS_FALSE));

    EXPECT_CALL(m_objMockIAosNConfiguration,
            IsGGsmaRcsTelephonyFeatureTagUsedAsAvailableVoiceCallType())
            .Times(AnyNumber())
            .WillRepeatedly(Return(IMS_FALSE));

    ProcessCapabilitiesChanged(objNewCapabilities);

    EXPECT_TRUE(IsHandleBlocked(AosHandle::BLOCK_VOLTE_CAPABILITY));
    EXPECT_TRUE(IsHandleBlocked(AosHandle::BLOCK_VILTE_CAPABILITY));
    EXPECT_FALSE(IsHandleBlocked(AosHandle::BLOCK_VOWIFI_CAPABILITY));
    EXPECT_FALSE(IsHandleBlocked(AosHandle::BLOCK_VIWIFI_CAPABILITY));
    EXPECT_FALSE(IsHoldingBlockForWifi(AosHandle::BLOCK_VOWIFI_CAPABILITY));
    EXPECT_FALSE(IsHoldingBlockForWifi(AosHandle::BLOCK_VIWIFI_CAPABILITY));
    EXPECT_TRUE(IsEqualCapabilities(GetCapabilities(), objExpectedCapabilities));
    EXPECT_EQ(m_pAosHandleMtc->GetFeatureTagList().GetUnavailableFeatures(), ImsAosFeature::NONE);
}

TEST_F(AosHandleMtcTest, ProcessCapabilitiesChanged_Test10)
{
    // Test10: no capability(LTE,IWLAN,NR), Current network=WLAN & invalid cellular
    // Expectation: block BLOCK_VOWIFI_CAPABILITY, BLOCK_VIWIFI_CAPABILITY,
    //              No block/holding block BLOCK_VOLTE_CAPABILITY, BLOCK_VILTE_CAPABILITY
    //              set none capa for the network.

    Init();

    ImsMap<IMS_UINT32, IMS_UINT32> objNewCapabilities, objExpectedCapabilities;

    objExpectedCapabilities.Add(static_cast<IMS_UINT32>(AosNetworkType::LTE),
            static_cast<IMS_UINT32>(AosCapability::NONE));
    objExpectedCapabilities.Add(static_cast<IMS_UINT32>(AosNetworkType::IWLAN),
            static_cast<IMS_UINT32>(AosCapability::NONE));
    objExpectedCapabilities.Add(static_cast<IMS_UINT32>(AosNetworkType::NR),
            static_cast<IMS_UINT32>(AosCapability::NONE));
    objExpectedCapabilities.Add(static_cast<IMS_UINT32>(AosNetworkType::UTRAN),
            static_cast<IMS_UINT32>(AosCapability::NONE));

    SetNetworkType(NW_REPORT_RADIO_WLAN);
    SetEpdgEnabled(IMS_TRUE);

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

    ProcessCapabilitiesChanged(objNewCapabilities);

    EXPECT_FALSE(IsHandleBlocked(AosHandle::BLOCK_VOLTE_CAPABILITY));
    EXPECT_FALSE(IsHandleBlocked(AosHandle::BLOCK_VILTE_CAPABILITY));
    EXPECT_FALSE(IsHoldingBlockForMobile(AosHandle::BLOCK_VOLTE_CAPABILITY));
    EXPECT_FALSE(IsHoldingBlockForMobile(AosHandle::BLOCK_VILTE_CAPABILITY));
    EXPECT_TRUE(IsHandleBlocked(AosHandle::BLOCK_VOWIFI_CAPABILITY));
    EXPECT_TRUE(IsHandleBlocked(AosHandle::BLOCK_VIWIFI_CAPABILITY));
    EXPECT_TRUE(IsEqualCapabilities(GetCapabilities(), objExpectedCapabilities));
    EXPECT_EQ(m_pAosHandleMtc->GetFeatureTagList().GetUnavailableFeatures(), ImsAosFeature::NONE);
}

TEST_F(AosHandleMtcTest, ProcessCapabilitiesChanged_CallComposer)
{
    // Test: Call composer capability is changed.
    // Expectation: Call composer blocked/unblocked depending on the capability.
    //              Call composer feature is excluded/included depending on the capability.

    Init();

    ImsMap<IMS_UINT32, IMS_UINT32> objNewCapabilities;

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

    EXPECT_CALL(m_objMockIAosNetTracker, GetMobileNetworkType())
            .Times(AnyNumber())
            .WillRepeatedly(Return(NW_REPORT_RADIO_LTE));

    EXPECT_CALL(m_objMockIAosNConfiguration, IsWfcImsAvailable())
            .Times(AnyNumber())
            .WillRepeatedly(Return(IMS_TRUE));

    EXPECT_CALL(m_objMockIAosNConfiguration,
            IsGGsmaRcsTelephonyFeatureTagUsedAsAvailableVoiceCallType())
            .Times(AnyNumber())
            .WillRepeatedly(Return(IMS_FALSE));

    EXPECT_FALSE(IsHandleBlocked(AosHandle::BLOCK_CALL_COMPOSER_CAPABILITY));
    EXPECT_TRUE(m_pAosHandleMtc->GetFeatureTagList().HasFeature(
            ImsAosFeature::CALL_COMPOSER_VIA_TELEPHONY));

    ProcessCapabilitiesChanged(objNewCapabilities);

    EXPECT_TRUE(IsHandleBlocked(AosHandle::BLOCK_CALL_COMPOSER_CAPABILITY));
    EXPECT_FALSE(m_pAosHandleMtc->GetFeatureTagList().HasFeature(
            ImsAosFeature::CALL_COMPOSER_VIA_TELEPHONY));

    objNewCapabilities.SetValue(static_cast<IMS_UINT32>(AosNetworkType::LTE),
            static_cast<IMS_UINT32>(AosCapability::VOICE) |
                    static_cast<IMS_UINT32>(AosCapability::VIDEO) |
                    static_cast<IMS_UINT32>(AosCapability::CALL_COMPOSER));

    ProcessCapabilitiesChanged(objNewCapabilities);

    EXPECT_FALSE(IsHandleBlocked(AosHandle::BLOCK_CALL_COMPOSER_CAPABILITY));
    EXPECT_TRUE(m_pAosHandleMtc->GetFeatureTagList().HasFeature(
            ImsAosFeature::CALL_COMPOSER_VIA_TELEPHONY));
}

TEST_F(AosHandleMtcTest, ProcessNetworkChanged_Test1)
{
    // Test1: Capa=(LTE:voice,video / IWLAN:video / NR:none), no unavailable policy, network=LTE
    // Blocked BLOCK_NETWORK
    // Expectation: block none

    ImsMap<IMS_UINT32, IMS_UINT32> objCapabilities;
    objCapabilities.Add(static_cast<IMS_UINT32>(AosNetworkType::LTE),
            static_cast<IMS_UINT32>(AosCapability::VOICE) |
                    static_cast<IMS_UINT32>(AosCapability::VIDEO) |
                    static_cast<IMS_UINT32>(AosCapability::CALL_COMPOSER) |
                    static_cast<IMS_UINT32>(AosCapability::TEXT));
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

    ImsMap<IMS_UINT32, IMS_UINT32> objCapabilities;
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

    ImsMap<IMS_UINT32, IMS_UINT32> objCapabilities;
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

    ImsMap<IMS_UINT32, IMS_UINT32> objCapabilities;
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

    ImsMap<IMS_UINT32, IMS_UINT32> objCapabilitiesVoiceVideo;
    objCapabilitiesVoiceVideo.Add(static_cast<IMS_UINT32>(AosNetworkType::LTE),
            static_cast<IMS_UINT32>(AosCapability::VOICE) |
                    static_cast<IMS_UINT32>(AosCapability::VIDEO));

    ImsMap<IMS_UINT32, IMS_UINT32> objCapabilitiesVoice;
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

    ImsMap<IMS_UINT32, IMS_UINT32> objCapabilities;
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

    ImsMap<IMS_UINT32, IMS_UINT32> objCapabilities;
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

    ImsMap<IMS_UINT32, IMS_UINT32> objCapabilities;
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

    ImsMap<IMS_UINT32, IMS_UINT32> objCapabilities;
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

    ImsMap<IMS_UINT32, IMS_UINT32> objCapabilities;
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

TEST_F(AosHandleMtcTest, ProcessNetworkChanged_CallComposer)
{
    // Test: Capa=(LTE:voice,video,callcomposer / IWLAN:voice / NR:none), no unavailable policy
    // Expectation: Call composer is get available/unavailable depending on the capability.

    Init();

    ImsMap<IMS_UINT32, IMS_UINT32> objCapabilities;
    objCapabilities.Add(static_cast<IMS_UINT32>(AosNetworkType::LTE),
            static_cast<IMS_UINT32>(AosCapability::VOICE) |
                    static_cast<IMS_UINT32>(AosCapability::VIDEO) |
                    static_cast<IMS_UINT32>(AosCapability::CALL_COMPOSER));
    objCapabilities.Add(static_cast<IMS_UINT32>(AosNetworkType::IWLAN),
            static_cast<IMS_UINT32>(AosCapability::VOICE));
    objCapabilities.Add(static_cast<IMS_UINT32>(AosNetworkType::NR),
            static_cast<IMS_UINT32>(AosCapability::NONE));

    EXPECT_FALSE(IsHandleBlocked(AosHandle::BLOCK_CALL_COMPOSER_CAPABILITY));
    EXPECT_TRUE(m_pAosHandleMtc->GetFeatureTagList().HasFeature(
            ImsAosFeature::CALL_COMPOSER_VIA_TELEPHONY));

    SetCapabilities(objCapabilities);
    SetNetworkType(NW_REPORT_RADIO_WLAN);
    SetEpdgEnabled(IMS_TRUE);

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

    EXPECT_TRUE(IsHandleBlocked(AosHandle::BLOCK_CALL_COMPOSER_CAPABILITY));
    EXPECT_FALSE(m_pAosHandleMtc->GetFeatureTagList().HasFeature(
            ImsAosFeature::CALL_COMPOSER_VIA_TELEPHONY));

    SetNetworkType(NW_REPORT_RADIO_LTE);
    SetEpdgEnabled(IMS_FALSE);

    ProcessNetworkChanged();

    EXPECT_FALSE(IsHandleBlocked(AosHandle::BLOCK_CALL_COMPOSER_CAPABILITY));
    EXPECT_TRUE(m_pAosHandleMtc->GetFeatureTagList().HasFeature(
            ImsAosFeature::CALL_COMPOSER_VIA_TELEPHONY));
}

TEST_F(AosHandleMtcTest, ShouldBlockTextCapabilityIfUeMovesToTheNetworkHasNoTextCapability)
{
    // GIVEN
    Init();

    ImsMap<IMS_UINT32, IMS_UINT32> objCapabilities;
    objCapabilities.Add(static_cast<IMS_UINT32>(AosNetworkType::LTE),
            static_cast<IMS_UINT32>(AosCapability::VOICE) |
                    static_cast<IMS_UINT32>(AosCapability::VIDEO) |
                    static_cast<IMS_UINT32>(AosCapability::TEXT));
    objCapabilities.Add(static_cast<IMS_UINT32>(AosNetworkType::IWLAN),
            static_cast<IMS_UINT32>(AosCapability::VIDEO) |
                    static_cast<IMS_UINT32>(AosCapability::VOICE));
    objCapabilities.Add(static_cast<IMS_UINT32>(AosNetworkType::NR),
            static_cast<IMS_UINT32>(AosCapability::VOICE));

    SetCapabilities(objCapabilities);
    SetNetworkType(NW_REPORT_RADIO_NR);
    SetEpdgEnabled(IMS_FALSE);

    ON_CALL(m_objMockIAosNConfiguration, IsWfcImsAvailable()).WillByDefault(Return(IMS_TRUE));

    ON_CALL(m_objMockIAosNConfiguration,
            IsGGsmaRcsTelephonyFeatureTagUsedAsAvailableVoiceCallType())
            .WillByDefault(Return(IMS_FALSE));

    ON_CALL(m_objMockIAosNConfiguration, IsRegWithFeatureTagUnavailableSupported())
            .WillByDefault(Return(IMS_FALSE));

    // WHEN
    ProcessNetworkChanged();

    // THEN
    EXPECT_TRUE(IsHandleBlocked(AosHandle::BLOCK_TEXT_CAPABILITY));
}

TEST_F(AosHandleMtcTest, ShouldUnblockTextCapabilityIfUeMovesToTheNetworkHasTextCapability)
{
    // GIVEN
    Init();

    ImsMap<IMS_UINT32, IMS_UINT32> objCapabilities;
    objCapabilities.Add(static_cast<IMS_UINT32>(AosNetworkType::LTE),
            static_cast<IMS_UINT32>(AosCapability::VOICE) |
                    static_cast<IMS_UINT32>(AosCapability::VIDEO) |
                    static_cast<IMS_UINT32>(AosCapability::TEXT));
    objCapabilities.Add(static_cast<IMS_UINT32>(AosNetworkType::IWLAN),
            static_cast<IMS_UINT32>(AosCapability::VIDEO) |
                    static_cast<IMS_UINT32>(AosCapability::VOICE));
    objCapabilities.Add(static_cast<IMS_UINT32>(AosNetworkType::NR),
            static_cast<IMS_UINT32>(AosCapability::VOICE));

    SetCapabilities(objCapabilities);
    SetNetworkType(NW_REPORT_RADIO_LTE);
    SetEpdgEnabled(IMS_FALSE);
    AddBlock(AosHandle::BLOCK_TEXT_CAPABILITY);

    ON_CALL(m_objMockIAosNConfiguration, IsWfcImsAvailable()).WillByDefault(Return(IMS_TRUE));

    ON_CALL(m_objMockIAosNConfiguration,
            IsGGsmaRcsTelephonyFeatureTagUsedAsAvailableVoiceCallType())
            .WillByDefault(Return(IMS_FALSE));

    ON_CALL(m_objMockIAosNConfiguration, IsRegWithFeatureTagUnavailableSupported())
            .WillByDefault(Return(IMS_FALSE));

    // WHEN
    ProcessNetworkChanged();

    // THEN
    EXPECT_FALSE(IsHandleBlocked(AosHandle::BLOCK_TEXT_CAPABILITY));
}

TEST_F(AosHandleMtcTest, ProcessNetworkChanged_RefreshSsacInfoOnLte)
{
    SsacInfo objSsacInfoBarred;
    SsacInfo objSsacInfoNotBarred;

    objSsacInfoBarred.nBarringFactorForVoice = 0;
    objSsacInfoBarred.nBarringFactorForVideo = 100;
    objSsacInfoBarred.nBarringTimeSecForVoice = 60;
    objSsacInfoBarred.nBarringTimeSecForVideo = 0;

    objSsacInfoNotBarred.nBarringFactorForVoice = 100;
    objSsacInfoNotBarred.nBarringFactorForVideo = 100;
    objSsacInfoNotBarred.nBarringTimeSecForVoice = 0;
    objSsacInfoNotBarred.nBarringTimeSecForVideo = 0;

    EXPECT_CALL(m_objTestImsRadioService.GetMockImsRadio(), GetSsacInfo())
            .Times(2)
            .WillOnce(ReturnRef(objSsacInfoNotBarred))
            .WillOnce(ReturnRef(objSsacInfoBarred));

    EXPECT_CALL(m_objMockIAosNConfiguration, IsRequiredVolteBlockBySsac())
            .Times(AnyNumber())
            .WillRepeatedly(Return(IMS_TRUE));

    EXPECT_CALL(m_objMockIAosNConfiguration, GetVolteHysTime())
            .Times(AnyNumber())
            .WillRepeatedly(Return(0));

    SetNetworkType(NW_REPORT_RADIO_LTE);
    SetEpdgEnabled(IMS_FALSE);

    EXPECT_CALL(m_objMockIAosNConfiguration, IsRegWithFeatureTagUnavailableSupported())
            .Times(AnyNumber())
            .WillRepeatedly(Return(IMS_FALSE));

    SetSsacBarred(IMS_TRUE);

    Init();

    ProcessNetworkChanged();
    EXPECT_FALSE(IsSsacBarred());

    ProcessNetworkChanged();
    EXPECT_TRUE(IsSsacBarred());
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

TEST_F(AosHandleMtcTest, ProcessVopsStateChanged_Test4)
{
    // Test4: ignore_vops config is true, vops changed to on/off
    // Expectation: no block BLOCK_VOPS, only the vops state is updated

    SetVopsIgnoredForVolteEnabled(IMS_TRUE);

    ProcessVopsStateChanged(IMS_VOICE_OVER_PS_NOT_SUPPORTED);

    EXPECT_FALSE(IsHandleBlocked(AosHandle::BLOCK_VOPS));
    EXPECT_EQ(GetVopsState(), IMS_VOICE_OVER_PS_NOT_SUPPORTED);

    ProcessVopsStateChanged(IMS_VOICE_OVER_PS_SUPPORTED);

    EXPECT_FALSE(IsHandleBlocked(AosHandle::BLOCK_VOPS));
    EXPECT_EQ(GetVopsState(), IMS_VOICE_OVER_PS_SUPPORTED);
}

TEST_F(AosHandleMtcTest, ProcessVopsStateChanged_Test5)
{
    // Test5: ignore_vops config is true/false, no update state,
    // Expectation: set/reset block BLOCK_VOPS as the state
    //              No change m_nVopsState
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

    SetVopsIgnoredForVolteEnabled(IMS_TRUE);
    SetVopsState(IMS_VOICE_OVER_PS_NOT_SUPPORTED);

    ProcessVopsStateChanged(IMS_VOICE_OVER_PS_SUPPORTED, IMS_FALSE);
    EXPECT_EQ(GetHoldingVopsState(), IMS_VOICE_OVER_PS_SUPPORTED);
    EXPECT_EQ(GetVopsState(), IMS_VOICE_OVER_PS_NOT_SUPPORTED);
    EXPECT_FALSE(IsHandleBlocked(AosHandle::BLOCK_VOPS));

    ProcessVopsStateChanged(IMS_VOICE_OVER_PS_NOT_SUPPORTED, IMS_FALSE);
    EXPECT_EQ(GetHoldingVopsState(), IMS_VOICE_OVER_PS_SUPPORTED);
    EXPECT_EQ(GetVopsState(), IMS_VOICE_OVER_PS_NOT_SUPPORTED);
    EXPECT_TRUE(IsHandleBlocked(AosHandle::BLOCK_VOPS));

    SetVopsIgnoredForVolteEnabled(IMS_TRUE);

    ProcessVopsStateChanged(IMS_VOICE_OVER_PS_SUPPORTED, IMS_FALSE);
    EXPECT_EQ(GetHoldingVopsState(), IMS_VOICE_OVER_PS_SUPPORTED);
    EXPECT_EQ(GetVopsState(), IMS_VOICE_OVER_PS_NOT_SUPPORTED);
    EXPECT_FALSE(IsHandleBlocked(AosHandle::BLOCK_VOPS));

    ProcessVopsStateChanged(IMS_VOICE_OVER_PS_NOT_SUPPORTED, IMS_FALSE);
    EXPECT_EQ(GetHoldingVopsState(), IMS_VOICE_OVER_PS_SUPPORTED);
    EXPECT_EQ(GetVopsState(), IMS_VOICE_OVER_PS_NOT_SUPPORTED);
    EXPECT_TRUE(IsHandleBlocked(AosHandle::BLOCK_VOPS));
}

TEST_F(AosHandleMtcTest, ProcessVopsStateChanged_Test6)
{
    // Test6: call idle, no unavailalble policy, network=LTE
    // Expectation: Start/Stop VolteHysTimer. Plmn block if the condition is met.
    //              BLOCK_VOPS blocked.
    //              No change m_nVopsState and m_nHoldingVopsState

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

    EXPECT_CALL(m_objMockIAosNConfiguration, GetVolteHysTime())
            .Times(AnyNumber())
            .WillRepeatedly(Return(60));

    EXPECT_CALL(m_objMockIAosNConfiguration, IsPlmnBlockWithTimeoutOnVoiceCallUnavailable())
            .Times(2)
            .WillOnce(Return(IMS_TRUE))
            .WillOnce(Return(IMS_FALSE));

    EXPECT_CALL(m_objMockIAosApplication, RequestCmd(ImsAosControl::PLMN_BLOCK_WITH_TIMEOUT, 0))
            .Times(1);

    ProcessVopsStateChanged(IMS_VOICE_OVER_PS_NOT_SUPPORTED);
    EXPECT_EQ(GetHoldingVopsState(), IMS_VOICE_OVER_PS_SUPPORTED);
    EXPECT_EQ(GetVopsState(), IMS_VOICE_OVER_PS_NOT_SUPPORTED);
    EXPECT_TRUE(IsHandleBlocked(AosHandle::BLOCK_VOPS));
    EXPECT_FALSE(IsVolteHysTimerRunning());

    ProcessVopsStateChanged(IMS_VOICE_OVER_PS_SUPPORTED);
    EXPECT_EQ(GetHoldingVopsState(), IMS_VOICE_OVER_PS_SUPPORTED);
    EXPECT_EQ(GetVopsState(), IMS_VOICE_OVER_PS_SUPPORTED);
    EXPECT_TRUE(IsHandleBlocked(AosHandle::BLOCK_VOPS));
    EXPECT_TRUE(IsVolteHysTimerRunning());

    ProcessVopsStateChanged(IMS_VOICE_OVER_PS_NOT_SUPPORTED);
    EXPECT_EQ(GetHoldingVopsState(), IMS_VOICE_OVER_PS_SUPPORTED);
    EXPECT_EQ(GetVopsState(), IMS_VOICE_OVER_PS_NOT_SUPPORTED);
    EXPECT_TRUE(IsHandleBlocked(AosHandle::BLOCK_VOPS));
    EXPECT_FALSE(IsVolteHysTimerRunning());
}

TEST_F(AosHandleMtcTest, ProcessVopsStateChanged_VolteHysTimerRunning_by_Ssac)
{
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

    EXPECT_CALL(m_objMockIAosNConfiguration, GetVolteHysTime())
            .Times(AnyNumber())
            .WillRepeatedly(Return(60));

    EXPECT_CALL(m_objMockIAosNConfiguration, IsPlmnBlockWithTimeoutOnVoiceCallUnavailable())
            .Times(AnyNumber())
            .WillRepeatedly(Return(IMS_FALSE));

    EXPECT_CALL(m_objMockIAosNConfiguration, IsRequiredVolteBlockBySsac())
            .Times(AnyNumber())
            .WillRepeatedly(Return(IMS_TRUE));

    ASSERT_EQ(GetVopsState(), IMS_VOICE_OVER_PS_SUPPORTED);
    ASSERT_FALSE(IsHandleBlocked(AosHandle::BLOCK_VOPS));

    SsacInfo objSsacInfo;
    objSsacInfo.nBarringFactorForVoice = 0;
    ImsRadio_OnSsacChanged(objSsacInfo);
    ASSERT_TRUE(IsSsacBarred());
    ASSERT_TRUE(IsHandleBlocked(AosHandle::BLOCK_SSAC));

    objSsacInfo.nBarringFactorForVoice = 100;
    ImsRadio_OnSsacChanged(objSsacInfo);
    ASSERT_TRUE(IsVolteHysTimerRunning());
    ASSERT_FALSE(IsSsacBarred());
    ASSERT_TRUE(IsHandleBlocked(AosHandle::BLOCK_SSAC));

    ProcessVopsStateChanged(IMS_VOICE_OVER_PS_NOT_SUPPORTED);
    EXPECT_FALSE(IsVolteHysTimerRunning());
    EXPECT_EQ(GetVopsState(), IMS_VOICE_OVER_PS_NOT_SUPPORTED);
    EXPECT_TRUE(IsHandleBlocked(AosHandle::BLOCK_VOPS));
    EXPECT_FALSE(IsHandleBlocked(AosHandle::BLOCK_SSAC));
}

TEST_F(AosHandleMtcTest, VopsChangeWithPlmnChange_Plmn1_Off_Plmn2_On)
{
    EXPECT_CALL(m_objMockIAosCallTracker, IsNormalCallActive())
            .Times(AnyNumber())
            .WillRepeatedly(Return(IMS_FALSE));

    SetNetworkType(NW_REPORT_RADIO_LTE);
    EXPECT_CALL(m_objMockIAosNetTracker, GetMobileNetworkType())
            .Times(AnyNumber())
            .WillRepeatedly(Return(NW_REPORT_RADIO_LTE));

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

    EXPECT_CALL(m_objMockIAosNConfiguration, GetVolteHysTime())
            .Times(AnyNumber())
            .WillRepeatedly(Return(60));

    EXPECT_CALL(m_objMockIAosNConfiguration, IsPlmnBlockWithTimeoutOnVoiceCallUnavailable())
            .Times(AnyNumber())
            .WillRepeatedly(Return(IMS_FALSE));

    // Plmn1_Off
    ProcessVopsStateChanged(IMS_VOICE_OVER_PS_NOT_SUPPORTED);
    ASSERT_TRUE(IsHandleBlocked(AosHandle::BLOCK_VOPS));
    ASSERT_EQ(GetVopsState(), IMS_VOICE_OVER_PS_NOT_SUPPORTED);

    // Plmn2_On
    ProcessVopsStateChanged(IMS_VOICE_OVER_PS_SUPPORTED);
    EXPECT_TRUE(IsVolteHysTimerRunning());
    EXPECT_TRUE(IsHandleBlocked(AosHandle::BLOCK_VOPS));
    ServicePhone_PlmnChanged();
    EXPECT_FALSE(IsVolteHysTimerRunning());
    EXPECT_FALSE(IsHandleBlocked(AosHandle::BLOCK_VOPS));
}

TEST_F(AosHandleMtcTest, VopsChangeWithPlmnChange_Plmn1_Off_Plmn2_Off_Plmn1_On)
{
    EXPECT_CALL(m_objMockIAosCallTracker, IsNormalCallActive())
            .Times(AnyNumber())
            .WillRepeatedly(Return(IMS_FALSE));

    SetNetworkType(NW_REPORT_RADIO_LTE);
    EXPECT_CALL(m_objMockIAosNetTracker, GetMobileNetworkType())
            .Times(AnyNumber())
            .WillRepeatedly(Return(NW_REPORT_RADIO_LTE));

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

    EXPECT_CALL(m_objMockIAosNConfiguration, GetVolteHysTime())
            .Times(AnyNumber())
            .WillRepeatedly(Return(60));

    EXPECT_CALL(m_objMockIAosNConfiguration, IsPlmnBlockWithTimeoutOnVoiceCallUnavailable())
            .Times(AnyNumber())
            .WillRepeatedly(Return(IMS_FALSE));

    // Plmn1_Off
    ProcessVopsStateChanged(IMS_VOICE_OVER_PS_NOT_SUPPORTED);
    ASSERT_TRUE(IsHandleBlocked(AosHandle::BLOCK_VOPS));
    ASSERT_EQ(GetVopsState(), IMS_VOICE_OVER_PS_NOT_SUPPORTED);

    // Plmn2_Off (No vops event will come)
    ServicePhone_PlmnChanged();
    EXPECT_TRUE(IsHandleBlocked(AosHandle::BLOCK_VOPS));
    EXPECT_FALSE(IsVolteHysTimerRunning());

    // Plmn1_On
    ProcessVopsStateChanged(IMS_VOICE_OVER_PS_SUPPORTED);
    EXPECT_TRUE(IsHandleBlocked(AosHandle::BLOCK_VOPS));
    EXPECT_TRUE(IsVolteHysTimerRunning());
    ServicePhone_PlmnChanged();
    EXPECT_FALSE(IsHandleBlocked(AosHandle::BLOCK_VOPS));
    EXPECT_FALSE(IsVolteHysTimerRunning());
}

TEST_F(AosHandleMtcTest, VopsChangeWithPlmnChange_Plmn1_Off_Plmn2_Off_Plmn2_On)
{
    EXPECT_CALL(m_objMockIAosCallTracker, IsNormalCallActive())
            .Times(AnyNumber())
            .WillRepeatedly(Return(IMS_FALSE));

    SetNetworkType(NW_REPORT_RADIO_LTE);
    EXPECT_CALL(m_objMockIAosNetTracker, GetMobileNetworkType())
            .Times(AnyNumber())
            .WillRepeatedly(Return(NW_REPORT_RADIO_LTE));

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

    EXPECT_CALL(m_objMockIAosNConfiguration, GetVolteHysTime())
            .Times(AnyNumber())
            .WillRepeatedly(Return(60));

    EXPECT_CALL(m_objMockIAosNConfiguration, IsPlmnBlockWithTimeoutOnVoiceCallUnavailable())
            .Times(AnyNumber())
            .WillRepeatedly(Return(IMS_FALSE));

    // Plmn1_Off
    ProcessVopsStateChanged(IMS_VOICE_OVER_PS_NOT_SUPPORTED);
    ASSERT_TRUE(IsHandleBlocked(AosHandle::BLOCK_VOPS));
    ASSERT_EQ(GetVopsState(), IMS_VOICE_OVER_PS_NOT_SUPPORTED);

    // Plmn2_Off (No vops event will come)
    ServicePhone_PlmnChanged();
    EXPECT_TRUE(IsHandleBlocked(AosHandle::BLOCK_VOPS));
    EXPECT_FALSE(IsVolteHysTimerRunning());

    // Plmn2_On
    ProcessVopsStateChanged(IMS_VOICE_OVER_PS_SUPPORTED);
    EXPECT_TRUE(IsVolteHysTimerRunning());
    EXPECT_TRUE(IsHandleBlocked(AosHandle::BLOCK_VOPS));
    EXPECT_EQ(GetVopsState(), IMS_VOICE_OVER_PS_SUPPORTED);
    ProcessVolteHysTimerExpired();
    EXPECT_FALSE(IsVolteHysTimerRunning());
    EXPECT_FALSE(IsHandleBlocked(AosHandle::BLOCK_VOPS));
}

TEST_F(AosHandleMtcTest, VopsChangeWithPlmnChange_Plmn1_Off_Plmn1_On_Plmn2_On)
{
    EXPECT_CALL(m_objMockIAosCallTracker, IsNormalCallActive())
            .Times(AnyNumber())
            .WillRepeatedly(Return(IMS_FALSE));

    SetNetworkType(NW_REPORT_RADIO_LTE);
    EXPECT_CALL(m_objMockIAosNetTracker, GetMobileNetworkType())
            .Times(AnyNumber())
            .WillRepeatedly(Return(NW_REPORT_RADIO_LTE));

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

    EXPECT_CALL(m_objMockIAosNConfiguration, GetVolteHysTime())
            .Times(AnyNumber())
            .WillRepeatedly(Return(60));

    EXPECT_CALL(m_objMockIAosNConfiguration, IsPlmnBlockWithTimeoutOnVoiceCallUnavailable())
            .Times(AnyNumber())
            .WillRepeatedly(Return(IMS_FALSE));

    // Plmn1_Off
    ProcessVopsStateChanged(IMS_VOICE_OVER_PS_NOT_SUPPORTED);
    ASSERT_TRUE(IsHandleBlocked(AosHandle::BLOCK_VOPS));
    ASSERT_EQ(GetVopsState(), IMS_VOICE_OVER_PS_NOT_SUPPORTED);

    // Plmn1_On
    ProcessVopsStateChanged(IMS_VOICE_OVER_PS_SUPPORTED);
    EXPECT_TRUE(IsVolteHysTimerRunning());
    EXPECT_TRUE(IsHandleBlocked(AosHandle::BLOCK_VOPS));
    EXPECT_EQ(GetVopsState(), IMS_VOICE_OVER_PS_SUPPORTED);

    // Plmn2_On (No vops event will come)
    ServicePhone_PlmnChanged();
    EXPECT_FALSE(IsVolteHysTimerRunning());
    EXPECT_FALSE(IsHandleBlocked(AosHandle::BLOCK_VOPS));
}

TEST_F(AosHandleMtcTest, VopsChangeWithPlmnChange_Plmn1_Off_Plmn1_On_Plmn2_Off)
{
    EXPECT_CALL(m_objMockIAosCallTracker, IsNormalCallActive())
            .Times(AnyNumber())
            .WillRepeatedly(Return(IMS_FALSE));

    SetNetworkType(NW_REPORT_RADIO_LTE);
    EXPECT_CALL(m_objMockIAosNetTracker, GetMobileNetworkType())
            .Times(AnyNumber())
            .WillRepeatedly(Return(NW_REPORT_RADIO_LTE));

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

    EXPECT_CALL(m_objMockIAosNConfiguration, GetVolteHysTime())
            .Times(AnyNumber())
            .WillRepeatedly(Return(60));

    EXPECT_CALL(m_objMockIAosNConfiguration, IsPlmnBlockWithTimeoutOnVoiceCallUnavailable())
            .Times(AnyNumber())
            .WillRepeatedly(Return(IMS_FALSE));

    // Plmn1_Off
    ProcessVopsStateChanged(IMS_VOICE_OVER_PS_NOT_SUPPORTED);
    ASSERT_TRUE(IsHandleBlocked(AosHandle::BLOCK_VOPS));
    ASSERT_EQ(GetVopsState(), IMS_VOICE_OVER_PS_NOT_SUPPORTED);

    // Plmn1_On
    ProcessVopsStateChanged(IMS_VOICE_OVER_PS_SUPPORTED);
    EXPECT_TRUE(IsVolteHysTimerRunning());
    EXPECT_TRUE(IsHandleBlocked(AosHandle::BLOCK_VOPS));
    EXPECT_EQ(GetVopsState(), IMS_VOICE_OVER_PS_SUPPORTED);

    // Plmn2_Off
    ProcessVopsStateChanged(IMS_VOICE_OVER_PS_NOT_SUPPORTED);
    EXPECT_FALSE(IsVolteHysTimerRunning());
    EXPECT_TRUE(IsHandleBlocked(AosHandle::BLOCK_VOPS));
    EXPECT_EQ(GetVopsState(), IMS_VOICE_OVER_PS_NOT_SUPPORTED);
    ServicePhone_PlmnChanged();  // nothing to do
}

TEST_F(AosHandleMtcTest, ReevaluateUnavailableFeature_Test1)
{
    // Test1: Vops change / Video Capable / Ssac not barred
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
    // Test2: Vops change / Video NOT capable / Ssac not barred
    // Expectation: Add/Remove mmtel to/from unavailable feature / No block vops
    //              Notify to AosRegistration if unavailable feature is changed

    EXPECT_CALL(m_objMockIAosNConfiguration, IsRegWithFeatureTagUnavailableSupported())
            .Times(AnyNumber())
            .WillRepeatedly(Return(IMS_TRUE));

    SetNetworkType(NW_REPORT_RADIO_LTE);

    ImsMap<IMS_UINT32, IMS_UINT32> objCapabilitiesVoice;
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

TEST_F(AosHandleMtcTest, ReevaluateUnavailableFeature_Test4)
{
    // Test4: Vops change on Ssac barred / Video Capable
    // Expectation: mmtel, video are always in unavailable feature / No block vops
    //              Notify to AosRegistration if unavailable feature is changed

    EXPECT_CALL(m_objMockIAosNConfiguration, IsRegWithFeatureTagUnavailableSupported())
            .Times(AnyNumber())
            .WillRepeatedly(Return(IMS_TRUE));

    SetNetworkType(NW_REPORT_RADIO_LTE);

    EXPECT_CALL(m_objMockIAosNConfiguration, IsVopsIgnoredForVolteEnabled())
            .Times(AnyNumber())
            .WillRepeatedly(Return(IMS_FALSE));

    EXPECT_CALL(m_objMockIAosNConfiguration, IsRequiredVolteBlockBySsac())
            .Times(AnyNumber())
            .WillRepeatedly(Return(IMS_TRUE));

    SsacInfo objSsacInfo;
    objSsacInfo.nBarringFactorForVoice = 0;
    ImsRadio_OnSsacChanged(objSsacInfo);

    ASSERT_TRUE(IsSsacBarred());

    EXPECT_CALL(
            m_objMockIAosRegistration, RequestCmd(IAosRegistration::CMD_UNAVAILABLE_FEATURE_TAG, 0))
            .Times(0);

    ProcessVopsStateChanged(IMS_VOICE_OVER_PS_NOT_SUPPORTED);
    EXPECT_TRUE(m_pAosHandleMtc->GetFeatureTagList().HasUnavailableFeature(ImsAosFeature::MMTEL));
    EXPECT_TRUE(m_pAosHandleMtc->GetFeatureTagList().HasUnavailableFeature(ImsAosFeature::VIDEO));
    EXPECT_FALSE(IsHandleBlocked(AosHandle::BLOCK_VOPS));

    ProcessVopsStateChanged(IMS_VOICE_OVER_PS_SUPPORTED);
    EXPECT_TRUE(m_pAosHandleMtc->GetFeatureTagList().HasUnavailableFeature(ImsAosFeature::MMTEL));
    EXPECT_TRUE(m_pAosHandleMtc->GetFeatureTagList().HasUnavailableFeature(ImsAosFeature::VIDEO));
    EXPECT_FALSE(IsHandleBlocked(AosHandle::BLOCK_VOPS));
}

TEST_F(AosHandleMtcTest, ReevaluateUnavailableFeature_Test5)
{
    // Test5: Ssac change / Video Capable / Vops supported
    // Expectation: Add/Remove mmtel, video to/from unavailable feature / No block ssac
    //              Notify to AosRegistration if unavailable feature is changed

    EXPECT_CALL(m_objMockIAosNConfiguration, IsRegWithFeatureTagUnavailableSupported())
            .Times(AnyNumber())
            .WillRepeatedly(Return(IMS_TRUE));

    SetNetworkType(NW_REPORT_RADIO_LTE);

    EXPECT_CALL(m_objMockIAosNConfiguration, IsVopsIgnoredForVolteEnabled())
            .Times(AnyNumber())
            .WillRepeatedly(Return(IMS_FALSE));

    EXPECT_CALL(m_objMockIAosNConfiguration, IsRequiredVolteBlockBySsac())
            .Times(AnyNumber())
            .WillRepeatedly(Return(IMS_TRUE));

    EXPECT_CALL(
            m_objMockIAosRegistration, RequestCmd(IAosRegistration::CMD_UNAVAILABLE_FEATURE_TAG, 0))
            .Times(2);

    SsacInfo objSsacInfo;
    objSsacInfo.nBarringFactorForVoice = 0;
    ImsRadio_OnSsacChanged(objSsacInfo);
    EXPECT_TRUE(m_pAosHandleMtc->GetFeatureTagList().HasUnavailableFeature(ImsAosFeature::MMTEL));
    EXPECT_TRUE(m_pAosHandleMtc->GetFeatureTagList().HasUnavailableFeature(ImsAosFeature::VIDEO));
    EXPECT_FALSE(IsHandleBlocked(AosHandle::BLOCK_SSAC));

    objSsacInfo.nBarringFactorForVoice = 100;
    ImsRadio_OnSsacChanged(objSsacInfo);
    EXPECT_FALSE(m_pAosHandleMtc->GetFeatureTagList().HasUnavailableFeature(ImsAosFeature::MMTEL));
    EXPECT_FALSE(m_pAosHandleMtc->GetFeatureTagList().HasUnavailableFeature(ImsAosFeature::VIDEO));
    EXPECT_FALSE(IsHandleBlocked(AosHandle::BLOCK_SSAC));
}

TEST_F(AosHandleMtcTest, ReevaluateUnavailableFeature_Test6)
{
    // Test6: Ssac change on vops not supported / Video Capable
    // Expectation: mmtel, video are always in unavailable feature / No block ssac
    //              Notify to AosRegistration if unavailable feature is changed

    EXPECT_CALL(m_objMockIAosNConfiguration, IsRegWithFeatureTagUnavailableSupported())
            .Times(AnyNumber())
            .WillRepeatedly(Return(IMS_TRUE));

    SetNetworkType(NW_REPORT_RADIO_LTE);

    EXPECT_CALL(m_objMockIAosNConfiguration, IsVopsIgnoredForVolteEnabled())
            .Times(AnyNumber())
            .WillRepeatedly(Return(IMS_FALSE));

    EXPECT_CALL(m_objMockIAosNConfiguration, IsRequiredVolteBlockBySsac())
            .Times(AnyNumber())
            .WillRepeatedly(Return(IMS_TRUE));

    EXPECT_CALL(
            m_objMockIAosRegistration, RequestCmd(IAosRegistration::CMD_UNAVAILABLE_FEATURE_TAG, 0))
            .Times(1);

    ProcessVopsStateChanged(IMS_VOICE_OVER_PS_NOT_SUPPORTED);

    SsacInfo objSsacInfo;
    objSsacInfo.nBarringFactorForVoice = 0;
    ImsRadio_OnSsacChanged(objSsacInfo);
    EXPECT_TRUE(m_pAosHandleMtc->GetFeatureTagList().HasUnavailableFeature(ImsAosFeature::MMTEL));
    EXPECT_TRUE(m_pAosHandleMtc->GetFeatureTagList().HasUnavailableFeature(ImsAosFeature::VIDEO));
    EXPECT_FALSE(IsHandleBlocked(AosHandle::BLOCK_SSAC));

    objSsacInfo.nBarringFactorForVoice = 100;
    ImsRadio_OnSsacChanged(objSsacInfo);
    EXPECT_TRUE(m_pAosHandleMtc->GetFeatureTagList().HasUnavailableFeature(ImsAosFeature::MMTEL));
    EXPECT_TRUE(m_pAosHandleMtc->GetFeatureTagList().HasUnavailableFeature(ImsAosFeature::VIDEO));
    EXPECT_FALSE(IsHandleBlocked(AosHandle::BLOCK_SSAC));
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

TEST_F(AosHandleMtcTest, IsPlmnBlockCondition_Test1)
{
    // Test1: IsPlmnBlockWithTimeoutOnVoiceCallUnavailable is false or network type is invalid
    // Expectation: return false

    EXPECT_CALL(m_objMockIAosNConfiguration, IsPlmnBlockWithTimeoutOnVoiceCallUnavailable())
            .Times(2)
            .WillOnce(Return(IMS_FALSE))
            .WillOnce(Return(IMS_TRUE));

    SetNetworkType(NW_REPORT_RADIO_LTE);
    EXPECT_FALSE(IsPlmnBlockCondition());

    SetNetworkType(NW_REPORT_RADIO_WCDMA);
    EXPECT_FALSE(IsPlmnBlockCondition());
}

TEST_F(AosHandleMtcTest, IsPlmnBlockCondition_Test2)
{
    // Test2: IsPlmnBlockWithTimeoutOnVoiceCallUnavailable is true, network type is LTE
    // Expectation: return false if cs voice is available else return true.

    EXPECT_CALL(m_objMockIAosNConfiguration, IsPlmnBlockWithTimeoutOnVoiceCallUnavailable())
            .Times(AnyNumber())
            .WillRepeatedly(Return(IMS_TRUE));

    SetNetworkType(NW_REPORT_RADIO_LTE);

    SetCsVoiceAvailable(IMS_FALSE);
    EXPECT_TRUE(IsPlmnBlockCondition());

    SetCsVoiceAvailable(IMS_TRUE);
    EXPECT_FALSE(IsPlmnBlockCondition());
}

TEST_F(AosHandleMtcTest, IsPlmnBlockCondition_Nr)
{
    // Test2: IsPlmnBlockWithTimeoutOnVoiceCallUnavailable is true, network type is NR
    // Expectation: return true regardless of cs voice availability

    EXPECT_CALL(m_objMockIAosNConfiguration, IsPlmnBlockWithTimeoutOnVoiceCallUnavailable())
            .Times(AnyNumber())
            .WillRepeatedly(Return(IMS_TRUE));

    SetNetworkType(NW_REPORT_RADIO_NR);

    SetCsVoiceAvailable(IMS_FALSE);
    EXPECT_TRUE(IsPlmnBlockCondition());

    SetCsVoiceAvailable(IMS_TRUE);
    EXPECT_TRUE(IsPlmnBlockCondition());
}

TEST_F(AosHandleMtcTest, ProcessHoldingVopsState_Test)
{
    // Expectation: return true if vops not supported && normal call active
    //              or vops supported && holding vops is not supported
    //              else return false

    EXPECT_CALL(m_objMockIAosCallTracker, IsNormalCallActive())
            .Times(2)
            .WillOnce(Return(IMS_TRUE))
            .WillOnce(Return(IMS_FALSE));

    EXPECT_TRUE(ProcessHoldingVopsState(IMS_VOICE_OVER_PS_NOT_SUPPORTED));
    EXPECT_EQ(GetHoldingVopsState(), IMS_VOICE_OVER_PS_NOT_SUPPORTED);

    EXPECT_TRUE(ProcessHoldingVopsState(IMS_VOICE_OVER_PS_SUPPORTED));
    EXPECT_EQ(GetHoldingVopsState(), IMS_VOICE_OVER_PS_SUPPORTED);

    EXPECT_FALSE(ProcessHoldingVopsState(IMS_VOICE_OVER_PS_NOT_SUPPORTED));
    EXPECT_EQ(GetHoldingVopsState(), IMS_VOICE_OVER_PS_SUPPORTED);

    EXPECT_FALSE(ProcessHoldingVopsState(IMS_VOICE_OVER_PS_SUPPORTED));
    EXPECT_EQ(GetHoldingVopsState(), IMS_VOICE_OVER_PS_SUPPORTED);
}

TEST_F(AosHandleMtcTest, ProcessHoldingSsacState_Test)
{
    // Expectation: return true if m_bSsacHeld is changed else return false

    EXPECT_CALL(m_objMockIAosCallTracker, IsNormalCallActive())
            .Times(2)
            .WillOnce(Return(IMS_FALSE))
            .WillOnce(Return(IMS_TRUE));

    EXPECT_FALSE(ProcessHoldingSsacState(0));
    EXPECT_FALSE(IsSsacHeld());

    EXPECT_FALSE(ProcessHoldingSsacState(100));
    EXPECT_FALSE(IsSsacHeld());

    EXPECT_TRUE(ProcessHoldingSsacState(0));
    EXPECT_TRUE(IsSsacHeld());

    EXPECT_TRUE(ProcessHoldingSsacState(100));
    EXPECT_FALSE(IsSsacHeld());
}

TEST_F(AosHandleMtcTest, ProcessVolteHysTimerExpired_Test)
{
    // Expectation: Stop the timer / Unblock vops or ssac if blocked

    AddBlock(AosHandle::BLOCK_VOPS);
    StartVolteHysTimer(60);
    ProcessVolteHysTimerExpired();
    EXPECT_FALSE(IsHandleBlocked(AosHandle::BLOCK_VOPS));

    AddHoldingBlockForMobile(AosHandle::BLOCK_VOPS);
    StartVolteHysTimer(60);
    ProcessVolteHysTimerExpired();
    EXPECT_FALSE(IsHoldingBlockForMobile(AosHandle::BLOCK_VOPS));

    AddBlock(AosHandle::BLOCK_SSAC);
    StartVolteHysTimer(60);
    ProcessVolteHysTimerExpired();
    EXPECT_FALSE(IsHandleBlocked(AosHandle::BLOCK_SSAC));

    AddHoldingBlockForMobile(AosHandle::BLOCK_SSAC);
    StartVolteHysTimer(60);
    ProcessVolteHysTimerExpired();
    EXPECT_FALSE(IsHoldingBlockForMobile(AosHandle::BLOCK_SSAC));

    SetVopsState(IMS_VOICE_OVER_PS_NOT_SUPPORTED);
    SetSsacBarred(IMS_TRUE);
    StartVolteHysTimer(60);
    EXPECT_TRUE(IsVolteHysTimerRunning());
    ProcessVolteHysTimerExpired();
    EXPECT_FALSE(IsVolteHysTimerRunning());
}

TEST_F(AosHandleMtcTest, StartVolteHysTimer_Test)
{
    EXPECT_FALSE(StartVolteHysTimer(0));
    EXPECT_FALSE(IsVolteHysTimerRunning());

    EXPECT_TRUE(StartVolteHysTimer(60));
    EXPECT_TRUE(IsVolteHysTimerRunning());

    EXPECT_FALSE(StartVolteHysTimer(60));
    EXPECT_TRUE(IsVolteHysTimerRunning());

    StopVolteHysTimer();
}

TEST_F(AosHandleMtcTest, StopVolteHysTimer_Test)
{
    StartVolteHysTimer(60);
    EXPECT_TRUE(IsVolteHysTimerRunning());

    StopVolteHysTimer();
    EXPECT_FALSE(IsVolteHysTimerRunning());
}

TEST_F(AosHandleMtcTest, IsVolteHysTimerRunning_Test)
{
    EXPECT_FALSE(IsVolteHysTimerRunning());

    StartVolteHysTimer(60);
    EXPECT_TRUE(IsVolteHysTimerRunning());

    StopVolteHysTimer();
    EXPECT_FALSE(IsVolteHysTimerRunning());
}

TEST_F(AosHandleMtcTest, NConfiguration_NotifyConfigChanged_Test1)
{
    // Test1: VopsIgnoredForVolteEnabled not changed
    // Expectation: No action

    EXPECT_CALL(m_objMockIAosNConfiguration, IsVopsIgnoredForVolteEnabled())
            .Times(2)
            .WillOnce(Return(IMS_FALSE))
            .WillOnce(Return(IMS_TRUE));

    SetVopsIgnoredForVolteEnabled(IMS_FALSE);
    NConfiguration_NotifyConfigChanged();
    EXPECT_FALSE(IsVopsIgnoredForVolteEnabled());

    SetVopsIgnoredForVolteEnabled(IMS_TRUE);
    NConfiguration_NotifyConfigChanged();
    EXPECT_TRUE(IsVopsIgnoredForVolteEnabled());
}

TEST_F(AosHandleMtcTest, NConfiguration_NotifyConfigChanged_Test2)
{
    // Test2: VopsIgnoredForVolteEnabled changed, invalid network
    // Expectation: update with the config value

    EXPECT_CALL(m_objMockIAosNConfiguration, IsVopsIgnoredForVolteEnabled())
            .Times(2)
            .WillOnce(Return(IMS_FALSE))
            .WillOnce(Return(IMS_TRUE));

    SetNetworkType(NW_REPORT_RADIO_GSM);

    SetVopsIgnoredForVolteEnabled(IMS_TRUE);
    NConfiguration_NotifyConfigChanged();
    EXPECT_FALSE(IsVopsIgnoredForVolteEnabled());

    SetVopsIgnoredForVolteEnabled(IMS_FALSE);
    NConfiguration_NotifyConfigChanged();
    EXPECT_TRUE(IsVopsIgnoredForVolteEnabled());
}

TEST_F(AosHandleMtcTest, NConfiguration_NotifyConfigChanged_Test3)
{
    // Test3: VopsIgnoredForVolteEnabled changed, valid network, vops supported
    // Expectation: update with the config value

    EXPECT_CALL(m_objMockIAosNConfiguration, IsVopsIgnoredForVolteEnabled())
            .Times(2)
            .WillOnce(Return(IMS_FALSE))
            .WillOnce(Return(IMS_TRUE));

    SetNetworkType(NW_REPORT_RADIO_LTE);
    SetVopsState(IMS_VOICE_OVER_PS_SUPPORTED);

    SetVopsIgnoredForVolteEnabled(IMS_TRUE);
    NConfiguration_NotifyConfigChanged();
    EXPECT_FALSE(IsVopsIgnoredForVolteEnabled());

    SetVopsIgnoredForVolteEnabled(IMS_FALSE);
    NConfiguration_NotifyConfigChanged();
    EXPECT_TRUE(IsVopsIgnoredForVolteEnabled());
}

TEST_F(AosHandleMtcTest, NConfiguration_NotifyConfigChanged_Test4)
{
    // Test4: VopsIgnoredForVolteEnabled changed, valid network, vops not supported
    // Expectation: update with the config value
    //              Set/Reset BLOCK_VOPS if the config is false/true
    //              No update for vops state

    EXPECT_CALL(m_objMockIAosNConfiguration, IsVopsIgnoredForVolteEnabled())
            .Times(2)
            .WillOnce(Return(IMS_TRUE))
            .WillOnce(Return(IMS_FALSE));

    SetNetworkType(NW_REPORT_RADIO_LTE);
    SetVopsState(IMS_VOICE_OVER_PS_NOT_SUPPORTED);
    AddBlock(AosHandle::BLOCK_VOPS);

    SetVopsIgnoredForVolteEnabled(IMS_FALSE);
    NConfiguration_NotifyConfigChanged();
    EXPECT_FALSE(IsHandleBlocked(AosHandle::BLOCK_VOPS));
    EXPECT_EQ(GetVopsState(), IMS_VOICE_OVER_PS_NOT_SUPPORTED);
    EXPECT_TRUE(IsVopsIgnoredForVolteEnabled());

    NConfiguration_NotifyConfigChanged();
    EXPECT_TRUE(IsHandleBlocked(AosHandle::BLOCK_VOPS));
    EXPECT_EQ(GetVopsState(), IMS_VOICE_OVER_PS_NOT_SUPPORTED);
    EXPECT_FALSE(IsVopsIgnoredForVolteEnabled());
}

TEST_F(AosHandleMtcTest, ImsRadio_OnSsacChanged_Test1)
{
    // Test1: Ignoring case
    // Expectation: m_bSsacBarred not changed

    SsacInfo objSsacInfo;
    objSsacInfo.nBarringFactorForVoice = 0;

    SetSsacBarred(IMS_FALSE);

    EXPECT_CALL(m_objMockIAosNConfiguration, IsRequiredVolteBlockBySsac())
            .Times(3)
            .WillOnce(Return(IMS_FALSE))
            .WillOnce(Return(IMS_TRUE))
            .WillOnce(Return(IMS_TRUE));

    ImsRadio_OnSsacChanged(objSsacInfo);
    EXPECT_FALSE(IsSsacBarred());

    EXPECT_CALL(m_objMockIAosCallTracker, IsNormalCallActive())
            .Times(2)
            .WillOnce(Return(IMS_TRUE))
            .WillOnce(Return(IMS_FALSE));

    ImsRadio_OnSsacChanged(objSsacInfo);
    EXPECT_FALSE(IsSsacBarred());

    SetNetworkType(NW_REPORT_RADIO_NR);

    ImsRadio_OnSsacChanged(objSsacInfo);
    EXPECT_FALSE(IsSsacBarred());
}

TEST_F(AosHandleMtcTest, ImsRadio_OnSsacChanged_Test2)
{
    // Test2: Barring factor is 0, PlmnBlockCondition is true
    // Expectation: m_SsacBarred is set to true, Plmn block is requested.

    EXPECT_CALL(m_objMockIAosNConfiguration, IsRequiredVolteBlockBySsac())
            .Times(AnyNumber())
            .WillRepeatedly(Return(IMS_TRUE));

    SetNetworkType(NW_REPORT_RADIO_LTE);
    SetSsacBarred(IMS_FALSE);

    SsacInfo objSsacInfo;
    objSsacInfo.nBarringFactorForVoice = 0;

    EXPECT_CALL(m_objMockIAosNConfiguration, IsPlmnBlockWithTimeoutOnVoiceCallUnavailable())
            .Times(1)
            .WillOnce(Return(IMS_TRUE));

    SetRoamingState(IMS_ROAMING_STATE_OFF);

    EXPECT_CALL(m_objMockIAosApplication, RequestCmd(ImsAosControl::PLMN_BLOCK_WITH_TIMEOUT, 0))
            .Times(1);

    ImsRadio_OnSsacChanged(objSsacInfo);
    EXPECT_TRUE(IsSsacBarred());
    EXPECT_TRUE(IsHandleBlocked(AosHandle::BLOCK_SSAC));
}

TEST_F(AosHandleMtcTest, ImsRadio_OnSsacChanged_Test3)
{
    // Test3: Barring factor is 0, PlmnBlockCondition is false
    // Expectation: m_SsacBarred is set to true, BLOCK_SSAC is blocked

    EXPECT_CALL(m_objMockIAosNConfiguration, IsRequiredVolteBlockBySsac())
            .Times(AnyNumber())
            .WillRepeatedly(Return(IMS_TRUE));

    SetNetworkType(NW_REPORT_RADIO_LTE);
    SetDataConnected(IMS_TRUE);
    SetSsacBarred(IMS_FALSE);

    SsacInfo objSsacInfo;
    objSsacInfo.nBarringFactorForVoice = 0;

    EXPECT_CALL(m_objMockIAosNConfiguration, IsWfcImsAvailable())
            .Times(1)
            .WillOnce(Return(IMS_TRUE));

    EXPECT_CALL(m_objMockIAosNConfiguration, IsPlmnBlockWithTimeoutOnVoiceCallUnavailable())
            .Times(1)
            .WillOnce(Return(IMS_FALSE));

    ImsRadio_OnSsacChanged(objSsacInfo);
    EXPECT_TRUE(IsSsacBarred());
    EXPECT_TRUE(IsHandleBlocked(AosHandle::BLOCK_SSAC));
}

TEST_F(AosHandleMtcTest, ImsRadio_OnSsacChanged_Test4)
{
    // Test4: Barring factor is 100, BLOCK_SSAC not blocked
    // Expectation: m_SsacBarred is set to false

    EXPECT_CALL(m_objMockIAosNConfiguration, IsRequiredVolteBlockBySsac())
            .Times(AnyNumber())
            .WillRepeatedly(Return(IMS_TRUE));

    SetSsacBarred(IMS_TRUE);

    SsacInfo objSsacInfo;
    objSsacInfo.nBarringFactorForVoice = 100;

    ImsRadio_OnSsacChanged(objSsacInfo);
    EXPECT_FALSE(IsSsacBarred());
}

TEST_F(AosHandleMtcTest, ImsRadio_OnSsacChanged_Test5)
{
    // Test5: Barring factor is 100, BLOCK_SSAC blocked, Not Volte_hys timer conditions
    // Expectation: m_SsacBarred is set to false, BLOCK_SSAC is not blocked

    EXPECT_CALL(m_objMockIAosNConfiguration, IsRequiredVolteBlockBySsac())
            .Times(AnyNumber())
            .WillRepeatedly(Return(IMS_TRUE));

    SetDataConnected(IMS_TRUE);

    EXPECT_CALL(m_objMockIAosNConfiguration, IsWfcImsAvailable())
            .Times(AnyNumber())
            .WillRepeatedly(Return(IMS_TRUE));

    EXPECT_CALL(m_objMockIAosNConfiguration, GetVolteHysTime())
            .Times(3)
            .WillOnce(Return(0))
            .WillOnce(Return(60))
            .WillOnce(Return(60));

    SsacInfo objSsacInfo;
    objSsacInfo.nBarringFactorForVoice = 100;

    SetSsacBarred(IMS_TRUE);
    AddBlock(AosHandle::BLOCK_SSAC);

    ImsRadio_OnSsacChanged(objSsacInfo);
    EXPECT_FALSE(IsSsacBarred());
    EXPECT_FALSE(IsHandleBlocked(AosHandle::BLOCK_SSAC));

    SetSsacBarred(IMS_TRUE);
    AddBlock(AosHandle::BLOCK_SSAC);
    SetNetworkType(NW_REPORT_RADIO_NR);

    ImsRadio_OnSsacChanged(objSsacInfo);
    EXPECT_FALSE(IsSsacBarred());
    EXPECT_FALSE(IsHandleBlocked(AosHandle::BLOCK_SSAC));

    SetSsacBarred(IMS_TRUE);
    AddBlock(AosHandle::BLOCK_SSAC);
    SetNetworkType(NW_REPORT_RADIO_LTE);
    AddBlock(AosHandle::BLOCK_VOPS);

    ImsRadio_OnSsacChanged(objSsacInfo);
    EXPECT_FALSE(IsSsacBarred());
    EXPECT_FALSE(IsHandleBlocked(AosHandle::BLOCK_SSAC));
}

TEST_F(AosHandleMtcTest, ImsRadio_OnSsacChanged_Test6)
{
    // Test6: Barring factor is 100, BLOCK_SSAC blocked, Volte_hys timer conditions
    // Expectation: m_SsacBarred is set to false, BLOCK_SSAC is blocked, the timer is running.

    EXPECT_CALL(m_objMockIAosNConfiguration, IsRequiredVolteBlockBySsac())
            .Times(AnyNumber())
            .WillRepeatedly(Return(IMS_TRUE));

    EXPECT_CALL(m_objMockIAosNConfiguration, GetVolteHysTime()).Times(1).WillOnce(Return(60));

    SetNetworkType(NW_REPORT_RADIO_LTE);

    SsacInfo objSsacInfo;
    objSsacInfo.nBarringFactorForVoice = 100;

    SetSsacBarred(IMS_TRUE);
    AddBlock(AosHandle::BLOCK_SSAC);

    ImsRadio_OnSsacChanged(objSsacInfo);
    EXPECT_FALSE(IsSsacBarred());
    EXPECT_TRUE(IsHandleBlocked(AosHandle::BLOCK_SSAC));
    EXPECT_TRUE(IsVolteHysTimerRunning());

    StopVolteHysTimer();
}

TEST_F(AosHandleMtcTest, ImsRadio_OnSsacChanged_VolteHysTimerRunning_by_Vops)
{
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

    EXPECT_CALL(m_objMockIAosNConfiguration, GetVolteHysTime())
            .Times(AnyNumber())
            .WillRepeatedly(Return(60));

    EXPECT_CALL(m_objMockIAosNConfiguration, IsPlmnBlockWithTimeoutOnVoiceCallUnavailable())
            .Times(AnyNumber())
            .WillRepeatedly(Return(IMS_FALSE));

    EXPECT_CALL(m_objMockIAosNConfiguration, IsRequiredVolteBlockBySsac())
            .Times(AnyNumber())
            .WillRepeatedly(Return(IMS_TRUE));

    SetNetworkType(NW_REPORT_RADIO_LTE);

    ProcessVopsStateChanged(IMS_VOICE_OVER_PS_NOT_SUPPORTED);
    ASSERT_EQ(GetVopsState(), IMS_VOICE_OVER_PS_NOT_SUPPORTED);
    ASSERT_TRUE(IsHandleBlocked(AosHandle::BLOCK_VOPS));

    ProcessVopsStateChanged(IMS_VOICE_OVER_PS_SUPPORTED);
    ASSERT_TRUE(IsVolteHysTimerRunning());
    ASSERT_EQ(GetVopsState(), IMS_VOICE_OVER_PS_SUPPORTED);
    ASSERT_TRUE(IsHandleBlocked(AosHandle::BLOCK_VOPS));

    SsacInfo objSsacInfo;
    objSsacInfo.nBarringFactorForVoice = 0;

    ImsRadio_OnSsacChanged(objSsacInfo);
    EXPECT_FALSE(IsVolteHysTimerRunning());
    EXPECT_TRUE(IsSsacBarred());
    EXPECT_TRUE(IsHandleBlocked(AosHandle::BLOCK_SSAC));
    EXPECT_FALSE(IsHandleBlocked(AosHandle::BLOCK_VOPS));
}

TEST_F(AosHandleMtcTest, ServicePhone_PlmnChanged_Test)
{
    // Expectation: Do nothing if invalid network or Volte_hys time is 0.
    // Vops state is changed to support in advance if it will be changed.
    // Run VolteHysTimerExpired if the timer is running.

    EXPECT_CALL(m_objMockIAosNetTracker, GetMobileNetworkType())
            .Times(AnyNumber())
            .WillOnce(Return(NW_REPORT_RADIO_CDMA))
            .WillRepeatedly(Return(NW_REPORT_RADIO_LTE));
    ServicePhone_PlmnChanged();  // Nothing to do on invalid network

    EXPECT_CALL(m_objMockIAosNConfiguration, GetVolteHysTime())
            .Times(AnyNumber())
            .WillOnce(Return(0))
            .WillRepeatedly(Return(60));
    ServicePhone_PlmnChanged();  // Nothing to do for no hys timer support

    AddBlock(AosHandle::BLOCK_VOPS);
    StartVolteHysTimer(60);
    ASSERT_TRUE(IsVolteHysTimerRunning());
    ServicePhone_PlmnChanged();
    EXPECT_FALSE(IsVolteHysTimerRunning());
    EXPECT_FALSE(IsHandleBlocked(AosHandle::BLOCK_VOPS));
}

TEST_F(AosHandleMtcTest, Timer_TimerExpired_Test)
{
    // Expectation: Do nothing if the timer is null or other than VolteHysTimer.
    //              Else run ProcessVolteHysTimerExpired.

    StartVolteHysTimer(60);
    Timer_TimerExpired(IMS_NULL);
    EXPECT_TRUE(IsVolteHysTimerRunning());

    ITimer* piTestTimer = AosUtil::GetInstance()->StartTimer(
            60 * 1000, m_pAosHandleMtc, "AosHandleMtcTest_Timer");

    Timer_TimerExpired(piTestTimer);
    EXPECT_TRUE(IsVolteHysTimerRunning());

    Timer_TimerExpired(GetVolteHysTimer());
    EXPECT_FALSE(IsVolteHysTimerRunning());

    AosUtil::GetInstance()->StopTimer(piTestTimer, "AosHandleMtcTest_Timer");
}

TEST_F(AosHandleMtcTest, StopVolteHysTimer_PdnLost_Then_UmtsGsm)
{
    EXPECT_CALL(m_objMockIAosNConfiguration, IsRegWithFeatureTagUnavailableSupported())
            .Times(AnyNumber())
            .WillRepeatedly(Return(IMS_FALSE));

    EXPECT_CALL(m_objMockIAosConnection, IsEpdgEnabled())
            .Times(AnyNumber())
            .WillRepeatedly(Return(IMS_FALSE));
    EXPECT_CALL(m_objMockIAosNConfiguration, GetVolteHysTime())
            .Times(AnyNumber())
            .WillRepeatedly(Return(60));

    EXPECT_CALL(m_objMockIAosConnection, GetState())
            .Times(AnyNumber())
            .WillRepeatedly(Return(IAosConnection::STATE_IDLE));

    EXPECT_CALL(m_objMockIAosNetTracker, GetNetworkType())
            .Times(AnyNumber())
            .WillRepeatedly(Return(NW_REPORT_RADIO_LTE));

    EXPECT_CALL(m_objMockIAosNetTracker, IsSuspended())
            .Times(AnyNumber())
            .WillRepeatedly(Return(IMS_FALSE));

    SetNetSrvIn(IMS_TRUE);
    SetHandleState(AosHandle::STATE_CONNECTED);
    SetDataConnected(IMS_TRUE);
    SetNetworkType(NW_REPORT_RADIO_LTE);
    AddBlock(AosHandle::BLOCK_SSAC);
    StartVolteHysTimer(60);

    m_pAosHandleMtc->NetTracker_StatusChanged();  // Data changed
    EXPECT_FALSE(IsDataConnected());
    EXPECT_EQ(GetNetworkType(), NW_REPORT_RADIO_LTE);
    EXPECT_TRUE(IsVolteHysTimerRunning());

    EXPECT_CALL(m_objMockIAosNetTracker, GetNetworkType())
            .Times(AnyNumber())
            .WillRepeatedly(Return(NW_REPORT_RADIO_GSM));

    m_pAosHandleMtc->NetTracker_StatusChanged();  // Moves to unsupported RAT(=GSM)
    EXPECT_EQ(GetNetworkType(), NW_REPORT_RADIO_GSM);
    EXPECT_FALSE(IsVolteHysTimerRunning());
    EXPECT_FALSE(IsHandleBlocked(AosHandle::BLOCK_SSAC));
}

TEST_F(AosHandleMtcTest, StopVolteHysTimer_UmtsGsm_Then_PdnLost)
{
    EXPECT_CALL(m_objMockIAosNConfiguration, IsRegWithFeatureTagUnavailableSupported())
            .Times(AnyNumber())
            .WillRepeatedly(Return(IMS_FALSE));

    EXPECT_CALL(m_objMockIAosConnection, IsEpdgEnabled())
            .Times(AnyNumber())
            .WillRepeatedly(Return(IMS_FALSE));
    EXPECT_CALL(m_objMockIAosNConfiguration, GetVolteHysTime())
            .Times(AnyNumber())
            .WillRepeatedly(Return(60));

    EXPECT_CALL(m_objMockIAosConnection, GetState())
            .Times(AnyNumber())
            .WillRepeatedly(Return(IAosConnection::STATE_ACTIVE));

    EXPECT_CALL(m_objMockIAosNetTracker, GetNetworkType())
            .Times(AnyNumber())
            .WillRepeatedly(Return(NW_REPORT_RADIO_WCDMA));

    EXPECT_CALL(m_objMockIAosNetTracker, IsSuspended())
            .Times(AnyNumber())
            .WillRepeatedly(Return(IMS_FALSE));

    SetNetSrvIn(IMS_TRUE);
    SetHandleState(AosHandle::STATE_CONNECTED);
    SetDataConnected(IMS_TRUE);
    SetNetworkType(NW_REPORT_RADIO_LTE);
    AddBlock(AosHandle::BLOCK_SSAC);
    StartVolteHysTimer(60);

    m_pAosHandleMtc->NetTracker_StatusChanged();  // Moves to unsupported RAT(=WCDMA)
    EXPECT_EQ(GetNetworkType(), NW_REPORT_RADIO_WCDMA);
    EXPECT_TRUE(IsDataConnected());
    EXPECT_TRUE(IsVolteHysTimerRunning());

    EXPECT_CALL(m_objMockIAosConnection, GetState())
            .Times(AnyNumber())
            .WillRepeatedly(Return(IAosConnection::STATE_IDLE));

    m_pAosHandleMtc->NetTracker_StatusChanged();  // Data changed
    EXPECT_FALSE(IsDataConnected());
    EXPECT_EQ(GetNetworkType(), NW_REPORT_RADIO_WCDMA);
    EXPECT_FALSE(IsVolteHysTimerRunning());
    EXPECT_FALSE(IsHandleBlocked(AosHandle::BLOCK_SSAC));
}
