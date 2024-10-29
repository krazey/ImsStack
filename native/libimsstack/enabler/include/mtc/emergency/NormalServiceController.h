/*
 * Copyright (C) 2023 The Android Open Source Project
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

#ifndef NORMAL_SERVICE_CONTROLLER_H_
#define NORMAL_SERVICE_CONTROLLER_H_

#include "ImsTypeDef.h"
#include "IMtcCallStateListener.h"
#include "call/IMtcCall.h"
#include "emergency/IMtcEmergencyServiceManager.h"
#include "helper/IMtcAosStateListener.h"

class IMtcContext;

using EmergencyServiceState = IuMtcService::EmergencyServiceState;

class NormalServiceController : public IEmergencyServiceController, public IMtcCallStateListener
{
public:
    explicit NormalServiceController(
            IN IMtcEmergencyServiceManager& objServiceManager, IN IMtcContext& objContext);
    virtual ~NormalServiceController();
    NormalServiceController(IN const NormalServiceController&) = delete;
    NormalServiceController& operator=(IN const NormalServiceController&) = delete;

    void Start() override;
    inline void Close() override {}
    inline ServiceType GetServiceType() const override { return ServiceType::NORMAL; }

    void OnCallStateChanged(IN CallKey nCallKey, IN IMtcCall::State eState, IN Type eType,
            IN IMS_BOOL bEmergency, IN IMS_SINT32 nReason) override;
    void OnTotalCallStateChanged(IN State) override {}

private:
    const LOCAL IMS_SINT32 REASON_UNSPECIFIED = -1;

    IMtcEmergencyServiceManager& m_objServiceManager;
    IMtcContext& m_objContext;

    void HandleServiceOpened();
    void HandleServiceUnavailable(IN IMS_UINT32 eAosReason);

    void AddListeners();
    void RemoveListeners();

    void Notify(IN EmergencyServiceState eState, IN IMS_SINT32 eReason = REASON_UNSPECIFIED) const;
    void Finish();
};

#endif
