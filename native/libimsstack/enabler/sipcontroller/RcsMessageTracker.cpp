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
#include "RcsMessageTracker.h"

#include "Connector.h"
#include "ICoreService.h"
#include "ImsCore.h"
#include "config/IMConstants.h"
#include "ImsServiceConfig.h"
#include "IServiceFilterCriteria.h"
#include "ISipClientConnection.h"
#include "ISipHeader.h"
#include "ISipMessage.h"
#include "IURcsMessageService.h"
#include "IURcsMessageConstant.h"
#include "ServiceTrace.h"
#include "ServiceMessage.h"
#include "SipMethod.h"

__IMS_TRACE_TAG_USER_DECL__("IMS_SNC");

/* -------------------------------------------------------------------------------------------------
    StateMachine
-------------------------------------------------------------------------------------------------*/
BEGIN_STATE_MAP(RcsMessageTracker)
STATE_ENTRY(INITIATED)
STATE_ENTRY(SENDING)
STATE_ENTRY(TERMINATED)
END_STATE_MAP()

BEGIN_STATE_MSG_MAP(RcsMessageTracker, INITIATED)
STATE_MSG_ENTRY(IUSncService::SEND_MESSAGE_CMD, &RcsMessageTracker::StateINITIATED_SendMessage)
STATE_MSG_ENTRY(IUSncService::NOTIFY_MESSAGE_RECEIVE_ERROR_CMD,
        &RcsMessageTracker::StateINITIATED_NotifyReceiveError)
END_STATE_MSG_MAP()

BEGIN_STATE_MSG_MAP(RcsMessageTracker, SENDING)
STATE_MSG_ENTRY(MESSAGE_SENT, &RcsMessageTracker::StateSENDING_Sent)
STATE_MSG_ENTRY(MESSAGE_SENDFAILED, &RcsMessageTracker::StateSENDING_SendFailed)
END_STATE_MSG_MAP()

BEGIN_STATE_MSG_MAP(RcsMessageTracker, TERMINATED)
END_STATE_MSG_MAP()

PUBLIC
RcsMessageTracker::RcsMessageTracker(ISipConnectionFactory* _piscf, IN IMS_SINT32 nSimSlot) :
        ImsStateMachine(INITIATED),
        m_piscf(_piscf),
        m_piscc(IMS_NULL),
        m_nSimSlot(nSimSlot),
        m_nSessionId(0),
        m_strListenerThread(AString::ConstEmpty())
{
    IMS_TRACE_MEM(
            "SNC_MSG", "IM_M : RcsMessageTracker = %" PFLS_u, sizeof(RcsMessageTracker), 0, 0);
    IMS_TRACE_I("RcsMessageTracker", 0, 0, 0);

    (void)m_nSimSlot;
}

PUBLIC VIRTUAL RcsMessageTracker::~RcsMessageTracker()
{
    IMS_TRACE_MEM(
            "SNC_MSG", "IM_F : RcsMessageTracker = %" PFLS_u, sizeof(RcsMessageTracker), 0, 0);
    IMS_TRACE_I("~RcsMessageTracker", 0, 0, 0);
}

PUBLIC
void RcsMessageTracker::SetSessionId(IN IMS_UINTP _nSessionId)
{
    m_nSessionId = _nSessionId;
}

PUBLIC
IMS_UINTP RcsMessageTracker::GetSessionId()
{
    return m_nSessionId;
}

PUBLIC
IMS_BOOL RcsMessageTracker::HandleMessage(IN IMSMSG& objMSG)
{
    IMS_TRACE_D("HandleMessage : STATE[%d] MSG[%d]", GetState(), objMSG.nMSG, 0);

    IMS_BOOL bRetVal = OnStateMessage(objMSG);

    if (bRetVal == IMS_FALSE)
    {
        IMS_TRACE_E(0, "HandleMessage : Not Handled!", 0, 0, 0);
    }
    return bRetVal;
}

PUBLIC
void RcsMessageTracker::Abort(IN IMS_SINT32 nReason, IN const IMS_BOOL bNeedAnswer /* = IMS_TRUE*/)
{
    IMS_TRACE_I("Abort : reason[%d]", nReason, 0, 0);
    // ToDo
    if (GetState() != TERMINATED && bNeedAnswer == IMS_TRUE)
    {
        // NotifyMessageSendResult(nReason);
    }
    SetState(TERMINATED);
}

PUBLIC
void RcsMessageTracker::SetListenerThread(IN const AString& strThread)
{
    m_strListenerThread = strThread;
}

PROTECTED
void RcsMessageTracker::PostNotification(IN IMS_SINT32 nMSG, IN IMS_UINTP npParam)
{
    if (m_strListenerThread.GetLength() == 0)
    {
        IMS_TRACE_I("PostNotification : Listener is NULL! [%d]", nMSG, 0, 0);
        return;
    }

    IMSMSG objUIMsg(nMSG, 0, reinterpret_cast<IMS_UINTP>(npParam));
    MessageService::PostMessage(m_strListenerThread, objUIMsg);
}

// START -- STATE MACHINE
PRIVATE
IMS_BOOL RcsMessageTracker::StateINITIATED_SendMessage(IN IMSMSG& objMSG)
{
    IMS_TRACE_D("StateINITIATED_SendMessage", 0, 0, 0);
    IUSncSendMessageParam* pParam = REINTERPRET_CAST(IUSncSendMessageParam*, objMSG.nLparam);
    if (SendMessage(pParam) == IMS_SUCCESS)
    {
        SetState(SENDING);
    }
    else
    {
        SetState(TERMINATED);
        IUSncSendFailureIndParam* pFailureParam =
                REINTERPRET_CAST(IUSncSendFailureIndParam*, objMSG.nLparam);
        /*
        Error Cases
            IUSncService::OPEN_MESSAGE_CMD:
            IUSncService::SEND_MESSAGE_CMD:
            IUSncService::CLOSE_SESSION_CMD:
            IUSncService::NOTIFY_MESSAGE_RECEIVE_ERROR_CMD:
        */
        // Internal Tracker Obj is Null.
        pFailureParam->m_nReason = IURcsMessageFailureReason::MESSAGE_FAILURE_REASON_UNKNOWN;
        // ToDo
        // pFailureParam->szTId = ;
        IMS_SINT32 nMsg = objMSG.GetName();
        PostNotification(nMsg, reinterpret_cast<IMS_UINTP>(pFailureParam));
    }
    return IMS_TRUE;
}

PRIVATE
IMS_BOOL RcsMessageTracker::StateINITIATED_NotifyReceiveError(IN IMSMSG& objMSG)
{
    IMS_TRACE_D("StateINITIATED_SendMessage", 0, 0, 0);
    IUSncNotifyErrorCmdParam* pParam = REINTERPRET_CAST(IUSncNotifyErrorCmdParam*, objMSG.nLparam);

    // When the application’s SipDelegateConnection is unreachable due to the application crashing.
    // Send Error Response from Framework

    NotifyReceiveError(pParam);

    return IMS_TRUE;
}

PRIVATE
IMS_BOOL RcsMessageTracker::StateSENDING_Sent(IN IMSMSG& objMSG)
{
    IMS_TRACE_D("StateSENDING_Sent", 0, 0, 0);
    IUSncSentMessageIndParam* pParam = REINTERPRET_CAST(IUSncSentMessageIndParam*, objMSG.nLparam);
    PostNotification(IUSncService::SEND_MESSAGE_FAILURE_IND, reinterpret_cast<IMS_UINTP>(pParam));
    return IMS_TRUE;
}

PRIVATE
IMS_BOOL RcsMessageTracker::StateSENDING_SendFailed(IN IMSMSG& objMSG)
{
    IMS_TRACE_D("StateSENDING_SendFailed", 0, 0, 0);
    IUSncSendFailureIndParam* pParam = REINTERPRET_CAST(IUSncSendFailureIndParam*, objMSG.nLparam);
    PostNotification(IUSncService::SEND_MESSAGE_FAILURE_IND, reinterpret_cast<IMS_UINTP>(pParam));
    return IMS_TRUE;
}

PRIVATE
IMS_RESULT RcsMessageTracker::SendMessage(IN IUSncSendMessageParam* pParam)
{
    IMS_TRACE_D("SendMessage()", 0, 0, 0);
    SetState(SENDING);
    (void)pParam;
    // ex)
    if (m_piscf == IMS_NULL)
    {
        return IMS_FAILURE;
    }

    if (pParam->m_nType == ISipMessage::TYPE_REQUEST)
    {
        SendRequest(pParam);
    }
    else if (pParam->m_nType == ISipMessage::TYPE_RESPONSE)
    {
        CreateResponse();
    }
    else
    {
        return IMS_FAILURE;
    }
    return IMS_SUCCESS;
}

PRIVATE
IMS_RESULT RcsMessageTracker::SendRequest(IN IUSncSendMessageParam* pParam)
{
    (void)pParam;
    SipMethod objMethod;

    m_piscc = m_piscf->CreateClientConnection(
            /*IN const SipMethod& objMethod*/ objMethod,
            /*IN const SipAddress* pFrom*/ IMS_NULL,
            /*IN const SipAddress* pTo*/ IMS_NULL);

    m_piscc->SetListener(this);
    ISipMessage* piMessage = m_piscc->GetMessage();
    piMessage->CopyHeadersAndBodyParts(IMS_NULL);

    IMS_RESULT result = m_piscc->Send();

    if (result != IMS_SUCCESS)
    {
        IUSncSendFailureIndParam* pFailureParam = new IUSncSendFailureIndParam();
        pFailureParam->m_nReason = IURcsMessageFailureReason::MESSAGE_FAILURE_REASON_UNKNOWN;

        // ToDo
        pFailureParam->m_strTId = "viaTransactionId";
        IMSMSG objMSG(MESSAGE_SENDFAILED, 0, reinterpret_cast<IMS_UINTP>(pFailureParam));

        HandleMessage(objMSG);
        return IMS_FAILURE;
    }

    IUSncSentMessageIndParam* pSentParam = new IUSncSentMessageIndParam();

    // ToDo
    pSentParam->m_strTId = "viaTransactionId";
    IMSMSG objMSG(MESSAGE_SENT, 0, reinterpret_cast<IMS_UINTP>(pSentParam));

    HandleMessage(objMSG);
    return IMS_SUCCESS;
}

PRIVATE
IMS_RESULT RcsMessageTracker::CreateResponse()
{
    return IMS_SUCCESS;
}

PRIVATE
IMS_RESULT RcsMessageTracker::NotifyReceiveError(IN IUSncNotifyErrorCmdParam* pParam)
{
    // ToDo
    (void)pParam;
    return IMS_SUCCESS;
}

// Message Response
PUBLIC
VIRTUAL void RcsMessageTracker::ClientConnection_NotifyResponse(
        IN ISipClientConnection* piScc, IN ISipClientConnection* piForkedScc)
{
    (void)piForkedScc;
    ISipMessage* isMessage = piScc->GetMessage();
    isMessage->GetStatusCode();
}