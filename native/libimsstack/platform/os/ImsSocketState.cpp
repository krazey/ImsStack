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
#include "ImsSocketState.h"
#include "ServiceMemory.h"
#include "ServiceMutex.h"
#include "ServiceTrace.h"

__IMS_TRACE_TAG_IPL__;

GLOBAL void ImsSocketState_ExitInstance()
{
    ImsSocketState* pState = ImsSocketState::GetInstance();

    pState->DestroyDeadSockets();

    if (pState->m_piLock != IMS_NULL)
    {
        MutexService::GetMutexService()->DestroyMutex(pState->m_piLock);
        pState->m_piLock = IMS_NULL;
    }
}

GLOBAL IMS_BOOL ImsSocketState_InitInstance()
{
    ImsSocketState* pState = ImsSocketState::GetInstance();

    pState->m_piLock = MutexService::GetMutexService()->CreateMutex();

    if (pState->m_piLock == IMS_NULL)
    {
        return IMS_FALSE;
    }

    return IMS_TRUE;
}

PRIVATE
ImsSocketState::ImsSocketState() :
        m_piLock(IMS_NULL),
        m_objHandle2Object(ImsMap<IMS_SOCKET, ImsSocket*>()),
        m_objDeadSockets(ImsList<ImsSocket*>())
{
}

PUBLIC
ImsSocketState::~ImsSocketState() {}

PUBLIC
void ImsSocketState::AddDeadSocket(IN ImsSocket* pSocket)
{
    LockGuard objLock(m_piLock);

    m_objDeadSockets.Append(pSocket);

    IMS_TRACE_I("DEAD SOCKET (%p) :: ADD (size=%d)", pSocket, m_objDeadSockets.GetSize(), 0);
}

PUBLIC
void ImsSocketState::DestroyDeadSockets()
{
    ImsList<ImsSocket*> objTmpDeadSockets;

    {
        LockGuard objLock(m_piLock);

        if (!m_objDeadSockets.IsEmpty())
        {
            objTmpDeadSockets = m_objDeadSockets;
            m_objDeadSockets.Clear();
        }
    }

    if (!objTmpDeadSockets.IsEmpty())
    {
        for (IMS_UINT32 i = 0; i < objTmpDeadSockets.GetSize(); ++i)
        {
            ImsSocket* pSocket = objTmpDeadSockets.GetAt(i);

            if (pSocket != IMS_NULL)
            {
                IMS_TRACE_D("DEAD SOCKET (%p) :: DESTROY", pSocket, 0, 0);
                delete pSocket;
            }
        }
    }
}

PUBLIC
void ImsSocketState::AttachHandle(IN IMS_SOCKET hSocket, IN ImsSocket* pSocket)
{
    LockGuard objLock(m_piLock);

    m_objHandle2Object.Add(hSocket, pSocket);

    IMS_TRACE_I("SOCKET (%u, %p) IS ATTACHED (size=%d)", hSocket, pSocket,
            m_objHandle2Object.GetSize());
}

PUBLIC
void ImsSocketState::DetachAll()
{
    ImsList<ImsSocket*> objSockets;

    {
        LockGuard objLock(m_piLock);

        if (!m_objHandle2Object.IsEmpty())
        {
            for (IMS_UINT32 i = 0; i < m_objHandle2Object.GetSize(); ++i)
            {
                objSockets.Append(m_objHandle2Object.GetValueAt(i));
            }

            m_objHandle2Object.Clear();
        }
    }

    for (IMS_UINT32 i = 0; i < objSockets.GetSize(); ++i)
    {
        ImsSocket* pSocket = objSockets.GetAt(i);

        if (pSocket != IMS_NULL)
        {
            pSocket->Abort();
            delete pSocket;
        }
    }
}

PUBLIC
void ImsSocketState::DetachAll(IN IMS_CONNECTION hConnection)
{
    ImsList<ImsSocket*> objSockets;

    {
        LockGuard objLock(m_piLock);

        if (!m_objHandle2Object.IsEmpty())
        {
            for (IMS_UINT32 i = 0; i < m_objHandle2Object.GetSize(); ++i)
            {
                ImsSocket* pSocket = m_objHandle2Object.GetValueAt(i);

                if ((pSocket != IMS_NULL) && (pSocket->GetNetworkConnection() == hConnection))
                {
                    objSockets.Append(pSocket);
                }
            }
        }
    }

    for (IMS_UINT32 i = 0; i < objSockets.GetSize(); ++i)
    {
        ImsSocket* pSocket = objSockets.GetAt(i);

        if (pSocket != IMS_NULL)
        {
            pSocket->ClosedByDataConnection();
        }
    }
}

PUBLIC
void ImsSocketState::DetachHandle(IN IMS_SOCKET hSocket)
{
    LockGuard objLock(m_piLock);

    m_objHandle2Object.Remove(hSocket);

    IMS_TRACE_I("SOCKET (%u) IS DETACHED (size=%d)", hSocket, m_objHandle2Object.GetSize(), 0);
}

PUBLIC
ImsSocket* ImsSocketState::LookupHandle(IN IMS_SOCKET hSocket)
{
    ImsSocket* pSocket = IMS_NULL;
    IMS_SLONG nKeyIndex;

    LockGuard objLock(m_piLock);

    if ((nKeyIndex = m_objHandle2Object.GetIndexOfKey(hSocket)) >= 0)
    {
        pSocket = m_objHandle2Object.GetValueAt(nKeyIndex);
    }

    return pSocket;
}

PUBLIC
IMS_BOOL ImsSocketState::IsEmpty() const
{
    LockGuard objLock(m_piLock);

    return m_objHandle2Object.IsEmpty();
}

PUBLIC GLOBAL ImsSocketState* ImsSocketState::GetInstance()
{
    static ImsSocketState* s_pSocketState = IMS_NULL;

    if (s_pSocketState == IMS_NULL)
    {
        s_pSocketState = new ImsSocketState();
    }

    return s_pSocketState;
}
