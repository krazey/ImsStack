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
#include "RcsMessageService.h"

#include "Connector.h"

#include "ImsCore.h"
#include "config/IMConstants.h"
#include "ImsServiceConfig.h"

#include "ServiceMemory.h"
#include "ServiceMessage.h"
#include "ServiceTrace.h"
#include "SipMethod.h"
#include "RcsMessageTracker.h"

#include "ICoreService.h"
#include "IDirectCoreServiceListener.h"
#include "IServiceFilterCriteria.h"
#include "ISipClientConnection.h"
#include "ISipServerConnection.h"
#include "ISipConnectionFactory.h"
#include "ISipConnectionFactoryListener.h"
#include "ISipHeader.h"
#include "ISipMessage.h"
#include "IURcsMessageService.h"
#include "IURcsMessageConstant.h"

__IMS_TRACE_TAG_USER_DECL__("IMS_SNC");

PUBLIC
RcsMessageService::RcsMessageService(IN const AString& strName, IN const IMS_SINT32 nSlotId) :
        ImsService(strName),
        m_nSlotId(nSlotId),
        m_piCoreService(IMS_NULL),
        m_piscf(IMS_NULL)
{
    IMS_TRACE_I(
            "+RcsMessageService : Service Name = %s\n Slot = %d", GetName().GetStr(), m_nSlotId, 0);
    IMS_TRACE_MEM("SNC_MSG", "IM_M : RCSIMService = %" PFLS_u, sizeof(RcsMessageService), 0, 0);

    EnableCoreService();
}

PUBLIC
VIRTUAL RcsMessageService::~RcsMessageService()
{
    DisableCoreService();
    objRcsMessages.Clear();
}

PRIVATE
void RcsMessageService::EnableCoreService()
{
    IMS_TRACE_I("EnableCoreService", 0, 0, 0);

    AString strServiceID("serviceId=");
    strServiceID += ImsServiceConfig::GetServiceName(ImsServiceId::SIP_DELEGATE);
    m_piCoreService = DYNAMIC_CAST(ICoreService*,
            (Connector::Open(ImsCore::CONNECTION_SCHEME,
                    ImsServiceConfig::GetAppName(ImsAppId::SIP_DELEGATE), strServiceID)));

    if (m_piCoreService == IMS_NULL)
    {
        IMS_TRACE_E(0, "m_piCoreService is null, Creating failed", 0, 0, 0);
        return;
    }

    m_piscf = m_piCoreService->CreateSipConnectionFactory();

    m_piscf->SetListener(this);                // mid-dialog incoming request
    m_piCoreService->SetDirectListener(this);  // standalone Sip incoming request

    // to let the core engine to notify MessageService
    // when it receives a invitation with the Accept-Contact header
    if (!RegisterIMServiceTag())
    {
        IMS_TRACE_E(0, "Register IM IARI tag failed", 0, 0, 0);
    }
}

PRIVATE
void RcsMessageService::DisableCoreService()
{
    IMS_TRACE_I("DisableCoreService", 0, 0, 0);
    if (m_piCoreService != IMS_NULL)
    {
        m_piCoreService->SetListener(IMS_NULL);
        m_piCoreService->Close();
        m_piCoreService = IMS_NULL;
    }
}

PRIVATE
VIRTUAL IMS_BOOL RcsMessageService::OnMessage(IN IMSMSG& objMSG)
{
    IMS_TRACE_I("\nOnMessage : Service Name = %s Msg = %d", GetName().GetStr(), objMSG.nMSG, 0);
    IMS_SINT32 nMSG = objMSG.GetName();

    if (nMSG == IUSncService::OPENMESSAGE_CMD)
    {
        return HandleOpenMSG(objMSG);
    }
    else if (nMSG == IUSncService::SENDMESSAGE_CMD)
    {
        return HandleSessionMSG(objMSG);
    }
    else if (nMSG == IUSncService::NOTIFYMESSAGERECEIVEERROR_CMD)
    {
        return HandleNotifyReceiveErrorMSG(objMSG);
    }
    else if (nMSG == IUSncService::CLOSESESSION_CMD)
    {
        HandleCloseSessionMSG(objMSG);
    }
    else
    {
        return IMS_FALSE;
    }
    return IMS_TRUE;
}

PRIVATE
IMS_BOOL RcsMessageService::CloseSession(IN IMS_UINTP nSessionId)
{
    IMS_TRACE_D("CloseSession : SessionId= %" PFLS_u, nSessionId, 0, 0);

    IMS_BOOL bRet = IMS_FALSE;
    RcsMessageTracker* pRcsMessage = objRcsMessages.Get(nSessionId);

    if (pRcsMessage != IMS_NULL)
    {
        objRcsMessages.Remove(nSessionId);
        pRcsMessage->Abort(IURcsMessageTerminateReason::USER_ACTION, IMS_FALSE);

        bRet = IMS_TRUE;
    }

    IMS_TRACE_D("Remaining sessions [%d]", objRcsMessages.GetSize(), 0, 0);

    return bRet;
}

PRIVATE
void RcsMessageService::HandleErrorCase(IN IMSMSG& objMSG)
{
    IMS_SINT32 nMsg = objMSG.GetName();

    IUSncSendFailureIndParam* pParam = REINTERPRET_CAST(IUSncSendFailureIndParam*, objMSG.nLparam);

    // Internal Tracker Obj is Null.
    pParam->nReason = IURcsMessageFailureReason::MESSAGE_FAILURE_REASON_UNKNOWN;
    PostNotification(nMsg, reinterpret_cast<IMS_UINTP>(pParam));
}

PRIVATE
void RcsMessageService::PostNotification(IN IMS_SINT32 nMSG, IN IMS_UINTP npParam)
{
    IMSMSG objUIMsg(nMSG, 0, reinterpret_cast<IMS_UINTP>(npParam));
    MessageService::PostMessage(AString("JniSipControllerServiceThread"), objUIMsg);
}

PRIVATE
IMS_BOOL RcsMessageService::RegisterIMServiceTag()
{
    if (m_piCoreService != IMS_NULL)
    {
        IServiceFilterCriteria* piSFC = m_piCoreService->GetFilterCriteria();

        if (piSFC != IMS_NULL)
        {
            AString strTagBase("*;");

            SipMethod objMethod(SipMethod::INVITE);
            TriggerPoint objTP(objMethod);

            AString strIMTag;
            strIMTag.Append(TextParser::CHAR_ASTERISK);
            strIMTag.Append(TextParser::CHAR_SEMICOLON);
            strIMTag.Append(IMConstants::TAG_OMA_IM);

            AString strHTTPTag;
            strHTTPTag.Append(TextParser::CHAR_ASTERISK);
            strHTTPTag.Append(TextParser::CHAR_SEMICOLON);
            strHTTPTag.Append(IMConstants::TAG_HTTP_FILETRANSFER);

            AString strGeolocationTag;
            strGeolocationTag.Append(TextParser::CHAR_ASTERISK);
            strGeolocationTag.Append(TextParser::CHAR_SEMICOLON);
            strGeolocationTag.Append(IMConstants::TAG_GEOLOCATIONPUSH);

            AString strCPMSessionTag(strTagBase);
            strCPMSessionTag.Append(IMConstants::TAG_CPM_SESSION);

            AString strCPMMessageTag(strTagBase);
            strCPMMessageTag.Append(IMConstants::TAG_CPM_MSG);

            AString strCPMDeferredMessageTag(strTagBase);
            strCPMDeferredMessageTag.Append(IMConstants::TAG_CPM_DEFERRED);

            AString strCPMLargeMessageTag(strTagBase);
            strCPMLargeMessageTag.Append(IMConstants::TAG_CPM_LARGE_MSG);

            AString strCPMFileTransferTag(strTagBase);
            strCPMFileTransferTag.Append(IMConstants::TAG_CPM_FILE_TRANSFER);

            AString strCPMSystemMsgTag(strTagBase);
            strCPMSystemMsgTag.Append(IMConstants::TAG_CPM_SYSTEM_MSG);

            // Sets the trigger point for INVITE
            objTP.RemoveAllHeaders();
            objTP.AddHeader(ISipHeader::ACCEPT_CONTACT, strHTTPTag);
            piSFC->AddTriggerPoint(objTP);

            objTP.RemoveAllHeaders();
            objTP.AddHeader(ISipHeader::ACCEPT_CONTACT, strGeolocationTag);
            piSFC->AddTriggerPoint(objTP);

            objTP.RemoveAllHeaders();
            objTP.AddHeader(ISipHeader::ACCEPT_CONTACT, strCPMSessionTag);
            piSFC->AddTriggerPoint(objTP);

            objTP.RemoveAllHeaders();
            objTP.AddHeader(ISipHeader::ACCEPT_CONTACT, strCPMMessageTag);
            piSFC->AddTriggerPoint(objTP);

            objTP.RemoveAllHeaders();
            objTP.AddHeader(ISipHeader::ACCEPT_CONTACT, strCPMDeferredMessageTag);
            piSFC->AddTriggerPoint(objTP);

            objTP.RemoveAllHeaders();
            objTP.AddHeader(ISipHeader::ACCEPT_CONTACT, strCPMLargeMessageTag);
            piSFC->AddTriggerPoint(objTP);

            objTP.RemoveAllHeaders();
            objTP.AddHeader(ISipHeader::ACCEPT_CONTACT, strCPMFileTransferTag);
            piSFC->AddTriggerPoint(objTP);

            objTP.RemoveAllHeaders();
            objTP.AddHeader(ISipHeader::ACCEPT_CONTACT, strCPMSystemMsgTag);
            piSFC->AddTriggerPoint(objTP);

            SipMethod objMessageMethod(SipMethod::MESSAGE);
            TriggerPoint objMessageTP(objMessageMethod);

            // Sets the trigger point for MESSAGE
            AString strRevokeMessageTag(strTagBase);
            strRevokeMessageTag.Append(IMConstants::TAG_CPM_CFS);

            objMessageTP.RemoveAllHeaders();
            objMessageTP.AddHeader(ISipHeader::ACCEPT_CONTACT, strCPMSessionTag);
            piSFC->AddTriggerPoint(objMessageTP);

            objMessageTP.RemoveAllHeaders();
            objMessageTP.AddHeader(ISipHeader::ACCEPT_CONTACT, strCPMMessageTag);
            piSFC->AddTriggerPoint(objMessageTP);

            objMessageTP.RemoveAllHeaders();
            objMessageTP.AddHeader(ISipHeader::ACCEPT_CONTACT, strCPMDeferredMessageTag);
            piSFC->AddTriggerPoint(objMessageTP);

            objMessageTP.RemoveAllHeaders();
            objMessageTP.AddHeader(ISipHeader::ACCEPT_CONTACT, strCPMLargeMessageTag);
            piSFC->AddTriggerPoint(objMessageTP);

            objMessageTP.RemoveAllHeaders();
            objMessageTP.AddHeader(ISipHeader::ACCEPT_CONTACT, strCPMFileTransferTag);
            piSFC->AddTriggerPoint(objMessageTP);

            objMessageTP.RemoveAllHeaders();
            objMessageTP.AddHeader(ISipHeader::ACCEPT_CONTACT, strCPMSystemMsgTag);
            piSFC->AddTriggerPoint(objMessageTP);

            objMessageTP.RemoveAllHeaders();
            objMessageTP.AddHeader(ISipHeader::ACCEPT_CONTACT, strRevokeMessageTag);
            piSFC->AddTriggerPoint(objMessageTP);

            piSFC->SetCalleePreference(objMethod);
            piSFC->SetCalleePreference(objMessageMethod);
            return IMS_TRUE;
        }
    }
    return IMS_FALSE;
}

PRIVATE
void RcsMessageService::convertMessage(ISipMessage* message)
{
    IUSncMessageParam* pOutParam = new IUSncMessageParam();
    // StartLine
    AString startLine = message->ToByteArray(ISipMessage::OPT_START_LINE).ToString();
    pOutParam->pszStartLine = startLine.GetStr();

    // HeaderSection
    AString headerSection = message->ToByteArray(ISipMessage::OPT_HEADER_PART).ToString();
    pOutParam->pszHeaderSection = headerSection.GetStr();

    // BodyContent
    AString objContent = message->ToByteArray(ISipMessage::OPT_BODY_PART).ToString();
    pOutParam->pszContent = objContent.GetStr();
}

IMS_BOOL RcsMessageService::HandleOpenMSG(IN IMSMSG& objMSG)
{
    IMS_TRACE_D("HandleOpenMSG()", 0, 0, 0);
    IUSncOpenCmdParam* pParam = REINTERPRET_CAST(IUSncOpenCmdParam*, objMSG.nLparam);
    if (pParam == IMS_NULL)
    {
        return IMS_FALSE;
    }

    IMS_UINTP nSessionId = pParam->nSessionID;
    AString strListenerThread(pParam->szThread);
    if (strListenerThread.GetLength() > 0)
    {
        RcsMessageTracker* pRcsMessageTracker = objRcsMessages.Get(nSessionId);
        if (pRcsMessageTracker == IMS_NULL)
        {
            pRcsMessageTracker = new RcsMessageTracker(m_piscf, m_nSlotId);
            pRcsMessageTracker->SetSessionId(nSessionId);
            pRcsMessageTracker->SetListenerThread(strListenerThread);
            objRcsMessages.Add(pRcsMessageTracker);
        }
        else
        {
            pRcsMessageTracker->SetListenerThread(strListenerThread);
        }
    }
    else
    {
        IMS_TRACE_E(0, "Exception!! SessionId[%" PFLS_u "] Message[%d] nReason[%d]", nSessionId,
                objMSG.GetName(),
                IURcsMessageFailureReason::MESSAGE_FAILURE_REASON_INVALID_PARAMETER);
        // TODO::PostNotification
        return IMS_FALSE;
    }

    delete pParam;
    return IMS_TRUE;
}

IMS_BOOL RcsMessageService::HandleSessionMSG(IN IMSMSG& objMSG)
{
    IMS_TRACE_D("HandleSessionMSG()", 0, 0, 0);
    IUSncSessionData* pParam = REINTERPRET_CAST(IUSncSessionData*, objMSG.nLparam);
    if (pParam == IMS_NULL)
    {
        return IMS_FALSE;
    }

    RcsMessageTracker* pRcsMessageTracker = objRcsMessages.Get(pParam->nSessionID);

    if (pRcsMessageTracker == IMS_NULL)
    {
        IMS_TRACE_E(0, "HandleSessionMSG : NO Tracker! [%" PFLS_u "]", pParam->nSessionID, 0, 0);

        HandleErrorCase(objMSG);
        return IMS_FALSE;
    }

    if (pRcsMessageTracker->HandleMessage(objMSG) == IMS_FALSE)
    {
        IMS_TRACE_E(0, "HandleSessionMSG : NOT HANDLED! [%" PFLS_u "]", pParam->nSessionID, 0, 0);

        delete pParam;
        return IMS_FALSE;
    }
    return IMS_TRUE;
}

IMS_BOOL RcsMessageService::HandleCloseSessionMSG(IN IMSMSG& objMSG)
{
    IMS_TRACE_D("HandleCloseSessionMSG()", 0, 0, 0);
    IUSncCloseSessionCmdParam* pParam =
            REINTERPRET_CAST(IUSncCloseSessionCmdParam*, objMSG.nLparam);

    if (pParam != IMS_NULL)
    {
        CloseSession(pParam->nSessionID);

        delete pParam;
    }
    else
    {
        return IMS_FALSE;
    }
    return IMS_TRUE;
}

IMS_BOOL RcsMessageService::HandleNotifyReceiveErrorMSG(IN IMSMSG& objMSG)
{
    IMS_TRACE_D("HandleNotifyReceiveErrorMSG()", 0, 0, 0);
    IUSncNotifyErrorCmdParam* pParam = REINTERPRET_CAST(IUSncNotifyErrorCmdParam*, objMSG.nLparam);
    if (pParam == IMS_NULL)
    {
        return IMS_FALSE;
    }

    RcsMessageTracker* pRcsMessageTracker = objRcsMessages.Get(pParam->nSessionID);

    if (pRcsMessageTracker == IMS_NULL)
    {
        IMS_TRACE_E(0, "HandleSessionMSG : NO Tracker! [%" PFLS_u "]", pParam->nSessionID, 0, 0);

        HandleErrorCase(objMSG);
        return IMS_FALSE;
    }

    if (pRcsMessageTracker->HandleMessage(objMSG) == IMS_FALSE)
    {
        IMS_TRACE_E(0, "HandleSessionMSG : NOT HANDLED! [%" PFLS_u "]", pParam->nSessionID, 0, 0);

        delete pParam;
        return IMS_FALSE;
    }
    return IMS_TRUE;
}

// standalone SIP Incoming Request
PUBLIC
VIRTUAL IMS_SINT32 RcsMessageService::DirectCoreService_TransactionReceived(
        IN ICoreService* piService, IN ISipConnectionFactory* piScf)
{
    IMS_TRACE_I("DirectCoreService_TransactionReceived()", 0, 0, 0);

    if (m_piCoreService != piService)
    {
        IMS_TRACE_E(0, "SERVICE MISMATCHED", 0, 0, 0);
        return m_piCoreService->RESULT_DIRECT_TXN_NOT_HANDLED;
    }

    ISipServerConnection* piSsc = piScf->GetNewServerConnection();
    ISipDialog* piDialog = piSsc->GetDialog();
    piScf->SetDialog(piDialog);
    piScf->SetSscForCancel(piSsc);

    ISipMessage* piMessage = piSsc->GetMessage();
    convertMessage(piMessage);

    return m_piCoreService->RESULT_DIRECT_TXN_HANDLED;
}

// Mid-Dialog SIP Incoming Request
PUBLIC
VIRTUAL void RcsMessageService::ConnectionFactory_NotifyRequest(
        IN ISipConnectionFactory* piScFactory, IN ISipServerConnection* piSsc)
{
    IMS_TRACE_I("ConnectionFactory_NotifyRequest()", 0, 0, 0);
    (void)piScFactory;

    ISipMessage* piMessage = piSsc->GetMessage();

    if (piMessage == IMS_NULL)
    {
        IMS_TRACE_E(0, "SIPMessage is null!", 0, 0, 0);
        return;
    }
    convertMessage(piMessage);
}
