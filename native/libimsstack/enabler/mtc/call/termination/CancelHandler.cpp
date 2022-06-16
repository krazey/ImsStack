#include "SipStatusCode.h"
#include "call/termination/CancelHandler.h"
#include "utility/MessageUtil.h"

__IMS_TRACE_TAG_COM_MTC__;

PRIVATE
const AString CancelHandler::REASON_TEXT_CALL_BUSY = "busy everywhere";
const AString CancelHandler::REASON_TEXT_CALL_COMPLETED = "call completed elsewhere";
const AString CancelHandler::REASON_TEXT_CALL_DECLINED = "declined";

PUBLIC
CancelHandler::CancelHandler() {}

PUBLIC
CancelHandler::~CancelHandler() {}

PUBLIC
CallReasonInfo CancelHandler::Handle(IN const IMessage& objMessage) const
{
    IMS_SINT32 nReasonCause = 0;
    AString strReasonText;
    if (!MessageUtil::GetCauseAndTextFromReasonHeader(&objMessage, nReasonCause, strReasonText))
    {
        IMS_TRACE_D("Handle : No Reason header", 0, 0, 0);
        return CallReasonInfo(CODE_USER_TERMINATED_BY_REMOTE);
    }

    return GetCallReasonInfoFromReasonHeader(nReasonCause, strReasonText);
}

PRIVATE
CallReasonInfo CancelHandler::GetCallReasonInfoFromReasonHeader(
        IN IMS_SINT32 nCause, IN const AString& strText) const
{
    AString strNormalizedText = strText.SimplifyWSP().MakeLower();

    if (nCause == SipStatusCode::SC_200 && strNormalizedText.Contains(REASON_TEXT_CALL_COMPLETED))
    {
        return CallReasonInfo(CODE_ANSWERED_ELSEWHERE);
    }
    else if (nCause == SipStatusCode::SC_600 && strNormalizedText.Contains(REASON_TEXT_CALL_BUSY))
    {
        return CallReasonInfo(CODE_REJECTED_ELSEWHERE);
    }
    else if (nCause == SipStatusCode::SC_603 &&
            strNormalizedText.Contains(REASON_TEXT_CALL_DECLINED))
    {
        return CallReasonInfo(CODE_REJECTED_ELSEWHERE);
    }

    return CallReasonInfo(CODE_USER_TERMINATED_BY_REMOTE);
}
