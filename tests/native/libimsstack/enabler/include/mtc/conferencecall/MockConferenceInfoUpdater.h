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

#ifndef MOCK_CONFERENCE_INFO_UPDATER_H_
#define MOCK_CONFERENCE_INFO_UPDATER_H_

#include "ImsTypeDef.h"
#include "conferencecall/ConferenceInfoUpdater.h"
#include <gmock/gmock.h>

class AString;
class ConferenceFactory;
class ConferenceParticipantList;
class IMessageUtils;

class MockConferenceInfoUpdater : public ConferenceInfoUpdater
{
public:
    MockConferenceInfoUpdater(IN ConferenceFactory& objFactory,
            IN MtcConfigurationProxy& objConfigProxy, IN IMessageUtils& objMessageUtils) :
            ConferenceInfoUpdater(objFactory, objConfigProxy, objMessageUtils)
    {
    }
    ~MockConferenceInfoUpdater() override {}
    MOCK_METHOD(IMS_UINT32, Update,
            (IN ConferenceParticipantList * pParticipantList, IN const AString& strEventPackage),
            (override));
};

#endif
