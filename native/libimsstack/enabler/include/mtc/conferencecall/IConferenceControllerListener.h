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

#ifndef INTERFACE_CONFERENCE_CONTROLLER_LISTENER_H_
#define INTERFACE_CONFERENCE_CONTROLLER_LISTENER_H_

class ConferenceController;

/**
 * @brief Listener for events from the ConferenceController.
 *
 * This interface allows for receiving notifications about the lifecycle of a
 * ConferenceController, such as when it is closed.
 */
class IConferenceControllerListener
{
public:
    virtual ~IConferenceControllerListener() = default;

    /**
     * @brief Notifies that the ConferenceController has been closed and is about to be destroyed.
     *
     * This callback is invoked when the ConferenceController has completed all its operations
     * and is ready to be deallocated. The listener should release any references to the
     * controller.
     *
     * @param pController A pointer to the ConferenceController that has been closed.
     */
    virtual void OnClosed(IN ConferenceController* pController) = 0;
};

#endif
