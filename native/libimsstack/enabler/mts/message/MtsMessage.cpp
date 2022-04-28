#include "ServiceTrace.h"
#include "ServicePhoneInfo.h"

#include "IMSStrLib.h"
#include "ImsAos.h"
#include "IImsAos.h"
#include "ISIPHeader.h"
#include "SIPAddress.h"
#include "SIPParsingHelper.h"
#include "SIPStatusCode.h"
#include "SIPHeaderName.h"
#include "SystemConfig.h"
#include "IMessage.h"
#include "IMessageBodyPart.h"
#include "IPageMessage.h"
#include "IMtsMessage.h"
#include "IUMts.h"
#include "MtsClient.h"
#include "utility/MtsDynamicLoader.h"
#include "message/MtsMessageController.h"
#include "utility/MtsSmUtils.h"
#include "utility/MtsSipFormUtils.h"
#include "message/MtsMessage.h"
#include "utility/MtsTrm.h"

__IMS_TRACE_TAG_COM_SMS__;

PUBLIC
MtsMessage::MtsMessage(IN IMS_SINT32 nSlotId, IN MtsMessageController* pMtsMessageController,
        IN IMS_BOOL bIsSmsEServiceType) :
        m_piPageMessage(IMS_NULL),
        m_strDestination(AString::ConstNull()),
        m_bIsSmsEServiceType(bIsSmsEServiceType),
        m_nSmsFormat(MtsSmUtils::MTS_SMS_FORMAT_INVALID),
        m_nMrOfRp(-1),
        m_nSmsTrxType(MtsSmUtils::MTS_SMS_TRX_TYPE_INVALID),
        m_nMti(MtsSmUtils::MTS_SMS_MTI_NONE),
        m_nSmSize(0),
        m_nSeqId(-1),
        m_strImpu(AString::ConstNull()),
        m_nSlotId(nSlotId),
        m_pMtsMessageController(pMtsMessageController)
{
    IMS_TRACE_I("+MtsMessage", 0, 0, 0);
    SetTrmInfo(nSlotId, IMS_TRUE);
}

PUBLIC
MtsMessage::~MtsMessage()
{
    IMS_TRACE_I("~MtsMessage", 0, 0, 0);

    if (m_piPageMessage != IMS_NULL)
    {
        m_piPageMessage->Destroy();
        m_piPageMessage = IMS_NULL;
    }
    else
    {
        IMS_TRACE_E(0, "IPageMessage is null", 0, 0, 0);
    }

    SetTrmInfo(m_nSlotId, IMS_FALSE);
}

PUBLIC
void MtsMessage::SendMessage(
        IN IPageMessage* piPageMessage,
        IN const AString& strDestination,
        IN const IMS_UINT32 nSmsType,
        IN const ByteArray& objSms)
{
    if (IMS_NULL == piPageMessage)
    {
        IMS_TRACE_E(0, "piPageMessage is null", 0, 0, 0);
        MtsClient_ReportTransmissionResult(MtsClient::MO_IMS_PERM_FAILURE, nSmsType);
        delete this;
        return;
    }

    m_piPageMessage = piPageMessage;
    IMessage* pIMessage = piPageMessage->GetNextRequest();
    if (IMS_FALSE == ConstructSendMessage(pIMessage, objSms, nSmsType))
    {
        delete this;
        return;
    }

    /*
     * We sends a SMS message giving it SMS data burst and the Content-Type.
     */
    MtsMessageController* pMtsMessageController = GetMtsMessageController();

    if (IMS_NULL == pMtsMessageController)
    {
        IMS_TRACE_E(0, "pMtsMessageController is null", 0, 0, 0);
        MtsClient_ReportTransmissionResult(MtsClient::MO_IMS_PERM_FAILURE, nSmsType);
        delete this;
        return;
    }

    MtsSipFormUtils* pMtsSipFormUtils = pMtsMessageController->GetMtsUtils()->GetMtsSipFormUtils();

    if (IMS_NULL == pMtsSipFormUtils)
    {
        IMS_TRACE_E(0, "pMtsSipFormUtils is null", 0, 0, 0);
        MtsClient_ReportTransmissionResult(MtsClient::MO_IMS_PERM_FAILURE, nSmsType);
        delete this;
        return;
    }

    if (IMS_FAILURE == piPageMessage->Send(objSms,
            pMtsSipFormUtils->FormContentTypeEnumToStr(nSmsType)))
    {
        IMS_TRACE_E(0, "Failed to send a IPageMessage", 0, 0, 0);
        MtsClient_ReportTransmissionResult(MtsClient::MO_IMS_TEMP_FAILURE, nSmsType);
        delete this;
        return;
    }

    IMS_TRACE_I("SMS is sent successfully", 0, 0, 0);

    /*
     * Register itself as the listener of iPageMessage events,
     * so that it receives the result of sending.
     */
    piPageMessage->SetListener(this);

    /*
     * add this MtsMessage object,
     * so that MtsService can detect the error case of sending multiple sms msgs at a time.
     */
    pMtsMessageController->Add(this);

    SetSendMsgInfo(objSms, nSmsType);
    SetDestination(strDestination);

    PrintMsgInfo();

    return;
}

PUBLIC
void MtsMessage::ReceiveMessage(IN IPageMessage* piPageMessage, IN const AString& strImpu)
{
    IMS_UINT32        nMtResult = 0;
    ByteArray         objSms = ByteArray::ConstNull();

    MtsMessageController* pMtsMessageController = GetMtsMessageController();

    if (IMS_NULL == pMtsMessageController)
    {
        IMS_TRACE_E(0, "pMtsMessageController is null", 0, 0, 0);
        delete this;
        return;
    }

    MtsClient* pMtsClient = MtsClient::GetInstance(m_nSlotId);
    MtsSmUtils* pMtsSmUtils = MtsSmUtils::GetInstance(m_nSlotId);

    m_strImpu = strImpu;

    if (Processing_ReceiveMessage(piPageMessage, strImpu, objSms) != IMS_TRUE)
    {
        return;
    }

    m_nMti = pMtsSmUtils->GetMti(MtsSmUtils::MTS_SMS_FORMAT_3GPP, objSms);
    if (m_nMti == MtsSmUtils::MTS_3GPP_MTI_RP_DATA_From_N && IsProcessingMtsMessage())
    {
        IMS_TRACE_E(0, "Prev SMS_DELIVER is not processed yet Hence Adding to the List", 0, 0, 0);
        pMtsMessageController->Add(this);
        return;
    }

    if (m_nMti == MtsSmUtils::MTS_3GPP_MTI_RP_DATA_From_N)
    {
        SetProcessingMtsMessage();
    }

    nMtResult = pMtsClient->PassReceivedMessage(objSms, m_nSmsFormat, m_nSlotId);

    if (Result_ReceiveMessage(piPageMessage, nMtResult, IMS_TRUE) != IMS_TRUE)
    {
        IMS_TRACE_E(0, "SMS isn't sent successfully", 0, 0, 0);
        ResetProcessingMtsMessage();
        delete this;
        return;
    }

    IMS_TRACE_I("SMS is sent successfully", 0, 0, 0);
}

PUBLIC
void MtsMessage::Retry_MtsMessageInPending()
{
    IMS_UINT32        nMtResult = 0;
    ByteArray         objSms = ByteArray::ConstNull();
    MtsClient* pMtsClient = MtsClient::GetInstance(m_nSlotId);

    if (IsProcessingMtsMessage())
    {
        return;
    }

    SetProcessingMtsMessage();

    Processing_ReceiveMessage(m_piPageMessage, m_strImpu, objSms);

    nMtResult = pMtsClient->PassReceivedMessage(objSms, m_nSmsFormat, m_nSlotId);
    if (Result_ReceiveMessage(m_piPageMessage, nMtResult, IMS_FALSE) != IMS_TRUE)
    {
        ResetProcessingMtsMessage();
        CleanMtsMessage();
    }
}

PUBLIC
IMS_BOOL MtsMessage::IsReceivedMessage()
{
    // Please call this function after set m_nSmsTrxType value by SendMessage() or ReceiveMessage().
    if (m_nSmsTrxType == MtsSmUtils::MTS_SMS_TRX_TYPE_INVALID)
    {
        IMS_TRACE_E(0, "SMS TRX type is invalid", 0, 0, 0);
    }

    return (m_nSmsTrxType == MtsSmUtils::MTS_SMS_TRX_TYPE_RECEIVE);
}

PUBLIC
AString& MtsMessage::GetDestination()
{
    return m_strDestination;
}

PUBLIC
IMS_SINT32 MtsMessage::GetMessageReference()
{
    return m_nMrOfRp;
}

PUBLIC
IMS_BOOL MtsMessage::IsProcessingMtsMessage()
{
    IMS_BOOL bReturn = IMS_FALSE;
    MtsMessageController* pMtsMessageController = GetMtsMessageController();

    if (pMtsMessageController != IMS_NULL)
    {
        bReturn = pMtsMessageController->m_bProcessingMsg;
    }
    return bReturn;
}

PUBLIC
void MtsMessage::SetProcessingMtsMessage()
{
    MtsMessageController* pMtsMessageController = GetMtsMessageController();

    if (pMtsMessageController != IMS_NULL)
    {
        pMtsMessageController->m_bProcessingMsg = IMS_TRUE;
    }
}

PUBLIC
void MtsMessage::ResetProcessingMtsMessage()
{
    MtsMessageController* pMtsMessageController = GetMtsMessageController();

    if (pMtsMessageController != IMS_NULL)
    {
        pMtsMessageController->m_bProcessingMsg = IMS_FALSE;
    }
}

PUBLIC
IPageMessage* MtsMessage::GetPageMessage()
{
    return m_piPageMessage;
}

PUBLIC
void MtsMessage::TerminateMessage(IN IMS_BOOL bIs1xCallTerm)
{
    IMS_TRACE_I("Destroy MtsMessage", 0, 0, 0);

    if (m_nSmsTrxType != MtsSmUtils::MTS_SMS_TRX_TYPE_RECEIVE)
    {
        // if it's a sending mo message, then report the permanent error to WMS.
        if (bIs1xCallTerm)
        {
            MtsClient_ReportTransmissionResult(MtsClient::MO_IMS_PERM_FAILURE, GetContentType());
        }
        else
        {
            MtsClient_ReportTransmissionResult(MtsClient::MO_IMS_TEMP_FAILURE, GetContentType());
        }
    }

    delete this;
}

PUBLIC
void MtsMessage::TerminateMessageEx(IN IMS_UINT32 nReason)
{
    IMS_TRACE_I("Destroy MtsMessage (Ex)", 0, 0, 0);

    if (m_nSmsTrxType != MtsSmUtils::MTS_SMS_TRX_TYPE_RECEIVE)
    {
        MtsClient_ReportTransmissionResult(nReason, GetContentType());
    }

    delete this;
}

PUBLIC
void MtsMessage::SetSeqId(IN IMS_SINT32 nSeqId)
{
    IMS_TRACE_I("MtsMessage::SetSeqId - m_nSeqId(%d)", nSeqId, 0, 0);
    m_nSeqId = nSeqId;
}

PUBLIC
void MtsMessage::PrintMsgInfo()
{
    if (m_nSmsFormat == MtsSmUtils::MTS_SMS_FORMAT_3GPP)
    {
        IMS_TRACE_I("[PrintMsgInfo] 3GPP :: (%s), RP_MR(%d), DataSize(%d)",
            MtsSmUtils::GetMtiStringFrom3gpp(m_nMti), m_nMrOfRp, m_nSmSize);
    }
    else if (m_nSmsFormat == MtsSmUtils::MTS_SMS_FORMAT_3GPP2)
    {
        IMS_TRACE_I("[PrintMsgInfo] 3GPP2 :: (%s), DataSize(%d)",
            MtsSmUtils::GetMtiStringFrom3gpp2(m_nMti), m_nSmSize, 0);
    }
    else
    {
        IMS_TRACE_E(0, "[PrintMsgInfo] SMS Format INFO INVALID", 0, 0, 0);
    }
}

PUBLIC
IMS_SINT32 MtsMessage::GetMti()
{
    return m_nMti;
}

PUBLIC
void MtsMessage::PageMessageDelivered(IN IPageMessage* piPageMessage)
{
    IMS_TRACE_I("MtsMessage::PageMessageDelivered", 0, 0, 0);
    if (IMS_NULL == piPageMessage)
    {
        IMS_TRACE_E(0, "Passed piPageMessage is null", 0, 0, 0);
        /*
         * even though it's internal operation fails,
         * it is sure that a successful response is received.
         */
        CleanMtsMessagewithReportResponse(SIPStatusCode::SC_200);
        return;
    }

    IMSList<IMessage*> objResponses = piPageMessage
            ->GetPreviousResponses(IMessage::PAGEMESSAGE_SEND);
    if (0 == objResponses.GetSize())
    {
        IMS_TRACE_E(0, "No received responses in the page message", 0, 0, 0);

        /*
         * even though it's internal operation fails,
         * it is sure that a successful response is received.
         */
        CleanMtsMessagewithReportResponse(SIPStatusCode::SC_200);
        return;
    }

    IMessage* piMessage = objResponses.GetAt(objResponses.GetSize() - 1);
    if (IMS_NULL == piMessage)
    {
        IMS_TRACE_E(0, "No received responses at the last index (%d) in the page message",
                objResponses.GetSize() - 1, 0, 0);
        CleanMtsMessagewithReportResponse(SIPStatusCode::SC_200);
        return;
    }

    // report success send results.
    MtsClient_ReportTransmissionResult(piMessage->GetStatusCode(), GetContentType());
    if (m_nMti == MtsSmUtils::MTS_3GPP_MTI_RP_ACK_From_MS ||
            m_nMti == MtsSmUtils::MTS_3GPP_MTI_RP_ERROR_From_MS)
    {
        IMS_TRACE_I("Response recv for ACK( Del Report / Error )", 0, 0, 0);
        ResetProcessingMtsMessage();

        CleanOperatorMtsMessage();
    }

    // remove this MtsMessage, so that MtsService send any following sms messages.
    CleanMtsMessage();
    return;

}

PUBLIC
void MtsMessage::PageMessageDeliveryFailed(IN IPageMessage* piPageMessage)
{
    IMS_TRACE_I("MtsMessage::PageMessageDeliveryFailed", 0, 0, 0);
    if (IMS_NULL == piPageMessage)
    {
        IMS_TRACE_E(0, "IMS Internal Error!! Passed piPageMessage is null!!", 0, 0, 0);
        // Here is any failure response is good enough, 480 Temporarily Unavailable is sufficient.
        DeliveryFailed_PageMessageNull();
        return;
    }

    IMSList<IMessage*> objResponses = piPageMessage->GetPreviousResponses(
            IMessage::PAGEMESSAGE_SEND);
    if (0 == objResponses.GetSize())
    {
        IMS_TRACE_E(0, "IMS Internal Error!! No received responses in the page message!!", 0, 0, 0);
        DeliveryFailed_TimerF();
        return;
    }

    IMessage* piMessage = objResponses.GetAt(objResponses.GetSize() - 1);
    if (IMS_NULL == piMessage)
    {
        IMS_TRACE_E(0, "No received responses at the last index (%d) in the page message!!",
            objResponses.GetSize() - 1, 0, 0);

        // Here is any failure response is good enough, 480 Temporarily Unavailable is sufficient.
        DeliveryFailed_MessageNull();
        return;
    }

    // report failure send results.
    if (IMS_FALSE == HandleDeliveryResponse(piMessage))
    {
        return;
    }

    if (m_nMti == MtsSmUtils::MTS_3GPP_MTI_RP_ACK_From_MS
            || m_nMti == MtsSmUtils::MTS_3GPP_MTI_RP_ERROR_From_MS)
    {
        ResetProcessingMtsMessage();
        //Remove DELIVER-MESSAGE;
        CleanOperatorMtsMessage();
    }

    // remove this MtsMessage, so that MtsService send any following sms messages.
    CleanMtsMessage();
    return;

}

PUBLIC
IMS_RESULT MtsMessage::MessageMediator_AdjustMessage(
        IN_OUT ISIPMessage* piSIPMsg, IN IMS_SINT32 nMessage)
{
    IMS_TRACE_D("MtsMessage::MessageMediator_AdjustMessage(), nMessage = %d", nMessage, 0, 0);
    if (piSIPMsg == IMS_NULL)
    {
        return IMS_FALSE;
    }
    return IMS_TRUE;
}

PUBLIC
IMS_BOOL MtsMessage::ConstructSendMessage(
        IN IMessage* piMessage, IN const ByteArray& objSms, IN const IMS_UINT32 nSmsType)
{
    if (IMS_NULL == piMessage)
    {
        IMS_TRACE_E(0, "Failed to create an object of IMessage", 0, 0, 0);
        MtsClient_ReportTransmissionResult(MtsClient::MO_IMS_PERM_FAILURE, nSmsType);
        return IMS_FALSE;
    }

    // TODO: Add SIP Header for MESSAGE Method here.

    if (IMS_FAILURE == piMessage->AddHeader(AString("Request-Disposition"), AString("no-fork")))
    {
        IMS_TRACE_E(0, "Failed to add the Request-Disposition header", 0, 0, 0 );
        MtsClient_ReportTransmissionResult(MtsClient::MO_IMS_PERM_FAILURE, nSmsType);
        return IMS_FALSE;
    }

    if (nSmsType == MtsSmUtils::MTS_SMS_FORMAT_3GPP)
    {
        MtsSmUtils* pMtsSmUtils = MtsSmUtils::GetInstance(MtsMessage::GetSlotId());
        m_nMti = pMtsSmUtils->GetMti(MtsSmUtils::MTS_SMS_FORMAT_3GPP, objSms);
        if (m_nMti == MtsSmUtils::MTS_3GPP_MTI_RP_ACK_From_MS ||
                m_nMti == MtsSmUtils::MTS_3GPP_MTI_RP_ERROR_From_MS)
        {
            AString strCallId = GetPreviousCallId(objSms);
            // Set the Call-ID in the In-Reply-To header
            if (strCallId.GetLength())
            {
                piMessage->AddHeader("In-Reply-To", strCallId);
            }
        }
    }

    return IMS_TRUE;
}

PUBLIC
AString MtsMessage::GetPreviousCallId(IN const ByteArray& objSms)
{
    AString strCallId;
    MtsSmUtils* pMtsSmUtils = MtsSmUtils::GetInstance(MtsMessage::GetSlotId());

    // Get an IMtsMessage object with the message reference
    MtsMessageController* pMtsMessageController = GetMtsMessageController();

    if (IMS_NULL == pMtsMessageController)
    {
        IMS_TRACE_E(0, "pMtsMessageController is null", 0, 0, 0);
        delete this;
        return IMS_NULL;
    }

    IMtsMessage* piMtsMessage = pMtsMessageController->Search(pMtsSmUtils->GetRpMr(objSms));

    if (piMtsMessage != IMS_NULL)
    {
        IPageMessage* piPageMessage = piMtsMessage->GetPageMessage();
        if (piPageMessage != IMS_NULL)
        {
            IMessage* piPrevMessage = piPageMessage->GetPreviousRequest(IMessage::PAGEMESSAGE_SEND);

            IMSList<AString> objCallId = piPrevMessage->GetHeaders("Call-ID");
            if (!objCallId .IsEmpty())
            {
                strCallId = objCallId .GetAt(0);
                IMS_TRACE_D(
                        "MtsMessage::GetPreviousCallId(), Call-ID = %s", strCallId.GetStr(), 0, 0);
            }
        }
    }
    return strCallId;
}

PUBLIC
void MtsMessage::SetSendMsgInfo(IN const ByteArray& objSms, IN const IMS_UINT32 nSmsType)
{
    // Mark SMS Msg Info
    m_nSmsTrxType = MtsSmUtils::MTS_SMS_TRX_TYPE_SEND;
    m_nSmSize = objSms.GetLength();

    MtsSmUtils* pMtsSmUtils = MtsSmUtils::GetInstance(MtsMessage::GetSlotId());
    if (nSmsType == MtsSmUtils::MTS_SMS_FORMAT_3GPP)
    {
        m_nSmsFormat = MtsSmUtils::MTS_SMS_FORMAT_3GPP;
        m_nMti = pMtsSmUtils->GetMti(MtsSmUtils::MTS_SMS_FORMAT_3GPP, objSms);
        m_nMrOfRp = pMtsSmUtils->GetRpMr(objSms);
    }
    else if (nSmsType == MtsSmUtils::MTS_SMS_FORMAT_3GPP2)
    {
        m_nSmsFormat = MtsSmUtils::MTS_SMS_FORMAT_3GPP2;
        m_nMti = pMtsSmUtils->GetMti(MtsSmUtils::MTS_SMS_FORMAT_3GPP2, objSms);
    }
}

PUBLIC
IMS_BOOL MtsMessage::HandleDeliveryResponse(IN IMessage* piMessage)
{
    IMS_UINT32 nResponse = piMessage->GetStatusCode();
    MtsClient_ReportTransmissionResult(nResponse, GetContentType());
    return IMS_TRUE;
}

PUBLIC
void MtsMessage::DeliveryFailed_PageMessageNull()
{
    // Here is any failure response is good enough, 480 Temporarily Unavailable is sufficient.
    CleanMtsMessagewithReportResponse(SIPStatusCode::SC_480);
}

PUBLIC
void MtsMessage::DeliveryFailed_TimerF()
{
    // Here is any failure response is good enough, 480 Temporarily Unavailable is sufficient.
    CleanMtsMessagewithReportResponse(SIPStatusCode::SC_480);
}

PUBLIC
void MtsMessage::DeliveryFailed_MessageNull()
{
    // Here is any failure response is good enough, 480 Temporarily Unavailable is sufficient.
    CleanMtsMessagewithReportResponse(SIPStatusCode::SC_480);
}

PROTECTED
IMS_BOOL MtsMessage::Result_ReceiveMessage(
        IN IPageMessage* piPageMessage, IMS_UINT32 nMtResult, IMS_BOOL bAdded)
{
    MtsMessageController* pMtsMessageController = GetMtsMessageController();;

    if (pMtsMessageController == IMS_NULL)
    {
        IMS_TRACE_E(0, "pMtsMessageController is invalid", 0, 0, 0);
        piPageMessage->Reject(480, 0);
        return IMS_FALSE;
    }

    switch (nMtResult)
    {
        case MtsClient::MT_SUCCESS :
        {
            IMS_TRACE_I("It has successfully delivered the received SMS to WMS", 0, 0, 0);

            piPageMessage->Accept();

            if (m_nMti == MtsSmUtils::MTS_3GPP_MTI_RP_DATA_From_N)
            {
                IMS_TRACE_I("bAdded = %s", _TRACE_B_(bAdded), 0, 0);
                if (bAdded)
                {
                    pMtsMessageController->Add(this);
                }
            }
            else
            {
                delete this;
            }

            return IMS_TRUE;
            break;
        }
        case MtsClient::MT_SMS_FORMAT_FAILURE :
        {
            IMS_TRACE_I("Failed to deliver the received SMS to WMS, SMS format mismatch", 0, 0, 0);

            piPageMessage->Reject(415, 0);

            return IMS_FALSE;
            break;
        }
        case MtsClient::MT_SMS_NODATA_FAILURE :
        {
            IMS_TRACE_I("Failed to deliver the received SMS to WMS, SMS data invalid", 0, 0, 0);

            piPageMessage->Reject(400, 0 );

            return IMS_FALSE;
            break;
        }
        case MtsClient::MT_FAILURE :
        {
            IMS_TRACE_I("Failed to deliver the received SMS to WMS, some error", 0, 0, 0);

            piPageMessage->Reject(480);

            return IMS_FALSE;
            break;
        }
        default :
        {
            IMS_TRACE_I("Failed to deliver the received SMS to WMS, some error", 0, 0, 0);

            piPageMessage->Reject(500);

            return IMS_FALSE;
            break;
        }
    }
}

PROTECTED
IMS_BOOL MtsMessage::Processing_ReceiveMessage(
        IN IPageMessage* piPageMessage, IN const AString& strImpu, ByteArray& objSms)
{
    IMSList<IMessageBodyPart*> objMessageBodies = IMSList<IMessageBodyPart*>();
    IMS_UINT32 nContentSmsType  = MtsSmUtils::MTS_SMS_FORMAT_INVALID;
    AString strTempSmsgw;

    (void)strImpu;

    MtsMessageController* pMtsMessageController = GetMtsMessageController();

    if (pMtsMessageController == IMS_NULL)
    {
        IMS_TRACE_E(0, "pMtsMessageController is invalid", 0, 0, 0);
        return IMS_FALSE;
    }

    MtsSipFormUtils* pMtsSipFormUtils = pMtsMessageController->GetMtsUtils()->GetMtsSipFormUtils();

    if (pMtsSipFormUtils == IMS_NULL)
    {
        IMS_TRACE_E(0, "pMtsSipFormUtils is invalid", 0, 0, 0);
        return IMS_FALSE;
    }

    MtsSmUtils* pMtsSmUtils = MtsSmUtils::GetInstance(MtsMessage::GetSlotId());

    if (IMS_NULL == piPageMessage)
    {
        IMS_TRACE_E(0, "piPageMessage_ is null", 0, 0, 0);

        delete this;
        return IMS_FALSE;
    }

    m_piPageMessage = piPageMessage;

    IMessage* piMessage = piPageMessage->GetPreviousRequest(IMessage::PAGEMESSAGE_SEND);

    if (IMS_NULL == piMessage)
    {
        IMS_TRACE_E(0,"Failed to fetch IMessage from IPageMessage", 0,0,0);

        piPageMessage->Reject(400, 0 );

        delete this;
        return IMS_FALSE;
    }

    IMSList<AString> objSipToHeaderList = piMessage->GetHeaders("To");

    if (objSipToHeaderList.GetSize() <= 0)
    {
        IMS_TRACE_E(0,"To header is not found in IMessage!!", 0,0,0);

        piPageMessage->Reject(400, 0 );

        delete this;
        return IMS_FALSE;
    }

    objMessageBodies = piMessage->GetBodyParts();

    if (0 == objMessageBodies.GetSize())
    {
        IMS_TRACE_E(0, "No body exists", 0, 0, 0 );

        piPageMessage->Reject(400, 0 );

        delete this;
        return IMS_FALSE;
    }

    /*
     * We only support 3GPP SMS or 3GPP2 SMS according to Verizon specification.
     * If a received SMS is not one of these, we response a SIP failure final response.
     */
    AString strContentType = objMessageBodies.GetAt(0)->GetHeader(AString("Content-Type"));

    nContentSmsType  = pMtsSipFormUtils->FormContentTypeStrToEnum(strContentType);

    if (MtsSmUtils::MTS_SMS_FORMAT_INVALID == nContentSmsType )
    {
        piPageMessage->Reject(415, 0 );

        delete this;
        return IMS_FALSE;
    }

    objSms = objMessageBodies.GetAt(0)->GetContent();

    // Mark SMS Msg Info
    m_nSmsTrxType = MtsSmUtils::MTS_SMS_TRX_TYPE_RECEIVE;
    m_nSmSize = objSms.GetLength();

    if (nContentSmsType  == MtsSmUtils::MTS_SMS_FORMAT_3GPP)
    {
        m_nSmsFormat = MtsSmUtils::MTS_SMS_FORMAT_3GPP;
        m_nMti = pMtsSmUtils->GetMti(MtsSmUtils::MTS_SMS_FORMAT_3GPP, objSms);
        m_nMrOfRp = pMtsSmUtils->GetRpMr(objSms);
    }
    else if (nContentSmsType  == MtsSmUtils::MTS_SMS_FORMAT_3GPP2)
    {
        m_nSmsFormat = MtsSmUtils::MTS_SMS_FORMAT_3GPP2;
        m_nMti = pMtsSmUtils->GetMti(MtsSmUtils::MTS_SMS_FORMAT_3GPP2, objSms);
    }

    if (GetSmsgwFromReceivedMessage(piPageMessage, strTempSmsgw) == IMS_TRUE)
    {
        SetDestination(strTempSmsgw);
        pMtsMessageController->SetLastIpsmgwAddr(strTempSmsgw);
    }

    PrintMsgInfo();

    return IMS_TRUE;

}

PUBLIC
IMS_SINT32 MtsMessage::GetSlotId()
{
    return m_nSlotId;
}

PUBLIC
MtsMessageController* MtsMessage::GetMtsMessageController()
{
    MtsMessageController* pMtsMessageController = IMS_NULL;

    if (m_pMtsMessageController != IMS_NULL)
    {
        pMtsMessageController = m_pMtsMessageController;
    }
    else
    {
        IMS_TRACE_E(0, "m_pMtsMessageController is invalid", 0, 0, 0);
    }

    return pMtsMessageController;
}

PUBLIC
void MtsMessage::CleanMtsMessagewithReportResponse(
        IN IMS_UINT32 nResponse, IN IMS_BOOL bSendToAos, IN IMS_UINT32 nType)
{
    MtsClient_ReportTransmissionResult(nResponse, GetContentType());
    MtsMessageController* pMtsMessageController = GetMtsMessageController();

    if (pMtsMessageController == IMS_NULL)
    {
        IMS_TRACE_E(0, "pMtsMessageController is null", 0, 0, 0);
        return;
    }

    MtsStrName* pMtsStrName = pMtsMessageController->GetMtsUtils()->GetMtsStrName();

    if (pMtsStrName == IMS_NULL)
    {
        IMS_TRACE_E(0, "pMtsMessageController is null", 0, 0, 0);
        return;
    }

    if (IMS_TRUE == bSendToAos)
    {
        IImsAos* piImsAos = ImsAos::GetImsAos(
                pMtsStrName->GetMtsAppId(), pMtsStrName->GetMtsServiceId(), m_nSlotId);
        if (piImsAos != IMS_NULL)
        {
            piImsAos->Control(nType);
        }
    }

    CleanMtsMessage();
}

PUBLIC
void MtsMessage::CleanMtsMessage()
{
    MtsMessageController* pMtsMessageController = GetMtsMessageController();

    if (pMtsMessageController != IMS_NULL)
    {
        pMtsMessageController->Remove(this);
    }
    delete this;
}

PUBLIC
void MtsMessage::CleanOperatorMtsMessage()
{
    MtsMessageController* pMtsMessageController = GetMtsMessageController();

    if (pMtsMessageController == IMS_NULL )
    {
        IMS_TRACE_E(0, "pMtsMessageController is null", 0, 0, 0);
        return;
    }
    //Remove DELIVER-MESSAGE;
    IMtsMessage* piMtsMessage = pMtsMessageController->Search(m_nMrOfRp);

    if (piMtsMessage != IMS_NULL)
    {
        pMtsMessageController->Remove(piMtsMessage);
        delete DYNAMIC_CAST(MtsMessage*, piMtsMessage);
    }
    else
    {
        IMS_TRACE_E(0, "piMtsMessage is null", 0, 0, 0);
    }
}

PROTECTED
void MtsMessage::SetDestination(IN const AString& strDestination)
{
    m_strDestination = strDestination;
}

PROTECTED
IMS_UINT32 MtsMessage::GetContentType() const
{
    return m_nSmsFormat;
}

PROTECTED
void MtsMessage::GetUserPartFromUris(IN const AString& strUri, OUT AString& strUserPart) const
{
    SIPAddress objSIPAddress;

    if (!objSIPAddress.Create(strUri))
    {
        IMS_TRACE_E(0, "Creating SIPAddress failed, strUri = %s ", strUri.GetStr(), 0, 0);
        return;
    }

    const SIPAddress::UserInfoPart* pUserInfoPart = objSIPAddress.GetUserInfoPart();

    if (pUserInfoPart != IMS_NULL)
    {
        strUserPart = pUserInfoPart->GetUser();
    }
    else
    {
        if (objSIPAddress.IsSchemeSIP() || objSIPAddress.IsSchemeSIPS())
        {
            strUserPart = objSIPAddress.GetUser();
        }
        else if (objSIPAddress.IsSchemeTEL())
        {
            strUserPart = objSIPAddress.GetHost();
        }
    }
}

PROTECTED
IMS_BOOL MtsMessage::GetSmsgwFromReceivedMessage(
        IN const IPageMessage* piPageMessage, OUT AString& strSmsgw)
{
    if (piPageMessage== IMS_NULL)
    {
        return IMS_FALSE;
    }

    IMSList<AString> objPAssrtIdHdrList = piPageMessage->GetRemoteUserId();

    if (!objPAssrtIdHdrList.IsEmpty())
    {
        strSmsgw = objPAssrtIdHdrList.GetAt(0);

        IMS_TRACE_D("P-Asserted-Identity for SMS-GW is %s", strSmsgw.GetStr(), 0, 0);

        if (strSmsgw.GetLength() != 0)
        {
            return IMS_TRUE;
        }
    }

    IMS_TRACE_I("P-Asserted-Identity is not included, then the From header is used for SMS-GW",
            0, 0, 0);

    IMessage* piMessage = piPageMessage->GetPreviousRequest(IMessage::PAGEMESSAGE_SEND);

    if (piMessage == IMS_NULL)
    {
        IMS_TRACE_E(0,"Failed to fetch IMessage from IPageMessage, Here it sends 400 response",
                0, 0, 0);
        return IMS_FALSE;
    }

    IMSList<AString> objFromHdrList = piMessage->GetHeaders(SIPHeaderName::FROM);

    if (objFromHdrList.IsEmpty())
    {
        IMS_TRACE_E(0,"From header is not found in IMessage!!", 0,0,0);
        return IMS_FALSE;
    }

    GetUriFromHeaders(objFromHdrList.GetAt(0), strSmsgw);

    if (strSmsgw.GetLength() == 0)
    {
        IMS_TRACE_E(0,"Failed to get SIP address from the From header!!", 0, 0, 0);
        return IMS_FALSE;
    }

    IMS_TRACE_I("From header for SMS-GW is %s", strSmsgw.GetStr(), 0, 0);

    return IMS_TRUE;
}

PROTECTED
void MtsMessage::GetUriFromHeaders(IN const AString& strFromHdr, OUT AString& strUri) const
{
    SIPAddress objSIPAddress;

    if (!objSIPAddress.Create(strFromHdr))
    {
        IMS_TRACE_E(0, "Creating SIPAddress(From:%s) failed", strFromHdr.GetStr(), 0, 0);
        return;
    }

    strUri = objSIPAddress.ToString();

    IMS_TRACE_I("SIPAddress :: uri=%s", strUri.GetStr(), 0, 0);
}

PROTECTED
IMS_SINT32 MtsMessage::GetRetryAfterValue(IN IMessage* piMessage)
{
    IMSList<AString> objHeaderList = piMessage->GetHeaders(SIPHeaderName::RETRY_AFTER);

    if (objHeaderList.IsEmpty())
    {
        IMS_TRACE_E(0, "Error Response Message has not Retry-After Header", 0, 0, 0);
        return -1;
    }

    AString strHeader = objHeaderList.GetAt(objHeaderList.GetSize() - 1);

    if (strHeader.GetLength() == 0)
    {
        return -1;
    }

    ISIPHeader* piHeader = SIPParsingHelper::CreateHeader(ISIPHeader::RETRY_AFTER_SEC, strHeader);

    if (piHeader == IMS_NULL)
    {
        return -1;
    }

    IMS_SINT32 nValue = piHeader->GetValueInt();

    piHeader->Destroy();

    IMS_TRACE_I("Retry-After :: value=%d", nValue, 0, 0);

    return nValue;
}

PROTECTED
void MtsMessage::MtsClient_ReportTransmissionResult(
        IN IMS_UINT32 nResponse, IN IMS_UINT32 nSmsType)
{
    MtsClient::GetInstance(m_nSlotId)->ReportTransmissionResult(nResponse, nSmsType, m_nSeqId,
            m_nSlotId);
}

PROTECTED
void MtsMessage::MtsClient_RequestRegistrationRecovery(IN IMS_SINT32 nRecoveryType)
{
    MtsClient::GetInstance(m_nSlotId)->RequestRegistrationRecovery(nRecoveryType, m_nSlotId);
}

PRIVATE
void MtsMessage::SetTrmInfo(IN IMS_SINT32 nSlotId, IN IMS_BOOL bSmsState)
{
    MtsTrm* piTrm = MtsTrm::GetInstance(nSlotId);
    IMS_TRACE_I("SetTrmInfo nSlotId : [%d], bSmsState[%d]", nSlotId, bSmsState, 0);
    if (piTrm->IsTRMSupported())
    {
        piTrm->Set(bSmsState);
    }

}
