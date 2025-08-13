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
#define LOG_NIDEBUG 0
#define LOG_NDDEBUG 0

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <cutils/log.h>

#include "ServiceMemory.h"
#include "ServiceMutex.h"
#include "ImsConstDef.h"
#include "ImsTraceNode.h"
#include "OsTrace.h"

class OsTraceNode : public ImsTraceNode
{
public:
    explicit OsTraceNode(IN const IMS_CHAR* pszTag) :
            ImsTraceNode(pszTag)
    {
    }
    ~OsTraceNode() override = default;

public:
    inline IMS_BOOL IsWritable() const { return (GetLength() <= MAX_TRACE_SIZE); }

protected:
    inline IMS_SINT32 Vsnprintf(OUT IMS_CHAR* pszBuffer, IN IMS_UINT32 nBuffSize,
            IN const IMS_CHAR* pszFormat, IN va_list args) override
    {
        return vsnprintf(pszBuffer, nBuffSize, pszFormat, args);
    }

public:
    // Android log system truncates log messages larger than 1k.
    static constexpr IMS_SINT32 MAX_TRACE_SIZE = 1024;
};

PUBLIC
OsTrace::OsTrace() :
        ImsTrace(),
        m_piMutex(IMS_NULL),
        m_bLogging(IMS_FALSE),
        m_objTraceNodes(ImsList<OsTraceNode*>())
{
    m_piMutex = MutexService::GetMutexService()->CreateMutex();
}

PUBLIC VIRTUAL OsTrace::~OsTrace()
{
    if (m_piMutex != IMS_NULL)
    {
        MutexService::GetMutexService()->DestroyMutex(m_piMutex);
    }
}

PUBLIC VIRTUAL void OsTrace::OutV(IN IMS_SINT32 nCategory, IN const IMS_CHAR* pszTag,
        IN IMS_UINT32 nModule, IN const IMS_CHAR* pszFile, IN IMS_UINT32 nLine,
        IN const IMS_CHAR* pszFormat, IN va_list args)
{
    (void)nModule;

#if 0
    // Check logging filters; Check only the least significant bits (7bits).
    if (!IsOptionEnabled(nCategory))
        return;

    // Check filters
    if (!IsModuleEnabled(nModule))
        return;
#endif

    // Drop the current trace node if there are many nodes
    IMS_SINT32 nTraceNodeCount = GetTraceNodeCount();

    if (nTraceNodeCount > 200)
    {
        (void)ALOG(LOG_ERROR, IMS_LOG_TAG, "[IPL] TOO MANY NODE\n");
        return;
    }

    OsTraceNode* pNode = new OsTraceNode(pszTag);

    if (pNode == IMS_NULL)
    {
        (void)ALOG(LOG_ERROR, IMS_LOG_TAG, "[IPL] ALLOCATING NODE FAILED\n");
        return;
    }

    AString strComponentName = ImsTraceNode::GetComponentName(pszFile);
    AString strSuffix;

    if (strComponentName.GetLength() > 0)
    {
        strSuffix.Sprintf(" [@%s:%d", strComponentName.GetStr(), nLine);
    }

    pNode->Format(pszFormat, args, strSuffix);

    // When the node is processing...
    if (IsLogging())
    {
        AddTraceNode(pNode);
        return;
    }

    if ((nTraceNodeCount == 0) && pNode->IsWritable())
    {
        OutputString(nCategory, pNode->GetBuffer(), pNode->GetLength());

        delete pNode;
        return;
    }

    AddTraceNode(pNode);

    ImsList<OsTraceNode*> objTmpTraceNodes;

    if (m_piMutex != IMS_NULL)
    {
        m_piMutex->Lock();
        objTmpTraceNodes = m_objTraceNodes;
        m_objTraceNodes.Clear();
        m_piMutex->Unlock();
    }
    else
    {
        objTmpTraceNodes = m_objTraceNodes;
        m_objTraceNodes.Clear();
    }

    SetLogging(IMS_TRUE);

    while (!objTmpTraceNodes.IsEmpty())
    {
        pNode = objTmpTraceNodes.GetAt(0);

        if (pNode != IMS_NULL)
        {
            OutputString(nCategory, pNode->GetBuffer(), pNode->GetLength());

            delete pNode;
        }

        objTmpTraceNodes.RemoveAt(0);
    }

    SetLogging(IMS_FALSE);
}

PRIVATE VIRTUAL const IMS_CHAR* OsTrace::GetDirName() const
{
    return IMS_SOLUTION_STORAGE_ROOT_DIR "/";
}

PRIVATE VIRTUAL void OsTrace::OutputString(IN IMS_SINT32 nCategory, IN IMS_CHAR* pszTrace,
        IN IMS_UINT32 nLength, IN const IMS_CHAR* pszLogTag /*= IMS_NULL*/)
{
    IMS_UINT32 nOption = GetOption();

    ImsTrace::OutputString(nCategory, pszTrace, nLength, pszLogTag);

    // If serial logging is set ...
    if ((nOption & ITraceOption::OPT_MEDIUM_SERIAL) != 0)
    {
        IMS_UINT32 nCurrentPos = 0;
        const IMS_UINT32 MAX_LOG_BUFF = OsTraceNode::MAX_TRACE_SIZE - 1;
        const IMS_CHAR* pszImsLogTag = (pszLogTag == IMS_NULL) ? IMS_LOG_TAG : pszLogTag;

        while (nLength > 0)
        {
            IMS_UINT32 nLengthToPrint =
                    (nLength >= OsTraceNode::MAX_TRACE_SIZE) ? MAX_LOG_BUFF : nLength;

            IMS_CHAR cCharToRestore = pszTrace[nCurrentPos + nLengthToPrint];
            pszTrace[nCurrentPos + nLengthToPrint] = '\0';

            IMS_CHAR* pszOutString = &(pszTrace[nCurrentPos]);

            nCurrentPos += nLengthToPrint;
            nLength -= nLengthToPrint;

            if (nCategory == ITrace::CAT_I)
            {
                (void)ALOG(LOG_INFO, pszImsLogTag, "%s", pszOutString);
            }
            else if (nCategory == ITrace::CAT_E)
            {
                (void)ALOG(LOG_ERROR, pszImsLogTag, "%s", pszOutString);
            }
            else
            {
                (void)ALOG(LOG_DEBUG, pszImsLogTag, "%s", pszOutString);
            }

            pszTrace[nCurrentPos] = cCharToRestore;
        }
    }
}

PRIVATE
void OsTrace::AddTraceNode(IN OsTraceNode* pNode)
{
    if (m_piMutex != IMS_NULL)
    {
        m_piMutex->Lock();
        m_objTraceNodes.Append(pNode);
        m_piMutex->Unlock();
    }
    else
    {
        m_objTraceNodes.Append(pNode);
    }
}

PRIVATE
IMS_SINT32 OsTrace::GetTraceNodeCount()
{
    IMS_SINT32 nCount = 0;

    if (m_piMutex != IMS_NULL)
    {
        m_piMutex->Lock();
        nCount = static_cast<IMS_SINT32>(m_objTraceNodes.GetSize());
        m_piMutex->Unlock();
    }
    else
    {
        nCount = static_cast<IMS_SINT32>(m_objTraceNodes.GetSize());
    }

    return nCount;
}

PRIVATE
IMS_BOOL OsTrace::IsLogging()
{
    IMS_BOOL bLoggingL;

    if (m_piMutex != IMS_NULL)
    {
        m_piMutex->Lock();
        bLoggingL = m_bLogging;
        m_piMutex->Unlock();
    }
    else
    {
        bLoggingL = m_bLogging;
    }

    return bLoggingL;
}

PRIVATE
void OsTrace::SetLogging(IN IMS_BOOL bLogging)
{
    if (m_piMutex != IMS_NULL)
    {
        m_piMutex->Lock();
        m_bLogging = bLogging;
        m_piMutex->Unlock();
    }
    else
    {
        m_bLogging = bLogging;
    }
}
