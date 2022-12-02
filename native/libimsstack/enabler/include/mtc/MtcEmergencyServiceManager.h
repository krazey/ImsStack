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

#ifndef MTC_EMERGENCY_SERVICE_MANAGER_H_
#define MTC_EMERGENCY_SERVICE_MANAGER_H_

#include "IuMtcService.h"

class IJniMtcServiceThread;
class IMtcContext;

class MtcEmergencyServiceManager
{
public:
    explicit MtcEmergencyServiceManager(IN IMtcContext& objContext);
    virtual ~MtcEmergencyServiceManager();
    MtcEmergencyServiceManager(IN const MtcEmergencyServiceManager&) = delete;
    MtcEmergencyServiceManager& operator=(IN const MtcEmergencyServiceManager&) = delete;

    virtual void OpenEmergencyService(IN IJniMtcServiceThread* pThread);
    virtual void HandleServiceStatus(IN ServiceStatus eStatus, IN IJniMtcServiceThread* pThread);

private:
    void HandleServiceIdle(OUT IMS_BOOL& bStateChanged);
    void HandleServiceActive(OUT IMS_BOOL& bStateChanged);
    void HandleServiceSuspended(OUT IMS_BOOL& bStateChanged);
    void SetState(IN IuMtcService::EmergencyServiceState eState, OUT IMS_BOOL& bChanged);
    void NotifyEmergencyServiceChanged(IN IMS_SINT32 eReason, IN IJniMtcServiceThread* pThread);

    IMtcContext& m_objContext;

protected:
    // open to unit test
    IuMtcService::EmergencyServiceState m_eState;
};

#endif
