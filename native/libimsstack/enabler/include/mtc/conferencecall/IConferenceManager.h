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

#ifndef INTERFACE_CONFERENCE_MANAGER_H_
#define INTERFACE_CONFERENCE_MANAGER_H_

#include "ImsTypeDef.h"
#include "call/IMtcCall.h"

class IConferenceController;
enum class ConferenceType;

/**
 * @brief An interface for managing conference controllers.
 *
 * This manager is responsible for creating and retrieving conference controllers,
 * which handle the logic for different types of conference call operations.
 */
class IConferenceManager
{
public:
    virtual ~IConferenceManager() {}

    /**
     * @brief Creates a new conference controller for a specific call and conference type.
     *
     * This method is called to initiate a conference operation, such as merging calls or
     * starting a new group call. It returns a reference to the newly created controller.
     *
     * @param nCallKey The unique identifier for the call that will host the conference.
     * @param eType The type of conference operation to be performed.
     * @return A reference to the created {@link IConferenceController}.
     */
    virtual IConferenceController& CreateController(
            IN CallKey nCallKey, IN ConferenceType eType) = 0;

    /**
     * @brief Gets an existing conference controller associated with a call key.
     *
     * @param nCallKey The unique identifier for the conference call.
     * @return A pointer to the {@link IConferenceController} if found, otherwise nullptr.
     */
    virtual IConferenceController* GetController(IN IMS_UINTP nCallKey) const = 0;
};

/**
 * @brief Defines the types of conference call operations.
 */
enum class ConferenceType
{
    /** A participant in an existing conference call. */
    PARTICIPANT,
    /** Starting a conference call with multiple target numbers. */
    GROUP_CALL,
    /** Merging two existing calls into a conference. */
    MERGE_CALL,
    /** Expanding a 1-to-1 call into a conference. */
    EXPAND_CALL
};

#endif
