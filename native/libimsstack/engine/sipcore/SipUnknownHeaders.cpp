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

#include "SipHeader.h"
#include "SipPrivate.h"
#include "SipUnknownHeaders.h"

PUBLIC
SipUnknownHeaders::Header::Header(IN const AString& strName_) :
        strCompactName(AString::ConstNull()),
        strName(strName_)
{
}

PUBLIC
SipUnknownHeaders::Header::Header(IN const SipUnknownHeaders::Header& other) :
        strCompactName(other.strCompactName),
        strName(other.strName),
        objBodys(other.objBodys)
{
}

PUBLIC
SipUnknownHeaders::Header::~Header() {}

PUBLIC
SipUnknownHeaders::Header& SipUnknownHeaders::Header::operator=(
        IN const SipUnknownHeaders::Header& other)
{
    if (this != &other)
    {
        strCompactName = other.strCompactName;
        strName = other.strName;
        objBodys = other.objBodys;
    }

    return (*this);
}

PUBLIC
IMS_BOOL SipUnknownHeaders::Header::Equals(IN const AString& strName) const
{
    if (strName.IsNULL() || strName.IsEmpty())
    {
        return IMS_FALSE;
    }

    if (strCompactName.EqualsIgnoreCase(strName))
    {
        return IMS_TRUE;
    }

    return this->strName.EqualsIgnoreCase(strName);
}

PUBLIC
SipUnknownHeaders::SipUnknownHeaders() {}

PUBLIC
SipUnknownHeaders::SipUnknownHeaders(IN const SipUnknownHeaders& other)
{
    for (IMS_UINT32 i = 0; i < other.m_objHeaders.GetSize(); ++i)
    {
        const Header* pHeader = other.m_objHeaders.GetAt(i);

        Header* pNewHeader = new Header(*pHeader);

        if (pNewHeader != IMS_NULL)
        {
            m_objHeaders.Append(pNewHeader);
        }
    }
}

PUBLIC
SipUnknownHeaders::~SipUnknownHeaders()
{
    Clear();
}

PUBLIC
SipUnknownHeaders& SipUnknownHeaders::operator=(IN const SipUnknownHeaders& other)
{
    if (this != &other)
    {
        Clear();

        for (IMS_UINT32 i = 0; i < other.m_objHeaders.GetSize(); ++i)
        {
            const Header* pHeader = other.m_objHeaders.GetAt(i);
            Header* pNewHeader = new Header(*pHeader);

            if (pNewHeader != IMS_NULL)
            {
                m_objHeaders.Append(pNewHeader);
            }
        }
    }

    return (*this);
}

PUBLIC
IMS_RESULT SipUnknownHeaders::AddHeader(IN const AString& strName, IN const AString& strBody)
{
    Header* pHeader = FindHeader(strName);

    if (pHeader == IMS_NULL)
    {
        pHeader = new Header(strName);

        if (pHeader == IMS_NULL)
        {
            SipPrivate::SetLastError(SipError::NO_MEMORY);
            return IMS_FAILURE;
        }

        if (!pHeader->objBodys.Append(strBody))
        {
            delete pHeader;

            SipPrivate::SetLastError(SipError::LIST_OPERATION_FAILED);
            return IMS_FAILURE;
        }

        if (!m_objHeaders.Append(pHeader))
        {
            delete pHeader;

            SipPrivate::SetLastError(SipError::LIST_OPERATION_FAILED);
            return IMS_FAILURE;
        }
    }
    else
    {
        if (!pHeader->objBodys.Append(strBody))
        {
            SipPrivate::SetLastError(SipError::LIST_OPERATION_FAILED);
            return IMS_FAILURE;
        }
    }

    return IMS_SUCCESS;
}

PUBLIC
void SipUnknownHeaders::Clear()
{
    while (!m_objHeaders.IsEmpty())
    {
        Header* pHeader = m_objHeaders.GetAt(0);

        if (pHeader != IMS_NULL)
        {
            delete pHeader;
        }

        m_objHeaders.RemoveAt(0);
    }
}

PUBLIC
AString SipUnknownHeaders::GetHeader(IN const AString& strName, IN IMS_SINT32 nIndex /*= 0*/) const
{
    Header* pHeader = FindHeader(strName);

    if (pHeader == IMS_NULL)
    {
        return AString::ConstNull();
    }

    return pHeader->objBodys.GetAt(nIndex);
}

PUBLIC
AString SipUnknownHeaders::GetHeaderBodys(IN IMS_SINT32 nPos) const
{
    Header* pHeader = m_objHeaders.GetAt(nPos);

    if (pHeader != IMS_NULL)
    {
        AString strBodys;

        if (!pHeader->objBodys.IsEmpty())
        {
            strBodys = pHeader->objBodys.GetAt(0);
        }

        for (IMS_UINT32 i = 1; i < pHeader->objBodys.GetSize(); ++i)
        {
            strBodys.Append(TextParser::CHAR_COMMA);
            strBodys.Append(TextParser::CHAR_SP);
            strBodys.Append(pHeader->objBodys.GetAt(i));
        }

        return strBodys;
    }

    return AString::ConstEmpty();
}

PUBLIC
AString SipUnknownHeaders::GetHeaderBodys(IN const AString& strName) const
{
    Header* pHeader = FindHeader(strName);

    if (pHeader == IMS_NULL)
    {
        return AString::ConstNull();
    }

    AString strBodys;

    if (!pHeader->objBodys.IsEmpty())
    {
        strBodys = pHeader->objBodys.GetAt(0);
    }

    for (IMS_UINT32 i = 1; i < pHeader->objBodys.GetSize(); ++i)
    {
        strBodys.Append(TextParser::CHAR_COMMA);
        strBodys.Append(TextParser::CHAR_SP);
        strBodys.Append(pHeader->objBodys.GetAt(i));
    }

    return strBodys;
}

PUBLIC
IMS_SINT32 SipUnknownHeaders::GetHeaderCount(IN const AString& strName) const
{
    Header* pHeader = FindHeader(strName);

    if (pHeader == IMS_NULL)
    {
        return 0;
    }

    return pHeader->objBodys.GetSize();
}

PUBLIC
const AString& SipUnknownHeaders::GetHeaderName(
        IN IMS_SINT32 nPos, IN IMS_BOOL bCompactForm /*= IMS_FALSE*/) const
{
    Header* pHeader = m_objHeaders.GetAt(nPos);

    if (pHeader != IMS_NULL)
    {
        if (bCompactForm)
        {
            return (pHeader->strCompactName.GetLength() > 0) ? pHeader->strCompactName
                                                             : pHeader->strName;
        }
        else
        {
            return pHeader->strName;
        }
    }

    return AString::ConstNull();
}

PUBLIC
ImsList<AString> SipUnknownHeaders::GetHeaders(IN const AString& strName) const
{
    Header* pHeader = FindHeader(strName);

    if (pHeader == IMS_NULL)
    {
        return ImsList<AString>();
    }

    return pHeader->objBodys;
}

PUBLIC
IMS_BOOL SipUnknownHeaders::OverwriteHeaders(IN const SipUnknownHeaders& objOther)
{
    for (IMS_UINT32 i = 0; i < objOther.m_objHeaders.GetSize(); ++i)
    {
        Header* pHeader = objOther.m_objHeaders.GetAt(i);

        if (pHeader != IMS_NULL)
        {
            DeleteHeader(pHeader->strName);

            for (IMS_UINT32 j = 0; j < pHeader->objBodys.GetSize(); ++j)
            {
                const AString& strBody = pHeader->objBodys.GetAt(j);

                if (AddHeader(pHeader->strName, strBody) != IMS_SUCCESS)
                {
                    return IMS_FALSE;
                }
            }
        }
    }

    return IMS_TRUE;
}

PUBLIC
IMS_RESULT SipUnknownHeaders::PrependHeader(IN const AString& strName, IN const AString& strBody)
{
    Header* pHeader = FindHeader(strName);

    if (pHeader == IMS_NULL)
    {
        pHeader = new Header(strName);

        if (pHeader == IMS_NULL)
        {
            SipPrivate::SetLastError(SipError::NO_MEMORY);
            return IMS_FAILURE;
        }

        if (!pHeader->objBodys.Prepend(strBody))
        {
            delete pHeader;

            SipPrivate::SetLastError(SipError::LIST_OPERATION_FAILED);
            return IMS_FAILURE;
        }

        if (!m_objHeaders.Append(pHeader))
        {
            delete pHeader;

            SipPrivate::SetLastError(SipError::LIST_OPERATION_FAILED);
            return IMS_FAILURE;
        }
    }
    else
    {
        if (!pHeader->objBodys.Prepend(strBody))
        {
            SipPrivate::SetLastError(SipError::LIST_OPERATION_FAILED);
            return IMS_FAILURE;
        }
    }

    return IMS_SUCCESS;
}

PUBLIC
void SipUnknownHeaders::RemoveHeader(IN const AString& strName)
{
    Header* pHeader = FindHeader(strName);

    if (pHeader == IMS_NULL)
    {
        return;
    }

    pHeader->objBodys.RemoveAt(0);

    if (pHeader->objBodys.GetSize() == 0)
    {
        DeleteHeader(strName);
    }
}

PUBLIC
IMS_RESULT SipUnknownHeaders::SetHeader(IN const AString& strName, IN const AString& strBody)
{
    Header* pHeader = FindHeader(strName);

    if (pHeader == IMS_NULL)
    {
        pHeader = new Header(strName);

        if (pHeader == IMS_NULL)
        {
            SipPrivate::SetLastError(SipError::NO_MEMORY);
            return IMS_FAILURE;
        }

        if (pHeader->objBodys.IsEmpty())
        {
            if (!pHeader->objBodys.InsertAt(strBody, 0))
            {
                delete pHeader;

                SipPrivate::SetLastError(SipError::LIST_OPERATION_FAILED);
                return IMS_FAILURE;
            }
        }
        else
        {
            if (!pHeader->objBodys.SetAt(strBody, 0))
            {
                delete pHeader;

                SipPrivate::SetLastError(SipError::LIST_OPERATION_FAILED);
                return IMS_FAILURE;
            }
        }

        if (!m_objHeaders.Append(pHeader))
        {
            delete pHeader;

            SipPrivate::SetLastError(SipError::LIST_OPERATION_FAILED);
            return IMS_FAILURE;
        }
    }
    else
    {
        if (pHeader->objBodys.IsEmpty())
        {
            if (!pHeader->objBodys.InsertAt(strBody, 0))
            {
                SipPrivate::SetLastError(SipError::LIST_OPERATION_FAILED);
                return IMS_FAILURE;
            }
        }
        else
        {
            if (!pHeader->objBodys.SetAt(strBody, 0))
            {
                SipPrivate::SetLastError(SipError::LIST_OPERATION_FAILED);
                return IMS_FAILURE;
            }
        }
    }

    return IMS_SUCCESS;
}

PRIVATE
void SipUnknownHeaders::DeleteHeader(IN const AString& strName)
{
    for (IMS_UINT32 i = 0; i < m_objHeaders.GetSize(); ++i)
    {
        Header* pHeader = m_objHeaders.GetAt(i);

        if (pHeader != IMS_NULL)
        {
            if (pHeader->Equals(strName))
            {
                delete pHeader;
                m_objHeaders.RemoveAt(i);
                return;
            }
        }
    }
}

PRIVATE
SipUnknownHeaders::Header* SipUnknownHeaders::FindHeader(IN const AString& strName) const
{
    for (IMS_UINT32 i = 0; i < m_objHeaders.GetSize(); ++i)
    {
        Header* pHeader = m_objHeaders.GetAt(i);

        if (pHeader != IMS_NULL)
        {
            if (pHeader->Equals(strName))
            {
                return pHeader;
            }
        }
    }

    return IMS_NULL;
}
