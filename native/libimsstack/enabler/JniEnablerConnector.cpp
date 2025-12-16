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
#include "ImsMap.h"
#include "ServiceMutex.h"
#include "ServiceTrace.h"

#include "IJniEnabler.h"
#include "INativeEnabler.h"
#include "JniEnablerConnector.h"

__IMS_TRACE_TAG_USER_DECL__("JNI");

JniEnablerConnector* JniEnablerConnector::s_pConnector = IMS_NULL;

class JniConnection
{
public:
    inline explicit JniConnection(IN IMS_SINT32 nSlotId, IN EnablerType eType) :
            m_nSlotId(nSlotId),
            m_eType(eType),
            m_piNativeEnabler(IMS_NULL),
            m_objJniEnablers(ImsMap<IMS_ULONG, IJniEnabler*>())
    {
        IMS_TRACE_D("+JniConnection type[%d]", m_eType, 0, 0);
    }

    inline ~JniConnection()
    {
        IMS_TRACE_D("~JniConnection type[%d]", m_eType, 0, 0);
        m_objJniEnablers.Clear();
    }

    JniConnection(IN const JniConnection&) = delete;
    JniConnection& operator=(IN const JniConnection&) = delete;

    inline void SetJniEnabler(IN IMS_ULONG nKey, IN IJniEnabler* piJniEnabler)
    {
        IMS_SLONG nIndex = m_objJniEnablers.GetIndexOfKey(nKey);
        if (nIndex < 0)
        {
            // in unit test case, this can be null
            if (piJniEnabler != IMS_NULL)
            {
                m_objJniEnablers.Add(nKey, piJniEnabler);
            }
        }
        else
        {
            if (piJniEnabler == IMS_NULL)
            {
                m_objJniEnablers.RemoveAt(nIndex);
            }
            else
            {
                // never reach here.
                m_objJniEnablers.SetValueAt(nIndex, piJniEnabler);
            }
        }

        if (m_piNativeEnabler)
        {
            m_piNativeEnabler->NotifyJniEnablerSet();
        }
    }

    inline IJniEnabler* GetJniEnabler(IN IMS_ULONG nKey) const
    {
        IMS_SLONG nIndex = m_objJniEnablers.GetIndexOfKey(nKey);
        if (nIndex >= 0)
        {
            return m_objJniEnablers.GetValueAt(nIndex);
        }
        return IMS_NULL;
    }

    inline void SetNativeEnabler(IN INativeEnabler* piNativeEnabler)
    {
        m_piNativeEnabler = piNativeEnabler;
        for (IMS_UINT32 i = 0; i < m_objJniEnablers.GetSize(); ++i)
        {
            m_objJniEnablers.GetValueAt(i)->NotifyNativeEnablerSet();
        }
    }

    inline INativeEnabler* GetNativeEnabler() const { return m_piNativeEnabler; }

    inline IMS_BOOL IsTarget(IN IMS_SINT32 nSlotId, IN EnablerType eType) const
    {
        return m_nSlotId == nSlotId && m_eType == eType;
    }

    inline IMS_BOOL IsEmpty() const
    {
        return m_piNativeEnabler == IMS_NULL && m_objJniEnablers.GetSize() == 0;
    }

private:
    IMS_SINT32 m_nSlotId;
    EnablerType m_eType;
    INativeEnabler* m_piNativeEnabler;
    ImsMap<IMS_ULONG, IJniEnabler*> m_objJniEnablers;
};

PRIVATE JniEnablerConnector::JniEnablerConnector() :
        m_objConnections(ImsList<JniConnection*>()),
        m_piLock(MutexService::GetMutexService()->CreateMutex())
{
    IMS_TRACE_I("+JniEnablerConnector", 0, 0, 0);
}

PUBLIC JniEnablerConnector::~JniEnablerConnector()
{
    IMS_TRACE_I("~JniEnablerConnector", 0, 0, 0);
    MutexService::GetMutexService()->DestroyMutex(m_piLock);

    for (IMS_UINT32 i = 0; i < m_objConnections.GetSize(); ++i)
    {
        delete m_objConnections.GetAt(i);
    }
    m_objConnections.Clear();
    s_pConnector = IMS_NULL;
}

PUBLIC GLOBAL JniEnablerConnector& JniEnablerConnector::GetInstance()
{
    if (s_pConnector == IMS_NULL)
    {
        s_pConnector = new JniEnablerConnector();
    }
    return *s_pConnector;
}

PUBLIC
void JniEnablerConnector::SetNativeEnabler(
        IN IMS_SINT32 nSlotId, IN EnablerType eType, IN INativeEnabler* piNativeEnabler)
{
    LockGuard objLock(m_piLock);
    IMS_TRACE_D("SetNativeEnabler slot[%d] type[%d]", nSlotId, eType, 0);

    JniConnection& objConnection = CreateConnectionIfNotExist(nSlotId, eType);
    objConnection.SetNativeEnabler(piNativeEnabler);

    CheckAndRemoveConnection(&objConnection);
    // TODO: destroy this if there is no connections? Consider LockGuard.
}

PUBLIC
void JniEnablerConnector::SetJniEnabler(IN IMS_SINT32 nSlotId, IN EnablerType eType,
        IN IJniEnabler* piJniEnabler, IN IMS_ULONG nKey /* = KEY_UNIQUE*/)
{
    LockGuard objLock(m_piLock);
    IMS_TRACE_D("SetJniEnabler slot[%d] type[%d] key[%d]", nSlotId, eType, nKey);

    JniConnection& objConnection = CreateConnectionIfNotExist(nSlotId, eType);
    objConnection.SetJniEnabler(nKey, piJniEnabler);

    CheckAndRemoveConnection(&objConnection);
    // TODO: destroy this if there is no connections? Consider LockGuard.
}

PUBLIC
INativeEnabler* JniEnablerConnector::GetNativeEnabler(IN IMS_SINT32 nSlotId, IN EnablerType eType)
{
    LockGuard objLock(m_piLock);
    const JniConnection* pConnection = GetConnection(nSlotId, eType);
    if (pConnection == IMS_NULL)
    {
        return IMS_NULL;
    }
    return pConnection->GetNativeEnabler();
}

PUBLIC
IJniEnabler* JniEnablerConnector::GetJniEnabler(
        IN IMS_SINT32 nSlotId, IN EnablerType eType, IN IMS_ULONG nKey /* = KEY_UNIQUE*/)
{
    LockGuard objLock(m_piLock);
    const JniConnection* pConnection = GetConnection(nSlotId, eType);
    if (pConnection == IMS_NULL)
    {
        return IMS_NULL;
    }
    return pConnection->GetJniEnabler(nKey);
}

PRIVATE
JniConnection* JniEnablerConnector::GetConnection(IN IMS_SINT32 nSlotId, IN EnablerType eType) const
{
    for (IMS_UINT32 i = 0; i < m_objConnections.GetSize(); ++i)
    {
        JniConnection* pConnection = m_objConnections.GetAt(i);
        if (pConnection->IsTarget(nSlotId, eType))
        {
            return pConnection;
        }
    }
    return IMS_NULL;
}

PRIVATE
JniConnection& JniEnablerConnector::CreateConnectionIfNotExist(
        IN IMS_SINT32 nSlotId, IN EnablerType eType)
{
    JniConnection* pConnection = GetConnection(nSlotId, eType);
    if (pConnection == IMS_NULL)
    {
        pConnection = new JniConnection(nSlotId, eType);
        m_objConnections.Append(pConnection);
    }

    return *pConnection;
}

PRIVATE
void JniEnablerConnector::CheckAndRemoveConnection(IN JniConnection* pConnection)
{
    if (pConnection->IsEmpty() == IMS_FALSE)
    {
        return;
    }

    for (IMS_UINT32 i = 0; i < m_objConnections.GetSize(); ++i)
    {
        if (pConnection == m_objConnections.GetAt(i))
        {
            delete pConnection;
            m_objConnections.RemoveAt(i);
            return;
        }
    }
}
