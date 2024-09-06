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
#include "ServiceNetwork.h"

#include "IMediaNetworkConnectionListener.h"
#include "MediaNetworkConnectionWatcher.h"

__IMS_TRACE_TAG_MEDIA__;

PUBLIC MediaNetworkConnectionWatcher::MediaNetworkConnectionWatcher(
        IN const IpAddress& objIpAddress) :
        m_piListener(IMS_NULL),
        m_pNetConnection(NetworkService::GetNetworkService()->FindConnection(objIpAddress)),
        m_nMediaConnectionType(0),
        m_nMtu(0)
{
    UpdateParameters(m_pNetConnection);

    if (m_pNetConnection != NULL)
    {
        m_pNetConnection->AddReferenceListener(this);
    }
}

PUBLIC MediaNetworkConnectionWatcher::~MediaNetworkConnectionWatcher()
{
    IMS_TRACE_D("~NetConnectionWatcher()", 0, 0, 0);

    if (m_pNetConnection != IMS_NULL)
    {
        m_pNetConnection->RemoveReferenceListener(this);
    }

    m_pNetConnection = IMS_NULL;
}

PUBLIC void MediaNetworkConnectionWatcher::SetListener(
        IN IMediaNetworkConnectionListener* piListener)
{
    IMS_TRACE_D("SetListener()", 0, 0, 0);

    m_piListener = piListener;
}

PUBLIC void MediaNetworkConnectionWatcher::NetworkConnection_OnConnected(
        IN INetworkConnection* pNetConnection)
{
    UpdateParameters(pNetConnection);
}

PUBLIC void MediaNetworkConnectionWatcher::NetworkConnection_OnDisconnected(
        IN INetworkConnection* pNetConnection, IN IMS_SINT32 nErrorCode)
{
    (void)pNetConnection;
    (void)nErrorCode;
    /** do nothing */
}

PUBLIC void MediaNetworkConnectionWatcher::NetworkConnection_OnConnectionFailed(
        IN INetworkConnection* pNetConnection, IN IMS_SINT32 nErrorCode)
{
    (void)pNetConnection;
    (void)nErrorCode;
    /** do nothing */
}

PUBLIC void MediaNetworkConnectionWatcher::NetworkConnection_OnIpChanged(
        IN INetworkConnection* pNetConnection)
{
    UpdateParameters(pNetConnection);
}

PUBLIC void MediaNetworkConnectionWatcher::NetworkConnection_OnIpcanChanged(
        IN INetworkConnection* pNetConnection)
{
    UpdateParameters(pNetConnection);
}

PUBLIC void MediaNetworkConnectionWatcher::NetworkConnection_OnPcscfChanged(
        IN INetworkConnection* pNetConnection)
{
    (void)pNetConnection;
    /** do nothing */
}

PRIVATE IMS_SINT32 MediaNetworkConnectionWatcher::ConvertNetworkType(
        IN INetworkConnection* pNetConnection)
{
    if (pNetConnection == IMS_NULL)
    {
        return UNKNOWN;
    }

    AString strRAT = AString::ConstNull();
    IMS_SINT32 NetworkType = UNKNOWN;

    pNetConnection->GetExtraInfo(AString("rat"), strRAT);

    if (strRAT.EqualsIgnoreCase(AString("WiFi")) || pNetConnection->IsePDGEnabled())
    {
        NetworkType = IWLAN;
    }
    else if (strRAT.EqualsIgnoreCase(AString("LTE")))
    {
        NetworkType = EUTRAN;
    }
    else if (strRAT.EqualsIgnoreCase(AString("NR")))
    {
        NetworkType = NGRAN;
    }
    else if (strRAT.EqualsIgnoreCase(AString("3G")))
    {
        NetworkType = UTRAN;
    }
    else if (strRAT.EqualsIgnoreCase(AString("2G")))
    {
        NetworkType = GERAN;
    }

    return NetworkType;
}

PRIVATE const IMS_CHAR* MediaNetworkConnectionWatcher::PrintNetworkType(
        IN IMS_SINT32 nMediaConnectionType)
{
    switch (nMediaConnectionType)
    {
        case UNKNOWN:
            return "UNKNOWN";
        case GERAN:
            return "GERAN";
        case UTRAN:
            return "UTRAN";
        case EUTRAN:
            return "EUTRAN";
        case CDMA2000:
            return "CDMA2000";
        case IWLAN:
            return "IWLAN";
        case NGRAN:
            return "NGRAN";
        default:
            return "UNKNOWN";
    }
}

PRIVATE void MediaNetworkConnectionWatcher::UpdateParameters(IN INetworkConnection* pNetConnection)
{
    if (pNetConnection == IMS_NULL)
    {
        return;
    }

    IMS_SINT32 nMediaConnectionType = ConvertNetworkType(pNetConnection);
    IMS_SINT32 nMtu = pNetConnection->GetMtu();

    IMS_TRACE_D("UpdateParameters() - type[%s], mtu[%d]", PrintNetworkType(nMediaConnectionType),
            nMtu, 0);

    if (nMediaConnectionType != m_nMediaConnectionType)
    {
        m_nMediaConnectionType = nMediaConnectionType;

        if (m_piListener != IMS_NULL)
        {
            m_piListener->OnNetworkConnectionChanged(m_nMediaConnectionType);
        }
    }

    if (nMtu != m_nMtu)
    {
        m_nMtu = nMtu;

        if (m_piListener != IMS_NULL)
        {
            m_piListener->OnMediaMtuChanged(nMtu);
        }
    }
}
