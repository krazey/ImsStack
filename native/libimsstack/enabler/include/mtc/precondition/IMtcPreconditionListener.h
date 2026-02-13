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

#ifndef INTERFACE_MTC_PRECONDITION_LISTENER_H_
#define INTERFACE_MTC_PRECONDITION_LISTENER_H_

#include "ImsTypeDef.h"

class ISession;
enum class QosLossPolicy;

class IMtcPreconditionListener
{
public:
    virtual ~IMtcPreconditionListener(){};

    /**
     * @brief Handles the successful reservation of QoS.
     *
     * @param piSession The session for which QoS was reserved.
     * @param eMediaType The media type for which QoS was reserved.
     */
    virtual void QosReserved(IN ISession* piSession, IN IMS_UINT32 eMediaType) = 0;

    /**
     * @brief Handles the failure of QoS reservation.
     *
     * @param piSession The session for which QoS reservation failed.
     * @param eNextAction The next action to take based on the QoS loss policy.
     */
    virtual void QosReserveFailed(IN ISession* piSession, IN QosLossPolicy eNextAction) = 0;
};

#endif
