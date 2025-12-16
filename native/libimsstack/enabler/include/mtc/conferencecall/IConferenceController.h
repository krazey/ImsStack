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

#ifndef INTERFACE_CONFERENCE_CONTROLLER_H_
#define INTERFACE_CONFERENCE_CONTROLLER_H_

#include "ImsTypeDef.h"

class ConferenceParticipantList;
class SuppService;
struct MediaInfo;
template <class T>
class ImsList;

/**
 * @brief Represents the state of an individual call within a conference.
 */
enum class IndividualCallState
{
    IDLE,     // Initial state.
    HOST,     // Call with the conference focus.
    JOINING,  // Call being referred to the conference call.
    JOINED,   // Call joined to the conference call and not yet terminated.
    INVITED,  // Call joined to the conference call.
};

/**
 * @brief IConferenceController is an interface for managing conference calls.
 *
 * It defines methods to handle various conference-related commands such as
 * starting a group call, mergeing calls, expanding a call to a conference call,
 * adding/removing participants, and transiting a call to a conference call.
 */
class IConferenceController
{
public:
    /**
     * @brief Defines commands used for conference call control.
     */
    enum
    {
        /** Command to initiate a group call with multiple target numbers. */
        GROUPCALL,
        /** Command to merge two calls into a conference. */
        MERGE,
        /** Command to expand a 1-on-1 call into a conference by adding another call. */
        EXPAND,
        /** Command to add a participant to an existing conference. */
        ADD,
        /** Command to remove a participant from an existing conference. */
        REMOVE,
        /** Command indicating existing 1-on-1 call has successfully joined the conference. */
        JOINED
    };
    virtual ~IConferenceController() {}

    /**
     * @brief Processes a conference-related command.
     *
     * This version includes call, media, supplementary service information, and ConfUser list.
     *
     * @param nCmd The command to be processed (e.g., GROUPCALL).
     * @param objUsers A list of conference users related to the command.
     * @param objCallInfo Call-related information.
     * @param objMediaInfo Media information for the call.
     * @param objSuppServices Supplementary services information.
     */
    virtual void ProcessCommand(IN IMS_UINT32 nCmd, IN ImsList<ConfUser*>& objUsers,
            IN CallInfo& objCallInfo, IN MediaInfo& objMediaInfo,
            IN ImsList<SuppService*>& objSuppServices) = 0;

    /**
     * @brief Processes a conference-related command.
     *
     * This version uses only ConfUser list.
     *
     * @param nCmd The command to be processed (e.g., MERGE, EXPAND, ADD, REMOVE).
     * @param objUsers A list of conference users related to the command.
     */
    virtual void ProcessCommand(IN IMS_UINT32 nCmd, IN ImsList<ConfUser*>& objUsers) = 0;

    /**
     * @brief Gets the status of a specific call within the conference, identified by its CallKey.
     *
     * @param nKey The unique identifier for the call whose status is to be retrieved.
     * @return The state of the call {@link IndividualCallState}.
     */
    virtual IndividualCallState GetCallStatusInConference(IN CallKey nKey) const = 0;
};

#endif
