#include "call/IMtcCallContext.h"
#include "CallInfo.h"
#include "IMessage.h"
#include "ServiceSystemTime.h"
#include "ServiceTrace.h"
#include "SipStatusCode.h"
#include "call/termination/UpdateErrorHandler.h"

__IMS_TRACE_TAG_COM_MTC__;

PUBLIC
UpdateErrorHandler::UpdateErrorHandler(IN IMtcCallContext& objContext) :
        m_objContext(objContext)
{
}

PUBLIC
UpdateErrorHandler::~UpdateErrorHandler() {}

PUBLIC
FailReason UpdateErrorHandler::Handle(IN const IMessage* piMessage) const
{
    if (piMessage == IMS_NULL)
    {
        IMS_TRACE_I("Handle : piMessage is null", 0, 0, 0);
        return FailReason(FAIL_REASON_SESSION_SERVERERROR);
    }

    return GetFailReasonForResponse(*piMessage);
}

PRIVATE
FailReason UpdateErrorHandler::GetFailReasonForResponse(IN const IMessage& objMessage) const
{
    IMS_SINT32 nStatusCode = objMessage.GetStatusCode();
    IMS_ASSERT(nStatusCode >= SipStatusCode::SC_300);

    if (SipStatusCode::SC_300 <= nStatusCode && nStatusCode < SipStatusCode::SC_400)
    {
        return GetFailReasonFor3xxResponse(objMessage);
    }
    else if (SipStatusCode::SC_400 <= nStatusCode && nStatusCode < SipStatusCode::SC_500)
    {
        return GetFailReasonFor4xxResponse(objMessage);
    }
    else if (SipStatusCode::SC_500 <= nStatusCode && nStatusCode < SipStatusCode::SC_600)
    {
        return GetFailReasonFor5xxResponse(objMessage);
    }
    else if (SipStatusCode::SC_600 <= nStatusCode && nStatusCode < SipStatusCode::SC_MAX)
    {
        return GetFailReasonFor6xxResponse(objMessage);
    }
    return FailReason(FAIL_REASON_SESSION_SERVERERROR);
}

PRIVATE
FailReason UpdateErrorHandler::GetFailReasonFor3xxResponse(IN const IMessage& objMessage) const
{
    IMS_SINT32 nStatusCode = objMessage.GetStatusCode();

    return FailReason(FAIL_REASON_SESSION_SERVERERROR, nStatusCode);
}

PRIVATE
FailReason UpdateErrorHandler::GetFailReasonFor4xxResponse(IN const IMessage& objMessage) const
{
    IMS_SINT32 nStatusCode = objMessage.GetStatusCode();

    switch (nStatusCode)
    {
        case SipStatusCode::SC_404:
        case SipStatusCode::SC_405:
        case SipStatusCode::SC_410:
        case SipStatusCode::SC_416:
        case SipStatusCode::SC_480:
        case SipStatusCode::SC_481:
        case SipStatusCode::SC_482:
        case SipStatusCode::SC_483:
        case SipStatusCode::SC_484:
        case SipStatusCode::SC_485:
        case SipStatusCode::SC_489:
            return FailReason(FAIL_REASON_SESSION_DESTROYED, nStatusCode);

        case SipStatusCode::SC_491:
            return FailReason(FAIL_REASON_SESSION_RETRY,
                    GetGlareTimeMillisecond(m_objContext.GetCallInfo().ePeerType));
    }

    return FailReason(FAIL_REASON_SESSION_SERVERERROR, nStatusCode);
}

PRIVATE
FailReason UpdateErrorHandler::GetFailReasonFor5xxResponse(IN const IMessage& objMessage) const
{
    IMS_SINT32 nStatusCode = objMessage.GetStatusCode();

    switch (nStatusCode)
    {
        case SipStatusCode::SC_501:
        case SipStatusCode::SC_502:
            return FailReason(FAIL_REASON_SESSION_DESTROYED, nStatusCode);
    }

    return FailReason(FAIL_REASON_SESSION_SERVERERROR, nStatusCode);
}

PRIVATE
FailReason UpdateErrorHandler::GetFailReasonFor6xxResponse(IN const IMessage& objMessage) const
{
    IMS_SINT32 nStatusCode = objMessage.GetStatusCode();

    switch (nStatusCode)
    {
        case SipStatusCode::SC_604:
            return FailReason(FAIL_REASON_SESSION_DESTROYED, nStatusCode);
    }

    return FailReason(FAIL_REASON_SESSION_SERVERERROR, nStatusCode);
}

PRIVATE
IMS_UINT32 UpdateErrorHandler::GetGlareTimeMillisecond(IN PeerType ePeerType) const
{
    IMS_UINT32 nUpperT = 0;
    IMS_UINT32 nBaseT = 0;

    // RFC 3261 14.1: Glare condition for 491 response
    if (ePeerType == PeerType::MO)
    {
        nUpperT = 4000;
        nBaseT = 2100;
    }
    else
    {
        nUpperT = 2000;
        nBaseT = 0;
    }

    return nBaseT + IMS_SYS_GetRandom(nUpperT - nBaseT);
}
