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

// == INCLUDES =========================================================
#include "IMSMap.h"
#include "MediaConnectionWatcher.h"
#include "IMediaConnectionWatcherListener.h"
#include "ServiceNetwork.h"
#include "ServiceTrace.h"
#include "ServicePhoneInfo.h"
#include "ServiceEvent.h"

/////////////////////////////////////////////////
// For Calculating MTU SIZE

// Enable only 1 item
// #define CALCULATE_EPDG_MTU_FROM_DATA
#define CALCULATE_EPDG_MTU_FIXED_VALUE

#define MTU_MOBILE     1500
#define MTU_EPDG       1280
#define SIZE_OF_IP_SEC 60
#define SIZE_OF_IPV6   60
#define SIZE_OF_IPV4   40
#define SIZE_OF_RTP    20
#define SIZE_OF_GUARD  200
//////////////////////////////////////////////////

// == DEFINES =========================================================
__IMS_TRACE_TAG_USER_DECL__("MED.MF.BASE");

static IMSMap<IMS_SINT32, MediaConnectionWatcher*>* g_pMapMediaConnectionWatcher = IMS_NULL;

// == CREATOR ==============================================================
PRIVATE
MediaConnectionWatcher::MediaConnectionWatcher()
//: m_bVoWIFISupport(IMS_TRUE) // to get VoWIFI Support
{
    IMS_TRACE_D("MediaConnectionWatcher()", 0, 0, 0);
    objWatchers.Clear();
}

PRIVATE
MediaConnectionWatcher::~MediaConnectionWatcher()
{
    IMS_TRACE_D("~MediaConnectionWatcher()", 0, 0, 0);
    objWatchers.Clear();

    for (IMS_UINT32 nIdx = 0; nIdx < g_pMapMediaConnectionWatcher->GetSize(); nIdx++)
    {
        if (g_pMapMediaConnectionWatcher->GetIndexOfKey(nIdx) >= 0)
        {
            MediaConnectionWatcher* pWatcher = g_pMapMediaConnectionWatcher->GetValue(nIdx);

            if (pWatcher == this)
            {
                g_pMapMediaConnectionWatcher->Remove(nIdx);
            }
        }
    }

    if (g_pMapMediaConnectionWatcher->IsEmpty())
    {
        delete g_pMapMediaConnectionWatcher;
        g_pMapMediaConnectionWatcher = IMS_NULL;
    }
}

// == PUBLIC METHOD ==============================================================
PUBLIC GLOBAL MediaConnectionWatcher* MediaConnectionWatcher::GetMediaConnectionWatcher(
        IN IMS_SINT32 nSlotId)
{
    if (g_pMapMediaConnectionWatcher == IMS_NULL)
    {
        g_pMapMediaConnectionWatcher = new IMSMap<IMS_SINT32, MediaConnectionWatcher*>();
    }

    if (g_pMapMediaConnectionWatcher->GetIndexOfKey(nSlotId) < 0)
    {
        MediaConnectionWatcher* pMediaConnectionWatcher = new MediaConnectionWatcher();
        g_pMapMediaConnectionWatcher->Add(nSlotId, pMediaConnectionWatcher);
    }

    return g_pMapMediaConnectionWatcher->GetValue(nSlotId);
}

PUBLIC VIRTUAL IMS_BOOL MediaConnectionWatcher::GetMediaConnectionType(IN AString& strPdn,
        IN IMS_SINT32 nSlotID, OUT INetworkConnection*& piNetConnection,
        OUT IMS_SINT32& nMediaConnectionType, OUT IMS_UINT32& nNetworkInterfaceId)
{
    NetConnectionWatcher* pNetConnectionWatcher = IMS_NULL;
    IMS_BOOL bRet = IMS_TRUE;

    if (strPdn.IsNULL() || strPdn.IsEmpty())
    {
        IMS_TRACE_E(0, "CreateSessionGetMediaConnectionType() - PDN is NULL", 0, 0, 0);
        nNetworkInterfaceId = 0;
        nMediaConnectionType = MEDIA_CONNECTION_INVALID;
        bRet = IMS_FALSE;
        goto EXIT_GetMediaConnectionType_PDN;
    }

    piNetConnection = NetworkService::GetNetworkService()->FindConnection(strPdn, nSlotID);
    if (piNetConnection == IMS_NULL)
    {
        IMS_TRACE_E(0, "CreateSessionGetMediaConnectionType() - piNetConnction is NULL", 0, 0, 0);
        nNetworkInterfaceId = 0;
        nMediaConnectionType = MEDIA_CONNECTION_INVALID;
        bRet = IMS_FALSE;
        goto EXIT_GetMediaConnectionType_PDN;
    }

    nMediaConnectionType = convertNetworkType(piNetConnection);
    nNetworkInterfaceId = piNetConnection->GetIfaceId();
    pNetConnectionWatcher = findNetConnectionWatcher(piNetConnection);

    if (pNetConnectionWatcher != NULL)
    {
        nMediaConnectionType = pNetConnectionWatcher->GetMediaConnectionType();
    }

    IMS_TRACE_I("GetMediaConnectionType() - (strPdn(%s)) = %s /  %d", strPdn.GetStr(),
            printNetworkType(nMediaConnectionType), nNetworkInterfaceId);

EXIT_GetMediaConnectionType_PDN:
    return bRet;
}

PUBLIC VIRTUAL IMS_BOOL MediaConnectionWatcher::GetMediaConnectionType(IN IPAddress& objIpAddress,
        OUT INetworkConnection*& piNetConnection, OUT IMS_SINT32& nMediaConnectionType,
        OUT IMS_UINT32& nNetworkInterfaceId)
{
    NetConnectionWatcher* pNetConnectionWatcher = IMS_NULL;
    IMS_BOOL bRet = IMS_TRUE;

    if (objIpAddress.IsNoneAddress() || objIpAddress.IsUnknownAddress())
    {
        nNetworkInterfaceId = 0;
        nMediaConnectionType = MEDIA_CONNECTION_INVALID;
        bRet = IMS_FALSE;
        goto EXIT_GetMediaConnectionType_IPADDR;
    }

    piNetConnection = NetworkService::GetNetworkService()->FindConnection(objIpAddress);

    if (piNetConnection == IMS_NULL)
    {
        IMS_TRACE_E(0, "GetMediaConnectionType() - piNetConnction is NULL", 0, 0, 0);
        nNetworkInterfaceId = 0;
        nMediaConnectionType = MEDIA_CONNECTION_INVALID;
        bRet = IMS_FALSE;
        goto EXIT_GetMediaConnectionType_IPADDR;
    }

    nMediaConnectionType = convertNetworkType(piNetConnection);
    nNetworkInterfaceId = piNetConnection->GetIfaceId();
    pNetConnectionWatcher = findNetConnectionWatcher(piNetConnection);

    if (pNetConnectionWatcher != NULL)
    {
        nMediaConnectionType = pNetConnectionWatcher->GetMediaConnectionType();
    }

    IMS_TRACE_I("GetMediaConnectionType() - (objIpAddress(%s)) = %s /  %d",
            objIpAddress.ToString().GetStr(), printNetworkType(nMediaConnectionType),
            nNetworkInterfaceId);

EXIT_GetMediaConnectionType_IPADDR:
    return bRet;
}

PUBLIC VIRTUAL IMS_BOOL MediaConnectionWatcher::GetMediaConnectionType(
        IN INetworkConnection* piNetConnection, OUT IMS_SINT32& nMediaConnectionType,
        OUT IMS_UINT32& nNetworkInterfaceId)
{
    NetConnectionWatcher* pNetConnectionWatcher = IMS_NULL;
    IMS_BOOL bRet = IMS_TRUE;

    if (piNetConnection == IMS_NULL)
    {
        IMS_TRACE_E(0, "GetMediaConnectionType() - piNetConnction is NULL", 0, 0, 0);
        nMediaConnectionType = MEDIA_CONNECTION_INVALID;
        nNetworkInterfaceId = 0;
        bRet = IMS_FALSE;
        goto EXIT_GetMediaConnectionType_INETCON;
    }

    nMediaConnectionType = convertNetworkType(piNetConnection);
    nNetworkInterfaceId = piNetConnection->GetIfaceId();
    pNetConnectionWatcher = findNetConnectionWatcher(piNetConnection);
    if (pNetConnectionWatcher != NULL)
    {
        nMediaConnectionType = pNetConnectionWatcher->GetMediaConnectionType();
    }

    IMS_TRACE_I("GetMediaConnectionType() = %s /  %d", printNetworkType(nMediaConnectionType),
            nNetworkInterfaceId, 0);

EXIT_GetMediaConnectionType_INETCON:
    return bRet;
}

PUBLIC VIRTUAL IMS_BOOL MediaConnectionWatcher::GetRtpFragmentSize(IN AString& strPdn,
        IN IMS_SINT32 nSlotID, OUT INetworkConnection*& piNetConnection,
        OUT IMS_SINT32& nRtpFragmentSize)
{
    NetConnectionWatcher* pNetConnectionWatcher = IMS_NULL;
    IMS_BOOL bRet = IMS_TRUE;
    nRtpFragmentSize = 0;

    if (strPdn.IsNULL() || strPdn.IsEmpty())
    {
        IMS_TRACE_E(0, "GetRtpFragmentSize() - PDN is NULL", 0, 0, 0);
        bRet = IMS_FALSE;
        goto EXIT_GetRtpFragmentSize_PDN;
    }

    piNetConnection = NetworkService::GetNetworkService()->FindConnection(strPdn, nSlotID);
    if (piNetConnection == IMS_NULL)
    {
        IMS_TRACE_E(0, "GetRtpFragmentSize() - piNetConnection is NULL", 0, 0, 0);
        bRet = IMS_FALSE;
        goto EXIT_GetRtpFragmentSize_PDN;
    }

    pNetConnectionWatcher = findNetConnectionWatcher(piNetConnection);
    if (pNetConnectionWatcher != NULL)
    {
        nRtpFragmentSize = CalculateRtpFragmentSize(pNetConnectionWatcher);
    }
    else
    {
        nRtpFragmentSize = CalculateRtpFragmentSize(piNetConnection);
    }

    IMS_TRACE_I("GetRtpFragmentSize() - strPdn(%s) nRtpFragmentSize(%d)", strPdn.GetStr(),
            nRtpFragmentSize, 0);

EXIT_GetRtpFragmentSize_PDN:
    return bRet;
}

PUBLIC VIRTUAL IMS_BOOL MediaConnectionWatcher::GetRtpFragmentSize(IN IPAddress& objIpAddress,
        OUT INetworkConnection*& piNetConnection, OUT IMS_SINT32& nRtpFragmentSize)
{
    NetConnectionWatcher* pNetConnectionWatcher = IMS_NULL;
    IMS_BOOL bRet = IMS_TRUE;
    nRtpFragmentSize = 0;

    if (objIpAddress.IsNoneAddress() || objIpAddress.IsUnknownAddress())
    {
        bRet = IMS_FALSE;
        goto EXIT_GetRtpFragmentSize_IPADDR;
    }

    piNetConnection = NetworkService::GetNetworkService()->FindConnection(objIpAddress);
    if (piNetConnection == IMS_NULL)
    {
        IMS_TRACE_E(0, "GetRtpFragmentSize() - piNetConnection is NULL", 0, 0, 0);
        bRet = IMS_FALSE;
        goto EXIT_GetRtpFragmentSize_IPADDR;
    }

    pNetConnectionWatcher = findNetConnectionWatcher(piNetConnection);
    if (pNetConnectionWatcher != NULL)
    {
        nRtpFragmentSize = CalculateRtpFragmentSize(pNetConnectionWatcher);
    }
    else
    {
        nRtpFragmentSize = CalculateRtpFragmentSize(piNetConnection);
    }

    IMS_TRACE_I("GetRtpFragmentSize() - (objIpAddress(%s)) nRtpFragmentSize(%d)",
            objIpAddress.ToString().GetStr(), nRtpFragmentSize, 0);

EXIT_GetRtpFragmentSize_IPADDR:
    return bRet;
}

PUBLIC VIRTUAL IMS_BOOL MediaConnectionWatcher::GetRtpFragmentSize(
        IN INetworkConnection* piNetConnection, OUT IMS_SINT32& nRtpFragmentSize)
{
    NetConnectionWatcher* pNetConnectionWatcher = IMS_NULL;
    IMS_BOOL bRet = IMS_TRUE;
    nRtpFragmentSize = 0;

    if (piNetConnection == IMS_NULL)
    {
        IMS_TRACE_E(0, "GetRtpFragmentSize() - piNetConnection is NULL", 0, 0, 0);
        bRet = IMS_FALSE;
        goto EXIT_GetRtpFragmentSize_INETCON;
    }

    pNetConnectionWatcher = findNetConnectionWatcher(piNetConnection);
    if (pNetConnectionWatcher != NULL)
    {
        nRtpFragmentSize = CalculateRtpFragmentSize(pNetConnectionWatcher);
    }
    else
    {
        nRtpFragmentSize = CalculateRtpFragmentSize(piNetConnection);
    }

    IMS_TRACE_I("GetRtpFragmentSize() - nRtpFragmentSize(%d)", nRtpFragmentSize, 0, 0);

EXIT_GetRtpFragmentSize_INETCON:
    return bRet;
}

PUBLIC VIRTUAL IMS_BOOL MediaConnectionWatcher::SetListener(
        IN IMediaConnectionWatcherListener* piListener, IN AString& strPdn, IN IMS_SINT32 nSlotID,
        OUT IMS_SINT32& nMediaConnectionType, OUT IMS_UINT32& nNetworkInterfaceId)
{
    INetworkConnection* piNetConnection = IMS_NULL;
    NetConnectionWatcher* pNetConnectionWatcher = IMS_NULL;

    if (!GetMediaConnectionType(
                strPdn, nSlotID, piNetConnection, nMediaConnectionType, nNetworkInterfaceId))
    {
        IMS_TRACE_E(0, "SetListener() - piNetConnction is NULL", 0, 0, 0);
        return IMS_FALSE;
    }

    // First, Find previous list.
    for (IMS_UINT32 i = 0; i < objWatchers.GetSize(); i++)
    {
        NetConnectionWatcher* pTempNetConnectionWatcher = objWatchers.GetAt(i);
        if (pTempNetConnectionWatcher != IMS_NULL)
        {
            if (pTempNetConnectionWatcher->m_piNetConnection == piNetConnection)
            {
                IMS_TRACE_D("SetListener() - Found piNetConnction[%x]", piNetConnection, 0, 0);

                pTempNetConnectionWatcher->AddListener(piListener);
                pTempNetConnectionWatcher->SetMediaConnectionType(nMediaConnectionType);
                return IMS_TRUE;
            }
        }
    }

    // If it doesn't have, Add new List.
    pNetConnectionWatcher = new NetConnectionWatcher();
    pNetConnectionWatcher->SetINetConnection(piNetConnection);
    pNetConnectionWatcher->AddListener(piListener);
    objWatchers.Append(pNetConnectionWatcher);

    IMS_TRACE_D("SetListener() - new Set Listener [%x]", piNetConnection, 0, 0);

    return IMS_TRUE;
}

PUBLIC VIRTUAL IMS_BOOL MediaConnectionWatcher::SetListener(
        IN IMediaConnectionWatcherListener* piListener, IN IPAddress& objIpAddress,
        OUT IMS_SINT32& nMediaConnectionType, OUT IMS_UINT32& nNetworkInterfaceId)
{
    INetworkConnection* piNetConnection = IMS_NULL;
    NetConnectionWatcher* pNetConnectionWatcher = IMS_NULL;

    if (!GetMediaConnectionType(
                objIpAddress, piNetConnection, nMediaConnectionType, nNetworkInterfaceId))
    {
        IMS_TRACE_E(0, "SetListener() - piNetConnction is NULL", 0, 0, 0);
        return IMS_FALSE;
    }

    // First, Find previous list.
    for (IMS_UINT32 i = 0; i < objWatchers.GetSize(); i++)
    {
        NetConnectionWatcher* pTempNetConnectionWatcher = objWatchers.GetAt(i);
        if (pTempNetConnectionWatcher != IMS_NULL)
        {
            if (pTempNetConnectionWatcher->m_piNetConnection == piNetConnection)
            {
                IMS_TRACE_D("SetListener() - Found piNetConnction[%x]", piNetConnection, 0, 0);

                pTempNetConnectionWatcher->AddListener(piListener);
                pTempNetConnectionWatcher->SetMediaConnectionType(nMediaConnectionType);
                return IMS_TRUE;
            }
        }
    }

    // If it doesn't have, Add new List.
    pNetConnectionWatcher = new NetConnectionWatcher();
    pNetConnectionWatcher->SetINetConnection(piNetConnection);
    pNetConnectionWatcher->AddListener(piListener);
    objWatchers.Append(pNetConnectionWatcher);
    IMS_TRACE_D("SetListener() - new Set Listener [%d]", piNetConnection, 0, 0);

    return IMS_TRUE;
}

PUBLIC VIRTUAL IMS_BOOL MediaConnectionWatcher::ReleaseListener(
        IN IMediaConnectionWatcherListener* piListener)
{
    IMS_TRACE_D("ReleaseListener()) -- objWatchers Size[%d]", objWatchers.GetSize(), 0, 0);

    if (piListener == IMS_NULL)
    {
        IMS_TRACE_E(0, "ReleaseListener()) --- piListener NULL", 0, 0, 0);
        return IMS_FALSE;
    }

    for (IMS_SINT32 i = objWatchers.GetSize() - 1; i >= 0; i--)
    {
        NetConnectionWatcher* pTempNetConnectionWatcher = objWatchers.GetAt(i);
        if (pTempNetConnectionWatcher != IMS_NULL)
        {
            pTempNetConnectionWatcher->RemoveListener(piListener);
            if (pTempNetConnectionWatcher->GetListenerLength() == 0)
            {
                delete pTempNetConnectionWatcher;
                pTempNetConnectionWatcher = IMS_NULL;
                objWatchers.RemoveAt(i);
            }
        }
    }

    return IMS_TRUE;
}

PUBLIC VIRTUAL void MediaConnectionWatcher::Event_NotifyEvent(
        IN IMS_SINT32 nEvent, IN IMS_UINT32 nWParam, IN IMS_UINT32 nLParam)
{
    (void)nWParam;
    (void)nLParam;

    switch (nEvent)
    {
        default:
        {
            IMS_TRACE_D("Event_NotifyEvent() - Invalid Event(%d)", nEvent, 0, 0);
        }
        break;
    }
}

PRIVATE
MediaConnectionWatcher::NetConnectionWatcher* MediaConnectionWatcher::findNetConnectionWatcher(
        IN INetworkConnection* piNetConnection)
{
    NetConnectionWatcher* pNetConnectionWatcher = IMS_NULL;

    for (IMS_UINT32 nIndex = 0; nIndex < objWatchers.GetSize(); nIndex++)
    {
        pNetConnectionWatcher = objWatchers.GetAt(nIndex);
        if (pNetConnectionWatcher->m_piNetConnection == piNetConnection)
        {
            return pNetConnectionWatcher;
        }
    }

    return IMS_NULL;
}

PRIVATE
void MediaConnectionWatcher::clearMediaConnectionWatcher()
{
    NetConnectionWatcher* pTempNetConnectionWatcher = IMS_NULL;

    for (IMS_UINT32 nIndex = 0; nIndex < objWatchers.GetSize(); nIndex++)
    {
        pTempNetConnectionWatcher = objWatchers.GetAt(nIndex);
        if (pTempNetConnectionWatcher != IMS_NULL)
        {
            delete pTempNetConnectionWatcher;
            pTempNetConnectionWatcher = IMS_NULL;
        }
    }
    objWatchers.Clear();

    return;
}

PUBLIC GLOBAL IMS_SINT32 MediaConnectionWatcher::CalculateRtpFragmentSize(
        IN INetworkConnection* piNetConnection
        /*, IN IMS_SINT32 nSlotId*/)
{
    IMS_SINT32 nRtpFragmentSize = 0;
    IMS_UINT32 nIPV6orV4 = IPAddress::IPV6;

    if (piNetConnection == IMS_NULL)
    {
        goto Exit_CalculateRtpFragmentSize_NetConnection;
    }

    nIPV6orV4 = IPAddress(piNetConnection->GetLocalAddress()).GetVersion();

    // TODO - 20220415 - Need to implement this requirement later
    /*if (IMS_OPERATOR(DCM, nSlotId))
    {
        nRtpFragmentSize = piNetConnection->GetMtu() - SIZE_OF_GUARD;
    }
    else // if (!ImsOperator::GetInstance()->IsOperator(OPERATOR_DCM))
    {*/
    switch (nIPV6orV4)
    {
        case IPAddress::IPV4:
        {
            nRtpFragmentSize = piNetConnection->GetMtu() - SIZE_OF_GUARD;  // X - 200
        }
        break;
        case IPAddress::IPV6:
        {
            /*
            nRtpFragmentSize = MTU_EPDG;            // 1280
            nRtpFragmentSize -= SIZE_OF_IP_SEC;     // 1280 - 60
            nRtpFragmentSize -= SIZE_OF_IPV6;       // 1220 - 60
            nRtpFragmentSize -= SIZE_OF_RTP;        // 1160 - 20  = 1140
            */
            nRtpFragmentSize = 1140;
        }
        break;
        default:
        {
            nRtpFragmentSize = 0;
            goto Exit_CalculateRtpFragmentSize_NetConnection;
        }
        break;
    }
    //} //end of TODO

    if (nRtpFragmentSize <= 0)
    {
        nRtpFragmentSize = 0;
    }

Exit_CalculateRtpFragmentSize_NetConnection:

    IMS_TRACE_D("CalculateRtpFragmentSize() - nRtpFragmentSize(%d)", nRtpFragmentSize, 0, 0);

    return nRtpFragmentSize;
}

PUBLIC
GLOBAL
IMS_SINT32
MediaConnectionWatcher::
                        CalculateRtpFragmentSize(IN MediaConnectionWatcher::NetConnectionWatcher*
                                        pNetConnectionWatcher /*,
IN IMS_SINT32 nSlotId*/)
{
    IMS_SINT32 nRtpFragmentSize = 0;
    IMS_UINT32 nIPV6orV4 = IPAddress::IPV6;

    if (pNetConnectionWatcher == IMS_NULL || pNetConnectionWatcher->m_piNetConnection == IMS_NULL)
    {
        goto Exit_CalculateRtpFragmentSize_NetWatcher;
    }

    nIPV6orV4 = IPAddress(pNetConnectionWatcher->m_piNetConnection->GetLocalAddress()).GetVersion();

    // TODO - 20220415 - Need to implement this requirement later
    /*if (IMS_OPERATOR(DCM, nSlotId))
    {
        nRtpFragmentSize = pNetConnectionWatcher->GetMtuSize() - SIZE_OF_GUARD;
    }
    else // if (!ImsOperator::GetInstance()->IsOperator(OPERATOR_DCM))
    {*/
    switch (nIPV6orV4)
    {
        case IPAddress::IPV4:
        {
            nRtpFragmentSize = pNetConnectionWatcher->GetMtuSize() - SIZE_OF_GUARD;  // X - 200
        }
        break;
        case IPAddress::IPV6:
        {
            /*
            nRtpFragmentSize = MTU_EPDG;            // 1280
            nRtpFragmentSize -= SIZE_OF_IP_SEC;     // 1280 - 60
            nRtpFragmentSize -= SIZE_OF_IPV6;       // 1220 - 60
            nRtpFragmentSize -= SIZE_OF_RTP;        // 1160 - 20  = 1140
            */
            nRtpFragmentSize = 1140;
        }
        break;
        default:
        {
            nRtpFragmentSize = 0;
            goto Exit_CalculateRtpFragmentSize_NetWatcher;
        }
        break;
    }
    //} // end of TODO

    if (nRtpFragmentSize <= 0)
    {
        nRtpFragmentSize = 0;
    }

Exit_CalculateRtpFragmentSize_NetWatcher:

    IMS_TRACE_D("CalculateRtpFragmentSize() - nRtpFragmentSize(%d)", nRtpFragmentSize, 0, 0);

    return nRtpFragmentSize;
}

PRIVATE GLOBAL IMS_SINT32 MediaConnectionWatcher::convertNetworkType(
        IN INetworkConnection* piNetConnection)
{
    AString strRAT = AString::ConstNull();

    if (piNetConnection == IMS_NULL)
    {
        return MEDIA_CONNECTION_INVALID;
    }

    // 1st Condition : WIFI
    piNetConnection->GetExtraInfo(AString("rat"), strRAT);
    if (strRAT.EqualsIgnoreCase(AString("WiFi")))
    {
        return MEDIA_CONNECTION_WIFI;
    }

    // 2nd Condition : Mobile EPDG
    if (piNetConnection->IsePDGEnabled())
    {
        return MEDIA_CONNECTION_MOBILE_EPDG;
    }

    // 3rd Condition : LTE or 3G or NR
    if (strRAT.EqualsIgnoreCase(AString("LTE")) || strRAT.EqualsIgnoreCase(AString("3G")) ||
            strRAT.EqualsIgnoreCase(AString("NR")))
    {
        return MEDIA_CONNECTION_MOBILE;
    }

    return MEDIA_CONNECTION_INVALID;
}

PRIVATE GLOBAL const IMS_CHAR* MediaConnectionWatcher::printNetworkType(
        IN IMS_SINT32 nMediaConnectionType)
{
    switch (nMediaConnectionType)
    {
        case MEDIA_CONNECTION_MOBILE:
            return "MEDIA_CONNECTION_MOBILE";
        case MEDIA_CONNECTION_WIFI:
            return "MEDIA_CONNECTION_WIFI";
        case MEDIA_CONNECTION_MOBILE_EPDG:
            return "MEDIA_CONNECTION_MOBILE_EPDG";
        default:
            return "MEDIA_CONNECTION_INVALID";
    }
}

PUBLIC
MediaConnectionWatcher::NetConnectionWatcher::NetConnectionWatcher() :
        m_piNetConnection(IMS_NULL),
        m_nMediaConnectionType(MEDIA_CONNECTION_INVALID),
        m_nMtuSize(0)
{
    IMS_TRACE_D("NetConnectionWatcher()", 0, 0, 0);
    m_objListeners.Clear();
    return;
}

PUBLIC VIRTUAL MediaConnectionWatcher::NetConnectionWatcher::~NetConnectionWatcher()
{
    IMS_TRACE_D("~NetConnectionWatcher()", 0, 0, 0);
    if (m_piNetConnection != IMS_NULL)
    {
        m_piNetConnection->RemoveReferenceListener(this);
    }
    m_piNetConnection = IMS_NULL;
    m_objListeners.Clear();
    return;
}

PUBLIC
IMS_BOOL MediaConnectionWatcher::NetConnectionWatcher::AddListener(
        IN IMediaConnectionWatcherListener* piListener)
{
    if (piListener == IMS_NULL)
    {
        IMS_TRACE_E(0, "AddListener() - Listener NULL", 0, 0, 0);
        return IMS_FALSE;
    }

    if (hasListener(piListener))
    {
        // Already Have.
        IMS_TRACE_D("AddListener() - Already Have", 0, 0, 0);
        return IMS_TRUE;
    }

    // IMS_TRACE_D("AddListener() - piListener[%x]", piListener, 0, 0);

    m_objListeners.Append(piListener);

    IMS_TRACE_D("AddListener() - Size[%d]", m_objListeners.GetSize(), 0, 0);

    return IMS_TRUE;
}

PUBLIC
IMS_BOOL MediaConnectionWatcher::NetConnectionWatcher::RemoveListener(
        IN IMediaConnectionWatcherListener* piListener)
{
    IMS_TRACE_D("RemoveListener() - Size[%d]", m_objListeners.GetSize(), 0, 0);

    if (piListener == IMS_NULL)
    {
        IMS_TRACE_E(0, "RemoveListener() - Listener NULL", 0, 0, 0);
        return IMS_FALSE;
    }

    IMS_UINT32 nIndex = 0;

    if (hasListener(piListener, nIndex))
    {
        IMS_TRACE_D("RemoveListener() - piListener[%x]", piListener, 0, 0);

        m_objListeners.RemoveAt(nIndex);
    }

    return IMS_TRUE;
}

PUBLIC
IMS_SINT32 MediaConnectionWatcher::NetConnectionWatcher::GetMediaConnectionType()
{
    return m_nMediaConnectionType;
}

PUBLIC
IMS_UINT32 MediaConnectionWatcher::NetConnectionWatcher::GetNetworkInterfaceID()
{
    if (m_piNetConnection == IMS_NULL)
    {
        return 0;
    }

    return m_piNetConnection->GetIfaceId();
}

PUBLIC
void MediaConnectionWatcher::NetConnectionWatcher::SetMediaConnectionType(
        IN IMS_SINT32 nMediaConnectionType)
{
    m_nMediaConnectionType = nMediaConnectionType;
    return;
}

PUBLIC
IMS_BOOL MediaConnectionWatcher::NetConnectionWatcher::SetINetConnection(
        IN INetworkConnection* piNetConnection)
{
    if (piNetConnection == IMS_NULL)
    {
        IMS_TRACE_E(0, "SetINetConnection() - piNetConnction is NULL", 0, 0, 0);
        return IMS_FALSE;
    }
    m_piNetConnection = piNetConnection;
    m_piNetConnection->AddReferenceListener(this);
    m_nMediaConnectionType = MediaConnectionWatcher::convertNetworkType(piNetConnection);
    setMtuSize(m_piNetConnection->GetMtu());

    return IMS_TRUE;
}

PUBLIC
IMS_UINT32 MediaConnectionWatcher::NetConnectionWatcher::GetListenerLength()
{
    IMS_TRACE_D("GetListenerLength() - Length[%d]", m_objListeners.GetSize(), 0, 0);
    return m_objListeners.GetSize();
}

PUBLIC
void MediaConnectionWatcher::NetConnectionWatcher::NotifyWifiEarlyRouteSetupChanged(
        IN IMS_SINT32 nMtuSize)
{
    IMS_TRACE_D("NotifyWifiEarlyRouteSetupChanged - Listener Size(%d) MtuSize(%d)",
            m_objListeners.GetSize(), nMtuSize, 0);

    setMtuSize(nMtuSize);
    if (m_nMediaConnectionType == MEDIA_CONNECTION_MOBILE_EPDG)
    {
        IMS_TRACE_E(0, "NotifyWifiEarlyRouteSetupChanged - Already ePDG", 0, 0, 0);
        return;
    }

    IMediaConnectionWatcherListener* piListener = IMS_NULL;
    m_nMediaConnectionType = MEDIA_CONNECTION_MOBILE_EPDG;

    for (IMS_UINT32 i = 0; i < m_objListeners.GetSize(); i++)
    {
        piListener = m_objListeners.GetAt(i);

        IMS_TRACE_D("NotifyWifiEarlyRouteSetupChanged() - \
                Listener Size(%d) Call Listener(%d)",
                m_objListeners.GetSize(), i, 0);

        if (piListener != IMS_NULL)
        {
            IMS_TRACE_D("NotifyWifiEarlyRouteSetupChanged()", 0, 0, 0);
            piListener->NotifyWifiEarlyRouteSetup(m_piNetConnection->GetIfaceId());
        }
    }
}

PUBLIC
IMS_SINT32 MediaConnectionWatcher::NetConnectionWatcher::GetMtuSize()
{
    if (m_piNetConnection == IMS_NULL)
    {
        return 0;
    }

    if (m_nMtuSize > 0)
    {
        return m_nMtuSize;
    }

    return m_piNetConnection->GetMtu();
}

/* INetworkConnectionListener Interface Impl */
PUBLIC VIRTUAL void MediaConnectionWatcher::NetConnectionWatcher::NetworkConnection_OnConnected(
        IN INetworkConnection* piNetConnection)
{
    IMS_TRACE_D("Connection_Connected() - Listener Size[%d]", m_objListeners.GetSize(), 0, 0);
    IMediaConnectionWatcherListener* piListener = IMS_NULL;

    if (piNetConnection == IMS_NULL)
    {
        IMS_TRACE_E(0, "Connection_Connected() - piNetConnction NULL", 0, 0, 0);
        m_nMediaConnectionType = MEDIA_CONNECTION_INVALID;
        return;
    }

    if (piNetConnection != m_piNetConnection)
    {
        IMS_TRACE_E(0, "Connection_Connected() - invalid piNetConnction", 0, 0, 0);
        return;
    }

    setMtuSize(m_piNetConnection->GetMtu());
    m_nMediaConnectionType = MediaConnectionWatcher::convertNetworkType(piNetConnection);
    for (IMS_UINT32 i = 0; i < m_objListeners.GetSize(); i++)
    {
        piListener = m_objListeners.GetAt(i);

        IMS_TRACE_D("Connection_Connected() - Listener Size[%d] Call Listener[%d]",
                m_objListeners.GetSize(), i, 0);

        if (piListener != IMS_NULL)
        {
            IMS_TRACE_D("Connection_Connected()", 0, 0, 0);
            piListener->NotifyIPChanged(piNetConnection->GetLocalAddress().IsIPv6Address());
        }
    }
}

PUBLIC VIRTUAL void MediaConnectionWatcher::NetConnectionWatcher::NetworkConnection_OnDisconnected(
        IN INetworkConnection* piNetConnection, IN IMS_SINT32 nErrorCode)
{
    (void)piNetConnection;
    (void)nErrorCode;
    IMS_TRACE_D("Connection_Disconnected()", 0, 0, 0);
    // Do Nothing
    m_nMediaConnectionType = MEDIA_CONNECTION_INVALID;
    setMtuSize(0);

    return;
}

PUBLIC VIRTUAL void
MediaConnectionWatcher::NetConnectionWatcher::NetworkConnection_OnConnectionFailed(
        IN INetworkConnection* piNetConnection, IN IMS_SINT32 nErrorCode)
{
    (void)piNetConnection;
    (void)nErrorCode;

    IMS_TRACE_D("Connection_ConnectionFailed()", 0, 0, 0);
    // Do Nothing
    m_nMediaConnectionType = MEDIA_CONNECTION_INVALID;
    setMtuSize(0);

    return;
}

PUBLIC VIRTUAL void MediaConnectionWatcher::NetConnectionWatcher::NetworkConnection_OnIpChanged(
        IN INetworkConnection* piNetConnection)
{
    IMS_TRACE_D("Connection_IpChanged() - Listener Size[%d]", m_objListeners.GetSize(), 0, 0);
    IMediaConnectionWatcherListener* piListener = IMS_NULL;

    if (piNetConnection == IMS_NULL)
    {
        IMS_TRACE_E(0, "Connection_IpChanged() - piNetConnction NULL", 0, 0, 0);
        m_nMediaConnectionType = MEDIA_CONNECTION_INVALID;
        return;
    }

    if (piNetConnection != m_piNetConnection)
    {
        IMS_TRACE_E(0, "Connection_IpChanged() - invalid piNetConnction", 0, 0, 0);
        return;
    }

    IMS_TRACE_D("Connection_IpChanged() - prev(%s) next(%s)",
            printNetworkType(m_nMediaConnectionType),
            printNetworkType(MediaConnectionWatcher::convertNetworkType(piNetConnection)), 0);

    m_nMediaConnectionType = MediaConnectionWatcher::convertNetworkType(piNetConnection);
    setMtuSize(m_piNetConnection->GetMtu());

    for (IMS_UINT32 i = 0; i < m_objListeners.GetSize(); i++)
    {
        piListener = m_objListeners.GetAt(i);
        IMS_TRACE_D(
                "Connection_IpChanged() - Listener Size[%d][%d]", m_objListeners.GetSize(), i, 0);
        if (piListener != IMS_NULL)
        {
            IMS_TRACE_D("Connection_IpChanged()", 0, 0, 0);
            piListener->NotifyIPChanged(piNetConnection->GetLocalAddress().IsIPv6Address());
        }
    }

    return;
}

PUBLIC VIRTUAL void MediaConnectionWatcher::NetConnectionWatcher::NetworkConnection_OnIpcanChanged(
        IN INetworkConnection* piNetConnection)
{
    IMS_TRACE_D("Connection_IpcanCatChanged() - Listener Size[%d]", m_objListeners.GetSize(), 0, 0);
    IMediaConnectionWatcherListener* piListener = IMS_NULL;

    if (piNetConnection == IMS_NULL)
    {
        IMS_TRACE_E(0, "Connection_IpcanCatChanged() - piNetConnction NULL", 0, 0, 0);
        m_nMediaConnectionType = MEDIA_CONNECTION_INVALID;
        return;
    }

    if (piNetConnection != m_piNetConnection)
    {
        IMS_TRACE_E(0, "Connection_IpcanCatChanged() - invalid piNetConnction", 0, 0, 0);
        return;
    }

    if (m_nMediaConnectionType == convertNetworkType(m_piNetConnection))
    {
        IMS_TRACE_E(0, "Connection_IpcanCatChanged - Already Same with previous ConnectionType", 0,
                0, 0);
        return;
    }
    m_nMediaConnectionType = convertNetworkType(m_piNetConnection);

    if (m_nMediaConnectionType == MEDIA_CONNECTION_INVALID)
    {
        IMS_TRACE_D("Connection_IpcanCatChanged() - connection Type is invalid", 0, 0, 0);
        setMtuSize(0);
        return;
    }
    setMtuSize(m_piNetConnection->GetMtu());

    IMS_TRACE_D("Connection_IpcanCatChanged() - connectionType(%s), IFID(%d) MtuSize(%d)",
            printNetworkType(m_nMediaConnectionType), piNetConnection->GetIfaceId(), m_nMtuSize);
    for (IMS_UINT32 i = 0; i < m_objListeners.GetSize(); i++)
    {
        piListener = m_objListeners.GetAt(i);
        IMS_TRACE_D("Connection_IpcanCatChanged() - Listener Size[%d][%d]",
                m_objListeners.GetSize(), i, 0);
        if (piListener != IMS_NULL)
        {
            piListener->NotifyMediaConnection(
                    m_piNetConnection, m_nMediaConnectionType, piNetConnection->GetIfaceId());
        }
    }

    return;
}

PUBLIC VIRTUAL void MediaConnectionWatcher::NetConnectionWatcher::NetworkConnection_OnPcscfChanged(
        IN INetworkConnection* piNetConnection)
{
    (void)piNetConnection;

    // Do Nothing
    return;
}

PRIVATE
IMS_BOOL MediaConnectionWatcher::NetConnectionWatcher::hasListener(
        IN IMediaConnectionWatcherListener* piListener)
{
    IMS_UINT32 nIndex = 0;
    return hasListener(piListener, nIndex);
}

PRIVATE
IMS_BOOL MediaConnectionWatcher::NetConnectionWatcher::hasListener(
        IN IMediaConnectionWatcherListener* piListener, OUT IMS_UINT32& nIndex)
{
    if (piListener == IMS_NULL)
    {
        IMS_TRACE_E(0, "hasListener() - Listener NULL", 0, 0, 0);
        return IMS_FALSE;
    }

    IMediaConnectionWatcherListener* piTempListener = IMS_NULL;
    for (IMS_UINT32 i = 0; i < m_objListeners.GetSize(); i++)
    {
        piTempListener = m_objListeners.GetAt(i);
        if (piTempListener == piListener)
        {
            nIndex = i;
            return IMS_TRUE;
        }
    }

    return IMS_FALSE;
}

PRIVATE
void MediaConnectionWatcher::NetConnectionWatcher::setMtuSize(IN IMS_SINT32 nMtuSize)
{
    IMS_TRACE_D("setMtuSize() - Before(%d) After(%d)", m_nMtuSize, nMtuSize, 0);
    m_nMtuSize = nMtuSize;
    return;
}
