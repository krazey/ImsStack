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

#ifndef INTERFACE_MTC_IMS_EVENT_RECEIVER_H_
#define INTERFACE_MTC_IMS_EVENT_RECEIVER_H_

#include "ImsTypeDef.h"

class IMtcImsEventListener;

using ImsEvent = IMS_SINT32;

class IMtcImsEventReceiver
{
public:
    static const IMS_UINT32 UNKNOWN_VALUE;

    virtual ~IMtcImsEventReceiver() {}

    /**
     * Gets cached WParam value for the given event.
     *
     * @param nEvent Event to inspect.
     * @return Stored parameter of the event.
     *         `UNKNOWN_VALUE` if the event haven't occurred or is not supported.
     */
    virtual IMS_UINT32 GetWParam(IN ImsEvent nEvent) = 0;

    /**
     * Gets cached LParam value for the given event.
     *
     * @param nEvent Event to inspect.
     * @return Stored parameter of the event.
     *         `UNKNOWN_VALUE` if the event haven't occurred or is not supported.
     */
    virtual IMS_UINT32 GetLParam(IN ImsEvent nEvent) = 0;

    /**
     * Adds a listener for the event. The listener will be notified if the value for the event is
     * changed. A listener can associated to one or more events.
     * The listener must remove itself by calling `RemoveListener()` before destroy.
     * Nothing happens if the event is not supported.
     *
     * @param pListener Listener to be notified.
     * @param nEvent Event to listen.
     */
    virtual void AddListener(IN IMtcImsEventListener* pListener, IN ImsEvent nEvent) = 0;

    /**
     * Removes a listener for the associated events.
     * Nothing happens if the listener is not associated to the event.
     *
     * @param pListener Listener to be removed.
     * @param nEvent Event to be deregistered.
     */
    virtual void RemoveListener(IN IMtcImsEventListener* pListener, IN ImsEvent nEvent) = 0;
};

class IMtcImsEventListener
{
public:
    virtual ~IMtcImsEventListener() {}

    /**
     * Notifies the event. See ImsEventDef.h for the events and corresponding parameters.
     *
     * @param nEvent occurred event.
     * @param nWParam Parameter for the event.
     * @param nLParam Additional parameter for the event.
     */
    virtual void OnImsEventNotified(
            IN ImsEvent nEvent, IN IMS_UINT32 nWParam, IN IMS_UINT32 nLParam) = 0;
};

#endif
