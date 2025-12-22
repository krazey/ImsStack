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

#include "ServiceTrace.h"
#include "SipAddress.h"
#include "SipParameter.h"
#include "conferencecall/ConferenceConfigurationHelper.h"
#include "conferencecall/ConferenceConst.h"
#include "conferencecall/ConferenceDef.h"
#include "conferencecall/ConferenceFactory.h"
#include "conferencecall/ConferenceInfoUpdater.h"
#include "conferencecall/ConferenceParticipantList.h"
#include "configuration/MtcConfigurationProxy.h"
#include "utility/IMessageUtils.h"
#include "algorithm"
#include <vector>

__IMS_TRACE_TAG_COM_MTC__;

PUBLIC
ConferenceInfoUpdater::ConferenceInfoUpdater(IN ConferenceFactory& objFactory,
        IN MtcConfigurationProxy& objConfigProxy, IN IMessageUtils& objMessageUtils) :
        m_objConfigProxy(objConfigProxy),
        m_objMessageUtils(objMessageUtils),
        m_pConferenceInfo(IMS_NULL),
        m_objFactory(objFactory),
        m_pParticipantList(IMS_NULL),
        m_nInfoState(ConferenceInfo::STATE_INVALID),
        m_eCurrentMatchPolicy(MatchingPolicy::USERENTITY),
        m_bHostInfoInUsers(IMS_FALSE)
{
    IMS_TRACE_I("+ConferenceInfoUpdater", 0, 0, 0);
}

PUBLIC VIRTUAL ConferenceInfoUpdater::~ConferenceInfoUpdater()
{
    IMS_TRACE_I("~ConferenceInfoUpdater", 0, 0, 0);

    Clear();
}

PUBLIC
IMS_UINT32 ConferenceInfoUpdater::Update(
        IN ConferenceParticipantList* pParticipantList, IN const AString& strEventPackage)
{
    IMS_TRACE_D("Update", 0, 0, 0);

    if (ParseConferenceInfo(strEventPackage) == IMS_FAILURE)
    {
        IMS_TRACE_E(0, "Malformed XML", 0, 0, 0);
        Clear();
        return RESULT_MALFORMED_XML;
    }

    m_nInfoState = m_pConferenceInfo->GetState();

    if (m_nInfoState == ConferenceInfo::STATE_DELETED)
    {
        Clear();
        return RESULT_INFO_DELETED;
    }

    m_pParticipantList = pParticipantList;

    if (CheckValidVersion() == IMS_FAILURE)
    {
        IMS_TRACE_E(0, "Invalid Version", 0, 0, 0);

        Clear();
        return RESULT_INVALID_VERSION;
    }

    IMS_RESULT eUserUpdateResult = UpdateParticipantList();

    // update xml version and max-user-count value.
    UpdateDescription();
    Clear();

    return eUserUpdateResult == IMS_SUCCESS ? RESULT_UPDATED : RESULT_NOTHING_UPDATED;
}

PUBLIC GLOBAL const IMS_CHAR* ConferenceInfoUpdater::ConvertPolicyToString(
        IN MatchingPolicy ePolicy)
{
    switch (ePolicy)
    {
        case MatchingPolicy::ORDER_LEG_ID:
            return "ORDER_LEG_ID";
        case MatchingPolicy::ORDER:
            return "ORDER";
        case MatchingPolicy::REFER_TO_URI:
            return "REFER_TO_URI";
        default:  // MatchingPolicy::USERENTITY:
            return "USERENTITY";
    }
}

PUBLIC GLOBAL const IMS_CHAR* ConferenceInfoUpdater::ConvertStatusToString(IN IMS_SINT32 nStatus)
{
    switch (nStatus)
    {
        case STATUS_CONNECTED:
            return ConferenceConst::STR_STATUS_CONNECTED;
        case STATUS_DISCONNECTED:
            return ConferenceConst::STR_STATUS_DISCONNECTED;
        case STATUS_ON_HOLD:
            return ConferenceConst::STR_STATUS_ON_HOLD;
        case STATUS_MUTED_VIA_FOCUS:
            return ConferenceConst::STR_STATUS_MUTED_VIA_FOCUS;
        case STATUS_PENDING:
            return ConferenceConst::STR_STATUS_PENDING;
        case STATUS_ALERTING:
            return ConferenceConst::STR_STATUS_ALERTING;
        case STATUS_DIALING_IN:
            return ConferenceConst::STR_STATUS_DIALING_IN;
        case STATUS_DIALING_OUT:
            return ConferenceConst::STR_STATUS_DIALING_OUT;
        case STATUS_DISCONNECTING:
            return ConferenceConst::STR_STATUS_DISCONNECTING;
        case STATUS_FAIL:
            return ConferenceConst::STR_STATUS_CONNECT_FAIL;

        default:
            return "__STATUS_IDLE__";
    }
}

PRIVATE
void ConferenceInfoUpdater::Clear()
{
    delete m_pConferenceInfo;
    m_pConferenceInfo = IMS_NULL;

    m_nInfoState = ConferenceInfo::STATE_INVALID;
    m_objNotMatchedUsers.Clear();
    m_bHostInfoInUsers = IMS_FALSE;
}

PRIVATE
IMS_RESULT ConferenceInfoUpdater::ParseConferenceInfo(IN const AString& strEventPackage)
{
    m_pConferenceInfo = m_objFactory.CreateInfo();
    IMS_BOOL bResult = m_pConferenceInfo->Parse(strEventPackage);

    return bResult ? IMS_SUCCESS : IMS_FAILURE;
}

PRIVATE
IMS_RESULT ConferenceInfoUpdater::CheckValidVersion() const
{
    if (ConferenceConfigurationHelper::IsPackageVersionCheckRequired(m_objConfigProxy) == IMS_FALSE)
    {
        return IMS_SUCCESS;
    }

    IMS_SINT32 nPreviousXmlVersion = m_pParticipantList->GetXmlVersion();
    if (nPreviousXmlVersion < 0)
    {
        return IMS_SUCCESS;
    }

    IMS_SINT32 nVersion = m_pConferenceInfo->GetVersion();
    if (m_nInfoState == ConferenceInfo::STATE_PARTIAL)
    {
        if (nPreviousXmlVersion + 1 == nVersion)
        {
            return IMS_SUCCESS;
        }
    }
    else
    {
        if (nPreviousXmlVersion < nVersion)
        {
            return IMS_SUCCESS;
        }
    }

    return IMS_FAILURE;
}

PRIVATE
IMS_RESULT ConferenceInfoUpdater::UpdateDescription()
{
    m_pParticipantList->SetXmlVersion((IMS_SINT32)m_pConferenceInfo->GetVersion());
    m_pParticipantList->SetMaxUserCount(
            m_pConferenceInfo->GetConferenceDescription().GetMaxUserCount());

    return IMS_SUCCESS;
}

PRIVATE
IMS_RESULT ConferenceInfoUpdater::UpdateParticipantList()
{
    if (IsInitialNotifyWithoutUsers())
    {
        IMS_TRACE_I("UpdateParticipantList : initial NOTIFY without users", 0, 0, 0);
        return IMS_FAILURE;
    }

    SetParticipantsMatchingStarted();

    std::vector<MatchingPolicy> objPolicies{
            MatchingPolicy::USERENTITY, MatchingPolicy::REFER_TO_URI, MatchingPolicy::ORDER};

    IMS_BOOL bFound = std::any_of(objPolicies.begin(), objPolicies.end(),
            [&](MatchingPolicy ePolicy)
            {
                return FindAndUpdate(ePolicy);
            });

    if (!bFound)
    {
        IMS_TRACE_E(0, "UpdateParticipantList : No update found.", 0, 0, 0);
    }

    if (m_nInfoState == ConferenceInfo::STATE_FULL)
    {
        SetDeletedParticipantToDisconnected();
    }

    return IMS_SUCCESS;
}

PRIVATE
IMS_BOOL ConferenceInfoUpdater::FindAndUpdate(IN MatchingPolicy ePolicy)
{
    IMS_TRACE_I("FindAndUpdate : policy[%s] - START", ConvertPolicyToString(ePolicy), 0, 0);

    m_eCurrentMatchPolicy = ePolicy;
    IMS_BOOL bCompleted = IMS_TRUE;

    ImsList<ConferenceInfo::User*> objUsers;
    if (m_objNotMatchedUsers.IsEmpty())
    {
        // initial attempt case.
        objUsers = m_pConferenceInfo->GetUsers();
    }
    else
    {
        // subsequential attempt cases.
        objUsers = m_objNotMatchedUsers;
    }

    for (IMS_UINT32 i = 0; i < objUsers.GetSize(); i++)
    {
        ConferenceInfo::User* pUser = objUsers.GetAt(i);

        IMS_SINT32 nIndex = FindParticipant(pUser, i);

        if (IsLocalUri(pUser->GetEntity()))
        {
            continue;
        }

        if (nIndex == -1)
        {
            bCompleted = IMS_FALSE;
            AddNotMatchedUserList(pUser);
        }
        else
        {
            AString strUserEntityXml = m_objMessageUtils.GetUserPart(pUser->GetEntity());
            AString strReferTo = m_objMessageUtils.GetUserPart(
                    m_pParticipantList->GetAt(nIndex)->GetReferInviteUri());
            IMS_TRACE_I("FindAndUpdate : index[%d] status[%s]", nIndex,
                    ConvertStatusToString(pUser->GetEndPoints().GetAt(0)->GetStatus()), 0);
            IMS_TRACE_I("FindAndUpdate : Entity[%s]=[%s]Refer-To", strUserEntityXml.GetStr(),
                    strReferTo.GetStr(), 0);
            RemoveFromNotMatchedUserList(pUser);
            UpdateParticipant(pUser, nIndex);
        }
    }

    // For 'Joined & subscription' case.
    if ((m_eCurrentMatchPolicy == MatchingPolicy::ORDER) &&
            (m_pConferenceInfo->GetState() == ConferenceInfo::STATE_FULL) &&
            ConferenceConfigurationHelper::IsSubscriptionForParticipantRequired(m_objConfigProxy))
    {
        IMS_TRACE_I("FindAndUpdate : subscription by participant case [%d]",
                m_objNotMatchedUsers.GetSize(), 0, 0);
        for (IMS_SINT32 index = static_cast<IMS_SINT32>(m_objNotMatchedUsers.GetSize()) - 1;
                index >= 0; index--)
        {
            const ConferenceInfo::User* pNotMatchedUser = m_objNotMatchedUsers.GetAt(index);
            RemoveFromNotMatchedUserList(pNotMatchedUser);
            UpdateParticipant(pNotMatchedUser, -1);
        }

        if (m_objNotMatchedUsers.GetSize() == 0)
        {
            bCompleted = IMS_TRUE;
        }
    }

    IMS_TRACE_I("FindAndUpdate : policy[%s] complete[%s] - END", ConvertPolicyToString(ePolicy),
            _TRACE_B_(bCompleted), 0);
    return bCompleted;
}

PRIVATE
IMS_BOOL ConferenceInfoUpdater::UpdateParticipant(
        IN const ConferenceInfo::User* pUser, IN IMS_SINT32 nParticipantIndex)
{
    // update nParticipantIndex-th participant in the list using pUser.
    if (nParticipantIndex >= 0 &&
            m_pParticipantList->GetSize() <= DYNAMIC_CAST(IMS_UINT32, nParticipantIndex))
    {
        return IMS_FALSE;
    }

    if (nParticipantIndex < 0)
    {
        // An already disconnected/deleted user can be remained in the CEP.
        // Or, a real new user can be added through the CEP.
        IMS_TRACE_D("UpdateParticipant : New user is added via CEP.", 0, 0, 0);
        nParticipantIndex = m_pParticipantList->GetSize();
        ConfUser* pNewUserAddedByCep = new ConfUser();
        m_pParticipantList->AddUser(pNewUserAddedByCep);
        delete pNewUserAddedByCep;
    }

    ConferenceParticipantList::ConferenceParticipant* pParticipant =
            m_pParticipantList->GetAt(DYNAMIC_CAST(IMS_UINT32, nParticipantIndex));

    pParticipant->SetInfoUpdated(IMS_TRUE);
    pParticipant->SetMatchingCompleted(IMS_TRUE);

    ConfUser* pConfUser = pParticipant->GetConfUser();

    AString strUserEntity = m_objMessageUtils.GetUserPart(pUser->GetEntity());
    pConfUser->strTarget = strUserEntity;
    pConfUser->strUserEntity = pUser->GetEntity();
    pConfUser->strEpEntity = pUser->GetEndPoints().GetAt(0)->GetEntity();
    pConfUser->eStatus = pUser->GetEndPoints().GetAt(0)->GetStatus();

    if (pConfUser->eStatus == STATUS_DISCONNECTED)
    {
        pParticipant->SetDisconnectedExplicitly(IMS_TRUE);
    }

    if (pConfUser->eStatus == STATUS_DISCONNECTING)
    {
        pConfUser->eStatus = STATUS_DISCONNECTED;
    }

    return IMS_TRUE;
}

PRIVATE
void ConferenceInfoUpdater::SetParticipantsMatchingStarted()
{
    for (IMS_UINT32 i = 0; i < m_pParticipantList->GetSize(); i++)
    {
        ConferenceParticipantList::ConferenceParticipant* pParticipant =
                m_pParticipantList->GetAt(i);
        pParticipant->SetMatchingCompleted(IMS_FALSE);
    }
}

PRIVATE
void ConferenceInfoUpdater::SetDeletedParticipantToDisconnected()
{
    // set status to disconnected if the participant in not in the user list in full state info.
    for (IMS_UINT32 i = 0; i < m_pParticipantList->GetSize(); i++)
    {
        ConferenceParticipantList::ConferenceParticipant* pParticipant =
                m_pParticipantList->GetAt(i);

        if (pParticipant->IsInfoUpdated() == IMS_TRUE &&
                pParticipant->IsMatchingCompleted() == IMS_FALSE)
        {
            IMS_TRACE_I("SetDeletedParticipantToDisconnected : [%s] deleted",
                    pParticipant->GetUserEntity().GetStr(), 0, 0);
            pParticipant->GetConfUser()->eStatus = STATUS_DISCONNECTED;
            pParticipant->SetDisconnectedExplicitly(IMS_TRUE);
            pParticipant->SetMatchingCompleted(IMS_TRUE);
        }
    }
}

PRIVATE
IMS_SINT32 ConferenceInfoUpdater::FindParticipant(
        IN const ConferenceInfo::User* pUser, IN IMS_UINT32 nIndexInXml)
{
    switch (m_eCurrentMatchPolicy)
    {
        case MatchingPolicy::ORDER_LEG_ID:
            return FindParticipantByOrderLegId(pUser);

        case MatchingPolicy::ORDER:
            return FindParticipantByOrder(nIndexInXml, pUser);

        case MatchingPolicy::USERENTITY:
            return FindParticipantByUserEntity(pUser);

        default:  // MatchingPolicy::REFER_TO_URI:
            return FindParticipantByReferToUri(pUser);
    }
}

PRIVATE
IMS_SINT32 ConferenceInfoUpdater::FindParticipantByOrder(
        IN IMS_UINT32 nIndexInXml, IN const ConferenceInfo::User* pUser)
{
    const AString& strUserEntity = pUser->GetEntity();
    IMS_TRACE_I("FindParticipantByOrder : user-entity=[%s]", strUserEntity.GetStr(), 0, 0);

    if (!m_bHostInfoInUsers && IsLocalUri(pUser->GetEntity()))
    {
        m_bHostInfoInUsers = IMS_TRUE;
        return -1;
    }

    if (m_bHostInfoInUsers)
    {
        nIndexInXml--;
    }

    IMS_UINT32 nLocalSize = m_pParticipantList->GetSize();

    if (nLocalSize <= nIndexInXml)
    {
        return -1;
    }

    for (IMS_UINT32 i = 0; i < nLocalSize; i++)
    {
        if (IsInvalidStatusUpdate(i, pUser))
        {
            continue;
        }

        if (m_pParticipantList->GetAt(i)->GetUserEntity().GetLength() > 0)
        {
            continue;
        }

        if (m_pParticipantList->GetAt(i)->IsInfoUpdated() == IMS_FALSE)
        {
            return i;
        }
    }

    return -1;
}

PRIVATE
IMS_SINT32 ConferenceInfoUpdater::FindParticipantByOrderLegId(IN const ConferenceInfo::User* pUser)
{
    const AString& strUserEntity = pUser->GetEntity();
    IMS_TRACE_I("FindParticipantByOrderLegId : user-entity=[%s]", strUserEntity.GetStr(), 0, 0);

    SipAddress objSIPAddress(strUserEntity);
    const SipParameter* pParameter = objSIPAddress.GetParameter(ConferenceConst::LEG_ID);

    if (pParameter != IMS_NULL)
    {
        IMS_BOOL bOk = IMS_FALSE;
        IMS_UINT32 nLegId = pParameter->GetValue().ToUInt32(&bOk);
        if (bOk)
        {
            return nLegId - 1;
        }
    }
    return -1;
}

PRIVATE
IMS_SINT32 ConferenceInfoUpdater::FindParticipantByReferToUri(IN const ConferenceInfo::User* pUser)
{
    IMS_UINT32 nMaxMatchingCount = 0;
    IMS_SINT32 nLocalParticipantIndex = -1;
    AString strUserEntity = m_objMessageUtils.GetUserPart(pUser->GetEntity());
    IMS_TRACE_I("FindParticipantByReferToUri : user-entity=[%s]", strUserEntity.GetStr(), 0, 0);

    for (IMS_UINT32 i = 0; i < m_pParticipantList->GetSize(); i++)
    {
        if (m_pParticipantList->GetAt(i)->IsMatchingCompleted() == IMS_TRUE)
        {
            continue;
        }

        if (m_pParticipantList->GetAt(i)->GetUserEntity().GetLength() > 0)
        {
            continue;
        }

        AString strReferToUri =
                m_objMessageUtils.GetUserPart(m_pParticipantList->GetAt(i)->GetReferInviteUri());

        if (!IsSamePrivacyUri(strReferToUri, strUserEntity))
        {
            continue;
        }

        if (IsInvalidStatusUpdate(i, pUser))
        {
            continue;
        }

        IMS_UINT32 nCount = GetMatchingCount(strReferToUri, strUserEntity, m_objMessageUtils);
        if (nCount > 0 && nCount >= nMaxMatchingCount)
        {
            nLocalParticipantIndex = i;
            nMaxMatchingCount = nCount;
        }
    }

    if (nLocalParticipantIndex != -1)
    {
        return nLocalParticipantIndex;
    }

    // to match anonymous uris using Matching "Score".
    IMS_UINT32 nMaxMatchingScore = 0;
    for (IMS_UINT32 i = 0; i < m_pParticipantList->GetSize(); i++)
    {
        if (m_pParticipantList->GetAt(i)->IsMatchingCompleted() == IMS_TRUE ||
                m_pParticipantList->GetAt(i)->IsInfoUpdated() == IMS_TRUE)
        {
            continue;
        }

        if (IsInvalidStatusUpdate(i, pUser))
        {
            continue;
        }

        AString strReferToUri =
                m_objMessageUtils.GetUserPart(m_pParticipantList->GetAt(i)->GetReferInviteUri());
        if (!IsSamePrivacyUri(strReferToUri, strUserEntity))
        {
            continue;
        }

        IMS_UINT32 nScore = GetMatchingScore(strReferToUri, strUserEntity);
        IMS_TRACE_I(
                "FindParticipantByReferToUri : %s score=[%d]", strReferToUri.GetStr(), nScore, 0);
        if (nScore > nMaxMatchingScore)
        {
            nLocalParticipantIndex = i;
            nMaxMatchingScore = nScore;
        }
        else if (nScore == nMaxMatchingScore)
        {
            nLocalParticipantIndex = -1;
        }
    }

    return nLocalParticipantIndex;
}

PRIVATE
IMS_SINT32 ConferenceInfoUpdater::FindParticipantByUserEntity(IN const ConferenceInfo::User* pUser)
{
    IMS_SINT32 nLocalParticipantIndex = -1;
    AString strUserEntity = m_objMessageUtils.GetUserPart(pUser->GetEntity());
    IMS_TRACE_I("FindParticipantByUserEntity : user-entity[%s]", strUserEntity.GetStr(), 0, 0);

    for (IMS_UINT32 i = 0; i < m_pParticipantList->GetSize(); i++)
    {
        if (m_pParticipantList->GetAt(i)->IsMatchingCompleted() == IMS_TRUE)
        {
            continue;
        }

        AString strParticipantUserEntity =
                m_objMessageUtils.GetUserPart(m_pParticipantList->GetAt(i)->GetUserEntity());

        IMS_TRACE_I("FindParticipantByUserEntity : [%s]", strParticipantUserEntity.GetStr(), 0, 0);
        if (!IsSamePrivacyUri(strParticipantUserEntity, strUserEntity))
        {
            continue;
        }

        if (IsInvalidStatusUpdate(i, pUser))
        {
            continue;
        }

        IMS_BOOL bSameUri =
                IsSameUri(strParticipantUserEntity, strUserEntity, m_objMessageUtils, IMS_FALSE);
        if (bSameUri &&
                !IsSameUriParameter(
                        m_pParticipantList->GetAt(i)->GetUserEntity(), pUser->GetEntity()))
        {
            IMS_TRACE_I("FindParticipantByUserEntity : different URI parameter", 0, 0, 0);
            continue;
        }

        if (IsAnonymousUri(strUserEntity) && !bSameUri)
        {
            // if anonymous uri "user-entity", they must be absolutely same. no prefix is allowed.
            continue;
        }

        if (bSameUri)
        {
            IMS_TRACE_I("FindParticipantByUserEntity : same uri", 0, 0, 0);
            nLocalParticipantIndex = i;
            break;
        }

        if (IsSameUri(strParticipantUserEntity, strUserEntity, m_objMessageUtils))
        {
            IMS_TRACE_I("FindParticipantByUserEntity : same allow prefix uri", 0, 0, 0);
            nLocalParticipantIndex = i;
            break;
        }
    }

    return nLocalParticipantIndex;
}

PRIVATE
ImsList<ConferenceInfo::User*> ConferenceInfoUpdater::GetSameUserEntities(
        IN const ConferenceInfo::User* pUser) const
{
    const AString& strUserEntity = pUser->GetEntity();

    const ImsList<ConferenceInfo::User*> objUsers = m_pConferenceInfo->GetUsers();
    ImsList<ConferenceInfo::User*> objSameEntityUsers;

    for (IMS_UINT32 i = 0; i < objUsers.GetSize(); i++)
    {
        const ConferenceInfo::User* pTempUser = objUsers.GetAt(i);
        if (pUser == pTempUser)
        {
            continue;
        }

        if (strUserEntity.Equals(objUsers.GetAt(i)->GetEntity()))
        {
            objSameEntityUsers.Append(objUsers.GetAt(i));
        }
    }

    return objSameEntityUsers;
}

PRIVATE
void ConferenceInfoUpdater::AddNotMatchedUserList(IN ConferenceInfo::User* pUser)
{
    for (IMS_UINT32 i = 0; i < m_objNotMatchedUsers.GetSize(); i++)
    {
        if (m_objNotMatchedUsers.GetAt(i) == pUser)
        {
            IMS_TRACE_D("AddNotMatchedUserList : It's already added", 0, 0, 0);
            return;
        }
    }

    IMS_TRACE_D("AddNotMatchedUserList : Added [%s]", pUser->GetEntity().GetStr(), 0, 0);
    m_objNotMatchedUsers.Append(pUser);
}

PRIVATE
void ConferenceInfoUpdater::RemoveFromNotMatchedUserList(IN const ConferenceInfo::User* pUser)
{
    for (IMS_UINT32 i = 0; i < m_objNotMatchedUsers.GetSize(); i++)
    {
        if (m_objNotMatchedUsers.GetAt(i) == pUser)
        {
            m_objNotMatchedUsers.RemoveAt(i);
            IMS_TRACE_D("RemoveFromNotMatchedUserList : Removed [%s]", pUser->GetEntity().GetStr(),
                    0, 0);
            return;
        }
    }
}

PRIVATE
IMS_BOOL ConferenceInfoUpdater::IsInitialNotifyWithoutUsers() const
{
    if (m_pParticipantList->GetXmlVersion() > 0)
    {
        return IMS_FALSE;
    }

    IMS_UINT32 nSize = m_pConferenceInfo->GetUsers().GetSize();
    if (nSize == 0)
    {
        return IMS_TRUE;
    }

    if (nSize == 1)
    {
        if (IsLocalUri(m_pConferenceInfo->GetUsers().GetAt(0)->GetEntity()))
        {
            return IMS_TRUE;
        }
    }

    return IMS_FALSE;
}

PRIVATE
IMS_BOOL ConferenceInfoUpdater::IsInvalidStatusUpdate(
        IN IMS_UINT32 nParticipantIndex, IN const ConferenceInfo::User* pUser) const
{
    IMS_UINT32 nOldStatus = m_pParticipantList->GetAt(nParticipantIndex)->GetConfUser()->eStatus;
    IMS_UINT32 nNewStatus = pUser->GetEndPoints().GetAt(0)->GetStatus();

    // to prevent updating from 'disconnected' to 'connected'.
    // This case can be happened in case that inviting a once disconnected user.
    if (!IsConnectedStatusCategory(nOldStatus) && IsConnectedStatusCategory(nNewStatus))
    {
        IMS_TRACE_D("IsInvalidStatusUpdate : disconnected user cannot be connected", 0, 0, 0);
        return IMS_TRUE;
    }

    IMS_BOOL bInfoUpdated = m_pParticipantList->GetAt(nParticipantIndex)->IsInfoUpdated();

    // to prevent updating a newly invited user to 'disconnected' without 'connected' status.
    if (bInfoUpdated == IMS_FALSE)
    {
        if (!IsConnectedStatusCategory(nNewStatus))
        {
            IMS_TRACE_D("IsInvalidStatusUpdate : invited user connot be directly disconnected", 0,
                    0, 0);
            return IMS_TRUE;
        }
    }

    // to match a proper user entity if there are duplicated 'user entity' in C-NOTIFY
    // This case can be happened when a user is repeatedly 'connected' and 'disconnected so
    // the conference info contains 'connected' and 'disconnected' status for a same user.
    if (bInfoUpdated == IMS_TRUE)
    {
        if (IsConnectedStatusCategory(nOldStatus) && !IsConnectedStatusCategory(nNewStatus))
        {
            ImsList<ConferenceInfo::User*> objSameEntityUsers = GetSameUserEntities(pUser);
            for (IMS_UINT32 i = 0; i < objSameEntityUsers.GetSize(); i++)
            {
                if (IsConnectedStatusCategory(
                            objSameEntityUsers.GetAt(i)->GetEndPoints().GetAt(0)->GetStatus()))
                {
                    IMS_TRACE_D("IsInvalidStatusUpdate : there is another user entity matched", 0,
                            0, 0);
                    return IMS_TRUE;
                }
            }
        }
    }

    // in partial state conference info, disconnected user information is not updated again.
    if (bInfoUpdated == IMS_TRUE && m_nInfoState == ConferenceInfo::STATE_PARTIAL)
    {
        if (m_pParticipantList->GetAt(nParticipantIndex)->IsDisconnectedExplicitly())
        {
            IMS_TRACE_D("IsInvalidStatusUpdate : already disconnected participant", 0, 0, 0);
            return IMS_TRUE;
        }
    }

    return IMS_FALSE;
}

PRIVATE
IMS_BOOL ConferenceInfoUpdater::IsLocalUri(IN const AString& strUserEntity) const
{
    // 'local' represents near device.
    AString strUserPart = m_objMessageUtils.GetUserPart(strUserEntity);
    if (strUserPart.GetLength() == 0)
    {
        IMS_TRACE_D("IsLocalUri : length 0", 0, 0, 0);
        return IMS_FALSE;
    }

    AString strLocalUserPart = m_objMessageUtils.GetUserPart(m_pParticipantList->GetLocalUri());
    if (strLocalUserPart.GetLength() == 0)
    {
        IMS_TRACE_D("IsLocalUri : length 0", 0, 0, 0);
        return IMS_FALSE;
    }

    IMS_BOOL bSame = IsSameUri(strUserPart, strLocalUserPart, m_objMessageUtils);
    IMS_TRACE_D("IsLocalUri : uri[%s] isLocal[%s]", strUserPart.GetStr(), _TRACE_B_(bSame), 0);

    return bSame;
}

PRIVATE GLOBAL IMS_BOOL ConferenceInfoUpdater::IsSameUri(IN const AString& strUriA,
        IN const AString& strUriB, IN IMessageUtils& objMessageUtils,
        IN IMS_BOOL bAllowPrefix /* = IMS_TRUE*/)
{
    IMS_SINT32 nLengthA = strUriA.GetLength();
    IMS_SINT32 nLengthB = strUriB.GetLength();

    IMS_UINT32 nLongerLength = nLengthA > nLengthB ? nLengthA : nLengthB;
    IMS_UINT32 nMargin = bAllowPrefix ? 3 : 0;
    if (GetMatchingCount(strUriA, strUriB, objMessageUtils) + nMargin >= nLongerLength)
    {
        // +821040044404 vs 01040044404
        // 10 + 3 >= 13(or 12)
        return IMS_TRUE;
    }

    return IMS_FALSE;
}

PRIVATE GLOBAL IMS_BOOL ConferenceInfoUpdater::IsSameUriParameter(
        IN const AString& strUserEntityA, IN const AString& strUserEntityB)
{
    // Checks only "user=phone" URI parameter.
    const AString USER_PHONE(";user=phone");
    return strUserEntityA.Contains(USER_PHONE) == strUserEntityB.Contains(USER_PHONE);
}

PRIVATE GLOBAL IMS_BOOL ConferenceInfoUpdater::IsAnonymousUri(IN const AString& strUserEntity)
{
    if (strUserEntity.MakeLower().Contains(ConferenceConst::ANONYMOUS))
    {
        return IMS_TRUE;
    }

    return IMS_FALSE;
}

PRIVATE GLOBAL IMS_BOOL ConferenceInfoUpdater::IsSamePrivacyUri(
        IN const AString& strUriA, IN const AString& strUriB)
{
    if (IsAnonymousUri(strUriA) && IsAnonymousUri(strUriB))
    {
        return IMS_TRUE;
    }

    if (!IsAnonymousUri(strUriA) && !IsAnonymousUri(strUriB))
    {
        return IMS_TRUE;
    }

    return IMS_FALSE;
}

PRIVATE GLOBAL IMS_BOOL ConferenceInfoUpdater::IsConnectedStatusCategory(IN IMS_UINT32 nStatus)
{
    switch (nStatus)
    {
        case STATUS_IDLE:
        case STATUS_PROGRESSING:
        case STATUS_CONNECTED:
        case STATUS_ON_HOLD:
        case STATUS_MUTED_VIA_FOCUS:
        case STATUS_PENDING:
        case STATUS_ALERTING:
        case STATUS_DIALING_IN:
        case STATUS_DIALING_OUT:
            return IMS_TRUE;
        default:
            return IMS_FALSE;
    }
}

PRIVATE GLOBAL IMS_UINT32 ConferenceInfoUpdater::GetMatchingScore(
        IN const AString& strUriA, IN const AString& strUriB)
{
    if (IsAnonymousUri(strUriA) && IsAnonymousUri(strUriB))
    {
        return 1;
    }

    return 0;
}

PRIVATE GLOBAL IMS_UINT32 ConferenceInfoUpdater::GetMatchingCount(
        IN const AString& strUriA, IN const AString& strUriB, IN IMessageUtils& objMessageUtils)
{
    AString strA = objMessageUtils.GetUserPart(strUriA);
    if (strA.GetLength() == 0)
    {
        strA = strUriA;
    }

    AString strB = objMessageUtils.GetUserPart(strUriB);
    if (strB.GetLength() == 0)
    {
        strB = strUriB;
    }

    IMS_UINT32 nParticipantEntityLength = strA.GetLength();
    IMS_UINT32 nXmlEntityLength = strB.GetLength();
    IMS_UINT32 nMaxCount = nParticipantEntityLength > nXmlEntityLength ? nXmlEntityLength
                                                                       : nParticipantEntityLength;
    IMS_UINT32 nMatchingCount = 0;

    for (IMS_UINT32 i = 1; i <= nMaxCount; i++)
    {
        if (strA[nParticipantEntityLength - i] == strB[nXmlEntityLength - i])
        {
            nMatchingCount = i;
            continue;
        }
        else
        {
            break;
        }
    }

    return nMatchingCount;
}
