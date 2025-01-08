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

#include "IMtcCallStateListener.h"
#include "ImsActivity.h"
#include "ImsMap.h"
#include "ImsMessage.h"
#include "ImsTypeDef.h"
#include "ServiceTrace.h"
#include "call/IMtcCall.h"
#include "call/IMtcCallContext.h"
#include "call/IMtcCallManager.h"
#include "helper/CallStateProxy.h"

__IMS_TRACE_TAG_COM_MTC__;

PUBLIC
CallStateProxy::CallStateProxy(IN IMtcCallManager& objCallManager) :
        ImsActivity(),
        m_objSynchronousListeners(ImsList<IMtcCallStateListener*>()),
        m_objAsynchronousListeners(ImsList<IMtcCallStateListener*>()),
        m_objCallManager(objCallManager),
        m_eTotalState(IMtcCall::State::IDLE)
{
    IMS_TRACE_D("+CallStateProxy", 0, 0, 0);
}

PUBLIC VIRTUAL CallStateProxy::~CallStateProxy() {}

PUBLIC
void CallStateProxy::AddListener(IN IMtcCallStateListener* pListener)
{
    if (pListener->IsSynchronousCallRequired())
    {
        for (IMS_UINT32 i = 0; i < m_objSynchronousListeners.GetSize(); i++)
        {
            if (pListener == m_objSynchronousListeners.GetAt(i))
            {
                return;
            }
        }
        m_objSynchronousListeners.Append(pListener);
        IMS_TRACE_D("AddListener sync size=[%d]", m_objSynchronousListeners.GetSize(), 0, 0);
    }
    else
    {
        for (IMS_UINT32 i = 0; i < m_objAsynchronousListeners.GetSize(); i++)
        {
            if (pListener == m_objAsynchronousListeners.GetAt(i))
            {
                return;
            }
        }
        m_objAsynchronousListeners.Append(pListener);
        IMS_TRACE_D("AddListener async size=[%d]", m_objAsynchronousListeners.GetSize(), 0, 0);
    }
}

PUBLIC
void CallStateProxy::RemoveListener(IN IMtcCallStateListener* pListener)
{
    if (pListener->IsSynchronousCallRequired())
    {
        for (IMS_UINT32 i = 0; i < m_objSynchronousListeners.GetSize(); i++)
        {
            if (pListener == m_objSynchronousListeners.GetAt(i))
            {
                m_objSynchronousListeners.RemoveAt(i);
                IMS_TRACE_D(
                        "RemoveListener sync size=[%d]", m_objSynchronousListeners.GetSize(), 0, 0);
                return;
            }
        }
    }
    else
    {
        for (IMS_UINT32 i = 0; i < m_objAsynchronousListeners.GetSize(); i++)
        {
            if (pListener == m_objAsynchronousListeners.GetAt(i))
            {
                m_objAsynchronousListeners.RemoveAt(i);
                IMS_TRACE_D("RemoveListener async size=[%d]", m_objAsynchronousListeners.GetSize(),
                        0, 0);
                return;
            }
        }
    }
}

PUBLIC
void CallStateProxy::UpdateCallState(IN CallKey nCallkey, IN IMtcCall::State eState,
        IN CallType eCallType, IN IMS_BOOL bEmergency, IN IMS_SINT32 nReason /* = CODE_NONE */)
{
    IMS_BOOL bTotalCallStateUpdated = UpdateTotalCallState();
    IMS_TRACE_D("UpdateCallState state[%d] totalStateUpdated[%s]", eState,
            _TRACE_B_(bTotalCallStateUpdated), 0);

    CallStateDetails* pDetails =
            new CallStateDetails(nCallkey, static_cast<IMtcCallStateListener::State>(eState),
                    static_cast<IMtcCallStateListener::Type>(eCallType), bEmergency, nReason);

    if (m_objSynchronousListeners.GetSize() > 0)
    {
        NotifyToListeners(IMS_TRUE, pDetails, bTotalCallStateUpdated);
    }
    if (m_objAsynchronousListeners.GetSize() > 0)
    {
        // TODO: use OperationAsyncRunner?
        PostMessage(MESSAGE_ASYNC_NOTIFY, reinterpret_cast<IMS_UINTP>(pDetails),
                static_cast<IMS_UINTP>(bTotalCallStateUpdated));
    }
    else
    {
        delete pDetails;
    }
}

PUBLIC
void CallStateProxy::NotifyCallSessionReleased(
        IN CallKey nCallKey, IN IMS_BOOL bEmergency, IN IMS_BOOL bEstablished)
{
    NotifyCallSessionReleased(m_objSynchronousListeners, nCallKey, bEmergency, bEstablished);
    NotifyCallSessionReleased(m_objAsynchronousListeners, nCallKey, bEmergency, bEstablished);
}

PUBLIC VIRTUAL IMS_BOOL CallStateProxy::DispatchMessage(IN ImsMessage& objMsg)
{
    switch (objMsg.GetName())
    {
        case MESSAGE_ASYNC_NOTIFY:
        {
            CallStateDetails* pDetails = reinterpret_cast<CallStateDetails*>(objMsg.nWparam);
            NotifyToListeners(IMS_FALSE, pDetails, static_cast<IMS_BOOL>(objMsg.nLparam));
            delete pDetails;
            return IMS_TRUE;
        }
        default:
            break;
    }

    return IMS_FALSE;
}

PRIVATE
IMS_BOOL CallStateProxy::UpdateTotalCallState()
{
    IMtcCall::State eCalculatedState = CalculateTotalCallState();
    if (m_eTotalState == eCalculatedState)
    {
        return IMS_FALSE;
    }

    m_eTotalState = eCalculatedState;
    return IMS_TRUE;
}

PRIVATE
IMtcCall::State CallStateProxy::CalculateTotalCallState()
{
    ImsList<IMtcCall*> objCalls = m_objCallManager.GetCalls();
    IMtcCall::State eTotalState = IMtcCall::State::IDLE;

    for (IMS_UINT32 i = 0; i < objCalls.GetSize(); i++)
    {
        switch (objCalls.GetAt(i)->GetState())
        {
            case IMtcCall::State::ESTABLISHED:
            case IMtcCall::State::UPDATING:
                return IMtcCall::State::ESTABLISHED;

            case IMtcCall::State::INCOMING:
            case IMtcCall::State::ALERTING:
                eTotalState = IMtcCall::State::INCOMING;
                break;

            case IMtcCall::State::OUTGOING:
                eTotalState = IMtcCall::State::OUTGOING;
                break;

            case IMtcCall::State::IDLE:
                eTotalState =
                        objCalls.GetAt(i)->GetCallContext().GetCallInfo().ePeerType == PeerType::MO
                        ? IMtcCall::State::OUTGOING
                        : IMtcCall::State::INCOMING;
                break;
            case IMtcCall::State::TERMINATING:
                break;
        }
    }

    return eTotalState;
}

PRIVATE
void CallStateProxy::NotifyToListeners(IN IMS_BOOL bSynchronous, IN CallStateDetails* pDetails,
        IN IMS_BOOL bTotalCallStateUpdated) const
{
    IMS_TRACE_D("NotifyToListeners sync[%s]", _TRACE_B_(bSynchronous), 0, 0);

    if (bSynchronous)
    {
        NotifyCallState(m_objSynchronousListeners, pDetails);
        if (bTotalCallStateUpdated)
        {
            NotifyTotalCallState(m_objSynchronousListeners);
        }
    }
    else
    {
        NotifyCallState(m_objAsynchronousListeners, pDetails);
        if (bTotalCallStateUpdated)
        {
            NotifyTotalCallState(m_objAsynchronousListeners);
        }
    }
}

PRIVATE
void CallStateProxy::NotifyCallState(
        IN ImsList<IMtcCallStateListener*> objListeners, IN CallStateDetails* pDetails)
{
    for (IMS_UINT32 i = 0; i < objListeners.GetSize(); i++)
    {
        objListeners.GetAt(i)->OnCallStateChanged(pDetails->nCallKey, pDetails->eState,
                pDetails->eType, pDetails->bEmergency, pDetails->nReason);
    }
}

PRIVATE
void CallStateProxy::NotifyTotalCallState(IN ImsList<IMtcCallStateListener*> objListeners) const
{
    for (IMS_UINT32 i = 0; i < objListeners.GetSize(); i++)
    {
        objListeners.GetAt(i)->OnTotalCallStateChanged(
                static_cast<IMtcCallStateListener::State>(m_eTotalState));
    }
}

PRIVATE
void CallStateProxy::NotifyCallSessionReleased(IN ImsList<IMtcCallStateListener*> objListeners,
        IN CallKey nCallKey, IN IMS_BOOL bEmergency, IN IMS_BOOL bEstablished) const
{
    for (IMS_UINT32 i = 0; i < objListeners.GetSize(); i++)
    {
        objListeners.GetAt(i)->OnCallSessionReleased(nCallKey, bEmergency, bEstablished);
    }
}
