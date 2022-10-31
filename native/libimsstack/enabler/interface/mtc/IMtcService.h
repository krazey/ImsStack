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

#ifndef INTERFACE_MTC_SERVICE_H_
#define INTERFACE_MTC_SERVICE_H_

#include "INativeEnabler.h"
#include "ImsTypeDef.h"
#include "helper/ISrvccStateListener.h"

class AString;
class ICoreService;
class IMtcAosConnector;
class IMtcAosStateListener;
class ISrvccStateListener;
enum class ServiceStatus;
enum class ServiceType;
enum class SrvccState;

class IMtcService : public INativeEnabler
{
public:
    virtual ~IMtcService() {}

    virtual ServiceType GetServiceType() const = 0;
    virtual void AddAosStateListener(IN IMtcAosStateListener* piListener) = 0;
    virtual void RemoveAosStateListener(IN IMtcAosStateListener* piListener) = 0;
    virtual void AddSrvccStateListener(IN ISrvccStateListener* piListener) = 0;
    virtual void RemoveSrvccStateListener(IN ISrvccStateListener* piListener) = 0;

    virtual IMS_BOOL IsActive() const = 0;
    virtual IMS_BOOL IsEmergency() const = 0;
    virtual IMS_BOOL IsWlanIpCanType() const = 0;
    virtual ServiceStatus GetOldStatus() const = 0;
    virtual ServiceStatus GetStatus() const = 0;
    virtual ICoreService* GetICoreService() const = 0;
    virtual IMtcAosConnector* GetAosConnector() const = 0;
    virtual SrvccState GetSrvccState() const = 0;

    virtual void UpdateSrvccState(IN SrvccState eState) = 0;
    virtual void SetTerminalBasedCallWaiting(IN IMS_BOOL bEnabled) = 0;
    virtual IMS_BOOL IsTerminalBasedCallWaitingEnabled() const = 0;
    virtual void OpenEmergencyService() = 0;
};

enum class ServiceStatus
{
    SERVICE_IDLE,
    SERVICE_ACTIVE,
    SERVICE_SUSPENDED,
};

enum class ServiceType
{
    UNKNOWN = 0,
    NORMAL = 1 << 0,
    EMERGENCY = 1 << 1,
};

#endif
