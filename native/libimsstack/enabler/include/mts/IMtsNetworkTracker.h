/*
 * Copyright (C) 2025 The Android Open Source Project
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

#ifndef INTERFACE_MTS_NETWORK_TRACKER_H_
#define INTERFACE_MTS_NETWORK_TRACKER_H_

class IMtsNetworkTracker
{
public:
    virtual ~IMtsNetworkTracker() {}

    /**
     * @brief Gets the service state of the current device's cellular data network.
     *
     * @return The cellular data network type:
     * - {@code STATE_IN_SERVICE}: Properly connected to the network and able to communicate.
     * - {@code STATE_OUT_OF_SERVICE}: Out of network service area or unable to use service.
     * - {@code STATE_EMERGENCY_ONLY}: Can only make emergency calls, but no regular service.
     * - {@code STATE_POWER_OFF}: Modem is powered off, preventing network service usage.
     */
    virtual IMS_SINT32 GetCellularServiceState() const = 0;

    /**
     * @brief Gets the current LTE network attach state.
     *
     * @return The current LTE network attach state. One of the following is returned:
     * - {@code IMS_LTE_INFO_UNKNOWN}: LTE network state is unknown.
     * - {@code IMS_LTE_INFO_EPS_ONLY_ATTACHED}: Attached to LTE network with EPS only.
     * - {@code IMS_LTE_INFO_COMBINED_ATTACHED}: Attached to LTE network with combined.
     */
    virtual IMS_UINT32 GetLteAttachState() const = 0;

    /**
     * @brief Gets the current data network type.
     *
     * @return The current data network type directly from TelephonyManager.
     *         {@code INetworkWatcher::RADIOTECH_TYPE_INVALID} If the network watcher service is
     *         unavailable.
     */
    virtual IMS_SINT32 GetNetworkType() const = 0;

    /**
     * @brief Checks if the device is currently connected to a roaming network.
     *
     * @return IMS_TRUE if device is in roaming state, IMS_FALSE otherwise.
     */
    virtual IMS_BOOL IsInRoamingState() const = 0;
};

#endif
