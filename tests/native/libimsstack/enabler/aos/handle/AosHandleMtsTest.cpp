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

#include "CarrierConfig.h"
#include "INetworkWatcher.h"

#include "handle/AosHandle.h"
#include "handle/AosHandleMts.h"
#include "interface/IAosAppContext.h"
#include "interface/IAosNConfiguration.h"
#include "interface/IAosConnection.h"
#include "provider/AosProvider.h"
#include "provider/AosUtil.h"

#include "interface/MockIAosAppContext.h"
#include "interface/MockIAosApplication.h"
#include "interface/MockIAosNConfiguration.h"
#include "interface/MockIAosNetTracker.h"
#include "interface/MockIAosConnection.h"

using ::testing::_;
using ::testing::AnyNumber;
using ::testing::Return;
using ::testing::ReturnRef;

#define DECLARE_USING(Base)       \
    using Base::IsFeatureBlocked; \
    using Base::Request;

class TestAosHandleMts : public AosHandleMts
{
public:
    DECLARE_USING(AosHandleMts)

    inline TestAosHandleMts(IN IAosAppContext* piAppContext, IN const AString& strAppId,
            IN const AString& strServiceId, IN const IMS_SINT32 nServiceType) :
            AosHandleMts(piAppContext, strAppId, strServiceId, nServiceType)
    {
    }
};

class AosHandleMtsTest : public ::testing::Test
{
public:
    TestAosHandleMts* m_pAosHandleMts;

    MockIAosAppContext m_objMockIAosAppContext;
    MockIAosApplication m_objMockIAosApplication;
    MockIAosNetTracker m_objMockIAosNetTracker;
    MockIAosConnection m_objMockIAosConnection;

    IAosNConfiguration* m_piAosNConfiguration;
    MockIAosNConfiguration m_objMockIAosNConfiguration;

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

        ON_CALL(m_objMockIAosAppContext, GetConnection())
                .WillByDefault(Return(&m_objMockIAosConnection));
        ON_CALL(m_objMockIAosConnection, IsEpdgEnabled()).WillByDefault(Return(IMS_FALSE));
        ON_CALL(m_objMockIAosConnection, GetState())
                .WillByDefault(Return(IAosConnection::STATE_ACTIVE));

        m_piAosNConfiguration = AosProvider::GetInstance()->GetNConfiguration();
        AosProvider::GetInstance()->SetNConfiguration(
                static_cast<IAosNConfiguration*>(&m_objMockIAosNConfiguration));

        EXPECT_CALL(m_objMockIAosNConfiguration, SetListener(_)).Times(1);

        const AString strAppId = AString("ims.app.mts.test");
        const AString strServiceId = AString("ims.service.mts.test");
        const IMS_UINT32 nServiceType = -1;
        m_pAosHandleMts = new TestAosHandleMts(
                &m_objMockIAosAppContext, strAppId, strServiceId, nServiceType);

        ASSERT_TRUE(m_pAosHandleMts != nullptr);

        m_pAosHandleMts->m_bDataConnected = IMS_TRUE;
    }

    virtual void TearDown() override
    {
        if (m_pAosHandleMts != nullptr)
        {
            delete m_pAosHandleMts;
            m_pAosHandleMts = nullptr;
        }

        AosProvider::GetInstance()->SetNConfiguration(m_piAosNConfiguration);
    }

    ImsMap<IMS_UINT32, IMS_UINT32> GetCapabilities() { return m_pAosHandleMts->m_objCapabilities; }

    IMS_UINT32 GetSupportedRats() { return m_pAosHandleMts->m_nSupportedRats; }

    void InitializeSupportedRats() { m_pAosHandleMts->InitializeSupportedRats(); }

    void Init() { m_pAosHandleMts->Init(); }

    void SetBlocked(IN IMS_BOOL bBlocked) { m_pAosHandleMts->m_bBlocked = bBlocked; }

    IMS_BOOL IsBlocked() { return m_pAosHandleMts->AosHandle::IsBlocked(); }

    IImsAosInfo* GetAosInfo() { return m_pAosHandleMts->m_piInfo; }

    void InitializeServiceBlock() { m_pAosHandleMts->InitializeServiceBlock(); }

    IMS_BOOL IsHandleBlocked(IN IMS_UINT32 nBlock)
    {
        return m_pAosHandleMts->AosHandle::IsHandleBlocked(nBlock);
    }

    IMS_BOOL IsHandleBlocked() { return m_pAosHandleMts->IsHandleBlocked(); }

    void ProcessCapabilitiesChanged(IN const ImsMap<IMS_UINT32, IMS_UINT32>& objCapabilities)
    {
        m_pAosHandleMts->ProcessCapabilitiesChanged(objCapabilities);
    }

    void InitializeServiceFeature() { m_pAosHandleMts->InitializeServiceFeature(); }

    void AddBlock(IN IMS_UINT32 nBlock)
    {
        m_pAosHandleMts->AddBlock(nBlock, m_pAosHandleMts->m_nBlocks);
    }

    void ClearBlocks() { m_pAosHandleMts->m_nBlocks = AosHandle::BLOCK_NONE; }

    void SetMtcBlocked(IN IMS_BOOL bBlocked) { m_pAosHandleMts->m_bMtcBlocked = bBlocked; }

    IMS_BOOL IsSupportedNetworkTypeForCellular(IN IMS_UINT32 nType) const
    {
        return m_pAosHandleMts->IsSupportedNetworkTypeForCellular(nType);
    }

    void Handle_Notify(IN IMS_UINT32 nType, IN IMS_BOOL bBlocked)
    {
        m_pAosHandleMts->Handle_Notify(nType, bBlocked);
    }

    void SetNetworkType(IN IMS_UINT32 nNetworkType)
    {
        m_pAosHandleMts->m_nNetworkType = nNetworkType;
    }

    void SetCapabilities(IN IMS_UINT32 nCapaNetworkType, IN IMS_UINT32 nCapabilities)
    {
        m_pAosHandleMts->m_objCapabilities.SetValue(nCapaNetworkType, nCapabilities);
    }

    void NetTracker_StatusChanged() { m_pAosHandleMts->NetTracker_StatusChanged(); }

    inline void SetServiceType(IN IMS_UINT32 nServiceType)
    {
        m_pAosHandleMts->m_nServiceType = nServiceType;
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

TEST_F(AosHandleMtsTest, Constructor_Test)
{
    ASSERT_FALSE(GetCapabilities().GetIndexOfKey(static_cast<IMS_UINT32>(AosNetworkType::LTE)) < 0);
    ASSERT_FALSE(
            GetCapabilities().GetIndexOfKey(static_cast<IMS_UINT32>(AosNetworkType::IWLAN)) < 0);
    ASSERT_FALSE(GetCapabilities().GetIndexOfKey(static_cast<IMS_UINT32>(AosNetworkType::NR)) < 0);

    EXPECT_TRUE((GetCapabilities().GetValue(static_cast<IMS_UINT32>(AosNetworkType::LTE)) &
                        static_cast<IMS_UINT32>(AosCapability::SMS)) > 0);
    EXPECT_TRUE((GetCapabilities().GetValue(static_cast<IMS_UINT32>(AosNetworkType::IWLAN)) &
                        static_cast<IMS_UINT32>(AosCapability::SMS)) > 0);
    EXPECT_TRUE((GetCapabilities().GetValue(static_cast<IMS_UINT32>(AosNetworkType::NR)) &
                        static_cast<IMS_UINT32>(AosCapability::SMS)) > 0);
}

TEST_F(AosHandleMtsTest, NConfiguration_NotifyConfigChanged_Test)
{
    // Test: no supported rat
    // Expectation: Supported rat = 0

    ImsVector<IMS_SINT32> objTestRats;
    EXPECT_CALL(m_objMockIAosNConfiguration, GetSmsOverImsSupportedRats())
            .Times(1)
            .WillOnce(ReturnRef(objTestRats));

    m_pAosHandleMts->NConfiguration_NotifyConfigChanged();

    EXPECT_EQ(GetSupportedRats(), NW_REPORT_RADIO_INVALID);
}

TEST_F(AosHandleMtsTest, InitializeSupportedRats_Test)
{
    // Test: supported rats = EUTRAN, UTRAN, GERAN, NGRAN
    // Expectation: Supported rat = LTE, WCDMA, HSPA, GSM, EDGE, NR

    ImsVector<IMS_SINT32> objTestRats;
    objTestRats.Add(CarrierConfig::Ims::ACCESS_NETWORK_TYPE_EUTRAN);
    objTestRats.Add(CarrierConfig::Ims::ACCESS_NETWORK_TYPE_UTRAN);
    objTestRats.Add(CarrierConfig::Ims::ACCESS_NETWORK_TYPE_GERAN);
    objTestRats.Add(CarrierConfig::Ims::ACCESS_NETWORK_TYPE_NGRAN);

    EXPECT_CALL(m_objMockIAosNConfiguration, GetSmsOverImsSupportedRats())
            .Times(1)
            .WillOnce(ReturnRef(objTestRats));

    InitializeSupportedRats();

    EXPECT_EQ(GetSupportedRats(),
            (NW_REPORT_RADIO_LTE | NW_REPORT_RADIO_WCDMA | NW_REPORT_RADIO_HSPA |
                    NW_REPORT_RADIO_GSM | NW_REPORT_RADIO_EDGE | NW_REPORT_RADIO_NR));
}

TEST_F(AosHandleMtsTest, Init_Test)
{
    ImsVector<IMS_SINT32> objTestRats;
    objTestRats.Add(CarrierConfig::Ims::ACCESS_NETWORK_TYPE_EUTRAN);
    objTestRats.Add(CarrierConfig::Ims::ACCESS_NETWORK_TYPE_NGRAN);

    EXPECT_CALL(m_objMockIAosNConfiguration, GetSmsOverImsSupportedRats())
            .Times(1)
            .WillOnce(ReturnRef(objTestRats));

    EXPECT_CALL(m_objMockIAosNConfiguration, IsCdmalessFeatureTagRequired())
            .Times(1)
            .WillOnce(Return(IMS_TRUE));

    EXPECT_CALL(m_objMockIAosApplication, Reconfig()).Times(1);

    Init();

    EXPECT_EQ(GetSupportedRats(), (NW_REPORT_RADIO_LTE | NW_REPORT_RADIO_NR));
    EXPECT_FALSE(IsBlocked());
    EXPECT_TRUE(m_pAosHandleMts->GetFeatureTagList().HasFeature(ImsAosFeature::SMSIP));
}

TEST_F(AosHandleMtsTest, ShouldNotBlockIfSmsCapabilityIsNotBlockedWhenInitializeServiceBlock)
{
    // WHEN
    InitializeServiceBlock();

    // THEN
    EXPECT_FALSE(IsBlocked());
}

TEST_F(AosHandleMtsTest, ShouldBlockIfSmsCapabilityIsBlockedWhenInitializeServiceBlock)
{
    // GIVEN
    AddBlock(AosHandle::BLOCK_SMS_CAPABILITY);

    // WHEN
    InitializeServiceBlock();

    // THEN
    EXPECT_TRUE(IsBlocked());
}

TEST_F(AosHandleMtsTest, FeatureIsAddedIfHandleIsNotBlockedWhenInitializeServiceFeature)
{
    // WHEN
    InitializeServiceFeature();

    // THEN
    EXPECT_TRUE(m_pAosHandleMts->GetFeatureTagList().HasFeature(ImsAosFeature::SMSIP));
}

TEST_F(AosHandleMtsTest, FeatureIsNotAddedIfHandleIsBlockedWhenInitializeServiceFeature)
{
    // GIVEN
    SetBlocked(IMS_TRUE);

    // WHEN
    InitializeServiceFeature();

    // THEN
    EXPECT_FALSE(m_pAosHandleMts->GetFeatureTagList().HasFeature(ImsAosFeature::SMSIP));
}

TEST_F(AosHandleMtsTest, ShouldSetBlockIfSmsCapabilityIsRemovedWhenRatIs3G)
{
    // GIVEN
    ImsMap<IMS_UINT32, IMS_UINT32> objNewCapabilities;
    SetNetworkType(NW_REPORT_RADIO_WCDMA);

    // WHEN
    ProcessCapabilitiesChanged(objNewCapabilities);

    // THEN
    EXPECT_TRUE(IsHandleBlocked());
}

TEST_F(AosHandleMtsTest, ShouldResetBlockIfSmsCapabilityIsAddedWhenRatIsLte)
{
    // GIVEN
    ImsMap<IMS_UINT32, IMS_UINT32> objNewCapabilities;
    SetNetworkType(NW_REPORT_RADIO_LTE);
    ProcessCapabilitiesChanged(objNewCapabilities);
    EXPECT_TRUE(IsHandleBlocked());
    objNewCapabilities.Add(static_cast<IMS_UINT32>(AosNetworkType::LTE),
            static_cast<IMS_UINT32>(AosCapability::SMS));

    // WHEN
    ProcessCapabilitiesChanged(objNewCapabilities);

    // THEN
    EXPECT_FALSE(IsHandleBlocked());
}

TEST_F(AosHandleMtsTest, DoNothingIfEmergencyServiceWhenCapabilityChanged)
{
    // GIVEN
    SetServiceType(ImsAosService::EMERGENCY_MTS);

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
            static_cast<IMS_UINT32>(AosCapability::SMS));
    objExpectedCapabilities.Add(static_cast<IMS_UINT32>(AosNetworkType::IWLAN),
            static_cast<IMS_UINT32>(AosCapability::SMS));
    objExpectedCapabilities.Add(static_cast<IMS_UINT32>(AosNetworkType::NR),
            static_cast<IMS_UINT32>(AosCapability::SMS));
    objExpectedCapabilities.Add(static_cast<IMS_UINT32>(AosNetworkType::UTRAN),
            static_cast<IMS_UINT32>(AosCapability::SMS));

    // WHEN
    ProcessCapabilitiesChanged(objNewCapabilities);

    // THEN
    EXPECT_TRUE(IsEqualCapabilities(GetCapabilities(), objExpectedCapabilities));
}

TEST_F(AosHandleMtsTest, DoNothingIfSmsCapabilityIsNotChanged)
{
    // GIVEN
    ImsMap<IMS_UINT32, IMS_UINT32> objNewCapabilities;
    objNewCapabilities.Add(static_cast<IMS_UINT32>(AosNetworkType::LTE),
            static_cast<IMS_UINT32>(AosCapability::SMS));
    SetNetworkType(NW_REPORT_RADIO_LTE);

    // WHEN
    ProcessCapabilitiesChanged(objNewCapabilities);

    // THEN
    EXPECT_FALSE(IsHandleBlocked());
}

TEST_F(AosHandleMtsTest, DoNothingIfCurrentRatIsNotSupported)
{
    // GIVEN
    ImsMap<IMS_UINT32, IMS_UINT32> objNewCapabilities;
    SetNetworkType(NW_REPORT_RADIO_GSM);

    // WHEN
    ProcessCapabilitiesChanged(objNewCapabilities);

    // THEN
    EXPECT_FALSE(IsHandleBlocked());
}

TEST_F(AosHandleMtsTest, ShouldSetBlockIfRatIsChangedToTheOneHasNoCapability)
{
    // GIVEN
    SetCapabilities(static_cast<IMS_UINT32>(AosNetworkType::UTRAN),
            static_cast<IMS_UINT32>(AosCapability::NONE));
    SetNetworkType(NW_REPORT_RADIO_LTE);
    ON_CALL(m_objMockIAosNetTracker, GetNetworkType()).WillByDefault(Return(NW_REPORT_RADIO_WCDMA));

    // WHEN
    NetTracker_StatusChanged();

    // THEN
    EXPECT_TRUE(IsHandleBlocked());
}

TEST_F(AosHandleMtsTest, ShouldResetBlockIfRatIsChangedToTheOneHasCapability)
{
    // GIVEN
    AddBlock(AosHandle::BLOCK_SMS_CAPABILITY);
    SetCapabilities(static_cast<IMS_UINT32>(AosNetworkType::UTRAN),
            static_cast<IMS_UINT32>(AosCapability::NONE));
    SetNetworkType(NW_REPORT_RADIO_WCDMA);
    ON_CALL(m_objMockIAosNetTracker, GetNetworkType()).WillByDefault(Return(NW_REPORT_RADIO_LTE));

    // WHEN
    NetTracker_StatusChanged();

    // THEN
    EXPECT_FALSE(IsHandleBlocked());
}

TEST_F(AosHandleMtsTest, DoNothingWhenNetworkIsChangedIfWifiTestMode)
{
    // GIVEN
    IMS_BOOL bIsWifiTest = AosUtil::GetInstance()->IsWifiTest();
    AosUtil::GetInstance()->SetWifiTest(IMS_TRUE);

    EXPECT_CALL(m_objMockIAosNetTracker, GetNetworkType()).Times(0);

    // WHEN
    NetTracker_StatusChanged();

    // THEN: The GIVEN condition should be met.

    // Clean Up
    AosUtil::GetInstance()->SetWifiTest(bIsWifiTest);
}

TEST_F(AosHandleMtsTest, HandleIsBlockedIfSmsCapabilityIsBlocked)
{
    // GIVEN
    AddBlock(AosHandle::BLOCK_SMS_CAPABILITY);

    // WHEN & THEN
    EXPECT_TRUE(IsHandleBlocked());
}

TEST_F(AosHandleMtsTest, HandleIsBlockedIfMtcHandleIsBlocked)
{
    // GIVEN
    SetMtcBlocked(IMS_TRUE);

    // WHEN & THEN
    EXPECT_TRUE(IsHandleBlocked());
}

TEST_F(AosHandleMtsTest, HandleIsNotBlockedIfSmsCapabilityAndMtcHandleAreNotBlocked)
{
    // WHEN & THEN
    EXPECT_FALSE(IsHandleBlocked());
}

TEST_F(AosHandleMtsTest, SmsIsBlockedIfSmsCapabilityBlocked)
{
    // GIVEN
    AddBlock(AosHandle::BLOCK_SMS_CAPABILITY);

    // WHEN & THEN
    EXPECT_TRUE(m_pAosHandleMts->IsFeatureBlocked(ImsAosFeature::SMSIP));
}

TEST_F(AosHandleMtsTest, SmsIsBlockedIfLimitedSmsBlocked)
{
    // GIVEN
    AddBlock(AosHandle::BLOCK_LIMITED_SMS);

    // WHEN & THEN
    EXPECT_TRUE(m_pAosHandleMts->IsFeatureBlocked(ImsAosFeature::SMSIP));
}

TEST_F(AosHandleMtsTest, ShouldNotBlockNonMtsFeatures)
{
    // WHEN & THEN
    EXPECT_FALSE(m_pAosHandleMts->IsFeatureBlocked(ImsAosFeature::TEXT));
}

TEST_F(AosHandleMtsTest, IsSupportedNetworkTypeForCellular_Test)
{
    ImsVector<IMS_SINT32> objRats;
    objRats.Add(CarrierConfig::Ims::ACCESS_NETWORK_TYPE_EUTRAN);
    objRats.Add(CarrierConfig::Ims::ACCESS_NETWORK_TYPE_UTRAN);
    objRats.Add(CarrierConfig::Ims::ACCESS_NETWORK_TYPE_GERAN);
    objRats.Add(CarrierConfig::Ims::ACCESS_NETWORK_TYPE_NGRAN);
    objRats.Add(CarrierConfig::Ims::ACCESS_NETWORK_TYPE_IWLAN);

    EXPECT_CALL(m_objMockIAosNConfiguration, GetSmsOverImsSupportedRats())
            .Times(1)
            .WillOnce(ReturnRef(objRats));

    InitializeSupportedRats();

    EXPECT_FALSE(IsSupportedNetworkTypeForCellular(NW_REPORT_RADIO_AMPS));
    EXPECT_FALSE(IsSupportedNetworkTypeForCellular(NW_REPORT_RADIO_CDMA));
    EXPECT_TRUE(IsSupportedNetworkTypeForCellular(NW_REPORT_RADIO_EDGE));
    EXPECT_FALSE(IsSupportedNetworkTypeForCellular(NW_REPORT_RADIO_AMPS));
    EXPECT_FALSE(IsSupportedNetworkTypeForCellular(NW_REPORT_RADIO_EHRPD));
    EXPECT_FALSE(IsSupportedNetworkTypeForCellular(NW_REPORT_RADIO_EVDODO));
    EXPECT_FALSE(IsSupportedNetworkTypeForCellular(NW_REPORT_RADIO_GPS));
    EXPECT_TRUE(IsSupportedNetworkTypeForCellular(NW_REPORT_RADIO_GSM));
    EXPECT_FALSE(IsSupportedNetworkTypeForCellular(NW_REPORT_RADIO_HDR));
    EXPECT_TRUE(IsSupportedNetworkTypeForCellular(NW_REPORT_RADIO_HSPA));
    EXPECT_TRUE(IsSupportedNetworkTypeForCellular(NW_REPORT_RADIO_LTE));
    EXPECT_TRUE(IsSupportedNetworkTypeForCellular(NW_REPORT_RADIO_NR));
    EXPECT_TRUE(IsSupportedNetworkTypeForCellular(NW_REPORT_RADIO_WCDMA));
    EXPECT_TRUE(IsSupportedNetworkTypeForCellular(NW_REPORT_RADIO_WLAN));
}

TEST_F(AosHandleMtsTest, Handle_Notify_Test1)
{
    // Test1: type = non Mtc
    // Expectation: do nothing

    EXPECT_FALSE(IsBlocked());

    Handle_Notify(ImsAosService::SIP_CONTROLLER, IMS_FALSE);
    EXPECT_FALSE(IsBlocked());

    Handle_Notify(ImsAosService::SIP_CONTROLLER, IMS_TRUE);
    EXPECT_FALSE(IsBlocked());
}

TEST_F(AosHandleMtsTest, Handle_Notify_Test2)
{
    // Test2: type = Mtc, same block status
    // Expectation: do nothing

    EXPECT_FALSE(IsBlocked());

    Handle_Notify(ImsAosService::MTC, IMS_FALSE);
    EXPECT_FALSE(IsBlocked());

    SetBlocked(IMS_TRUE);
    Handle_Notify(ImsAosService::MTC, IMS_TRUE);
    EXPECT_TRUE(IsBlocked());
}

TEST_F(AosHandleMtsTest, Handle_Notify_Test3)
{
    // Test3: type = Mtc, mts not blocked, mtc blocked then unblocked
    // Expectation: mts blocked if mtc blocked

    Handle_Notify(ImsAosService::MTC, IMS_TRUE);
    EXPECT_TRUE(IsBlocked());

    Handle_Notify(ImsAosService::MTC, IMS_FALSE);
    EXPECT_FALSE(IsBlocked());
}

TEST_F(AosHandleMtsTest,
        ShouldBlockSmsFeatureIfRegModeChangedToLimitedWhileSmsIsUnavailableInLimitedReg)
{
    // GIVEN
    ImsVector<IMS_SINT32> objUnavailableFeatures;
    objUnavailableFeatures.Add(CarrierConfig::Ims::REG_FEATURE_SMS);
    ON_CALL(m_objMockIAosNConfiguration, GetUnavailableFeaturesInLimitedReg())
            .WillByDefault(ReturnRef(objUnavailableFeatures));

    // WHEN
    m_pAosHandleMts->Request(IAosHandle::TYPE_LIMITED_MODE, IAosHandle::STATE_ADD);

    // THEN
    EXPECT_TRUE(IsHandleBlocked(AosHandle::BLOCK_LIMITED_SMS));
}

TEST_F(AosHandleMtsTest,
        ShouldUnblockSmsFeatureIfRegModeChangedToNormalWhileSmsIsUnavailableInLimitedReg)
{
    // GIVEN
    ImsVector<IMS_SINT32> objUnavailableFeatures;
    objUnavailableFeatures.Add(CarrierConfig::Ims::REG_FEATURE_SMS);
    ON_CALL(m_objMockIAosNConfiguration, GetUnavailableFeaturesInLimitedReg())
            .WillByDefault(ReturnRef(objUnavailableFeatures));
    AddBlock(AosHandle::BLOCK_LIMITED_SMS);

    // WHEN
    m_pAosHandleMts->Request(IAosHandle::TYPE_LIMITED_MODE, IAosHandle::STATE_REMOVE);

    // THEN
    EXPECT_FALSE(IsHandleBlocked(AosHandle::BLOCK_LIMITED_SMS));
}
