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
#include "ImsNetworkConnectionState.h"
#include "ServiceMemory.h"
#include "ServiceMutex.h"

GLOBAL void ImsNetworkConnectionState_ExitInstance()
{
    ImsNetworkConnectionState* pState = ImsNetworkConnectionState::GetInstance();

    if (pState->m_piLock != IMS_NULL)
    {
        MutexService::GetMutexService()->DestroyMutex(pState->m_piLock);
        pState->m_piLock = IMS_NULL;
    }
}

GLOBAL IMS_BOOL ImsNetworkConnectionState_InitInstance()
{
    ImsNetworkConnectionState* pState = ImsNetworkConnectionState::GetInstance();

    pState->m_piLock = MutexService::GetMutexService()->CreateMutex();

    if (pState->m_piLock == IMS_NULL)
    {
        return IMS_FALSE;
    }

    return IMS_TRUE;
}

PRIVATE
ImsNetworkConnectionState::ImsNetworkConnectionState() :
        m_piLock(IMS_NULL),
        m_objNetConnectionList(IMSList<ImsNetworkConnection*>()),
        m_nHandleForMobile(HANDLE_MOBILE_MIN),
        m_nHandleForWiFi(HANDLE_WIFI_MIN)
{
}

PUBLIC
ImsNetworkConnectionState::~ImsNetworkConnectionState() {}

PUBLIC
void ImsNetworkConnectionState::AttachHandle(IN ImsNetworkConnection* pConnection)
{
    LockGuard objLock(m_piLock);

    m_objNetConnectionList.Append(pConnection);
}

PUBLIC
void ImsNetworkConnectionState::DetachAll()
{
    IMSList<ImsNetworkConnection*> objNetConnections;

    {
        LockGuard objLock(m_piLock);

        if (!m_objNetConnectionList.IsEmpty())
        {
            for (IMS_UINT32 i = 0; i < m_objNetConnectionList.GetSize(); ++i)
            {
                objNetConnections.Append(m_objNetConnectionList.GetAt(i));
            }

            m_objNetConnectionList.Clear();
        }
    }

    for (IMS_UINT32 i = 0; i < objNetConnections.GetSize(); ++i)
    {
        ImsNetworkConnection* pConnection = objNetConnections.GetAt(i);

        if (pConnection != IMS_NULL)
        {
            delete pConnection;
        }
    }
}

PUBLIC
void ImsNetworkConnectionState::DetachHandle(IN const AString& strNetProfile, IN IMS_SINT32 nSlotId)
{
    LockGuard objLock(m_piLock);

    for (IMS_UINT32 i = 0; i < m_objNetConnectionList.GetSize(); ++i)
    {
        ImsNetworkConnection* pConnection = m_objNetConnectionList.GetAt(i);

        if (pConnection != IMS_NULL)
        {
            if ((nSlotId == pConnection->GetSlotId()) &&
                    strNetProfile.EqualsIgnoreCase(pConnection->GetProfileName()))
            {
                m_objNetConnectionList.RemoveAt(i);
                return;
            }
        }
    }
}

PUBLIC
ImsNetworkConnection* ImsNetworkConnectionState::LookupHandle(IN IMS_CONNECTION hConnection)
{
    LockGuard objLock(m_piLock);

    for (IMS_UINT32 i = 0; i < m_objNetConnectionList.GetSize(); ++i)
    {
        ImsNetworkConnection* pConnection = m_objNetConnectionList.GetAt(i);

        if (pConnection != IMS_NULL)
        {
            if (hConnection == pConnection->GetHandle())
            {
                return pConnection;
            }
        }
    }

    return IMS_NULL;
}

PUBLIC
ImsNetworkConnection* ImsNetworkConnectionState::LookupHandle(IN const IPAddress& objIpAddr)
{
    LockGuard objLock(m_piLock);

    for (IMS_UINT32 i = 0; i < m_objNetConnectionList.GetSize(); ++i)
    {
        ImsNetworkConnection* pConnection = m_objNetConnectionList.GetAt(i);

        if (pConnection != IMS_NULL)
        {
            if (pConnection->Equals(objIpAddr))
            {
                return pConnection;
            }
        }
    }

    return IMS_NULL;
}

PUBLIC
ImsNetworkConnection* ImsNetworkConnectionState::LookupHandle(
        IN const AString& strNetProfile, IN IMS_SINT32 nSlotId)
{
    if (strNetProfile.GetLength() == 0)
    {
        return IMS_NULL;
    }

    LockGuard objLock(m_piLock);

    for (IMS_UINT32 i = 0; i < m_objNetConnectionList.GetSize(); ++i)
    {
        ImsNetworkConnection* pConnection = m_objNetConnectionList.GetAt(i);

        if (pConnection != IMS_NULL)
        {
            if ((nSlotId == pConnection->GetSlotId()) &&
                    strNetProfile.EqualsIgnoreCase(pConnection->GetProfileName()))
            {
                return pConnection;
            }
        }
    }

    return IMS_NULL;
}

ImsNetworkConnection* ImsNetworkConnectionState::LookupHandle(
        IN IMS_SINT32 nApnType, IN IMS_SINT32 nSlotId)
{
    LockGuard objLock(m_piLock);

    for (IMS_UINT32 i = 0; i < m_objNetConnectionList.GetSize(); ++i)
    {
        ImsNetworkConnection* pConnection = m_objNetConnectionList.GetAt(i);

        if (pConnection != IMS_NULL)
        {
            if ((nSlotId == pConnection->GetSlotId()) && (nApnType == pConnection->GetApnType()))
            {
                return pConnection;
            }
        }
    }

    return IMS_NULL;
}

PUBLIC
IMS_BOOL ImsNetworkConnectionState::IsHandlePresent(IN IMS_CONNECTION hConnection)
{
    LockGuard objLock(m_piLock);

    for (IMS_UINT32 i = 0; i < m_objNetConnectionList.GetSize(); ++i)
    {
        ImsNetworkConnection* pConnection = m_objNetConnectionList.GetAt(i);

        if (pConnection != IMS_NULL)
        {
            if (hConnection == pConnection->GetHandle())
            {
                return IMS_TRUE;
            }
        }
    }

    return IMS_FALSE;
}

PUBLIC
IMS_BOOL ImsNetworkConnectionState::IsEmpty() const
{
    LockGuard objLock(m_piLock);
    return m_objNetConnectionList.IsEmpty();
}

IMS_CONNECTION ImsNetworkConnectionState::GetAndIncrementHandle(IN IMS_BOOL bMobile /*= IMS_TRUE*/)
{
    IMS_UINT32 nMinHandle = bMobile ? HANDLE_MOBILE_MIN : HANDLE_WIFI_MIN;
    IMS_UINT32 nMaxHandle = bMobile ? HANDLE_MOBILE_MAX : HANDLE_WIFI_MAX;
    IMS_UINT32 nNextHandle = GetNextHandle(bMobile);

    for (IMS_UINT32 i = nNextHandle; i <= nMaxHandle; i++)
    {
        if (!IsHandlePresent(static_cast<IMS_CONNECTION>(i)))
        {
            if (i == nMaxHandle)
            {
                SetNextHandle(nMinHandle, bMobile);
            }
            else
            {
                SetNextHandle(i + 1, bMobile);
            }

            return static_cast<IMS_CONNECTION>(i);
        }
    }

    for (IMS_UINT32 i = nMinHandle; i < nNextHandle; i++)
    {
        if (!IsHandlePresent(static_cast<IMS_CONNECTION>(i)))
        {
            SetNextHandle(i + 1, bMobile);
            return static_cast<IMS_CONNECTION>(i);
        }
    }

    return static_cast<IMS_CONNECTION>(0);
}

PUBLIC GLOBAL ImsNetworkConnectionState* ImsNetworkConnectionState::GetInstance()
{
    static ImsNetworkConnectionState* s_pState = IMS_NULL;

    if (s_pState == IMS_NULL)
    {
        s_pState = new ImsNetworkConnectionState();
    }

    return s_pState;
}

PUBLIC
IMS_UINT32 ImsNetworkConnectionState::GetNextHandle(IN IMS_BOOL bMobile /*= IMS_TRUE*/)
{
    LockGuard objLock(m_piLock);

    return bMobile ? m_nHandleForMobile : m_nHandleForWiFi;
}

PUBLIC
void ImsNetworkConnectionState::SetNextHandle(
        IN IMS_UINT32 nHandle, IN IMS_BOOL bMobile /*= IMS_TRUE*/)
{
    LockGuard objLock(m_piLock);

    if (bMobile)
    {
        m_nHandleForMobile = nHandle;
    }
    else
    {
        m_nHandleForWiFi = nHandle;
    }
}
