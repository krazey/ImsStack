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

#include "MtcDef.h"
#include "conferencecall/ConferenceInfo.h"

class ConferenceParticipantList;

class ConferenceInfoUpdater
{
public:
    explicit ConferenceInfoUpdater();
    virtual ~ConferenceInfoUpdater();
    ConferenceInfoUpdater(IN const ConferenceInfoUpdater&) = delete;
    ConferenceInfoUpdater& operator=(IN const ConferenceInfoUpdater&) = delete;

public:
    virtual IMS_UINT32 Update(
            IN ConferenceParticipantList* pParticipantList, IN const AString& strEventPackage);

protected:
    IMS_RESULT ParseConferenceInfo(IN const AString& strEventPackage);
    IMS_RESULT CheckValidVersion() const;
    IMS_RESULT UpdateDescription();
    IMS_RESULT UpdateParticipantList();
    IMS_BOOL FindAndUpdate(IN IMS_UINT32 nPolicy);
    IMS_BOOL UpdateParticipant(IN ConferenceInfo::User* pUser, IN IMS_SINT32 nParticipantIndex);
    void SetParticipantsMatchingStarted();
    void SetDeletedParticipantToDisconnected();

    IMS_SINT32 FindParticipant(IN const ConferenceInfo::User* pUser, IN IMS_UINT32 nIndexInXml);
    IMS_SINT32 FindParticipantByOrder(
            IN IMS_UINT32 nIndexInXml, IN const ConferenceInfo::User* pUser);
    IMS_SINT32 FindParticipantByOrderLegId(IN const ConferenceInfo::User* pUser);
    IMS_SINT32 FindParticipantByReferToUri(IN const ConferenceInfo::User* pUser);
    IMS_SINT32 FindParticipantByUserEntity(IN const ConferenceInfo::User* pUser);

    void DetermineMatchingPolicy();
    IMS_UINT32 GetMatchingScore(IN const AString& strUriA, IN const AString& strUriB) const;
    IMS_UINT32 GetMatchingCount(IN const AString& strUriA, IN const AString& strUriB) const;

    void Clear();

    IMS_BOOL HasLegId(IN const AString& strUserEntity) const;
    IMS_BOOL IsSameUri(IN const AString& strUriA, IN const AString& strUriB,
            IN IMS_BOOL bAllowPrefix = IMS_TRUE) const;
    IMS_BOOL IsLocalUri(IN const AString& strUserEntity) const;
    IMS_BOOL IsAnonymousUri(IN const AString& strUserEntity) const;
    IMS_BOOL IsSamePrivacyUri(IN const AString& strUriA, IN const AString& strUriB) const;
    IMS_BOOL IsAllParticipantUpdated() const;
    IMS_BOOL IsInvalidStatusUpdate(
            IN IMS_UINT32 nParticipantIndex, IN const ConferenceInfo::User* pUser) const;
    IMSList<ConferenceInfo::User*> GetSameUserEntities(IN const ConferenceInfo::User* pUser) const;
    void AddNotMatchedUserList(IN ConferenceInfo::User* pUser);
    void RemoveFromNotMatchedUserList(IN ConferenceInfo::User* pUser);
    IMS_BOOL IsInitialNotifyWithoutUsers() const;

    IMS_BOOL IsConnectedStatusCategory(IN IMS_UINT32 nStatus) const;
    const IMS_CHAR* ConvertPolicyToString(IN IMS_SINT32 nPolicy) const;
    const IMS_CHAR* ConvertStatusToString(IN IMS_SINT32 nStatus) const;

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

    enum
    {
        MATCH_POLICY_NOT_DEFINED = 0,
        MATCH_POLICY_ORDER_LEG_ID = 1,
        MATCH_POLICY_ORDER = 2,
        MATCH_POLICY_REFER_TO_URI = 3,
        MATCH_POLICY_USERENTITY = 4,
        MATCH_POLICY_INVALID_ANONYMOUS = 5
    };

private:
    ConferenceInfo* m_pConferenceInfo;
    ConferenceParticipantList* m_pParticipantList;
    IMS_UINT32 m_nInfoState;
    IMS_UINT32 m_nCurrentMatchPolicy;
    IMSList<ConferenceInfo::User*> m_objNotMatchedUsers;
    IMS_BOOL m_bHostInfoInUsers;
};

#endif
