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

#include "CallReasonInfo.h"
#include "CarrierConfig.h"
#include "Engine.h"
#include "IConfiguration.h"
#include "Ims3gpp.h"
#include "ImsAosParameter.h"
#include "ImsEventDef.h"
#include "MockIMtcImsEventReceiver.h"
#include "MockIMtcService.h"
#include "PlatformContext.h"
#include "TestConfigService.h"
#include "call/IMtcCall.h"
#include "call/MockEpsFallbackTrigger.h"
#include "call/MockIMtcCall.h"
#include "call/MockIMtcCallContext.h"
#include "call/MockIMtcCallManager.h"
#include "call/MockIMtcSession.h"
#include "call/termination/StartErrorHandler.h"
#include "configuration/MockIMtcConfigurationManager.h"
#include "configuration/MtcConfigurationProxy.h"
#include "core/MockIMessage.h"
#include "core/MockISession.h"
#include "helper/MockIMtcAosConnector.h"
#include "helper/MockIPassiveTimerHolder.h"
#include "internal/Ims3gpp.h"
#include "media/MockIMtcMediaManager.h"
#include "sipcore/ISipHeader.h"
#include "sipcore/SipStatusCode.h"
#include "util/ByteArray.h"
#include "utility/MockIMessageUtils.h"
#include <gtest/gtest.h>

using ::testing::_;
using ::testing::Return;
using ::testing::ReturnRef;

namespace android
{

class StartErrorHandlerTest : public ::testing::Test
{
public:
    MockIMtcCallContext objCallContext;
    MockIMtcService objMtcService;
    MockIMtcAosConnector objAosConnector;
    MockIMessage objMessage;
    MockIMtcConfigurationManager* pConfigurationManager;
    MtcConfigurationProxy* pConfigurationProxy;
    CallInfo objCallInfo;
    MockIMessageUtils objMessageUtils;
    MockIMtcSession objMtcSession;
    MockISession objSession;
    MockIMtcImsEventReceiver objImsEventReceiver;
    Ims3gppData objIms3gppData;
    TestConfigService* m_pConfigService;
    MockIMtcCallManager objCallManager;

    StartErrorHandler* pHandler;

protected:
    virtual void SetUp() override
    {
        m_pConfigService = new TestConfigService();
        m_pConfigService->SetCarrierConfig(&(m_pConfigService->GetMockCarrierConfig()));
        PlatformContext::GetInstance()->SetService(
                PlatformContext::SERVICE_CONFIG, m_pConfigService);

        ON_CALL(objCallContext, GetMessageUtils).WillByDefault(ReturnRef(objMessageUtils));
        ON_CALL(objMessageUtils, GetCauseFromReasonHeader).WillByDefault(Return(-1));
        ON_CALL(objCallContext, GetService).WillByDefault(ReturnRef(objMtcService));
        ON_CALL(objMtcService, GetAosConnector).WillByDefault(Return(&objAosConnector));

        pConfigurationManager = new MockIMtcConfigurationManager();
        pConfigurationProxy = new MtcConfigurationProxy(pConfigurationManager);
        ON_CALL(objCallContext, GetConfigurationProxy)
                .WillByDefault(ReturnRef(*pConfigurationProxy));

        ON_CALL(objCallContext, GetCallInfo).WillByDefault(ReturnRef(objCallInfo));
        ON_CALL(objCallContext, GetSession()).WillByDefault(Return(&objMtcSession));
        ON_CALL(objMtcSession, GetISession()).WillByDefault(ReturnRef(objSession));

        ON_CALL(objCallContext, GetImsEventReceiver).WillByDefault(ReturnRef(objImsEventReceiver));
        ON_CALL(objImsEventReceiver, GetWParam(IMS_EVENT_ROAMING_STATE))
                .WillByDefault(Return(IMS_ROAMING_STATE_OFF));
        ON_CALL(objImsEventReceiver, GetWParam(IMS_EVENT_LTE_INFO))
                .WillByDefault(Return(IMS_LTE_INFO_COMBINED_ATTACHED));

        ON_CALL(objMessage, GetReasonPhrase()).WillByDefault(ReturnRef(AString::ConstNull()));
        ON_CALL(objCallContext, GetCallManager).WillByDefault(ReturnRef(objCallManager));

        pHandler = new StartErrorHandler(objCallContext, objSession);
    }

    virtual void TearDown() override
    {
        PlatformContext::GetInstance()->SetService(PlatformContext::SERVICE_CONFIG, IMS_NULL);

        delete m_pConfigService;
        delete pConfigurationProxy;
        delete pHandler;
    }

    void SetMessageCode(IN IMS_SINT32 nStatusCode)
    {
        ON_CALL(objMessage, GetStatusCode).WillByDefault(Return(nStatusCode));
    }

    void SetCsfbConfig(IN IMS_SINT32 nStatusCode)
    {
        ON_CALL(*pConfigurationManager, IsRejectCodeForCsfb(nStatusCode))
                .WillByDefault(Return(IMS_TRUE));
    }

    void SetTcallTimerConfig(IN IMS_SINT32 nPolicy)
    {
        ON_CALL(*pConfigurationManager, GetPolicyForTcallTimerExpiryOfVolteCall)
                .WillByDefault(Return(nPolicy));
        ON_CALL(*pConfigurationManager, GetPolicyForTcallTimerExpiryOfVowifiCall)
                .WillByDefault(Return(nPolicy));
    }

    void SetUp504RegRestoration(IN IMS_SINT32 nPolicy)
    {
        SetMessageCode(SipStatusCode::SC_504);

        AString strPathHeader("sip:anyPath");
        ON_CALL(objAosConnector, GetPathHeaderValue).WillByDefault(Return(AString(strPathHeader)));
        AString strServiceRoute("sip:anyServiceRoute");
        ON_CALL(objAosConnector, GetServiceRouteHeaderValue)
                .WillByDefault(Return(AString(strServiceRoute)));

        ON_CALL(objMessageUtils, ContainsAddressInPaid(&objMessage, strPathHeader))
                .WillByDefault(Return(IMS_FALSE));
        ON_CALL(objMessageUtils, ContainsAddressInPaid(&objMessage, strServiceRoute))
                .WillByDefault(Return(IMS_TRUE));

        objIms3gppData.eType = Ims3gpp::TYPE_ALTERNATIVE_SERVICE;
        objIms3gppData.eAlternativeServiceType = Ims3gpp::AlternativeService::TYPE_RESTORATION;
        objIms3gppData.eAlternativeServiceAction =
                Ims3gpp::AlternativeService::ACTION_INITIAL_REGISTRATION;
        ON_CALL(objMessageUtils, GetIms3gppData(&objMessage)).WillByDefault(Return(objIms3gppData));

        ON_CALL(*pConfigurationManager, GetRegistrationRestorationModeOn504ForInvite)
                .WillByDefault(Return(nPolicy));
    }

    IMS_BOOL CheckHandleResult(IN IMS_SINT32 nCode)
    {
        CallReasonInfo objResult = pHandler->Handle(&objMessage);
        return objResult == CallReasonInfo(nCode);
    }

    IMS_BOOL CheckHandleResult(IN IMS_SINT32 nCode, IN IMS_SINT32 nExtraCode)
    {
        CallReasonInfo objResult = pHandler->Handle(&objMessage);
        return objResult == CallReasonInfo(nCode, nExtraCode);
    }

    IMS_BOOL CheckHandleResult(
            IN IMS_SINT32 nCode, IN IMS_SINT32 nExtraCode, IN const AString& strExtraMessage)
    {
        CallReasonInfo objResult = pHandler->Handle(&objMessage);
        return objResult == CallReasonInfo(nCode, nExtraCode, strExtraMessage);
    }
};

TEST_F(StartErrorHandlerTest, HandleReturnsNetworkNoResponseByTransactionTimeoutOfEcc)
{
    objCallInfo.bEmergency = IMS_TRUE;
    SetMessageCode(SipStatusCode::SC_INVALID);

    EXPECT_CALL(*pConfigurationManager, IsRetryEmergencyCallOverEmergencyPdnWithNextPcscf())
            .Times(2)
            .WillOnce(Return(IMS_FALSE))
            .WillOnce(Return(IMS_TRUE));
    EXPECT_CALL(objMtcService, IsEmergency()).Times(1).WillOnce(Return(IMS_FALSE));

    EXPECT_TRUE(CheckHandleResult(CODE_NETWORK_RESP_TIMEOUT, EXTRA_CODE_METHOD_INVITE));
    EXPECT_TRUE(CheckHandleResult(CODE_NETWORK_RESP_TIMEOUT, EXTRA_CODE_METHOD_INVITE));
}

TEST_F(StartErrorHandlerTest, HandleReturnsNetworkNoResponseByNullIMessageOfEcc)
{
    objCallInfo.bEmergency = IMS_TRUE;
    ON_CALL(*pConfigurationManager, IsRetryEmergencyCallOverEmergencyPdnWithNextPcscf())
            .WillByDefault(Return(IMS_TRUE));
    ON_CALL(objMtcService, IsEmergency()).WillByDefault(Return(IMS_TRUE));
    ON_CALL(objMessageUtils, GetNumberOfPreviousResponses(&objSession, IMessage::SESSION_START))
            .WillByDefault(Return(2));

    CallReasonInfo objResult = pHandler->Handle(IMS_NULL);
    EXPECT_TRUE(objResult == CallReasonInfo(CODE_NETWORK_RESP_TIMEOUT, EXTRA_CODE_METHOD_INVITE));
}

TEST_F(StartErrorHandlerTest, HandleTransactionTimeoutInVoLte)
{
    SetMessageCode(SipStatusCode::SC_INVALID);
    ON_CALL(objMtcService, IsWlanIpCanType).WillByDefault(Return(IMS_FALSE));

    SetTcallTimerConfig(CarrierConfig::ImsVoice::MO_CALL_REQUEST_TIMEOUT_POLICY_CALL_END);
    EXPECT_CALL(objAosConnector, Control(_)).Times(0);
    EXPECT_TRUE(CheckHandleResult(CODE_NETWORK_RESP_TIMEOUT, EXTRA_CODE_METHOD_INVITE));

    SetTcallTimerConfig(CarrierConfig::ImsVoice::MO_CALL_REQUEST_TIMEOUT_POLICY_WAIT_FOR_RESPONSE);
    EXPECT_CALL(objAosConnector, Control(ImsAosControl::PCSCF_NEXT)).Times(1);
    EXPECT_TRUE(CheckHandleResult(CODE_NETWORK_RESP_TIMEOUT, EXTRA_CODE_METHOD_INVITE));

    SetTcallTimerConfig(CarrierConfig::ImsVoice::MO_CALL_REQUEST_TIMEOUT_POLICY_CSFB);
    EXPECT_CALL(objAosConnector, Control(_)).Times(0);
    EXPECT_TRUE(CheckHandleResult(
            CODE_LOCAL_CALL_CS_RETRY_REQUIRED, EXTRA_CODE_CALL_RETRY_SILENT_REDIAL));

    SetTcallTimerConfig(CarrierConfig::ImsVoice::MO_CALL_REQUEST_TIMEOUT_POLICY_CSFB_IF_AVAILABLE);
    EXPECT_CALL(objAosConnector, Control(_)).Times(0);
    EXPECT_TRUE(CheckHandleResult(
            CODE_LOCAL_CALL_CS_RETRY_REQUIRED, EXTRA_CODE_CALL_RETRY_SILENT_REDIAL));

    SetTcallTimerConfig(
            CarrierConfig::ImsVoice::MO_CALL_REQUEST_TIMEOUT_POLICY_INITIAL_REGISTER_CURRENT_PCSCF);
    EXPECT_CALL(objAosConnector, Control(ImsAosControl::REGISTER_REINITIATE)).Times(1);
    EXPECT_TRUE(CheckHandleResult(CODE_NETWORK_RESP_TIMEOUT, EXTRA_CODE_METHOD_INVITE));

    SetTcallTimerConfig(
            CarrierConfig::ImsVoice::MO_CALL_REQUEST_TIMEOUT_POLICY_INITIAL_REGISTER_NEXT_PCSCF);
    EXPECT_CALL(objAosConnector, Control(ImsAosControl::PCSCF_NEXT)).Times(1);
    EXPECT_TRUE(CheckHandleResult(CODE_NETWORK_RESP_TIMEOUT, EXTRA_CODE_METHOD_INVITE));

    SetTcallTimerConfig(CarrierConfig::ImsVoice::
                    MO_CALL_REQUEST_TIMEOUT_POLICY_INITIAL_REGISTER_WITH_PDN_RECONNECT_AFTER_CSFB);
    EXPECT_CALL(objAosConnector, Control(ImsAosControl::REGISTER_REINITIATE_BY_CSFB)).Times(1);
    EXPECT_TRUE(CheckHandleResult(
            CODE_LOCAL_CALL_CS_RETRY_REQUIRED, EXTRA_CODE_CALL_RETRY_SILENT_REDIAL));
}

TEST_F(StartErrorHandlerTest, HandleTransactionTimeoutInVoWiFi)
{
    SetMessageCode(SipStatusCode::SC_INVALID);
    ON_CALL(objMtcService, IsWlanIpCanType).WillByDefault(Return(IMS_TRUE));

    SetTcallTimerConfig(CarrierConfig::ImsVoice::MO_CALL_REQUEST_TIMEOUT_POLICY_CALL_END);
    EXPECT_CALL(objAosConnector, Control(_)).Times(0);
    EXPECT_TRUE(CheckHandleResult(CODE_NETWORK_RESP_TIMEOUT, EXTRA_CODE_METHOD_INVITE));

    SetTcallTimerConfig(CarrierConfig::ImsVoice::MO_CALL_REQUEST_TIMEOUT_POLICY_WAIT_FOR_RESPONSE);
    EXPECT_CALL(objAosConnector, Control(ImsAosControl::PCSCF_NEXT)).Times(1);
    EXPECT_TRUE(CheckHandleResult(CODE_NETWORK_RESP_TIMEOUT, EXTRA_CODE_METHOD_INVITE));

    SetTcallTimerConfig(CarrierConfig::ImsVoice::MO_CALL_REQUEST_TIMEOUT_POLICY_CSFB);
    EXPECT_CALL(objAosConnector, Control(_)).Times(0);
    EXPECT_TRUE(CheckHandleResult(
            CODE_LOCAL_CALL_CS_RETRY_REQUIRED, EXTRA_CODE_CALL_RETRY_SILENT_REDIAL));

    SetTcallTimerConfig(CarrierConfig::ImsVoice::MO_CALL_REQUEST_TIMEOUT_POLICY_CSFB_IF_AVAILABLE);
    EXPECT_CALL(objAosConnector, Control(_)).Times(0);
    EXPECT_TRUE(CheckHandleResult(
            CODE_LOCAL_CALL_CS_RETRY_REQUIRED, EXTRA_CODE_CALL_RETRY_SILENT_REDIAL));

    SetTcallTimerConfig(
            CarrierConfig::ImsVoice::MO_CALL_REQUEST_TIMEOUT_POLICY_INITIAL_REGISTER_CURRENT_PCSCF);
    EXPECT_CALL(objAosConnector, Control(ImsAosControl::REGISTER_REINITIATE)).Times(1);
    EXPECT_TRUE(CheckHandleResult(CODE_NETWORK_RESP_TIMEOUT, EXTRA_CODE_METHOD_INVITE));

    SetTcallTimerConfig(
            CarrierConfig::ImsVoice::MO_CALL_REQUEST_TIMEOUT_POLICY_INITIAL_REGISTER_NEXT_PCSCF);
    EXPECT_CALL(objAosConnector, Control(ImsAosControl::PCSCF_NEXT)).Times(1);
    EXPECT_TRUE(CheckHandleResult(CODE_NETWORK_RESP_TIMEOUT, EXTRA_CODE_METHOD_INVITE));

    SetTcallTimerConfig(CarrierConfig::ImsVoice::
                    MO_CALL_REQUEST_TIMEOUT_POLICY_INITIAL_REGISTER_WITH_PDN_RECONNECT_AFTER_CSFB);
    EXPECT_CALL(objAosConnector, Control(ImsAosControl::REGISTER_REINITIATE_BY_CSFB)).Times(1);
    EXPECT_TRUE(CheckHandleResult(
            CODE_LOCAL_CALL_CS_RETRY_REQUIRED, EXTRA_CODE_CALL_RETRY_SILENT_REDIAL));

    SetTcallTimerConfig(
            CarrierConfig::ImsVoice::MO_CALL_REQUEST_TIMEOUT_POLICY_REDIAL_BY_NETWORK_CONTEXT);
    ON_CALL(*pConfigurationManager, IsRequiredCdmalessFeatureTag).WillByDefault(Return(IMS_TRUE));
    ON_CALL(objMtcService, IsWlanIpCanType).WillByDefault(Return(IMS_TRUE));
    EXPECT_CALL(objAosConnector, Control(ImsAosControl::REGISTER_REINITIATE)).Times(1);
    EXPECT_TRUE(CheckHandleResult(CODE_NETWORK_RESP_TIMEOUT, EXTRA_CODE_METHOD_INVITE));
}

TEST_F(StartErrorHandlerTest, HandleTransactionTimeoutForEpsfb)
{
    SetMessageCode(SipStatusCode::SC_INVALID);
    ON_CALL(objMtcService, IsWlanIpCanType).WillByDefault(Return(IMS_FALSE));

    SetTcallTimerConfig(
            CarrierConfig::ImsVoice::MO_CALL_REQUEST_TIMEOUT_POLICY_REDIAL_BY_NETWORK_CONTEXT);
    ON_CALL(*pConfigurationManager, GetEpsFallbackWatchdogTime).WillByDefault(Return(-1));
    EXPECT_TRUE(CheckHandleResult(
            CODE_LOCAL_CALL_CS_RETRY_REQUIRED, EXTRA_CODE_CALL_RETRY_SILENT_REDIAL));

    ON_CALL(*pConfigurationManager, GetEpsFallbackWatchdogTime).WillByDefault(Return(6000));
    MockEpsFallbackTrigger objEpsFbTrigger(objCallContext);
    ON_CALL(objCallContext, GetEpsFallbackTrigger).WillByDefault(ReturnRef(objEpsFbTrigger));
    ON_CALL(objMtcService, IsNr).WillByDefault(Return(IMS_FALSE));
    EXPECT_TRUE(CheckHandleResult(
            CODE_LOCAL_CALL_CS_RETRY_REQUIRED, EXTRA_CODE_CALL_RETRY_SILENT_REDIAL));

    ON_CALL(*pConfigurationManager, IsRequiredCdmalessFeatureTag).WillByDefault(Return(IMS_TRUE));
    EXPECT_CALL(objAosConnector, Control(ImsAosControl::REGISTER_REINITIATE)).Times(1);
    EXPECT_TRUE(CheckHandleResult(CODE_NETWORK_RESP_TIMEOUT, EXTRA_CODE_METHOD_INVITE));

    ON_CALL(objImsEventReceiver, GetWParam(IMS_EVENT_LTE_INFO))
            .WillByDefault(Return(IMS_LTE_INFO_EPS_ONLY_ATTACHED));
    EXPECT_CALL(objAosConnector, Control(ImsAosControl::REGISTER_REINITIATE)).Times(1);
    EXPECT_TRUE(CheckHandleResult(CODE_NETWORK_RESP_TIMEOUT, EXTRA_CODE_METHOD_INVITE));

    ON_CALL(objImsEventReceiver, GetWParam(IMS_EVENT_LTE_INFO))
            .WillByDefault(Return(IMS_LTE_INFO_COMBINED_ATTACHED));

    ON_CALL(objImsEventReceiver, GetWParam(IMS_EVENT_ROAMING_STATE))
            .WillByDefault(Return(IMS_ROAMING_STATE_ON));
    EXPECT_CALL(objAosConnector, Control(ImsAosControl::REGISTER_REINITIATE_BY_CSFB)).Times(1);
    EXPECT_TRUE(CheckHandleResult(
            CODE_LOCAL_CALL_CS_RETRY_REQUIRED, EXTRA_CODE_CALL_RETRY_SILENT_REDIAL));

    ON_CALL(objMtcService, IsNr).WillByDefault(Return(IMS_TRUE));
    EXPECT_CALL(objAosConnector, Control(_)).Times(0);
    EXPECT_TRUE(CheckHandleResult(CODE_INTERNAL_REDIAL, EXTRA_CODE_REDIAL_AFTER_EPS_FALLBACK));
}

TEST_F(StartErrorHandlerTest, HandleReturnsCsfbIfStatusCodeIsIncludedInCsfbConfiguration)
{
    const IMS_SINT32 ANY_REJECT_CODE = SipStatusCode::SC_408;
    SetMessageCode(ANY_REJECT_CODE);
    SetCsfbConfig(ANY_REJECT_CODE);

    EXPECT_TRUE(CheckHandleResult(
            CODE_LOCAL_CALL_CS_RETRY_REQUIRED, EXTRA_CODE_CALL_RETRY_SILENT_REDIAL));
}

TEST_F(StartErrorHandlerTest, HandleReturnsRedialEmergencyWithNextPcscf)
{
    SetMessageCode(SipStatusCode::SC_600);
    objCallInfo.bEmergency = IMS_TRUE;
    ON_CALL(*pConfigurationManager, IsRetryEmergencyCallOverEmergencyPdnWithNextPcscf())
            .WillByDefault(Return(IMS_TRUE));
    ON_CALL(objMtcService, IsEmergency()).WillByDefault(Return(IMS_TRUE));
    ON_CALL(objMessageUtils, GetNumberOfPreviousResponses(&objSession, IMessage::SESSION_START))
            .WillByDefault(Return(1));

    EXPECT_CALL(objAosConnector, Control(ImsAosControl::E_REGISTER_FAKE_WITH_NEXT_PCSCF)).Times(1);
    EXPECT_TRUE(
            CheckHandleResult(CODE_INTERNAL_REDIAL, EXTRA_CODE_REDIAL_EMERGENCY_WITH_NEXT_PCSCF));
}

TEST_F(StartErrorHandlerTest, HandleRedirectionBy3xxResponses)
{
    const IMS_SINT32 ANY_REJECT_CODE = SipStatusCode::SC_300;
    SetMessageCode(ANY_REJECT_CODE);

    AString strAnyContactUri("sip:anyContactUri");
    ON_CALL(objMessageUtils, GetHeaderValue(&objMessage, ISipHeader::CONTACT_NORMAL, _))
            .WillByDefault(Return(strAnyContactUri));
    EXPECT_TRUE(CheckHandleResult(
            CODE_INTERNAL_REDIAL, EXTRA_CODE_REDIAL_FOR_REDIRECTION, strAnyContactUri));

    ON_CALL(objMessageUtils, GetHeaderValue(&objMessage, ISipHeader::CONTACT_NORMAL, _))
            .WillByDefault(Return(""));
    EXPECT_TRUE(CheckHandleResult(CODE_SIP_REDIRECTED, ANY_REJECT_CODE));

    SetCsfbConfig(ANY_REJECT_CODE);
    EXPECT_TRUE(CheckHandleResult(
            CODE_LOCAL_CALL_CS_RETRY_REQUIRED, EXTRA_CODE_CALL_RETRY_SILENT_REDIAL));
}

TEST_F(StartErrorHandlerTest, Handle380Response)
{
    SetMessageCode(SipStatusCode::SC_380);

    ON_CALL(objMessageUtils, GetSosTypeFromServiceUrn(&objMessage, ISipHeader::CONTACT_NORMAL, _))
            .WillByDefault(Return(EXTRA_CODE_EMERGENCYSERVICE_INVALID));

    // Ims3gppData no AlternativeService TYPE_EMERGENCY
    Ims3gppData objIms3gppData;
    ON_CALL(objMessageUtils, GetIms3gppData(&objMessage)).WillByDefault(Return(objIms3gppData));
    EXPECT_TRUE(CheckHandleResult(CODE_SIP_REDIRECTED, SipStatusCode::SC_380));

    // Ims3gppData no AlternativeService TYPE_RESTORATION
    objIms3gppData.eType = Ims3gpp::TYPE_ALTERNATIVE_SERVICE;
    objIms3gppData.eAlternativeServiceType = Ims3gpp::AlternativeService::TYPE_RESTORATION;
    ON_CALL(objMessageUtils, GetIms3gppData(&objMessage)).WillByDefault(Return(objIms3gppData));
    EXPECT_TRUE(CheckHandleResult(CODE_SIP_REDIRECTED, SipStatusCode::SC_380));

    SetCsfbConfig(SipStatusCode::SC_380);
    EXPECT_TRUE(CheckHandleResult(
            CODE_LOCAL_CALL_CS_RETRY_REQUIRED, EXTRA_CODE_CALL_RETRY_SILENT_REDIAL));
}

TEST_F(StartErrorHandlerTest, Handle380ResponseForEmergencyCall)
{
    SetMessageCode(SipStatusCode::SC_380);
    objCallInfo.bEmergency = IMS_TRUE;
    ON_CALL(*pConfigurationManager, IsRetryEmergencyCallOverEmergencyPdnWithNextPcscf())
            .WillByDefault(Return(IMS_TRUE));
    ON_CALL(objMtcService, IsEmergency()).WillByDefault(Return(IMS_TRUE));

    EXPECT_TRUE(CheckHandleResult(CODE_SIP_REDIRECTED, SipStatusCode::SC_380));
}

TEST_F(StartErrorHandlerTest, Handle380ResponseWithUeUnDetectableEmergencyCall)
{
    SetMessageCode(SipStatusCode::SC_380);

    IMS_SINT32 nCategoryInContact = EXTRA_CODE_EMERGENCYSERVICE_POLICE;
    // by configuration
    ON_CALL(objMessageUtils, GetSosTypeFromServiceUrn(&objMessage, ISipHeader::CONTACT_NORMAL, _))
            .WillByDefault(Return(nCategoryInContact));
    ON_CALL(*pConfigurationManager,
            IsEmergencyRetryWithoutChecking380ContentForNonUeDetectableEmergencyCall)
            .WillByDefault(Return(IMS_TRUE));
    EXPECT_TRUE(CheckHandleResult(
            CODE_SIP_ALTERNATE_EMERGENCY_CALL, EXTRA_CODE_EMERGENCYSERVICE_POLICE));

    // no Non UE Detectable ECC but contains AlternativeService TYPE_EMERGENCY
    ON_CALL(*pConfigurationManager,
            IsEmergencyRetryWithoutChecking380ContentForNonUeDetectableEmergencyCall)
            .WillByDefault(Return(IMS_FALSE));

    Ims3gppData objIms3gppData;
    objIms3gppData.eType = Ims3gpp::TYPE_ALTERNATIVE_SERVICE;
    objIms3gppData.eAlternativeServiceType = Ims3gpp::AlternativeService::TYPE_EMERGENCY;
    ON_CALL(objMessageUtils, GetIms3gppData(&objMessage)).WillByDefault(Return(objIms3gppData));

    // Non UE Detectable : No path feature tag (empty tag)
    ON_CALL(objAosConnector, GetSupportedHeaderValue).WillByDefault(Return(AString()));
    EXPECT_TRUE(CheckHandleResult(CODE_SIP_ALTERNATE_EMERGENCY_CALL, nCategoryInContact));

    // Non UE Detectable : No path feature tag (other tag)
    ON_CALL(objAosConnector, GetSupportedHeaderValue).WillByDefault(Return(AString("any")));
    EXPECT_TRUE(CheckHandleResult(CODE_SIP_ALTERNATE_EMERGENCY_CALL, nCategoryInContact));

    // no Non UE Detectable : path feature tag exists but Path header value is different
    ON_CALL(objAosConnector, GetSupportedHeaderValue).WillByDefault(Return(AString("path")));
    AString strAnyPath("sip:anyPath");
    ON_CALL(objAosConnector, GetPathHeaderValue).WillByDefault(Return(AString(strAnyPath)));
    ON_CALL(objMessageUtils, ContainsAddressInPaid(&objMessage, strAnyPath))
            .WillByDefault(Return(IMS_FALSE));
    EXPECT_TRUE(CheckHandleResult(
            CODE_SIP_ALTERNATE_EMERGENCY_CALL, EXTRA_CODE_EMERGENCYSERVICE_GENERIC));

    // Non UE Detectable
    ON_CALL(objMessageUtils, ContainsAddressInPaid(&objMessage, strAnyPath))
            .WillByDefault(Return(IMS_TRUE));
    EXPECT_TRUE(CheckHandleResult(CODE_SIP_ALTERNATE_EMERGENCY_CALL, nCategoryInContact));
}

TEST_F(StartErrorHandlerTest, Handle4xxResponses)
{
    SetMessageCode(SipStatusCode::SC_400);
    EXPECT_TRUE(CheckHandleResult(CODE_SIP_BAD_REQUEST, SipStatusCode::SC_400));

    SetMessageCode(SipStatusCode::SC_401);
    EXPECT_TRUE(CheckHandleResult(CODE_SIP_CLIENT_ERROR, SipStatusCode::SC_401));

    SetMessageCode(SipStatusCode::SC_405);
    EXPECT_TRUE(CheckHandleResult(CODE_SIP_METHOD_NOT_ALLOWED, SipStatusCode::SC_405));

    SetMessageCode(SipStatusCode::SC_406);
    EXPECT_TRUE(CheckHandleResult(CODE_SIP_NOT_ACCEPTABLE, SipStatusCode::SC_406));

    SetMessageCode(SipStatusCode::SC_408);
    EXPECT_TRUE(CheckHandleResult(CODE_SIP_REQUEST_TIMEOUT, SipStatusCode::SC_408));

    SetMessageCode(SipStatusCode::SC_410);
    EXPECT_TRUE(CheckHandleResult(CODE_SIP_NOT_REACHABLE, SipStatusCode::SC_410));

    SetMessageCode(SipStatusCode::SC_413);
    EXPECT_TRUE(CheckHandleResult(CODE_SIP_REQUEST_ENTITY_TOO_LARGE, SipStatusCode::SC_413));

    SetMessageCode(SipStatusCode::SC_414);
    EXPECT_TRUE(CheckHandleResult(CODE_SIP_REQUEST_URI_TOO_LARGE, SipStatusCode::SC_414));

    SetMessageCode(SipStatusCode::SC_415);
    EXPECT_TRUE(CheckHandleResult(CODE_SIP_NOT_SUPPORTED, SipStatusCode::SC_415));

    SetMessageCode(SipStatusCode::SC_416);
    EXPECT_TRUE(CheckHandleResult(CODE_SIP_NOT_SUPPORTED, SipStatusCode::SC_416));

    SetMessageCode(SipStatusCode::SC_420);
    EXPECT_TRUE(CheckHandleResult(CODE_SIP_NOT_SUPPORTED, SipStatusCode::SC_420));

    SetMessageCode(SipStatusCode::SC_421);
    EXPECT_TRUE(CheckHandleResult(CODE_SIP_EXTENSION_REQUIRED, SipStatusCode::SC_421));

    SetMessageCode(SipStatusCode::SC_422);
    EXPECT_TRUE(CheckHandleResult(CODE_SIP_INTERVAL_TOO_BRIEF, SipStatusCode::SC_422));

    SetMessageCode(SipStatusCode::SC_480);
    EXPECT_TRUE(CheckHandleResult(CODE_SIP_TEMPRARILY_UNAVAILABLE, SipStatusCode::SC_480));

    SetMessageCode(SipStatusCode::SC_481);
    EXPECT_TRUE(CheckHandleResult(CODE_SIP_TRANSACTION_DOES_NOT_EXIST, SipStatusCode::SC_481));

    SetMessageCode(SipStatusCode::SC_482);
    EXPECT_TRUE(CheckHandleResult(CODE_SIP_LOOP_DETECTED, SipStatusCode::SC_482));

    SetMessageCode(SipStatusCode::SC_483);
    EXPECT_TRUE(CheckHandleResult(CODE_SIP_TOO_MANY_HOPS, SipStatusCode::SC_483));

    SetMessageCode(SipStatusCode::SC_484);
    EXPECT_TRUE(CheckHandleResult(CODE_SIP_BAD_ADDRESS, SipStatusCode::SC_484));

    SetMessageCode(SipStatusCode::SC_485);
    EXPECT_TRUE(CheckHandleResult(CODE_SIP_AMBIGUOUS, SipStatusCode::SC_485));

    SetMessageCode(SipStatusCode::SC_486);
    EXPECT_TRUE(CheckHandleResult(CODE_SIP_BUSY, SipStatusCode::SC_486));

    SetMessageCode(SipStatusCode::SC_487);
    EXPECT_TRUE(CheckHandleResult(CODE_SIP_REQUEST_CANCELLED, SipStatusCode::SC_487));

    SetMessageCode(SipStatusCode::SC_491);
    EXPECT_TRUE(CheckHandleResult(CODE_SIP_REQUEST_PENDING, SipStatusCode::SC_491));

    SetMessageCode(SipStatusCode::SC_493);
    EXPECT_TRUE(CheckHandleResult(CODE_SIP_UNDECIPHERABLE, SipStatusCode::SC_493));

    SetMessageCode(SipStatusCode::SC_499);
    EXPECT_TRUE(CheckHandleResult(CODE_SIP_NOT_REACHABLE, SipStatusCode::SC_499));

    const IMS_SINT32 ANY_REJECT_CODE = SipStatusCode::SC_412;
    SetMessageCode(ANY_REJECT_CODE);
    EXPECT_TRUE(CheckHandleResult(CODE_SIP_SERVER_ERROR, ANY_REJECT_CODE));
}

TEST_F(StartErrorHandlerTest, Handle403Response)
{
    SetMessageCode(SipStatusCode::SC_403);

    // SIP_403_POLICY_TERMINATE_CALL case
    ON_CALL(*pConfigurationManager, GetPolicyFor403ResponseForInvite)
            .WillByDefault(Return(CarrierConfig::ImsVoice::SIP_403_POLICY_TERMINATE_CALL));
    EXPECT_CALL(objAosConnector, Control(_)).Times(0);
    EXPECT_TRUE(CheckHandleResult(CODE_SIP_FORBIDDEN, SipStatusCode::SC_403));

    // SIP_403_POLICY_TERMINATE_CALL_AND_RECOVER_REGISTRATION case
    ON_CALL(*pConfigurationManager, GetPolicyFor403ResponseForInvite)
            .WillByDefault(Return(CarrierConfig::ImsVoice::
                            SIP_403_POLICY_TERMINATE_CALL_AND_RECOVER_REGISTRATION));
    EXPECT_CALL(objAosConnector, Control(ImsAosControl::REGISTER_REINITIATE)).Times(1);
    EXPECT_TRUE(CheckHandleResult(CODE_SIP_FORBIDDEN, SipStatusCode::SC_403));

    // SIP_403_POLICY_TERMINATE_CALL_AND_REFRESH_REGISTRATION case
    ON_CALL(*pConfigurationManager, GetPolicyFor403ResponseForInvite)
            .WillByDefault(Return(CarrierConfig::ImsVoice::
                            SIP_403_POLICY_TERMINATE_CALL_AND_REFRESH_REGISTRATION));
    EXPECT_CALL(objAosConnector, Control(ImsAosControl::REGISTER_REFRESH)).Times(1);
    EXPECT_TRUE(CheckHandleResult(CODE_SIP_FORBIDDEN, SipStatusCode::SC_403));

    // SIP_403_POLICY_CSFB case
    ON_CALL(*pConfigurationManager, GetPolicyFor403ResponseForInvite)
            .WillByDefault(Return(CarrierConfig::ImsVoice::SIP_403_POLICY_CSFB));
    EXPECT_CALL(objAosConnector, Control(_)).Times(0);
    EXPECT_TRUE(CheckHandleResult(
            CODE_LOCAL_CALL_CS_RETRY_REQUIRED, EXTRA_CODE_CALL_RETRY_SILENT_REDIAL));

    // SIP_403_POLICY_CSFB_AND_RECOVER_REGISTRATION case
    ON_CALL(*pConfigurationManager, GetPolicyFor403ResponseForInvite)
            .WillByDefault(
                    Return(CarrierConfig::ImsVoice::SIP_403_POLICY_CSFB_AND_RECOVER_REGISTRATION));
    EXPECT_CALL(objAosConnector, Control(ImsAosControl::REGISTER_REINITIATE_BY_CSFB)).Times(1);
    EXPECT_TRUE(CheckHandleResult(
            CODE_LOCAL_CALL_CS_RETRY_REQUIRED, EXTRA_CODE_CALL_RETRY_SILENT_REDIAL));
}

TEST_F(StartErrorHandlerTest, Handle403ResponseForEmergencyCall)
{
    SetMessageCode(SipStatusCode::SC_403);
    objCallInfo.bEmergency = IMS_TRUE;
    ON_CALL(*pConfigurationManager, IsRetryEmergencyCallOverEmergencyPdnWithNextPcscf())
            .WillByDefault(Return(IMS_TRUE));
    ON_CALL(objMtcService, IsEmergency()).WillByDefault(Return(IMS_TRUE));
    ON_CALL(objMessageUtils, GetNumberOfPreviousResponses(&objSession, IMessage::SESSION_START))
            .WillByDefault(Return(2));

    EXPECT_TRUE(CheckHandleResult(CODE_SIP_FORBIDDEN, SipStatusCode::SC_403));
}

TEST_F(StartErrorHandlerTest, Handle403ResponseForMaxCallLimitInReasonHeader)
{
    SetMessageCode(SipStatusCode::SC_403);
    ReasonHeaderValue objValue;
    objValue.strText = "Simultaneous Call Limit Has Already Been Reached";
    ON_CALL(objMessageUtils, GetCauseAndTextFromReasonHeader(&objMessage, _))
            .WillByDefault(Return(objValue));

    EXPECT_TRUE(CheckHandleResult(CODE_MAXIMUM_NUMBER_OF_CALLS_REACHED));
}

TEST_F(StartErrorHandlerTest, Handle403ResponseForMaxCallLimitInReasonPhrase)
{
    SetMessageCode(SipStatusCode::SC_403);
    ReasonHeaderValue objValue;
    ON_CALL(objMessageUtils, GetCauseAndTextFromReasonHeader(&objMessage, _))
            .WillByDefault(Return(objValue));

    AString strReasonPhrase = "Simultaneous Call Limit Has Already Been Reached";
    ON_CALL(objMessage, GetReasonPhrase()).WillByDefault(ReturnRef(strReasonPhrase));

    EXPECT_TRUE(CheckHandleResult(CODE_MAXIMUM_NUMBER_OF_CALLS_REACHED));
}

TEST_F(StartErrorHandlerTest, Handle404Response)
{
    SetMessageCode(SipStatusCode::SC_404);
    EXPECT_TRUE(CheckHandleResult(CODE_SIP_NOT_FOUND, SipStatusCode::SC_404));

    objCallInfo.bUssi = IMS_TRUE;
    EXPECT_TRUE(CheckHandleResult(
            CODE_LOCAL_CALL_CS_RETRY_REQUIRED, EXTRA_CODE_CALL_RETRY_SILENT_REDIAL));
}

TEST_F(StartErrorHandlerTest, Handle407Response)
{
    SetMessageCode(SipStatusCode::SC_407);
    EXPECT_TRUE(CheckHandleResult(CODE_SIP_PROXY_AUTHENTICATION_REQUIRED, SipStatusCode::SC_407));
}

TEST_F(StartErrorHandlerTest, Handle488Response)
{
    SetMessageCode(SipStatusCode::SC_488);

    // 1. SDP body exists and get the supported media type from it.
    ON_CALL(objMessageUtils, HasSdp(&objMessage)).WillByDefault(Return(IMS_TRUE));

    MockIMtcMediaManager objMediaManager;
    ON_CALL(objCallContext, GetMediaManager()).WillByDefault(ReturnRef(objMediaManager));

    IMS_UINT32 eMediaTypes = (MEDIATYPE_AUDIO | MEDIATYPE_VIDEO);
    ON_CALL(objMediaManager, GetSupportedMediaTypesFromSdp(&objSession))
            .WillByDefault(Return(eMediaTypes));

    AString strMediaTypes("audiovideo");
    EXPECT_TRUE(CheckHandleResult(
            CODE_INTERNAL_REDIAL, EXTRA_CODE_REDIAL_FOR_SDP_CHANGE, strMediaTypes));

    // 2. SDP body exists and there's no supported media type from it.
    eMediaTypes = MEDIATYPE_NONE;
    ON_CALL(objMediaManager, GetSupportedMediaTypesFromSdp(&objSession))
            .WillByDefault(Return(eMediaTypes));
    EXPECT_TRUE(CheckHandleResult(CODE_SIP_NOT_ACCEPTABLE, SipStatusCode::SC_488));

    // 3. No SDP body and it's not required to CSFB.
    ON_CALL(objMessageUtils, HasSdp(&objMessage)).WillByDefault(Return(IMS_FALSE));
    EXPECT_TRUE(CheckHandleResult(CODE_SIP_NOT_ACCEPTABLE, SipStatusCode::SC_488));

    // 4. No SDP body and it's required to CSFB.
    SetCsfbConfig(SipStatusCode::SC_488);
    EXPECT_TRUE(CheckHandleResult(
            CODE_LOCAL_CALL_CS_RETRY_REQUIRED, EXTRA_CODE_CALL_RETRY_SILENT_REDIAL));
}

TEST_F(StartErrorHandlerTest, Handle5xxResponses)
{
    SetMessageCode(SipStatusCode::SC_501);
    EXPECT_TRUE(CheckHandleResult(CODE_SIP_SERVER_INTERNAL_ERROR, SipStatusCode::SC_501));

    SetMessageCode(SipStatusCode::SC_502);
    EXPECT_TRUE(CheckHandleResult(CODE_SIP_SERVER_ERROR, SipStatusCode::SC_502));

    SetMessageCode(SipStatusCode::SC_505);
    EXPECT_TRUE(CheckHandleResult(CODE_SIP_SERVER_ERROR, SipStatusCode::SC_505));

    SetMessageCode(SipStatusCode::SC_513);
    EXPECT_TRUE(CheckHandleResult(CODE_SIP_SERVER_ERROR, SipStatusCode::SC_513));

    SetMessageCode(SipStatusCode::SC_580);
    EXPECT_TRUE(CheckHandleResult(CODE_SIP_SERVER_ERROR, SipStatusCode::SC_580));
}

TEST_F(StartErrorHandlerTest, Handle500Response)
{
    // TODO: more tests
    SetMessageCode(SipStatusCode::SC_500);
    EXPECT_TRUE(CheckHandleResult(CODE_SIP_SERVER_ERROR, SipStatusCode::SC_500));
}

TEST_F(StartErrorHandlerTest,
        Handle503ResponseWithoutRetryAfterWithCsfbConfigEnabledInvokesCsfbWithoutSettingTimer)
{
    SetCsfbConfig(SipStatusCode::SC_503);
    SetMessageCode(SipStatusCode::SC_503);
    ON_CALL(objMessageUtils, GetHeaderValueInt(&objMessage, ISipHeader::RETRY_AFTER_ANY, _))
            .WillByDefault(Return(-1));

    MockIPassiveTimerHolder objPassiveTimer;
    ON_CALL(objCallContext, GetPassiveTimerHolder).WillByDefault(ReturnRef(objPassiveTimer));
    EXPECT_CALL(
            objPassiveTimer, AddTimer(IPassiveTimerHolder::Type::CALL_BLOCKED_BY_RETRY_AFTER, _, _))
            .Times(0);
    EXPECT_CALL(objAosConnector, RegisterWithNextPcscf(0)).Times(0);
    EXPECT_TRUE(CheckHandleResult(
            CODE_LOCAL_CALL_CS_RETRY_REQUIRED, EXTRA_CODE_CALL_RETRY_SILENT_REDIAL));
}

TEST_F(StartErrorHandlerTest, Handle503ResponseWithActiveCallReturnsServiceUnavailable)
{
    SetMessageCode(SipStatusCode::SC_503);
    ON_CALL(objMessageUtils, GetHeaderValueInt(&objMessage, ISipHeader::RETRY_AFTER_ANY, _))
            .WillByDefault(Return(-1));

    ImsList<IMtcCall*> objCalls;
    MockIMtcCall objCall;
    objCalls.Append(&objCall);
    ON_CALL(objCallManager, GetCallsByState(_)).WillByDefault(Return(objCalls));

    EXPECT_TRUE(CheckHandleResult(CODE_SIP_SERVICE_UNAVAILABLE, SipStatusCode::SC_503));
}

TEST_F(StartErrorHandlerTest,
        Handle503ResponseWithoutRetryAfterInvokesCallTerminatingIfAosConnectorIsNull)
{
    SetMessageCode(SipStatusCode::SC_503);
    ON_CALL(objMessageUtils, GetHeaderValueInt(&objMessage, ISipHeader::RETRY_AFTER_ANY, _))
            .WillByDefault(Return(-1));

    ON_CALL(objMtcService, GetAosConnector).WillByDefault(Return(nullptr));
    MockIPassiveTimerHolder objPassiveTimer;
    ON_CALL(objCallContext, GetPassiveTimerHolder).WillByDefault(ReturnRef(objPassiveTimer));
    EXPECT_CALL(
            objPassiveTimer, AddTimer(IPassiveTimerHolder::Type::CALL_BLOCKED_BY_RETRY_AFTER, _, _))
            .Times(0);
    EXPECT_CALL(objAosConnector, RegisterWithNextPcscf(0)).Times(0);

    EXPECT_TRUE(CheckHandleResult(CODE_LOCAL_INTERNAL_ERROR));
}

TEST_F(StartErrorHandlerTest, Handle503ResponseWithoutRetryAfterInvokesCallingAosAndRedialing)
{
    SetMessageCode(SipStatusCode::SC_503);
    ON_CALL(objMessageUtils, GetHeaderValueInt(&objMessage, ISipHeader::RETRY_AFTER_ANY, _))
            .WillByDefault(Return(-1));

    EXPECT_CALL(objAosConnector, RegisterWithNextPcscf(0)).Times(1);
    EXPECT_TRUE(CheckHandleResult(CODE_INTERNAL_REDIAL, EXTRA_CODE_REDIAL_WITH_NEXT_PCSCF));
}

TEST_F(StartErrorHandlerTest,
        Handle503ResponseWithRetryAfterAndCsfbConfigEnabledInvokesCsfbWithSettingTimer)
{
    IMS_SINT32 nAnyRetryAfter = 10;
    SetCsfbConfig(SipStatusCode::SC_503);
    SetMessageCode(SipStatusCode::SC_503);
    ON_CALL(objMessageUtils, GetHeaderValueInt(&objMessage, ISipHeader::RETRY_AFTER_ANY, _))
            .WillByDefault(Return(nAnyRetryAfter));
    ON_CALL(m_pConfigService->GetMockCarrierConfig(),
            GetInt(CarrierConfig::Ims::KEY_SIP_TIMER_B_MILLIS_INT, _))
            .WillByDefault(Return((nAnyRetryAfter - 1) * 1000));
    Engine::GetConfiguration()->RefreshConfigs(objCallContext.GetSlotId());

    MockIPassiveTimerHolder objPassiveTimer;
    ON_CALL(objCallContext, GetPassiveTimerHolder).WillByDefault(ReturnRef(objPassiveTimer));
    EXPECT_CALL(objPassiveTimer,
            AddTimer(IPassiveTimerHolder::Type::CALL_BLOCKED_BY_RETRY_AFTER, nAnyRetryAfter * 1000,
                    _))
            .Times(1);

    EXPECT_CALL(objAosConnector, RegisterWithNextPcscf(nAnyRetryAfter)).Times(0);
    EXPECT_TRUE(CheckHandleResult(
            CODE_LOCAL_CALL_CS_RETRY_REQUIRED, EXTRA_CODE_CALL_RETRY_SILENT_REDIAL));
}

TEST_F(StartErrorHandlerTest,
        Handle503ResponseWithRetryAfterInvokesCallingAosAndRedialingIfRetryAfterIsBiggerThanTimerb)
{
    IMS_SINT32 nAnyRetryAfter = 10;
    SetMessageCode(SipStatusCode::SC_503);
    ON_CALL(objMessageUtils, GetHeaderValueInt(&objMessage, ISipHeader::RETRY_AFTER_ANY, _))
            .WillByDefault(Return(nAnyRetryAfter));
    ON_CALL(m_pConfigService->GetMockCarrierConfig(),
            GetInt(CarrierConfig::Ims::KEY_SIP_TIMER_B_MILLIS_INT, _))
            .WillByDefault(Return((nAnyRetryAfter - 1) * 1000));
    Engine::GetConfiguration()->RefreshConfigs(objCallContext.GetSlotId());

    EXPECT_CALL(objAosConnector, RegisterWithNextPcscf(_)).Times(1);
    MockIPassiveTimerHolder objPassiveTimer;
    ON_CALL(objCallContext, GetPassiveTimerHolder).WillByDefault(ReturnRef(objPassiveTimer));
    EXPECT_CALL(objPassiveTimer,
            AddTimer(IPassiveTimerHolder::Type::CALL_BLOCKED_BY_RETRY_AFTER, nAnyRetryAfter * 1000,
                    _))
            .Times(0);

    EXPECT_TRUE(CheckHandleResult(CODE_INTERNAL_REDIAL, EXTRA_CODE_REDIAL_WITH_NEXT_PCSCF));
}

TEST_F(StartErrorHandlerTest,
        Handle503ResponseWithRetryAfterInvokesCsfbWithSettingTimerIfRetryAfterIsSmallerThanTimerbAndEpsCombinedAttached)
{
    IMS_SINT32 nAnyRetryAfter = 10;
    SetMessageCode(SipStatusCode::SC_503);
    ON_CALL(objMessageUtils, GetHeaderValueInt(&objMessage, ISipHeader::RETRY_AFTER_ANY, _))
            .WillByDefault(Return(nAnyRetryAfter));
    ON_CALL(m_pConfigService->GetMockCarrierConfig(),
            GetInt(CarrierConfig::Ims::KEY_SIP_TIMER_B_MILLIS_INT, _))
            .WillByDefault(Return((nAnyRetryAfter + 1) * 1000));
    Engine::GetConfiguration()->RefreshConfigs(objCallContext.GetSlotId());

    ON_CALL(objImsEventReceiver, GetWParam(IMS_EVENT_LTE_INFO))
            .WillByDefault(Return(IMS_LTE_INFO_COMBINED_ATTACHED));

    EXPECT_CALL(objAosConnector, RegisterWithNextPcscf(_)).Times(0);
    MockIPassiveTimerHolder objPassiveTimer;
    ON_CALL(objCallContext, GetPassiveTimerHolder).WillByDefault(ReturnRef(objPassiveTimer));
    EXPECT_CALL(objPassiveTimer,
            AddTimer(IPassiveTimerHolder::Type::CALL_BLOCKED_BY_RETRY_AFTER, nAnyRetryAfter * 1000,
                    _))
            .Times(1);

    EXPECT_TRUE(CheckHandleResult(
            CODE_LOCAL_CALL_CS_RETRY_REQUIRED, EXTRA_CODE_CALL_RETRY_SILENT_REDIAL));
}

TEST_F(StartErrorHandlerTest,
        Handle503ResponseWithRetryAfterInvokesRedialWithoutSettingTimerIfRetryAfterisSmallerThanTimerbAndEpsOnlyAttached)
{
    IMS_SINT32 nAnyRetryAfter = 10;
    SetMessageCode(SipStatusCode::SC_503);
    ON_CALL(objMessageUtils, GetHeaderValueInt(&objMessage, ISipHeader::RETRY_AFTER_ANY, _))
            .WillByDefault(Return(nAnyRetryAfter));
    ON_CALL(m_pConfigService->GetMockCarrierConfig(),
            GetInt(CarrierConfig::Ims::KEY_SIP_TIMER_B_MILLIS_INT, _))
            .WillByDefault(Return((nAnyRetryAfter + 1) * 1000));
    Engine::GetConfiguration()->RefreshConfigs(objCallContext.GetSlotId());

    ON_CALL(objImsEventReceiver, GetWParam(IMS_EVENT_LTE_INFO))
            .WillByDefault(Return(IMS_LTE_INFO_EPS_ONLY_ATTACHED));

    EXPECT_CALL(objAosConnector, RegisterWithNextPcscf(_)).Times(0);
    MockIPassiveTimerHolder objPassiveTimer;
    ON_CALL(objCallContext, GetPassiveTimerHolder).WillByDefault(ReturnRef(objPassiveTimer));
    EXPECT_CALL(objPassiveTimer,
            AddTimer(IPassiveTimerHolder::Type::CALL_BLOCKED_BY_RETRY_AFTER, nAnyRetryAfter * 1000,
                    _))
            .Times(0);

    EXPECT_TRUE(CheckHandleResult(CODE_INTERNAL_REDIAL, EXTRA_CODE_REDIAL_BY_RETRY_AFTER, "10000"));
}

TEST_F(StartErrorHandlerTest, Handle503ResponseForEmergencyCall)
{
    SetMessageCode(SipStatusCode::SC_503);
    objCallInfo.bEmergency = IMS_TRUE;
    EXPECT_TRUE(CheckHandleResult(CODE_SIP_SERVICE_UNAVAILABLE, SipStatusCode::SC_503));
}

TEST_F(StartErrorHandlerTest, Handle504ResponseDoesNotRestoreRegistration)
{
    SetMessageCode(SipStatusCode::SC_504);

    AString strPathHeader("sip:anyPath");
    ON_CALL(objAosConnector, GetPathHeaderValue).WillByDefault(Return(AString(strPathHeader)));
    AString strServiceRoute("sip:anyServiceRoute");
    ON_CALL(objAosConnector, GetServiceRouteHeaderValue)
            .WillByDefault(Return(AString(strServiceRoute)));

    ON_CALL(objMessageUtils, ContainsAddressInPaid(&objMessage, strPathHeader))
            .WillByDefault(Return(IMS_FALSE));
    ON_CALL(objMessageUtils, ContainsAddressInPaid(&objMessage, strServiceRoute))
            .WillByDefault(Return(IMS_FALSE));
    EXPECT_CALL(objAosConnector, Control(_)).Times(0);
    EXPECT_TRUE(CheckHandleResult(CODE_SIP_SERVER_TIMEOUT, SipStatusCode::SC_504));

    ON_CALL(objMessageUtils, ContainsAddressInPaid(&objMessage, strServiceRoute))
            .WillByDefault(Return(IMS_TRUE));

    Ims3gppData objIms3gppData;
    ON_CALL(objMessageUtils, GetIms3gppData(&objMessage)).WillByDefault(Return(objIms3gppData));
    EXPECT_TRUE(CheckHandleResult(CODE_SIP_SERVER_TIMEOUT, SipStatusCode::SC_504));

    objIms3gppData.eType = Ims3gpp::TYPE_ALTERNATIVE_SERVICE;
    ON_CALL(objMessageUtils, GetIms3gppData(&objMessage)).WillByDefault(Return(objIms3gppData));
    EXPECT_TRUE(CheckHandleResult(CODE_SIP_SERVER_TIMEOUT, SipStatusCode::SC_504));

    objIms3gppData.eAlternativeServiceType = Ims3gpp::AlternativeService::TYPE_RESTORATION;
    ON_CALL(objMessageUtils, GetIms3gppData(&objMessage)).WillByDefault(Return(objIms3gppData));
    EXPECT_TRUE(CheckHandleResult(CODE_SIP_SERVER_TIMEOUT, SipStatusCode::SC_504));

    SetCsfbConfig(SipStatusCode::SC_504);
    EXPECT_TRUE(CheckHandleResult(
            CODE_LOCAL_CALL_CS_RETRY_REQUIRED, EXTRA_CODE_CALL_RETRY_SILENT_REDIAL));
}

TEST_F(StartErrorHandlerTest, Handle504ResponseWithConfigNotAvailable)
{
    SetUp504RegRestoration(CarrierConfig::ImsVoice::REGISTRATION_RESTORATION_NOT_AVAILABLE);
    EXPECT_TRUE(CheckHandleResult(CODE_SIP_SERVER_TIMEOUT, SipStatusCode::SC_504));
}

TEST_F(StartErrorHandlerTest, Handle504ResponseWithConfigRegisterNextPcscf)
{
    SetUp504RegRestoration(
            CarrierConfig::ImsVoice::REGISTRATION_RESTORATION_INITIAL_REGISTER_WITH_NEXT_PCSCF);

    EXPECT_CALL(objAosConnector, Control(ImsAosControl::PCSCF_NEXT)).Times(1);
    EXPECT_TRUE(CheckHandleResult(CODE_SIP_SERVER_TIMEOUT, SipStatusCode::SC_504));
}

TEST_F(StartErrorHandlerTest, Handle504ResponseWithConfigRecoverRegistration)
{
    SetUp504RegRestoration(CarrierConfig::ImsVoice::REGISTRATION_RESTORATION_RECOVER_REGISTRATION);

    EXPECT_CALL(objAosConnector, Control(ImsAosControl::REGISTER_REINITIATE)).Times(1);
    EXPECT_TRUE(CheckHandleResult(CODE_SIP_SERVER_TIMEOUT, SipStatusCode::SC_504));
}

TEST_F(StartErrorHandlerTest, Handle504ResponseWithConfigRecoverByNetworkContext)
{
    SetUp504RegRestoration(
            CarrierConfig::ImsVoice::REGISTRATION_RESTORATION_RECOVER_BY_NETWORK_CONTEXT);
    SetCsfbConfig(SipStatusCode::SC_504);

    // combined attached case
    EXPECT_CALL(objAosConnector, Control(_)).Times(0);
    EXPECT_TRUE(CheckHandleResult(
            CODE_LOCAL_CALL_CS_RETRY_REQUIRED, EXTRA_CODE_CALL_RETRY_SILENT_REDIAL));

    // eps only attached case
    ON_CALL(objImsEventReceiver, GetWParam(IMS_EVENT_LTE_INFO))
            .WillByDefault(Return(IMS_LTE_INFO_EPS_ONLY_ATTACHED));
    EXPECT_CALL(objAosConnector, Control(ImsAosControl::PCSCF_NEXT)).Times(1);
    EXPECT_TRUE(CheckHandleResult(CODE_SIP_SERVER_TIMEOUT, SipStatusCode::SC_504));
}

TEST_F(StartErrorHandlerTest, Handle504ResponseWithConfigRecoverWithoutPdnReconnection)
{
    SetUp504RegRestoration(CarrierConfig::ImsVoice::
                    REGISTRATION_RESTORATION_RECOVER_REGISTRATION_WITHOUT_PDN_RECONNECT);

    EXPECT_CALL(objAosConnector, Control(ImsAosControl::REGISTER_REINITIATE)).Times(1);
    EXPECT_TRUE(CheckHandleResult(CODE_SIP_SERVER_TIMEOUT, SipStatusCode::SC_504));
}

TEST_F(StartErrorHandlerTest, Handle504ResponseForEmergencyCall)
{
    SetMessageCode(SipStatusCode::SC_504);
    objCallInfo.bEmergency = IMS_TRUE;
    EXPECT_TRUE(CheckHandleResult(CODE_SIP_SERVER_TIMEOUT, SipStatusCode::SC_504));
}

TEST_F(StartErrorHandlerTest, Handle6xxResponses)
{
    SetMessageCode(SipStatusCode::SC_600);
    EXPECT_TRUE(CheckHandleResult(CODE_SIP_BUSY, SipStatusCode::SC_600));

    SetMessageCode(SipStatusCode::SC_603);
    EXPECT_TRUE(CheckHandleResult(CODE_SIP_USER_REJECTED, SipStatusCode::SC_603));

    SetMessageCode(SipStatusCode::SC_604);
    EXPECT_TRUE(CheckHandleResult(CODE_SIP_NOT_REACHABLE, SipStatusCode::SC_604));

    SetMessageCode(SipStatusCode::SC_606);
    EXPECT_TRUE(CheckHandleResult(CODE_SIP_NOT_ACCEPTABLE, SipStatusCode::SC_606));

    const IMS_SINT32 ANY_REJECT_CODE = SipStatusCode::SC_699;
    SetMessageCode(ANY_REJECT_CODE);
    EXPECT_TRUE(CheckHandleResult(CODE_SIP_GLOBAL_ERROR, ANY_REJECT_CODE));
}

TEST_F(StartErrorHandlerTest, ExtraCodeIsSetByReasonHeader)
{
    IMS_SINT32 nAnyCause = 12345;
    ON_CALL(objMessageUtils, GetCauseFromReasonHeader).WillByDefault(Return(nAnyCause));

    SetMessageCode(SipStatusCode::SC_603);
    EXPECT_TRUE(CheckHandleResult(CODE_SIP_USER_REJECTED, nAnyCause));
}

}  // namespace android
