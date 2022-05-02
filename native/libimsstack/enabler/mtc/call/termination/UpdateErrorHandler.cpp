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
UpdateErrorHandler::~UpdateErrorHandler()
{
}

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
    IMS_ASSERT(nStatusCode >= SIPStatusCode::SC_300);

    if (SIPStatusCode::SC_300 <= nStatusCode && nStatusCode < SIPStatusCode::SC_400)
    {
        return GetFailReasonFor3xxResponse(objMessage);
    }
    else if (SIPStatusCode::SC_400 <= nStatusCode && nStatusCode < SIPStatusCode::SC_500)
    {
        return GetFailReasonFor4xxResponse(objMessage);
    }
    else if (SIPStatusCode::SC_500 <= nStatusCode && nStatusCode < SIPStatusCode::SC_600)
    {
        return GetFailReasonFor5xxResponse(objMessage);
    }
    else if (SIPStatusCode::SC_600 <= nStatusCode && nStatusCode < SIPStatusCode::SC_MAX)
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
        case SIPStatusCode::SC_404:
        case SIPStatusCode::SC_405:
        case SIPStatusCode::SC_410:
        case SIPStatusCode::SC_416:
        case SIPStatusCode::SC_480:
        case SIPStatusCode::SC_481:
        case SIPStatusCode::SC_482:
        case SIPStatusCode::SC_483:
        case SIPStatusCode::SC_484:
        case SIPStatusCode::SC_485:
        case SIPStatusCode::SC_489:
            return FailReason(FAIL_REASON_SESSION_DESTROYED, nStatusCode);

        case SIPStatusCode::SC_491:
            return FailReason(
                    FAIL_REASON_SESSION_RETRY,
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
        case SIPStatusCode::SC_501:
        case SIPStatusCode::SC_502:
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
        case SIPStatusCode::SC_604:
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
