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

#ifndef INTERFACE_MTS_TRAFFIC_H_
#define INTERFACE_MTS_TRAFFIC_H_

#include "ImsTypeDef.h"
#include "MtsDef.h"
#include "IImsRadio.h"

class IMtsTraffic : public IImsRadioConnectionListener
{
public:
    virtual ~IMtsTraffic() override {}

    /**
     * @brief Gets the direction of the SMS traffic.
     *
     * @return The direction of the traffic. (e.g., IImsRadio::DIRECTION_MO)
     */
    virtual IMS_UINT32 GetDirection() const = 0;

    /**
     * @brief Gets the type of the SMS traffic.
     *
     * @return The type of the traffic (e.g., IImsRadio::TRAFFIC_TYPE_SMS).
     */

    virtual IMS_UINT32 GetTrafficType() const = 0;

    /**
     * @brief Checks whether the radio guard timer is currently active.
     *
     * @return {@code IMS_TRUE} If the radio guard timer is active,
     *         {@code IMS_FALSE} otherwise.
     */
    virtual IMS_BOOL IsRadioGuardTimerActive() = 0;

    /**
     * @brief Starts the radio guard timer.
     */
    virtual void StartRadioGuardTimer(IN IMS_UINT32 nDuration = MTS_RADIO_GUARD_TIMER_MS) = 0;
};

#endif
