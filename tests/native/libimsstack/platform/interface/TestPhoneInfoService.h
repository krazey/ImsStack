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
#ifndef TEST_PHONE_INFO_SERVICE_H_
#define TEST_PHONE_INFO_SERVICE_H_

#include "MockIIsim.h"
#include "MockINetworkWatcher.h"
#include "MockIPhoneInfoCall.h"
#include "MockIPhoneInfoDevice.h"
#include "MockIPhoneInfoLocation.h"
#include "MockIPhoneInfoPower.h"
#include "MockIPhoneInfoSubscriber.h"
#include "MockIUsim.h"
#include "MockIWifiWatcher.h"
#include "ServicePhoneInfo.h"

class TestPhoneInfoService : public PhoneInfoService
{
public:
    inline IDeviceInfo* GetDeviceInfo() override { return &m_objDeviceInfo; }
    inline IPowerInfo* GetPowerInfo() override { return &m_objPowerInfo; }

    inline ICallInfo* GetCallInfo(IN IMS_SINT32 /*nSlotId*/) override { return &m_objCallInfo; }
    inline ILocationInfo* GetLocationInfo(IN IMS_SINT32 /*nSlotId*/) override
    {
        return &m_objLocationInfo;
    }
    inline ISubscriberInfo* GetSubscriberInfo(IN IMS_SINT32 /*nSlotId*/) override
    {
        return &m_objSubscriberInfo;
    }

    inline IIsim* GetIsim(IN IMS_SINT32 /*nSlotId*/) override { return &m_objIsim; }
    inline IUsim* GetUsim(IN IMS_SINT32 /*nSlotId*/) override { return &m_objUsim; }

    inline INetworkWatcher* GetNetworkWatcher(IN IMS_SINT32 /*nSlotId*/) override
    {
        return &m_objNetworkWatcher;
    }
    inline IWifiWatcher* GetWifiWatcher() override { return &m_objWifiWatcher; }

    inline MockIDeviceInfo& GetMockDeviceInfo() { return m_objDeviceInfo; }
    inline MockIPowerInfo& GetMockPowerInfo() { return m_objPowerInfo; }
    inline MockICallInfo& GetMockCallInfo() { return m_objCallInfo; }
    inline MockILocationInfo& GetMockLocationInfo() { return m_objLocationInfo; }
    inline MockISubscriberInfo& GetMockSubscriberInfo() { return m_objSubscriberInfo; }
    inline MockIIsim& GetMockIsim() { return m_objIsim; }
    inline MockIUsim& GetMockUsim() { return m_objUsim; }
    inline MockINetworkWatcher& GetMockNetworkWatcher() { return m_objNetworkWatcher; }
    inline MockIWifiWatcher& GetMockWifiWatcher() { return m_objWifiWatcher; }

private:
    MockIDeviceInfo m_objDeviceInfo;
    MockIPowerInfo m_objPowerInfo;
    MockICallInfo m_objCallInfo;
    MockILocationInfo m_objLocationInfo;
    MockISubscriberInfo m_objSubscriberInfo;
    MockIIsim m_objIsim;
    MockIUsim m_objUsim;
    MockINetworkWatcher m_objNetworkWatcher;
    MockIWifiWatcher m_objWifiWatcher;
};

#endif
