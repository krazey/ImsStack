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

#ifndef INTERFACE_MTC_CALL_STATE_LISTENER_H_
#define INTERFACE_MTC_CALL_STATE_LISTENER_H_

#include "call/IMtcCall.h"

/**
 * @brief Interface for listening to call state changes.
 */
class IMtcCallStateListener
{
public:
    using State = IMtcCall::State;
    using Type = CallType;

    virtual ~IMtcCallStateListener() = default;

    /**
     * @brief Notifies that the call state has changed.
     *
     * @param nCallKey The unique identifier of the call.
     * @param eState The new state of the call.
     * @param eType The type of the call (e.g. Voice, Video).
     * @param bEmergency IMS_TRUE if the call is an emergency call, IMS_FALSE otherwise.
     * @param nReason The reason code associated with the state change.
     */
    virtual void OnCallStateChanged(IN CallKey nCallKey, IN State eState, IN Type eType,
            IN IMS_BOOL bEmergency, IN IMS_SINT32 nReason) = 0;

    /**
     * @brief Notifies that the overall call state has changed.
     *
     * @param eState The aggregated state of all calls.
     */
    virtual void OnTotalCallStateChanged(IN State eState) = 0;

    /**
     * @brief Notifies that the SIP session associated with a call has been released.
     *
     * This function is called under the following conditions:
     * 1. Emergency call: BYE/CANCEL is completed with a guard timer by T1.
     * 2. Normal call: BYE/CANCEL is started and MtcCall is destroyed.
     *
     * @param nCallKey The unique identifier of the call.
     * @param bEmergency IMS_TRUE if the call was an emergency call, IMS_FALSE otherwise.
     * @param bEstablished IMS_TRUE if the call had a confirmed dialog, IMS_FALSE otherwise.
     */
    virtual void OnCallSessionReleased(IN [[maybe_unused]] CallKey nCallKey,
            IN [[maybe_unused]] IMS_BOOL bEmergency, IN [[maybe_unused]] IMS_BOOL bEstablished)
    {
    }

    /**
     * @brief Checks if the listener requires synchronous callbacks.
     *
     * @return IMS_TRUE if synchronous callbacks are required, IMS_FALSE otherwise.
     */
    inline virtual IMS_BOOL IsSynchronousCallRequired() { return IMS_FALSE; }
};

#endif
