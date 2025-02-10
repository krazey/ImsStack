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
#ifndef INTERFACE_IMS_AOS_INFO_H_
#define INTERFACE_IMS_AOS_INFO_H_

#include "AString.h"

/**
 * @brief This class provides an interface to get and notify the information for AoS.
 *
 * The IImsAosInfo is the interface between service enabler and AoS enabler. \n
 * It provides the APIs to get and notify the detailed information.
 *
 * @see IImsAos
 */

class IImsAosInfo
{
public:
    virtual ~IImsAosInfo(){};

    /**
     * @brief Get the associated URI that is the topmost one and authorized and registered.
     *
     * @return The network provisioned user identity.
     */
    virtual AString GetAssociatedUri() = 0;

    /**
     * @brief Get the connection type that is used for IMS registration.
     *
     * @return Connection type for IMS registration. (e.g. NetworkPolicy::APN_IMS)
     */
    virtual IMS_SINT32 GetConnectionType() = 0;

    /**
     * @brief Get the features that are registered for all IMS service
     *
     * @return Indicated the fetures that are registered for all IMS service
     *         (@see ImsAosFeature)
     */
    virtual IMS_UINT32 GetImsFeatures() = 0;

    /**
     * @brief Get the AoS state.
     *
     * @return The enum for Aos state (e.g. IMS_STATE_AVAILABLE, ...)
     */
    virtual IMS_UINT32 GetImsState() = 0;

    /**
     * @brief Get the IPCAN type.
     *
     * @return IPCAN type for IMS (e.g. IIpcan::CATEGORY_MOBILE, IIpcan::CATEGORY_WLAN)
     */
    virtual IMS_SINT32 GetIpcanType() = 0;

    /**
     * @brief Get the last value of Path header in 200 OK response for IMS registration.
     *
     * @return The last value in Path header.
     */
    virtual AString GetLastPathHeaderValue() = 0;

    /**
     * @brief Get the local address that is used for IMS registration.
     *
     * @return Local address for IMS registration.
     */
    virtual AString GetLocalAddress() = 0;

    /**
     * @brief Get the local port that is used for IMS registration.
     *
     * @return Local port for IMS registration.
     */
    virtual IMS_UINT32 GetLocalPort() = 0;

    /**
     * @brief Get the network type where IMS is registered.
     *
     * @return Network type registered. (e.g. NW_REPORT_RADIO_LTE, NW_REPORT_RADIO_WLAN ...
     *         defined as NETRADIO_ENTYPE)
     */
    virtual IMS_UINT32 GetRegisteredNetworkType() = 0;

    /**
     * @brief Get the value of Path header in 200 OK response for IMS registration.
     *
     * @return The topmost value in Path header.
     */
    virtual AString GetPathHeaderValue() = 0;

    /**
     * @brief Get the PCSCF address that is used for IMS registration.
     *
     * @return PCSCF address for IMS registration.
     */
    virtual AString GetPcscfAddress() = 0;

    /**
     * @brief Get the PCSCF port that is used for IMS registration.
     *
     * @return PCSCF port for IMS registration.
     */
    virtual IMS_UINT32 GetPcscfPort() = 0;

    /**
     * @brief Get the registration mode when the service is connected.
     *
     * @return The enum for registration mode (e.g. REG_MODE_NORMAL, ...)
     */
    virtual IMS_UINT32 GetRegistrationMode() = 0;

    /**
     * @brief Get the value of Supported header in 200 OK response for IMS registration.
     *
     * @return The value in Supported header.
     */
    virtual AString GetSupportedHeaderValue() = 0;

    /**
     * @brief Get the value of Service-Route header in 200 OK response for IMS registration.
     *
     * @return The topmost value in Service-Route header.
     */
    virtual AString GetServiceRouteHeaderValue() = 0;

    /**
     * @brief Notify the emergency call state.
     *
     * @param bIsInitialized Indicated whether emergency call is initialized or done.
     */
    virtual void NotifyEmergencyCallState(IN IMS_BOOL bIsInitialized) = 0;

    /**
     * @brief Set the publish state for waiting the un-publish procedure when IMS is de-registering.
     *
     * @param bIsStarted Indicated whether publish is started or terminated.
     */
    virtual void NotifyPublishState(IN IMS_BOOL bIsStarted) = 0;

    /**
     * @brief Notify the emergency sms state.
     *
     * @param bIsInitialized Indicated whether emergency sms is initialized or done.
     */
    virtual void NotifyEmergencySmsState(IN IMS_BOOL bIsInitialized) = 0;

    /**
     * @brief Notify the call state to be EPS fallback.
     *
     * @param nState Indicated the call state to be EPS fallback.
     *               Possible values are,
     *               IImsAosInfo::EPSFB_CALL_START
     *               IImsAosInfo::EPSFB_CALL_FAILED
     */
    virtual void NotifyEpsfbCallState(IN IMS_UINT32 nState) = 0;

public:
    enum
    {
        /// not specified registration mode
        REG_MODE_UNKNOWN = 0,
        /// normal registration mode
        REG_MODE_NORMAL,
        /// registered with the IMSI based IMPU
        REG_MODE_ADMIN,
        /// fake registration except for REG_MODE_NOUICC
        REG_MODE_INTERNAL,
        /// fake registration with no UICC cause
        REG_MODE_NOUICC
    };

    enum
    {
        /// registered state
        IMS_STATE_AVAILABLE = 0,
        /// not registered state with the block reason except for forbidden or unsubscribed
        IMS_STATE_UNAVAILABLE,
        /// the state that data is connecting or registration is trying
        IMS_STATE_PENDING,
        /// registration is blocked with the forbidden cause
        IMS_STATE_FORBIDDEN,
        /// registration is blocked with the unsubscribed cause
        IMS_STATE_UNSUBSCRIBED
    };

    enum
    {
        /// The registration is destroyed and blocked until LTE is attached
        EPSFB_CALL_START = 1,
        /// The registration block for EPS_FB is reset. It's invoked when registration is
        /// not completed and call is handled as failure
        EPSFB_CALL_FAILED = 2
    };
};

#endif  // INTERFACE_IMS_AOS_INFO_H_
