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
#include "ServiceEvent.h"

#include "IMtsCallTrackerListener.h"
#include "MtsCallTracker.h"
#include "MtsStringDef.h"

__IMS_TRACE_TAG_COM_SMS__;

PUBLIC
MtsCallTracker::MtsCallTracker(IN IMS_SINT32 nSlotId) :
        m_nSlotId(nSlotId),
        m_nEmergencyState(CALL_STATE_IDLE),
        m_objEmergencyCalls(IMSMap<IMS_SINTP, IMS_UINT32>()),
        m_objListeners(IMSList<IMtsCallTrackerListener*>())
{
    IMS_TRACE_I("MtsCallTracker [slot_%d]", m_nSlotId, 0, 0);
    // TO-DO: Listen the mtc call tracker
}

PUBLIC
MtsCallTracker::~MtsCallTracker()
{
    IMS_TRACE_I("~MtsCallTracker [slot_%d]", m_nSlotId, 0, 0);
    m_objListeners.Clear();
}

PUBLIC
IMS_BOOL MtsCallTracker::IsEmergencyCallActive() const
{
    IMS_TRACE_I("IsEmergencyCallActive : m_nEmergencyState[%d]", m_nEmergencyState, 0, 0);
    return (m_nEmergencyState > CALL_STATE_IDLE);
}

PUBLIC
IMS_SINT32 MtsCallTracker::GetSlotId() const
{
    return m_nSlotId;
}

PUBLIC
IMS_UINT32 MtsCallTracker::GetCallState(IN IMS_UINT32 nType) const
{
    if (nType == CALL_TYPE_EMERGENCY)
    {
        return m_nEmergencyState;
    }

    return CALL_STATE_IDLE;
}

PUBLIC
IMS_UINT32 MtsCallTracker::GetSessionType(IN IMS_UINT32 nType) const
{
    IMS_TRACE_I("GetSessionType: nType[%d]", nType, 0, 0);
    return SESSION_TYPE_NONE;
}

PUBLIC
void MtsCallTracker::AddListener(IN IMtsCallTrackerListener* piListener)
{
    for (IMS_UINT32 i = 0; i < m_objListeners.GetSize(); ++i)
    {
        IMtsCallTrackerListener* pTmpListener = m_objListeners.GetAt(i);

        if (pTmpListener == piListener)
        {
            IMS_TRACE_I("AddListener : Listener is already added", 0, 0, 0);
            return;
        }
    }

    m_objListeners.Append(piListener);
}

PUBLIC
void MtsCallTracker::RemoveListener(IN IMtsCallTrackerListener* piListener)
{
    for (IMS_UINT32 i = 0; i < m_objListeners.GetSize(); ++i)
    {
        IMtsCallTrackerListener* pTmpListener = m_objListeners.GetAt(i);

        if (pTmpListener == piListener)
        {
            m_objListeners.RemoveAt(i);
            IMS_TRACE_I("RemoveListener : Listener is removed", 0, 0, 0);
            return;
        }
    }
}

PRIVATE
void MtsCallTracker::AddOrUpdateCall(
        IN IMSMap<IMS_SINTP, IMS_UINT32>& objCalls, IN IMS_SINTP nKey, IN IMS_UINT32 nState)
{
    IMS_SINT32 nAt = objCalls.GetIndexOfKey(nKey);

    if (nAt >= 0)
    {
        objCalls.SetValueAt(nAt, nState);
        return;
    }

    if (!objCalls.Add(nKey, nState))
    {
        IMS_TRACE_E(0, "AddOrUpdateCall : map error", 0, 0, 0);
    }
}

PRIVATE
void MtsCallTracker::RemoveCall(IN IMSMap<IMS_SINTP, IMS_UINT32>& objCalls, IN IMS_SINTP nKey)
{
    IMS_SINT32 nAt = objCalls.GetIndexOfKey(nKey);

    if (nAt < 0)
    {
        return;
    }

    objCalls.RemoveAt(nAt);
}

PRIVATE
IMS_UINT32 MtsCallTracker::GetConvertedState(IN IMS_UINT32 nState)
{
    switch (nState)
    {
            /* _UC_TO_MTC_
                    case IUCCallListener::UC_CALL_STATE_IDLE:
                        return CALL_STATE_IDLE;

                    case IUCCallListener::UC_CALL_STATE_TERMINATING:
                        return CALL_STATE_TERMINATING;

                    case IUCCallListener::UC_CALL_STATE_RINGBACK:
                        return CALL_STATE_RINGBACK;

                    case IUCCallListener::UC_CALL_STATE_RINGING:
                        return CALL_STATE_RINGING;

                    case IUCCallListener::UC_CALL_STATE_ALERTING:
                        return CALL_STATE_ALERTING;

                    case IUCCallListener::UC_CALL_STATE_OFFHOOK:
                        return CALL_STATE_OFFHOOK;
            */
        default:
            break;
    }

    return CALL_STATE_IDLE;
}

PRIVATE
IMS_UINT32 MtsCallTracker::GetTotalState(IN IMSMap<IMS_SINTP, IMS_UINT32>& objCalls)
{
    if (objCalls.IsEmpty())
    {
        return CALL_STATE_IDLE;
    }

    if (objCalls.GetSize() > 1)
    {
        return CALL_STATE_OFFHOOK;
    }

    return objCalls.GetValueAt(0);
}

PRIVATE
IMS_UINT32 MtsCallTracker::GetState(IN IMS_UINT32 nType) const
{
    if (nType == CALL_TYPE_EMERGENCY)
    {
        return m_nEmergencyState;
    }

    return CALL_STATE_IDLE;
}

PRIVATE
void MtsCallTracker::SetState(IN IMS_UINT32 nType, IN IMS_UINT32 nState)
{
    switch (nType)
    {
        case CALL_TYPE_EMERGENCY:
            m_nEmergencyState = nState;
            break;

        default:
            break;
    }
}

PRIVATE
void MtsCallTracker::Notify(IN IMS_UINT32 nType, IN IMS_UINT32 nState)
{
    for (IMS_UINT32 nAt = 0; nAt < m_objListeners.GetSize(); nAt++)
    {
        IMtsCallTrackerListener* piListener = m_objListeners.GetAt(nAt);

        if (piListener != IMS_NULL)
        {
            piListener->CallTracker_StateChanged(nType, nState);
        }
    }
}

PRIVATE
void MtsCallTracker::ProcessEmergencyChanged(IN IMS_SINTP nKey, IN IMS_UINT32 nState)
{
    if (nState == CALL_STATE_IDLE)
    {
        RemoveCall(m_objEmergencyCalls, nKey);
    }
    else
    {
        AddOrUpdateCall(m_objEmergencyCalls, nKey, nState);
    }

    IMS_UINT32 nCurrState = GetTotalState(m_objEmergencyCalls);
    IMS_TRACE_I("ProcessEmergencyChanged : old [%s]->curr[%s]", PS_CallState(m_nEmergencyState),
            PS_CallState(nCurrState), 0);

    if (m_nEmergencyState != nCurrState)
    {
        SetState(CALL_TYPE_EMERGENCY, nCurrState);
        Notify(CALL_TYPE_EMERGENCY, nCurrState);
    }
}

PRIVATE
void MtsCallTracker::ChangedCallState(IN IMS_UINTP nParam)
{
    if (nParam == IMS_NULL)
    {
        return;
    }
    /* _UC_TO_MTC_
        IUCCallListenChangedStateParam* piState =
                reinterpret_cast<IUCCallListenChangedStateParam*>(nParam);

        if (piState->nKey == IMS_NULL)
        {
            delete piState;
            return;
        }

        IMS_TRACE_I("ChangedCallState : nKey[%p], nService[%d], nState[%d]", piState->nKey,
        piState->eServiceType, piState->eState); switch (piState->eServiceType)
        {
            case IUCCallListener::SERVICETYPE_EMERGENCY:
                ProcessEmergencyChanged(piState->nKey, GetConvertedState(piState->eState));
                break;
            default:
                break;
        }

        delete piState;
    */
}

PRIVATE
void MtsCallTracker::ChangedCallTotalState(IN IMS_UINTP nParam)
{
    if (nParam == IMS_NULL)
    {
        return;
    }
    /* _UC_TO_MTC_
        IUCCallListenChangedTotalStateParam* piTotalState =
                reinterpret_cast<IUCCallListenChangedTotalStateParam*>(nParam);

        if (piTotalState->eState == IUCCallListener::UC_CALL_STATE_IDLE)
        {
            IMS_UINT32 nEmergencyCount = m_objEmergencyCalls.GetSize();
            IMS_TRACE_I("ChangedCallTotalState : Emergency[%d]", nEmergencyCount, 0, 0);

            if (nEmergencyCount > 0)
            {
                m_objEmergencyCalls.Clear();

                SetState(CALL_TYPE_EMERGENCY, CALL_STATE_IDLE);
                Notify(CALL_TYPE_EMERGENCY, CALL_STATE_IDLE);
            }
        }

        delete piTotalState;
    */
}
