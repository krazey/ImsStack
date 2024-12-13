/*
 * Copyright (C) 2024 The Android Open Source Project
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

#include "CallReasonInfo.h"
#include "ImsAosParameter.h"
#include "MockIMessage.h"
#include "MockIMtcService.h"
#include "MockISession.h"
#include "PlatformContext.h"
#include "SipStatusCode.h"
#include "TestConfigService.h"
#include "TestPhoneInfoService.h"
#include "call/MockIMtcCallContext.h"
#include "call/MockIMtcSession.h"
#include "call/termination/EmergencyStartErrorHandler.h"
#include "configuration/MockMtcConfigurationProxy.h"
#include "helper/MockIMtcAosConnector.h"
#include "utility/MockIMessageUtils.h"

using ::testing::_;
using ::testing::Return;
using ::testing::ReturnRef;

namespace android
{

class EmergencyStartErrorHandlerTest : public ::testing::Test
{
public:
    MockIMtcCallContext objCallContext;
    MockIMtcService objMtcService;
    MockIMtcAosConnector objAosConnector;
    MockIMessage objMessage;
    MockMtcConfigurationProxy objConfigurationProxy;
    MockIMessageUtils objMessageUtils;
    MockIMtcSession objMtcSession;
    MockISession objSession;
    TestConfigService* m_pConfigService;
    EmergencyStartErrorHandler* pHandler;
    ImsVector<AString> objConfigurationArrary;
    TestPhoneInfoService m_objPhoneInfoService;

protected:
    virtual void SetUp() override
    {
        m_pConfigService = new TestConfigService();
        m_pConfigService->SetCarrierConfig(&(m_pConfigService->GetMockCarrierConfig()));
        PlatformContext::GetInstance()->SetService(
                PlatformContext::SERVICE_CONFIG, m_pConfigService);
        PlatformContext::GetInstance()->SetService(
                PlatformContext::SERVICE_PHONE_INFO, &m_objPhoneInfoService);

        ON_CALL(objCallContext, GetConfigurationProxy)
                .WillByDefault(ReturnRef(objConfigurationProxy));
        ON_CALL(objCallContext, GetService).WillByDefault(ReturnRef(objMtcService));
        ON_CALL(objCallContext, GetMessageUtils).WillByDefault(ReturnRef(objMessageUtils));
        ON_CALL(objCallContext, GetSession(&objSession)).WillByDefault(Return(&objMtcSession));
        ON_CALL(objMtcSession, GetISession).WillByDefault(ReturnRef(objSession));
        ON_CALL(objMtcService, GetAosConnector).WillByDefault(Return(&objAosConnector));
        ON_CALL(objMessageUtils, GetCauseFromReasonHeader).WillByDefault(Return(-1));
        pHandler = new EmergencyStartErrorHandler(objCallContext, objSession);
    }

    virtual void TearDown() override
    {
        PlatformContext::GetInstance()->SetService(PlatformContext::SERVICE_CONFIG, IMS_NULL);
        PlatformContext::GetInstance()->SetService(PlatformContext::SERVICE_PHONE_INFO, IMS_NULL);

        delete m_pConfigService;
        delete pHandler;
    }

    void SetRequireImmediateTerminationCode(
            IN IMS_SINT32 nStatusCode, IN IMS_SINT32 nCallReasonInfoCode)
    {
        objConfigurationArrary.Clear();
        AString strConfiguration;
        strConfiguration.Sprintf("%d:%d", nStatusCode, nCallReasonInfoCode);
        objConfigurationArrary.Push(strConfiguration);
        ON_CALL(objConfigurationProxy,
                GetStringArray(ConfigEmergency::
                                KEY_REJECT_CODE_REQUIRE_IMMEDIATE_TERMINATION_STRING_ARRAY))
                .WillByDefault(Return(objConfigurationArrary));
        ON_CALL(objMessage, GetStatusCode).WillByDefault(Return(nStatusCode));
    }

    void SetNotRequireImmediateTerminationCode(IN IMS_SINT32 nStatusCode)
    {
        objConfigurationArrary.Clear();
        ON_CALL(objConfigurationProxy,
                GetStringArray(ConfigEmergency::
                                KEY_REJECT_CODE_REQUIRE_IMMEDIATE_TERMINATION_STRING_ARRAY))
                .WillByDefault(Return(objConfigurationArrary));
        ON_CALL(objMessage, GetStatusCode).WillByDefault(Return(nStatusCode));
    }

    IMS_BOOL CheckHandleResult(IN IMS_SINT32 nCode, IN IMS_SINT32 nExtraCode)
    {
        CallReasonInfo objResult = pHandler->Handle(&objMessage);
        return objResult == CallReasonInfo(nCode, nExtraCode);
    }
};

TEST_F(EmergencyStartErrorHandlerTest, HandleRedialEmergencyWithNextPcscf)
{
    ON_CALL(objConfigurationProxy,
            GetBoolean(ConfigEmergency::
                            KEY_RETRY_EMERGENCY_CALL_OVER_EMERGENCY_PDN_WITH_NEXT_PCSCF_BOOL))
            .WillByDefault(Return(IMS_TRUE));
    ON_CALL(objMtcService, IsEmergency()).WillByDefault(Return(IMS_TRUE));
    SetNotRequireImmediateTerminationCode(SipStatusCode::SC_INVALID);
    ON_CALL(objMessageUtils, GetNumberOfPreviousResponses(&objSession, IMessage::SESSION_START))
            .WillByDefault(Return(1));
    EXPECT_CALL(objAosConnector, Control(ImsAosControl::E_REGISTER_FAKE_WITH_NEXT_PCSCF)).Times(1);
    EXPECT_TRUE(
            CheckHandleResult(CODE_INTERNAL_REDIAL, EXTRA_CODE_REDIAL_EMERGENCY_WITH_NEXT_PCSCF));

    ON_CALL(objConfigurationProxy,
            GetBoolean(ConfigEmergency::
                            KEY_RETRY_EMERGENCY_CALL_OVER_EMERGENCY_PDN_WITH_NEXT_PCSCF_BOOL))
            .WillByDefault(Return(IMS_FALSE));
    EXPECT_CALL(objAosConnector, Control(ImsAosControl::E_REGISTER_FAKE_WITH_NEXT_PCSCF)).Times(0);
    EXPECT_TRUE(
            CheckHandleResult(CODE_LOCAL_CALL_CS_RETRY_REQUIRED, EXTRA_CODE_CALL_RETRY_EMERGENCY));

    ON_CALL(objConfigurationProxy,
            GetBoolean(ConfigEmergency::
                            KEY_RETRY_EMERGENCY_CALL_OVER_EMERGENCY_PDN_WITH_NEXT_PCSCF_BOOL))
            .WillByDefault(Return(IMS_TRUE));
    ON_CALL(objMtcService, IsEmergency()).WillByDefault(Return(IMS_FALSE));
    EXPECT_CALL(objAosConnector, Control(ImsAosControl::E_REGISTER_FAKE_WITH_NEXT_PCSCF)).Times(0);
    EXPECT_TRUE(
            CheckHandleResult(CODE_LOCAL_CALL_CS_RETRY_REQUIRED, EXTRA_CODE_CALL_RETRY_EMERGENCY));

    ON_CALL(objMtcService, IsEmergency()).WillByDefault(Return(IMS_TRUE));
    SetNotRequireImmediateTerminationCode(SipStatusCode::SC_300);
    EXPECT_CALL(objAosConnector, Control(ImsAosControl::E_REGISTER_FAKE_WITH_NEXT_PCSCF)).Times(0);
    EXPECT_TRUE(
            CheckHandleResult(CODE_LOCAL_CALL_CS_RETRY_REQUIRED, EXTRA_CODE_CALL_RETRY_EMERGENCY));

    SetNotRequireImmediateTerminationCode(SipStatusCode::SC_INVALID);
    ON_CALL(objMessageUtils, GetNumberOfPreviousResponses(&objSession, IMessage::SESSION_START))
            .WillByDefault(Return(2));
    EXPECT_CALL(objAosConnector, Control(ImsAosControl::E_REGISTER_FAKE_WITH_NEXT_PCSCF)).Times(0);
    EXPECT_TRUE(
            CheckHandleResult(CODE_LOCAL_CALL_CS_RETRY_REQUIRED, EXTRA_CODE_CALL_RETRY_EMERGENCY));
}

TEST_F(EmergencyStartErrorHandlerTest, GetCallReasonInfoTimeout)
{
    SetRequireImmediateTerminationCode(SipStatusCode::SC_INVALID, CODE_NETWORK_RESP_TIMEOUT);
    EXPECT_TRUE(CheckHandleResult(CODE_NETWORK_RESP_TIMEOUT, EXTRA_CODE_METHOD_INVITE));
}

TEST_F(EmergencyStartErrorHandlerTest, RejectCodeRequireImmediateTermination)
{
    SetRequireImmediateTerminationCode(486, 338);
    EXPECT_TRUE(CheckHandleResult(CODE_SIP_BUSY, SipStatusCode::SC_486));
}

TEST_F(EmergencyStartErrorHandlerTest, RejectCodeNotRequireImmediateTermination)
{
    SetNotRequireImmediateTerminationCode(486);
    EXPECT_TRUE(
            CheckHandleResult(CODE_LOCAL_CALL_CS_RETRY_REQUIRED, EXTRA_CODE_CALL_RETRY_EMERGENCY));
}

TEST_F(EmergencyStartErrorHandlerTest, RejectCodeRequireTempFailure)
{
    ON_CALL(objConfigurationProxy,
            Contains(ConfigEmergency::KEY_REJECT_CODE_REQUIRE_TEMP_FAILURE_INT_ARRAY, 486))
            .WillByDefault(Return(IMS_TRUE));
    ON_CALL(m_objPhoneInfoService.GetMockCallInfo(), IsCrossSimRedialingAvailable)
            .WillByDefault(Return(IMS_TRUE));
    SetNotRequireImmediateTerminationCode(486);
    EXPECT_TRUE(CheckHandleResult(CODE_EMERGENCY_TEMP_FAILURE, -1));
}

TEST_F(EmergencyStartErrorHandlerTest, RejectCodeRequirePermFailure)
{
    ON_CALL(objConfigurationProxy,
            Contains(ConfigEmergency::KEY_REJECT_CODE_REQUIRE_TEMP_FAILURE_INT_ARRAY, 486))
            .WillByDefault(Return(IMS_FALSE));
    ON_CALL(objConfigurationProxy,
            Contains(ConfigEmergency::KEY_REJECT_CODE_REQUIRE_PERM_FAILURE_INT_ARRAY, 486))
            .WillByDefault(Return(IMS_TRUE));
    ON_CALL(m_objPhoneInfoService.GetMockCallInfo(), IsCrossSimRedialingAvailable)
            .WillByDefault(Return(IMS_TRUE));
    SetNotRequireImmediateTerminationCode(486);
    EXPECT_TRUE(CheckHandleResult(CODE_EMERGENCY_PERM_FAILURE, -1));
}

TEST_F(EmergencyStartErrorHandlerTest, RejectCodeNotRequireCrossSimRedialing)
{
    ON_CALL(objConfigurationProxy,
            Contains(ConfigEmergency::KEY_REJECT_CODE_REQUIRE_TEMP_FAILURE_INT_ARRAY, 486))
            .WillByDefault(Return(IMS_FALSE));
    ON_CALL(objConfigurationProxy,
            Contains(ConfigEmergency::KEY_REJECT_CODE_REQUIRE_PERM_FAILURE_INT_ARRAY, 486))
            .WillByDefault(Return(IMS_FALSE));
    SetNotRequireImmediateTerminationCode(486);
    EXPECT_TRUE(
            CheckHandleResult(CODE_LOCAL_CALL_CS_RETRY_REQUIRED, EXTRA_CODE_CALL_RETRY_EMERGENCY));

    ON_CALL(objConfigurationProxy,
            Contains(ConfigEmergency::KEY_REJECT_CODE_REQUIRE_TEMP_FAILURE_INT_ARRAY, 486))
            .WillByDefault(Return(IMS_TRUE));
    ON_CALL(m_objPhoneInfoService.GetMockCallInfo(), IsCrossSimRedialingAvailable)
            .WillByDefault(Return(IMS_FALSE));
    EXPECT_TRUE(
            CheckHandleResult(CODE_LOCAL_CALL_CS_RETRY_REQUIRED, EXTRA_CODE_CALL_RETRY_EMERGENCY));
}

TEST_F(EmergencyStartErrorHandlerTest,
        HandleRedialsWithVoipFromRttIfSilentRedialByRttEmergencyRejectionRequired)
{
    ON_CALL(objMtcSession, GetCallType()).WillByDefault(Return(CallType::RTT));
    ON_CALL(objConfigurationProxy,
            GetBoolean(
                    CarrierConfig::ImsEmergency::KEY_SILENT_REDIAL_WITH_VOIP_BY_RTT_REJECTION_BOOL))
            .WillByDefault(Return(IMS_TRUE));
    EXPECT_TRUE(
            CheckHandleResult(CODE_INTERNAL_REDIAL, EXTRA_CODE_REDIAL_BY_RTT_EMERGENCY_REJECTION));
}

TEST_F(EmergencyStartErrorHandlerTest, HandleDoesNotRedialWithVoipIfEmergencyCallIsNotRtt)
{
    ON_CALL(objMtcSession, GetCallType()).WillByDefault(Return(CallType::VOIP));
    EXPECT_FALSE(
            CheckHandleResult(CODE_INTERNAL_REDIAL, EXTRA_CODE_REDIAL_BY_RTT_EMERGENCY_REJECTION));
}

TEST_F(EmergencyStartErrorHandlerTest,
        HandleDoesNotRedialWithVoipFromRttIfSilentRedialByRttEmergencyRejectionIsNotRequired)
{
    ON_CALL(objMtcSession, GetCallType()).WillByDefault(Return(CallType::RTT));
    ON_CALL(objConfigurationProxy,
            GetBoolean(
                    CarrierConfig::ImsEmergency::KEY_SILENT_REDIAL_WITH_VOIP_BY_RTT_REJECTION_BOOL))
            .WillByDefault(Return(IMS_FALSE));
    EXPECT_FALSE(
            CheckHandleResult(CODE_INTERNAL_REDIAL, EXTRA_CODE_REDIAL_BY_RTT_EMERGENCY_REJECTION));
}

}  // namespace android
