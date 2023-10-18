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

#include "IMessage.h"
#include "ServiceTrace.h"
#include "SipStatusCode.h"
#include "call/IMtcCallContext.h"
#include "call/termination/EarlyUpdateErrorHandler.h"
#include "call/termination/UpdateErrorHandler.h"

__IMS_TRACE_TAG_COM_MTC__;

PUBLIC
EarlyUpdateErrorHandler::EarlyUpdateErrorHandler(IN IMtcCallContext& objContext) :
        m_objContext(objContext)
{
}

PUBLIC
EarlyUpdateErrorHandler::~EarlyUpdateErrorHandler() {}

PUBLIC
CallReasonInfo EarlyUpdateErrorHandler::Handle(IN const IMessage* piMessage)
{
    if (IsTransactionTimeout(piMessage))
    {
        IMS_TRACE_I("Handle : Timeout", 0, 0, 0);
        return CallReasonInfo(CODE_NETWORK_RESP_TIMEOUT, EXTRA_CODE_METHOD_UPDATE);
    }

    IMS_SINT32 nStatusCode = piMessage->GetStatusCode();
    IMS_ASSERT(nStatusCode >= SipStatusCode::SC_300);

    if (nStatusCode == SipStatusCode::SC_491)
    {
        return CallReasonInfo(CODE_SIP_REQUEST_PENDING,
                UpdateErrorHandler::GetGlareTimeMillisecond(m_objContext.GetCallInfo().ePeerType));
    }

    return CallReasonInfo(CODE_REJECT_INTERNAL_ERROR, nStatusCode);
}

PRIVATE
IMS_BOOL EarlyUpdateErrorHandler::IsTransactionTimeout(IN const IMessage* piMessage)
{
    if (piMessage == IMS_NULL)
    {
        return IMS_TRUE;
    }

    return piMessage->GetStatusCode() == SipStatusCode::SC_INVALID;
}
