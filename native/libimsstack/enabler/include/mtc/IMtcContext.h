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

#include "IMtcService.h"
#include "ImsTypeDef.h"
#include <functional>

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
class IMtcSipInterfaceFactory;
class IMultiEndpointManager;
class IPassiveTimerHolder;
class MtcConfigurationProxy;
class OperationAsyncRunner;

class IMtcContext
{
public:
    virtual ~IMtcContext() = default;

    /**
     * @brief Gets
     *
     * @return
     */
    virtual IMS_SINT32 GetSlotId() = 0;

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
     * @brief Gets
     *
     * @param objOperation
     * @return
     */
    virtual OperationAsyncRunner* GetAsyncRunner(IN std::function<void()> objOperation) = 0;

    /**
     * @brief Gets
     *
     * @return
     */
    virtual IMessageUtils& GetMessageUtils() = 0;

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
     * @brief Checks
     *
     * @return
     */
    virtual IMS_BOOL IsWifiTestMode() = 0;
};

#endif
