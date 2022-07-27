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
#ifndef OS_FACTORY_H_
#define OS_FACTORY_H_

#include "IOsFactory.h"

class OsFactory : public IOsFactory
{
public:
    inline OsFactory() {}
    inline virtual ~OsFactory() {}

public:
    OsFactory(IN const OsFactory&) = delete;
    OsFactory& operator=(IN const OsFactory&) = delete;

public:
    inline void Destroy() override { delete this; }

    // Platform: common
    ImsTrace* CreateTrace() override;
    ImsMutex* CreateMutex(IN const AString& strName = AString::ConstNull()) override;

    ImsFile* CreateFile() override;
    IFileUtil* CreateFileUtil() override;
    void DestroyFileUtil(IN IFileUtil*& piFileUtil) override;

    ImsThread* CreateThread(IN const AString& strName, IN IMS_SINT32 nSlotId) override;
    IMS_ULONG GetCurrentThreadId() override;

    ImsTimer* CreateTimer() override;
    ISystemTime* CreateSystemTime() override;
    void DestroySystemTime(IN ISystemTime*& piSysTime) override;

    IEventReceiver* CreateEventReceiver(IN IMS_SINT32 nSlotId) override;
    void DestroyEventReceiver(IN IEventReceiver*& piEventReceiver) override;
    IEventSender* CreateEventSender() override;
    void DestroyEventSender(IN IEventSender*& piEventSender) override;

    // Platform: network
    ImsNetworkConnection* CreateNetworkConnection(
            IN const AString& strProfile, IN IMS_SINT32 nSlotId) override;
    ImsNetworkConnection* CreateNetworkConnection(
            IN IMS_SINT32 nApnType, IN IMS_SINT32 nSlotId) override;

    INetworkIpSec* CreateNetworkIpSec() override;
    void DestroyNetworkIpSec(IN INetworkIpSec*& piIpSec) override;

    IIpcan* CreateIpcan() override;
    void DestroyIpcan(IN IIpcan*& piIpcan) override;

    ImsFdSet* CreateFdSet(IN IMS_SINT32 nType = ImsFdSet::TYPE_SELECT) override;
    ImsSocket* CreateSocket() override;
    ImsSocket* CreateSslSocket(IN SslCertificate* pCertificate) override;

    // Platform: utilities
    ISystemUtil* GetSystemUtil() override;
    ISystemProperty* GetSystemProperty() override;
    IZLib* GetZLib() override;

    // Platform: device
    IPowerInfo* CreatePowerInfo() override;
    void DestroyPowerInfo(IN IPowerInfo*& piPowerInfo) override;

    IDeviceInfo* CreateDeviceInfo() override;
    void DestroyDeviceInfo(IN IDeviceInfo*& piDeviceInfo) override;

    ISubscriberInfo* CreateSubscriberInfo(IN IMS_SINT32 nSlotId) override;
    void DestroySubscriberInfo(IN ISubscriberInfo*& piSubscriberInfo) override;

    INetworkWatcher* CreateNetworkWatcher(IN IMS_SINT32 nSlotId) override;
    void DestroyNetworkWatcher(IN INetworkWatcher*& piNetworkWatcher) override;

    IWifiWatcher* CreateWifiWatcher() override;
    void DestroyWifiWatcher(IN IWifiWatcher*& piWifiWatcher) override;

    ICallInfo* CreateCallInfo(IN IMS_SINT32 nSlotId) override;
    void DestroyCallInfo(IN ICallInfo*& piCallInfo) override;

    ILocationInfo* CreateLocationInfo(IN IMS_SINT32 nSlotId) override;
    void DestroyLocationInfo(IN ILocationInfo*& piLocationInfo) override;

    ImsIsim* CreateIsim(IN IMS_SINT32 nSlotId) override;
    ImsUsim* CreateUsim(IN IMS_SINT32 nSlotId) override;
    ImsCarrierConfig* CreateCarrierConfig(IN IMS_SINT32 nSlotId) override;
};

#endif
