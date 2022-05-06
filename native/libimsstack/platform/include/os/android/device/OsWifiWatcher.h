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
#ifndef OS_WIFI_WATCHER_H_
#define OS_WIFI_WATCHER_H_

#include "IWifiWatcher.h"
#include "system-intf/ISystemListener.h"

class System;

class OsWifiWatcher : public IWifiWatcher, public ISystemListener
{
public:
    OsWifiWatcher();
    virtual ~OsWifiWatcher();

    OsWifiWatcher(IN const OsWifiWatcher&) = delete;
    OsWifiWatcher& operator=(IN const OsWifiWatcher&) = delete;

public:
    // IWifiWatcher
    IMS_SINT32 GetState() override;

    // ISystemListener
    void System_NotifyEvent(
            IN IMS_UINT32 nEvent, IN IMS_UINTP nWParam, IN IMS_UINTP nLParam) override;

private:
    void UpdateWifiStateChanged(IN IMS_UINT32 nState);
    void UpdateWifiNetworkStateChanged(IN IMS_UINT32 nDetailedState);

private:
    IMS_SINT32 m_nWifiState;
};

#endif
