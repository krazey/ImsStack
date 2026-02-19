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
#include "configuration/MtcConfigurationProxy.h"
#include "utility/IMessageUtils.h"

__IMS_TRACE_TAG_COM_MTC__;

const LOCAL AString REASON_TEXT_CALL_BUSY_TYPE1 = "another device sent all devices busy response";
const LOCAL AString REASON_TEXT_CALL_COMPLETED_TYPE1 = "call completion elsewhere";

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
    return GetCallReasonInfo(m_objContext, objMessage);
}

PRIVATE
CallReasonInfo CancelHandler::GetCallReasonInfo(
        IMtcCallContext& objContext, const IMessage& objMessage)
{
    ReasonHeaderValue objReasonResult = objContext.GetMessageUtils().GetPrioritizedReasonHeader(
            &objMessage, {REASON_SIP_PROTOCOL, REASON_Q850_PROTOCOL, AString::ConstNull()});

    const IMS_SINT32 nCode = GetCodeFromReason(objReasonResult);
    CallReasonInfo objReasonInfo(nCode);

    EnrichCallReasonInfo(objContext, objReasonResult, objReasonInfo);

    IMS_TRACE_D("GetCallReasonInfo: code=[%d], cause=[%d], extraMessage=[%s]", objReasonInfo.nCode,
            objReasonInfo.nExtraCode, objReasonInfo.strExtraMessage.GetStr());

    return objReasonInfo;
}

PRIVATE
IMS_SINT32 CancelHandler::GetCodeFromReason(const ReasonHeaderValue& objReasonResult)
{
    if (objReasonResult.strProtocol.EqualsIgnoreCase(REASON_SIP_PROTOCOL))
    {
        const AString strNormalizedText = objReasonResult.strText.SimplifyWsp().MakeLower();
        if (strNormalizedText.Contains(REASON_TEXT_CALL_BUSY_TYPE1))
        {
            return CODE_REJECTED_ELSEWHERE;
        }
        else if (strNormalizedText.Contains(REASON_TEXT_CALL_COMPLETED_TYPE1))
        {
            return CODE_ANSWERED_ELSEWHERE;
        }

        // Fallback to cause code for SIP protocol.
        switch (objReasonResult.nCause)
        {
            case SipStatusCode::SC_200:
                return CODE_ANSWERED_ELSEWHERE;
            case SipStatusCode::SC_600:
            case SipStatusCode::SC_603:
                return CODE_REJECTED_ELSEWHERE;
        }
    }
    return CODE_USER_TERMINATED_BY_REMOTE;
}

PRIVATE
void CancelHandler::EnrichCallReasonInfo(IMtcCallContext& objContext,
        const ReasonHeaderValue& objReasonResult, CallReasonInfo& objReasonInfo)
{
    if (objContext.GetConfigurationProxy().GetBoolean(
                ConfigVoice::KEY_ENRICH_CALLREASONINFO_WITH_REASON_HEADER_BOOL))
    {
        objReasonInfo.strExtraMessage =
                CallReasonInfo::FormatExtraMessageFromReason(objReasonResult.strProtocol,
                        objReasonResult.nCause, objReasonResult.strText, IMS_FALSE);
        objReasonInfo.nExtraCode = objReasonResult.nCause;
    }
}
