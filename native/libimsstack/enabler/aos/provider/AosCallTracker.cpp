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

#include "ImsVector.h"
#include "ServiceTrace.h"
#include "ServiceEvent.h"
#include "IAosService.h"
#include "ImsEventDef.h"

#include "../../interface/mtc/MtcConnector.h"
#include "interface/IAosNConfiguration.h"
#include "interface/IAosCallTrackerListener.h"
#include "provider/AosUtil.h"
#include "provider/AosCallTracker.h"
#include "provider/AosProvider.h"

__IMS_TRACE_TAG_AOS__;

#define AOSTAG m_strTag.GetStr()

PUBLIC
AosCallTracker::AosCallTracker(IN IMS_SINT32 nSlotId) :
        m_nSlotId(nSlotId),
        m_eCsState(CallState::IDLE),
        m_eNormalState(CallState::IDLE),
        m_eEmergencyState(CallState::IDLE),
        m_nNormalCallType(static_cast<IMS_UINT32>(CallType::UNKNOWN)),
        m_eActiveCsState(CallState::IDLE),
        m_objNormalCalls(ImsMap<CallKey, CallState>()),
        m_objEmergencyCalls(ImsMap<CallKey, CallState>()),
        m_objNormalCallTypes(ImsMap<CallKey, CallType>()),
        m_objListeners(ImsList<IAosCallTrackerListener*>())
{
    IMS_TRACE_MEM("AOS_MEM", "AOS_M : [SLOT%d] AosCallTracker = %" PFLS_u "/%" PFLS_x, m_nSlotId,
            sizeof(AosCallTracker), this);

    IMS_EVENT_AddListenerForSlotId(IMS_EVENT_CSCALL_STATE, this, m_nSlotId);

    m_strTag.Sprintf("%d", m_nSlotId);
}

PUBLIC VIRTUAL AosCallTracker::~AosCallTracker()
{
    IMS_TRACE_MEM("AOS_MEM", "AOS_F : [SLOT%d] AosCallTracker = %" PFLS_u "/%" PFLS_x, m_nSlotId,
            sizeof(AosCallTracker), this);

    m_objListeners.Clear();

    AosProvider::GetInstance()->GetService(m_nSlotId)->RemoveListener(
            DYNAMIC_CAST(IAosServicePhoneListener*, this));
    IMS_EVENT_RemoveListenerForSlotId(IMS_EVENT_CSCALL_STATE, this, m_nSlotId);

    MtcConnector::RemoveCallStateListener(m_nSlotId, DYNAMIC_CAST(IMtcCallStateListener*, this));
}

PUBLIC VIRTUAL IMS_BOOL AosCallTracker::SetMtcReady() const
{
    A_IMS_TRACE_I(AOSTAG, "SetMtcReady", 0, 0, 0);
    MtcConnector::AddCallStateListener(m_nSlotId, DYNAMIC_CAST(IMtcCallStateListener*, this));

    return IMS_TRUE;
}

PUBLIC VIRTUAL IMS_BOOL AosCallTracker::IsCsCallActive() const
{
    A_IMS_TRACE_I(AOSTAG, "IsCsCallActive :: active (%s) , state (%d)",
            _TRACE_B_(m_eCsState > m_eActiveCsState), m_eCsState, 0);
    return (m_eCsState > m_eActiveCsState);
}

PUBLIC VIRTUAL IMS_BOOL AosCallTracker::IsNormalCallActive() const
{
    A_IMS_TRACE_I(AOSTAG, "IsNormalCallActive :: active (%s) , state (%d)",
            _TRACE_B_(m_eNormalState > CallState::IDLE), m_eNormalState, 0);
    return (m_eNormalState > CallState::IDLE);
}

PUBLIC VIRTUAL IMS_BOOL AosCallTracker::IsEmergencyCallActive() const
{
    A_IMS_TRACE_I(AOSTAG, "IsEmergencyCallActive :: active (%s) , state (%d)",
            _TRACE_B_(m_eEmergencyState > CallState::IDLE), m_eEmergencyState, 0);
    return (m_eEmergencyState > CallState::IDLE);
}

PUBLIC VIRTUAL IMS_BOOL AosCallTracker::IsVideoCallingActive() const
{
    if (!IsExistCallType(CallType::VT) || m_objNormalCallTypes.IsEmpty())
    {
        A_IMS_TRACE_I(AOSTAG, "IsVideoCallingActive(%s)", "false", 0, 0);
        return IMS_FALSE;
    }

    IMS_BOOL bIsActive = IMS_FALSE;

    for (IMS_UINT32 nAt = 0; nAt < m_objNormalCallTypes.GetSize(); ++nAt)
    {
        CallType eCallType = m_objNormalCallTypes.GetValueAt(nAt);

        if (eCallType != CallType::VT)
        {
            continue;
        }

        CallKey eKey = m_objNormalCallTypes.GetKeyAt(nAt);
        CallState eState = m_objNormalCalls.GetValue(eKey);

        if (eState == CallState::OFFHOOK)
        {
            bIsActive = IMS_TRUE;
            break;
        }
    }

    A_IMS_TRACE_I(AOSTAG, "IsVideoCallingActive(%s)", _TRACE_B_(bIsActive), 0, 0);

    return bIsActive;
}

PUBLIC VIRTUAL IMS_SINT32 AosCallTracker::GetSlotId() const
{
    return m_nSlotId;
}

PUBLIC VIRTUAL CallState AosCallTracker::GetCallState(IN IMS_UINT32 nType) const
{
    return GetState(nType);
}

PUBLIC VIRTUAL void AosCallTracker::SetCsCallStateWatchMode()
{
    A_IMS_TRACE_D(AOSTAG, "SetCsCallStateWatchMode", 0, 0, 0);

    IMS_EVENT_RemoveListenerForSlotId(IMS_EVENT_CSCALL_STATE, this, m_nSlotId);
    AosProvider::GetInstance()->GetService(m_nSlotId)->AddListener(
            DYNAMIC_CAST(IAosServicePhoneListener*, this));
}

PUBLIC VIRTUAL void AosCallTracker::SetActiveCsCallState(IN CallState eActiveCsState)
{
    A_IMS_TRACE_D(AOSTAG, "SetActiveCsCallState :: (%d)", eActiveCsState, 0, 0);
    m_eActiveCsState = eActiveCsState;
}

PUBLIC VIRTUAL void AosCallTracker::SetListener(IN IAosCallTrackerListener* piListener)
{
    for (IMS_UINT32 i = 0; i < m_objListeners.GetSize(); ++i)
    {
        IAosCallTrackerListener* pTmpListener = m_objListeners.GetAt(i);

        if (pTmpListener == piListener)
        {
            A_IMS_TRACE_D(AOSTAG, "SetListener() :: Listener (%" PFLS_x ") is already set",
                    piListener, 0, 0);
            return;
        }
    }

    m_objListeners.Append(piListener);
}

PUBLIC VIRTUAL void AosCallTracker::RemoveListener(IN IAosCallTrackerListener* piListener)
{
    for (IMS_UINT32 i = 0; i < m_objListeners.GetSize(); ++i)
    {
        IAosCallTrackerListener* pTmpListener = m_objListeners.GetAt(i);

        if (pTmpListener == piListener)
        {
            m_objListeners.RemoveAt(i);

            A_IMS_TRACE_D(AOSTAG, "RemoveListener :: Listener (%" PFLS_x ") is removed", piListener,
                    0, 0);
            return;
        }
    }
}

PROTECTED
template <typename T>
void AosCallTracker::AddOrUpdateCall(OUT ImsMap<CallKey, T>& objCalls, IN CallKey eKey, IN T eValue)
{
    IMS_SINT32 nAt = objCalls.GetIndexOfKey(eKey);

    if (nAt >= 0)
    {
        objCalls.SetValueAt(nAt, eValue);
        return;
    }

    if (!objCalls.Add(eKey, eValue))
    {
        A_IMS_TRACE_D(AOSTAG, "AddCall :: map error", 0, 0, 0);
    }
}

PROTECTED
template <typename T>
void AosCallTracker::RemoveCall(OUT ImsMap<CallKey, T>& objCalls, IN CallKey eKey)
{
    IMS_SINT32 nAt = objCalls.GetIndexOfKey(eKey);

    if (nAt < 0)
    {
        return;
    }

    objCalls.RemoveAt(nAt);
}

PROTECTED
CallState AosCallTracker::GetConvertedState(IN IMtcCall::State eState)
{
    switch (eState)
    {
        case IMtcCall::State::IDLE:
            return CallState::NEW;

        case IMtcCall::State::OUTGOING:
            return CallState::RINGBACK;

        case IMtcCall::State::INCOMING:
            return CallState::RINGING;

        case IMtcCall::State::ALERTING:
            return CallState::ALERTING;

        case IMtcCall::State::ESTABLISHED:  // FALL-THROUGH
        case IMtcCall::State::UPDATING:
            return CallState::OFFHOOK;

        default:
            // IMtcCall::State::TERMINATING
            return CallState::IDLE;
    }
}

PROTECTED
CallState AosCallTracker::GetTotalState(IN ImsMap<CallKey, CallState>& objCalls)
{
    if (objCalls.IsEmpty())
    {
        return CallState::IDLE;
    }

    if (objCalls.GetSize() > 1)
    {
        return CallState::OFFHOOK;
    }

    return objCalls.GetValueAt(0);
}

PROTECTED
IMS_UINT32 AosCallTracker::GetTotalCallType(IN ImsMap<CallKey, CallType>& objCallTypes)
{
    IMS_UINT32 nTotalCallType = static_cast<IMS_UINT32>(CallType::UNKNOWN);

    if (objCallTypes.IsEmpty())
    {
        return nTotalCallType;
    }

    for (IMS_UINT32 nAt = 0; nAt < objCallTypes.GetSize(); ++nAt)
    {
        CallType eCallType = objCallTypes.GetValueAt(nAt);

        if (eCallType != CallType::UNKNOWN)
        {
            nTotalCallType |= 0x1 << static_cast<IMS_UINT32>(eCallType);
        }
    }

    return nTotalCallType;
}

PROTECTED
IMS_BOOL AosCallTracker::IsExistCallType(IN CallType eCallType) const
{
    return (m_nNormalCallType & (0x1 << static_cast<IMS_UINT32>(eCallType)));
}

PROTECTED
CallState AosCallTracker::GetState(IN IMS_UINT32 nType) const
{
    if (nType == TYPE_CS)
    {
        return m_eCsState;
    }

    if (nType == TYPE_NORMAL)
    {
        return m_eNormalState;
    }

    if (nType == TYPE_EMERGENCY)
    {
        return m_eEmergencyState;
    }

    return CallState::IDLE;
}

PROTECTED
void AosCallTracker::SetState(IN IMS_UINT32 nType, IN CallState eState)
{
    switch (nType)
    {
        case TYPE_CS:
            m_eCsState = eState;
            break;

        case TYPE_NORMAL:
            m_eNormalState = eState;
            break;

        case TYPE_EMERGENCY:
            m_eEmergencyState = eState;
            break;

        default:
            break;
    }
}

PROTECTED
void AosCallTracker::Notify(IN IMS_UINT32 nType, IN CallState eState)
{
    for (IMS_UINT32 nAt = 0; nAt < m_objListeners.GetSize(); nAt++)
    {
        IAosCallTrackerListener* piListener = m_objListeners.GetAt(nAt);

        if (piListener != IMS_NULL)
        {
            piListener->CallTracker_StateChanged(nType, eState);
        }
    }
}

PROTECTED void AosCallTracker::ProcessCsChanged(IN CallState eState)
{
    if (m_eCsState != eState)
    {
        A_IMS_TRACE_I(AOSTAG, "ProcessCsChanged :: old (%s) -> curr (%s)",
                StateToString(m_eCsState), StateToString(eState), 0);

        SetState(TYPE_CS, eState);
        Notify(TYPE_CS, eState);
    }
}

PROTECTED void AosCallTracker::ProcessEmergencyChanged(IN CallKey eKey, IN CallState eState)
{
    if (eState == CallState::IDLE)
    {
        RemoveCall(m_objEmergencyCalls, eKey);
    }
    else
    {
        AddOrUpdateCall(m_objEmergencyCalls, eKey, eState);
    }

    CallState eCurrState = GetTotalState(m_objEmergencyCalls);

    A_IMS_TRACE_I(AOSTAG, "ProcessEmergencyChanged :: old (%s) -> curr (%s)",
            StateToString(m_eEmergencyState), StateToString(eCurrState), 0);

    if (m_eEmergencyState != eCurrState)
    {
        SetState(TYPE_EMERGENCY, eCurrState);
        Notify(TYPE_EMERGENCY, eCurrState);
    }
}

PROTECTED void AosCallTracker::ProcessNormalChanged(
        IN CallKey nCallKey, IN CallState eCallState, IN CallType eCallType)
{
    if (eCallState == CallState::IDLE)
    {
        RemoveCall(m_objNormalCalls, nCallKey);
        RemoveCall(m_objNormalCallTypes, nCallKey);
    }
    else
    {
        AddOrUpdateCall(m_objNormalCalls, nCallKey, eCallState);
        AddOrUpdateCall(m_objNormalCallTypes, nCallKey, eCallType);
    }

    CallState eCurrState = GetTotalState(m_objNormalCalls);
    IMS_UINT32 nCurrCallTypes = GetTotalCallType(m_objNormalCallTypes);

    A_IMS_TRACE_I(AOSTAG, "ProcessNormalChanged :: Old State(%s) -> Curr State(%s)",
            StateToString(m_eNormalState), StateToString(eCurrState), 0);
    PrintCallTypes(nCurrCallTypes);

    if (m_eNormalState != eCurrState || m_nNormalCallType != nCurrCallTypes)
    {
        SetState(TYPE_NORMAL, eCurrState);
        m_nNormalCallType = nCurrCallTypes;

        Notify(TYPE_NORMAL, eCurrState);
    }
}

PROTECTED VIRTUAL void AosCallTracker::Event_NotifyEvent(
        IN IMS_SINT32 nEvent, IN IMS_UINT32 nWParam, IN IMS_UINT32 nLParam)
{
    A_IMS_TRACE_I(AOSTAG, "Event_NotifyEvent :: [E(%d)/W(%d)/L(%d)]", nEvent, nWParam, nLParam);

    if (nEvent == IMS_EVENT_CSCALL_STATE)
    {
        if (nWParam == IMS_CSCALL_STATE_IDLE)
        {
            ProcessCsChanged(CallState::IDLE);
        }
        else if (nWParam == IMS_CSCALL_STATE_INCOMING)
        {
            ProcessCsChanged(CallState::RINGING);
        }
        else if (nWParam == IMS_CSCALL_STATE_ACTIVE)
        {
            ProcessCsChanged(CallState::OFFHOOK);
        }
    }
}

PUBLIC VIRTUAL void AosCallTracker::ServicePhone_PreciseCallStateChanged(IN PreciseCallState eState)
{
    A_IMS_TRACE_I(AOSTAG, "ServicePhone_PreciseCallStateChanged :: eState (%d)", eState, 0, 0);

    if (eState == PreciseCallState::NOT_VALID || eState == PreciseCallState::IDLE ||
            eState == PreciseCallState::DIALING || eState == PreciseCallState::DISCONNECTED ||
            eState == PreciseCallState::DISCONNECTING)
    {
        ProcessCsChanged(CallState::IDLE);
    }
    else if (eState == PreciseCallState::INCOMING || eState == PreciseCallState::WAITING ||
            eState == PreciseCallState::ALERTING)
    {
        ProcessCsChanged(CallState::RINGING);
    }
    else if (eState == PreciseCallState::ACTIVE || eState == PreciseCallState::HOLDING)
    {
        ProcessCsChanged(CallState::OFFHOOK);
    }
}

PROTECTED VIRTUAL void AosCallTracker::OnCallStateChanged(IN CallKey nCallKey, IN State eState,
        IN CallType eType, IN IMS_BOOL bEmergency, IN IMS_SINT32 nReason)
{
    CallState eCallState = GetConvertedState(eState);

    AString strLog;
    strLog.Sprintf("CallKey=%" PFLS_x ", State=%s, Type=%s, Emergency=%s, Reason=%d", nCallKey,
            StateToString(eCallState), CallTypeToString(eType), _TRACE_B_(bEmergency), nReason);
    A_IMS_TRACE_I(AOSTAG, "OnCallStateChanged :: (%s)", strLog.GetStr(), 0, 0);

    if (bEmergency)
    {
        ProcessEmergencyChanged(nCallKey, eCallState);
        return;
    }

    switch (eType)
    {
        case CallType::VOIP:  // FALL-THROUGH
        case CallType::VT:    // FALL-THROUGH
        case CallType::RTT:   // FALL-THROUGH
        case CallType::VIDEO_RTT:
            ProcessNormalChanged(nCallKey, eCallState, eType);
            break;

        default:
            break;
    }
}

PROTECTED VIRTUAL void AosCallTracker::OnTotalCallStateChanged(IN State /* eState */) {}

PROTECTED GLOBAL const IMS_CHAR* AosCallTracker::TypeToString(IN IMS_UINT32 nType)
{
    switch (nType)
    {
        case TYPE_CS:
            return "TYPE_CS";

        case TYPE_NORMAL:
            return "TYPE_NORMAL";

        case TYPE_EMERGENCY:
            return "TYPE_EMERGENCY";

        default:
            return "__INVALID__";
    }
}

PROTECTED GLOBAL const IMS_CHAR* AosCallTracker::StateToString(IN CallState eState)
{
    switch (eState)
    {
        case CallState::IDLE:
            return "IDLE";

        case CallState::NEW:
            return "NEW";

        case CallState::RINGBACK:
            return "RINGBACK";

        case CallState::RINGING:
            return "RINGING";

        case CallState::ALERTING:
            return "ALERTING";

        case CallState::OFFHOOK:
            return "OFFHOOK";

        default:
            return "INVALID";
    }
}

PROTECTED GLOBAL const IMS_CHAR* AosCallTracker::CallTypeToString(IN CallType eType)
{
    switch (eType)
    {
        case CallType::VOIP:
            return "VOIP";

        case CallType::VT:
            return "VT";

        case CallType::RTT:
            return "RTT";

        case CallType::VIDEO_RTT:
            return "VIDEO_RTT";

        default:
            return "UNKNOWN";
    }
}

PROTECTED GLOBAL AString AosCallTracker::PrintCallTypes(IN IMS_UINT32 nCallTypes)
{
    AString strCallTypes(AString::ConstEmpty());

    if (nCallTypes & (0x1 << static_cast<IMS_UINT32>(CallType::VOIP)))
    {
        strCallTypes.Append("VOIP");
    }

    if (nCallTypes & (0x1 << static_cast<IMS_UINT32>(CallType::VT)))
    {
        if (strCallTypes.GetLength() != 0)
        {
            strCallTypes.Append(", ");
        }

        strCallTypes.Append("VT");
    }

    if (nCallTypes & (0x1 << static_cast<IMS_UINT32>(CallType::RTT)))
    {
        if (strCallTypes.GetLength() != 0)
        {
            strCallTypes.Append(", ");
        }

        strCallTypes.Append("RTT");
    }

    if (nCallTypes & (0x1 << static_cast<IMS_UINT32>(CallType::VIDEO_RTT)))
    {
        if (strCallTypes.GetLength() != 0)
        {
            strCallTypes.Append(", ");
        }

        strCallTypes.Append("VIDEO_RTT");
    }

    if (strCallTypes.GetLength() == 0)
    {
        strCallTypes.Append("NONE");
    }

    A_IMS_TRACE_I(AOSTAG, "Call Types (%s)", strCallTypes.GetStr(), 0, 0);

    return strCallTypes;
}