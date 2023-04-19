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

#include "AStringBuffer.h"
#include "ServiceTrace.h"
#include "call/CallConnectionIdManager.h"
#include "call/IMtcCallManager.h"
#include "conferencecall/ConferenceParticipantList.h"

__IMS_TRACE_TAG_COM_MTC__;

PRIVATE
ConferenceParticipantList::ConferenceParticipantList() :
        m_objParticipants(ImsList<ConferenceParticipant*>()),
        m_strLocalUri(AString::ConstNull()),
        m_nVersion(-1),
        m_nMaxUserCount(0)
{
    IMS_TRACE_D("+ConferenceParticipantList", 0, 0, 0);
}

PUBLIC
ConferenceParticipantList::~ConferenceParticipantList()
{
    IMS_TRACE_D("~ConferenceParticipantList", 0, 0, 0);

    for (IMS_UINT32 i = 0; i < m_objParticipants.GetSize(); i++)
    {
        delete m_objParticipants.GetAt(i);
    }

    m_objParticipants.Clear();
}

PUBLIC
ConferenceParticipantList::ConferenceParticipant::~ConferenceParticipant()
{
    IMS_TRACE_D("~ConferenceParticipant", 0, 0, 0);

    if (m_pConfUser != IMS_NULL)
    {
        delete m_pConfUser;
    }
}

PUBLIC
void ConferenceParticipantList::ConferenceParticipant::Login()
{
    AStringBuffer objBuffer(256);
    objBuffer.Append("ConnectionId=");
    objBuffer.Append(m_pConfUser->nConnectionId);
    objBuffer.Append(" Target=");
    objBuffer.Append(m_pConfUser->strTarget);
    objBuffer.Append(" UserEntity=");
    objBuffer.Append(m_pConfUser->strUserEntity);
    objBuffer.Append(" EPEntity=");
    objBuffer.Append(m_pConfUser->strEpEntity);
    objBuffer.Append(" DisplayName=");
    objBuffer.Append(m_pConfUser->strDisplayName);
    objBuffer.Append(" Status=");
    objBuffer.Append(m_pConfUser->eStatus);

    IMS_TRACE_I("Login : %s", objBuffer.GetString().GetStr(), 0, 0);
}

PUBLIC
void ConferenceParticipantList::AddUser(IN const ConfUser* pConfUser)
{
    ConferenceParticipant* pParticipant = new ConferenceParticipant();
    m_objParticipants.Append(pParticipant);
    pParticipant->SetConfUser(pConfUser);  // TODO: CopyConfUser
}

PUBLIC
void ConferenceParticipantList::RemoveUser(IN const ConfUser* pConfUser)
{
    IMS_TRACE_I("RemoveUser", 0, 0, 0);

    IMS_SINT32 nIndex = FindParticipant(pConfUser);
    if (nIndex >= 0)
    {
        delete m_objParticipants.GetAt(nIndex);
        m_objParticipants.RemoveAt(nIndex);
    }
}

PUBLIC
void ConferenceParticipantList::RemoveUser(IN IMS_UINT32 nIndex)
{
    IMS_TRACE_I("RemoveUser", 0, 0, 0);
    delete m_objParticipants.GetAt(nIndex);
    m_objParticipants.RemoveAt(nIndex);
}

PUBLIC
ConfUser* ConferenceParticipantList::GetConfUser(IN IConferenceReference* piConfReference) const
{
    IMS_TRACE_I("GetConfUser", 0, 0, 0);

    for (IMS_SINT32 i = 0; i < static_cast<IMS_SINT32>(m_objParticipants.GetSize()); i++)
    {
        ConferenceParticipant* pTemp = m_objParticipants.GetAt(i);
        if (pTemp->GetReference() == piConfReference)
        {
            return pTemp->GetConfUser();
        }
    }

    return IMS_NULL;
}

PUBLIC
IMS_BOOL ConferenceParticipantList::IsConnectedUser(
        IN const ConfUser* pConfUser, IN IMS_BOOL bIncludingConnecting /* = IMS_FALSE*/) const
{
    if (pConfUser == IMS_NULL)
    {
        return IMS_FALSE;
    }

    if (pConfUser->eStatus == STATUS_CONNECTED || pConfUser->eStatus == STATUS_ON_HOLD)
    {
        return IMS_TRUE;
    }

    if (bIncludingConnecting)
    {
        if (pConfUser->eStatus == STATUS_IDLE || pConfUser->eStatus == STATUS_PENDING ||
                pConfUser->eStatus == STATUS_ALERTING || pConfUser->eStatus == STATUS_DIALING_IN ||
                pConfUser->eStatus == STATUS_DIALING_OUT)
        {
            return IMS_TRUE;
        }
    }

    return IMS_FALSE;
}

PUBLIC
ImsList<ConfUser*> ConferenceParticipantList::GetConfUsers(IN IMS_BOOL bCopy /* = IMS_FALSE*/) const
{
    ImsList<ConfUser*> objTempUsers;
    for (IMS_UINT32 i = 0; i < m_objParticipants.GetSize(); i++)
    {
        if (bCopy == IMS_TRUE)
        {
            ConfUser* pUser = new ConfUser(*(m_objParticipants.GetAt(i)->GetConfUser()));
            objTempUsers.Append(pUser);
        }
        else
        {
            objTempUsers.Append(m_objParticipants.GetAt(i)->GetConfUser());
        }
    }

    return objTempUsers;
}

PUBLIC
void ConferenceParticipantList::SetReference(
        IN IConferenceReference* piReference, IN const ConfUser* pConfUser)
{
    ConferenceParticipant* pTemp = m_objParticipants.GetAt(FindParticipant(pConfUser));
    if (pTemp != IMS_NULL)
    {
        pTemp->SetReference(piReference);
    }
}

PUBLIC
IConferenceReference* ConferenceParticipantList::GetReference(IN const ConfUser* pConfUser) const
{
    ConferenceParticipant* pTemp = m_objParticipants.GetAt(FindParticipant(pConfUser));
    if (pTemp != IMS_NULL)
    {
        return pTemp->GetReference();
    }
    return IMS_NULL;
}

PUBLIC
void ConferenceParticipantList::ResetReference(IN IConferenceReference* piConfReference)
{
    for (IMS_SINT32 i = 0; i < static_cast<IMS_SINT32>(m_objParticipants.GetSize()); i++)
    {
        ConferenceParticipant* pTemp = m_objParticipants.GetAt(i);
        if (pTemp->GetReference() == piConfReference)
        {
            pTemp->SetReference(IMS_NULL);
        }
    }
}

PUBLIC
void ConferenceParticipantList::SetReferInviteUri(
        IN const AString& strReferInviteUri, IN const ConfUser* pConfUser)
{
    ConferenceParticipant* pTemp = m_objParticipants.GetAt(FindParticipant(pConfUser));
    if (pTemp != IMS_NULL)
    {
        pTemp->SetReferInviteUri(strReferInviteUri);
    }
}

PUBLIC
AString ConferenceParticipantList::GetReferInviteUri(IN const ConfUser* pConfUser)
{
    ConferenceParticipant* pTemp = m_objParticipants.GetAt(FindParticipant(pConfUser));
    if (pTemp != IMS_NULL)
    {
        return pTemp->GetReferInviteUri();
    }
    return AString::ConstEmpty();
}

PUBLIC
IMS_SINT32 ConferenceParticipantList::FindParticipant(IN IMS_UINT32 nConnectionId)
{
    for (IMS_SINT32 i = 0; i < static_cast<IMS_SINT32>(m_objParticipants.GetSize()); i++)
    {
        if (m_objParticipants.GetAt(i)->GetConfUser()->nConnectionId == nConnectionId)
        {
            return i;
        }
    }

    return -1;
}

PUBLIC
void ConferenceParticipantList::ReOrder(IN IMtcCallManager& objCallManager,
        IN const CallConnectionIdManager& objConnectionIdManager)
{
    ImsList<ConferenceParticipant*> objTemp;

    ImsList<IMtcCall*> objCalls = objCallManager.GetCalls();
    for (IMS_UINT32 nSessIndex = 0; nSessIndex < objCalls.GetSize(); nSessIndex++)
    {
        IMtcCall* piTempCall = objCalls.GetAt(nSessIndex);

        for (IMS_UINT32 nIndex = 0; nIndex < m_objParticipants.GetSize(); nIndex++)
        {
            ConferenceParticipant* pTempParticipant = m_objParticipants.GetAt(nIndex);
            ConfUser* pTempUser = pTempParticipant ? pTempParticipant->GetConfUser() : IMS_NULL;
            if (pTempUser &&
                    objConnectionIdManager.GetCallKey(pTempUser->nConnectionId) ==
                            piTempCall->GetKey())
            {
                objTemp.Append(pTempParticipant);
                break;
            }
        }
    }

    m_objParticipants.Clear();
    for (IMS_UINT32 nIndex = 0; nIndex < objTemp.GetSize(); nIndex++)
    {
        m_objParticipants.Append(objTemp.GetAt(nIndex));
    }
    objTemp.Clear();
}

PUBLIC
void ConferenceParticipantList::Login()
{
    for (IMS_UINT32 i = 0; i < m_objParticipants.GetSize(); i++)
    {
        m_objParticipants.GetAt(i)->Login();
    }
}

PUBLIC
ConfUser* ConferenceParticipantList::GetConfUser(IN IMS_UINT32 nIndex) const
{
    if (nIndex >= GetSize())
    {
        return IMS_NULL;
    }

    return m_objParticipants.GetAt(nIndex)->GetConfUser();
}

PUBLIC
IMS_UINT32 ConferenceParticipantList::GetConnectedParticipantSize(
        IN IMS_BOOL bIncludingConnecting /* = IMS_FALSE*/)
{
    IMS_UINT32 nCount = 0;
    ImsList<ConfUser*> objConfUsers = GetConfUsers();
    for (IMS_UINT32 i = 0; i < objConfUsers.GetSize(); i++)
    {
        if (IsConnectedUser(objConfUsers.GetAt(i), bIncludingConnecting))
        {
            nCount++;
        }
    }

    return nCount;
}

PRIVATE
IMS_SINT32 ConferenceParticipantList::FindParticipant(IN const ConfUser* pConfUser) const
{
    for (IMS_SINT32 i = 0; i < static_cast<IMS_SINT32>(m_objParticipants.GetSize()); i++)
    {
        if (m_objParticipants.GetAt(i)->GetConfUser() == pConfUser)
        {
            return i;
        }
    }

    return -1;
}
