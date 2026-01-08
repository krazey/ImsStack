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

#include "AosAppRequestType.h"
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
using ::testing::DoAll;
using ::testing::Return;
using ::testing::ReturnRef;
using ::testing::SetArgReferee;

#define DECLARE_USING(Base)                         \
    using Base::CheckSuspended;                     \
    using Base::CleanUp;                            \
    using Base::GetFeatures;                        \
    using Base::GetVideoBlockReasonForIpcan;        \
    using Base::GetVoiceBlockReasonForIpcan;        \
    using Base::Init;                               \
    using Base::InitializeFeatureTags;              \
    using Base::InitializeHoldingBlocksPolicy;      \
    using Base::InitializeServiceBlock;             \
    using Base::InitializeServiceFeature;           \
    using Base::IsBlocked;                          \
    using Base::IsBlockForMobile;                   \
    using Base::IsBlockForWifi;                     \
    using Base::IsCsFeatureTagRequired;             \
    using Base::IsFeatureBlocked;                   \
    using Base::IsHandleBlocked;                    \
    using Base::IsInvalidMobileNetwork;             \
    using Base::IsPlmnBlockCondition;               \
    using Base::IsVoiceCapableOnWiFiCalling;        \
    using Base::IsVolteHysTimerRunning;             \
    using Base::IsVolteHysTimerStartingCondition;   \
    using Base::ImsRadio_OnSsacChanged;             \
    using Base::NConfiguration_NotifyConfigChanged; \
    using Base::ProcessBlockChanged;                \
    using Base::ProcessCapabilitiesChanged;         \
    using Base::ProcessFeatureBlock;                \
    using Base::ProcessHoldingSsacState;            \
    using Base::ProcessHoldingVopsState;            \
    using Base::ProcessNetworkChanged;              \
    using Base::ProcessVolteHysTimerExpired;        \
    using Base::ProcessVopsStateChanged;            \
    using Base::ReevaluateCapabilities;             \
    using Base::ReevaluateUnavailableFeature;       \
    using Base::Request;                            \
    using Base::ResetSuspendedReason;               \
    using Base::ServicePhone_PlmnChanged;           \
    using Base::ServicePhone_VopsStateChanged;      \
    using Base::SetHandleState;                     \
    using Base::SetSuspendedReason;                 \
    using Base::StartVolteHysTimer;                 \
    using Base::StopVolteHysTimer;                  \
    using Base::Timer_TimerExpired;                 \
    using Base::UpdateGGsmaRcsTelephonyFeatureTag;

class TestAosHandleMtc : public AosHandleMtc
{
public:
    DECLARE_USING(AosHandleMtc)

    inline TestAosHandleMtc(IN IAosAppContext* piAppContext, IN const AString& strAppId,
            IN const AString& strServiceId, IN const IMS_SINT32 nServiceType) :
            AosHandleMtc(piAppContext, strAppId, strServiceId, nServiceType)
    {
    }

    inline ImsMap<IMS_UINT32, IMS_UINT32> GetCapabilities() { return m_objCapabilities; }
    inline void SetCapabilities(IN const ImsMap<IMS_UINT32, IMS_UINT32>& objNewCapabilities)
    {
        m_objCapabilities = objNewCapabilities;
    }
    inline void SetHoldingVopsState(IN IMS_UINT32 nState) { m_nHoldingVopsState = nState; }
    inline IMS_UINT32 GetHoldingVopsState() { return m_nHoldingVopsState; }
    inline IMS_UINT32 GetVopsState() { return m_nVopsState; }
    inline void SetVopsState(IN IMS_UINT32 nState) { m_nVopsState = nState; }
    inline void SetVopsIgnoredForVolteEnabled(IN IMS_BOOL bIgnored)
    {
        m_bVopsIgnoredForVolteEnabled = bIgnored;
    }
    inline IMS_BOOL IsVopsIgnoredForVolteEnabled() { return m_bVopsIgnoredForVolteEnabled; }
    inline void SetVopsPlmn(IN const AString& strPlmn) { m_strVopsPlmn = strPlmn; }
    inline void SetSsacPlmn(IN const AString& strPlmn) { m_strSsacPlmn = strPlmn; }
    inline void SetEpdgEnabled(IN IMS_BOOL bEnabled) { m_bEpdgEnabled = bEnabled; }
    inline IMS_BOOL GetNetSrvIn() { return m_bNetSrvIn; }
    inline void SetNetSrvIn(IMS_BOOL bNetSrvIn) { m_bNetSrvIn = bNetSrvIn; }
    inline void SetDataConnected(IN IMS_BOOL bConnected) { m_bDataConnected = bConnected; }
    inline IMS_BOOL GetDataConnected() { return m_bDataConnected; }
    inline void SetRoamingState(IN IMS_UINT32 nState) { m_nRoamingState = nState; }
    inline void SetCsVoiceAvailable(IN IMS_BOOL bIsCsVoiceAvailable)
    {
        m_bCsVoiceAvailable = bIsCsVoiceAvailable;
    }
    inline void SetSsacBarred(IN IMS_BOOL bIsSsacBarred) { m_bSsacBarred = bIsSsacBarred; }
    inline void SetSsacHeld(IN IMS_BOOL bIsSsacHeld) { m_bSsacHeld = bIsSsacHeld; }
    inline IMS_UINT32 GetStoredNetworkType() { return m_nNetworkType; }
    inline void SetNetworkType(IMS_UINT32 nNetworkType) { m_nNetworkType = nNetworkType; }
    inline IMS_BOOL IsSsacBarred() { return m_bSsacBarred; }
    inline IMS_BOOL IsSsacHeld() { return m_bSsacHeld; }
    inline ITimer* GetVolteHysTimer() { return m_piVolteHysTimer; }
    inline void SetServiceType(IN IMS_UINT32 nServiceType) { m_nServiceType = nServiceType; }

    IMS_BOOL IsHandleBlockedBase(IN IMS_UINT32 nBlock) const
    {
        return AosHandle::IsHandleBlocked(nBlock);
    }
    IMS_BOOL IsHandleBlockedBase() const { return AosHandle::IsHandleBlocked(); }
    void AddFeature(IN IMS_UINT32 nFeature) { m_objFeatureTagList.AddFeature(nFeature); }
    void RemoveFeature(IN IMS_UINT32 nFeature) { m_objFeatureTagList.RemoveFeature(nFeature); }
    void AddBindedFeature(IN IMS_UINT32 nFeature)
    {
        m_objBindedFeatureTagList.AddFeature(nFeature);
    }
    IMS_BOOL AddFeatureTag(IN const AString& strName, IN const AString& strValue)
    {
        return m_objFeatureTagList.AddFeatureTag(strName, strValue);
    }
    IMS_BOOL AddBindedFeatureTag(IN const AString& strName, IN const AString& strValue)
    {
        return m_objBindedFeatureTagList.AddFeatureTag(strName, strValue);
    }
    void AddBlock(IN IMS_UINT32 nBlock) { AosHandle::AddBlock(nBlock, m_nBlocks); }
    void RemoveBlock(IN IMS_UINT32 nBlock) { AosHandle::RemoveBlock(nBlock, m_nBlocks); }
    IMS_BOOL IsHoldingBlockForMobile(IN IMS_UINT32 nBlock)
    {
        return AosHandle::IsHandleBlocked(m_nHoldingBlocksForMobile, nBlock);
    }
    IMS_BOOL IsHoldingBlockForWifi(IN IMS_UINT32 nBlock)
    {
        return AosHandle::IsHandleBlocked(m_nHoldingBlocksForWifi, nBlock);
    }
    void AddHoldingBlockForMobile(IN IMS_UINT32 nBlock)
    {
        AosHandle::AddBlock(nBlock, m_nHoldingBlocksForMobile);
    }
    void AddHoldingBlockForWifi(IN IMS_UINT32 nBlock)
    {
        AosHandle::AddBlock(nBlock, m_nHoldingBlocksForWifi);
    }
    void SetVolteHysTimerBlockedForVops() { SetVolteHysTimerBlock(VolteHysTimerBlock::VOPS); }
    IMS_BOOL IsVolteHysTimerBlockedForVops()
    {
        return IsVolteHysTimerBlocked(VolteHysTimerBlock::VOPS);
    }
    void SetVolteHysTimerBlockedForSsac() { SetVolteHysTimerBlock(VolteHysTimerBlock::SSAC); }
    IMS_BOOL IsVolteHysTimerBlockedForSsac()
    {
        return IsVolteHysTimerBlocked(VolteHysTimerBlock::SSAC);
    }
};

class AosHandleMtcTest : public ::testing::Test
{
public:
    TestAosHandleMtc* m_pAosHandleMtc;

    MockIAosAppContext m_objMockIAosAppContext;
    MockIAosApplication m_objMockIAosApplication;
    MockIAosConnection m_objMockIAosConnection;
    // cppcheck-suppress unusedStructMember
    MockIAosHandle m_objMockIAosHandle;
    MockIAosNetTracker m_objMockIAosNetTracker;
    MockIAosRegistration m_objMockIAosRegistration;
    // cppcheck-suppress unusedStructMember
    MockIImsAosListener m_objMockIImsAosListener;

    IAosNConfiguration* m_piAosNConfiguration;
    MockIAosNConfiguration m_objMockIAosNConfiguration;

    IAosCallTracker* m_piAosCallTracker;
    MockIAosCallTracker m_objMockIAosCallTracker;

    IAosService* m_piAosService;
    MockIAosService m_objMockIAosService;

    PlatformService* m_pPlatformService;
    TestImsRadioService m_objTestImsRadioService;

    ImsVector<IMS_SINT32> m_objKeepRegWithMmtelFeatureTagPolicy;

protected:
    void SetUp() override
    {
        ON_CALL(m_objMockIAosAppContext, GetSlotId()).WillByDefault(Return(0));

        const AString strValue = AString("test");
        ON_CALL(m_objMockIAosAppContext, GetProfileId()).WillByDefault(ReturnRef(strValue));

        const AString strAppId = AString("ims.app.mtc.test");
        const AString strServiceId = AString("ims.service.mtc.test");
        const IMS_UINT32 nServiceType = -1;

        m_piAosNConfiguration = AosProvider::GetInstance()->GetNConfiguration();
        AosProvider::GetInstance()->SetNConfiguration(&m_objMockIAosNConfiguration);

        ON_CALL(m_objMockIAosAppContext, GetApp()).WillByDefault(Return(&m_objMockIAosApplication));
        ON_CALL(m_objMockIAosAppContext, GetNetTracker())
                .WillByDefault(Return(&m_objMockIAosNetTracker));

        m_piAosCallTracker = AosProvider::GetInstance()->GetCallTracker();
        AosProvider::GetInstance()->SetCallTracker(&m_objMockIAosCallTracker, 0);

        ON_CALL(m_objMockIAosAppContext, GetRegistration())
                .WillByDefault(Return(&m_objMockIAosRegistration));

        ON_CALL(m_objMockIAosAppContext, GetConnection())
                .WillByDefault(Return(&m_objMockIAosConnection));

        ON_CALL(m_objMockIAosNConfiguration, IsVopsIgnoredForVolteEnabled())
                .WillByDefault(Return(IMS_FALSE));

        ON_CALL(m_objMockIAosNConfiguration, IsWfcImsAvailable()).WillByDefault(Return(IMS_FALSE));

        ON_CALL(m_objMockIAosNConfiguration,
                IsGGsmaRcsTelephonyFeatureTagUsedAsAvailableVoiceCallType())
                .WillByDefault(Return(IMS_FALSE));

        ON_CALL(m_objMockIAosNConfiguration, IsVerstatSupportedBasedOnNetworkForReg())
                .WillByDefault(Return(IMS_TRUE));

        m_objKeepRegWithMmtelFeatureTagPolicy.Clear();
        ON_CALL(m_objMockIAosNConfiguration, GetKeepRegWithMmtelFeatureTagPolicy())
                .WillByDefault(ReturnRef(m_objKeepRegWithMmtelFeatureTagPolicy));

        m_piAosService = AosProvider::GetInstance()->GetService();
        AosProvider::GetInstance()->SetService(&m_objMockIAosService);

        m_pPlatformService =
                PlatformContext::GetInstance()->GetService(PlatformContext::SERVICE_RADIO);
        PlatformContext::GetInstance()->SetService(
                PlatformContext::SERVICE_RADIO, &m_objTestImsRadioService);

        m_pAosHandleMtc = new TestAosHandleMtc(
                &m_objMockIAosAppContext, strAppId, strServiceId, nServiceType);

        ASSERT_TRUE(m_pAosHandleMtc != nullptr);

        m_pAosHandleMtc->SetDataConnected(IMS_TRUE);
        m_pAosHandleMtc->SetVopsIgnoredForVolteEnabled(IMS_FALSE);
    }

    void TearDown() override
    {
        if (m_pAosHandleMtc != nullptr)
        {
            m_pAosHandleMtc->StopVolteHysTimer();
            delete m_pAosHandleMtc;
            m_pAosHandleMtc = nullptr;
        }

        AosProvider::GetInstance()->SetNConfiguration(m_piAosNConfiguration);
        AosProvider::GetInstance()->SetCallTracker(m_piAosCallTracker, 0);
        AosProvider::GetInstance()->SetService(m_piAosService);
        PlatformContext::GetInstance()->SetService(
                PlatformContext::SERVICE_RADIO, m_pPlatformService);

        m_objKeepRegWithMmtelFeatureTagPolicy.Clear();
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
};

TEST_F(AosHandleMtcTest, Constructor)
{
    ASSERT_FALSE(m_pAosHandleMtc->GetCapabilities().GetIndexOfKey(
                         static_cast<IMS_UINT32>(AosNetworkType::LTE)) < 0);
    ASSERT_FALSE(m_pAosHandleMtc->GetCapabilities().GetIndexOfKey(
                         static_cast<IMS_UINT32>(AosNetworkType::IWLAN)) < 0);
    ASSERT_FALSE(m_pAosHandleMtc->GetCapabilities().GetIndexOfKey(
                         static_cast<IMS_UINT32>(AosNetworkType::NR)) < 0);

    EXPECT_TRUE((m_pAosHandleMtc->GetCapabilities().GetValue(
                         static_cast<IMS_UINT32>(AosNetworkType::LTE)) &
                        static_cast<IMS_UINT32>(AosCapability::VOICE)) > 0);
    EXPECT_TRUE((m_pAosHandleMtc->GetCapabilities().GetValue(
                         static_cast<IMS_UINT32>(AosNetworkType::LTE)) &
                        static_cast<IMS_UINT32>(AosCapability::VIDEO)) > 0);
    EXPECT_TRUE((m_pAosHandleMtc->GetCapabilities().GetValue(
                         static_cast<IMS_UINT32>(AosNetworkType::LTE)) &
                        static_cast<IMS_UINT32>(AosCapability::CALL_COMPOSER)) > 0);
    EXPECT_TRUE((m_pAosHandleMtc->GetCapabilities().GetValue(
                         static_cast<IMS_UINT32>(AosNetworkType::IWLAN)) &
                        static_cast<IMS_UINT32>(AosCapability::VOICE)) > 0);
    EXPECT_TRUE((m_pAosHandleMtc->GetCapabilities().GetValue(
                         static_cast<IMS_UINT32>(AosNetworkType::IWLAN)) &
                        static_cast<IMS_UINT32>(AosCapability::VIDEO)) > 0);
    EXPECT_TRUE((m_pAosHandleMtc->GetCapabilities().GetValue(
                         static_cast<IMS_UINT32>(AosNetworkType::NR)) &
                        static_cast<IMS_UINT32>(AosCapability::VOICE)) > 0);
    EXPECT_TRUE((m_pAosHandleMtc->GetCapabilities().GetValue(
                         static_cast<IMS_UINT32>(AosNetworkType::NR)) &
                        static_cast<IMS_UINT32>(AosCapability::VIDEO)) > 0);
    EXPECT_TRUE((m_pAosHandleMtc->GetCapabilities().GetValue(
                         static_cast<IMS_UINT32>(AosNetworkType::NR)) &
                        static_cast<IMS_UINT32>(AosCapability::CALL_COMPOSER)) > 0);
}

TEST_F(AosHandleMtcTest, Destructor)
{
    EXPECT_CALL(m_objMockIAosNConfiguration, RemoveListener(_)).Times(1);
}

TEST_F(AosHandleMtcTest, ShouldReturnBindedFeaturesIfCallComposerFeatureTagForB2cIsTrue)
{
    // GIVEN
    m_pAosHandleMtc->SetHandleState(AosHandle::STATE_CONNECTED);

    IMS_UINT32 nExpectedFeatures =
            ImsAosFeature::MMTEL | ImsAosFeature::VIDEO | ImsAosFeature::TEXT;
    m_pAosHandleMtc->GetBindedFeatureTagList().AddFeature(ImsAosFeature::MMTEL);
    m_pAosHandleMtc->GetBindedFeatureTagList().AddFeature(ImsAosFeature::VIDEO);
    m_pAosHandleMtc->GetBindedFeatureTagList().AddFeature(ImsAosFeature::TEXT);
    ON_CALL(m_objMockIAosNConfiguration, IsB2cCallComposerFeatureTagInRegContact())
            .WillByDefault(Return(IMS_TRUE));

    // WHEN
    IMS_UINT32 nFeatures = m_pAosHandleMtc->GetFeatures();

    // THEN
    EXPECT_EQ(nFeatures, nExpectedFeatures);
}

TEST_F(AosHandleMtcTest,
        ShouldReturnBindedFeaturesIfB2cCallComposerNotCapableAndCallComposerFeatureTagForB2cIsFalse)
{
    // GIVEN
    m_pAosHandleMtc->SetHandleState(AosHandle::STATE_CONNECTED);
    m_pAosHandleMtc->SetNetworkType(NW_REPORT_RADIO_LTE);

    IMS_UINT32 nExpectedFeatures =
            ImsAosFeature::MMTEL | ImsAosFeature::VIDEO | ImsAosFeature::TEXT;
    m_pAosHandleMtc->GetBindedFeatureTagList().AddFeature(ImsAosFeature::MMTEL);
    m_pAosHandleMtc->GetBindedFeatureTagList().AddFeature(ImsAosFeature::VIDEO);
    m_pAosHandleMtc->GetBindedFeatureTagList().AddFeature(ImsAosFeature::TEXT);

    ImsMap<IMS_UINT32, IMS_UINT32> objCapabilities;
    objCapabilities.Add(static_cast<IMS_UINT32>(AosNetworkType::LTE),
            static_cast<IMS_UINT32>(AosCapability::VOICE) |
                    static_cast<IMS_UINT32>(AosCapability::VIDEO) |
                    static_cast<IMS_UINT32>(AosCapability::TEXT));
    m_pAosHandleMtc->SetCapabilities(objCapabilities);

    ON_CALL(m_objMockIAosNConfiguration, IsB2cCallComposerFeatureTagInRegContact())
            .WillByDefault(Return(IMS_FALSE));

    // WHEN
    IMS_UINT32 nFeatures = m_pAosHandleMtc->GetFeatures();

    // THEN
    EXPECT_EQ(nFeatures, nExpectedFeatures);
}

TEST_F(AosHandleMtcTest,
        ShouldAddCallComposerFeatureIfB2cCallComposerCapableAndCallComposerFeatureTagForB2cIsFalse)
{
    // GIVEN
    m_pAosHandleMtc->SetHandleState(AosHandle::STATE_CONNECTED);
    m_pAosHandleMtc->SetNetworkType(NW_REPORT_RADIO_LTE);

    IMS_UINT32 nExpectedFeatures = ImsAosFeature::MMTEL | ImsAosFeature::VIDEO |
            ImsAosFeature::TEXT | ImsAosFeature::CALL_COMPOSER_VIA_TELEPHONY;
    m_pAosHandleMtc->GetBindedFeatureTagList().AddFeature(ImsAosFeature::MMTEL);
    m_pAosHandleMtc->GetBindedFeatureTagList().AddFeature(ImsAosFeature::VIDEO);
    m_pAosHandleMtc->GetBindedFeatureTagList().AddFeature(ImsAosFeature::TEXT);

    ImsMap<IMS_UINT32, IMS_UINT32> objCapabilities;
    objCapabilities.Add(static_cast<IMS_UINT32>(AosNetworkType::LTE),
            static_cast<IMS_UINT32>(AosCapability::VOICE) |
                    static_cast<IMS_UINT32>(AosCapability::VIDEO) |
                    static_cast<IMS_UINT32>(AosCapability::TEXT) |
                    static_cast<IMS_UINT32>(AosCapability::CALL_COMPOSER_BUSINESS_ONLY));
    m_pAosHandleMtc->SetCapabilities(objCapabilities);

    ON_CALL(m_objMockIAosNConfiguration, IsB2cCallComposerFeatureTagInRegContact())
            .WillByDefault(Return(IMS_FALSE));

    // WHEN
    IMS_UINT32 nFeatures = m_pAosHandleMtc->GetFeatures();

    // THEN
    EXPECT_EQ(nFeatures, nExpectedFeatures);
}

TEST_F(AosHandleMtcTest, ShouldAddVerstatFeatureIfConfiguredToNotConsiderNetworkFeature)
{
    // GIVEN
    m_pAosHandleMtc->SetHandleState(AosHandle::STATE_CONNECTED);
    ON_CALL(m_objMockIAosNConfiguration, IsVerstatSupportedBasedOnNetworkForReg())
            .WillByDefault(Return(IMS_FALSE));

    // WHEN
    IMS_UINT32 nFeatures = m_pAosHandleMtc->GetFeatures();

    // THEN
    EXPECT_TRUE((nFeatures & ImsAosFeature::VERSTAT) > 0);
}

TEST_F(AosHandleMtcTest, ShouldNotAddVerstatFeatureIfNoNetworkFeature)
{
    // GIVEN
    m_pAosHandleMtc->SetHandleState(AosHandle::STATE_CONNECTED);
    ON_CALL(m_objMockIAosNConfiguration, IsVerstatSupportedBasedOnNetworkForReg())
            .WillByDefault(Return(IMS_TRUE));
    ON_CALL(m_objMockIAosRegistration,
            GetProperty(IAosRegistration::PROPERTY_SUPPORT_CALLING_NUMBER_VERIFICATION, _, _))
            .WillByDefault(DoAll(SetArgReferee<1>(AosSupportability::NOT_SUPPORTED), Return(0)));

    // WHEN
    IMS_UINT32 nFeatures = m_pAosHandleMtc->GetFeatures();

    // THEN
    EXPECT_FALSE((nFeatures & ImsAosFeature::VERSTAT) > 0);
}

TEST_F(AosHandleMtcTest, ShouldAddVerstatFeatureIfNetworkFeatureExists)
{
    // GIVEN
    m_pAosHandleMtc->SetHandleState(AosHandle::STATE_CONNECTED);
    ON_CALL(m_objMockIAosNConfiguration, IsVerstatSupportedBasedOnNetworkForReg())
            .WillByDefault(Return(IMS_TRUE));
    ON_CALL(m_objMockIAosRegistration,
            GetProperty(IAosRegistration::PROPERTY_SUPPORT_CALLING_NUMBER_VERIFICATION, _, _))
            .WillByDefault(DoAll(SetArgReferee<1>(AosSupportability::SUPPORTED), Return(0)));

    // WHEN
    IMS_UINT32 nFeatures = m_pAosHandleMtc->GetFeatures();

    // THEN
    EXPECT_TRUE((nFeatures & ImsAosFeature::VERSTAT) > 0);
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

TEST_F(AosHandleMtcTest, ShouldPlmnBlockAndApplyHoldingVopsAndSsacWhenCallTerminated)
{
    // GIVEN
    m_pAosHandleMtc->SetNetworkType(NW_REPORT_RADIO_LTE);
    m_pAosHandleMtc->SetHoldingVopsState(IMS_VOICE_OVER_PS_NOT_SUPPORTED);
    m_pAosHandleMtc->SetSsacHeld(IMS_TRUE);

    ON_CALL(m_objMockIAosNConfiguration,
            IsGGsmaRcsTelephonyFeatureTagUsedAsAvailableVoiceCallType())
            .WillByDefault(Return(IMS_FALSE));
    ON_CALL(m_objMockIAosNConfiguration, IsWfcImsAvailable()).WillByDefault(Return(IMS_TRUE));
    ON_CALL(m_objMockIAosNConfiguration, IsPlmnBlockWithTimeoutOnVoiceCallUnavailable())
            .WillByDefault(Return(IMS_TRUE));

    EXPECT_CALL(m_objMockIAosApplication,
            RequestCmd(ImsAosControl::PLMN_BLOCK_WITH_TIMEOUT, AosReason::VOPS_NOT_SUPPORTED));

    // WHEN
    m_pAosHandleMtc->CallTracker_StateChanged(IAosCallTracker::TYPE_NORMAL, CallState::IDLE);

    // THEN: The GIVEN and the following expectations should be met.
    EXPECT_EQ(m_pAosHandleMtc->GetVopsState(), IMS_VOICE_OVER_PS_NOT_SUPPORTED);
    EXPECT_EQ(m_pAosHandleMtc->GetHoldingVopsState(), IMS_VOICE_OVER_PS_SUPPORTED);
    EXPECT_TRUE(m_pAosHandleMtc->IsHandleBlockedBase(AosHandle::BLOCK_VOPS));
    EXPECT_TRUE(m_pAosHandleMtc->IsSsacBarred());
    EXPECT_FALSE(m_pAosHandleMtc->IsSsacHeld());
    EXPECT_TRUE(m_pAosHandleMtc->IsHandleBlockedBase(AosHandle::BLOCK_SSAC));
}

TEST_F(AosHandleMtcTest, ShouldPlmnBlockIfKeepMmtelRegPolicyIsOnlyForVops)
{
    // GIVEN
    m_pAosHandleMtc->SetNetworkType(NW_REPORT_RADIO_LTE);
    m_pAosHandleMtc->SetHoldingVopsState(IMS_VOICE_OVER_PS_NOT_SUPPORTED);
    m_pAosHandleMtc->SetSsacHeld(IMS_TRUE);
    m_objKeepRegWithMmtelFeatureTagPolicy.Add(CarrierConfig::Ims::UNAVAILABLE_FEATURE_POLICY_VOPS);

    ON_CALL(m_objMockIAosNConfiguration,
            IsGGsmaRcsTelephonyFeatureTagUsedAsAvailableVoiceCallType())
            .WillByDefault(Return(IMS_FALSE));
    ON_CALL(m_objMockIAosNConfiguration, IsWfcImsAvailable()).WillByDefault(Return(IMS_TRUE));
    ON_CALL(m_objMockIAosNConfiguration, IsPlmnBlockWithTimeoutOnVoiceCallUnavailable())
            .WillByDefault(Return(IMS_TRUE));

    EXPECT_CALL(m_objMockIAosApplication,
            RequestCmd(ImsAosControl::PLMN_BLOCK_WITH_TIMEOUT, AosReason::VOPS_NOT_SUPPORTED))
            .Times(0);
    EXPECT_CALL(m_objMockIAosApplication,
            RequestCmd(ImsAosControl::PLMN_BLOCK_WITH_TIMEOUT, AosReason::SSAC_BARRED));

    // WHEN
    m_pAosHandleMtc->CallTracker_StateChanged(IAosCallTracker::TYPE_NORMAL, CallState::IDLE);

    // THEN: The GIVEN and the following expectations should be met.
    EXPECT_TRUE(m_pAosHandleMtc->GetFeatureTagList().HasUnavailableFeature(ImsAosFeature::MMTEL));
    EXPECT_TRUE(m_pAosHandleMtc->GetFeatureTagList().HasUnavailableFeature(ImsAosFeature::VIDEO));
    EXPECT_EQ(m_pAosHandleMtc->GetVopsState(), IMS_VOICE_OVER_PS_NOT_SUPPORTED);
    EXPECT_EQ(m_pAosHandleMtc->GetHoldingVopsState(), IMS_VOICE_OVER_PS_SUPPORTED);
    EXPECT_FALSE(m_pAosHandleMtc->IsHandleBlockedBase(AosHandle::BLOCK_VOPS));
    EXPECT_TRUE(m_pAosHandleMtc->IsSsacBarred());
    EXPECT_FALSE(m_pAosHandleMtc->IsSsacHeld());
    EXPECT_TRUE(m_pAosHandleMtc->IsHandleBlockedBase(AosHandle::BLOCK_SSAC));
}

TEST_F(AosHandleMtcTest,
        ShouldSetHoldingVopsAndSsacToUnavilableFeatureIfPolicyExistsWhenCallTerminated)
{
    // GIVEN
    m_pAosHandleMtc->SetNetworkType(NW_REPORT_RADIO_LTE);
    m_pAosHandleMtc->SetHoldingVopsState(IMS_VOICE_OVER_PS_NOT_SUPPORTED);
    m_pAosHandleMtc->SetSsacHeld(IMS_TRUE);
    m_objKeepRegWithMmtelFeatureTagPolicy.Add(CarrierConfig::Ims::UNAVAILABLE_FEATURE_POLICY_VOPS);
    m_objKeepRegWithMmtelFeatureTagPolicy.Add(CarrierConfig::Ims::UNAVAILABLE_FEATURE_POLICY_SSAC);

    ON_CALL(m_objMockIAosNConfiguration,
            IsGGsmaRcsTelephonyFeatureTagUsedAsAvailableVoiceCallType())
            .WillByDefault(Return(IMS_FALSE));
    ON_CALL(m_objMockIAosNConfiguration, IsWfcImsAvailable()).WillByDefault(Return(IMS_TRUE));
    ON_CALL(m_objMockIAosNConfiguration, IsPlmnBlockWithTimeoutOnVoiceCallUnavailable())
            .WillByDefault(Return(IMS_TRUE));

    EXPECT_CALL(m_objMockIAosApplication, RequestCmd(ImsAosControl::PLMN_BLOCK_WITH_TIMEOUT, _))
            .Times(0);

    // WHEN
    m_pAosHandleMtc->CallTracker_StateChanged(IAosCallTracker::TYPE_NORMAL, CallState::IDLE);

    // THEN: The GIVEN and the following expectations should be met.
    EXPECT_TRUE(m_pAosHandleMtc->GetFeatureTagList().HasUnavailableFeature(ImsAosFeature::MMTEL));
    EXPECT_TRUE(m_pAosHandleMtc->GetFeatureTagList().HasUnavailableFeature(ImsAosFeature::VIDEO));
    EXPECT_EQ(m_pAosHandleMtc->GetVopsState(), IMS_VOICE_OVER_PS_NOT_SUPPORTED);
    EXPECT_EQ(m_pAosHandleMtc->GetHoldingVopsState(), IMS_VOICE_OVER_PS_SUPPORTED);
    EXPECT_FALSE(m_pAosHandleMtc->IsHandleBlockedBase(AosHandle::BLOCK_VOPS));
    EXPECT_TRUE(m_pAosHandleMtc->IsSsacBarred());
    EXPECT_FALSE(m_pAosHandleMtc->IsSsacHeld());
    EXPECT_FALSE(m_pAosHandleMtc->IsHandleBlockedBase(AosHandle::BLOCK_SSAC));
}

TEST_F(AosHandleMtcTest, ReportSsacBarredWhenPlmnBlockDueToHoldingSsac)
{
    // GIVEN
    ON_CALL(m_objMockIAosNConfiguration, IsVopsIgnoredForVolteEnabled())
            .WillByDefault(Return(IMS_FALSE));
    ON_CALL(m_objMockIAosNConfiguration,
            IsGGsmaRcsTelephonyFeatureTagUsedAsAvailableVoiceCallType())
            .WillByDefault(Return(IMS_FALSE));
    m_pAosHandleMtc->SetNetworkType(NW_REPORT_RADIO_LTE);
    ON_CALL(m_objMockIAosNConfiguration, IsWfcImsAvailable()).WillByDefault(Return(IMS_TRUE));
    m_pAosHandleMtc->SetHoldingVopsState(IMS_VOICE_OVER_PS_SUPPORTED);
    m_pAosHandleMtc->SetSsacHeld(IMS_TRUE);
    ON_CALL(m_objMockIAosNConfiguration, IsPlmnBlockWithTimeoutOnVoiceCallUnavailable())
            .WillByDefault(Return(IMS_TRUE));

    // EXPECT
    EXPECT_CALL(m_objMockIAosApplication,
            RequestCmd(ImsAosControl::PLMN_BLOCK_WITH_TIMEOUT, AosReason::SSAC_BARRED));

    // WHEN
    m_pAosHandleMtc->CallTracker_StateChanged(IAosCallTracker::TYPE_NORMAL, CallState::IDLE);
}

TEST_F(AosHandleMtcTest, TemporarilyVoiceCapableDuringWiFiCallingWhenWfcDisabledAndVideoAvailable)
{
    // GIVEN
    ImsMap<IMS_UINT32, IMS_UINT32> objCapabilities;
    objCapabilities.Add(static_cast<IMS_UINT32>(AosNetworkType::LTE),
            static_cast<IMS_UINT32>(AosCapability::VOICE) |
                    static_cast<IMS_UINT32>(AosCapability::VIDEO) |
                    static_cast<IMS_UINT32>(AosCapability::TEXT));
    objCapabilities.Add(static_cast<IMS_UINT32>(AosNetworkType::IWLAN),
            static_cast<IMS_UINT32>(AosCapability::VIDEO));
    m_pAosHandleMtc->SetCapabilities(objCapabilities);
    m_pAosHandleMtc->SetNetworkType(NW_REPORT_RADIO_WLAN);
    m_pAosHandleMtc->SetEpdgEnabled(IMS_TRUE);
    m_pAosHandleMtc->SetDataConnected(IMS_TRUE);
    m_pAosHandleMtc->AddBlock(AosHandle::BLOCK_VOWIFI_CAPABILITY);

    ON_CALL(m_objMockIAosNConfiguration,
            IsGGsmaRcsTelephonyFeatureTagUsedAsAvailableVoiceCallType())
            .WillByDefault(Return(IMS_TRUE));
    ON_CALL(m_objMockIAosNConfiguration, IsVideoOverWifiSupportedWithoutVoice())
            .WillByDefault(Return(IMS_TRUE));
    ON_CALL(m_objMockIAosNConfiguration, IsWfcImsAvailable()).WillByDefault(Return(IMS_TRUE));
    ON_CALL(m_objMockIAosCallTracker, IsNormalCallActive()).WillByDefault(Return(IMS_TRUE));

    // WHEN
    m_pAosHandleMtc->CallTracker_StateChanged(IAosCallTracker::TYPE_NORMAL, CallState::OFFHOOK);

    // THEN
    EXPECT_FALSE(m_pAosHandleMtc->IsHandleBlockedBase(AosHandle::BLOCK_VOWIFI_CAPABILITY));
}

TEST_F(AosHandleMtcTest, BackToVoiceNotCapableIfWiFiCallingFinishedWhenWfcDisabledAndVideoAvailable)
{
    // GIVEN
    ImsMap<IMS_UINT32, IMS_UINT32> objCapabilities;
    objCapabilities.Add(static_cast<IMS_UINT32>(AosNetworkType::LTE),
            static_cast<IMS_UINT32>(AosCapability::VOICE) |
                    static_cast<IMS_UINT32>(AosCapability::VIDEO) |
                    static_cast<IMS_UINT32>(AosCapability::TEXT));
    objCapabilities.Add(static_cast<IMS_UINT32>(AosNetworkType::IWLAN),
            static_cast<IMS_UINT32>(AosCapability::VIDEO));
    m_pAosHandleMtc->SetCapabilities(objCapabilities);
    m_pAosHandleMtc->SetNetworkType(NW_REPORT_RADIO_WLAN);
    m_pAosHandleMtc->SetEpdgEnabled(IMS_TRUE);
    m_pAosHandleMtc->SetDataConnected(IMS_TRUE);

    ON_CALL(m_objMockIAosNConfiguration,
            IsGGsmaRcsTelephonyFeatureTagUsedAsAvailableVoiceCallType())
            .WillByDefault(Return(IMS_TRUE));
    ON_CALL(m_objMockIAosNConfiguration, IsVideoOverWifiSupportedWithoutVoice())
            .WillByDefault(Return(IMS_TRUE));
    ON_CALL(m_objMockIAosNConfiguration, IsWfcImsAvailable()).WillByDefault(Return(IMS_TRUE));
    ON_CALL(m_objMockIAosCallTracker, IsNormalCallActive()).WillByDefault(Return(IMS_FALSE));

    // WHEN
    m_pAosHandleMtc->CallTracker_StateChanged(IAosCallTracker::TYPE_NORMAL, CallState::IDLE);

    // THEN
    EXPECT_TRUE(m_pAosHandleMtc->IsHandleBlockedBase(AosHandle::BLOCK_VOWIFI_CAPABILITY));
}

TEST_F(AosHandleMtcTest, CsVolteFeatureTagDuringWiFiCallingWhenWfcDisabledAndVideoAvailable)
{
    // GIVEN
    ImsMap<IMS_UINT32, IMS_UINT32> objCapabilities;
    objCapabilities.Add(static_cast<IMS_UINT32>(AosNetworkType::LTE),
            static_cast<IMS_UINT32>(AosCapability::VOICE) |
                    static_cast<IMS_UINT32>(AosCapability::VIDEO) |
                    static_cast<IMS_UINT32>(AosCapability::TEXT));
    objCapabilities.Add(static_cast<IMS_UINT32>(AosNetworkType::IWLAN),
            static_cast<IMS_UINT32>(AosCapability::VIDEO));
    m_pAosHandleMtc->SetCapabilities(objCapabilities);
    m_pAosHandleMtc->SetNetworkType(NW_REPORT_RADIO_WLAN);
    m_pAosHandleMtc->SetEpdgEnabled(IMS_TRUE);
    m_pAosHandleMtc->SetDataConnected(IMS_TRUE);
    m_pAosHandleMtc->AddBlock(AosHandle::BLOCK_VOWIFI_CAPABILITY);

    m_pAosHandleMtc->AddFeature(ImsAosFeature::VIDEO);
    m_pAosHandleMtc->AddFeatureTag(FeatureTags::RCS_TELEPHONY, AosString::STR_CS_WITH_DQ);
    m_pAosHandleMtc->AddBindedFeature(ImsAosFeature::VIDEO);
    m_pAosHandleMtc->AddBindedFeatureTag(FeatureTags::RCS_TELEPHONY, AosString::STR_CS_WITH_DQ);

    AosFeatureTagList objExpectedFeatureTagListOffhook;
    objExpectedFeatureTagListOffhook.AddFeature(ImsAosFeature::MMTEL);
    objExpectedFeatureTagListOffhook.AddFeature(ImsAosFeature::VIDEO);
    objExpectedFeatureTagListOffhook.AddFeatureTag(FeatureTags::RCS_TELEPHONY, AosString::STR_CS);
    objExpectedFeatureTagListOffhook.AddFeatureTag(
            FeatureTags::RCS_TELEPHONY, AosString::STR_VOLTE);

    ON_CALL(m_objMockIAosNConfiguration,
            IsGGsmaRcsTelephonyFeatureTagUsedAsAvailableVoiceCallType())
            .WillByDefault(Return(IMS_TRUE));
    ON_CALL(m_objMockIAosNConfiguration, IsVideoOverWifiSupportedWithoutVoice())
            .WillByDefault(Return(IMS_TRUE));
    ON_CALL(m_objMockIAosNConfiguration, IsWfcImsAvailable()).WillByDefault(Return(IMS_TRUE));
    ON_CALL(m_objMockIAosCallTracker, IsNormalCallActive()).WillByDefault(Return(IMS_TRUE));

    // WHEN
    m_pAosHandleMtc->CallTracker_StateChanged(IAosCallTracker::TYPE_NORMAL, CallState::OFFHOOK);

    // THEN
    EXPECT_TRUE(m_pAosHandleMtc->GetFeatureTagList().Equals(objExpectedFeatureTagListOffhook));
}

TEST_F(AosHandleMtcTest, CsFeatureTagIfWiFiCallingFinishedWhenWfcDisabledAndVideoAvailable)
{
    // GIVEN
    ImsMap<IMS_UINT32, IMS_UINT32> objCapabilities;
    objCapabilities.Add(static_cast<IMS_UINT32>(AosNetworkType::LTE),
            static_cast<IMS_UINT32>(AosCapability::VOICE) |
                    static_cast<IMS_UINT32>(AosCapability::VIDEO) |
                    static_cast<IMS_UINT32>(AosCapability::TEXT));
    objCapabilities.Add(static_cast<IMS_UINT32>(AosNetworkType::IWLAN),
            static_cast<IMS_UINT32>(AosCapability::VIDEO));
    m_pAosHandleMtc->SetCapabilities(objCapabilities);
    m_pAosHandleMtc->SetNetworkType(NW_REPORT_RADIO_WLAN);
    m_pAosHandleMtc->SetEpdgEnabled(IMS_TRUE);
    m_pAosHandleMtc->SetDataConnected(IMS_TRUE);
    m_pAosHandleMtc->AddBlock(AosHandle::BLOCK_VOWIFI_CAPABILITY);

    m_pAosHandleMtc->AddFeature(ImsAosFeature::MMTEL);
    m_pAosHandleMtc->AddFeature(ImsAosFeature::VIDEO);
    m_pAosHandleMtc->AddFeatureTag(FeatureTags::RCS_TELEPHONY, AosString::STR_CS);
    m_pAosHandleMtc->AddFeatureTag(FeatureTags::RCS_TELEPHONY, AosString::STR_VOLTE);
    m_pAosHandleMtc->AddBindedFeature(ImsAosFeature::MMTEL);
    m_pAosHandleMtc->AddBindedFeature(ImsAosFeature::VIDEO);
    m_pAosHandleMtc->AddBindedFeatureTag(FeatureTags::RCS_TELEPHONY, AosString::STR_CS);
    m_pAosHandleMtc->AddBindedFeatureTag(FeatureTags::RCS_TELEPHONY, AosString::STR_VOLTE);

    AosFeatureTagList objExpectedFeatureTagListIdle;
    objExpectedFeatureTagListIdle.AddFeature(ImsAosFeature::VIDEO);
    objExpectedFeatureTagListIdle.AddFeatureTag(
            FeatureTags::RCS_TELEPHONY, AosString::STR_CS_WITH_DQ);

    ON_CALL(m_objMockIAosNConfiguration,
            IsGGsmaRcsTelephonyFeatureTagUsedAsAvailableVoiceCallType())
            .WillByDefault(Return(IMS_TRUE));
    ON_CALL(m_objMockIAosNConfiguration, IsVideoOverWifiSupportedWithoutVoice())
            .WillByDefault(Return(IMS_TRUE));
    ON_CALL(m_objMockIAosNConfiguration, IsWfcImsAvailable()).WillByDefault(Return(IMS_TRUE));
    ON_CALL(m_objMockIAosCallTracker, IsNormalCallActive()).WillByDefault(Return(IMS_FALSE));
    ON_CALL(m_objMockIAosNetTracker, GetMobileNetworkType())
            .WillByDefault(Return(NW_REPORT_RADIO_GSM));

    // WHEN
    m_pAosHandleMtc->CallTracker_StateChanged(IAosCallTracker::TYPE_NORMAL, CallState::IDLE);

    // THEN
    EXPECT_TRUE(m_pAosHandleMtc->GetFeatureTagList().Equals(objExpectedFeatureTagListIdle));
}

TEST_F(AosHandleMtcTest, SetBlockNetworkIfNetworkIs3gWhenCallTerminated)
{
    // GIVEN
    m_pAosHandleMtc->SetNetworkType(NW_REPORT_RADIO_WCDMA);

    // WHEN
    m_pAosHandleMtc->CallTracker_StateChanged(IAosCallTracker::TYPE_NORMAL, CallState::IDLE);

    // THEN
    EXPECT_TRUE(m_pAosHandleMtc->IsHandleBlockedBase(AosHandle::BLOCK_NETWORK));
}

TEST_F(AosHandleMtcTest, SetMmtelAsUnavailableFeatureIfNetworkIs3gWhenCallTerminated)
{
    // GIVEN
    m_pAosHandleMtc->SetNetworkType(NW_REPORT_RADIO_WCDMA);
    m_objKeepRegWithMmtelFeatureTagPolicy.Add(CarrierConfig::Ims::UNAVAILABLE_FEATURE_POLICY_3G);

    // WHEN
    m_pAosHandleMtc->CallTracker_StateChanged(IAosCallTracker::TYPE_NORMAL, CallState::IDLE);

    // THEN
    EXPECT_TRUE(m_pAosHandleMtc->GetFeatureTagList().HasUnavailableFeature(ImsAosFeature::MMTEL));
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

    m_pAosHandleMtc->SetNetSrvIn(IMS_TRUE);
    m_pAosHandleMtc->SetHandleState(AosHandle::STATE_CONNECTED);
    m_pAosHandleMtc->SetSuspendedReason(AosReason::SUSPEND_NO_LTE_COVERAGE);
    m_pAosHandleMtc->SetDataConnected(IMS_TRUE);
    m_pAosHandleMtc->AddBlock(AosHandle::BLOCK_VILTE_CAPABILITY);

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

    m_pAosHandleMtc->SetListener(&m_objMockIImsAosListener);
    EXPECT_CALL(m_objMockIImsAosListener, ImsAos_Resumed()).Times(1);

    m_pAosHandleMtc->NetTracker_StatusChanged();

    EXPECT_EQ(m_pAosHandleMtc->GetSuspendedReason(), AosReason::SUSPEND_NONE);
    EXPECT_FALSE(m_pAosHandleMtc->IsHandleBlockedBase(AosHandle::BLOCK_VILTE_CAPABILITY));
}

TEST_F(AosHandleMtcTest, NetTracker_StatusChanged_Test2)
{
    // Test2: Srv In, Network changed to 2G(GSM)
    // Expectation: Set SUSPEND_NO_LTE_COVERAGE, call ImsAos_Suspended()

    m_pAosHandleMtc->SetNetSrvIn(IMS_TRUE);
    m_pAosHandleMtc->SetHandleState(AosHandle::STATE_CONNECTED);
    m_pAosHandleMtc->SetDataConnected(IMS_TRUE);

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

    m_pAosHandleMtc->SetListener(&m_objMockIImsAosListener);
    EXPECT_CALL(m_objMockIImsAosListener, ImsAos_Suspended(ImsAosReason::SUSPEND_NO_RAT_COVERAGE))
            .Times(1);

    m_pAosHandleMtc->NetTracker_StatusChanged();

    EXPECT_EQ(m_pAosHandleMtc->GetSuspendedReason(), AosReason::SUSPEND_NO_LTE_COVERAGE);
}

TEST_F(AosHandleMtcTest, NetTracker_StatusChanged_Test3)
{
    // Test3: Srv In, Network not changed (LTE)
    // Expectation: No suspended reason, no call ImsAos_Suspended() or ImsAos_Resumed()

    m_pAosHandleMtc->SetNetSrvIn(IMS_TRUE);
    m_pAosHandleMtc->SetNetworkType(NW_REPORT_RADIO_LTE);
    m_pAosHandleMtc->SetHandleState(AosHandle::STATE_CONNECTED);
    m_pAosHandleMtc->SetDataConnected(IMS_TRUE);

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

    m_pAosHandleMtc->SetListener(&m_objMockIImsAosListener);
    EXPECT_CALL(m_objMockIImsAosListener, ImsAos_Suspended(_)).Times(0);
    EXPECT_CALL(m_objMockIImsAosListener, ImsAos_Resumed()).Times(0);

    m_pAosHandleMtc->NetTracker_StatusChanged();

    EXPECT_EQ(m_pAosHandleMtc->GetSuspendedReason(), AosReason::SUSPEND_NONE);
}

TEST_F(AosHandleMtcTest, NetTracker_StatusChanged_Test4)
{
    // Test4: Srv In, Network changed to 3G(WCDMA) / Unavailable feature policy is existed
    // Expectation: Set SUSPEND_NO_LTE_COVERAGE, call ImsAos_Suspended()

    m_pAosHandleMtc->SetNetSrvIn(IMS_TRUE);
    m_pAosHandleMtc->SetHandleState(AosHandle::STATE_CONNECTED);
    m_pAosHandleMtc->SetDataConnected(IMS_TRUE);
    m_objKeepRegWithMmtelFeatureTagPolicy.Add(CarrierConfig::Ims::UNAVAILABLE_FEATURE_POLICY_3G);

    EXPECT_CALL(m_objMockIAosNetTracker, IsSuspended())
            .Times(AnyNumber())
            .WillRepeatedly(Return(IMS_FALSE));

    EXPECT_CALL(m_objMockIAosNetTracker, GetNetworkType())
            .Times(AnyNumber())
            .WillRepeatedly(Return(NW_REPORT_RADIO_WCDMA));

    EXPECT_CALL(m_objMockIAosConnection, GetState())
            .Times(AnyNumber())
            .WillRepeatedly(Return(IAosConnection::STATE_ACTIVE));

    EXPECT_CALL(m_objMockIAosConnection, IsEpdgEnabled())
            .Times(AnyNumber())
            .WillRepeatedly(Return(IMS_FALSE));

    m_pAosHandleMtc->SetListener(&m_objMockIImsAosListener);
    EXPECT_CALL(m_objMockIImsAosListener, ImsAos_Suspended(ImsAosReason::SUSPEND_NO_RAT_COVERAGE))
            .Times(1);

    m_pAosHandleMtc->NetTracker_StatusChanged();

    EXPECT_EQ(m_pAosHandleMtc->GetSuspendedReason(), AosReason::SUSPEND_NO_LTE_COVERAGE);
}

TEST_F(AosHandleMtcTest, NetTracker_StatusChanged_Test5)
{
    // Test5: Srv In, Network changed to 3G(WCDMA) / No unavailable feature policy
    // Expectation: Suspended reason SUSPEND_NONE (cleared when detach), call ImsAos_Suspended()

    m_pAosHandleMtc->SetNetSrvIn(IMS_TRUE);
    m_pAosHandleMtc->SetHandleState(AosHandle::STATE_CONNECTED);
    m_pAosHandleMtc->SetDataConnected(IMS_TRUE);

    EXPECT_CALL(m_objMockIAosNetTracker, IsSuspended())
            .Times(AnyNumber())
            .WillRepeatedly(Return(IMS_FALSE));

    EXPECT_CALL(m_objMockIAosNetTracker, GetNetworkType())
            .Times(AnyNumber())
            .WillRepeatedly(Return(NW_REPORT_RADIO_WCDMA));

    EXPECT_CALL(m_objMockIAosConnection, GetState())
            .Times(AnyNumber())
            .WillRepeatedly(Return(IAosConnection::STATE_ACTIVE));

    EXPECT_CALL(m_objMockIAosConnection, IsEpdgEnabled())
            .Times(AnyNumber())
            .WillRepeatedly(Return(IMS_FALSE));

    m_pAosHandleMtc->SetListener(&m_objMockIImsAosListener);
    EXPECT_CALL(m_objMockIImsAosListener, ImsAos_Suspended(ImsAosReason::SUSPEND_NO_RAT_COVERAGE))
            .Times(1);

    m_pAosHandleMtc->NetTracker_StatusChanged();

    EXPECT_TRUE(m_pAosHandleMtc->IsHandleBlockedBase(AosHandle::BLOCK_NETWORK));
    EXPECT_EQ(m_pAosHandleMtc->GetSuspendedReason(), AosReason::SUSPEND_NONE);
}

TEST_F(AosHandleMtcTest, NetTracker_StatusChanged_Test6)
{
    // Test6: Srv Out, Network changed to 3G(WCDMA) / No unavailable feature policy
    // Expectation: Suspended reason SUSPEND_NONE (cleared when detach), Blocked BLOCK_NETWORK

    m_pAosHandleMtc->SetNetSrvIn(IMS_TRUE);
    m_pAosHandleMtc->SetHandleState(AosHandle::STATE_CONNECTING);
    m_pAosHandleMtc->SetDataConnected(IMS_TRUE);

    EXPECT_CALL(m_objMockIAosNetTracker, IsSuspended())
            .Times(AnyNumber())
            .WillRepeatedly(Return(IMS_TRUE));

    EXPECT_CALL(m_objMockIAosNetTracker, GetNetworkType())
            .Times(AnyNumber())
            .WillRepeatedly(Return(NW_REPORT_RADIO_WCDMA));

    EXPECT_CALL(m_objMockIAosConnection, GetState())
            .Times(AnyNumber())
            .WillRepeatedly(Return(IAosConnection::STATE_ACTIVE));

    EXPECT_CALL(m_objMockIAosConnection, IsEpdgEnabled())
            .Times(AnyNumber())
            .WillRepeatedly(Return(IMS_FALSE));

    m_pAosHandleMtc->NetTracker_StatusChanged();

    EXPECT_TRUE(m_pAosHandleMtc->IsHandleBlockedBase(AosHandle::BLOCK_NETWORK));
    EXPECT_EQ(m_pAosHandleMtc->GetSuspendedReason(), AosReason::SUSPEND_NONE);
}

TEST_F(AosHandleMtcTest, InitializeHoldingBlocksPolicy_Test)
{
    EXPECT_CALL(m_objMockIAosNConfiguration, IsDeregOn3gNetwork())
            .Times(AnyNumber())
            .WillRepeatedly(Return(IMS_TRUE));

    m_pAosHandleMtc->InitializeHoldingBlocksPolicy();

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
    // Expectation: Set m_bBlocked depending on AosHandleMtc::m_pAosHandleMtc->IsHandleBlocked()

    m_pAosHandleMtc->AddBlock(AosHandle::BLOCK_VOPS);
    m_pAosHandleMtc->InitializeServiceBlock();
    EXPECT_TRUE(m_pAosHandleMtc->IsHandleBlockedBase());

    m_pAosHandleMtc->RemoveBlock(AosHandle::BLOCK_VOPS);
    m_pAosHandleMtc->InitializeServiceBlock();
    EXPECT_FALSE(m_pAosHandleMtc->IsHandleBlockedBase());
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

    ON_CALL(m_objMockIAosNConfiguration, IsNetworkInitiatedUssdOverImsSupported())
            .WillByDefault(Return(IMS_TRUE));

    m_pAosHandleMtc->InitializeServiceFeature();

    EXPECT_TRUE(m_pAosHandleMtc->GetFeatureTagList().HasFeature(ImsAosFeature::MMTEL));
    EXPECT_TRUE(m_pAosHandleMtc->GetFeatureTagList().HasFeature(ImsAosFeature::VIDEO));
    EXPECT_TRUE(m_pAosHandleMtc->GetFeatureTagList().HasFeature(
            ImsAosFeature::CALL_COMPOSER_VIA_TELEPHONY));
    EXPECT_TRUE(m_pAosHandleMtc->GetFeatureTagList().HasFeature(ImsAosFeature::TEXT));
    EXPECT_TRUE(m_pAosHandleMtc->GetFeatureTagList().HasFeature(ImsAosFeature::VERSTAT));
    EXPECT_TRUE(m_pAosHandleMtc->GetFeatureTagList().HasFeature(ImsAosFeature::NW_INIT_USSI));
}

TEST_F(AosHandleMtcTest, InitializeServiceFeature_Test2)
{
    // Test2: No support
    // Expectation: no features

    m_pAosHandleMtc->AddBlock(AosHandle::BLOCK_VOLTE_CAPABILITY);
    m_pAosHandleMtc->AddBlock(AosHandle::BLOCK_VILTE_CAPABILITY);
    m_pAosHandleMtc->AddBlock(AosHandle::BLOCK_CALL_COMPOSER_CAPABILITY);

    EXPECT_CALL(m_objMockIAosNConfiguration, IsVerstatForRegistrationSupported())
            .Times(AnyNumber())
            .WillRepeatedly(Return(IMS_FALSE));

    ON_CALL(m_objMockIAosNConfiguration, IsNetworkInitiatedUssdOverImsSupported())
            .WillByDefault(Return(IMS_FALSE));

    m_pAosHandleMtc->InitializeServiceFeature();

    EXPECT_FALSE(m_pAosHandleMtc->GetFeatureTagList().HasFeature(ImsAosFeature::MMTEL));
    EXPECT_FALSE(m_pAosHandleMtc->GetFeatureTagList().HasFeature(ImsAosFeature::VIDEO));
    EXPECT_FALSE(m_pAosHandleMtc->GetFeatureTagList().HasFeature(
            ImsAosFeature::CALL_COMPOSER_VIA_TELEPHONY));
    EXPECT_FALSE(m_pAosHandleMtc->GetFeatureTagList().HasFeature(ImsAosFeature::VERSTAT));
    EXPECT_FALSE(m_pAosHandleMtc->GetFeatureTagList().HasFeature(ImsAosFeature::NW_INIT_USSI));
}

TEST_F(AosHandleMtcTest,
        TextFeatureShouldBeAddedToTheListIfTextCapabilityIsNotBlockedWhenInitializeServiceFeature)
{
    // WHEN
    m_pAosHandleMtc->InitializeServiceFeature();

    // THEN
    EXPECT_TRUE(m_pAosHandleMtc->GetFeatureTagList().HasFeature(ImsAosFeature::TEXT));
}

TEST_F(AosHandleMtcTest,
        TextFeatureShouldNotBeAddedToTheListIfTextCapabilityIsBlockedWhenInitializeServiceFeature)
{
    // GIVEN
    m_pAosHandleMtc->AddBlock(AosHandle::BLOCK_TEXT_CAPABILITY);

    // WHEN
    m_pAosHandleMtc->InitializeServiceFeature();

    // THEN
    EXPECT_FALSE(m_pAosHandleMtc->GetFeatureTagList().HasFeature(ImsAosFeature::TEXT));
}

TEST_F(AosHandleMtcTest, InitializeFeatureTags_Test)
{
    // Expectation: Include +cdmaless, "cs,volte" feature tags

    m_pAosHandleMtc->AddFeature(ImsAosFeature::MMTEL);

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

    m_pAosHandleMtc->InitializeFeatureTags();

    EXPECT_TRUE(m_pAosHandleMtc->GetFeatureTagList().Equals(objExpectedFeatureTagList));
}

TEST_F(AosHandleMtcTest, UpdateGGsmaRcsTelephonyFeatureTag_Test1)
{
    // Test1: vzw vowifi "cs" feature tag for call test. Idle -> Offhook -> Idle
    // Expectation: "cs" in idle, "cs,volte" in offhook

    m_pAosHandleMtc->AddFeature(ImsAosFeature::VIDEO);
    m_pAosHandleMtc->AddFeatureTag(FeatureTags::RCS_TELEPHONY, AosString::STR_CS_WITH_DQ);
    m_pAosHandleMtc->AddBindedFeature(ImsAosFeature::VIDEO);
    m_pAosHandleMtc->AddBindedFeatureTag(FeatureTags::RCS_TELEPHONY, AosString::STR_CS_WITH_DQ);

    m_pAosHandleMtc->SetEpdgEnabled(IMS_TRUE);

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

    m_pAosHandleMtc->UpdateGGsmaRcsTelephonyFeatureTag();
    EXPECT_TRUE(m_pAosHandleMtc->GetFeatureTagList().Equals(objExpectedFeatureTagListOffhook));

    m_pAosHandleMtc->UpdateGGsmaRcsTelephonyFeatureTag();
    EXPECT_TRUE(m_pAosHandleMtc->GetFeatureTagList().Equals(objExpectedFeatureTagListIdle));
}

TEST_F(AosHandleMtcTest, UpdateGGsmaRcsTelephonyFeatureTag_Test2)
{
    // Test2: VZW VoWiFi IMS Registration Table Test - Wifi Only.
    // Expectation: "cs,volte" if mmtel / "cs" if no mmtel but video / none if no mmtel and video

    m_pAosHandleMtc->AddFeature(ImsAosFeature::MMTEL);
    m_pAosHandleMtc->AddFeature(ImsAosFeature::VIDEO);
    m_pAosHandleMtc->AddFeatureTag(FeatureTags::RCS_TELEPHONY, AosString::STR_CS);
    m_pAosHandleMtc->AddFeatureTag(FeatureTags::RCS_TELEPHONY, AosString::STR_VOLTE);

    m_pAosHandleMtc->SetEpdgEnabled(IMS_TRUE);

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
    m_pAosHandleMtc->RemoveFeature(ImsAosFeature::MMTEL);
    m_pAosHandleMtc->UpdateGGsmaRcsTelephonyFeatureTag();
    EXPECT_TRUE(m_pAosHandleMtc->GetFeatureTagList().Equals(objExpectedFeatureTagListVideo));

    // Mmtel
    m_pAosHandleMtc->AddFeature(ImsAosFeature::MMTEL);
    m_pAosHandleMtc->UpdateGGsmaRcsTelephonyFeatureTag();
    EXPECT_TRUE(m_pAosHandleMtc->GetFeatureTagList().Equals(objExpectedFeatureTagListMmtel));

    // No mmtel and video
    m_pAosHandleMtc->RemoveFeature(ImsAosFeature::MMTEL);
    m_pAosHandleMtc->RemoveFeature(ImsAosFeature::VIDEO);
    m_pAosHandleMtc->UpdateGGsmaRcsTelephonyFeatureTag();
    EXPECT_TRUE(m_pAosHandleMtc->GetFeatureTagList().Equals(objExpectedFeatureTagListNoFeature));
}

TEST_F(AosHandleMtcTest, UpdateGGsmaRcsTelephonyFeatureTag_Test3)
{
    // Test3: VZW VoWiFi IMS Registration Table Test - CS Roam + Wifi
    // Expectation: "cs,volte" if mmtel / "cs" if no mmtel but video / none if no mmtel and video

    m_pAosHandleMtc->AddFeature(ImsAosFeature::MMTEL);
    m_pAosHandleMtc->AddFeature(ImsAosFeature::VIDEO);
    m_pAosHandleMtc->AddFeatureTag(FeatureTags::RCS_TELEPHONY, AosString::STR_CS);
    m_pAosHandleMtc->AddFeatureTag(FeatureTags::RCS_TELEPHONY, AosString::STR_VOLTE);

    m_pAosHandleMtc->SetEpdgEnabled(IMS_TRUE);

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
    m_pAosHandleMtc->RemoveFeature(ImsAosFeature::MMTEL);
    m_pAosHandleMtc->UpdateGGsmaRcsTelephonyFeatureTag();
    EXPECT_TRUE(m_pAosHandleMtc->GetFeatureTagList().Equals(objExpectedFeatureTagListVideo));

    // Mmtel
    m_pAosHandleMtc->AddFeature(ImsAosFeature::MMTEL);
    m_pAosHandleMtc->UpdateGGsmaRcsTelephonyFeatureTag();
    EXPECT_TRUE(m_pAosHandleMtc->GetFeatureTagList().Equals(objExpectedFeatureTagListMmtel));

    // No mmtel and video
    m_pAosHandleMtc->RemoveFeature(ImsAosFeature::MMTEL);
    m_pAosHandleMtc->RemoveFeature(ImsAosFeature::VIDEO);
    m_pAosHandleMtc->UpdateGGsmaRcsTelephonyFeatureTag();
    EXPECT_TRUE(m_pAosHandleMtc->GetFeatureTagList().Equals(objExpectedFeatureTagListNoFeature));
}

TEST_F(AosHandleMtcTest, UpdateGGsmaRcsTelephonyFeatureTag_Test4)
{
    // Test4: VZW VoWiFi IMS Registration Table Test - Changing to Wifi Only.
    // Expectation: "cs,volte" if mmtel / "cs" if no mmtel but video / none if no mmtel and video

    m_pAosHandleMtc->AddFeature(ImsAosFeature::MMTEL);
    m_pAosHandleMtc->AddFeature(ImsAosFeature::VIDEO);
    m_pAosHandleMtc->AddFeatureTag(FeatureTags::RCS_TELEPHONY, AosString::STR_CS);
    m_pAosHandleMtc->AddFeatureTag(FeatureTags::RCS_TELEPHONY, AosString::STR_VOLTE);

    m_pAosHandleMtc->SetEpdgEnabled(IMS_TRUE);

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
    m_pAosHandleMtc->RemoveFeature(ImsAosFeature::MMTEL);
    m_pAosHandleMtc->UpdateGGsmaRcsTelephonyFeatureTag();
    EXPECT_TRUE(m_pAosHandleMtc->GetFeatureTagList().Equals(objExpectedFeatureTagListVideo));

    // Mmtel
    m_pAosHandleMtc->AddFeature(ImsAosFeature::MMTEL);
    m_pAosHandleMtc->UpdateGGsmaRcsTelephonyFeatureTag();
    EXPECT_TRUE(m_pAosHandleMtc->GetFeatureTagList().Equals(objExpectedFeatureTagListMmtel));

    // No mmtel and video
    m_pAosHandleMtc->RemoveFeature(ImsAosFeature::MMTEL);
    m_pAosHandleMtc->RemoveFeature(ImsAosFeature::VIDEO);
    m_pAosHandleMtc->UpdateGGsmaRcsTelephonyFeatureTag();
    EXPECT_TRUE(m_pAosHandleMtc->GetFeatureTagList().Equals(objExpectedFeatureTagListNoFeature));
}

TEST_F(AosHandleMtcTest,
        ShouldBlockMmtelFeatureIfRegModeChangedToLimitedWhileMmtelIsUnavailableInLimitedReg)
{
    // GIVEN
    ImsVector<IMS_SINT32> objUnavailableFeatures;
    objUnavailableFeatures.Add(CarrierConfig::Ims::REG_FEATURE_MMTEL);
    ON_CALL(m_objMockIAosNConfiguration, GetUnavailableFeaturesInLimitedReg())
            .WillByDefault(ReturnRef(objUnavailableFeatures));

    // WHEN
    m_pAosHandleMtc->Request(IAosHandle::TYPE_LIMITED_MODE, IAosHandle::STATE_ADD);

    // THEN
    EXPECT_TRUE(m_pAosHandleMtc->IsHandleBlockedBase(AosHandle::BLOCK_LIMITED_MMTEL));
}

TEST_F(AosHandleMtcTest,
        ShouldUnblockMmtelFeatureIfRegModeChangedToNormalWhileMmtelIsUnavailableInLimitedReg)
{
    // GIVEN
    ImsVector<IMS_SINT32> objUnavailableFeatures;
    objUnavailableFeatures.Add(CarrierConfig::Ims::REG_FEATURE_MMTEL);
    ON_CALL(m_objMockIAosNConfiguration, GetUnavailableFeaturesInLimitedReg())
            .WillByDefault(ReturnRef(objUnavailableFeatures));
    m_pAosHandleMtc->AddBlock(AosHandle::BLOCK_LIMITED_MMTEL);

    // WHEN
    m_pAosHandleMtc->Request(IAosHandle::TYPE_LIMITED_MODE, IAosHandle::STATE_REMOVE);

    // THEN
    EXPECT_FALSE(m_pAosHandleMtc->IsHandleBlockedBase(AosHandle::BLOCK_LIMITED_MMTEL));
}

TEST_F(AosHandleMtcTest,
        ShouldBlockVideoFeatureIfRegModeChangedToLimitedWhileVideoIsUnavailableInLimitedReg)
{
    // GIVEN
    ImsVector<IMS_SINT32> objUnavailableFeatures;
    objUnavailableFeatures.Add(CarrierConfig::Ims::REG_FEATURE_VIDEO);
    ON_CALL(m_objMockIAosNConfiguration, GetUnavailableFeaturesInLimitedReg())
            .WillByDefault(ReturnRef(objUnavailableFeatures));

    // WHEN
    m_pAosHandleMtc->Request(IAosHandle::TYPE_LIMITED_MODE, IAosHandle::STATE_ADD);

    // THEN
    EXPECT_TRUE(m_pAosHandleMtc->IsHandleBlockedBase(AosHandle::BLOCK_LIMITED_VIDEO));
}

TEST_F(AosHandleMtcTest,
        ShouldUnblockVideoFeatureIfRegModeChangedToNormalWhileVideoIsUnavailableInLimitedReg)
{
    // GIVEN
    ImsVector<IMS_SINT32> objUnavailableFeatures;
    objUnavailableFeatures.Add(CarrierConfig::Ims::REG_FEATURE_VIDEO);
    ON_CALL(m_objMockIAosNConfiguration, GetUnavailableFeaturesInLimitedReg())
            .WillByDefault(ReturnRef(objUnavailableFeatures));
    m_pAosHandleMtc->AddBlock(AosHandle::BLOCK_LIMITED_VIDEO);

    // WHEN
    m_pAosHandleMtc->Request(IAosHandle::TYPE_LIMITED_MODE, IAosHandle::STATE_REMOVE);

    // THEN
    EXPECT_FALSE(m_pAosHandleMtc->IsHandleBlockedBase(AosHandle::BLOCK_LIMITED_VIDEO));
}

TEST_F(AosHandleMtcTest,
        ShouldBlockTextFeatureIfRegModeChangedToLimitedWhileTextIsUnavailableInLimitedReg)
{
    // GIVEN
    ImsVector<IMS_SINT32> objUnavailableFeatures;
    objUnavailableFeatures.Add(CarrierConfig::Ims::REG_FEATURE_TEXT);
    ON_CALL(m_objMockIAosNConfiguration, GetUnavailableFeaturesInLimitedReg())
            .WillByDefault(ReturnRef(objUnavailableFeatures));

    // WHEN
    m_pAosHandleMtc->Request(IAosHandle::TYPE_LIMITED_MODE, IAosHandle::STATE_ADD);

    // THEN
    EXPECT_TRUE(m_pAosHandleMtc->IsHandleBlockedBase(AosHandle::BLOCK_LIMITED_TEXT));
}

TEST_F(AosHandleMtcTest,
        ShouldUnblockTextFeatureIfRegModeChangedToNormalWhileTextIsUnavailableInLimitedReg)
{
    // GIVEN
    ImsVector<IMS_SINT32> objUnavailableFeatures;
    objUnavailableFeatures.Add(CarrierConfig::Ims::REG_FEATURE_TEXT);
    ON_CALL(m_objMockIAosNConfiguration, GetUnavailableFeaturesInLimitedReg())
            .WillByDefault(ReturnRef(objUnavailableFeatures));
    m_pAosHandleMtc->AddBlock(AosHandle::BLOCK_LIMITED_TEXT);

    // WHEN
    m_pAosHandleMtc->Request(IAosHandle::TYPE_LIMITED_MODE, IAosHandle::STATE_REMOVE);

    // THEN
    EXPECT_FALSE(m_pAosHandleMtc->IsHandleBlockedBase(AosHandle::BLOCK_LIMITED_TEXT));
}

TEST_F(AosHandleMtcTest, CheckSuspended_WifiTest)
{
    IMS_BOOL bIsWifiTest = AosUtil::GetInstance()->IsWifiTest();
    AosUtil::GetInstance()->SetWifiTest(IMS_TRUE);

    EXPECT_CALL(m_objMockIAosAppContext, GetNetTracker()).Times(0);
    EXPECT_CALL(m_objMockIAosNetTracker, IsSuspended()).Times(0);
    EXPECT_CALL(m_objMockIAosNetTracker, GetNetworkType()).Times(0);

    m_pAosHandleMtc->CheckSuspended();

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

    m_pAosHandleMtc->CheckSuspended();
    EXPECT_EQ(m_pAosHandleMtc->GetSuspendedReason(), AosReason::SUSPEND_NO_SERVICE);

    EXPECT_FALSE(m_pAosHandleMtc->GetNetSrvIn());
    EXPECT_EQ(m_pAosHandleMtc->GetStoredNetworkType(), NW_REPORT_RADIO_LTE);
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

    m_pAosHandleMtc->CheckSuspended();
    EXPECT_EQ(m_pAosHandleMtc->GetSuspendedReason(),
            (AosReason::SUSPEND_NO_SERVICE | AosReason::SUSPEND_NO_LTE_COVERAGE));

    EXPECT_FALSE(m_pAosHandleMtc->GetNetSrvIn());
    EXPECT_EQ(m_pAosHandleMtc->GetStoredNetworkType(), NW_REPORT_RADIO_GSM);
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

    m_pAosHandleMtc->CheckSuspended();
    EXPECT_EQ(m_pAosHandleMtc->GetSuspendedReason(), AosReason::NONE);

    EXPECT_TRUE(m_pAosHandleMtc->GetNetSrvIn());
    EXPECT_EQ(m_pAosHandleMtc->GetStoredNetworkType(), NW_REPORT_RADIO_LTE);
}

TEST_F(AosHandleMtcTest, SetSuspendedReason_ResetSuspendedReason_Test)
{
    // Expectation: Only SUSPEND_NO_SERVICE and SUSPEND_NO_LTE_COVERAGE are allowed to be set/reset.

    m_pAosHandleMtc->SetSuspendedReason(AosReason::SUSPEND_NO_SERVICE);
    EXPECT_EQ(m_pAosHandleMtc->GetSuspendedReason(), AosReason::SUSPEND_NO_SERVICE);

    m_pAosHandleMtc->SetSuspendedReason(AosReason::SUSPEND_NO_LTE_COVERAGE);
    EXPECT_EQ(m_pAosHandleMtc->GetSuspendedReason(),
            (AosReason::SUSPEND_NO_SERVICE | AosReason::SUSPEND_NO_LTE_COVERAGE));

    m_pAosHandleMtc->SetSuspendedReason(AosReason::SUSPEND_CS_CALL);
    EXPECT_EQ(m_pAosHandleMtc->GetSuspendedReason(),
            (AosReason::SUSPEND_NO_SERVICE | AosReason::SUSPEND_NO_LTE_COVERAGE));

    m_pAosHandleMtc->ResetSuspendedReason(AosReason::SUSPEND_NO_SERVICE);
    EXPECT_EQ(m_pAosHandleMtc->GetSuspendedReason(), AosReason::SUSPEND_NO_LTE_COVERAGE);

    m_pAosHandleMtc->ResetSuspendedReason(AosReason::SUSPEND_CS_CALL);
    EXPECT_EQ(m_pAosHandleMtc->GetSuspendedReason(), AosReason::SUSPEND_NO_LTE_COVERAGE);

    m_pAosHandleMtc->ResetSuspendedReason(AosReason::SUSPEND_NO_LTE_COVERAGE);
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

    m_pAosHandleMtc->Init();

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

    EXPECT_FALSE(m_pAosHandleMtc->IsVopsIgnoredForVolteEnabled());

    m_pAosHandleMtc->StartVolteHysTimer(60);
    ASSERT_TRUE(m_pAosHandleMtc->IsVolteHysTimerRunning());
    m_pAosHandleMtc->CleanUp();
    EXPECT_FALSE(m_pAosHandleMtc->IsVolteHysTimerRunning());
}

TEST_F(AosHandleMtcTest, IsHandleBlocked_Test1)
{
    // Test1: Epdg enabled, BLOCK_VOWIFI_CAPABILITY, ViWiFi without Voice NOT supported
    // Expectation: return true

    m_pAosHandleMtc->SetEpdgEnabled(IMS_TRUE);
    m_pAosHandleMtc->AddBlock(AosHandle::BLOCK_VOWIFI_CAPABILITY);

    EXPECT_CALL(m_objMockIAosNConfiguration, IsVideoOverWifiSupportedWithoutVoice())
            .Times(AnyNumber())
            .WillRepeatedly(Return(IMS_FALSE));

    EXPECT_TRUE(m_pAosHandleMtc->IsHandleBlocked());
}

TEST_F(AosHandleMtcTest, IsHandleBlocked_Test2)
{
    // Test2: Epdg enabled, BLOCK_VOWIFI_CAPABILITY, BLOCK_VIWIFI_CAPABILITY
    //        ViWiFi without Voice supported
    // Expectation: return true

    m_pAosHandleMtc->SetEpdgEnabled(IMS_TRUE);
    m_pAosHandleMtc->AddBlock(AosHandle::BLOCK_VOWIFI_CAPABILITY);
    m_pAosHandleMtc->AddBlock(AosHandle::BLOCK_VIWIFI_CAPABILITY);

    EXPECT_CALL(m_objMockIAosNConfiguration, IsVideoOverWifiSupportedWithoutVoice())
            .Times(AnyNumber())
            .WillRepeatedly(Return(IMS_TRUE));

    EXPECT_TRUE(m_pAosHandleMtc->IsHandleBlocked());
}

TEST_F(AosHandleMtcTest, IsHandleBlocked_Test3)
{
    // Test3: Epdg enabled, BLOCK_VOWIFI_CAPABILITY
    //        ViWiFi without Voice supported
    // Expectation: return false

    m_pAosHandleMtc->SetEpdgEnabled(IMS_TRUE);
    m_pAosHandleMtc->AddBlock(AosHandle::BLOCK_VOWIFI_CAPABILITY);

    EXPECT_CALL(m_objMockIAosNConfiguration, IsVideoOverWifiSupportedWithoutVoice())
            .Times(AnyNumber())
            .WillRepeatedly(Return(IMS_TRUE));

    EXPECT_FALSE(m_pAosHandleMtc->IsHandleBlocked());
}

TEST_F(AosHandleMtcTest, IsHandleBlocked_Test4)
{
    // Test4: Epdg not enabled
    // Expectation: return true if BLOCK_VOPS or BLOCK_VOLTE_CAPABILITY or BLOCK_NETWORK or BLOCK_3G
    //              or BLOCK_SSAC
    //              else return false

    m_pAosHandleMtc->AddBlock(AosHandle::BLOCK_VOPS);
    m_pAosHandleMtc->AddBlock(AosHandle::BLOCK_VOLTE_CAPABILITY);
    m_pAosHandleMtc->AddBlock(AosHandle::BLOCK_NETWORK);
    m_pAosHandleMtc->AddBlock(AosHandle::BLOCK_3G);
    m_pAosHandleMtc->AddBlock(AosHandle::BLOCK_SSAC);
    EXPECT_TRUE(m_pAosHandleMtc->IsHandleBlocked());

    m_pAosHandleMtc->RemoveBlock(AosHandle::BLOCK_VOPS);
    EXPECT_TRUE(m_pAosHandleMtc->IsHandleBlocked());

    m_pAosHandleMtc->RemoveBlock(AosHandle::BLOCK_VOLTE_CAPABILITY);
    EXPECT_TRUE(m_pAosHandleMtc->IsHandleBlocked());

    m_pAosHandleMtc->RemoveBlock(AosHandle::BLOCK_NETWORK);
    EXPECT_TRUE(m_pAosHandleMtc->IsHandleBlocked());

    m_pAosHandleMtc->RemoveBlock(AosHandle::BLOCK_3G);
    EXPECT_TRUE(m_pAosHandleMtc->IsHandleBlocked());

    m_pAosHandleMtc->RemoveBlock(AosHandle::BLOCK_SSAC);
    EXPECT_FALSE(m_pAosHandleMtc->IsHandleBlocked());
}

TEST_F(AosHandleMtcTest, IsHandleBlocked_Test5)
{
    // Test5: Epdg enabled, BLOCK_VOWIFI_CAPABILITY
    //        ViWiFi without Voice supported, Changing to invalid network
    // Expectation: return true

    m_pAosHandleMtc->SetEpdgEnabled(IMS_TRUE);
    m_pAosHandleMtc->AddBlock(AosHandle::BLOCK_VOWIFI_CAPABILITY);

    EXPECT_CALL(m_objMockIAosNConfiguration, IsVideoOverWifiSupportedWithoutVoice())
            .Times(AnyNumber())
            .WillRepeatedly(Return(IMS_TRUE));

    EXPECT_CALL(m_objMockIAosNetTracker, GetMobileNetworkType())
            .Times(AnyNumber())
            .WillRepeatedly(Return(NW_REPORT_RADIO_LTE));

    EXPECT_CALL(m_objMockIAosNetTracker, GetMobileChangingNetworkType())
            .Times(AnyNumber())
            .WillRepeatedly(Return(NW_REPORT_RADIO_INVALID));

    EXPECT_FALSE(m_pAosHandleMtc->IsHandleBlocked());
}

TEST_F(AosHandleMtcTest, MmtelIsBlockedIfVolteCapabilityBlocked)
{
    // GIVEN
    m_pAosHandleMtc->AddBlock(AosHandle::BLOCK_VOLTE_CAPABILITY);

    // WHEN & THEN
    EXPECT_TRUE(m_pAosHandleMtc->IsFeatureBlocked(ImsAosFeature::MMTEL));
}

TEST_F(AosHandleMtcTest, MmtelIsBlockedIfVowifiCapabilityBlocked)
{
    // GIVEN
    m_pAosHandleMtc->AddBlock(AosHandle::BLOCK_VOWIFI_CAPABILITY);

    // WHEN & THEN
    EXPECT_TRUE(m_pAosHandleMtc->IsFeatureBlocked(ImsAosFeature::MMTEL));
}

TEST_F(AosHandleMtcTest, MmtelIsBlockedIfVopsBlocked)
{
    // GIVEN
    m_pAosHandleMtc->AddBlock(AosHandle::BLOCK_VOPS);

    // WHEN & THEN
    EXPECT_TRUE(m_pAosHandleMtc->IsFeatureBlocked(ImsAosFeature::MMTEL));
}

TEST_F(AosHandleMtcTest, MmtelIsBlockedIfSsacBlocked)
{
    // GIVEN
    m_pAosHandleMtc->AddBlock(AosHandle::BLOCK_SSAC);

    // WHEN & THEN
    EXPECT_TRUE(m_pAosHandleMtc->IsFeatureBlocked(ImsAosFeature::MMTEL));
}

TEST_F(AosHandleMtcTest, MmtelIsBlockedIfLimitedMmtelBlocked)
{
    // GIVEN
    m_pAosHandleMtc->AddBlock(AosHandle::BLOCK_LIMITED_MMTEL);

    // WHEN & THEN
    EXPECT_TRUE(m_pAosHandleMtc->IsFeatureBlocked(ImsAosFeature::MMTEL));
}

TEST_F(AosHandleMtcTest, VideoIsBlockedIfVilteCapabilityBlocked)
{
    // GIVEN
    m_pAosHandleMtc->AddBlock(AosHandle::BLOCK_VILTE_CAPABILITY);

    // WHEN & THEN
    EXPECT_TRUE(m_pAosHandleMtc->IsFeatureBlocked(ImsAosFeature::VIDEO));
}

TEST_F(AosHandleMtcTest, VideoIsBlockedIfViwifiCapabilityBlocked)
{
    // GIVEN
    m_pAosHandleMtc->AddBlock(AosHandle::BLOCK_VIWIFI_CAPABILITY);

    // WHEN & THEN
    EXPECT_TRUE(m_pAosHandleMtc->IsFeatureBlocked(ImsAosFeature::VIDEO));
}

TEST_F(AosHandleMtcTest, VideoIsBlockedIfLimitedVideoBlocked)
{
    // GIVEN
    m_pAosHandleMtc->AddBlock(AosHandle::BLOCK_LIMITED_VIDEO);

    // WHEN & THEN
    EXPECT_TRUE(m_pAosHandleMtc->IsFeatureBlocked(ImsAosFeature::VIDEO));
}

TEST_F(AosHandleMtcTest, TextIsBlockedIfTextCapabilityBlocked)
{
    // GIVEN
    m_pAosHandleMtc->AddBlock(AosHandle::BLOCK_TEXT_CAPABILITY);

    // WHEN & THEN
    EXPECT_TRUE(m_pAosHandleMtc->IsFeatureBlocked(ImsAosFeature::TEXT));
}

TEST_F(AosHandleMtcTest, TextIsBlockedIfLimitedTextBlocked)
{
    // GIVEN
    m_pAosHandleMtc->AddBlock(AosHandle::BLOCK_LIMITED_TEXT);

    // WHEN & THEN
    EXPECT_TRUE(m_pAosHandleMtc->IsFeatureBlocked(ImsAosFeature::TEXT));
}

TEST_F(AosHandleMtcTest, CallComposerIsBlockedIfCallComposerCapabilityBlocked)
{
    // GIVEN
    m_pAosHandleMtc->AddBlock(AosHandle::BLOCK_CALL_COMPOSER_CAPABILITY);

    // WHEN & THEN
    EXPECT_TRUE(m_pAosHandleMtc->IsFeatureBlocked(ImsAosFeature::CALL_COMPOSER_VIA_TELEPHONY));
}

TEST_F(AosHandleMtcTest, ShouldNotBlockNonMtcFeatures)
{
    // WHEN & THEN
    EXPECT_FALSE(m_pAosHandleMtc->IsFeatureBlocked(ImsAosFeature::SMSIP));
}

TEST_F(AosHandleMtcTest, ShouldAddFeatureIfTheFeatureNotBlocked)
{
    // GIVEN
    m_pAosHandleMtc->GetFeatureTagList().RemoveFeature(ImsAosFeature::VIDEO);

    // WHEN
    m_pAosHandleMtc->ProcessFeatureBlock(ImsAosFeature::VIDEO, IMS_FALSE);

    // THEN
    EXPECT_TRUE(m_pAosHandleMtc->GetFeatureTagList().HasFeature(ImsAosFeature::VIDEO));
}

TEST_F(AosHandleMtcTest, ShouldNotAddFeatureIfTheFeatureBlocked)
{
    // GIVEN
    m_pAosHandleMtc->GetFeatureTagList().RemoveFeature(ImsAosFeature::VIDEO);
    m_pAosHandleMtc->AddBlock(AosHandle::BLOCK_VILTE_CAPABILITY);

    // WHEN
    m_pAosHandleMtc->ProcessFeatureBlock(ImsAosFeature::VIDEO, IMS_FALSE);

    // THEN
    EXPECT_FALSE(m_pAosHandleMtc->GetFeatureTagList().HasFeature(ImsAosFeature::VIDEO));
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

    m_pAosHandleMtc->ProcessFeatureBlock(ImsAosFeature::MMTEL, IMS_FALSE);
    EXPECT_TRUE(m_pAosHandleMtc->GetFeatureTagList().Equals(objExpectedFeatureTagListMmtel));

    m_pAosHandleMtc->ProcessFeatureBlock(ImsAosFeature::MMTEL, IMS_TRUE);
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

    EXPECT_CALL(
            m_objMockIAosHandle, Handle_Notify(ImsAosService::MTC, m_pAosHandleMtc->IsBlocked()))
            .Times(0);

    m_pAosHandleMtc->ProcessBlockChanged();
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
            .WillOnce(Return(nullptr));

    EXPECT_CALL(
            m_objMockIAosHandle, Handle_Notify(ImsAosService::MTC, m_pAosHandleMtc->IsBlocked()))
            .Times(0);

    m_pAosHandleMtc->ProcessBlockChanged();
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
            .WillOnce(Return(&m_objMockIAosHandle));

    EXPECT_CALL(
            m_objMockIAosHandle, Handle_Notify(ImsAosService::MTC, m_pAosHandleMtc->IsBlocked()))
            .Times(1);

    m_pAosHandleMtc->ProcessBlockChanged();
}

TEST_F(AosHandleMtcTest, DoNothingIfEmergencyServiceWhenCapabilityChanged)
{
    // GIVEN
    m_pAosHandleMtc->SetServiceType(ImsAosService::EMERGENCY_MTC);

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
    m_pAosHandleMtc->ProcessCapabilitiesChanged(objNewCapabilities);

    // THEN
    EXPECT_TRUE(IsEqualCapabilities(m_pAosHandleMtc->GetCapabilities(), objExpectedCapabilities));
}

TEST_F(AosHandleMtcTest,
        BlockCallComposerIfB2cCallComposerIsRemovedWhileCallComposerFeatureTagForB2cIsTrue)
{
    // GIVEN
    m_pAosHandleMtc->SetNetworkType(NW_REPORT_RADIO_LTE);
    ON_CALL(m_objMockIAosNConfiguration, IsB2cCallComposerFeatureTagInRegContact())
            .WillByDefault(Return(IMS_TRUE));

    ImsMap<IMS_UINT32, IMS_UINT32> objCapabilities;
    objCapabilities.Add(static_cast<IMS_UINT32>(AosNetworkType::LTE),
            static_cast<IMS_UINT32>(AosCapability::VOICE) |
                    static_cast<IMS_UINT32>(AosCapability::VIDEO) |
                    static_cast<IMS_UINT32>(AosCapability::CALL_COMPOSER_BUSINESS_ONLY));
    m_pAosHandleMtc->SetCapabilities(objCapabilities);

    // WHEN
    objCapabilities.SetValue(static_cast<IMS_UINT32>(AosNetworkType::LTE),
            static_cast<IMS_UINT32>(AosCapability::VOICE) |
                    static_cast<IMS_UINT32>(AosCapability::VIDEO));
    m_pAosHandleMtc->ProcessCapabilitiesChanged(objCapabilities);

    // THEN
    EXPECT_TRUE(m_pAosHandleMtc->IsHandleBlockedBase(AosHandle::BLOCK_CALL_COMPOSER_CAPABILITY));
}

TEST_F(AosHandleMtcTest,
        UnblockCallComposerIfB2cCallComposerIsAddedWhileCallComposerFeatureTagForB2cIsTrue)
{
    // GIVEN
    m_pAosHandleMtc->SetNetworkType(NW_REPORT_RADIO_LTE);
    m_pAosHandleMtc->AddBlock(AosHandle::BLOCK_CALL_COMPOSER_CAPABILITY);
    ON_CALL(m_objMockIAosNConfiguration, IsB2cCallComposerFeatureTagInRegContact())
            .WillByDefault(Return(IMS_TRUE));

    ImsMap<IMS_UINT32, IMS_UINT32> objCapabilities;
    objCapabilities.Add(static_cast<IMS_UINT32>(AosNetworkType::LTE),
            static_cast<IMS_UINT32>(AosCapability::VOICE) |
                    static_cast<IMS_UINT32>(AosCapability::VIDEO));
    m_pAosHandleMtc->SetCapabilities(objCapabilities);

    // WHEN
    objCapabilities.SetValue(static_cast<IMS_UINT32>(AosNetworkType::LTE),
            static_cast<IMS_UINT32>(AosCapability::VOICE) |
                    static_cast<IMS_UINT32>(AosCapability::VIDEO) |
                    static_cast<IMS_UINT32>(AosCapability::CALL_COMPOSER_BUSINESS_ONLY));
    m_pAosHandleMtc->ProcessCapabilitiesChanged(objCapabilities);

    // THEN
    EXPECT_FALSE(m_pAosHandleMtc->IsHandleBlockedBase(AosHandle::BLOCK_CALL_COMPOSER_CAPABILITY));
}

TEST_F(AosHandleMtcTest,
        KeepBlockCallComposerIfB2cCallComposerIsRemovedWhileCallComposerFeatureTagForB2cIsFalse)
{
    // GIVEN
    m_pAosHandleMtc->SetNetworkType(NW_REPORT_RADIO_LTE);
    m_pAosHandleMtc->AddBlock(AosHandle::BLOCK_CALL_COMPOSER_CAPABILITY);
    ON_CALL(m_objMockIAosNConfiguration, IsB2cCallComposerFeatureTagInRegContact())
            .WillByDefault(Return(IMS_FALSE));

    ImsMap<IMS_UINT32, IMS_UINT32> objCapabilities;
    objCapabilities.Add(static_cast<IMS_UINT32>(AosNetworkType::LTE),
            static_cast<IMS_UINT32>(AosCapability::VOICE) |
                    static_cast<IMS_UINT32>(AosCapability::VIDEO) |
                    static_cast<IMS_UINT32>(AosCapability::CALL_COMPOSER_BUSINESS_ONLY));
    m_pAosHandleMtc->SetCapabilities(objCapabilities);

    // WHEN
    objCapabilities.SetValue(static_cast<IMS_UINT32>(AosNetworkType::LTE),
            static_cast<IMS_UINT32>(AosCapability::VOICE) |
                    static_cast<IMS_UINT32>(AosCapability::VIDEO));
    m_pAosHandleMtc->ProcessCapabilitiesChanged(objCapabilities);

    // THEN
    EXPECT_TRUE(m_pAosHandleMtc->IsHandleBlockedBase(AosHandle::BLOCK_CALL_COMPOSER_CAPABILITY));
}

TEST_F(AosHandleMtcTest,
        KeepBlockCallComposerIfB2cCallComposerIsAddedWhileCallComposerFeatureTagForB2cIsFalse)
{
    // GIVEN
    m_pAosHandleMtc->SetNetworkType(NW_REPORT_RADIO_LTE);
    m_pAosHandleMtc->AddBlock(AosHandle::BLOCK_CALL_COMPOSER_CAPABILITY);
    ON_CALL(m_objMockIAosNConfiguration, IsB2cCallComposerFeatureTagInRegContact())
            .WillByDefault(Return(IMS_FALSE));

    ImsMap<IMS_UINT32, IMS_UINT32> objCapabilities;
    objCapabilities.Add(static_cast<IMS_UINT32>(AosNetworkType::LTE),
            static_cast<IMS_UINT32>(AosCapability::VOICE) |
                    static_cast<IMS_UINT32>(AosCapability::VIDEO));
    m_pAosHandleMtc->SetCapabilities(objCapabilities);

    // WHEN
    objCapabilities.SetValue(static_cast<IMS_UINT32>(AosNetworkType::LTE),
            static_cast<IMS_UINT32>(AosCapability::VOICE) |
                    static_cast<IMS_UINT32>(AosCapability::VIDEO) |
                    static_cast<IMS_UINT32>(AosCapability::CALL_COMPOSER_BUSINESS_ONLY));
    m_pAosHandleMtc->ProcessCapabilitiesChanged(objCapabilities);

    // THEN
    EXPECT_TRUE(m_pAosHandleMtc->IsHandleBlockedBase(AosHandle::BLOCK_CALL_COMPOSER_CAPABILITY));
}

TEST_F(AosHandleMtcTest, ShouldBlockTextCapabilityIfItIsNotInTheListWhenCapabilitiesChanged)
{
    // GIVEN
    m_pAosHandleMtc->SetNetworkType(NW_REPORT_RADIO_LTE);

    ImsMap<IMS_UINT32, IMS_UINT32> objCapabilities;
    objCapabilities.Add(static_cast<IMS_UINT32>(AosNetworkType::LTE),
            static_cast<IMS_UINT32>(AosCapability::VOICE) |
                    static_cast<IMS_UINT32>(AosCapability::VIDEO) |
                    static_cast<IMS_UINT32>(AosCapability::TEXT));
    m_pAosHandleMtc->SetCapabilities(objCapabilities);

    // WHEN
    objCapabilities.SetValue(static_cast<IMS_UINT32>(AosNetworkType::LTE),
            static_cast<IMS_UINT32>(AosCapability::VOICE) |
                    static_cast<IMS_UINT32>(AosCapability::VIDEO));
    m_pAosHandleMtc->ProcessCapabilitiesChanged(objCapabilities);

    // THEN
    EXPECT_TRUE(m_pAosHandleMtc->IsHandleBlockedBase(AosHandle::BLOCK_TEXT_CAPABILITY));
}

TEST_F(AosHandleMtcTest, ShouldUnblockTextCapabilityIfItIsInTheListWhenCapabilitiesChanged)
{
    // GIVEN
    m_pAosHandleMtc->SetNetworkType(NW_REPORT_RADIO_LTE);

    ImsMap<IMS_UINT32, IMS_UINT32> objCapabilities;
    objCapabilities.Add(static_cast<IMS_UINT32>(AosNetworkType::LTE),
            static_cast<IMS_UINT32>(AosCapability::VOICE) |
                    static_cast<IMS_UINT32>(AosCapability::VIDEO));
    m_pAosHandleMtc->SetCapabilities(objCapabilities);
    m_pAosHandleMtc->AddBlock(AosHandle::BLOCK_TEXT_CAPABILITY);

    // WHEN
    objCapabilities.SetValue(static_cast<IMS_UINT32>(AosNetworkType::LTE),
            static_cast<IMS_UINT32>(AosCapability::VOICE) |
                    static_cast<IMS_UINT32>(AosCapability::VIDEO) |
                    static_cast<IMS_UINT32>(AosCapability::TEXT));
    m_pAosHandleMtc->ProcessCapabilitiesChanged(objCapabilities);

    // THEN
    EXPECT_FALSE(m_pAosHandleMtc->IsHandleBlockedBase(AosHandle::BLOCK_TEXT_CAPABILITY));
}

TEST_F(AosHandleMtcTest, RemoveHoldingVoWifiBlockIfVoWifiChangedToCapableDuringOnLte)
{
    // GIVEN
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

    m_pAosHandleMtc->SetNetworkType(NW_REPORT_RADIO_LTE);
    m_pAosHandleMtc->SetDataConnected(IMS_TRUE);
    m_pAosHandleMtc->SetEpdgEnabled(IMS_FALSE);
    m_pAosHandleMtc->AddHoldingBlockForWifi(AosHandle::BLOCK_VOWIFI_CAPABILITY);

    ON_CALL(m_objMockIAosNConfiguration, IsWfcImsAvailable()).WillByDefault(Return(IMS_TRUE));

    // WHEN
    m_pAosHandleMtc->ProcessCapabilitiesChanged(objNewCapabilities);

    // THEN
    EXPECT_FALSE(m_pAosHandleMtc->IsHoldingBlockForWifi(AosHandle::BLOCK_VOWIFI_CAPABILITY));
}

TEST_F(AosHandleMtcTest, RemoveHoldingViWifiBlockIfViWifiChangedToCapableDuringOnLte)
{
    // GIVEN
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

    m_pAosHandleMtc->SetNetworkType(NW_REPORT_RADIO_LTE);
    m_pAosHandleMtc->SetDataConnected(IMS_TRUE);
    m_pAosHandleMtc->SetEpdgEnabled(IMS_FALSE);
    m_pAosHandleMtc->AddHoldingBlockForWifi(AosHandle::BLOCK_VIWIFI_CAPABILITY);

    ON_CALL(m_objMockIAosNConfiguration, IsWfcImsAvailable()).WillByDefault(Return(IMS_TRUE));

    // WHEN
    m_pAosHandleMtc->ProcessCapabilitiesChanged(objNewCapabilities);

    // THEN
    EXPECT_FALSE(m_pAosHandleMtc->IsHoldingBlockForWifi(AosHandle::BLOCK_VIWIFI_CAPABILITY));
}

TEST_F(AosHandleMtcTest, RemoveHoldingVolteBlockIfVolteChangedToCapableDuringOnWifi)
{
    // GIVEN
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

    m_pAosHandleMtc->SetNetworkType(NW_REPORT_RADIO_WLAN);
    m_pAosHandleMtc->SetDataConnected(IMS_TRUE);
    m_pAosHandleMtc->SetEpdgEnabled(IMS_TRUE);
    m_pAosHandleMtc->AddHoldingBlockForMobile(AosHandle::BLOCK_VOLTE_CAPABILITY);

    ON_CALL(m_objMockIAosNetTracker, GetMobileNetworkType())
            .WillByDefault(Return(NW_REPORT_RADIO_LTE));
    ON_CALL(m_objMockIAosNConfiguration, IsWfcImsAvailable()).WillByDefault(Return(IMS_TRUE));

    // WHEN
    m_pAosHandleMtc->ProcessCapabilitiesChanged(objNewCapabilities);

    // THEN
    EXPECT_FALSE(m_pAosHandleMtc->IsHoldingBlockForMobile(AosHandle::BLOCK_VOLTE_CAPABILITY));
}

TEST_F(AosHandleMtcTest, RemoveHoldingVilteBlockIfVilteChangedToCapableDuringOnWifi)
{
    // GIVEN
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

    m_pAosHandleMtc->SetNetworkType(NW_REPORT_RADIO_WLAN);
    m_pAosHandleMtc->SetDataConnected(IMS_TRUE);
    m_pAosHandleMtc->SetEpdgEnabled(IMS_TRUE);
    m_pAosHandleMtc->AddHoldingBlockForMobile(AosHandle::BLOCK_VILTE_CAPABILITY);

    ON_CALL(m_objMockIAosNetTracker, GetMobileNetworkType())
            .WillByDefault(Return(NW_REPORT_RADIO_LTE));
    ON_CALL(m_objMockIAosNConfiguration, IsWfcImsAvailable()).WillByDefault(Return(IMS_TRUE));

    // WHEN
    m_pAosHandleMtc->ProcessCapabilitiesChanged(objNewCapabilities);

    // THEN
    EXPECT_FALSE(m_pAosHandleMtc->IsHoldingBlockForMobile(AosHandle::BLOCK_VILTE_CAPABILITY));
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

    m_pAosHandleMtc->SetNetworkType(NW_REPORT_RADIO_WLAN);
    m_pAosHandleMtc->SetDataConnected(IMS_TRUE);
    m_pAosHandleMtc->SetEpdgEnabled(IMS_TRUE);
    m_pAosHandleMtc->AddHoldingBlockForMobile(AosHandle::BLOCK_VILTE_CAPABILITY);

    EXPECT_CALL(m_objMockIAosNetTracker, GetMobileNetworkType())
            .Times(AnyNumber())
            .WillRepeatedly(Return(NW_REPORT_RADIO_GSM));

    EXPECT_CALL(m_objMockIAosNConfiguration, IsWfcImsAvailable())
            .Times(AnyNumber())
            .WillRepeatedly(Return(IMS_TRUE));

    EXPECT_TRUE(m_pAosHandleMtc->IsHoldingBlockForMobile(AosHandle::BLOCK_VILTE_CAPABILITY));

    m_pAosHandleMtc->ProcessCapabilitiesChanged(objNewCapabilities);

    EXPECT_TRUE(m_pAosHandleMtc->IsHoldingBlockForMobile(AosHandle::BLOCK_VILTE_CAPABILITY));
    EXPECT_TRUE(IsEqualCapabilities(m_pAosHandleMtc->GetCapabilities(), objNewCapabilities));
}

TEST_F(AosHandleMtcTest, ProcessCapabilitiesChanged_Test9)
{
    // Test9: No capability(LTE,IWLAN,NR), Current network=LTE, Wfc unavailable
    // Expectation: block BLOCK_VOLTE_CAPABILITY, BLOCK_VILTE_CAPABILITY
    //              No block/holding block BLOCK_VOWIFI_CAPABILITY, BLOCK_VIWIFI_CAPABILITY
    //              set none capa for the network.

    m_pAosHandleMtc->Init();

    ImsMap<IMS_UINT32, IMS_UINT32> objNewCapabilities, objExpectedCapabilities;

    objExpectedCapabilities.Add(static_cast<IMS_UINT32>(AosNetworkType::LTE),
            static_cast<IMS_UINT32>(AosCapability::NONE));
    objExpectedCapabilities.Add(static_cast<IMS_UINT32>(AosNetworkType::IWLAN),
            static_cast<IMS_UINT32>(AosCapability::NONE));
    objExpectedCapabilities.Add(static_cast<IMS_UINT32>(AosNetworkType::NR),
            static_cast<IMS_UINT32>(AosCapability::NONE));
    objExpectedCapabilities.Add(static_cast<IMS_UINT32>(AosNetworkType::UTRAN),
            static_cast<IMS_UINT32>(AosCapability::NONE));

    m_pAosHandleMtc->SetNetworkType(NW_REPORT_RADIO_LTE);

    EXPECT_CALL(m_objMockIAosNConfiguration, IsWfcImsAvailable())
            .Times(AnyNumber())
            .WillRepeatedly(Return(IMS_FALSE));

    EXPECT_CALL(m_objMockIAosNConfiguration,
            IsGGsmaRcsTelephonyFeatureTagUsedAsAvailableVoiceCallType())
            .Times(AnyNumber())
            .WillRepeatedly(Return(IMS_FALSE));

    m_pAosHandleMtc->ProcessCapabilitiesChanged(objNewCapabilities);

    EXPECT_TRUE(m_pAosHandleMtc->IsHandleBlockedBase(AosHandle::BLOCK_VOLTE_CAPABILITY));
    EXPECT_TRUE(m_pAosHandleMtc->IsHandleBlockedBase(AosHandle::BLOCK_VILTE_CAPABILITY));
    EXPECT_FALSE(m_pAosHandleMtc->IsHandleBlockedBase(AosHandle::BLOCK_VOWIFI_CAPABILITY));
    EXPECT_FALSE(m_pAosHandleMtc->IsHandleBlockedBase(AosHandle::BLOCK_VIWIFI_CAPABILITY));
    EXPECT_FALSE(m_pAosHandleMtc->IsHoldingBlockForWifi(AosHandle::BLOCK_VOWIFI_CAPABILITY));
    EXPECT_FALSE(m_pAosHandleMtc->IsHoldingBlockForWifi(AosHandle::BLOCK_VIWIFI_CAPABILITY));
    EXPECT_TRUE(IsEqualCapabilities(m_pAosHandleMtc->GetCapabilities(), objExpectedCapabilities));
    EXPECT_EQ(m_pAosHandleMtc->GetFeatureTagList().GetUnavailableFeatures(), ImsAosFeature::NONE);
}

TEST_F(AosHandleMtcTest, ProcessCapabilitiesChanged_Test10)
{
    // Test10: no capability(LTE,IWLAN,NR), Current network=WLAN & invalid cellular
    // Expectation: block BLOCK_VOWIFI_CAPABILITY, BLOCK_VIWIFI_CAPABILITY,
    //              No block/holding block BLOCK_VOLTE_CAPABILITY, BLOCK_VILTE_CAPABILITY
    //              set none capa for the network.

    m_pAosHandleMtc->Init();

    ImsMap<IMS_UINT32, IMS_UINT32> objNewCapabilities, objExpectedCapabilities;

    objExpectedCapabilities.Add(static_cast<IMS_UINT32>(AosNetworkType::LTE),
            static_cast<IMS_UINT32>(AosCapability::NONE));
    objExpectedCapabilities.Add(static_cast<IMS_UINT32>(AosNetworkType::IWLAN),
            static_cast<IMS_UINT32>(AosCapability::NONE));
    objExpectedCapabilities.Add(static_cast<IMS_UINT32>(AosNetworkType::NR),
            static_cast<IMS_UINT32>(AosCapability::NONE));
    objExpectedCapabilities.Add(static_cast<IMS_UINT32>(AosNetworkType::UTRAN),
            static_cast<IMS_UINT32>(AosCapability::NONE));

    m_pAosHandleMtc->SetNetworkType(NW_REPORT_RADIO_WLAN);
    m_pAosHandleMtc->SetEpdgEnabled(IMS_TRUE);

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

    m_pAosHandleMtc->ProcessCapabilitiesChanged(objNewCapabilities);

    EXPECT_FALSE(m_pAosHandleMtc->IsHandleBlockedBase(AosHandle::BLOCK_VOLTE_CAPABILITY));
    EXPECT_FALSE(m_pAosHandleMtc->IsHandleBlockedBase(AosHandle::BLOCK_VILTE_CAPABILITY));
    EXPECT_FALSE(m_pAosHandleMtc->IsHoldingBlockForMobile(AosHandle::BLOCK_VOLTE_CAPABILITY));
    EXPECT_FALSE(m_pAosHandleMtc->IsHoldingBlockForMobile(AosHandle::BLOCK_VILTE_CAPABILITY));
    EXPECT_TRUE(m_pAosHandleMtc->IsHandleBlockedBase(AosHandle::BLOCK_VOWIFI_CAPABILITY));
    EXPECT_TRUE(m_pAosHandleMtc->IsHandleBlockedBase(AosHandle::BLOCK_VIWIFI_CAPABILITY));
    EXPECT_TRUE(IsEqualCapabilities(m_pAosHandleMtc->GetCapabilities(), objExpectedCapabilities));
    EXPECT_EQ(m_pAosHandleMtc->GetFeatureTagList().GetUnavailableFeatures(), ImsAosFeature::NONE);
}

TEST_F(AosHandleMtcTest, ProcessCapabilitiesChanged_CallComposer)
{
    // Test: Call composer capability is changed.
    // Expectation: Call composer blocked/unblocked depending on the capability.
    //              Call composer feature is excluded/included depending on the capability.

    m_pAosHandleMtc->Init();

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

    m_pAosHandleMtc->SetNetworkType(NW_REPORT_RADIO_LTE);

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

    EXPECT_FALSE(m_pAosHandleMtc->IsHandleBlockedBase(AosHandle::BLOCK_CALL_COMPOSER_CAPABILITY));
    EXPECT_TRUE(m_pAosHandleMtc->GetFeatureTagList().HasFeature(
            ImsAosFeature::CALL_COMPOSER_VIA_TELEPHONY));

    m_pAosHandleMtc->ProcessCapabilitiesChanged(objNewCapabilities);

    EXPECT_TRUE(m_pAosHandleMtc->IsHandleBlockedBase(AosHandle::BLOCK_CALL_COMPOSER_CAPABILITY));
    EXPECT_FALSE(m_pAosHandleMtc->GetFeatureTagList().HasFeature(
            ImsAosFeature::CALL_COMPOSER_VIA_TELEPHONY));

    objNewCapabilities.SetValue(static_cast<IMS_UINT32>(AosNetworkType::LTE),
            static_cast<IMS_UINT32>(AosCapability::VOICE) |
                    static_cast<IMS_UINT32>(AosCapability::VIDEO) |
                    static_cast<IMS_UINT32>(AosCapability::CALL_COMPOSER));

    m_pAosHandleMtc->ProcessCapabilitiesChanged(objNewCapabilities);

    EXPECT_FALSE(m_pAosHandleMtc->IsHandleBlockedBase(AosHandle::BLOCK_CALL_COMPOSER_CAPABILITY));
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

    m_pAosHandleMtc->SetVopsIgnoredForVolteEnabled(IMS_TRUE);
    m_pAosHandleMtc->SetCapabilities(objCapabilities);
    m_pAosHandleMtc->SetNetworkType(NW_REPORT_RADIO_LTE);
    m_pAosHandleMtc->AddBlock(AosHandle::BLOCK_NETWORK);

    EXPECT_CALL(m_objMockIAosNConfiguration, IsWfcImsAvailable())
            .Times(AnyNumber())
            .WillRepeatedly(Return(IMS_TRUE));

    EXPECT_CALL(m_objMockIAosNConfiguration,
            IsGGsmaRcsTelephonyFeatureTagUsedAsAvailableVoiceCallType())
            .Times(AnyNumber())
            .WillRepeatedly(Return(IMS_FALSE));

    m_pAosHandleMtc->ProcessNetworkChanged();
    EXPECT_FALSE(m_pAosHandleMtc->IsHandleBlockedBase());
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

    m_pAosHandleMtc->SetCapabilities(objCapabilities);
    m_pAosHandleMtc->SetEpdgEnabled(IMS_TRUE);
    m_pAosHandleMtc->SetNetworkType(NW_REPORT_RADIO_WLAN);

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

    m_pAosHandleMtc->ProcessNetworkChanged();
    EXPECT_TRUE(m_pAosHandleMtc->IsHandleBlockedBase(AosHandle::BLOCK_VOWIFI_CAPABILITY));
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

    m_pAosHandleMtc->SetCapabilities(objCapabilities);
    m_pAosHandleMtc->SetNetworkType(NW_REPORT_RADIO_NR);

    EXPECT_CALL(m_objMockIAosNConfiguration, IsWfcImsAvailable())
            .Times(AnyNumber())
            .WillRepeatedly(Return(IMS_TRUE));

    EXPECT_CALL(m_objMockIAosNConfiguration,
            IsGGsmaRcsTelephonyFeatureTagUsedAsAvailableVoiceCallType())
            .Times(AnyNumber())
            .WillRepeatedly(Return(IMS_FALSE));

    EXPECT_CALL(m_objMockIAosNConfiguration, IsSmsOverImsAvailableWithoutVoiceCapability())
            .Times(AnyNumber())
            .WillRepeatedly(Return(IMS_TRUE));

    m_pAosHandleMtc->ProcessNetworkChanged();
    EXPECT_TRUE(m_pAosHandleMtc->IsHandleBlockedBase(AosHandle::BLOCK_VOLTE_CAPABILITY));
    EXPECT_TRUE(m_pAosHandleMtc->IsHandleBlockedBase(AosHandle::BLOCK_VILTE_CAPABILITY));
}

TEST_F(AosHandleMtcTest, ShouldBlockCapabilitiesIfNotCapableEvenIfKeepMmtelRegPolicyExists)
{
    // GIVEN
    ImsMap<IMS_UINT32, IMS_UINT32> objCapabilities;
    objCapabilities.Add(static_cast<IMS_UINT32>(AosNetworkType::LTE),
            static_cast<IMS_UINT32>(AosCapability::VOICE) |
                    static_cast<IMS_UINT32>(AosCapability::VIDEO));
    objCapabilities.Add(static_cast<IMS_UINT32>(AosNetworkType::IWLAN),
            static_cast<IMS_UINT32>(AosCapability::NONE));
    objCapabilities.Add(static_cast<IMS_UINT32>(AosNetworkType::NR),
            static_cast<IMS_UINT32>(AosCapability::NONE));

    m_pAosHandleMtc->SetCapabilities(objCapabilities);
    m_pAosHandleMtc->SetNetworkType(NW_REPORT_RADIO_NR);
    m_pAosHandleMtc->SetVopsState(IMS_VOICE_OVER_PS_NOT_SUPPORTED);
    m_pAosHandleMtc->GetFeatureTagList().AddUnavailableFeature(ImsAosFeature::MMTEL);
    m_pAosHandleMtc->GetFeatureTagList().AddUnavailableFeature(ImsAosFeature::VIDEO);
    m_objKeepRegWithMmtelFeatureTagPolicy.Add(CarrierConfig::Ims::UNAVAILABLE_FEATURE_POLICY_VOPS);

    ON_CALL(m_objMockIAosNConfiguration, IsWfcImsAvailable()).WillByDefault(Return(IMS_TRUE));
    ON_CALL(m_objMockIAosNConfiguration,
            IsGGsmaRcsTelephonyFeatureTagUsedAsAvailableVoiceCallType())
            .WillByDefault(Return(IMS_FALSE));

    // WHEN
    m_pAosHandleMtc->ProcessNetworkChanged();

    // THEN
    EXPECT_TRUE(m_pAosHandleMtc->IsHandleBlockedBase(AosHandle::BLOCK_VOLTE_CAPABILITY));
    EXPECT_TRUE(m_pAosHandleMtc->IsHandleBlockedBase(AosHandle::BLOCK_VILTE_CAPABILITY));
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

    m_pAosHandleMtc->SetCapabilities(objCapabilities);
    m_pAosHandleMtc->SetNetworkType(NW_REPORT_RADIO_GSM);

    EXPECT_CALL(m_objMockIAosNConfiguration, IsWfcImsAvailable())
            .Times(AnyNumber())
            .WillRepeatedly(Return(IMS_TRUE));

    EXPECT_CALL(m_objMockIAosNConfiguration,
            IsGGsmaRcsTelephonyFeatureTagUsedAsAvailableVoiceCallType())
            .Times(AnyNumber())
            .WillRepeatedly(Return(IMS_FALSE));

    EXPECT_CALL(m_objMockIAosNConfiguration, IsSmsOverImsAvailableWithoutVoiceCapability())
            .Times(AnyNumber())
            .WillRepeatedly(Return(IMS_TRUE));

    m_pAosHandleMtc->ProcessNetworkChanged();
    EXPECT_FALSE(m_pAosHandleMtc->IsHandleBlockedBase());
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

    m_pAosHandleMtc->SetCapabilities(objCapabilities);
    m_pAosHandleMtc->SetNetworkType(NW_REPORT_RADIO_WCDMA);

    EXPECT_CALL(m_objMockIAosNConfiguration, IsWfcImsAvailable())
            .Times(AnyNumber())
            .WillRepeatedly(Return(IMS_TRUE));

    EXPECT_CALL(m_objMockIAosNConfiguration,
            IsGGsmaRcsTelephonyFeatureTagUsedAsAvailableVoiceCallType())
            .Times(AnyNumber())
            .WillRepeatedly(Return(IMS_FALSE));

    EXPECT_CALL(m_objMockIAosNConfiguration, IsSmsOverImsAvailableWithoutVoiceCapability())
            .Times(AnyNumber())
            .WillRepeatedly(Return(IMS_TRUE));

    m_pAosHandleMtc->ProcessNetworkChanged();
    EXPECT_TRUE(m_pAosHandleMtc->IsHandleBlockedBase(AosHandle::BLOCK_NETWORK));

    m_pAosHandleMtc->SetNetworkType(NW_REPORT_RADIO_LTE);
    m_pAosHandleMtc->ProcessNetworkChanged();
    EXPECT_FALSE(m_pAosHandleMtc->IsHandleBlockedBase(AosHandle::BLOCK_NETWORK));
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

    m_pAosHandleMtc->SetCapabilities(objCapabilities);
    m_pAosHandleMtc->SetNetworkType(NW_REPORT_RADIO_WCDMA);
    m_pAosHandleMtc->SetDataConnected(IMS_FALSE);
    m_pAosHandleMtc->SetEpdgEnabled(IMS_FALSE);

    EXPECT_CALL(m_objMockIAosNConfiguration, IsWfcImsAvailable())
            .Times(AnyNumber())
            .WillRepeatedly(Return(IMS_TRUE));

    EXPECT_CALL(m_objMockIAosNConfiguration,
            IsGGsmaRcsTelephonyFeatureTagUsedAsAvailableVoiceCallType())
            .Times(AnyNumber())
            .WillRepeatedly(Return(IMS_FALSE));

    EXPECT_CALL(m_objMockIAosNConfiguration, IsSmsOverImsAvailableWithoutVoiceCapability())
            .Times(AnyNumber())
            .WillRepeatedly(Return(IMS_TRUE));

    m_pAosHandleMtc->ProcessNetworkChanged();
    EXPECT_FALSE(m_pAosHandleMtc->IsHandleBlockedBase(AosHandle::BLOCK_NETWORK));
    EXPECT_TRUE(m_pAosHandleMtc->IsHoldingBlockForMobile(AosHandle::BLOCK_NETWORK));

    m_pAosHandleMtc->SetNetworkType(NW_REPORT_RADIO_LTE);
    m_pAosHandleMtc->ProcessNetworkChanged();
    EXPECT_FALSE(m_pAosHandleMtc->IsHandleBlockedBase(AosHandle::BLOCK_NETWORK));
    EXPECT_FALSE(m_pAosHandleMtc->IsHoldingBlockForMobile(AosHandle::BLOCK_NETWORK));
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

    m_pAosHandleMtc->SetCapabilities(objCapabilities);
    m_pAosHandleMtc->SetNetworkType(NW_REPORT_RADIO_WLAN);
    m_pAosHandleMtc->SetEpdgEnabled(IMS_TRUE);
    m_pAosHandleMtc->AddHoldingBlockForMobile(AosHandle::BLOCK_NETWORK);

    EXPECT_CALL(m_objMockIAosNConfiguration, IsWfcImsAvailable())
            .Times(AnyNumber())
            .WillRepeatedly(Return(IMS_TRUE));

    EXPECT_CALL(m_objMockIAosNConfiguration,
            IsGGsmaRcsTelephonyFeatureTagUsedAsAvailableVoiceCallType())
            .Times(AnyNumber())
            .WillRepeatedly(Return(IMS_FALSE));

    EXPECT_CALL(m_objMockIAosNConfiguration, IsSmsOverImsAvailableWithoutVoiceCapability())
            .Times(AnyNumber())
            .WillRepeatedly(Return(IMS_TRUE));

    m_pAosHandleMtc->ProcessNetworkChanged();
    EXPECT_TRUE(m_pAosHandleMtc->IsHoldingBlockForMobile(AosHandle::BLOCK_NETWORK));
}

TEST_F(AosHandleMtcTest, ProcessNetworkChanged_CallComposer)
{
    // Test: Capa=(LTE:voice,video,callcomposer / IWLAN:voice / NR:none), no unavailable policy
    // Expectation: Call composer is get available/unavailable depending on the capability.

    m_pAosHandleMtc->Init();

    ImsMap<IMS_UINT32, IMS_UINT32> objCapabilities;
    objCapabilities.Add(static_cast<IMS_UINT32>(AosNetworkType::LTE),
            static_cast<IMS_UINT32>(AosCapability::VOICE) |
                    static_cast<IMS_UINT32>(AosCapability::VIDEO) |
                    static_cast<IMS_UINT32>(AosCapability::CALL_COMPOSER));
    objCapabilities.Add(static_cast<IMS_UINT32>(AosNetworkType::IWLAN),
            static_cast<IMS_UINT32>(AosCapability::VOICE));
    objCapabilities.Add(static_cast<IMS_UINT32>(AosNetworkType::NR),
            static_cast<IMS_UINT32>(AosCapability::NONE));

    EXPECT_FALSE(m_pAosHandleMtc->IsHandleBlockedBase(AosHandle::BLOCK_CALL_COMPOSER_CAPABILITY));
    EXPECT_TRUE(m_pAosHandleMtc->GetFeatureTagList().HasFeature(
            ImsAosFeature::CALL_COMPOSER_VIA_TELEPHONY));

    m_pAosHandleMtc->SetCapabilities(objCapabilities);
    m_pAosHandleMtc->SetNetworkType(NW_REPORT_RADIO_WLAN);
    m_pAosHandleMtc->SetEpdgEnabled(IMS_TRUE);

    EXPECT_CALL(m_objMockIAosNConfiguration, IsWfcImsAvailable())
            .Times(AnyNumber())
            .WillRepeatedly(Return(IMS_TRUE));

    EXPECT_CALL(m_objMockIAosNConfiguration,
            IsGGsmaRcsTelephonyFeatureTagUsedAsAvailableVoiceCallType())
            .Times(AnyNumber())
            .WillRepeatedly(Return(IMS_FALSE));

    m_pAosHandleMtc->ProcessNetworkChanged();

    EXPECT_TRUE(m_pAosHandleMtc->IsHandleBlockedBase(AosHandle::BLOCK_CALL_COMPOSER_CAPABILITY));
    EXPECT_FALSE(m_pAosHandleMtc->GetFeatureTagList().HasFeature(
            ImsAosFeature::CALL_COMPOSER_VIA_TELEPHONY));

    m_pAosHandleMtc->SetNetworkType(NW_REPORT_RADIO_LTE);
    m_pAosHandleMtc->SetEpdgEnabled(IMS_FALSE);

    m_pAosHandleMtc->ProcessNetworkChanged();

    EXPECT_FALSE(m_pAosHandleMtc->IsHandleBlockedBase(AosHandle::BLOCK_CALL_COMPOSER_CAPABILITY));
    EXPECT_TRUE(m_pAosHandleMtc->GetFeatureTagList().HasFeature(
            ImsAosFeature::CALL_COMPOSER_VIA_TELEPHONY));
}

TEST_F(AosHandleMtcTest, ShouldBlockTextCapabilityIfUeMovesToTheNetworkHasNoTextCapability)
{
    // GIVEN
    m_pAosHandleMtc->Init();

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

    m_pAosHandleMtc->SetCapabilities(objCapabilities);
    m_pAosHandleMtc->SetNetworkType(NW_REPORT_RADIO_NR);
    m_pAosHandleMtc->SetEpdgEnabled(IMS_FALSE);

    ON_CALL(m_objMockIAosNConfiguration, IsWfcImsAvailable()).WillByDefault(Return(IMS_TRUE));

    ON_CALL(m_objMockIAosNConfiguration,
            IsGGsmaRcsTelephonyFeatureTagUsedAsAvailableVoiceCallType())
            .WillByDefault(Return(IMS_FALSE));

    // WHEN
    m_pAosHandleMtc->ProcessNetworkChanged();

    // THEN
    EXPECT_TRUE(m_pAosHandleMtc->IsHandleBlockedBase(AosHandle::BLOCK_TEXT_CAPABILITY));
}

TEST_F(AosHandleMtcTest, ShouldUnblockTextCapabilityIfUeMovesToTheNetworkHasTextCapability)
{
    // GIVEN
    m_pAosHandleMtc->Init();

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

    m_pAosHandleMtc->SetCapabilities(objCapabilities);
    m_pAosHandleMtc->SetNetworkType(NW_REPORT_RADIO_LTE);
    m_pAosHandleMtc->SetEpdgEnabled(IMS_FALSE);
    m_pAosHandleMtc->AddBlock(AosHandle::BLOCK_TEXT_CAPABILITY);

    ON_CALL(m_objMockIAosNConfiguration, IsWfcImsAvailable()).WillByDefault(Return(IMS_TRUE));

    ON_CALL(m_objMockIAosNConfiguration,
            IsGGsmaRcsTelephonyFeatureTagUsedAsAvailableVoiceCallType())
            .WillByDefault(Return(IMS_FALSE));

    // WHEN
    m_pAosHandleMtc->ProcessNetworkChanged();

    // THEN
    EXPECT_FALSE(m_pAosHandleMtc->IsHandleBlockedBase(AosHandle::BLOCK_TEXT_CAPABILITY));
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

    m_pAosHandleMtc->SetNetworkType(NW_REPORT_RADIO_LTE);
    m_pAosHandleMtc->SetEpdgEnabled(IMS_FALSE);
    m_pAosHandleMtc->SetSsacBarred(IMS_TRUE);
    m_pAosHandleMtc->Init();

    m_pAosHandleMtc->ProcessNetworkChanged();
    EXPECT_FALSE(m_pAosHandleMtc->IsSsacBarred());

    m_pAosHandleMtc->ProcessNetworkChanged();
    EXPECT_TRUE(m_pAosHandleMtc->IsSsacBarred());
}

TEST_F(AosHandleMtcTest, ResetBlockNetworkIfRatIsChangedToInvalid)
{
    // GIVEN
    m_pAosHandleMtc->AddBlock(AosHandle::BLOCK_NETWORK);
    m_pAosHandleMtc->SetNetworkType(NW_REPORT_RADIO_INVALID);

    // WHEN
    m_pAosHandleMtc->ProcessNetworkChanged();

    // THEN
    EXPECT_FALSE(m_pAosHandleMtc->IsHandleBlockedBase(AosHandle::BLOCK_NETWORK));
}

TEST_F(AosHandleMtcTest, ResetHoldingBlockNetworkIfRatIsChangedToInvalid)
{
    // GIVEN
    m_pAosHandleMtc->AddHoldingBlockForMobile(AosHandle::BLOCK_NETWORK);
    m_pAosHandleMtc->SetNetworkType(NW_REPORT_RADIO_INVALID);

    // WHEN
    m_pAosHandleMtc->ProcessNetworkChanged();

    // THEN
    EXPECT_FALSE(m_pAosHandleMtc->IsHoldingBlockForMobile(AosHandle::BLOCK_NETWORK));
}

TEST_F(AosHandleMtcTest, NoCheckVopsStateOnRatsOtherThanNeitherLteNorNr)
{
    // GIVEN
    m_pAosHandleMtc->SetNetworkType(NW_REPORT_RADIO_WCDMA);

    EXPECT_CALL(m_objMockIAosNetTracker, IsImsVoiceCallSupported()).Times(0);

    // WHEN
    m_pAosHandleMtc->ProcessNetworkChanged();

    // THEN: The GIVEN condition should be met.
}

TEST_F(AosHandleMtcTest, NoVopsStateChangeIfNewStateIsSameWithOldStateOnLte)
{
    // GIVEN
    m_pAosHandleMtc->SetNetworkType(NW_REPORT_RADIO_LTE);
    m_pAosHandleMtc->SetVopsState(IMS_VOICE_OVER_PS_NOT_SUPPORTED);

    ON_CALL(m_objMockIAosNetTracker, IsImsVoiceCallSupported()).WillByDefault(Return(IMS_FALSE));

    // WHEN
    m_pAosHandleMtc->ProcessNetworkChanged();

    // THEN
    EXPECT_EQ(m_pAosHandleMtc->GetVopsState(), IMS_VOICE_OVER_PS_NOT_SUPPORTED);
}

TEST_F(AosHandleMtcTest, NoVopsStateChangeIfProcessingIsHeldOnNr)
{
    // GIVEN
    m_pAosHandleMtc->SetNetworkType(NW_REPORT_RADIO_NR);
    m_pAosHandleMtc->SetVopsState(IMS_VOICE_OVER_PS_SUPPORTED);

    ON_CALL(m_objMockIAosNetTracker, IsImsVoiceCallSupported()).WillByDefault(Return(IMS_FALSE));
    ON_CALL(m_objMockIAosCallTracker, IsNormalCallActive()).WillByDefault(Return(IMS_TRUE));

    // WHEN
    m_pAosHandleMtc->ProcessNetworkChanged();

    // THEN
    EXPECT_EQ(m_pAosHandleMtc->GetVopsState(), IMS_VOICE_OVER_PS_SUPPORTED);
}

TEST_F(AosHandleMtcTest, VopsStateIsChangedIfNewStateIsDifferentToOldStateAndNoActiveCallOnLte)
{
    // GIVEN
    m_pAosHandleMtc->SetNetworkType(NW_REPORT_RADIO_LTE);
    m_pAosHandleMtc->SetVopsState(IMS_VOICE_OVER_PS_SUPPORTED);

    ON_CALL(m_objMockIAosNetTracker, IsImsVoiceCallSupported()).WillByDefault(Return(IMS_FALSE));
    ON_CALL(m_objMockIAosCallTracker, IsNormalCallActive()).WillByDefault(Return(IMS_FALSE));

    // WHEN
    m_pAosHandleMtc->ProcessNetworkChanged();

    // THEN
    EXPECT_EQ(m_pAosHandleMtc->GetVopsState(), IMS_VOICE_OVER_PS_NOT_SUPPORTED);
}

TEST_F(AosHandleMtcTest, VolteHysTimerIsStartedIfVopsIsSupportedOnNetworkChangeToLteRoaming)
{
    // GIVEN
    m_pAosHandleMtc->SetNetworkType(NW_REPORT_RADIO_LTE);
    m_pAosHandleMtc->SetVopsState(IMS_VOICE_OVER_PS_NOT_SUPPORTED);
    m_pAosHandleMtc->SetVopsPlmn(AString("123456"));
    m_pAosHandleMtc->AddBlock(AosHandle::BLOCK_VOPS);
    m_pAosHandleMtc->SetRoamingState(IMS_ROAMING_STATE_ON);

    ON_CALL(m_objMockIAosNetTracker, IsImsVoiceCallSupported()).WillByDefault(Return(IMS_TRUE));
    ON_CALL(m_objMockIAosNetTracker, GetNetworkOperator()).WillByDefault(Return(AString("123456")));
    ON_CALL(m_objMockIAosCallTracker, IsNormalCallActive()).WillByDefault(Return(IMS_FALSE));
    ON_CALL(m_objMockIAosNConfiguration, GetVolteHysTime()).WillByDefault(Return(60));

    // WHEN
    m_pAosHandleMtc->ProcessNetworkChanged();

    // THEN
    EXPECT_TRUE(m_pAosHandleMtc->IsVolteHysTimerRunning());
}

TEST_F(AosHandleMtcTest, VolteHysTimerIsNotStartedIfVopsIsSupportedOnNetworkChangeToLteHome)
{
    // GIVEN
    m_pAosHandleMtc->SetNetworkType(NW_REPORT_RADIO_LTE);
    m_pAosHandleMtc->SetVopsState(IMS_VOICE_OVER_PS_NOT_SUPPORTED);
    m_pAosHandleMtc->SetVopsPlmn(AString("123456"));
    m_pAosHandleMtc->AddBlock(AosHandle::BLOCK_VOPS);
    m_pAosHandleMtc->SetRoamingState(IMS_ROAMING_STATE_OFF);

    ON_CALL(m_objMockIAosNetTracker, IsImsVoiceCallSupported()).WillByDefault(Return(IMS_TRUE));
    ON_CALL(m_objMockIAosNetTracker, GetNetworkOperator()).WillByDefault(Return(AString("123456")));
    ON_CALL(m_objMockIAosCallTracker, IsNormalCallActive()).WillByDefault(Return(IMS_FALSE));
    ON_CALL(m_objMockIAosNConfiguration, GetVolteHysTime()).WillByDefault(Return(60));

    // WHEN
    m_pAosHandleMtc->ProcessNetworkChanged();

    // THEN
    EXPECT_FALSE(m_pAosHandleMtc->IsVolteHysTimerRunning());
}

TEST_F(AosHandleMtcTest, VopsBlockIsReleasedIfVopsIsSupportedOnNetworkChangeToLteAndNoVolteHysTime)
{
    // GIVEN
    m_pAosHandleMtc->SetNetworkType(NW_REPORT_RADIO_LTE);
    m_pAosHandleMtc->SetVopsState(IMS_VOICE_OVER_PS_NOT_SUPPORTED);
    m_pAosHandleMtc->AddBlock(AosHandle::BLOCK_VOPS);

    ON_CALL(m_objMockIAosNetTracker, IsImsVoiceCallSupported()).WillByDefault(Return(IMS_TRUE));
    ON_CALL(m_objMockIAosCallTracker, IsNormalCallActive()).WillByDefault(Return(IMS_FALSE));
    ON_CALL(m_objMockIAosNConfiguration, GetVolteHysTime()).WillByDefault(Return(0));

    // WHEN
    m_pAosHandleMtc->ProcessNetworkChanged();

    // THEN
    EXPECT_FALSE(m_pAosHandleMtc->IsHandleBlockedBase(AosHandle::BLOCK_VOPS));
}

TEST_F(AosHandleMtcTest, VopsBlockIsSetIfVopsIsNotSupportedOnNetworkChangeToLte)
{
    // GIVEN
    m_pAosHandleMtc->SetNetworkType(NW_REPORT_RADIO_LTE);
    m_pAosHandleMtc->SetVopsState(IMS_VOICE_OVER_PS_SUPPORTED);

    ON_CALL(m_objMockIAosNetTracker, IsImsVoiceCallSupported()).WillByDefault(Return(IMS_FALSE));
    ON_CALL(m_objMockIAosCallTracker, IsNormalCallActive()).WillByDefault(Return(IMS_FALSE));

    // WHEN
    m_pAosHandleMtc->ProcessNetworkChanged();

    // THEN
    EXPECT_TRUE(m_pAosHandleMtc->IsHandleBlockedBase(AosHandle::BLOCK_VOPS));
}

TEST_F(AosHandleMtcTest, ShouldRequestPlmnBlockIfVopsIsNotSupportedOnNetworkChangeToLte)
{
    // GIVEN
    m_pAosHandleMtc->SetNetworkType(NW_REPORT_RADIO_LTE);
    m_pAosHandleMtc->SetVopsState(IMS_VOICE_OVER_PS_SUPPORTED);

    ON_CALL(m_objMockIAosNetTracker, IsImsVoiceCallSupported()).WillByDefault(Return(IMS_FALSE));
    ON_CALL(m_objMockIAosCallTracker, IsNormalCallActive()).WillByDefault(Return(IMS_FALSE));
    ON_CALL(m_objMockIAosNConfiguration, IsPlmnBlockWithTimeoutOnVoiceCallUnavailable())
            .WillByDefault(Return(IMS_TRUE));

    EXPECT_CALL(m_objMockIAosApplication,
            RequestCmd(ImsAosControl::PLMN_BLOCK_WITH_TIMEOUT, AosReason::VOPS_NOT_SUPPORTED));

    // WHEN
    m_pAosHandleMtc->ProcessNetworkChanged();

    // THEN: The GIVEN condition should be met.
}

TEST_F(AosHandleMtcTest, ShouldStopVolteHysTimerIfVopsIsNotSupportedOnNetworkChangeToLte)
{
    // GIVEN
    m_pAosHandleMtc->SetNetworkType(NW_REPORT_RADIO_LTE);
    m_pAosHandleMtc->SetVopsState(IMS_VOICE_OVER_PS_SUPPORTED);

    ON_CALL(m_objMockIAosNetTracker, IsImsVoiceCallSupported()).WillByDefault(Return(IMS_FALSE));
    ON_CALL(m_objMockIAosCallTracker, IsNormalCallActive()).WillByDefault(Return(IMS_FALSE));

    m_pAosHandleMtc->StartVolteHysTimer(60);

    // WHEN
    m_pAosHandleMtc->ProcessNetworkChanged();

    // THEN
    EXPECT_FALSE(m_pAosHandleMtc->IsVolteHysTimerRunning());
}

TEST_F(AosHandleMtcTest, ShouldKeepCsVolteFeatureTagIfCallHandoverToWlanWhenWfcOffAndVideoAvailable)
{
    ImsMap<IMS_UINT32, IMS_UINT32> objCapabilities;
    objCapabilities.Add(static_cast<IMS_UINT32>(AosNetworkType::LTE),
            static_cast<IMS_UINT32>(AosCapability::VOICE) |
                    static_cast<IMS_UINT32>(AosCapability::VIDEO) |
                    static_cast<IMS_UINT32>(AosCapability::TEXT));
    objCapabilities.Add(static_cast<IMS_UINT32>(AosNetworkType::IWLAN),
            static_cast<IMS_UINT32>(AosCapability::VIDEO));
    m_pAosHandleMtc->SetCapabilities(objCapabilities);
    m_pAosHandleMtc->SetNetworkType(NW_REPORT_RADIO_WLAN);
    m_pAosHandleMtc->SetEpdgEnabled(IMS_TRUE);
    m_pAosHandleMtc->SetDataConnected(IMS_TRUE);

    m_pAosHandleMtc->AddFeature(ImsAosFeature::MMTEL);
    m_pAosHandleMtc->AddFeature(ImsAosFeature::VIDEO);
    m_pAosHandleMtc->AddFeatureTag(FeatureTags::RCS_TELEPHONY, AosString::STR_CS);
    m_pAosHandleMtc->AddFeatureTag(FeatureTags::RCS_TELEPHONY, AosString::STR_VOLTE);
    m_pAosHandleMtc->AddBindedFeature(ImsAosFeature::MMTEL);
    m_pAosHandleMtc->AddBindedFeature(ImsAosFeature::VIDEO);
    m_pAosHandleMtc->AddBindedFeatureTag(FeatureTags::RCS_TELEPHONY, AosString::STR_CS);
    m_pAosHandleMtc->AddBindedFeatureTag(FeatureTags::RCS_TELEPHONY, AosString::STR_VOLTE);

    AosFeatureTagList objExpectedFeatureTagListOffhook;
    objExpectedFeatureTagListOffhook.AddFeature(ImsAosFeature::MMTEL);
    objExpectedFeatureTagListOffhook.AddFeature(ImsAosFeature::VIDEO);
    objExpectedFeatureTagListOffhook.AddFeatureTag(FeatureTags::RCS_TELEPHONY, AosString::STR_CS);
    objExpectedFeatureTagListOffhook.AddFeatureTag(
            FeatureTags::RCS_TELEPHONY, AosString::STR_VOLTE);

    ON_CALL(m_objMockIAosNConfiguration,
            IsGGsmaRcsTelephonyFeatureTagUsedAsAvailableVoiceCallType())
            .WillByDefault(Return(IMS_TRUE));
    ON_CALL(m_objMockIAosNConfiguration, IsVideoOverWifiSupportedWithoutVoice())
            .WillByDefault(Return(IMS_TRUE));
    ON_CALL(m_objMockIAosNConfiguration, IsWfcImsAvailable()).WillByDefault(Return(IMS_TRUE));
    ON_CALL(m_objMockIAosCallTracker, IsNormalCallActive()).WillByDefault(Return(IMS_TRUE));

    // WHEN
    m_pAosHandleMtc->ProcessNetworkChanged();

    // THEN
    EXPECT_TRUE(m_pAosHandleMtc->GetFeatureTagList().Equals(objExpectedFeatureTagListOffhook));
}

TEST_F(AosHandleMtcTest, ShouldSetCsFeatureTagIfNoCallOnHandoverToWlanWhenWfcOffAndVideoAvailable)
{
    ImsMap<IMS_UINT32, IMS_UINT32> objCapabilities;
    objCapabilities.Add(static_cast<IMS_UINT32>(AosNetworkType::LTE),
            static_cast<IMS_UINT32>(AosCapability::VOICE) |
                    static_cast<IMS_UINT32>(AosCapability::VIDEO) |
                    static_cast<IMS_UINT32>(AosCapability::TEXT));
    objCapabilities.Add(static_cast<IMS_UINT32>(AosNetworkType::IWLAN),
            static_cast<IMS_UINT32>(AosCapability::VIDEO));
    m_pAosHandleMtc->SetCapabilities(objCapabilities);
    m_pAosHandleMtc->SetNetworkType(NW_REPORT_RADIO_WLAN);
    m_pAosHandleMtc->SetEpdgEnabled(IMS_TRUE);
    m_pAosHandleMtc->SetDataConnected(IMS_TRUE);

    m_pAosHandleMtc->AddFeature(ImsAosFeature::MMTEL);
    m_pAosHandleMtc->AddFeature(ImsAosFeature::VIDEO);
    m_pAosHandleMtc->AddFeatureTag(FeatureTags::RCS_TELEPHONY, AosString::STR_CS);
    m_pAosHandleMtc->AddFeatureTag(FeatureTags::RCS_TELEPHONY, AosString::STR_VOLTE);
    m_pAosHandleMtc->AddBindedFeature(ImsAosFeature::MMTEL);
    m_pAosHandleMtc->AddBindedFeature(ImsAosFeature::VIDEO);
    m_pAosHandleMtc->AddBindedFeatureTag(FeatureTags::RCS_TELEPHONY, AosString::STR_CS);
    m_pAosHandleMtc->AddBindedFeatureTag(FeatureTags::RCS_TELEPHONY, AosString::STR_VOLTE);

    AosFeatureTagList objExpectedFeatureTagListIdle;
    objExpectedFeatureTagListIdle.AddFeature(ImsAosFeature::VIDEO);
    objExpectedFeatureTagListIdle.AddFeatureTag(
            FeatureTags::RCS_TELEPHONY, AosString::STR_CS_WITH_DQ);

    ON_CALL(m_objMockIAosNConfiguration,
            IsGGsmaRcsTelephonyFeatureTagUsedAsAvailableVoiceCallType())
            .WillByDefault(Return(IMS_TRUE));
    ON_CALL(m_objMockIAosNConfiguration, IsVideoOverWifiSupportedWithoutVoice())
            .WillByDefault(Return(IMS_TRUE));
    ON_CALL(m_objMockIAosNConfiguration, IsWfcImsAvailable()).WillByDefault(Return(IMS_TRUE));
    ON_CALL(m_objMockIAosCallTracker, IsNormalCallActive()).WillByDefault(Return(IMS_FALSE));
    ON_CALL(m_objMockIAosNetTracker, GetMobileNetworkType())
            .WillByDefault(Return(NW_REPORT_RADIO_GSM));

    // WHEN
    m_pAosHandleMtc->ProcessNetworkChanged();

    // THEN
    EXPECT_TRUE(m_pAosHandleMtc->GetFeatureTagList().Equals(objExpectedFeatureTagListIdle));
}

TEST_F(AosHandleMtcTest, ProcessVopsStateChanged_Test1)
{
    // Test1: call active
    // Expectation: set/reset m_nHoldingVopsState as the state
    //              No change m_nVopsState

    EXPECT_CALL(m_objMockIAosCallTracker, IsNormalCallActive())
            .Times(AnyNumber())
            .WillRepeatedly(Return(IMS_TRUE));

    m_pAosHandleMtc->ProcessVopsStateChanged(IMS_VOICE_OVER_PS_NOT_SUPPORTED, AString("123456"));
    EXPECT_EQ(m_pAosHandleMtc->GetHoldingVopsState(), IMS_VOICE_OVER_PS_NOT_SUPPORTED);
    EXPECT_EQ(m_pAosHandleMtc->GetVopsState(), IMS_VOICE_OVER_PS_SUPPORTED);
    EXPECT_FALSE(m_pAosHandleMtc->IsHandleBlockedBase(AosHandle::BLOCK_VOPS));

    m_pAosHandleMtc->ProcessVopsStateChanged(IMS_VOICE_OVER_PS_SUPPORTED, AString("123456"));
    EXPECT_EQ(m_pAosHandleMtc->GetHoldingVopsState(), IMS_VOICE_OVER_PS_SUPPORTED);
    EXPECT_EQ(m_pAosHandleMtc->GetVopsState(), IMS_VOICE_OVER_PS_SUPPORTED);
    EXPECT_FALSE(m_pAosHandleMtc->IsHandleBlockedBase(AosHandle::BLOCK_VOPS));
}

TEST_F(AosHandleMtcTest,
        ShouldUpdateHoldingVopsStateWhenVopsIsChangedToTrueIfHoldingVopsStateIsFalse)
{
    // GIVEN
    m_pAosHandleMtc->SetVopsState(IMS_VOICE_OVER_PS_SUPPORTED);
    m_pAosHandleMtc->SetHoldingVopsState(IMS_VOICE_OVER_PS_NOT_SUPPORTED);
    m_pAosHandleMtc->SetVopsPlmn(AString("123456"));

    // WHEN
    m_pAosHandleMtc->ServicePhone_VopsStateChanged(IMS_VOICE_OVER_PS_SUPPORTED, AString("123456"));

    // THEN
    EXPECT_EQ(m_pAosHandleMtc->GetHoldingVopsState(), IMS_VOICE_OVER_PS_SUPPORTED);
    EXPECT_EQ(m_pAosHandleMtc->GetVopsState(), IMS_VOICE_OVER_PS_SUPPORTED);
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

    m_pAosHandleMtc->SetNetworkType(NW_REPORT_RADIO_LTE);

    EXPECT_CALL(m_objMockIAosNConfiguration, IsWfcImsAvailable())
            .Times(AnyNumber())
            .WillRepeatedly(Return(IMS_TRUE));

    EXPECT_CALL(m_objMockIAosNConfiguration,
            IsGGsmaRcsTelephonyFeatureTagUsedAsAvailableVoiceCallType())
            .Times(AnyNumber())
            .WillRepeatedly(Return(IMS_FALSE));

    m_pAosHandleMtc->ProcessVopsStateChanged(IMS_VOICE_OVER_PS_NOT_SUPPORTED, AString("123456"));
    EXPECT_EQ(m_pAosHandleMtc->GetHoldingVopsState(), IMS_VOICE_OVER_PS_SUPPORTED);
    EXPECT_EQ(m_pAosHandleMtc->GetVopsState(), IMS_VOICE_OVER_PS_NOT_SUPPORTED);
    EXPECT_TRUE(m_pAosHandleMtc->IsHandleBlockedBase(AosHandle::BLOCK_VOPS));

    m_pAosHandleMtc->ProcessVopsStateChanged(IMS_VOICE_OVER_PS_SUPPORTED, AString("123456"));
    EXPECT_EQ(m_pAosHandleMtc->GetHoldingVopsState(), IMS_VOICE_OVER_PS_SUPPORTED);
    EXPECT_EQ(m_pAosHandleMtc->GetVopsState(), IMS_VOICE_OVER_PS_SUPPORTED);
    EXPECT_FALSE(m_pAosHandleMtc->IsHandleBlockedBase(AosHandle::BLOCK_VOPS));
}

TEST_F(AosHandleMtcTest, NoBlockVopsIfKeepMmtelRegPolicyExists)
{
    // GIVEN
    m_pAosHandleMtc->SetNetworkType(NW_REPORT_RADIO_LTE);
    m_objKeepRegWithMmtelFeatureTagPolicy.Add(CarrierConfig::Ims::UNAVAILABLE_FEATURE_POLICY_VOPS);

    ON_CALL(m_objMockIAosCallTracker, IsNormalCallActive()).WillByDefault(Return(IMS_FALSE));
    ON_CALL(m_objMockIAosNConfiguration, IsWfcImsAvailable()).WillByDefault(Return(IMS_TRUE));
    ON_CALL(m_objMockIAosNConfiguration,
            IsGGsmaRcsTelephonyFeatureTagUsedAsAvailableVoiceCallType())
            .WillByDefault(Return(IMS_FALSE));

    // WHEN
    m_pAosHandleMtc->ProcessVopsStateChanged(IMS_VOICE_OVER_PS_NOT_SUPPORTED, AString("TestPlmn"));

    // THEN
    EXPECT_FALSE(m_pAosHandleMtc->IsHandleBlockedBase(AosHandle::BLOCK_VOPS));
}

TEST_F(AosHandleMtcTest, ProcessVopsStateChanged_Test6)
{
    // Test6: call idle, no unavailalble policy, network=LTE roaming
    // Expectation: Start/Stop VolteHysTimer. Plmn block if the condition is met.
    //              BLOCK_VOPS blocked.
    //              No change m_nVopsState and m_nHoldingVopsState

    EXPECT_CALL(m_objMockIAosCallTracker, IsNormalCallActive())
            .Times(AnyNumber())
            .WillRepeatedly(Return(IMS_FALSE));

    m_pAosHandleMtc->SetNetworkType(NW_REPORT_RADIO_LTE);
    m_pAosHandleMtc->SetRoamingState(IMS_ROAMING_STATE_ON);

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

    EXPECT_CALL(m_objMockIAosApplication,
            RequestCmd(ImsAosControl::PLMN_BLOCK_WITH_TIMEOUT, AosReason::VOPS_NOT_SUPPORTED))
            .Times(1);

    m_pAosHandleMtc->ProcessVopsStateChanged(IMS_VOICE_OVER_PS_NOT_SUPPORTED, AString("123456"));
    EXPECT_EQ(m_pAosHandleMtc->GetHoldingVopsState(), IMS_VOICE_OVER_PS_SUPPORTED);
    EXPECT_EQ(m_pAosHandleMtc->GetVopsState(), IMS_VOICE_OVER_PS_NOT_SUPPORTED);
    EXPECT_TRUE(m_pAosHandleMtc->IsHandleBlockedBase(AosHandle::BLOCK_VOPS));
    EXPECT_FALSE(m_pAosHandleMtc->IsVolteHysTimerRunning());

    m_pAosHandleMtc->ProcessVopsStateChanged(IMS_VOICE_OVER_PS_SUPPORTED, AString("123456"));
    EXPECT_EQ(m_pAosHandleMtc->GetHoldingVopsState(), IMS_VOICE_OVER_PS_SUPPORTED);
    EXPECT_EQ(m_pAosHandleMtc->GetVopsState(), IMS_VOICE_OVER_PS_SUPPORTED);
    EXPECT_TRUE(m_pAosHandleMtc->IsHandleBlockedBase(AosHandle::BLOCK_VOPS));
    EXPECT_TRUE(m_pAosHandleMtc->IsVolteHysTimerRunning());

    m_pAosHandleMtc->ProcessVopsStateChanged(IMS_VOICE_OVER_PS_NOT_SUPPORTED, AString("123456"));
    EXPECT_EQ(m_pAosHandleMtc->GetHoldingVopsState(), IMS_VOICE_OVER_PS_SUPPORTED);
    EXPECT_EQ(m_pAosHandleMtc->GetVopsState(), IMS_VOICE_OVER_PS_NOT_SUPPORTED);
    EXPECT_TRUE(m_pAosHandleMtc->IsHandleBlockedBase(AosHandle::BLOCK_VOPS));
    EXPECT_FALSE(m_pAosHandleMtc->IsVolteHysTimerRunning());
}

TEST_F(AosHandleMtcTest, ProcessVopsStateChanged_VolteHysTimerRunning_by_Ssac)
{
    EXPECT_CALL(m_objMockIAosCallTracker, IsNormalCallActive())
            .Times(AnyNumber())
            .WillRepeatedly(Return(IMS_FALSE));

    m_pAosHandleMtc->SetNetworkType(NW_REPORT_RADIO_LTE);
    m_pAosHandleMtc->SetRoamingState(IMS_ROAMING_STATE_ON);

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

    ASSERT_EQ(m_pAosHandleMtc->GetVopsState(), IMS_VOICE_OVER_PS_SUPPORTED);
    ASSERT_FALSE(m_pAosHandleMtc->IsHandleBlockedBase(AosHandle::BLOCK_VOPS));

    SsacInfo objSsacInfo;
    objSsacInfo.nBarringFactorForVoice = 0;
    m_pAosHandleMtc->ImsRadio_OnSsacChanged(objSsacInfo);
    ASSERT_TRUE(m_pAosHandleMtc->IsSsacBarred());
    ASSERT_TRUE(m_pAosHandleMtc->IsHandleBlockedBase(AosHandle::BLOCK_SSAC));

    objSsacInfo.nBarringFactorForVoice = 100;
    m_pAosHandleMtc->ImsRadio_OnSsacChanged(objSsacInfo);
    ASSERT_TRUE(m_pAosHandleMtc->IsVolteHysTimerRunning());
    ASSERT_FALSE(m_pAosHandleMtc->IsSsacBarred());
    ASSERT_TRUE(m_pAosHandleMtc->IsHandleBlockedBase(AosHandle::BLOCK_SSAC));

    m_pAosHandleMtc->ProcessVopsStateChanged(IMS_VOICE_OVER_PS_NOT_SUPPORTED, AString("123456"));
    EXPECT_FALSE(m_pAosHandleMtc->IsVolteHysTimerRunning());
    EXPECT_EQ(m_pAosHandleMtc->GetVopsState(), IMS_VOICE_OVER_PS_NOT_SUPPORTED);
    EXPECT_TRUE(m_pAosHandleMtc->IsHandleBlockedBase(AosHandle::BLOCK_VOPS));
    EXPECT_FALSE(m_pAosHandleMtc->IsHandleBlockedBase(AosHandle::BLOCK_SSAC));
}

TEST_F(AosHandleMtcTest, ShouldNotStartVolteHysTimeWhenVopsChanged_Plmn1_Off_Plmn2_On)
{
    // GIVEN
    m_pAosHandleMtc->SetNetworkType(NW_REPORT_RADIO_LTE);

    ON_CALL(m_objMockIAosNetTracker, GetMobileNetworkType())
            .WillByDefault(Return(NW_REPORT_RADIO_LTE));
    ON_CALL(m_objMockIAosCallTracker, IsNormalCallActive()).WillByDefault(Return(IMS_FALSE));
    ON_CALL(m_objMockIAosNConfiguration, GetVolteHysTime()).WillByDefault(Return(60));
    ON_CALL(m_objMockIAosNConfiguration, IsWfcImsAvailable()).WillByDefault(Return(IMS_TRUE));
    ON_CALL(m_objMockIAosNConfiguration,
            IsGGsmaRcsTelephonyFeatureTagUsedAsAvailableVoiceCallType())
            .WillByDefault(Return(IMS_FALSE));
    ON_CALL(m_objMockIAosNConfiguration, IsPlmnBlockWithTimeoutOnVoiceCallUnavailable())
            .WillByDefault(Return(IMS_FALSE));

    // GIVEN - Plmn1_Off
    m_pAosHandleMtc->SetVopsState(IMS_VOICE_OVER_PS_NOT_SUPPORTED);
    m_pAosHandleMtc->SetVopsPlmn(AString("111111"));
    m_pAosHandleMtc->AddBlock(AosHandle::BLOCK_VOPS);

    // WHEN - Plmn2_On
    m_pAosHandleMtc->ServicePhone_PlmnChanged(AString("222222"));  // PlmnChanged comes first
    m_pAosHandleMtc->ServicePhone_VopsStateChanged(IMS_VOICE_OVER_PS_SUPPORTED, AString("222222"));

    // THEN
    EXPECT_FALSE(m_pAosHandleMtc->IsVolteHysTimerRunning());
    EXPECT_FALSE(m_pAosHandleMtc->IsHandleBlockedBase(AosHandle::BLOCK_VOPS));
}

TEST_F(AosHandleMtcTest, ShouldNotStartVolteHysTimeWhenVopsChanged_Plmn1_Off_Plmn2_Off_Plmn1_On)
{
    // GIVEN
    m_pAosHandleMtc->SetNetworkType(NW_REPORT_RADIO_LTE);

    ON_CALL(m_objMockIAosNetTracker, GetMobileNetworkType())
            .WillByDefault(Return(NW_REPORT_RADIO_LTE));
    ON_CALL(m_objMockIAosCallTracker, IsNormalCallActive()).WillByDefault(Return(IMS_FALSE));
    ON_CALL(m_objMockIAosNConfiguration, GetVolteHysTime()).WillByDefault(Return(60));
    ON_CALL(m_objMockIAosNConfiguration, IsWfcImsAvailable()).WillByDefault(Return(IMS_TRUE));
    ON_CALL(m_objMockIAosNConfiguration,
            IsGGsmaRcsTelephonyFeatureTagUsedAsAvailableVoiceCallType())
            .WillByDefault(Return(IMS_FALSE));
    ON_CALL(m_objMockIAosNConfiguration, IsPlmnBlockWithTimeoutOnVoiceCallUnavailable())
            .WillByDefault(Return(IMS_FALSE));

    // GIVEN - Plmn1_Off
    m_pAosHandleMtc->SetVopsState(IMS_VOICE_OVER_PS_NOT_SUPPORTED);
    m_pAosHandleMtc->SetVopsPlmn(AString("111111"));
    m_pAosHandleMtc->AddBlock(AosHandle::BLOCK_VOPS);

    // GIVEN - Plmn2_Off
    m_pAosHandleMtc->ServicePhone_PlmnChanged(AString("222222"));
    m_pAosHandleMtc->ServicePhone_VopsStateChanged(
            IMS_VOICE_OVER_PS_NOT_SUPPORTED, AString("222222"));

    // WHEN - Plmn1_On
    m_pAosHandleMtc->ServicePhone_PlmnChanged(AString("111111"));  // PlmnChanged comes first
    m_pAosHandleMtc->ServicePhone_VopsStateChanged(IMS_VOICE_OVER_PS_SUPPORTED, AString("111111"));

    // THEN
    EXPECT_FALSE(m_pAosHandleMtc->IsHandleBlockedBase(AosHandle::BLOCK_VOPS));
    EXPECT_FALSE(m_pAosHandleMtc->IsVolteHysTimerRunning());
}

TEST_F(AosHandleMtcTest, ShouldStartVolteHysTimeWhenVopsChanged_Plmn1_Off_Plmn2_Off_Plmn2_On)
{
    // GIVEN
    m_pAosHandleMtc->SetNetworkType(NW_REPORT_RADIO_LTE);
    m_pAosHandleMtc->SetRoamingState(IMS_ROAMING_STATE_ON);

    ON_CALL(m_objMockIAosNetTracker, GetMobileNetworkType())
            .WillByDefault(Return(NW_REPORT_RADIO_LTE));
    ON_CALL(m_objMockIAosCallTracker, IsNormalCallActive()).WillByDefault(Return(IMS_FALSE));
    ON_CALL(m_objMockIAosNConfiguration, GetVolteHysTime()).WillByDefault(Return(60));
    ON_CALL(m_objMockIAosNConfiguration, IsWfcImsAvailable()).WillByDefault(Return(IMS_TRUE));
    ON_CALL(m_objMockIAosNConfiguration,
            IsGGsmaRcsTelephonyFeatureTagUsedAsAvailableVoiceCallType())
            .WillByDefault(Return(IMS_FALSE));
    ON_CALL(m_objMockIAosNConfiguration, IsPlmnBlockWithTimeoutOnVoiceCallUnavailable())
            .WillByDefault(Return(IMS_FALSE));

    // GIVEN - Plmn1_Off
    m_pAosHandleMtc->SetVopsState(IMS_VOICE_OVER_PS_NOT_SUPPORTED);
    m_pAosHandleMtc->SetVopsPlmn(AString("111111"));
    m_pAosHandleMtc->AddBlock(AosHandle::BLOCK_VOPS);

    // GIVEN - Plmn2_Off
    m_pAosHandleMtc->ServicePhone_PlmnChanged(AString("222222"));
    m_pAosHandleMtc->ServicePhone_VopsStateChanged(
            IMS_VOICE_OVER_PS_NOT_SUPPORTED, AString("222222"));

    // WHEN - Plmn2_On
    // Only VopsChanged comes
    m_pAosHandleMtc->ServicePhone_VopsStateChanged(IMS_VOICE_OVER_PS_SUPPORTED, AString("222222"));

    // THEN
    EXPECT_TRUE(m_pAosHandleMtc->IsVolteHysTimerRunning());
    EXPECT_TRUE(m_pAosHandleMtc->IsHandleBlockedBase(AosHandle::BLOCK_VOPS));
    EXPECT_EQ(m_pAosHandleMtc->GetVopsState(), IMS_VOICE_OVER_PS_SUPPORTED);
}

TEST_F(AosHandleMtcTest, ShouldStopVolteHysTimeWhenVopsChanged_Plmn1_Off_Plmn1_On_Plmn2_On)
{
    // GIVEN
    m_pAosHandleMtc->SetNetworkType(NW_REPORT_RADIO_LTE);
    m_pAosHandleMtc->SetRoamingState(IMS_ROAMING_STATE_ON);

    ON_CALL(m_objMockIAosNetTracker, GetMobileNetworkType())
            .WillByDefault(Return(NW_REPORT_RADIO_LTE));
    ON_CALL(m_objMockIAosCallTracker, IsNormalCallActive()).WillByDefault(Return(IMS_FALSE));
    ON_CALL(m_objMockIAosNConfiguration, GetVolteHysTime()).WillByDefault(Return(60));
    ON_CALL(m_objMockIAosNConfiguration, IsWfcImsAvailable()).WillByDefault(Return(IMS_TRUE));
    ON_CALL(m_objMockIAosNConfiguration,
            IsGGsmaRcsTelephonyFeatureTagUsedAsAvailableVoiceCallType())
            .WillByDefault(Return(IMS_FALSE));
    ON_CALL(m_objMockIAosNConfiguration, IsPlmnBlockWithTimeoutOnVoiceCallUnavailable())
            .WillByDefault(Return(IMS_FALSE));

    // GIVEN - Plmn1_Off
    m_pAosHandleMtc->SetVopsState(IMS_VOICE_OVER_PS_NOT_SUPPORTED);
    m_pAosHandleMtc->SetVopsPlmn(AString("111111"));
    m_pAosHandleMtc->AddBlock(AosHandle::BLOCK_VOPS);
    m_pAosHandleMtc->SetHandleState(AosHandle::STATE_DISCONNECTED);

    // GIVEN - Plmn1_On
    // Only VopsChanged comes
    m_pAosHandleMtc->ServicePhone_VopsStateChanged(IMS_VOICE_OVER_PS_SUPPORTED, AString("111111"));
    EXPECT_TRUE(m_pAosHandleMtc->IsVolteHysTimerRunning());
    EXPECT_TRUE(m_pAosHandleMtc->IsHandleBlockedBase(AosHandle::BLOCK_VOPS));
    EXPECT_EQ(m_pAosHandleMtc->GetVopsState(), IMS_VOICE_OVER_PS_SUPPORTED);

    // WHEN - Plmn2_On
    m_pAosHandleMtc->ServicePhone_PlmnChanged(AString("222222"));  // PlmnChanged comes first
    m_pAosHandleMtc->ServicePhone_VopsStateChanged(IMS_VOICE_OVER_PS_SUPPORTED, AString("222222"));

    // THEN
    EXPECT_FALSE(m_pAosHandleMtc->IsVolteHysTimerRunning());
    EXPECT_FALSE(m_pAosHandleMtc->IsHandleBlockedBase(AosHandle::BLOCK_VOPS));
}

TEST_F(AosHandleMtcTest, ShouldStopVolteHysTimeWhenVopsChanged_Plmn1_Off_Plmn1_On_Plmn2_Off)
{
    // GIVEN
    m_pAosHandleMtc->SetNetworkType(NW_REPORT_RADIO_LTE);
    m_pAosHandleMtc->SetRoamingState(IMS_ROAMING_STATE_ON);

    ON_CALL(m_objMockIAosNetTracker, GetMobileNetworkType())
            .WillByDefault(Return(NW_REPORT_RADIO_LTE));
    ON_CALL(m_objMockIAosCallTracker, IsNormalCallActive()).WillByDefault(Return(IMS_FALSE));
    ON_CALL(m_objMockIAosNConfiguration, GetVolteHysTime()).WillByDefault(Return(60));
    ON_CALL(m_objMockIAosNConfiguration, IsWfcImsAvailable()).WillByDefault(Return(IMS_TRUE));
    ON_CALL(m_objMockIAosNConfiguration,
            IsGGsmaRcsTelephonyFeatureTagUsedAsAvailableVoiceCallType())
            .WillByDefault(Return(IMS_FALSE));
    ON_CALL(m_objMockIAosNConfiguration, IsPlmnBlockWithTimeoutOnVoiceCallUnavailable())
            .WillByDefault(Return(IMS_FALSE));

    // GIVEN - Plmn1_Off
    m_pAosHandleMtc->SetVopsState(IMS_VOICE_OVER_PS_NOT_SUPPORTED);
    m_pAosHandleMtc->SetVopsPlmn(AString("111111"));
    m_pAosHandleMtc->AddBlock(AosHandle::BLOCK_VOPS);

    // GIVEN - Plmn1_On
    // Only VopsChanged comes
    m_pAosHandleMtc->ServicePhone_VopsStateChanged(IMS_VOICE_OVER_PS_SUPPORTED, AString("111111"));
    EXPECT_TRUE(m_pAosHandleMtc->IsVolteHysTimerRunning());
    EXPECT_TRUE(m_pAosHandleMtc->IsHandleBlockedBase(AosHandle::BLOCK_VOPS));
    EXPECT_EQ(m_pAosHandleMtc->GetVopsState(), IMS_VOICE_OVER_PS_SUPPORTED);

    // WHEN - Plmn2_Off
    m_pAosHandleMtc->ServicePhone_PlmnChanged(AString("222222"));  // PlmnChanged comes first
    m_pAosHandleMtc->ServicePhone_VopsStateChanged(
            IMS_VOICE_OVER_PS_NOT_SUPPORTED, AString("222222"));

    // THEN
    EXPECT_FALSE(m_pAosHandleMtc->IsVolteHysTimerRunning());
    EXPECT_TRUE(m_pAosHandleMtc->IsHandleBlockedBase(AosHandle::BLOCK_VOPS));
    EXPECT_EQ(m_pAosHandleMtc->GetVopsState(), IMS_VOICE_OVER_PS_NOT_SUPPORTED);
}

TEST_F(AosHandleMtcTest, ShouldNotStartVolteHysTimeWhenVopsChanged_Plmn1_Off_Plmn2_3g_Plmn1_On)
{
    // GIVEN
    m_pAosHandleMtc->SetNetworkType(NW_REPORT_RADIO_LTE);

    ON_CALL(m_objMockIAosNetTracker, GetMobileNetworkType())
            .WillByDefault(Return(NW_REPORT_RADIO_LTE));
    ON_CALL(m_objMockIAosCallTracker, IsNormalCallActive()).WillByDefault(Return(IMS_FALSE));
    ON_CALL(m_objMockIAosNConfiguration, GetVolteHysTime()).WillByDefault(Return(60));
    ON_CALL(m_objMockIAosNConfiguration, IsWfcImsAvailable()).WillByDefault(Return(IMS_TRUE));
    ON_CALL(m_objMockIAosNConfiguration,
            IsGGsmaRcsTelephonyFeatureTagUsedAsAvailableVoiceCallType())
            .WillByDefault(Return(IMS_FALSE));
    ON_CALL(m_objMockIAosNConfiguration, IsPlmnBlockWithTimeoutOnVoiceCallUnavailable())
            .WillByDefault(Return(IMS_FALSE));

    // GIVEN - Plmn1_Off
    m_pAosHandleMtc->SetVopsState(IMS_VOICE_OVER_PS_NOT_SUPPORTED);
    m_pAosHandleMtc->SetVopsPlmn(AString("111111"));
    m_pAosHandleMtc->AddBlock(AosHandle::BLOCK_VOPS);

    // GIVEN - Plmn2_3g
    m_pAosHandleMtc->ServicePhone_PlmnChanged(AString("222222"));  // Only PlmnChanged comes

    // WHEN - Plmn1_On
    m_pAosHandleMtc->ServicePhone_PlmnChanged(AString("111111"));  // PlmnChanged comes first
    m_pAosHandleMtc->ServicePhone_VopsStateChanged(IMS_VOICE_OVER_PS_SUPPORTED, AString("111111"));

    // THEN
    EXPECT_FALSE(m_pAosHandleMtc->IsHandleBlockedBase(AosHandle::BLOCK_VOPS));
    EXPECT_FALSE(m_pAosHandleMtc->IsVolteHysTimerRunning());
}

TEST_F(AosHandleMtcTest, VopsStateIsNotUpdatedIfRatIsNeitherLteNorNr)
{
    // GIVEN
    m_pAosHandleMtc->SetVopsState(IMS_VOICE_OVER_PS_SUPPORTED);
    m_pAosHandleMtc->SetNetworkType(NW_REPORT_RADIO_WCDMA);

    // WHEN
    m_pAosHandleMtc->ProcessVopsStateChanged(IMS_VOICE_OVER_PS_NOT_SUPPORTED, AString("123456"));

    // THEN
    EXPECT_EQ(m_pAosHandleMtc->GetVopsState(), IMS_VOICE_OVER_PS_SUPPORTED);
}

TEST_F(AosHandleMtcTest, ShouldNotifyMtcIfB2cCallComposerCapabilityIsChanged)
{
    // GIVEN
    m_pAosHandleMtc->SetHandleState(AosHandle::STATE_CONNECTED);
    m_pAosHandleMtc->SetNetworkType(NW_REPORT_RADIO_LTE);

    ImsMap<IMS_UINT32, IMS_UINT32> objCapabilities;
    objCapabilities.Add(static_cast<IMS_UINT32>(AosNetworkType::LTE),
            static_cast<IMS_UINT32>(AosCapability::VOICE) |
                    static_cast<IMS_UINT32>(AosCapability::VIDEO) |
                    static_cast<IMS_UINT32>(AosCapability::TEXT) |
                    static_cast<IMS_UINT32>(AosCapability::CALL_COMPOSER_BUSINESS_ONLY));
    m_pAosHandleMtc->SetCapabilities(objCapabilities);
    m_pAosHandleMtc->SetListener(&m_objMockIImsAosListener);
    ON_CALL(m_objMockIAosNConfiguration, IsB2cCallComposerFeatureTagInRegContact())
            .WillByDefault(Return(IMS_FALSE));

    EXPECT_CALL(m_objMockIImsAosListener, ImsAos_Connected(_, _));

    // WHEN
    m_pAosHandleMtc->ReevaluateCapabilities();

    // THEN: The GIVEN condition should be met.
}

TEST_F(AosHandleMtcTest, ReevaluateUnavailableFeature_Test1)
{
    // Test1: Vops change / Video Capable / Ssac not barred
    // Expectation: Add/Remove mmtel, video to/from unavailable feature / No block vops
    //              Notify to AosRegistration if unavailable feature is changed

    m_objKeepRegWithMmtelFeatureTagPolicy.Add(CarrierConfig::Ims::UNAVAILABLE_FEATURE_POLICY_VOPS);
    m_pAosHandleMtc->SetNetworkType(NW_REPORT_RADIO_LTE);

    EXPECT_CALL(m_objMockIAosRegistration,
            RequestCmd(IAosRegistration::CMD_UPDATE_FEATURE_WITHOUT_REG, 0))
            .Times(2);

    m_pAosHandleMtc->ProcessVopsStateChanged(IMS_VOICE_OVER_PS_NOT_SUPPORTED, AString("TestPlmn"));
    EXPECT_TRUE(m_pAosHandleMtc->GetFeatureTagList().HasUnavailableFeature(ImsAosFeature::MMTEL));
    EXPECT_TRUE(m_pAosHandleMtc->GetFeatureTagList().HasUnavailableFeature(ImsAosFeature::VIDEO));
    EXPECT_FALSE(m_pAosHandleMtc->IsHandleBlockedBase(AosHandle::BLOCK_VOPS));

    m_pAosHandleMtc->ProcessVopsStateChanged(IMS_VOICE_OVER_PS_SUPPORTED, AString("TestPlmn"));
    EXPECT_FALSE(m_pAosHandleMtc->GetFeatureTagList().HasUnavailableFeature(ImsAosFeature::MMTEL));
    EXPECT_FALSE(m_pAosHandleMtc->GetFeatureTagList().HasUnavailableFeature(ImsAosFeature::VIDEO));
    EXPECT_FALSE(m_pAosHandleMtc->IsHandleBlockedBase(AosHandle::BLOCK_VOPS));
}

TEST_F(AosHandleMtcTest, ReevaluateUnavailableFeature_Test2)
{
    // Test2: Vops change / Video NOT capable / Ssac not barred
    // Expectation: Add/Remove mmtel to/from unavailable feature / No block vops
    //              Notify to AosRegistration if unavailable feature is changed

    m_objKeepRegWithMmtelFeatureTagPolicy.Add(CarrierConfig::Ims::UNAVAILABLE_FEATURE_POLICY_VOPS);
    m_pAosHandleMtc->SetNetworkType(NW_REPORT_RADIO_LTE);

    ImsMap<IMS_UINT32, IMS_UINT32> objCapabilitiesVoice;
    objCapabilitiesVoice.Add(static_cast<IMS_UINT32>(AosNetworkType::LTE),
            static_cast<IMS_UINT32>(AosCapability::VOICE));

    EXPECT_CALL(m_objMockIAosRegistration,
            RequestCmd(IAosRegistration::CMD_UPDATE_FEATURE_WITHOUT_REG, 0))
            .Times(2);

    m_pAosHandleMtc->ProcessCapabilitiesChanged(objCapabilitiesVoice);

    m_pAosHandleMtc->ProcessVopsStateChanged(IMS_VOICE_OVER_PS_NOT_SUPPORTED, AString("TestPlmn"));
    EXPECT_TRUE(m_pAosHandleMtc->GetFeatureTagList().HasUnavailableFeature(ImsAosFeature::MMTEL));
    EXPECT_FALSE(m_pAosHandleMtc->GetFeatureTagList().HasUnavailableFeature(ImsAosFeature::VIDEO));
    EXPECT_FALSE(m_pAosHandleMtc->IsHandleBlockedBase(AosHandle::BLOCK_VOPS));

    m_pAosHandleMtc->ProcessVopsStateChanged(IMS_VOICE_OVER_PS_SUPPORTED, AString("TestPlmn"));
    EXPECT_FALSE(m_pAosHandleMtc->GetFeatureTagList().HasUnavailableFeature(ImsAosFeature::MMTEL));
    EXPECT_FALSE(m_pAosHandleMtc->GetFeatureTagList().HasUnavailableFeature(ImsAosFeature::VIDEO));
    EXPECT_FALSE(m_pAosHandleMtc->IsHandleBlockedBase(AosHandle::BLOCK_VOPS));
}

TEST_F(AosHandleMtcTest, ReevaluateUnavailableFeature_Test3)
{
    // Test3: Network change
    // Expectation: Unavailable feature on 3G but LTE / No block network on 3G
    //              Notify to AosRegistration if unavailable feature is changed

    m_objKeepRegWithMmtelFeatureTagPolicy.Add(CarrierConfig::Ims::UNAVAILABLE_FEATURE_POLICY_3G);
    m_pAosHandleMtc->SetNetworkType(NW_REPORT_RADIO_HSPA);

    EXPECT_CALL(m_objMockIAosRegistration,
            RequestCmd(IAosRegistration::CMD_UPDATE_FEATURE_WITHOUT_REG, 0))
            .Times(2);

    m_pAosHandleMtc->ProcessNetworkChanged();
    EXPECT_TRUE(m_pAosHandleMtc->GetFeatureTagList().HasUnavailableFeature(ImsAosFeature::MMTEL));
    EXPECT_TRUE(m_pAosHandleMtc->GetFeatureTagList().HasUnavailableFeature(ImsAosFeature::VIDEO));
    EXPECT_FALSE(m_pAosHandleMtc->IsHandleBlockedBase(AosHandle::BLOCK_NETWORK));

    m_pAosHandleMtc->SetNetworkType(NW_REPORT_RADIO_LTE);

    m_pAosHandleMtc->ProcessNetworkChanged();
    EXPECT_FALSE(m_pAosHandleMtc->GetFeatureTagList().HasUnavailableFeature(ImsAosFeature::MMTEL));
    EXPECT_FALSE(m_pAosHandleMtc->GetFeatureTagList().HasUnavailableFeature(ImsAosFeature::VIDEO));
    EXPECT_FALSE(m_pAosHandleMtc->IsHandleBlockedBase(AosHandle::BLOCK_NETWORK));
}

TEST_F(AosHandleMtcTest, ReevaluateUnavailableFeature_Test4)
{
    // Test4: Vops change on Ssac barred / Video Capable
    // Expectation: mmtel, video are always in unavailable feature / No block vops
    //              Notify to AosRegistration if unavailable feature is changed

    m_objKeepRegWithMmtelFeatureTagPolicy.Add(CarrierConfig::Ims::UNAVAILABLE_FEATURE_POLICY_VOPS);
    m_objKeepRegWithMmtelFeatureTagPolicy.Add(CarrierConfig::Ims::UNAVAILABLE_FEATURE_POLICY_SSAC);

    m_pAosHandleMtc->SetNetworkType(NW_REPORT_RADIO_LTE);

    EXPECT_CALL(m_objMockIAosNConfiguration, IsRequiredVolteBlockBySsac())
            .Times(AnyNumber())
            .WillRepeatedly(Return(IMS_TRUE));

    SsacInfo objSsacInfo;
    objSsacInfo.nBarringFactorForVoice = 0;
    m_pAosHandleMtc->ImsRadio_OnSsacChanged(objSsacInfo);

    ASSERT_TRUE(m_pAosHandleMtc->IsSsacBarred());

    EXPECT_CALL(m_objMockIAosRegistration,
            RequestCmd(IAosRegistration::CMD_UPDATE_FEATURE_WITHOUT_REG, 0))
            .Times(0);

    m_pAosHandleMtc->ProcessVopsStateChanged(IMS_VOICE_OVER_PS_NOT_SUPPORTED, AString("123456"));
    EXPECT_TRUE(m_pAosHandleMtc->GetFeatureTagList().HasUnavailableFeature(ImsAosFeature::MMTEL));
    EXPECT_TRUE(m_pAosHandleMtc->GetFeatureTagList().HasUnavailableFeature(ImsAosFeature::VIDEO));
    EXPECT_FALSE(m_pAosHandleMtc->IsHandleBlockedBase(AosHandle::BLOCK_VOPS));

    m_pAosHandleMtc->ProcessVopsStateChanged(IMS_VOICE_OVER_PS_SUPPORTED, AString("123456"));
    EXPECT_TRUE(m_pAosHandleMtc->GetFeatureTagList().HasUnavailableFeature(ImsAosFeature::MMTEL));
    EXPECT_TRUE(m_pAosHandleMtc->GetFeatureTagList().HasUnavailableFeature(ImsAosFeature::VIDEO));
    EXPECT_FALSE(m_pAosHandleMtc->IsHandleBlockedBase(AosHandle::BLOCK_VOPS));
}

TEST_F(AosHandleMtcTest, ReevaluateUnavailableFeature_Test5)
{
    // Test5: Ssac change / Video Capable / Vops supported
    // Expectation: Add/Remove mmtel, video to/from unavailable feature / No block ssac
    //              Notify to AosRegistration if unavailable feature is changed

    m_objKeepRegWithMmtelFeatureTagPolicy.Add(CarrierConfig::Ims::UNAVAILABLE_FEATURE_POLICY_SSAC);
    m_pAosHandleMtc->SetNetworkType(NW_REPORT_RADIO_LTE);

    EXPECT_CALL(m_objMockIAosNConfiguration, IsRequiredVolteBlockBySsac())
            .Times(AnyNumber())
            .WillRepeatedly(Return(IMS_TRUE));

    EXPECT_CALL(m_objMockIAosRegistration,
            RequestCmd(IAosRegistration::CMD_UPDATE_FEATURE_WITHOUT_REG, 0))
            .Times(2);

    SsacInfo objSsacInfo;
    objSsacInfo.nBarringFactorForVoice = 0;
    m_pAosHandleMtc->ImsRadio_OnSsacChanged(objSsacInfo);
    EXPECT_TRUE(m_pAosHandleMtc->GetFeatureTagList().HasUnavailableFeature(ImsAosFeature::MMTEL));
    EXPECT_TRUE(m_pAosHandleMtc->GetFeatureTagList().HasUnavailableFeature(ImsAosFeature::VIDEO));
    EXPECT_FALSE(m_pAosHandleMtc->IsHandleBlockedBase(AosHandle::BLOCK_SSAC));

    objSsacInfo.nBarringFactorForVoice = 100;
    m_pAosHandleMtc->ImsRadio_OnSsacChanged(objSsacInfo);
    EXPECT_FALSE(m_pAosHandleMtc->GetFeatureTagList().HasUnavailableFeature(ImsAosFeature::MMTEL));
    EXPECT_FALSE(m_pAosHandleMtc->GetFeatureTagList().HasUnavailableFeature(ImsAosFeature::VIDEO));
    EXPECT_FALSE(m_pAosHandleMtc->IsHandleBlockedBase(AosHandle::BLOCK_SSAC));
}

TEST_F(AosHandleMtcTest, ReevaluateUnavailableFeature_Test6)
{
    // Test6: Ssac change on vops not supported / Video Capable
    // Expectation: mmtel, video are always in unavailable feature / No block ssac
    //              Notify to AosRegistration if unavailable feature is changed

    m_objKeepRegWithMmtelFeatureTagPolicy.Add(CarrierConfig::Ims::UNAVAILABLE_FEATURE_POLICY_VOPS);
    m_objKeepRegWithMmtelFeatureTagPolicy.Add(CarrierConfig::Ims::UNAVAILABLE_FEATURE_POLICY_SSAC);

    m_pAosHandleMtc->SetNetworkType(NW_REPORT_RADIO_LTE);

    EXPECT_CALL(m_objMockIAosNConfiguration, IsVopsIgnoredForVolteEnabled())
            .Times(AnyNumber())
            .WillRepeatedly(Return(IMS_FALSE));

    EXPECT_CALL(m_objMockIAosNConfiguration, IsRequiredVolteBlockBySsac())
            .Times(AnyNumber())
            .WillRepeatedly(Return(IMS_TRUE));

    EXPECT_CALL(m_objMockIAosRegistration,
            RequestCmd(IAosRegistration::CMD_UPDATE_FEATURE_WITHOUT_REG, 0))
            .Times(1);

    m_pAosHandleMtc->ProcessVopsStateChanged(IMS_VOICE_OVER_PS_NOT_SUPPORTED, AString("123456"));

    SsacInfo objSsacInfo;
    objSsacInfo.nBarringFactorForVoice = 0;
    m_pAosHandleMtc->ImsRadio_OnSsacChanged(objSsacInfo);
    EXPECT_TRUE(m_pAosHandleMtc->GetFeatureTagList().HasUnavailableFeature(ImsAosFeature::MMTEL));
    EXPECT_TRUE(m_pAosHandleMtc->GetFeatureTagList().HasUnavailableFeature(ImsAosFeature::VIDEO));
    EXPECT_FALSE(m_pAosHandleMtc->IsHandleBlockedBase(AosHandle::BLOCK_SSAC));

    objSsacInfo.nBarringFactorForVoice = 100;
    m_pAosHandleMtc->ImsRadio_OnSsacChanged(objSsacInfo);
    EXPECT_TRUE(m_pAosHandleMtc->GetFeatureTagList().HasUnavailableFeature(ImsAosFeature::MMTEL));
    EXPECT_TRUE(m_pAosHandleMtc->GetFeatureTagList().HasUnavailableFeature(ImsAosFeature::VIDEO));
    EXPECT_FALSE(m_pAosHandleMtc->IsHandleBlockedBase(AosHandle::BLOCK_SSAC));
}

TEST_F(AosHandleMtcTest, VoiceAvailableWhenEpdgEnabledToWlanEvenIfVopsNotSupported)
{
    // GIVEN
    m_objKeepRegWithMmtelFeatureTagPolicy.Add(CarrierConfig::Ims::UNAVAILABLE_FEATURE_POLICY_VOPS);
    m_pAosHandleMtc->SetVopsState(IMS_VOICE_OVER_PS_NOT_SUPPORTED);
    m_pAosHandleMtc->SetNetworkType(NW_REPORT_RADIO_WLAN);
    m_pAosHandleMtc->SetEpdgEnabled(IMS_TRUE);
    m_pAosHandleMtc->GetFeatureTagList().AddUnavailableFeature(ImsAosFeature::MMTEL);

    // WHEN
    m_pAosHandleMtc->ProcessNetworkChanged();

    // THEN
    EXPECT_FALSE(m_pAosHandleMtc->GetFeatureTagList().HasUnavailableFeature(ImsAosFeature::MMTEL));
}

TEST_F(AosHandleMtcTest, NotSetBlockNetworkIfCallExistsWhenNetworkChangedTo3g)
{
    // GIVEN
    m_pAosHandleMtc->SetNetworkType(NW_REPORT_RADIO_WCDMA);
    ON_CALL(m_objMockIAosCallTracker, IsNormalCallActive()).WillByDefault(Return(IMS_TRUE));

    // WHEN
    m_pAosHandleMtc->ProcessNetworkChanged();

    // THEN
    EXPECT_FALSE(m_pAosHandleMtc->IsHandleBlockedBase(AosHandle::BLOCK_NETWORK));
}

TEST_F(AosHandleMtcTest, NotSetMmtelAsUnavailableFeatureIfCallExistsWhenNetworkChangedTo3g)
{
    // GIVEN
    m_pAosHandleMtc->SetNetworkType(NW_REPORT_RADIO_WCDMA);
    m_objKeepRegWithMmtelFeatureTagPolicy.Add(CarrierConfig::Ims::UNAVAILABLE_FEATURE_POLICY_3G);

    ON_CALL(m_objMockIAosCallTracker, IsNormalCallActive()).WillByDefault(Return(IMS_TRUE));

    // WHEN
    m_pAosHandleMtc->ProcessNetworkChanged();

    // THEN
    EXPECT_FALSE(m_pAosHandleMtc->GetFeatureTagList().HasUnavailableFeature(ImsAosFeature::MMTEL));
}

TEST_F(AosHandleMtcTest, GetVoiceBlockReasonForIpcan_Test)
{
    m_pAosHandleMtc->SetNetworkType(NW_REPORT_RADIO_LTE);
    EXPECT_EQ(m_pAosHandleMtc->GetVoiceBlockReasonForIpcan(), AosHandle::BLOCK_VOLTE_CAPABILITY);

    m_pAosHandleMtc->SetNetworkType(NW_REPORT_RADIO_WLAN);
    EXPECT_EQ(m_pAosHandleMtc->GetVoiceBlockReasonForIpcan(), AosHandle::BLOCK_VOWIFI_CAPABILITY);
}

TEST_F(AosHandleMtcTest, GetVideoBlockReasonForIpcan_Test)
{
    m_pAosHandleMtc->SetNetworkType(NW_REPORT_RADIO_LTE);
    EXPECT_EQ(m_pAosHandleMtc->GetVideoBlockReasonForIpcan(), AosHandle::BLOCK_VILTE_CAPABILITY);

    m_pAosHandleMtc->SetNetworkType(NW_REPORT_RADIO_WLAN);
    EXPECT_EQ(m_pAosHandleMtc->GetVideoBlockReasonForIpcan(), AosHandle::BLOCK_VIWIFI_CAPABILITY);
}

TEST_F(AosHandleMtcTest, IsCsFeatureTagRequired_Test)
{
    EXPECT_CALL(m_objMockIAosNConfiguration, IsVideoOverWifiSupportedWithoutVoice())
            .Times(AnyNumber())
            .WillRepeatedly(Return(IMS_FALSE));
    EXPECT_FALSE(m_pAosHandleMtc->IsCsFeatureTagRequired());

    EXPECT_CALL(m_objMockIAosNConfiguration, IsVideoOverWifiSupportedWithoutVoice())
            .Times(AnyNumber())
            .WillRepeatedly(Return(IMS_TRUE));
    m_pAosHandleMtc->RemoveFeature(ImsAosFeature::VIDEO);
    EXPECT_FALSE(m_pAosHandleMtc->IsCsFeatureTagRequired());

    m_pAosHandleMtc->AddFeature(ImsAosFeature::VIDEO);
    m_pAosHandleMtc->SetEpdgEnabled(IMS_FALSE);
    EXPECT_FALSE(m_pAosHandleMtc->IsCsFeatureTagRequired());

    m_pAosHandleMtc->SetEpdgEnabled(IMS_TRUE);
    EXPECT_CALL(m_objMockIAosNetTracker, GetMobileNetworkType())
            .Times(AnyNumber())
            .WillRepeatedly(Return(NW_REPORT_RADIO_LTE));
    EXPECT_CALL(m_objMockIAosNetTracker, GetMobileChangingNetworkType())
            .Times(2)
            .WillOnce(Return(NW_REPORT_RADIO_LTE))
            .WillOnce(Return(NW_REPORT_RADIO_GSM));
    EXPECT_FALSE(m_pAosHandleMtc->IsCsFeatureTagRequired());
    EXPECT_TRUE(m_pAosHandleMtc->IsCsFeatureTagRequired());
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

    EXPECT_FALSE(m_pAosHandleMtc->IsInvalidMobileNetwork());
    EXPECT_TRUE(m_pAosHandleMtc->IsInvalidMobileNetwork());
    EXPECT_TRUE(m_pAosHandleMtc->IsInvalidMobileNetwork());
    EXPECT_TRUE(m_pAosHandleMtc->IsInvalidMobileNetwork());
}

TEST_F(AosHandleMtcTest, IsPlmnBlockCondition_Test1)
{
    // Test1: IsPlmnBlockWithTimeoutOnVoiceCallUnavailable is false or network type is invalid
    // Expectation: return false

    EXPECT_CALL(m_objMockIAosNConfiguration, IsPlmnBlockWithTimeoutOnVoiceCallUnavailable())
            .Times(2)
            .WillOnce(Return(IMS_FALSE))
            .WillOnce(Return(IMS_TRUE));

    m_pAosHandleMtc->SetNetworkType(NW_REPORT_RADIO_LTE);
    EXPECT_FALSE(m_pAosHandleMtc->IsPlmnBlockCondition());

    m_pAosHandleMtc->SetNetworkType(NW_REPORT_RADIO_WCDMA);
    EXPECT_FALSE(m_pAosHandleMtc->IsPlmnBlockCondition());
}

TEST_F(AosHandleMtcTest, IsPlmnBlockCondition_Test2)
{
    // Test2: IsPlmnBlockWithTimeoutOnVoiceCallUnavailable is true, network type is LTE
    // Expectation: return false if cs voice is available else return true.

    EXPECT_CALL(m_objMockIAosNConfiguration, IsPlmnBlockWithTimeoutOnVoiceCallUnavailable())
            .Times(AnyNumber())
            .WillRepeatedly(Return(IMS_TRUE));

    m_pAosHandleMtc->SetNetworkType(NW_REPORT_RADIO_LTE);

    m_pAosHandleMtc->SetCsVoiceAvailable(IMS_FALSE);
    EXPECT_TRUE(m_pAosHandleMtc->IsPlmnBlockCondition());

    m_pAosHandleMtc->SetCsVoiceAvailable(IMS_TRUE);
    EXPECT_FALSE(m_pAosHandleMtc->IsPlmnBlockCondition());
}

TEST_F(AosHandleMtcTest, IsPlmnBlockCondition_Nr)
{
    // Test2: IsPlmnBlockWithTimeoutOnVoiceCallUnavailable is true, network type is NR
    // Expectation: return true regardless of cs voice availability

    EXPECT_CALL(m_objMockIAosNConfiguration, IsPlmnBlockWithTimeoutOnVoiceCallUnavailable())
            .Times(AnyNumber())
            .WillRepeatedly(Return(IMS_TRUE));

    m_pAosHandleMtc->SetNetworkType(NW_REPORT_RADIO_NR);

    m_pAosHandleMtc->SetCsVoiceAvailable(IMS_FALSE);
    EXPECT_TRUE(m_pAosHandleMtc->IsPlmnBlockCondition());

    m_pAosHandleMtc->SetCsVoiceAvailable(IMS_TRUE);
    EXPECT_TRUE(m_pAosHandleMtc->IsPlmnBlockCondition());
}

TEST_F(AosHandleMtcTest,
        VoiceNotCapableOnWiFiCallingIfNotUseGGsmaRcsTelephonyFeatureTagAsAvailableVoiceCallType)
{
    // GIVEN
    ON_CALL(m_objMockIAosNConfiguration,
            IsGGsmaRcsTelephonyFeatureTagUsedAsAvailableVoiceCallType())
            .WillByDefault(Return(IMS_FALSE));

    // WHEN & THEN
    EXPECT_FALSE(m_pAosHandleMtc->IsVoiceCapableOnWiFiCalling());
}

TEST_F(AosHandleMtcTest, VoiceNotCapableOnWiFiCallingIfNetworkTypeIsNotWlan)
{
    // GIVEN
    ON_CALL(m_objMockIAosNConfiguration,
            IsGGsmaRcsTelephonyFeatureTagUsedAsAvailableVoiceCallType())
            .WillByDefault(Return(IMS_TRUE));

    m_pAosHandleMtc->SetNetworkType(NW_REPORT_RADIO_LTE);

    // WHEN & THEN
    EXPECT_FALSE(m_pAosHandleMtc->IsVoiceCapableOnWiFiCalling());
}

TEST_F(AosHandleMtcTest, VoiceNotCapableOnWiFiCallingIfCallTrackerIsNull)
{
    // GIVEN
    ON_CALL(m_objMockIAosNConfiguration,
            IsGGsmaRcsTelephonyFeatureTagUsedAsAvailableVoiceCallType())
            .WillByDefault(Return(IMS_TRUE));

    m_pAosHandleMtc->SetNetworkType(NW_REPORT_RADIO_WLAN);
    AosProvider::GetInstance()->SetCallTracker(nullptr, 0);

    // WHEN & THEN
    EXPECT_FALSE(m_pAosHandleMtc->IsVoiceCapableOnWiFiCalling());
}

TEST_F(AosHandleMtcTest, VoiceNotCapableOnWiFiCallingIfNoActiveCall)
{
    // GIVEN
    ON_CALL(m_objMockIAosCallTracker, IsNormalCallActive()).WillByDefault(Return(IMS_FALSE));
    ON_CALL(m_objMockIAosNConfiguration,
            IsGGsmaRcsTelephonyFeatureTagUsedAsAvailableVoiceCallType())
            .WillByDefault(Return(IMS_TRUE));
    m_pAosHandleMtc->SetNetworkType(NW_REPORT_RADIO_WLAN);

    // WHEN & THEN
    EXPECT_FALSE(m_pAosHandleMtc->IsVoiceCapableOnWiFiCalling());
}

TEST_F(AosHandleMtcTest, VoiceCapableOnWiFiCallingIfWiFiCallIsActive)
{
    // GIVEN
    ON_CALL(m_objMockIAosCallTracker, IsNormalCallActive()).WillByDefault(Return(IMS_TRUE));
    ON_CALL(m_objMockIAosNConfiguration,
            IsGGsmaRcsTelephonyFeatureTagUsedAsAvailableVoiceCallType())
            .WillByDefault(Return(IMS_TRUE));
    m_pAosHandleMtc->SetNetworkType(NW_REPORT_RADIO_WLAN);

    // WHEN & THEN
    EXPECT_TRUE(m_pAosHandleMtc->IsVoiceCapableOnWiFiCalling());
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

    EXPECT_TRUE(m_pAosHandleMtc->ProcessHoldingVopsState(IMS_VOICE_OVER_PS_NOT_SUPPORTED));
    EXPECT_EQ(m_pAosHandleMtc->GetHoldingVopsState(), IMS_VOICE_OVER_PS_NOT_SUPPORTED);

    EXPECT_TRUE(m_pAosHandleMtc->ProcessHoldingVopsState(IMS_VOICE_OVER_PS_SUPPORTED));
    EXPECT_EQ(m_pAosHandleMtc->GetHoldingVopsState(), IMS_VOICE_OVER_PS_SUPPORTED);

    EXPECT_FALSE(m_pAosHandleMtc->ProcessHoldingVopsState(IMS_VOICE_OVER_PS_NOT_SUPPORTED));
    EXPECT_EQ(m_pAosHandleMtc->GetHoldingVopsState(), IMS_VOICE_OVER_PS_SUPPORTED);

    EXPECT_FALSE(m_pAosHandleMtc->ProcessHoldingVopsState(IMS_VOICE_OVER_PS_SUPPORTED));
    EXPECT_EQ(m_pAosHandleMtc->GetHoldingVopsState(), IMS_VOICE_OVER_PS_SUPPORTED);
}

TEST_F(AosHandleMtcTest, ProcessHoldingSsacState_Test)
{
    // Expectation: return true if m_bSsacHeld is changed else return false

    EXPECT_CALL(m_objMockIAosCallTracker, IsNormalCallActive())
            .Times(2)
            .WillOnce(Return(IMS_FALSE))
            .WillOnce(Return(IMS_TRUE));

    EXPECT_FALSE(m_pAosHandleMtc->ProcessHoldingSsacState(0));
    EXPECT_FALSE(m_pAosHandleMtc->IsSsacHeld());

    EXPECT_FALSE(m_pAosHandleMtc->ProcessHoldingSsacState(100));
    EXPECT_FALSE(m_pAosHandleMtc->IsSsacHeld());

    EXPECT_TRUE(m_pAosHandleMtc->ProcessHoldingSsacState(0));
    EXPECT_TRUE(m_pAosHandleMtc->IsSsacHeld());

    EXPECT_TRUE(m_pAosHandleMtc->ProcessHoldingSsacState(100));
    EXPECT_FALSE(m_pAosHandleMtc->IsSsacHeld());
}

TEST_F(AosHandleMtcTest, ProcessVolteHysTimerExpired_Test)
{
    // Expectation: Stop the timer / Unblock vops or ssac if blocked

    m_pAosHandleMtc->AddBlock(AosHandle::BLOCK_VOPS);
    m_pAosHandleMtc->StartVolteHysTimer(60);
    m_pAosHandleMtc->ProcessVolteHysTimerExpired();
    EXPECT_FALSE(m_pAosHandleMtc->IsHandleBlockedBase(AosHandle::BLOCK_VOPS));

    m_pAosHandleMtc->AddHoldingBlockForMobile(AosHandle::BLOCK_VOPS);
    m_pAosHandleMtc->StartVolteHysTimer(60);
    m_pAosHandleMtc->ProcessVolteHysTimerExpired();
    EXPECT_FALSE(m_pAosHandleMtc->IsHoldingBlockForMobile(AosHandle::BLOCK_VOPS));

    m_pAosHandleMtc->AddBlock(AosHandle::BLOCK_SSAC);
    m_pAosHandleMtc->StartVolteHysTimer(60);
    m_pAosHandleMtc->ProcessVolteHysTimerExpired();
    EXPECT_FALSE(m_pAosHandleMtc->IsHandleBlockedBase(AosHandle::BLOCK_SSAC));

    m_pAosHandleMtc->AddHoldingBlockForMobile(AosHandle::BLOCK_SSAC);
    m_pAosHandleMtc->StartVolteHysTimer(60);
    m_pAosHandleMtc->ProcessVolteHysTimerExpired();
    EXPECT_FALSE(m_pAosHandleMtc->IsHoldingBlockForMobile(AosHandle::BLOCK_SSAC));

    m_pAosHandleMtc->SetVopsState(IMS_VOICE_OVER_PS_NOT_SUPPORTED);
    m_pAosHandleMtc->SetSsacBarred(IMS_TRUE);
    m_pAosHandleMtc->StartVolteHysTimer(60);
    EXPECT_TRUE(m_pAosHandleMtc->IsVolteHysTimerRunning());
    m_pAosHandleMtc->ProcessVolteHysTimerExpired();
    EXPECT_FALSE(m_pAosHandleMtc->IsVolteHysTimerRunning());
}

TEST_F(AosHandleMtcTest, StartVolteHysTimer_Test)
{
    EXPECT_FALSE(m_pAosHandleMtc->StartVolteHysTimer(0));
    EXPECT_FALSE(m_pAosHandleMtc->IsVolteHysTimerRunning());

    EXPECT_TRUE(m_pAosHandleMtc->StartVolteHysTimer(60));
    EXPECT_TRUE(m_pAosHandleMtc->IsVolteHysTimerRunning());

    EXPECT_FALSE(m_pAosHandleMtc->StartVolteHysTimer(60));
    EXPECT_TRUE(m_pAosHandleMtc->IsVolteHysTimerRunning());

    m_pAosHandleMtc->StopVolteHysTimer();
}

TEST_F(AosHandleMtcTest, StopVolteHysTimer_Test)
{
    m_pAosHandleMtc->StartVolteHysTimer(60);
    EXPECT_TRUE(m_pAosHandleMtc->IsVolteHysTimerRunning());

    m_pAosHandleMtc->StopVolteHysTimer();
    EXPECT_FALSE(m_pAosHandleMtc->IsVolteHysTimerRunning());
}

TEST_F(AosHandleMtcTest, IsVolteHysTimerRunning_Test)
{
    EXPECT_FALSE(m_pAosHandleMtc->IsVolteHysTimerRunning());

    m_pAosHandleMtc->StartVolteHysTimer(60);
    EXPECT_TRUE(m_pAosHandleMtc->IsVolteHysTimerRunning());

    m_pAosHandleMtc->StopVolteHysTimer();
    EXPECT_FALSE(m_pAosHandleMtc->IsVolteHysTimerRunning());
}

TEST_F(AosHandleMtcTest, NConfiguration_NotifyConfigChanged_Test1)
{
    // Test1: VopsIgnoredForVolteEnabled not changed
    // Expectation: No action

    EXPECT_CALL(m_objMockIAosNConfiguration, IsVopsIgnoredForVolteEnabled())
            .Times(2)
            .WillOnce(Return(IMS_FALSE))
            .WillOnce(Return(IMS_TRUE));

    m_pAosHandleMtc->SetVopsIgnoredForVolteEnabled(IMS_FALSE);
    m_pAosHandleMtc->NConfiguration_NotifyConfigChanged();
    EXPECT_FALSE(m_pAosHandleMtc->IsVopsIgnoredForVolteEnabled());

    m_pAosHandleMtc->SetVopsIgnoredForVolteEnabled(IMS_TRUE);
    m_pAosHandleMtc->NConfiguration_NotifyConfigChanged();
    EXPECT_TRUE(m_pAosHandleMtc->IsVopsIgnoredForVolteEnabled());
}

TEST_F(AosHandleMtcTest, NConfiguration_NotifyConfigChanged_Test2)
{
    // Test2: VopsIgnoredForVolteEnabled changed, invalid network
    // Expectation: update with the config value

    EXPECT_CALL(m_objMockIAosNConfiguration, IsVopsIgnoredForVolteEnabled())
            .Times(2)
            .WillOnce(Return(IMS_FALSE))
            .WillOnce(Return(IMS_TRUE));

    m_pAosHandleMtc->SetNetworkType(NW_REPORT_RADIO_GSM);

    m_pAosHandleMtc->SetVopsIgnoredForVolteEnabled(IMS_TRUE);
    m_pAosHandleMtc->NConfiguration_NotifyConfigChanged();
    EXPECT_FALSE(m_pAosHandleMtc->IsVopsIgnoredForVolteEnabled());

    m_pAosHandleMtc->SetVopsIgnoredForVolteEnabled(IMS_FALSE);
    m_pAosHandleMtc->NConfiguration_NotifyConfigChanged();
    EXPECT_TRUE(m_pAosHandleMtc->IsVopsIgnoredForVolteEnabled());
}

TEST_F(AosHandleMtcTest, NConfiguration_NotifyConfigChanged_Test3)
{
    // Test3: VopsIgnoredForVolteEnabled changed, valid network, vops supported
    // Expectation: update with the config value

    EXPECT_CALL(m_objMockIAosNConfiguration, IsVopsIgnoredForVolteEnabled())
            .Times(2)
            .WillOnce(Return(IMS_FALSE))
            .WillOnce(Return(IMS_TRUE));

    m_pAosHandleMtc->SetNetworkType(NW_REPORT_RADIO_LTE);
    m_pAosHandleMtc->SetVopsState(IMS_VOICE_OVER_PS_SUPPORTED);

    m_pAosHandleMtc->SetVopsIgnoredForVolteEnabled(IMS_TRUE);
    m_pAosHandleMtc->NConfiguration_NotifyConfigChanged();
    EXPECT_FALSE(m_pAosHandleMtc->IsVopsIgnoredForVolteEnabled());

    m_pAosHandleMtc->SetVopsIgnoredForVolteEnabled(IMS_FALSE);
    m_pAosHandleMtc->NConfiguration_NotifyConfigChanged();
    EXPECT_TRUE(m_pAosHandleMtc->IsVopsIgnoredForVolteEnabled());
}

TEST_F(AosHandleMtcTest,
        ShouldUpdateVopsStateToSupportedIfVopsIgnoredForVolteEnabledIsChangedToTrue)
{
    // GIVEN
    m_pAosHandleMtc->SetNetworkType(NW_REPORT_RADIO_LTE);
    m_pAosHandleMtc->SetVopsState(IMS_VOICE_OVER_PS_NOT_SUPPORTED);
    m_pAosHandleMtc->AddBlock(AosHandle::BLOCK_VOPS);

    ON_CALL(m_objMockIAosNConfiguration, IsVopsIgnoredForVolteEnabled())
            .WillByDefault(Return(IMS_TRUE));

    // WHEN
    m_pAosHandleMtc->NConfiguration_NotifyConfigChanged();

    // THEN
    EXPECT_FALSE(m_pAosHandleMtc->IsHandleBlockedBase(AosHandle::BLOCK_VOPS));
    EXPECT_EQ(m_pAosHandleMtc->GetVopsState(), IMS_VOICE_OVER_PS_SUPPORTED);
    EXPECT_TRUE(m_pAosHandleMtc->IsVopsIgnoredForVolteEnabled());
    EXPECT_FALSE(m_pAosHandleMtc->IsVolteHysTimerRunning());
}

TEST_F(AosHandleMtcTest, ShouldKeepVopsStateToSupportedIfVopsIgnoredForVolteEnabledIsChangedToTrue)
{
    // GIVEN
    m_pAosHandleMtc->SetNetworkType(NW_REPORT_RADIO_LTE);
    m_pAosHandleMtc->SetVopsState(IMS_VOICE_OVER_PS_SUPPORTED);

    ON_CALL(m_objMockIAosNConfiguration, IsVopsIgnoredForVolteEnabled())
            .WillByDefault(Return(IMS_TRUE));

    // WHEN
    m_pAosHandleMtc->NConfiguration_NotifyConfigChanged();

    // THEN
    EXPECT_FALSE(m_pAosHandleMtc->IsHandleBlockedBase(AosHandle::BLOCK_VOPS));
    EXPECT_EQ(m_pAosHandleMtc->GetVopsState(), IMS_VOICE_OVER_PS_SUPPORTED);
    EXPECT_TRUE(m_pAosHandleMtc->IsVopsIgnoredForVolteEnabled());
}

TEST_F(AosHandleMtcTest,
        ShouldUpdateVopsStateToNotSupportedIfVopsIgnoredForVolteEnabledIsChangedToFalse)
{
    // GIVEN
    m_pAosHandleMtc->SetNetworkType(NW_REPORT_RADIO_LTE);
    m_pAosHandleMtc->SetVopsState(IMS_VOICE_OVER_PS_SUPPORTED);
    m_pAosHandleMtc->SetVopsIgnoredForVolteEnabled(IMS_TRUE);

    ON_CALL(m_objMockIAosNConfiguration, IsVopsIgnoredForVolteEnabled())
            .WillByDefault(Return(IMS_FALSE));
    ON_CALL(m_objMockIAosNetTracker, IsImsVoiceCallSupported()).WillByDefault(Return(IMS_FALSE));
    ON_CALL(m_objMockIAosNetTracker, GetNetworkOperator()).WillByDefault(Return("111111"));

    // WHEN
    m_pAosHandleMtc->NConfiguration_NotifyConfigChanged();

    // THEN
    EXPECT_TRUE(m_pAosHandleMtc->IsHandleBlockedBase(AosHandle::BLOCK_VOPS));
    EXPECT_EQ(m_pAosHandleMtc->GetVopsState(), IMS_VOICE_OVER_PS_NOT_SUPPORTED);
    EXPECT_FALSE(m_pAosHandleMtc->IsVopsIgnoredForVolteEnabled());
}

TEST_F(AosHandleMtcTest, ShouldKeepVopsStateToSupportedIfVopsIgnoredForVolteEnabledIsChangedToFalse)
{
    // GIVEN
    m_pAosHandleMtc->SetNetworkType(NW_REPORT_RADIO_LTE);
    m_pAosHandleMtc->SetVopsState(IMS_VOICE_OVER_PS_SUPPORTED);
    m_pAosHandleMtc->SetVopsIgnoredForVolteEnabled(IMS_TRUE);

    ON_CALL(m_objMockIAosNConfiguration, IsVopsIgnoredForVolteEnabled())
            .WillByDefault(Return(IMS_FALSE));
    ON_CALL(m_objMockIAosNetTracker, IsImsVoiceCallSupported()).WillByDefault(Return(IMS_TRUE));
    ON_CALL(m_objMockIAosNetTracker, GetNetworkOperator()).WillByDefault(Return("111111"));

    // WHEN
    m_pAosHandleMtc->NConfiguration_NotifyConfigChanged();

    // THEN
    EXPECT_FALSE(m_pAosHandleMtc->IsHandleBlockedBase(AosHandle::BLOCK_VOPS));
    EXPECT_EQ(m_pAosHandleMtc->GetVopsState(), IMS_VOICE_OVER_PS_SUPPORTED);
    EXPECT_FALSE(m_pAosHandleMtc->IsVopsIgnoredForVolteEnabled());
}

TEST_F(AosHandleMtcTest, ImsRadio_OnSsacChanged_Test1)
{
    // Test1: Ignoring case
    // Expectation: m_bSsacBarred not changed

    SsacInfo objSsacInfo;
    objSsacInfo.nBarringFactorForVoice = 0;

    m_pAosHandleMtc->SetSsacBarred(IMS_FALSE);

    EXPECT_CALL(m_objMockIAosNConfiguration, IsRequiredVolteBlockBySsac())
            .Times(3)
            .WillOnce(Return(IMS_FALSE))
            .WillOnce(Return(IMS_TRUE))
            .WillOnce(Return(IMS_TRUE));

    m_pAosHandleMtc->ImsRadio_OnSsacChanged(objSsacInfo);
    EXPECT_FALSE(m_pAosHandleMtc->IsSsacBarred());

    EXPECT_CALL(m_objMockIAosCallTracker, IsNormalCallActive())
            .Times(2)
            .WillOnce(Return(IMS_TRUE))
            .WillOnce(Return(IMS_FALSE));

    m_pAosHandleMtc->ImsRadio_OnSsacChanged(objSsacInfo);
    EXPECT_FALSE(m_pAosHandleMtc->IsSsacBarred());

    m_pAosHandleMtc->SetNetworkType(NW_REPORT_RADIO_NR);

    m_pAosHandleMtc->ImsRadio_OnSsacChanged(objSsacInfo);
    EXPECT_FALSE(m_pAosHandleMtc->IsSsacBarred());
}

TEST_F(AosHandleMtcTest, ImsRadio_OnSsacChanged_Test2)
{
    // Test2: Barring factor is 0, PlmnBlockCondition is true
    // Expectation: m_SsacBarred is set to true, Plmn block is requested.

    EXPECT_CALL(m_objMockIAosNConfiguration, IsRequiredVolteBlockBySsac())
            .Times(AnyNumber())
            .WillRepeatedly(Return(IMS_TRUE));

    m_pAosHandleMtc->SetNetworkType(NW_REPORT_RADIO_LTE);
    m_pAosHandleMtc->SetSsacBarred(IMS_FALSE);

    SsacInfo objSsacInfo;
    objSsacInfo.nBarringFactorForVoice = 0;

    EXPECT_CALL(m_objMockIAosNConfiguration, IsPlmnBlockWithTimeoutOnVoiceCallUnavailable())
            .Times(1)
            .WillOnce(Return(IMS_TRUE));

    m_pAosHandleMtc->SetRoamingState(IMS_ROAMING_STATE_OFF);

    EXPECT_CALL(m_objMockIAosApplication,
            RequestCmd(ImsAosControl::PLMN_BLOCK_WITH_TIMEOUT, AosReason::SSAC_BARRED))
            .Times(1);

    m_pAosHandleMtc->ImsRadio_OnSsacChanged(objSsacInfo);
    EXPECT_TRUE(m_pAosHandleMtc->IsSsacBarred());
    EXPECT_TRUE(m_pAosHandleMtc->IsHandleBlockedBase(AosHandle::BLOCK_SSAC));
}

TEST_F(AosHandleMtcTest, ImsRadio_OnSsacChanged_Test3)
{
    // Test3: Barring factor is 0, PlmnBlockCondition is false
    // Expectation: m_SsacBarred is set to true, BLOCK_SSAC is blocked

    EXPECT_CALL(m_objMockIAosNConfiguration, IsRequiredVolteBlockBySsac())
            .Times(AnyNumber())
            .WillRepeatedly(Return(IMS_TRUE));

    m_pAosHandleMtc->SetNetworkType(NW_REPORT_RADIO_LTE);
    m_pAosHandleMtc->SetDataConnected(IMS_TRUE);
    m_pAosHandleMtc->SetSsacBarred(IMS_FALSE);

    SsacInfo objSsacInfo;
    objSsacInfo.nBarringFactorForVoice = 0;

    EXPECT_CALL(m_objMockIAosNConfiguration, IsWfcImsAvailable())
            .Times(1)
            .WillOnce(Return(IMS_TRUE));

    EXPECT_CALL(m_objMockIAosNConfiguration, IsPlmnBlockWithTimeoutOnVoiceCallUnavailable())
            .Times(1)
            .WillOnce(Return(IMS_FALSE));

    m_pAosHandleMtc->ImsRadio_OnSsacChanged(objSsacInfo);
    EXPECT_TRUE(m_pAosHandleMtc->IsSsacBarred());
    EXPECT_TRUE(m_pAosHandleMtc->IsHandleBlockedBase(AosHandle::BLOCK_SSAC));
}

TEST_F(AosHandleMtcTest, ImsRadio_OnSsacChanged_Test4)
{
    // Test4: Barring factor is 100, BLOCK_SSAC not blocked
    // Expectation: m_SsacBarred is set to false

    EXPECT_CALL(m_objMockIAosNConfiguration, IsRequiredVolteBlockBySsac())
            .Times(AnyNumber())
            .WillRepeatedly(Return(IMS_TRUE));

    m_pAosHandleMtc->SetSsacBarred(IMS_TRUE);

    SsacInfo objSsacInfo;
    objSsacInfo.nBarringFactorForVoice = 100;

    m_pAosHandleMtc->ImsRadio_OnSsacChanged(objSsacInfo);
    EXPECT_FALSE(m_pAosHandleMtc->IsSsacBarred());
}

TEST_F(AosHandleMtcTest, NotStartVolteHysTimerIfSsacUnbarredOnConnectingState)
{
    // GIVEN
    SsacInfo objSsacInfo;
    objSsacInfo.nBarringFactorForVoice = 100;
    m_pAosHandleMtc->SetHandleState(AosHandle::STATE_CONNECTING);
    m_pAosHandleMtc->SetNetworkType(NW_REPORT_RADIO_LTE);
    m_pAosHandleMtc->SetSsacBarred(IMS_TRUE);
    m_pAosHandleMtc->AddBlock(AosHandle::BLOCK_SSAC);

    ON_CALL(m_objMockIAosNConfiguration, IsRequiredVolteBlockBySsac())
            .WillByDefault(Return(IMS_TRUE));
    ON_CALL(m_objMockIAosNConfiguration, GetVolteHysTime()).WillByDefault(Return(60));

    // WHEN
    m_pAosHandleMtc->ImsRadio_OnSsacChanged(objSsacInfo);

    // THEN
    EXPECT_FALSE(m_pAosHandleMtc->IsSsacBarred());
    EXPECT_FALSE(m_pAosHandleMtc->IsHandleBlockedBase(AosHandle::BLOCK_SSAC));
    EXPECT_FALSE(m_pAosHandleMtc->IsVolteHysTimerRunning());
}

TEST_F(AosHandleMtcTest, NotStartVolteHysTimerIfSsacUnbarredOnConnectedState)
{
    // GIVEN
    SsacInfo objSsacInfo;
    objSsacInfo.nBarringFactorForVoice = 100;
    m_pAosHandleMtc->SetHandleState(AosHandle::STATE_CONNECTED);
    m_pAosHandleMtc->SetNetworkType(NW_REPORT_RADIO_LTE);
    m_pAosHandleMtc->SetSsacBarred(IMS_TRUE);
    m_pAosHandleMtc->AddBlock(AosHandle::BLOCK_SSAC);

    ON_CALL(m_objMockIAosNConfiguration, IsRequiredVolteBlockBySsac())
            .WillByDefault(Return(IMS_TRUE));
    ON_CALL(m_objMockIAosNConfiguration, GetVolteHysTime()).WillByDefault(Return(60));

    // WHEN
    m_pAosHandleMtc->ImsRadio_OnSsacChanged(objSsacInfo);

    // THEN
    EXPECT_FALSE(m_pAosHandleMtc->IsSsacBarred());
    EXPECT_FALSE(m_pAosHandleMtc->IsHandleBlockedBase(AosHandle::BLOCK_SSAC));
    EXPECT_FALSE(m_pAosHandleMtc->IsVolteHysTimerRunning());
}

TEST_F(AosHandleMtcTest, NotStartVolteHysTimerIfSsacUnbarredOnDisconnectingState)
{
    // GIVEN
    SsacInfo objSsacInfo;
    objSsacInfo.nBarringFactorForVoice = 100;
    m_pAosHandleMtc->SetHandleState(AosHandle::STATE_DISCONNECTING);
    m_pAosHandleMtc->SetNetworkType(NW_REPORT_RADIO_LTE);
    m_pAosHandleMtc->SetSsacBarred(IMS_TRUE);
    m_pAosHandleMtc->AddBlock(AosHandle::BLOCK_SSAC);

    ON_CALL(m_objMockIAosNConfiguration, IsRequiredVolteBlockBySsac())
            .WillByDefault(Return(IMS_TRUE));
    ON_CALL(m_objMockIAosNConfiguration, GetVolteHysTime()).WillByDefault(Return(60));

    // WHEN
    m_pAosHandleMtc->ImsRadio_OnSsacChanged(objSsacInfo);

    // THEN
    EXPECT_FALSE(m_pAosHandleMtc->IsSsacBarred());
    EXPECT_FALSE(m_pAosHandleMtc->IsHandleBlockedBase(AosHandle::BLOCK_SSAC));
    EXPECT_FALSE(m_pAosHandleMtc->IsVolteHysTimerRunning());
}

TEST_F(AosHandleMtcTest, NotStartVolteHysTimerIfSsacUnbarredOnVopsNotSupported)
{
    // GIVEN
    SsacInfo objSsacInfo;
    objSsacInfo.nBarringFactorForVoice = 100;
    m_pAosHandleMtc->SetHandleState(AosHandle::STATE_DISCONNECTED);
    m_pAosHandleMtc->SetNetworkType(NW_REPORT_RADIO_LTE);
    m_pAosHandleMtc->SetRoamingState(IMS_ROAMING_STATE_ON);
    m_pAosHandleMtc->SetSsacBarred(IMS_TRUE);
    m_pAosHandleMtc->AddBlock(AosHandle::BLOCK_SSAC);
    m_pAosHandleMtc->SetVopsState(IMS_VOICE_OVER_PS_NOT_SUPPORTED);
    m_pAosHandleMtc->AddBlock(AosHandle::BLOCK_VOPS);

    ON_CALL(m_objMockIAosNConfiguration, IsRequiredVolteBlockBySsac())
            .WillByDefault(Return(IMS_TRUE));
    ON_CALL(m_objMockIAosNConfiguration, GetVolteHysTime()).WillByDefault(Return(60));

    // WHEN
    m_pAosHandleMtc->ImsRadio_OnSsacChanged(objSsacInfo);

    // THEN
    EXPECT_FALSE(m_pAosHandleMtc->IsSsacBarred());
    EXPECT_FALSE(m_pAosHandleMtc->IsHandleBlockedBase(AosHandle::BLOCK_SSAC));
    EXPECT_FALSE(m_pAosHandleMtc->IsVolteHysTimerRunning());
}

TEST_F(AosHandleMtcTest, NotStartVolteHysTimerIfSsacUnbarredOnNonLte)
{
    // GIVEN
    SsacInfo objSsacInfo;
    objSsacInfo.nBarringFactorForVoice = 100;
    m_pAosHandleMtc->SetHandleState(AosHandle::STATE_DISCONNECTED);
    m_pAosHandleMtc->SetNetworkType(NW_REPORT_RADIO_NR);
    m_pAosHandleMtc->SetRoamingState(IMS_ROAMING_STATE_ON);
    m_pAosHandleMtc->SetSsacBarred(IMS_TRUE);
    m_pAosHandleMtc->AddBlock(AosHandle::BLOCK_SSAC);

    ON_CALL(m_objMockIAosNConfiguration, IsRequiredVolteBlockBySsac())
            .WillByDefault(Return(IMS_TRUE));
    ON_CALL(m_objMockIAosNConfiguration, GetVolteHysTime()).WillByDefault(Return(60));

    // WHEN
    m_pAosHandleMtc->ImsRadio_OnSsacChanged(objSsacInfo);

    // THEN
    EXPECT_FALSE(m_pAosHandleMtc->IsSsacBarred());
    EXPECT_FALSE(m_pAosHandleMtc->IsHandleBlockedBase(AosHandle::BLOCK_SSAC));
    EXPECT_FALSE(m_pAosHandleMtc->IsVolteHysTimerRunning());
}

TEST_F(AosHandleMtcTest, NotStartVolteHysTimerIfSsacUnbarredOnLteHome)
{
    // GIVEN
    SsacInfo objSsacInfo;
    objSsacInfo.nBarringFactorForVoice = 100;
    m_pAosHandleMtc->SetHandleState(AosHandle::STATE_DISCONNECTED);
    m_pAosHandleMtc->SetNetworkType(NW_REPORT_RADIO_LTE);
    m_pAosHandleMtc->SetSsacBarred(IMS_TRUE);
    m_pAosHandleMtc->AddBlock(AosHandle::BLOCK_SSAC);
    m_pAosHandleMtc->SetRoamingState(IMS_ROAMING_STATE_OFF);

    ON_CALL(m_objMockIAosNConfiguration, IsRequiredVolteBlockBySsac())
            .WillByDefault(Return(IMS_TRUE));
    ON_CALL(m_objMockIAosNConfiguration, GetVolteHysTime()).WillByDefault(Return(60));

    // WHEN
    m_pAosHandleMtc->ImsRadio_OnSsacChanged(objSsacInfo);

    // THEN
    EXPECT_FALSE(m_pAosHandleMtc->IsSsacBarred());
    EXPECT_FALSE(m_pAosHandleMtc->IsHandleBlockedBase(AosHandle::BLOCK_SSAC));
    EXPECT_FALSE(m_pAosHandleMtc->IsVolteHysTimerRunning());
}

TEST_F(AosHandleMtcTest, StartVolteHysTimerIfSsacUnbarredOnLteRoaming)
{
    // GIVEN
    SsacInfo objSsacInfo;
    objSsacInfo.nBarringFactorForVoice = 100;
    m_pAosHandleMtc->SetHandleState(AosHandle::STATE_DISCONNECTED);
    m_pAosHandleMtc->SetNetworkType(NW_REPORT_RADIO_LTE);
    m_pAosHandleMtc->SetSsacBarred(IMS_TRUE);
    m_pAosHandleMtc->AddBlock(AosHandle::BLOCK_SSAC);
    m_pAosHandleMtc->SetRoamingState(IMS_ROAMING_STATE_ON);

    ON_CALL(m_objMockIAosNConfiguration, IsRequiredVolteBlockBySsac())
            .WillByDefault(Return(IMS_TRUE));
    ON_CALL(m_objMockIAosNConfiguration, GetVolteHysTime()).WillByDefault(Return(60));

    // WHEN
    m_pAosHandleMtc->ImsRadio_OnSsacChanged(objSsacInfo);

    // THEN
    EXPECT_FALSE(m_pAosHandleMtc->IsSsacBarred());
    EXPECT_TRUE(m_pAosHandleMtc->IsHandleBlockedBase(AosHandle::BLOCK_SSAC));
    EXPECT_TRUE(m_pAosHandleMtc->IsVolteHysTimerRunning());
}

TEST_F(AosHandleMtcTest, ImsRadio_OnSsacChanged_VolteHysTimerRunning_by_Vops)
{
    EXPECT_CALL(m_objMockIAosCallTracker, IsNormalCallActive())
            .Times(AnyNumber())
            .WillRepeatedly(Return(IMS_FALSE));

    m_pAosHandleMtc->SetNetworkType(NW_REPORT_RADIO_LTE);
    m_pAosHandleMtc->SetRoamingState(IMS_ROAMING_STATE_ON);

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

    m_pAosHandleMtc->SetNetworkType(NW_REPORT_RADIO_LTE);

    m_pAosHandleMtc->ProcessVopsStateChanged(IMS_VOICE_OVER_PS_NOT_SUPPORTED, AString("123456"));
    ASSERT_EQ(m_pAosHandleMtc->GetVopsState(), IMS_VOICE_OVER_PS_NOT_SUPPORTED);
    ASSERT_TRUE(m_pAosHandleMtc->IsHandleBlockedBase(AosHandle::BLOCK_VOPS));

    m_pAosHandleMtc->ProcessVopsStateChanged(IMS_VOICE_OVER_PS_SUPPORTED, AString("123456"));
    ASSERT_TRUE(m_pAosHandleMtc->IsVolteHysTimerRunning());
    ASSERT_EQ(m_pAosHandleMtc->GetVopsState(), IMS_VOICE_OVER_PS_SUPPORTED);
    ASSERT_TRUE(m_pAosHandleMtc->IsHandleBlockedBase(AosHandle::BLOCK_VOPS));

    SsacInfo objSsacInfo;
    objSsacInfo.nBarringFactorForVoice = 0;

    m_pAosHandleMtc->ImsRadio_OnSsacChanged(objSsacInfo);
    EXPECT_FALSE(m_pAosHandleMtc->IsVolteHysTimerRunning());
    EXPECT_TRUE(m_pAosHandleMtc->IsSsacBarred());
    EXPECT_TRUE(m_pAosHandleMtc->IsHandleBlockedBase(AosHandle::BLOCK_SSAC));
    EXPECT_FALSE(m_pAosHandleMtc->IsHandleBlockedBase(AosHandle::BLOCK_VOPS));
}

TEST_F(AosHandleMtcTest, ShouldUnblockVolteHysTimerIfSsacBarred)
{
    // GIVEN
    m_pAosHandleMtc->SetNetworkType(NW_REPORT_RADIO_LTE);

    ON_CALL(m_objMockIAosNConfiguration, IsRequiredVolteBlockBySsac())
            .WillByDefault(Return(IMS_TRUE));

    // WHEN
    SsacInfo objSsacInfo;
    objSsacInfo.nBarringFactorForVoice = 0;
    m_pAosHandleMtc->ImsRadio_OnSsacChanged(objSsacInfo);

    // THEN
    EXPECT_FALSE(m_pAosHandleMtc->IsVolteHysTimerBlockedForSsac());
}

TEST_F(AosHandleMtcTest, DoNothingWhenPlmnChangedIfVolteHysTimeIsEqualToZero)
{
    // GIVEN
    ON_CALL(m_objMockIAosNConfiguration, GetVolteHysTime()).WillByDefault(Return(0));

    // WHEN
    m_pAosHandleMtc->ServicePhone_PlmnChanged(AString("123456"));
}

TEST_F(AosHandleMtcTest, ShouldBlockVolteHysTimerForVopsIfPlmnChangedOnVopsNotSupported)
{
    // GIVEN
    m_pAosHandleMtc->SetVopsPlmn(AString("111111"));
    m_pAosHandleMtc->SetVopsState(IMS_VOICE_OVER_PS_NOT_SUPPORTED);

    ON_CALL(m_objMockIAosNConfiguration, GetVolteHysTime()).WillByDefault(Return(60));

    // WHEN
    m_pAosHandleMtc->ServicePhone_PlmnChanged(AString("222222"));

    // THEN
    EXPECT_TRUE(m_pAosHandleMtc->IsVolteHysTimerBlockedForVops());
}

TEST_F(AosHandleMtcTest, ShouldNotBlockVolteHysTimerForVopsIfPlmnChangedOnVopsSupported)
{
    // GIVEN
    m_pAosHandleMtc->SetVopsPlmn(AString("111111"));
    m_pAosHandleMtc->SetVopsState(IMS_VOICE_OVER_PS_SUPPORTED);

    ON_CALL(m_objMockIAosNConfiguration, GetVolteHysTime()).WillByDefault(Return(60));

    // WHEN
    m_pAosHandleMtc->ServicePhone_PlmnChanged(AString("222222"));

    // THEN
    EXPECT_FALSE(m_pAosHandleMtc->IsVolteHysTimerBlockedForVops());
}

TEST_F(AosHandleMtcTest, ShouldNotUnblockVolteHysTimerForVopsIfPlmnIsNotChanged)
{
    // GIVEN
    m_pAosHandleMtc->SetVopsPlmn(AString("111111"));
    m_pAosHandleMtc->SetVolteHysTimerBlockedForVops();
    ON_CALL(m_objMockIAosNConfiguration, GetVolteHysTime()).WillByDefault(Return(60));

    // WHEN
    m_pAosHandleMtc->ServicePhone_PlmnChanged(AString("111111"));

    // THEN
    EXPECT_TRUE(m_pAosHandleMtc->IsVolteHysTimerBlockedForVops());
}

TEST_F(AosHandleMtcTest, ShouldBlockVolteHysTimerForSsacIfPlmnChangedOnSsacBarred)
{
    // GIVEN
    m_pAosHandleMtc->SetSsacPlmn(AString("111111"));
    m_pAosHandleMtc->SetSsacBarred(IMS_TRUE);

    ON_CALL(m_objMockIAosNConfiguration, GetVolteHysTime()).WillByDefault(Return(60));

    // WHEN
    m_pAosHandleMtc->ServicePhone_PlmnChanged(AString("222222"));

    // THEN
    EXPECT_TRUE(m_pAosHandleMtc->IsVolteHysTimerBlockedForSsac());
}

TEST_F(AosHandleMtcTest, ShouldNotBlockVolteHysTimerForSsacIfPlmnChangedOnSsacNotBarred)
{
    // GIVEN
    m_pAosHandleMtc->SetSsacPlmn(AString("111111"));
    m_pAosHandleMtc->SetSsacBarred(IMS_FALSE);

    ON_CALL(m_objMockIAosNConfiguration, GetVolteHysTime()).WillByDefault(Return(60));

    // WHEN
    m_pAosHandleMtc->ServicePhone_PlmnChanged(AString("222222"));

    // THEN
    EXPECT_FALSE(m_pAosHandleMtc->IsVolteHysTimerBlockedForSsac());
}

TEST_F(AosHandleMtcTest, ShouldNotUnblockVolteHysTimerForSsacIfPlmnIsNotChanged)
{
    // GIVEN
    m_pAosHandleMtc->SetSsacPlmn(AString("111111"));
    m_pAosHandleMtc->SetVolteHysTimerBlockedForSsac();
    ON_CALL(m_objMockIAosNConfiguration, GetVolteHysTime()).WillByDefault(Return(60));

    // WHEN
    m_pAosHandleMtc->ServicePhone_PlmnChanged(AString("111111"));

    // THEN
    EXPECT_TRUE(m_pAosHandleMtc->IsVolteHysTimerBlockedForSsac());
}

TEST_F(AosHandleMtcTest, ShouldNotBlockVolteHysTimerForSsacIfPlmnIsNotChanged)
{
    // GIVEN
    m_pAosHandleMtc->SetSsacPlmn(AString("111111"));
    ON_CALL(m_objMockIAosNConfiguration, GetVolteHysTime()).WillByDefault(Return(60));

    // WHEN
    m_pAosHandleMtc->ServicePhone_PlmnChanged(AString("111111"));

    // THEN
    EXPECT_FALSE(m_pAosHandleMtc->IsVolteHysTimerBlockedForSsac());
}

TEST_F(AosHandleMtcTest, ShouldStopVolteHysTimerIfRunningWhenPlmnChanged)
{
    // GIVEN
    ON_CALL(m_objMockIAosNConfiguration, GetVolteHysTime()).WillByDefault(Return(60));
    m_pAosHandleMtc->AddBlock(AosHandle::BLOCK_VOPS);
    m_pAosHandleMtc->StartVolteHysTimer(60);

    // WHEN
    m_pAosHandleMtc->ServicePhone_PlmnChanged(AString("111111"));

    // THEN
    EXPECT_FALSE(m_pAosHandleMtc->IsVolteHysTimerRunning());
    EXPECT_FALSE(m_pAosHandleMtc->IsHandleBlockedBase(AosHandle::BLOCK_VOPS));
}

TEST_F(AosHandleMtcTest, Timer_TimerExpired_Test)
{
    // Expectation: Do nothing if the timer is null or other than VolteHysTimer.
    //              Else run ProcessVolteHysTimerExpired.

    m_pAosHandleMtc->StartVolteHysTimer(60);
    m_pAosHandleMtc->Timer_TimerExpired(IMS_NULL);
    EXPECT_TRUE(m_pAosHandleMtc->IsVolteHysTimerRunning());

    ITimer* piTestTimer = AosUtil::GetInstance()->StartTimer(
            60 * 1000, m_pAosHandleMtc, "AosHandleMtcTest_Timer");

    m_pAosHandleMtc->Timer_TimerExpired(piTestTimer);
    EXPECT_TRUE(m_pAosHandleMtc->IsVolteHysTimerRunning());

    m_pAosHandleMtc->Timer_TimerExpired(m_pAosHandleMtc->GetVolteHysTimer());
    EXPECT_FALSE(m_pAosHandleMtc->IsVolteHysTimerRunning());

    AosUtil::GetInstance()->StopTimer(piTestTimer, "AosHandleMtcTest_Timer");
}

TEST_F(AosHandleMtcTest, StopVolteHysTimer_PdnLost_Then_UmtsGsm)
{
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

    m_pAosHandleMtc->SetNetSrvIn(IMS_TRUE);
    m_pAosHandleMtc->SetHandleState(AosHandle::STATE_CONNECTED);
    m_pAosHandleMtc->SetDataConnected(IMS_TRUE);
    m_pAosHandleMtc->SetNetworkType(NW_REPORT_RADIO_LTE);
    m_pAosHandleMtc->AddBlock(AosHandle::BLOCK_SSAC);
    m_pAosHandleMtc->StartVolteHysTimer(60);

    m_pAosHandleMtc->NetTracker_StatusChanged();  // Data changed
    EXPECT_FALSE(m_pAosHandleMtc->GetDataConnected());
    EXPECT_EQ(m_pAosHandleMtc->GetStoredNetworkType(), NW_REPORT_RADIO_LTE);
    EXPECT_TRUE(m_pAosHandleMtc->IsVolteHysTimerRunning());

    EXPECT_CALL(m_objMockIAosNetTracker, GetNetworkType())
            .Times(AnyNumber())
            .WillRepeatedly(Return(NW_REPORT_RADIO_GSM));

    m_pAosHandleMtc->NetTracker_StatusChanged();  // Moves to unsupported RAT(=GSM)
    EXPECT_EQ(m_pAosHandleMtc->GetStoredNetworkType(), NW_REPORT_RADIO_GSM);
    EXPECT_FALSE(m_pAosHandleMtc->IsVolteHysTimerRunning());
    EXPECT_FALSE(m_pAosHandleMtc->IsHandleBlockedBase(AosHandle::BLOCK_SSAC));
}

TEST_F(AosHandleMtcTest, StopVolteHysTimer_UmtsGsm_Then_PdnLost)
{
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

    m_pAosHandleMtc->SetNetSrvIn(IMS_TRUE);
    m_pAosHandleMtc->SetHandleState(AosHandle::STATE_CONNECTED);
    m_pAosHandleMtc->SetDataConnected(IMS_TRUE);
    m_pAosHandleMtc->SetNetworkType(NW_REPORT_RADIO_LTE);
    m_pAosHandleMtc->AddBlock(AosHandle::BLOCK_SSAC);
    m_pAosHandleMtc->StartVolteHysTimer(60);

    m_pAosHandleMtc->NetTracker_StatusChanged();  // Moves to unsupported RAT(=WCDMA)
    EXPECT_EQ(m_pAosHandleMtc->GetStoredNetworkType(), NW_REPORT_RADIO_WCDMA);
    EXPECT_TRUE(m_pAosHandleMtc->GetDataConnected());
    EXPECT_TRUE(m_pAosHandleMtc->IsVolteHysTimerRunning());

    EXPECT_CALL(m_objMockIAosConnection, GetState())
            .Times(AnyNumber())
            .WillRepeatedly(Return(IAosConnection::STATE_IDLE));

    m_pAosHandleMtc->NetTracker_StatusChanged();  // Data changed
    EXPECT_FALSE(m_pAosHandleMtc->GetDataConnected());
    EXPECT_EQ(m_pAosHandleMtc->GetStoredNetworkType(), NW_REPORT_RADIO_WCDMA);
    EXPECT_FALSE(m_pAosHandleMtc->IsVolteHysTimerRunning());
    EXPECT_FALSE(m_pAosHandleMtc->IsHandleBlockedBase(AosHandle::BLOCK_SSAC));
}

TEST_F(AosHandleMtcTest,
        ShouldBlockVolteHysTimerForVopsIfPdnLostOnNetworkChangeToUmtsOnVopsNotSupported)
{
    // GIVEN
    m_pAosHandleMtc->SetNetSrvIn(IMS_TRUE);
    m_pAosHandleMtc->SetHandleState(AosHandle::STATE_CONNECTED);
    m_pAosHandleMtc->SetDataConnected(IMS_TRUE);
    m_pAosHandleMtc->SetNetworkType(NW_REPORT_RADIO_LTE);
    m_pAosHandleMtc->SetVopsState(IMS_VOICE_OVER_PS_NOT_SUPPORTED);

    ON_CALL(m_objMockIAosNConfiguration, GetVolteHysTime()).WillByDefault(Return(60));
    ON_CALL(m_objMockIAosConnection, GetState())
            .WillByDefault(Return(IAosConnection::STATE_ACTIVE));
    ON_CALL(m_objMockIAosNetTracker, GetNetworkType()).WillByDefault(Return(NW_REPORT_RADIO_WCDMA));
    ON_CALL(m_objMockIAosNetTracker, IsSuspended()).WillByDefault(Return(IMS_FALSE));

    m_pAosHandleMtc->NetTracker_StatusChanged();  // Moves to unsupported RAT(=WCDMA)

    ON_CALL(m_objMockIAosConnection, GetState()).WillByDefault(Return(IAosConnection::STATE_IDLE));

    // WHEN
    m_pAosHandleMtc->NetTracker_StatusChanged();  // Data changed

    // THEN
    EXPECT_EQ(m_pAosHandleMtc->GetStoredNetworkType(), NW_REPORT_RADIO_WCDMA);
    EXPECT_FALSE(m_pAosHandleMtc->GetDataConnected());
    EXPECT_TRUE(m_pAosHandleMtc->IsVolteHysTimerBlockedForVops());
}

TEST_F(AosHandleMtcTest,
        ShouldNotBlockVolteHysTimerForVopsIfPdnLostOnNetworkChangeToUmtsOnVopsSupported)
{
    // GIVEN
    m_pAosHandleMtc->SetNetSrvIn(IMS_TRUE);
    m_pAosHandleMtc->SetHandleState(AosHandle::STATE_CONNECTED);
    m_pAosHandleMtc->SetDataConnected(IMS_TRUE);
    m_pAosHandleMtc->SetNetworkType(NW_REPORT_RADIO_LTE);
    m_pAosHandleMtc->SetVopsState(IMS_VOICE_OVER_PS_SUPPORTED);

    ON_CALL(m_objMockIAosNConfiguration, GetVolteHysTime()).WillByDefault(Return(60));
    ON_CALL(m_objMockIAosConnection, GetState())
            .WillByDefault(Return(IAosConnection::STATE_ACTIVE));
    ON_CALL(m_objMockIAosNetTracker, GetNetworkType()).WillByDefault(Return(NW_REPORT_RADIO_WCDMA));
    ON_CALL(m_objMockIAosNetTracker, IsSuspended()).WillByDefault(Return(IMS_FALSE));

    m_pAosHandleMtc->NetTracker_StatusChanged();  // Moves to unsupported RAT(=WCDMA)

    ON_CALL(m_objMockIAosConnection, GetState()).WillByDefault(Return(IAosConnection::STATE_IDLE));

    // WHEN
    m_pAosHandleMtc->NetTracker_StatusChanged();  // Data changed

    // THEN
    EXPECT_EQ(m_pAosHandleMtc->GetStoredNetworkType(), NW_REPORT_RADIO_WCDMA);
    EXPECT_FALSE(m_pAosHandleMtc->GetDataConnected());
    EXPECT_FALSE(m_pAosHandleMtc->IsVolteHysTimerBlockedForVops());
}

TEST_F(AosHandleMtcTest, ShouldUnblockVolteHysTimerForVopsIfVopsStillNotSupportedOnNetworkChange)
{
    // GIVEN
    m_pAosHandleMtc->SetVopsIgnoredForVolteEnabled(IMS_FALSE);
    m_pAosHandleMtc->SetVopsState(IMS_VOICE_OVER_PS_NOT_SUPPORTED);
    m_pAosHandleMtc->SetVopsPlmn(AString("123456"));
    m_pAosHandleMtc->SetVolteHysTimerBlockedForVops();
    m_pAosHandleMtc->SetDataConnected(IMS_TRUE);
    m_pAosHandleMtc->SetNetSrvIn(IMS_TRUE);
    m_pAosHandleMtc->SetNetworkType(NW_REPORT_RADIO_WCDMA);

    ON_CALL(m_objMockIAosNConfiguration, GetVolteHysTime()).WillByDefault(Return(60));
    ON_CALL(m_objMockIAosConnection, GetState())
            .WillByDefault(Return(IAosConnection::STATE_ACTIVE));
    ON_CALL(m_objMockIAosNetTracker, GetNetworkType()).WillByDefault(Return(NW_REPORT_RADIO_LTE));
    ON_CALL(m_objMockIAosNetTracker, IsSuspended()).WillByDefault(Return(IMS_FALSE));
    ON_CALL(m_objMockIAosNetTracker, IsImsVoiceCallSupported()).WillByDefault(Return(IMS_FALSE));
    ON_CALL(m_objMockIAosNetTracker, GetNetworkOperator()).WillByDefault(Return(AString("123456")));

    // WHEN
    m_pAosHandleMtc->NetTracker_StatusChanged();  // Network changed

    // THEN
    EXPECT_FALSE(m_pAosHandleMtc->IsVolteHysTimerBlockedForVops());
}

TEST_F(AosHandleMtcTest, ShouldBlockVolteHysTimerForSsacIfPdnLostOnNetworkChangeToUmtsOnSsacBarred)
{
    // GIVEN
    m_pAosHandleMtc->SetNetSrvIn(IMS_TRUE);
    m_pAosHandleMtc->SetHandleState(AosHandle::STATE_CONNECTED);
    m_pAosHandleMtc->SetDataConnected(IMS_TRUE);
    m_pAosHandleMtc->SetNetworkType(NW_REPORT_RADIO_LTE);
    m_pAosHandleMtc->SetSsacBarred(IMS_TRUE);

    ON_CALL(m_objMockIAosNConfiguration, IsRequiredVolteBlockBySsac())
            .WillByDefault(Return(IMS_TRUE));
    ON_CALL(m_objMockIAosNConfiguration, GetVolteHysTime()).WillByDefault(Return(60));
    ON_CALL(m_objMockIAosConnection, GetState())
            .WillByDefault(Return(IAosConnection::STATE_ACTIVE));
    ON_CALL(m_objMockIAosNetTracker, GetNetworkType()).WillByDefault(Return(NW_REPORT_RADIO_WCDMA));
    ON_CALL(m_objMockIAosNetTracker, IsSuspended()).WillByDefault(Return(IMS_FALSE));

    m_pAosHandleMtc->NetTracker_StatusChanged();  // Moves to unsupported RAT(=WCDMA)

    ON_CALL(m_objMockIAosConnection, GetState()).WillByDefault(Return(IAosConnection::STATE_IDLE));

    // WHEN
    m_pAosHandleMtc->NetTracker_StatusChanged();  // Data changed

    // THEN
    EXPECT_EQ(m_pAosHandleMtc->GetStoredNetworkType(), NW_REPORT_RADIO_WCDMA);
    EXPECT_FALSE(m_pAosHandleMtc->GetDataConnected());
    EXPECT_TRUE(m_pAosHandleMtc->IsVolteHysTimerBlockedForSsac());
}

TEST_F(AosHandleMtcTest,
        ShouldNotBlockVolteHysTimerForSsacIfPdnLostOnNetworkChangeToUmtsOnSsacNotBarred)
{
    // GIVEN
    m_pAosHandleMtc->SetNetSrvIn(IMS_TRUE);
    m_pAosHandleMtc->SetHandleState(AosHandle::STATE_CONNECTED);
    m_pAosHandleMtc->SetDataConnected(IMS_TRUE);
    m_pAosHandleMtc->SetNetworkType(NW_REPORT_RADIO_LTE);
    m_pAosHandleMtc->SetSsacBarred(IMS_FALSE);

    ON_CALL(m_objMockIAosNConfiguration, IsRequiredVolteBlockBySsac())
            .WillByDefault(Return(IMS_TRUE));
    ON_CALL(m_objMockIAosNConfiguration, GetVolteHysTime()).WillByDefault(Return(60));
    ON_CALL(m_objMockIAosConnection, GetState())
            .WillByDefault(Return(IAosConnection::STATE_ACTIVE));
    ON_CALL(m_objMockIAosNetTracker, GetNetworkType()).WillByDefault(Return(NW_REPORT_RADIO_WCDMA));
    ON_CALL(m_objMockIAosNetTracker, IsSuspended()).WillByDefault(Return(IMS_FALSE));

    m_pAosHandleMtc->NetTracker_StatusChanged();  // Moves to unsupported RAT(=WCDMA)

    ON_CALL(m_objMockIAosConnection, GetState()).WillByDefault(Return(IAosConnection::STATE_IDLE));

    // WHEN
    m_pAosHandleMtc->NetTracker_StatusChanged();  // Data changed

    // THEN
    EXPECT_EQ(m_pAosHandleMtc->GetStoredNetworkType(), NW_REPORT_RADIO_WCDMA);
    EXPECT_FALSE(m_pAosHandleMtc->GetDataConnected());
    EXPECT_FALSE(m_pAosHandleMtc->IsVolteHysTimerBlockedForSsac());
}

TEST_F(AosHandleMtcTest, ShouldUnblockVolteHysTimerForSsacIfSsacStillBarredOnNetworkChange)
{
    // GIVEN
    SsacInfo objSsacInfoBarred;
    objSsacInfoBarred.nBarringFactorForVoice = 0;
    objSsacInfoBarred.nBarringFactorForVideo = 100;
    objSsacInfoBarred.nBarringTimeSecForVoice = 60;
    objSsacInfoBarred.nBarringTimeSecForVideo = 0;

    m_pAosHandleMtc->SetSsacBarred(IMS_TRUE);
    m_pAosHandleMtc->SetSsacPlmn(AString("123456"));
    m_pAosHandleMtc->SetVolteHysTimerBlockedForSsac();
    m_pAosHandleMtc->SetDataConnected(IMS_TRUE);
    m_pAosHandleMtc->SetNetSrvIn(IMS_TRUE);
    m_pAosHandleMtc->SetNetworkType(NW_REPORT_RADIO_WCDMA);
    m_pAosHandleMtc->Init();

    ON_CALL(m_objMockIAosNConfiguration, IsRequiredVolteBlockBySsac())
            .WillByDefault(Return(IMS_TRUE));
    ON_CALL(m_objTestImsRadioService.GetMockImsRadio(), GetSsacInfo())
            .WillByDefault(ReturnRef(objSsacInfoBarred));
    ON_CALL(m_objMockIAosNConfiguration, GetVolteHysTime()).WillByDefault(Return(60));
    ON_CALL(m_objMockIAosConnection, GetState())
            .WillByDefault(Return(IAosConnection::STATE_ACTIVE));
    ON_CALL(m_objMockIAosNetTracker, GetNetworkType()).WillByDefault(Return(NW_REPORT_RADIO_LTE));
    ON_CALL(m_objMockIAosNetTracker, IsSuspended()).WillByDefault(Return(IMS_FALSE));
    ON_CALL(m_objMockIAosNetTracker, GetNetworkOperator()).WillByDefault(Return(AString("123456")));

    // WHEN
    m_pAosHandleMtc->NetTracker_StatusChanged();  // Network changed

    // THEN
    EXPECT_FALSE(m_pAosHandleMtc->IsVolteHysTimerBlockedForSsac());
}
