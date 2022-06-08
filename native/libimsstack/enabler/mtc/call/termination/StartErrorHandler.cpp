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
#include "helper/MtcAosConnector.h"
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
FailReason StartErrorHandler::Handle(IN const IMessage* piMessage) const
{
    if (IsTransactionTimeout(piMessage))
    {
        IMS_TRACE_I("Handle : Timeout", 0, 0, 0);
        return GetFailReasonForTransactionTimeout();
    }

    if (!m_objContext.GetCallInfo().bEmergency && IsRetry1xRequiredForNormalCall(*piMessage))
    {
        return FailReason(FAIL_REASON_SESSION_RETRY1X);
    }

    return HandleResponse(*piMessage);
}

PRIVATE
FailReason StartErrorHandler::GetFailReasonForTransactionTimeout() const
{
    if (m_objContext.GetCallInfo().bEmergency)
    {
        return FailReason(FAIL_REASON_SESSION_RETRY1X);
    }
    else
    {
        return FailReason(FAIL_REASON_SESSION_SERVERERROR);
    }
}

PRIVATE
FailReason StartErrorHandler::HandleResponse(IN const IMessage& objMessage) const
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
    return FailReason(FAIL_REASON_SESSION_SERVERERROR);
}

PRIVATE
FailReason StartErrorHandler::Handle3xxResponse(IN const IMessage& objMessage) const
{
    IMS_SINT32 nStatusCode = objMessage.GetStatusCode();

    switch (nStatusCode)
    {
        case SipStatusCode::SC_380:
            return Handle380Response(objMessage);
    }

    return FailReason(FAIL_REASON_SESSION_SERVERERROR, nStatusCode);
}

PRIVATE
FailReason StartErrorHandler::Handle380Response(IN const IMessage& objMessage) const
{
    IMS_SINT32 eSosType =
            MessageUtil::GetSosTypeFromServiceUrn(&objMessage, ISipHeader::CONTACT_NORMAL);
    if (eSosType != CODE_EMERGENCYSERVICE_INVALID && IsNonUeDetectableEmergencyCall(objMessage))
    {
        return FailReason(FAIL_REASON_SESSION_RETRY_R_RAT, eSosType);
    }

    if (HasEmergencyServiceTypeInBody(objMessage))
    {
        // Set to CODE_1XRETRY_NORMAL even though it's emergency service.
        // Call app will retry according to the UX scenario.
        return FailReason(FAIL_REASON_SESSION_RETRY1X, CODE_1XRETRY_NORMAL);
    }
    else
    {
        return FailReason(FAIL_REASON_SESSION_RETRY1X);
    }
}

PRIVATE
FailReason StartErrorHandler::Handle4xxResponse(IN const IMessage& objMessage) const
{
    IMS_SINT32 nStatusCode = objMessage.GetStatusCode();

    switch (nStatusCode)
    {
        case SipStatusCode::SC_401:
            return FailReason(FAIL_REASON_SESSION_SERVER_AUTH,
                    MessageUtil::GetCauseFromReasonHeader(&objMessage));
        case SipStatusCode::SC_403:
            return Handle403Response();
        case SipStatusCode::SC_404:
            return Handle404Response();
        case SipStatusCode::SC_406:
            return FailReason(FAIL_REASON_SESSION_NOTACCEPTABLE,
                    MessageUtil::GetCauseFromReasonHeader(&objMessage));
        case SipStatusCode::SC_407:
            return Handle407Response();
        case SipStatusCode::SC_408:
            return FailReason(FAIL_REASON_SESSION_TIMEOUT,
                    MessageUtil::GetCauseFromReasonHeader(&objMessage));
        case SipStatusCode::SC_415:
        case SipStatusCode::SC_416:
            return FailReason(FAIL_REASON_SESSION_NOTSUPPORTED,
                    MessageUtil::GetCauseFromReasonHeader(&objMessage));
        case SipStatusCode::SC_422:
            // re-INVITE is sent by the engine without notification if it has Min-SE header
            return FailReason(FAIL_REASON_SESSION_RETRY1X);
        case SipStatusCode::SC_480:
            return FailReason(FAIL_REASON_SESSION_TEMPUNAVAILABLE,
                    MessageUtil::GetCauseFromReasonHeader(&objMessage));
        case SipStatusCode::SC_484:
            return FailReason(FAIL_REASON_SESSION_BADADDRESS,
                    MessageUtil::GetCauseFromReasonHeader(&objMessage));
        case SipStatusCode::SC_486:
            return FailReason(
                    FAIL_REASON_SESSION_BUSY, MessageUtil::GetCauseFromReasonHeader(&objMessage));
        case SipStatusCode::SC_487:
            return FailReason(FAIL_REASON_SESSION_SERVER_REQUEST_TERMINATED,
                    MessageUtil::GetCauseFromReasonHeader(&objMessage));
        case SipStatusCode::SC_488:
            return FailReason(FAIL_REASON_SESSION_NOTACCEPTABLEHERE,
                    MessageUtil::GetCauseFromReasonHeader(&objMessage));
        case SipStatusCode::SC_499:
            return FailReason(FAIL_REASON_SESSION_NOTREACHABLE,
                    MessageUtil::GetCauseFromReasonHeader(&objMessage));
    }

    return FailReason(FAIL_REASON_SESSION_SERVERERROR, nStatusCode);
}

PRIVATE
FailReason StartErrorHandler::Handle403Response() const
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

    return FailReason(FAIL_REASON_SESSION_FORBIDDEN, SipStatusCode::SC_403);
}

PRIVATE
FailReason StartErrorHandler::Handle404Response() const
{
    if (m_objContext.GetCallInfo().bUssi)
    {
        return FailReason(FAIL_REASON_SESSION_RETRY1X);
    }
    else
    {
        return FailReason(FAIL_REASON_SESSION_NOTFOUND);
    }
}

PRIVATE
FailReason StartErrorHandler::Handle407Response() const
{
    if (m_objContext.GetCallInfo().bEmergency)
    {
        return FailReason(FAIL_REASON_SESSION_RETRY1X);
    }
    else
    {
        return FailReason(FAIL_REASON_SESSION_SERVERERROR, SipStatusCode::SC_407);
    }
}

PRIVATE
FailReason StartErrorHandler::Handle5xxResponse(IN const IMessage& objMessage) const
{
    IMS_SINT32 nStatusCode = objMessage.GetStatusCode();

    switch (nStatusCode)
    {
        case SipStatusCode::SC_500:
            return Handle500Response(objMessage);
        case SipStatusCode::SC_501:
        case SipStatusCode::SC_502:
            return FailReason(FAIL_REASON_SESSION_SERVERERROR, nStatusCode);
        case SipStatusCode::SC_503:
            return Handle503Response(objMessage);
        case SipStatusCode::SC_504:
            return Handle504Response(objMessage);
        case SipStatusCode::SC_505:
        case SipStatusCode::SC_513:
        case SipStatusCode::SC_580:
            return FailReason(FAIL_REASON_SESSION_SERVERERROR, nStatusCode);
    }

    return Handle500Response(objMessage);
}

PRIVATE
FailReason StartErrorHandler::Handle500Response(IN const IMessage& objMessage) const
{
    if (!MessageUtil::IsHeaderPresent(&objMessage, ISipHeader::RETRY_AFTER_SEC))
    {
        if (IsIpcanResourceUnavailable(objMessage))
        {
            // TS 24.229 5.1.3.1: There's the method to examine headers but no further behavior.
            return FailReason(FAIL_REASON_SESSION_SERVER_INTERNAL_ERROR,
                    MessageUtil::GetCauseFromReasonHeader(&objMessage));
        }
    }

    return FailReason(FAIL_REASON_SESSION_SERVER_INTERNAL_ERROR,
            MessageUtil::GetCauseFromReasonHeader(&objMessage));
}

PRIVATE
FailReason StartErrorHandler::Handle503Response(IN const IMessage& objMessage) const
{
    IMS_SINT32 nRetryAfter =
            MessageUtil::GetHeaderValueInt(&objMessage, ISipHeader::RETRY_AFTER_ANY);
    if (nRetryAfter > 0)
    {
        // TODO: Set block and CSFB for nRetryAfter duration
        return FailReason(FAIL_REASON_SESSION_RETRY1X);
    }

    return FailReason(FAIL_REASON_SESSION_SERVER_INTERNAL_ERROR,
            MessageUtil::GetCauseFromReasonHeader(&objMessage));
}

PRIVATE
FailReason StartErrorHandler::Handle504Response(IN const IMessage& objMessage) const
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

    return FailReason(FAIL_REASON_SESSION_SERVERERROR, SipStatusCode::SC_504);
}

PRIVATE
FailReason StartErrorHandler::Handle6xxResponse(IN const IMessage& objMessage) const
{
    IMS_SINT32 nStatusCode = objMessage.GetStatusCode();

    switch (nStatusCode)
    {
        case SipStatusCode::SC_603:
            return FailReason(FAIL_REASON_SESSION_SERVER_DECLINE,
                    MessageUtil::GetCauseFromReasonHeader(&objMessage));
    }

    return FailReason(FAIL_REASON_SESSION_SERVERERROR, nStatusCode);
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
    MtcAosConnector* pAosConnector = GetAosConnector();
    if (pAosConnector)
    {
        pAosConnector->Control(nCommand);
    }
}

PRIVATE
AString StartErrorHandler::GetPathHeader() const
{
    MtcAosConnector* pAosConnector = GetAosConnector();
    return pAosConnector ? pAosConnector->GetPathHeaderValue() : AString::ConstNull();
}

PRIVATE
AString StartErrorHandler::GetLastPathHeader() const
{
    MtcAosConnector* pAosConnector = GetAosConnector();
    return pAosConnector ? pAosConnector->GetLastPathHeaderValue() : AString::ConstNull();
}

PRIVATE
AString StartErrorHandler::GetServiceRouteHeader() const
{
    MtcAosConnector* pAosConnector = GetAosConnector();
    return pAosConnector ? pAosConnector->GetServiceRouteHeaderValue() : AString::ConstNull();
}

PRIVATE
AString StartErrorHandler::GetSupported() const
{
    MtcAosConnector* pAosConnector = GetAosConnector();
    return pAosConnector ? pAosConnector->GetSupportedHeaderValue() : AString::ConstNull();
}

PRIVATE
MtcAosConnector* StartErrorHandler::GetAosConnector() const
{
    MtcAosConnector* pAosConnector = m_objContext.GetService().GetAosConnector();
    if (pAosConnector == IMS_NULL)
    {
        IMS_TRACE_E(0, "GetAosConnector : AosConnector is null", 0, 0, 0);
    }
    return pAosConnector;
}
