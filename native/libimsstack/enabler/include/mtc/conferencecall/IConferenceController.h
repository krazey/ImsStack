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

#include "MtcDef.h"

class ConferenceParticipantList;

enum class IndividualCallState
{
    IDLE,
    JOINING,
    JOINED,
    INVITED
};

class IConferenceController
{
public:
    enum /*class Command*/
    {
        GROUPCALL,
        MERGE,
        EXPAND,
        ADD,
        REMOVE,
        JOINED
    };
    virtual ~IConferenceController() {}
    virtual void ProcessCommand(IN IMS_UINT32 nCmd, IN IMSList<ConfUser*>& objUsers,
            IN CallInfo& objCallInfo, IN MediaInfo& objMediaInfo,
            IN IMSMap<SuppType, SuppService*>& objSuppServices) = 0;
    virtual void ProcessCommand(IN IMS_UINT32 nCmd, IN IMSList<ConfUser*>& objUsers) = 0;
    virtual IMS_SINT32 GetState() const = 0;
    virtual IndividualCallState GetCallStatusInConference(IN CallKey nKey) const = 0;
};

#endif
