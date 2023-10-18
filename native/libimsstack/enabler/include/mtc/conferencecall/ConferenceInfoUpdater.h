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

#ifndef CONFERENCE_INFO_UPDATER_H_
#define CONFERENCE_INFO_UPDATER_H_

#include "ImsList.h"
#include "MtcDef.h"
#include "conferencecall/ConferenceInfo.h"

class ConferenceFactory;
class ConferenceParticipantList;
class MtcConfigurationProxy;

enum class MatchingPolicy
{
    ORDER_LEG_ID = 0,
    ORDER = 1,
    REFER_TO_URI = 2,
    USERENTITY = 3
};

class ConferenceInfoUpdater
{
public:
    explicit ConferenceInfoUpdater(
            IN ConferenceFactory& objFactory, IN MtcConfigurationProxy& objConfigProxy);
    virtual ~ConferenceInfoUpdater();
    ConferenceInfoUpdater(IN const ConferenceInfoUpdater&) = delete;
    ConferenceInfoUpdater& operator=(IN const ConferenceInfoUpdater&) = delete;

public:
    virtual IMS_UINT32 Update(
            IN ConferenceParticipantList* pParticipantList, IN const AString& strEventPackage);

    static const IMS_CHAR* ConvertPolicyToString(IN MatchingPolicy ePolicy);
    static const IMS_CHAR* ConvertStatusToString(IN IMS_SINT32 nStatus);

protected:
    IMS_RESULT ParseConferenceInfo(IN const AString& strEventPackage);
    IMS_RESULT CheckValidVersion() const;
    IMS_RESULT UpdateDescription();
    IMS_RESULT UpdateParticipantList();
    IMS_BOOL FindAndUpdate(IN MatchingPolicy ePolicy);
    IMS_BOOL UpdateParticipant(IN ConferenceInfo::User* pUser, IN IMS_SINT32 nParticipantIndex);
    void SetParticipantsMatchingStarted();
    void SetDeletedParticipantToDisconnected();

    IMS_SINT32 FindParticipant(IN const ConferenceInfo::User* pUser, IN IMS_UINT32 nIndexInXml);
    IMS_SINT32 FindParticipantByOrder(
            IN IMS_UINT32 nIndexInXml, IN const ConferenceInfo::User* pUser);
    IMS_SINT32 FindParticipantByOrderLegId(IN const ConferenceInfo::User* pUser);
    IMS_SINT32 FindParticipantByReferToUri(IN const ConferenceInfo::User* pUser);
    IMS_SINT32 FindParticipantByUserEntity(IN const ConferenceInfo::User* pUser);

    static IMS_UINT32 GetMatchingScore(IN const AString& strUriA, IN const AString& strUriB);
    static IMS_UINT32 GetMatchingCount(IN const AString& strUriA, IN const AString& strUriB);

    void Clear();

    static IMS_BOOL IsSameUri(IN const AString& strUriA, IN const AString& strUriB,
            IN IMS_BOOL bAllowPrefix = IMS_TRUE);
    IMS_BOOL IsLocalUri(IN const AString& strUserEntity) const;
    static IMS_BOOL IsAnonymousUri(IN const AString& strUserEntity);
    static IMS_BOOL IsSamePrivacyUri(IN const AString& strUriA, IN const AString& strUriB);
    IMS_BOOL IsInvalidStatusUpdate(
            IN IMS_UINT32 nParticipantIndex, IN const ConferenceInfo::User* pUser) const;
    ImsList<ConferenceInfo::User*> GetSameUserEntities(IN const ConferenceInfo::User* pUser) const;
    void AddNotMatchedUserList(IN ConferenceInfo::User* pUser);
    void RemoveFromNotMatchedUserList(IN ConferenceInfo::User* pUser);
    IMS_BOOL IsInitialNotifyWithoutUsers() const;

    static IMS_BOOL IsConnectedStatusCategory(IN IMS_UINT32 nStatus);

private:
    void ModifyParticipantInfoByConfig(IN ConfUser* pConfUser);

public:
    enum
    {
        RESULT_UPDATED = 0,
        RESULT_MALFORMED_XML = 1,
        RESULT_NOTHING_UPDATED = 2,  // ambiguous??
        RESULT_INVALID_VERSION = 3,  // ambiguous??
        RESULT_INFO_DELETED = 4,
        RESULT_AMBIGUOUS = 5
    };

private:
    MtcConfigurationProxy& m_objConfigProxy;
    ConferenceInfo* m_pConferenceInfo;
    ConferenceFactory& m_objFactory;
    ConferenceParticipantList* m_pParticipantList;
    IMS_UINT32 m_nInfoState;
    MatchingPolicy m_eCurrentMatchPolicy;
    ImsList<ConferenceInfo::User*> m_objNotMatchedUsers;
    IMS_BOOL m_bHostInfoInUsers;
};

#endif
