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
    inline TestPhoneInfoService() :
            PhoneInfoService(),
            m_piDeviceInfo(&m_objDeviceInfo),
            m_piPowerInfo(&m_objPowerInfo),
            m_piCallInfo(&m_objCallInfo),
            m_piLocationInfo(&m_objLocationInfo),
            m_piSubscriberInfo(&m_objSubscriberInfo),
            m_piIsim(&m_objIsim),
            m_piUsim(&m_objUsim),
            m_piNetworkWatcher(&m_objNetworkWatcher),
            m_piWifiWatcher(&m_objWifiWatcher)
    {
    }

    inline IDeviceInfo* GetDeviceInfo() override { return m_piDeviceInfo; }
    inline IPowerInfo* GetPowerInfo() override { return m_piPowerInfo; }

    inline ICallInfo* GetCallInfo(IN IMS_SINT32 /*nSlotId*/) override { return m_piCallInfo; }
    inline ILocationInfo* GetLocationInfo(IN IMS_SINT32 /*nSlotId*/) override
    {
        return m_piLocationInfo;
    }
    inline ISubscriberInfo* GetSubscriberInfo(IN IMS_SINT32 /*nSlotId*/) override
    {
        return m_piSubscriberInfo;
    }

    inline IIsim* GetIsim(IN IMS_SINT32 /*nSlotId*/) override { return m_piIsim; }
    inline IUsim* GetUsim(IN IMS_SINT32 /*nSlotId*/) override { return m_piUsim; }

    inline INetworkWatcher* GetNetworkWatcher(IN IMS_SINT32 /*nSlotId*/) override
    {
        return m_piNetworkWatcher;
    }
    inline IWifiWatcher* GetWifiWatcher() override { return m_piWifiWatcher; }

    inline MockIDeviceInfo& GetMockDeviceInfo() { return m_objDeviceInfo; }
    inline MockIPowerInfo& GetMockPowerInfo() { return m_objPowerInfo; }
    inline MockICallInfo& GetMockCallInfo() { return m_objCallInfo; }
    inline MockILocationInfo& GetMockLocationInfo() { return m_objLocationInfo; }
    inline MockISubscriberInfo& GetMockSubscriberInfo() { return m_objSubscriberInfo; }
    inline MockIIsim& GetMockIsim() { return m_objIsim; }
    inline MockIUsim& GetMockUsim() { return m_objUsim; }
    inline MockINetworkWatcher& GetMockNetworkWatcher() { return m_objNetworkWatcher; }
    inline MockIWifiWatcher& GetMockWifiWatcher() { return m_objWifiWatcher; }
    inline void SetDeviceInfo(IN IDeviceInfo* piDeviceInfo) { m_piDeviceInfo = piDeviceInfo; }
    inline void SetPowerInfo(IN IPowerInfo* piPowerInfo) { m_piPowerInfo = piPowerInfo; }
    inline void SetCallInfo(IN ICallInfo* piCallInfo) { m_piCallInfo = piCallInfo; }
    inline void SetLocationInfo(IN ILocationInfo* piLocationInfo)
    {
        m_piLocationInfo = piLocationInfo;
    }
    inline void SetSubscriberInfo(IN ISubscriberInfo* piSubscriberInfo)
    {
        m_piSubscriberInfo = piSubscriberInfo;
    }
    inline void SetIsim(IN IIsim* piIsim) { m_piIsim = piIsim; }
    inline void SetUsim(IN IUsim* piUsim) { m_piUsim = piUsim; }
    inline void SetNetworkWatcher(IN INetworkWatcher* piNetworkWatcher)
    {
        m_piNetworkWatcher = piNetworkWatcher;
    }
    inline void SetWifiWatcher(IN IWifiWatcher* piWifiWatcher) { m_piWifiWatcher = piWifiWatcher; }

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

    IDeviceInfo* m_piDeviceInfo;
    IPowerInfo* m_piPowerInfo;
    ICallInfo* m_piCallInfo;
    ILocationInfo* m_piLocationInfo;
    ISubscriberInfo* m_piSubscriberInfo;
    IIsim* m_piIsim;
    IUsim* m_piUsim;
    INetworkWatcher* m_piNetworkWatcher;
    IWifiWatcher* m_piWifiWatcher;
};

#endif
