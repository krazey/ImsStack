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

#ifndef INTERFACE_MEDIA_QOS_EVENT_LISTENER_H_
#define INTERFACE_MEDIA_QOS_EVENT_LISTENER_H_

#include "ImsTypeDef.h"
#include "precondition/QosDef.h"

class ISession;

/**
 * @brief Interface for listening to Quality of Service (QoS) events.
 *
 * This interface is implemented by classes that need to monitor the QoS status of media streams
 * within a session. It provides notifications when QoS resources become available or are lost,
 * which is critical for preconditions handling and call setup.
 */
class IMediaQosEventListener
{
public:
    virtual ~IMediaQosEventListener(){};

    /**
     * @brief Notifies that the QoS status has changed for a specific session and media type.
     *
     * This callback is invoked when the underlying media layer reports a change in resource
     * reservation status.
     *
     * @param piSession The #ISession associated with the QoS event.
     * @param eStatus The new #QosStatus (e.g., AVAILABLE, LOST).
     * @param eMediaType The media type(s) affected by the status change (e.g., #MEDIATYPE_AUDIO).
     * @param bNeedToNotify Indicates if this event should be propagated to listeners (e.g. UI).
     */
    virtual void OnQosStatusChanged(IN ISession* piSession, IN QosStatus eStatus,
            IN IMS_UINT32 eMediaType, IN IMS_BOOL bNeedToNotify = IMS_TRUE) = 0;
};
#endif
