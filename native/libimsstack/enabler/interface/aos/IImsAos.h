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
#ifndef INTERFACE_IMS_AOS_H_
#define INTERFACE_IMS_AOS_H_

#include "AString.h"
#include "ImsList.h"

class IImsAosInfo;
class IImsAosListener;
class IImsAosMonitor;
class ImsAosFeatureTag;

/**
 * @brief This class provides the interface for AoS.
 *
 * The IImsAos is the interface between service enabler and AoS enabler. \n
 * It provides the APIs to get the detailed information and set the listeners that are related to
 * the registration status for each service and all services.
 *
 * @see IImsAosInfo, IImsAosListener, IImsAosMonitor, ImsAosParameter.h
 */

class IImsAos
{
public:
    virtual ~IImsAos(){};

    /**
     * @brief Control the operation for registration
     *
     * @param nType Indicate the control type for registration (@see ImsAosControl)
     * @return If requested type is available, returns IMS_TRUE.
     */
    virtual IMS_BOOL Control(IN IMS_UINT32 nType) = 0;

    /**
     * @brief Get the interface for AoS in order to get the detail information and notify
     *        the service information.
     *
     * @return @see IImsAosInfo
     */
    virtual IImsAosInfo* GetAosInfo() = 0;

    /**
     * @brief Get the features that are registered for the called service.
     *
     * @return Indicated the fetures that contains in 200 OK response for registration.
     *         (@see ImsAosFeature)
     */
    virtual IMS_UINT32 GetFeatures() = 0;

    /**
     * @brief Get the suspended reason
     *
     * @return Indicate the suspended reason
     */
    virtual IMS_UINT32 GetSuspendedReason() = 0;

    /**
     * @brief Indicate whether the feature is registered or not
     *
     * @param nFeature Indicate the feature for registration. (@see ImsAosFeature)
     * @return If the feature is registered, returns IMS_TRUE.
     */
    virtual IMS_BOOL IsFeatureConnected(IN IMS_UINT32 nFeature) = 0;

    /**
     * @brief Indicate whether the service is connected or not
     *
     * @return If the service is connected, returns IMS_TRUE.
     */
    virtual IMS_BOOL IsImsConnected() = 0;

    /**
     * @brief Indicate whether the service is suspended or not
     *        even though registration is maintained
     *
     * @return If the service is suspended, returns IMS_TRUE.
     */
    virtual IMS_BOOL IsImsSuspended() = 0;

    /**
     * @brief Set the listener that provides the each service statsus related to registration
     *        status.
     *
     * @param piListener Listener to be set
     */
    virtual void SetListener(IN IImsAosListener* piListener) = 0;

    /**
     * @brief Set the listener that provides the every service status and the specific information
     *
     * @param piMonitor Listener to be set
     */
    virtual void SetMonitor(IN IImsAosMonitor* piMonitor) = 0;

    /**
     * @brief Set the service ready for registering the listener to get the enabler information
     *
     * @param bReady enabler service is started or not
     * @param nService @see ImsAosService
     *
     * @return If complete to set ready, returns IMS_TRUE.
     */
    virtual IMS_BOOL SetReady(IN IMS_BOOL bReady, IN IMS_UINT32 nService) = 0;

    /**
     * @brief Update the features for registration procedure. If features are changed
     *        and reigistration is done, reregistration is tried with the changed features.
     *
     * @param nFeatures Indicate the features for registration (@see ImsAosFeature)
     */
    virtual void UpdateFeature(IN IMS_UINT32 nFeatures) = 0;

    /**
     * @brief Update the features for registration procedure. If features are changed
     *        and reigistration is done, reregistration is tried with the changed features.
     *
     * @param objFeatureTag Indicate the list of the feature-tag (@see ImsAosFeatureTag)
     */
    virtual void UpdateFeature(IN ImsList<ImsAosFeatureTag*>& objFeatureTag) = 0;

    /**
     * @brief Register with next PCSCF.
     *        Current PCSCF is marked as unavailable for nUnavailableTimeForCurrentPcscf value.
     *        Aos would trigger PCSCF discovery procedure if there is no available next PCSCF.
     *        This will terminate current registration.
     *        Then the callback ImsAos_Disconnected() will be called with REG_NEW_REQUIRED reason.
     *        (@see IImsAosListener::ImsAos_Disconnected)
     *        (@see ImsAosReason::REG_NEW_REQUIRED)
     *
     * @param nUnavailableTimeForCurrentPcscf
     *        The duration(sec) of marking current PCSCF as unavailable.
     */
    virtual void RegisterWithNextPcscf(IN IMS_UINT32 nUnavailableTimeForCurrentPcscf) = 0;
};

#endif  // INTERFACE_IMS_AOS_H_
