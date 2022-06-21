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

class MtcDialingPlan;
class MtcCallController;
class IMtcCallManager;
class MtcVonrManager;
class MtcConfigurationProxy;
class CallStateProxy;
class MtcAosConnector;
class MtcImsEventReceiver;
class MtcSipInterfaceFactory;
class ConferenceManager;
class EctManager;
class MtcEmergencyServiceManager;

class IMtcContext
{
public:
    virtual IMS_SINT32 GetSlotId() = 0;
    virtual IMtcService* GetServiceByType(IN ServiceType eServiceType) = 0;
    virtual MtcDialingPlan& GetDialingPlan() = 0;
    virtual MtcCallController& GetCallController() = 0;
    virtual IMtcCallManager& GetCallManager() = 0;
    virtual MtcVonrManager& GetVonrManager() = 0;
    virtual MtcConfigurationProxy& GetConfigurationProxy() = 0;
    virtual CallStateProxy& GetCallStateProxy() = 0;
    virtual MtcImsEventReceiver& GetImsEventReceiver() = 0;
    virtual MtcAosConnector* GetAosConnector(IN ServiceType eServiceType) = 0;
    virtual MtcSipInterfaceFactory& GetSipInterfaceFactory() = 0;
    virtual ConferenceManager& GetConferenceManager() = 0;
    virtual EctManager* GetEctManager() = 0;
    virtual MtcEmergencyServiceManager* GetEmergencyServiceManager() = 0;
};

#endif
