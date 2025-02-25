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

#include "AString.h"
#include "CarrierConfig.h"
#include "Engine.h"
#include "IConfiguration.h"
#include "IMessage.h"
#include "IMessageBodyPart.h"
#include "ISession.h"
#include "ISipConfig.h"
#include "ISipConfigV.h"
#include "ISipHeader.h"
#include "ISipMessage.h"
#include "Ims3gpp.h"
#include "ImsAosParameter.h"
#include "ImsEventDef.h"
#include "MtcImsEventReceiver.h"
#include "ServiceTrace.h"
#include "SipAddress.h"
#include "SipStatusCode.h"
#include "call/EpsFallbackTrigger.h"
#include "call/IMtcCallContext.h"
#include "call/IMtcSession.h"
#include "call/MtcCallManager.h"
#include "call/termination/DefaultStatusCodeAndReasonCodeSets.h"
#include "call/termination/EmergencyStartErrorHandler.h"
#include "call/termination/StartErrorHandler.h"
#include "configuration/MtcConfigurationProxy.h"
#include "configuration/MtcConfigurationResolver.h"
#include "helper/IMtcAosConnector.h"
#include "helper/IPassiveTimerHolder.h"
#include "helper/MtcTimerWrapper.h"
#include "media/IMtcMediaManager.h"
#include "media/MtcMediaUtil.h"
#include "utility/IMessageUtils.h"
#include <unordered_map>

__IMS_TRACE_TAG_COM_MTC__;

LOCAL const AString REASON_TEXT_MAX_CALL_LIMIT_REACHED_VZW =
        "simultaneous call limit has already been reached";

// clang-format off
const std::unordered_map<IMS_SINT32, StartErrorHandler::ActionFunc>
        StartErrorHandler::objActionFuncMap = {
    {ConfigVoice::START_ERROR_ACTION_CSFB,
            &StartErrorHandler::HandleCsfb},
    {ConfigVoice::START_ERROR_ACTION_SILENT_REINVITE,
            &StartErrorHandler::HandleSilentReinvite},
    {ConfigVoice::START_ERROR_ACTION_SILENT_REINVITE_BY_SDP_CONTENT,
            &StartErrorHandler::HandleSilentReinviteBySdpContent},
    {ConfigVoice::START_ERROR_ACTION_SILENT_REINVITE_BY_RETRY_AFTER,
            &StartErrorHandler::HandleSilentReinviteByRetryAfter},
    {ConfigVoice::START_ERROR_ACTION_REGISTRATION_RESTORATION_ON_IMS3GPP_BY_POLICY,
            &StartErrorHandler::HandleRegistrationRestorationOnIms3gppByPolicy},
    {ConfigVoice::START_ERROR_ACTION_REDIRECTION_BY_CONTACT,
            &StartErrorHandler::HandleRedirectionByContact},
    {ConfigVoice::START_ERROR_ACTION_NON_UE_DETECTABLE_EMERGENCY_CALL,
            &StartErrorHandler::HandleNonUeDetectableEmergencyCall},
    {ConfigVoice::START_ERROR_ACTION_HANDLE_FORBIDDEN_BY_POLICY,
            &StartErrorHandler::HandleForbiddenByPolicy},
    {ConfigVoice::START_ERROR_ACTION_TERMINATE_BY_REASON_PHRASE,
            &StartErrorHandler::HandleTerminateByReasonPhrase},
    {ConfigVoice::START_ERROR_ACTION_USSI_CSFB,
            &StartErrorHandler::HandleUssiCsfb},
    {ConfigVoice::START_ERROR_ACTION_BLOCK_CALL_BY_TIMER,
            &StartErrorHandler::HandleBlockCallByTimer},
    {ConfigVoice::START_ERROR_ACTION_TRIGGER_EPSFB,
            &StartErrorHandler::HandleTriggerEpsfb},
    {ConfigVoice::START_ERROR_ACTION_TERMINATE_BY_RESPONSE_SOURCE,
            &StartErrorHandler::HandleTerminateByResponseSource},
    {ConfigVoice::START_ERROR_ACTION_TERMINATE_BY_REASON_HEADER_TEXT,
            &StartErrorHandler::HandleTerminateByReasonHeaderText}
};
// clang-format on

PUBLIC
StartErrorHandler::StartErrorHandler(IN IMtcCallContext& objContext, IN ISession& objSession) :
        m_objContext(objContext),
        m_objSession(objSession)
{
}

PUBLIC
StartErrorHandler::~StartErrorHandler() {}

PUBLIC
CallReasonInfo StartErrorHandler::Handle(IN const IMessage* piMessage) const
{
    if (m_objContext.GetCallInfo().IsEmergency())
    {
        return EmergencyStartErrorHandler(m_objContext, m_objSession).Handle(piMessage);
    }

    if (IsTransactionTimeout(piMessage))
    {
        IMS_TRACE_I("Handle : Timeout", 0, 0, 0);
        return HandleTransactionTimeout();
    }

    ImsVector<IMS_SINT32> objActions = MtcConfigurationResolver::LookupActionForStatusCode(
            m_objContext.GetConfigurationProxy(),
            ConfigVoice::KEY_REJECT_CODE_AND_ACTION_SET_STRING_ARRAY, piMessage->GetStatusCode());
    for (IMS_UINT32 i = 0; i < objActions.GetSize(); ++i)
    {
        auto it = objActionFuncMap.find(objActions.GetAt(i));
        if (it != objActionFuncMap.end())
        {
            CallReasonInfo objResult = (this->*(it->second))(*piMessage);
            if (objResult.nCode != CODE_NONE)
            {
                return objResult;
            }
        }
    }

    return GetDefaultCallReasonInfo(m_objContext, *piMessage);
}

PUBLIC GLOBAL CallReasonInfo StartErrorHandler::GetDefaultCallReasonInfo(
        IN IMtcCallContext& objContext, IN const IMessage& objMessage)
{
    IMS_SINT32 nReasonCode = GetDefaultReasonCode(objContext, objMessage.GetStatusCode());
    IMS_TRACE_I("GetDefaultCallReasonInfo [%d]", nReasonCode, 0, 0);
    return CallReasonInfo(nReasonCode, GetDefaultExtraCode(objContext, objMessage));
}

PUBLIC GLOBAL IMS_SINT32 StartErrorHandler::GetDefaultReasonCode(
        IN IMtcCallContext& objContext, IN IMS_SINT32 nStatusCode)
{
    IMS_SINT32 nReasonCode = MtcConfigurationResolver::LookupReasonCodeByStatusCodeForNormal(
            objContext.GetConfigurationProxy(), nStatusCode);

    if (nReasonCode != CODE_NONE)
    {
        return nReasonCode;
    }

    auto it = s_defaultStatusCodeAndReasonCodeMap.find(nStatusCode);
    if (it != s_defaultStatusCodeAndReasonCodeMap.end())
    {
        nReasonCode = it->second;
    }

    return nReasonCode == CODE_NONE ? CODE_SIP_SERVER_ERROR : nReasonCode;
}

PUBLIC GLOBAL IMS_SINT32 StartErrorHandler::GetDefaultExtraCode(
        IN IMtcCallContext& objContext, IN const IMessage& objMessage)
{
    IMS_SINT32 nExtraCode = objContext.GetMessageUtils().GetCauseFromReasonHeader(&objMessage);
    if (nExtraCode == -1)
    {
        nExtraCode = objMessage.GetStatusCode();
    }
    return nExtraCode;
}

PRIVATE
CallReasonInfo StartErrorHandler::HandleTransactionTimeout() const
{
    if (EpsFallbackTrigger::ShouldTriggerByMoRequestTimeout(m_objContext))
    {
        return CallReasonInfo(CODE_INTERNAL_REDIAL, EXTRA_CODE_REDIAL_AFTER_EPS_FALLBACK);
    }

    const IMS_CHAR* pszKey = m_objContext.GetService().IsWlanIpCanType()
            ? ConfigWfc::KEY_POLICY_FOR_TCALL_TIMER_EXPIRY_OF_VOWIFI_CALL_INT
            : ConfigVoice::KEY_POLICY_FOR_TCALL_TIMER_EXPIRY_OF_VOLTE_CALL_INT;

    const IMS_SINT32 nPolicy = m_objContext.GetConfigurationProxy().GetInt(pszKey);
    IMS_SINT32 nReason = CODE_NETWORK_RESP_TIMEOUT;
    IMS_SINT32 nExtraCode = EXTRA_CODE_METHOD_INVITE;
    switch (nPolicy)
    {
        case ConfigVoice::MO_CALL_REQUEST_TIMEOUT_POLICY_CALL_END:
            break;
        case ConfigVoice::
                MO_CALL_REQUEST_TIMEOUT_POLICY_INITIAL_REGISTER_PCSCF_DISCOVERY_AFTER_CSFB:
            if (m_objContext.GetService().IsCsfbAvailable())
            {
                nReason = CODE_LOCAL_CALL_CS_RETRY_REQUIRED;
                nExtraCode = EXTRA_CODE_CALL_RETRY_SILENT_REDIAL;
            }
            ControlAos(ImsAosControl::PCSCF_NEXT_WITH_DISCOVERY);
            break;
        case ConfigVoice::MO_CALL_REQUEST_TIMEOUT_POLICY_CSFB:
            if (m_objContext.GetService().IsCsfbAvailable())
            {
                nReason = CODE_LOCAL_CALL_CS_RETRY_REQUIRED;
                nExtraCode = EXTRA_CODE_CALL_RETRY_SILENT_REDIAL;
            }
            break;
        case ConfigVoice::MO_CALL_REQUEST_TIMEOUT_POLICY_INITIAL_REGISTER_AFTER_CSFB_IF_AVAILBLE:
            return RegisterAfterMayPerformCsfb();
        case ConfigVoice::MO_CALL_REQUEST_TIMEOUT_POLICY_INITIAL_REGISTER_CURRENT_PCSCF:
            ControlAos(ImsAosControl::REGISTER_REINITIATE);
            break;
        case ConfigVoice::MO_CALL_REQUEST_TIMEOUT_POLICY_INITIAL_REGISTER_NEXT_PCSCF:
            ControlAos(ImsAosControl::PCSCF_NEXT);
            break;
        case ConfigVoice::
                MO_CALL_REQUEST_TIMEOUT_POLICY_INITIAL_REGISTER_WITH_PDN_RECONNECT_AFTER_CSFB:
            if (m_objContext.GetService().IsCsfbAvailable())
            {
                nReason = CODE_LOCAL_CALL_CS_RETRY_REQUIRED;
                nExtraCode = EXTRA_CODE_CALL_RETRY_SILENT_REDIAL;
                ControlAos(ImsAosControl::REGISTER_REINITIATE_BY_CSFB);  // TODO: check timing
            }
            break;
        default:  // ConfigVoice::MO_CALL_REQUEST_TIMEOUT_POLICY_SILENT_REDIAL
            nReason = CODE_INTERNAL_REDIAL;
            nExtraCode = EXTRA_CODE_REDIAL_BY_REQUEST_TIMEOUT;
            break;
    }

    return CallReasonInfo(nReason, nExtraCode);
}

PRIVATE
CallReasonInfo StartErrorHandler::HandleCsfb(IN const IMessage& /*objMessage*/) const
{
    IMS_TRACE_I("HandleCsfb", 0, 0, 0);
    if (m_objContext.GetService().IsCsfbAvailable())
    {
        return CallReasonInfo(
                CODE_LOCAL_CALL_CS_RETRY_REQUIRED, EXTRA_CODE_CALL_RETRY_SILENT_REDIAL);
    }
    return CallReasonInfo(CODE_NONE);
}

PRIVATE
CallReasonInfo StartErrorHandler::HandleSilentReinvite(IN const IMessage& /*objMessage*/) const
{
    IMS_TRACE_I("HandleSilentReinvite", 0, 0, 0);
    return CallReasonInfo(CODE_INTERNAL_REDIAL, EXTRA_CODE_REDIAL_BY_ERROR_RESPONSE);
}

PRIVATE
CallReasonInfo StartErrorHandler::HandleSilentReinviteBySdpContent(
        IN const IMessage& objMessage) const
{
    IMS_TRACE_I("HandleSilentReinviteBySdpContent", 0, 0, 0);
    if (m_objContext.GetMessageUtils().HasSdp(&objMessage))
    {
        IMS_UINT32 eMediaTypes =
                m_objContext.GetMediaManager().GetSupportedMediaTypesFromSdp(&m_objSession);
        if (eMediaTypes != MEDIATYPE_NONE)
        {
            return CallReasonInfo(CODE_INTERNAL_REDIAL, EXTRA_CODE_REDIAL_FOR_SDP_CHANGE,
                    MtcMediaUtil::MediaTypesToString(eMediaTypes));
        }
    }

    return CallReasonInfo(CODE_NONE);
}

PRIVATE
CallReasonInfo StartErrorHandler::HandleSilentReinviteByRetryAfter(
        IN const IMessage& objMessage) const
{
    IMS_TRACE_I("HandleSilentReinviteByRetryAfter", 0, 0, 0);
    IMS_SINT32 nRetryAfter = m_objContext.GetMessageUtils().GetHeaderValueInt(
            &objMessage, ISipHeader::RETRY_AFTER_ANY);
    if (nRetryAfter <= 0)
    {
        return CallReasonInfo(CODE_NONE);
    }
    AString strRetryAfter;
    strRetryAfter.SetNumber(nRetryAfter * 1000);
    return CallReasonInfo(CODE_INTERNAL_REDIAL, EXTRA_CODE_REDIAL_BY_RETRY_AFTER, strRetryAfter);
}

PRIVATE
CallReasonInfo StartErrorHandler::HandleRegistrationRestorationOnIms3gppByPolicy(
        IN const IMessage& objMessage) const
{
    IMS_TRACE_I("HandleRegistrationRestorationOnIms3gppByPolicy", 0, 0, 0);
    if (m_objContext.GetConfigurationProxy().GetBoolean(ConfigVoice::
                        KEY_REGISTRATION_RESTORATION_FOR_INVITE_REQUIRE_HEADER_VALIDATION_BOOL))
    {
        if (!m_objContext.GetMessageUtils().ContainsAddressInPaid(&objMessage, GetPathHeader()) &&
                !m_objContext.GetMessageUtils().ContainsAddressInPaid(
                        &objMessage, GetServiceRouteHeader()))
        {
            IMS_TRACE_E(0, "P-Asserted-Identity header validation failed.", 0, 0, 0);
            return CallReasonInfo(CODE_NONE);
        }
    }

    if (IsInitialRegistrationRequired(objMessage))
    {
        const IMS_SINT32 nPolicy = m_objContext.GetConfigurationProxy().GetInt(
                ConfigVoice::KEY_REGISTRATION_RESTORATION_MODE_ON_504_FOR_INVITE_INT);
        switch (nPolicy)
        {
            case ConfigVoice::REGISTRATION_RESTORATION_NOT_AVAILABLE:
                break;
            case ConfigVoice::REGISTRATION_RESTORATION_RECOVER_BY_NETWORK_CONTEXT:
                if (m_objContext.GetService().IsCsfbAvailable())
                {
                    break;
                }
                __IMS_FALLTHROUGH__
            case ConfigVoice::REGISTRATION_RESTORATION_INITIAL_REGISTER_WITH_NEXT_PCSCF:
                ControlAos(ImsAosControl::PCSCF_NEXT);
                break;

            case ConfigVoice::REGISTRATION_RESTORATION_RECOVER_REGISTRATION:
                // If there is an operator that requires PDN reconnect, AoS I/F should be added.
            case ConfigVoice::REGISTRATION_RESTORATION_RECOVER_REGISTRATION_WITHOUT_PDN_RECONNECT:
                ControlAos(ImsAosControl::REGISTER_REINITIATE);
                break;
        }
    }

    return CallReasonInfo(CODE_NONE);
}

PRIVATE
CallReasonInfo StartErrorHandler::HandleRedirectionByContact(IN const IMessage& objMessage) const
{
    IMS_TRACE_I("HandleRedirectionByContact", 0, 0, 0);
    AString strContact =
            m_objContext.GetMessageUtils().GetHeaderValue(&objMessage, ISipHeader::CONTACT_NORMAL);
    if (strContact.GetLength() > 0)
    {
        return CallReasonInfo(CODE_INTERNAL_REDIAL, EXTRA_CODE_REDIAL_FOR_REDIRECTION, strContact);
    }
    return CallReasonInfo(CODE_NONE);
}

PRIVATE
CallReasonInfo StartErrorHandler::HandleNonUeDetectableEmergencyCall(
        IN const IMessage& objMessage) const
{
    IMS_SINT32 eSosType = m_objContext.GetMessageUtils().GetSosTypeFromServiceUrn(
            &objMessage, ISipHeader::CONTACT_NORMAL);
    if (eSosType == EXTRA_CODE_EMERGENCYSERVICE_INVALID)
    {
        return CallReasonInfo(CODE_NONE);
    }

    if (!m_objContext.GetConfigurationProxy().GetBoolean(ConfigEmergency::
            KEY_EMERGENCY_RETRY_WITHOUT_CHECKING_380_CONTENT_FOR_NON_UE_DETECTABLE_EMERGENCY_CALL_BOOL))
    {
        if (m_objContext.GetMessageUtils().GetHeaderValue(&objMessage, ISipHeader::
                P_ASSERTED_IDENTITY) != GetPathHeader())
        {
            return CallReasonInfo(CODE_NONE);
        }
    }

    if (IsAlternativeEmergencyService(objMessage))
    {
        AString strCountrySpecificUrn;
        if (eSosType == EXTRA_CODE_EMERGENCYSERVICE_COUNTRY_SPECIFIC)
        {
            strCountrySpecificUrn = m_objContext.GetMessageUtils().GetUri(
                    &objMessage, IMS_FALSE, ISipHeader::CONTACT_NORMAL);
        }
        return CallReasonInfo(CODE_SIP_ALTERNATE_EMERGENCY_CALL, eSosType, strCountrySpecificUrn);
    }

    return CallReasonInfo(CODE_NONE);
}

PRIVATE
CallReasonInfo StartErrorHandler::HandleForbiddenByPolicy(IN const IMessage& /*objMessage*/) const
{
    IMS_TRACE_I("HandleForbiddenByPolicy", 0, 0, 0);
    const IMS_SINT32 nPolicy = m_objContext.GetConfigurationProxy().GetInt(
            ConfigVoice::KEY_POLICY_FOR_403_RESPONSE_FOR_INVITE_INT);
    switch (nPolicy)
    {
        case ConfigVoice::SIP_403_POLICY_TERMINATE_CALL:
            break;

        case ConfigVoice::SIP_403_POLICY_TERMINATE_CALL_AND_RECOVER_REGISTRATION:
            ControlAos(ImsAosControl::REGISTER_REINITIATE);
            break;

        case ConfigVoice::SIP_403_POLICY_TERMINATE_CALL_AND_REFRESH_REGISTRATION:
            ControlAos(ImsAosControl::REGISTER_REFRESH);
            break;

        case ConfigVoice::SIP_403_POLICY_CSFB:
            if (m_objContext.GetService().IsCsfbAvailable())
            {
                return CallReasonInfo(
                        CODE_LOCAL_CALL_CS_RETRY_REQUIRED, EXTRA_CODE_CALL_RETRY_SILENT_REDIAL);
            }
            break;

        case ConfigVoice::SIP_403_POLICY_CSFB_AND_RECOVER_REGISTRATION:
            if (m_objContext.GetService().IsCsfbAvailable())
            {
                ControlAos(ImsAosControl::REGISTER_REINITIATE_BY_CSFB);
                return CallReasonInfo(
                        CODE_LOCAL_CALL_CS_RETRY_REQUIRED, EXTRA_CODE_CALL_RETRY_SILENT_REDIAL);
            }
            break;
    }

    return CallReasonInfo(CODE_NONE);
}

PRIVATE
CallReasonInfo StartErrorHandler::HandleTerminateByReasonPhrase(IN const IMessage& objMessage) const
{
    IMS_TRACE_I("HandleTerminateByReasonPhrase", 0, 0, 0);

    const AString strNormalizedReasonPhrase = objMessage.GetReasonPhrase().SimplifyWsp();
    if (strNormalizedReasonPhrase.MakeLower().Contains(REASON_TEXT_MAX_CALL_LIMIT_REACHED_VZW))
    {
        return CallReasonInfo(GetDefaultReasonCode(m_objContext, objMessage.GetStatusCode()), -1,
                strNormalizedReasonPhrase);
    }
    return CallReasonInfo(CODE_NONE);
}

PRIVATE
CallReasonInfo StartErrorHandler::HandleUssiCsfb(IN const IMessage& /*objMessage*/) const
{
    IMS_TRACE_I("HandleUssiCsfb", 0, 0, 0);
    if (m_objContext.GetCallInfo().bUssi && m_objContext.GetService().IsCsfbAvailable())
    {
        return CallReasonInfo(
                CODE_LOCAL_CALL_CS_RETRY_REQUIRED, EXTRA_CODE_CALL_RETRY_SILENT_REDIAL);
    }
    return CallReasonInfo(CODE_NONE);
}

PRIVATE
CallReasonInfo StartErrorHandler::HandleBlockCallByTimer(IN const IMessage& objMessage) const
{
    IMS_TRACE_I("HandleBlockCallByTimer", 0, 0, 0);
    IMS_SINT32 nRetryAfter = m_objContext.GetMessageUtils().GetHeaderValueInt(
            &objMessage, ISipHeader::RETRY_AFTER_ANY);
    IMS_SINT32 nRetryAfterInMillis = nRetryAfter * 1000;
    if (m_objContext.GetService().IsCsfbAvailable() &&
            IsCsfbActionRequiredStatusCode(objMessage.GetStatusCode()))
    {
        SetTimerForImsCallBlocking(nRetryAfterInMillis);
        return CallReasonInfo(
                CODE_LOCAL_CALL_CS_RETRY_REQUIRED, EXTRA_CODE_CALL_RETRY_SILENT_REDIAL);
    }

    if (!m_objContext.GetCallManager().GetCallsByState(IMtcCall::State::ESTABLISHED).IsEmpty())
    {
        return CallReasonInfo(CODE_NONE);
    }

    if (IsRegisterWithNextPcscfAndRedialRequiredFor503(nRetryAfter))
    {
        if (RegisterFor503(nRetryAfter))
        {
            return CallReasonInfo(CODE_INTERNAL_REDIAL, EXTRA_CODE_REDIAL_WITH_NEXT_PCSCF);
        }

        return CallReasonInfo(CODE_LOCAL_INTERNAL_ERROR);
    }

    if (m_objContext.GetService().IsCsfbAvailable())
    {
        SetTimerForImsCallBlocking(nRetryAfterInMillis);
        return CallReasonInfo(
                CODE_LOCAL_CALL_CS_RETRY_REQUIRED, EXTRA_CODE_CALL_RETRY_SILENT_REDIAL);
    }

    AString strRetryAfter;
    strRetryAfter.SetNumber(nRetryAfterInMillis);
    return CallReasonInfo(CODE_INTERNAL_REDIAL, EXTRA_CODE_REDIAL_BY_RETRY_AFTER, strRetryAfter);
}

PRIVATE
CallReasonInfo StartErrorHandler::HandleTriggerEpsfb(IN const IMessage& /*objMessage*/) const
{
    IMS_TRACE_I("HandleTriggerEpsfb", 0, 0, 0);
    if (m_objContext.GetService().IsNr())
    {
        return CallReasonInfo(CODE_INTERNAL_REDIAL, EXTRA_CODE_REDIAL_AFTER_EPS_FALLBACK);
    }
    return CallReasonInfo(CODE_NONE);
}

PRIVATE
CallReasonInfo StartErrorHandler::HandleTerminateByResponseSource(
        IN const IMessage& objMessage) const
{
    IMS_TRACE_I("HandleTerminateByResponseSource", 0, 0, 0);
    if (!m_objContext.GetMessageUtils().IsHeaderPresent(&objMessage, ISipHeader::RETRY_AFTER_SEC))
    {
        if (IsIpcanResourceUnavailable(objMessage))
        {
            // TS 24.229 5.1.3.1: There's the method to examine headers but no further behavior.
            return CallReasonInfo(
                    CODE_SIP_SERVER_ERROR, GetDefaultExtraCode(m_objContext, objMessage));
        }
    }

    return CallReasonInfo(CODE_NONE);
}

CallReasonInfo StartErrorHandler::HandleTerminateByReasonHeaderText(
        IN const IMessage& objMessage) const
{
    ReasonHeaderValue objValue =
            m_objContext.GetMessageUtils().GetCauseAndTextFromReasonHeader(&objMessage);

    if (objValue.strText.GetLength() > 0)
    {
        return CallReasonInfo(GetDefaultReasonCode(m_objContext, objMessage.GetStatusCode()), -1,
                objValue.strText.SimplifyWsp());
    }

    return CallReasonInfo(CODE_NONE);
}

PRIVATE
CallReasonInfo StartErrorHandler::RegisterAfterMayPerformCsfb() const
{
    IMS_TRACE_I("RegisterAfterMayPerformCsfb", 0, 0, 0);

    if (m_objContext.GetService().IsCsfbAvailable())
    {
        ControlAos(ImsAosControl::REGISTER_REINITIATE_BY_CSFB);
        return CallReasonInfo(
                CODE_LOCAL_CALL_CS_RETRY_REQUIRED, EXTRA_CODE_CALL_RETRY_SILENT_REDIAL);
    }

    ControlAos(ImsAosControl::REGISTER_REINITIATE);
    return CallReasonInfo(CODE_NETWORK_RESP_TIMEOUT, EXTRA_CODE_METHOD_INVITE);
}

PRIVATE
IMS_BOOL StartErrorHandler::IsTransactionTimeout(IN const IMessage* piMessage)
{
    return piMessage == IMS_NULL;
}

PRIVATE
IMS_BOOL StartErrorHandler::IsIpcanResourceUnavailable(IN const IMessage& objMessage) const
{
    IMS_SINT32 nCause =
            m_objContext.GetMessageUtils().GetCauseFromReasonHeader(&objMessage, "FAILURE_CAUSE");

    AString strFeParameter = m_objContext.GetMessageUtils().GetParameterValue(
            &objMessage, "fe", ISipHeader::UNKNOWN, "Response-Source");

    return nCause == 1 && strFeParameter.Contains("urn:3gpp:fe:p-cscf.orig");
}

PRIVATE
IMS_BOOL StartErrorHandler::IsAlternativeEmergencyService(IN const IMessage& objMessage) const
{
    Ims3gppData objIms3gppData = m_objContext.GetMessageUtils().GetIms3gppData(&objMessage);
    if (objIms3gppData.eType != Ims3gpp::TYPE_ALTERNATIVE_SERVICE)
    {
        return IMS_FALSE;
    }

    return objIms3gppData.eAlternativeServiceType == Ims3gpp::AlternativeService::TYPE_EMERGENCY;
}

PRIVATE
IMS_BOOL StartErrorHandler::IsInitialRegistrationRequired(IN const IMessage& objMessage) const
{
    Ims3gppData objIms3gppData = m_objContext.GetMessageUtils().GetIms3gppData(&objMessage);
    if (objIms3gppData.eType != Ims3gpp::TYPE_ALTERNATIVE_SERVICE)
    {
        return IMS_FALSE;
    }

    return objIms3gppData.eAlternativeServiceType ==
            Ims3gpp::AlternativeService::TYPE_RESTORATION &&
            objIms3gppData.eAlternativeServiceAction ==
            Ims3gpp::AlternativeService::ACTION_INITIAL_REGISTRATION;
}

PRIVATE
IMS_BOOL StartErrorHandler::IsCsfbActionRequiredStatusCode(IN IMS_SINT32 nStatusCode) const
{
    return MtcConfigurationResolver::LookupActionForStatusCode(m_objContext.GetConfigurationProxy(),
            ConfigVoice::KEY_REJECT_CODE_AND_ACTION_SET_STRING_ARRAY, nStatusCode)
            .Contains(ConfigVoice::START_ERROR_ACTION_CSFB);
}

PRIVATE
void StartErrorHandler::ControlAos(IN IMS_UINT32 nCommand) const
{
    IMtcAosConnector* pAosConnector = m_objContext.GetService().GetAosConnector();
    if (pAosConnector)
    {
        pAosConnector->Control(nCommand);
    }
}

PRIVATE
IMS_BOOL StartErrorHandler::RegisterFor503(IN IMS_SINT32 nRetryAfter) const
{
    IMtcAosConnector* pAosConnector = m_objContext.GetService().GetAosConnector();
    if (pAosConnector)
    {
        pAosConnector->RegisterWithNextPcscf(nRetryAfter > 0 ? nRetryAfter : 0);
        return IMS_TRUE;
    }

    return IMS_FALSE;
}

PRIVATE
IMS_BOOL StartErrorHandler::IsRegisterWithNextPcscfAndRedialRequiredFor503(
        IN IMS_SINT32 nRetryAfter) const
{
    return nRetryAfter <= 0 ||
            nRetryAfter * 1000 > Engine::GetConfiguration()
                                         ->GetSipConfig(m_objContext.GetSlotId())
                                         ->GetSipConfigV()
                                         ->GetTimerValue(ISipConfigV::TIMER_B);
}

PRIVATE
AString StartErrorHandler::GetPathHeader() const
{
    IMtcAosConnector* pAosConnector = m_objContext.GetService().GetAosConnector();
    return pAosConnector ? pAosConnector->GetPathHeaderValue() : AString::ConstNull();
}

PRIVATE
AString StartErrorHandler::GetServiceRouteHeader() const
{
    IMtcAosConnector* pAosConnector = m_objContext.GetService().GetAosConnector();
    return pAosConnector ? pAosConnector->GetServiceRouteHeaderValue() : AString::ConstNull();
}

PRIVATE
AString StartErrorHandler::GetSupported() const
{
    IMtcAosConnector* pAosConnector = m_objContext.GetService().GetAosConnector();
    return pAosConnector ? pAosConnector->GetSupportedHeaderValue() : AString::ConstNull();
}

PRIVATE
void StartErrorHandler::SetTimerForImsCallBlocking(IN IMS_SINT32 nRetryAfterInMillis) const
{
    if (nRetryAfterInMillis > 0)
    {
        m_objContext.GetPassiveTimerHolder().AddTimer(
                IPassiveTimerHolder::Type::CALL_BLOCKED_BY_RETRY_AFTER, nRetryAfterInMillis);
    }
}
