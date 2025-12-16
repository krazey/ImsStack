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

#ifndef MOCK_I_CONFERENCE_CONTROLLER_H_
#define MOCK_I_CONFERENCE_CONTROLLER_H_

#include "ImsTypeDef.h"
#include "call/IMtcCall.h"
#include "conferencecall/IConferenceController.h"
#include <gmock/gmock.h>

class ConferenceParticipantList;
class SuppService;
struct ConfUser;
struct MediaInfo;
template <typename T>
class ImsList;

class MockIConferenceController : public IConferenceController
{
public:
    MOCK_METHOD(void, ProcessCommand,
            (IN IMS_UINT32 nCmd, (IN ImsList<ConfUser*> & objUsers), IN CallInfo& objCallInfo,
                    IN MediaInfo& objMediaInfo, (IN ImsList<SuppService*> & objSuppServices)),
            (override));
    MOCK_METHOD(void, ProcessCommand, (IN IMS_UINT32 nCmd, (IN ImsList<ConfUser*> & objUsers)),
            (override));
    MOCK_METHOD(
            IndividualCallState, GetCallStatusInConference, (IN CallKey nKey), (const, override));
};

#endif
