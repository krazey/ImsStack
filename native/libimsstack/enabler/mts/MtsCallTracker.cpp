#include "ServiceTrace.h"
#include "ServiceEvent.h"

#include "IMtsCallTrackerListener.h"
#include "MtsCallTracker.h"

__IMS_TRACE_TAG_COM_SMS__;

PUBLIC
MtsCallTracker::MtsCallTracker(IN IMS_SINT32 nSlotId_) :
        nSlotId(nSlotId_),
        nEmergencyState(STATE_IDLE),
        objEmergencyCalls(IMSMap<IMS_SINTP, IMS_UINT32>()),
        objListeners(IMSList<IMtsCallTrackerListener*>())
{
    IMS_TRACE_I("MtsCallTracker constructor: nSlotId : [%d]", nSlotId, 0, 0);
    // TO-DO: Listen the mtc call tracker
}

PUBLIC
MtsCallTracker::~MtsCallTracker()
{
    IMS_TRACE_I("MtsCallTracker extinctor: nSlotId : [%d]", nSlotId, 0, 0);
    objListeners.Clear();
}

PUBLIC
IMS_BOOL MtsCallTracker::IsEmergencyCallActive() const
{
    IMS_TRACE_I("IsEmergencyCallActive :: nEmergencyState : [%d]", nEmergencyState, 0, 0);
    return (nEmergencyState > STATE_IDLE);
}

PUBLIC
IMS_SINT32 MtsCallTracker::GetSlotId() const
{
    return nSlotId;
}

PUBLIC
IMS_UINT32 MtsCallTracker::GetCallState(IN IMS_UINT32 nType) const
{
    if (nType == TYPE_EMERGENCY)
    {
        return nEmergencyState;
    }

    return STATE_IDLE;
}

PUBLIC
IMS_UINT32 MtsCallTracker::GetSessionType(IN IMS_UINT32 nType) const
{
    IMS_TRACE_I("GetSessionType :: nType : [%d]", nType, 0, 0);
    return SESSION_TYPE_NONE;
}

PUBLIC
void MtsCallTracker::AddListener(IN IMtsCallTrackerListener* piListener)
{
    for (IMS_UINT32 i = 0; i < objListeners.GetSize(); ++i)
    {
        IMtsCallTrackerListener* pTmpListener = objListeners.GetAt(i);

        if (pTmpListener == piListener)
        {
            IMS_TRACE_I("AddListener:: Listener is already added", 0, 0, 0);
            return;
        }
    }

    objListeners.Append(piListener);
}

PUBLIC
void MtsCallTracker::RemoveListener(IN IMtsCallTrackerListener* piListener)
{
    for (IMS_UINT32 i = 0; i < objListeners.GetSize(); ++i)
    {
        IMtsCallTrackerListener* pTmpListener = objListeners.GetAt(i);

        if (pTmpListener == piListener)
        {
            objListeners.RemoveAt(i);
            IMS_TRACE_I("RemoveListener:: Listener is removed", 0, 0, 0);
            return;
        }
    }
}

PROTECTED
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
        IMS_TRACE_E(0, "AddOrUpdateCall :: map error", 0, 0, 0);
    }
}

PROTECTED
void MtsCallTracker::RemoveCall(IN IMSMap<IMS_SINTP, IMS_UINT32>& objCalls, IN IMS_SINTP nKey)
{
    IMS_SINT32 nAt = objCalls.GetIndexOfKey(nKey);

    if (nAt < 0)
    {
        return;
    }

    objCalls.RemoveAt(nAt);
}

PROTECTED
IMS_UINT32 MtsCallTracker::GetConvertedState(IN IMS_UINT32 nState)
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

PROTECTED
IMS_UINT32 MtsCallTracker::GetTotalState(IN IMSMap<IMS_SINTP, IMS_UINT32>& objCalls)
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

PROTECTED
IMS_UINT32 MtsCallTracker::GetState(IN IMS_UINT32 nType) const
{
    if (nType == TYPE_EMERGENCY)
    {
        return nEmergencyState;
    }

    return STATE_IDLE;
}

PROTECTED
void MtsCallTracker::SetState(IN IMS_UINT32 nType, IN IMS_UINT32 nState)
{
    switch (nType)
    {
        case TYPE_EMERGENCY:
            nEmergencyState = nState;
            break;

        default:
            break;
    }
}

PROTECTED
void MtsCallTracker::Notify(IN IMS_UINT32 nType, IN IMS_UINT32 nState)
{
    for (IMS_UINT32 nAt = 0; nAt < objListeners.GetSize(); nAt++)
    {
        IMtsCallTrackerListener* piListener = objListeners.GetAt(nAt);

        if (piListener != IMS_NULL)
        {
            piListener->CallTracker_StateChanged(nType, nState);
        }
    }
}

PROTECTED
void MtsCallTracker::ProcessEmergencyChanged(IN IMS_SINTP nKey, IN IMS_UINT32 nState)
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
    IMS_TRACE_I("ProcessEmergencyChanged :: old (%s) -> curr (%s)", StateToString(nEmergencyState),
            StateToString(nCurrState), 0);

    if (nEmergencyState != nCurrState)
    {
        SetState(TYPE_EMERGENCY, nCurrState);
        Notify(TYPE_EMERGENCY, nCurrState);
    }
}

PROTECTED
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

        IMS_TRACE_I("ChangedCallState :: nKey (%p) , nService (%d) , nState (%d)", piState->nKey,
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

PROTECTED
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
            IMS_UINT32 nEmergencyCount = objEmergencyCalls.GetSize();
            IMS_TRACE_I("ChangedCallTotalState :: Emergency(%d)", nEmergencyCount, 0, 0);

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

PROTECTED GLOBAL const IMS_CHAR* MtsCallTracker::TypeToString(IN IMS_UINT32 nType)
{
    switch (nType)
    {
        case TYPE_EMERGENCY:
            return "TYPE_EMERGENCY";

        default:
            return "__INVALID__";
    }
}

PROTECTED GLOBAL const IMS_CHAR* MtsCallTracker::StateToString(IN IMS_UINT32 nState)
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
