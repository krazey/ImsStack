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
#include "ServiceMutex.h"

#include "RegInfo.h"
#include "RegInfoManager.h"
#include "RegInfoParser.h"

__IMS_TRACE_TAG_REG__;

PRIVATE
RegInfoManager::RegInfoManager() :
        m_piLock(IMS_NULL),
        m_objParsers(IMSList<RegInfoParser*>()),
        m_objRegInfos(IMSMap<RegKey, RegInfo*>())
{
    m_piLock = MutexService::GetMutexService()->CreateMutex();
}

PRIVATE VIRTUAL RegInfoManager::~RegInfoManager()
{
    LockGuard objLock(m_piLock);

    if (!m_objRegInfos.IsEmpty())
    {
        for (IMS_UINT32 i = 0; i < m_objRegInfos.GetSize(); ++i)
        {
            RegInfo* pRegInfo = m_objRegInfos.GetValueAt(i);

            if (pRegInfo != IMS_NULL)
            {
                delete pRegInfo;
            }
        }

        m_objRegInfos.Clear();
    }

    if (!m_objParsers.IsEmpty())
    {
        for (IMS_UINT32 i = 0; i < m_objParsers.GetSize(); ++i)
        {
            RegInfoParser* pParser = m_objParsers.GetAt(i);

            if (pParser != IMS_NULL)
            {
                delete pParser;
            }
        }

        m_objParsers.Clear();
    }

    MutexService::GetMutexService()->DestroyMutex(m_piLock);
}

PUBLIC
IMS_BOOL RegInfoManager::CreateRegInfo(IN const RegKey& objRegKey)
{
    RegInfo* pRegInfo = const_cast<RegInfo*>(GetRegInfo(objRegKey));

    if (pRegInfo == IMS_NULL)
    {
        pRegInfo = new RegInfo();

        if (pRegInfo == IMS_NULL)
        {
            IMS_TRACE_E(0, "Creating a RegInfo(%d:%d) failed", objRegKey.GetSlotId(),
                    objRegKey.GetFlowId(), 0);
            return IMS_FALSE;
        }

        LockGuard objLock(m_piLock);

        if (!m_objRegInfos.Add(objRegKey, pRegInfo))
        {
            delete pRegInfo;

            return IMS_FALSE;
        }
    }

    return IMS_TRUE;
}

PUBLIC
void RegInfoManager::DestroyRegInfo(IN const RegKey& objRegKey)
{
    LockGuard objLock(m_piLock);
    IMS_SLONG nIndex = m_objRegInfos.GetIndexOfKey(objRegKey);

    if (nIndex < 0)
    {
        return;
    }

    RegInfo* pRegInfo = m_objRegInfos.GetValueAt(nIndex);

    if (pRegInfo != IMS_NULL)
    {
        delete pRegInfo;
    }

    m_objRegInfos.RemoveAt(nIndex);

    IMS_TRACE_I("RegInfo(%d:%d) is destroyed", objRegKey.GetSlotId(), objRegKey.GetFlowId(), 0);
}

PUBLIC
RegInfo* RegInfoManager::GetRegInfo(IN const RegKey& objRegKey)
{
    LockGuard objLock(m_piLock);
    IMS_SLONG nIndex = m_objRegInfos.GetIndexOfKey(objRegKey);

    if (nIndex < 0)
    {
        return IMS_NULL;
    }

    return m_objRegInfos.GetValueAt(nIndex);
}

PUBLIC
const RegInfo* RegInfoManager::GetRegInfo(IN const RegKey& objRegKey) const
{
    LockGuard objLock(m_piLock);
    IMS_SLONG nIndex = m_objRegInfos.GetIndexOfKey(objRegKey);

    if (nIndex < 0)
    {
        return IMS_NULL;
    }

    return m_objRegInfos.GetValueAt(nIndex);
}

PUBLIC
IMS_BOOL RegInfoManager::Initialize()
{
    return IMS_TRUE;
}

PUBLIC
IMS_BOOL RegInfoManager::Update(IN const RegKey& objRegKey, IN const AString& strRegInfo)
{
    if (strRegInfo.IsNULL())
    {
        return IMS_TRUE;
    }

    const RegInfo* pRegInfo = GetRegInfo(objRegKey);

    if (pRegInfo == IMS_NULL)
    {
        IMS_TRACE_E(
                0, "No matched RegInfo(%d:%d)", objRegKey.GetSlotId(), objRegKey.GetFlowId(), 0);
        return IMS_FALSE;
    }

    RegInfoParser* pParser = new RegInfoParser(objRegKey);

    if (pParser == IMS_NULL)
    {
        IMS_TRACE_E(0, "Creating RegInfoParser failed", 0, 0, 0);
        return IMS_FALSE;
    }

    if (!AddRegInfoParser(pParser))
    {
        delete pParser;

        return IMS_FALSE;
    }

    pParser->SetListener(this);

    if (!pParser->Parse(strRegInfo))
    {
        IMS_TRACE_E(0, "Parsing 'reginfo' failed", 0, 0, 0);

        RemoveRegInfoParser(pParser);
        return IMS_FALSE;
    }

    return IMS_TRUE;
}

PUBLIC
void RegInfoManager::DisplayRegInfo()
{
    for (IMS_UINT32 i = 0; i < m_objRegInfos.GetSize(); ++i)
    {
        RegInfo* pRegInfo = m_objRegInfos.GetValueAt(i);

        IMS_TRACE_D("___ REG INFO (%d) -- START ___", i, 0, 0);
        pRegInfo->DisplayRegInfo();
        IMS_TRACE_D("___ REG INFO (%d) -- END ___\n", i, 0, 0);
    }
}

PUBLIC GLOBAL RegInfoManager* RegInfoManager::GetInstance()
{
    static RegInfoManager* s_pRegInfoMngr = IMS_NULL;

    if (s_pRegInfoMngr == IMS_NULL)
    {
        s_pRegInfoMngr = new RegInfoManager();
    }

    return s_pRegInfoMngr;
}

PRIVATE VIRTUAL void RegInfoManager::RegInfoParser_ParsingCompleted(
        IN RegInfoParser* pParser, IN IDocument* piDocument)
{
    RegInfo* pRegInfo = GetRegInfo(pParser->GetRegKey());

    if (pRegInfo != IMS_NULL)
    {
        pRegInfo->Update(piDocument);
    }
    else
    {
        IMS_TRACE_D("No matching RegInfo(%d:%d)", pParser->GetRegKey().GetSlotId(),
                pParser->GetRegKey().GetFlowId(), 0);
    }

    RemoveRegInfoParser(pParser);
}

PRIVATE VIRTUAL void RegInfoManager::RegInfoParser_ParsingFailed(IN RegInfoParser* pParser)
{
    RegInfo* pRegInfo = GetRegInfo(pParser->GetRegKey());

    IMS_TRACE_D("Parsing 'reginfo' failed", 0, 0, 0);

    if (pRegInfo != IMS_NULL)
    {
        pRegInfo->Update(IMS_NULL);
    }

    RemoveRegInfoParser(pParser);
}

PRIVATE
IMS_BOOL RegInfoManager::AddRegInfoParser(IN RegInfoParser* pParser)
{
    LockGuard objLock(m_piLock);
    return m_objParsers.Append(pParser);
}

PRIVATE
void RegInfoManager::RemoveRegInfoParser(IN RegInfoParser*& pParser)
{
    LockGuard objLock(m_piLock);

    for (IMS_UINT32 i = 0; i < m_objParsers.GetSize(); ++i)
    {
        RegInfoParser* pTmpParser = m_objParsers.GetAt(i);

        if (pParser == pTmpParser)
        {
            delete pParser;
            pParser = IMS_NULL;

            m_objParsers.RemoveAt(i);
            return;
        }
    }
}
