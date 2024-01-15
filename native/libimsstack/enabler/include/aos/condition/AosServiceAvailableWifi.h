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
#ifndef AOS_SERVICE_AVAILABLE_WIFI_H_
#define AOS_SERVICE_AVAILABLE_WIFI_H_

#include "INetworkPingListener.h"
#include "IWifiWatcher.h"
#include "condition/AosServiceAvailable.h"

class ILocationProperties;

class AosServiceAvailableWifi :
        public AosServiceAvailable,
        public INetworkPingListener,
        public IWifiWatcherListener
{
public:
    AosServiceAvailableWifi();
    virtual ~AosServiceAvailableWifi();

    IMS_BOOL StartToCheckNetworkConnection() final;
    IMS_BOOL StopToCheckNetworkConnection(IN IMS_BOOL bNeedToCheckAvailable = IMS_TRUE) final;

    void SetLocation(IN ILocationProperties* piTestLocation);

    enum
    {
        STATE_BAD_NETWORK_NONE = 0,
        STATE_BAD_NETWORK_CHECKING,
        STATE_BAD_NETWORK_DETECTED
    };

private:
    void RegisterListener() final;
    void DeregisterListener() final;

    // IWifiWatcherListener
    void WifiWatcher_NotifyStateChanged(IN IWifiWatcher* pIWifiWatcher) final;

    // INetworkPingListener
    void NetworkPing_NotifyResult(IN INetworkPing* piPing, IN IMS_SINT32 nResult) final;

    void HandleCallStateChanged(IN IMS_UINT32 nState, IN IMS_SINT32 nStateEx) final;
    void HandleAirplaneModeChanged(IN IMS_UINT32 nState) final;
    void HandleWifiConnectionChanged() final;
    void HandleLocationInfoChanged() final;

    IMS_BOOL CheckServiceAvailable() final;

    void ProcessBadConnectionReported();
    void ClearBadNetworkState();

    IMS_SINT32 RequestNetPing();
    static const IMS_CHAR* PingResultToString(IN IMS_SINT32 nResult);

private:
    AString m_strCountry;
    IMS_UINT32 m_nBadNetworkState;
    IMS_BOOL m_bWifiState;
    INetworkPing* m_piNetPing;

    static const IMS_UINT32 TIME_BAD_NETWORK_CHECK = 3000;

private:
    // Use only for Unit test
    ILocationProperties* m_piTestLocation;

    friend class AosServiceAvailableWifiTest;
    friend class AosConditionTest;
};

#endif  // AOS_SERVICE_AVAILABLE_WIFI_H_
