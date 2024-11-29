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

#include "ISipConfig.h"
#include "MockICarrierConfig.h"
#include "PlatformContext.h"
#include "TestConfigService.h"
#include "private/SipConfig.h"

using ::testing::_;
using ::testing::AnyNumber;
using ::testing::Return;

namespace android
{
static const IMS_SINT32 TEST_CARRIER_CONFIG_INT = 1000;

class SipConfigTest : public ::testing::Test
{
public:
    SipConfigTest();

protected:
    virtual void SetUp() override
    {
        m_pConfigService = new TestConfigService();

        m_pConfigService->SetCarrierConfig(&m_objMockICarrierConfig);

        m_pOldConfigService = PlatformContext::GetInstance()->SetService(
                PlatformContext::SERVICE_CONFIG, m_pConfigService);

        ON_CALL(m_pConfigService->GetMockCarrierConfig(), AddListener(_)).WillByDefault(Return());
        ON_CALL(m_pConfigService->GetMockCarrierConfig(), RemoveListener(_))
                .WillByDefault(Return());

        EXPECT_CALL(m_objMockICarrierConfig, GetBoolean(_, _))
                .Times(AnyNumber())
                .WillRepeatedly(Return(IMS_TRUE));
        EXPECT_CALL(m_objMockICarrierConfig, GetInt(_, _))
                .Times(AnyNumber())
                .WillRepeatedly(Return(TEST_CARRIER_CONFIG_INT));
    }

    virtual void TearDown() override
    {
        PlatformContext::GetInstance()->SetService(
                PlatformContext::SERVICE_CONFIG, m_pOldConfigService);

        if (m_pConfigService != IMS_NULL)
        {
            delete m_pConfigService;
            m_pConfigService = IMS_NULL;
        }
    }

protected:
    PlatformService* m_pOldConfigService;
    TestConfigService* m_pConfigService;
    MockICarrierConfig m_objMockICarrierConfig;
    AStringArray m_objAllowMethods;
};

SipConfigTest::SipConfigTest() :
        m_pOldConfigService(IMS_NULL),
        m_pConfigService(IMS_NULL)
{
    m_objAllowMethods.AddElement("INVITE");
    m_objAllowMethods.AddElement("BYE");
    m_objAllowMethods.AddElement("CANCEL");
    m_objAllowMethods.AddElement("ACK");
    m_objAllowMethods.AddElement("NOTIFY");
    m_objAllowMethods.AddElement("UPDATE");
    m_objAllowMethods.AddElement("REFER");
    m_objAllowMethods.AddElement("PRACK");
    m_objAllowMethods.AddElement("INFO");
    m_objAllowMethods.AddElement("MESSAGE");
    m_objAllowMethods.AddElement("OPTIONS");
}

TEST_F(SipConfigTest, Refresh)
{
    SipConfig objSipConfig(IMS_SLOT_0);

    objSipConfig.Init();

    const AStringArray& objAllowMethods = objSipConfig.GetRegAllowMethods();
    EXPECT_EQ(m_objAllowMethods.GetElements(), objAllowMethods.GetElements());

    objSipConfig.Refresh();
    const AStringArray& objAllowMethodsForRefresh = objSipConfig.GetRegAllowMethods();
    EXPECT_EQ(m_objAllowMethods.GetElements(), objAllowMethodsForRefresh.GetElements());
}

TEST_F(SipConfigTest, TcpTimerValuesCopyConstructor)
{
    SipConfig::TcpTimerValues objSipConfigTcpTimerValues;

    EXPECT_EQ(objSipConfigTcpTimerValues.m_nTvConnectionWaiting, -1);
    EXPECT_EQ(objSipConfigTcpTimerValues.m_nTvKeepAlive, SipConfig::TcpTimerValues::PERMANENT);
    EXPECT_EQ(objSipConfigTcpTimerValues.m_nTvWouldblockWaiting, -1);

    objSipConfigTcpTimerValues.m_nTvConnectionWaiting = 1;
    objSipConfigTcpTimerValues.m_nTvKeepAlive = 10;
    objSipConfigTcpTimerValues.m_nTvWouldblockWaiting = 0;

    // This is a test to verify the copy constructor.
    // NOLINTNEXTLINE(performance-unnecessary-copy-initialization)
    SipConfig::TcpTimerValues objCopiedSipConfigTcpTimerValues(objSipConfigTcpTimerValues);

    EXPECT_EQ(objSipConfigTcpTimerValues.m_nTvConnectionWaiting, 1);
    EXPECT_EQ(objSipConfigTcpTimerValues.m_nTvKeepAlive, 10);
    EXPECT_EQ(objSipConfigTcpTimerValues.m_nTvWouldblockWaiting, 0);
}

TEST_F(SipConfigTest, TcpTimerValuesAssignmentOperator)
{
    SipConfig::TcpTimerValues objSipConfigTcpTimerValues;

    EXPECT_EQ(objSipConfigTcpTimerValues.m_nTvConnectionWaiting, -1);
    EXPECT_EQ(objSipConfigTcpTimerValues.m_nTvKeepAlive, SipConfig::TcpTimerValues::PERMANENT);
    EXPECT_EQ(objSipConfigTcpTimerValues.m_nTvWouldblockWaiting, -1);

    objSipConfigTcpTimerValues.m_nTvConnectionWaiting = 1;
    objSipConfigTcpTimerValues.m_nTvKeepAlive = 10;
    objSipConfigTcpTimerValues.m_nTvWouldblockWaiting = 0;

    SipConfig::TcpTimerValues objCopiedSipConfigTcpTimerValues;
    objCopiedSipConfigTcpTimerValues = objSipConfigTcpTimerValues;

    EXPECT_EQ(objSipConfigTcpTimerValues.m_nTvConnectionWaiting, 1);
    EXPECT_EQ(objSipConfigTcpTimerValues.m_nTvKeepAlive, 10);
    EXPECT_EQ(objSipConfigTcpTimerValues.m_nTvWouldblockWaiting, 0);
}

TEST_F(SipConfigTest, GetUaVersion)
{
    SipConfig objSipConfig(IMS_SLOT_0);

    EXPECT_EQ(objSipConfig.GetUaVersion(), AString::ConstNull());

    AString strValue("UaVersion");

    EXPECT_CALL(
            m_objMockICarrierConfig, GetString(CarrierConfig::Ims::KEY_IMS_USER_AGENT_STRING, _))
            .Times(1)
            .WillOnce(Return(strValue));

    objSipConfig.Init();

    EXPECT_EQ(objSipConfig.GetUaVersion(), strValue);
}

TEST_F(SipConfigTest, GetTimerValue)
{
    SipConfig objSipConfig(IMS_SLOT_0);

    // default T1 value
    EXPECT_EQ(objSipConfig.GetTimerValueT1(), SipConfigV::DEFAULT_TIMER_T1);
    // default T2 value
    EXPECT_EQ(objSipConfig.GetTimerValueT2(), SipConfigV::DEFAULT_TIMER_T2);

    objSipConfig.Init();

    // T1 value
    EXPECT_EQ(objSipConfig.GetTimerValueT1(), TEST_CARRIER_CONFIG_INT);
    // T2 value
    EXPECT_EQ(objSipConfig.GetTimerValueT2(), TEST_CARRIER_CONFIG_INT);
}

TEST_F(SipConfigTest, Update)
{
    SipConfig objSipConfig(IMS_SLOT_0);
    IConfigurable* piConfigurable =
            static_cast<const ISipConfig*>(&objSipConfig)->GetConfigurable();

    EXPECT_TRUE(piConfigurable->Update(IConfigurable::CP_I_START_SIP));
    EXPECT_TRUE(piConfigurable->Update(IConfigurable::CP_I_END_SIP));

    IMS_SINT32 nIntValue = 10;
    AString strIntValue;

    strIntValue.SetNumber(nIntValue);

    EXPECT_TRUE(piConfigurable->Update(IConfigurable::CP_I_TIMER_T1, strIntValue));
    EXPECT_TRUE(piConfigurable->Update(IConfigurable::CP_I_TIMER_T2, strIntValue));

    EXPECT_TRUE(piConfigurable->Update(IConfigurable::CP_I_TIMER_100_TRYING, strIntValue));

    EXPECT_TRUE(piConfigurable->Update(IConfigurable::CP_I_SIP_FEATURES));
    EXPECT_TRUE(piConfigurable->Update(IConfigurable::CP_I_SIP_FEATURES, strIntValue));

    EXPECT_TRUE(piConfigurable->Update(IConfigurable::CP_I_TCP_CRITERION_LENGTH));
    EXPECT_TRUE(piConfigurable->Update(IConfigurable::CP_I_TCP_CRITERION_LENGTH, strIntValue));

    EXPECT_TRUE(piConfigurable->Update(IConfigurable::CP_I_REG_EXPIRES));
    EXPECT_TRUE(piConfigurable->Update(IConfigurable::CP_I_REG_EXPIRES, strIntValue));

    EXPECT_TRUE(piConfigurable->Update(IConfigurable::CP_I_REG_SUB));
    EXPECT_TRUE(piConfigurable->Update(IConfigurable::CP_I_REG_SUB, AString("true")));

    EXPECT_TRUE(piConfigurable->Update(IConfigurable::CP_I_REG_SUB_EXPIRES));
    EXPECT_TRUE(piConfigurable->Update(IConfigurable::CP_I_REG_SUB_EXPIRES, strIntValue));

    EXPECT_FALSE(piConfigurable->Update(IConfigurable::CP_I_MAX));

    EXPECT_EQ(objSipConfig.GetSipFeatureCaps(), strIntValue.ToUInt32(IMS_NULL, 16));

    EXPECT_TRUE(piConfigurable->Update(IConfigurable::CP_I_SIP_FEATURES));

    EXPECT_TRUE(objSipConfig.IsGruuConfigured());
    EXPECT_EQ(objSipConfig.GetDefaultScheme(), AString("sip"));
    EXPECT_EQ(objSipConfig.GetDeviceId(), ISipConfig::DEVICE_ID_UUID_IMEI_SHA1);

    EXPECT_EQ(objSipConfig.GetTcpCriterionLength(), nIntValue);
    EXPECT_EQ(objSipConfig.GetTransportType(), SipConfig::TRANSPORT_TYPE_UDP);
    EXPECT_EQ(objSipConfig.GetTimerValue100Trying(), nIntValue);

    const SipConfig::TcpTimerValues& objSipConfigTcpTimerValues = objSipConfig.GetTimerValueTcp();

    EXPECT_EQ(objSipConfigTcpTimerValues.m_nTvConnectionWaiting, -1);
    EXPECT_EQ(objSipConfigTcpTimerValues.m_nTvKeepAlive, SipConfig::TcpTimerValues::PERMANENT);
    EXPECT_EQ(objSipConfigTcpTimerValues.m_nTvWouldblockWaiting, -1);

    EXPECT_TRUE(objSipConfig.IsAuthenticationAlgorithmRequired());
    EXPECT_TRUE(objSipConfig.IsCompactFormConfigured());
    EXPECT_TRUE(objSipConfig.IsContactInAll1xxRequired());
    EXPECT_FALSE(objSipConfig.IsDisplayNameDquotRequired());
    EXPECT_TRUE(objSipConfig.IsExpiresHeaderInRegRequired());
    EXPECT_TRUE(objSipConfig.IsIpSecConfigured());
    EXPECT_FALSE(objSipConfig.IsKeepAliveConfigured());
    EXPECT_FALSE(objSipConfig.IsMultipleRegConfigured());
    EXPECT_FALSE(objSipConfig.IsNoAcceptContactHeaderInBYE());
    EXPECT_TRUE(objSipConfig.IsPermissionSipConfigured());
    EXPECT_FALSE(objSipConfig.IsPermissionSipsConfigured());
    EXPECT_TRUE(objSipConfig.IsPPreferredIdInRegSubRequired());
    EXPECT_FALSE(objSipConfig.IsRouteHeaderInRegRequired());
    EXPECT_TRUE(objSipConfig.IsRportConfigured());
    EXPECT_FALSE(objSipConfig.IsTransportErrorReportOnTxnRequired());
    EXPECT_TRUE(objSipConfig.IsTrustDomainConfigured());
    EXPECT_TRUE(objSipConfig.IsUdpFallbackConfigured());
    EXPECT_TRUE(objSipConfig.IsUserAgentConfigured());
    EXPECT_FALSE(objSipConfig.IsUserAgentSetByContext());
    EXPECT_TRUE(objSipConfig.IsSdpNegotiationRequiredForNonRpr());
    EXPECT_TRUE(objSipConfig.IsRequestUriValidationRequiredInMidDialog());
    EXPECT_TRUE(objSipConfig.IsSessionTimerUpdateRequiredByReInvite());
    EXPECT_TRUE(objSipConfig.IsSipInstanceParamRequiredInContactForNonRegisterRequest());
    EXPECT_TRUE(objSipConfig.IsSessionIdHeaderSupported());
    EXPECT_EQ(objSipConfig.GetHideMacInPaniHeaderPolicy(), ISipConfig::HIDE_MAC_IN_PANI);
    EXPECT_FALSE(objSipConfig.IsLocalTimezoneParameterSupportedInPaniHeader());
    EXPECT_EQ(objSipConfig.GetRegExpiration(), -1);
    EXPECT_FALSE(objSipConfig.IsRegExpirationConfigured());
    EXPECT_TRUE(objSipConfig.IsRegSubscriptionConfigured());
    EXPECT_FALSE(objSipConfig.IsRegSubExpirationConfigured());
    EXPECT_EQ(objSipConfig.GetRegSubExpiration(), -1);
}

TEST_F(SipConfigTest, CarrierConfig_NotifyConfigChanged)
{
    SipConfig objSipConfig(IMS_SLOT_0);
    ICarrierConfigListener* piCarrierConfigListener = &objSipConfig;

    objSipConfig.Init();

    const AStringArray& objAllowMethods = objSipConfig.GetRegAllowMethods();
    EXPECT_EQ(m_objAllowMethods.GetElements(), objAllowMethods.GetElements());

    piCarrierConfigListener->CarrierConfig_NotifyConfigChanged(IMS_SLOT_0);
    const AStringArray& objAllowMethodsForRefresh = objSipConfig.GetRegAllowMethods();
    EXPECT_EQ(m_objAllowMethods.GetElements(), objAllowMethodsForRefresh.GetElements());
}

}  // namespace android
