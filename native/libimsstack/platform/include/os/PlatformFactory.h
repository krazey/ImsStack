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

class ICSCallInfo;
class IDeviceInfo;
class IEventReceiver;
class IEventSender;
class IFileUtil;
class IIPCAN;
class IISIM;
class ILocationInfo;
class IMemHeap;
class INetIPSec;
class INetWatcherInfo;
class IPhoneInfoCall;
class IPowerInfo;
class ISRVCC;
class ISubscriberInfo;
class ISystemProperty;
class ISystemTime;
class ISystemUtil;
class ITRM;
class IThread;
class IUSIM;
class IVoNR;
class IWifiWatcher;
class IZLib;
class ImsContentCursor;
class ImsContentProvider;
class ImsContentTable;
class ImsFile;
class ImsMutex;
class ImsNetworkConnection;
class ImsThread;
class ImsTimer;
class ImsTrace;
class ImsCarrierConfig;
class SSLCertificate;

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

    // DB - ContentProvider
    static ImsContentTable* CreateContentTable();
    static ImsContentCursor* CreateContentCursor();
    static ImsContentProvider* CreateContentProvider();

    // Platform: network
    static ImsNetworkConnection* CreateNetworkConnection(
            IN const AString& strProfile, IN IMS_SINT32 nSlotId);
    static ImsNetworkConnection* CreateNetworkConnection(
            IN IMS_SINT32 nApnType, IN IMS_SINT32 nSlotId);

    static INetIPSec* CreateNetworkIpSec();
    static void DestroyNetworkIpSec(IN INetIPSec*& piIpSec);

    static IIPCAN* CreateIpcan();
    static void DestroyIpcan(IN IIPCAN*& piIpcan);

    static ImsFdSet* CreateFdSet(IN IMS_SINT32 nType = ImsFdSet::TYPE_SELECT);
    static ImsSocket* CreateSocket();
    static ImsSocket* CreateSslSocket(IN SSLCertificate* pCertificate);

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

    static INetWatcherInfo* CreateNetworkWatcher(IN IMS_SINT32 nSlotId);
    static void DestroyNetworkWatcher(IN INetWatcherInfo*& piNetworkWatcher);

    static IWifiWatcher* CreateWifiWatcher();
    static void DestroyWifiWatcher(IN IWifiWatcher*& piWifiWatcher);

    static ICSCallInfo* CreateCsCallInfo(IN IMS_SINT32 nSlotId);
    static void DestroyCsCallInfo(IN ICSCallInfo*& piCsCallInfo);

    static IPhoneInfoCall* CreatePhoneInfoCall(IN IMS_SINT32 nSlotId);
    static void DestroyPhoneInfoCall(IN IPhoneInfoCall*& piPhoneInfoCall);

    static ISRVCC* CreateSrvcc(IN IMS_SINT32 nSlotId);
    static void DestroySrvcc(IN ISRVCC*& piSrvcc);

    static ILocationInfo* CreateLocationInfo(IN IMS_SINT32 nSlotId);
    static void DestroyLocationInfo(IN ILocationInfo*& piLocationInfo);

    static IISIM* CreateIsim(IN IMS_SINT32 nSlotId);
    static void DestroyIsim(IN IISIM*& piIsim);

    static IUSIM* CreateUsim(IN IMS_SINT32 nSlotId);
    static void DestroyUsim(IN IUSIM*& piUsim);

    static ITRM* CreateTrm();
    static void DestroyTrm(IN ITRM*& piTrm);

    static IVoNR* CreateVoNr(IN IMS_SINT32 nSlotId);
    static void DestroyVoNr(IN IVoNR*& piVoNr);

    static ImsCarrierConfig* CreateCarrierConfig(IN IMS_SINT32 nSlotId);
    static void DestroyCarrierConfig(IN ImsCarrierConfig*& pCarrierConfig);
};

#endif
