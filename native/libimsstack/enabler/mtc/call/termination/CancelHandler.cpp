#include "SipStatusCode.h"
#include "call/termination/CancelHandler.h"
#include "utility/MessageUtil.h"

__IMS_TRACE_TAG_COM_MTC__;

PRIVATE
const AString CancelHandler::REASON_TEXT_CALL_BUSY = "busy everywhere";
const AString CancelHandler::REASON_TEXT_CALL_COMPLETED = "call completed elsewhere";
const AString CancelHandler::REASON_TEXT_CALL_DECLINED = "declined";

PUBLIC
CancelHandler::CancelHandler()
{
}

PUBLIC
CancelHandler::~CancelHandler()
{
}

PUBLIC
FailReason CancelHandler::Handle(IN const IMessage& objMessage) const
{
    IMS_SINT32 nReasonCause = 0;
    AString strReasonText;
    if (!MessageUtil::GetCauseAndTextFromReasonHeader(&objMessage, nReasonCause, strReasonText))
    {
        IMS_TRACE_D("Handle : No Reason header", 0, 0, 0);
        return FailReason(FAIL_REASON_SESSION_TERMINATED);
    }

    return GetFailReasonFromReasonHeader(nReasonCause, strReasonText);
}

PRIVATE
FailReason CancelHandler::GetFailReasonFromReasonHeader(
        IN IMS_SINT32 nCause, IN const AString& strText) const
{
    AString strNormalizedText = strText.SimplifyWSP().MakeLower();

    if (nCause == SIPStatusCode::SC_200 &&
            strNormalizedText.Contains(REASON_TEXT_CALL_COMPLETED))
    {
        return FailReason(FAIL_REASON_SESSION_MULTIDEVICE_ACCEPTED);
    }
    else if (nCause == SIPStatusCode::SC_600 &&
            strNormalizedText.Contains(REASON_TEXT_CALL_BUSY))
    {
        return FailReason(FAIL_REASON_SESSION_MULTIDEVICE_REJECTED);
    }
    else if (nCause == SIPStatusCode::SC_603 &&
            strNormalizedText.Contains(REASON_TEXT_CALL_DECLINED))
    {
        return FailReason(FAIL_REASON_SESSION_MULTIDEVICE_REJECTED);
    }

    return FailReason(FAIL_REASON_SESSION_TERMINATED);
}
