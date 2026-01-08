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

#ifndef INTERFACE_PASSIVE_TIMER_HOLDER_H_
#define INTERFACE_PASSIVE_TIMER_HOLDER_H_

#include "ImsTypeDef.h"

class IPassiveTimerListener;

/**
 * This class holds timers with their duration period. When the timer expires,
 * the listener is notified and the timer is removed from the internal list.
 * All timers in the list are released once a normal registration is disconnedted
 */
class IPassiveTimerHolder
{
public:
    enum class Type
    {
        CALL_BLOCKED_BY_RETRY_AFTER,
        SSAC_VOICE_BARRING,
        SSAC_VIDEO_BARRING,
        PRE_ALERTING_GUARD,
        REGISTRATION_TO_18X,
        RTT_AUTO_UPGRADE_GUARD,
        WAIT_FOR_HANDOVER_TO_RETRY_OVER_IMS_PDN,
    };

    virtual ~IPassiveTimerHolder() = default;

    /**
     * @brief Adds a timer with a specific type.
     *
     * @param eType The type of the timer.
     * @param nTimeInMillis The duration time of the timer to be active. Do nothing if < 0.
     * @param bAllowReset Reset the active timer if it is true.
     * @param bKeepOnAosDisconnect Not release the timer when AOS_DISCONNECTED if it is true
     */
    virtual void AddTimer(IN IPassiveTimerHolder::Type eType, IN IMS_SINT32 nTimeInMillis,
            IN IMS_BOOL bAllowReset = IMS_FALSE, IN IMS_BOOL bKeepOnAosDisconnect = IMS_FALSE) = 0;

    /**
     * @brief Removes a timer with a specific type.
     *
     * @param eType The type of the timer.
     */
    virtual void RemoveTimer(IN IPassiveTimerHolder::Type eType) = 0;

    /**
     * @brief Gets existence of a timer with specific type.
     *
     * @param eType the type of the timer.
     * @return True if the timer is active.
     */
    virtual IMS_BOOL IsActive(IN IPassiveTimerHolder::Type eType) const = 0;

    /**
     * @brief Adds a listener with a specific type. It does nothing if the timer of given type is
     * not activated
     *
     * @param eType The type the listener interested in.
     * @param pPassiveTimerListener The listener.
     */
    virtual void AddListener(IN IPassiveTimerHolder::Type eType,
            IN IPassiveTimerListener* pPassiveTimerListener) = 0;

    /**
     * @brief Removes a listener that has a specific type. It does nothing if the timer of given
     * type is not activated. Each listener must release itself when it disappears.
     *
     * @param eType The type the listener interested in.
     * @param pPassiveTimerListener The listener.
     */
    virtual void RemoveListener(IN IPassiveTimerHolder::Type eType,
            IN IPassiveTimerListener* pPassiveTimerListener) = 0;
};

#endif
