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
STATE_MSG_ENTRY(IUSncService::SENDMESSAGE_CMD, &RcsMessageTracker::StateINITIATED_SendMessage)
STATE_MSG_ENTRY(IUSncService::NOTIFYMESSAGERECEIVEERROR_CMD,
        &RcsMessageTracker::StateINITIATED_NotifyReceiveError)
END_STATE_MSG_MAP()

BEGIN_STATE_MSG_MAP(RcsMessageTracker, SENDING)
STATE_MSG_ENTRY(MESSAGE_SENT, &RcsMessageTracker::StateSENDING_Sent)
STATE_MSG_ENTRY(MESSAGE_SENDFAILED, &RcsMessageTracker::StateSENDING_SendFailed)
END_STATE_MSG_MAP()

BEGIN_STATE_MSG_MAP(RcsMessageTracker, TERMINATED)
END_STATE_MSG_MAP()

// MO
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

    (void)m_piscf;
    (void)m_piscc;
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
    (void)bNeedAnswer;
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
    (void)objMSG;
    return IMS_TRUE;
}

PRIVATE
IMS_BOOL RcsMessageTracker::StateINITIATED_NotifyReceiveError(IN IMSMSG& objMSG)
{
    IMS_TRACE_D("StateINITIATED_SendMessage", 0, 0, 0);
    (void)objMSG;
    return IMS_TRUE;
}

PRIVATE
IMS_BOOL RcsMessageTracker::StateSENDING_Sent(IN IMSMSG& objMSG)
{
    IMS_TRACE_D("StateSENDING_Sent", 0, 0, 0);
    (void)objMSG;
    return IMS_TRUE;
}

PRIVATE
IMS_BOOL RcsMessageTracker::StateSENDING_SendFailed(IN IMSMSG& objMSG)
{
    IMS_TRACE_D("StateSENDING_SendFailed", 0, 0, 0);
    (void)objMSG;
    return IMS_TRUE;
}

PRIVATE
IMS_RESULT RcsMessageTracker::SendMessage(IN IUSncMessageParam* pParam)
{
    IMS_TRACE_D("SendMessage()", 0, 0, 0);
    (void)pParam;
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
    (void)piScc;
    (void)piForkedScc;
}