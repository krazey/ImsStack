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
#ifndef PLATFORM_FACTORY_H_
#define PLATFORM_FACTORY_H_

#include "ImsFdSet.h"
#include "ImsSocket.h"

class ICallInfo;
class IDeviceInfo;
class IEventReceiver;
class IEventSender;
class IFileUtil;
class IIpcan;
class IIsim;
class ILocationInfo;
class IMemHeap;
class INetworkIpSec;
class INetworkWatcher;
class IPowerInfo;
class ISubscriberInfo;
class ISystemProperty;
class ISystemTime;
class ISystemUtil;
class IThread;
class IUsim;
class IWifiWatcher;
class IZLib;
class ImsFile;
class ImsMutex;
class ImsNetworkConnection;
class ImsThread;
class ImsTimer;
class ImsTrace;
class ImsCarrierConfig;
class SslCertificate;

class PlatformFactory
{
public:
    // Platform: common
    static IMemHeap* GetHeap();
    static ImsTrace* CreateTrace();
    static ImsMutex* CreateMutex(IN const AString& strName = AString::ConstNull());
    static ImsFile* CreateFile();
    static IFileUtil* CreateFileUtil();
    static void DestroyFileUtil(IN IFileUtil*& piFileUtil);

    static ImsThread* CreateThread();
    static ImsThread* CreateThreadEx();
    static IMS_ULONG GetCurrentThreadId();

    static ImsTimer* CreateTimer();

    static ISystemTime* CreateSystemTime();
    static void DestroySystemTime(IN ISystemTime*& piSysTime);

    static IEventReceiver* CreateEventReceiver(IN IMS_SINT32 nSlotId);
    static void DestroyEventReceiver(IN IEventReceiver*& piEventReceiver);

    static IEventSender* CreateEventSender();
    static void DestroyEventSender(IN IEventSender*& piEventSender);

    // Platform: network
    static ImsNetworkConnection* CreateNetworkConnection(
            IN const AString& strProfile, IN IMS_SINT32 nSlotId);
    static ImsNetworkConnection* CreateNetworkConnection(
            IN IMS_SINT32 nApnType, IN IMS_SINT32 nSlotId);

    static INetworkIpSec* CreateNetworkIpSec();
    static void DestroyNetworkIpSec(IN INetworkIpSec*& piIpSec);

    static IIpcan* CreateIpcan();
    static void DestroyIpcan(IN IIpcan*& piIpcan);

    static ImsFdSet* CreateFdSet(IN IMS_SINT32 nType = ImsFdSet::TYPE_SELECT);
    static ImsSocket* CreateSocket();
    static ImsSocket* CreateSslSocket(IN SslCertificate* pCertificate);

    // Platform: utilities
    static ISystemUtil* GetSystemUtil();
    static ISystemProperty* GetSystemProperty();
    static IZLib* GetZLib();

    // Platform: device
    static IPowerInfo* CreatePowerInfo();
    static void DestroyPowerInfo(IN IPowerInfo*& piPowerInfo);

    static IDeviceInfo* CreateDeviceInfo();
    static void DestroyDeviceInfo(IN IDeviceInfo*& piDeviceInfo);

    static ISubscriberInfo* CreateSubscriberInfo(IN IMS_SINT32 nSlotId);
    static void DestroySubscriberInfo(IN ISubscriberInfo*& piSubscriberInfo);

    static INetworkWatcher* CreateNetworkWatcher(IN IMS_SINT32 nSlotId);
    static void DestroyNetworkWatcher(IN INetworkWatcher*& piNetworkWatcher);

    static IWifiWatcher* CreateWifiWatcher();
    static void DestroyWifiWatcher(IN IWifiWatcher*& piWifiWatcher);

    static ICallInfo* CreateCallInfo(IN IMS_SINT32 nSlotId);
    static void DestroyCallInfo(IN ICallInfo*& piCallInfo);

    static ILocationInfo* CreateLocationInfo(IN IMS_SINT32 nSlotId);
    static void DestroyLocationInfo(IN ILocationInfo*& piLocationInfo);

    static IIsim* CreateIsim(IN IMS_SINT32 nSlotId);
    static void DestroyIsim(IN IIsim*& piIsim);

    static IUsim* CreateUsim(IN IMS_SINT32 nSlotId);
    static void DestroyUsim(IN IUsim*& piUsim);

    static ImsCarrierConfig* CreateCarrierConfig(IN IMS_SINT32 nSlotId);
    static void DestroyCarrierConfig(IN ImsCarrierConfig*& pCarrierConfig);
};

#endif
