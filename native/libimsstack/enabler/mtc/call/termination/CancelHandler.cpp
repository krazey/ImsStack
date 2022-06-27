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
    AString strNormalizedText = strText.SimplifyWsp().MakeLower();

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
