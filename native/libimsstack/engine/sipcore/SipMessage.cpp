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
#include "ServiceMemory.h"
#include "ServiceTrace.h"

#include "ISipHeader.h"
#include "SipError.h"
#include "SipMessage.h"
#include "SipMessageBodyPart.h"
#include "SipPrivate.h"
#include "SipStack.h"

__IMS_TRACE_TAG_SIP_CORE__;

namespace sipcore
{

PUBLIC
SipMessage::SipMessage(IN IMS_SINT32 nType /*= ISipMessage::TYPE_REQUEST*/) :
        m_nType(nType),
        m_strRequestUri(AString::ConstNull()),
        m_bBodyPartParsed(IMS_FALSE),
        m_pSipMsg(IMS_NULL)
{
    m_pSipMsg = SipStack::CreateMessage(m_nType);
}

PUBLIC
SipMessage::SipMessage(IN ::SipMessage* pSipMsg) :
        m_nType(TYPE_ANY),
        m_strRequestUri(AString::ConstNull()),
        m_bBodyPartParsed(IMS_FALSE),
        m_pSipMsg(pSipMsg)
{
    Init(IMS_FALSE);
}

PUBLIC
SipMessage::SipMessage(IN ::SipMessage* pSipMsg, IN IMS_BOOL bMessageClone) :
        m_nType(TYPE_ANY),
        m_strRequestUri(AString::ConstNull()),
        m_bBodyPartParsed(IMS_FALSE),
        m_pSipMsg(pSipMsg)
{
    Init(bMessageClone);
}

PUBLIC VIRTUAL SipMessage::~SipMessage()
{
    SipStack::FreeMessage(m_pSipMsg);

    if (!m_objBodyParts.IsEmpty())
    {
        for (IMS_UINT32 i = 0; i < m_objBodyParts.GetSize(); ++i)
        {
            SipMessageBodyPart* pBodyPart = m_objBodyParts.GetAt(i);

            if (pBodyPart != IMS_NULL)
            {
                delete pBodyPart;
            }
        }

        m_objBodyParts.Clear();
    }

#ifdef __IMS_SIP_DEBUG__
    IMS_TRACE_D("dtor: SipMessage", 0, 0, 0);
#endif
}

PUBLIC
SipMessage& SipMessage::operator=(IN const SipMessage& other)
{
    if (this != &other)
    {
        SipStack::FreeMessage(m_pSipMsg);

        m_nType = other.m_nType;
        m_objMethod = other.m_objMethod;
        m_strRequestUri = other.m_strRequestUri;
        m_objStatusCode = other.m_objStatusCode;
        m_objUnknownHeaders = other.m_objUnknownHeaders;
        m_bBodyPartParsed = other.m_bBodyPartParsed;

        if (!m_objBodyParts.IsEmpty())
        {
            for (IMS_UINT32 i = 0; i < m_objBodyParts.GetSize(); ++i)
            {
                SipMessageBodyPart* pBodyPart = m_objBodyParts.GetAt(i);

                if (pBodyPart != IMS_NULL)
                {
                    delete pBodyPart;
                }
            }

            m_objBodyParts.Clear();
        }

        for (IMS_UINT32 i = 0; i < other.m_objBodyParts.GetSize(); ++i)
        {
            const SipMessageBodyPart* pBodyPart = other.m_objBodyParts.GetAt(i);
            SipMessageBodyPart* pNewBodyPart =
                    DYNAMIC_CAST(SipMessageBodyPart*, pBodyPart->Clone());

            if (pNewBodyPart != IMS_NULL)
            {
                m_objBodyParts.Append(pNewBodyPart);
            }
        }

        m_pSipMsg = other.m_pSipMsg;

        if (m_pSipMsg != IMS_NULL)
        {
            SipStack::AddReference(m_pSipMsg);
        }
    }

    return (*this);
}

PUBLIC VIRTUAL void SipMessage::Destroy()
{
    delete this;
}

PUBLIC VIRTUAL ISipMessage* SipMessage::Clone() const
{
    SipMessage* pNewMessage = new SipMessage(m_nType);

    if (pNewMessage == IMS_NULL)
    {
        SipPrivate::SetLastError(SipError::NO_MEMORY);
        return IMS_NULL;
    }

    pNewMessage->m_objMethod = m_objMethod;
    pNewMessage->m_strRequestUri = m_strRequestUri;
    pNewMessage->m_objStatusCode = m_objStatusCode;
    pNewMessage->m_objUnknownHeaders = m_objUnknownHeaders;
    pNewMessage->m_bBodyPartParsed = m_bBodyPartParsed;

    for (IMS_UINT32 i = 0; i < m_objBodyParts.GetSize(); ++i)
    {
        const SipMessageBodyPart* pBodyPart = m_objBodyParts.GetAt(i);
        SipMessageBodyPart* pNewBodyPart = DYNAMIC_CAST(SipMessageBodyPart*, pBodyPart->Clone());

        if (pNewBodyPart == IMS_NULL)
        {
            delete pNewMessage;

            SipPrivate::SetLastError(SipError::NO_MEMORY);
            return IMS_NULL;
        }

        if (!pNewMessage->m_objBodyParts.Append(pNewBodyPart))
        {
            delete pNewBodyPart;
            delete pNewMessage;

            SipPrivate::SetLastError(SipError::LIST_OPERATION_FAILED);
            return IMS_NULL;
        }
    }

    if (m_pSipMsg != IMS_NULL)
    {
        SipStack::FreeMessage(pNewMessage->m_pSipMsg);
        pNewMessage->m_pSipMsg = SipStack::CloneMessage(m_pSipMsg);

        if (pNewMessage->m_pSipMsg == IMS_NULL)
        {
            delete pNewMessage;

            SipPrivate::SetLastError(SipError::NO_MEMORY);
            return IMS_NULL;
        }
    }

    SipPrivate::SetLastError(SipError::NO_ERROR);
    return pNewMessage;
}

PUBLIC VIRTUAL IMS_RESULT SipMessage::AddHeader(IN IMS_SINT32 nType, IN const AString& strValue,
        IN const AString& strName /*= AString::ConstNull()*/)
{
    SipPrivate::SetLastError(SipError::NO_ERROR);

    if (SipStack::IsUnknownHeader(nType, strName))
    {
        return m_objUnknownHeaders.AddHeader(strName, strValue);
    }
    else
    {
        SipHeaderBase* pSipHdr = SipStack::DecodeHeader(nType, strName, strValue);

        if (pSipHdr == IMS_NULL)
        {
            SipPrivate::SetLastError(SipError::GENERAL_ERROR);
            return IMS_FAILURE;
        }

        if (!SipStack::AppendHeader(pSipHdr, m_pSipMsg))
        {
            SipStack::FreeHeaderEx(pSipHdr);

            SipPrivate::SetLastError(SipError::GENERAL_ERROR);
            return IMS_FAILURE;
        }

        SipStack::FreeHeaderEx(pSipHdr);

        return IMS_SUCCESS;
    }
}

PUBLIC VIRTUAL IMS_UINT32 SipMessage::GetCSeqNumber() const
{
    return SipStack::GetCSeqNumber(m_pSipMsg);
}

PUBLIC VIRTUAL AString SipMessage::GetHeader(IN IMS_SINT32 nType, IN IMS_SINT32 nIndex /*= 0*/,
        IN const AString& strName /*= AString::ConstNull()*/) const
{
    SipPrivate::SetLastError(SipError::NO_ERROR);

    if (SipStack::IsUnknownHeader(nType, strName))
    {
        return m_objUnknownHeaders.GetHeader(strName, nIndex);
    }
    else
    {
        if (SipStack::GetHeaderCount(m_pSipMsg, nType) == 0)
        {
            return AString::ConstNull();
        }

        SipHeaderBase* pSipHdr = SipStack::GetHeader(m_pSipMsg, nType, nIndex);
        AString strHeader;

        SipStack::EncodeHeaderBody(pSipHdr, IMS_TRUE, strHeader);

        SipStack::FreeHeaderEx(pSipHdr);

        return strHeader;
    }
}

PUBLIC VIRTUAL IMS_SINT32 SipMessage::GetHeaderCount(
        IN IMS_SINT32 nType, IN const AString& strName /*= AString::ConstNull()*/) const
{
    SipPrivate::SetLastError(SipError::NO_ERROR);

    if (m_pSipMsg == IMS_NULL)
    {
        return 0;
    }

    if (SipStack::IsUnknownHeader(nType, strName))
    {
        return m_objUnknownHeaders.GetHeaderCount(strName);
    }
    else
    {
        return SipStack::GetHeaderCount(m_pSipMsg, nType);
    }
}

PUBLIC VIRTUAL ImsList<AString> SipMessage::GetHeaders(
        IN IMS_SINT32 nType, IN const AString& strName /*= AString::ConstNull()*/) const
{
    SipPrivate::SetLastError(SipError::NO_ERROR);

    if (SipStack::IsUnknownHeader(nType, strName))
    {
        return m_objUnknownHeaders.GetHeaders(strName);
    }
    else
    {
        ImsList<AString> objHeaders;
        IMS_SINT32 nHCount = SipStack::GetHeaderCount(m_pSipMsg, nType);
        AString strHeader;

        for (IMS_SINT32 i = 0; i < nHCount; ++i)
        {
            SipHeaderBase* pSipHdr = SipStack::GetHeader(m_pSipMsg, nType, i);

            if (SipStack::IsValidHeader(pSipHdr))
            {
                SipStack::EncodeHeaderBody(pSipHdr, IMS_TRUE, strHeader);

                if (!objHeaders.Append(strHeader))
                {
                    SipStack::FreeHeaderEx(pSipHdr);
                    return ImsList<AString>();
                }
            }

            SipStack::FreeHeaderEx(pSipHdr);
        }

        return objHeaders;
    }
}

PUBLIC VIRTUAL IMS_RESULT SipMessage::PrependHeader(IN IMS_SINT32 nType, IN const AString& strValue,
        IN const AString& strName /*= AString::ConstNull()*/)
{
    SipPrivate::SetLastError(SipError::NO_ERROR);

    if (SipStack::IsUnknownHeader(nType, strName))
    {
        return m_objUnknownHeaders.PrependHeader(strName, strValue);
    }
    else
    {
        SipHeaderBase* pSipHdr = SipStack::DecodeHeader(nType, strName, strValue);

        if (pSipHdr == IMS_NULL)
        {
            SipPrivate::SetLastError(SipError::GENERAL_ERROR);
            return IMS_FAILURE;
        }

        if (!SipStack::PrependHeader(pSipHdr, m_pSipMsg))
        {
            SipStack::FreeHeaderEx(pSipHdr);

            SipPrivate::SetLastError(SipError::GENERAL_ERROR);
            return IMS_FAILURE;
        }

        SipStack::FreeHeaderEx(pSipHdr);

        return IMS_SUCCESS;
    }
}

PUBLIC VIRTUAL void SipMessage::RemoveHeader(
        IN IMS_SINT32 nType, IN const AString& strName /*= AString::ConstNull()*/)
{
    SipPrivate::SetLastError(SipError::NO_ERROR);

    if (SipStack::IsUnknownHeader(nType, strName))
    {
        m_objUnknownHeaders.RemoveHeader(strName);
    }
    else
    {
        SipStack::RemoveHeader(nType, m_pSipMsg);
    }
}

PUBLIC VIRTUAL IMS_RESULT SipMessage::SetHeader(IN IMS_SINT32 nType, IN const AString& strValue,
        IN const AString& strName /*= AString::ConstNull()*/)
{
    SipPrivate::SetLastError(SipError::NO_ERROR);

    if (SipStack::IsUnknownHeader(nType, strName))
    {
        return m_objUnknownHeaders.SetHeader(strName, strValue);
    }
    else
    {
        SipHeaderBase* pSipHdr = SipStack::DecodeHeader(nType, strName, strValue);

        if (pSipHdr == IMS_NULL)
        {
            SipPrivate::SetLastError(SipError::GENERAL_ERROR);
            return IMS_FAILURE;
        }

        if (!SipStack::SetHeader(pSipHdr, m_pSipMsg))
        {
            SipStack::FreeHeaderEx(pSipHdr);

            SipPrivate::SetLastError(SipError::GENERAL_ERROR);
            return IMS_FAILURE;
        }

        SipStack::FreeHeaderEx(pSipHdr);

        return IMS_SUCCESS;
    }
}

PUBLIC VIRTUAL ISipMessageBodyPart* SipMessage::CreateBodyPart()
{
    SipMessageBodyPart* pBodyPart = new SipMessageBodyPart(IMS_FALSE);

    SipPrivate::SetLastError(SipError::NO_ERROR);

    if (pBodyPart == IMS_NULL)
    {
        SipPrivate::SetLastError(SipError::NO_MEMORY);
        return IMS_NULL;
    }

    if (!m_objBodyParts.Append(pBodyPart))
    {
        delete pBodyPart;

        SipPrivate::SetLastError(SipError::LIST_OPERATION_FAILED);
        return IMS_NULL;
    }

    return pBodyPart;
}

PUBLIC VIRTUAL ISipMessageBodyPart* SipMessage::CreateSdpBodyPart()
{
    SipMessageBodyPart* pBodyPart = new SipMessageBodyPart(IMS_TRUE);

    SipPrivate::SetLastError(SipError::NO_ERROR);

    if (pBodyPart == IMS_NULL)
    {
        SipPrivate::SetLastError(SipError::NO_MEMORY);
        return IMS_NULL;
    }

    if (!m_objBodyParts.Prepend(pBodyPart))
    {
        delete pBodyPart;

        SipPrivate::SetLastError(SipError::LIST_OPERATION_FAILED);
        return IMS_NULL;
    }

    return pBodyPart;
}

PUBLIC VIRTUAL ImsList<ISipMessageBodyPart*> SipMessage::GetBodyParts() const
{
    if (m_objBodyParts.IsEmpty())
    {
        return ImsList<ISipMessageBodyPart*>();
    }

    ImsList<ISipMessageBodyPart*> objSipBodyParts;

    for (IMS_UINT32 i = 0; i < m_objBodyParts.GetSize(); ++i)
    {
        SipMessageBodyPart* pBodyPart = m_objBodyParts.GetAt(i);

        if (!pBodyPart->IsSdpBodyPart())
        {
            objSipBodyParts.Append(pBodyPart);
        }
    }

    return objSipBodyParts;
}

PUBLIC VIRTUAL ISipMessageBodyPart* SipMessage::GetSdpBodyPart() const
{
    if (m_objBodyParts.IsEmpty())
    {
        return IMS_NULL;
    }

    for (IMS_UINT32 i = 0; i < m_objBodyParts.GetSize(); ++i)
    {
        SipMessageBodyPart* pBodyPart = m_objBodyParts.GetAt(i);

        if (pBodyPart->IsSdpBodyPart())
        {
            return pBodyPart;
        }
    }

    return IMS_NULL;
}

PUBLIC VIRTUAL ImsList<ISipMessageBodyPart*> SipMessage::GetSdpBodyParts() const
{
    if (m_objBodyParts.IsEmpty())
    {
        return ImsList<ISipMessageBodyPart*>();
    }

    ImsList<ISipMessageBodyPart*> objSipBodyParts;

    for (IMS_UINT32 i = 0; i < m_objBodyParts.GetSize(); ++i)
    {
        SipMessageBodyPart* pBodyPart = m_objBodyParts.GetAt(i);

        if (pBodyPart->IsSdpBodyPart())
        {
            objSipBodyParts.Append(pBodyPart);
        }
    }

    return objSipBodyParts;
}

PUBLIC VIRTUAL IMS_RESULT SipMessage::CopyHeadersAndBodyParts(IN const ISipMessage* piSipMsg)
{
    const SipMessage* pMessage = DYNAMIC_CAST(const SipMessage*, piSipMsg);

    if (pMessage == IMS_NULL)
    {
        SipPrivate::SetLastError(SipError::ILLEGAL_ARGUMENT);
        return IMS_FAILURE;
    }

    // Overwrites any known headers
    if (!SipStack::OverwriteHeaders(pMessage->m_pSipMsg, m_pSipMsg))
    {
        SipPrivate::SetLastError(SipError::GENERAL_ERROR);
        return IMS_FAILURE;
    }

    // Overwrites any unknown headers
    if (!m_objUnknownHeaders.OverwriteHeaders(pMessage->m_objUnknownHeaders))
    {
        SipPrivate::SetLastError(SipError::GENERAL_ERROR);
        return IMS_FAILURE;
    }

    // Copy any message body parts if present
    if (pMessage->m_objBodyParts.IsEmpty())
    {
        SipPrivate::SetLastError(SipError::NO_ERROR);
        return IMS_SUCCESS;
    }

    for (IMS_UINT32 i = 0; i < pMessage->m_objBodyParts.GetSize(); ++i)
    {
        const SipMessageBodyPart* pBodyPart = pMessage->m_objBodyParts.GetAt(i);
        SipMessageBodyPart* pNewBodyPart = new SipMessageBodyPart(pBodyPart->IsSdpBodyPart());

        if (pNewBodyPart == IMS_NULL)
        {
            return IMS_FAILURE;
        }

        (*pNewBodyPart) = (*pBodyPart);

        if (!m_objBodyParts.Append(pNewBodyPart))
        {
            delete pNewBodyPart;

            SipPrivate::SetLastError(SipError::LIST_OPERATION_FAILED);
            return IMS_FAILURE;
        }
    }

    SipPrivate::SetLastError(SipError::NO_ERROR);
    return IMS_SUCCESS;
}

PUBLIC VIRTUAL IMS_BOOL SipMessage::IsHeaderPresent(
        IN IMS_SINT32 nType, IN const AString& strName /*= AString::ConstNull()*/) const
{
    if (SipStack::IsUnknownHeader(nType, strName))
    {
        return m_objUnknownHeaders.IsHeaderPresent(strName);
    }

    return SipStack::IsHeaderPresent(m_pSipMsg, nType);
}

PUBLIC VIRTUAL IMS_BOOL SipMessage::IsMessageRpr() const
{
    return SipStack::IsMessageRpr(m_pSipMsg);
}

PUBLIC VIRTUAL IMS_BOOL SipMessage::IsOptionRequired(IN const AString& strOption) const
{
    return SipStack::IsOptionRequired(m_pSipMsg, strOption);
}

PUBLIC VIRTUAL IMS_BOOL SipMessage::IsOptionSupported(IN const AString& strOption) const
{
    return SipStack::IsOptionSupported(m_pSipMsg, strOption);
}

PUBLIC VIRTUAL void SipMessage::RemoveBodyParts()
{
    if (!m_objBodyParts.IsEmpty())
    {
        for (IMS_UINT32 i = 0; i < m_objBodyParts.GetSize(); ++i)
        {
            SipMessageBodyPart* pBodyPart = m_objBodyParts.GetAt(i);

            if (pBodyPart != IMS_NULL)
                delete pBodyPart;
        }

        m_objBodyParts.Clear();

        SipStack::RemoveAllMessageBodies(m_pSipMsg);
    }
}

PUBLIC VIRTUAL ByteArray SipMessage::ToByteArray(IN IMS_SINT32 nOptions /*= OPT_ALL*/) const
{
    ByteArray objMessage;

    if (!SipStack::EncodePartialMessage(m_pSipMsg, nOptions, objMessage))
    {
        return ByteArray::ConstNull();
    }

    return objMessage;
}

PUBLIC
SipMessageBodyPart* SipMessage::GetBodyPart() const
{
    if (m_objBodyParts.IsEmpty())
    {
        return IMS_NULL;
    }

    return m_objBodyParts.GetAt(0);
}

PUBLIC
IMS_RESULT SipMessage::SetRequestUri(IN const AString& strUri)
{
    if (strUri.IsNULL() || strUri.IsEmpty())
    {
        // Remove Request-URI if set previously
        m_strRequestUri = AString::ConstNull();
    }
    else
    {
        SipAddrSpec* pAddrSpec = SipStack::DecodeAddrSpec(strUri);

        if (pAddrSpec == IMS_NULL)
        {
            SipPrivate::SetLastError(SipError::ILLEGAL_ARGUMENT);
            return IMS_FAILURE;
        }

        SipStack::EncodeAddrSpec(pAddrSpec, IMS_TRUE, m_strRequestUri);

        SipStack::FreeAddrSpec(pAddrSpec);
    }

    SipPrivate::SetLastError(SipError::NO_ERROR);

    return IMS_SUCCESS;
}

PUBLIC
void SipMessage::UpdateRequestUri()
{
    SipAddrSpec* pAddrSpec = SipStack::GetRequestUri(m_pSipMsg);

    if (pAddrSpec == IMS_NULL)
    {
        // Remove Request-URI if set previously
        m_strRequestUri = AString::ConstNull();
    }
    else
    {
        SipStack::EncodeAddrSpec(pAddrSpec, IMS_TRUE, m_strRequestUri);
        SipStack::FreeAddrSpec(pAddrSpec);
    }
}

PUBLIC
IMS_BOOL SipMessage::CreateBodyParts()
{
    if (m_bBodyPartParsed)
    {
        return IMS_TRUE;
    }

    if (m_objBodyParts.IsEmpty())
    {
        m_bBodyPartParsed = IMS_TRUE;
        return IMS_TRUE;
    }

    // If message does not include a MIME body, do not parse the message body
    if (!SipStack::HasMimeMessageBody(m_pSipMsg))
    {
        m_bBodyPartParsed = IMS_TRUE;

        if (SipStack::IsMessageBodyCompressed(m_pSipMsg))
        {
            if (!SipStack::UncompressMessageBody(m_pSipMsg))
            {
                IMS_TRACE_E(0, "Uncompressing SIP message body failed", 0, 0, 0);
                return IMS_FALSE;
            }

            // Clear the previous message body parts.
            while (!m_objBodyParts.IsEmpty())
            {
                SipMessageBodyPart* pBodyPart = m_objBodyParts.GetAt(0);

                delete pBodyPart;

                m_objBodyParts.RemoveAt(0);
            }

            if (!ExtractBodyParts())
            {
                return IMS_FALSE;
            }
        }

        return IMS_TRUE;
    }

    if (!SipStack::DecodeMessageBody(m_pSipMsg))
    {
        IMS_TRACE_E(0, "Parsing SIP message body failed", 0, 0, 0);
        return IMS_FALSE;
    }

    // Clear the previous message body parts.
    while (!m_objBodyParts.IsEmpty())
    {
        SipMessageBodyPart* pBodyPart = m_objBodyParts.GetAt(0);

        delete pBodyPart;

        m_objBodyParts.RemoveAt(0);
    }

    if (!ExtractBodyParts())
    {
        return IMS_FALSE;
    }

    m_bBodyPartParsed = IMS_TRUE;

    return IMS_TRUE;
}

PUBLIC
IMS_BOOL SipMessage::FormMessage()
{
    if (m_pSipMsg == IMS_NULL)
    {
        return IMS_FALSE;
    }

    /*
     * Set properties
     *    In case of CSeq header, it will set by the SIP transaction with a correct value.
     */
    if (m_nType == TYPE_REQUEST)
    {
        if (!SipStack::SetRequestLine(m_objMethod.ToString(), m_strRequestUri, m_pSipMsg))
        {
            return IMS_FALSE;
        }
    }
    else
    {
        if (!SipStack::SetStatusLine(
                    m_objStatusCode.ToInt(), m_objStatusCode.GetReasonPhrase(), m_pSipMsg))
        {
            return IMS_FALSE;
        }
    }

    // Set unknown headers
    if (m_objUnknownHeaders.GetCount() > 0)
    {
        AString strName;
        AString strValue;
        IMS_BOOL bCompactForm =
                ((SipPrivate::GetEncodingOptions() & SipPrivate::OPT_E_SHORTFORM) != 0);

        for (IMS_SINT32 i = m_objUnknownHeaders.GetCount() - 1; i >= 0; --i)
        {
            strName = m_objUnknownHeaders.GetHeaderName(i, bCompactForm);
            strValue = m_objUnknownHeaders.GetHeaderBodys(i);

            (void)SipStack::PrependUnknownHeader(strName, strValue, m_pSipMsg);
        }
    }

    // Set message body parts
    if (!m_objBodyParts.IsEmpty())
    {
        for (IMS_UINT32 i = 0; i < m_objBodyParts.GetSize(); ++i)
        {
            SipMessageBodyPart* pBodyPart = m_objBodyParts.GetAt(i);

            if (pBodyPart == IMS_NULL)
            {
                continue;
            }

            if (!pBodyPart->FormMessageBody())
            {
                IMS_TRACE_E(0, "Forming a message body failed", 0, 0, 0);
                continue;  // throw exception
            }

            if (!SipStack::AppendMessageBody(pBodyPart->GetMessageBody(), m_pSipMsg))
            {
                IMS_TRACE_E(0, "Appending a message body failed", 0, 0, 0);
                continue;  // throw exception
            }
        }

        // Do more things related to the message body handling
        //  : Content-Type / Content-Length and so on .
        if (!SipStack::CorrectMessageBody(m_pSipMsg))
        {
            IMS_TRACE_E(0, "Correcting SIP message body failed", 0, 0, 0);
            return IMS_FALSE;
        }
    }

    return IMS_TRUE;
}

PUBLIC
IMS_BOOL SipMessage::FormMessageOnChallenge()
{
    while (SipStack::GetHeaderCount(m_pSipMsg, ISipHeader::UNKNOWN) > 0)
    {
        SipStack::RemoveHeader(ISipHeader::UNKNOWN, m_pSipMsg);
    }

    // Set unknown headers
    if (m_objUnknownHeaders.GetCount() > 0)
    {
        AString strName;
        AString strValue;
        IMS_BOOL bCompactForm =
                ((SipPrivate::GetEncodingOptions() & SipPrivate::OPT_E_SHORTFORM) != 0);

        for (IMS_SINT32 i = 0; i < m_objUnknownHeaders.GetCount(); ++i)
        {
            strName = m_objUnknownHeaders.GetHeaderName(i, bCompactForm);
            strValue = m_objUnknownHeaders.GetHeaderBodys(i);

            (void)SipStack::PrependUnknownHeader(strName, strValue, m_pSipMsg);
        }
    }

    // Set message body parts
    if (!m_objBodyParts.IsEmpty())
    {
        // Message body is formed in the re-submitted request
        if (SipStack::GetMessageBodyCount(m_pSipMsg) == 0)
        {
            for (IMS_UINT32 i = 0; i < m_objBodyParts.GetSize(); ++i)
            {
                SipMessageBodyPart* pBodyPart = m_objBodyParts.GetAt(i);

                if (pBodyPart == IMS_NULL)
                {
                    continue;
                }

                if (!pBodyPart->FormMessageBody())
                {
                    IMS_TRACE_E(0, "Forming a message body failed", 0, 0, 0);
                    continue;  // throw exception
                }

                if (!SipStack::AppendMessageBody(pBodyPart->GetMessageBody(), m_pSipMsg))
                {
                    IMS_TRACE_E(0, "Appending a message body failed", 0, 0, 0);
                    continue;  // throw exception
                }
            }
        }

        // Do more things related to the message body handling
        //  : Content-Type / Content-Length and so on .
        if (!SipStack::CorrectMessageBody(m_pSipMsg))
        {
            IMS_TRACE_E(0, "Correcting SIP message body failed", 0, 0, 0);
            return IMS_FALSE;
        }
    }

    return IMS_TRUE;
}

PUBLIC
IMS_BOOL SipMessage::FormMessageOnRetransmission()
{
    // Set message body parts
    if (!m_objBodyParts.IsEmpty())
    {
        // Do more things related to the message body handling
        //  : Content-Type / Content-Length and so on .
        if (!SipStack::CorrectMessageBody(m_pSipMsg))
        {
            IMS_TRACE_E(0, "Correcting SIP message body failed", 0, 0, 0);
            return IMS_FALSE;
        }
    }

    return IMS_TRUE;
}

PUBLIC GLOBAL SipMessage* SipMessage::CreateMessage(IN const ByteArray& objMessage)
{
    ::SipMessage* pSipMsg = IMS_NULL;

    if (!SipStack::DecodeMessage(objMessage.GetData(), objMessage.GetLength(),
                SipPrivate::OPTIONS_D_PARTIAL, pSipMsg))
    {
        return IMS_NULL;
    }

    SipMessage* pMessage = new SipMessage(pSipMsg);

    if (pMessage == IMS_NULL)
    {
        SipStack::FreeMessage(pSipMsg);
        return IMS_NULL;
    }

    SipStack::FreeMessage(pSipMsg);

    if (!pMessage->CreateBodyParts())
    {
        delete pMessage;
        return IMS_NULL;
    }

    return pMessage;
}

PRIVATE
void SipMessage::Init(IN IMS_BOOL bMessageClone)
{
    if (m_pSipMsg != IMS_NULL)
    {
        if (bMessageClone)
        {
            m_pSipMsg = SipStack::CloneMessage(m_pSipMsg);
        }
        else
        {
            SipStack::AddReference(m_pSipMsg);
        }
    }

    if (m_pSipMsg != IMS_NULL)
    {
        if (SipStack::IsRequestMessage(m_pSipMsg))
        {
            m_nType = TYPE_REQUEST;
        }
        else
        {
            m_nType = TYPE_RESPONSE;
        }

        ExtractProperties();
        ExtractUnknownHeaders();
        ExtractBodyParts();
    }
}

PRIVATE
IMS_BOOL SipMessage::ExtractBodyParts()
{
    if (m_pSipMsg == IMS_NULL)
    {
        return IMS_FALSE;
    }

    IMS_SINT32 nBodyCount = SipStack::GetMessageBodyCount(m_pSipMsg);

    if (nBodyCount == 0)
    {
        return IMS_TRUE;
    }

    SipMessageBodyPart* pBodyPart;

    if (nBodyCount == 1)
    {
        SipMsgBody* pMsgBody = SipStack::GetMessageBody(m_pSipMsg, 0);

        if (SipStack::HasSdpMessageBody(m_pSipMsg))
        {
            pBodyPart = new SipMessageBodyPart(pMsgBody, IMS_TRUE);
        }
        else
        {
            pBodyPart = new SipMessageBodyPart(pMsgBody, IMS_FALSE);
        }

        if (pBodyPart == IMS_NULL)
        {
            SipStack::FreeMessageBody(pMsgBody);
            return IMS_FALSE;
        }

        SipStack::FreeMessageBody(pMsgBody);

        // Set "Content-" headers
        SipHeaderBase* pSipHdr = SipStack::GetHeader(m_pSipMsg, ISipHeader::CONTENT_TYPE);
        pBodyPart->SetHeader(pSipHdr, ISipMessageBodyPart::CONTENT_TYPE);

        SipStack::FreeHeaderEx(pSipHdr);

        if (!m_objBodyParts.Append(pBodyPart))
        {
            delete pBodyPart;
            return IMS_FALSE;
        }
    }
    else
    {
        for (IMS_SINT32 i = 0; i < nBodyCount; ++i)
        {
            SipMsgBody* pMsgBody = SipStack::GetMessageBody(m_pSipMsg, i);

            if (pMsgBody == IMS_NULL)
            {
                return IMS_FALSE;
            }

            if (SipStack::IsMessageBodySdp(pMsgBody))
            {
                pBodyPart = new SipMessageBodyPart(pMsgBody, IMS_TRUE);
            }
            else
            {
                pBodyPart = new SipMessageBodyPart(pMsgBody, IMS_FALSE);
            }

            if (pBodyPart == IMS_NULL)
            {
                SipStack::FreeMessageBody(pMsgBody);
                return IMS_FALSE;
            }

            SipStack::FreeMessageBody(pMsgBody);

            if (!m_objBodyParts.Append(pBodyPart))
            {
                delete pBodyPart;
                return IMS_FALSE;
            }
        }
    }

    return IMS_TRUE;
}

PRIVATE
IMS_BOOL SipMessage::ExtractProperties()
{
    if ((m_pSipMsg == IMS_NULL) || (m_nType == TYPE_ANY))
    {
        return IMS_FALSE;
    }

    m_objMethod = SipStack::GetMethod(m_pSipMsg);

    if (m_nType == TYPE_REQUEST)
    {
        SipAddrSpec* pAddrSpec = SipStack::GetRequestUri(m_pSipMsg);

        SipStack::EncodeAddrSpec(pAddrSpec, IMS_TRUE, m_strRequestUri);
        SipStack::FreeAddrSpec(pAddrSpec);
    }
    else
    {
        m_objStatusCode = SipStack::GetStatusCodeEx(m_pSipMsg);
    }

    return IMS_TRUE;
}

PRIVATE
IMS_BOOL SipMessage::ExtractUnknownHeaders()
{
    if (m_pSipMsg == IMS_NULL)
    {
        return IMS_FALSE;
    }
    else
    {
        m_objUnknownHeaders.Clear();

        IMS_SINT32 nHCount = SipStack::GetHeaderCount(m_pSipMsg, ISipHeader::UNKNOWN);

        if (nHCount == 0)
        {
            return IMS_TRUE;
        }

        for (IMS_SINT32 i = 0; i < nHCount; ++i)
        {
            SipHeaderBase* pSipHdr = SipStack::GetHeader(m_pSipMsg, ISipHeader::UNKNOWN, i);

            m_objUnknownHeaders.AddHeader(SipStack::GetUnknownHeaderName(pSipHdr),
                    SipStack::GetUnknownHeaderBody(pSipHdr));

            SipStack::FreeHeaderEx(pSipHdr);
        }

        return IMS_TRUE;
    }
}

}  // namespace sipcore
