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
#include "TextParser.h"

#include "CallControlHelper.h"
#include "ISipDialog.h"
#include "Replaces.h"

PRIVATE
CallControlHelper::CallControlHelper() :
        m_nGlobalSessionId(0),
        m_objSessions(ImsMap<AString, Replaces*>())
{
}

PRIVATE
CallControlHelper::~CallControlHelper()
{
    if (!m_objSessions.IsEmpty())
    {
        for (IMS_UINT32 i = 0; i < m_objSessions.GetSize(); ++i)
        {
            Replaces* pReplaces = m_objSessions.GetValueAt(i);

            if (pReplaces != IMS_NULL)
            {
                delete pReplaces;
            }
        }

        m_objSessions.Clear();
    }
}

PUBLIC
IMS_BOOL CallControlHelper::AddSession(IN const AString& strSessionId, IN Replaces* pReplaces)
{
    if (pReplaces == IMS_NULL)
    {
        return IMS_FALSE;
    }

    IMS_SLONG nIndex = m_objSessions.GetIndexOfKey(strSessionId);

    if (nIndex >= 0)
    {
        Replaces* pOldReplaces = m_objSessions.GetValueAt(nIndex);

        if (pOldReplaces != IMS_NULL)
        {
            delete pOldReplaces;
        }

        m_objSessions.RemoveAt(nIndex);
    }

    return m_objSessions.Add(strSessionId, pReplaces);
}

PUBLIC
void CallControlHelper::RemoveSession(IN const AString& strSessionId)
{
    IMS_SLONG nIndex = m_objSessions.GetIndexOfKey(strSessionId);

    if (nIndex < 0)
    {
        return;
    }

    Replaces* pReplaces = m_objSessions.GetValueAt(nIndex);

    if (pReplaces != IMS_NULL)
    {
        delete pReplaces;
    }

    m_objSessions.RemoveAt(nIndex);
}

PUBLIC
Replaces* CallControlHelper::GetReplacesFromSessionId(IN const AString& strSessionId)
{
    if (m_objSessions.IsEmpty())
    {
        return IMS_NULL;
    }

    IMS_SLONG nIndex = m_objSessions.GetIndexOfKey(strSessionId);

    if (nIndex < 0)
    {
        return IMS_NULL;
    }

    return m_objSessions.GetValueAt(nIndex);
}

PUBLIC
const AString& CallControlHelper::GetSessionIdFromReplaces(IN const Replaces* pReplaces)
{
    if (pReplaces == IMS_NULL)
    {
        return AString::ConstNull();
    }

    for (IMS_UINT32 i = 0; i < m_objSessions.GetSize(); ++i)
    {
        const Replaces* pTmpReplaces = m_objSessions.GetValueAt(i);

        if (pTmpReplaces == IMS_NULL)
        {
            continue;
        }

        if (pTmpReplaces->IsSameDialog(pReplaces))
        {
            return m_objSessions.GetKeyAt(i);
        }
    }

    return AString::ConstNull();
}

PUBLIC GLOBAL Replaces* CallControlHelper::CreateReplaces(IN IMS_BOOL bMo, IN ISipDialog* piDialog)
{
    (void)bMo;

    if (piDialog == IMS_NULL)
    {
        return IMS_NULL;
    }

    Replaces* pReplaces = new Replaces(piDialog->GetComponent(ISipDialog::COMPONENT_CALL_ID),
            piDialog->GetComponent(ISipDialog::COMPONENT_LOCAL_TAG),
            piDialog->GetComponent(ISipDialog::COMPONENT_REMOTE_TAG));

    if (pReplaces == IMS_NULL)
    {
        return IMS_NULL;
    }

    return pReplaces;
}

PUBLIC GLOBAL const AString CallControlHelper::CreateSessionId()
{
    CallControlHelper* pCch = CallControlHelper::GetInstance();

    pCch->m_nGlobalSessionId++;

    AString strSessionId;

    strSessionId.Sprintf("sid%08x", pCch->m_nGlobalSessionId);

    if (pCch->m_nGlobalSessionId == 0xFFFFFFFE)
    {
        pCch->m_nGlobalSessionId = 0;
    }

    return strSessionId;
}

PUBLIC GLOBAL CallControlHelper* CallControlHelper::GetInstance()
{
    static CallControlHelper* s_pCcHelper = IMS_NULL;

    if (s_pCcHelper == IMS_NULL)
    {
        s_pCcHelper = new CallControlHelper();
    }

    return s_pCcHelper;
}
