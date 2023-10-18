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
#include "provider/AosProvider.h"

#include "interface/MockIAosAppContext.h"
#include "interface/MockIAosApplication.h"
#include "interface/MockIAosNConfiguration.h"
#include "interface/MockIAosNetTracker.h"

using ::testing::_;
using ::testing::AnyNumber;
using ::testing::Return;
using ::testing::ReturnRef;

class AosHandleMtsTest : public ::testing::Test
{
public:
    AosHandleMts* m_pAosHandleMts;

    MockIAosAppContext m_objMockIAosAppContext;
    MockIAosApplication m_objMockIAosApplication;
    MockIAosNetTracker m_objMockIAosNetTracker;

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

        m_piAosNConfiguration = AosProvider::GetInstance()->GetNConfiguration();
        AosProvider::GetInstance()->SetNConfiguration(
                static_cast<IAosNConfiguration*>(&m_objMockIAosNConfiguration));

        EXPECT_CALL(m_objMockIAosNConfiguration, SetListener(_)).Times(1);

        const AString strAppId = AString("ims.app.mts.test");
        const AString strServiceId = AString("ims.service.mts.test");
        const IMS_UINT32 nServiceType = -1;
        m_pAosHandleMts = new AosHandleMts(static_cast<IAosAppContext*>(&m_objMockIAosAppContext),
                strAppId, strServiceId, nServiceType);

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

    EXPECT_CALL(m_objMockIAosNConfiguration, IsSmsOverImsSupported())
            .Times(1)
            .WillOnce(Return(IMS_TRUE));

    EXPECT_CALL(m_objMockIAosNConfiguration, IsSmsOverIpEnabled())
            .Times(1)
            .WillOnce(Return(IMS_TRUE));

    EXPECT_CALL(m_objMockIAosNConfiguration, IsCdmalessFeatureTagRequired())
            .Times(1)
            .WillOnce(Return(IMS_TRUE));

    EXPECT_CALL(m_objMockIAosApplication, Reconfig()).Times(1);

    Init();

    EXPECT_EQ(GetSupportedRats(), (NW_REPORT_RADIO_LTE | NW_REPORT_RADIO_NR));
    EXPECT_FALSE(IsBlocked());
    EXPECT_TRUE(m_pAosHandleMts->GetFeatureTagList().HasFeature(ImsAosFeature::SMSIP));
}

TEST_F(AosHandleMtsTest, InitializeServiceBlock_Test)
{
    // Expectation: Block sms over ims if SmsOverImsSupported or SmsOverIpEnabled is false

    EXPECT_CALL(m_objMockIAosNConfiguration, IsSmsOverImsSupported())
            .Times(4)
            .WillOnce(Return(IMS_FALSE))
            .WillOnce(Return(IMS_FALSE))
            .WillOnce(Return(IMS_TRUE))
            .WillOnce(Return(IMS_TRUE));

    EXPECT_CALL(m_objMockIAosNConfiguration, IsSmsOverIpEnabled())
            .Times(4)
            .WillOnce(Return(IMS_FALSE))
            .WillOnce(Return(IMS_TRUE))
            .WillOnce(Return(IMS_FALSE))
            .WillOnce(Return(IMS_TRUE));

    InitializeServiceBlock();
    EXPECT_TRUE(IsHandleBlocked(AosHandle::BLOCK_SMS_OVER_IP_NETWORK_INDICATION));
    EXPECT_TRUE(IsBlocked());

    InitializeServiceBlock();
    EXPECT_TRUE(IsHandleBlocked(AosHandle::BLOCK_SMS_OVER_IP_NETWORK_INDICATION));
    EXPECT_TRUE(IsBlocked());

    InitializeServiceBlock();
    EXPECT_TRUE(IsHandleBlocked(AosHandle::BLOCK_SMS_OVER_IP_NETWORK_INDICATION));
    EXPECT_TRUE(IsBlocked());

    InitializeServiceBlock();
    EXPECT_FALSE(IsHandleBlocked(AosHandle::BLOCK_SMS_OVER_IP_NETWORK_INDICATION));
    EXPECT_FALSE(IsBlocked());
}

TEST_F(AosHandleMtsTest, InitializeServiceFeature_Test)
{
    // Expectation: SMSIP feature is existed only if not blocked

    InitializeServiceFeature();
    EXPECT_TRUE(m_pAosHandleMts->GetFeatureTagList().HasFeature(ImsAosFeature::SMSIP));

    SetBlocked(IMS_TRUE);
    InitializeServiceFeature();
    EXPECT_FALSE(m_pAosHandleMts->GetFeatureTagList().HasFeature(ImsAosFeature::SMSIP));
}

TEST_F(AosHandleMtsTest, ProcessCapabilitiesChanged_Test)
{
    ImsMap<IMS_UINT32, IMS_UINT32> objNewCapabilities;
    ProcessCapabilitiesChanged(objNewCapabilities);
}

TEST_F(AosHandleMtsTest, IsHandleBlocked_Test)
{
    // Expectation: return false if sms_capa or sms_over_ip is blocked or mtc blocked.
    //              otherwise return true

    AddBlock(AosHandle::BLOCK_SMS_CAPABILITY);
    EXPECT_TRUE(IsHandleBlocked());

    ClearBlocks();
    AddBlock(AosHandle::BLOCK_SMS_OVER_IP_NETWORK_INDICATION);
    EXPECT_TRUE(IsHandleBlocked());

    ClearBlocks();
    SetMtcBlocked(IMS_TRUE);
    EXPECT_TRUE(IsHandleBlocked());

    ClearBlocks();
    SetMtcBlocked(IMS_FALSE);
    EXPECT_FALSE(IsHandleBlocked());
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

TEST_F(AosHandleMtsTest, Handle_Notify_Test4)
{
    // Test4: type = Mtc, mts not blocked, mtc blocked.
    //        Then mts blocked by sms_over_ip_indication and mtc unblocked
    // Expectation: mts blocked when mtc is blocked and mts keep block even if mtc unblocked.

    Handle_Notify(ImsAosService::MTC, IMS_TRUE);
    EXPECT_TRUE(IsBlocked());

    AddBlock(AosHandle::BLOCK_SMS_OVER_IP_NETWORK_INDICATION);

    Handle_Notify(ImsAosService::MTC, IMS_FALSE);
    EXPECT_TRUE(IsBlocked());
}