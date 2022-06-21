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

#include "IMtcContext.h"
#include "IuMtcService.h"

class JniMtcServiceThread;

class MtcEmergencyServiceManager
{
public:
    MtcEmergencyServiceManager(IN IMtcContext& objContext);
    virtual ~MtcEmergencyServiceManager();
    MtcEmergencyServiceManager(IN const MtcEmergencyServiceManager&) = delete;
    MtcEmergencyServiceManager& operator=(IN const MtcEmergencyServiceManager&) = delete;

    void OpenEmergencyService(IN JniMtcServiceThread* pServiceThread);
    void ProcessServiceStatus(IN ServiceStatus eStatus, IN JniMtcServiceThread* pServiceThread);
    void SetStatus(IN IuMtcService::EmergencyServiceStatus eStatus,
            IN JniMtcServiceThread* pServiceThread);

private:
    IMtcContext& m_objContext;
    IuMtcService::EmergencyServiceStatus m_eStatus;

};

#endif
