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
#include "ImsLib.h"
#include "ImsStrLib.h"
#include "ServiceMemory.h"

// clang-format off
PRIVATE GLOBAL ByteArray::Data ByteArray::SHARED_NULL =
{
    0, 0, ByteArray::SHARED_NULL.byValue,
    {
        0
    }
};
// clang-format on

static IMS_SINT32 byteArray_AllocateMore(IN IMS_SIZE_T nAlloc, IN IMS_SIZE_T nExtra)
{
    const IMS_SIZE_T nPage = 1 << 12;  // 4096 bytes
    IMS_SIZE_T nNumAlloc;

    nAlloc += nExtra;

    if (nAlloc < (1 << 5))  // 32 bytes
    {
        nNumAlloc = (1 << 3) + ((nAlloc >> 3) << 3);
    }
    else
    {
        nNumAlloc = (nAlloc < nPage) ? (1 << 3) : nPage;

        while (nNumAlloc < nAlloc)
        {
            nNumAlloc *= 2;
        }
    }

    return LONG_TO_INT(nNumAlloc - nExtra);
}

PUBLIC
ByteArray::ByteRef::ByteRef(IN ByteArray& objBa, IN IMS_SINT32 nIndex) :
        m_objBa(objBa),
        m_nIndex(nIndex)
{
}

PUBLIC
// This assignment operator is not used to change the "m_nIndex",
// but to change the character itself.
// cppcheck-suppress operatorEqVarError
ByteArray::ByteRef& ByteArray::ByteRef::operator=(IN const ByteRef& other)
{
    if (this != &other)
    {
        m_objBa.m_pData->pValue[m_nIndex] = other.m_objBa.m_pData->pValue[other.m_nIndex];
    }

    return (*this);
}

PUBLIC
ByteArray::ByteRef& ByteArray::ByteRef::operator=(IN IMS_BYTE byte)
{
    m_objBa.m_pData->pValue[m_nIndex] = byte;
    return (*this);
}

PUBLIC
ByteArray::ByteArray() :
        m_pData(&SHARED_NULL)
{
}

PUBLIC
ByteArray::ByteArray(IN IMS_BYTE byte)
{
    m_pData = static_cast<Data*>(IMS_MEM_Malloc(sizeof(Data) + 1));

    if (m_pData == IMS_NULL)
    {
        m_pData = &SHARED_NULL;
    }
    else
    {
        m_pData->nAlloc = m_pData->nSize = 1;
        m_pData->pValue = m_pData->byValue;
        m_pData->byValue[0] = byte;
        m_pData->byValue[1] = '\0';
    }
}

PUBLIC
ByteArray::ByteArray(IN const IMS_CHAR* pValue)
{
    IMS_UINT32 nSize = IMS_StrLen(pValue);

    if ((pValue == IMS_NULL) || (nSize == 0))
    {
        m_pData = &SHARED_NULL;
    }
    else
    {
        m_pData = static_cast<Data*>(IMS_MEM_Malloc(sizeof(Data) + nSize));

        if (m_pData == IMS_NULL)
        {
            m_pData = &SHARED_NULL;
        }
        else
        {
            m_pData->nAlloc = m_pData->nSize = nSize;
            m_pData->pValue = m_pData->byValue;
            IMS_MEM_Memcpy(m_pData->byValue, pValue, nSize);
            m_pData->byValue[nSize] = '\0';
        }
    }
}

PUBLIC
ByteArray::ByteArray(IN const AString& strValue)
{
    IMS_SINT32 nSize = strValue.GetLength();

    if (nSize == 0)
    {
        m_pData = &SHARED_NULL;
    }
    else
    {
        m_pData = static_cast<Data*>(IMS_MEM_Malloc(sizeof(Data) + nSize));

        if (m_pData == IMS_NULL)
        {
            m_pData = &SHARED_NULL;
        }
        else
        {
            m_pData->nAlloc = m_pData->nSize = nSize;
            m_pData->pValue = m_pData->byValue;
            IMS_MEM_Memcpy(m_pData->byValue, strValue.GetStr(), nSize);
            m_pData->byValue[nSize] = '\0';
        }
    }
}

PUBLIC
ByteArray::ByteArray(IN const IMS_BYTE* pValue, IN IMS_SINT32 nSize)
{
    if ((pValue == IMS_NULL) || (nSize <= 0))
    {
        m_pData = &SHARED_NULL;
    }
    else
    {
        m_pData = static_cast<Data*>(IMS_MEM_Malloc(sizeof(Data) + nSize));

        if (m_pData == IMS_NULL)
        {
            m_pData = &SHARED_NULL;
        }
        else
        {
            m_pData->nAlloc = m_pData->nSize = nSize;
            m_pData->pValue = m_pData->byValue;
            IMS_MEM_Memcpy(m_pData->byValue, pValue, nSize);
            m_pData->byValue[nSize] = '\0';
        }
    }
}

PUBLIC
ByteArray::ByteArray(IN const ByteArray& other)
{
    if (other.m_pData == &SHARED_NULL)
    {
        m_pData = &SHARED_NULL;
    }
    else
    {
        m_pData = static_cast<Data*>(IMS_MEM_Malloc(sizeof(Data) + other.m_pData->nAlloc));

        if (m_pData == IMS_NULL)
        {
            m_pData = &SHARED_NULL;
        }
        else
        {
            m_pData->nAlloc = other.m_pData->nAlloc;
            m_pData->nSize = other.m_pData->nSize;
            m_pData->pValue = m_pData->byValue;
            IMS_MEM_Memcpy(m_pData->byValue, other.m_pData->pValue, m_pData->nSize);
            m_pData->byValue[m_pData->nSize] = '\0';
        }
    }
}

PUBLIC
ByteArray::~ByteArray()
{
    Clear();
}

PUBLIC
ByteArray& ByteArray::operator=(IN const ByteArray& other)
{
    if (this != &other)
    {
        if ((other.m_pData->nSize == 0) || (other.m_pData->pValue == IMS_NULL))
        {
            Clear();
        }
        else
        {
            IMS_SINT32 nLen = other.m_pData->nSize;

            if ((nLen > m_pData->nAlloc) ||
                    ((nLen < m_pData->nSize) && (nLen < (m_pData->nAlloc >> 1))))
            {
                Realloc(nLen);
            }

            IMS_MEM_Memcpy(m_pData->pValue, other.m_pData->pValue, nLen);
            m_pData->nSize = nLen;
            m_pData->pValue[m_pData->nSize] = '\0';
        }
    }

    return (*this);
}

PUBLIC
ByteArray& ByteArray::operator=(IN IMS_BYTE byte)
{
    if ((1 > m_pData->nAlloc) || ((1 < m_pData->nSize) && (1 < (m_pData->nAlloc >> 1))))
    {
        Realloc(1);
    }

    m_pData->pValue[0] = byte;
    m_pData->pValue[1] = '\0';
    m_pData->nSize = 1;

    return (*this);
}

PUBLIC
IMS_BYTE ByteArray::operator[](IN IMS_SINT32 nIndex) const
{
    IMS_ASSERT(nIndex >= 0 && nIndex < GetLength());

    return m_pData->pValue[nIndex];
}

PUBLIC
ByteArray::ByteRef ByteArray::operator[](IN IMS_SINT32 nIndex)
{
    IMS_ASSERT(nIndex >= 0 && nIndex < GetLength());

    return ByteRef(*this, nIndex);
}

PUBLIC
IMS_BYTE ByteArray::operator[](IN IMS_UINT32 nIndex) const
{
    IMS_ASSERT(nIndex < IMS_UINT32(GetLength()));

    return m_pData->pValue[nIndex];
}

PUBLIC
ByteArray::ByteRef ByteArray::operator[](IN IMS_UINT32 nIndex)
{
    IMS_ASSERT(nIndex < IMS_UINT32(GetLength()));

    return ByteRef(*this, nIndex);
}

PUBLIC
ByteArray& ByteArray::Append(IN IMS_BYTE byte)
{
    if (m_pData->nSize + 1 > m_pData->nAlloc)
    {
        Realloc(byteArray_AllocateMore(m_pData->nSize + 1, sizeof(Data)));
    }

    m_pData->pValue[m_pData->nSize++] = byte;
    m_pData->pValue[m_pData->nSize] = '\0';

    return (*this);
}

PUBLIC
ByteArray& ByteArray::Append(IN const IMS_BYTE* pValue, IN IMS_SINT32 nSize)
{
    if ((nSize <= 0) || (pValue == IMS_NULL))
    {
        return (*this);
    }

    if (m_pData->nSize + nSize > m_pData->nAlloc)
    {
        Realloc(byteArray_AllocateMore(m_pData->nSize + nSize, sizeof(Data)));
    }

    IMS_MEM_Memcpy(m_pData->pValue + m_pData->nSize, pValue, nSize);
    m_pData->nSize += nSize;
    m_pData->pValue[m_pData->nSize] = '\0';

    return (*this);
}

PUBLIC
ByteArray& ByteArray::Append(IN const ByteArray& objBa)
{
    if (m_pData == &SHARED_NULL)
    {
        (*this) = objBa;
    }
    else if (objBa.m_pData != &SHARED_NULL)
    {
        if (m_pData->nSize + objBa.m_pData->nSize > m_pData->nAlloc)
        {
            Realloc(byteArray_AllocateMore(m_pData->nSize + objBa.m_pData->nSize, sizeof(Data)));
        }

        IMS_MEM_Memcpy(
                m_pData->pValue + m_pData->nSize, objBa.m_pData->pValue, objBa.m_pData->nSize);
        m_pData->nSize += objBa.m_pData->nSize;
        m_pData->pValue[m_pData->nSize] = '\0';
    }

    return (*this);
}

PUBLIC
ByteArray& ByteArray::Erase(IN IMS_SINT32 nOffset, IN IMS_SINT32 nCount)
{
    if ((nCount <= 0) || (nOffset >= m_pData->nSize) || (nOffset < 0))
    {
        return (*this);
    }

    if ((nOffset + nCount) >= m_pData->nSize)
    {
        Resize(nOffset);
    }
    else
    {
        IMS_MEM_Memmove(m_pData->pValue + nOffset, m_pData->pValue + nOffset + nCount,
                m_pData->nSize - nOffset - nCount);
        Resize(m_pData->nSize - nCount);
    }

    return (*this);
}

PUBLIC
ByteArray& ByteArray::Prepend(IN IMS_BYTE byte)
{
    if (m_pData->nSize + 1 > m_pData->nAlloc)
    {
        Realloc(byteArray_AllocateMore(m_pData->nSize + 1, sizeof(Data)));
    }

    IMS_MEM_Memmove(m_pData->pValue + 1, m_pData->pValue, m_pData->nSize);
    m_pData->pValue[0] = byte;
    ++(m_pData->nSize);
    m_pData->pValue[m_pData->nSize] = '\0';

    return (*this);
}

PUBLIC
ByteArray& ByteArray::Prepend(IN const IMS_BYTE* pValue, IN IMS_SINT32 nSize)
{
    if ((nSize <= 0) || (pValue == IMS_NULL))
    {
        return (*this);
    }

    if (m_pData->nSize + nSize > m_pData->nAlloc)
    {
        Realloc(byteArray_AllocateMore(m_pData->nSize + nSize, sizeof(Data)));
    }

    IMS_MEM_Memmove(m_pData->pValue + nSize, m_pData->pValue, m_pData->nSize);
    IMS_MEM_Memcpy(m_pData->pValue, pValue, nSize);
    m_pData->nSize += nSize;
    m_pData->pValue[m_pData->nSize] = '\0';

    return (*this);
}

PUBLIC
ByteArray& ByteArray::Prepend(IN const ByteArray& objBa)
{
    if (m_pData == &SHARED_NULL)
    {
        (*this) = objBa;
    }
    else if (objBa.m_pData != &SHARED_NULL)
    {
        ByteArray objTmp = (*this);

        (*this) = objBa;
        Append(objTmp);
    }

    return (*this);
}

PUBLIC
void ByteArray::Attach(IN const IMS_BYTE* pValue, IN IMS_SINT32 nSize)
{
    Data* pNewData = static_cast<Data*>(IMS_MEM_Malloc(sizeof(Data)));

    if (pValue)
    {
        pNewData->pValue = const_cast<IMS_BYTE*>(pValue);
    }
    else
    {
        pNewData->pValue = pNewData->byValue;
        nSize = 0;
    }

    pNewData->nAlloc = pNewData->nSize = nSize;
    pNewData->byValue[0] = '\0';

    Clear();

    m_pData = pNewData;
}

PUBLIC
void ByteArray::Detach()
{
    if (m_pData->pValue != m_pData->byValue)
    {
        Realloc(m_pData->nSize);
    }
}

PUBLIC
void ByteArray::Resize(IN IMS_SINT32 nSize)
{
    if (nSize <= 0)
    {
        if (m_pData == &SHARED_NULL)
        {
            return;
        }

        Clear();
    }
    else
    {
        if ((nSize > m_pData->nAlloc) ||
                ((nSize < m_pData->nSize) && (nSize < (m_pData->nAlloc >> 1))))
        {
            Realloc(byteArray_AllocateMore(nSize, sizeof(Data)));
        }

        if (m_pData->nAlloc >= nSize)
        {
            m_pData->nSize = nSize;
            m_pData->pValue = m_pData->byValue;
            m_pData->byValue[nSize] = '\0';
        }
    }
}

PUBLIC
ByteArray ByteArray::GetSubData(IN IMS_SINT32 nOffset, IN IMS_SINT32 nCount /*= -1*/) const
{
    if ((m_pData == &SHARED_NULL) || (nOffset >= m_pData->nSize))
    {
        return ByteArray::ConstNull();
    }

    if (nCount < 0)
    {
        nCount = m_pData->nSize - nOffset;
    }

    if (nOffset < 0)
    {
        nCount += nOffset;
        nOffset = 0;
    }

    if ((nCount + nOffset) > m_pData->nSize)
    {
        nCount = m_pData->nSize - nOffset;
    }

    if ((nOffset == 0) && (nCount == m_pData->nSize))
    {
        return (*this);
    }

    return ByteArray(m_pData->pValue + nOffset, nCount);
}

PUBLIC
IMS_BOOL ByteArray::Equals(IN IMS_BYTE byte) const
{
    if (m_pData->nSize != 1)
    {
        return IMS_FALSE;
    }

    return (m_pData->pValue[0] == byte);
}

PUBLIC
IMS_BOOL ByteArray::Equals(IN const ByteArray& objBa) const
{
    if (m_pData == objBa.m_pData)
    {
        return IMS_TRUE;
    }

    if (m_pData->nSize != objBa.m_pData->nSize)
    {
        return IMS_FALSE;
    }

    return IMS_MEM_Memcmp(m_pData->pValue, objBa.m_pData->pValue, m_pData->nSize) == 0;
}

PUBLIC
AString ByteArray::ToString() const
{
    if (m_pData == &SHARED_NULL)
    {
        return AString::ConstNull();
    }

    return AString(reinterpret_cast<const IMS_CHAR*>(m_pData->pValue), m_pData->nSize);
}

PUBLIC
AString ByteArray::ToHexString() const
{
    if (m_pData == &SHARED_NULL)
    {
        return AString::ConstNull();
    }

    AString strTemp;
    AString strHexString;

    for (IMS_SINT32 i = 0; i < m_pData->nSize; ++i)
    {
        strTemp.Sprintf("%02x", m_pData->pValue[i]);
        strHexString.Append(strTemp);
    }

    return strHexString;
}

PUBLIC GLOBAL const ByteArray& ByteArray::ConstNull()
{
    static const ByteArray CONST_NULL;
    return CONST_NULL;
}

PRIVATE
void ByteArray::Clear()
{
    if (m_pData == &SHARED_NULL)
    {
        return;
    }

    IMS_MEM_Free(m_pData);
    m_pData = &SHARED_NULL;
}

PRIVATE
void ByteArray::Realloc(IN IMS_SINT32 nAlloc)
{
    Data* pNewData;

    if ((m_pData == &SHARED_NULL) || (m_pData->pValue != m_pData->byValue))
    {
        pNewData = static_cast<Data*>(IMS_MEM_Malloc(sizeof(Data) + nAlloc));

        if (pNewData == IMS_NULL)
        {
            return;
        }

        pNewData->nSize = IMS_MIN(nAlloc, m_pData->nSize);
        IMS_MEM_Memcpy(pNewData->byValue, m_pData->pValue, pNewData->nSize);
        pNewData->byValue[pNewData->nSize] = '\0';
        pNewData->nAlloc = nAlloc;
        pNewData->pValue = pNewData->byValue;

        Clear();
    }
    else
    {
        pNewData = static_cast<Data*>(IMS_MEM_Realloc(m_pData, sizeof(Data) + nAlloc));

        if (pNewData == IMS_NULL)
        {
            return;
        }

        pNewData->nAlloc = nAlloc;
        pNewData->pValue = pNewData->byValue;
    }

    m_pData = pNewData;
}
