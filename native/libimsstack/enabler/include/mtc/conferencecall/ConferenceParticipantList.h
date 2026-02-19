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

#ifndef CONFERENCE_PARTICIPANT_LIST_H_
#define CONFERENCE_PARTICIPANT_LIST_H_

#include "ImsList.h"
#include "ImsTypeDef.h"
#include "conferencecall/ConferenceDef.h"

class CallConnectionIdManager;
class IConferenceReference;
class IMtcCallManager;

class ConferenceParticipantList
{
public:
    struct ConferenceParticipant
    {
    public:
        inline ConferenceParticipant() :
                m_pConfUser(IMS_NULL),
                m_pIReference(IMS_NULL),
                m_strReferInviteUri(AString::ConstEmpty()),
                m_bInfoUpdated(IMS_FALSE),
                m_bMatchingCompleted(IMS_FALSE),
                m_bDisconnectionNotified(IMS_FALSE),
                m_bDisconnectedExplicitly(IMS_FALSE)
        {
        }
        ~ConferenceParticipant();
        ConferenceParticipant(IN const ConferenceParticipant&) = delete;
        ConferenceParticipant& operator=(IN const ConferenceParticipant&) = delete;

    public:
        inline void SetConfUser(IN const ConfUser* pConfUser)
        {
            m_pConfUser = new ConfUser(*pConfUser);
        }
        inline ConfUser* GetConfUser() { return m_pConfUser; }

        inline const AString& GetUserEntity() const { return m_pConfUser->strUserEntity; }

        inline void SetReference(IN IConferenceReference* piReference)
        {
            m_pIReference = piReference;
        }
        inline IConferenceReference* GetReference() { return m_pIReference; }

        inline void SetReferInviteUri(IN const AString& strReferInviteUri)
        {
            m_strReferInviteUri = strReferInviteUri;
        }
        inline const AString& GetReferInviteUri() const { return m_strReferInviteUri; }

        inline void SetInfoUpdated(IN IMS_BOOL bInfoUpdated) { m_bInfoUpdated = bInfoUpdated; }
        inline IMS_BOOL IsInfoUpdated() { return m_bInfoUpdated; }

        inline void SetMatchingCompleted(IN IMS_BOOL bMatchingCompleted)
        {
            m_bMatchingCompleted = bMatchingCompleted;
        }
        inline IMS_BOOL IsMatchingCompleted() { return m_bMatchingCompleted; }

        inline void SetDisconnectionNotified(IN IMS_BOOL bDisconnectionNotified)
        {
            m_bDisconnectionNotified = bDisconnectionNotified;
        }
        inline IMS_BOOL IsDisconnectionNotified() { return m_bDisconnectionNotified; }

        inline void SetDisconnectedExplicitly(IN IMS_BOOL bDisconnectedExplicitly)
        {
            m_bDisconnectedExplicitly = bDisconnectedExplicitly;
        }
        inline IMS_BOOL IsDisconnectedExplicitly() { return m_bDisconnectedExplicitly; }

        void LogLn() const;

    private:
        ConfUser* m_pConfUser;
        IConferenceReference* m_pIReference;
        AString m_strReferInviteUri;

        // set true once info is updated first time by C-NOTIFY
        IMS_BOOL m_bInfoUpdated;
        // set true every time info is updated by C-NOTIFY and set false after updating is done.
        IMS_BOOL m_bMatchingCompleted;
        // set true once "disconnected" or "disconnecting" info is notifed to ISIL
        IMS_BOOL m_bDisconnectionNotified;
        IMS_BOOL m_bDisconnectedExplicitly;
    };

public:
    explicit ConferenceParticipantList();
    virtual ~ConferenceParticipantList();
    ConferenceParticipantList(IN const ConferenceParticipantList&) = delete;
    ConferenceParticipantList& operator=(IN const ConferenceParticipantList&) = delete;

public:
    inline void SetLocalUri(IN const AString& strLocalUri) { m_strLocalUri = strLocalUri; }

    inline const AString& GetLocalUri() const { return m_strLocalUri; }

    inline void SetXmlVersion(IN IMS_SINT32 nVersion) { m_nVersion = nVersion; }

    inline IMS_SINT32 GetXmlVersion() const { return m_nVersion; }

    inline void SetMaxUserCount(IN IMS_SINT32 nMaxUserCount) { m_nMaxUserCount = nMaxUserCount; }

    inline virtual IMS_SINT32 GetMaxUserCount() const { return m_nMaxUserCount; }

    virtual void AddUser(IN const ConfUser* pConfUser);
    virtual void RemoveUser(IN const ConfUser* pConfUser);
    virtual void RemoveUser(IN IMS_UINT32 nIndex);
    virtual ConfUser* GetConfUser(IN IConferenceReference* piConfReference) const;

    virtual IMS_BOOL IsConnectedUser(
            IN const ConfUser* pConfUser, IN IMS_BOOL bIncludingConnecting = IMS_FALSE) const;

    // the order of this ConfUser is same with the order of REFER sent.
    virtual ImsList<ConfUser*> GetConfUsers(IN IMS_BOOL bCopy = IMS_FALSE) const;

    virtual void SetReference(IN IConferenceReference* piReference, IN const ConfUser* pConfUser);
    virtual IConferenceReference* GetReference(IN const ConfUser* pConfUser) const;
    virtual void ResetReference(IN IConferenceReference* piConfReference);

    virtual void SetReferInviteUri(
            IN const AString& strReferInviteUri, IN const ConfUser* pConfUser);
    virtual AString GetReferInviteUri(IN const ConfUser* pConfUser);

    virtual IMS_SINT32 FindParticipant(IN IMS_UINT32 nConnectionId);
    void ReOrder(IN IMtcCallManager& objCallManager,
            IN const CallConnectionIdManager& objConnectionIdManager);
    void LogLn() const;

    virtual inline IMS_UINT32 GetSize() const { return m_objParticipants.GetSize(); }

    virtual inline ConferenceParticipant* GetAt(IN IMS_UINT32 nAt)
    {
        return m_objParticipants.GetAt(nAt);
    }

    virtual ConfUser* GetConfUser(IN IMS_UINT32 nIndex) const;

    virtual IMS_UINT32 GetConnectedParticipantSize(IN IMS_BOOL bIncludingConnecting = IMS_FALSE);

private:
    IMS_SINT32 FindParticipant(IN const ConfUser* pConfUser) const;

private:
    friend class ConferenceInfoUpdater;

    // the order of this list is same as the order of sending REFER.
    ImsList<ConferenceParticipant*> m_objParticipants;

    // this is host uri in most cases. if participant subscribes, this is not a host.
    AString m_strLocalUri;
    IMS_SINT32 m_nVersion;
    IMS_UINT32 m_nMaxUserCount;
};

#endif
