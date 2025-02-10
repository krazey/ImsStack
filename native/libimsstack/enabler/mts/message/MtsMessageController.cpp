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

#include "ByteArray.h"
#include "CarrierConfig.h"
#include "Configuration.h"
#include "Engine.h"
#include "GeolocationHelper.h"
#include "GeolocationPidfCreator.h"
#include "ServiceConfig.h"
#include "ServiceMessage.h"
#include "ServiceTimer.h"
#include "ServiceTrace.h"
#include "SipAddress.h"
#include "SipHeaderName.h"
#include "SipParsingHelper.h"
#include "SipStatusCode.h"
#include "ICoreService.h"
#include "IMessage.h"
#include "ISipMessage.h"
#include "ISipHeader.h"
#include "IPageMessage.h"
#include "IMessageBodyPart.h"
#include "IuMtsService.h"
#include "MtsStringDef.h"
#include "MtsService.h"
#include "MtsServiceState.h"
#include "helper/MtsTransactionTimerUpdateHelper.h"
#include "message/IMtsMessage.h"
#include "message/MtsErrorHandler.h"
#include "message/MtsMessage.h"
#include "message/MtsMessageController.h"
#include "utility/MtsDynamicLoader.h"

__IMS_TRACE_TAG_COM_MTS__;

PUBLIC
MtsMessageController::MtsMessageController(IN IMS_SINT32 nSlotId, IN IMtsService* piMtsService,
        IN MtsDynamicLoader* pMtsDynamicLoader) :
        m_objMsgList(ImsList<IMtsMessage*>()),
        m_bProcessingMsg(IMS_FALSE),
        m_nSlotId(nSlotId),
        m_strLastRcvIpsmgwAddr(AString::ConstNull()),
        m_piMtsService(piMtsService),
        m_piMtsErrorHandler(IMS_NULL),
        m_pMtsDynamicLoader(pMtsDynamicLoader),
        m_piRetryAfterTimer(IMS_NULL),
        m_objRetryFunction(),
        m_pRetryContent(IMS_NULL),
        m_objTimerUpdateHelper(ConfigService::GetConfigService()->GetCarrierConfig(m_nSlotId),
                Engine::GetConfiguration()->GetSipConfig(m_nSlotId))
{
    IMS_TRACE_I("+MtsMessageController [slot_%d]", m_nSlotId, 0, 0);

    m_piMtsService->SetListener(this);
    m_piMtsErrorHandler =
            new MtsErrorHandler(ConfigService::GetConfigService()->GetCarrierConfig(m_nSlotId));
}

PUBLIC MtsMessageController::~MtsMessageController()
{
    IMS_TRACE_I("~MtsMessageController [slot_%d]", m_nSlotId, 0, 0);

    DestroyMtsMessage();
    StopRetryAfterTimer();
    m_objRetryFunction = {};
    delete m_piMtsErrorHandler;
}

PUBLIC
void MtsMessageController::PageMessageDelivered(IN IPageMessage* piPageMessage)
{
    IMS_TRACE_I("PageMessageDelivered", 0, 0, 0);

    IMtsMessage* piMtsMessage = Search(piPageMessage);
    if (piMtsMessage == IMS_NULL)
    {
        IMS_TRACE_E(0, "No IMtsMessage matching IPageMessage in the Message list", 0, 0, 0);
        TerminateAllMessages();
        CleanRetryContent();
        return;
    }

    IMessage* piMessage = piPageMessage->GetPreviousResponse(IMessage::PAGEMESSAGE_SEND);
    if (piMessage == IMS_NULL)
    {
        IMS_TRACE_E(0, "No received responses in the page message", 0, 0, 0);
        ReportTransmissionResult(
                SipStatusCode::SC_200, piMtsMessage->GetSmsFormat(), piMtsMessage->GetSeqId());
        CleanMtsMessage(piMtsMessage);
        CleanRetryContent();
        return;
    }

    // report success send results.
    ReportTransmissionResult(
            piMessage->GetStatusCode(), piMtsMessage->GetSmsFormat(), piMtsMessage->GetSeqId());

    IMS_SINT32 nMti = piMtsMessage->GetMti();
    if (nMti == SMS_3GPP_MTI_RP_ACK_FROM_MS || nMti == SMS_3GPP_MTI_RP_ERROR_FROM_MS)
    {
        IMS_TRACE_I("PageMessageDelivered : Response recv for ACK( Del Report / Error )", 0, 0, 0);
        m_bProcessingMsg = IMS_FALSE;
        CleanMtsMessageWithRpMr(piMtsMessage->GetMessageReference());
    }

    // remove this MtsMessage, so that MtsService send any following sms messages.
    CleanMtsMessage(piMtsMessage);
    CleanRetryContent();
    return;
}

PUBLIC
void MtsMessageController::PageMessageDeliveryFailed(IN IPageMessage* piPageMessage)
{
    IMS_TRACE_I("PageMessageDeliveryFailed", 0, 0, 0);
    IMtsMessage* piMtsMessage = Search(piPageMessage);
    if (piMtsMessage == IMS_NULL)
    {
        IMS_TRACE_E(0, "No IMtsMessage matching IPageMessage in the Message list", 0, 0, 0);
        TerminateAllMessages();
        CleanRetryContent();
        return;
    }

    IMessage* piMessage = piPageMessage->GetPreviousResponse(IMessage::PAGEMESSAGE_SEND);
    IMS_SINT32 nResult =
            m_piMtsErrorHandler->Handle(m_piMtsService, m_pMtsDynamicLoader, piMessage);
    if (nResult == MO_ERROR_BY_RETRY_AFTER)
    {
        StartRetryAfterTimer(m_piMtsErrorHandler->GetRetryAfterValue());
        CleanMtsMessage(piMtsMessage);
        return;
    }

    // report failure send results.
    ReportTransmissionResult(nResult, piMtsMessage->GetSmsFormat(), piMtsMessage->GetSeqId());
    m_piMtsErrorHandler->ResetRetryAfterStatus();

    if (piMessage == IMS_NULL)
    {
        IMS_TRACE_E(0, "No received responses in the page message", 0, 0, 0);
        CleanMtsMessage(piMtsMessage);
        CleanRetryContent();
        return;
    }

    IMS_SINT32 nMti = piMtsMessage->GetMti();
    if (nMti == SMS_3GPP_MTI_RP_ACK_FROM_MS || nMti == SMS_3GPP_MTI_RP_ERROR_FROM_MS)
    {
        m_bProcessingMsg = IMS_FALSE;
        CleanMtsMessageWithRpMr(piMtsMessage->GetMessageReference());
    }

    // remove this MtsMessage, so that MtsService send any following sms messages.
    CleanMtsMessage(piMtsMessage);
    CleanRetryContent();
    return;
}

PUBLIC void MtsMessageController::NotifyMoSms(IN SmsFormatType eSmsFormat, IN ByteArray* pContent,
        IN const AString& strAddress, IN IMS_SINT32 nSeqId, IN IMS_BOOL bEmergency)
{
    IMS_TRACE_I("NotifyMoSms", 0, 0, 0);

    if (SendMtsMessage(eSmsFormat, pContent, strAddress, nSeqId, bEmergency) == IMS_FAILURE)
    {
        delete pContent;
    }
}

PUBLIC void MtsMessageController::NotifyMtSms(IN IPageMessage* piPageMessage)
{
    IMS_TRACE_I("NotifyMtSms", 0, 0, 0);

    ReceiveMtsMessage(piPageMessage, IMS_FALSE);
}

PUBLIC void MtsMessageController::OnServiceDisconnected()
{
    IMS_TRACE_I("OnDisconnected", 0, 0, 0);

    TerminateAllMessages();
}

PUBLIC void MtsMessageController::OnServiceSuspended()
{
    IMS_TRACE_I("OnSuspended", 0, 0, 0);

    TerminateAllMessages();
}

PUBLIC
void MtsMessageController::Timer_TimerExpired(IN ITimer* piTimer)
{
    IMS_TRACE_I("Timer_TimerExpired", 0, 0, 0);

    if (piTimer == IMS_NULL)
    {
        return;
    }
    else if (piTimer == m_piRetryAfterTimer)
    {
        if (m_objRetryFunction != nullptr)
        {
            m_objRetryFunction();
        }

        piTimer->KillTimer();
        TimerService::GetTimerService()->DestroyTimer(piTimer);
        m_piRetryAfterTimer = IMS_NULL;
    }
    else
    {
        IMS_TRACE_I("Timer_TimerExpired : can't find the expired timer", 0, 0, 0);
        return;
    }
}

PROTECTED IMS_BOOL MtsMessageController::OnMessage(IN ImsMessage& /*objMsg*/)
{
    IMS_TRACE_I("OnMessage", 0, 0, 0);

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
            Retry_MtsMessageInPending(piMtsMessage);
            break;
        }
    }

    return IMS_TRUE;
}

PRIVATE void MtsMessageController::DestroyMtsMessage()
{
    IMS_UINT32 nMsgSize = m_objMsgList.GetSize();

    IMS_TRACE_I("DestroyMtsMessage : nMsgSize[%d], m_nSlotId[%d]", nMsgSize, m_nSlotId, 0);

    if (nMsgSize == 0)
    {
        IMS_TRACE_I("objMsgList Size is 0", 0, 0, 0);
        return;
    }

    for (IMS_UINT32 index = 0; index < nMsgSize; index++)
    {
        IMtsMessage* pDeleteMessage = m_objMsgList.GetAt(index);

        if (pDeleteMessage != IMS_NULL)
        {
            delete pDeleteMessage;
        }
    }
    m_objMsgList.Clear();
}

PRIVATE void MtsMessageController::Add(IN IMtsMessage* piMtsMessage)
{
    m_objMsgList.Append(piMtsMessage);

    IMS_TRACE_I("Add : messageCount(after)[%d]", m_objMsgList.GetSize(), 0, 0);
}

PRIVATE void MtsMessageController::Remove(IN const IMtsMessage* piMtsMessage)
{
    IMS_BOOL bHasDeliverMsg = IMS_FALSE;

    for (IMS_UINT32 i = 0; i < m_objMsgList.GetSize(); i++)
    {
        IMtsMessage* piTmpMtsMessage = m_objMsgList.GetAt(i);
        if (piTmpMtsMessage == piMtsMessage)
        {
            m_objMsgList.RemoveAt(i);
            break;
        }
    }

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

    IMS_TRACE_I("Remove : messageCount[%d], hasDeliverMsg[%s]", m_objMsgList.GetSize(),
            _TRACE_B_(bHasDeliverMsg), 0);
}

PRIVATE IMtsMessage* MtsMessageController::Search(IN const IPageMessage* piPageMessage)
{
    if (piPageMessage == IMS_NULL)
    {
        return IMS_NULL;
    }

    IMS_TRACE_I("Search : messageCount[%d]", m_objMsgList.GetSize(), 0, 0);

    for (IMS_UINT32 i = 0; i < m_objMsgList.GetSize(); i++)
    {
        IMtsMessage* piTmpMtsMessage = m_objMsgList.GetAt(i);

        if (IsReceivedMessage(piTmpMtsMessage))
        {
            continue;
        }

        if (piTmpMtsMessage->GetPageMessage() == piPageMessage)
        {
            return piTmpMtsMessage;
        }
    }

    return IMS_NULL;
}

PRIVATE IMtsMessage* MtsMessageController::Search(
        IN const AString& strDestination, IN IMS_SINT32 nMti)
{
    if (strDestination.GetLength() == 0)
    {
        return IMS_NULL;
    }

    if (nMti != SMS_3GPP_MTI_RP_DATA_FROM_MS && nMti != SMS_MTI_NONE)
    {
        return IMS_NULL;
    }

    IMS_TRACE_I("Search : messageCount[%d], size[%d]", m_objMsgList.GetSize(),
            strDestination.GetLength(), 0);

    for (IMS_UINT32 i = 0; i < m_objMsgList.GetSize(); i++)
    {
        IMtsMessage* piTmpMtsMessage = m_objMsgList.GetAt(i);

        if (IsReceivedMessage(piTmpMtsMessage))
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

PRIVATE IMtsMessage* MtsMessageController::Search(IN IMS_SINT32 nMessageReference,
        IN MtsTransactionType eMessageType /* = MESSAGE_TYPE_RECEIVE*/)
{
    IMS_TRACE_I("Search : messageCount[%d]", m_objMsgList.GetSize(), 0, 0);

    for (IMS_UINT32 i = 0; i < m_objMsgList.GetSize(); i++)
    {
        IMtsMessage* piMtsMessage = m_objMsgList.GetAt(i);
        IMS_BOOL bIsReceived = IsReceivedMessage(piMtsMessage);

        if ((eMessageType == MtsTransactionType::MESSAGE_TYPE_RECEIVE && bIsReceived != IMS_TRUE) ||
                (eMessageType == MtsTransactionType::MESSAGE_TYPE_SEND && bIsReceived == IMS_TRUE))
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

PRIVATE void MtsMessageController::ReceiveMtsMessage(
        IN IPageMessage* piPageMessage, IN IMS_BOOL bEmergency)
{
    IMS_TRACE_I("ReceiveMtsMessage : bEmergency[%s]", _TRACE_B_(bEmergency), 0, 0);

    if (m_piMtsService->GetIMtsServiceState()->IsMtServiceBlocked())
    {
        IMS_TRACE_E(0, "Mts is NOTREADY STATE", 0, 0, 0);

        // 480 Temporarily Unavailable.
        piPageMessage->Reject(SipStatusCode::SC_480, 0);
        piPageMessage->Destroy();
        return;
    }

    ICoreService* pMtsICoreService = m_piMtsService->GetICoreService(bEmergency);

    if (pMtsICoreService == IMS_NULL)
    {
        IMS_TRACE_E(0, "Failed to Get Mts CoreService!!", 0, 0, 0);

        piPageMessage->Reject(SipStatusCode::SC_480, 0);
        piPageMessage->Destroy();
        return;
    }

    // When SIP MESSAGE(RP-ACK) is received before SIP 202 Accepted,
    // remove the MtsMessage(RP-DATA) for further MO SMS request.
    CleanMtsMessageWithInReplyTo(piPageMessage);

    IMtsMessage* piMtsMessage = new MtsMessage(m_nSlotId);
    AString strImpu = pMtsICoreService->GetAuthorizedUserId().GetUri();
    piMtsMessage->SetImpu(strImpu);

    const ByteArray& objContent = ProcessReceivedMessage(piPageMessage, piMtsMessage);
    if (objContent.IsNull())
    {
        delete piMtsMessage;
        return;
    }

    if ((piMtsMessage->GetSmsFormat() == SmsFormatType::SMSFORMAT_3GPP) &&
            (piMtsMessage->GetMti() == SMS_3GPP_MTI_RP_DATA_FROM_N))
    {
        if (m_bProcessingMsg)
        {
            IMS_TRACE_E(0, "Previous SMS_DELIVER is not processed yet Hence Adding to the List", 0,
                    0, 0);
            Add(piMtsMessage);
            return;
        }
        m_bProcessingMsg = IMS_TRUE;
    }

    ReportMtSms(piMtsMessage->GetSmsFormat(), objContent.GetLength(), objContent.GetData());
    RespondReceivedMessage(piPageMessage, piMtsMessage, IMS_TRUE);

    IMS_TRACE_I("ReceiveMessage : SMS is received successfully", 0, 0, 0);
}

PRIVATE IMS_RESULT MtsMessageController::SendMtsMessage(IN SmsFormatType eSmsFormat,
        IN ByteArray* pContent, IN const AString& strAddress, IN IMS_SINT32 nSeqId,
        IN IMS_BOOL bEmergency)
{
    IMS_TRACE_I("SendMtsMessage : eSmsFormat[%s], nSeqId[%d], bEmergency[%s]",
            PS_SmsFormatType(eSmsFormat), nSeqId, _TRACE_B_(bEmergency));

    if (strAddress.GetLength() == 0 || pContent->IsNull())
    {
        IMS_TRACE_E(0, "invalid input param", 0, 0, 0);
        ReportTransmissionResult(MO_ERROR_GENERIC, eSmsFormat, nSeqId);
        return IMS_FAILURE;
    }

    if (m_piMtsService->GetIMtsServiceState()->IsMoServiceBlocked())
    {
        IMS_TRACE_E(0, "Mts is not READY STATE ", 0, 0, 0);
        ReportTransmissionResult(MO_ERROR_GENERIC, eSmsFormat, nSeqId);
        return IMS_FAILURE;
    }

    AString strDestination;
    if (FormDestinationByMti(eSmsFormat, *pContent, strAddress, nSeqId, strDestination) ==
            IMS_FALSE)
    {
        IMS_TRACE_E(0, "Failed to form the destination!!", 0, 0, 0);
        // Does not need to report the transmission result here
        return IMS_FAILURE;
    }

    ICoreService* pMtsICoreService = GetICoreService(bEmergency);
    if (pMtsICoreService == IMS_NULL)
    {
        IMS_TRACE_E(0, "Fail to get MtsICoreService instance ", 0, 0, 0);
        ReportTransmissionResult(MO_ERROR_GENERIC, eSmsFormat, nSeqId);
        return IMS_FAILURE;
    }

    AString strImpu = pMtsICoreService->GetAuthorizedUserId().GetUri();
    if (strImpu.GetLength() == 0)
    {
        IMS_TRACE_E(0, "Failed to form basic information for sending sms!!", 0, 0, 0);
        ReportTransmissionResult(MO_ERROR_GENERIC, eSmsFormat, nSeqId);
        return IMS_FAILURE;
    }

    IMS_SINT32 nGsmMti = SMS_MTI_NONE;
    if (eSmsFormat == SmsFormatType::SMSFORMAT_3GPP)
    {
        nGsmMti = m_pMtsDynamicLoader->GetMtsSmUtils()->GetMti(
                SmsFormatType::SMSFORMAT_3GPP, *pContent);
    }

    /*
     * we can not send a message when there is a sending message
     * of which the destination address is the same.
     */
    IMtsMessage* piMtsMessage = Search(strDestination, nGsmMti);
    if (piMtsMessage != IMS_NULL)
    {
        IMS_TRACE_E(0, "there's a sending sms message with the same destination address", 0, 0, 0);
        piMtsMessage->PrintInfo();
        ReportTransmissionResult(MO_ERROR_RETRY, eSmsFormat, nSeqId);
        return IMS_FAILURE;
    }

    IPageMessage* piPageMessage = pMtsICoreService->CreatePageMessage(strImpu, strDestination);
    if (piPageMessage == IMS_NULL)
    {
        IMS_TRACE_E(0, "piPageMessage is null", 0, 0, 0);
        ReportTransmissionResult(MO_ERROR_GENERIC, eSmsFormat, nSeqId);
        delete piMtsMessage;
        return IMS_FAILURE;
    }

    piMtsMessage = new MtsMessage(m_nSlotId);
    piMtsMessage->SetSeqId(nSeqId);
    IMessage* piMessage = piPageMessage->GetNextRequest();
    if (ConstructSendMessage(piMessage, *pContent, eSmsFormat, bEmergency) == IMS_FALSE)
    {
        ReportTransmissionResult(MO_ERROR_GENERIC, eSmsFormat, nSeqId);
        delete piMtsMessage;
        return IMS_FAILURE;
    }

    // We sends a SMS message giving it SMS data burst and the Content-Type.
    m_objTimerUpdateHelper.SetMessageTransactionTimer(nGsmMti);

    if (piPageMessage->Send(*pContent,
                m_pMtsDynamicLoader->GetMtsSipFormUtils()->FormContentTypeEnumToStr(eSmsFormat)) ==
            IMS_FAILURE)
    {
        IMS_TRACE_E(0, "Failed to send a IPageMessage", 0, 0, 0);
        m_objTimerUpdateHelper.ResetMessageTransactionTimer(nGsmMti);
        ReportTransmissionResult(MO_ERROR_RETRY, eSmsFormat, nSeqId);
        delete piMtsMessage;
        return IMS_FAILURE;
    }

    IMS_TRACE_I("SendMtsMessage : SMS is sent successfully", 0, 0, 0);
    m_objTimerUpdateHelper.ResetMessageTransactionTimer(nGsmMti);

    /*
     * Register itself as the listener of iPageMessage events,
     * so that it receives the result of sending.
     */
    piPageMessage->SetListener(this);

    /*
     * add this MtsMessage object,
     * so that MtsService can detect the error case of sending multiple sms msgs at a time.
     */
    Add(piMtsMessage);

    m_pRetryContent = pContent;
    m_objRetryFunction = [=, this]()
    {
        NotifyMoSms(eSmsFormat, pContent, strAddress, nSeqId, bEmergency);
    };

    SetMessageInfo(piPageMessage, *pContent, eSmsFormat, strDestination,
            MtsTransactionType::MESSAGE_TYPE_SEND, piMtsMessage);
    piMtsMessage->PrintInfo();

    return IMS_SUCCESS;
}

PRIVATE void MtsMessageController::ReportMoStatus(
        IN IMS_SINT32 nReason, IN SmsFormatType eSmsFormat, IN IMS_SINT32 nSeqId /* = -1 */)
{
    IMS_CHAR acLog[128 + 1] = {
            0,
    };
    IMS_Sprintf(acLog, 128, "reason (%s, %d) , SMS Format (%s) , nSeqId (%d)",
            PS_MoStatus(nReason), nReason, PS_SmsFormatType(eSmsFormat), nSeqId);
    IMS_TRACE_I("ReportMoStatus :  %s", acLog, 0, 0);

    m_piMtsService->ReportMoStatus(nReason, eSmsFormat, nSeqId);
}

PRIVATE void MtsMessageController::ReportMtSms(
        IN SmsFormatType eSmsFormat, IN IMS_UINT32 nContentLength, IN const IMS_BYTE* pbyContent)
{
    IMS_TRACE_I("ReportMtSMS : eSmsFormat[%s], Length[%d]", PS_SmsFormatType(eSmsFormat),
            nContentLength, 0);

    AString strContent = AString::ConstNull();
    strContent.Attach(reinterpret_cast<const IMS_CHAR*>(pbyContent), nContentLength);
    ByteArray objContent = strContent.ToBase64();

    m_piMtsService->ReportMtSms(eSmsFormat, objContent);
}

PRIVATE
IMS_BOOL MtsMessageController::ConstructSendMessage(IN IMessage* piMessage,
        IN const ByteArray& objContent, IN SmsFormatType eSmsFormat, IN IMS_BOOL bEmergency)
{
    if (piMessage == IMS_NULL)
    {
        IMS_TRACE_E(0, "Failed to create an object of IMessage", 0, 0, 0);
        return IMS_FALSE;
    }

    // TODO: Add SIP Header for MESSAGE Method here.

    if (piMessage->AddHeader(AString("Request-Disposition"), AString("no-fork")) == IMS_FAILURE)
    {
        IMS_TRACE_E(0, "Failed to add the Request-Disposition header", 0, 0, 0);
        return IMS_FALSE;
    }

    if (eSmsFormat == SmsFormatType::SMSFORMAT_3GPP)
    {
        IMS_SINT32 nMti = m_pMtsDynamicLoader->GetMtsSmUtils()->GetMti(
                SmsFormatType::SMSFORMAT_3GPP, objContent);
        if (nMti == SMS_3GPP_MTI_RP_ACK_FROM_MS || nMti == SMS_3GPP_MTI_RP_ERROR_FROM_MS)
        {
            IMtsMessage* piMtsMessage =
                    Search(m_pMtsDynamicLoader->GetMtsSmUtils()->GetRpMr(objContent));
            AString strCallId = GetPreviousCallId(piMtsMessage);
            // Set the Call-ID in the In-Reply-To header
            if (strCallId.GetLength())
            {
                piMessage->AddHeader("In-Reply-To", strCallId);
            }
        }
    }

    if (bEmergency)
    {
        ICarrierConfig* piCc = ConfigService::GetConfigService()->GetCarrierConfig(m_nSlotId);
        IMS_BOOL bSupportPidf = piCc->GetBoolean(
                CarrierConfig::ImsSms::KEY_SMS_GEOLOCATION_PIDF_FOR_EMERGENCY_BOOL);

        if (bSupportPidf)
        {
            SetLocationToMessage(piMessage);
        }
    }

    return IMS_TRUE;
}

PRIVATE
IMS_BOOL MtsMessageController::FormDestinationByMti(IN SmsFormatType eSmsFormat,
        IN const ByteArray& objContent, IN const AString& strAddress, IN IMS_SINT32 nSeqId,
        OUT AString& strDestination)
{
    IMS_BOOL bIsAckOrError = IMS_FALSE;
    AString strLastIpSmgw;

    if (eSmsFormat == SmsFormatType::SMSFORMAT_3GPP)
    {
        IMS_SINT32 nGsmMti = m_pMtsDynamicLoader->GetMtsSmUtils()->GetMti(
                SmsFormatType::SMSFORMAT_3GPP, objContent);
        bIsAckOrError =
                (nGsmMti == SMS_3GPP_MTI_RP_ACK_FROM_MS || nGsmMti == SMS_3GPP_MTI_RP_ERROR_FROM_MS)
                ? IMS_TRUE
                : IMS_FALSE;

        if (bIsAckOrError)
        {
            IMtsMessage* pMtsMessage =
                    Search(m_pMtsDynamicLoader->GetMtsSmUtils()->GetRpMr(objContent));

            if (pMtsMessage != IMS_NULL)
            {
                strLastIpSmgw = pMtsMessage->GetDestination();
            }
            else
            {
                if (nGsmMti == SMS_3GPP_MTI_RP_ERROR_FROM_MS)
                {
                    // this logic should be applied only RP-ERROR case of response for RP-ACK
                    IMS_TRACE_E(
                            0, "MtsMessage is null; MTI[%d], but report success", nGsmMti, 0, 0);
                    ReportTransmissionResult(SipStatusCode::SC_200, eSmsFormat, nSeqId);
                    return IMS_FALSE;
                }
                else
                {
                    IMS_TRACE_E(0, "MtsMessage is null; MTI[%d]", nGsmMti, 0, 0);
                    ReportTransmissionResult(MO_ERROR_GENERIC, eSmsFormat, nSeqId);
                    return IMS_FALSE;
                }
            }
        }

        if (nGsmMti == SMS_3GPP_MTI_RP_SMMA)
        {
            strLastIpSmgw = GetLastIpsmgwAddr();
            bIsAckOrError = IMS_TRUE;
        }
    }

    if (m_pMtsDynamicLoader->GetMtsSipFormUtils()->FormDestination(
            strAddress, bIsAckOrError, strLastIpSmgw, strDestination) == IMS_FALSE)
    {
        IMS_TRACE_E(0, "Failed to form the destination!!", 0, 0, 0);
        ReportTransmissionResult(MO_ERROR_GENERIC, eSmsFormat, nSeqId);
        return IMS_FALSE;
    }

    return IMS_TRUE;
}

PRIVATE
const ByteArray& MtsMessageController::ProcessReceivedMessage(
        IN IPageMessage* piPageMessage, IN IMtsMessage* piMtsMessage)
{
    AString strTempSmsgw = AString::ConstNull();

    IMessage* piMessage = piPageMessage->GetPreviousRequest(IMessage::PAGEMESSAGE_SEND);

    if (piMessage == IMS_NULL)
    {
        IMS_TRACE_E(0, "Failed to fetch IMessage from IPageMessage", 0, 0, 0);

        piPageMessage->Reject(400, 0);

        return ByteArray::ConstNull();
    }

    ImsList<AString> objSipToHeaderList = piMessage->GetHeaders("To");

    if (objSipToHeaderList.GetSize() <= 0)
    {
        IMS_TRACE_E(0, "To header is not found in IMessage!!", 0, 0, 0);

        piPageMessage->Reject(400, 0);

        return ByteArray::ConstNull();
    }

    ImsList<IMessageBodyPart*> objMessageBodies = piMessage->GetBodyParts();

    if (0 == objMessageBodies.GetSize())
    {
        IMS_TRACE_E(0, "No body exists", 0, 0, 0);

        piPageMessage->Reject(400, 0);

        return ByteArray::ConstNull();
    }

    /*
     * We only support 3GPP SMS or 3GPP2 SMS according to Verizon specification.
     * If a received SMS is not one of these, we response a SIP failure final response.
     */
    AString strContentType = objMessageBodies.GetAt(0)->GetHeader(AString("Content-Type"));

    SmsFormatType eContentSmsType =
            m_pMtsDynamicLoader->GetMtsSipFormUtils()->FormContentTypeStrToEnum(strContentType);

    if (eContentSmsType == SmsFormatType::SMSFORMAT_INVALID)
    {
        piPageMessage->Reject(415, 0);

        return ByteArray::ConstNull();
    }

    if (GetSmsgwFromReceivedMessage(piPageMessage, strTempSmsgw) == IMS_TRUE)
    {
        SetLastIpsmgwAddr(strTempSmsgw);
    }

    const ByteArray& objContent = objMessageBodies.GetAt(0)->GetContent();
    if (objContent.GetLength() > 255)  // WMS_MAX_LEN
    {
        IMS_TRACE_E(0, "Too Large RPDU(%d)", objContent.GetLength(), 0, 0);

        return ByteArray::ConstNull();
    }

    SetMessageInfo(piPageMessage, objContent, eContentSmsType, strTempSmsgw,
            MtsTransactionType::MESSAGE_TYPE_RECEIVE, piMtsMessage);
    piMtsMessage->PrintInfo();

    return objContent;
}

PRIVATE void MtsMessageController::ReportTransmissionResult(
        IN IMS_SINT32 nResponse, IN SmsFormatType eSmsFormat, IN IMS_SINT32 nSeqId /*= -1*/)
{
    IMS_TRACE_I("ReportTransmissionResult : nResponse[%d] eSmsFormat[%s]", nResponse,
            PS_SmsFormatType(eSmsFormat), 0);

    IMS_SINT32 nResultCode = MO_INVALID;

    switch (nResponse)
    {
        case SipStatusCode::SC_200:
        case SipStatusCode::SC_202:
            nResultCode = MO_SUCCESS;
            break;

        case MO_ERROR_GENERIC:
            nResultCode = MO_ERROR_GENERIC;
            break;

        case MO_ERROR_FALLBACK:
            nResultCode = MO_ERROR_FALLBACK;
            break;

        default:
            nResultCode = MO_ERROR_RETRY;
            break;
    }

    ReportMoStatus(nResultCode, eSmsFormat, nSeqId);
}

PRIVATE
void MtsMessageController::RespondReceivedMessage(
        IN IPageMessage* piPageMessage, IN IMtsMessage* piMtsMessage, IN IMS_BOOL bAdded)
{
    piPageMessage->Accept();
    if (piMtsMessage->GetMti() == SMS_3GPP_MTI_RP_DATA_FROM_N)
    {
        IMS_TRACE_I("RespondReceivedMessage : bAdded[%s]", _TRACE_B_(bAdded), 0, 0);
        if (bAdded)
        {
            Add(piMtsMessage);
        }
    }
    else
    {
        delete piMtsMessage;
    }
}

PRIVATE
void MtsMessageController::Retry_MtsMessageInPending(IN IMtsMessage* piMtsMessage)
{
    if (m_bProcessingMsg)
    {
        return;
    }
    m_bProcessingMsg = IMS_TRUE;

    IPageMessage* piPageMessage = piMtsMessage->GetPageMessage();
    SmsFormatType eSmsFormat = piMtsMessage->GetSmsFormat();
    const ByteArray& objContent = ProcessReceivedMessage(piPageMessage, piMtsMessage);
    ReportMtSms(eSmsFormat, objContent.GetLength(), objContent.GetData());
    RespondReceivedMessage(piPageMessage, piMtsMessage, IMS_FALSE);
}

PRIVATE
void MtsMessageController::CleanMtsMessage(IMtsMessage* piMtsMessage)
{
    Remove(piMtsMessage);
    delete piMtsMessage;
}

PRIVATE
void MtsMessageController::CleanMtsMessageWithRpMr(IMS_SINT32 nMrOfRp)
{
    IMS_TRACE_D("CleanMtsMessageWithRpMr : nMrOfRp[%d]", nMrOfRp, 0, 0);
    IMtsMessage* piMtsMessage = Search(nMrOfRp);

    if (piMtsMessage != IMS_NULL)
    {
        Remove(piMtsMessage);
        delete piMtsMessage;
    }
    else
    {
        IMS_TRACE_E(0, "piMtsMessage is null", 0, 0, 0);
    }
}

PRIVATE void MtsMessageController::CleanMtsMessageWithInReplyTo(IN IPageMessage* piPageMessage)
{
    if (m_objMsgList.GetSize() == 0)
    {
        return;
    }

    // received page message
    IMessage* piCurrentMessage = piPageMessage != IMS_NULL
            ? piPageMessage->GetPreviousRequest(IMessage::PAGEMESSAGE_SEND)
            : IMS_NULL;
    ISipMessage* piCurrentSipMessage =
            piCurrentMessage != IMS_NULL ? piCurrentMessage->GetMessage() : IMS_NULL;
    AString strInReplyTo = piCurrentSipMessage != IMS_NULL
            ? piCurrentSipMessage->GetHeader(ISipHeader::IN_REPLY_TO)
            : AString::ConstEmpty();

    if (strInReplyTo.GetLength() <= 0)
    {
        return;
    }

    for (IMS_UINT32 i = 0; i < m_objMsgList.GetSize(); i++)
    {
        IMtsMessage* piMtsMessage = m_objMsgList.GetAt(i);
        if (IsReceivedMessage(piMtsMessage) == IMS_TRUE)
        {
            continue;
        }

        // sent page message
        IPageMessage* piTargetPageMessage = piMtsMessage->GetPageMessage();
        IMessage* piTargetMessage =
                piTargetPageMessage->GetPreviousRequest(IMessage::PAGEMESSAGE_SEND);
        ISipMessage* piTargetSipMessage =
                piTargetMessage != IMS_NULL ? piTargetMessage->GetMessage() : IMS_NULL;
        AString strCallId = piTargetSipMessage != IMS_NULL
                ? piTargetSipMessage->GetHeader(ISipHeader::CALL_ID)
                : AString::ConstEmpty();

        if (strCallId.EqualsIgnoreCase(strInReplyTo))
        {
            Remove(piMtsMessage);
            delete piMtsMessage;
            return;
        }
    }
}

PRIVATE
void MtsMessageController::CleanRetryContent()
{
    delete m_pRetryContent;
    m_pRetryContent = IMS_NULL;
}

PRIVATE
void MtsMessageController::TerminateAllMessages()
{
    IMS_TRACE_I("TerminateAllMessages : messageCount[%d]", m_objMsgList.GetSize(), 0, 0);

    if (m_objMsgList.IsEmpty() == IMS_TRUE)
    {
        IMS_TRACE_I("Msg Size is 0", 0, 0, 0);
        return;
    }

    for (IMS_UINT32 index = 0; index < m_objMsgList.GetSize(); index++)
    {
        IMtsMessage* piMtsMessage = m_objMsgList.GetAt(index);
        if (piMtsMessage != IMS_NULL)
        {
            TerminateMessage(piMtsMessage);
        }
    }
    m_objMsgList.Clear();

    m_bProcessingMsg = IMS_FALSE;
}

PRIVATE
void MtsMessageController::TerminateMessage(IN IMtsMessage* piMtsMessage)
{
    IMS_TRACE_I("TerminateMessage : Destroy MtsMessage", 0, 0, 0);

    if (piMtsMessage->GetTransactionType() == MtsTransactionType::MESSAGE_TYPE_SEND)
    {
        // if it's a sending mo message, then report the mo status reason as MO_ERROR_GENERIC
        ReportTransmissionResult(
                MO_ERROR_GENERIC, piMtsMessage->GetSmsFormat(), piMtsMessage->GetSeqId());
    }

    delete piMtsMessage;
}

PRIVATE const AString& MtsMessageController::GetLastIpsmgwAddr()
{
    if (m_strLastRcvIpsmgwAddr.GetLength() <= 0)
    {
        IMS_TRACE_E(0, "m_strLastRcvIpsmgwAddr is empty", 0, 0, 0);
        return AString::ConstNull();
    }

    IMS_TRACE_D(
            "GetLastIpsmgwAddr : Get Last IPSMGW Addr [%s]", m_strLastRcvIpsmgwAddr.GetStr(), 0, 0);

    return m_strLastRcvIpsmgwAddr;
}

PRIVATE void MtsMessageController::SetLastIpsmgwAddr(IN const AString& strSmgwAddr)
{
    if (strSmgwAddr.GetLength() <= 0)
    {
        IMS_TRACE_E(0, "Last IPSMGW address is empty", 0, 0, 0);
        return;
    }

    IMS_TRACE_D("SetLastIpsmgwAddr : Set Last IPSMGW Address [%s]", strSmgwAddr.GetStr(), 0, 0);
    m_strLastRcvIpsmgwAddr = strSmgwAddr;
}

PRIVATE void MtsMessageController::SetLocationToMessage(IN IMessage* piMessage)
{
    IMS_TRACE_I("SetLocationToMessage", 0, 0, 0);

    GeolocationPidfCreator* pPidfCreator =
            GeolocationHelper::GetInstance()->GetPidfCreator(m_nSlotId);

    if (pPidfCreator == IMS_NULL)
    {
        return;
    }

    ByteArray objContent;

    if (!pPidfCreator->CreateWithPositionAndCountry(AString::ConstNull(), objContent))
    {
        IMS_TRACE_I("SetLocationToMessage : Creating a location information failed", 0, 0, 0);
        return;
    }

    ISipMessageBodyPart* piBodyPart = piMessage->GetMessage()->CreateBodyPart();

    // Set a Location
    piBodyPart->SetContent(objContent);

    // Set a Location Content-Type
    piBodyPart->SetHeader(ISipMessageBodyPart::CONTENT_TYPE, "application/pidf+xml");

    AString strNewContentId = GeolocationHelper::CreateContentId(m_nSlotId);

    // Set a Location Content-ID
    AString strContentId;
    strContentId.Sprintf("<%s>", strNewContentId.GetStr());
    piBodyPart->SetHeader(ISipMessageBodyPart::CONTENT_ID, strContentId);

    // Set the Content-Length header
    AString strClen;
    strClen.SetNumber(objContent.GetLength());
    piBodyPart->SetHeader(
            ISipMessageBodyPart::CONTENT_UNKNOWN, strClen, SipHeaderName::CONTENT_LENGTH);

    // Set Geolocation header
    AString strGeolocation;
    strGeolocation.Sprintf("<cid:%s>", strNewContentId.GetStr());
    piMessage->AddHeader(SipHeaderName::GEOLOCATION, strGeolocation.GetStr());
}

PRIVATE
ICoreService* MtsMessageController::GetICoreService(IN IMS_BOOL bEmergency) const
{
    if (bEmergency &&
            ConfigService::GetConfigService()->GetCarrierConfig(m_nSlotId)->GetBoolean(
                    CarrierConfig::KEY_SUPPORT_EMERGENCY_SMS_OVER_IMS_BOOL))
    {
        return m_piMtsService->GetICoreService(IMS_TRUE);
    }
    else
    {
        return m_piMtsService->GetICoreService(IMS_FALSE);
    }
}

PRIVATE
AString MtsMessageController::GetPreviousCallId(IN const IMtsMessage* piMtsMessage) const
{
    AString strCallId;
    if (piMtsMessage != IMS_NULL)
    {
        IPageMessage* piPageMessage = piMtsMessage->GetPageMessage();
        if (piPageMessage != IMS_NULL)
        {
            IMessage* piPrevMessage = piPageMessage->GetPreviousRequest(IMessage::PAGEMESSAGE_SEND);

            ImsList<AString> objCallId = piPrevMessage->GetHeaders("Call-ID");
            if (!objCallId.IsEmpty())
            {
                strCallId = objCallId.GetAt(0);
                IMS_TRACE_D("GetPreviousCallId : Call-ID[%s]", strCallId.GetStr(), 0, 0);
            }
        }
    }
    return strCallId;
}

PRIVATE
IMS_BOOL MtsMessageController::GetSmsgwFromReceivedMessage(
        IN const IPageMessage* piPageMessage, OUT AString& strSmsgw)
{
    if (piPageMessage == IMS_NULL)
    {
        return IMS_FALSE;
    }

    ImsList<AString> objPAssrtIdHdrList = piPageMessage->GetRemoteUserId();

    if (!objPAssrtIdHdrList.IsEmpty())
    {
        strSmsgw = objPAssrtIdHdrList.GetAt(0);

        IMS_TRACE_D("GetSmsgwFromReceivedMessage : P-Asserted-Identity for SMS-GW is %s",
                strSmsgw.GetStr(), 0, 0);

        if (strSmsgw.GetLength() != 0)
        {
            return IMS_TRUE;
        }
    }

    IMS_TRACE_I("GetSmsgwFromReceivedMessage : P-Asserted-Identity is not included, then the From "
                "header is used for SMS-GW",
            0, 0, 0);

    IMessage* piMessage = piPageMessage->GetPreviousRequest(IMessage::PAGEMESSAGE_SEND);

    if (piMessage == IMS_NULL)
    {
        IMS_TRACE_E(0, "Failed to fetch IMessage from IPageMessage, Here it sends 400 response", 0,
                0, 0);
        return IMS_FALSE;
    }

    ImsList<AString> objFromHdrList = piMessage->GetHeaders(SipHeaderName::FROM);

    if (objFromHdrList.IsEmpty())
    {
        IMS_TRACE_E(0, "From header is not found in IMessage!!", 0, 0, 0);
        return IMS_FALSE;
    }

    GetUriFromHeaders(objFromHdrList.GetAt(0), strSmsgw);

    if (strSmsgw.GetLength() == 0)
    {
        IMS_TRACE_E(0, "Failed to get SIP address from the From header!!", 0, 0, 0);
        return IMS_FALSE;
    }

    IMS_TRACE_I(
            "GetSmsgwFromReceivedMessage : From header for SMS-GW is %s", strSmsgw.GetStr(), 0, 0);

    return IMS_TRUE;
}

PRIVATE
void MtsMessageController::GetUriFromHeaders(IN const AString& strFromHdr, OUT AString& strUri)
{
    SipAddress objSIPAddress;

    if (!objSIPAddress.Create(strFromHdr))
    {
        IMS_TRACE_E(0, "Creating SipAddress(From:%s) failed", strFromHdr.GetStr(), 0, 0);
        return;
    }

    strUri = objSIPAddress.ToString();

    IMS_TRACE_I("GetUriFromHeaders : strUri[%s]", strUri.GetStr(), 0, 0);
}

PRIVATE IMS_BOOL MtsMessageController::IsDeliverMessage(IN IPageMessage* piPageMessage)
{
    IMS_TRACE_I("IsDeliverMessage", 0, 0, 0);

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

    ImsList<IMessageBodyPart*> objMessageBodies = piMessage->GetBodyParts();

    if (objMessageBodies.IsEmpty())
    {
        // It will send 400 Error response, In Operator Specific MtsMessage
        return IMS_FALSE;
    }

    AString strContentType = objMessageBodies.GetAt(0)->GetHeader(AString("Content-Type"));
    SmsFormatType nContentSmsType =
            m_pMtsDynamicLoader->GetMtsSipFormUtils()->FormContentTypeStrToEnum(strContentType);

    if (SmsFormatType::SMSFORMAT_INVALID == nContentSmsType)
    {
        // It will send 415 Error response, In Operator Specific MtsMessage
        return IMS_FALSE;
    }

    ByteArray objContent = objMessageBodies.GetAt(0)->GetContent();

    if (nContentSmsType == SmsFormatType::SMSFORMAT_3GPP)
    {
        IMS_SINT32 nGsmMti = m_pMtsDynamicLoader->GetMtsSmUtils()->GetMti(
                SmsFormatType::SMSFORMAT_3GPP, objContent);

        if (nGsmMti == SMS_3GPP_MTI_RP_DATA_FROM_N || nGsmMti == SMS_3GPP_MTI_RP_ACK_FROM_N ||
                nGsmMti == SMS_3GPP_MTI_RP_ERROR_FROM_N)
        {
            return IMS_TRUE;
        }
    }

    return IMS_FALSE;
}

PRIVATE
IMS_BOOL MtsMessageController::IsReceivedMessage(IN IMtsMessage* piMtsMessage)
{
    return (piMtsMessage->GetTransactionType() == MtsTransactionType::MESSAGE_TYPE_RECEIVE);
}

PRIVATE
void MtsMessageController::SetMessageInfo(IN IPageMessage* piPageMessage,
        IN const ByteArray& objContent, IN SmsFormatType eSmsFormat,
        IN const AString& strDestination, IN MtsTransactionType eMessageType,
        OUT IMtsMessage* piMtsMessage)
{
    // Mark SMS Msg Info
    piMtsMessage->SetPageMessage(piPageMessage);
    piMtsMessage->SetTransactionType(eMessageType);
    piMtsMessage->SetSmSize(objContent.GetLength());
    piMtsMessage->SetDestination(strDestination);

    MtsSmUtils* pMtsSmUtils = m_pMtsDynamicLoader->GetMtsSmUtils();
    if (eSmsFormat == SmsFormatType::SMSFORMAT_3GPP)
    {
        piMtsMessage->SetSmsFormat(SmsFormatType::SMSFORMAT_3GPP);
        piMtsMessage->SetMti(pMtsSmUtils->GetMti(SmsFormatType::SMSFORMAT_3GPP, objContent));
        piMtsMessage->SetMessageReference(pMtsSmUtils->GetRpMr(objContent));
    }
    else if (eSmsFormat == SmsFormatType::SMSFORMAT_3GPP2)
    {
        piMtsMessage->SetSmsFormat(SmsFormatType::SMSFORMAT_3GPP2);
        piMtsMessage->SetMti(pMtsSmUtils->GetMti(SmsFormatType::SMSFORMAT_3GPP2, objContent));
    }
}

PRIVATE
void MtsMessageController::StartRetryAfterTimer(IN IMS_SINT32 nRetryAfterValue)
{
    IMS_TRACE_I("StartRetryAfterTimer : Duration[%d]", nRetryAfterValue, 0, 0);

    if (m_piRetryAfterTimer != IMS_NULL)
    {
        m_piRetryAfterTimer->KillTimer();
        TimerService::GetTimerService()->DestroyTimer(m_piRetryAfterTimer);
    }

    m_piRetryAfterTimer = TimerService::GetTimerService()->CreateTimer();
    m_piRetryAfterTimer->SetTimer(nRetryAfterValue * 1000, this);
}

PRIVATE
void MtsMessageController::StopRetryAfterTimer()
{
    IMS_TRACE_I("StopRetryAfterTimer", 0, 0, 0);

    if (m_piRetryAfterTimer != IMS_NULL)
    {
        m_piRetryAfterTimer->KillTimer();
        TimerService::GetTimerService()->DestroyTimer(m_piRetryAfterTimer);
        m_piRetryAfterTimer = IMS_NULL;
    }
}
