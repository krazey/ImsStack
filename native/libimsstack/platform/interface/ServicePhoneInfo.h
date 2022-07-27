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
#include "IUsim.h"
#include "IWifiWatcher.h"
#include "PlatformService.h"

class PhoneInfoServicePrivate;

class PhoneInfoService : public PlatformService
{
public:
    PhoneInfoService();
    PhoneInfoService(IN const PhoneInfoService&) = delete;
    PhoneInfoService& operator=(IN const PhoneInfoService&) = delete;

protected:
    virtual ~PhoneInfoService();

public:
    virtual IDeviceInfo* GetDeviceInfo();
    virtual IPowerInfo* GetPowerInfo();

    virtual ICallInfo* GetCallInfo(IN IMS_SINT32 nSlotId);
    virtual ILocationInfo* GetLocationInfo(IN IMS_SINT32 nSlotId);
    virtual ISubscriberInfo* GetSubscriberInfo(IN IMS_SINT32 nSlotId);

    virtual IIsim* GetIsim(IN IMS_SINT32 nSlotId);
    virtual IUsim* GetUsim(IN IMS_SINT32 nSlotId);

    virtual INetworkWatcher* GetNetworkWatcher(IN IMS_SINT32 nSlotId);
    virtual IWifiWatcher* GetWifiWatcher();

    void DispatchServiceMessage(IN ImsMessage& objMsg);
    static PhoneInfoService* GetPhoneInfoService();

private:
    PhoneInfoServicePrivate* m_pPrivate;
};

#endif
