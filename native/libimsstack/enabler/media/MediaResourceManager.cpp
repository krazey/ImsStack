/**
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

#include "ServiceTrace.h"
#include "ServiceSystemTime.h"
#include "ServiceNetworkPolicy.h"

#include "IJniMedia.h"
#include "MediaManager.h"
#include "MediaNetworkConnectionWatcher.h"
#include "MediaResourceManager.h"
#include "config/MediaConfiguration.h"

#define MEDIA_RESOURCEMNGR_IP_ADDR_LEN 46

__IMS_TRACE_TAG_USER_DECL__("MED.RM");

PUBLIC MediaResourceManager::MediaResourceManager(IN IMS_SINT32 nSlotId) :
        m_nSlotId(nSlotId),
        m_nPdnType(-1),
        m_bIsIpv6(IMS_FALSE),
        m_nNetworkType(MediaNetworkConnectionWatcher::UNKNOWN),
        m_nMtu(1500),
        m_lstUsedRtpPort(ImsList<IMS_UINT32>()),
        m_pNetworkConnectionWatcher(IMS_NULL)
{
}

PUBLIC MediaResourceManager::~MediaResourceManager()
{
    if (m_pNetworkConnectionWatcher != IMS_NULL)
    {
        delete m_pNetworkConnectionWatcher;
    }
}

PUBLIC IMS_UINT32 MediaResourceManager::AcquireRtpPort(IN MediaConfiguration* pConfig)
{
    if (pConfig == IMS_NULL)
    {
        IMS_TRACE_D("AcquireRtpPort() - config is null", 0, 0, 0);
        return 0;
    }

    return MediaResourceManager::AcquireRtpPort(pConfig->GetPortRtp(), pConfig->GetPortRtpEnd());
}

PUBLIC IMS_UINT32 MediaResourceManager::AcquireRtpPort(
        IN IMS_UINT32 nRangeStart, IN IMS_UINT32 nRangeEnd)
{
    const IMS_UINT32 RTP_PORT_MAX = 0xffff;
    IMS_UINT32 nInitialPort = 0;
    IMS_UINT32 nChosenPort = 0;
    IMS_BOOL bFoundSamePort = IMS_FALSE;

    IMS_TRACE_D("AcquireRtpPort() - nRangeStart[%d], nRangeEnd[%d]", nRangeStart, nRangeEnd, 0);

    // STEP 0. Exception handling : INVALID case
    if (nRangeStart > RTP_PORT_MAX || nRangeEnd > RTP_PORT_MAX)
    {
        IMS_TRACE_E(0, "AcquireRtpPort() - out of start[%d], end[%d]", nRangeStart, nRangeEnd, 0);
        return 0;
    }
    else if (nRangeStart > nRangeEnd)
    {  // when Start Port is bigger than End Port, swapping them.
        IMS_UINT32 nTempPort = nRangeStart;
        nRangeStart = nRangeEnd;
        nRangeEnd = nTempPort;
    }
    // Exception handling : Only One Case
    else if (nRangeStart == nRangeEnd)
    {
        m_lstUsedRtpPort.Append(nRangeStart);
        return nRangeStart;
    }

    // STEP 1. Get Number between start and end number according to random rule.
    nInitialPort = nRangeStart + (IMS_SYS_GetRandom0() % (nRangeEnd - nRangeStart));

    // STEP 2. Adjust selected RTP port to a even number
    if (nInitialPort % 2 == 1)
    {
        nInitialPort++;
    }

    // STEP 3. Compare this number to activated RTP ports.
    //      If found a same number, increase number and re-find.
    nChosenPort = nInitialPort;

    do
    {
        bFoundSamePort = IMS_FALSE;
        for (IMS_UINT32 i = 0; i < m_lstUsedRtpPort.GetSize(); i++)
        {
            IMS_UINT32 nPort = m_lstUsedRtpPort.GetAt(i);
            if (nPort == nChosenPort)
            {
                IMS_TRACE_D("AcquireRtpPort() - Found same port[%d / %d]", i, nPort, 0);
                bFoundSamePort = IMS_TRUE;
                break;
            }
        }

        if (bFoundSamePort == IMS_TRUE)
        {
            // Increase RTP Port
            nChosenPort += 2;
            if (nChosenPort > nRangeEnd)
            {
                nChosenPort = nRangeStart;
                if (nChosenPort % 2 == 1)
                {
                    nChosenPort++;
                }
            }

            // No more exist usable port, then use the initial port.
            if (nChosenPort == nInitialPort)
            {
                nChosenPort = 0;
                break;
            }
        }
    } while (bFoundSamePort == IMS_TRUE);

    // STEP 4. Save a selected RTP port to list
    if (nChosenPort != 0)
    {
        m_lstUsedRtpPort.Append(nChosenPort);
    }

    IMS_TRACE_D("AcquireRtpPort() - nChosenPort[%d]", nChosenPort, 0, 0);

    return nChosenPort;
}

PUBLIC IMS_BOOL MediaResourceManager::ReleaseRtpPort(IN IMS_UINT32 nPort)
{
    IMS_TRACE_D("ReleaseRtpPort() - port[%d], list size[%d]", nPort, m_lstUsedRtpPort.GetSize(), 0);

    for (IMS_UINT32 i = 0; i < m_lstUsedRtpPort.GetSize(); i++)
    {
        IMS_UINT32 nListPort = m_lstUsedRtpPort.GetAt(i);

        if (nListPort == nPort)
        {
            m_lstUsedRtpPort.RemoveAt(i);
            return IMS_TRUE;
        }
    }

    if (nPort != 0)
    {
        IMS_TRACE_D("ReleaseRtpPort() - no matched port[%d]", nPort, 0, 0);
    }

    return IMS_FALSE;
}

PUBLIC IMS_BOOL MediaResourceManager::UpdatePdn(
        IN IMS_SINT32 nPdnType, IN const IpAddress& objIpAddress)
{
    if (m_nPdnType == nPdnType)
    {
        return IMS_TRUE;
    }

    m_nPdnType = nPdnType;
    m_bIsIpv6 = objIpAddress.IsIPv6Address();

    if (m_pNetworkConnectionWatcher != IMS_NULL)
    {
        delete m_pNetworkConnectionWatcher;
    }

    IMS_TRACE_D("UpdatePdn() - PdnType[%d], bIpv6[%d]", m_nPdnType, m_bIsIpv6, 0);

    m_pNetworkConnectionWatcher = new MediaNetworkConnectionWatcher(objIpAddress);
    m_pNetworkConnectionWatcher->SetListener(this);
    m_nNetworkType = m_pNetworkConnectionWatcher->GetNetworkType();
    m_nMtu = m_pNetworkConnectionWatcher->GetMtu();
    IMS_TRACE_D("UpdatePdn() - Network[%d], Mtu[%d]", m_nNetworkType, m_nMtu, 0);

    return IMS_TRUE;
}

IMS_SINT32 MediaResourceManager::GetNetworkType()
{
    return m_nNetworkType;
}

IMS_SINT32 MediaResourceManager::GetMtu()
{
    return m_nMtu;
}

void MediaResourceManager::OnNetworkConnectionChanged(IN const IMS_UINT32 nRatType)
{
    IMS_TRACE_D("OnNetworkConnectionChanged() - NetworkType[%d]", nRatType, 0, 0);
    m_nNetworkType = nRatType;

    MediaManager::GetInstance(m_nSlotId)->SendMessage(
            IJniMedia::CHANGE_NETWORK_CONNECTION, 0, nRatType);
}

void MediaResourceManager::OnMediaMtuChanged(IN const IMS_UINT32 nMtu)
{
    IMS_TRACE_D("OnMediaMtuChanged() - Mtu[%d]", nMtu, 0, 0);
    m_nMtu = nMtu;

    MediaManager::GetInstance(m_nSlotId)->SendMessage(IJniMedia::CHANGE_MTU, 0, nMtu);
}
