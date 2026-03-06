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

#ifndef INTERFACE_MTC_AOS_CONNECTOR_H_
#define INTERFACE_MTC_AOS_CONNECTOR_H_

#include "AString.h"
#include "ImsTypeDef.h"

/**
 * @brief Interface for connecting to the AoS (Always on Service) enabler.
 *
 * This interface serves as a wrapper around {@link IImsAos} and {@link IImsAosInfo}, providing
 * a unified access point to interact with the IMS registration layer. It allows querying
 * registration status, controlling registration procedures, and retrieving network information.
 */
class IMtcAosConnector
{
public:
    virtual ~IMtcAosConnector() {}

    // IImsAos interface wrappers.

    /**
     * @brief Gets the features that are registered for the called service.
     *
     * @return A bitmask indicating the features contained in the 200 OK response for registration.
     *         See {@link ImsAosFeature} for possible values.
     */
    virtual IMS_UINT32 GetFeatures() const = 0;

    /**
     * @brief Gets the reason why the service is suspended.
     *
     * @return The suspended reason code. (e.g., {@link ImsAosReason#AIRPLANE_MODE}, {@link
     * ImsAosReason#REG_ALL_PCSCF_FAILED})
     */
    virtual IMS_UINT32 GetSuspendedReason() const = 0;

    /**
     * @brief Checks if a specific feature is currently registered.
     *
     * @param nFeature The feature to check (e.g., {@link ImsAosFeature#MMTEL}).
     * @return IMS_TRUE if the feature is registered, IMS_FALSE otherwise.
     */
    virtual IMS_BOOL IsFeatureConnected(IN IMS_UINT32 nFeature) const = 0;

    /**
     * @brief Checks if the IMS service is connected.
     *
     * @return IMS_TRUE if the service is connected, IMS_FALSE otherwise.
     */
    virtual IMS_BOOL IsImsConnected() const = 0;

    /**
     * @brief Checks if the IMS service is suspended.
     *
     * Registration might be maintained even if the service is suspended.
     *
     * @return IMS_TRUE if the service is suspended, IMS_FALSE otherwise.
     */
    virtual IMS_BOOL IsImsSuspended() const = 0;

    /**
     * @brief Sets the readiness of the service to receive enabler information.
     *
     * This registers the listener to get the enabler information.
     *
     * @param bReady IMS_TRUE if the enabler service is started and ready, IMS_FALSE otherwise.
     * @param nService The service type (e.g., {@link ImsAosService#MTC}).
     */
    virtual void SetReady(IN IMS_BOOL bReady, IN IMS_UINT32 nService) const = 0;

    /**
     * @brief Updates the features required for registration.
     *
     * If the features are changed and registration is already done, a re-registration
     * is triggered with the updated features.
     *
     * @param nFeatures A bitmask of features to register (e.g., {@link ImsAosFeature}).
     */
    virtual void UpdateFeature(IN IMS_UINT32 nFeatures) const = 0;

    /**
     * @brief Controls the IMS registration operation.
     *
     * @param nType The control type (e.g., {@link ImsAosControl::REGISTER_START}).
     * @return IMS_TRUE if the requested operation is available/accepted, IMS_FALSE otherwise.
     */
    virtual IMS_BOOL Control(IN IMS_UINT32 nType) const = 0;

    // IImsAosInfo interface wrappers.

    /**
     * @brief Gets the associated URI.
     *
     * This returns the topmost, authorized, and registered URI from the network provisioned
     * user identity.
     *
     * @return The associated URI string.
     */
    virtual AString GetAssociatedUri() const = 0;

    /**
     * @brief Gets the connection type used for IMS registration.
     *
     * @return The connection type (e.g., {@link NetworkPolicy#APN_IMS}).
     */
    virtual IMS_UINT32 GetConnectionType() const = 0;

    /**
     * @brief Gets the current IMS state.
     *
     * @return The IMS state (e.g., {@link IImsAosInfo#IMS_STATE_AVAILABLE}).
     */
    virtual IMS_UINT32 GetImsState() const = 0;

    /**
     * @brief Gets the IP-CAN (IP Connectivity Access Network) type.
     *
     * @return The IP-CAN type (e.g., {@link IIpcan#CATEGORY_MOBILE}, {@link IIpcan#CATEGORY_WLAN}).
     */
    virtual IMS_UINT32 GetIpcanType() const = 0;

    /**
     * @brief Gets the last value of the Path header from the 200 OK response for IMS registration.
     *
     * @return The last Path header value.
     */
    virtual AString GetLastPathHeaderValue() const = 0;

    /**
     * @brief Gets the local IP address used for IMS registration.
     *
     * @return The local IP address string.
     */
    virtual AString GetLocalAddress() const = 0;

    /**
     * @brief Gets the local port used for IMS registration.
     *
     * @return The local port number.
     */
    virtual IMS_UINT32 GetLocalPort() const = 0;

    /**
     * @brief Gets the network type where IMS is registered.
     *
     * @return The registered network type (e.g., {@link NW_REPORT_RADIO_LTE},
     *         {@link NW_REPORT_RADIO_WLAN}).
     */
    virtual IMS_UINT32 GetRegisteredNetworkType() const = 0;

    /**
     * @brief Gets the value of the Path header from the 200 OK response for IMS registration.
     *
     * @return The topmost Path header value.
     */
    virtual AString GetPathHeaderValue() const = 0;

    /**
     * @brief Gets the P-CSCF address used for IMS registration.
     *
     * @return The P-CSCF address string.
     */
    virtual AString GetPcscfAddress() const = 0;

    /**
     * @brief Gets the P-CSCF port used for IMS registration.
     *
     * @return The P-CSCF port number.
     */
    virtual IMS_UINT32 GetPcscfPort() const = 0;

    /**
     * @brief Gets the registration mode.
     *
     * @return The registration mode (e.g., {@link IImsAosInfo#REG_MODE_NORMAL}).
     */
    virtual IMS_UINT32 GetRegistrationMode() const = 0;

    /**
     * @brief Gets the value of the Supported header from the 200 OK response for IMS registration.
     *
     * @return The Supported header value.
     */
    virtual AString GetSupportedHeaderValue() const = 0;

    /**
     * @brief Gets the value of the Service-Route header from the 200 OK response for IMS
     * registration.
     *
     * @return The topmost Service-Route header value.
     */
    virtual AString GetServiceRouteHeaderValue() const = 0;

    /**
     * @brief Checks if Cross-SIM calling is connected.
     *
     * @return IMS_TRUE if Cross-SIM is connected, IMS_FALSE otherwise.
     */
    virtual IMS_BOOL IsCrossSimConnected() const = 0;

    /**
     * @brief Notifies the emergency call state.
     *
     * @param bIsInitialized IMS_TRUE if the emergency call is initialized, IMS_FALSE if done.
     */
    virtual void NotifyEmergencyCallState(IN IMS_BOOL bIsInitialized) const = 0;

    /**
     * @brief Notifies the EPS fallback call state.
     *
     * @param nState The EPS fallback state. Possible values:
     *               {@link IImsAosInfo#EPSFB_CALL_START},
     *               {@link IImsAosInfo#EPSFB_CALL_FAILED}.
     */
    virtual void NotifyEpsfbCallState(IN IMS_UINT32 nState) const = 0;

    /**
     * @brief Triggers registration with the next available P-CSCF.
     *
     * The current P-CSCF is marked as unavailable for the specified duration.
     * This will terminate the current registration and trigger a new registration attempt.
     * If no next P-CSCF is available, a P-CSCF discovery procedure may be triggered.
     * The callback {@link IImsAosListener#ImsAos_Disconnected} will be called with
     * {@link ImsAosReason#REG_NEW_REQUIRED}.
     *
     * @param nUnavailableTimeForCurrentPcscf The duration (in seconds) to mark the current
     *        P-CSCF as unavailable.
     */
    virtual void RegisterWithNextPcscf(IN IMS_UINT32 nUnavailableTimeForCurrentPcscf) const = 0;

    /**
     * @brief Requests AoS to destroy the current registration and trigger an initial registration
     *        with the current P-CSCF after a delay.
     *
     * If {@code nAfterSec} is 0, the behavior is equivalent to calling
     * {@code Control(ImsAosControl::REGISTER_REINITIATE)}.
     *
     * @param nAfterSec The duration (in seconds) to wait before starting the new registration.
     */
    virtual void ReinitiateRegistration(IN IMS_UINT32 nAfterSec) const = 0;
};

#endif
