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
#ifndef AOS_SERVICE_AVAILABLE_CELLULAR_H_
#define AOS_SERVICE_AVAILABLE_CELLULAR_H_

#include "condition/AosServiceAvailable.h"

class AosServiceAvailableCellular : public AosServiceAvailable
{
public:
    AosServiceAvailableCellular();
    virtual ~AosServiceAvailableCellular();

    IMS_BOOL IsVopsSupported();

protected:
    void HandleNetworkStateChanged() final;
    void HandleRoamingChanged(IN IMS_UINT32 nState) override;
    void HandleAirplaneModeChanged(IN IMS_UINT32 nState) override;
    void HandleVopsChanged(IN IMS_UINT32 nState) override;
    IMS_BOOL CheckServiceAvailable() final;

private:
    IMS_BOOL m_bVopsState;
    IMS_BOOL m_bNetworkServiceIn;
};

#endif  // AOS_SERVICE_AVAILABLE_CELLULAR_H_
