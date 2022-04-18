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

#include "ServiceEvent.h"
#include "ServicePhoneInfo.h"
#include "ServiceNetworkPolicy.h"
#include "ServiceNetwork.h"
#include "ServiceSystemTime.h"
#include "MediaResourceMngr.h"
#include "MediaDef.h"
#include "config/MediaSessionConfigFactory.h"
#include "MediaConnectionWatcher.h"

// == DEFINES =========================================================
#define MEDIA_RESOURCEMNGR_IP_ADDR_LEN         46

__IMS_TRACE_TAG_USER_DECL__("MED.RM");

// == Constructor, Destructor, Operator Overloading ========================================
PUBLIC
MediaResourceMngr::MediaResourceMngr(IN IMS_SINT32 nSlotID) :
        m_nSlotId(nSlotID),
        m_objIMSPDN(PDNResource()),
        m_objEmergencyPDN(PDNResource()),
        m_lstUsedRtpPort(IMSList<IMS_UINT32>()),
        m_nSupportedNetworkTypeFlag(0)
{
    // set IP changed listener
    SetMediaConnectionWatcherListener();
}

PUBLIC
MediaResourceMngr::~MediaResourceMngr()
{
    // unset IP changed listener
    UnsetMediaConnectionWatcherListener();
}

PUBLIC
IMS_BOOL MediaResourceMngr::UpdatePdnResource(IN IMS_SINT32 nPdnType, IN IMS_BOOL nIsIPv6)
{
    if (nPdnType != PDN_EMERGENCY)
    {
        // Stetp 0-1. Only modem IP will be used in LTE ran
        // Get current Radio Network
        IMS_SINT32 nRadioType = PhoneInfoService::GetPhoneInfoService()->
                GetNetworkWatcher(m_nSlotId)->GetNetworkType();
        IMS_TRACE_D("UpdatePdnResource() - nRadioType[%d], nSlotID[%u]", nRadioType, m_nSlotId, 0);
        if ((ConvertMediaNetworkType(nRadioType) & GetSupportedNetworkTypeFlag()) == 0)
        {
            return IMS_FALSE;
        }
    }

    PDNResource* pLocalPdn = NULL;
    MEDIA_SERVICE_TYPE eMediaServiceType = MEDIA_SERVICE_DEFAULT;

    // Select a PDN and service type
    if (nPdnType == PDN_EMERGENCY)
    {
        eMediaServiceType = MEDIA_SERVICE_EMERGENCY;
        pLocalPdn = &m_objEmergencyPDN;
    }
    else
    {
        eMediaServiceType = MEDIA_SERVICE_DEFAULT;
        pLocalPdn = &m_objIMSPDN;
    }

    // acquire APN name
    UpdateAPN(eMediaServiceType, pLocalPdn);

    // update ip version
    pLocalPdn->nIsIPv6 = nIsIPv6;
    return IMS_TRUE;
}

PUBLIC
void MediaResourceMngr::ResetPdnResource(IN IMS_SINT32 nPdnType)
{
    IMS_TRACE_D("ResetPdnResource() - Entered. PDNType[%d]", nPdnType, 0, 0);

    if (nPdnType == PDN_EMERGENCY)
    {
        m_objEmergencyPDN.m_bModemIPUpdated = IMS_FALSE;
        m_objEmergencyPDN.strIPAddrOfIPv6 = AString::ConstNull();
    }
    else
    {
        m_objIMSPDN.m_bModemIPUpdated = IMS_FALSE;
        m_objIMSPDN.strIPAddrOfIPv6 = AString::ConstNull();
    }
}

PUBLIC
void MediaResourceMngr::UpdateAPN(IN MEDIA_SERVICE_TYPE eMediaServiceType, PDNResource* pLocalPdn)
{
    INetworkConnection* pConnection = GetNetConnection(eMediaServiceType);

    if (pConnection == IMS_NULL)
    {
        IMS_TRACE_D("UpdateAPN() - pConnection is NULL", 0, 0, 0);
        return;
    }

    pConnection->GetExtraInfo("apn", pLocalPdn->strApnName);
}

PUBLIC
AString MediaResourceMngr::GetApnName(IN MEDIA_SERVICE_TYPE eServiceType)
{
    if (eServiceType == MEDIA_SERVICE_EMERGENCY)
    {
        return m_objEmergencyPDN.strApnName;
    }

    return m_objIMSPDN.strApnName;
}

PUBLIC
IMS_UINT32 MediaResourceMngr::AcquireRtpPort(IN MediaConfiguration* pConfig)
{
    if (pConfig == IMS_NULL)
    {
        IMS_TRACE_D("AcquireRtpPort() - config is null", 0, 0, 0);
        return 0;
    }

    return MediaResourceMngr::AcquireRtpPort(pConfig->GetPortRtp(), pConfig->GetPortRtpEnd());
}

PUBLIC
IMS_UINT32 MediaResourceMngr::AcquireRtpPort(IN IMS_UINT32 nRangeStart, IN IMS_UINT32 nRangeEnd)
{
    const IMS_UINT32 RTP_PORT_MAX = 0xffff;
    IMS_UINT32 nInitialPort = 0;
    IMS_UINT32 nTempPort    = 0;
    IMS_UINT32 nChosenPort = 0;
    IMS_BOOL bFoundSamePort = IMS_FALSE;

    IMS_TRACE_D("AcquireRtpPort() - nRangeStart[%d], nRangeEnd[%d]",
            nRangeStart, nRangeEnd, 0);

    // STEP 0. Exception handling : INVALID case
    if (nRangeStart > RTP_PORT_MAX || nRangeEnd > RTP_PORT_MAX)
    {
        IMS_TRACE_E(0, "AcquireRtpPort() - out of start[%d], end[%d]", nRangeStart, nRangeEnd, 0);
        return 0;
    }
    else if (nRangeStart > nRangeEnd)
    {// when Start Port is bigger than End Port, swapping them.
        nTempPort   = nRangeStart;
        nRangeStart = nRangeEnd;
        nRangeEnd   = nTempPort;
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

    do {
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
    } while(bFoundSamePort == IMS_TRUE);

    // STEP 4. Save a selected RTP port to list
    if (nChosenPort != 0)
    {
        m_lstUsedRtpPort.Append(nChosenPort);
    }

    IMS_TRACE_D("AcquireRtpPort() - End. nChosenPort[%d]", nChosenPort, 0, 0);

    return nChosenPort;
}

PUBLIC
void MediaResourceMngr::ReleaseRtpPort(IN IMS_UINT32 nPort)
{
    IMS_TRACE_D("ReleaseRtpPort [%d]", nPort, 0, 0);
    IMS_UINT32 i = 0;
    for (i = 0; i < m_lstUsedRtpPort.GetSize(); i++)
    {
        IMS_UINT32 nListPort = m_lstUsedRtpPort.GetAt(i);
        if (nListPort == nPort)
        {
            m_lstUsedRtpPort.RemoveAt(i);
            IMS_TRACE_I("ReleaseRtpPort() - port[%d],list[%d],Size[%d]",
                    nPort, i, m_lstUsedRtpPort.GetSize());
            return;
        }
    }

    if (nPort != 0)
    {
        IMS_TRACE_D("ReleaseRtpPort() - no matched port[%d], Size[%d]",
                nPort, m_lstUsedRtpPort.GetSize(), 0);
    }
}

/* IMediaConnectionWatcherListener Interface Impl */
PUBLIC
void MediaResourceMngr::NotifyMediaConnection(IN INetworkConnection *piNetConnection,
        IN IMS_SINT32 nMediaConnectionType, IN IMS_UINT32 nNetworkInterfaceId)
{
    (void)piNetConnection;
    (void)nMediaConnectionType;
    (void)nNetworkInterfaceId;
    // do nothing
}

PUBLIC
void MediaResourceMngr::NotifyIPChanged(IMS_BOOL bIsIPv6)
{
    IMS_TRACE_I("NotifyIPChanged() : bIpV6[%d] SlotId[%d]", bIsIPv6, GetSlotId(), 0);
    ResetPdnResource(PDN_IMS);
    ResetPdnResource(PDN_EMERGENCY);
    UpdatePdnResource(PDN_IMS, bIsIPv6);
    UpdatePdnResource(PDN_EMERGENCY, bIsIPv6);
}

PUBLIC
VIRTUAL void MediaResourceMngr::NotifyWifiEarlyRouteSetup(IN IMS_UINT32 nNetworkInferfaceID)
{
    (void)nNetworkInferfaceID;
    //IMS_TRACE_I("NotifyWifiEarlyRouteSetup", 0, 0, 0);
    return;
}

// == PUBLIC METHOD ==============================================================
PUBLIC
IMediaConnectionWatcher* MediaResourceMngr::GetMediaConnectionWatcher()
{
    return MediaConnectionWatcher::GetMediaConnectionWatcher(m_nSlotId);
}

PUBLIC
IMS_BOOL MediaResourceMngr::GetMediaConnectionWatcherInfo(IN IPAddress &objIpAddress,
        OUT IMS_BOOL &bWIFICondition, OUT IMS_UINT32 &nNetworkInterfaceId)
{
    IMediaConnectionWatcher* piMediaConnectionWatcher = IMS_NULL;
    INetworkConnection *piNetConnection  = IMS_NULL;
    IMS_SINT32 nMediaConnectionType = IMediaConnectionWatcher::MEDIA_CONNECTION_INVALID;
    IMS_BOOL bRet = IMS_FALSE;

    if (objIpAddress.IsNoneAddress())
    {
        IMS_TRACE_E(0, "GetMediaConnectionWatcherInfo() invalid ip address", 0, 0, 0);
        return bRet;
    }

    piMediaConnectionWatcher = GetMediaConnectionWatcher();
    if (piMediaConnectionWatcher == IMS_NULL)
    {
        IMS_TRACE_E(0, "GetMediaConnectionWatcherInfo() - NULL", 0, 0, 0);
        bWIFICondition = IMS_FALSE;
        nNetworkInterfaceId = 0;
        return bRet;
    }

    piMediaConnectionWatcher->GetMediaConnectionType(objIpAddress,
            piNetConnection, nMediaConnectionType, nNetworkInterfaceId);
    switch (nMediaConnectionType)
    {
        case IMediaConnectionWatcher::MEDIA_CONNECTION_MOBILE_EPDG:
        case IMediaConnectionWatcher::MEDIA_CONNECTION_WIFI:
            bWIFICondition = IMS_TRUE;
            bRet = IMS_TRUE;
            break;
        case IMediaConnectionWatcher::MEDIA_CONNECTION_MOBILE:
            bWIFICondition = IMS_FALSE;
            bRet = IMS_TRUE;
            break;
        default:
            bWIFICondition = IMS_FALSE;
            bRet = IMS_FALSE;
            break;
    }

    return bRet;
}

PUBLIC
IMS_UINT32 MediaResourceMngr::GetRtpFragmentSize(IN IPAddress &objIpAddress)
{
    IMediaConnectionWatcher* piMediaConnectionWatcher = GetMediaConnectionWatcher();
    IMS_SINT32 nRtpFragmentSize = 0;
    INetworkConnection *piNetConnection  = IMS_NULL;
    if (piMediaConnectionWatcher != IMS_NULL)
    {
        piMediaConnectionWatcher->GetRtpFragmentSize(objIpAddress, piNetConnection,
                nRtpFragmentSize);
    }
    return nRtpFragmentSize;
}

PUBLIC
INetworkConnection* MediaResourceMngr::GetNetConnection(IN MEDIA_SERVICE_TYPE eServiceType)
{
    const NetworkPolicy* pImsNetworkPolicy = IMS_NULL;

    if (eServiceType == MEDIA_SERVICE_EMERGENCY)
    {
        pImsNetworkPolicy = NetworkServicePolicy::GetInstance()->GetPolicy(
                NetworkPolicy::APN_EMERGENCY, m_nSlotId);
    }
    else
    {
        pImsNetworkPolicy = NetworkServicePolicy::GetInstance()->GetPolicy(
                NetworkPolicy::APN_IMS, m_nSlotId);
    }

    if (pImsNetworkPolicy == IMS_NULL)
    {
        IMS_TRACE_D("GetNetConnection() - pImsNetworkPolicy is NULL", 0, 0, 0);
        return IMS_NULL;
    }

    INetworkConnection* pConnection = NetworkService::GetNetworkService()->FindConnection(
            pImsNetworkPolicy->GetName(), m_nSlotId);
    if (pConnection == IMS_NULL)
    {
        IMS_TRACE_D("GetNetConnection() - pConnection is NULL", 0, 0, 0);
        return IMS_NULL;
    }

    return pConnection;
}

PUBLIC
MEDIA_NETWORK_TYPE MediaResourceMngr::ConvertMediaNetworkType(IMS_SINT32 nRadioType)
{
    MEDIA_NETWORK_TYPE eMediaNetwork = MEDIA_NETWORK_NONE;

    switch (nRadioType)
    {
        case INetworkWatcher::RADIOTECH_TYPE_LTE :
            eMediaNetwork = MEDIA_NETWORK_LTE;
            break;
        case INetworkWatcher::RADIOTECH_TYPE_HSPAP :
            eMediaNetwork = MEDIA_NETWORK_HSPA_PLUS;
            break;
        case INetworkWatcher::RADIOTECH_TYPE_UMTS :
        case INetworkWatcher::RADIOTECH_TYPE_HSPA :
        case INetworkWatcher::RADIOTECH_TYPE_HSDPA :
        case INetworkWatcher::RADIOTECH_TYPE_HSUPA :
        case INetworkWatcher::RADIOTECH_TYPE_CDMA :
            eMediaNetwork = MEDIA_NETWORK_HSPA;
            break;
        case INetworkWatcher::RADIOTECH_TYPE_EHRPD :
            eMediaNetwork = MEDIA_NETWORK_EHRPD;
            break;
        default :
            eMediaNetwork = MEDIA_NETWORK_LTE;
            break;
    }

    return eMediaNetwork;
}

PUBLIC
IMS_SINT32 MediaResourceMngr::GetSupportedNetworkTypeFlag()
{
    if (m_nSupportedNetworkTypeFlag == 0)   // initial check..
    {
        m_nSupportedNetworkTypeFlag |= (IMS_UINT32)MEDIA_NETWORK_LTE;
    }   // TODO_MEDIA Need to be updated later for NR

    IMS_TRACE_D("GetSupportedNetworkTypeFlag() - nSupportedNetworkTypeFlag[%d]",
            m_nSupportedNetworkTypeFlag, 0, 0);

    return  m_nSupportedNetworkTypeFlag;
}

PUBLIC
IMS_BOOL MediaResourceMngr::SetMediaConnectionWatcherListener()
{
    IMS_TRACE_I("SetMediaConnectionWatcherListener()", 0, 0, 0);
    IMediaConnectionWatcher *piMediaConnectionWatcher =
            MediaResourceMngr::GetMediaConnectionWatcher();

    const NetworkPolicy* pImsNetworkPolicy = IMS_NULL;
    pImsNetworkPolicy = NetworkServicePolicy::GetInstance()->GetPolicy(
            NetworkPolicy::APN_IMS, m_nSlotId);

    if (piMediaConnectionWatcher == IMS_NULL || pImsNetworkPolicy == IMS_NULL)
    {
        IMS_TRACE_E(0, "SetMediaConnectionWatcherListener() - null", 0, 0, 0);
        return IMS_FALSE;
    }

    IMS_SINT32 nMediaConnectionType;
    IMS_UINT32 nNetworkInterfaceId;
    AString strPolicyName = pImsNetworkPolicy->GetName();

    return piMediaConnectionWatcher->SetListener(this, strPolicyName, m_nSlotId,
            nMediaConnectionType, nNetworkInterfaceId);
}

PUBLIC
IMS_BOOL MediaResourceMngr::UnsetMediaConnectionWatcherListener()
{
    IMS_TRACE_I("UnsetMediaConnectionWatcherListener()", 0, 0, 0);
    IMediaConnectionWatcher *piMediaConnectionWatcher =
            MediaResourceMngr::GetMediaConnectionWatcher();

    if (piMediaConnectionWatcher == IMS_NULL)
    {
        IMS_TRACE_E(0, "UnsetMediaConnectionWatcherListener() - null", 0, 0, 0);
        return IMS_FALSE;
    }

    return piMediaConnectionWatcher->ReleaseListener(this);
}

PUBLIC
void MediaResourceMngr::SetSlotId(IN IMS_SINT32 nSlotId)
{
    m_nSlotId = nSlotId;
}

PUBLIC
IMS_SINT32 MediaResourceMngr::GetSlotId()
{
    return m_nSlotId;
}