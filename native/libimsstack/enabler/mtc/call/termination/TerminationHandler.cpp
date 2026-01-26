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

#include "AString.h"
#include "IMessage.h"
#include "ISession.h"
#include "ServiceTrace.h"
#include "call/IMtcCallContext.h"
#include "call/termination/TerminationHandler.h"
#include "configuration/MtcConfigurationProxy.h"
#include "utility/IMessageUtils.h"

__IMS_TRACE_TAG_COM_MTC__;

LOCAL const AString REASON_TEXT_CALL_PULLED_TYPE1 = "call has been pulled by another device";

PUBLIC
TerminationHandler::TerminationHandler(IN IMtcCallContext& objContext) :
        m_objContext(objContext)
{
}

PUBLIC
TerminationHandler::~TerminationHandler() {}

PUBLIC
CallReasonInfo TerminationHandler::Handle(IN const ISession& objSession) const
{
    // Step 1: Get the base reason info from the ISession termination reason.
    CallReasonInfo objReasonInfo =
            GetCallReasonInfoFromSessionTerminationReason(objSession.GetTerminationReason());

    const IMessage* piMessage = objSession.GetPreviousRequest(IMessage::SESSION_TERMINATE);
    if (!piMessage)
    {
        return objReasonInfo;
    }
    // Step 2: Use the shared utility to get the prioritized reason header.
    ReasonHeaderValue objReasonResult = m_objContext.GetMessageUtils().GetPrioritizedReasonHeader(
            piMessage, {REASON_SIP_PROTOCOL, REASON_Q850_PROTOCOL, AString::ConstNull()});

    // Step 3: Handle call pull scenario.
    if (IsByCallPull(objReasonResult) && objReasonInfo.nCode == CODE_USER_TERMINATED_BY_REMOTE)
    {
        objReasonInfo.nCode = CODE_CALL_END_CAUSE_CALL_PULL;
        objReasonInfo.nExtraCode = -1;
        // add extramessage as the configuration next
    }

    // Step 4: Add extra message if the configuration is enabled.
    EnrichReasonInfoWithMessage(objReasonResult, objReasonInfo);

    IMS_TRACE_D("Handle : code=[%d], cause=[%d], extraMessage=[%s]", objReasonInfo.nCode,
            objReasonInfo.nExtraCode, objReasonInfo.strExtraMessage.GetStr());
    return objReasonInfo;
}

PRIVATE void TerminationHandler::EnrichReasonInfoWithMessage(
        IN const ReasonHeaderValue& objReasonResult, IN_OUT CallReasonInfo& objReasonInfo) const
{
    // This helper remains unchanged.
    if (m_objContext.GetConfigurationProxy().GetBoolean(
                ConfigVoice::KEY_ENRICH_CALLREASONINFO_WITH_REASON_HEADER_BOOL) ||
            objReasonInfo.nCode == CODE_USER_TERMINATED_BY_REMOTE)
    {
        objReasonInfo.strExtraMessage =
                CallReasonInfo::FormatExtraMessageFromReason(objReasonResult.strProtocol,
                        objReasonResult.nCause, objReasonResult.strText, IMS_TRUE);
    }
}

PRIVATE
CallReasonInfo TerminationHandler::GetCallReasonInfoFromSessionTerminationReason(
        IN IMS_SINT32 nTerminationReason)
{
    switch (nTerminationReason)
    {
        case ISession::TERMINATION_REASON_INVALID:
        case ISession::TERMINATION_REASON_UNKNOWN:
        case ISession::TERMINATION_REASON_REMOTE_ACTION:
            return CallReasonInfo(CODE_USER_TERMINATED_BY_REMOTE, nTerminationReason);

        case ISession::TERMINATION_REASON_USER_ACTION:
            return CallReasonInfo(CODE_USER_TERMINATED, nTerminationReason);

        case ISession::TERMINATION_REASON_REFRESH_408:
        case ISession::TERMINATION_REASON_REFRESH_481:
            return CallReasonInfo(CODE_SIP_REQUEST_TIMEOUT, nTerminationReason);

        case ISession::TERMINATION_REASON_REFRESH_TXN_TIMEOUT:
        case ISession::TERMINATION_REASON_REFRESH_TIMEOUT:
            return CallReasonInfo(CODE_NETWORK_RESP_TIMEOUT, EXTRA_CODE_METHOD_UPDATE);

        case ISession::TERMINATION_REASON_SERVICE_CLOSED:
            return CallReasonInfo(CODE_LOCAL_NOT_REGISTERED, nTerminationReason);
    }

    return CallReasonInfo(CODE_USER_TERMINATED_BY_REMOTE, nTerminationReason);
}

PRIVATE IMS_BOOL TerminationHandler::IsByCallPull(IN const ReasonHeaderValue& objReasonValue) const
{
    if (!objReasonValue.strProtocol.EqualsIgnoreCase(REASON_SIP_PROTOCOL))
    {
        return IMS_FALSE;
    }
    const AString strNormalizedText = objReasonValue.strText.SimplifyWsp().MakeLower();
    return strNormalizedText.Contains(REASON_TEXT_CALL_PULLED_TYPE1);
}
