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
#ifndef SERVICE_PHONE_INFO_H_
#define SERVICE_PHONE_INFO_H_

#include "IIsim.h"
#include "INetworkWatcher.h"
#include "IPhoneInfoCall.h"
#include "IPhoneInfoDevice.h"
#include "IPhoneInfoLocation.h"
#include "IPhoneInfoPower.h"
#include "IPhoneInfoSubscriber.h"
#include "ISrvcc.h"
#include "ITrm.h"
#include "IUsim.h"
#include "IWifiWatcher.h"

class PhoneInfoServicePrivate;

class PhoneInfoService
{
private:
    PhoneInfoService();
    ~PhoneInfoService();

public:
    PhoneInfoService(IN const PhoneInfoService&) = delete;
    PhoneInfoService& operator=(IN const PhoneInfoService&) = delete;

public:
    void DispatchServiceMessage(IN ImsMessage& objMsg);

    IDeviceInfo* GetDeviceInfo();
    IPowerInfo* GetPowerInfo();

    ICallInfo* GetCallInfo(IN IMS_SINT32 nSlotId);
    ILocationInfo* GetLocationInfo(IN IMS_SINT32 nSlotId);
    ISubscriberInfo* GetSubscriberInfo(IN IMS_SINT32 nSlotId);

    IIsim* GetIsim(IN IMS_SINT32 nSlotId);
    IUsim* GetUsim(IN IMS_SINT32 nSlotId);

    inline INetworkWatcher* GetNetworkWatcher()
            __IMS_DEPRECATED__("Use GetNetworkWatcher(IMS_SINT32) instead")
    { return GetNetworkWatcher(IMS_SLOT_0); }
    INetworkWatcher* GetNetworkWatcher(IN IMS_SINT32 nSlotId);
    IWifiWatcher* GetWifiWatcher();

    ISrvcc* GetSrvcc(IN IMS_SINT32 nSlotId);
    ITrm* GetTrm();

    static PhoneInfoService* GetPhoneInfoService();

private:
    PhoneInfoServicePrivate* m_pPrivate;
};

#endif
