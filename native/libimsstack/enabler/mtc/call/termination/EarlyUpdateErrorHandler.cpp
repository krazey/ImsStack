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
#include "ServiceTrace.h"
#include "SipStatusCode.h"
#include "call/IMtcCallContext.h"
#include "call/MtcCallManager.h"
#include "call/termination/EarlyUpdateErrorHandler.h"
#include "call/termination/StartErrorHandler.h"
#include "call/termination/UpdateErrorHandler.h"
#include "helper/IMtcAosConnector.h"
#include "helper/IPassiveTimerHolder.h"
#include "utility/IMessageUtils.h"

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

    if (nStatusCode == SipStatusCode::SC_503)
    {
        return Handle503Response(*piMessage);
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

PRIVATE
CallReasonInfo EarlyUpdateErrorHandler::Handle503Response(IN const IMessage& objMessage) const
{
    if (!m_objContext.GetCallManager().GetCallsByState(IMtcCall::State::ESTABLISHED).IsEmpty())
    {
        return CallReasonInfo(CODE_REJECT_INTERNAL_ERROR, objMessage.GetStatusCode());
    }

    IMS_SINT32 nRetryAfter = m_objContext.GetMessageUtils().GetHeaderValueInt(
            &objMessage, ISipHeader::RETRY_AFTER_ANY);
    IMS_SINT32 nRetryAfterInMillis = nRetryAfter * 1000;
    if (IsRegisterWithNextPcscfAndRedialRequiredFor503(nRetryAfter))
    {
        if (RegisterFor503(nRetryAfter))
        {
            return CallReasonInfo(CODE_INTERNAL_REDIAL, EXTRA_CODE_REDIAL_WITH_NEXT_PCSCF);
        }

        return CallReasonInfo(CODE_REJECT_INTERNAL_ERROR);
    }

    if (m_objContext.GetService().IsCsfbAvailable())
    {
        SetTimerForImsCallBlocking(nRetryAfterInMillis);
        return CallReasonInfo(
                CODE_LOCAL_CALL_CS_RETRY_REQUIRED, EXTRA_CODE_CALL_RETRY_SILENT_REDIAL);
    }

    AString strRetryAfter;
    strRetryAfter.SetNumber(nRetryAfterInMillis);
    return CallReasonInfo(CODE_INTERNAL_REDIAL, EXTRA_CODE_REDIAL_BY_RETRY_AFTER, strRetryAfter);
}

PRIVATE
IMS_BOOL EarlyUpdateErrorHandler::RegisterFor503(IN IMS_SINT32 nRetryAfter) const
{
    IMtcAosConnector* pAosConnector = m_objContext.GetService().GetAosConnector();
    if (pAosConnector)
    {
        pAosConnector->RegisterWithNextPcscf(nRetryAfter > 0 ? nRetryAfter : 0);
        return IMS_TRUE;
    }

    return IMS_FALSE;
}

PRIVATE
IMS_BOOL EarlyUpdateErrorHandler::IsRegisterWithNextPcscfAndRedialRequiredFor503(
        IN IMS_SINT32 nRetryAfter) const
{
    return nRetryAfter <= 0 ||
            nRetryAfter * 1000 > Engine::GetConfiguration()
                                         ->GetSipConfig(m_objContext.GetSlotId())
                                         ->GetSipConfigV()
                                         ->GetTimerValue(ISipConfigV::TIMER_F);
}

PRIVATE
void EarlyUpdateErrorHandler::SetTimerForImsCallBlocking(IN IMS_SINT32 nRetryAfterInMillis) const
{
    if (nRetryAfterInMillis > 0)
    {
        m_objContext.GetPassiveTimerHolder().AddTimer(
                IPassiveTimerHolder::Type::CALL_BLOCKED_BY_RETRY_AFTER, nRetryAfterInMillis);
    }
}
