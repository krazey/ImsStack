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

#ifndef INTERFACE_CONFERENCE_OPERATION_QUEUE_LISTENER_H_
#define INTERFACE_CONFERENCE_OPERATION_QUEUE_LISTENER_H_

/**
 * @brief Listener for events from the {@link ConferenceOperationQueue}.
 *
 * This interface allows for receiving notifications when the operation queue is ready
 * to process the next operation.
 */
class IConferenceOperationQueueListener
{
public:
    virtual ~IConferenceOperationQueueListener() = default;

    /**
     * @brief Notifies that the operation queue is ready to process the next operation.
     *
     * This callback is invoked when a new operation can be started, for example,
     * after the previous operation has completed or a delay timer has expired.
     */
    virtual void OnOperationReady() = 0;
};

#endif
