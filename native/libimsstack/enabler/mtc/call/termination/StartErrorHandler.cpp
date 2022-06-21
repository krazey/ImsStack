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

#include "CarrierConfig.h"
#include "IMessage.h"
#include "Ims3gpp.h"
#include "ImsAosParameter.h"
#include "ISipHeader.h"
#include "ServiceTrace.h"
#include "SipAddress.h"
#include "SipStatusCode.h"
#include "call/IMtcCallContext.h"
#include "call/termination/StartErrorHandler.h"
#include "configuration/MtcConfigurationProxy.h"
#include "helper/IMtcAosConnector.h"
#include "utility/MessageUtil.h"

__IMS_TRACE_TAG_COM_MTC__;

PUBLIC
StartErrorHandler::StartErrorHandler(IN IMtcCallContext& objContext) :
        m_objContext(objContext)
{
}

PUBLIC
StartErrorHandler::~StartErrorHandler() {}

PUBLIC
CallReasonInfo StartErrorHandler::Handle(IN const IMessage* piMessage) const
{
    if (IsTransactionTimeout(piMessage))
    {
        IMS_TRACE_I("Handle : Timeout", 0, 0, 0);
        return GetCallReasonInfoForTransactionTimeout();
    }

    if (!m_objContext.GetCallInfo().bEmergency && IsRetry1xRequiredForNormalCall(*piMessage))
    {
        return CallReasonInfo(CODE_LOCAL_CALL_CS_RETRY_REQUIRED);
    }

    return HandleResponse(*piMessage);
}

PRIVATE
CallReasonInfo StartErrorHandler::GetCallReasonInfoForTransactionTimeout() const
{
    if (m_objContext.GetCallInfo().bEmergency)
    {
        return CallReasonInfo(CODE_LOCAL_CALL_CS_RETRY_REQUIRED);
    }
    else
    {
        return CallReasonInfo(CODE_NETWORK_RESP_TIMEOUT);
    }
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
    return CallReasonInfo(CODE_SIP_SERVER_ERROR);
}

PRIVATE
CallReasonInfo StartErrorHandler::Handle3xxResponse(IN const IMessage& objMessage) const
{
    IMS_SINT32 nStatusCode = objMessage.GetStatusCode();

    switch (nStatusCode)
    {
        case SipStatusCode::SC_380:
            return Handle380Response(objMessage);
    }

    return CallReasonInfo(CODE_SIP_SERVER_ERROR, nStatusCode);
}

PRIVATE
CallReasonInfo StartErrorHandler::Handle380Response(IN const IMessage& objMessage) const
{
    IMS_SINT32 eSosType =
            MessageUtil::GetSosTypeFromServiceUrn(&objMessage, ISipHeader::CONTACT_NORMAL);
    if (eSosType != EXTRA_CODE_EMERGENCYSERVICE_INVALID &&
            IsNonUeDetectableEmergencyCall(objMessage))
    {
        // TODO : need to modify this after emergency domain selection policy is decided.
        return CallReasonInfo(
                CODE_LOCAL_CALL_CS_RETRY_REQUIRED /*FAIL_REASON_SESSION_RETRY_R_RAT*/, eSosType);
    }

    if (HasEmergencyServiceTypeInBody(objMessage))
    {
        // Set to EXTRA_CODE_CALL_RETRY_NORMAL even though it's emergency service.
        // Call app will retry according to the UX scenario.
        return CallReasonInfo(CODE_LOCAL_CALL_CS_RETRY_REQUIRED, EXTRA_CODE_CALL_RETRY_NORMAL);
    }
    else
    {
        return CallReasonInfo(CODE_LOCAL_CALL_CS_RETRY_REQUIRED);
    }
}

PRIVATE
CallReasonInfo StartErrorHandler::Handle4xxResponse(IN const IMessage& objMessage) const
{
    IMS_SINT32 nStatusCode = objMessage.GetStatusCode();

    switch (nStatusCode)
    {
        case SipStatusCode::SC_401:
            return CallReasonInfo(
                    CODE_SIP_CLIENT_ERROR, MessageUtil::GetCauseFromReasonHeader(&objMessage));
        case SipStatusCode::SC_403:
            return Handle403Response();
        case SipStatusCode::SC_404:
            return Handle404Response();
        case SipStatusCode::SC_406:
            return CallReasonInfo(
                    CODE_SIP_NOT_ACCEPTABLE, MessageUtil::GetCauseFromReasonHeader(&objMessage));
        case SipStatusCode::SC_407:
            return Handle407Response();
        case SipStatusCode::SC_408:
            return CallReasonInfo(
                    CODE_SIP_REQUEST_TIMEOUT, MessageUtil::GetCauseFromReasonHeader(&objMessage));
        case SipStatusCode::SC_415:
        case SipStatusCode::SC_416:
            return CallReasonInfo(
                    CODE_SIP_NOT_SUPPORTED, MessageUtil::GetCauseFromReasonHeader(&objMessage));
        case SipStatusCode::SC_422:
            // re-INVITE is sent by the engine without notification if it has Min-SE header
            return CallReasonInfo(CODE_LOCAL_CALL_CS_RETRY_REQUIRED);
        case SipStatusCode::SC_480:
            return CallReasonInfo(CODE_SIP_TEMPORARILY_UNAVAILABLE,
                    MessageUtil::GetCauseFromReasonHeader(&objMessage));
        case SipStatusCode::SC_484:
            return CallReasonInfo(
                    CODE_SIP_BAD_ADDRESS, MessageUtil::GetCauseFromReasonHeader(&objMessage));
        case SipStatusCode::SC_486:
            return CallReasonInfo(
                    CODE_SIP_BUSY, MessageUtil::GetCauseFromReasonHeader(&objMessage));
        case SipStatusCode::SC_487:
            return CallReasonInfo(
                    CODE_SIP_REQUEST_CANCELLED, MessageUtil::GetCauseFromReasonHeader(&objMessage));
        case SipStatusCode::SC_488:
            return CallReasonInfo(
                    CODE_SIP_NOT_ACCEPTABLE, MessageUtil::GetCauseFromReasonHeader(&objMessage));
        case SipStatusCode::SC_499:
            return CallReasonInfo(
                    CODE_SIP_NOT_REACHABLE, MessageUtil::GetCauseFromReasonHeader(&objMessage));
    }

    return CallReasonInfo(CODE_SIP_SERVER_ERROR, nStatusCode);
}

PRIVATE
CallReasonInfo StartErrorHandler::Handle403Response() const
{
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
    }

    return CallReasonInfo(CODE_SIP_FORBIDDEN, SipStatusCode::SC_403);
}

PRIVATE
CallReasonInfo StartErrorHandler::Handle404Response() const
{
    if (m_objContext.GetCallInfo().bUssi)
    {
        return CallReasonInfo(CODE_LOCAL_CALL_CS_RETRY_REQUIRED);
    }
    else
    {
        return CallReasonInfo(CODE_SIP_NOT_FOUND);
    }
}

PRIVATE
CallReasonInfo StartErrorHandler::Handle407Response() const
{
    if (m_objContext.GetCallInfo().bEmergency)
    {
        return CallReasonInfo(CODE_LOCAL_CALL_CS_RETRY_REQUIRED);
    }
    else
    {
        return CallReasonInfo(CODE_SIP_SERVER_ERROR, SipStatusCode::SC_407);
    }
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
        case SipStatusCode::SC_502:
            return CallReasonInfo(CODE_SIP_SERVER_ERROR, nStatusCode);
        case SipStatusCode::SC_503:
            return Handle503Response(objMessage);
        case SipStatusCode::SC_504:
            return Handle504Response(objMessage);
        case SipStatusCode::SC_505:
        case SipStatusCode::SC_513:
        case SipStatusCode::SC_580:
            return CallReasonInfo(CODE_SIP_SERVER_ERROR, nStatusCode);
    }

    return Handle500Response(objMessage);
}

PRIVATE
CallReasonInfo StartErrorHandler::Handle500Response(IN const IMessage& objMessage) const
{
    if (!MessageUtil::IsHeaderPresent(&objMessage, ISipHeader::RETRY_AFTER_SEC))
    {
        if (IsIpcanResourceUnavailable(objMessage))
        {
            // TS 24.229 5.1.3.1: There's the method to examine headers but no further behavior.
            return CallReasonInfo(
                    CODE_SIP_SERVER_ERROR, MessageUtil::GetCauseFromReasonHeader(&objMessage));
        }
    }

    return CallReasonInfo(
            CODE_SIP_SERVER_ERROR, MessageUtil::GetCauseFromReasonHeader(&objMessage));
}

PRIVATE
CallReasonInfo StartErrorHandler::Handle503Response(IN const IMessage& objMessage) const
{
    IMS_SINT32 nRetryAfter =
            MessageUtil::GetHeaderValueInt(&objMessage, ISipHeader::RETRY_AFTER_ANY);
    if (nRetryAfter > 0)
    {
        // TODO: Set block and CSFB for nRetryAfter duration
        return CallReasonInfo(CODE_LOCAL_CALL_CS_RETRY_REQUIRED);
    }

    return CallReasonInfo(
            CODE_SIP_SERVER_ERROR, MessageUtil::GetCauseFromReasonHeader(&objMessage));
}

PRIVATE
CallReasonInfo StartErrorHandler::Handle504Response(IN const IMessage& objMessage) const
{
    if (MessageUtil::ContainsAddressInPaid(&objMessage, GetPathHeader()) ||
            MessageUtil::ContainsAddressInPaid(&objMessage, GetServiceRouteHeader()))
    {
        if (MessageUtil::IsInitialRegistrationRequired(&objMessage))
        {
            const IMS_SINT32 nPolicy = m_objContext.GetConfigurationProxy().GetInt(
                    Feature::REGISTRATION_RESTORATION_MODE_ON_504_FOR_INVITE);
            switch (nPolicy)
            {
                case CarrierConfig::ImsVoice::REGISTRATION_RESTORATION_NOT_AVAILABLE:
                    break;

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

    return CallReasonInfo(CODE_SIP_SERVER_ERROR, SipStatusCode::SC_504);
}

PRIVATE
CallReasonInfo StartErrorHandler::Handle6xxResponse(IN const IMessage& objMessage) const
{
    IMS_SINT32 nStatusCode = objMessage.GetStatusCode();

    switch (nStatusCode)
    {
        case SipStatusCode::SC_603:
            return CallReasonInfo(
                    CODE_SIP_USER_REJECTED, MessageUtil::GetCauseFromReasonHeader(&objMessage));
    }

    return CallReasonInfo(CODE_SIP_SERVER_ERROR, nStatusCode);
}

PRIVATE
IMS_BOOL StartErrorHandler::IsTransactionTimeout(IN const IMessage* piMessage) const
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
    IMS_SINT32 nStatusCode = objMessage.GetStatusCode();
    return m_objContext.GetConfigurationProxy().Is(Feature::REJECT_CODE_FOR_CSFB, nStatusCode);
}

PRIVATE
IMS_BOOL StartErrorHandler::IsNonUeDetectableEmergencyCall(IN const IMessage& objMessage) const
{
    if (m_objContext.GetConfigurationProxy().Is(Feature::
            EMERGENCY_RETRY_WITHOUT_CHECKING380_CONTENT_FOR_NON_UE_DETECTABLE_EMERGENCY_CALL))
    {
        return IMS_TRUE;
    }

    if (!HasEmergencyServiceTypeInBody(objMessage))
    {
        return IMS_FALSE;
    }

    // Loose checking for some carriers don't use Path header during registration
    AString strSupported = GetSupported();
    if (strSupported.GetLength() <= 0 || !strSupported.Contains("path"))
    {
        return IMS_TRUE;
    }

    return MessageUtil::ContainsAddressInPaid(&objMessage, GetPathHeader());
}

PRIVATE
IMS_BOOL StartErrorHandler::IsIpcanResourceUnavailable(IN const IMessage& objMessage) const
{
    IMS_SINT32 nCause = MessageUtil::GetCauseFromReasonHeader(&objMessage, "FAILURE_CAUSE");

    AString strFeParameter;
    MessageUtil::GetParameterValue(
            &objMessage, "fe", ISipHeader::UNKNOWN, strFeParameter, "Response-Source");

    return nCause == 1 && strFeParameter.Contains("urn:3gpp:fe:p-cscf.orig");
}

PRIVATE
IMS_BOOL StartErrorHandler::HasEmergencyServiceTypeInBody(IN const IMessage& objMessage) const
{
    Ims3gpp objIms3gpp;
    if (MessageUtil::GetIms3gppFromBody(&objMessage, objIms3gpp) == IMS_FAILURE)
    {
        return IMS_FALSE;
    }

    return objIms3gpp.GetAlternativeService().GetType() ==
            Ims3gpp::AlternativeService::TYPE_EMERGENCY;
}

PRIVATE
void StartErrorHandler::ControlAos(IMS_UINT32 nCommand) const
{
    IMtcAosConnector* pAosConnector = GetAosConnector();
    if (pAosConnector)
    {
        pAosConnector->Control(nCommand);
    }
}

PRIVATE
AString StartErrorHandler::GetPathHeader() const
{
    IMtcAosConnector* pAosConnector = GetAosConnector();
    return pAosConnector ? pAosConnector->GetPathHeaderValue() : AString::ConstNull();
}

PRIVATE
AString StartErrorHandler::GetLastPathHeader() const
{
    IMtcAosConnector* pAosConnector = GetAosConnector();
    return pAosConnector ? pAosConnector->GetLastPathHeaderValue() : AString::ConstNull();
}

PRIVATE
AString StartErrorHandler::GetServiceRouteHeader() const
{
    IMtcAosConnector* pAosConnector = GetAosConnector();
    return pAosConnector ? pAosConnector->GetServiceRouteHeaderValue() : AString::ConstNull();
}

PRIVATE
AString StartErrorHandler::GetSupported() const
{
    IMtcAosConnector* pAosConnector = GetAosConnector();
    return pAosConnector ? pAosConnector->GetSupportedHeaderValue() : AString::ConstNull();
}

PRIVATE
IMtcAosConnector* StartErrorHandler::GetAosConnector() const
{
    IMtcAosConnector* pAosConnector = m_objContext.GetService().GetAosConnector();
    if (pAosConnector == IMS_NULL)
    {
        IMS_TRACE_E(0, "GetAosConnector : AosConnector is null", 0, 0, 0);
    }
    return pAosConnector;
}
