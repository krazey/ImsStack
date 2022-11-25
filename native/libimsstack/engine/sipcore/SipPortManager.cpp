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
#include "ServiceNetwork.h"
#include "ServiceSystemTime.h"
#include "ServiceTrace.h"

#include "SipPortManager.h"

__IMS_TRACE_TAG_SIP__;

PRIVATE
SipPortManager::SipPortManager() :
        m_nPortCStart(0),
        m_nPortCEnd(CLIENT_PORT_END),
        m_nNextPortC(0)
{
}

PRIVATE
SipPortManager::~SipPortManager() {}

PUBLIC
void SipPortManager::Clear()
{
    m_nPortCStart = 0;
    m_nPortCEnd = CLIENT_PORT_END;

    m_nNextPortC = 0;
}

PUBLIC
void SipPortManager::SetPortC(IN IMS_SINT32 nPortStart, IN IMS_SINT32 nPortEnd)
{
    IMS_TRACE_D("SetPortC :: Range (%d-%d)", nPortStart, nPortEnd, 0);

    m_nPortCStart = nPortStart;
    m_nPortCEnd = nPortEnd;

    if ((m_nPortCEnd <= 0) || (m_nPortCEnd > CLIENT_PORT_MAX))
    {
        m_nPortCEnd = CLIENT_PORT_END;
    }
    else if (m_nPortCStart >= m_nPortCEnd)
    {
        m_nPortCStart = 0;
    }

    // Select a starting random port number
    IMS_SINT32 nStartingPort =
            static_cast<IMS_SINT32>(IMS_SYS_GetSRandom0()) % (m_nPortCEnd - m_nPortCStart);

    nStartingPort += m_nPortCStart;

    SetNextPortC(nStartingPort);
}

PUBLIC GLOBAL SipPortManager* SipPortManager::GetInstance()
{
    static SipPortManager* s_pPortManager = IMS_NULL;

    if (s_pPortManager == IMS_NULL)
    {
        s_pPortManager = new SipPortManager();
    }

    return s_pPortManager;
}

PRIVATE
IMS_SINT32 SipPortManager::SelectNextPortC(IN const IPAddress& objIp) const
{
    IMS_SINT32 nSelectedPort = 0;
    IMS_SINT32 nCurrentPort = GetNextPortC();

    if (nCurrentPort <= 0)
    {
        for (IMS_SINT32 i = (m_nPortCStart + 1); i < m_nPortCEnd; ++i)
        {
            if (IsPortAvailable(objIp, i))
            {
                nSelectedPort = i;
                break;
            }
        }
    }
    else if (nCurrentPort < m_nPortCEnd)
    {
        for (IMS_SINT32 i = nCurrentPort; i < m_nPortCEnd; ++i)
        {
            if (IsPortAvailable(objIp, i))
            {
                nSelectedPort = i;
                break;
            }
        }
    }

    if (nSelectedPort != 0)
    {
        SetNextPortC(nSelectedPort + 1);
    }

    return nSelectedPort;
}

PRIVATE
void SipPortManager::SetNextPortC(IN IMS_SINT32 nPort) const
{
    if (m_nNextPortC != nPort)
    {
        // Round-robin
        if (nPort == m_nPortCEnd)
        {
            nPort = (m_nPortCStart + 1);
        }
        else if (nPort == m_nPortCStart)
        {
            nPort = (m_nPortCStart + 1);
        }

        IMS_TRACE_D("SetNextPortC :: %d >> %d", m_nNextPortC, nPort, 0);

        m_nNextPortC = nPort;
    }
}

PRIVATE
IMS_BOOL SipPortManager::IsPortAvailable(IN const IpAddress& objIp, IN IMS_SINT32 nPort)
{
    NetworkService* pNetworkService = NetworkService::GetNetworkService();

    if (pNetworkService == IMS_NULL)
    {
        return IMS_TRUE;
    }

    return pNetworkService->CheckIpAndPortAvailability(objIp, nPort, ISocket::TYPE_STREAM);
}
