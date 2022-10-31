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

#include "call/IMtcCall.h"
#include "conferencecall/IConferenceControllerListener.h"

class IConferenceController;
enum class ConferenceType;

class IConferenceManager
{
public:
    virtual ~IConferenceManager() {}

    virtual IConferenceController& CreateController(
            IN CallKey nCallKey, IN ConferenceType eType) = 0;
    virtual IConferenceController* GetController(IN IMS_UINTP nCallKey) const = 0;
};

enum class ConferenceType
{
    PARTICIPANT,
    GROUP_CALL,  // aka. start conference
    MERGE_CALL,
    EXPAND_CALL
};

#endif
