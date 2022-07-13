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

#include "ServiceMessage.h"
#include "ServiceTrace.h"
#include "SipStatusCode.h"
#include "SipHeaderName.h"
#include "IMessage.h"
#include "ISipMessage.h"
#include "ISipHeader.h"
#include "IPageMessage.h"
#include "IMessageBodyPart.h"
#include "IuMts.h"
#include "MtsApp.h"
#include "MtsService.h"
#include "message/IMtsMessage.h"
#include "IMtsMessageControllerListener.h"
#include "message/MtsMessage.h"
#include "message/MtsMessageController.h"
#include "utility/MtsDynamicLoader.h"

__IMS_TRACE_TAG_COM_SMS__;

PUBLIC
MtsMessageController::MtsMessageController(
        IN IMS_SINT32 nSlotId, IN MtsService* pMtsService, IN MtsDynamicLoader* pMtsDynamicLoader) :
        m_nSlotId(nSlotId),
        m_bProcessingMsg(IMS_FALSE),
        m_nCallTypeMsg(TYPE_CS),
        m_nCallStateMsg(STATE_IDLE),
        m_strLastRcvIpsmgwAddr(AString::ConstNull()),
        m_objMsgList(IMSList<IMtsMessage*>()),
        m_piMtsMessageControllerListener(IMS_NULL),
        m_objRPAckedMsgs(IMSList<IMtsMessage*>()),
        m_pMtsService(pMtsService),
        m_pMtsDynamicLoader(pMtsDynamicLoader)
{
    IMS_TRACE_I("+MtsMessageController [slot_%d]", m_nSlotId, 0, 0);

    m_pMtsService->SetListener(this);
}

PUBLIC MtsMessageController::~MtsMessageController()
{
    IMS_TRACE_I("~MtsMessageController [slot_%d]", m_nSlotId, 0, 0);

    DestroyMtsMessage();
}

PUBLIC void MtsMessageController::DestroyMtsMessage()
{
    IMS_UINT32 nMsgSize = m_objMsgList.GetSize();

    IMS_TRACE_I("DestroyMtsMessage() - MsgSize=[%d] m_nSlotId=[%d]", nMsgSize, m_nSlotId, 0);

    for (IMS_UINT32 index = 0; index < nMsgSize; index++)
    {
        IMtsMessage* pDeleteMessage = m_objMsgList.GetAt(index);

        if (pDeleteMessage != IMS_NULL)
        {
            delete DYNAMIC_CAST(MtsMessage*, pDeleteMessage);
            pDeleteMessage = IMS_NULL;
        }
    }
    m_objMsgList.Clear();

    IMS_UINT32 nBlockSize = m_objRPAckedMsgs.GetSize();
    for (IMS_UINT32 index = 0; index < nBlockSize; index++)
    {
        IMtsMessage* pDeleteBlock = m_objRPAckedMsgs.GetAt(index);

        if (pDeleteBlock != IMS_NULL)
        {
            delete DYNAMIC_CAST(MtsMessage*, pDeleteBlock);
            pDeleteBlock = IMS_NULL;
        }
    }
    m_objRPAckedMsgs.Clear();
    m_nCallTypeMsg = TYPE_CS;
    m_nCallStateMsg = STATE_IDLE;
}

PUBLIC void MtsMessageController::Add(IN IMtsMessage* piMtsMessage)
{
    m_objMsgList.Append(piMtsMessage);

    IMS_TRACE_I("Add() - messageCount(after)=%d", m_objMsgList.GetSize(), 0, 0);
}

PUBLIC void MtsMessageController::Remove(IN IMtsMessage* piMtsMessage)
{
    IMS_SINT32 nRemoveIndex = -1;
    IMS_BOOL bHasDeliverMsg = IMS_FALSE;

    for (IMS_UINT32 i = 0; i < m_objMsgList.GetSize(); i++)
    {
        IMtsMessage* piTmpMtsMessage = m_objMsgList.GetAt(i);

        if (piTmpMtsMessage == piMtsMessage)
        {
            nRemoveIndex = i;
            break;
        }
    }
    if (nRemoveIndex >= 0)
    {
        m_objMsgList.RemoveAt(nRemoveIndex);
    }

    if (m_objMsgList.GetSize() == 0)
    {
        if (m_piMtsMessageControllerListener != IMS_NULL)
        {
            m_piMtsMessageControllerListener->MtsMessageController_NoTransaction();
        }
    }
    else
    {
        for (IMS_UINT32 i = 0; i < m_objMsgList.GetSize(); i++)
        {
            IMtsMessage* piTmpMtsMessage = m_objMsgList.GetAt(i);
            if (IsDeliverMessage(piTmpMtsMessage->GetPageMessage()))
            {
                PostMessage(0, 0, 0);
                bHasDeliverMsg = IMS_TRUE;
                break;
            }
        }
    }

    nRemoveIndex = -1;
    for (IMS_UINT32 i = 0; i < m_objRPAckedMsgs.GetSize(); i++)
    {
        IMtsMessage* piTmpMtsMessage = m_objRPAckedMsgs.GetAt(i);

        if (piTmpMtsMessage == piMtsMessage)
        {
            nRemoveIndex = i;
            break;
        }
    }
    if (nRemoveIndex >= 0)
    {
        m_objRPAckedMsgs.RemoveAt(nRemoveIndex);
    }

    IMS_TRACE_I("Remove() - messageCount=%d, hasDeliverMsg=%s", m_objMsgList.GetSize(),
            _TRACE_B_(bHasDeliverMsg), 0);
}

PUBLIC IMtsMessage* MtsMessageController::Search(IN const AString& strDestination)
{
    IMtsMessage* piTmpMtsMessage = IMS_NULL;

    if (strDestination.GetLength() == 0)
    {
        return IMS_NULL;
    }

    IMS_TRACE_I("Search() - messageCount=%d size=%d", m_objMsgList.GetSize(),
            strDestination.GetLength(), 0);

    for (IMS_UINT32 i = 0; i < m_objMsgList.GetSize(); i++)
    {
        piTmpMtsMessage = m_objMsgList.GetAt(i);

        if (piTmpMtsMessage->IsReceivedMessage())
        {
            continue;
        }

        IMS_TRACE_D("searched sms message's destination = %s",
                piTmpMtsMessage->GetDestination().GetStr(), 0, 0);

        if (piTmpMtsMessage->GetDestination().Equals(strDestination))
        {
            return piTmpMtsMessage;
        }
    }

    return IMS_NULL;
}

PUBLIC IMtsMessage* MtsMessageController::Search(
        IN const AString& strDestination, IN IMS_SINT32 nMti)
{
    IMtsMessage* piTmpMtsMessage = IMS_NULL;

    if (strDestination.GetLength() == 0)
    {
        return IMS_NULL;
    }

    if (nMti != MtsSmUtils::MTS_3GPP_MTI_RP_DATA_From_MS && nMti != MtsSmUtils::MTS_SMS_MTI_NONE)
    {
        return IMS_NULL;
    }

    IMS_TRACE_I("Search() - messageCount=%d size=%d", m_objMsgList.GetSize(),
            strDestination.GetLength(), 0);

    for (IMS_UINT32 i = 0; i < m_objMsgList.GetSize(); i++)
    {
        piTmpMtsMessage = m_objMsgList.GetAt(i);

        if (piTmpMtsMessage->IsReceivedMessage())
        {
            continue;
        }

        if (piTmpMtsMessage->GetMti() == nMti)
        {
            IMS_TRACE_D("searched sms message's destination = %s",
                    piTmpMtsMessage->GetDestination().GetStr(), 0, 0);

            if (piTmpMtsMessage->GetDestination().Equals(strDestination))
            {
                return piTmpMtsMessage;
            }
        }
    }

    return IMS_NULL;
}

PUBLIC IMtsMessage* MtsMessageController::Search(
        IN IMS_SINT32 nMessageReference, IN IMS_SINT32 nMessageType /* = MESSAGE_TYPE_RECEIVE*/)
{
    IMS_TRACE_I("MtsMessageController::Search() - messageCount=%d", m_objMsgList.GetSize(), 0, 0);

    for (IMS_UINT32 i = 0; i < m_objMsgList.GetSize(); i++)
    {
        IMtsMessage* piMtsMessage = m_objMsgList.GetAt(i);
        IMS_BOOL bIsReceived = piMtsMessage->IsReceivedMessage();

        if ((nMessageType == MESSAGE_TYPE_RECEIVE && bIsReceived != IMS_TRUE) ||
                (nMessageType == MESSAGE_TYPE_SEND && bIsReceived == IMS_TRUE))
        {
            continue;
        }

        if (piMtsMessage->GetMessageReference() == nMessageReference)
        {
            return piMtsMessage;
        }
    }

    return IMS_NULL;
}

PUBLIC void MtsMessageController::RegisterNoTransactionListener(
        IN IMtsMessageControllerListener* piListener)
{
    m_piMtsMessageControllerListener = piListener;
}

PUBLIC void MtsMessageController::DeregisterNoTransactionListener(
        IN IMtsMessageControllerListener* piListener)
{
    if (m_piMtsMessageControllerListener == piListener)
    {
        m_piMtsMessageControllerListener = IMS_NULL;
    }
}

PUBLIC IMS_BOOL MtsMessageController::HasMessageSendingReceiving()
{
    return !m_objMsgList.IsEmpty();
}

PUBLIC void MtsMessageController::TerminateAllPendingMessages(IN IMS_BOOL bIs1xCallTerm)
{
    IMS_TRACE_I("TerminateAllPendingMessages() - messageCount=%d", m_objMsgList.GetSize(), 0, 0);

    if (m_objMsgList.IsEmpty() == IMS_TRUE)
    {
        IMS_TRACE_I("Msg Size is NULL", 0, 0, 0);
        return;
    }

    IMS_UINT32 nMsgSize = m_objMsgList.GetSize();

    if (nMsgSize <= 0)
    {
        IMS_TRACE_I("Msg Size is NULL", 0, 0, 0);
        m_bProcessingMsg = IMS_FALSE;
        return;
    }

    for (IMS_UINT32 index = 0; index < nMsgSize; index++)
    {
        IMtsMessage* piMtsMessage = m_objMsgList.GetAt(index);
        if (piMtsMessage != IMS_NULL)
        {
            piMtsMessage->TerminateMessage(bIs1xCallTerm);
        }
    }
    m_objMsgList.Clear();

    m_bProcessingMsg = IMS_FALSE;
}

PUBLIC void MtsMessageController::TerminateAllPendingMessagesEx(IN IMS_UINT32 nReason)
{
    IMS_TRACE_I("TerminateAllPendingMessagesEx() - messageCount=%d", m_objMsgList.GetSize(), 0, 0);

    if (m_objMsgList.IsEmpty() == IMS_TRUE)
    {
        IMS_TRACE_I("Msg Size is NULL", 0, 0, 0);
        return;
    }

    IMS_UINT32 nMsgSize = m_objMsgList.GetSize();

    if (nMsgSize <= 0)
    {
        IMS_TRACE_I("Msg Size is NULL", 0, 0, 0);
        return;
    }

    for (IMS_UINT32 index = 0; index < nMsgSize; index++)
    {
        IMtsMessage* piMtsMessage = m_objMsgList.GetAt(index);

        if (piMtsMessage != IMS_NULL)
        {
            piMtsMessage->TerminateMessageEx(nReason);
        }
    }

    m_objMsgList.Clear();
}

PUBLIC const AString& MtsMessageController::GetLastIpsmgwAddr()
{
    if (m_strLastRcvIpsmgwAddr.GetLength() <= 0)
    {
        IMS_TRACE_E(0, "m_strLastRcvIpsmgwAddr is empty", 0, 0, 0);
        return AString::ConstNull();
    }

    IMS_TRACE_D("Get Last IPSMGW Addr [%s]", m_strLastRcvIpsmgwAddr.GetStr(), 0, 0);

    return m_strLastRcvIpsmgwAddr;
}

PUBLIC void MtsMessageController::SetLastIpsmgwAddr(IN const AString& strSmgwAddr)
{
    if (strSmgwAddr.GetLength() <= 0)
    {
        IMS_TRACE_E(0, "Last IPSMGW address is empty", 0, 0, 0);
        return;
    }

    IMS_TRACE_D("Set Last IPSMGW Address [%s]", strSmgwAddr.GetStr(), 0, 0);
    m_strLastRcvIpsmgwAddr = strSmgwAddr;
}

PUBLIC IMS_BOOL MtsMessageController::IsDeliverMessage(IN IPageMessage* piPageMessage)
{
    IMS_TRACE_I("MtsMessageController::IsDeliverMessage ", 0, 0, 0);

    if (piPageMessage == IMS_NULL)
    {
        // It will send 400 Error response, In Operator Specific MtsMessage
        return IMS_FALSE;
    }

    IMessage* piMessage = piPageMessage->GetPreviousRequest(IMessage::PAGEMESSAGE_SEND);

    if (piMessage == IMS_NULL)
    {
        // It will send 400 Error response, In Operator Specific MtsMessage
        return IMS_FALSE;
    }

    IMSList<IMessageBodyPart*> objMessageBodies = piMessage->GetBodyParts();

    if (objMessageBodies.IsEmpty())
    {
        // It will send 400 Error response, In Operator Specific MtsMessage
        return IMS_FALSE;
    }

    if (m_pMtsDynamicLoader == IMS_NULL)
    {
        IMS_TRACE_E(0, "m_pMtsDynamicLoader is null", 0, 0, 0);
        return IMS_FALSE;
    }

    MtsSipFormUtils* pMtsSipFormUtils = m_pMtsDynamicLoader->GetMtsSipFormUtils();
    AString strContentType = objMessageBodies.GetAt(0)->GetHeader(AString("Content-Type"));
    IMS_UINT32 nContentSMSType = pMtsSipFormUtils->FormContentTypeStrToEnum(strContentType);

    if (MtsApp::SMSFORMAT_INVALID == nContentSMSType)
    {
        // It will send 415 Error response, In Operator Specific MtsMessage
        return IMS_FALSE;
    }

    ByteArray objSms = objMessageBodies.GetAt(0)->GetContent();

    if (nContentSMSType == MtsApp::SMSFORMAT_3GPP)
    {
        IMS_SINT32 nGsmMti =
                m_pMtsDynamicLoader->GetMtsSmUtils()->GetMti(MtsApp::SMSFORMAT_3GPP, objSms);

        if (nGsmMti == MtsSmUtils::MTS_3GPP_MTI_RP_DATA_From_N ||
                nGsmMti == MtsSmUtils::MTS_3GPP_MTI_RP_ACK_From_N ||
                nGsmMti == MtsSmUtils::MTS_3GPP_MTI_RP_ERROR_From_N)
        {
            return IMS_TRUE;
        }
    }

    return IMS_FALSE;
}

PUBLIC ICoreService* MtsMessageController::GetICoreService()
{
    if (m_pMtsService == IMS_NULL)
    {
        IMS_TRACE_E(0, "m_piMtsService is NULL", 0, 0, 0);
        return IMS_NULL;
    }

    return m_pMtsService->GetICoreService();
}

PUBLIC MtsDynamicLoader* MtsMessageController::GetMtsUtils()
{
    return m_pMtsDynamicLoader;
}

PUBLIC void MtsMessageController::SetCallStateType(IN IMS_UINT32 nType, IN IMS_UINT32 nState)
{
    IMS_TRACE_I("SetCallStateType() - nType = [%d], nState = [%d]", nType, nState, 0);
    m_nCallTypeMsg = nType;
    m_nCallStateMsg = nState;
}

PUBLIC IMS_BOOL MtsMessageController::IsEmergencyCalling()
{
    IMS_TRACE_I("IsEmergencyCalling() - m_nCallTypeMsg=[%d], m_nCallStateMsg=[%d]", m_nCallTypeMsg,
            m_nCallStateMsg, 0);
    if ((m_nCallTypeMsg == TYPE_EMERGENCY) && (m_nCallStateMsg != STATE_IDLE))
    {
        IMS_TRACE_I("MtsMessageController::IsEmergencyCalling, Now is Under E911-Call", 0, 0, 0);
        return IMS_TRUE;
    }

    IMS_TRACE_I("MtsMessageController::IsEmergencyCalling, Now is NOT Under E911-Call", 0, 0, 0);
    return IMS_FALSE;
}

PUBLIC IMS_RESULT MtsMessageController::ReportMoStatus(IN IMS_UINT32 nReason,
        IN IMS_UINT32 nSmsformat, IN IMS_UINT8 nRetryAfter /* = 0 */,
        IN IMS_SINT32 nSeqId /* = -1 */)
{
    IMS_CHAR acLog[128 + 1] = { 0, };
    IMS_Sprintf(acLog, 128, "reason (%s, %d) , SMS Format (%s) , nSeqId (%d)",
            (MO_SUCCESS == nReason) ? "success" : "failure", nReason,
            (MtsApp::SMSFORMAT_3GPP == nSmsformat) ? "3GPP" : "3GPP2", nSeqId);

    IMS_TRACE_I("ReportMoStatus ::  %s", acLog, 0, 0);

    m_pMtsService->ReportMoStatus(nReason, nSmsformat, nRetryAfter, nSeqId);

    return IMS_SUCCESS;
}

PUBLIC IMS_UINT32 MtsMessageController::ReportMtSms(IN IMS_UINT32 nSmsFormat,
        IN IMS_UINT32 nSmsLength, IN const IMS_BYTE* pbySmsData)
{
    IMS_TRACE_I("ReportMtSMS() - SMS Format(%s) Length(%d)",
            (MtsApp::SMSFORMAT_3GPP == nSmsFormat) ? "3GPP" : "3GPP2", nSmsLength, 0);

    AString strData = AString::ConstNull();
    strData.Attach(reinterpret_cast<const IMS_CHAR*>(pbySmsData), nSmsLength);
    ByteArray objData = strData.ToBase64();

    m_pMtsService->ReportMtSms(nSmsFormat, objData);

    // TODO: Call back is being considered

    return MT_SUCCESS;
}

PUBLIC void MtsMessageController::ReportTransmissionResult(IN IMS_UINT32 nResponse,
        IN IMS_UINT32 nSmsType, IN IMS_SINT32 nSeqId /*= -1*/)
{
    IMS_TRACE_I("ReportTransmissionResult() - nResponse(%d) nSmsType(%d)", nResponse, nSmsType, 0);

    IMS_UINT32 nResultCode = MO_INVALID;

    if ((nResponse == SipStatusCode::SC_200) || (nResponse == SipStatusCode::SC_202))
    {
        IMS_TRACE_I("Reporting SUCCESS, nResponse is %d", nResponse, 0, 0);

        nResultCode = MO_SUCCESS;
    }

    ReportMoStatus(nResultCode, nSmsType, 0, nSeqId);
}

PUBLIC void MtsMessageController::ReportTransmissionFailureWithRetryTime(
        IN const IMS_UINT32 nSmsType, IN const IMS_UINT8 nRetryTime, IN IMS_SINT32 nSeqId /*= -1*/)
{
    IMS_UINT32 nResultCode = MO_IMS_TEMP_FAILURE;
    IMS_TRACE_I("ReportTransmissionFailureWithRetryTime() - resultCode=%d, smsType=%d, "
            "retryTime=%d", nResultCode, nSmsType, nRetryTime);

    ReportMoStatus(nResultCode, nSmsType, nRetryTime, nSeqId);
}

PUBLIC VIRTUAL void MtsMessageController::NotifyMoSms(IN IMS_UINT32 nSmsFormat, IN const ByteArray& objData,
        IN const AString& strAddress, IN IMS_SINT32 nSeqId)
{
    IMS_TRACE_I("NotifyMoSms()", 0, 0, 0);

    SendMtsMessage(nSmsFormat, objData, strAddress, nSeqId, IMS_FALSE);
}

PUBLIC VIRTUAL void MtsMessageController::NotifyMtSms(IN IPageMessage* piMessage)
{
    IMS_TRACE_I("NotifyMtSms()", 0, 0, 0);

    ReceiveMtsMessage(piMessage, IMS_FALSE);
}

PROTECTED IMS_BOOL MtsMessageController::OnMessage(IN IMSMSG& /*objMSG*/)
{
    IMS_TRACE_I("MtsMessageController::OnMessage ", 0, 0, 0);

    for (IMS_UINT32 i = 0; i < m_objMsgList.GetSize(); i++)
    {
        IMtsMessage* piMtsMessage = m_objMsgList.GetAt(i);

        if (piMtsMessage == IMS_NULL)
        {
            IMS_TRACE_E(0, "IMtsMessage is null", 0, 0, 0);
            continue;
        }

        if (IsDeliverMessage(piMtsMessage->GetPageMessage()))
        {
            piMtsMessage->Retry_MtsMessageInPending();
            break;
        }
    }

    return IMS_TRUE;
}

PRIVATE void MtsMessageController::ReceiveMtsMessage(
        IN IPageMessage* piPageMessage, IN IMS_BOOL bIsSmsEServiceType)
{
    IMS_TRACE_I("ReceiveMtsMessage() - bIsSmsEServiceType: %d", bIsSmsEServiceType, 0, 0);

    if (m_pMtsDynamicLoader == IMS_NULL)
    {
        IMS_TRACE_E(0, "m_pMtsDynamicLoader is null", 0, 0, 0);
        piPageMessage->Reject(SipStatusCode::SC_480, 0);
        piPageMessage->Destroy();
        return;
    }

    MtsServiceState* pMtsServiceState = m_pMtsDynamicLoader->GetMtsServiceState();

    if ((pMtsServiceState == IMS_NULL) ||
            ((pMtsServiceState != IMS_NULL) && (pMtsServiceState->IsMtServiceBlocked())))
    {
        IMS_TRACE_E(0, "Mts is NOTREADY STATE", 0, 0, 0);

        // 480 Temporarily Unavailable.
        piPageMessage->Reject(SipStatusCode::SC_480, 0);
        piPageMessage->Destroy();
        return;
    }

    ICoreService* pMtsICoreService = m_pMtsService->GetICoreService();
    AString strImpu;

    if (pMtsICoreService != IMS_NULL)
    {
        strImpu = pMtsICoreService->GetAuthorizedUserId().GetUri();
    }
    else
    {
        IMS_TRACE_E(0, "Failed to Get Mts CoreService!!", 0, 0, 0);

        piPageMessage->Reject(SipStatusCode::SC_480, 0);
        piPageMessage->Destroy();
        return;
    }

    IMS_TRACE_I("ReceiveMtsMessage: call CreateMtsMessage with bIsSmsEServiceType(%d)",
            bIsSmsEServiceType, 0, 0);
    IMtsMessage* piMtsMessage = new MtsMessage(m_nSlotId, this, bIsSmsEServiceType);
    UpdateRPAckMap(piPageMessage);
    piMtsMessage->ReceiveMessage(piPageMessage, strImpu);
}

PRIVATE void MtsMessageController::SendMtsMessage(IN IMS_UINT32 nSmsFormat,
        IN const ByteArray& objData, IN const AString& strAddress, IN IMS_SINT32 nSeqId,
        IN IMS_BOOL bIsSmsEServiceType)
{
    if (strAddress.GetLength() == 0)
    {
        IMS_TRACE_I("Target address is invalid", 0, 0, 0);
        ReportTransmissionResult(MO_IMS_PERM_FAILURE, nSmsFormat, nSeqId);
        return;
    }

    if (objData.GetLength() == 0)
    {
        IMS_TRACE_E(0, "Sending SMS bin Size zero", 0, 0, 0);
        ReportTransmissionResult(MO_IMS_PERM_FAILURE, nSmsFormat, nSeqId);
        return;
    }

    if (m_pMtsDynamicLoader == IMS_NULL)
    {
        IMS_TRACE_E(0, "m_pMtsDynamicLoader is null", 0, 0, 0);
        ReportTransmissionResult(MO_IMS_PERM_FAILURE, nSmsFormat, nSeqId);
        return;
    }

    MtsServiceState* pMtsServiceState = m_pMtsDynamicLoader->GetMtsServiceState();

    if (pMtsServiceState == IMS_NULL)
    {
        IMS_TRACE_E(0, "MtsServiceState is null", 0, 0, 0);
        ReportTransmissionResult(MO_IMS_PERM_FAILURE, nSmsFormat, nSeqId);
        return;
    }

    if (pMtsServiceState->IsMoServiceBlocked())
    {
        IMS_TRACE_E(0, "Mts is not READY STATE ", 0, 0, 0);
        ReportTransmissionResult(MO_IMS_PERM_FAILURE, nSmsFormat, nSeqId);
        return;
    }

    if (pMtsServiceState->IsTemporaryServiceBlocked())
    {
        IMS_TRACE_E(0, "Mts service is temporarily blocked", 0, 0, 0);
        ReportTransmissionResult(MO_IMS_LIMITEDSMSSVCREGI, nSmsFormat);
        return;
    }

    ICoreService* pMtsICoreService = m_pMtsService->GetICoreService();

    if (pMtsICoreService == IMS_NULL)
    {
        IMS_TRACE_E(0, "Fail to get MtsICoreService instance ", 0, 0, 0);
        ReportTransmissionResult(MO_IMS_PERM_FAILURE, nSmsFormat, nSeqId);
        return;
    }

    IMS_TRACE_I("SendMtsMessage() - nSmsFormat(%s), nSeqId(%d), bIsSmsEServiceType(%d)",
            (nSmsFormat == 1) ? "3GPP" : "3GPP2", nSeqId, bIsSmsEServiceType);

    IMS_BOOL bIsGsmAckorError = IMS_FALSE;
    AString strLastIpSmgw;
    AString strDestination;
    IMS_SINT32 nGsmMti = MtsSmUtils::MTS_SMS_MTI_NONE;

    if (nSmsFormat == MtsApp::SMSFORMAT_3GPP)
    {
        nGsmMti = m_pMtsDynamicLoader->GetMtsSmUtils()->GetMti(MtsApp::SMSFORMAT_3GPP, objData);
        bIsGsmAckorError = (nGsmMti == MtsSmUtils::MTS_3GPP_MTI_RP_ACK_From_MS ||
                nGsmMti == MtsSmUtils::MTS_3GPP_MTI_RP_ERROR_From_MS) ? IMS_TRUE : IMS_FALSE;

        if (bIsGsmAckorError)
        {
            IMtsMessage* pMtsMessage =
                    Search(m_pMtsDynamicLoader->GetMtsSmUtils()->GetRpMr(objData));

            if (pMtsMessage != IMS_NULL)
            {
                strLastIpSmgw = pMtsMessage->GetDestination();
            }
            else
            {
                if (nGsmMti == MtsSmUtils::MTS_3GPP_MTI_RP_ERROR_From_MS)
                {
                    // this logic should be applied only RP-ERROR case of response for RP-ACK
                    IMS_TRACE_E(0, "MtsMessage is null; MTI=%d, but report success", nGsmMti, 0, 0);
                    ReportTransmissionResult(SipStatusCode::SC_200, nSmsFormat, nSeqId);
                    return;
                }
                else
                {
                    IMS_TRACE_E(0, "MtsMessage is null; MTI=%d", nGsmMti, 0, 0);
                    ReportTransmissionResult(MO_IMS_PERM_FAILURE, nSmsFormat, nSeqId);
                    return;
                }
            }
        }

        if (nGsmMti == MtsSmUtils::MTS_3GPP_MTI_RP_SMMA)
        {
            strLastIpSmgw = GetLastIpsmgwAddr();
            bIsGsmAckorError = IMS_TRUE;
        }
    }

    if (IMS_FALSE == m_pMtsDynamicLoader->GetMtsSipFormUtils()->FormDestination(
            strAddress, bIsGsmAckorError, strLastIpSmgw, strDestination))
    {
        IMS_TRACE_E(0, "Failed to form the destination!!", 0, 0, 0);
        ReportTransmissionResult(MO_IMS_PERM_FAILURE, nSmsFormat, nSeqId);
        return;
    }

    /*
     * we can not send a message when there is a sending message
     * of which the destination address is the same.
     */
    AString strImpu = pMtsICoreService->GetAuthorizedUserId().GetUri();

    if (strImpu.GetLength() == 0)
    {
        IMS_TRACE_E(0, "Failed to form basic information for sending sms!!", 0, 0, 0);
        ReportTransmissionResult(MO_IMS_PERM_FAILURE, nSmsFormat, nSeqId);
        return;
    }

    IMtsMessage* piMtsMessage = Search(strDestination, nGsmMti);
    IMS_TRACE_D("strDestination : [%s]", strDestination.GetStr(), 0, 0);

    if (piMtsMessage != IMS_NULL)
    {
        IMS_BOOL bRpAcked = IMS_FALSE;
        IMS_TRACE_I("piMtsMessage != IMS_NULL", 0, 0, 0);

        for (IMS_UINT32 i = 0; i < m_objRPAckedMsgs.GetSize(); i++)
        {
            if (m_objRPAckedMsgs.GetAt(i) == piMtsMessage)
            {
                IMS_TRACE_I("RP_ACK has been received so keep sending the new one", 0, 0, 0);
                bRpAcked = IMS_TRUE;
                break;
            }
        }

        if (bRpAcked == IMS_FALSE)
        {
            IMS_TRACE_E(
                    0, "there's a sending sms message with the same destination address", 0, 0, 0);
            piMtsMessage->PrintMsgInfo();
            ReportTransmissionResult(MO_IMS_TEMP_FAILURE, nSmsFormat, nSeqId);
            return;
        }
    }

    IMS_TRACE_I("SendMtsMessage() - call CreateMtsMessage with bIsSmsEServiceType", 0, 0, 0);
    IPageMessage* piPageMessage = pMtsICoreService->CreatePageMessage(strImpu, strDestination);
    piMtsMessage = new MtsMessage(m_nSlotId, this, bIsSmsEServiceType);
    piMtsMessage->SetSeqId(nSeqId);
    piMtsMessage->SendMessage(piPageMessage, strDestination, nSmsFormat, objData);
}

PRIVATE void MtsMessageController::UpdateRPAckMap(IN IPageMessage* pIPageMessage)
{
    if (m_objMsgList.GetSize() == 0)
    {
        return;
    }

    // received page message
    IMessage* pICurrentMessage = pIPageMessage != IMS_NULL
            ? pIPageMessage->GetPreviousRequest(IMessage::PAGEMESSAGE_SEND)
            : IMS_NULL;
    ISipMessage* pICurrentSIPMessage =
            pICurrentMessage != IMS_NULL ? pICurrentMessage->GetMessage() : IMS_NULL;
    AString strInReplyTo = pICurrentSIPMessage != IMS_NULL
            ? pICurrentSIPMessage->GetHeader(ISipHeader::UNKNOWN, 0, SipHeaderName::IN_REPLY_TO)
            : AString::ConstEmpty();

    if (strInReplyTo.GetLength() <= 0)
    {
        return;
    }

    for (IMS_UINT32 i = 0; i < m_objMsgList.GetSize(); i++)
    {
        if (m_objMsgList.GetAt(i)->IsReceivedMessage() == IMS_TRUE)
        {
            continue;
        }

        // sent page message
        IPageMessage* piTargetPageMessage = m_objMsgList.GetAt(i)->GetPageMessage();
        IMessage* pITargetMessage =
                piTargetPageMessage->GetPreviousRequest(IMessage::PAGEMESSAGE_SEND);
        ISipMessage* pITargetSIPMessage =
                pITargetMessage != IMS_NULL ? pITargetMessage->GetMessage() : IMS_NULL;
        AString strCallId = pITargetSIPMessage != IMS_NULL
                ? pITargetSIPMessage->GetHeader(ISipHeader::CALL_ID)
                : AString::ConstEmpty();

        if (strCallId.GetLength() <= 0)
        {
            continue;
        }

        if (strCallId.EqualsIgnoreCase(strInReplyTo))
        {
            m_objRPAckedMsgs.Append(m_objMsgList.GetAt(i));
            return;
        }
    }
}
