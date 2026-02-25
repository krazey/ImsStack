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

#ifndef INTERFACE_MTC_CONTEXT_H_
#define INTERFACE_MTC_CONTEXT_H_

#include "ImsTypeDef.h"
#include "helper/MtcTimerWrapper.h"
#include <functional>
#include <memory>

class CallConnectionIdManager;
class ICallStateProxy;
class IConferenceManager;
class IEctManager;
class ILastComeFirstServedHelper;
class IMessageUtils;
class IMtcAosConnector;
class IMtcCallController;
class IMtcCallManager;
class IMtcDialingPlan;
class IMtcEmergencyServiceManager;
class IMtcImsEventReceiver;
class IMtcRadioChecker;
class IMtcService;
class IMtcSipInterfaceFactory;
class IMultiEndpointManager;
class IPassiveTimerHolder;
class ISubscriberConfig;
class MtcConfigurationProxy;
class MtcLocationRefresher;
class RttAutoUpgrader;
class OperationAsyncRunner;
enum class ServiceType;

/**
 * @brief Interface that provides access to the context and components of the MTC enabler.
 *
 * This interface acts as a central registry or service locator, allowing various parts of
 * the MTC application to retrieve shared instances of managers, controllers, and utility classes
 * (e.g., {@code IMtcCallManager}, {@code IMtcCallController}, {@code IMtcDialingPlan}).
 */
class IMtcContext
{
public:
    virtual ~IMtcContext() = default;

    /**
     * @brief Gets current slot ID.
     *
     * @return Slot ID.
     */
    virtual IMS_SINT32 GetSlotId() const = 0;

    /**
     * @brief Gets {@code ISubscriberConfig} from Configuration using current slot ID.
     *
     * @return Instance of {@code ISubscriberConfig}. Could be null.
     */
    virtual const ISubscriberConfig* GetSubscriberConfig() const = 0;

    /**
     * @brief Gets the MTC service instance for the specified service type.
     *
     * @param eServiceType The type of service (e.g., {@code ServiceType::NORMAL},
     *                     {@code ServiceType::EMERGENCY}).
     * @return Pointer to the {@code IMtcService} instance.
     */
    virtual IMtcService* GetServiceByType(IN ServiceType eServiceType) = 0;

    /**
     * @brief Gets the Dialing Plan instance.
     *
     * @return Reference to the {@code IMtcDialingPlan} instance.
     */
    virtual IMtcDialingPlan& GetDialingPlan() = 0;

    /**
     * @brief Gets the Call Controller instance.
     *
     * @return Reference to the {@code IMtcCallController} instance.
     */
    virtual IMtcCallController& GetCallController() = 0;

    /**
     * @brief Gets the Call Manager instance.
     *
     * @return Reference to the {@code IMtcCallManager} instance.
     */
    virtual IMtcCallManager& GetCallManager() = 0;

    /**
     * @brief Gets the Radio Checker instance.
     *
     * @return Reference to the {@code IMtcRadioChecker} instance.
     */
    virtual IMtcRadioChecker& GetRadioChecker() = 0;

    /**
     * @brief Gets the Configuration Proxy instance.
     *
     * @return Reference to the {@code MtcConfigurationProxy} instance.
     */
    virtual MtcConfigurationProxy& GetConfigurationProxy() = 0;

    /**
     * @brief Gets the Call State Proxy instance.
     *
     * @return Reference to the {@code ICallStateProxy} instance.
     */
    virtual ICallStateProxy& GetCallStateProxy() = 0;

    /**
     * @brief Gets the IMS Event Receiver instance.
     *
     * @return Reference to the {@code IMtcImsEventReceiver} instance.
     */
    virtual IMtcImsEventReceiver& GetImsEventReceiver() = 0;

    /**
     * @brief Gets the AOS Connector instance for the specified service type.
     *
     * @param eServiceType The type of service (e.g., {@code ServiceType::NORMAL},
     *                     {@code ServiceType::EMERGENCY}).
     * @return Pointer to the {@code IMtcAosConnector} instance.
     */
    virtual IMtcAosConnector* GetAosConnector(IN ServiceType eServiceType) = 0;

    /**
     * @brief Gets the SIP Interface Factory instance.
     *
     * @return Reference to the {@code IMtcSipInterfaceFactory} instance.
     */
    virtual IMtcSipInterfaceFactory& GetSipInterfaceFactory() = 0;

    /**
     * @brief Gets the Conference Manager instance.
     *
     * @return Reference to the {@code IConferenceManager} instance.
     */
    virtual IConferenceManager& GetConferenceManager() = 0;

    /**
     * @brief Gets the ECT (Explicit Call Transfer) Manager instance.
     *
     * @return Reference to the {@code IEctManager} instance.
     */
    virtual IEctManager& GetEctManager() = 0;

    /**
     * @brief Gets the Emergency Service Manager instance.
     *
     * @return Reference to the {@code IMtcEmergencyServiceManager} instance.
     */
    virtual IMtcEmergencyServiceManager& GetEmergencyServiceManager() = 0;

    /**
     * @brief Runs an operation asynchronously using the EnablerThread.
     *
     * This method schedules an asynchronous operation to be executed.
     * The provided `pOwner` serves as the primary object managing the operation's context.
     * It is the caller's responsibility to ensure that the owner object is appropriately
     * set to represent the resources used in the operation.
     *
     * @param pOwner The object that serves as the owner or context of the operation.
     *               This object must remain valid while the operation is being executed.
     * @param objOperation A callable object (such as a lambda function) that defines the operation
     *                     to be executed.
     *
     * @note The caller must invoke {@code ReleaseAsyncOperation} when the owner object is being
     *       destroyed.
     *       This ensures that resources related to the operation are not accessed incorrectly after
     *       the owner is no longer valid.
     */
    virtual void RunAsyncOperation(IN void* pOwner, IN std::function<void()> objOperation) = 0;

    /**
     * @brief Releases all asynchronous operations associated with the specified owner.
     *
     * This method ensures that any operations linked to the provided `pOwner` are properly
     * released or terminated. It is essential to call this method before the owner object is
     * destroyed to prevent invalid resource access or undefined behavior.
     *
     * @param pOwner The object that owns the operations to be released. This should match
     *               the owner object provided in previous `RunAsync` calls.
     *
     * @note Failure to call this method before destroying the owner object may result in
     *       resource leaks or operations accessing invalid memory.
     */
    virtual void ReleaseAsyncOperation(IN void* pOwner) = 0;

    /**
     * @brief Gets the Message Utils instance.
     *
     * @return Reference to the {@code IMessageUtils} instance.
     */
    virtual IMessageUtils& GetMessageUtils() = 0;

    /**
     * @brief Creates a new {@code MtcTimerWrapper} instance.
     *
     * @return A unique pointer to the new {@code MtcTimerWrapper} instance.
     */
    virtual std::unique_ptr<MtcTimerWrapper> CreateTimer() = 0;

    /**
     * @brief Gets the Passive Timer Holder instance.
     *
     * @return Reference to the {@code IPassiveTimerHolder} instance.
     */
    virtual IPassiveTimerHolder& GetPassiveTimerHolder() = 0;

    /**
     * @brief Gets the Multi-Endpoint Manager instance.
     *
     * @return Pointer to the {@code IMultiEndpointManager} instance.
     */
    virtual IMultiEndpointManager* GetMultiEndpointManager() = 0;

    /**
     * @brief Gets the Last Come First Served Helper instance.
     *
     * @return Reference to the {@code ILastComeFirstServedHelper} instance.
     */
    virtual ILastComeFirstServedHelper& GetLastComeFirstServedHelper() = 0;

    /**
     * @brief Gets the Call Connection ID Manager instance.
     *
     * @return Reference to the {@code CallConnectionIdManager} instance.
     */
    virtual CallConnectionIdManager& GetCallConnectionIdManager() = 0;

    /**
     * @brief Gets the Location Refresher instance.
     *
     * @return Reference to the {@code MtcLocationRefresher} instance.
     */
    virtual MtcLocationRefresher& GetLocationRefresher() = 0;

    /**
     * @brief Initializes the RTT (Real-time Text) Auto Upgrader.
     */
    virtual void CreateRttAutoUpgrader() = 0;

    /**
     * @brief Destroys the RTT (Real-time Text) Auto Upgrader.
     */
    virtual void DestroyRttAutoUpgrader() = 0;

    /**
     * @brief Checks if the device is in Wi-Fi test mode.
     *
     * @return {@code IMS_TRUE} if in Wi-Fi test mode, {@code IMS_FALSE} otherwise.
     */
    virtual IMS_BOOL IsWifiTestMode() = 0;
};

#endif
