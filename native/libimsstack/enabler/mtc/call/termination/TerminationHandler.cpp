#include "ISession.h"
#include "call/termination/TerminationHandler.h"

PUBLIC
TerminationHandler::TerminationHandler() {}

PUBLIC
TerminationHandler::~TerminationHandler() {}

PUBLIC
CallReasonInfo TerminationHandler::Handle(IN const ISession& objSession) const
{
    return GetCallReasonInfoFromSessionTerminationReason(objSession.GetTerminationReason());
}

PRIVATE
CallReasonInfo TerminationHandler::GetCallReasonInfoFromSessionTerminationReason(
        IN IMS_SINT32 nTerminationReason) const
{
    switch (nTerminationReason)
    {
        case ISession::TERMINATION_REASON_INVALID:
            return CallReasonInfo(CODE_USER_TERMINATED_BY_REMOTE, nTerminationReason);

        case ISession::TERMINATION_REASON_UNKNOWN:
            return CallReasonInfo(CODE_USER_TERMINATED_BY_REMOTE, nTerminationReason);

        case ISession::TERMINATION_REASON_USER_ACTION:
            return CallReasonInfo(CODE_USER_TERMINATED, nTerminationReason);

        case ISession::TERMINATION_REASON_REMOTE_ACTION:
            return CallReasonInfo(CODE_USER_TERMINATED_BY_REMOTE, nTerminationReason);

        case ISession::TERMINATION_REASON_REFRESH_408:
            return CallReasonInfo(CODE_SIP_REQUEST_TIMEOUT, nTerminationReason);

        case ISession::TERMINATION_REASON_REFRESH_481:
            return CallReasonInfo(CODE_SIP_REQUEST_TIMEOUT, nTerminationReason);

        case ISession::TERMINATION_REASON_REFRESH_TXN_TIMEOUT:
            return CallReasonInfo(CODE_NETWORK_RESP_TIMEOUT, nTerminationReason);

        case ISession::TERMINATION_REASON_REFRESH_TIMEOUT:
            return CallReasonInfo(CODE_NETWORK_RESP_TIMEOUT, nTerminationReason);

        case ISession::TERMINATION_REASON_SERVICE_CLOSED:
            return CallReasonInfo(CODE_LOCAL_SERVICE_UNAVAILABLE, nTerminationReason);

        case ISession::TERMINATION_REASON_MAX:
            return CallReasonInfo(CODE_USER_TERMINATED_BY_REMOTE, nTerminationReason);
    }

    return CallReasonInfo(CODE_USER_TERMINATED_BY_REMOTE, nTerminationReason);
}
