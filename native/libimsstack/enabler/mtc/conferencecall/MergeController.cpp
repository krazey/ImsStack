#include "ServiceTrace.h"
#include "call/IMtcCallManager.h"
#include "conferencecall/MergeController.h"
#include "conferencecall/ConferenceConfigurationWrapper.h"
#include "conferencecall/ConferenceUtils.h"
#include "call/IMtcCallContext.h"
#include "IMSList.h"
#include "CallInfo.h"
#include "MtcDef.h"
#include "conferencecall/ConferenceOperationQueue.h"
#include "conferencecall/CallConnectionIdManager.h"
#include "helper/MtcSupplementaryService.h"

__IMS_TRACE_TAG_COM_MTC__;

PUBLIC
MergeController::MergeController(IN CallKey nConfCallKey, IMtcContext& objContext,
        IN CallConnectionIdManager& objConnectionIdManager) :
        ConferenceController(nConfCallKey, objContext, objConnectionIdManager)
{
    IMS_TRACE_I("+MergeController", 0, 0, 0);
}

PUBLIC VIRTUAL
MergeController::~MergeController()
{
    IMS_TRACE_I("~MergeController", 0, 0, 0);
}

PROTECTED VIRTUAL
void MergeController::ProcessMerge(IN IMSList<ConfUser*>& objUsers)
{
    IMS_TRACE_I("ProcessMerge user size[%d]", objUsers.GetSize(), 0, 0);

    if (IsReadyToPerformCmd() == IMS_FALSE)
    {
        m_objNotifier.NotifyMergeFailed(FailReason(FAIL_REASON_NONE));
        // SendClosed(); required?
        return;
    }

    if (GetState() == STATE_CREATED && objUsers.GetSize() < 2)
    {
        m_objNotifier.NotifyMergeFailed(FailReason(FAIL_REASON_NONE));
        SendClosed();
        return;
    }

    IMS_UINT32 nStartIndex = AddUserToParticipantList(objUsers, IMS_TRUE);
    if (ConferenceConfigurationWrapper::IsReferUsed() == IMS_FALSE)
    {
        return ProcessMergeWithoutRefer(objUsers);
    }

    IMS_BOOL bSubFirstAndRefer = ConferenceConfigurationWrapper::IsSubscriptionFirst();
    IMS_SINT32 nOldState = GetState();
    SetState(STATE_MERGING);

    // clear ConfUser list for not use in resource list in UCSession.
    ClearListForConfUsers(objUsers);

    if (nOldState == STATE_CREATED)
    {
        m_objOperationQueue.CreateNPut(CONTROL_OPERATION_CREATE_CONFERENCE_SESSION, objUsers);

        if (bSubFirstAndRefer == IMS_TRUE &&
                ConferenceConfigurationWrapper::IsConferenceSubscriptionRequired())
        {
            m_objOperationQueue.CreateNPut(CONTROL_OPERATION_SUBSCRIBE);
        }
    }

    IMS_TRACE_I("ProcessMerge [%d]", m_objParticipantList.GetSize(), 0, 0);
    for (IMS_UINT32 i = nStartIndex; i < m_objParticipantList.GetSize(); i++)
    {
        m_objOperationQueue.CreateNPut(CONTROL_OPERATION_REFER_INVITE,
                m_objParticipantList.GetConfUsers().GetAt(i));
        m_objOperationQueue.CreateNPut(CONTROL_OPERATION_TERMINATE_1TO1_SESSION,
                m_objParticipantList.GetConfUsers().GetAt(i)->nConnectionId);
    }

    m_objOperationQueue.CreateNPut(CONTROL_OPERATION_NOTIFY_RESULT_TO_UI);

    if (nOldState == STATE_CREATED)
    {
        if (bSubFirstAndRefer == IMS_FALSE &&
                ConferenceConfigurationWrapper::IsConferenceSubscriptionRequired())
        {
            m_objOperationQueue.CreateNPut(CONTROL_OPERATION_SUBSCRIBE);
        }
    }
    m_objOperationQueue.CreateNPut(CONTROL_OPERATION_NOTIFY_RESULT_TO_UCSESSION);
    m_objOperationQueue.SetAddingOperationSetCompleted();
}

PUBLIC VIRTUAL
void MergeController::StartConferenceCall(
        IN ConferenceOperationQueue::ConferenceOperation* /*pOperation*/)
{
    // TODO: CallType?
    // TODO: factory uri.
    IMSList<ConfUser*> objTempUser;
    GetConferenceCall()->StartConference(CallType::VOIP, AString::ConstNull(), objTempUser);
}

PROTECTED VIRTUAL
IMS_BOOL MergeController::IsStartFinalSipfragWaitTimer() const
{
    IMS_TRACE_I("IsStartFinalSipfragWaitTimer : [%d]", m_nConditionFinalSipfragTimer, 0, 0);

#if 0 // to cover the case there is not even R-NOTIFY [100 Trying]
    if (IsConditionMet(CONDITION_SIPFRAG_100_RECEIVED) == IMS_FALSE)
    {
        return IMS_FALSE;
    }
#endif

    if (IsConditionMet(CONDITION_1TO1_TERMINATED) == IMS_FALSE)
    {
        return IMS_FALSE;
    }

    return IMS_TRUE;
}

PROTECTED VIRTUAL
void MergeController::Recover()
{
    // do not call SendClosed() in Recovery logic.
    // it must be called in each logic based on the context.
    IMS_TRACE_I("Recover", 0, 0, 0);

    switch (m_objOperationQueue.GetTypeOfCurrentOperation())
    {
        case CONTROL_OPERATION_CREATE_CONFERENCE_SESSION:
            RecoverOnCreating();
            break;

        case CONTROL_OPERATION_TERMINATE_1TO1_SESSION:
        case CONTROL_OPERATION_REFER_INVITE:
            RecoverOnReferring();
            break;
        case CONTROL_OPERATION_REFER_BYE:
            // to be handled in OnReferenceStartFailed.
            break;
        case CONTROL_OPERATION_SUBSCRIBE:
            RecoverOnSubscribing();
            break;

        default:
            IMS_TRACE_I("Recover : not handled.", 0, 0, 0);
            break;
    }
}

PROTECTED VIRTUAL
void MergeController::OnIndividualCallTerminated(IN IMS_UINTP nCallKey)
{
    ConferenceController::OnIndividualCallTerminated(nCallKey);

    if (m_pSubscription == IMS_NULL &&
            ConferenceConfigurationWrapper::IsReferUsed() == IMS_FALSE)
    {
        UpdateUserStateBySessionTerminated(nCallKey);
    }
}

PRIVATE
void MergeController::ProcessMergeWithoutRefer(IN IMSList<ConfUser*>& objUsers)
{
    IMS_TRACE_I("ProcessMergeWithoutRefer", 0, 0, 0);

    IMS_SINT32 nOldState = GetState();
    SetState(STATE_MERGING);

    if (nOldState == STATE_CREATED)
    {
        m_objOperationQueue.CreateNPut(CONTROL_OPERATION_CREATE_CONFERENCE_SESSION, objUsers);
        m_objOperationQueue.CreateNPut(CONTROL_OPERATION_NOTIFY_RESULT_TO_UI);
        m_objOperationQueue.SetAddingOperationSetCompleted();
    }
}

PRIVATE
void MergeController::UpdateUserStateBySessionTerminated(IN IMS_UINTP nCallKey)
{
    IMS_TRACE_I("UpdateUserStateBySessionTerminated : [%" PFLS_x "]", nCallKey, 0, 0);

    // KDDI
    for (IMS_UINT32 i = 0; i < m_objParticipantList.GetSize(); i++)
    {
        ConfUser* pUser = m_objParticipantList.GetConfUser(i);
        if (m_objConnectionIdManager.GetCallKey(
                m_objParticipantList.GetConfUser(i)->nConnectionId) == nCallKey)
        {
            pUser->eStatus = CONFINFO_STATUS_DISCONNECTED;
            NotifyUsersInfo();
            break;
        }
    }

    if (m_objParticipantList.GetConnectedParticipantSize(IMS_TRUE) == 0)
    {
        IMS_TRACE_D("UpdateUserStateBySessionTerminated : terminate conference by alone", 0, 0, 0);
        m_objOperationQueue.CreateNPut(CONTROL_OPERATION_TERMINATE_CONFERENCE,
                FAIL_REASON_CONF_ALONE, IMS_TRUE);
    }
}

PRIVATE
void MergeController::RecoverOnCreating()
{
    RecoverOnConferenceCallFailed();
}

PRIVATE
void MergeController::RecoverOnReferring()
{
    if (RecoverOnConferenceCallFailed())
    {
        return;
    }

    ConfUser* pConfUser = m_objOperationQueue.GetUsersOfCurrentOperation().GetAt(0);
    if (pConfUser && pConfUser->eStatus == CONFINFO_STATUS_CONNECTED)
    {
        // abnormal case. network doesn't send R-NOTIFY 200 but C-NOTIFY connected is received
        // the timer must be stopped.
        CompleteCurrentAndDoNextOperation(CONTROL_OPERATION_REFER_INVITE, pConfUser);
        return;
    }

    if (m_objParticipantList.GetConnectedParticipantSize() == 0)
    {
        IMS_TRACE_I("RecoverOnReferring : failure before start inviting members", 0, 0, 0);
        ClearIndividualCallOnMergeFailed(); // ??
        m_objNotifier.NotifyMergeFailed(FailReason(FAIL_REASON_UNKNOWN, -1));
        m_objOperationQueue.Clear();
        SetState(STATE_IDLE);
        return;
    }

    if (pConfUser && m_objParticipantList.GetSize() <= 2)
    {
        IMS_TRACE_I("RecoverOnReferring : failure after at least one member added.", 0, 0, 0);

        pConfUser->eStatus = CONFINFO_STATUS_DISCONNECTED;
        CompleteCurrentAndDoNextOperation(CONTROL_OPERATION_REFER_INVITE, pConfUser);
    }
    else
    {
        IMS_TRACE_I("RecoverOnReferring : failure during additional adding", 0, 0, 0);
        ClearIndividualCallOnMergeFailed();
        m_objNotifier.NotifyMergeFailed(FailReason(FAIL_REASON_UNKNOWN, -1));
        m_objOperationQueue.Clear();
        SetState(STATE_IDLE);
        return;
    }
}

PRIVATE
void MergeController::RecoverOnSubscribing()
{
    // TODO: remove.
}

PRIVATE
IMS_BOOL MergeController::RecoverOnConferenceCallFailed()
{
    IMS_TRACE_I("RecoverOnConferenceCallFailed", 0, 0, 0);

    IMtcCall* piConfCall = GetConferenceCall();
    if ((piConfCall->GetState() != IMtcCall::State::ESTABLISHED &&
            piConfCall->GetState() != IMtcCall::State::UPDATING))
    {
        ClearIndividualCallOnMergeFailed();
        m_objNotifier.NotifyMergeFailed(FailReason(FAIL_REASON_SESSION_TERMINATED, -1));
        m_objOperationQueue.Clear();
        return IMS_TRUE;
    }

    return IMS_FALSE;
}

PRIVATE
void MergeController::ClearIndividualCallOnMergeFailed()
{
    // this must be called before 'merge failed' event.
    IMS_TRACE_I("ClearIndividualCallOnMergeFailed", 0, 0, 0);

    for (IMS_UINT32 i = 0; i < m_objParticipantList.GetSize(); i++)
    {
        if (m_objParticipantList.GetAt(i)->IsInfoUpdated())
        {
            continue;
        }

        ConfUser* pTempUser = m_objParticipantList.GetConfUser(i);
        if (pTempUser == IMS_NULL)
        {
            continue;
        }
        CallKey nTempCallKey = m_objConnectionIdManager.GetCallKey(pTempUser->nConnectionId);

        IConferenceReference* piConfReference = m_objParticipantList.GetAt(i)->GetReference();

        if (piConfReference != IMS_NULL)
        {
            IMS_UINT32 nResponseCode = piConfReference->GetResponseCode();
            if (SIPStatusCode::IsFinalFailure(nResponseCode)
                    || nResponseCode == SIPStatusCode::SC_INVALID)
            {
                // refer is rejected. so, call can be maintained
                IMS_TRACE_I("ClearIndividualCallOnMergeFailed : refer is rejected.", 0, 0, 0);
                continue;
            }

            // refer is sent. so, call is considered terminated
            IMS_TRACE_I("ClearIndividualCallOnMergeFailed : refer is sent.", 0, 0, 0);
            // TODO: objContext.GetCallManager().GetCallByCallKey(pTempUser->nCallKey)
            // TODO: get JniMtcCallThread from IMtcCallContext of confuser.
            m_objNotifier.NotifyIndividualCallTerminated(nTempCallKey);
            continue;
        }

        if (pTempUser->eStatus != CONFINFO_STATUS_IDLE)
        {
            // state is updated already
            IMS_TRACE_I("ClearIndividualCallOnMergeFailed : user state changed", 0, 0, 0);
            // TODO: objContext.GetCallManager().GetCallByCallKey(pTempUser->nCallKey)
            // TODO: get JniMtcCallThread from IMtcCallContext of confuser.
            m_objNotifier.NotifyIndividualCallTerminated(nTempCallKey);
            continue;
        }

        IMtcCall* piTemp = m_objCallManager.GetCallByCallKey(nTempCallKey);
        if (piTemp == IMS_NULL || piTemp->GetState() == IMtcCall::State::TERMINATING)
        {
            IMS_TRACE_I("ClearIndividualCallOnMergeFailed : 1-to-1 is terminating", 0, 0, 0);
            // TODO: objContext.GetCallManager().GetCallByCallKey(pTempUser->nCallKey)
            // TODO: get JniMtcCallThread from IMtcCallContext of confuser.
            m_objNotifier.NotifyIndividualCallTerminated(nTempCallKey);
        }
    }
}
