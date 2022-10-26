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

#include "IMSTypeDef.h"
#include "IMtcService.h"
#include <functional>

class IMtcDialingPlan;
class IMtcCallController;
class IMtcCallManager;
class MtcConfigurationProxy;
class ICallStateProxy;
class IMtcAosConnector;
class IMtcImsEventReceiver;
class IMtcSipInterfaceFactory;
class IConferenceManager;
class IEctManager;
class MtcEmergencyServiceManager;
class OperationAsyncRunner;
class IMessageUtils;
class IMtcCallTrafficChecker;

class IMtcContext
{
public:
    virtual IMS_SINT32 GetSlotId() = 0;
    virtual IMtcService* GetServiceByType(IN ServiceType eServiceType) = 0;
    virtual IMtcDialingPlan& GetDialingPlan() = 0;
    virtual IMtcCallController& GetCallController() = 0;
    virtual IMtcCallManager& GetCallManager() = 0;
    virtual IMtcCallTrafficChecker& GetCallTrafficChecker() = 0;
    virtual MtcConfigurationProxy& GetConfigurationProxy() = 0;
    virtual ICallStateProxy& GetCallStateProxy() = 0;
    virtual IMtcImsEventReceiver& GetImsEventReceiver() = 0;
    virtual IMtcAosConnector* GetAosConnector(IN ServiceType eServiceType) = 0;
    virtual IMtcSipInterfaceFactory& GetSipInterfaceFactory() = 0;
    virtual IConferenceManager& GetConferenceManager() = 0;
    virtual IEctManager* GetEctManager() = 0;
    virtual MtcEmergencyServiceManager* GetEmergencyServiceManager() = 0;
    virtual OperationAsyncRunner* GetAsyncRunner(IN std::function<void()> objOperation) = 0;
    virtual IMessageUtils& GetMessageUtils() = 0;
    virtual IMS_BOOL IsWifiTestMode() = 0;
};

#endif
