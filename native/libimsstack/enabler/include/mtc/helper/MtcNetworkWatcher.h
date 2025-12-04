
/*
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

#ifndef MTC_NETWORK_WATCHER_H_
#define MTC_NETWORK_WATCHER_H_

#include "IMtcService.h"
#include "INetworkWatcher.h"

/**
 * This class checks the service's network status and notifies the listener if there's a change.
 */
class MtcNetworkWatcher : public INetworkWatcherListener
{
public:
    explicit MtcNetworkWatcher(IN IMtcService& objService, IN IMS_SINT32 nSlotId);
    MtcNetworkWatcher(const MtcNetworkWatcher&) = delete;
    MtcNetworkWatcher& operator=(const MtcNetworkWatcher&) = delete;
    virtual ~MtcNetworkWatcher() override;

    virtual void AddListener(IN IMtcNetworkWatcherListener& objListener);
    virtual void RemoveListener(IN IN IMtcNetworkWatcherListener& objListener);

    /**
     * @brief Returns the current RAT type.
     *
     * One of the following is returned.
     * - {@code INetworkWatcher::RADIOTECH_TYPE_NR}
     * - {@code INetworkWatcher::RADIOTECH_TYPE_LTE}
     * - {@code INetworkWatcher::RADIOTECH_TYPE_IWLAN}
     * - {@code INetworkWatcher::RADIOTECH_TYPE_UNKNOWN}: Other RATs
     *
     * @return Current RAT type.
     *         {@code INetworkWatcher::RADIOTECH_TYPE_INVALID} if the service is disconnected.
     */
    virtual IMS_SINT32 GetRatType() const;

    /**
     * @brief Returns the current RAT type regardless of the current IP-CAN type.
     *
     * One of the following is returned.
     * - {@code INetworkWatcher::RADIOTECH_TYPE_NR}
     * - {@code INetworkWatcher::RADIOTECH_TYPE_LTE}
     * - {@code INetworkWatcher::RADIOTECH_TYPE_UNKNOWN}: Other mobile RATs

     * @return Current mobile RAT type.
     *         {@code INetworkWatcher::RADIOTECH_TYPE_INVALID} if the service is disconnected.
     */
    virtual IMS_SINT32 GetMobileRatType() const;

    /**
     * @brief Returns the last connected RAT type.
     *
     * One of the following is returned.
     * - {@code INetworkWatcher::RADIOTECH_TYPE_NR}
     * - {@code INetworkWatcher::RADIOTECH_TYPE_LTE}
     * - {@code INetworkWatcher::RADIOTECH_TYPE_IWLAN}
     * - {@code INetworkWatcher::RADIOTECH_TYPE_UNKNOWN}: Other RATs
     *
     * @return Last connected RAT type.
     *         Never be {@code INetworkWatcher::RADIOTECH_TYPE_INVALID} if connected once.
     */
    virtual IMS_SINT32 GetLastConnectedRatType() const { return m_eLastConnectedRatType; }

    /**
     * @brief Updates internal states when the service is connected and notifies listeners
     *        if there's a change.
     *
     * @param eIpcan Connected IP-CAN. See {@code IIpcan::CATEGORY_*}.
     */
    virtual void OnConnected(IN IMS_UINT32 eIpcan);

    /**
     * @brief Updates internal states when the service is disconnected and notifies listeners
     *        {@code INetworkWatcher::RADIOTECH_TYPE_INVALID} if there's a change.
     */
    virtual void OnDisconnected();

    /**
     * @brief Updates it's internal RAT type and notifies listeners if there's a change and
     *        not using IWLAN.
     *
     * Visible for testing.
     */
    virtual void UpdateMobileRat(IN IMS_SINT32 eRatType);

    void NetworkWatcher_NotifyStatus(IN INetworkWatcher* piNetWatcherInfo) override;

private:
    void NotifyIfChanged();
    static IMS_SINT32 ConvertCellularRatType(IN const NETRADIO_ENTYPE eRatType);

    ServiceType m_eServiceType;
    IMS_BOOL m_bServiceConnected;
    INetworkWatcher* m_piNetWatcher;
    IMS_UINT32 m_eIpcanType;
    /** Mobile RAT information regardless of service connection status. */
    IMS_SINT32 m_eMobileRatType;
    /** RAT type before receiving an last RAT changing event. */
    IMS_SINT32 m_eOldRatType;
    IMS_SINT32 m_eLastConnectedRatType;

    ImsList<IMtcNetworkWatcherListener*> m_objListeners;
};

#endif
