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
#ifndef INTERFACE_IMS_TRAFFIC_TIMER_H_
#define INTERFACE_IMS_TRAFFIC_TIMER_H_

#include "ImsTypeDef.h"

class IImsTrafficTimerListener
{
protected:
    virtual ~IImsTrafficTimerListener() = default;

public:
    /**
     * @brief Notifies that the IMS traffic timer is expired.
     *
     * @param nSlotId The slot ID
     * @param nType The IMS traffic type
     */
    virtual void ImsTrafficTimer_Expired(IN IMS_UINT32 nSlotId, IN IMS_UINT32 nType) = 0;
};

class IImsTrafficTimer
{
protected:
    virtual ~IImsTrafficTimer() = default;

public:
    /**
     * @brief Starts the IMS traffic timer.
     */
    virtual void Start() = 0;

    /**
     * @brief Stops the IMS traffic timer.
     */
    virtual void Stop() = 0;

    /**
     * @brief Sets the listener to be notified for IMS traffic timer.
     *
     * @param piListener The listener to be set
     */
    virtual void SetListener(IN IImsTrafficTimerListener* piListener) = 0;
};

#endif
