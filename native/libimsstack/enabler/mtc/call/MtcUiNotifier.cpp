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

#include "CallReasonInfo.h"
#include "IJniMtcCallThread.h"
#include "IJniMtcServiceThread.h"
#include "IuMtcCall.h"
#include "IuMtcService.h"
#include "JniEnablerConnector.h"
#include "ServiceTrace.h"
#include "call/IMtcCallContext.h"
#include "call/MtcUiNotifier.h"
#include "call/ParticipantInfo.h"

__IMS_TRACE_TAG_COM_MTC__;

PUBLIC
MtcUiNotifier::MtcUiNotifier(IN IMtcCallContext& objContext) :
        m_objContext(objContext)
{
}

PUBLIC
MtcUiNotifier::~MtcUiNotifier() {}

PUBLIC
void MtcUiNotifier::SendPreIncomingCallReceived(IN CallKey nKey)
{
    IJniEnabler* piJniEnabler = JniEnablerConnector::GetInstance().GetJniEnabler(
            m_objContext.GetSlotId(), EnablerType::MTC_SERVICE);
    if (piJniEnabler == IMS_NULL)
    {
        IMS_TRACE_E(0, "JniMtcService is null", 0, 0, 0);
        return;
    }

    IJniMtcServiceThread* piServiceThread =
            reinterpret_cast<IJniMtcServiceThread*>(piJniEnabler->GetJniThread());

    if (piServiceThread == IMS_NULL)
    {
        IMS_TRACE_E(0, "JniMtcServiceThread is null", 0, 0, 0);
        return;
    }

    piServiceThread->OnPreIncomingCallReceived(nKey);
}

PUBLIC
void MtcUiNotifier::SendIncomingCallReceived(IN CallKey nKey, IN CallInfo& objCallInfo,
        IN const MediaInfo& objMediaInfo, IN const ImsMap<SuppType, SuppService*>& objSuppServices,
        IN ParticipantInfo& objParticipantInfo)
{
    IJniMtcCallThread* piThread = GetCallThread();
    if (piThread == IMS_NULL)
    {
        IMS_TRACE_E(0, "SendIncomingCallReceived : Not available", 0, 0, 0);
        return;
    }

    JniCallInfo objJniCallInfo = m_objContext.CreateJniCallInfo();
    objJniCallInfo.bUssi = objCallInfo.bUssi;

    piThread->OnIncomingCallReceived(nKey, objJniCallInfo, objMediaInfo, objSuppServices,
            objParticipantInfo.GetOipType(), objParticipantInfo.GetRemoteNumber());
}

PUBLIC
void MtcUiNotifier::SendStarted(IN CallInfo* /* pCallInfo */, IN const MediaInfo& objMediaInfo,
        IN const ImsMap<SuppType, SuppService*>& objSuppServices)
{
    IMS_TRACE_I("SendStarted", 0, 0, 0);

    IJniMtcCallThread* piThread = GetCallThread();
    if (piThread == IMS_NULL)
    {
        return;
    }

    piThread->OnStarted(m_objContext.CreateJniCallInfo(), objMediaInfo, objSuppServices);
}

PUBLIC
void MtcUiNotifier::SendStartFailed(IN const CallReasonInfo& objReason)
{
    IMS_TRACE_I("SendStartFailed : %s", _TRACE_CR_(objReason), 0, 0);

    IJniMtcCallThread* piThread = GetCallThread();
    if (piThread == IMS_NULL)
    {
        return;
    }

    piThread->OnStartFailed(objReason);
}

PUBLIC
void MtcUiNotifier::SendProgressing(IN CallInfo* /* pCallInfo */, IN const MediaInfo& objMediaInfo,
        IN const ImsMap<SuppType, SuppService*>& objSuppServices,
        IN IMS_BOOL bAlerted /*= IMS_FALSE */)
{
    IMS_TRACE_I("SendProgressing", 0, 0, 0);

    IJniMtcCallThread* piThread = GetCallThread();
    if (piThread == IMS_NULL)
    {
        return;
    }

    piThread->OnProgressing(
            m_objContext.CreateJniCallInfo(), objMediaInfo, objSuppServices, bAlerted);
}

PUBLIC
void MtcUiNotifier::SendHeld(IN CallInfo* /* pCallInfo */, IN const MediaInfo& objMediaInfo,
        IN const ImsMap<SuppType, SuppService*>& objSuppServices)
{
    IMS_TRACE_I("SendHeld", 0, 0, 0);

    IJniMtcCallThread* piThread = GetCallThread();
    if (piThread == IMS_NULL)
    {
        return;
    }

    piThread->OnHeld(m_objContext.CreateJniCallInfo(), objMediaInfo, objSuppServices);
}

PUBLIC
void MtcUiNotifier::SendHoldFailed(IN const CallReasonInfo& objReason)
{
    IMS_TRACE_I("SendHoldFailed : %s", _TRACE_CR_(objReason), 0, 0);

    IJniMtcCallThread* piThread = GetCallThread();
    if (piThread == IMS_NULL)
    {
        return;
    }

    piThread->OnHoldFailed(objReason);
}

PUBLIC
void MtcUiNotifier::SendResumed(IN CallInfo* /* pCallInfo */, IN const MediaInfo& objMediaInfo,
        IN const ImsMap<SuppType, SuppService*>& objSuppServices)
{
    IMS_TRACE_I("SendResumed", 0, 0, 0);

    IJniMtcCallThread* piThread = GetCallThread();
    if (piThread == IMS_NULL)
    {
        return;
    }

    piThread->OnResumed(m_objContext.CreateJniCallInfo(), objMediaInfo, objSuppServices);
}

PUBLIC
void MtcUiNotifier::SendResumeFailed(IN const CallReasonInfo& objReason)
{
    IMS_TRACE_I("SendResumeFailed : %s", _TRACE_CR_(objReason), 0, 0);

    IJniMtcCallThread* piThread = GetCallThread();
    if (piThread == IMS_NULL)
    {
        return;
    }

    piThread->OnResumeFailed(objReason);
}

PUBLIC
void MtcUiNotifier::SendHeldBy(IN CallInfo* /* pCallInfo */, IN const MediaInfo& objMediaInfo,
        IN const ImsMap<SuppType, SuppService*>& objSuppServices)
{
    IMS_TRACE_I("SendHeldBy", 0, 0, 0);

    IJniMtcCallThread* piThread = GetCallThread();
    if (piThread == IMS_NULL)
    {
        return;
    }

    piThread->OnHeldBy(m_objContext.CreateJniCallInfo(), objMediaInfo, objSuppServices);
}

PUBLIC
void MtcUiNotifier::SendResumedBy(IN CallInfo* /* pCallInfo */, IN const MediaInfo& objMediaInfo,
        IN const ImsMap<SuppType, SuppService*>& objSuppServices)
{
    IMS_TRACE_I("SendResumedBy", 0, 0, 0);

    IJniMtcCallThread* piThread = GetCallThread();
    if (piThread == IMS_NULL)
    {
        return;
    }

    piThread->OnResumedBy(m_objContext.CreateJniCallInfo(), objMediaInfo, objSuppServices);
}

PUBLIC
void MtcUiNotifier::SendTerminated(IN const CallReasonInfo& objReason)
{
    IMS_TRACE_I("SendTerminated : %s", _TRACE_CR_(objReason), 0, 0);

    IJniMtcCallThread* piThread = GetCallThread();
    if (piThread == IMS_NULL)
    {
        return;
    }

    piThread->OnTerminated(objReason);
}

PUBLIC
void MtcUiNotifier::SendIncomingResume(IN CallInfo* /* pCallInfo */,
        IN const MediaInfo& objMediaInfo, IN const ImsMap<SuppType, SuppService*>& objSuppServices)
{
    IMS_TRACE_I("SendIncomingResume", 0, 0, 0);

    IJniMtcCallThread* piThread = GetCallThread();
    if (piThread == IMS_NULL)
    {
        return;
    }

    piThread->OnIncomingResume(m_objContext.CreateJniCallInfo(), objMediaInfo, objSuppServices);
}

PUBLIC
void MtcUiNotifier::SendIncomingUpdate(IN CallType eCallTypeToUpdate, IN CallInfo* /* pCallInfo */,
        IN const MediaInfo& objMediaInfo, IN const ImsMap<SuppType, SuppService*>& objSuppServices)
{
    IMS_TRACE_I("SendIncomingUpdate", 0, 0, 0);

    IJniMtcCallThread* piThread = GetCallThread();
    if (piThread == IMS_NULL)
    {
        return;
    }

    JniCallInfo objJniCallInfo = m_objContext.CreateJniCallInfo();
    objJniCallInfo.eCallType = eCallTypeToUpdate;

    piThread->OnIncomingUpdate(objJniCallInfo, objMediaInfo, objSuppServices);
}

PUBLIC
void MtcUiNotifier::SendUpdated(IN CallInfo* /* pCallInfo */, IN const MediaInfo& objMediaInfo,
        IN const ImsMap<SuppType, SuppService*>& objSuppServices)
{
    IMS_TRACE_I("SendUpdated", 0, 0, 0);

    IJniMtcCallThread* piThread = GetCallThread();
    if (piThread == IMS_NULL)
    {
        return;
    }

    piThread->OnUpdated(m_objContext.CreateJniCallInfo(), objMediaInfo, objSuppServices);
}

PUBLIC
void MtcUiNotifier::SendUpdateFailed(IN const CallReasonInfo& objReason)
{
    IMS_TRACE_I("SendUpdateFailed : %s", _TRACE_CR_(objReason), 0, 0);

    IJniMtcCallThread* piThread = GetCallThread();
    if (piThread == IMS_NULL)
    {
        return;
    }

    piThread->OnUpdateFailed(objReason);
}

PUBLIC
void MtcUiNotifier::SendUpdatedBy(IN CallInfo* /* pCallInfo */, IN const MediaInfo& objMediaInfo,
        IN const ImsMap<SuppType, SuppService*>& objSuppServices)
{
    IMS_TRACE_I("SendUpdatedBy", 0, 0, 0);

    IJniMtcCallThread* piThread = GetCallThread();
    if (piThread == IMS_NULL)
    {
        return;
    }

    piThread->OnUpdatedBy(m_objContext.CreateJniCallInfo(), objMediaInfo, objSuppServices);
}

PUBLIC
void MtcUiNotifier::SendNotifyInfo(IN IMS_UINT32 eType,
        IN AString strValue /*= AString::ConstNull() */, IN IMS_SINT32 nValue /*= -1 */,
        IN IMS_BOOL bValue /*= IMS_FALSE */)
{
    IMS_TRACE_I("SendNotifyInfo : Type[%d]", eType, 0, 0);
    IMS_TRACE_D("SendNotifyInfo : [%s][%d][%s]", strValue.GetStr(), nValue, _TRACE_B_(bValue));

    IJniMtcCallThread* piThread = GetCallThread();
    if (piThread == IMS_NULL)
    {
        return;
    }

    piThread->OnInformationNotificationReceived(eType, strValue, nValue, bValue);
}

PUBLIC
void MtcUiNotifier::SendExpanded(IN CallInfo* /*pCallInfo*/, IN const MediaInfo& /*objMediaInfo*/,
        IN const ImsMap<SuppType, SuppService*>& /*objSuppServices*/)
{
    IMS_TRACE_I("SendExpanded", 0, 0, 0);

    IJniMtcCallThread* piThread = GetCallThread();
    if (piThread == IMS_NULL)
    {
        return;
    }
}

PUBLIC
void MtcUiNotifier::SendExpandFailed(IN const CallReasonInfo& objReason)
{
    IMS_TRACE_I("SendExpandFailed : %s", _TRACE_CR_(objReason), 0, 0);

    IJniMtcCallThread* piThread = GetCallThread();
    if (piThread == IMS_NULL)
    {
        return;
    }
}

PUBLIC
void MtcUiNotifier::SendExpandedBy(IN CallInfo* /*pCallInfo*/, IN const MediaInfo& /*objMediaInfo*/,
        IN const ImsMap<SuppType, SuppService*>& /*objSuppServices*/,
        IN IMS_SINTP /*nReplaceKey = 0 */)
{
    IMS_TRACE_I("SendExpandedBy", 0, 0, 0);

    IJniMtcCallThread* piThread = GetCallThread();
    if (piThread == IMS_NULL)
    {
        return;
    }
}

PUBLIC
void MtcUiNotifier::SendMerged(IN CallInfo* /* pCallInfo */, IN const MediaInfo& objMediaInfo,
        IN const ImsMap<SuppType, SuppService*>& objSuppServices,
        IN ImsList<ConfUser*>& lstConfUser)
{
    IMS_TRACE_I("SendMerged", 0, 0, 0);

    IJniMtcCallThread* piThread = GetCallThread();
    if (piThread == IMS_NULL)
    {
        return;
    }

    piThread->OnMerged(
            m_objContext.CreateJniCallInfo(), objMediaInfo, objSuppServices, lstConfUser);
}

PUBLIC
void MtcUiNotifier::SendMergeFailed(IN const CallReasonInfo& objReason)
{
    IMS_TRACE_I("SendMergeFailed : %s", _TRACE_CR_(objReason), 0, 0);

    IJniMtcCallThread* piThread = GetCallThread();
    if (piThread == IMS_NULL)
    {
        return;
    }

    piThread->OnMergeFailed(objReason);
}

PUBLIC
void MtcUiNotifier::SendJoined(IN IMS_BOOL bResult, IN const CallReasonInfo& objReason)
{
    IMS_TRACE_I("SendJoined : Result[%s] %s", _TRACE_B_(bResult), _TRACE_CR_(objReason), 0);

    IJniMtcCallThread* piThread = GetCallThread();
    if (piThread == IMS_NULL)
    {
        return;
    }

    if (bResult)
    {
        piThread->OnConferenceParticipantAdded();
    }
    else
    {
        piThread->OnConferenceParticipantAddFailed(objReason);
    }
}

PUBLIC
void MtcUiNotifier::SendDropped(IN IMS_BOOL bResult, IN const CallReasonInfo& objReason)
{
    IMS_TRACE_I("SendDropped : Result[%s] %s", _TRACE_B_(bResult), _TRACE_CR_(objReason), 0);

    IJniMtcCallThread* piThread = GetCallThread();
    if (piThread == IMS_NULL)
    {
        return;
    }

    if (bResult)
    {
        piThread->OnConferenceParticipantRemoved();
    }
    else
    {
        piThread->OnConferenceParticipantRemoveFailed(objReason);
    }
}

PUBLIC
void MtcUiNotifier::SendNotifyUsersInfo(IN ImsList<ConfUser*>& lstConfUser)
{
    IMS_TRACE_I("SendNotifyUsersInfo : Size[%d]", lstConfUser.GetSize(), 0, 0);

    IJniMtcCallThread* piThread = GetCallThread();
    if (piThread == IMS_NULL)
    {
        return;
    }

    piThread->OnConferenceParticipantsInfoChanged(lstConfUser);
}

PUBLIC
void MtcUiNotifier::SendNotifyConfInfo(IN AString strDisplayText, IN AString strSubject,
        IN IMS_SINT32 nMaxUserCount, IN IMS_UINT32 nUserCount, IN AString strHostEntity)
{
    IMS_TRACE_D("SendNotifyConfInfo : [%s][%s]", strDisplayText.GetStr(), strSubject.GetStr(), 0);
    IMS_TRACE_D(
            "SendNotifyConfInfo : [%d][%d][%s]", nMaxUserCount, nUserCount, strHostEntity.GetStr());

    IJniMtcCallThread* piThread = GetCallThread();
    if (piThread == IMS_NULL)
    {
        return;
    }

    piThread->OnConferenceInfoChanged(
            strDisplayText, strSubject, nUserCount, nMaxUserCount, strHostEntity);
}

PUBLIC
void MtcUiNotifier::SendReplacedBy(IN CallInfo* /*pCallInfo*/, IN const MediaInfo& /*objMediaInfo*/,
        IN const ImsMap<SuppType, SuppService*>& /*objSuppServices*/, IN IMS_SINTP nKey,
        IN IMS_UINTP /*nType*/)
{
    IMS_TRACE_I("SendReplacedBy : Key[%" PFLS_u "]", nKey, 0, 0);

    IJniMtcCallThread* piThread = GetCallThread();
    if (piThread == IMS_NULL)
    {
        return;
    }
}

PUBLIC
void MtcUiNotifier::SendEctCompleted(IN IMS_RESULT nResult, IN const CallReasonInfo& objReason)
{
    IMS_TRACE_I("SendECTCompleted : Result[%d]", nResult, 0, 0);

    IJniMtcCallThread* piThread = GetCallThread();
    if (piThread == IMS_NULL)
    {
        return;
    }
    piThread->OnEctCompleted(nResult, objReason);
}

PUBLIC
void MtcUiNotifier::SendCallPushCompleted(IN IMS_BOOL bResult, IN const CallReasonInfo& objReason)
{
    IMS_TRACE_I(
            "SendCallPushCompleted : Result[%s] %s", _TRACE_B_(bResult), _TRACE_CR_(objReason), 0);

    IJniMtcCallThread* piThread = GetCallThread();
    if (piThread == IMS_NULL)
    {
        return;
    }
}

PRIVATE
IJniMtcCallThread* MtcUiNotifier::GetCallThread()
{
    IJniEnabler* piJniMtcCall = JniEnablerConnector::GetInstance().GetJniEnabler(
            m_objContext.GetSlotId(), EnablerType::MTC_CALL, m_objContext.GetCallKey());
    if (piJniMtcCall == IMS_NULL)
    {
        IMS_TRACE_D("GetCallThread no JniMtcCall", 0, 0, 0);
        return IMS_NULL;
    }

    return reinterpret_cast<IJniMtcCallThread*>(piJniMtcCall->GetJniThread());
}
