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

#include "ServiceTrace.h"
#include "SipStatusCode.h"
#include "call/IMtcCallContext.h"
#include "call/termination/CancelHandler.h"
#include "utility/IMessageUtils.h"

__IMS_TRACE_TAG_COM_MTC__;

LOCAL const AString SIP_PROTOCOL = "SIP";
LOCAL const AString REASON_TEXT_CALL_BUSY = "busy everywhere";
LOCAL const AString REASON_TEXT_CALL_COMPLETED = "call completed elsewhere";
LOCAL const AString REASON_TEXT_CALL_DECLINED = "declined";

LOCAL const AString REASON_TEXT_CALL_BUSY_VZW = "another device sent all devices busy response";
LOCAL const AString REASON_TEXT_CALL_COMPLETED_VZW = "call completion elsewhere";

PUBLIC
CancelHandler::CancelHandler(IN IMtcCallContext& objContext) :
        m_objContext(objContext)
{
}

PUBLIC
CancelHandler::~CancelHandler() {}

PUBLIC
CallReasonInfo CancelHandler::Handle(IN const IMessage& objMessage) const
{
    ReasonHeaderValue objValue = m_objContext.GetMessageUtils().GetCauseAndTextFromReasonHeader(
            &objMessage, SIP_PROTOCOL);
    IMS_TRACE_D("Handle : [%d] [%s]", objValue.nCause, objValue.strText.GetStr(), 0);

    return GetCallReasonInfoFromReasonHeader(objValue.nCause, objValue.strText);
}

PRIVATE
CallReasonInfo CancelHandler::GetCallReasonInfoFromReasonHeader(
        IN IMS_SINT32 nCause, IN const AString& strText)
{
    const AString strNormalizedText = strText.SimplifyWsp().MakeLower();

    if (nCause == SipStatusCode::SC_200 && strNormalizedText.Contains(REASON_TEXT_CALL_COMPLETED))
    {
        return CallReasonInfo(CODE_ANSWERED_ELSEWHERE);
    }
    else if ((nCause == SipStatusCode::SC_600 &&
                     strNormalizedText.Contains(REASON_TEXT_CALL_BUSY)) ||
            (nCause == SipStatusCode::SC_603 &&
                    strNormalizedText.Contains(REASON_TEXT_CALL_DECLINED)))
    {
        return CallReasonInfo(CODE_REJECTED_ELSEWHERE);
    }

    if (strNormalizedText.Contains(REASON_TEXT_CALL_BUSY_VZW))
    {
        return CallReasonInfo(CODE_REJECTED_ELSEWHERE);
    }
    else if (strNormalizedText.Contains(REASON_TEXT_CALL_COMPLETED_VZW))
    {
        return CallReasonInfo(CODE_ANSWERED_ELSEWHERE);
    }

    return CallReasonInfo(CODE_USER_TERMINATED_BY_REMOTE);
}
