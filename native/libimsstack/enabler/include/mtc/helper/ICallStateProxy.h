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

#ifndef INTERFACE_CALL_STATE_PROXY_H_
#define INTERFACE_CALL_STATE_PROXY_H_

#include "CallReasonInfo.h"
#include "ImsTypeDef.h"
#include "call/IMtcCall.h"

class IMtcCallStateListener;

/**
 * @brief Interface for proxying call state changes to registered listeners.
 *
 * This interface allows components to register for call state updates and provides
 * methods to broadcast these updates to all registered listeners.
 */
class ICallStateProxy
{
public:
    virtual ~ICallStateProxy() {}

public:
    /**
     * @brief Adds a listener for call state events.
     *
     * @param pListener The listener object to add.
     */
    virtual void AddListener(IN IMtcCallStateListener* pListener) = 0;

    /**
     * @brief Removes a listener for call state events.
     *
     * @param pListener The listener object to remove.
     */
    virtual void RemoveListener(IN IMtcCallStateListener* pListener) = 0;

    /**
     * @brief Notifies registered listeners of a call state update.
     *
     * @param nCallkey The unique identifier of the call.
     * @param eState The new state of the call.
     * @param eCallType The type of the call (e.g. Voice, Video).
     * @param bEmergency IMS_TRUE if the call is an emergency call, IMS_FALSE otherwise.
     * @param nReason The reason code for the state change (default is CODE_NONE).
     */
    virtual void UpdateCallState(IN CallKey nCallkey, IN IMtcCall::State eState,
            IN CallType eCallType, IN IMS_BOOL bEmergency, IN IMS_SINT32 nReason = CODE_NONE) = 0;

    /**
     * @brief Notifies listeners that the SIP session associated with a call has been released.
     *
     * This function is called under the following conditions:
     * 1. Emergency call: BYE/CANCEL is completed with a guard timer by T1.
     * 2. Normal call: BYE/CANCEL is started and MtcCall is destroyed.
     *
     * @param nCallkey The unique identifier of the call.
     * @param bEmergency IMS_TRUE if the call was an emergency call, IMS_FALSE otherwise.
     * @param bEstablished IMS_TRUE if the call had a confirmed dialog, IMS_FALSE otherwise.
     */
    virtual void NotifyCallSessionReleased(
            IN CallKey nCallkey, IN IMS_BOOL bEmergency, IN IMS_BOOL bEstablished) = 0;
};

#endif
