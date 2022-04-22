#include "conferencecall/ConferenceEventNotifier.h"
#include "IuMtcCall.h"
#include "ServiceMemory.h"
#include "ServiceTrace.h"
#include "call/IMtcCallContext.h"
#include "call/IMtcCallManager.h"
#include "call/MtcUiNotifier.h"
#include "conferencecall/ConferenceParticipantList.h"
#include "conferencecall/CallConnectionIdManager.h"
#include "media/IMtcMediaManager.h"
#include "helper/MtcSupplementaryService.h"

__IMS_TRACE_TAG_COM_MTC__;

PUBLIC
ConferenceEventNotifier::ConferenceEventNotifier(IN IMtcCallContext& objConfCallContext,
        IN CallConnectionIdManager& objConnectionIdManager) :
        m_objConfCallContext(objConfCallContext),
        m_objConnectionIdManager(objConnectionIdManager)
{
    // TODO: memory leak.
    IMS_TRACE_I("+ConferenceEventNotifier", 0, 0, 0);
}

PUBLIC VIRTUAL
ConferenceEventNotifier::~ConferenceEventNotifier()
{
    IMS_TRACE_I("~ConferenceEventNotifier", 0, 0, 0);
}

PUBLIC
void ConferenceEventNotifier::NotifyMerged(IN ConferenceParticipantList& objParticipantList)
{
    IMS_TRACE_I("NotifyMerged", 0, 0, 0);

    objParticipantList.Login();

    m_objConfCallContext.GetUiNotifier().SendMerged(CloneCallInfo(), CloneMediaInfo(),
            m_objConfCallContext.GetSupplementaryService().GetAll(),
            objParticipantList.GetConfUsers(IMS_TRUE));
}

PUBLIC
void ConferenceEventNotifier::NotifyMergeFailed(IN FailReason failReason)
{
    IMS_TRACE_I("NotifyMergeFailed", 0, 0, 0);

    m_objConfCallContext.GetUiNotifier().SendMergeFailed(failReason);
}

PUBLIC
void ConferenceEventNotifier::NotifyGroupCallStarted()
{
    IMS_TRACE_I("NotifyGroupCallStarted", 0, 0, 0);

    // piCall->SetStartedToUI();

    IUUCSessionStartedParam* pParam = new IUUCSessionStartedParam();
    pParam->pCallInfo = CloneCallInfo();
    pParam->pMediaInfo = CloneMediaInfo();
    pParam->objSuppServices = m_objConfCallContext.GetSupplementaryService().GetAll();
}

PUBLIC
void ConferenceEventNotifier::NotifyGroupCallFailed(IN FailReason failReason)
{
    IMS_TRACE_I("NotifyGroupCallFailed", 0, 0, 0);

    IUUCSessionStartFailedParam* pParam = new IUUCSessionStartFailedParam();
    pParam->failReason = failReason;
}

PUBLIC
void ConferenceEventNotifier::NotifyExpanded()
{
    IMS_TRACE_I("NotifyExpanded", 0, 0, 0);

    // piCall->SetStartedToUI();

    IUUCSessionConfExpandedParam* pParam = new IUUCSessionConfExpandedParam();
    pParam->pCallInfo = CloneCallInfo();
    pParam->pMediaInfo = CloneMediaInfo();
    pParam->objSuppServices = m_objConfCallContext.GetSupplementaryService().GetAll();
}

PUBLIC
void ConferenceEventNotifier::NotifyExpandFailed(IN FailReason failReason)
{
    IMS_TRACE_I("NotifyExpandFailed", 0, 0, 0);

    IUUCSessionConfExpandFailedParam* pParam = new IUUCSessionConfExpandFailedParam();
    pParam->failReason = failReason;
}

PUBLIC
void ConferenceEventNotifier::NotifyDropped(IN FailReason failReason,
        IN ConferenceParticipantList& objParticipantList)
{
    IMS_TRACE_I("NotifyDropped", 0, 0, 0);

    m_objConfCallContext.GetUiNotifier().SendDropped(IMS_TRUE, failReason);

    NotifyUsersInfo(objParticipantList);
}

PUBLIC
void ConferenceEventNotifier::NotifyDropFailed(IN FailReason failReason,
        IN ConferenceParticipantList& objParticipantList)
{
    IMS_TRACE_I("NotifyDropFailed", 0, 0, 0);

    m_objConfCallContext.GetUiNotifier().SendDropped(IMS_FALSE, failReason);

    NotifyUsersInfo(objParticipantList);
}

PUBLIC
void ConferenceEventNotifier::NotifyJoined(IN FailReason failReason,
        IN ConferenceParticipantList& objParticipantList)
{
    IMS_TRACE_I("NotifyJoined", 0, 0, 0);

    m_objConfCallContext.GetUiNotifier().SendJoined(IMS_TRUE, failReason);

    NotifyUsersInfo(objParticipantList);
}

PUBLIC
void ConferenceEventNotifier::NotifyJoinFailed(IN FailReason failReason,
        IN ConferenceParticipantList& objParticipantList)
{
    IMS_TRACE_I("NotifyJoinFailed", 0, 0, 0);

    m_objConfCallContext.GetUiNotifier().SendJoined(IMS_FALSE, failReason);

    NotifyUsersInfo(objParticipantList);
}

PUBLIC
void ConferenceEventNotifier::NotifyConferenceInfo(IN ConferenceParticipantList& objParticipantList)
{
    IMS_TRACE_I("NotifyConferenceInfo : max-user-count=[%d]",
            objParticipantList.GetMaxUserCount(), 0, 0);

    m_objConfCallContext.GetUiNotifier().SendNotifyConfInfo("", "",
            objParticipantList.GetMaxUserCount(), objParticipantList.GetSize(), "");
}

PUBLIC
void ConferenceEventNotifier::NotifyUsersInfo(IN ConferenceParticipantList& objParticipantList)
{
    IMS_TRACE_I("NotifyUsersInfo", 0, 0, 0);
    objParticipantList.Login();

    IMSList<ConfUser*> objUsers = objParticipantList.GetConfUsers(IMS_TRUE);
    CheckDisconnectedConfUsersInfo(objParticipantList, objUsers);
    m_objConfCallContext.GetUiNotifier().SendNotifyUsersInfo(objUsers);
}

PUBLIC
void ConferenceEventNotifier::NotifyIndividualCallTerminated(IN CallKey nKey)
{
    IMS_TRACE_I("NotifyIndividualCallTerminated ", 0, 0, 0);

    IUUCSessionTerminatedParam* pParam = new IUUCSessionTerminatedParam();
    pParam->failReason =
            FailReason(FAIL_REASON_SESSION_TERMINATED, FAIL_REASON_SESSION_TERMINATED);

    m_objConfCallContext.GetCallManager().GetCallByCallKey(nKey)->GetCallContext().GetUiNotifier()
            .SendTerminated(
                FailReason(FAIL_REASON_SESSION_TERMINATED, FAIL_REASON_SESSION_TERMINATED));
}

PRIVATE
CallInfo* ConferenceEventNotifier::CloneCallInfo()
{
    // TODO: deprecated.
    return new CallInfo(m_objConfCallContext.GetCallInfo());
}

PRIVATE
MediaInfo* ConferenceEventNotifier::CloneMediaInfo()
{
    // TODO: deprecated.
    MediaInfo objMediaInfo;
    m_objConfCallContext.GetMediaManager().GetMediaInfo(objMediaInfo);
    return new MediaInfo(objMediaInfo);
}

PRIVATE
void ConferenceEventNotifier::CheckDisconnectedConfUsersInfo(
        IN ConferenceParticipantList& objParticipantList, IN_OUT IMSList<ConfUser*>& objUsers)
{
    for (IMS_SINT32 i = (objParticipantList.GetSize() - 1); i >= 0; i--)
    {
        ConferenceParticipantList::ConferenceParticipant* pParticipant =
                objParticipantList.GetAt(i);

        if (pParticipant == IMS_NULL)
        {
            continue;
        }

        IMS_UINT32 nStatus = pParticipant->GetConfUser()->eStatus;
        if (nStatus == CONFINFO_STATUS_DISCONNECTED ||
                nStatus == CONFINFO_STATUS_DISCONNECTING ||
                (nStatus >= CONFINFO_STATUS_FAIL && nStatus <= CONFINFO_STATUS_INTSERVERERROR))
                // TODO: list up all the status names, not range.
        {
            if (pParticipant->IsDisconnectionNotified())
            {
                ConfUser* pConfUser = objUsers.GetAt(i);
                IMS_TRACE_D("CheckDisconnectedConfUsersInfo : target[%s] status[%d]",
                        pConfUser->aStrTarget.GetStr(), pConfUser->eStatus, 0);

                delete pConfUser;
                objUsers.RemoveAt(i);
            }
            else
            {
                pParticipant->SetDisconnectionNotified(IMS_TRUE);
                m_objConnectionIdManager.OnConferenceParticipantDisconnected(
                        pParticipant->GetConfUser()->nConnectionId);
            }
        }
        else
        {
            // ??
            pParticipant->SetDisconnectionNotified(IMS_FALSE);
        }
    }
}
