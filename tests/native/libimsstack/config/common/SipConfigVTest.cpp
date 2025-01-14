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

#include "ISipConfigV.h"
#include "MockICarrierConfig.h"
#include "PlatformContext.h"
#include "TestConfigService.h"

#include "CarrierConfig.h"

#include "private/SipConfigV.h"

using ::testing::_;
using ::testing::AnyNumber;
using ::testing::Return;
using ::testing::ReturnRoundRobin;

namespace android
{

class SipConfigVTest : public ::testing::Test
{
public:
    SipConfigVTest();
    virtual ~SipConfigVTest() {}

protected:
    virtual void SetUp() override
    {
        m_pConfigService = new TestConfigService();

        PlatformContext::GetInstance()->SetService(
                PlatformContext::SERVICE_CONFIG, m_pConfigService);

        ON_CALL(m_pConfigService->GetMockCarrierConfig(), AddListener(_)).WillByDefault(Return());
        ON_CALL(m_pConfigService->GetMockCarrierConfig(), RemoveListener(_))
                .WillByDefault(Return());
    }
    virtual void TearDown() override
    {
        PlatformContext::GetInstance()->SetService(PlatformContext::SERVICE_CONFIG, IMS_NULL);

        if (m_pConfigService != IMS_NULL)
        {
            delete m_pConfigService;
            m_pConfigService = IMS_NULL;
        }
    }

protected:
    TestConfigService* m_pConfigService;
    AStringArray m_objAllowMethods;
};

SipConfigVTest::SipConfigVTest() :
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

TEST_F(SipConfigVTest, Init)
{
    const AString strUaString("IMS-client");
    const IMS_SINT32 nMinSe = 90;
    const IMS_SINT32 nSessionExpires = 900;
    MockICarrierConfig& objCarrierConfig = m_pConfigService->GetMockCarrierConfig();

    ON_CALL(objCarrierConfig, GetInt(CarrierConfig::Ims::KEY_REQUEST_URI_TYPE_INT, _))
            .WillByDefault(ReturnRoundRobin({CarrierConfig::Ims::REQUEST_URI_FORMAT_SIP,
                    CarrierConfig::Ims::REQUEST_URI_FORMAT_TEL}));
    ON_CALL(objCarrierConfig, GetString(CarrierConfig::Ims::KEY_IMS_USER_AGENT_STRING, _))
            .WillByDefault(Return(strUaString));
    ON_CALL(objCarrierConfig, GetInt(CarrierConfig::Ims::KEY_SIP_TIMER_T1_MILLIS_INT, _))
            .WillByDefault(Return(SipConfigV::DEFAULT_TIMER_T1));
    ON_CALL(objCarrierConfig, GetInt(CarrierConfig::Ims::KEY_SIP_TIMER_T2_MILLIS_INT, _))
            .WillByDefault(Return(SipConfigV::DEFAULT_TIMER_T2));
    ON_CALL(objCarrierConfig, GetInt(CarrierConfig::Ims::KEY_SIP_TIMER_T4_MILLIS_INT, _))
            .WillByDefault(Return(SipConfigV::DEFAULT_TIMER_T4));
    ON_CALL(objCarrierConfig, GetInt(CarrierConfig::Ims::KEY_SIP_TIMER_B_MILLIS_INT, _))
            .WillByDefault(Return(SipConfigV::DEFAULT_TIMER_T1 * 64));
    ON_CALL(objCarrierConfig, GetInt(CarrierConfig::Ims::KEY_SIP_TIMER_C_MILLIS_INT, _))
            .WillByDefault(Return(SipConfigV::DEFAULT_TIMER_C));
    ON_CALL(objCarrierConfig, GetInt(CarrierConfig::Ims::KEY_SIP_TIMER_D_MILLIS_INT, _))
            .WillByDefault(Return(SipConfigV::DEFAULT_TIMER_D));
    ON_CALL(objCarrierConfig, GetInt(CarrierConfig::Ims::KEY_SIP_TIMER_F_MILLIS_INT, _))
            .WillByDefault(Return(SipConfigV::DEFAULT_TIMER_T1 * 64));
    ON_CALL(objCarrierConfig, GetInt(CarrierConfig::Ims::KEY_SIP_TIMER_H_MILLIS_INT, _))
            .WillByDefault(Return(SipConfigV::DEFAULT_TIMER_T1 * 64));
    ON_CALL(objCarrierConfig, GetInt(CarrierConfig::Ims::KEY_SIP_TIMER_J_MILLIS_INT, _))
            .WillByDefault(Return(SipConfigV::DEFAULT_TIMER_T1 * 64));
    ON_CALL(objCarrierConfig,
            GetBoolean(CarrierConfig::ImsVoice::KEY_SESSION_TIMER_SUPPORTED_BOOL, _))
            .WillByDefault(Return(IMS_TRUE));
    ON_CALL(objCarrierConfig,
            GetBoolean(CarrierConfig::Ims::KEY_SUPPORT_LOCAL_SESSION_TIMER_BOOL, _))
            .WillByDefault(Return(IMS_TRUE));
    ON_CALL(objCarrierConfig, GetInt(CarrierConfig::ImsVoice::KEY_SESSION_REFRESHER_TYPE_INT, _))
            .WillByDefault(ReturnRoundRobin({CarrierConfig::ImsVoice::SESSION_REFRESHER_TYPE_UAC,
                    CarrierConfig::ImsVoice::SESSION_REFRESHER_TYPE_UAS}));
    ON_CALL(objCarrierConfig, GetInt(CarrierConfig::ImsVoice::KEY_SESSION_REFRESH_METHOD_INT, _))
            .WillByDefault(ReturnRoundRobin({CarrierConfig::ImsVoice::SESSION_REFRESH_METHOD_INVITE,
                    CarrierConfig::ImsVoice::SESSION_REFRESH_METHOD_UPDATE_PREFERRED}));
    ON_CALL(objCarrierConfig,
            GetInt(CarrierConfig::ImsVoice::KEY_MINIMUM_SESSION_EXPIRES_TIMER_SEC_INT, _))
            .WillByDefault(Return(nMinSe));
    ON_CALL(objCarrierConfig, GetInt(CarrierConfig::ImsVoice::KEY_SESSION_EXPIRES_TIMER_SEC_INT, _))
            .WillByDefault(Return(nSessionExpires));
    ON_CALL(objCarrierConfig,
            GetBoolean(CarrierConfig::Ims::
                               KEY_SESSION_TIMER_UPDATE_REQUIRED_IN_SESSION_UPDATE_BY_REINVITE_BOOL,
                    _))
            .WillByDefault(Return(IMS_TRUE));
    ON_CALL(objCarrierConfig,
            GetBoolean(CarrierConfig::Ims::KEY_SDP_NEGOTIATION_REQUIRED_FOR_NON_RPR_BOOL, _))
            .WillByDefault(Return(IMS_TRUE));

    SipConfigV objSipConfigV(IMS_SLOT_0);

    objSipConfigV.Init();

    const AStringArray& objAllowMethods = objSipConfigV.GetAllowMethods();
    EXPECT_EQ(m_objAllowMethods.GetElements(), objAllowMethods.GetElements());
    EXPECT_EQ(objSipConfigV.GetTargetScheme(), SipConfigV::TARGET_SCHEME_SIP);
    EXPECT_EQ(objSipConfigV.GetServiceVersion(), strUaString);
    EXPECT_EQ(objSipConfigV.GetTimerValueT1(), SipConfigV::DEFAULT_TIMER_T1);
    EXPECT_EQ(objSipConfigV.GetTimerValueT2(), SipConfigV::DEFAULT_TIMER_T2);
    EXPECT_EQ(objSipConfigV.GetTimerValueT4(), SipConfigV::DEFAULT_TIMER_T4);
    EXPECT_EQ(objSipConfigV.GetTimerValueB(), SipConfigV::DEFAULT_TIMER_T1 * 64);
    EXPECT_EQ(objSipConfigV.GetTimerValueC(), SipConfigV::DEFAULT_TIMER_C);
    EXPECT_EQ(objSipConfigV.GetTimerValueD(), SipConfigV::DEFAULT_TIMER_D);
    EXPECT_EQ(objSipConfigV.GetTimerValueF(), SipConfigV::DEFAULT_TIMER_T1 * 64);
    EXPECT_EQ(objSipConfigV.GetTimerValueH(), SipConfigV::DEFAULT_TIMER_T1 * 64);
    EXPECT_EQ(objSipConfigV.GetTimerValueJ(), SipConfigV::DEFAULT_TIMER_T1 * 64);
    EXPECT_TRUE(objSipConfigV.IsSessionTimerSupported());
    EXPECT_TRUE((objSipConfigV.GetSessionHeaders() &
                        SipConfigV::SESSION_HEADER_LOCAL_TIMER_REQUIRED) != 0);
    EXPECT_EQ(objSipConfigV.GetSessionRefresher(), SipConfigV::SESSION_REFRESHER_LOCAL);
    EXPECT_EQ(objSipConfigV.GetSessionMethod(), SipConfigV::SESSION_REFRESH_INVITE);
    EXPECT_EQ(objSipConfigV.GetSessionMinSe(), nMinSe);
    EXPECT_EQ(objSipConfigV.GetSessionExpires(), nSessionExpires);
    EXPECT_FALSE(objSipConfigV.IsSessionNoRefreshByReInvite());
    EXPECT_TRUE(objSipConfigV.IsSessionSdpNonRprAllowed());

    objSipConfigV.Init();

    EXPECT_EQ(objSipConfigV.GetTargetScheme(), SipConfigV::TARGET_SCHEME_TEL);
    EXPECT_EQ(objSipConfigV.GetSessionRefresher(), SipConfigV::SESSION_REFRESHER_REMOTE);
    EXPECT_EQ(objSipConfigV.GetSessionMethod(), SipConfigV::SESSION_REFRESH_UPDATE_PREFERRED);
}

TEST_F(SipConfigVTest, Refresh)
{
    SipConfigV objSipConfigV(IMS_SLOT_0);

    objSipConfigV.Init();

    const AStringArray& objAllowMethods = objSipConfigV.GetAllowMethods();
    EXPECT_EQ(m_objAllowMethods.GetElements(), objAllowMethods.GetElements());

    ICarrierConfigListener& objListener = DYNAMIC_CAST(ICarrierConfigListener&, objSipConfigV);
    objListener.CarrierConfig_NotifyConfigChanged(IMS_SLOT_0);
    const AStringArray& objAllowMethodsForRefresh = objSipConfigV.GetAllowMethods();
    EXPECT_EQ(m_objAllowMethods.GetElements(), objAllowMethodsForRefresh.GetElements());
}

TEST_F(SipConfigVTest, GetSessionHeaders)
{
    ON_CALL(m_pConfigService->GetMockCarrierConfig(),
            GetBoolean(CarrierConfig::Ims::KEY_SUPPORT_LOCAL_SESSION_TIMER_BOOL, _))
            .WillByDefault(ReturnRoundRobin({IMS_FALSE, IMS_TRUE}));
    ON_CALL(m_pConfigService->GetMockCarrierConfig(),
            GetBoolean(CarrierConfig::Ims::KEY_ALLOW_SESSION_TIMER_TURN_OFF_BOOL, _))
            .WillByDefault(ReturnRoundRobin({IMS_FALSE, IMS_TRUE}));

    SipConfigV objSipConfigV(IMS_SLOT_0);
    objSipConfigV.Init();

    IMS_SINT32 nDefaultSessionHeaders =
            SipConfigV::SESSION_HEADER_SESSION_EXPIRES | SipConfigV::SESSION_HEADER_MIN_SE;
    EXPECT_EQ(objSipConfigV.GetSessionHeaders(), nDefaultSessionHeaders);

    // Refresh
    ICarrierConfigListener& objListener = DYNAMIC_CAST(ICarrierConfigListener&, objSipConfigV);
    objListener.CarrierConfig_NotifyConfigChanged(IMS_SLOT_0);

    nDefaultSessionHeaders |= SipConfigV::SESSION_HEADER_LOCAL_TIMER_REQUIRED;
    nDefaultSessionHeaders |= SipConfigV::SESSION_HEADER_SESSION_TIMER_TURN_OFF_ALLOWED;
    EXPECT_EQ(objSipConfigV.GetSessionHeaders(), nDefaultSessionHeaders);
}

TEST_F(SipConfigVTest, Update)
{
    SipConfigV objSipConfigV(IMS_SLOT_0);
    IConfigurable* piConfigurable =
            static_cast<const ISipConfigV*>(&objSipConfigV)->GetConfigurable();

    EXPECT_TRUE(piConfigurable->Update(IConfigurable::CP_I_START_SIP_V));
    EXPECT_TRUE(piConfigurable->Update(IConfigurable::CP_I_END_SIP_V));

    IMS_SINT32 nIntValue = 10;
    AString strIntValue;

    strIntValue.SetNumber(nIntValue);

    MockICarrierConfig& objCarrierConfig = m_pConfigService->GetMockCarrierConfig();
    AString strUaString("IMS-client");

    EXPECT_CALL(objCarrierConfig, GetString(CarrierConfig::Ims::KEY_IMS_USER_AGENT_STRING, _))
            .Times(2)
            .WillOnce(Return(strUaString));

    EXPECT_TRUE(piConfigurable->Update(IConfigurable::CP_I_TIMER_T1, strIntValue));
    EXPECT_TRUE(piConfigurable->Update(IConfigurable::CP_I_TIMER_T2, strIntValue));
    EXPECT_TRUE(piConfigurable->Update(IConfigurable::CP_I_TIMER_T4, strIntValue));
    EXPECT_TRUE(piConfigurable->Update(IConfigurable::CP_I_TIMER_A, strIntValue));
    EXPECT_TRUE(piConfigurable->Update(IConfigurable::CP_I_TIMER_B, strIntValue));
    EXPECT_TRUE(piConfigurable->Update(IConfigurable::CP_I_TIMER_C, strIntValue));
    EXPECT_TRUE(piConfigurable->Update(IConfigurable::CP_I_TIMER_D, strIntValue));
    EXPECT_TRUE(piConfigurable->Update(IConfigurable::CP_I_TIMER_E, strIntValue));
    EXPECT_TRUE(piConfigurable->Update(IConfigurable::CP_I_TIMER_F, strIntValue));
    EXPECT_TRUE(piConfigurable->Update(IConfigurable::CP_I_TIMER_G, strIntValue));
    EXPECT_TRUE(piConfigurable->Update(IConfigurable::CP_I_TIMER_H, strIntValue));
    EXPECT_TRUE(piConfigurable->Update(IConfigurable::CP_I_TIMER_I, strIntValue));
    EXPECT_TRUE(piConfigurable->Update(IConfigurable::CP_I_TIMER_J, strIntValue));
    EXPECT_TRUE(piConfigurable->Update(IConfigurable::CP_I_TIMER_K, strIntValue));
    EXPECT_TRUE(piConfigurable->Update(IConfigurable::CP_I_UA_VERSION));
    EXPECT_TRUE(piConfigurable->Update(IConfigurable::CP_I_FEATURE_TAG_OPTIONS, strIntValue));
    EXPECT_TRUE(piConfigurable->Update(IConfigurable::CP_I_SESSION_MINSE, strIntValue));
    EXPECT_TRUE(piConfigurable->Update(IConfigurable::CP_I_SESSION_EXPIRES, strIntValue));

    EXPECT_EQ(objSipConfigV.GetTargetNumberFormat(), ISipConfigV::TARGET_NUMBER_FORMAT_LOCAL);
    EXPECT_EQ(objSipConfigV.GetTargetScheme(), ISipConfigV::TARGET_SCHEME_TEL);
    EXPECT_EQ(objSipConfigV.GetPreferredId(), SipConfigV::PREFERRED_ID_DEFAULT);
    EXPECT_EQ(objSipConfigV.GetServiceVersion(), strUaString);
    EXPECT_EQ(objSipConfigV.GetPredefinedPaniForEutran(), AString::ConstNull());
    EXPECT_EQ(objSipConfigV.GetPredefinedPaniForUtran(), AString::ConstNull());

    EXPECT_TRUE(objSipConfigV.IsTimerValueConfiguredOnRuntime());
    EXPECT_TRUE(objSipConfigV.IsSessionTimerSupported());
    EXPECT_TRUE(objSipConfigV.IsPageMessageRespByApp());
    EXPECT_TRUE(objSipConfigV.IsReferenceRespByApp());

    EXPECT_EQ(objSipConfigV.GetTimerValueT2(), nIntValue);
    EXPECT_EQ(objSipConfigV.GetTimerValueT4(), nIntValue);
    EXPECT_EQ(objSipConfigV.GetTimerValueA(), nIntValue);
    EXPECT_EQ(objSipConfigV.GetTimerValueB(), nIntValue);
    EXPECT_EQ(objSipConfigV.GetTimerValueC(), nIntValue);
    EXPECT_EQ(objSipConfigV.GetTimerValueD(), nIntValue);
    EXPECT_EQ(objSipConfigV.GetTimerValueE(), nIntValue);
    EXPECT_EQ(objSipConfigV.GetTimerValueF(), nIntValue);
    EXPECT_EQ(objSipConfigV.GetTimerValueG(), nIntValue);
    EXPECT_EQ(objSipConfigV.GetTimerValueH(), nIntValue);
    EXPECT_EQ(objSipConfigV.GetTimerValueI(), nIntValue);
    EXPECT_EQ(objSipConfigV.GetTimerValueJ(), nIntValue);
    EXPECT_EQ(objSipConfigV.GetTimerValueK(), nIntValue);

    EXPECT_CALL(objCarrierConfig, GetBoolean(_, _))
            .Times(AnyNumber())
            .WillRepeatedly(Return(IMS_TRUE));

    nIntValue = 1000;

    EXPECT_CALL(objCarrierConfig, GetInt(_, _))
            .Times(AnyNumber())
            .WillRepeatedly(Return(nIntValue));

    EXPECT_TRUE(piConfigurable->Update(IConfigurable::CP_I_SIP_ALL));

    EXPECT_EQ(objSipConfigV.GetTimerValueT2(), nIntValue);
    EXPECT_EQ(objSipConfigV.GetTimerValueT4(), nIntValue);
    EXPECT_EQ(objSipConfigV.GetTimerValueA(), nIntValue);
    EXPECT_EQ(objSipConfigV.GetTimerValueB(), nIntValue);
    EXPECT_EQ(objSipConfigV.GetTimerValueC(), nIntValue);
    EXPECT_EQ(objSipConfigV.GetTimerValueD(), nIntValue);
    EXPECT_EQ(objSipConfigV.GetTimerValueE(), nIntValue);
    EXPECT_EQ(objSipConfigV.GetTimerValueF(), nIntValue);
    EXPECT_EQ(objSipConfigV.GetTimerValueG(), nIntValue);
    EXPECT_EQ(objSipConfigV.GetTimerValueH(), nIntValue);
    EXPECT_EQ(objSipConfigV.GetTimerValueI(), nIntValue);
    EXPECT_EQ(objSipConfigV.GetTimerValueJ(), nIntValue);
    EXPECT_EQ(objSipConfigV.GetTimerValueK(), nIntValue);

    EXPECT_FALSE(piConfigurable->Update(IConfigurable::CP_I_MAX));
}

}  // namespace android
