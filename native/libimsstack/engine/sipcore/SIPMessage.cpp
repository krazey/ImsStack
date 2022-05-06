/*
    Author
    <table>
    date      author                    description
    --------  --------------            ----------
    20090326  toastops@                 Created
    </table>

    Description

*/

#include "ServiceMemory.h"
#include "SIPPrivate.h"
#include "SIPMessage.h"

__IMS_TRACE_TAG_SIP__;

PUBLIC
SIPMessage::SIPMessage(IN IMS_SINT32 nType_ /* = ISipMessage::TYPE_REQUEST */) :
        nType(nType_),
        strRequestURI(AString::ConstNull()),
        bBodyPartParsed(IMS_FALSE),
        pstMessage(IMS_NULL)
{
    //---------------------------------------------------------------------------------------------

    pstMessage = SIPStack::CreateMessage(nType);
}

PUBLIC
SIPMessage::SIPMessage(IN SipMessage* pstMessage_) :
        nType(TYPE_ANY),
        strRequestURI(AString::ConstNull()),
        bBodyPartParsed(IMS_FALSE),
        pstMessage(pstMessage_)
{
    Init(IMS_FALSE);
}

PUBLIC
SIPMessage::SIPMessage(IN SipMessage* pstMessage_, IN IMS_BOOL bMessageClone) :
        nType(TYPE_ANY),
        strRequestURI(AString::ConstNull()),
        bBodyPartParsed(IMS_FALSE),
        pstMessage(pstMessage_)
{
    //---------------------------------------------------------------------------------------------

    Init(bMessageClone);
}

PUBLIC VIRTUAL SIPMessage::~SIPMessage()
{
    //---------------------------------------------------------------------------------------------

    SIPStack::FreeMessage(pstMessage);

    if (!objBodyParts.IsEmpty())
    {
        for (IMS_UINT32 i = 0; i < objBodyParts.GetSize(); ++i)
        {
            SIPMessageBodyPart* pBodyPart = objBodyParts.GetAt(i);

            if (pBodyPart != IMS_NULL)
                delete pBodyPart;
        }

        objBodyParts.Clear();
    }

#ifdef __IMS_SIP_DEBUG__
    IMS_TRACE_D("Destructor :: SIPMessage", 0, 0, 0);
#endif
}

/*

Remarks

*/
PUBLIC
SIPMessage& SIPMessage::operator=(IN CONST SIPMessage& objRHS)
{
    //---------------------------------------------------------------------------------------------

    if (this != &objRHS)
    {
        SIPStack::FreeMessage(pstMessage);

        nType = objRHS.nType;
        objMethod = objRHS.objMethod;
        strRequestURI = objRHS.strRequestURI;
        objStatusCode = objRHS.objStatusCode;
        objUnknownHeaders = objRHS.objUnknownHeaders;
        bBodyPartParsed = objRHS.bBodyPartParsed;

        if (!objBodyParts.IsEmpty())
        {
            for (IMS_UINT32 i = 0; i < objBodyParts.GetSize(); ++i)
            {
                SIPMessageBodyPart* pBodyPart = objBodyParts.GetAt(i);

                if (pBodyPart != IMS_NULL)
                    delete pBodyPart;
            }

            objBodyParts.Clear();
        }

        for (IMS_UINT32 i = 0; i < objRHS.objBodyParts.GetSize(); ++i)
        {
            const SIPMessageBodyPart* pBodyPart = objRHS.objBodyParts.GetAt(i);
            SIPMessageBodyPart* pNewBodyPart =
                    DYNAMIC_CAST(SIPMessageBodyPart*, pBodyPart->Clone());

            if (pNewBodyPart != IMS_NULL)
            {
                objBodyParts.Append(pNewBodyPart);
            }
        }

        pstMessage = objRHS.pstMessage;

        if (pstMessage != IMS_NULL)
        {
            SIPStack::AddReference(pstMessage);
        }
    }

    return (*this);
}

/*

Remarks

*/
PUBLIC VIRTUAL void SIPMessage::Destroy()
{
    //---------------------------------------------------------------------------------------------

    delete this;
}

/*

Remarks

*/
PUBLIC VIRTUAL ISipMessage* SIPMessage::Clone() const
{
    SIPMessage* pNewMessage = new SIPMessage(nType);

    //---------------------------------------------------------------------------------------------

    if (pNewMessage == IMS_NULL)
    {
        SIPPrivate::SetLastError(SipError::NO_MEMORY);
        return IMS_NULL;
    }

    pNewMessage->objMethod = objMethod;
    pNewMessage->strRequestURI = strRequestURI;
    pNewMessage->objStatusCode = objStatusCode;
    pNewMessage->objUnknownHeaders = objUnknownHeaders;
    pNewMessage->bBodyPartParsed = bBodyPartParsed;

    for (IMS_UINT32 i = 0; i < objBodyParts.GetSize(); ++i)
    {
        const SIPMessageBodyPart* pBodyPart = objBodyParts.GetAt(i);
        SIPMessageBodyPart* pNewBodyPart = DYNAMIC_CAST(SIPMessageBodyPart*, pBodyPart->Clone());

        if (pNewBodyPart == IMS_NULL)
        {
            delete pNewMessage;

            SIPPrivate::SetLastError(SipError::NO_MEMORY);
            return IMS_NULL;
        }

        if (!pNewMessage->objBodyParts.Append(pNewBodyPart))
        {
            delete pNewBodyPart;
            delete pNewMessage;

            SIPPrivate::SetLastError(SipError::LIST_OPERATION_FAILED);
            return IMS_NULL;
        }
    }

    if (pstMessage != IMS_NULL)
    {
        SIPStack::FreeMessage(pNewMessage->pstMessage);
        pNewMessage->pstMessage = SIPStack::CloneMessage(pstMessage);

        if (pNewMessage->pstMessage == IMS_NULL)
        {
            delete pNewMessage;

            SIPPrivate::SetLastError(SipError::NO_MEMORY);
            return IMS_NULL;
        }
    }

    SIPPrivate::SetLastError(SipError::NO_ERROR);
    return pNewMessage;
}

/*

Remarks

*/
PUBLIC VIRTUAL IMS_RESULT SIPMessage::AddHeader(IN IMS_SINT32 nType, IN CONST AString& strValue,
        IN CONST AString& strName /* = AString::ConstNull() */)
{
    //---------------------------------------------------------------------------------------------

    SIPPrivate::SetLastError(SipError::NO_ERROR);

    if (SIPStack::IsUnknownHeader(nType, strName))
    {
        return objUnknownHeaders.AddHeader(strName, strValue);
    }
    else
    {
        SipHeaderBase* pstHeader = SIPStack::DecodeHeader(nType, strName, strValue);

        if (pstHeader == IMS_NULL)
        {
            SIPPrivate::SetLastError(SipError::GENERAL_ERROR);
            return IMS_FAILURE;
        }

        if (!SIPStack::AppendHeader(pstHeader, pstMessage))
        {
            SIPStack::FreeHeaderEx(pstHeader);

            SIPPrivate::SetLastError(SipError::GENERAL_ERROR);
            return IMS_FAILURE;
        }

        SIPStack::FreeHeaderEx(pstHeader);

        return IMS_SUCCESS;
    }
}

/*

Remarks

*/
PUBLIC VIRTUAL IMS_UINT32 SIPMessage::GetCSeqNumber() const
{
    //---------------------------------------------------------------------------------------------

    return SIPStack::GetCSeqNumber(pstMessage);
}

/*

Remarks

*/
PUBLIC VIRTUAL AString SIPMessage::GetHeader(IN IMS_SINT32 nType, IN IMS_SINT32 nIndex /* = 0 */,
        IN CONST AString& strName /* = AString::ConstNull() */) const
{
    //---------------------------------------------------------------------------------------------

    SIPPrivate::SetLastError(SipError::NO_ERROR);

    if (SIPStack::IsUnknownHeader(nType, strName))
    {
        return objUnknownHeaders.GetHeader(strName, nIndex);
    }
    else
    {
        if (SIPStack::GetHeaderCount(pstMessage, nType) == 0)
        {
            return AString::ConstNull();
        }

        SipHeaderBase* pstHeader = SIPStack::GetHeader(pstMessage, nType, nIndex);
        AString strHeader;

        SIPStack::EncodeHeaderBody(pstHeader, IMS_TRUE, strHeader);

        SIPStack::FreeHeaderEx(pstHeader);

        return strHeader;
    }
}

/*

Remarks

*/
PUBLIC VIRTUAL IMS_SINT32 SIPMessage::GetHeaderCount(
        IN IMS_SINT32 nType, IN CONST AString& strName /* = AString::ConstNull() */) const
{
    //---------------------------------------------------------------------------------------------

    SIPPrivate::SetLastError(SipError::NO_ERROR);

    if (pstMessage == IMS_NULL)
        return 0;

    if (SIPStack::IsUnknownHeader(nType, strName))
    {
        return objUnknownHeaders.GetHeaderCount(strName);
    }
    else
    {
        return SIPStack::GetHeaderCount(pstMessage, nType);
    }
}

/*

Remarks

*/
PUBLIC VIRTUAL IMSList<AString> SIPMessage::GetHeaders(
        IN IMS_SINT32 nType, IN CONST AString& strName /* = AString::ConstNull() */) const
{
    //---------------------------------------------------------------------------------------------

    SIPPrivate::SetLastError(SipError::NO_ERROR);

    if (SIPStack::IsUnknownHeader(nType, strName))
    {
        return objUnknownHeaders.GetHeaders(strName);
    }
    else
    {
        IMSList<AString> objHeaders;
        IMS_SINT32 nHCount = SIPStack::GetHeaderCount(pstMessage, nType);
        AString strHeader;

        for (IMS_SINT32 i = 0; i < nHCount; ++i)
        {
            SipHeaderBase* pstHeader = SIPStack::GetHeader(pstMessage, nType, i);

            if (SIPStack::IsValidHeader(pstHeader))
            {
                SIPStack::EncodeHeaderBody(pstHeader, IMS_TRUE, strHeader);

                if (!objHeaders.Append(strHeader))
                {
                    SIPStack::FreeHeaderEx(pstHeader);
                    return IMSList<AString>();  // ???, throw exception
                }
            }

            SIPStack::FreeHeaderEx(pstHeader);
        }

        return objHeaders;
    }
}

/*

Remarks

*/
PUBLIC VIRTUAL IMS_RESULT SIPMessage::PrependHeader(IN IMS_SINT32 nType, IN CONST AString& strValue,
        IN CONST AString& strName /* = AString::ConstNull() */)
{
    //---------------------------------------------------------------------------------------------

    SIPPrivate::SetLastError(SipError::NO_ERROR);

    if (SIPStack::IsUnknownHeader(nType, strName))
    {
        return objUnknownHeaders.PrependHeader(strName, strValue);
    }
    else
    {
        SipHeaderBase* pstHeader = SIPStack::DecodeHeader(nType, strName, strValue);

        if (pstHeader == IMS_NULL)
        {
            SIPPrivate::SetLastError(SipError::GENERAL_ERROR);
            return IMS_FAILURE;
        }

        if (!SIPStack::PrependHeader(pstHeader, pstMessage))
        {
            SIPStack::FreeHeaderEx(pstHeader);

            SIPPrivate::SetLastError(SipError::GENERAL_ERROR);
            return IMS_FAILURE;
        }

        SIPStack::FreeHeaderEx(pstHeader);

        return IMS_SUCCESS;
    }
}

/*

Remarks

*/
PUBLIC VIRTUAL void SIPMessage::RemoveHeader(
        IN IMS_SINT32 nType, IN CONST AString& strName /* = AString::ConstNull() */)
{
    //---------------------------------------------------------------------------------------------

    SIPPrivate::SetLastError(SipError::NO_ERROR);

    if (SIPStack::IsUnknownHeader(nType, strName))
        objUnknownHeaders.RemoveHeader(strName);
    else
        SIPStack::RemoveHeader(nType, pstMessage);
}

/*

Remarks

*/
PUBLIC VIRTUAL IMS_RESULT SIPMessage::SetHeader(IN IMS_SINT32 nType, IN CONST AString& strValue,
        IN CONST AString& strName /* = AString::ConstNull() */)
{
    //---------------------------------------------------------------------------------------------

    SIPPrivate::SetLastError(SipError::NO_ERROR);

    if (SIPStack::IsUnknownHeader(nType, strName))
    {
        return objUnknownHeaders.SetHeader(strName, strValue);
    }
    else
    {
        SipHeaderBase* pstHeader = SIPStack::DecodeHeader(nType, strName, strValue);

        if (pstHeader == IMS_NULL)
        {
            SIPPrivate::SetLastError(SipError::GENERAL_ERROR);
            return IMS_FAILURE;
        }

        if (!SIPStack::SetHeader(pstHeader, pstMessage))
        {
            SIPStack::FreeHeaderEx(pstHeader);

            SIPPrivate::SetLastError(SipError::GENERAL_ERROR);
            return IMS_FAILURE;
        }

        SIPStack::FreeHeaderEx(pstHeader);

        return IMS_SUCCESS;
    }
}

/*

Remarks

*/
PUBLIC VIRTUAL ISipMessageBodyPart* SIPMessage::CreateBodyPart()
{
    SIPMessageBodyPart* pBodyPart = new SIPMessageBodyPart(IMS_FALSE);

    //---------------------------------------------------------------------------------------------

    SIPPrivate::SetLastError(SipError::NO_ERROR);

    if (pBodyPart == IMS_NULL)
    {
        SIPPrivate::SetLastError(SipError::NO_MEMORY);
        return IMS_NULL;
    }

    if (!objBodyParts.Append(pBodyPart))
    {
        delete pBodyPart;

        SIPPrivate::SetLastError(SipError::LIST_OPERATION_FAILED);
        return IMS_NULL;
    }

    return pBodyPart;
}

/*

Remarks

*/
PUBLIC VIRTUAL ISipMessageBodyPart* SIPMessage::CreateSdpBodyPart()
{
    SIPMessageBodyPart* pBodyPart = new SIPMessageBodyPart(IMS_TRUE);

    //---------------------------------------------------------------------------------------------

    SIPPrivate::SetLastError(SipError::NO_ERROR);

    if (pBodyPart == IMS_NULL)
    {
        SIPPrivate::SetLastError(SipError::NO_MEMORY);
        return IMS_NULL;
    }

    if (!objBodyParts.Prepend(pBodyPart))
    {
        delete pBodyPart;

        SIPPrivate::SetLastError(SipError::LIST_OPERATION_FAILED);
        return IMS_NULL;
    }

    return pBodyPart;
}

/*

Remarks

*/
PUBLIC VIRTUAL IMSList<ISipMessageBodyPart*> SIPMessage::GetBodyParts() const
{
    //---------------------------------------------------------------------------------------------

    if (objBodyParts.IsEmpty())
    {
        return IMSList<ISipMessageBodyPart*>();
    }

    IMSList<ISipMessageBodyPart*> objSIPBodyParts;

    for (IMS_UINT32 i = 0; i < objBodyParts.GetSize(); ++i)
    {
        SIPMessageBodyPart* pBodyPart = objBodyParts.GetAt(i);

        if (!pBodyPart->IsSDPBodyPart())
        {
            objSIPBodyParts.Append(pBodyPart);
        }
    }

    return objSIPBodyParts;
}

/*

Remarks

*/
PUBLIC VIRTUAL ISipMessageBodyPart* SIPMessage::GetSdpBodyPart() const
{
    //---------------------------------------------------------------------------------------------

    if (objBodyParts.IsEmpty())
        return IMS_NULL;

    for (IMS_UINT32 i = 0; i < objBodyParts.GetSize(); ++i)
    {
        SIPMessageBodyPart* pBodyPart = objBodyParts.GetAt(i);

        if (pBodyPart->IsSDPBodyPart())
        {
            return pBodyPart;
        }
    }

    return IMS_NULL;
}

/*

Remarks

*/
PUBLIC VIRTUAL IMSList<ISipMessageBodyPart*> SIPMessage::GetSdpBodyParts() const
{
    //---------------------------------------------------------------------------------------------

    if (objBodyParts.IsEmpty())
    {
        return IMSList<ISipMessageBodyPart*>();
    }

    IMSList<ISipMessageBodyPart*> objSIPBodyParts;

    for (IMS_UINT32 i = 0; i < objBodyParts.GetSize(); ++i)
    {
        SIPMessageBodyPart* pBodyPart = objBodyParts.GetAt(i);

        if (pBodyPart->IsSDPBodyPart())
        {
            objSIPBodyParts.Append(pBodyPart);
        }
    }

    return objSIPBodyParts;
}

/*

Remarks

*/
PUBLIC VIRTUAL IMS_RESULT SIPMessage::CopyHeadersAndBodyParts(IN CONST ISipMessage* piSIPMsg)
{
    const SIPMessage* pSIPMsg = DYNAMIC_CAST(const SIPMessage*, piSIPMsg);

    //---------------------------------------------------------------------------------------------

    if (pSIPMsg == IMS_NULL)
    {
        SIPPrivate::SetLastError(SipError::ILLEGAL_ARGUMENT);
        return IMS_FAILURE;
    }

    // Overwrites any known headers
    if (!SIPStack::OverwriteHeaders(pSIPMsg->pstMessage, pstMessage))
    {
        SIPPrivate::SetLastError(SipError::GENERAL_ERROR);
        return IMS_FAILURE;
    }

    // Overwrites any unknown headers
    if (!objUnknownHeaders.OverwriteHeaders(pSIPMsg->objUnknownHeaders))
    {
        SIPPrivate::SetLastError(SipError::GENERAL_ERROR);
        return IMS_FAILURE;
    }

    // Copy any message body parts if present
    if (pSIPMsg->objBodyParts.IsEmpty())
    {
        SIPPrivate::SetLastError(SipError::NO_ERROR);
        return IMS_SUCCESS;
    }

    for (IMS_UINT32 i = 0; i < pSIPMsg->objBodyParts.GetSize(); ++i)
    {
        const SIPMessageBodyPart* pBodyPart = pSIPMsg->objBodyParts.GetAt(i);
        SIPMessageBodyPart* pNewBodyPart = new SIPMessageBodyPart(pBodyPart->IsSDPBodyPart());

        if (pNewBodyPart == IMS_NULL)
        {
            return IMS_FAILURE;
        }

        (*pNewBodyPart) = (*pBodyPart);

        if (!objBodyParts.Append(pNewBodyPart))
        {
            delete pNewBodyPart;

            SIPPrivate::SetLastError(SipError::LIST_OPERATION_FAILED);
            return IMS_FAILURE;
        }
    }

    SIPPrivate::SetLastError(SipError::NO_ERROR);
    return IMS_SUCCESS;
}

/*

Remarks

*/
PUBLIC VIRTUAL IMS_BOOL SIPMessage::IsHeaderPresent(
        IN IMS_SINT32 nType, IN CONST AString& strName /* = AString::ConstNull() */) const
{
    //---------------------------------------------------------------------------------------------

    if (SIPStack::IsUnknownHeader(nType, strName))
    {
        return objUnknownHeaders.IsHeaderPresent(strName);
    }

    return SIPStack::IsHeaderPresent(pstMessage, nType);
}

/*

Remarks

*/
PUBLIC VIRTUAL IMS_BOOL SIPMessage::IsMessageRpr() const
{
    //---------------------------------------------------------------------------------------------

    return SIPStack::IsMessageRPR(pstMessage);
}

/*

Remarks

*/
PUBLIC VIRTUAL IMS_BOOL SIPMessage::IsOptionRequired(IN CONST AString& strOption) const
{
    //---------------------------------------------------------------------------------------------

    return SIPStack::IsOptionRequired(pstMessage, strOption);
}

/*

Remarks

*/
PUBLIC VIRTUAL IMS_BOOL SIPMessage::IsOptionSupported(IN CONST AString& strOption) const
{
    //---------------------------------------------------------------------------------------------

    return SIPStack::IsOptionSupported(pstMessage, strOption);
}

/*

Remarks

*/
PUBLIC VIRTUAL void SIPMessage::RemoveBodyParts()
{
    //---------------------------------------------------------------------------------------------

    if (!objBodyParts.IsEmpty())
    {
        for (IMS_UINT32 i = 0; i < objBodyParts.GetSize(); ++i)
        {
            SIPMessageBodyPart* pBodyPart = objBodyParts.GetAt(i);

            if (pBodyPart != IMS_NULL)
                delete pBodyPart;
        }

        objBodyParts.Clear();

        SIPStack::RemoveAllMessageBodies(pstMessage);
    }
}

/*

Remarks

*/
PUBLIC VIRTUAL ByteArray SIPMessage::ToByteArray(IN IMS_SINT32 nOptions /* = OPT_ALL */) const
{
    ByteArray objMessage;

    //---------------------------------------------------------------------------------------------

    if (!SIPStack::EncodePartialMessage(pstMessage, nOptions, objMessage))
    {
        return ByteArray::ConstNull();
    }

    return objMessage;
}

/*

Remarks

*/
PUBLIC
SIPMessageBodyPart* SIPMessage::GetBodyPart() const
{
    //---------------------------------------------------------------------------------------------

    if (objBodyParts.IsEmpty())
        return IMS_NULL;

    return objBodyParts.GetAt(0);
}

/*

Remarks

*/
PUBLIC
IMS_RESULT SIPMessage::SetRequestURI(IN CONST AString& strURI)
{
    //---------------------------------------------------------------------------------------------

    if (strURI.IsNULL() || strURI.IsEmpty())
    {
        // Remove Request-URI if set previously
        strRequestURI = AString::ConstNull();
    }
    else
    {
        SipAddrSpec* pstAddrSpec = SIPStack::DecodeAddrSpec(strURI);

        if (pstAddrSpec == IMS_NULL)
        {
            SIPPrivate::SetLastError(SipError::ILLEGAL_ARGUMENT);
            return IMS_FAILURE;
        }

        SIPStack::EncodeAddrSpec(pstAddrSpec, IMS_TRUE, strRequestURI);

        SIPStack::FreeAddrSpec(pstAddrSpec);
    }

    SIPPrivate::SetLastError(SipError::NO_ERROR);

    return IMS_SUCCESS;
}

/*

Remarks

*/
PUBLIC
void SIPMessage::UpdateRequestURI()
{
    SipAddrSpec* pstAddrSpec = SIPStack::GetRequestUri(pstMessage);

    //---------------------------------------------------------------------------------------------

    if (pstAddrSpec == IMS_NULL)
    {
        // Remove Request-URI if set previously
        strRequestURI = AString::ConstNull();
    }
    else
    {
        SIPStack::EncodeAddrSpec(pstAddrSpec, IMS_TRUE, strRequestURI);
        SIPStack::FreeAddrSpec(pstAddrSpec);
    }
}

/*

Remarks

*/
PUBLIC
IMS_BOOL SIPMessage::CreateBodyParts()
{
    //---------------------------------------------------------------------------------------------

    if (bBodyPartParsed)
        return IMS_TRUE;

    if (objBodyParts.IsEmpty())
    {
        bBodyPartParsed = IMS_TRUE;
        return IMS_TRUE;
    }

    // If message does not include a MIME body, do not parse the message body
    if (!SIPStack::HasMIMEMessageBody(pstMessage))
    {
        bBodyPartParsed = IMS_TRUE;

        if (SIPStack::IsMessageBodyCompressed(pstMessage))
        {
            if (!SIPStack::UncompressMessageBody(pstMessage))
            {
                IMS_TRACE_E(0, "Uncompressing SIP message body failed", 0, 0, 0);
                return IMS_FALSE;
            }

            // Clear the previous message body parts ...
            while (!objBodyParts.IsEmpty())
            {
                SIPMessageBodyPart* pBodyPart = objBodyParts.GetAt(0);

                delete pBodyPart;

                objBodyParts.RemoveAt(0);
            }

            if (!ExtractBodyParts())
            {
                return IMS_FALSE;
            }
        }

        return IMS_TRUE;
    }

    if (!SIPStack::DecodeMessageBody(pstMessage))
    {
        IMS_TRACE_E(0, "Parsing SIP message body failed", 0, 0, 0);
        return IMS_FALSE;
    }

    // Clear the previous message body parts ...
    while (!objBodyParts.IsEmpty())
    {
        SIPMessageBodyPart* pBodyPart = objBodyParts.GetAt(0);

        delete pBodyPart;

        objBodyParts.RemoveAt(0);
    }

    if (!ExtractBodyParts())
    {
        return IMS_FALSE;
    }

    bBodyPartParsed = IMS_TRUE;

    return IMS_TRUE;
}

/*

Remarks

*/
PUBLIC
IMS_BOOL SIPMessage::FormMessage()
{
    //---------------------------------------------------------------------------------------------

    if (pstMessage == IMS_NULL)
        return IMS_FALSE;

    /*
     * Set properties
     *    In case of CSeq header, it will set by the SIP transaction with a correct value.
     */
    if (nType == TYPE_REQUEST)
    {
        if (!SIPStack::SetRequestLine(objMethod.ToString(), strRequestURI, pstMessage))
        {
            return IMS_FALSE;
        }
    }
    else
    {
        if (!SIPStack::SetStatusLine(
                    objStatusCode.ToInt(), objStatusCode.GetReasonPhrase(), pstMessage))
        {
            return IMS_FALSE;
        }
    }

    // Set unknown headers
    if (objUnknownHeaders.GetCount() > 0)
    {
        AString strName;
        AString strValue;
        IMS_BOOL bCompactForm =
                ((SIPPrivate::GetEncodingOptions() & SIPPrivate::OPT_E_SHORTFORM) != 0);

        for (IMS_SINT32 i = objUnknownHeaders.GetCount() - 1; i >= 0; --i)
        {
            strName = objUnknownHeaders.GetHeaderName(i, bCompactForm);
            strValue = objUnknownHeaders.GetHeaderBodys(i);

            (void)SIPStack::PrependUnknownHeader(strName, strValue, pstMessage);
        }
    }

    // Set message body parts
    if (!objBodyParts.IsEmpty())
    {
        for (IMS_UINT32 i = 0; i < objBodyParts.GetSize(); ++i)
        {
            SIPMessageBodyPart* pBodyPart = objBodyParts.GetAt(i);

            if (pBodyPart == IMS_NULL)
                continue;

            if (!pBodyPart->FormMessageBody())
            {
                IMS_TRACE_E(0, "Forming a message body failed", 0, 0, 0);
                continue;  // throw exception
            }

            if (!SIPStack::AppendMessageBody(pBodyPart->GetMessageBody(), pstMessage))
            {
                IMS_TRACE_E(0, "Appending a message body failed", 0, 0, 0);
                continue;  // throw exception
            }
        }

        // Do more things related to the message body handling
        //  : Content-Type / Content-Length and so on .
        if (!SIPStack::CorrectMessageBody(pstMessage))
        {
            IMS_TRACE_E(0, "Correcting SIP message body failed", 0, 0, 0);
            return IMS_FALSE;
        }
    }

    return IMS_TRUE;
}

/*

Remarks

*/
PUBLIC
IMS_BOOL SIPMessage::FormMessageOnChallenge()
{
    //---------------------------------------------------------------------------------------------

    // FIXME: need to improve the logic

    while (SIPStack::GetHeaderCount(pstMessage, ISipHeader::UNKNOWN) > 0)
    {
        SIPStack::RemoveHeader(ISipHeader::UNKNOWN, pstMessage);
    }

    // Set unknown headers
    if (objUnknownHeaders.GetCount() > 0)
    {
        AString strName;
        AString strValue;
        IMS_BOOL bCompactForm =
                ((SIPPrivate::GetEncodingOptions() & SIPPrivate::OPT_E_SHORTFORM) != 0);

        for (IMS_SINT32 i = 0; i < objUnknownHeaders.GetCount(); ++i)
        {
            strName = objUnknownHeaders.GetHeaderName(i, bCompactForm);
            strValue = objUnknownHeaders.GetHeaderBodys(i);

            (void)SIPStack::PrependUnknownHeader(strName, strValue, pstMessage);
        }
    }

    // Set message body parts
    if (!objBodyParts.IsEmpty())
    {
        // FIXME: need to improve the logic
        // Message body is formed in the re-submitted request
        if (SIPStack::GetMessageBodyCount(pstMessage) == 0)
        {
            for (IMS_UINT32 i = 0; i < objBodyParts.GetSize(); ++i)
            {
                SIPMessageBodyPart* pBodyPart = objBodyParts.GetAt(i);

                if (pBodyPart == IMS_NULL)
                    continue;

                if (!pBodyPart->FormMessageBody())
                {
                    IMS_TRACE_E(0, "Forming a message body failed", 0, 0, 0);
                    continue;  // throw exception
                }

                if (!SIPStack::AppendMessageBody(pBodyPart->GetMessageBody(), pstMessage))
                {
                    IMS_TRACE_E(0, "Appending a message body failed", 0, 0, 0);
                    continue;  // throw exception
                }
            }
        }

        // Do more things related to the message body handling
        //  : Content-Type / Content-Length and so on .
        if (!SIPStack::CorrectMessageBody(pstMessage))
        {
            IMS_TRACE_E(0, "Correcting SIP message body failed", 0, 0, 0);
            return IMS_FALSE;
        }
    }

    return IMS_TRUE;
}

/*

Remarks

*/
PUBLIC
IMS_BOOL SIPMessage::FormMessageOnRetransmission()
{
    // Set message body parts
    if (!objBodyParts.IsEmpty())
    {
        // Do more things related to the message body handling
        //  : Content-Type / Content-Length and so on .
        if (!SIPStack::CorrectMessageBody(pstMessage))
        {
            IMS_TRACE_E(0, "Correcting SIP message body failed", 0, 0, 0);
            return IMS_FALSE;
        }
    }

    return IMS_TRUE;
}

/*

Remarks

*/
PUBLIC GLOBAL SIPMessage* SIPMessage::CreateMessage(IN CONST ByteArray& objMessage)
{
    SipMessage* pstMessage = IMS_NULL;

    //---------------------------------------------------------------------------------------------

    if (!SIPStack::DecodeMessage(objMessage.GetData(), objMessage.GetLength(),
                SIPPrivate::OPTIONS_D_PARTIAL, pstMessage))
    {
        return IMS_NULL;
    }

    SIPMessage* pSIPMsg = new SIPMessage(pstMessage);

    if (pSIPMsg == IMS_NULL)
    {
        SIPStack::FreeMessage(pstMessage);
        return IMS_NULL;
    }

    SIPStack::FreeMessage(pstMessage);

    if (!pSIPMsg->CreateBodyParts())
    {
        delete pSIPMsg;
        return IMS_NULL;
    }

    return pSIPMsg;
}

/*

Remarks

*/
PRIVATE
void SIPMessage::Init(IN IMS_BOOL bMessageClone)
{
    //---------------------------------------------------------------------------------------------

    if (pstMessage != IMS_NULL)
    {
        if (bMessageClone)
        {
            pstMessage = SIPStack::CloneMessage(pstMessage);
        }
        else
        {
            SIPStack::AddReference(pstMessage);
        }
    }

    if (pstMessage != IMS_NULL)
    {
        if (SIPStack::IsRequestMessage(pstMessage))
        {
            nType = TYPE_REQUEST;
        }
        else
        {
            nType = TYPE_RESPONSE;
        }

        ExtractProperties();
        ExtractUnknownHeaders();
        ExtractBodyParts();
    }
}

/*

Remarks

*/
PRIVATE
IMS_BOOL SIPMessage::ExtractBodyParts()
{
    //---------------------------------------------------------------------------------------------

    if (pstMessage == IMS_NULL)
        return IMS_FALSE;

    IMS_SINT32 nBodyCount = SIPStack::GetMessageBodyCount(pstMessage);

    if (nBodyCount == 0)
        return IMS_TRUE;

    SIPMessageBodyPart* pBodyPart = IMS_NULL;

    if (nBodyCount == 1)
    {
        SipMsgBody* pstMsgBody = SIPStack::GetMessageBody(pstMessage, 0);

        if (SIPStack::HasSDPMessageBody(pstMessage))
            pBodyPart = new SIPMessageBodyPart(pstMsgBody, IMS_TRUE);
        else
            pBodyPart = new SIPMessageBodyPart(pstMsgBody, IMS_FALSE);

        if (pBodyPart == IMS_NULL)
        {
            SIPStack::FreeMessageBody(pstMsgBody);
            return IMS_FALSE;
        }

        SIPStack::FreeMessageBody(pstMsgBody);

        // Set "Content-" headers
        SipHeaderBase* pstHeader = SIPStack::GetHeader(pstMessage, ISipHeader::CONTENT_TYPE);
        pBodyPart->SetHeader(pstHeader, ISipMessageBodyPart::CONTENT_TYPE);

        SIPStack::FreeHeaderEx(pstHeader);

        if (!objBodyParts.Append(pBodyPart))
        {
            delete pBodyPart;
            return IMS_FALSE;
        }
    }
    else
    {
        for (IMS_SINT32 i = 0; i < nBodyCount; ++i)
        {
            SipMsgBody* pstMsgBody = SIPStack::GetMessageBody(pstMessage, i);

            if (pstMsgBody == IMS_NULL)
            {
                return IMS_FALSE;
            }

            if (SIPStack::IsMessageBodySDP(pstMsgBody))
                pBodyPart = new SIPMessageBodyPart(pstMsgBody, IMS_TRUE);
            else
                pBodyPart = new SIPMessageBodyPart(pstMsgBody, IMS_FALSE);

            if (pBodyPart == IMS_NULL)
            {
                SIPStack::FreeMessageBody(pstMsgBody);
                return IMS_FALSE;
            }

            SIPStack::FreeMessageBody(pstMsgBody);

            if (!objBodyParts.Append(pBodyPart))
            {
                delete pBodyPart;
                return IMS_FALSE;
            }
        }
    }

    return IMS_TRUE;
}

/*

Remarks

*/
PRIVATE
IMS_BOOL SIPMessage::ExtractProperties()
{
    //---------------------------------------------------------------------------------------------

    if ((pstMessage == IMS_NULL) || (nType == TYPE_ANY))
        return IMS_FALSE;

    objMethod = SIPStack::GetMethod(pstMessage);

    if (nType == TYPE_REQUEST)
    {
        SipAddrSpec* pstAddrSpec = SIPStack::GetRequestUri(pstMessage);

        SIPStack::EncodeAddrSpec(pstAddrSpec, IMS_TRUE, strRequestURI);
        SIPStack::FreeAddrSpec(pstAddrSpec);
    }
    else
    {
        objStatusCode = SIPStack::GetStatusCodeEx(pstMessage);
    }

    return IMS_TRUE;
}

/*

Remarks

*/
PRIVATE
IMS_BOOL SIPMessage::ExtractUnknownHeaders()
{
    //---------------------------------------------------------------------------------------------

    if (pstMessage == IMS_NULL)
    {
        return IMS_FALSE;
    }
    else
    {
        objUnknownHeaders.Clear();

        IMS_SINT32 nHCount = SIPStack::GetHeaderCount(pstMessage, ISipHeader::UNKNOWN);

        if (nHCount == 0)
        {
            return IMS_TRUE;
        }

        for (IMS_SINT32 i = 0; i < nHCount; ++i)
        {
            SipHeaderBase* pstHeader = SIPStack::GetHeader(pstMessage, ISipHeader::UNKNOWN, i);

            objUnknownHeaders.AddHeader(SIPStack::GetUnknownHeaderName(pstHeader),
                    SIPStack::GetUnknownHeaderBody(pstHeader));

            SIPStack::FreeHeaderEx(pstHeader);
        }

        return IMS_TRUE;
    }
}
