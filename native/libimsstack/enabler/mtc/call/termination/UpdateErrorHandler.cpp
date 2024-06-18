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
#include "SipMethod.h"
#include "SipStatusCode.h"
#include "call/IMtcCallContext.h"
#include "call/termination/UpdateErrorHandler.h"
#include "helper/IMtcAosConnector.h"
#include "helper/IPassiveTimerHolder.h"
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

    return HandleResponse(*piMessage);
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
CallReasonInfo UpdateErrorHandler::HandleResponse(IN const IMessage& objMessage) const
{
    IMS_SINT32 nStatusCode = objMessage.GetStatusCode();
    IMS_ASSERT(nStatusCode >= SipStatusCode::SC_300);

    if (SipStatusCode::SC_300 <= nStatusCode && nStatusCode < SipStatusCode::SC_400)
    {
        return Handle3xxResponse(objMessage);
    }
    else if (SipStatusCode::SC_400 <= nStatusCode && nStatusCode < SipStatusCode::SC_500)
    {
        return Handle4xxResponse(objMessage);
    }
    else if (SipStatusCode::SC_500 <= nStatusCode && nStatusCode < SipStatusCode::SC_600)
    {
        return Handle5xxResponse(objMessage);
    }
    else if (SipStatusCode::SC_600 <= nStatusCode && nStatusCode < SipStatusCode::SC_MAX)
    {
        return Handle6xxResponse(objMessage);
    }
    return CallReasonInfo(CODE_SIP_SERVER_ERROR);
}

PRIVATE
CallReasonInfo UpdateErrorHandler::Handle3xxResponse(IN const IMessage& objMessage)
{
    IMS_SINT32 nStatusCode = objMessage.GetStatusCode();

    return CallReasonInfo(CODE_SIP_SERVER_ERROR, nStatusCode);
}

PRIVATE
CallReasonInfo UpdateErrorHandler::Handle4xxResponse(IN const IMessage& objMessage) const
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
CallReasonInfo UpdateErrorHandler::Handle5xxResponse(IN const IMessage& objMessage) const
{
    IMS_SINT32 nStatusCode = objMessage.GetStatusCode();

    switch (nStatusCode)
    {
        case SipStatusCode::SC_501:
        case SipStatusCode::SC_502:
            return CallReasonInfo(CODE_USER_TERMINATED_BY_REMOTE, nStatusCode);
        case SipStatusCode::SC_503:
            return Handle503Response(objMessage);
    }

    return CallReasonInfo(CODE_SIP_SERVER_ERROR, nStatusCode);
}

PRIVATE
CallReasonInfo UpdateErrorHandler::Handle6xxResponse(IN const IMessage& objMessage)
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
CallReasonInfo UpdateErrorHandler::Handle503Response(IN const IMessage& objMessage) const
{
    IMS_SINT32 nRetryAfter = m_objContext.GetMessageUtils().GetHeaderValueInt(
            &objMessage, ISipHeader::RETRY_AFTER_ANY);
    if (IsRegisterWithNextPcscfRequiredFor503(nRetryAfter, objMessage.GetMethod()))
    {
        RegisterFor503(nRetryAfter);
        return CallReasonInfo(CODE_SIP_SERVICE_UNAVAILABLE, objMessage.GetStatusCode());
    }

    IMS_SINT32 nRetryAfterInMillis = nRetryAfter * 1000;
    m_objContext.GetPassiveTimerHolder().AddTimer(
            IPassiveTimerHolder::Type::CALL_BLOCKED_BY_RETRY_AFTER, nRetryAfterInMillis);

    return CallReasonInfo(CODE_SIP_SERVER_ERROR, objMessage.GetStatusCode());
}

PRIVATE
void UpdateErrorHandler::RegisterFor503(IN IMS_SINT32 nRetryAfter) const
{
    IMtcAosConnector* pAosConnector = m_objContext.GetService().GetAosConnector();
    if (pAosConnector)
    {
        pAosConnector->RegisterWithNextPcscf(nRetryAfter > 0 ? nRetryAfter : 0);
    }
}

PRIVATE
IMS_BOOL UpdateErrorHandler::IsRegisterWithNextPcscfRequiredFor503(
        IN IMS_SINT32 nRetryAfter, IN const SipMethod& objMethod) const
{
    return nRetryAfter <= 0 ||
            nRetryAfter * 1000 > Engine::GetConfiguration()
                                         ->GetSipConfig(m_objContext.GetSlotId())
                                         ->GetSipConfigV()
                                         ->GetTimerValue(objMethod.Equals(SipMethod::INVITE)
                                                         ? ISipConfigV::TIMER_B
                                                         : ISipConfigV::TIMER_F);
}
