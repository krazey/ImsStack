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

#ifndef MOCK_CONFERENCE_PARTICIPANT_LIST_H_
#define MOCK_CONFERENCE_PARTICIPANT_LIST_H_

#include "ImsList.h"
#include "MtcDef.h"
#include "conferencecall/ConferenceParticipantList.h"
#include <gmock/gmock.h>

class CallConnectionIdManager;
class IConferenceReference;
class IMtcCallManager;

class MockConferenceParticipantList : public ConferenceParticipantList
{
public:
    ~MockConferenceParticipantList() {}
    MOCK_METHOD(void, SetLocalUri, (IN const AString& strLocalUri), ());
    MOCK_METHOD(const AString&, GetLocalUri, (), (const));
    MOCK_METHOD(void, SetXmlVersion, (IN IMS_SINT32 nVersion), ());
    MOCK_METHOD(IMS_SINT32, GetXmlVersion, (), (const));
    MOCK_METHOD(void, SetMaxUserCount, (IN IMS_SINT32 nMaxUserCount), ());
    MOCK_METHOD(IMS_SINT32, GetMaxUserCount, (), (const, override));
    MOCK_METHOD(void, AddUser, (IN const ConfUser* pConfUser), (override));
    MOCK_METHOD(void, RemoveUser, (IN const ConfUser* pConfUser), (override));
    MOCK_METHOD(
            ConfUser*, GetConfUser, (IN IConferenceReference* piConfReference), (const, override));
    MOCK_METHOD(IMS_BOOL, IsConnectedUser,
            (IN const ConfUser* pConfUser, IN IMS_BOOL bIncludingConnecting), (const, override));
    MOCK_METHOD(IMSList<ConfUser*>, GetConfUsers, (IN IMS_BOOL bCopy), (const, override));
    MOCK_METHOD(void, SetReference,
            (IN IConferenceReference* piReference, IN const ConfUser* pConfUser), (override));
    MOCK_METHOD(
            IConferenceReference*, GetReference, (IN const ConfUser* pConfUser), (const, override));
    MOCK_METHOD(void, ResetReference, (IN IConferenceReference* piConfReference), (override));
    MOCK_METHOD(void, SetReferInviteUri,
            (IN AString strReferInviteUri, IN const ConfUser* pConfUser), (override));
    MOCK_METHOD(AString, GetReferInviteUri, (IN const ConfUser* pConfUser), (override));
    MOCK_METHOD(IMS_SINT32, FindParticipant, (IN IMS_UINT32 nConnectionId), (override));
    MOCK_METHOD(void, ReOrder,
            (IN IMtcCallManager& objCallManager,
                    IN CallConnectionIdManager& objConnectionIdManager),
            ());
    MOCK_METHOD(void, Login, (), ());
    MOCK_METHOD(IMS_UINT32, GetSize, (), (const, override));
    MOCK_METHOD(ConferenceParticipant*, GetAt, (IN IMS_UINT32 nAt), (override));
    MOCK_METHOD(ConfUser*, GetConfUser, (IN IMS_UINT32 nIndex), (const, override));
    MOCK_METHOD(IMS_UINT32, GetConnectedParticipantSize, (IN IMS_BOOL bIncludingConnecting),
            (override));
    MOCK_METHOD(IMS_SINT32, FindParticipant, (IN const ConfUser* pConfUser), (const));
};

#endif
