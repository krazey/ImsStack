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
#include "ImsEventDef.h"

//#include "UCConnector.h" _UC_TO_MTC_

#include "interface/IAosCallTrackerListener.h"
#include "interface/IAosService.h"
#include "provider/AosUtil.h"
#include "provider/AosCallTracker.h"
#include "provider/AosProvider.h"

__IMS_TRACE_TAG_USER_DECL__("AOS");

#define AOSTAG strTag.GetStr()

/*

Remarks

*/
PUBLIC
AosCallTracker::AosCallTracker(IN IMS_SINT32 nSlotId_)
    : nSlotId(nSlotId_)
    , nCSState(STATE_IDLE)
    , nNormalState(STATE_IDLE)
    , nEmergencyState(STATE_IDLE)
    , nNormalSessionType(SESSION_TYPE_NONE)
    , nActiveCSState(STATE_IDLE)
    , objNormalCalls(IMSMap<IMS_SINTP, IMS_UINT32>())
    , objEmergencyCalls(IMSMap<IMS_SINTP, IMS_UINT32>())
    , objNormalSessionTypes(IMSMap<IMS_SINTP, IMS_UINT32>())
    , objListeners(IMSList<IAosCallTrackerListener*>())
{
    IMS_TRACE_MEM("AOS_MEM", "AOS_M : [SLOT%d] AosCallTracker = %" PFLS_u "/%" PFLS_x, nSlotId,
        sizeof(AosCallTracker), this);

    IMS_EVENT_AddListenerForSlotId(IMS_EVENT_CSCALL_STATE, this, nSlotId);

    // listen the uc call tracker
    //UCConnector::AddCallTrackerListener(nSlotId, this); _UC_TO_MTC_

    strTag.Sprintf("%d", nSlotId);
}

/*

Remarks

*/
PUBLIC VIRTUAL
AosCallTracker::~AosCallTracker()
{
    IMS_TRACE_MEM("AOS_MEM", "AOS_F : [SLOT%d] AosCallTracker = %" PFLS_u "/%" PFLS_x, nSlotId,
        sizeof(AosCallTracker), this);

    //UCConnector::RemoveCallTrackerListener(nSlotId, this); _UC_TO_MTC_
    objListeners.Clear();

    AosProvider::GetInstance()->GetService(nSlotId)->RemoveListener(
            DYNAMIC_CAST(IAosServicePhoneListener*, this));
    IMS_EVENT_RemoveListenerForSlotId(IMS_EVENT_CSCALL_STATE, this, nSlotId);
}

/*

Remarks

*/
PUBLIC VIRTUAL
IMS_BOOL AosCallTracker::IsCSCallActive() const
{
    A_IMS_TRACE_I(AOSTAG, "IsCSCallActive :: active (%s) , state (%d)",
                    _TRACE_B_(nCSState > nActiveCSState), nCSState, 0);
    return (nCSState > nActiveCSState);
}

/*

Remarks

*/
PUBLIC VIRTUAL
IMS_BOOL AosCallTracker::IsNormalCallActive() const
{
    A_IMS_TRACE_I(AOSTAG, "IsNormalCallActive :: active (%s) , state (%d)",
                    _TRACE_B_(nNormalState > STATE_IDLE), nNormalState, 0);
    return (nNormalState > STATE_IDLE);
}


/*

Remarks

*/
PUBLIC VIRTUAL
IMS_BOOL AosCallTracker::IsVideoCallingActive() const
{
    if (!AosUtil::GetInstance()->IsFeatureOn(SESSION_TYPE_VT, nNormalSessionType)
            || objNormalSessionTypes.IsEmpty())
    {
        A_IMS_TRACE_I(AOSTAG, "IsVideoCallingActive(%s)", "false", 0, 0);
        return IMS_FALSE;
    }

    IMS_BOOL bSessionActive = IMS_FALSE;

    for (IMS_UINT32 nAt = 0; nAt < objNormalSessionTypes.GetSize(); ++nAt)
    {
        /* _UC_TO_MTC_
        IMS_UINT32 nSessionType = objNormalSessionTypes.GetValueAt(nAt);

        if (nSessionType != IUCCallListener::SESSIONTYPE_VT)
        {
            continue;
        }
        */

        IMS_SINTP nSessionKey = objNormalSessionTypes.GetKeyAt(nAt);
        IMS_UINT32 nSessionState = objNormalCalls.GetValue(nSessionKey);

        if (nSessionState == STATE_OFFHOOK)
        {
            bSessionActive = IMS_TRUE;
            break;
        }
    }

    A_IMS_TRACE_I(AOSTAG, "IsVideoCallingActive(%s)", _TRACE_B_(bSessionActive), 0, 0);

    return bSessionActive;
}


/*

Remarks

*/
PUBLIC VIRTUAL
IMS_BOOL AosCallTracker::IsEmergencyCallActive() const
{
    A_IMS_TRACE_I(AOSTAG, "IsEmergencyCallActive :: active (%s) , state (%d)",
                        _TRACE_B_(nEmergencyState > STATE_IDLE), nEmergencyState, 0);
    return (nEmergencyState > STATE_IDLE);
}

/*

Remarks

*/
PUBLIC VIRTUAL
IMS_SINT32 AosCallTracker::GetSlotId() const
{
    return nSlotId;
}

/*

Remarks

*/
PUBLIC VIRTUAL
IMS_UINT32 AosCallTracker::GetCallState(IN IMS_UINT32 nType) const
{
    if (nType == TYPE_CS)
    {
        return nCSState;
    }

    if (nType == TYPE_NORMAL)
    {
        return nNormalState;
    }

    if (nType == TYPE_EMERGENCY)
    {
        return nEmergencyState;
    }

    return STATE_IDLE;
}

/*

Remarks

*/
PUBLIC VIRTUAL
IMS_UINT32 AosCallTracker::GetSessionType(IN IMS_UINT32 nType) const
{
    if (nType == TYPE_NORMAL)
    {
        return nNormalSessionType;
    }

    return SESSION_TYPE_NONE;
}

/*

Remarks

*/
PUBLIC VIRTUAL
void AosCallTracker::SetCSCallStateWatchMode()
{
    A_IMS_TRACE_D(AOSTAG, "SetCSCallStateWatchMode", 0, 0, 0);

    IMS_EVENT_RemoveListenerForSlotId(IMS_EVENT_CSCALL_STATE, this, nSlotId);
    AosProvider::GetInstance()->GetService(nSlotId)->AddListener(
            DYNAMIC_CAST(IAosServicePhoneListener*, this));
}

/*

Remarks

*/
PUBLIC VIRTUAL
void AosCallTracker::SetActiveCSCallState(IN IMS_UINT32 nActiveCSState)
{
    A_IMS_TRACE_D(AOSTAG, "SetActiveCSCallState :: (%d)", nActiveCSState, 0, 0);
    this->nActiveCSState = nActiveCSState;
}

/*

Remarks

*/
PUBLIC VIRTUAL
void AosCallTracker::SetListener(IN IAosCallTrackerListener* piListener)
{
    for (IMS_UINT32 i = 0; i < objListeners.GetSize(); ++i)
    {
        IAosCallTrackerListener *pTmpListener = objListeners.GetAt(i);

        if (pTmpListener == piListener)
        {
            A_IMS_TRACE_D(AOSTAG, "SetListener() :: Listener (%" PFLS_x ") is already set",
                piListener, 0, 0);
            return;
        }
    }

    objListeners.Append(piListener);
}

/*

Remarks

*/
PUBLIC VIRTUAL
void AosCallTracker::RemoveListener(IN IAosCallTrackerListener* piListener)
{
    for (IMS_UINT32 i = 0; i < objListeners.GetSize(); ++i)
    {
        IAosCallTrackerListener *pTmpListener = objListeners.GetAt(i);

        if (pTmpListener == piListener)
        {
            objListeners.RemoveAt(i);

            A_IMS_TRACE_D(AOSTAG, "RemoveListener :: Listener (%" PFLS_x ") is removed",
                piListener, 0, 0);
            return;
        }
    }
}

/*

Remarks

*/
PROTECTED
void AosCallTracker::AddOrUpdateCall(IN IMSMap<IMS_SINTP, IMS_UINT32> &objCalls,
    IN IMS_SINTP nKey, IN IMS_UINT32 nState)
{
    IMS_SINT32 nAt = objCalls.GetIndexOfKey(nKey);

    if (nAt >= 0)
    {
        objCalls.SetValueAt(nAt, nState);
        return;
    }

    if (!objCalls.Add(nKey, nState))
    {
        A_IMS_TRACE_D(AOSTAG, "AddCall :: map error", 0, 0, 0);
    }
}

/*

Remarks

*/
PROTECTED
void AosCallTracker::RemoveCall(IN IMSMap<IMS_SINTP, IMS_UINT32> &objCalls, IN IMS_SINTP nKey)
{
    IMS_SINT32 nAt = objCalls.GetIndexOfKey(nKey);

    if (nAt < 0)
    {
        return;
    }

    objCalls.RemoveAt(nAt);
}

/*

Remarks

*/
PROTECTED
IMS_UINT32 AosCallTracker::GetConvertedState(IN IMS_UINT32 nState)
{
    switch (nState)
    {
        /* _UC_TO_MTC_
        case IUCCallListener::UC_CALL_STATE_IDLE:
            return STATE_IDLE;

        case IUCCallListener::UC_CALL_STATE_TERMINATING:
            return STATE_TERMINATING;

        case IUCCallListener::UC_CALL_STATE_RINGBACK:
            return STATE_RINGBACK;

        case IUCCallListener::UC_CALL_STATE_RINGING:
            return STATE_RINGING;

        case IUCCallListener::UC_CALL_STATE_ALERTING:
            return STATE_ALERTING;

        case IUCCallListener::UC_CALL_STATE_OFFHOOK:
            return STATE_OFFHOOK;
        */
        default:
            break;
    }

    return STATE_IDLE;
}

/*

Remarks

*/
PROTECTED
IMS_UINT32 AosCallTracker::GetTotalState(IN IMSMap<IMS_SINTP, IMS_UINT32> &objCalls)
{
    if (objCalls.IsEmpty())
    {
        return STATE_IDLE;
    }

    if (objCalls.GetSize() > 1)
    {
        return STATE_OFFHOOK;
    }

    return objCalls.GetValueAt(0);
}

/*

Remarks

*/
PROTECTED
IMS_UINT32 AosCallTracker::GetTotalSessionType
                (IN IMSMap<IMS_SINTP, IMS_UINT32> &objSessionTypes)
{
    if (objSessionTypes.IsEmpty())
    {
        return SESSION_TYPE_NONE;
    }

    IMS_UINT32 nTotalSessionType = SESSION_TYPE_NONE;

    for (IMS_UINT32 nAt = 0; nAt < objSessionTypes.GetSize(); ++nAt)
    {
        IMS_UINT32 nSessionType = objSessionTypes.GetValueAt(nAt);

        switch (nSessionType)
        {
            /* _UC_TO_MTC_
            case IUCCallListener::SESSIONTYPE_VOIP:
                nTotalSessionType |= SESSION_TYPE_VOIP;
                break;

            case IUCCallListener::SESSIONTYPE_VIDEOSHARE:
                nTotalSessionType |= Session_TYPE_VS;
                break;

            case IUCCallListener::SESSIONTYPE_VT:
                nTotalSessionType |= SESSION_TYPE_VT;
                break;
            */
            default:
                break;
        }
    }

    return nTotalSessionType;
}

/*

Remarks

*/
PROTECTED
IMS_UINT32 AosCallTracker::GetState(IN IMS_UINT32 nType) const
{
    if (nType == TYPE_CS)
    {
        return nCSState;
    }

    if (nType == TYPE_NORMAL)
    {
        return nNormalState;
    }

    if (nType == TYPE_EMERGENCY)
    {
        return nEmergencyState;
    }

    return STATE_IDLE;
}

/*

Remarks

*/
PROTECTED
void AosCallTracker::SetState(IN IMS_UINT32 nType, IN IMS_UINT32 nState)
{
    switch (nType)
    {
        case TYPE_CS:
            nCSState = nState;
            break;

        case TYPE_NORMAL:
            nNormalState = nState;
            break;

        case TYPE_EMERGENCY:
            nEmergencyState = nState;
            break;

        default:
            break;
    }
}

/*

Remarks

*/
PROTECTED
void AosCallTracker::Notify(IN IMS_UINT32 nType, IN IMS_UINT32 nState)
{
    for(IMS_UINT32 nAt = 0; nAt < objListeners.GetSize(); nAt++)
    {
        IAosCallTrackerListener *piListener = objListeners.GetAt(nAt);

        if (piListener != IMS_NULL)
        {
            piListener->CallTracker_StateChanged(nType, nState);
        }
    }
}

/*

Remarks

*/
PROTECTED VIRTUAL
void AosCallTracker::ProcessCSChanged(IN IMS_UINT32 nState)
{
    if (nCSState != nState)
    {
        A_IMS_TRACE_I(AOSTAG, "ProcessCSChanged :: old (%s) -> curr (%s)",
                StateToString(nCSState), StateToString(nState), 0);

        SetState(TYPE_CS, nState);
        Notify(TYPE_CS, nState);
    }
}

/*

Remarks

*/
PROTECTED VIRTUAL
void AosCallTracker::ProcessEmergencyChanged(IN IMS_SINTP nKey, IN IMS_UINT32 nState)
{
    if (nState == STATE_IDLE)
    {
        RemoveCall(objEmergencyCalls, nKey);
    }
    else
    {
        AddOrUpdateCall(objEmergencyCalls, nKey, nState);
    }

    IMS_UINT32 nCurrState = GetTotalState(objEmergencyCalls);

    A_IMS_TRACE_I(AOSTAG, "ProcessEmergencyChanged :: old (%s) -> curr (%s)",
            StateToString(nEmergencyState), StateToString(nCurrState), 0);

    if (nEmergencyState != nCurrState)
    {
        SetState(TYPE_EMERGENCY, nCurrState);
        Notify(TYPE_EMERGENCY, nCurrState);
    }
}

/*

Remarks

*/
PROTECTED VIRTUAL
void AosCallTracker::ProcessNormalChanged
        (IN IMS_SINTP nKey, IN IMS_UINT32 nState, IN IMS_SINT32 nSessionType)
{
    IMS_UINT32 nSessType = (nSessionType < 0) ? 0 : static_cast<IMS_UINT32>(nSessionType);

    if (nState == STATE_IDLE)
    {
        RemoveCall(objNormalCalls, nKey);
        RemoveCall(objNormalSessionTypes, nKey);
    }
    else
    {
        AddOrUpdateCall(objNormalCalls, nKey, nState);
        AddOrUpdateCall(objNormalSessionTypes, nKey, nSessType);
    }

    IMS_UINT32 nCurrState = GetTotalState(objNormalCalls);
    IMS_UINT32 nCurrSessType = GetTotalSessionType(objNormalSessionTypes);

    A_IMS_TRACE_I(AOSTAG, "ProcessNormalChanged :: Old State(%s) -> Curr State(%s)",
            StateToString(nNormalState), StateToString(nCurrState), 0);
    PrintSessionType(nCurrSessType);

    if ((nNormalState != nCurrState) || (nNormalSessionType != nCurrSessType))
    {
        SetState(TYPE_NORMAL, nCurrState);
        nNormalSessionType = nCurrSessType;

        Notify(TYPE_NORMAL, nCurrState);
    }
}

/*

Remarks

*/
PROTECTED VIRTUAL
void AosCallTracker::Event_NotifyEvent(IN IMS_SINT32 nEvent,
    IN IMS_UINT32 nWParam, IN IMS_UINT32 nLParam)
{
    A_IMS_TRACE_I(AOSTAG, "Event_NotifyEvent :: [E(%d)/W(%d)/L(%d)]",
        nEvent, nWParam, nLParam);

    if (nEvent == IMS_EVENT_CSCALL_STATE)
    {
        if (nWParam == IMS_CSCALL_STATE_IDLE)
        {
            ProcessCSChanged(STATE_IDLE);
        }
        else if (nWParam == IMS_CSCALL_STATE_INCOMING)
        {
            ProcessCSChanged(STATE_RINGING);
        }
        else if (nWParam == IMS_CSCALL_STATE_ACTIVE)
        {
            ProcessCSChanged(STATE_OFFHOOK);
        }
    }
}

PUBLIC VIRTUAL
void AosCallTracker::ServicePhone_PreciseCallStateChanged(IN PreciseCallState eState)
{
    A_IMS_TRACE_I(AOSTAG, "ServicePhone_PreciseCallStateChanged :: eState (%d)", eState, 0, 0);

    if (eState == PreciseCallState::NOT_VALID ||
            eState == PreciseCallState::IDLE ||
            eState == PreciseCallState::DIALING ||
            eState == PreciseCallState::DISCONNECTED ||
            eState == PreciseCallState::DISCONNECTING)
    {
        ProcessCSChanged(STATE_IDLE);
    }
    else if (eState == PreciseCallState::INCOMING ||
            eState == PreciseCallState::WAITING ||
            eState == PreciseCallState::ALERTING)
    {
        ProcessCSChanged(STATE_RINGING);
    }
    else if (eState == PreciseCallState::ACTIVE ||
            eState == PreciseCallState::HOLDING)
    {
        ProcessCSChanged(STATE_OFFHOOK);
    }
}

/*

Remarks

*/
PROTECTED VIRTUAL
void AosCallTracker::ChangedCallState(IN IMS_UINTP nParam)
{
    if (nParam == IMS_NULL)
    {
        return;
    }
/* _UC_TO_MTC_
    IUCCallListenChangedStateParam *piState =
            reinterpret_cast<IUCCallListenChangedStateParam*>(nParam);

    if (piState->nKey == IMS_NULL)
    {
        delete piState;
        return;
    }

    A_IMS_TRACE_I(AOSTAG, "ChangedCallState :: nKey (%p) , nService (%d) , nState (%d)",
            piState->nKey, piState->eServiceType, piState->eState);

    switch (piState->eServiceType)
    {
        case IUCCallListener::SERVICETYPE_VOIP: // FALL-THROUGH
        case IUCCallListener::SERVICETYPE_VT: // FALL-THROUGH
        case IUCCallListener::SERVICETYPE_UC:
            ProcessNormalChanged(
                piState->nKey, GetConvertedState(piState->eState), piState->eSessionType);
            break;

        case IUCCallListener::SERVICETYPE_EMERGENCY:
            ProcessEmergencyChanged(piState->nKey, GetConvertedState(piState->eState));
            break;

        default:
            break;
    }

    delete piState;
*/
}

/*

Remarks

*/
PROTECTED VIRTUAL
void AosCallTracker::ChangedCallTotalState(IN IMS_UINTP nParam)
{
    if (nParam == IMS_NULL)
    {
        return;
    }
/* _UC_TO_MTC_
    IUCCallListenChangedTotalStateParam *piTotalState =
            reinterpret_cast<IUCCallListenChangedTotalStateParam*>(nParam);

    if (piTotalState->eState == IUCCallListener::UC_CALL_STATE_IDLE)
    {
        IMS_UINT32 nNormalCount = objNormalCalls.GetSize();
        IMS_UINT32 nEmergencyCount = objEmergencyCalls.GetSize();
        IMS_UINT32 nNormalSessTypeCount = objNormalSessionTypes.GetSize();

        A_IMS_TRACE_I(AOSTAG,
            "ChangedCallTotalState :: Count Normal(%d) , Emergency(%d), SessionType(%d)",
            nNormalCount, nEmergencyCount, nNormalSessTypeCount);

        if (nNormalCount > 0 || nNormalSessTypeCount > 0)
        {
            objNormalCalls.Clear();
            objNormalSessionTypes.Clear();

            SetState(TYPE_NORMAL, STATE_IDLE);
            nNormalSessionType = SESSION_TYPE_NONE;

            Notify(TYPE_NORMAL, STATE_IDLE);
        }

        if (nEmergencyCount > 0)
        {
            objEmergencyCalls.Clear();

            SetState(TYPE_EMERGENCY, STATE_IDLE);
            Notify(TYPE_EMERGENCY, STATE_IDLE);
        }
    }

    delete piTotalState;
*/
}

/*

Remarks

*/
PROTECTED GLOBAL
const IMS_CHAR* AosCallTracker::TypeToString(IN IMS_UINT32 nType)
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

/*

Remarks

*/
PROTECTED GLOBAL
const IMS_CHAR* AosCallTracker::StateToString(IN IMS_UINT32 nState)
{
    switch (nState)
    {
        case STATE_IDLE:
            return "STATE_IDLE";

        case STATE_TERMINATING:
            return "STATE_TERMINATING";

        case STATE_RINGBACK:
            return "STATE_RINGBACK";

        case STATE_RINGING:
            return "STATE_RINGING";

        case STATE_ALERTING:
            return "STATE_ALERTING";

        case STATE_OFFHOOK:
            return "STATE_OFFHOOK";

        default:
            return "__INVALID__";
    }
}

/*

Remarks

*/
PROTECTED GLOBAL
void AosCallTracker::PrintSessionType(IN IMS_UINT32 nSessType)
{
    AString strSessionTypes(AString::ConstEmpty());

    if (AosUtil::GetInstance()->IsFeatureOn(SESSION_TYPE_VOIP, nSessType))
    {
        strSessionTypes.Append("VoIP");
    }

    if (AosUtil::GetInstance()->IsFeatureOn(Session_TYPE_VS, nSessType))
    {
        if (strSessionTypes.GetLength() != 0)
        {
            strSessionTypes.Append(", ");
        }

        strSessionTypes.Append("VideoShare");
    }

    if (AosUtil::GetInstance()->IsFeatureOn(SESSION_TYPE_VT, nSessType))
    {
        if (strSessionTypes.GetLength() != 0)
        {
            strSessionTypes.Append(", ");
        }

        strSessionTypes.Append("VT");
    }

    if (strSessionTypes.GetLength() == 0)
    {
        strSessionTypes.Append("NONE");
    }

    A_IMS_TRACE_I(AOSTAG, "Session Types (%s)", strSessionTypes.GetStr(), 0, 0);
}
