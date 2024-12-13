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
#include "ISipConfig.h"
#include "ISipConfigV.h"
#include "ISipHeader.h"
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
#include "call/termination/StartErrorHandler.h"
#include "configuration/MtcConfigurationProxy.h"
#include "core/IMessageBodyPart.h"
#include "core/ISession.h"
#include "helper/IMtcAosConnector.h"
#include "helper/IPassiveTimerHolder.h"
#include "helper/MtcTimerWrapper.h"
#include "media/IMtcMediaManager.h"
#include "media/MtcMediaUtil.h"
#include "sipcore/ISipMessage.h"
#include "utility/IMessageUtils.h"

__IMS_TRACE_TAG_COM_MTC__;

LOCAL const AString REASON_TEXT_MAX_CALL_LIMIT_REACHED_VZW =
        "simultaneous call limit has already been reached";

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
    if (m_objContext.GetCallInfo().bEmergency)
    {
        // According to Domain Selection and legacy Call Framework behavior,
        // IMS Stack should use same failure reason as normal call.
        // Emergency Call is redialed for all reasons except
        // CODE_USER_TERMINATED, CODE_USER_TERMINATED_BY_REMOTE, and CODE_SIP_USER_REJECTED
        IMS_TRACE_D("Handle : Emergency Call Failure.", 0, 0, 0);

        if (IsRedialEmergencyWithNextPcscfRequired(piMessage))
        {
            return HandleRedialEmergencyWithNextPcscf();
        }
    }

    if (IsTransactionTimeout(piMessage))
    {
        IMS_TRACE_I("Handle : Timeout", 0, 0, 0);
        return HandleTransactionTimeout();
    }

    if (IsConditionCheckRequiredBeforeRetry1x(*piMessage) == IMS_FALSE &&
            IsRetry1xRequiredForNormalCall(*piMessage))
    {
        return CallReasonInfo(
                CODE_LOCAL_CALL_CS_RETRY_REQUIRED, EXTRA_CODE_CALL_RETRY_SILENT_REDIAL);
    }

    if (m_objContext.GetCallInfo().bEmergency)
    {
        return CallReasonInfo(
                CODE_LOCAL_CALL_CS_RETRY_REQUIRED, EXTRA_CODE_CALL_RETRY_EMERGENCY);
    }

    return HandleResponse(*piMessage);
}

PRIVATE
CallReasonInfo StartErrorHandler::HandleTransactionTimeout() const
{
    if (m_objContext.GetCallInfo().bEmergency)
    {
        return CallReasonInfo(CODE_NETWORK_RESP_TIMEOUT, EXTRA_CODE_METHOD_INVITE);
    }

    Feature eFeature = m_objContext.GetService().IsWlanIpCanType()
            ? Feature::POLICY_FOR_TCALL_TIMER_EXPIRY_OF_VOWIFI_CALL
            : Feature::POLICY_FOR_TCALL_TIMER_EXPIRY_OF_VOLTE_CALL;

    const IMS_SINT32 nPolicy = m_objContext.GetConfigurationProxy().GetInt(eFeature);
    IMS_SINT32 nReason = CODE_NETWORK_RESP_TIMEOUT;
    IMS_SINT32 nExtraCode = EXTRA_CODE_METHOD_INVITE;
    switch (nPolicy)
    {
        case CarrierConfig::ImsVoice::MO_CALL_REQUEST_TIMEOUT_POLICY_CALL_END:
            break;
        case CarrierConfig::ImsVoice::MO_CALL_REQUEST_TIMEOUT_POLICY_WAIT_FOR_RESPONSE:
            ControlAos(ImsAosControl::PCSCF_NEXT);
            break;
        case CarrierConfig::ImsVoice::MO_CALL_REQUEST_TIMEOUT_POLICY_CSFB:
        case CarrierConfig::ImsVoice::MO_CALL_REQUEST_TIMEOUT_POLICY_CSFB_IF_AVAILABLE:
            if (m_objContext.GetService().IsEpsCombinedAttach())
            {
                nReason = CODE_LOCAL_CALL_CS_RETRY_REQUIRED;
                nExtraCode = EXTRA_CODE_CALL_RETRY_SILENT_REDIAL;
            }
            break;
        case CarrierConfig::ImsVoice::MO_CALL_REQUEST_TIMEOUT_POLICY_INITIAL_REGISTER_CURRENT_PCSCF:
            ControlAos(ImsAosControl::REGISTER_REINITIATE);
            break;
        case CarrierConfig::ImsVoice::MO_CALL_REQUEST_TIMEOUT_POLICY_INITIAL_REGISTER_NEXT_PCSCF:
            ControlAos(ImsAosControl::PCSCF_NEXT);
            break;
        case CarrierConfig::ImsVoice::
                MO_CALL_REQUEST_TIMEOUT_POLICY_INITIAL_REGISTER_WITH_PDN_RECONNECT_AFTER_CSFB:
            if (m_objContext.GetService().IsEpsCombinedAttach())
            {
                nReason = CODE_LOCAL_CALL_CS_RETRY_REQUIRED;
                nExtraCode = EXTRA_CODE_CALL_RETRY_SILENT_REDIAL;
                ControlAos(ImsAosControl::REGISTER_REINITIATE_BY_CSFB);  // TODO: check timing
            }
            break;
        case CarrierConfig::ImsVoice::MO_CALL_REQUEST_TIMEOUT_POLICY_SILENT_REDIAL:
            nReason = CODE_INTERNAL_REDIAL;
            nExtraCode = EXTRA_CODE_REDIAL_BY_REQUEST_TIMEOUT;
            break;
        case CarrierConfig::ImsVoice::MO_CALL_REQUEST_TIMEOUT_POLICY_REDIAL_BY_NETWORK_CONTEXT:
            return HandleRedialByNetworkContext();
    }

    return CallReasonInfo(nReason, nExtraCode);
}

PRIVATE
CallReasonInfo StartErrorHandler::HandleResponse(IN const IMessage& objMessage) const
{
    IMS_SINT32 nStatusCode = objMessage.GetStatusCode();

    if (SipStatusCode::SC_300 <= nStatusCode && nStatusCode < SipStatusCode::SC_400)
    {
        return Handle3xxResponse(objMessage);
    }
    else if (SipStatusCode::SC_400 <= nStatusCode && nStatusCode < SipStatusCode::SC_500)
    {
        return Handle4xxResponse(objMessage);
    }
    else if (SipStatusCode::SC_500 <= nStatusCode && nStatusCode < SipStatusCode::SC_600)
    {
        return Handle5xxResponse(objMessage);
    }
    else if (SipStatusCode::SC_600 <= nStatusCode && nStatusCode < SipStatusCode::SC_MAX)
    {
        return Handle6xxResponse(objMessage);
    }
    return CallReasonInfo(CODE_SIP_SERVER_ERROR, GetDefaultExtraCode(objMessage));
}

PRIVATE
CallReasonInfo StartErrorHandler::Handle3xxResponse(IN const IMessage& objMessage) const
{
    IMS_SINT32 nStatusCode = objMessage.GetStatusCode();

    switch (nStatusCode)
    {
        case SipStatusCode::SC_300:
        case SipStatusCode::SC_301:
        case SipStatusCode::SC_302:
        case SipStatusCode::SC_305:
            return HandleRedirection(objMessage);
        case SipStatusCode::SC_380:
            return Handle380Response(objMessage);
    }

    return CallReasonInfo(CODE_SIP_REDIRECTED, GetDefaultExtraCode(objMessage));
}

PRIVATE
CallReasonInfo StartErrorHandler::HandleRedirection(IN const IMessage& objMessage) const
{
    AString strContact =
            m_objContext.GetMessageUtils().GetHeaderValue(&objMessage, ISipHeader::CONTACT_NORMAL);
    if (strContact.GetLength() > 0)
    {
        // TODO: silent redial with the Contact header to be implemented
        return CallReasonInfo(CODE_INTERNAL_REDIAL, EXTRA_CODE_REDIAL_FOR_REDIRECTION, strContact);
    }

    if (IsRetry1xRequiredForNormalCall(objMessage))
    {
        return CallReasonInfo(
                CODE_LOCAL_CALL_CS_RETRY_REQUIRED, EXTRA_CODE_CALL_RETRY_SILENT_REDIAL);
    }

    return CallReasonInfo(CODE_SIP_REDIRECTED, GetDefaultExtraCode(objMessage));
}

PRIVATE
CallReasonInfo StartErrorHandler::Handle380Response(IN const IMessage& objMessage) const
{
    IMS_SINT32 eSosType = m_objContext.GetMessageUtils().GetSosTypeFromServiceUrn(
            &objMessage, ISipHeader::CONTACT_NORMAL);
    if (eSosType != EXTRA_CODE_EMERGENCYSERVICE_INVALID &&
            IsNonUeDetectableEmergencyCall(objMessage))
    {
        return CallReasonInfo(CODE_SIP_ALTERNATE_EMERGENCY_CALL, eSosType);
    }

    if (IsAlternativeEmergencyService(objMessage))
    {
        return CallReasonInfo(
                CODE_SIP_ALTERNATE_EMERGENCY_CALL, EXTRA_CODE_EMERGENCYSERVICE_GENERIC);
    }

    if (IsRetry1xRequiredForNormalCall(objMessage))
    {
        return CallReasonInfo(
                CODE_LOCAL_CALL_CS_RETRY_REQUIRED, EXTRA_CODE_CALL_RETRY_SILENT_REDIAL);
    }
    return CallReasonInfo(CODE_SIP_REDIRECTED, GetDefaultExtraCode(objMessage));
}

PRIVATE
CallReasonInfo StartErrorHandler::Handle4xxResponse(IN const IMessage& objMessage) const
{
    IMS_SINT32 nStatusCode = objMessage.GetStatusCode();

    switch (nStatusCode)
    {
        case SipStatusCode::SC_400:
            return CallReasonInfo(CODE_SIP_BAD_REQUEST, GetDefaultExtraCode(objMessage));
        case SipStatusCode::SC_401:
            return CallReasonInfo(CODE_SIP_CLIENT_ERROR, GetDefaultExtraCode(objMessage));
        case SipStatusCode::SC_403:
            return Handle403Response(objMessage);
        case SipStatusCode::SC_404:
            return Handle404Response();
        case SipStatusCode::SC_405:
            return CallReasonInfo(CODE_SIP_METHOD_NOT_ALLOWED, GetDefaultExtraCode(objMessage));
        case SipStatusCode::SC_406:
            return CallReasonInfo(CODE_SIP_NOT_ACCEPTABLE, GetDefaultExtraCode(objMessage));
        case SipStatusCode::SC_407:
            return Handle407Response();
        case SipStatusCode::SC_408:
            return CallReasonInfo(CODE_SIP_REQUEST_TIMEOUT, GetDefaultExtraCode(objMessage));
        case SipStatusCode::SC_410:
            return CallReasonInfo(CODE_SIP_NOT_REACHABLE, GetDefaultExtraCode(objMessage));
        case SipStatusCode::SC_413:
            return CallReasonInfo(
                    CODE_SIP_REQUEST_ENTITY_TOO_LARGE, GetDefaultExtraCode(objMessage));
        case SipStatusCode::SC_414:
            return CallReasonInfo(CODE_SIP_REQUEST_URI_TOO_LARGE, GetDefaultExtraCode(objMessage));
        case SipStatusCode::SC_415:
        case SipStatusCode::SC_416:
        case SipStatusCode::SC_420:
            return CallReasonInfo(CODE_SIP_NOT_SUPPORTED, GetDefaultExtraCode(objMessage));
        case SipStatusCode::SC_421:
            // re-INVITE should be sent with the extension of Require header using Supported header
            return CallReasonInfo(CODE_SIP_EXTENSION_REQUIRED, GetDefaultExtraCode(objMessage));
        case SipStatusCode::SC_422:
            // re-INVITE is sent by the engine without notification if it has Min-SE header
            // so, if there is no Min-SE header, CODE_SIP_INTERVAL_TOO_BRIEF will be used
            return CallReasonInfo(CODE_SIP_INTERVAL_TOO_BRIEF, GetDefaultExtraCode(objMessage));
        case SipStatusCode::SC_480:
            return CallReasonInfo(CODE_SIP_TEMPRARILY_UNAVAILABLE, GetDefaultExtraCode(objMessage));
        case SipStatusCode::SC_481:
            return CallReasonInfo(
                    CODE_SIP_TRANSACTION_DOES_NOT_EXIST, GetDefaultExtraCode(objMessage));
        case SipStatusCode::SC_482:
            return CallReasonInfo(CODE_SIP_LOOP_DETECTED, GetDefaultExtraCode(objMessage));
        case SipStatusCode::SC_483:
            return CallReasonInfo(CODE_SIP_TOO_MANY_HOPS, GetDefaultExtraCode(objMessage));
        case SipStatusCode::SC_484:
            return CallReasonInfo(CODE_SIP_BAD_ADDRESS, GetDefaultExtraCode(objMessage));
        case SipStatusCode::SC_485:
            return CallReasonInfo(CODE_SIP_AMBIGUOUS, GetDefaultExtraCode(objMessage));
        case SipStatusCode::SC_486:
            return CallReasonInfo(CODE_SIP_BUSY, GetDefaultExtraCode(objMessage));
        case SipStatusCode::SC_487:
            return CallReasonInfo(CODE_SIP_REQUEST_CANCELLED, GetDefaultExtraCode(objMessage));
        case SipStatusCode::SC_488:
            return Handle488Response(objMessage);
        case SipStatusCode::SC_491:
            return CallReasonInfo(CODE_SIP_REQUEST_PENDING, GetDefaultExtraCode(objMessage));
        case SipStatusCode::SC_493:
            return CallReasonInfo(CODE_SIP_UNDECIPHERABLE, GetDefaultExtraCode(objMessage));
        case SipStatusCode::SC_499:  // only for SKT VT
            return CallReasonInfo(CODE_SIP_NOT_REACHABLE, GetDefaultExtraCode(objMessage));
    }

    return CallReasonInfo(CODE_SIP_SERVER_ERROR, GetDefaultExtraCode(objMessage));
}

PRIVATE
CallReasonInfo StartErrorHandler::Handle403Response(IN const IMessage& objMessage) const
{
    if (IsByMaxCallLimit(objMessage))
    {
        return CallReasonInfo(CODE_MAXIMUM_NUMBER_OF_CALLS_REACHED);
    }

    const IMS_SINT32 nPolicy = m_objContext.GetConfigurationProxy().GetInt(
            Feature::POLICY_FOR_403_RESPONSE_FOR_INVITE);
    switch (nPolicy)
    {
        case CarrierConfig::ImsVoice::SIP_403_POLICY_TERMINATE_CALL:
            break;

        case CarrierConfig::ImsVoice::SIP_403_POLICY_TERMINATE_CALL_AND_RECOVER_REGISTRATION:
            ControlAos(ImsAosControl::REGISTER_REINITIATE);
            break;

        case CarrierConfig::ImsVoice::SIP_403_POLICY_TERMINATE_CALL_AND_REFRESH_REGISTRATION:
            ControlAos(ImsAosControl::REGISTER_REFRESH);
            break;

        case CarrierConfig::ImsVoice::SIP_403_POLICY_CSFB:
            if (m_objContext.GetService().IsEpsCombinedAttach())
            {
                return CallReasonInfo(
                        CODE_LOCAL_CALL_CS_RETRY_REQUIRED, EXTRA_CODE_CALL_RETRY_SILENT_REDIAL);
            }
            break;

        case CarrierConfig::ImsVoice::SIP_403_POLICY_CSFB_AND_RECOVER_REGISTRATION:
            if (m_objContext.GetService().IsEpsCombinedAttach())
            {
                ControlAos(ImsAosControl::REGISTER_REINITIATE_BY_CSFB);
                return CallReasonInfo(
                        CODE_LOCAL_CALL_CS_RETRY_REQUIRED, EXTRA_CODE_CALL_RETRY_SILENT_REDIAL);
            }
            break;
    }

    return CallReasonInfo(CODE_SIP_FORBIDDEN, SipStatusCode::SC_403);
}

PRIVATE
CallReasonInfo StartErrorHandler::Handle404Response() const
{
    if (m_objContext.GetCallInfo().bUssi && m_objContext.GetService().IsEpsCombinedAttach())
    {
        return CallReasonInfo(
                CODE_LOCAL_CALL_CS_RETRY_REQUIRED, EXTRA_CODE_CALL_RETRY_SILENT_REDIAL);
    }
    else
    {
        return CallReasonInfo(CODE_SIP_NOT_FOUND, SipStatusCode::SC_404);
    }
}

PRIVATE
CallReasonInfo StartErrorHandler::Handle407Response()
{
    // TODO: an initial INVITE must be sent with Authorization.
    return CallReasonInfo(CODE_SIP_PROXY_AUTHENTICATION_REQUIRED, SipStatusCode::SC_407);
}

PRIVATE
CallReasonInfo StartErrorHandler::Handle488Response(IN const IMessage& objMessage) const
{
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

    if (IsRetry1xRequiredForNormalCall(objMessage))
    {
        return CallReasonInfo(
                CODE_LOCAL_CALL_CS_RETRY_REQUIRED, EXTRA_CODE_CALL_RETRY_SILENT_REDIAL);
    }

    return CallReasonInfo(CODE_SIP_NOT_ACCEPTABLE, GetDefaultExtraCode(objMessage));
}

PRIVATE
CallReasonInfo StartErrorHandler::Handle5xxResponse(IN const IMessage& objMessage) const
{
    IMS_SINT32 nStatusCode = objMessage.GetStatusCode();

    switch (nStatusCode)
    {
        case SipStatusCode::SC_500:
            return Handle500Response(objMessage);
        case SipStatusCode::SC_501:
            return CallReasonInfo(CODE_SIP_SERVER_INTERNAL_ERROR, GetDefaultExtraCode(objMessage));
        case SipStatusCode::SC_502:
            return CallReasonInfo(CODE_SIP_SERVER_ERROR, GetDefaultExtraCode(objMessage));
        case SipStatusCode::SC_503:
            return Handle503Response(objMessage);
        case SipStatusCode::SC_504:
            return Handle504Response(objMessage);
        case SipStatusCode::SC_505:
        case SipStatusCode::SC_513:
        case SipStatusCode::SC_580:  // remote precondition failure case
            return CallReasonInfo(CODE_SIP_SERVER_ERROR, GetDefaultExtraCode(objMessage));
    }

    return Handle500Response(objMessage);
}

PRIVATE
CallReasonInfo StartErrorHandler::Handle500Response(IN const IMessage& objMessage) const
{
    if (!m_objContext.GetMessageUtils().IsHeaderPresent(&objMessage, ISipHeader::RETRY_AFTER_SEC))
    {
        if (IsIpcanResourceUnavailable(objMessage))
        {
            // TS 24.229 5.1.3.1: There's the method to examine headers but no further behavior.
            return CallReasonInfo(CODE_SIP_SERVER_ERROR, GetDefaultExtraCode(objMessage));
        }
    }

    return CallReasonInfo(CODE_SIP_SERVER_ERROR, GetDefaultExtraCode(objMessage));
}

PRIVATE
CallReasonInfo StartErrorHandler::Handle503Response(IN const IMessage& objMessage) const
{
    IMS_SINT32 nRetryAfter = m_objContext.GetMessageUtils().GetHeaderValueInt(
            &objMessage, ISipHeader::RETRY_AFTER_ANY);
    IMS_SINT32 nRetryAfterInMillis = nRetryAfter * 1000;
    if (IsRetry1xRequiredForNormalCall(objMessage))
    {
        SetTimerForImsCallBlocking(nRetryAfterInMillis);
        return CallReasonInfo(
                CODE_LOCAL_CALL_CS_RETRY_REQUIRED, EXTRA_CODE_CALL_RETRY_SILENT_REDIAL);
    }

    if (!m_objContext.GetCallManager().GetCallsByState(IMtcCall::State::ESTABLISHED).IsEmpty())
    {
        return CallReasonInfo(CODE_SIP_SERVICE_UNAVAILABLE, objMessage.GetStatusCode());
    }

    if (IsRegisterWithNextPcscfAndRedialRequiredFor503(nRetryAfter))
    {
        if (RegisterFor503(nRetryAfter))
        {
            return CallReasonInfo(CODE_INTERNAL_REDIAL, EXTRA_CODE_REDIAL_WITH_NEXT_PCSCF);
        }

        return CallReasonInfo(CODE_LOCAL_INTERNAL_ERROR);
    }

    if (m_objContext.GetService().IsEpsCombinedAttach())
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
CallReasonInfo StartErrorHandler::Handle504Response(IN const IMessage& objMessage) const
{
    if (m_objContext.GetMessageUtils().ContainsAddressInPaid(&objMessage, GetPathHeader()) ||
            m_objContext.GetMessageUtils().ContainsAddressInPaid(
                    &objMessage, GetServiceRouteHeader()))
    {
        if (IsInitialRegistrationRequired(objMessage))
        {
            const IMS_SINT32 nPolicy = m_objContext.GetConfigurationProxy().GetInt(
                    Feature::REGISTRATION_RESTORATION_MODE_ON_504_FOR_INVITE);
            switch (nPolicy)
            {
                case CarrierConfig::ImsVoice::REGISTRATION_RESTORATION_NOT_AVAILABLE:
                    break;
                case CarrierConfig::ImsVoice::REGISTRATION_RESTORATION_RECOVER_BY_NETWORK_CONTEXT:
                    if (m_objContext.GetService().IsEpsCombinedAttach())
                    {
                        break;
                    }
                    __IMS_FALLTHROUGH__
                case CarrierConfig::ImsVoice::
                        REGISTRATION_RESTORATION_INITIAL_REGISTER_WITH_NEXT_PCSCF:
                    ControlAos(ImsAosControl::PCSCF_NEXT);
                    break;

                case CarrierConfig::ImsVoice::REGISTRATION_RESTORATION_RECOVER_REGISTRATION:
                    // If there is an operator that requires PDN reconnect, AoS I/F should be added.
                case CarrierConfig::ImsVoice::
                        REGISTRATION_RESTORATION_RECOVER_REGISTRATION_WITHOUT_PDN_RECONNECT:
                    ControlAos(ImsAosControl::REGISTER_REINITIATE);
                    break;
            }
        }
    }

    if (IsRetry1xRequiredForNormalCall(objMessage))
    {
        return CallReasonInfo(
                CODE_LOCAL_CALL_CS_RETRY_REQUIRED, EXTRA_CODE_CALL_RETRY_SILENT_REDIAL);
    }

    return CallReasonInfo(CODE_SIP_SERVER_TIMEOUT, GetDefaultExtraCode(objMessage));
}

PRIVATE
CallReasonInfo StartErrorHandler::Handle6xxResponse(IN const IMessage& objMessage) const
{
    IMS_SINT32 nStatusCode = objMessage.GetStatusCode();

    switch (nStatusCode)
    {
        case SipStatusCode::SC_600:
            return CallReasonInfo(CODE_SIP_BUSY, GetDefaultExtraCode(objMessage));
        case SipStatusCode::SC_603:
            return CallReasonInfo(CODE_SIP_USER_REJECTED, GetDefaultExtraCode(objMessage));
        case SipStatusCode::SC_604:
            return CallReasonInfo(CODE_SIP_NOT_REACHABLE, GetDefaultExtraCode(objMessage));
        case SipStatusCode::SC_606:
            return CallReasonInfo(CODE_SIP_NOT_ACCEPTABLE, GetDefaultExtraCode(objMessage));
    }

    return CallReasonInfo(CODE_SIP_GLOBAL_ERROR, GetDefaultExtraCode(objMessage));
}

PRIVATE
CallReasonInfo StartErrorHandler::HandleRedialByNetworkContext() const
{
    if (EpsFallbackTrigger::IsRequired(m_objContext.GetConfigurationProxy()) &&
            m_objContext.GetService().IsNr())
    {
        return CallReasonInfo(CODE_INTERNAL_REDIAL, EXTRA_CODE_REDIAL_AFTER_EPS_FALLBACK);
    }

    if (m_objContext.GetConfigurationProxy().Is(Feature::REQUIRED_CDMALESS_FEATURE_TAG) &&
            !IsRoaming())
    {
        ControlAos(ImsAosControl::REGISTER_REINITIATE);
        return CallReasonInfo(CODE_NETWORK_RESP_TIMEOUT, EXTRA_CODE_METHOD_INVITE);
    }

    if (!m_objContext.GetService().IsEpsCombinedAttach())
    {
        ControlAos(ImsAosControl::REGISTER_REINITIATE);
        return CallReasonInfo(CODE_NETWORK_RESP_TIMEOUT, EXTRA_CODE_METHOD_INVITE);
    }

    if (m_objContext.GetService().IsWlanIpCanType())
    {
        ControlAos(ImsAosControl::REGISTER_REINITIATE);
        return CallReasonInfo(CODE_NETWORK_RESP_TIMEOUT, EXTRA_CODE_METHOD_INVITE);
    }

    ControlAos(ImsAosControl::REGISTER_REINITIATE_BY_CSFB);
    return CallReasonInfo(CODE_LOCAL_CALL_CS_RETRY_REQUIRED, EXTRA_CODE_CALL_RETRY_SILENT_REDIAL);
}

PRIVATE
CallReasonInfo StartErrorHandler::HandleRedialEmergencyWithNextPcscf() const
{
    ControlAos(ImsAosControl::E_REGISTER_FAKE_WITH_NEXT_PCSCF);
    return CallReasonInfo(CODE_INTERNAL_REDIAL, EXTRA_CODE_REDIAL_EMERGENCY_WITH_NEXT_PCSCF);
}

PRIVATE
IMS_SINT32 StartErrorHandler::GetDefaultExtraCode(IN const IMessage& objMessage) const
{
    IMS_SINT32 nExtraCode = m_objContext.GetMessageUtils().GetCauseFromReasonHeader(&objMessage);
    if (nExtraCode == -1)
    {
        nExtraCode = objMessage.GetStatusCode();
    }
    return nExtraCode;
}

PRIVATE
IMS_BOOL StartErrorHandler::IsTransactionTimeout(IN const IMessage* piMessage)
{
    if (piMessage == IMS_NULL)
    {
        return IMS_TRUE;
    }

    return piMessage->GetStatusCode() == SipStatusCode::SC_INVALID;
}

PRIVATE
IMS_BOOL StartErrorHandler::IsRetry1xRequiredForNormalCall(IN const IMessage& objMessage) const
{
    if (m_objContext.GetCallInfo().bEmergency)
    {
        return IMS_FALSE;
    }

    if (!m_objContext.GetService().IsEpsCombinedAttach())
    {
        return IMS_FALSE;
    }

    return m_objContext.GetConfigurationProxy().Is(
            Feature::REJECT_CODE_FOR_CSFB, objMessage.GetStatusCode());
}

PRIVATE
IMS_BOOL StartErrorHandler::IsConditionCheckRequiredBeforeRetry1x(IN const IMessage& objMessage)
{
    switch (objMessage.GetStatusCode())
    {
        case SipStatusCode::SC_300:
        case SipStatusCode::SC_301:
        case SipStatusCode::SC_302:
        case SipStatusCode::SC_305:
        case SipStatusCode::SC_380:
        case SipStatusCode::SC_403:
        // POLICY_FOR_403_RESPONSE_FOR_INVITE overrides REJECT_CODE_FOR_CSFB
        case SipStatusCode::SC_488:
        case SipStatusCode::SC_503:
        case SipStatusCode::SC_504:
            return IMS_TRUE;
    }

    return IMS_FALSE;
}

PRIVATE
IMS_BOOL StartErrorHandler::IsNonUeDetectableEmergencyCall(IN const IMessage& objMessage) const
{
    // clang-format off
    if (m_objContext.GetConfigurationProxy().Is(Feature::
            EMERGENCY_RETRY_WITHOUT_CHECKING380_CONTENT_FOR_NON_UE_DETECTABLE_EMERGENCY_CALL))
    {
        return IMS_TRUE;
    }
    // clang-format on
    if (!IsAlternativeEmergencyService(objMessage))
    {
        return IMS_FALSE;
    }
    // Loose checking for some carriers don't use Path header during registration
    AString strSupported = GetSupported();
    if (strSupported.GetLength() <= 0 || !strSupported.Contains("path"))
    {
        return IMS_TRUE;
    }
    return m_objContext.GetMessageUtils().ContainsAddressInPaid(&objMessage, GetPathHeader());
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
IMS_BOOL StartErrorHandler::IsByMaxCallLimit(IN const IMessage& objMessage) const
{
    const AString strNormalizedReasonPhrase =
            objMessage.GetReasonPhrase().SimplifyWsp().MakeLower();
    if (strNormalizedReasonPhrase.Contains(REASON_TEXT_MAX_CALL_LIMIT_REACHED_VZW))
    {
        return IMS_TRUE;
    }

    ReasonHeaderValue objValue =
            m_objContext.GetMessageUtils().GetCauseAndTextFromReasonHeader(&objMessage);
    const AString strNormalizedText = objValue.strText.SimplifyWsp().MakeLower();
    return strNormalizedText.Contains(REASON_TEXT_MAX_CALL_LIMIT_REACHED_VZW);
}

PRIVATE
IMS_BOOL StartErrorHandler::IsRedialEmergencyWithNextPcscfRequired(
        IN const IMessage* piMessage) const
{
    if (!m_objContext.GetConfigurationProxy().Is(
                Feature::RETRY_EMERGENCY_CALL_OVER_EMERGENCY_PDN_WITH_NEXT_PCSCF))
    {
        return IMS_FALSE;
    }

    if (!m_objContext.GetService().IsEmergency())
    {
        return IMS_FALSE;
    }

    IMS_SINT32 nStatusCode =
            (piMessage == IMS_NULL) ? SipStatusCode::SC_INVALID : piMessage->GetStatusCode();
    if (nStatusCode >= SipStatusCode::SC_300 && nStatusCode < SipStatusCode::SC_400)
    {
        return IMS_FALSE;
    }

    if (m_objContext.GetMessageUtils().GetNumberOfPreviousResponses(
                &m_objContext.GetSession()->GetISession(), IMessage::SESSION_START) > 1)
    {
        return IMS_FALSE;
    }

    // support re-dial emergency with next P-CSCF,
    // an emergency call is over sos pdn,
    // and it receives error response except 3xx before receiving 100 Trying.
    return IMS_TRUE;
}

PRIVATE
IMS_BOOL StartErrorHandler::IsRoaming() const
{
    return m_objContext.GetImsEventReceiver().GetWParam(IMS_EVENT_ROAMING_STATE) ==
            IMS_ROAMING_STATE_ON;
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
