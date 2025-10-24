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

#include "CallReasonInfo.h"
#include "Engine.h"
#include "IConfiguration.h"
#include "ISession.h"
#include "MtcDef.h"
#include "ServiceTrace.h"
#include "call/IMtcCall.h"
#include "call/IMtcCallContext.h"
#include "call/message/EmergencyMessageFormatter.h"
#include "call/message/MessageSender.h"
#include "helper/TransactionTimerUpdateHelper.h"
#include <memory>

__IMS_TRACE_TAG_COM_MTC__;

PUBLIC
MessageSender::MessageSender(IN IMtcCallContext& objContext, IN ISession& objSession) :
        m_objContext(objContext),
        m_objSession(objSession),
        m_pFormatter(nullptr),
        m_objTimerUpdateHelper(
                objContext, Engine::GetConfiguration()->GetSipConfig(objContext.GetSlotId()))
{
    IMS_TRACE_I("+MessageSender", 0, 0, 0);
    CreateFormatter();
}

PUBLIC VIRTUAL MessageSender::~MessageSender()
{
    IMS_TRACE_I("~MessageSender", 0, 0, 0);
}

PUBLIC
IMS_RESULT MessageSender::Start(IN CallType eCallType)
{
    if (m_pFormatter->FormStartMessage(eCallType) != IMS_SUCCESS)
    {
        return IMS_FAILURE;
    }

    m_objTimerUpdateHelper.SetInviteTransactionTimer();
    IMS_RESULT nResult = m_objSession.Start();
    m_objTimerUpdateHelper.ResetInviteTransactionTimer();
    return nResult;
}

PUBLIC
IMS_RESULT MessageSender::SendProvisionalResponse(IN IMS_SINT32 eStatusCode, IN IMS_BOOL bReliable,
        IN IMS_BOOL bIncludeSdp, IN IMS_BOOL bIncludeAlertInfo)
{
    if (m_pFormatter->FormProvisionalResponseMessage(bIncludeAlertInfo) != IMS_SUCCESS)
    {
        return IMS_FAILURE;
    }

    if (bReliable)
    {
        return m_objSession.SendRpr(eStatusCode, AString::ConstNull(), bIncludeSdp);
    }

    return m_objSession.SendProvisionalResponse(eStatusCode);
}

PUBLIC
IMS_RESULT MessageSender::SendPrack()
{
    if (m_pFormatter->FormPrackMessage() != IMS_SUCCESS)
    {
        return IMS_FAILURE;
    }

    m_objTimerUpdateHelper.SetNonInviteTransactionTimer();
    IMS_RESULT nResult = m_objSession.SendPrack();
    m_objTimerUpdateHelper.ResetNonInviteTransactionTimer();
    return nResult;
}

PUBLIC
IMS_RESULT MessageSender::RespondToPrack(IN IMS_SINT32 eStatusCode)
{
    if (m_pFormatter->FormPrackResponseMessage() != IMS_SUCCESS)
    {
        return IMS_FAILURE;
    }

    return m_objSession.RespondToPrack(eStatusCode);
}

PUBLIC
IMS_RESULT MessageSender::SendEarlyUpdate(IN UpdateType eUpdateType)
{
    if (m_pFormatter->FormEarlyUpdateMessage(eUpdateType) != IMS_SUCCESS)
    {
        return IMS_FAILURE;
    }

    m_objTimerUpdateHelper.SetNonInviteTransactionTimer();
    IMS_RESULT nResult = m_objSession.UpdateEarlyMedia();
    m_objTimerUpdateHelper.ResetNonInviteTransactionTimer();
    return nResult;
}

PUBLIC
IMS_RESULT MessageSender::RespondToEarlyUpdate(IN IMS_SINT32 eStatusCode)
{
    if (m_pFormatter->FormEarlyUpdateResponseMessage() != IMS_SUCCESS)
    {
        return IMS_FAILURE;
    }

    return m_objSession.RespondToEarlyUpdate(eStatusCode);
}

PUBLIC
IMS_RESULT MessageSender::Accept()
{
    if (m_pFormatter->FormAcceptMessage() != IMS_SUCCESS)
    {
        return IMS_FAILURE;
    }

    return m_objSession.Accept();
}

PUBLIC
IMS_RESULT MessageSender::Reject(IN const CallReasonInfo& objReason)
{
    IMS_SINT32 eStatusCode;
    AString strPhrase;

    if (m_pFormatter->FormRejectMessage(objReason, eStatusCode, strPhrase) != IMS_SUCCESS)
    {
        return IMS_FAILURE;
    }

    return m_objSession.RejectEx(eStatusCode, strPhrase);
}

PUBLIC
IMS_RESULT MessageSender::SendAck()
{
    if (m_pFormatter->FormAckMessage() != IMS_SUCCESS)
    {
        return IMS_FAILURE;
    }

    return m_objSession.SendAck();
}

PUBLIC
IMS_RESULT MessageSender::Update(IN UpdateType eUpdateType, IN IMS_BOOL bIncludeAlertInfo,
        IN IMS_SINT32 eMethod, IN IMS_BOOL bSessionRefresh)
{
    if (m_pFormatter->FormUpdateMessage(eUpdateType, bIncludeAlertInfo) != IMS_SUCCESS)
    {
        return IMS_FAILURE;
    }

    return m_objSession.UpdateEx(eMethod, bSessionRefresh);
}

PUBLIC
IMS_RESULT MessageSender::AcceptUpdate()
{
    if (m_pFormatter->FormAcceptUpdateMessage() != IMS_SUCCESS)
    {
        return IMS_FAILURE;
    }

    return m_objSession.Accept();
}

PUBLIC
IMS_RESULT MessageSender::CancelUpdate(IN const CallReasonInfo& objReason)
{
    if (m_pFormatter->FormCancelUpdateMessage(objReason) != IMS_SUCCESS)
    {
        return IMS_FAILURE;
    }

    return m_objSession.TerminateEx(IMS_FALSE);
}

PUBLIC
IMS_RESULT MessageSender::Terminate(IN IMS_BOOL bUseBye, IN const CallReasonInfo& objReason)
{
    if (m_pFormatter->FormTerminateMessage(objReason) != IMS_SUCCESS)
    {
        return IMS_FAILURE;
    }

    m_objSession.SetReasonForCallTermination(objReason.nCode);
    return m_objSession.TerminateEx(bUseBye);
}

PRIVATE
void MessageSender::CreateFormatter()
{
    if (m_objContext.GetCallInfo().IsEmergency())
    {
        m_pFormatter = std::make_unique<EmergencyMessageFormatter>(m_objContext, m_objSession);
        return;
    }

    m_pFormatter = std::make_unique<MessageFormatter>(m_objContext, m_objSession);
}
