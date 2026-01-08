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
     * @brief Gets ISubscriberConfig from Configuration using current slot ID.
     *
     * @return Instance of ISubscriberConfig. Could be null.
     */
    virtual const ISubscriberConfig* GetSubscriberConfig() const = 0;

    /**
     * @brief Gets
     *
     * @param eServiceType
     * @return
     */
    virtual IMtcService* GetServiceByType(IN ServiceType eServiceType) = 0;

    /**
     * @brief Gets
     *
     * @return
     */
    virtual IMtcDialingPlan& GetDialingPlan() = 0;

    /**
     * @brief Gets
     *
     * @return
     */
    virtual IMtcCallController& GetCallController() = 0;

    /**
     * @brief Gets
     *
     * @return
     */
    virtual IMtcCallManager& GetCallManager() = 0;

    /**
     * @brief Gets
     *
     * @return
     */
    virtual IMtcRadioChecker& GetRadioChecker() = 0;

    /**
     * @brief Gets
     *
     * @return
     */
    virtual MtcConfigurationProxy& GetConfigurationProxy() = 0;

    /**
     * @brief Gets
     *
     * @return
     */
    virtual ICallStateProxy& GetCallStateProxy() = 0;

    /**
     * @brief Gets
     *
     * @return
     */
    virtual IMtcImsEventReceiver& GetImsEventReceiver() = 0;

    /**
     * @brief Gets
     *
     * @param eServiceType
     * @return
     */
    virtual IMtcAosConnector* GetAosConnector(IN ServiceType eServiceType) = 0;

    /**
     * @brief Gets
     *
     * @return
     */
    virtual IMtcSipInterfaceFactory& GetSipInterfaceFactory() = 0;

    /**
     * @brief Gets
     *
     * @return
     */
    virtual IConferenceManager& GetConferenceManager() = 0;

    /**
     * @brief Gets
     *
     * @return
     */
    virtual IEctManager& GetEctManager() = 0;

    /**
     * @brief Gets
     *
     * @return
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
     * @brief Gets
     *
     * @return
     */
    virtual IMessageUtils& GetMessageUtils() = 0;

    /**
     * @brief Creates MtcTimerWrapper. Each time it is called, a new MtcTimerWrapper is created.
     *
     * @return MtcTimerWrapper.
     */
    virtual std::unique_ptr<MtcTimerWrapper> CreateTimer() = 0;

    /**
     * @brief Gets
     *
     * @return
     */
    virtual IPassiveTimerHolder& GetPassiveTimerHolder() = 0;

    /**
     * @brief Gets
     *
     * @return
     */
    virtual IMultiEndpointManager* GetMultiEndpointManager() = 0;

    /**
     * @brief Gets
     *
     * @return
     */
    virtual ILastComeFirstServedHelper& GetLastComeFirstServedHelper() = 0;

    /**
     * @brief Gets
     *
     * @return
     */
    virtual CallConnectionIdManager& GetCallConnectionIdManager() = 0;

    /**
     * @brief Returns an instance of {@link MtcLocationRefresher}.
     *
     * @return {@link MtcLocationRefresher} instance.
     */
    virtual MtcLocationRefresher& GetLocationRefresher() = 0;

    /**
     * @brief Creates
     *
     * @return
     */
    virtual void CreateRttAutoUpgrader() = 0;

    /**
     * @brief Destroy
     *
     * @return
     */
    virtual void DestroyRttAutoUpgrader() = 0;

    /**
     * @brief Checks
     *
     * @return
     */
    virtual IMS_BOOL IsWifiTestMode() = 0;
};

#endif
