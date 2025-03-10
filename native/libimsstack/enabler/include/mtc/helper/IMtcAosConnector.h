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

class IMtcAosConnector
{
public:
    virtual ~IMtcAosConnector() {}

    // IImsAos interface wrappers.

    /**
     * @brief Gets
     *
     * @return
     */
    virtual IMS_UINT32 GetFeatures() const = 0;

    /**
     * @brief Gets
     *
     * @return
     */
    virtual IMS_UINT32 GetSuspendedReason() const = 0;

    /**
     * @brief Checks
     *
     * @param nFeature
     * @return
     */
    virtual IMS_BOOL IsFeatureConnected(IN IMS_UINT32 nFeature) const = 0;

    /**
     * @brief Checks
     *
     * @return
     */
    virtual IMS_BOOL IsImsConnected() const = 0;

    /**
     * @brief Checks
     *
     * @return
     */
    virtual IMS_BOOL IsImsSuspended() const = 0;

    /**
     * @brief Sets
     *
     * @param bReady
     * @param nService
     */
    virtual void SetReady(IN IMS_BOOL bReady, IN IMS_UINT32 nService) const = 0;

    /**
     * @brief Updates
     *
     * @param nFeatures
     */
    virtual void UpdateFeature(IN IMS_UINT32 nFeatures) const = 0;

    /**
     * @brief Controls
     *
     * @param nType
     * @return
     */
    virtual IMS_BOOL Control(IN IMS_UINT32 nType) const = 0;

    // IImsAosInfo interface wrappers.

    /**
     * @brief Gets
     *
     * @return
     */
    virtual AString GetAssociatedUri() const = 0;

    /**
     * @brief Gets
     *
     * @return
     */
    virtual IMS_UINT32 GetConnectionType() const = 0;

    /**
     * @brief Gets
     *
     * @return
     */
    virtual IMS_UINT32 GetImsState() const = 0;

    /**
     * @brief Gets
     *
     * @return
     */
    virtual IMS_UINT32 GetIpcanType() const = 0;

    /**
     * @brief Gets
     *
     * @return
     */
    virtual AString GetLastPathHeaderValue() const = 0;

    /**
     * @brief Gets
     *
     * @return
     */
    virtual AString GetLocalAddress() const = 0;

    /**
     * @brief Gets
     *
     * @return
     */
    virtual IMS_UINT32 GetLocalPort() const = 0;

    /**
     * @brief Gets
     *
     * @return
     */
    virtual IMS_UINT32 GetRegisteredNetworkType() const = 0;

    /**
     * @brief Gets
     *
     * @return
     */
    virtual AString GetPathHeaderValue() const = 0;

    /**
     * @brief Gets
     *
     * @return
     */
    virtual AString GetPcscfAddress() const = 0;

    /**
     * @brief Gets
     *
     * @return
     */
    virtual IMS_UINT32 GetPcscfPort() const = 0;

    /**
     * @brief Gets
     *
     * @return
     */
    virtual IMS_UINT32 GetRegistrationMode() const = 0;

    /**
     * @brief Gets
     *
     * @return
     */
    virtual AString GetSupportedHeaderValue() const = 0;

    /**
     * @brief Gets
     *
     * @return
     */
    virtual AString GetServiceRouteHeaderValue() const = 0;

    /**
     * See {@link IImsAosInfo#IsCrossSimConnected}.
     */
    virtual IMS_BOOL IsCrossSimConnected() const = 0;

    /**
     * @brief Notifys
     *
     * @param bIsInitialized
     */
    virtual void NotifyEmergencyCallState(IN IMS_BOOL bIsInitialized) const = 0;

    /**
     * @brief Notifys
     *
     * @param nState
     */
    virtual void NotifyEpsfbCallState(IN IMS_UINT32 nState) const = 0;

    /**
     * @brief Calls the AoS to register with next P-CSCF.
     *        Current P-CSCF is marked as unavailable for nUnavailableTimeForCurrentPcscf value.
     *        AoS would trigger P-CSCF discovery procedure if there is no available next P-CSCF.
     *        This is only for handling 503 error response.
     *
     * @param nUnavailableTimeForCurrentPcscf The duration(sec) of marking current P-CSCF as
     *        unavailable.
     */
    virtual void RegisterWithNextPcscf(IN IMS_UINT32 nUnavailableTimeForCurrentPcscf) const = 0;
};

#endif
