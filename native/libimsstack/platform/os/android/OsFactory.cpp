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
#include "OsFactory.h"
#include "OsFile.h"
#include "OsMutex.h"
#include "OsPthread.h"
#include "OsSystemTime.h"
#include "OsThread.h"
#include "OsTimer.h"
#include "OsTimerService.h"
#include "OsTrace.h"
#include "OsUtil.h"
#include "ServiceMemory.h"
#include "device/OsCarrierConfig.h"
#include "device/OsDeviceInfo.h"
#include "device/OsImsRadio.h"
#include "device/OsIsim.h"
#include "device/OsLocationInfo.h"
#include "device/OsNetworkWatcher.h"
#include "device/OsPhoneInfoCall.h"
#include "device/OsPowerInfo.h"
#include "device/OsSubscriberInfo.h"
#include "device/OsUsim.h"
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
PUBLIC VIRTUAL ImsTrace* OsFactory::CreateTrace()
{
    return new OsTrace();
}

PUBLIC VIRTUAL ImsMutex* OsFactory::CreateMutex(
        IN const AString& /*strName = AString::ConstNull()*/)
{
    return new OsMutex();
}

PUBLIC VIRTUAL ImsFile* OsFactory::CreateFile()
{
    return new OsFile();
}

PUBLIC VIRTUAL IFileUtil* OsFactory::CreateFileUtil()
{
    return new OsFileUtil();
}

PUBLIC VIRTUAL void OsFactory::DestroyFileUtil(IN IFileUtil*& piFileUtil)
{
    OsFileUtil* pFileUtil = DYNAMIC_CAST(OsFileUtil*, piFileUtil);

    if (pFileUtil != IMS_NULL)
    {
        delete pFileUtil;
    }

    piFileUtil = IMS_NULL;
}

PUBLIC VIRTUAL ImsThread* OsFactory::CreateThread(IN const AString& strName, IN IMS_SINT32 nSlotId)
{
    ImsThread* pThread = new OsThread();

    if (!pThread->Create(strName, nSlotId))
    {
        delete pThread;
        return IMS_NULL;
    }

    return pThread;
}

PUBLIC VIRTUAL IMS_ULONG OsFactory::GetCurrentThreadId()
{
    return OsThread::GetCurrentThreadId();
}

PUBLIC VIRTUAL ImsTimer* OsFactory::CreateTimer()
{
    return new OsTimer();
}

PUBLIC VIRTUAL ISystemTime* OsFactory::CreateSystemTime()
{
    return new OsSystemTime();
}

PUBLIC VIRTUAL void OsFactory::DestroySystemTime(IN ISystemTime*& piSysTime)
{
    OsSystemTime* pSysTime = DYNAMIC_CAST(OsSystemTime*, piSysTime);

    if (pSysTime != IMS_NULL)
    {
        delete pSysTime;
    }

    piSysTime = IMS_NULL;
}

PUBLIC VIRTUAL IEventReceiver* OsFactory::CreateEventReceiver(IN IMS_SINT32 nSlotId)
{
    return new OsEventReceiver(nSlotId);
}

PUBLIC VIRTUAL void OsFactory::DestroyEventReceiver(IN IEventReceiver*& piEventReceiver)
{
    OsEventReceiver* pEventReceiver = DYNAMIC_CAST(OsEventReceiver*, piEventReceiver);

    if (pEventReceiver != IMS_NULL)
    {
        delete pEventReceiver;
    }

    piEventReceiver = IMS_NULL;
}

PUBLIC VIRTUAL IEventSender* OsFactory::CreateEventSender()
{
    return new OsEventSender();
}

PUBLIC VIRTUAL void OsFactory::DestroyEventSender(IN IEventSender*& piEventSender)
{
    OsEventSender* pEventSender = DYNAMIC_CAST(OsEventSender*, piEventSender);

    if (pEventSender != IMS_NULL)
    {
        delete pEventSender;
    }

    piEventSender = IMS_NULL;
}

// Platform: network
PUBLIC VIRTUAL ImsNetworkConnection* OsFactory::CreateNetworkConnection(
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

PUBLIC VIRTUAL ImsNetworkConnection* OsFactory::CreateNetworkConnection(
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

PUBLIC VIRTUAL INetworkIpSec* OsFactory::CreateNetworkIpSec(IN IMS_SINT32 nSlotId)
{
    return new OsNetworkIpSec(nSlotId);
}

PUBLIC VIRTUAL void OsFactory::DestroyNetworkIpSec(IN INetworkIpSec*& piIpSec)
{
    OsNetworkIpSec* pIpSec = DYNAMIC_CAST(OsNetworkIpSec*, piIpSec);

    if (pIpSec != IMS_NULL)
    {
        delete pIpSec;
    }

    piIpSec = IMS_NULL;
}

PUBLIC VIRTUAL IIpcan* OsFactory::CreateIpcan()
{
    return new OsIpcan();
}

PUBLIC VIRTUAL void OsFactory::DestroyIpcan(IN IIpcan*& piIpcan)
{
    OsIpcan* pIpcan = DYNAMIC_CAST(OsIpcan*, piIpcan);

    if (pIpcan != IMS_NULL)
    {
        delete pIpcan;
    }

    piIpcan = IMS_NULL;
}

PUBLIC VIRTUAL ImsFdSet* OsFactory::CreateFdSet(IN IMS_SINT32 nType /*= ImsFdSet::TYPE_SELECT*/)
{
    if (nType == ImsFdSet::TYPE_POLL)
    {
        return new OsPollFdSet();
    }

    return new OsSelectFdSet();
}

PUBLIC VIRTUAL ImsSocket* OsFactory::CreateSocket()
{
    return new OsSocket();
}

PUBLIC VIRTUAL ImsSocket* OsFactory::CreateSslSocket(IN SslCertificate* pCertificate)
{
    return new OsSslSocket(pCertificate);
}

// Platform: utilities
PUBLIC VIRTUAL ISystemUtil* OsFactory::GetSystemUtil()
{
    return OsUtil::GetInstance();
}

PUBLIC VIRTUAL ISystemProperty* OsFactory::GetSystemProperty()
{
    return OsUtil::GetInstance();
}

PUBLIC VIRTUAL IZLib* OsFactory::GetZLib()
{
    return OsUtil::GetInstance();
}

// Platform: device
PUBLIC VIRTUAL IPowerInfo* OsFactory::CreatePowerInfo()
{
    return new OsPowerInfo();
}

PUBLIC VIRTUAL void OsFactory::DestroyPowerInfo(IN IPowerInfo*& piPowerInfo)
{
    OsPowerInfo* pPowerInfo = DYNAMIC_CAST(OsPowerInfo*, piPowerInfo);

    if (pPowerInfo != IMS_NULL)
    {
        delete pPowerInfo;
    }

    piPowerInfo = IMS_NULL;
}

PUBLIC VIRTUAL IDeviceInfo* OsFactory::CreateDeviceInfo()
{
    return new OsDeviceInfo();
}

PUBLIC VIRTUAL void OsFactory::DestroyDeviceInfo(IN IDeviceInfo*& piDeviceInfo)
{
    OsDeviceInfo* pDeviceInfo = DYNAMIC_CAST(OsDeviceInfo*, piDeviceInfo);

    if (pDeviceInfo != IMS_NULL)
    {
        delete pDeviceInfo;
    }

    piDeviceInfo = IMS_NULL;
}

PUBLIC VIRTUAL ISubscriberInfo* OsFactory::CreateSubscriberInfo(IN IMS_SINT32 nSlotId)
{
    return new OsSubscriberInfo(nSlotId);
}

PUBLIC VIRTUAL void OsFactory::DestroySubscriberInfo(IN ISubscriberInfo*& piSubscriberInfo)
{
    OsSubscriberInfo* pSubscriberInfo = DYNAMIC_CAST(OsSubscriberInfo*, piSubscriberInfo);

    if (pSubscriberInfo != IMS_NULL)
    {
        delete pSubscriberInfo;
    }

    piSubscriberInfo = IMS_NULL;
}

PUBLIC VIRTUAL INetworkWatcher* OsFactory::CreateNetworkWatcher(IN IMS_SINT32 nSlotId)
{
    return new OsNetworkWatcher(nSlotId);
}

PUBLIC VIRTUAL void OsFactory::DestroyNetworkWatcher(IN INetworkWatcher*& piNetworkWatcher)
{
    OsNetworkWatcher* pNetworkWatcher = DYNAMIC_CAST(OsNetworkWatcher*, piNetworkWatcher);

    if (pNetworkWatcher != IMS_NULL)
    {
        delete pNetworkWatcher;
    }

    piNetworkWatcher = IMS_NULL;
}

PUBLIC VIRTUAL IWifiWatcher* OsFactory::CreateWifiWatcher()
{
    return new OsWifiWatcher();
}

PUBLIC VIRTUAL void OsFactory::DestroyWifiWatcher(IN IWifiWatcher*& piWifiWatcher)
{
    OsWifiWatcher* pWifiWatcher = DYNAMIC_CAST(OsWifiWatcher*, piWifiWatcher);

    if (pWifiWatcher != IMS_NULL)
    {
        delete pWifiWatcher;
    }

    piWifiWatcher = IMS_NULL;
}

PUBLIC VIRTUAL ICallInfo* OsFactory::CreateCallInfo(IN IMS_SINT32 nSlotId)
{
    return new OsPhoneInfoCall(nSlotId);
}

PUBLIC VIRTUAL void OsFactory::DestroyCallInfo(IN ICallInfo*& piCallInfo)
{
    OsPhoneInfoCall* pPhoneInfoCall = DYNAMIC_CAST(OsPhoneInfoCall*, piCallInfo);

    if (pPhoneInfoCall != IMS_NULL)
    {
        delete pPhoneInfoCall;
    }

    piCallInfo = IMS_NULL;
}

PUBLIC VIRTUAL ILocationInfo* OsFactory::CreateLocationInfo(IN IMS_SINT32 nSlotId)
{
    return new OsLocationInfo(nSlotId);
}

PUBLIC VIRTUAL void OsFactory::DestroyLocationInfo(IN ILocationInfo*& piLocationInfo)
{
    OsLocationInfo* pLocationInfo = DYNAMIC_CAST(OsLocationInfo*, piLocationInfo);

    if (pLocationInfo != IMS_NULL)
    {
        delete pLocationInfo;
    }

    piLocationInfo = IMS_NULL;
}

PUBLIC VIRTUAL ImsIsim* OsFactory::CreateIsim(IN IMS_SINT32 nSlotId)
{
    return new OsIsim(nSlotId);
}

PUBLIC VIRTUAL ImsUsim* OsFactory::CreateUsim(IN IMS_SINT32 nSlotId)
{
    return new OsUsim(nSlotId);
}

PUBLIC VIRTUAL ImsCarrierConfig* OsFactory::CreateCarrierConfig(IN IMS_SINT32 nSlotId)
{
    return new OsCarrierConfig(nSlotId);
}

PUBLIC VIRTUAL ImsRadio* OsFactory::CreateImsRadio(IN IMS_SINT32 nSlotId)
{
    return new OsImsRadio(nSlotId);
}
