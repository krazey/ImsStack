/*
 * Copyright (C) 2023 The Android Open Source Project
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
#ifndef INTERFACE_IMS_TRAFFIC_H_
#define INTERFACE_IMS_TRAFFIC_H_

#include "ImsTypeDef.h"

class IImsTrafficListener
{
protected:
    virtual ~IImsTrafficListener() = default;

public:
    /**
     * @brief Notifies the priority change  of IMS traffic type for the supported slots.
     */
    virtual void ImsTraffic_OnPriorityChanged() = 0;
};

class IImsTraffic
{
protected:
    virtual ~IImsTraffic() = default;

public:
    /**
     * @brief Disables the priority handling for the IMS traffic based on slot ID.
     *
     * @param nSlotId The slot ID
     */
    virtual void Disable(IN IMS_SINT32 nSlotId) = 0;

    /**
     * @brief Indicates whether Ims traffic is available or not after checking the traffic priority
     *        within the IMS stack for DSDS in advance. There is no interworking with modem.
     *
     * @param nSlotId The slot ID
     * @param nTrafficType The type for IMS traffic (@IImsRadio::ImsTrafficType)
     *
     * @return Returns the IMS traffic availability.
     */
    virtual IMS_BOOL IsAllowed(IN IMS_SINT32 nSlotId, IN IMS_UINT32 nTrafficType) = 0;

    /**
     * @brief Starts the IMS traffic based on slot ID.
     *
     * @param nSlotId The slot ID
     * @param nTrafficType The type for IMS traffic (@IImsRadio::ImsTrafficType)
     */
    virtual void Start(IN IMS_SINT32 nSlotId, IN IMS_UINT32 nTrafficType) = 0;

    /**
     * @brief Stops the IMS traffic based on slot ID.
     *
     * @param nSlotId The slot ID
     * @param nTrafficType The type for IMS traffic (@IImsRadio::ImsTrafficType)
     */
    virtual void Stop(IN IMS_SINT32 nSlotId, IN IMS_UINT32 nTrafficType) = 0;

    /**
     * @brief Sets modem's simultaneous calling support for a specified slot.
     *
     * @param nSlotId The slot ID.
     * @param bSupported The information whether modem supports simultaneous calling.
     */
    virtual void SetSimultaneousCallingSupported(IN IMS_SINT32 nSlotId, IN IMS_BOOL bSupported) = 0;

    /**
     * @brief Sets WLAN category for IPCAN.
     *
     * @param nSlotId The slot ID
     * @param bEnabled The information if IPCAN is WLAN or mobile
     */
    virtual void SetWlan(IN IMS_SINT32 nSlotId, IN IMS_BOOL bEnabled) = 0;

    /**
     * @brief Adds the listener to be notified for IMS traffic.
     *
     * @param piListener The listener to be added
     */
    virtual void AddListener(IN IImsTrafficListener* piListener) = 0;

    /**
     * @brief Removes the listener to be notified for IMS traffic.
     *
     * @param piListener The listener to be removed
     */
    virtual void RemoveListener(IN IImsTrafficListener* piListener) = 0;

    /**
     * @brief Notifies the IMS traffic related message.
     *
     * @param nWparam The internal event
     * @param nLparam The data object of the event
     */
    virtual void DispatchServiceMessage(IN IMS_UINTP nWparam, IN IMS_UINTP nLparam) = 0;

    enum
    {
        TRAFFIC_PRIORITY_NONE = 0x0,
        TRAFFIC_PRIORITY_REGISTRATION = 0x1,
        TRAFFIC_PRIORITY_SMS = 0x2,
        TRAFFIC_PRIORITY_VIDEO = 0x4,
        TRAFFIC_PRIORITY_VOICE = 0x8,
        TRAFFIC_PRIORITY_EMERGENCY_SMS = 0x10,
        TRAFFIC_PRIORITY_EMERGENCY = 0x20
    };
};

#endif
