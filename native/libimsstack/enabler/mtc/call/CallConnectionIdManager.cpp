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

#include "IMtcContext.h"
#include "ImsList.h"
#include "ImsTypeDef.h"
#include "ServiceTrace.h"
#include "call/CallConnectionIdManager.h"
#include "call/IMtcCall.h"
#include "conferencecall/IConferenceController.h"
#include "helper/ICallStateProxy.h"

__IMS_TRACE_TAG_COM_MTC__;

PUBLIC
CallConnectionIdManager::CallConnectionIdManager(IN IMtcContext& objContext) :
        m_objContext(objContext),
        m_objCallKeyConnections(ImsList<CallKeyConnection*>()),
        m_objControllers(ImsList<IConferenceController*>())
{
    IMS_TRACE_D("+CallConnectionIdManager", 0, 0, 0);
    m_objContext.GetCallStateProxy().AddListener(this);
}

PUBLIC
CallConnectionIdManager::~CallConnectionIdManager()
{
    IMS_TRACE_D("~CallConnectionIdManager", 0, 0, 0);
    m_objCallKeyConnections.Clear();
    m_objControllers.Clear();
}

PUBLIC VIRTUAL void CallConnectionIdManager::OnCallStateChanged(IN CallKey nCallKey,
        IN State eState, IN Type /*eType*/, IN IMS_BOOL /*bEmergency*/, IN IMS_SINT32 /*nReason*/)
{
    if (eState == State::IDLE)
    {
        IMS_TRACE_D("OnCallStateChanged - MtcCall created but no reference available yet", 0, 0, 0);
        return;
    }

    IMS_UINT32 nControllerIndexOfHost = 0;
    if (IsConferenceHost(nCallKey, nControllerIndexOfHost))
    {
        if (eState == State::TERMINATING)
        {
            // TODO: check if conference call is failed or terminated.
            // Or, set IsSynchronousCallRequired() false so ConferenceController::Recover()
            // is called first.
            ClearConnectionIdsInConference(nControllerIndexOfHost);
            return;
        }

        IMS_TRACE_D("OnCallStateChanged conference call. ignore.", 0, 0, 0);
        return;
    }

    if (eState == State::TERMINATING)
    {
        IMS_TRACE_D("OnCallStateChanged TERMINATING", 0, 0, 0);
        if (IsConferenceParticipant(nCallKey))
        {
            // remove CallKey from CallKeyConnectionId just in case duplicated CallKey(memory)
            // this should be done after this 'OnCallStateChanged()' because ConferenceContoller
            // will try to find CallKey using the connectionId when TERMINATING is passed to it.
            // Not to add message posting here, removing is moved into 'AddKeyConnectionId()'
            return;
        }

        RemoveKeyConnectionId(GetListIndexByCallKey(nCallKey));
    }
    else if (eState == State::OUTGOING || eState == State::INCOMING ||
            eState == State::ALERTING)  // TODO: all?
    {
        AddKeyConnectionId(nCallKey);
    }
}

PUBLIC VIRTUAL void CallConnectionIdManager::OnTotalCallStateChanged(IN State eState)
{
    if (eState != State::IDLE)
    {
        return;
    }

    IMS_SINT32 nSize = static_cast<IMS_SINT32>(m_objCallKeyConnections.GetSize());
    IMS_TRACE_D("OnTotalCallStateChanged IDLE - size[%d]", nSize, 0, 0);
    for (IMS_SINT32 i = nSize - 1; i >= 0; i--)
    {
        RemoveKeyConnectionId(i);
    }
    m_objCallKeyConnections.Clear();
    m_objControllers.Clear();
}

PUBLIC
void CallConnectionIdManager::OnConferenceCallStarted(
        IN IConferenceController* piController, IN IMS_BOOL bStarted)
{
    if (bStarted)
    {
        IMS_TRACE_D("OnConferenceCallStarted", 0, 0, 0);
        m_objControllers.Append(piController);
    }
    else
    {
        for (IMS_UINT32 i = 0; i < m_objControllers.GetSize(); i++)
        {
            if (m_objControllers.GetAt(i) != piController)
            {
                continue;
            }
            m_objControllers.RemoveAt(i);
        }
    }
}

PUBLIC
void CallConnectionIdManager::OnConferenceParticipantDisconnected(IN IMS_UINT32 nConnectionId)
{
    IMS_TRACE_D("OnConferenceParticipantDisconnected foreground call(ID[%d]) disconnected",
            nConnectionId, 0, 0);
    RemoveKeyConnectionId(GetListIndexByConnectionId(nConnectionId));
}

PUBLIC
IMS_SINT32 CallConnectionIdManager::GetIndex(IN CallKey nKey) const
{
    for (IMS_UINT32 i = 0; i < m_objCallKeyConnections.GetSize(); i++)
    {
        CallKeyConnection* pTemp = m_objCallKeyConnections.GetAt(i);
        if (pTemp->nKey == nKey)
        {
            return pTemp->nConnectionId;
        }
    }
    return -1;
}

PUBLIC
CallKey CallConnectionIdManager::GetCallKey(IN IMS_UINT32 nConnectionId) const
{
    for (IMS_UINT32 i = 0; i < m_objCallKeyConnections.GetSize(); i++)
    {
        CallKeyConnection* pTemp = m_objCallKeyConnections.GetAt(i);
        if (pTemp->nConnectionId == nConnectionId)
        {
            return pTemp->nKey;
        }
    }
    IMS_TRACE_E(0, "GetCallKey no CallKey for ID=[%d]", nConnectionId, 0, 0);
    return 0;
}

PRIVATE
IMS_UINT32 CallConnectionIdManager::GetNewIndex()
{
    if (m_objCallKeyConnections.IsEmpty())
    {
        return 1;  // ID starts from 1.
    }

    for (IMS_UINT32 i = 0; i < m_objCallKeyConnections.GetSize(); i++)
    {
        IMS_UINT32 nId = m_objCallKeyConnections.GetValueAt(i)->nConnectionId;

        if (nId != i + 1)
        {
            return i + 1;
        }
    }

    return m_objCallKeyConnections.GetValueAt(m_objCallKeyConnections.GetSize() - 1)
                   ->nConnectionId +
            1;
}

PRIVATE
IMS_SINT32 CallConnectionIdManager::GetListIndexByCallKey(IN CallKey nCallKey)
{
    for (IMS_UINT32 i = 0; i < m_objCallKeyConnections.GetSize(); i++)
    {
        if (m_objCallKeyConnections.GetAt(i)->nKey == nCallKey)
        {
            return i;
        }
    }

    return -1;
}

PRIVATE
IMS_SINT32 CallConnectionIdManager::GetListIndexByConnectionId(IN IMS_UINT32 nConnectionId)
{
    for (IMS_UINT32 i = 0; i < m_objCallKeyConnections.GetSize(); i++)
    {
        if (m_objCallKeyConnections.GetAt(i)->nConnectionId == nConnectionId)
        {
            return i;
        }
    }

    return -1;
}

PRIVATE
void CallConnectionIdManager::AddKeyConnectionId(IN CallKey nCallKey)
{
    IMS_SINT32 nListIndex = GetListIndexByCallKey(nCallKey);
    if (nListIndex >= 0)
    {
        return;
    }

    IMS_UINT32 nNewIndex = GetNewIndex();
    CallKeyConnection* pNewKeyConnection = new CallKeyConnection(nCallKey, nNewIndex);

    if (nNewIndex > m_objCallKeyConnections.GetSize())
    {
        m_objCallKeyConnections.Append(pNewKeyConnection);
    }
    else
    {
        // SORTING??
        // it's always sorted and the connectionId always starts from '1'.
        m_objCallKeyConnections.InsertAt(pNewKeyConnection, nNewIndex - 1);
    }

    IMS_TRACE_D("AddKeyConnectionId : list[%s] addId=[%d] key=[%d]", GetIds().GetStr(), nNewIndex,
            nCallKey);
}

PRIVATE
void CallConnectionIdManager::RemoveKeyConnectionId(IN IMS_SINT32 nIndex)
{
    if (nIndex < 0)
    {
        IMS_TRACE_E(0, "invalid index to remove [%d]", nIndex, 0, 0);
        // invalid
        return;
    }

    CallKeyConnection* pKeyConnection = m_objCallKeyConnections.GetAt(nIndex);
    IMS_UINT32 nConnectionId = pKeyConnection->nConnectionId;
    CallKey nKey = pKeyConnection->nKey;

    delete pKeyConnection;
    m_objCallKeyConnections.RemoveAt(nIndex);
    IMS_TRACE_D("RemoveKeyConnectionId : list[%s] removeId=[%d] key=[%d]", GetIds().GetStr(),
            nConnectionId, nKey);
}

PRIVATE
IMS_BOOL CallConnectionIdManager::IsConferenceParticipant(IN CallKey nCallKey)
{
    for (IMS_UINT32 i = 0; i < m_objControllers.GetSize(); i++)
    {
        if (m_objControllers.GetAt(i)->GetCallStatusInConference(nCallKey) !=
                IndividualCallState::IDLE)
        {
            return IMS_TRUE;
        }
    }
    return IMS_FALSE;
}

PRIVATE
IMS_BOOL CallConnectionIdManager::IsConferenceHost(
        IN CallKey nCallKey, OUT IMS_UINT32& nControllerIndex)
{
    for (IMS_UINT32 i = 0; i < m_objControllers.GetSize(); i++)
    {
        if (m_objControllers.GetAt(i)->GetCallStatusInConference(nCallKey) ==
                IndividualCallState::HOST)
        {
            nControllerIndex = i;
            return IMS_TRUE;
        }
    }
    return IMS_FALSE;
}

PRIVATE
AString CallConnectionIdManager::GetIds()
{
    AString strIds;
    for (IMS_UINT32 i = 0; i < m_objCallKeyConnections.GetSize(); i++)
    {
        AString temp;
        temp.SetNumber(m_objCallKeyConnections.GetValueAt(i)->nConnectionId);
        strIds += temp;
        if (i < m_objCallKeyConnections.GetSize() - 1)
        {
            strIds += ",";
        }
    }

    return strIds;
}

PRIVATE
void CallConnectionIdManager::ClearConnectionIdsInConference(IN IMS_UINT32 nControllerIndex)
{
    // no null check
    IConferenceController* piController = m_objControllers.GetAt(nControllerIndex);
    IMS_SINT32 nSize = m_objCallKeyConnections.GetSize();
    IMS_TRACE_D("ClearConnectionIdsInConference total size=[%d]", nSize, 0, 0);

    for (IMS_SINT32 i = nSize - 1; i >= 0; i--)
    {
        IMS_TRACE_D("ClearConnectionIdsInConference index[%d]", i, 0, 0);
        if (piController->GetCallStatusInConference(m_objCallKeyConnections.GetAt(i)->nKey) ==
                IndividualCallState::IDLE)
        {
            continue;
        }
        RemoveKeyConnectionId(i);
    }
    return;
}
