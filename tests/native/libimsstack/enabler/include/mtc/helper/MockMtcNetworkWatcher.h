/**
 * Copyright (C) 2024 The Android Open Source Project
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

#ifndef MOCK_MTC_NETWORK_WATCHER_H_
#define MOCK_MTC_NETWORK_WATCHER_H_

#include "ImsTypeDef.h"
#include "helper/MtcNetworkWatcher.h"
#include <gmock/gmock.h>

class IMtcNetworkWatcherListener;
class INetworkWatcher;

class MockMtcNetworkWatcher : public MtcNetworkWatcher
{
public:
    explicit MockMtcNetworkWatcher(IN IMtcService& objService, IN IMS_SINT32 nSlotId) :
            MtcNetworkWatcher(objService, nSlotId)
    {
    }
    ~MockMtcNetworkWatcher() override {}

    MOCK_METHOD(void, AddListener, (IN IMtcNetworkWatcherListener&), (override));
    MOCK_METHOD(void, RemoveListener, (IN IMtcNetworkWatcherListener&), (override));
    MOCK_METHOD(IMS_SINT32, GetRatType, (), (const, override));
    MOCK_METHOD(IMS_SINT32, GetMobileRatType, (), (const, override));
    MOCK_METHOD(IMS_SINT32, GetLastConnectedRatType, (), (const, override));
    MOCK_METHOD(void, OnConnected, (IN IMS_UINT32), (override));
    MOCK_METHOD(void, OnDisconnected, (), (override));
    MOCK_METHOD(void, UpdateMobileRat, (IN IMS_SINT32), (override));
    MOCK_METHOD(void, NetworkWatcher_NotifyStatus, (IN INetworkWatcher*), (override));
};

#endif
