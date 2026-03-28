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
#include "IImsAosInfo.h"
#include "ISipHeader.h"
#include "ImsAosParameter.h"
#include "ImsVector.h"
#include "MockIMessage.h"
#include "MockIMtcCallController.h"
#include "MockIMtcService.h"
#include "MockISession.h"
#include "PlatformContext.h"
#include "SipStatusCode.h"
#include "TestConfigService.h"
#include "TestPhoneInfoService.h"
#include "TextParser.h"
#include "call/MockIMtcCallContext.h"
#include "call/MockIMtcSession.h"
#include "call/MockISilentRedialHelper.h"
#include "call/termination/EmergencyStartErrorHandler.h"
#include "configuration/MockMtcConfigurationProxy.h"
#include "helper/MockIMtcAosConnector.h"
#include "utility/MockIMessageUtils.h"
#include <gtest/gtest.h>
#include <initializer_list>

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
    MockIMtcCallController objCallController;
    // cppcheck-suppress unusedStructMember
    MockISilentRedialHelper objRedialHelper;
    TestConfigService* m_pConfigService;
    ImsVector<AString> objActionSets;
    EmergencyStartErrorHandler* pHandler;
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
        ON_CALL(objCallContext, GetCallController).WillByDefault(ReturnRef(objCallController));
        ON_CALL(objCallController, GetActiveRedialHelper()).WillByDefault(Return(nullptr));
        pHandler = new EmergencyStartErrorHandler(objCallContext, objSession);
    }

    virtual void TearDown() override
    {
        PlatformContext::GetInstance()->SetService(PlatformContext::SERVICE_CONFIG, IMS_NULL);
        PlatformContext::GetInstance()->SetService(PlatformContext::SERVICE_PHONE_INFO, IMS_NULL);

        delete m_pConfigService;
        delete pHandler;
    }

    void SetMessageCode(IN IMS_SINT32 nStatusCode)
    {
        ON_CALL(objMessage, GetStatusCode).WillByDefault(Return(nStatusCode));
    }

    void SetActionConfig(IN IMS_SINT32 nStatusCode, IN IMS_SINT32 nSingleAction)
    {
        SetActionConfigs(nStatusCode, {nSingleAction});
    }

    void SetActionConfigs(IN IMS_SINT32 nStatusCode, std::initializer_list<IMS_SINT32> objActions)
    {
        AString strActionSet;
        strActionSet.SetNumber(nStatusCode);
        strActionSet += TextParser::STR_COLON;

        bool bFirst = true;
        for (IMS_SINT32 nAction : objActions)
        {
            if (!bFirst)
            {
                strActionSet += TextParser::STR_COMMA;
            }
            AString strAction;
            strAction.SetNumber(nAction);
            strActionSet += strAction;
            bFirst = false;
        }

        objActionSets.Add(strActionSet);
        ON_CALL(objConfigurationProxy,
                GetStringArray(ConfigEmergency::KEY_REJECT_CODE_AND_ACTION_SET_STRING_ARRAY))
                .WillByDefault(Return(objActionSets));
    }

    IMS_BOOL CheckHandleResult(IN IMS_SINT32 nCode, IN IMS_SINT32 nExtraCode)
    {
        CallReasonInfo objResult = pHandler->Handle(&objMessage);
        return objResult == CallReasonInfo(nCode, nExtraCode);
    }

    IMS_BOOL CheckHandleResult(
            IN IMS_SINT32 nCode, IN IMS_SINT32 nExtraCode, AString strExtraMessage)
    {
        CallReasonInfo objResult = pHandler->Handle(&objMessage);
        return objResult == CallReasonInfo(nCode, nExtraCode, strExtraMessage);
    }
};

TEST_F(EmergencyStartErrorHandlerTest, HandleRedialEmergencyWithNextPcscf)
{
    const IMS_SINT32 ANY_REJECT_CODE = SipStatusCode::SC_400;
    SetMessageCode(ANY_REJECT_CODE);
    SetActionConfig(ANY_REJECT_CODE,
            ConfigEmergency::START_ERROR_ACTION_SILENT_REINVITE_NEXT_PCSCF_IF_EPDN);

    // test normal
    ON_CALL(objMtcService, IsEmergency()).WillByDefault(Return(IMS_TRUE));

    ON_CALL(objMessageUtils, GetNumberOfPreviousResponses(&objSession, IMessage::SESSION_START))
            .WillByDefault(Return(1));
    EXPECT_CALL(objAosConnector, Control(ImsAosControl::E_REGISTER_FAKE_WITH_NEXT_PCSCF)).Times(1);
    EXPECT_TRUE(
            CheckHandleResult(CODE_INTERNAL_REDIAL, EXTRA_CODE_REDIAL_EMERGENCY_WITH_NEXT_PCSCF));

    // test no action case
    objActionSets.Clear();
    ON_CALL(objConfigurationProxy,
            GetStringArray(ConfigEmergency::KEY_REJECT_CODE_AND_ACTION_SET_STRING_ARRAY))
            .WillByDefault(Return(objActionSets));
    EXPECT_CALL(objAosConnector, Control(ImsAosControl::E_REGISTER_FAKE_WITH_NEXT_PCSCF)).Times(0);
    EXPECT_TRUE(
            CheckHandleResult(CODE_LOCAL_CALL_CS_RETRY_REQUIRED, EXTRA_CODE_CALL_RETRY_EMERGENCY));

    // test no Emergency PDN case
    SetActionConfig(ANY_REJECT_CODE,
            ConfigEmergency::START_ERROR_ACTION_SILENT_REINVITE_NEXT_PCSCF_IF_EPDN);
    ON_CALL(objMtcService, IsEmergency()).WillByDefault(Return(IMS_FALSE));
    EXPECT_CALL(objAosConnector, Control(ImsAosControl::E_REGISTER_FAKE_WITH_NEXT_PCSCF)).Times(0);
    EXPECT_TRUE(
            CheckHandleResult(CODE_LOCAL_CALL_CS_RETRY_REQUIRED, EXTRA_CODE_CALL_RETRY_EMERGENCY));

    // test error response after 100 Trying case
    ON_CALL(objMessageUtils, GetNumberOfPreviousResponses(&objSession, IMessage::SESSION_START))
            .WillByDefault(Return(2));
    EXPECT_CALL(objAosConnector, Control(ImsAosControl::E_REGISTER_FAKE_WITH_NEXT_PCSCF)).Times(0);
    EXPECT_TRUE(
            CheckHandleResult(CODE_LOCAL_CALL_CS_RETRY_REQUIRED, EXTRA_CODE_CALL_RETRY_EMERGENCY));
}

TEST_F(EmergencyStartErrorHandlerTest, HandleRedialWithAnonymousByNetworkRejection)
{
    SetMessageCode(SipStatusCode::SC_INVALID);

    ON_CALL(objAosConnector, GetRegistrationMode)
            .WillByDefault(Return(IImsAosInfo::REG_MODE_INTERNAL));

    EXPECT_CALL(objAosConnector, Control(ImsAosControl::E_REGISTER_FAKE_WITH_SAME_PCSCF)).Times(0);
    EXPECT_TRUE(
            CheckHandleResult(CODE_LOCAL_CALL_CS_RETRY_REQUIRED, EXTRA_CODE_CALL_RETRY_EMERGENCY));

    SetMessageCode(SipStatusCode::SC_403);
    EXPECT_CALL(objAosConnector, Control(ImsAosControl::E_REGISTER_FAKE_WITH_SAME_PCSCF)).Times(0);
    EXPECT_TRUE(
            CheckHandleResult(CODE_LOCAL_CALL_CS_RETRY_REQUIRED, EXTRA_CODE_CALL_RETRY_EMERGENCY));

    ON_CALL(objAosConnector, GetRegistrationMode)
            .WillByDefault(Return(IImsAosInfo::REG_MODE_NORMAL));
    EXPECT_CALL(objAosConnector, Control(ImsAosControl::E_REGISTER_FAKE_WITH_SAME_PCSCF)).Times(0);
    EXPECT_TRUE(
            CheckHandleResult(CODE_LOCAL_CALL_CS_RETRY_REQUIRED, EXTRA_CODE_CALL_RETRY_EMERGENCY));

    SetActionConfig(
            SipStatusCode::SC_403, ConfigEmergency::START_ERROR_ACTION_SILENT_REINVITE_ANONYMOUS);
    EXPECT_CALL(objAosConnector, Control(ImsAosControl::E_REGISTER_FAKE_WITH_SAME_PCSCF)).Times(1);
    EXPECT_TRUE(
            CheckHandleResult(CODE_INTERNAL_REDIAL, EXTRA_CODE_REDIAL_EMERGENCY_WITH_ANONYMOUS));
}

TEST_F(EmergencyStartErrorHandlerTest, GetCallReasonInfoTimeout)
{
    const IMS_SINT32 ANY_REJECT_CODE = SipStatusCode::SC_INVALID;
    SetMessageCode(ANY_REJECT_CODE);
    SetActionConfig(ANY_REJECT_CODE, ConfigEmergency::START_ERROR_ACTION_TERMINATE);
    EXPECT_TRUE(CheckHandleResult(CODE_NETWORK_RESP_TIMEOUT, EXTRA_CODE_METHOD_INVITE));
}

TEST_F(EmergencyStartErrorHandlerTest, RejectCodeRequireImmediateTermination)
{
    const IMS_SINT32 ANY_REJECT_CODE = SipStatusCode::SC_486;
    SetMessageCode(ANY_REJECT_CODE);
    SetActionConfig(ANY_REJECT_CODE, ConfigEmergency::START_ERROR_ACTION_TERMINATE);
    EXPECT_TRUE(CheckHandleResult(CODE_SIP_BUSY, SipStatusCode::SC_486));
}

TEST_F(EmergencyStartErrorHandlerTest, RejectCodeNotRequireImmediateTermination)
{
    const IMS_SINT32 ANY_REJECT_CODE = SipStatusCode::SC_486;
    SetMessageCode(ANY_REJECT_CODE);
    EXPECT_TRUE(
            CheckHandleResult(CODE_LOCAL_CALL_CS_RETRY_REQUIRED, EXTRA_CODE_CALL_RETRY_EMERGENCY));
}

TEST_F(EmergencyStartErrorHandlerTest, RejectCodeRequireTempFailure)
{
    const IMS_SINT32 ANY_REJECT_CODE = SipStatusCode::SC_486;
    SetMessageCode(ANY_REJECT_CODE);
    SetActionConfig(ANY_REJECT_CODE, ConfigEmergency::START_ERROR_ACTION_CROSS_SIM_TEMP_FAILURE);

    ON_CALL(m_objPhoneInfoService.GetMockCallInfo(), IsCrossSimRedialingAvailable)
            .WillByDefault(Return(IMS_TRUE));
    EXPECT_TRUE(CheckHandleResult(CODE_EMERGENCY_TEMP_FAILURE, -1));
}

TEST_F(EmergencyStartErrorHandlerTest, RejectCodeRequirePermFailure)
{
    const IMS_SINT32 ANY_REJECT_CODE = SipStatusCode::SC_486;
    SetMessageCode(ANY_REJECT_CODE);
    SetActionConfig(ANY_REJECT_CODE, ConfigEmergency::START_ERROR_ACTION_CROSS_SIM_PERM_FAILURE);
    ON_CALL(m_objPhoneInfoService.GetMockCallInfo(), IsCrossSimRedialingAvailable)
            .WillByDefault(Return(IMS_TRUE));
    EXPECT_TRUE(CheckHandleResult(CODE_EMERGENCY_PERM_FAILURE, -1));
}

TEST_F(EmergencyStartErrorHandlerTest, RejectCodeNotRequireCrossSimRedialing)
{
    const IMS_SINT32 ANY_REJECT_CODE = SipStatusCode::SC_486;
    SetMessageCode(ANY_REJECT_CODE);

    EXPECT_TRUE(
            CheckHandleResult(CODE_LOCAL_CALL_CS_RETRY_REQUIRED, EXTRA_CODE_CALL_RETRY_EMERGENCY));

    SetActionConfig(ANY_REJECT_CODE, ConfigEmergency::START_ERROR_ACTION_CROSS_SIM_TEMP_FAILURE);
    ON_CALL(m_objPhoneInfoService.GetMockCallInfo(), IsCrossSimRedialingAvailable)
            .WillByDefault(Return(IMS_FALSE));
    EXPECT_TRUE(
            CheckHandleResult(CODE_LOCAL_CALL_CS_RETRY_REQUIRED, EXTRA_CODE_CALL_RETRY_EMERGENCY));
}

TEST_F(EmergencyStartErrorHandlerTest,
        HandleRedialsWithVoipFromRttIfSilentRedialByRttEmergencyRejectionRequired)
{
    const IMS_SINT32 ANY_REJECT_CODE = SipStatusCode::SC_486;
    SetMessageCode(ANY_REJECT_CODE);
    SetActionConfig(ANY_REJECT_CODE,
            ConfigEmergency::START_ERROR_ACTION_SILENT_REINVITE_VOIP_BY_RTT_REJECTION);

    ON_CALL(objMtcSession, GetCallType()).WillByDefault(Return(CallType::RTT));

    EXPECT_TRUE(
            CheckHandleResult(CODE_INTERNAL_REDIAL, EXTRA_CODE_REDIAL_BY_RTT_EMERGENCY_REJECTION));
}

TEST_F(EmergencyStartErrorHandlerTest, HandleDoesNotRedialWithVoipIfEmergencyCallIsNotRtt)
{
    const IMS_SINT32 ANY_REJECT_CODE = SipStatusCode::SC_486;
    SetMessageCode(ANY_REJECT_CODE);
    SetActionConfig(ANY_REJECT_CODE,
            ConfigEmergency::START_ERROR_ACTION_SILENT_REINVITE_VOIP_BY_RTT_REJECTION);

    ON_CALL(objMtcSession, GetCallType()).WillByDefault(Return(CallType::VOIP));
    EXPECT_FALSE(
            CheckHandleResult(CODE_INTERNAL_REDIAL, EXTRA_CODE_REDIAL_BY_RTT_EMERGENCY_REJECTION));
}

TEST_F(EmergencyStartErrorHandlerTest,
        HandleDoesNotRedialWithVoipFromRttIfSilentRedialByRttEmergencyRejectionIsNotRequired)
{
    const IMS_SINT32 ANY_REJECT_CODE = SipStatusCode::SC_486;
    SetMessageCode(ANY_REJECT_CODE);

    ON_CALL(objMtcSession, GetCallType()).WillByDefault(Return(CallType::RTT));

    EXPECT_FALSE(
            CheckHandleResult(CODE_INTERNAL_REDIAL, EXTRA_CODE_REDIAL_BY_RTT_EMERGENCY_REJECTION));
}

TEST_F(EmergencyStartErrorHandlerTest, HandleRedialsWithRetryAfterInSipErrorResponse)
{
    const IMS_SINT32 ANY_REJECT_CODE = SipStatusCode::SC_486;
    SetMessageCode(ANY_REJECT_CODE);
    SetActionConfig(
            ANY_REJECT_CODE, ConfigEmergency::START_ERROR_ACTION_SILENT_REINVITE_BY_RETRY_AFTER);

    const IMS_SINT32 nRetryAfterInSeconds = 10;
    AString strRetryAfterInMillis;
    strRetryAfterInMillis.SetNumber(nRetryAfterInSeconds * 1000);
    ON_CALL(objMessageUtils,
            GetHeaderValueInt(&objMessage, ISipHeader::RETRY_AFTER_ANY, AString::ConstNull()))
            .WillByDefault(Return(nRetryAfterInSeconds));

    EXPECT_TRUE(CheckHandleResult(
            CODE_INTERNAL_REDIAL, EXTRA_CODE_REDIAL_BY_RETRY_AFTER, strRetryAfterInMillis));
}

TEST_F(EmergencyStartErrorHandlerTest, HandleDoesNotRedialsWithRetryAfterInSipErrorResponse)
{
    const IMS_SINT32 ANY_REJECT_CODE = SipStatusCode::SC_486;
    SetMessageCode(ANY_REJECT_CODE);

    const IMS_SINT32 nRetryAfterInSeconds = 10;
    AString strRetryAfterInMillis;
    strRetryAfterInMillis.SetNumber(nRetryAfterInSeconds * 1000);
    ON_CALL(objMessageUtils,
            GetHeaderValueInt(&objMessage, ISipHeader::RETRY_AFTER_ANY, AString::ConstNull()))
            .WillByDefault(Return(nRetryAfterInSeconds));

    EXPECT_FALSE(CheckHandleResult(
            CODE_INTERNAL_REDIAL, EXTRA_CODE_REDIAL_BY_RETRY_AFTER, strRetryAfterInMillis));
}

TEST_F(EmergencyStartErrorHandlerTest, HandleSilentReinviteToAlternatePcscfOnceInitiatesSingleRetry)
{
    const IMS_SINT32 ANY_REJECT_CODE = SipStatusCode::SC_503;
    SetMessageCode(ANY_REJECT_CODE);
    SetActionConfig(ANY_REJECT_CODE,
            ConfigEmergency::START_ERROR_ACTION_SILENT_REINVITE_TO_ALTERNATE_PCSCF_ONCE);

    EXPECT_CALL(objAosConnector, RegisterWithNextPcscf(0)).Times(1);

    EXPECT_TRUE(CheckHandleResult(CODE_INTERNAL_REDIAL, EXTRA_CODE_REDIAL_WITH_NEXT_PCSCF_ONCE));
}

TEST_F(EmergencyStartErrorHandlerTest,
        HandleSilentReinviteToAlternatePcscfOnceReturnsCsRetryRequiredIfAosConnectorIsNull)
{
    const IMS_SINT32 ANY_REJECT_CODE = SipStatusCode::SC_503;
    SetMessageCode(ANY_REJECT_CODE);
    SetActionConfig(ANY_REJECT_CODE,
            ConfigEmergency::START_ERROR_ACTION_SILENT_REINVITE_TO_ALTERNATE_PCSCF_ONCE);
    ON_CALL(objMtcService, GetAosConnector).WillByDefault(Return(nullptr));

    EXPECT_TRUE(
            CheckHandleResult(CODE_LOCAL_CALL_CS_RETRY_REQUIRED, EXTRA_CODE_CALL_RETRY_EMERGENCY));
}

TEST_F(EmergencyStartErrorHandlerTest, HandlePcscfOnceRetryTerminationCheckIfRetryFails)
{
    const IMS_SINT32 ANY_REJECT_CODE = SipStatusCode::SC_503;
    SetMessageCode(ANY_REJECT_CODE);
    SetActionConfig(ANY_REJECT_CODE,
            ConfigEmergency::START_ERROR_ACTION_SILENT_REINVITE_TO_ALTERNATE_PCSCF_ONCE);
    ON_CALL(objCallController, GetActiveRedialHelper()).WillByDefault(Return(&objRedialHelper));
    ON_CALL(objRedialHelper, GetType())
            .WillByDefault(Return(EXTRA_CODE_REDIAL_WITH_NEXT_PCSCF_ONCE));

    EXPECT_CALL(objAosConnector, RegisterWithNextPcscf(0)).Times(0);

    EXPECT_TRUE(CheckHandleResult(CODE_SIP_SERVICE_UNAVAILABLE, SipStatusCode::SC_503));
}

TEST_F(EmergencyStartErrorHandlerTest, HandlePcscfOnceRetryTerminationCheckIfNonOnceRetryInProgress)
{
    const IMS_SINT32 ANY_REJECT_CODE = SipStatusCode::SC_503;
    SetMessageCode(ANY_REJECT_CODE);
    SetActionConfig(ANY_REJECT_CODE,
            ConfigEmergency::START_ERROR_ACTION_SILENT_REINVITE_TO_ALTERNATE_PCSCF_ONCE);
    ON_CALL(objCallController, GetActiveRedialHelper()).WillByDefault(Return(&objRedialHelper));
    ON_CALL(objRedialHelper, GetType())
            .WillByDefault(Return(EXTRA_CODE_REDIAL_EMERGENCY_WITH_ANONYMOUS));

    EXPECT_CALL(objAosConnector, RegisterWithNextPcscf(0)).Times(1);

    EXPECT_TRUE(CheckHandleResult(CODE_INTERNAL_REDIAL, EXTRA_CODE_REDIAL_WITH_NEXT_PCSCF_ONCE));
}

TEST_F(EmergencyStartErrorHandlerTest, HandleSilentReinviteToAlternatePcscfNullAosConnector)
{
    const IMS_SINT32 ANY_REJECT_CODE = SipStatusCode::SC_503;
    SetMessageCode(ANY_REJECT_CODE);
    SetActionConfig(ANY_REJECT_CODE,
            ConfigEmergency::START_ERROR_ACTION_SILENT_REINVITE_TO_ALTERNATE_PCSCF);

    ON_CALL(objMtcService, GetAosConnector).WillByDefault(Return(nullptr));

    EXPECT_CALL(objAosConnector, RegisterWithNextPcscf(_)).Times(0);
    EXPECT_TRUE(
            CheckHandleResult(CODE_LOCAL_CALL_CS_RETRY_REQUIRED, EXTRA_CODE_CALL_RETRY_EMERGENCY));
}

TEST_F(EmergencyStartErrorHandlerTest, HandleSilentReinviteToAlternatePcscfSuccess)
{
    const IMS_SINT32 ANY_REJECT_CODE = SipStatusCode::SC_503;
    SetMessageCode(ANY_REJECT_CODE);
    SetActionConfig(ANY_REJECT_CODE,
            ConfigEmergency::START_ERROR_ACTION_SILENT_REINVITE_TO_ALTERNATE_PCSCF);

    EXPECT_CALL(objAosConnector, RegisterWithNextPcscf(0)).Times(1);
    EXPECT_TRUE(CheckHandleResult(CODE_INTERNAL_REDIAL, EXTRA_CODE_REDIAL_WITH_NEXT_PCSCF));
}

TEST_F(EmergencyStartErrorHandlerTest, HandleSetsHadInviteTransactionTimeoutIfTransactionTimeout)
{
    EXPECT_CALL(objCallContext, SetHadInviteTransactionTimeout(IMS_TRUE)).Times(1);
    pHandler->Handle(nullptr);
}

TEST_F(EmergencyStartErrorHandlerTest,
        HandleDoesNotSetHadInviteTransactionTimeoutIfNotTransactionTimeout)
{
    SetMessageCode(SipStatusCode::SC_403);
    EXPECT_CALL(objCallContext, SetHadInviteTransactionTimeout(_)).Times(0);
    pHandler->Handle(&objMessage);
}

}  // namespace android
