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

#ifndef INTERFACE_MTC_AOS_STATE_LISTENER_H_
#define INTERFACE_MTC_AOS_STATE_LISTENER_H_

#include "ImsTypeDef.h"

class IMtcService;

/**
 * @brief Represents the registration state of the Always On Service (AoS) for IMS.
 */
enum class MtcAosState
{
    CONNECTED,
    SUSPENDED,
    DISCONNECTING,
    DISCONNECTED,
};

/**
 * @brief Interface for a listener that receives notifications about Always On Service (AoS)
 *        registration state changes and events.
 */
class IMtcAosStateListener
{
public:
    virtual ~IMtcAosStateListener() {}

    /**
     * @brief Notifies of a change in the Always On Service (AoS) registration state.
     *
     * @param objMtcService The IMtcService instance associated with this state change.
     * @param eState The new MtcAosState registration state.
     * @param eAosReason A service-specific reason for the state change.
     * @param nDataFailureReason The reason for data connection failure.
     */
    virtual void OnAosStateChanged(IN IMtcService& objMtcService, IN MtcAosState eState,
            IN IMS_UINT32 eAosReason, IN IMS_SINT32 nDataFailureReason) = 0;

    /**
     * @brief Notifies of events related to the Always On Service registration.
     *
     * @param nType The type of the registration event that occurred.
     * @param nState The new state or value associated with the event.
     */
    virtual void OnEventNotify(IN IMS_UINT32 nType, IN IMS_UINT32 nState) = 0;
};

#endif
