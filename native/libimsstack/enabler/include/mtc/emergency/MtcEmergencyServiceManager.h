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

#include "ImsTypeDef.h"
#include "emergency/IMtcEmergencyServiceManager.h"
#include <memory>

class IMtcContext;

class MtcEmergencyServiceManager : public IMtcEmergencyServiceManager
{
public:
    explicit MtcEmergencyServiceManager(IN IMtcContext& objContext);
    virtual ~MtcEmergencyServiceManager();
    MtcEmergencyServiceManager(IN const MtcEmergencyServiceManager&) = delete;
    MtcEmergencyServiceManager& operator=(IN const MtcEmergencyServiceManager&) = delete;

    void StartOpen(IN ServiceType eServiceType) override;
    void StopOpen(IN IMS_BOOL bClose) override;

protected:
    // Visible for test
    std::unique_ptr<IEmergencyServiceController> m_pController;

private:
    IMtcContext& m_objContext;

    IEmergencyServiceController* CreateController(IN ServiceType eServiceType);
};

#endif
