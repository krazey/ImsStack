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
#include "ImsTraceNode.h"
#include "ServiceMemory.h"
#include "ServiceTrace.h"

PUBLIC
ImsTraceNode::ImsTraceNode(IN const IMS_CHAR* pszTag) :
        m_bAlloc(IMS_FALSE),
        m_nHeaderLength(0),
        m_nLength(0),
        m_pBuffer(&m_acBuffer[0])
{
    IMS_MEM_Memset(m_acBuffer, 0, MAX_BUFF_SIZE + 1);

    m_pBuffer[0] = '[';
    m_nHeaderLength = 1;

    // Tag
    if (pszTag != IMS_NULL)
    {
        while (*pszTag)
        {
            m_pBuffer[m_nHeaderLength++] = *pszTag;
            pszTag++;
        }
    }

    m_pBuffer[m_nHeaderLength++] = ']';
    m_pBuffer[m_nHeaderLength++] = ' ';
    m_pBuffer[m_nHeaderLength] = '\0';
}

PUBLIC VIRTUAL ImsTraceNode::~ImsTraceNode()
{
    if (m_bAlloc && (m_pBuffer != IMS_NULL))
    {
        delete[] m_pBuffer;
        m_pBuffer = IMS_NULL;
    }
}

PUBLIC
void ImsTraceNode::Format(IN const IMS_CHAR* pszFormat, IN va_list args,
        IN const AString& strSuffix /*= AString::ConstNull()*/)
{
    static const IMS_CHAR OVERFLOW[] = "___ TRACE OVERFLOW ___";
    // +1: one line-feed (LF)
    IMS_SINT32 nExtraLogSize = m_nHeaderLength + strSuffix.GetLength() + 1;
    IMS_SINT32 nBufferCount = MAX_BUFF_SIZE - nExtraLogSize;
    // It's possible for methods that use a va_list to invalidate the data in it upon use.
    // So, this copy will be used to call the next vsnprintf.
    va_list backup_args;
    va_copy(backup_args, args);
    m_nLength = Vsnprintf((m_pBuffer + m_nHeaderLength), nBufferCount, pszFormat, backup_args);
    va_end(backup_args);

    if (m_nLength < 0)
    {
        m_nLength = 22;
        IMS_MEM_Memcpy((m_pBuffer + m_nHeaderLength), OVERFLOW, m_nLength);
    }
    else if (m_nLength > nBufferCount)
    {
        if (m_bAlloc && (m_pBuffer != IMS_NULL))
        {
            delete[] m_pBuffer;
            m_pBuffer = IMS_NULL;
        }

        IMS_SINT32 nNewBufferSize = m_nLength + nExtraLogSize + 1;
        m_bAlloc = IMS_TRUE;
        m_pBuffer = new IMS_CHAR[nNewBufferSize];

        if (m_pBuffer == IMS_NULL)
        {
            return;
        }

        IMS_MEM_Memset(m_pBuffer, 0, nNewBufferSize);
        IMS_MEM_Memcpy(m_pBuffer, &m_acBuffer[0], m_nHeaderLength);
        va_copy(backup_args, args);
        m_nLength = Vsnprintf((m_pBuffer + m_nHeaderLength), m_nLength, pszFormat, backup_args);
        va_end(backup_args);
    }

    // Calculate a total log length
    m_nLength += m_nHeaderLength;

    // Add suffix
    if (strSuffix.GetLength() > 0)
    {
        IMS_MEM_Memcpy(&m_pBuffer[m_nLength], strSuffix.GetStr(), strSuffix.GetLength());
        m_nLength += strSuffix.GetLength();
    }

    // Add line-feed (LF)
    m_pBuffer[m_nLength] = 0x0a;
    m_nLength += 1;

    m_pBuffer[m_nLength] = '\0';
}

PUBLIC GLOBAL AString ImsTraceNode::GetComponentName(IN const IMS_CHAR* pszComponent)
{
    AString strName(pszComponent);

    if (strName.GetLength() > 0)
    {
        IMS_SINT32 nIndex = strName.GetLastIndexOf('/');

        if (nIndex != AString::NPOS)
        {
            strName = strName.GetSubStr(nIndex + 1);
        }

        nIndex = strName.GetLastIndexOf('.');

        if (nIndex != AString::NPOS)
        {
            strName = strName.GetSubStr(0, nIndex);
        }
    }

    return strName;
}
