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
ImsTraceNode::ImsTraceNode(IN IMS_SINT32 nCategory, IN const IMS_CHAR* pszTag) :
        m_bAlloc(IMS_FALSE),
        m_nHeaderLength(0),
        m_nLength(0),
        m_pBuffer(&m_acBuffer[0])
{
    IMS_MEM_Memset(m_acBuffer, 0, MAX_BUFF_SIZE + 1);

    // IMS prefix
    m_pBuffer[0] = 'I';
    m_pBuffer[1] = 'M';
    m_pBuffer[2] = 'S';
    m_pBuffer[3] = '.';

    m_nHeaderLength = 4;

    // Tag
    if (pszTag != IMS_NULL)
    {
        while (*pszTag)
        {
            m_pBuffer[m_nHeaderLength++] = *pszTag;
            pszTag++;
        }
    }

    // Postfix ('.' + I / D / E)
    m_pBuffer[m_nHeaderLength++] = '.';
    m_pBuffer[m_nHeaderLength++] = (nCategory & 0xFF);

    // Add misc ('>> ')
    m_pBuffer[m_nHeaderLength++] = '>';
    m_pBuffer[m_nHeaderLength++] = '>';
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
void ImsTraceNode::Format(IN const IMS_CHAR* pszFormat, IN va_list args)
{
    static const IMS_CHAR OVERFLOW[] = "___ TRACE OVERFLOW ___";
    IMS_SINT32 nBufferCount = MAX_BUFF_SIZE - (m_nHeaderLength + CRLF_SIZE);

    m_nLength = Vsnprintf((m_pBuffer + m_nHeaderLength), nBufferCount, pszFormat, args);

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

        m_bAlloc = IMS_TRUE;
        m_pBuffer = new IMS_CHAR[m_nLength + m_nHeaderLength + CRLF_SIZE + 1];

        if (m_pBuffer == IMS_NULL)
        {
            return;
        }

        IMS_MEM_Memcpy(m_pBuffer, &m_acBuffer[0], m_nHeaderLength);
        m_pBuffer[m_nHeaderLength] = '\0';

        m_nLength = Vsnprintf((m_pBuffer + m_nHeaderLength), m_nLength, pszFormat, args);
    }

    // Calculate a total log length
    m_nLength += m_nHeaderLength;

    // Add CRLF
    m_pBuffer[m_nLength] = 0x20;
    m_pBuffer[m_nLength + 1] = 0x0a;

    // Add prefix size
    m_nLength += CRLF_SIZE;

    m_pBuffer[m_nLength] = '\0';
}
