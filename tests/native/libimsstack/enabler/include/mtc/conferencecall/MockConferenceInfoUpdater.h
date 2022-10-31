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

#include "AString.h"
#include "MtcDef.h"
#include "conferencecall/ConferenceInfoUpdater.h"
#include <gmock/gmock.h>

class ConferenceFactory;
class ConferenceParticipantList;

class MockConferenceInfoUpdater : public ConferenceInfoUpdater
{
public:
    explicit MockConferenceInfoUpdater(IN ConferenceFactory& objFactory) :
            ConferenceInfoUpdater(objFactory)
    {
    }
    ~MockConferenceInfoUpdater() {}
    MOCK_METHOD(IMS_UINT32, Update,
            (IN ConferenceParticipantList* pParticipantList, IN const AString& strEventPackage),
            (override));
#if 0
    MOCK_METHOD(IMS_RESULT, ParseConferenceInfo, (IN const AString& strEventPackage), ());
    MOCK_METHOD(IMS_RESULT, CheckValidVersion, (), (const));
    MOCK_METHOD(IMS_RESULT, UpdateDescription, (), ());
    MOCK_METHOD(IMS_RESULT, UpdateParticipantList, (), ());
    MOCK_METHOD(IMS_BOOL, FindAndUpdate, (IN IMS_UINT32 nPolicy), ());
    MOCK_METHOD(IMS_BOOL, UpdateParticipant, (
            IN ConferenceInfo :: User* pUser, IN IMS_SINT32 nParticipantIndex), ());
    MOCK_METHOD(void, SetParticipantsMatchingStarted, (), ());
    MOCK_METHOD(void, SetDeletedParticipantToDisconnected, (), ());
    MOCK_METHOD(IMS_SINT32, FindParticipant, (
            IN const ConferenceInfo :: User* pUser, IN IMS_UINT32 nIndexInXml), ());
    MOCK_METHOD(IMS_SINT32, FindParticipantByOrder, (
            IN IMS_UINT32 nIndexInXml, IN const ConferenceInfo :: User* pUser), ());
    MOCK_METHOD(IMS_SINT32, FindParticipantByOrderLegId, (
            IN const ConferenceInfo :: User* pUser), ());
    MOCK_METHOD(IMS_SINT32, FindParticipantByReferToUri, (
            IN const ConferenceInfo :: User* pUser), ());
    MOCK_METHOD(IMS_SINT32, FindParticipantByUserEntity, (
            IN const ConferenceInfo :: User* pUser), ());
    MOCK_METHOD(void, DetermineMatchingPolicy, (), ());
    MOCK_METHOD(IMS_UINT32, GetMatchingScore, (
            IN const AString& strUriA, IN const AString& strUriB), (const));
    MOCK_METHOD(IMS_UINT32, GetMatchingCount, (
            IN const AString& strUriA, IN const AString& strUriB), (const));
    MOCK_METHOD(void, Clear, (), ());
    MOCK_METHOD(IMS_BOOL, HasLegId, (IN const AString& strUserEntity), (const));
    MOCK_METHOD(IMS_BOOL, IsSameUri, (
            IN const AString& strUriA, IN const AString& strUriB, IN IMS_BOOL bAllowPrefix), (const));
    MOCK_METHOD(IMS_BOOL, IsLocalUri, (IN const AString& strUserEntity), (const));
    MOCK_METHOD(IMS_BOOL, IsAnonymousUri, (IN const AString& strUserEntity), (const));
    MOCK_METHOD(IMS_BOOL, IsSamePrivacyUri, (
            IN const AString& strUriA, IN const AString& strUriB), (const));
    MOCK_METHOD(IMS_BOOL, IsAllParticipantUpdated, (), (const));
    MOCK_METHOD(IMS_BOOL, IsInvalidStatusUpdate, (
            IN IMS_UINT32 nParticipantIndex, IN const ConferenceInfo :: User* pUser), (const));
    MOCK_METHOD(IMSList<ConferenceInfo :: User*>, GetSameUserEntities, (
            IN const ConferenceInfo :: User* pUser), (const));
    MOCK_METHOD(void, AddNotMatchedUserList, (IN ConferenceInfo :: User* pUser), ());
    MOCK_METHOD(void, RemoveFromNotMatchedUserList, (IN ConferenceInfo :: User* pUser), ());
    MOCK_METHOD(IMS_BOOL, IsInitialNotifyWithoutUsers, (), (const));
    MOCK_METHOD(IMS_BOOL, IsConnectedStatusCategory, (IN IMS_UINT32 nStatus), (const));
    MOCK_METHOD(const IMS_CHAR*, ConvertPolicyToString, (IN IMS_SINT32 nPolicy), (const));
    MOCK_METHOD(const IMS_CHAR*, ConvertStatusToString, (IN IMS_SINT32 nStatus), (const));
    MOCK_METHOD(void, ModifyParticipantInfoByConfig, (IN ConfUser* pConfUser), ());
#endif
};

#endif
