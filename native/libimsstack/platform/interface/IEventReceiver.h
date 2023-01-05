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
#ifndef INTERFACE_EVENT_RECEIVER_H_
#define INTERFACE_EVENT_RECEIVER_H_

#include "IEventReceiverListener.h"

class IEventReceiver
{
protected:
    virtual ~IEventReceiver() = default;

public:
    /**
     * @brief Unregisters the specified event from the event receiver.
     *
     * @param nEvent The event type
     */
    virtual void ResetEvent(IN IMS_SINT32 nEvent) = 0;

    /**
     * @brief Registers the specified event on the event receiver.
     *
     * @param nEvent The event type
     */
    virtual void SetEvent(IN IMS_SINT32 nEvent) = 0;

    /**
     * @brief Sets the listener for the event notification.
     *
     * @param piListener The listener to receive the event notification
     */
    virtual void SetListener(IN IEventReceiverListener* piListener) = 0;
};

#endif
