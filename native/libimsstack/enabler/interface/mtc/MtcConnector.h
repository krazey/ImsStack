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

#ifndef MTC_CONNECTOR_H_
#define MTC_CONNECTOR_H_

#include "ImsTypeDef.h"

class IMtcCallStateListener;

/**
 * @brief A static utility class that acts as a bridge to the MTC call state event system.
 *
 * This connector provides a simple, static interface for clients to subscribe to (and unsubscribe
 * from) call state change notifications for a specific subscription slot. It abstracts the process
 * of retrieving the correct context and proxy for listener management.
 */
class MtcConnector
{
public:
    /**
     * @brief Registers a listener to receive call state change notifications.
     *
     * @param nSlotId The subscription slot ID to listen to.
     * @param pListener A pointer to the listener implementation that will receive the callbacks.
     */
    static void AddCallStateListener(IN IMS_SINT32 nSlotId, IN IMtcCallStateListener* pListener);

    /**
     * @brief Unregisters a listener, stopping it from receiving further call state notifications.
     *
     * @param nSlotId The subscription slot ID from which to remove the listener.
     * @param pListener A pointer to the listener implementation to be removed.
     */
    static void RemoveCallStateListener(IN IMS_SINT32 nSlotId, IN IMtcCallStateListener* pListener);
};

#endif
