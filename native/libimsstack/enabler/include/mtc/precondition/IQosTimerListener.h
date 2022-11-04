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

#ifndef INTERFACE_QOS_TIMER_LISTENER_H_
#define INTERFACE_QOS_TIMER_LISTENER_H_

#include "ImsTypeDef.h"

class QosTimer;

class IQosTimerListener
{
public:
    virtual ~IQosTimerListener(){};

    /**
     * @brief This method is to notify the timer expiration waiting for the QoS connection.
     * @param pQosTimer The QosTimer instance which has the expired timer.
     */
    virtual void OnWaitTimerExpired(IN QosTimer* pQosTimer) = 0;

    /**
     * @brief This method is to notify the timer expiration guarding the QoS inactivation.
     * @param pQosTimer The QosTimer instance which has the expired timer.
     */
    virtual void OnGuardInactiveTimerExpired(IN QosTimer* pQosTimer) = 0;

    /**
     * @brief This method is to notify the timer expiration to enable the QoS by force.
     *        It is called only in Test Mode.
     * @param pQosTimer The QosTimer instance which has the expired timer.
     */
    virtual void OnForceAvailableTimerExpired(IN QosTimer* pQosTimer) = 0;

    virtual void OnWaitTimerAfterHandOverExpired(IN QosTimer* pQosTimer) = 0;
};
#endif
