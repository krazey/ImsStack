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
#ifndef INTERFACE_OS_FACTORY_H_
#define INTERFACE_OS_FACTORY_H_

#include "AString.h"
#include "ImsFdSet.h"

class ICallInfo;
class IDeviceInfo;
class IEventReceiver;
class IEventSender;
class IFileUtil;
class IImsTraffic;
class IIpcan;
class ILocationInfo;
class INetworkIpSec;
class INetworkWatcher;
class IPowerInfo;
class ISubscriberInfo;
class ISystemProperty;
class ISystemTime;
class ISystemUtil;
class IWifiWatcher;
class IZLib;
class ImsCarrierConfig;
class ImsFile;
class ImsIsim;
class ImsMutex;
class ImsNetworkConnection;
class ImsRadio;
class ImsSocket;
class ImsThread;
class ImsTimer;
class ImsTrace;
class ImsUsim;
class SslCertificate;

class IOsFactory
{
protected:
    virtual ~IOsFactory() = default;

public:
    virtual void Destroy() = 0;
    // Platform: common
    virtual ImsTrace* CreateTrace() = 0;
    virtual ImsMutex* CreateMutex(IN const AString& strName = AString::ConstNull()) = 0;

    virtual ImsFile* CreateFile() = 0;
    virtual IFileUtil* CreateFileUtil() = 0;
    virtual void DestroyFileUtil(IN IFileUtil*& piFileUtil) = 0;

    virtual ImsThread* CreateThread(IN const AString& strName, IN IMS_SINT32 nSlotId) = 0;
    virtual IMS_ULONG GetCurrentThreadId() = 0;

    virtual ImsTimer* CreateTimer() = 0;
    virtual ISystemTime* CreateSystemTime() = 0;
    virtual void DestroySystemTime(IN ISystemTime*& piSysTime) = 0;

    virtual IEventReceiver* CreateEventReceiver(IN IMS_SINT32 nSlotId) = 0;
    virtual void DestroyEventReceiver(IN IEventReceiver*& piEventReceiver) = 0;
    virtual IEventSender* CreateEventSender() = 0;
    virtual void DestroyEventSender(IN IEventSender*& piEventSender) = 0;

    // Platform: network
    virtual ImsNetworkConnection* CreateNetworkConnection(
            IN const AString& strProfile, IN IMS_SINT32 nSlotId) = 0;
    virtual ImsNetworkConnection* CreateNetworkConnection(
            IN IMS_SINT32 nApnType, IN IMS_SINT32 nSlotId) = 0;

    virtual INetworkIpSec* CreateNetworkIpSec(IN IMS_SINT32 nSlotId) = 0;
    virtual void DestroyNetworkIpSec(IN INetworkIpSec*& piIpSec) = 0;

    virtual IIpcan* CreateIpcan() = 0;
    virtual void DestroyIpcan(IN IIpcan*& piIpcan) = 0;

    virtual ImsFdSet* CreateFdSet(IN IMS_SINT32 nType = ImsFdSet::TYPE_SELECT) = 0;
    virtual ImsSocket* CreateSocket() = 0;
    virtual ImsSocket* CreateSslSocket(IN SslCertificate* pCertificate) = 0;

    // Platform: utilities
    virtual ISystemUtil* GetSystemUtil() = 0;
    virtual ISystemProperty* GetSystemProperty() = 0;
    virtual IZLib* GetZLib() = 0;

    // Platform: device
    virtual IPowerInfo* CreatePowerInfo() = 0;
    virtual void DestroyPowerInfo(IN IPowerInfo*& piPowerInfo) = 0;

    virtual IDeviceInfo* CreateDeviceInfo() = 0;
    virtual void DestroyDeviceInfo(IN IDeviceInfo*& piDeviceInfo) = 0;

    virtual ISubscriberInfo* CreateSubscriberInfo(IN IMS_SINT32 nSlotId) = 0;
    virtual void DestroySubscriberInfo(IN ISubscriberInfo*& piSubscriberInfo) = 0;

    virtual INetworkWatcher* CreateNetworkWatcher(IN IMS_SINT32 nSlotId) = 0;
    virtual void DestroyNetworkWatcher(IN INetworkWatcher*& piNetworkWatcher) = 0;

    virtual IWifiWatcher* CreateWifiWatcher() = 0;
    virtual void DestroyWifiWatcher(IN IWifiWatcher*& piWifiWatcher) = 0;

    virtual ICallInfo* CreateCallInfo(IN IMS_SINT32 nSlotId) = 0;
    virtual void DestroyCallInfo(IN ICallInfo*& piCallInfo) = 0;

    virtual ILocationInfo* CreateLocationInfo(IN IMS_SINT32 nSlotId) = 0;
    virtual void DestroyLocationInfo(IN ILocationInfo*& piLocationInfo) = 0;

    virtual ImsIsim* CreateIsim(IN IMS_SINT32 nSlotId) = 0;
    virtual ImsUsim* CreateUsim(IN IMS_SINT32 nSlotId) = 0;
    virtual ImsCarrierConfig* CreateCarrierConfig(IN IMS_SINT32 nSlotId) = 0;

    virtual ImsRadio* CreateImsRadio(IN IMS_SINT32 nSlotId) = 0;
    virtual IImsTraffic* CreateImsTraffic() = 0;
    virtual void DestroyImsTraffic(IN IImsTraffic*& piImsTraffic) = 0;
};

#endif
