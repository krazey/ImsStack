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
#include "OsEventReceiver.h"
#include "OsEventSender.h"
#include "OsFile.h"
#include "OsHeap.h"
#include "OsMutex.h"
#include "OsPthread.h"
#include "OsSystemTime.h"
#include "OsThread.h"
#include "OsTimer.h"
#include "OsTimerService.h"
#include "OsTrace.h"
#include "OsUtil.h"
#include "PlatformFactory.h"
#include "ServiceMemory.h"
#include "ServiceNetworkPolicy.h"
#include "device/OsCarrierConfig.h"
#include "device/OsDeviceInfo.h"
#include "device/OsIsim.h"
#include "device/OsLocationInfo.h"
#include "device/OsPhoneInfoCall.h"
#include "device/OsPowerInfo.h"
#include "device/OsNetworkWatcher.h"
#include "device/OsSubscriberInfo.h"
#include "device/OsTrm.h"
#include "device/OsUsim.h"
#include "device/OsVoNr.h"
#include "device/OsWifiWatcher.h"
#include "network/OsIpcan.h"
#include "network/OsNetworkConnection.h"
#include "network/OsNetworkIpSec.h"
#include "network/OsPollFdSet.h"
#include "network/OsSelectFdSet.h"
#include "network/OsSocket.h"
#include "network/OsSslSocket.h"
#include "network/OsWifiConnection.h"

// Platform: common
PUBLIC GLOBAL IMemHeap* PlatformFactory::GetHeap()
{
    return OsHeap::GetHeap();
}

PUBLIC GLOBAL ImsTrace* PlatformFactory::CreateTrace()
{
    return new OsTrace();
}

PUBLIC GLOBAL ImsMutex* PlatformFactory::CreateMutex(
        IN const AString& strName /*= AString::ConstNull()*/)
{
    (void)strName;
    return new OsMutex();
}

PUBLIC GLOBAL ImsFile* PlatformFactory::CreateFile()
{
    return new OsFile();
}

PUBLIC GLOBAL IFileUtil* PlatformFactory::CreateFileUtil()
{
    return new OsFileUtil();
}

PUBLIC GLOBAL void PlatformFactory::DestroyFileUtil(IN IFileUtil*& piFileUtil)
{
    OsFileUtil* pFileUtil = DYNAMIC_CAST(OsFileUtil*, piFileUtil);

    if (pFileUtil != IMS_NULL)
    {
        delete pFileUtil;
    }

    piFileUtil = IMS_NULL;
}

PUBLIC GLOBAL ImsThread* PlatformFactory::CreateThread()
{
    return new OsThread();
}

PUBLIC GLOBAL ImsThread* PlatformFactory::CreateThreadEx()
{
    return new OsThread();
}

PUBLIC GLOBAL IMS_ULONG PlatformFactory::GetCurrentThreadId()
{
    return OsThread::GetCurrentThreadId();
}

PUBLIC GLOBAL ImsTimer* PlatformFactory::CreateTimer()
{
    return new OsTimer();
}

PUBLIC GLOBAL ISystemTime* PlatformFactory::CreateSystemTime()
{
    return new OsSystemTime();
}

PUBLIC GLOBAL void PlatformFactory::DestroySystemTime(IN ISystemTime*& piSysTime)
{
    OsSystemTime* pSysTime = DYNAMIC_CAST(OsSystemTime*, piSysTime);

    if (pSysTime != IMS_NULL)
    {
        delete pSysTime;
    }

    piSysTime = IMS_NULL;
}

PUBLIC GLOBAL IEventReceiver* PlatformFactory::CreateEventReceiver(IN IMS_SINT32 nSlotId)
{
    return new OsEventReceiver(nSlotId);
}

PUBLIC GLOBAL void PlatformFactory::DestroyEventReceiver(IN IEventReceiver*& piEventReceiver)
{
    OsEventReceiver* pEventReceiver = DYNAMIC_CAST(OsEventReceiver*, piEventReceiver);

    if (pEventReceiver != IMS_NULL)
    {
        delete pEventReceiver;
    }

    piEventReceiver = IMS_NULL;
}

PUBLIC GLOBAL IEventSender* PlatformFactory::CreateEventSender()
{
    return new OsEventSender();
}

PUBLIC GLOBAL void PlatformFactory::DestroyEventSender(IN IEventSender*& piEventSender)
{
    OsEventSender* pEventSender = DYNAMIC_CAST(OsEventSender*, piEventSender);

    if (pEventSender != IMS_NULL)
    {
        delete pEventSender;
    }

    piEventSender = IMS_NULL;
}

// Platform: network
PUBLIC GLOBAL ImsNetworkConnection* PlatformFactory::CreateNetworkConnection(
        IN const AString& strProfile, IN IMS_SINT32 nSlotId)
{
    if (NetworkPolicy::IsMobilePolicy(strProfile))
    {
        return new OsNetworkConnection(nSlotId);
    }
    else if (NetworkPolicy::IsWiFiPolicy(strProfile))
    {
        // In the moment, Wi-Fi connection isn't required for each slots.
        return new OsWifiConnection();
    }

    return IMS_NULL;
}

PUBLIC GLOBAL ImsNetworkConnection* PlatformFactory::CreateNetworkConnection(
        IN IMS_SINT32 nApnType, IN IMS_SINT32 nSlotId)
{
    if (NetworkPolicy::IsMobilePolicy(nApnType))
    {
        return new OsNetworkConnection(nSlotId);
    }
    else if (NetworkPolicy::IsWiFiPolicy(nApnType))
    {
        // In the moment, Wi-Fi connection isn't required for each slots.
        return new OsWifiConnection();
    }

    return IMS_NULL;
}

PUBLIC GLOBAL INetworkIpSec* PlatformFactory::CreateNetworkIpSec()
{
    return new OsNetworkIpSec();
}

PUBLIC GLOBAL void PlatformFactory::DestroyNetworkIpSec(IN INetworkIpSec*& piIpSec)
{
    OsNetworkIpSec* pIpSec = DYNAMIC_CAST(OsNetworkIpSec*, piIpSec);

    if (pIpSec != IMS_NULL)
    {
        delete pIpSec;
    }

    piIpSec = IMS_NULL;
}

PUBLIC GLOBAL IIpcan* PlatformFactory::CreateIpcan()
{
    return new OsIpcan();
}

PUBLIC GLOBAL void PlatformFactory::DestroyIpcan(IN IIpcan*& piIpcan)
{
    OsIpcan* pIpcan = DYNAMIC_CAST(OsIpcan*, piIpcan);

    if (pIpcan != IMS_NULL)
    {
        delete pIpcan;
    }

    piIpcan = IMS_NULL;
}

PUBLIC GLOBAL ImsFdSet* PlatformFactory::CreateFdSet(
        IN IMS_SINT32 nType /*= ImsFdSet::TYPE_SELECT*/)
{
    if (nType == ImsFdSet::TYPE_POLL)
    {
        return new OsPollFdSet();
    }

    return new OsSelectFdSet();
}

PUBLIC GLOBAL ImsSocket* PlatformFactory::CreateSocket()
{
    return new OsSocket();
}

PUBLIC GLOBAL ImsSocket* PlatformFactory::CreateSslSocket(IN SSLCertificate* pCertificate)
{
    return new OsSslSocket(pCertificate);
}

// Platform: utilities
PUBLIC GLOBAL ISystemUtil* PlatformFactory::GetSystemUtil()
{
    return OsUtil::GetInstance();
}

PUBLIC GLOBAL ISystemProperty* PlatformFactory::GetSystemProperty()
{
    return OsUtil::GetInstance();
}

PUBLIC GLOBAL IZLib* PlatformFactory::GetZLib()
{
    return OsUtil::GetInstance();
}

// Platform: device
PUBLIC GLOBAL IPowerInfo* PlatformFactory::CreatePowerInfo()
{
    return new OsPowerInfo();
}

PUBLIC GLOBAL void PlatformFactory::DestroyPowerInfo(IN IPowerInfo*& piPowerInfo)
{
    OsPowerInfo* pPowerInfo = DYNAMIC_CAST(OsPowerInfo*, piPowerInfo);

    if (pPowerInfo != IMS_NULL)
    {
        delete pPowerInfo;
    }

    piPowerInfo = IMS_NULL;
}

PUBLIC GLOBAL IDeviceInfo* PlatformFactory::CreateDeviceInfo()
{
    return new OsDeviceInfo();
}

PUBLIC GLOBAL void PlatformFactory::DestroyDeviceInfo(IN IDeviceInfo*& piDeviceInfo)
{
    OsDeviceInfo* pDeviceInfo = DYNAMIC_CAST(OsDeviceInfo*, piDeviceInfo);

    if (pDeviceInfo != IMS_NULL)
    {
        delete pDeviceInfo;
    }

    piDeviceInfo = IMS_NULL;
}

PUBLIC GLOBAL ISubscriberInfo* PlatformFactory::CreateSubscriberInfo(IN IMS_SINT32 nSlotId)
{
    return new OsSubscriberInfo(nSlotId);
}

PUBLIC GLOBAL void PlatformFactory::DestroySubscriberInfo(IN ISubscriberInfo*& piSubscriberInfo)
{
    OsSubscriberInfo* pSubscriberInfo = DYNAMIC_CAST(OsSubscriberInfo*, piSubscriberInfo);

    if (pSubscriberInfo != IMS_NULL)
    {
        delete pSubscriberInfo;
    }

    piSubscriberInfo = IMS_NULL;
}

PUBLIC GLOBAL INetworkWatcher* PlatformFactory::CreateNetworkWatcher(IN IMS_SINT32 nSlotId)
{
    return new OsNetworkWatcher(nSlotId);
}

PUBLIC GLOBAL void PlatformFactory::DestroyNetworkWatcher(IN INetworkWatcher*& piNetworkWatcher)
{
    OsNetworkWatcher* pNetworkWatcher = DYNAMIC_CAST(OsNetworkWatcher*, piNetworkWatcher);

    if (pNetworkWatcher != IMS_NULL)
    {
        delete pNetworkWatcher;
    }

    piNetworkWatcher = IMS_NULL;
}

PUBLIC GLOBAL IWifiWatcher* PlatformFactory::CreateWifiWatcher()
{
    return new OsWifiWatcher();
}

PUBLIC GLOBAL void PlatformFactory::DestroyWifiWatcher(IN IWifiWatcher*& piWifiWatcher)
{
    OsWifiWatcher* pWifiWatcher = DYNAMIC_CAST(OsWifiWatcher*, piWifiWatcher);

    if (pWifiWatcher != IMS_NULL)
    {
        delete pWifiWatcher;
    }

    piWifiWatcher = IMS_NULL;
}

PUBLIC GLOBAL ICallInfo* PlatformFactory::CreateCallInfo(IN IMS_SINT32 nSlotId)
{
    return new OsPhoneInfoCall(nSlotId);
}

PUBLIC GLOBAL void PlatformFactory::DestroyCallInfo(IN ICallInfo*& piPhoneInfoCall)
{
    OsPhoneInfoCall* pPhoneInfoCall = DYNAMIC_CAST(OsPhoneInfoCall*, piPhoneInfoCall);

    if (pPhoneInfoCall != IMS_NULL)
    {
        delete pPhoneInfoCall;
    }

    piPhoneInfoCall = IMS_NULL;
}

PUBLIC GLOBAL ILocationInfo* PlatformFactory::CreateLocationInfo(IN IMS_SINT32 nSlotId)
{
    return new OsLocationInfo(nSlotId);
}

PUBLIC GLOBAL void PlatformFactory::DestroyLocationInfo(IN ILocationInfo*& piLocationInfo)
{
    OsLocationInfo* pLocationInfo = DYNAMIC_CAST(OsLocationInfo*, piLocationInfo);

    if (pLocationInfo != IMS_NULL)
    {
        delete pLocationInfo;
    }

    piLocationInfo = IMS_NULL;
}

PUBLIC GLOBAL IIsim* PlatformFactory::CreateIsim(IN IMS_SINT32 nSlotId)
{
    return new OsIsim(nSlotId);
}

PUBLIC GLOBAL void PlatformFactory::DestroyIsim(IN IIsim*& piIsim)
{
    OsIsim* pIsim = DYNAMIC_CAST(OsIsim*, piIsim);

    if (pIsim != IMS_NULL)
    {
        delete pIsim;
    }

    piIsim = IMS_NULL;
}

PUBLIC GLOBAL IUsim* PlatformFactory::CreateUsim(IN IMS_SINT32 nSlotId)
{
    return new OsUsim(nSlotId);
}

PUBLIC GLOBAL void PlatformFactory::DestroyUsim(IN IUsim*& piUsim)
{
    OsUsim* pUsim = DYNAMIC_CAST(OsUsim*, piUsim);

    if (pUsim != IMS_NULL)
    {
        delete pUsim;
    }

    piUsim = IMS_NULL;
}

PUBLIC GLOBAL ITrm* PlatformFactory::CreateTrm()
{
    return new OsTrm();
}

PUBLIC GLOBAL void PlatformFactory::DestroyTrm(IN ITrm*& piTrm)
{
    OsTrm* pTrm = DYNAMIC_CAST(OsTrm*, piTrm);

    if (pTrm != IMS_NULL)
    {
        delete pTrm;
    }

    piTrm = IMS_NULL;
}

PUBLIC GLOBAL IVoNr* PlatformFactory::CreateVoNr(IN IMS_SINT32 nSlotId)
{
    return new OsVoNr(nSlotId);
}

PUBLIC GLOBAL void PlatformFactory::DestroyVoNr(IN IVoNr*& piVoNr)
{
    OsVoNr* pVoNr = DYNAMIC_CAST(OsVoNr*, piVoNr);

    if (pVoNr != IMS_NULL)
    {
        delete pVoNr;
    }

    piVoNr = IMS_NULL;
}

PUBLIC GLOBAL ImsCarrierConfig* PlatformFactory::CreateCarrierConfig(IN IMS_SINT32 nSlotId)
{
    return new OsCarrierConfig(nSlotId);
}

PUBLIC GLOBAL void PlatformFactory::DestroyCarrierConfig(IN ImsCarrierConfig*& pCarrierConfig)
{
    if (pCarrierConfig != IMS_NULL)
    {
        delete pCarrierConfig;
        pCarrierConfig = IMS_NULL;
    }
}
