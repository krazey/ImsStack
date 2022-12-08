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

#include "ISession.h"
#include "call/IMtcCallContext.h"
#include "call/termination/TerminationHandler.h"
#include "core/IMessage.h"
#include "utility/IMessageUtils.h"

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
    CallReasonInfo objReasonInfo =
            GetCallReasonInfoFromSessionTerminationReason(objSession.GetTerminationReason());

    if (objReasonInfo.nCode == CODE_USER_TERMINATED_BY_REMOTE)
    {
        IMessage* piMessage = objSession.GetPreviousRequest(IMessage::SESSION_TERMINATE);
        if (piMessage != IMS_NULL)
        {
            ReasonHeaderValue objValue =
                    m_objContext.GetMessageUtils().GetCauseAndTextFromReasonHeader(piMessage);
            objReasonInfo.strExtraMessage = objValue.strText;
        }
    }
    return objReasonInfo;
}

PRIVATE
CallReasonInfo TerminationHandler::GetCallReasonInfoFromSessionTerminationReason(
        IN IMS_SINT32 nTerminationReason)
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
            return CallReasonInfo(CODE_NETWORK_RESP_TIMEOUT, EXTRA_CODE_METHOD_UPDATE);

        case ISession::TERMINATION_REASON_REFRESH_TIMEOUT:
            return CallReasonInfo(CODE_NETWORK_RESP_TIMEOUT, EXTRA_CODE_METHOD_UPDATE);

        case ISession::TERMINATION_REASON_SERVICE_CLOSED:
            return CallReasonInfo(CODE_LOCAL_NOT_REGISTERED, nTerminationReason);
    }

    return CallReasonInfo(CODE_USER_TERMINATED_BY_REMOTE, nTerminationReason);
}
