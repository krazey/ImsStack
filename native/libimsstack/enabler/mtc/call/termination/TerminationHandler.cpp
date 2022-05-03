#include "ISession.h"
#include "call/termination/TerminationHandler.h"

PUBLIC
TerminationHandler::TerminationHandler() {}

PUBLIC
TerminationHandler::~TerminationHandler() {}

PUBLIC
FailReason TerminationHandler::Handle(IN const ISession& objSession) const
{
    return GetFailReasonFromSessionTerminationReason(objSession.GetTerminationReason());
}

PRIVATE
FailReason TerminationHandler::GetFailReasonFromSessionTerminationReason(
        IN IMS_SINT32 nTerminationReason) const
{
    switch (nTerminationReason)
    {
        case ISession::TERMINATION_REASON_INVALID:
            return FailReason(FAIL_REASON_SESSION_TERMINATED, nTerminationReason);

        case ISession::TERMINATION_REASON_UNKNOWN:
            return FailReason(FAIL_REASON_SESSION_TERMINATED, nTerminationReason);

        case ISession::TERMINATION_REASON_USER_ACTION:
            return FailReason(FAIL_REASON_SESSION_USERTERMINATE, nTerminationReason);

        case ISession::TERMINATION_REASON_REMOTE_ACTION:
            return FailReason(FAIL_REASON_SESSION_TERMINATED, nTerminationReason);

        case ISession::TERMINATION_REASON_REFRESH_408:
            return FailReason(FAIL_REASON_SESSION_REFRESH_OUT, nTerminationReason);

        case ISession::TERMINATION_REASON_REFRESH_481:
            return FailReason(FAIL_REASON_SESSION_REFRESH_OUT, nTerminationReason);

        case ISession::TERMINATION_REASON_REFRESH_TXN_TIMEOUT:
            return FailReason(FAIL_REASON_SESSION_REFRESH_OUT, nTerminationReason);

        case ISession::TERMINATION_REASON_REFRESH_TIMEOUT:
            return FailReason(FAIL_REASON_SESSION_REFRESH_OUT, nTerminationReason);

        case ISession::TERMINATION_REASON_SERVICE_CLOSED:
            return FailReason(FAIL_REASON_SERVICE_OUT, nTerminationReason);

        case ISession::TERMINATION_REASON_MAX:
            return FailReason(FAIL_REASON_SESSION_TERMINATED, nTerminationReason);
    }

    return FailReason(FAIL_REASON_SESSION_TERMINATED, nTerminationReason);
}
