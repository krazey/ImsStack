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
#ifndef MOCK_I_NETWORK_WATCHER_H_
#define MOCK_I_NETWORK_WATCHER_H_

#include <gmock/gmock.h>

#include "ImsList.h"
#include "ImsMessageDef.h"
#include "ServiceMessage.h"
#include "ServiceNetworkPolicy.h"
#include "ServiceThread.h"

#include "INetworkWatcher.h"

class MockINetworkWatcher : public INetworkWatcher
{
public:
    inline MockINetworkWatcher() {}
    inline virtual ~MockINetworkWatcher() {}

    MOCK_METHOD(IMS_UINT32, GetNetworkStatus, (IN const AString& strProfile), (override));
    MOCK_METHOD(NETRADIO_ENTYPE, GetNetRadioTechType,
            (IN const AString& strProfile, IN IMS_SINT32 nApnType), (override));
    MOCK_METHOD(NETRADIO_ENTYPE, GetNetRadioTechType, (), (override));
    MOCK_METHOD(NETRADIO_ENTYPE, GetNetVoiceRadioTechType, (), (override));
    MOCK_METHOD(NETSERVICE_ENTYPE, GetNetServiceType,
            (IN const AString& strProfile, IN IMS_SINT32 nApnType), (override));
    MOCK_METHOD(NETSERVICE_ENTYPE, GetNetServiceType, (), (override));
    MOCK_METHOD(NETSERVICE_ENTYPE, GetNetVoiceServiceType, (), (override));
    MOCK_METHOD(NETDOMAIN_ENTYPE, GetNetDomainType, (), (override));
    MOCK_METHOD(IMS_SINT32, GetNetworkType, (), (override));
    MOCK_METHOD(IMS_SINT32, GetRoamingState, (), (override));
    MOCK_METHOD(IMS_SINT32, GetVoiceRoamingType, (), (override));
    MOCK_METHOD(IMS_SINT32, GetDataRoamingType, (), (override));
    MOCK_METHOD(IMS_BOOL, IsImsEmergencyCallSupported, (), (override));
    MOCK_METHOD(IMS_BOOL, IsImsVoiceCallSupported, (), (override));
    MOCK_METHOD(IMS_BOOL, IsEmergencyOnly, (), (override));
    MOCK_METHOD(IMS_BOOL, IsEmergencyAttachSupported, (), (override));
    MOCK_METHOD(IMS_SINT32, GetMocnPlmnInfo, (), (override));
};

class MockINetworkWatcherListener : public INetworkWatcherListener
{
public:
    inline MockINetworkWatcherListener() {}
    inline virtual ~MockINetworkWatcherListener() {}

    MOCK_METHOD(
            void, NetworkWatcher_NotifyStatus, (IN INetworkWatcher * piNetworkWatcher), (override));
};

#endif
