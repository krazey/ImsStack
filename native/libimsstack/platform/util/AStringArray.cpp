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
#include "AStringArray.h"
#include "AStringBuffer.h"
#include "ServiceMemory.h"

PUBLIC
AStringArray::AStringArray() :
        m_objElements(ImsList<AString>())
{
}

PUBLIC
AStringArray::AStringArray(IN const ImsList<AString>& objElements) :
        m_objElements(objElements)
{
}

PUBLIC
AStringArray::AStringArray(IN const AStringArray& other) :
        m_objElements(other.m_objElements)
{
}

PUBLIC
AStringArray::~AStringArray()
{
    RemoveAllElements();
}

PUBLIC
AStringArray& AStringArray::operator=(IN const AStringArray& other)
{
    if (this != &other)
    {
        m_objElements = other.m_objElements;
    }

    return (*this);
}

PUBLIC
AStringArray& AStringArray::operator=(IN const ImsList<AString>& objElements)
{
    m_objElements = objElements;
    return (*this);
}

PUBLIC
IMS_BOOL AStringArray::Contains(
        IN const AString& strElem, IN IMS_BOOL bCaseSensitive /*= IMS_TRUE*/) const
{
    if (bCaseSensitive)
    {
        for (IMS_UINT32 i = 0; i < m_objElements.GetSize(); ++i)
        {
            const AString& strExValue = m_objElements.GetAt(i);

            if (strExValue.Equals(strElem))
            {
                return IMS_TRUE;
            }
        }
    }
    else
    {
        for (IMS_UINT32 i = 0; i < m_objElements.GetSize(); ++i)
        {
            const AString& strExValue = m_objElements.GetAt(i);

            if (strExValue.EqualsIgnoreCase(strElem))
            {
                return IMS_TRUE;
            }
        }
    }

    return IMS_FALSE;
}

PUBLIC
const AString& AStringArray::GetFirstElement() const
{
    if (m_objElements.GetSize() == 0)
    {
        return AString::ConstNull();
    }

    return m_objElements.GetAt(0);
}

PUBLIC
IMS_SINT32 AStringArray::GetIndexOf(IN const AString& strElem, IN IMS_SINT32 nOffset /*= 0*/,
        IN IMS_BOOL bCaseSensitive /*= IMS_TRUE*/) const
{
    if (nOffset < 0)
    {
        nOffset = 0;
    }

    if (bCaseSensitive)
    {
        for (IMS_UINT32 i = nOffset; i < m_objElements.GetSize(); ++i)
        {
            const AString& strExValue = m_objElements.GetAt(i);

            if (strExValue.Equals(strElem))
            {
                return i;
            }
        }
    }
    else
    {
        for (IMS_UINT32 i = nOffset; i < m_objElements.GetSize(); ++i)
        {
            const AString& strExValue = m_objElements.GetAt(i);

            if (strExValue.EqualsIgnoreCase(strElem))
            {
                return i;
            }
        }
    }

    return AString::NPOS;
}

PUBLIC
const AString& AStringArray::GetLastElement() const
{
    if (m_objElements.GetSize() == 0)
    {
        return AString::ConstNull();
    }

    return m_objElements.GetAt(m_objElements.GetSize() - 1);
}

PUBLIC
IMS_SINT32 AStringArray::GetLastIndexOf(IN const AString& strElem, IN IMS_SINT32 nOffset /*= 0*/,
        IN IMS_BOOL bCaseSensitive /*= IMS_TRUE*/) const
{
    if (nOffset < 0)
    {
        nOffset = 0;
    }

    if (bCaseSensitive)
    {
        for (IMS_SINT32 i = m_objElements.GetSize(); i >= nOffset; --i)
        {
            const AString& strExValue = m_objElements.GetAt(i);

            if (strExValue.Equals(strElem))
            {
                return i;
            }
        }
    }
    else
    {
        for (IMS_SINT32 i = m_objElements.GetSize(); i >= nOffset; --i)
        {
            const AString& strExValue = m_objElements.GetAt(i);

            if (strExValue.EqualsIgnoreCase(strElem))
            {
                return i;
            }
        }
    }

    return AString::NPOS;
}

PUBLIC
IMS_BOOL AStringArray::RemoveElement(
        IN const AString& strElem, IN IMS_BOOL bCaseSensitive /*= IMS_TRUE*/)
{
    if (bCaseSensitive)
    {
        for (IMS_UINT32 i = 0; i < m_objElements.GetSize(); ++i)
        {
            const AString& strExValue = m_objElements.GetAt(i);

            if (strExValue.Equals(strElem))
            {
                m_objElements.RemoveAt(i);
                return IMS_TRUE;
            }
        }
    }
    else
    {
        for (IMS_UINT32 i = 0; i < m_objElements.GetSize(); ++i)
        {
            const AString& strExValue = m_objElements.GetAt(i);

            if (strExValue.EqualsIgnoreCase(strElem))
            {
                m_objElements.RemoveAt(i);
                return IMS_TRUE;
            }
        }
    }

    return IMS_FALSE;
}

PUBLIC
AString AStringArray::ToString(IN IMS_CHAR cDelimiter /*= TextParser::CHAR_COMMA*/) const
{
    AStringBuffer objBuffer(128);

    for (IMS_UINT32 i = 0; i < m_objElements.GetSize(); ++i)
    {
        objBuffer.Append(m_objElements.GetAt(i));
        objBuffer.Append(cDelimiter);
    }

    if (objBuffer.GetLength() > 0)
    {
        objBuffer.Chop(1);
    }

    return static_cast<const AStringBuffer&>(objBuffer).GetString();
}

PUBLIC GLOBAL const AStringArray& AStringArray::ConstNull()
{
    static const AStringArray CONST_NULL;
    return CONST_NULL;
}
