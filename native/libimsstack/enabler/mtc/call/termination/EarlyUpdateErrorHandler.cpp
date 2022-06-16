#include "IMessage.h"
#include "ServiceTrace.h"
#include "SipStatusCode.h"
#include "call/termination/EarlyUpdateErrorHandler.h"

__IMS_TRACE_TAG_COM_MTC__;

PUBLIC
EarlyUpdateErrorHandler::EarlyUpdateErrorHandler() {}

PUBLIC
EarlyUpdateErrorHandler::~EarlyUpdateErrorHandler() {}

PUBLIC
CallReasonInfo EarlyUpdateErrorHandler::Handle(IN const IMessage* piMessage) const
{
    if (IsTransactionTimeout(piMessage))
    {
        IMS_TRACE_I("Handle : Timeout", 0, 0, 0);
        return CallReasonInfo(CODE_NETWORK_RESP_TIMEOUT);
    }

    IMS_SINT32 nStatusCode = piMessage->GetStatusCode();
    IMS_ASSERT(nStatusCode >= SipStatusCode::SC_300);

    return CallReasonInfo(CODE_SESSION_INTERNAL_ERROR, nStatusCode);
}

PRIVATE
IMS_BOOL EarlyUpdateErrorHandler::IsTransactionTimeout(IN const IMessage* piMessage) const
{
    if (piMessage == IMS_NULL)
    {
        return IMS_TRUE;
    }

    return piMessage->GetStatusCode() == SipStatusCode::SC_INVALID;
}
