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
#ifndef INTERFACE_IMS_AOS_MONITOR_H_
#define INTERFACE_IMS_AOS_MONITOR_H_

#include "ImsTypeDef.h"

/**
 * @brief This class provides an monitor interface for AoS.
 *
 * The IImsAosMonitor is the monitor interface between service enabler and AoS enabler. \n
 * It provides the APIs to notify the AoS status related to the registration
 * for the every service and the specific information
 *
 * @see IImsAos, ImsAosParameter.h
 */

class IImsAosMonitor
{
public:
    virtual ~IImsAosMonitor(){};
    /**
     * @brief Indicate the services that are registered with features over IPCAN type.
     *
     * @param nServices Indicate the services. (@see ImsAosService)
     * @param nIpcan Indicate the IP connectivity access network where IMS is registered. \n
     *                  - IIpcan::CATEGORY_MOBILE \n
     *                  - IIpcan::CATEGORY_WLAN
     */
    virtual void ImsAosMonitor_Connected(IN IMS_UINT32 nServices, IN IMS_UINT32 nIpcan) = 0;

    /**
     * @brief Notify the specific information that is required for any enabler.
     *
     * @param nType Indicate the type for the specific information. \n
     *                  - IImsAosMonitor::TYPE_HANDOVER \n
     *                  - IImsAosMonitor::TYPE_IPCAN \n
     *                  - IImsAosMonitor::TYPE_CROSS_SIM_STATUS
     * @param nState Indicate the detailed state for the designated type.
     */
    virtual void ImsAosMonitor_Notify(IN IMS_UINT32 nType, IN IMS_UINT32 nState) = 0;

    /// Types for notification
    enum
    {
        /// Handover notification from WIFI to Cellular like LTE or NR
        /// @param nState HANDOVER_WIFI_TO_CELLULAR_START, HANDOVER_WIFI_TO_CELLULAR_COMPLETED
        TYPE_HANDOVER = 1,

        /// IP-CAN type notifiction between mobile and wlan
        /// @param nState IIpcan::CATEGORY_MOBILE, IIpcan::CATEGORY_WLAN
        TYPE_IPCAN = 2,

        /// Cross SIM connection status change notification
        /// @param nState CROSS_SIM_DISCONNECTED, CROSS_SIM_CONNECTED
        TYPE_CROSS_SIM_STATUS = 3,

        /**
         * When an initial registration is required for any of the following reasons,
         * the registration recovery procedure is pended if an IMS call is ongoing.
         * The procedure will be initiated immediately after the call is released.
         *
         * Reasons: P-CSCF change or invocation of ImsAosControl::REGISTER_REINITIATE
         * / ImsAosControl::PCSCF_NEXT via enablers(MTC, etc).
         *
         * @param nState N/A
         */
        TYPE_REG_RECOVERY_PENDING = 4
    };

    /// Detailed state for the handover type
    /// Handover is started from WIFI to Cellular
    static const IMS_UINT32 HANDOVER_WIFI_TO_CELLULAR_START = 100;
    /// Handover is completed from WIFI to Cellular
    static const IMS_UINT32 HANDOVER_WIFI_TO_CELLULAR_COMPLETED = 101;

    /// Detailed state for the cross SIM status type
    /// Cross SIM is not used for IMS Service.
    static const IMS_UINT32 CROSS_SIM_DISCONNECTED = 0;
    /// Cross SIM is used for IMS Service
    static const IMS_UINT32 CROSS_SIM_CONNECTED = 1;
};

#endif  // INTERFACE_IMS_AOS_MONITOR_H_
