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

#include "IuMtcCall.h"
#include "IuMtcService.h"
#include "JniMtcCallThread.h"
#include "JniMtcServiceThread.h"
#include "call/IMtcCallContext.h"
#include "call/MtcUiNotifier.h"
#include "ServiceTrace.h"

__IMS_TRACE_TAG_COM_MTC__;

PUBLIC
MtcUiNotifier::MtcUiNotifier(IN IMtcCallContext& objContext) :
        m_objContext(objContext),
        m_pCallThread(IMS_NULL),
        m_pServiceThread(IMS_NULL)
{
}

PUBLIC
MtcUiNotifier::~MtcUiNotifier() {}

PUBLIC
void MtcUiNotifier::SendPreIncomingCallReceived(IN CallKey nKey)
{
    if (!m_pServiceThread)
    {
        IMS_TRACE_E(0, "SendPreIncomingCallReceived : Not available", 0, 0, 0);
        return;
    }

    m_pServiceThread->OnPreIncomingCallReceived(nKey);
}

PUBLIC
void MtcUiNotifier::SendIncomingCallReceived(IN CallKey nKey, IN CallInfo& objCallInfo,
        IN MediaInfo& objMediaInfo, IN const IMSMap<SuppType, SuppService*>& objSuppServices,
        IN ParticipantInfo& objParticipantInfo)
{
    if (!IsAvailableToSend())
    {
        IMS_TRACE_E(0, "SendIncomingCallReceived : Not available", 0, 0, 0);
        return;
    }

    JniCallInfo objJniCallInfo = m_objContext.CreateJniCallInfo();
    objJniCallInfo.bUssi = objCallInfo.bUssi;

    m_pCallThread->OnIncomingCallReceived(
            nKey, objJniCallInfo, &objMediaInfo, objSuppServices, &objParticipantInfo);
}

PUBLIC
void MtcUiNotifier::SendStarted(IN CallInfo* /* pCallInfo */, IN MediaInfo* pMediaInfo,
        IN const IMSMap<SuppType, SuppService*>& objSuppServices)
{
    IMS_TRACE_I("SendStarted", 0, 0, 0);

    if (!IsAvailableToSend())
    {
        return;
    }

    m_pCallThread->OnStarted(m_objContext.CreateJniCallInfo(), pMediaInfo, objSuppServices);
}

PUBLIC
void MtcUiNotifier::SendStartFailed(IN const CallReasonInfo& objReason)
{
    IMS_TRACE_I("SendStartFailed : %s", PS_FR(objReason), 0, 0);

    if (!IsAvailableToSend())
    {
        return;
    }

    m_pCallThread->OnStartFailed(objReason);
}

PUBLIC
void MtcUiNotifier::SendProgressing(IN CallInfo* /* pCallInfo */, IN MediaInfo* pMediaInfo,
        IN const IMSMap<SuppType, SuppService*>& objSuppServices,
        IN IMS_BOOL bAlerted /*= IMS_FALSE */)
{
    IMS_TRACE_I("SendProgressing", 0, 0, 0);

    if (!IsAvailableToSend())
    {
        return;
    }

    m_pCallThread->OnProgressing(
            m_objContext.CreateJniCallInfo(), pMediaInfo, objSuppServices, bAlerted);
}

PUBLIC
void MtcUiNotifier::SendHeld(IN CallInfo* /* pCallInfo */, IN MediaInfo* pMediaInfo,
        IN const IMSMap<SuppType, SuppService*>& objSuppServices)
{
    IMS_TRACE_I("SendHeld", 0, 0, 0);

    if (!IsAvailableToSend())
    {
        return;
    }

    m_pCallThread->OnHeld(m_objContext.CreateJniCallInfo(), pMediaInfo, objSuppServices);
}

PUBLIC
void MtcUiNotifier::SendHoldFailed(IN const CallReasonInfo& objReason)
{
    IMS_TRACE_I("SendHoldFailed : %s", PS_FR(objReason), 0, 0);

    if (!IsAvailableToSend())
    {
        return;
    }

    m_pCallThread->OnHoldFailed(objReason);
}

PUBLIC
void MtcUiNotifier::SendResumed(IN CallInfo* /* pCallInfo */, IN MediaInfo* pMediaInfo,
        IN const IMSMap<SuppType, SuppService*>& objSuppServices)
{
    IMS_TRACE_I("SendResumed", 0, 0, 0);

    if (!IsAvailableToSend())
    {
        return;
    }

    m_pCallThread->OnResumed(m_objContext.CreateJniCallInfo(), pMediaInfo, objSuppServices);
}

PUBLIC
void MtcUiNotifier::SendResumeFailed(IN const CallReasonInfo& objReason)
{
    IMS_TRACE_I("SendResumeFailed : %s", PS_FR(objReason), 0, 0);

    if (!IsAvailableToSend())
    {
        return;
    }

    m_pCallThread->OnResumeFailed(objReason);
}

PUBLIC
void MtcUiNotifier::SendHeldBy(IN CallInfo* /* pCallInfo */, IN MediaInfo* pMediaInfo,
        IN const IMSMap<SuppType, SuppService*>& objSuppServices)
{
    IMS_TRACE_I("SendHeldBy", 0, 0, 0);

    if (!IsAvailableToSend())
    {
        return;
    }

    m_pCallThread->OnHeldBy(m_objContext.CreateJniCallInfo(), pMediaInfo, objSuppServices);
}

PUBLIC
void MtcUiNotifier::SendResumedBy(IN CallInfo* /* pCallInfo */, IN MediaInfo* pMediaInfo,
        IN const IMSMap<SuppType, SuppService*>& objSuppServices)
{
    IMS_TRACE_I("SendResumedBy", 0, 0, 0);

    if (!IsAvailableToSend())
    {
        return;
    }

    m_pCallThread->OnResumedBy(m_objContext.CreateJniCallInfo(), pMediaInfo, objSuppServices);
}

PUBLIC
void MtcUiNotifier::SendTerminated(IN const CallReasonInfo& objReason)
{
    IMS_TRACE_I("SendTerminated : %s", PS_FR(objReason), 0, 0);

    if (!IsAvailableToSend())
    {
        return;
    }

    m_pCallThread->OnTerminated(objReason);
}

PUBLIC
void MtcUiNotifier::SendIncomingResume(IN CallInfo* /* pCallInfo */, IN MediaInfo* pMediaInfo,
        IN const IMSMap<SuppType, SuppService*>& objSuppServices)
{
    IMS_TRACE_I("SendIncomingResume", 0, 0, 0);

    if (!IsAvailableToSend())
    {
        return;
    }

    m_pCallThread->OnIncomingResume(m_objContext.CreateJniCallInfo(), pMediaInfo, objSuppServices);
}

PUBLIC
void MtcUiNotifier::SendIncomingUpdate(IN CallType eCallTypeToUpdate, IN CallInfo* /* pCallInfo */,
        IN MediaInfo* pMediaInfo, IN const IMSMap<SuppType, SuppService*>& objSuppServices)
{
    IMS_TRACE_I("SendIncomingUpdate", 0, 0, 0);

    if (!IsAvailableToSend())
    {
        return;
    }

    JniCallInfo objJniCallInfo = m_objContext.CreateJniCallInfo();
    objJniCallInfo.eCallType = eCallTypeToUpdate;

    m_pCallThread->OnIncomingUpdate(objJniCallInfo, pMediaInfo, objSuppServices);
}

PUBLIC
void MtcUiNotifier::SendUpdated(IN CallInfo* /* pCallInfo */, IN MediaInfo* pMediaInfo,
        IN const IMSMap<SuppType, SuppService*>& objSuppServices)
{
    IMS_TRACE_I("SendUpdated", 0, 0, 0);

    if (!IsAvailableToSend())
    {
        return;
    }

    m_pCallThread->OnUpdated(m_objContext.CreateJniCallInfo(), pMediaInfo, objSuppServices);
}

PUBLIC
void MtcUiNotifier::SendUpdateFailed(IN const CallReasonInfo& objReason)
{
    IMS_TRACE_I("SendUpdateFailed : %s", PS_FR(objReason), 0, 0);

    if (!IsAvailableToSend())
    {
        return;
    }

    m_pCallThread->OnUpdateFailed(objReason);
}

PUBLIC
void MtcUiNotifier::SendUpdatedBy(IN CallInfo* /* pCallInfo */, IN MediaInfo* pMediaInfo,
        IN const IMSMap<SuppType, SuppService*>& objSuppServices)
{
    IMS_TRACE_I("SendUpdatedBy", 0, 0, 0);

    if (!IsAvailableToSend())
    {
        return;
    }

    m_pCallThread->OnUpdatedBy(m_objContext.CreateJniCallInfo(), pMediaInfo, objSuppServices);
}

PUBLIC
void MtcUiNotifier::SendNotifyInfo(IN IMS_UINT32 eType,
        IN AString strValue /*= AString::ConstNull() */, IN IMS_SINT32 nValue /*= -1 */,
        IN IMS_BOOL bValue /*= IMS_FALSE */)
{
    IMS_TRACE_I("SendNotifyInfo : Type[%d]", eType, 0, 0);
    IMS_TRACE_D("SendNotifyInfo : [%s][%d][%s]", strValue.GetStr(), nValue, PS_BOOL(bValue));

    if (!IsAvailableToSend())
    {
        return;
    }

    m_pCallThread->OnInformationNotificationReceived(eType, strValue, nValue, bValue);
}

PUBLIC
void MtcUiNotifier::SendExpanded(IN CallInfo* /*pCallInfo*/, IN MediaInfo* /*pMediaInfo*/,
        IN const IMSMap<SuppType, SuppService*>& /*objSuppServices*/)
{
    IMS_TRACE_I("SendExpanded", 0, 0, 0);

    if (!IsAvailableToSend())
    {
        return;
    }
}

PUBLIC
void MtcUiNotifier::SendExpandFailed(IN const CallReasonInfo& objReason)
{
    IMS_TRACE_I("SendExpandFailed : %s", PS_FR(objReason), 0, 0);

    if (!IsAvailableToSend())
    {
        return;
    }
}

PUBLIC
void MtcUiNotifier::SendExpandedBy(IN CallInfo* /*pCallInfo*/, IN MediaInfo* /*pMediaInfo*/,
        IN const IMSMap<SuppType, SuppService*>& /*objSuppServices*/,
        IN IMS_SINTP /*nReplaceKey = 0 */)
{
    IMS_TRACE_I("SendExpandedBy", 0, 0, 0);

    if (!IsAvailableToSend())
    {
        return;
    }
}

PUBLIC
void MtcUiNotifier::SendMerged(IN CallInfo* /* pCallInfo */, IN MediaInfo* pMediaInfo,
        IN const IMSMap<SuppType, SuppService*>& objSuppServices, IN IMSList<ConfUser*> lstConfUser)
{
    IMS_TRACE_I("SendMerged", 0, 0, 0);

    if (!IsAvailableToSend())
    {
        return;
    }

    m_pCallThread->OnMerged(
            m_objContext.CreateJniCallInfo(), pMediaInfo, objSuppServices, lstConfUser);
}

PUBLIC
void MtcUiNotifier::SendMergeFailed(IN const CallReasonInfo& objReason)
{
    IMS_TRACE_I("SendMergeFailed : %s", PS_FR(objReason), 0, 0);

    if (!IsAvailableToSend())
    {
        return;
    }

    m_pCallThread->OnMergeFailed(objReason);
}

PUBLIC
void MtcUiNotifier::SendJoined(IN IMS_BOOL bResult, IN const CallReasonInfo& objReason)
{
    IMS_TRACE_I("SendJoined : Result[%s] %s", PS_BOOL(bResult), PS_FR(objReason), 0);

    if (!IsAvailableToSend())
    {
        return;
    }

    if (bResult)
    {
        m_pCallThread->OnConferenceParticipantAdded();
    }
    else
    {
        m_pCallThread->OnConferenceParticipantAddFailed(objReason);
    }
}

PUBLIC
void MtcUiNotifier::SendDropped(IN IMS_BOOL bResult, IN const CallReasonInfo& objReason)
{
    IMS_TRACE_I("SendDropped : Result[%s] %s", PS_BOOL(bResult), PS_FR(objReason), 0);

    if (!IsAvailableToSend())
    {
        return;
    }

    if (bResult)
    {
        m_pCallThread->OnConferenceParticipantRemoved();
    }
    else
    {
        m_pCallThread->OnConferenceParticipantRemoveFailed(objReason);
    }
}

PUBLIC
void MtcUiNotifier::SendNotifyUsersInfo(IN IMSList<ConfUser*> lstConfUser)
{
    IMS_TRACE_I("SendNotifyUsersInfo : Size[%d]", lstConfUser.GetSize(), 0, 0);

    if (!IsAvailableToSend())
    {
        return;
    }

    m_pCallThread->OnConferenceParticipantsInfoChanged(lstConfUser);
}

PUBLIC
void MtcUiNotifier::SendNotifyConfInfo(IN AString strDisplayText, IN AString strSubject,
        IN IMS_SINT32 nMaxUserCount, IN IMS_UINT32 nUserCount, IN AString strHostEntity)
{
    IMS_TRACE_D("SendNotifyConfInfo : [%s][%s]", strDisplayText.GetStr(), strSubject.GetStr(), 0);
    IMS_TRACE_D(
            "SendNotifyConfInfo : [%d][%d][%s]", nMaxUserCount, nUserCount, strHostEntity.GetStr());

    if (!IsAvailableToSend())
    {
        return;
    }

    m_pCallThread->OnConferenceInfoChanged(
            strDisplayText, strSubject, nUserCount, nMaxUserCount, strHostEntity);
}

PUBLIC
void MtcUiNotifier::SendReplacedBy(IN CallInfo* /*pCallInfo*/, IN MediaInfo* /*pMediaInfo*/,
        IN const IMSMap<SuppType, SuppService*>& /*objSuppServices*/, IN IMS_SINTP nKey,
        IN IMS_UINTP /*nType*/)
{
    IMS_TRACE_I("SendReplacedBy : Key[%" PFLS_u "]", nKey, 0, 0);

    if (!IsAvailableToSend())
    {
        return;
    }
}

PUBLIC
void MtcUiNotifier::SendEctCompleted(IN IMS_RESULT nResult, IN const CallReasonInfo& objReason)
{
    IMS_TRACE_I("SendECTCompleted : Result[%d]", nResult, 0, 0);

    if (!IsAvailableToSend())
    {
        return;
    }
    m_pCallThread->OnEctCompleted(nResult, objReason);
}

PUBLIC
void MtcUiNotifier::SendCallPushCompleted(IN IMS_BOOL bResult, IN const CallReasonInfo& objReason)
{
    IMS_TRACE_I("SendCallPushCompleted : Result[%s] %s", PS_BOOL(bResult), PS_FR(objReason), 0);

    if (!IsAvailableToSend())
    {
        return;
    }
}

PRIVATE
IMS_BOOL MtcUiNotifier::IsAvailableToSend()
{
    if (m_pCallThread == IMS_NULL)
    {
        IMS_TRACE_E(0, "IsAvailableToSend : Not available", 0, 0, 0);
        return IMS_FALSE;
    }

    return IMS_TRUE;
}
