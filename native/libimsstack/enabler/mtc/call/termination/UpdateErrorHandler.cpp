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

#include "Engine.h"
#include "IConfiguration.h"
#include "IMessage.h"
#include "ISipConfig.h"
#include "ISipConfigV.h"
#include "ISipHeader.h"
#include "ServiceSystemTime.h"
#include "ServiceTrace.h"
#include "SipStatusCode.h"
#include "call/IMtcCallContext.h"
#include "call/termination/UpdateErrorHandler.h"
#include "helper/IMtcAosConnector.h"
#include "utility/IMessageUtils.h"

__IMS_TRACE_TAG_COM_MTC__;

PUBLIC
UpdateErrorHandler::UpdateErrorHandler(IN IMtcCallContext& objContext) :
        m_objContext(objContext)
{
}

PUBLIC
UpdateErrorHandler::~UpdateErrorHandler() {}

PUBLIC
CallReasonInfo UpdateErrorHandler::Handle(IN const IMessage* piMessage) const
{
    if (piMessage == IMS_NULL)
    {
        IMS_TRACE_I("Handle : piMessage is null", 0, 0, 0);
        return CallReasonInfo(CODE_SIP_SERVER_ERROR);
    }

    if (piMessage->GetStatusCode() == SipStatusCode::SC_503)
    {
        Handle503Response(*piMessage);
    }

    return GetCallReasonInfoForResponse(*piMessage);
}

PUBLIC
IMS_UINT32 UpdateErrorHandler::GetGlareTimeMillisecond(IN PeerType ePeerType)
{
    IMS_UINT32 nUpperT = 0;
    IMS_UINT32 nBaseT = 0;

    // RFC 3261 14.1: Glare condition for 491 response
    if (ePeerType == PeerType::MO)
    {
        nUpperT = 4000;
        nBaseT = 2100;
    }
    else
    {
        nUpperT = 2000;
        nBaseT = 0;
    }

    return nBaseT + IMS_SYS_GetRandom(nUpperT - nBaseT);
}

PRIVATE
CallReasonInfo UpdateErrorHandler::GetCallReasonInfoForResponse(IN const IMessage& objMessage) const
{
    IMS_SINT32 nStatusCode = objMessage.GetStatusCode();
    IMS_ASSERT(nStatusCode >= SipStatusCode::SC_300);

    if (SipStatusCode::SC_300 <= nStatusCode && nStatusCode < SipStatusCode::SC_400)
    {
        return GetCallReasonInfoFor3xxResponse(objMessage);
    }
    else if (SipStatusCode::SC_400 <= nStatusCode && nStatusCode < SipStatusCode::SC_500)
    {
        return GetCallReasonInfoFor4xxResponse(objMessage);
    }
    else if (SipStatusCode::SC_500 <= nStatusCode && nStatusCode < SipStatusCode::SC_600)
    {
        return GetCallReasonInfoFor5xxResponse(objMessage);
    }
    else if (SipStatusCode::SC_600 <= nStatusCode && nStatusCode < SipStatusCode::SC_MAX)
    {
        return GetCallReasonInfoFor6xxResponse(objMessage);
    }
    return CallReasonInfo(CODE_SIP_SERVER_ERROR);
}

PRIVATE
CallReasonInfo UpdateErrorHandler::GetCallReasonInfoFor3xxResponse(IN const IMessage& objMessage)
{
    IMS_SINT32 nStatusCode = objMessage.GetStatusCode();

    return CallReasonInfo(CODE_SIP_SERVER_ERROR, nStatusCode);
}

PRIVATE
CallReasonInfo UpdateErrorHandler::GetCallReasonInfoFor4xxResponse(
        IN const IMessage& objMessage) const
{
    IMS_SINT32 nStatusCode = objMessage.GetStatusCode();

    switch (nStatusCode)
    {
        case SipStatusCode::SC_404:
        case SipStatusCode::SC_405:
        case SipStatusCode::SC_410:
        case SipStatusCode::SC_416:
        case SipStatusCode::SC_480:
        case SipStatusCode::SC_481:
        case SipStatusCode::SC_482:
        case SipStatusCode::SC_483:
        case SipStatusCode::SC_484:
        case SipStatusCode::SC_485:
        case SipStatusCode::SC_489:
            return CallReasonInfo(CODE_USER_TERMINATED_BY_REMOTE, nStatusCode);

        case SipStatusCode::SC_491:
            return CallReasonInfo(CODE_SIP_REQUEST_PENDING,
                    GetGlareTimeMillisecond(m_objContext.GetCallInfo().ePeerType));
    }

    return CallReasonInfo(CODE_SIP_SERVER_ERROR, nStatusCode);
}

PRIVATE
CallReasonInfo UpdateErrorHandler::GetCallReasonInfoFor5xxResponse(IN const IMessage& objMessage)
{
    IMS_SINT32 nStatusCode = objMessage.GetStatusCode();

    switch (nStatusCode)
    {
        case SipStatusCode::SC_501:
        case SipStatusCode::SC_502:
            return CallReasonInfo(CODE_USER_TERMINATED_BY_REMOTE, nStatusCode);
    }

    return CallReasonInfo(CODE_SIP_SERVER_ERROR, nStatusCode);
}

PRIVATE
CallReasonInfo UpdateErrorHandler::GetCallReasonInfoFor6xxResponse(IN const IMessage& objMessage)
{
    IMS_SINT32 nStatusCode = objMessage.GetStatusCode();

    switch (nStatusCode)
    {
        case SipStatusCode::SC_604:
            return CallReasonInfo(CODE_USER_TERMINATED_BY_REMOTE, nStatusCode);
    }

    return CallReasonInfo(CODE_SIP_SERVER_ERROR, nStatusCode);
}

PRIVATE
void UpdateErrorHandler::Handle503Response(IN const IMessage& objMessage) const
{
    IMS_SINT32 nRetryAfter = m_objContext.GetMessageUtils().GetHeaderValueInt(
            &objMessage, ISipHeader::RETRY_AFTER_ANY);

    RegisterWithNextPcscfIfRequired(nRetryAfter);
}

PRIVATE
void UpdateErrorHandler::RegisterWithNextPcscfIfRequired(IN IMS_SINT32 nRetryAfter) const
{
    IMtcAosConnector* pAosConnector = m_objContext.GetService().GetAosConnector();
    if (pAosConnector)
    {
        if (nRetryAfter < 0)
        {
            pAosConnector->RegisterWithNextPcscf(0);
            return;
        }

        if (nRetryAfter * 1000 > Engine::GetConfiguration()
                                         ->GetSipConfig(m_objContext.GetSlotId())
                                         ->GetSipConfigV()
                                         ->GetTimerValue(ISipConfigV::TIMER_F))
        {
            pAosConnector->RegisterWithNextPcscf(nRetryAfter);
            return;
        }
    }
}
