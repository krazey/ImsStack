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

#include "msg/SipMsgBody.h"

#include "SipError.h"
#include "SipMessageBodyPart.h"
#include "SipPrivate.h"
#include "SipStack.h"

__IMS_TRACE_TAG_SIP_CORE__;

PUBLIC
SipMessageBodyPart::SipMessageBodyPart(IN IMS_BOOL bSdpBody /*= IMS_FALSE*/) :
        m_bSdpBody(bSdpBody),
        m_pMsgBody(IMS_NULL)
{
    m_pMsgBody = SipStack::CreateMessageBody();
}

PUBLIC
SipMessageBodyPart::SipMessageBodyPart(
        IN SipMsgBody* pMsgBody, IN IMS_BOOL bSdpBody /*= IMS_FALSE*/) :
        m_bSdpBody(bSdpBody),
        m_pMsgBody(pMsgBody)
{
    if (m_pMsgBody != IMS_NULL)
    {
        SipStack::AddReference(m_pMsgBody);
        ExtractProperties();
    }
}

PUBLIC VIRTUAL SipMessageBodyPart::~SipMessageBodyPart()
{
    SipStack::FreeMessageBody(m_pMsgBody);

#ifdef __IMS_SIP_DEBUG__
    IMS_TRACE_D("dtor: SipMessageBodyPart", 0, 0, 0);
#endif
}

PUBLIC
SipMessageBodyPart& SipMessageBodyPart::operator=(IN const SipMessageBodyPart& other)
{
    if (this != &other)
    {
        SipStack::FreeMessageBody(m_pMsgBody);

        m_bSdpBody = other.m_bSdpBody;

        m_pMsgBody = other.m_pMsgBody;
        SipStack::AddReference(m_pMsgBody);

        m_objOtherMimeHeaders = other.m_objOtherMimeHeaders;
        m_objContent = other.m_objContent;
    }

    return (*this);
}

PUBLIC VIRTUAL void SipMessageBodyPart::Destroy()
{
    delete this;
}

PUBLIC VIRTUAL ISipMessageBodyPart* SipMessageBodyPart::Clone() const
{
    SipMessageBodyPart* pNewBodyPart = new SipMessageBodyPart(m_bSdpBody);

    if (pNewBodyPart == IMS_NULL)
    {
        SipPrivate::SetLastError(SipError::NO_MEMORY);
        return IMS_NULL;
    }

    if (m_pMsgBody != IMS_NULL)
    {
        SipStack::FreeMessageBody(pNewBodyPart->m_pMsgBody);
        pNewBodyPart->m_pMsgBody = SipStack::CloneMessageBody(m_pMsgBody);

        if (pNewBodyPart->m_pMsgBody == IMS_NULL)
        {
            delete pNewBodyPart;

            SipPrivate::SetLastError(SipError::NO_MEMORY);
            return IMS_NULL;
        }
    }

    pNewBodyPart->ExtractProperties();

    SipPrivate::SetLastError(SipError::NO_ERROR);
    return pNewBodyPart;
}

PUBLIC VIRTUAL void SipMessageBodyPart::CopyFrom(IN const ISipMessageBodyPart* piBodyPart)
{
    if (piBodyPart == IMS_NULL)
    {
        return;
    }

    const SipMessageBodyPart* pBodyPart = DYNAMIC_CAST(const SipMessageBodyPart*, piBodyPart);

    if (pBodyPart == IMS_NULL)
    {
        return;
    }

    operator=(*pBodyPart);
}

PUBLIC VIRTUAL AString SipMessageBodyPart::GetHeader(
        IN IMS_SINT32 nType, IN const AString& strName /*= AString::ConstNull()*/) const
{
    switch (nType)
    {
        case CONTENT_TYPE:               // FALL-THROUGH
        case CONTENT_DISPOSITION:        // FALL-THROUGH
        case CONTENT_TRANSFER_ENCODING:  // FALL-THROUGH
        case CONTENT_ID:                 // FALL-THROUGH
        case CONTENT_DESCRIPTION:
            return SipStack::GetMimeHeader(m_pMsgBody, nType);

        case CONTENT_UNKNOWN:
            if (strName.IsNULL())
            {
                return AString::ConstNull();
            }
            return m_objOtherMimeHeaders.GetHeader(strName);

        default:
            break;
    }

    return AString::ConstNull();
}

PUBLIC VIRTUAL void SipMessageBodyPart::SetHeader(IN IMS_SINT32 nType, IN const AString& strValue,
        IN const AString& strName /*= AString::ConstNull()*/)
{
    switch (nType)
    {
        case CONTENT_TYPE:               // FALL-THROUGH
        case CONTENT_DISPOSITION:        // FALL-THROUGH
        case CONTENT_TRANSFER_ENCODING:  // FALL-THROUGH
        case CONTENT_ID:                 // FALL-THROUGH
        case CONTENT_DESCRIPTION:
            if (!SipStack::SetMimeHeader(nType, strName, strValue, m_pMsgBody))
            {
                return;
            }
            break;

        case CONTENT_UNKNOWN:
            if (strName.IsNULL())
            {
                return;
            }
            m_objOtherMimeHeaders.AddHeader(strName, strValue);
            break;

        default:
            break;
    }
}

PUBLIC VIRTUAL void SipMessageBodyPart::SetContent(IN const ByteArray& objContent)
{
    // Set the content
    if (!SipStack::SetContent(objContent.GetData(), objContent.GetLength(), m_pMsgBody))
    {
        return;
    }

    // Load the content from SIP message body
    IMS_BYTE* pContent = IMS_NULL;
    IMS_SINT32 nContentLength = 0;

    if (SipStack::GetContent(m_pMsgBody, pContent, nContentLength))
    {
        // Set the reference of SIP message body
        this->m_objContent.Attach(pContent, nContentLength);
    }
}

PUBLIC
IMS_BOOL SipMessageBodyPart::FormMessageBody()
{
    // Set MIME content headers
    if (m_objOtherMimeHeaders.GetCount() > 0)
    {
        AString strName;
        AString strBody;

        for (IMS_SINT32 i = 0; i < m_objOtherMimeHeaders.GetCount(); ++i)
        {
            strName = m_objOtherMimeHeaders.GetHeaderName(i);
            strBody = m_objOtherMimeHeaders.GetHeaderBodys(i);

            if (!SipStack::SetMimeHeader(CONTENT_UNKNOWN, strName, strBody, m_pMsgBody))
            {
                return IMS_FALSE;
            }
        }
    }

    return IMS_TRUE;
}

PUBLIC
void SipMessageBodyPart::SetHeader(
        IN SipHeaderBase* pSipHdr, IN IMS_SINT32 nType /*= ISipMessageBodyPart::CONTENT_UNKNOWN*/)
{
    switch (nType)
    {
        case CONTENT_TYPE:               // FALL-THROUGH
        case CONTENT_DISPOSITION:        // FALL-THROUGH
        case CONTENT_TRANSFER_ENCODING:  // FALL-THROUGH
        case CONTENT_ID:                 // FALL-THROUGH
        case CONTENT_DESCRIPTION:
            if (!SipStack::SetMimeHeader(nType, pSipHdr, m_pMsgBody))
            {
                return;
            }
            break;

        case CONTENT_UNKNOWN:
            m_objOtherMimeHeaders.AddHeader(SipStack::GetUnknownHeaderName(pSipHdr),
                    SipStack::GetUnknownHeaderBody(pSipHdr));
            break;

        default:
            break;
    }
}

PRIVATE
IMS_BOOL SipMessageBodyPart::ExtractProperties()
{
    AString strHeader;
    IMS_SINT32 nHCount = SipStack::GetMimeHeaderCount(m_pMsgBody, CONTENT_UNKNOWN);

    // Extract an additional MIME content headers
    for (IMS_SINT32 i = 0; i < nHCount; ++i)
    {
        strHeader = SipStack::GetMimeHeader(m_pMsgBody, CONTENT_UNKNOWN, i);

        IMS_SINT32 nIndex = strHeader.GetIndexOf(TextParser::CHAR_COLON);

        if (nIndex != AString::NPOS)
        {
            AString strHName = strHeader.GetSubStr(0, nIndex).Trim();
            AString strHBody = strHeader.GetSubStr(nIndex + 1).Trim();

            m_objOtherMimeHeaders.AddHeader(strHName, strHBody);
        }
    }

    // Extract the content
    IMS_BYTE* pContent = IMS_NULL;
    IMS_SINT32 nContentLength = 0;

    if (SipStack::GetContent(m_pMsgBody, pContent, nContentLength))
    {
        // Set the reference of SIP message body
        m_objContent.Attach(pContent, nContentLength);
    }

    return IMS_TRUE;
}
