#include "IMessage.h"
#include "IMSList.h"
#include "call/IMtcCallManager.h"
#include "IMtcContext.h"
#include "ISession.h"
#include "ISIPHeader.h"
#include "IMtcService.h"
#include "utility/MessageUtil.h"
#include "call/MtcCallController.h"
#include "MtcDef.h"
#include "ussi/UssiConstants.h"
#include "IuMtcService.h"
#include "JniMtcCallThread.h"
#include "conferencecall/IConferenceController.h"
#include "conferencecall/ConferenceManager.h"
#include "IMtcContext.h"
#include "ect/EctManager.h"

PUBLIC
MtcCallController::MtcCallController(IN IMtcContext &objContext) :
        m_objContext(objContext),
        m_objCallManager(objContext.GetCallManager())
{
}

PUBLIC VIRTUAL
MtcCallController::~MtcCallController()
{
}

PUBLIC
void MtcCallController::TerminateCalls(
        IN KeyType eKeyType, IN Key nKey, IN const FailReason& objReason)
{
    IMSList<IMtcCall*> lstCalls;
    switch (eKeyType)
    {
        case KeyType::NONE:
            lstCalls = m_objCallManager.GetCalls();
            break;

        case KeyType::CALL_KEY:
            lstCalls.Append(m_objCallManager.GetCallByCallKey(nKey.nCallKey));
            break;

        case KeyType::CALL_TYPE:
            lstCalls = m_objCallManager.GetCallsByType(nKey.eCallType);
            break;

        case KeyType::SERVICE_TYPE:
            lstCalls = m_objCallManager.GetCallsByServiceType(nKey.eServiceType);
            break;

        default:
            return;
    }

    for (IMS_SIZE_T i = 0; i < lstCalls.GetSize(); i++)
    {
        IMtcCall* piCall = lstCalls.GetAt(i);

        if (piCall != IMS_NULL)
        {
            piCall->Terminate(objReason);
        }
    }
}

PUBLIC
void MtcCallController::RemoveCalls(IN KeyType eKeyType, IN Key nKey)
{
    IMSList<IMtcCall*> lstCalls;
    switch (eKeyType)
    {
        case KeyType::NONE:
            lstCalls = m_objCallManager.GetCalls();
            break;

        case KeyType::CALL_KEY:
            lstCalls.Append(m_objCallManager.GetCallByCallKey(nKey.nCallKey));
            break;

        case KeyType::CALL_TYPE:
            lstCalls = m_objCallManager.GetCallsByType(nKey.eCallType);
            break;

        case KeyType::SERVICE_TYPE:
            lstCalls = m_objCallManager.GetCallsByServiceType(nKey.eServiceType);
            break;

        default:
            return;
    }

    for (IMS_SIZE_T i = 0; i < lstCalls.GetSize(); i++)
    {
        IMtcCall* piCall = lstCalls.GetAt(i);

        if (piCall != IMS_NULL)
        {
            m_objCallManager.RemoveCall(piCall->GetKey());
        }
    }
}

PUBLIC
CallKey MtcCallController::Open(IN ServiceType eServiceType, IN CallInfo& objCallInfo)
{
    return m_objCallManager.CreateCall(eServiceType, objCallInfo)
            ->GetKey();
}

PUBLIC
void MtcCallController::Attach(IN CallKey nCallKey, IN JniMtcCallThread* pJniMtcCallThread,
        IN JniMediaSessionThread* pJniMediaThread)
{
    m_objCallManager.GetCallByCallKey(nCallKey)
            ->Attach(pJniMtcCallThread, pJniMediaThread);
}

PUBLIC
void MtcCallController::Detach(IN CallKey nCallKey)
{
    IMtcCall* piMtcCall = m_objCallManager.GetCallByCallKey(nCallKey);
    piMtcCall->Detach();
    m_objCallManager.RemoveCall(nCallKey);
}

PUBLIC
void MtcCallController::HandleIncoming(
        IN IMtcService* pService, IN ISession* piSession,IN JniMtcServiceThread* pServiceThread)
{
    if (IsUssi(piSession))
    {
        // TODO: eCallType = IuMtcService::CALLTYPE_USSI;
    }

    CallInfo objCallInfo;
    m_objCallManager.CreateCall(pService->GetServiceType(), objCallInfo)
            ->HandleIncoming(piSession, pServiceThread);
}

PUBLIC
void MtcCallController::Start(IN CallKey nCallKey, IN CallType eCallType,
        IN const AString& strTarget, IN MediaInfo* pMediaInfo,
        IN const IMSMap<SuppType, SuppService*>& objSuppServices, IN IDialogEvent* /* pDialog */)
{
    m_objCallManager.GetCallByCallKey(nCallKey)
            ->Start(eCallType, strTarget, pMediaInfo, objSuppServices);
}

PUBLIC
void MtcCallController::HandleUserAlert(IN CallKey nCallKey)
{
    m_objCallManager.GetCallByCallKey(nCallKey)
            ->HandleUserAlert();
}

PUBLIC
void MtcCallController::Accept(IN CallKey nCallKey, IN CallType eCallType,
        IN MediaInfo* pMediaInfo)
{
    m_objCallManager.GetCallByCallKey(nCallKey)
            ->Accept(eCallType, pMediaInfo);
}

PUBLIC
void MtcCallController::Reject(IN CallKey nCallKey, IN const FailReason& objReason)
{
    m_objCallManager.GetCallByCallKey(nCallKey)
            ->Reject(objReason);
}

PUBLIC
void MtcCallController::Hold(IN CallKey nCallKey, IN MediaInfo* pMediaInfo)
{
    m_objCallManager.GetCallByCallKey(nCallKey)
            ->Hold(pMediaInfo);
}

PUBLIC
void MtcCallController::Resume(IN CallKey nCallKey, IN MediaInfo* pMediaInfo)
{
    m_objCallManager.GetCallByCallKey(nCallKey)
            ->Resume(pMediaInfo);
}

PUBLIC
void MtcCallController::AcceptResume(IN CallKey nCallKey, IN CallType eCallType,
        IN MediaInfo* pMediaInfo)
{
    m_objCallManager.GetCallByCallKey(nCallKey)
            ->AcceptResume(eCallType, pMediaInfo);
}

PUBLIC
void MtcCallController::RejectResume(IN CallKey nCallKey, IN const FailReason& objReason)
{
    m_objCallManager.GetCallByCallKey(nCallKey)
            ->RejectResume(objReason);
}

PUBLIC
void MtcCallController::Terminate(IN CallKey nCallKey, IN const FailReason& objReason)
{
    m_objCallManager.GetCallByCallKey(nCallKey)
            ->Terminate(objReason);
}

PUBLIC
void MtcCallController::Update(IN CallKey nCallKey, IN CallType eCallType,
        IN MediaInfo* pMediaInfo)
{
    m_objCallManager.GetCallByCallKey(nCallKey)
            ->Convert(eCallType, pMediaInfo);
}

PUBLIC
void MtcCallController::CancelUpdate(IN CallKey nCallKey, IN const FailReason& objReason)
{
    m_objCallManager.GetCallByCallKey(nCallKey)
            ->CancelConvert(objReason);
}

PUBLIC
void MtcCallController::AcceptUpdate(IN CallKey nCallKey, IN CallType eCallType,
        IN MediaInfo* pMediaInfo)
{
    m_objCallManager.GetCallByCallKey(nCallKey)
            ->AcceptConvert(eCallType, pMediaInfo);
}

PUBLIC
void MtcCallController::RejectUpdate(IN CallKey nCallKey, IN const FailReason& objReason)
{
    m_objCallManager.GetCallByCallKey(nCallKey)
            ->RejectConvert(objReason);
}

PUBLIC
void MtcCallController::SendTransaction(IN CallKey nCallKey, IN const AString& strUssi)
{
    m_objCallManager.GetCallByCallKey(nCallKey)
            ->SendUssi(strUssi);
}

/*
PUBLIC
void MtcCallController::HandleConference(IN CallKey nCallKey, IN IMS_UINT32 nCmd,
        IN IMSList<ConfUser*>& objUsers)
{
    ConferenceCallProxy::GetInstance()->ProcessCmd(
            m_objContext.GetSlotId(),
            m_objCallManager.GetCallByCallKey(nCallKey),
            objMsg);
}
*/

PUBLIC
void MtcCallController::MergeToConference(IN CallKey nCallKey, IN IMSList<ConfUser*>& objUsers)
{
    IConferenceController* pController =
            m_objContext.GetConferenceManager().GetController(nCallKey);
    if (pController == IMS_NULL)
    {
        pController = &m_objContext.GetConferenceManager().CreateController(
                nCallKey, ConferenceType::MERGE_CALL);
    }
    pController->ProcessCommand(IConferenceController::MERGE, objUsers);
}

PUBLIC
void MtcCallController::AddToConference(IN CallKey nCallKey, IN IMSList<ConfUser*>& objUsers)
{
    IConferenceController* pController =
            m_objContext.GetConferenceManager().GetController(nCallKey);
    if (pController == IMS_NULL)
    {
        return;
    }
    pController->ProcessCommand(IConferenceController::ADD, objUsers);
}

PUBLIC
void MtcCallController::RemoveFromConference(IN CallKey nCallKey, IN IMSList<ConfUser*>& objUsers)
{
    IConferenceController* pController =
            m_objContext.GetConferenceManager().GetController(nCallKey);
    if (pController == IMS_NULL)
    {
        return;
    }
    pController->ProcessCommand(IConferenceController::REMOVE, objUsers);
}

PUBLIC
void MtcCallController::Transfer(IN CallKey nCallKey, IN const AString& strTarget)
{
    m_objContext.GetEctManager()->Transfer(nCallKey, strTarget);
}

PRIVATE
IMS_BOOL MtcCallController::IsUssi(IN ISession* piSession)
{
    IMessage* piMessage = piSession->GetPreviousRequest(IMessage::SESSION_START);
    if (piMessage == IMS_NULL)
    {
        return IMS_FALSE;
    }

    return MessageUtil::ContainsValue(
            piMessage, USSDConstants::HEADER_USSD_PACKAGE, ISIPHeader::UNKNOWN,
            USSDConstants::HEADER_RECVINFO);
}
