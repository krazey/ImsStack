#include "msg/sip_msgutil.h"
#include "sip_pf_datatypes.h"
#include "platform/sip_pf_string.h"
#include "platform/sip_pf_memory.h"

#include "sip_debug.h"
#include "SipTrace.h"
#include "sip_error.h"

#include "msg/SipMessage.h"
#include "msg/SipMsgBody.h"

SIP_INT32 sipGetMethodType(const SIP_CHAR* pszMethod)
{
    SIP_CHAR SipMethodArray[][SIP_MAX_HDR_LEN] = {
            "INVITE",
            "ACK",
            "OPTIONS",
            "BYE",
            "CANCEL",
            "REGISTER",
            "INFO",
            "PRACK",
            "SUBSCRIBE",
            "NOTIFY",
            "UPDATE",
            "MESSAGE",
            "REFER",
            "PUBLISH",
    };

    if ((pszMethod == SIP_NULL) || (SipPf_Strlen(pszMethod) > SIP_MAX_HDR_LEN))
    {
        return SipMessage::METHOD_INVALID;
    }

    for (SIP_INT16 nIndex = SIP_ZERO; nIndex < SipMessage::METHOD_END; nIndex++)
    {
        if (SipPf_Stricmp(pszMethod, SipMethodArray[nIndex]) == SIP_ZERO)
        {
            return nIndex;
        }
    }
    return SipMessage::METHOD_INVALID;
}

/****************************************************************************
  Class Member Function Implementations
 *****************************************************************************/

/******************************************************************************
 * Function name      : SipMessage::SipMessage
 *
 * Description     :
 *
 * Preconditions      :
 *
 * Side Effects      : none
 *****************************************************************************/
SipMessage::SipMessage() :
        m_eSipMsgType(SipMessage::TYPE_INVALID),
        m_pReqLine(SIP_NULL),
        m_pStatusLine(SIP_NULL),
        m_objHdrs(SIP_NULL),
        m_pMsgBodyList(SIP_NULL)
#ifdef SIP_BADMESSAGE_PARSING
        ,
        mbitMask(MANDATORY_HDR_NONE),
        m_pBadHdrList(SIP_NULL)
#endif
{
    m_objHdrs = new SipHeaders();
}

/******************************************************************************
 * Function name      : SipMessage::SipMessage
 *
 * Description     :
 *
 * Preconditions      :
 *
 * Side Effects      : none
 *****************************************************************************/
SipMessage::SipMessage(const SipMessage& objSipMsg) :
        m_eSipMsgType(objSipMsg.m_eSipMsgType),
        m_pReqLine(SIP_NULL),
        m_pStatusLine(SIP_NULL),
        m_objHdrs(SIP_NULL),
        m_pMsgBodyList(SIP_NULL)
#ifdef SIP_BADMESSAGE_PARSING
        ,
        mbitMask(objSipMsg.mbitMask),
        m_pBadHdrList(SIP_NULL)
#endif
{
    m_objHdrs = new SipHeaders();

    m_objHdrs->CopyHdrs(objSipMsg.m_objHdrs);

    if (objSipMsg.m_pReqLine != SIP_NULL)
    {
        m_pReqLine = new SipRequestLine(*(objSipMsg.m_pReqLine));
    }

    if (objSipMsg.m_pStatusLine != SIP_NULL)
    {
        m_pStatusLine = new SipStatusLine(*(objSipMsg.m_pStatusLine));
    }

    if (objSipMsg.m_pMsgBodyList != SIP_NULL)
    {
        m_pMsgBodyList = new SipMsgBodyList(*(objSipMsg.m_pMsgBodyList));
    }

#ifdef SIP_BADMESSAGE_PARSING
    if (objSipMsg.m_pBadHdrList != SIP_NULL)
    {
        m_pBadHdrList = new SipHeaderList(*(objSipMsg.m_pBadHdrList));
    }
#endif
}

/******************************************************************************
 * Function name      : SipMessage::~SipMessage
 *
 * Description     :
 *
 * Preconditions      :
 *
 * Side Effects      : none
 *****************************************************************************/
SipMessage::~SipMessage()
{
    if (m_pReqLine != SIP_NULL)
    {
        m_pReqLine->SipDelete();
    }
    if (m_pStatusLine != SIP_NULL)
    {
        m_pStatusLine->SipDelete();
    }
    if (m_pMsgBodyList != SIP_NULL)
    {
        m_pMsgBodyList->SipDelete();
    }
    if (m_objHdrs != SIP_NULL)
    {
        delete m_objHdrs;
    }
#ifdef SIP_BADMESSAGE_PARSING
    if (m_pBadHdrList != SIP_NULL)
    {
        m_pBadHdrList->SipDelete();
    }
#endif
}

/******************************************************************************
 * Function name      : SipMessage::SipMessage
 *
 * Description     :
 *
 * Preconditions      :
 *
 * Side Effects      : none
 *****************************************************************************/
SipMessage::SipMessage(SIP_INT32 eSipMsgType) :
        m_eSipMsgType(eSipMsgType),
        m_pReqLine(SIP_NULL),
        m_pStatusLine(SIP_NULL),
        m_objHdrs(SIP_NULL),
        m_pMsgBodyList(SIP_NULL)
#ifdef SIP_BADMESSAGE_PARSING
        ,
        mbitMask(MANDATORY_HDR_NONE),
        m_pBadHdrList(SIP_NULL)
#endif
{
    m_objHdrs = new SipHeaders();
}

/******************************************************************************
 * Function name      : SipMessage::SetRequestline
 *
 * Description     :
 *
 * Preconditions      :
 *
 * Side Effects      : none
 *****************************************************************************/
SIP_BOOL SipMessage::SetRequestline(SipRequestLine* pReqLine)
{
    if (m_pReqLine != SIP_NULL)
    {
        m_pReqLine->SipDelete();
    }

    m_pReqLine = pReqLine;
    return SIP_TRUE;
}
/******************************************************************************
 * Function name      : SipMessage::RemoveHdr
 *
 * Description     :
 *
 * Preconditions      :
 *
 * Side Effects      : none
 *****************************************************************************/
SIP_BOOL SipMessage::RemoveHdr(SIP_INT32 eHdrType)
{
    return m_objHdrs->RemoveHdr(eHdrType);
}
/******************************************************************************
 * Function name      : SipMessage::SetHeader
 *
 * Description     :
 *
 * Preconditions      :
 *
 * Side Effects      : none
 *****************************************************************************/
SIP_BOOL SipMessage::SetHeader(SipHeaderBase* pHdr)
{
    SIP_BOOL bStatus = SIP_FALSE;
    SIP_INT32 nType = pHdr->GetHdrType();

    if (nType == SipHeaderBase::UNKNOWN)
    {
        bStatus = m_objHdrs->AppendHdr(pHdr);
    }
    else
    {
        bStatus = m_objHdrs->SetHdr(pHdr);
    }

    if (bStatus == SIP_FALSE)
    {
        SIP_DEBUG_WARNING(ESIPTRACE_MODACCESSOR, "set header type %d Fail", nType, SIP_ZERO);
        return SIP_FALSE;
    }

    return SIP_TRUE;
}

/******************************************************************************
 * Function name      : SipMessage::SetMessageBody
 *
 * Description     :
 *
 * Preconditions      :
 *
 * Side Effects      : none
 *****************************************************************************/
SIP_BOOL SipMessage::SetMessageBody(SipMsgBody* pMsgBody)
{
    if (m_pMsgBodyList == SIP_NULL)
    {
        m_pMsgBodyList = new SipMsgBodyList();
    }
    return m_pMsgBodyList->AddBody(pMsgBody);
}

/******************************************************************************
 * Function name      : SipMessage::SetMessageType
 *
 * Description     :
 *
 * Preconditions      :
 *
 * Side Effects      : none
 *****************************************************************************/
SIP_BOOL SipMessage::SetMessageType(SIP_INT32 eMsgType)
{
    m_eSipMsgType = eMsgType;
    return SIP_TRUE;
}

/******************************************************************************
 * Function name      : SipMessage::SetStatusLine
 *
 * Description     :
 *
 * Preconditions      :
 *
 * Side Effects      : none
 *****************************************************************************/
SIP_BOOL SipMessage::SetStatusLine(SipStatusLine* pStatusLine)
{
    if (m_pStatusLine != SIP_NULL)
    {
        m_pStatusLine->SipDelete();
    }

    m_pStatusLine = pStatusLine;
    return SIP_TRUE;
}

/******************************************************************************
 * Function name      : SipMessage::AppendHeader
 *
 * Description     :
 *
 * Preconditions      :
 *
 * Side Effects      : none
 *****************************************************************************/
SIP_BOOL SipMessage::AppendHeader(SipHeaderBase* pHdr)
{
    if (m_objHdrs->AppendHdr(pHdr) == SIP_FALSE)
    {
        SIP_DEBUG_WARNING(ESIPTRACE_MODACCESSOR, "AppendHeader Fail", SIP_ZERO, SIP_ZERO);
        return SIP_FALSE;
    }
    return SIP_TRUE;
}

/******************************************************************************
 * Function name      : SipMessage::InsertHeader
 *
 * Description     :
 *
 * Preconditions      :
 *
 * Side Effects      : noneGLOBAL IMS_BOOL InsertHeader(IN SipHeaderBase* pstHeader,
                                             IN IMS_UINT32 nIndex,IN_OUT SipMessage *&pstMessage)
 *****************************************************************************/
SIP_BOOL SipMessage::InsertHeader(SipHeaderBase* pHdr, SIP_UINT32 nIndex)
{
    if (m_objHdrs->InsertHdr(pHdr, nIndex) == SIP_FALSE)
    {
        SIP_DEBUG_WARNING(ESIPTRACE_MODACCESSOR, "AppendHeader Fail", SIP_ZERO, SIP_ZERO);
        return SIP_FALSE;
    }
    return SIP_TRUE;
}

/******************************************************************************
 * Function name      : SipMessage::GetMethodType
 *
 * Description     :
 *
 * Preconditions      :
 *
 * Side Effects      : none
 *****************************************************************************/

SIP_INT32 SipMessage::GetMethodType()
{
    const SIP_CHAR* pszMethod = GetMethod();
    if (pszMethod == SIP_NULL)
    {
        return SipMessage::METHOD_INVALID;
    }
    return sipGetMethodType(pszMethod);
}

/******************************************************************************
 * Function name      : SipMessage::GetMethod
 *
 * Description     :
 *
 * Preconditions      :
 *
 * Side Effects      : none
 *****************************************************************************/
const SIP_CHAR* SipMessage::GetMethod()
{
    if (SipMessage::REQ_TYPE == m_eSipMsgType)
    {
        if (m_pReqLine == SIP_NULL)
        {
            return SIP_NULL;
        }
        return m_pReqLine->GetMethod();
    }

    if (m_pStatusLine == SIP_NULL)
    {
        SIP_DEBUG_WARNING(ESIPTRACE_MODENCODER, "status line is NULL", SIP_ZERO, SIP_ZERO);
        return SIP_NULL;
    }

    SipCSeqHeader* pCseqHdr = (SipCSeqHeader*)GetHdrObj(SipHeaderBase::CSEQ);
    if (pCseqHdr == SIP_NULL)
    {
        SIP_DEBUG_WARNING(ESIPTRACE_MODENCODER, "CSEQ header is NULL", SIP_ZERO, SIP_ZERO);
        return SIP_NULL;
    }

    const SIP_CHAR* pszMethod = pCseqHdr->GetMethod();
    pCseqHdr->SipDelete();

    return pszMethod;
}

/******************************************************************************
 * Function name      : SipMessage::GetHdrObj
 *
 * Description     :
 *
 * Preconditions      :
 *
 * Side Effects      : none
 *****************************************************************************/
SipHeaderBase* SipMessage::GetHdrObj(SIP_INT32 eHdrType)
{
    return m_objHdrs->getHdrObj(eHdrType, SIP_ZERO);
}

/******************************************************************************
 * Function name      : SipMessage::HasHeader
 *
 * Description     :
 *
 * Preconditions      :
 *
 * Side Effects      : none
 *****************************************************************************/
SIP_BOOL SipMessage::HasHeader(SIP_INT32 eHdrType) const
{
    SipHeaderBase* pHdrBase = m_objHdrs->getHdrObj(eHdrType);
    if (pHdrBase != SIP_NULL)
    {
        pHdrBase->SipDelete();
        return SIP_TRUE;
    }
    return SIP_FALSE;
}

/******************************************************************************
 * Function name      : IsReqLineExists
 *
 * Description     :
 *
 * Preconditions      :
 *
 * Side Effects      : none
 *****************************************************************************/
SIP_BOOL SipMessage::IsReqLineExists()
{
    SipRequestLine* pReqLine = GetReqLine();
    if (pReqLine != SIP_NULL)
    {
        pReqLine->SipDelete();
        return SIP_TRUE;
    }
    return SIP_FALSE;
}

/******************************************************************************
 * Function name      : IsStatusLineExists
 *
 * Description     :
 *
 * Preconditions      :
 *
 * Side Effects      : none
 *****************************************************************************/
SIP_BOOL SipMessage::IsStatusLineExists()
{
    SipStatusLine* pStatusLine = GetStatusLine();
    if (pStatusLine != SIP_NULL)
    {
        pStatusLine->SipDelete();
        return SIP_TRUE;
    }
    return SIP_FALSE;
}

/******************************************************************************
 * Function name      : HasMandatoryHdrs
 *
 * Description     :
 *
 * Preconditions      :
 *
 * Side Effects      : none
 *****************************************************************************/

#ifdef SIP_BADMESSAGE_PARSING
SIP_BOOL SipMessage::HasMandatoryHdrs()
{
    SIP_DEBUG_WARNING(ESIPTRACE_MODENCODER, "MANDATORY_HDR_MASK = %x", mbitMask, SIP_ZERO);

    if (mbitMask != MANDATORY_HDR_MASK)
    {
        return SIP_FALSE;
    }

    if (IsReqLineExists() == SIP_TRUE)
    {
        const SIP_CHAR* pszMethod = GetMethod();

        if ((SipPf_Stricmp(pszMethod, "PRACK") == 0) &&
                ((HasHeader(SipHeaderBase::RACK) == SIP_FALSE)))
        {
            return SIP_FALSE;
        }

        SipRequestLine* pReqLine = GetReqLine();
        if (pReqLine == SIP_NULL)
        {
            return SIP_FALSE;
        }

        const SIP_CHAR* pSipVer = pReqLine->GetSipVersion();
        SipAddrSpec* pReqUri = pReqLine->GetReqUri();
        if (((pReqUri != SIP_NULL) && (pszMethod != SIP_NULL) && (pSipVer != SIP_NULL)) ==
                SIP_FALSE)
        {
            pReqLine->SipDelete();
            if (pReqUri != SIP_NULL)
            {
                pReqUri->SipDelete();
            }
            return SIP_FALSE;
        }
        pReqLine->SipDelete();
        pReqUri->SipDelete();
    }
    return SIP_TRUE;
}

/******************************************************************************
 * Function name      : GetBadHeaderCount
 *
 * Description     :
 *
 * Preconditions      :
 *
 * Side Effects      : none
 *****************************************************************************/

SIP_INT32 SipMessage::GetBadHeaderCount() const
{
    if (m_pBadHdrList != SIP_NULL)
    {
        SIP_DEBUG_WARNING(
                ESIPTRACE_MODDECODER, "Bad header count = %d", m_pBadHdrList->GetSize(), SIP_ZERO);
        return m_pBadHdrList->GetSize();
    }
    return 0;
}

/******************************************************************************
 * Function name      : GetBadHdrs
 *
 * Description     :
 *
 * Preconditions      :
 *
 * Side Effects      : none
 *****************************************************************************/
SipHeaderList* SipMessage::GetBadHdrs()
{
    return m_pBadHdrList;
}

/******************************************************************************
 * Function name      : DeleteBadHdrList
 *
 * Description     :
 *
 * Preconditions      :
 *
 * Side Effects      : none
 *****************************************************************************/

SIP_VOID SipMessage::DeleteBadHdrList()
{
    if (m_pBadHdrList != SIP_NULL)
    {
        m_pBadHdrList->SipDelete();
        m_pBadHdrList = SIP_NULL;
    }
}
#endif
/******************************************************************************
 * Function name      : HasMIMEMessageBody
 *
 * Description     :
 *
 * Preconditions      :
 *
 * Side Effects      : none
 *****************************************************************************/
SIP_BOOL SipMessage::HasMIMEMessageBody()
{
    SipContentTypeHeader* pHdr =
            SIP_DYNAMIC_CAST(SipContentTypeHeader*, (GetHdrObj(SipHeaderBase::CONTENT_TYPE)));

    if (pHdr == SIP_NULL)
    {
        return SIP_FALSE;
    }

    const SIP_CHAR* pszMType = pHdr->GetMediaType();
    SIP_INT16 nResult = SipPf_Stricmp(pszMType, MULTIPART);
    pHdr->SipDelete();
    return (nResult == SIP_ZERO) ? SIP_TRUE : SIP_FALSE;
}

/******************************************************************************
 * Function name      : HasSDPMessageBody
 *
 * Description     :
 *
 * Preconditions      :
 *
 * Side Effects      : none
 *****************************************************************************/
SIP_BOOL SipMessage::HasSDPMessageBody()
{
    SipContentTypeHeader* pHdr =
            SIP_DYNAMIC_CAST(SipContentTypeHeader*, (GetHdrObj(SipHeaderBase::CONTENT_TYPE)));

    if (pHdr == SIP_NULL)
    {
        return SIP_FALSE;
    }

    const SIP_CHAR* pszSubMType = pHdr->GetSubMediaType();
    SIP_INT16 nResult = SipPf_Stricmp(pszSubMType, SDP);
    pHdr->SipDelete();
    return (nResult == SIP_ZERO) ? SIP_TRUE : SIP_FALSE;
}

/******************************************************************************
 * Function name      : SipMessage::GetHdrList
 *
 * Description     :
 *
 * Preconditions      :
 *
 * Side Effects      : none
 *****************************************************************************/
SipHeaderList* SipMessage::GetHdrList(SIP_INT32 eHdrType)
{
    return (SipHeaders::IsListHdr(eHdrType) == SIP_TRUE)
            ? (SipHeaderList*)m_objHdrs->getHdrObj(eHdrType)
            : SIP_NULL;
}

/******************************************************************************
 * Function name      : SipMessage::SetHdrList
 *
 * Description     :
 *
 * Preconditions      :
 *
 * Side Effects      : none
 *****************************************************************************/
SIP_BOOL SipMessage::SetHdrList(SipHeaderList* pHdrList)
{
    SIP_UINT32 nListSize = pHdrList->GetSize();
    for (SIP_UINT32 nIndex = SIP_ZERO; nIndex < nListSize; nIndex++)
    {
        SipHeaderBase* pHdrbase = pHdrList->GetObj(nIndex);
        if (pHdrbase != SIP_NULL)
        {
            SIP_BOOL bStatus = m_objHdrs->AppendHdr(pHdrbase);
            pHdrbase->SipDelete();
            if (bStatus == SIP_FALSE)
            {
                return SIP_FALSE;
            }
        }
    }
    return SIP_TRUE;
}

/******************************************************************************
 * Function name      : SipMessage::SetMsgBodyList
 *
 * Description     :
 *
 * Preconditions      :
 *
 * Side Effects      : none
 *****************************************************************************/
SIP_BOOL SipMessage::SetMsgBodyList(SipMsgBodyList* pMsgBodyList)
{
    (void)pMsgBodyList;
    return SIP_TRUE;
}

/******************************************************************************
 * Function name      : SipMessage::GetMsgBodyCount
 *
 * Description     :
 *
 * Preconditions      :
 *
 * Side Effects      : none
 *****************************************************************************/
SipMsgBody* SipMessage::GetMessageBody(SIP_INT32 nIndex)
{
    return (m_pMsgBodyList != SIP_NULL) ? m_pMsgBodyList->GetBodyByIndex(nIndex) : SIP_NULL;
}

/******************************************************************************
 * Function name      : SipMessage::AppendMessageBody
 *
 * Description     :
 *
 * Preconditions      :
 *
 * Side Effects      : none
 *****************************************************************************/
SIP_BOOL SipMessage::AppendMessageBody(SipMsgBody* pMsgBody)
{
    return SetMessageBody(pMsgBody);
}

/******************************************************************************
 * Function name      : SipMessage::GetMessageBody
 *
 * Description     :
 *
 * Preconditions      :
 *
 * Side Effects      : none
 *****************************************************************************/
SipMsgBody* SipMessage::GetMsgBody(SIP_UINT32 nPos)
{
    return (m_pMsgBodyList != SIP_NULL) ? m_pMsgBodyList->GetBodyByIndex(nPos) : SIP_NULL;
}

/******************************************************************************
 * Function name  : SipMessage::EncodeMsg
 *
 * Description   :
 *
 * Preconditions  :
 *
 * Side Effects  : none
 *****************************************************************************/
SIP_BOOL SipMessage::EncodeMsg(SIP_CHAR** ppSipMsgBuffer, /* in-out parameter*/
        SIP_UINT32* pSipMsgLength, /* in-out parameter*/ SIP_UINT32 nMsgOptions)
{
    if ((m_pReqLine == SIP_NULL) && (m_pStatusLine == SIP_NULL))
    {
        SIP_DEBUG_WARNING(ESIPTRACE_MODENCODER, "Start Line Missing", SIP_ZERO, SIP_ZERO);
        return SIP_FALSE;
    }

    if ((m_eSipMsgType != SipMessage::REQ_TYPE) && (m_eSipMsgType != SipMessage::RESP_TYPE))
    {
        SIP_DEBUG_WARNING(ESIPTRACE_MODENCODER, "Invalid sip message type", SIP_ZERO, SIP_ZERO);
        return SIP_FALSE;
    }

    SIP_CHAR* pCurrPos = *ppSipMsgBuffer;

    if (m_eSipMsgType == SipMessage::REQ_TYPE && m_pReqLine != SIP_NULL)
    {
        if (m_pReqLine->EncodeRequestLine(&pCurrPos) == SIP_FALSE)
        {
            SIP_DEBUG_WARNING(
                    ESIPTRACE_MODENCODER, "Encoding Request line Fail", SIP_ZERO, SIP_ZERO);
            return SIP_FALSE;
        }
    }
    else if (m_eSipMsgType == SipMessage::RESP_TYPE && m_pStatusLine != SIP_NULL)
    {
        if (m_pStatusLine->EncodeStatusLine(&pCurrPos) == SIP_FALSE)
        {
            SIP_DEBUG_WARNING(
                    ESIPTRACE_MODENCODER, "Encoding status line Fail", SIP_ZERO, SIP_ZERO);
            return SIP_FALSE;
        }
    }

    // Put CRLF at the end of Start Line
    SIP_ENC_CRLF(pCurrPos);

    SIP_TRACE_MESSAGE(ESIPTRACE_MODENCODER, "SipEnc_SipMsg:Encoded Msg(Start line) - %s",
            *ppSipMsgBuffer, SIP_ZERO);

    // Set content type header if it is not set
    if (this->m_pMsgBodyList != SIP_NULL)
    {
        /*Set the content type and content length headers before header encoding*/
        if (UpdateMessage(nMsgOptions) == SIP_FALSE)
        {
            SIP_DEBUG_WARNING(
                    ESIPTRACE_MODENCODER, "updating content headers fail", SIP_ZERO, SIP_ZERO);
            return SIP_FALSE;
        }
    }
    else
    {
        SipUnknownHeader* pUnknown = GetUnknownHdrObj(SipHeaderBase::CONTENT_LENGTH);
        if (pUnknown == SIP_NULL)
        {
            SetContentLengthHdr(SIP_ZERO, nMsgOptions);
        }
        else
        {
            // Content-Length header is present in message, reset the value to ZERO as no
            // body in the message.
            SIP_CHAR szLen[SIP_CONTLEN_LEN] = {0};

            SipPf_Sprintf(szLen, (SIP_CHAR*)"%d", 0);
            pUnknown->SetHeaderValue(szLen);

            pUnknown->SipDelete();
            AdjustContentLengthHdr();
        }
    }

    // Encoding of headers
    if (m_objHdrs->EncodeHdrs(&pCurrPos, nMsgOptions) == SIP_FALSE)
    {
        SIP_DEBUG_WARNING(ESIPTRACE_MODENCODER, "Headers Encoding Failed", SIP_ZERO, SIP_ZERO);
        return SIP_FALSE;
    }

    // Put CRLF at the end of List of Headers
    SIP_ENC_CRLF(pCurrPos);

    // Encode Message body if present
    if (m_pMsgBodyList != SIP_NULL)
    {
        SipContentTypeHeader* pContentType =
                SIP_STATIC_CAST(SipContentTypeHeader*, GetHdrObj(SipHeaderBase::CONTENT_TYPE));
        if (pContentType == SIP_NULL)
        {
            SIP_DEBUG_WARNING(ESIPTRACE_MODENCODER, "No ContentType header", SIP_ZERO, SIP_ZERO);
            return SIP_FALSE;
        }

        const SIP_CHAR* pszMediaType = pContentType->GetMediaType();
        if (pszMediaType == SIP_NULL)
        {
            SIP_DEBUG_WARNING(ESIPTRACE_MODENCODER, "Content Type Invalid", SIP_ZERO, SIP_ZERO);
            return SIP_FALSE;
        }

        const SIP_CHAR* pszSubMediaType = pContentType->GetSubMediaType();
        if (pszSubMediaType == SIP_NULL)
        {
            SIP_DEBUG_WARNING(ESIPTRACE_MODENCODER, "Content Type Invalid", SIP_ZERO, SIP_ZERO);
            return SIP_FALSE;
        }

        if (SipPf_Strstr(pszSubMediaType, "message-summary") != SIP_NULL)
        {
            SipMsgBody* pBody = m_pMsgBodyList->GetBodyByIndex(SIP_ZERO);
            if (pBody == SIP_NULL)
            {
                SIP_DEBUG_WARNING(ESIPTRACE_MODENCODER, "Msg Body NULL", SIP_ZERO, SIP_ZERO);
                return SIP_FALSE;
            }
            if (pBody->EncodeMessageSummaryMsgBody(&pCurrPos) == SIP_FALSE)
            {
                SIP_DEBUG_WARNING(
                        ESIPTRACE_MODENCODER, "Encode message-summary fail", SIP_ZERO, SIP_ZERO);
                return SIP_FALSE;
            }
            pBody->SipDelete();
        }
        else
        {
            /*Case of Single Body*/
            if (SipPf_Stricmp(pszMediaType, MULTIPART) != SIP_ZERO)
            {
                SipMsgBody* pBody = m_pMsgBodyList->GetBodyByIndex(SIP_ZERO);

                if (pBody == SIP_NULL)
                {
                    SIP_DEBUG_WARNING(ESIPTRACE_MODENCODER, "Msg Body NULL", SIP_ZERO, SIP_ZERO);
                    return SIP_FALSE;
                }

                if (pBody->EncodeSingleMsgBody(&pCurrPos) == SIP_FALSE)
                {
                    SIP_DEBUG_WARNING(ESIPTRACE_MODENCODER, "Encode single message body fail",
                            SIP_ZERO, SIP_ZERO);
                    return SIP_FALSE;
                }

                pBody->SipDelete();
            }
            /*Case of multipart body*/
            else
            {
                SIP_CHAR* pszBoundary = pContentType->GetBoundary();
                if (pszBoundary == SIP_NULL)
                {
                    SIP_DEBUG_WARNING(
                            ESIPTRACE_MODENCODER, "Boundary not found", SIP_ZERO, SIP_ZERO);
                    return SIP_FALSE;
                }

                if (m_pMsgBodyList->EncodeBody(&pCurrPos, pszBoundary) == SIP_FALSE)
                {
                    delete[] pszBoundary;
                    SIP_DEBUG_WARNING(ESIPTRACE_MODENCODER, "Encode multipart message body fail",
                            SIP_ZERO, SIP_ZERO);
                    return SIP_FALSE;
                }
                delete[] pszBoundary;
            }
        }
        pContentType->SipDelete();
    }

    // Calculate the length of the buffer
    SIP_UINT32 nMsgLength = pCurrPos - *ppSipMsgBuffer;

    *pSipMsgLength = nMsgLength;

    SIP_DEBUG_WARNING(ESIPTRACE_MODENCODER, "Encoded Message of length(%d)", nMsgLength, SIP_ZERO);

    return SIP_TRUE;
}

SIP_BOOL SipMessage::UpdateMessage(SIP_UINT32 nMsgOptions)
{
    if (m_pMsgBodyList == SIP_NULL)
    {
        SIP_DEBUG_WARNING(
                ESIPTRACE_MODENCODER, "Invalid param to UpdateMessage", SIP_ZERO, SIP_ZERO);
        return SIP_FALSE;
    }

    /*check for content type header and set the new one if not present*/
    if (HasHeader(SipHeaderBase::CONTENT_TYPE) == SIP_FALSE)
    {
        SIP_UINT16 nBodyCount = m_pMsgBodyList->GetMsgBodyCount();

        /*In case there is a single body then take out the
          content type header from the body and set into the SIP Message*/
        if (nBodyCount == SIP_ONE)
        {
            SipMsgBody* pMsgbody = m_pMsgBodyList->GetBodyByIndex(SIP_ZERO);
            if (pMsgbody == SIP_NULL)
            {
                SIP_DEBUG_WARNING(
                        ESIPTRACE_MODENCODER, "No body in message body list", SIP_ZERO, SIP_ZERO);
                return SIP_FALSE;
            }
            /*get the content type from the body*/
            SipContentTypeHeader* pTempContentType = pMsgbody->GetContentType();
            SipContentTypeHeader* pContentType = new SipContentTypeHeader(*pTempContentType);

            if (pTempContentType != SIP_NULL)
            {
                pTempContentType->SipDelete();
            }

            /*Set the header into the SIP message*/
            SetHeader(pContentType);

            /*Delete After Setting*/
            pContentType->SipDelete();

            /*Delete the message body*/
            pMsgbody->SipDelete();
        }
        /*Case of more than one bodies*/
        else
        {
            SipContentTypeHeader* pContentType = new SipContentTypeHeader();

            if (pContentType == SIP_NULL)
            {
                SIP_DEBUG_WARNING(
                        ESIPTRACE_MODENCODER, "Memory allocation Fail", SIP_ZERO, SIP_ZERO);
                return SIP_FALSE;
            }
            pContentType->SetMediaType(MULTIPART);
            pContentType->SetSubMediaType(MIXED);

            // Addition of boundary parameter is TBD

            SetHeader(pContentType);
            pContentType->SipDelete();
        }
    }

    /*Reset Content-length in message*/
    SIP_INT32 nLen = SIP_ZERO;

    /*Check the content type from message*/
    SipContentTypeHeader* pContentType =
            SIP_STATIC_CAST(SipContentTypeHeader*, GetHdrObj(SipHeaderBase::CONTENT_TYPE));

    if (pContentType != SIP_NULL)
    {
        /*Get the boundary*/
        SIP_CHAR* pszBoundary = pContentType->GetBoundary();
        /*Single Body*/
        if (pszBoundary == SIP_NULL)
        {
            nLen = m_pMsgBodyList->GetTotalBodyLen();
        }
        else
        {
            nLen = m_pMsgBodyList->GetTotalBodyLen(pszBoundary);
            delete[] pszBoundary;
        }
        pContentType->SipDelete();
    }
    else
    {
        nLen = SIP_ZERO;
    }

    /*Is content-length already present*/
    SipUnknownHeader* pUnknown = GetUnknownHdrObj(SipHeaderBase::CONTENT_LENGTH);
    if (pUnknown == SIP_NULL)
    { /*if not present add new*/
        SetContentLengthHdr(nLen, nMsgOptions);
    }
    else
    { /*If present validate and update*/
        SIP_INT32 nCurLen = 0;
        SIP_CHAR szLen[SIP_CONTLEN_LEN] = {0};
        const SIP_CHAR* pCurLen = pUnknown->GetHeaderValue();

        if (pCurLen != SIP_NULL)
        {
            nCurLen = SipPf_Atoi(pCurLen);
        }

        if (nLen != nCurLen)
        { /*Mismatch in content length & actual body size*/
            SipPf_Sprintf(szLen, (SIP_CHAR*)"%u", nLen);
            pUnknown->SetHeaderValue(szLen);
        }

        pUnknown->SipDelete();

        AdjustContentLengthHdr();
    }

    return SIP_TRUE;
}

SIP_BOOL SipMessage::DecMultiPartBody(
        SIP_CHAR* pBuffStart, SIP_CHAR* pBuffEnd, SIP_UINT32 nMsgBuffLen)
{
    (void)nMsgBuffLen;  // unused as of now

    // If body list already present, free it
    if (m_pMsgBodyList != SIP_NULL)
    {
        m_pMsgBodyList->SipDelete();
    }

    m_pMsgBodyList = new SipMsgBodyList();
    if (m_pMsgBodyList == SIP_NULL)
    {
        return SIP_FALSE;
    }

    SipContentTypeHeader* pContentType =
            (SipContentTypeHeader*)m_objHdrs->getHdrObj(SipHeaderBase::CONTENT_TYPE);
    if (pContentType == SIP_NULL)
    {
        SIP_DEBUG_WARNING(ESIPTRACE_MODDECODER, "Content Type Not present", SIP_ZERO, SIP_ZERO);
        return SIP_FALSE;
    }

    SIP_CHAR* pszBoundary = pContentType->GetBoundary();
    if (pszBoundary == SIP_NULL)
    {
        SIP_DEBUG_WARNING(
                ESIPTRACE_MODDECODER, "No boundary in Content Type Hdr", SIP_ZERO, SIP_ZERO);
        pContentType->SipDelete();
        return SIP_FALSE;
    }

    if (m_pMsgBodyList->DecodeMIMEBody(pBuffStart, pBuffEnd, pszBoundary) != SIP_TRUE)
    {
        SIP_DEBUG_WARNING(ESIPTRACE_MODDECODER, "DecodeMIMEBody Fail", SIP_ZERO, SIP_ZERO);
        delete[] pszBoundary;
        pContentType->SipDelete();
        return SIP_FALSE;
    }

    delete[] pszBoundary;
    pContentType->SipDelete();

    return SIP_TRUE;
}

SIP_BOOL SipMessage::DecCompleteMsg(SIP_CHAR* pMsgBuff, SIP_UINT32 nMsgBuffLen)
{
    if ((pMsgBuff == SIP_NULL) || (nMsgBuffLen <= SIP_ZERO))
    {
        SIP_DEBUG_WARNING(ESIPTRACE_MODDECODER, "DecCompleteMsg:Invalid input", SIP_ZERO, SIP_ZERO);
        return SIP_FALSE;
    }

    SIP_CHAR* pStartPt = pMsgBuff;
    SIP_CHAR* pEndPt = pMsgBuff + nMsgBuffLen;

    // Remove additional CRLF if present at the starting of incoming message.
    while (IS_CR(*pStartPt) && IS_LF(*(pStartPt + SIP_ONE)))
    {
        pStartPt = pStartPt + SIP_TWO;
    }

    /*Get the buffer till first terminating CRLF
      In case of Request  it should be request line
      In case of Response it should be status line*/
    SIP_BOOL bHdrEnd = SIP_FALSE;
    SIP_CHAR* pTempPos = SIP_NULL;

    if (sipFindTerminatingCRLF(pStartPt, pEndPt, &pTempPos, &bHdrEnd) == SIP_FALSE)
    {
        SIP_DEBUG_WARNING(
                ESIPTRACE_MODDECODER, "DecCompleteMsg:Incomplete Message", SIP_ZERO, SIP_ZERO);
        return SIP_FALSE;
    }

    /*Update the End Point to the End of Start line*/
    pEndPt = pTempPos;
    pTempPos = SIP_NULL;

    /*Now determine for request line or status line*/
    /*Find the first token*/
    if (sipFindPreDelimiter(pStartPt, pEndPt, &pTempPos, SPACE) == SIP_FALSE)
    {
        SIP_DEBUG_WARNING(
                ESIPTRACE_MODDECODER, "DecCompleteMsg:Invalid Start Line", SIP_ZERO, SIP_ZERO);
        return SIP_FALSE;
    }

    SIP_INT32 eMsgType = sipGetMsgType(pStartPt);
    SIP_UINT32 nDecLen = pEndPt - pStartPt + SIP_ONE;

    if (eMsgType == SipMessage::REQ_TYPE)
    {
        m_pReqLine = new SipRequestLine();
        if (m_pReqLine == SIP_NULL)
        {
            SIP_DEBUG_WARNING(ESIPTRACE_MODDECODER, "DecCompleteMsg:Memory Allocation fail",
                    SIP_ZERO, SIP_ZERO);
            return SIP_FALSE;
        }

        if (m_pReqLine->DecodeRequestLine(pStartPt, nDecLen) == SIP_FALSE)
        {
            SIP_DEBUG_WARNING(ESIPTRACE_MODDECODER, "DecCompleteMsg:Req Line Decoding fail",
                    SIP_ZERO, SIP_ZERO);
            return SIP_FALSE;
        }
    }
    else if (eMsgType == SipMessage::RESP_TYPE)
    {
        m_pStatusLine = new SipStatusLine();
        if (m_pStatusLine == SIP_NULL)
        {
            SIP_DEBUG_WARNING(ESIPTRACE_MODDECODER, "DecCompleteMsg:Memory Allocation fail",
                    SIP_ZERO, SIP_ZERO);
            return SIP_FALSE;
        }

        if (m_pStatusLine->DecodeStatusLine(pStartPt, nDecLen) == SIP_FALSE)
        {
            SIP_DEBUG_WARNING(ESIPTRACE_MODDECODER, "DecCompleteMsg:Status Line Decoding fail",
                    SIP_ZERO, SIP_ZERO);
            return SIP_FALSE;
        }
    }
    else
    {
        SIP_DEBUG_WARNING(
                ESIPTRACE_MODDECODER, "DecCompleteMsg:Message Type Invalid", SIP_ZERO, SIP_ZERO);
        return SIP_FALSE;
    }

    /*set the message type in message object*/
    this->m_eSipMsgType = eMsgType;

    /*Update the start point with header start*/
    pStartPt = pEndPt + SIP_THREE;
    pTempPos = SIP_NULL;

    /*update the end point of the message*/
    pEndPt = pMsgBuff + nMsgBuffLen;

    /*Header Decoding*/

    // sipfrag message decoding failure as it may not have headers.
    if (pStartPt >= pEndPt)
    {
        return SIP_TRUE;
    }

    while ((pStartPt < pEndPt) && (bHdrEnd == SIP_FALSE))
    {
        /*find next terminating CRLF*/
        nDecLen = SIP_ZERO;
        // Fail condition to be added
        if (sipFindTerminatingCRLF(pStartPt, pEndPt, &pTempPos, &bHdrEnd) == SIP_FALSE)
        {
            SIP_DEBUG_WARNING(
                    ESIPTRACE_MODDECODER, "DecCompleteMsg:Incomplete Message", SIP_ZERO, SIP_ZERO);
            return SIP_FALSE;
        }

        nDecLen = pTempPos - pStartPt + SIP_ONE;

        SIP_CHAR* pszHdrName = SIP_NULL;
        SIP_CHAR* pszHdrBody = SIP_NULL;

        if (m_objHdrs->DecodeHdrs(pStartPt, nDecLen, (SIP_CHAR**)&pszHdrName,
                    (SIP_CHAR**)&pszHdrBody) == SIP_FALSE)
        {
#ifdef SIP_BADMESSAGE_PARSING
            SipBadHeader* pBadHdr = new SipBadHeader();
            if (pBadHdr != SIP_NULL)
            {
                pBadHdr->SetHeaderName(pszHdrName);
                pBadHdr->SetValue(pszHdrBody);
                if (m_pBadHdrList == SIP_NULL)
                {
                    m_pBadHdrList = new SipHeaderList(SipHeaderBase::TYPE_INVALID);
                }
                m_pBadHdrList->AddHeader((SipHeaderBase*)pBadHdr);
                pBadHdr->SipDelete();
            }
#else
            if (pszHdrName != SIP_NULL)
            {
                delete[] pszHdrName;
            }
            if (pszHdrBody != SIP_NULL)
            {
                delete[] pszHdrBody;
            }

            return SIP_FALSE;
#endif
        }
        else
        {
#ifdef SIP_BADMESSAGE_PARSING
            SIP_INT32 eHdrType = sipGetHdrType(pszHdrName);
            switch (eHdrType)
            {
                case SipHeaderBase::FROM:
                {
                    mbitMask = (mbitMask | MANDATORY_HDR_FROM);
                    break;
                }
                case SipHeaderBase::TO:
                {
                    mbitMask = (mbitMask | MANDATORY_HDR_TO);
                    break;
                }
                case SipHeaderBase::CALL_ID:
                {
                    mbitMask = (mbitMask | MANDATORY_HDR_CALL_ID);
                    break;
                }
                case SipHeaderBase::CSEQ:
                {
                    mbitMask = (mbitMask | MANDATORY_HDR_CSEQ);
                    break;
                }
                case SipHeaderBase::VIA:
                {
                    mbitMask = (mbitMask | MANDATORY_HDR_VIA);
                    break;
                }
                default:
                    break;
            }
#endif
        }
        if (pszHdrName != SIP_NULL)
        {
            delete[] pszHdrName;
        }
        if (pszHdrBody != SIP_NULL)
        {
            delete[] pszHdrBody;
        }

        pStartPt = pTempPos + SIP_THREE;
        pTempPos = SIP_NULL;
    }

    /*Check for Header end completion*/
    if (bHdrEnd != SIP_TRUE)
    {
        SIP_DEBUG_WARNING(
                ESIPTRACE_MODDECODER, "DecCompleteMsg:Incomplete Message", SIP_ZERO, SIP_ZERO);
        return SIP_FALSE;
    }

    /*Check for CSeq Method Match*/
    if (m_pReqLine != SIP_NULL)
    {
        SipCSeqHeader* pCSeq = (SipCSeqHeader*)m_objHdrs->getHdrObj(SipHeaderBase::CSEQ);
        if (pCSeq != SIP_NULL)
        {
            const SIP_CHAR* pszMethod_ReqLine = m_pReqLine->GetMethod();
            const SIP_CHAR* pszMethod_CSeq = pCSeq->GetMethod();
            /*Compare Both the methods*/
            if ((SipPf_Stricmp(pszMethod_ReqLine, pszMethod_CSeq)))
            {
                SIP_DEBUG_WARNING(ESIPTRACE_MODDECODER, "DecCompleteMsg:Cseq Method Mismatch",
                        SIP_ZERO, SIP_ZERO);

                pCSeq->SipDelete();
                return SIP_FALSE;
            }
            pCSeq->SipDelete();
        }
    }

    /*Body decoding*/
    /*Update the start point to the start of Message body*/
    pStartPt = pStartPt + SIP_TWO;

    SipUnknownHeader* pContentLen = GetUnknownHdrObj(SipHeaderBase::CONTENT_LENGTH);
    if (pContentLen == SIP_NULL)
    {
        SIP_TRACE_NORMAL(ESIPTRACE_MODDECODER, "No Body present in message", SIP_ZERO, SIP_ZERO);
        return SIP_TRUE;
    }

    SIP_INT32 nContentLen = 0;
    const SIP_CHAR* pszHdrValue = pContentLen->GetHeaderValue();
    if (pszHdrValue != SIP_NULL)
    {
        nContentLen = SipPf_Atoi(pszHdrValue);
    }

    pContentLen->SipDelete();

    if (nContentLen == SIP_ZERO)
    {
        SIP_TRACE_NORMAL(ESIPTRACE_MODDECODER, "No message body", SIP_ZERO, SIP_ZERO);
        return SIP_TRUE;
    }

    if (nContentLen > (pEndPt - pStartPt))
    {
        SIP_DEBUG_WARNING(
                ESIPTRACE_MODDECODER, "DecCompleteMsg:Incomplete Message", SIP_ZERO, SIP_ZERO);
        return SIP_FALSE;
    }

    /*Now Check for MIME or Single Body*/
    // If body list already present, free it
    if (m_pMsgBodyList != SIP_NULL)
    {
        m_pMsgBodyList->SipDelete();
        m_pMsgBodyList = SIP_NULL;
    }

    m_pMsgBodyList = new SipMsgBodyList();
    if (m_pMsgBodyList == SIP_NULL)
    {
        SIP_DEBUG_WARNING(ESIPTRACE_MODDECODER, "DecCompleteMsg:Memory Allocation failed", SIP_ZERO,
                SIP_ZERO);
        return SIP_FALSE;
    }

    // Check Content-Encoding header
    SipHeaderBase* pContEnc = m_objHdrs->getHdrObj(SipHeaderBase::CONTENT_ENCODING, SIP_ZERO);
    if (pContEnc != SIP_NULL)
    {
        const SIP_CHAR* encType = pContEnc->GetValue();
        pContEnc->SipDelete();
        if (encType != SIP_NULL)
        {
            if (SipPf_Stricmp(encType, "gzip") == SIP_ZERO)
            {
                if (m_pMsgBodyList->DecodeSingleBody(pStartPt, pEndPt) == SIP_FALSE)
                {
                    SIP_DEBUG_WARNING(ESIPTRACE_MODDECODER,
                            "DecCompleteMsg, GZIP Decoding single body failed", SIP_ZERO, SIP_ZERO);
                    m_pMsgBodyList->SipDelete();
                    m_pMsgBodyList = SIP_NULL;
                }
                return SIP_FALSE;
            }
        }
    }

    SipContentTypeHeader* pContentType =
            (SipContentTypeHeader*)m_objHdrs->getHdrObj(SipHeaderBase::CONTENT_TYPE);
    if (pContentType == SIP_NULL)
    {
        SIP_DEBUG_WARNING(
                ESIPTRACE_MODDECODER, "DecCompleteMsg: No Content-Type header", SIP_ZERO, SIP_ZERO);
        if (m_pMsgBodyList->DecodeSingleBody(pStartPt, pEndPt) == SIP_FALSE)
        {
            SIP_DEBUG_WARNING(ESIPTRACE_MODDECODER, "DecCompleteMsg:Decode single body fail",
                    SIP_ZERO, SIP_ZERO);
            m_pMsgBodyList->SipDelete();
            m_pMsgBodyList = SIP_NULL;
            return SIP_FALSE;
        }
    }
    else
    {
        const SIP_CHAR* pszMType = pContentType->GetMediaType();
        if (pszMType == SIP_NULL)
        {
            SIP_TRACE_NORMAL(ESIPTRACE_MODDECODER, "Wronge Content Type", SIP_ZERO, SIP_ZERO);
            pContentType->SipDelete();
            m_pMsgBodyList->SipDelete();
            m_pMsgBodyList = SIP_NULL;
            return SIP_FALSE;
        }

        const SIP_CHAR* pszSubMType = pContentType->GetSubMediaType();
        if (pszSubMType == SIP_NULL)
        {
            SIP_TRACE_NORMAL(ESIPTRACE_MODDECODER, "Wronge Content Type", SIP_ZERO, SIP_ZERO);
            pContentType->SipDelete();
            m_pMsgBodyList->SipDelete();
            m_pMsgBodyList = SIP_NULL;
            return SIP_FALSE;
        }

        if (SipPf_Strstr(pszSubMType, "message-summary") != SIP_NULL)
        {
            if (m_pMsgBodyList->DecodeMessageSummaryBody(pStartPt, pEndPt) == SIP_FALSE)
            {
                SIP_DEBUG_WARNING(ESIPTRACE_MODDECODER,
                        "DecCompleteMsg:Decode message summary fail", SIP_ZERO, SIP_ZERO);
                m_pMsgBodyList->SipDelete();
                m_pMsgBodyList = SIP_NULL;
                return SIP_FALSE;
            }
        }
        else
        {
            SIP_BOOL bSingleBody = (SIP_BOOL)SipPf_Stricmp(pszMType, MULTIPART);

            /*bSingleBody will be false if it is a MIME body*/
            if (bSingleBody == SIP_FALSE)
            {
                SIP_CHAR* pszBoundary = pContentType->GetBoundary();
                if (pszBoundary == SIP_NULL)
                {
                    SIP_TRACE_NORMAL(ESIPTRACE_MODDECODER, "No boundary in Content Type Hdr",
                            SIP_ZERO, SIP_ZERO);

                    pContentType->SipDelete();
                    m_pMsgBodyList->SipDelete();
                    m_pMsgBodyList = SIP_NULL;
                    return SIP_FALSE;
                }
                if (m_pMsgBodyList->DecodeMIMEBody(pStartPt, pEndPt, pszBoundary) == SIP_FALSE)
                {
                    SIP_DEBUG_WARNING(ESIPTRACE_MODDECODER, "DecCompleteMsg:Hdr Decoding fail",
                            SIP_ZERO, SIP_ZERO);
                    delete[] pszBoundary;
                    pContentType->SipDelete();
                    m_pMsgBodyList->SipDelete();
                    m_pMsgBodyList = SIP_NULL;
                    return SIP_FALSE;
                }
                delete[] pszBoundary;
            }
            else
            {
                if (m_pMsgBodyList->DecodeSingleBody(pStartPt, pEndPt) == SIP_FALSE)
                {
                    SIP_DEBUG_WARNING(ESIPTRACE_MODDECODER, "DecCompleteMsg:Hdr Decoding fail",
                            SIP_ZERO, SIP_ZERO);
                    m_pMsgBodyList->SipDelete();
                    m_pMsgBodyList = SIP_NULL;
                    return SIP_FALSE;
                }
            }
        }

        pContentType->SipDelete();
    }

    return SIP_TRUE;
}

/*Set Content-length Header*/
SIP_VOID SipMessage::SetContentLengthHdr(SIP_UINT32 nLen, SIP_UINT32 nMsgOptions)
{
    SipUnknownHeader* pUnknown = new SipUnknownHeader();
    if (pUnknown != SIP_NULL)
    {
        SIP_CHAR szLen[SIP_CONTLEN_LEN] = {0};
        SipPf_Sprintf(szLen, (SIP_CHAR*)"%u", nLen);

        const SIP_CHAR* pszHdrName =
                ((nMsgOptions & ESIPMSGOPT_ENCSHORTFORM) == ESIPMSGOPT_ENCSHORTFORM)
                ? "l"
                : "Content-Length";

        pUnknown->SetHeaderName(pszHdrName);
        pUnknown->SetHeaderValue(szLen);
        SetHeader(pUnknown);

        pUnknown->SipDelete();
    }
}

/*Get Unknown header using name of the header*/
SipUnknownHeader* SipMessage::GetUnknownHdrObj(const SIP_CHAR* pszHdrName)
{
    if (pszHdrName == SIP_NULL)
    {
        return SIP_NULL;
    }

    SipHeaderList* pList = GetHdrList(SipHeaderBase::UNKNOWN);
    if (pList != SIP_NULL)
    {
        for (SIP_UINT32 i = 0; i < pList->GetSize(); i++)
        {
            SipUnknownHeader* pUnknown = (SipUnknownHeader*)pList->GetObj(i);
            if (pUnknown != SIP_NULL)
            {
                const SIP_CHAR* pszUKHdrName = pUnknown->GetHeaderName();
                if ((pszUKHdrName != SIP_NULL) &&
                        (SipPf_Stricmp(pszUKHdrName, pszHdrName) == SIP_ZERO))
                {
                    pList->SipDelete();
                    return pUnknown;
                }
                pUnknown->SipDelete();
            }
        }
        pList->SipDelete();
    }
    return SIP_NULL;
}

/*Get Unknown header using type of the header, Only Content-Length/l supported as of now*/
SipUnknownHeader* SipMessage::GetUnknownHdrObj(SIP_INT32 eType)
{
    SipHeaderList* pList = GetHdrList(SipHeaderBase::UNKNOWN);
    if (pList != SIP_NULL)
    {
        for (SIP_UINT32 i = 0; i < pList->GetSize(); i++)
        {
            SipUnknownHeader* pUnknown = (SipUnknownHeader*)pList->GetObj(i);
            if (pUnknown != SIP_NULL)
            {
                const SIP_CHAR* pszUKHdrName = pUnknown->GetHeaderName();
                switch (eType)
                {
                    case SipHeaderBase::CONTENT_LENGTH:
                    {
                        if (pszUKHdrName != SIP_NULL)
                        {
                            if ((!SipPf_Stricmp(pszUKHdrName, "Content-Length")) ||
                                    (!SipPf_Stricmp(pszUKHdrName, "l")))
                            {
                                pList->SipDelete();
                                return pUnknown;
                            }
                        }
                    }
                    break;

                        /*TODO - All required headers to be added on need basis.*/
                        /*other option - Use SipMessage::GetUnknownHdrObj(SIP_CHAR* strHdrName)*/
                    default:
                        break;
                }
                pUnknown->SipDelete();
            }
        }
        pList->SipDelete();
    }
    return SIP_NULL;
}

/* Adjust Content-Length header : move it to the last unknown header's position */
void SipMessage::AdjustContentLengthHdr()
{
    SipHeaderList* pList = GetHdrList(SipHeaderBase::UNKNOWN);

    if (pList == SIP_NULL)
    {
        return;
    }

    SIP_UINT32 nCount = pList->GetSize();

    if (nCount <= 1)
    {
        pList->SipDelete();
        return;
    }

    /* Content-Length header is the last one, then no action */
    nCount -= 1;

    SipUnknownHeader* pContentLength = SIP_NULL;

    for (SIP_UINT32 i = 0; i < nCount; i++)
    {
        SipUnknownHeader* pUnknown = SIP_DYNAMIC_CAST(SipUnknownHeader*, pList->GetObj(i));

        if (pUnknown != SIP_NULL)
        {
            const SIP_CHAR* pszName = pUnknown->GetHeaderName();

            if (pszName != SIP_NULL)
            {
                if (SipPf_Stricmp(pszName, "Content-Length") == 0 ||
                        SipPf_Stricmp(pszName, "l") == 0)
                {
                    pContentLength = pUnknown;

                    pList->RemoveHdr(i);
                    break;
                }
            }

            pUnknown->SipDelete();
        }
    }

    if (pContentLength != SIP_NULL)
    {
        pList->AddHeader(pContentLength);

        pContentLength->SipDelete();
    }

    pList->SipDelete();
}

void SipMessage::RemoveAllMessageBodies()
{
    if (m_pMsgBodyList != SIP_NULL)
    {
        m_pMsgBodyList->SipDelete();
        m_pMsgBodyList = SIP_NULL;
    }
}

SIP_UINT16 SipMessage::GetStatusCode() const
{
    if (m_eSipMsgType != SipMessage::RESP_TYPE)
    {
        return SIP_SC_INVALID;
    }

    if (m_pStatusLine == SIP_NULL)
    {
        return SIP_SC_INVALID;
    }

    return m_pStatusLine->GetStatusCodeAsInt();
}

SipHeaderBase* SipMessage::GetHeader(SIP_INT32 nType, SIP_UINT32 nIndex)
{
    SipHeaderList* pHdrList = GetHdrList(nType);
    SipHeaderBase* pHdr = SIP_NULL;

    if (pHdrList == SIP_NULL)
    {
        pHdr = GetHdrObj(nType);
    }
    else
    {
        pHdr = pHdrList->GetObj(nIndex);
        pHdrList->SipDelete();
    }
    return pHdr;
}

SIP_INT32 SipMessage::GetHeaderCount(SIP_INT32 nType)
{
    SipHeaderBase* pHdr = GetHdrList(nType);
    if (pHdr == SIP_NULL)
    {
        pHdr = GetHdrObj(nType);
        if (pHdr == SIP_NULL)
        {
            return SIP_ZERO;
        }
        pHdr->SipDelete();
        return SIP_ONE;
    }
    SIP_INT32 nHCount = ((SipHeaderList*)pHdr)->GetSize();
    pHdr->SipDelete();
    return nHCount;
}
