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

#ifndef MOCK_CONFERENCE_INFO_H_
#define MOCK_CONFERENCE_INFO_H_

#include "AString.h"
#include "MtcDef.h"
#include "conferencecall/ConferenceInfo.h"
#include <gmock/gmock.h>

class ConferenceParticipantList;

class MockConferenceDescription : public ConferenceInfo::ConferenceDescription
{
public:
    explicit MockConferenceDescription(IN IMS_UINT32 nMaxUserCount) :
            m_nMaxUserCount(nMaxUserCount)
    {
    }
    ~MockConferenceDescription() {}
    inline IMS_UINT32 GetMaxUserCount() const override { return m_nMaxUserCount; }

private:
    IMS_UINT32 m_nMaxUserCount;
};

class MockEndPoint : public ConferenceInfo::User::EndPoint
{
public:
    explicit MockEndPoint(IN const AString& strEntity, IN IMS_UINT32 nState,
            IN const AString& strDisplayText, IN IMS_UINT32 nStatus) :
            m_strEntity(strEntity),
            m_nState(nState),
            m_strDisplayText(strDisplayText),
            m_nStatus(nStatus)
    {
    }
    ~MockEndPoint() {}
    inline const AString& GetEntity() const override { return m_strEntity; }
    inline IMS_UINT32 GetState() const override { return m_nState; }
    inline const AString& GetDisplayText() const override { return m_strDisplayText; }
    inline IMS_UINT32 GetStatus() const override { return m_nStatus; }

private:
    AString m_strEntity;
    IMS_UINT32 m_nState;
    AString m_strDisplayText;
    IMS_UINT32 m_nStatus;
};

class MockUser : public ConferenceInfo::User
{
public:
    explicit MockUser(IN const AString& strEntity, IN IMS_UINT32 nState,
            IN const AString& strDisplayText, IN const ImsList<EndPoint*>& objEndPoints) :
            m_strEntity(strEntity),
            m_nState(nState),
            m_strDisplayText(strDisplayText),
            m_objEndPoints(objEndPoints)
    {
    }
    ~MockUser() { m_objEndPoints.Clear(); }
    inline const AString& GetEntity() const override { return m_strEntity; }
    inline IMS_UINT32 GetState() const override { return m_nState; }
    inline const AString& GetDisplayText() const override { return m_strDisplayText; }
    inline const ImsList<EndPoint*>& GetEndPoints() const override { return m_objEndPoints; }

private:
    AString m_strEntity;
    IMS_UINT32 m_nState;
    AString m_strDisplayText;
    ImsList<EndPoint*> m_objEndPoints;
};

class MockConferenceInfo : public ConferenceInfo
{
public:
    explicit MockConferenceInfo(IN ConferenceInfo::ConferenceDescription& objDescripiption,
            IN ImsList<ConferenceInfo::User*>& objUsers, IN IMS_UINT32 nState,
            IN IMS_SINT32 nVersion) :
            m_objDescription(objDescripiption),
            m_objUsers(objUsers),
            m_nState(nState),
            m_nVersion(nVersion)
    {
    }
    ~MockConferenceInfo() { m_objUsers.Clear(); }
    inline const ConferenceDescription& GetConferenceDescription() const override
    {
        return m_objDescription;
    }
    inline const ImsList<User*>& GetUsers() const override { return m_objUsers; }
    inline IMS_UINT32 GetState() const override { return m_nState; }
    inline IMS_SINT32 GetVersion() const override { return m_nVersion; }
    MOCK_METHOD(IMS_BOOL, Parse, (const AString&), (override));

private:
    ConferenceInfo::ConferenceDescription& m_objDescription;
    ImsList<ConferenceInfo::User*>& m_objUsers;
    IMS_UINT32 m_nState;
    IMS_SINT32 m_nVersion;
};

#endif
