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
#include "AStringBuffer.h"
#include "ServiceMemory.h"

PUBLIC
AStringBuffer::AStringBuffer() :
        m_strValue(AString::ConstNull())
{
}

PUBLIC
AStringBuffer::AStringBuffer(IN IMS_SINT32 nSize) :
        m_strValue(AString(nSize))
{
}

PUBLIC
AStringBuffer::AStringBuffer(IN const AString& strValue) :
        m_strValue(strValue)
{
}

PUBLIC
AStringBuffer::AStringBuffer(IN const AStringBuffer& other) :
        m_strValue(other.m_strValue)
{
}

PUBLIC
AStringBuffer& AStringBuffer::operator=(IN const AStringBuffer& other)
{
    if (this != &other)
    {
        m_strValue = other.m_strValue;
    }

    return (*this);
}

PUBLIC
AStringBuffer& AStringBuffer::operator=(IN const IMS_CHAR c)
{
    m_strValue = c;
    return (*this);
}

PUBLIC
AStringBuffer& AStringBuffer::operator=(IN const IMS_CHAR* pszValue)
{
    m_strValue = pszValue;
    return (*this);
}

PUBLIC
AStringBuffer& AStringBuffer::operator=(IN const AString& strValue)
{
    m_strValue = strValue;
    return (*this);
}

PUBLIC
AStringBuffer& AStringBuffer::operator+=(IN const AStringBuffer& other)
{
    m_strValue += other.m_strValue;
    return (*this);
}

PUBLIC
// NOLINTNEXTLINE(cert-dcl50-cpp)
AStringBuffer& AStringBuffer::Sprintf(IN const IMS_CHAR* pszFormat, ...)
{
    va_list ap;

    va_start(ap, pszFormat);
    m_strValue.Vsprintf(pszFormat, ap);
    va_end(ap);

    return (*this);
}

PUBLIC
AStringBuffer& AStringBuffer::Append(IN const IMS_SINT16 nValue)
{
    AString strTmpVal;

    strTmpVal.SetNumber(nValue);
    m_strValue.Append(strTmpVal);

    return (*this);
}

PUBLIC
AStringBuffer& AStringBuffer::Append(IN const IMS_UINT16 nValue)
{
    AString strTmpVal;

    strTmpVal.SetNumber(nValue);
    m_strValue.Append(strTmpVal);

    return (*this);
}

PUBLIC
AStringBuffer& AStringBuffer::Append(IN const IMS_SINT32 nValue)
{
    AString strTmpVal;

    strTmpVal.SetNumber(nValue);
    m_strValue.Append(strTmpVal);

    return (*this);
}

PUBLIC
AStringBuffer& AStringBuffer::Append(IN const IMS_UINT32 nValue)
{
    AString strTmpVal;

    strTmpVal.SetNumber(nValue);
    m_strValue.Append(strTmpVal);

    return (*this);
}

PUBLIC
AStringBuffer& AStringBuffer::Append(IN const IMS_SLONG nValue)
{
    AString strTmpVal;

    strTmpVal.SetNumber(static_cast<IMS_SINT64>(nValue));
    m_strValue.Append(strTmpVal);

    return (*this);
}

PUBLIC
AStringBuffer& AStringBuffer::Append(IN const IMS_ULONG nValue)
{
    AString strTmpVal;

    strTmpVal.SetNumber(static_cast<IMS_UINT64>(nValue));
    m_strValue.Append(strTmpVal);

    return (*this);
}

PUBLIC
AStringBuffer& AStringBuffer::Insert(IN IMS_SINT32 i, IN const IMS_SINT16 nValue)
{
    AString strTmpVal;

    strTmpVal.SetNumber(nValue);
    m_strValue.Insert(i, strTmpVal);

    return (*this);
}

PUBLIC
AStringBuffer& AStringBuffer::Insert(IN IMS_SINT32 i, IN const IMS_UINT16 nValue)
{
    AString strTmpVal;

    strTmpVal.SetNumber(nValue);
    m_strValue.Insert(i, strTmpVal);

    return (*this);
}

PUBLIC
AStringBuffer& AStringBuffer::Insert(IN IMS_SINT32 i, IN const IMS_SINT32 nValue)
{
    AString strTmpVal;

    strTmpVal.SetNumber(nValue);
    m_strValue.Insert(i, strTmpVal);

    return (*this);
}

PUBLIC
AStringBuffer& AStringBuffer::Insert(IN IMS_SINT32 i, IN const IMS_UINT32 nValue)
{
    AString strTmpVal;

    strTmpVal.SetNumber(nValue);
    m_strValue.Insert(i, strTmpVal);

    return (*this);
}

PUBLIC
AStringBuffer& AStringBuffer::Insert(IN IMS_SINT32 i, IN const IMS_SLONG nValue)
{
    AString strTmpVal;

    strTmpVal.SetNumber(static_cast<IMS_SINT64>(nValue));
    m_strValue.Insert(i, strTmpVal);

    return (*this);
}

PUBLIC
AStringBuffer& AStringBuffer::Insert(IN IMS_SINT32 i, IN const IMS_ULONG nValue)
{
    AString strTmpVal;

    strTmpVal.SetNumber(static_cast<IMS_UINT64>(nValue));
    m_strValue.Insert(i, strTmpVal);

    return (*this);
}
