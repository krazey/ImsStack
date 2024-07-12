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

#include "AString.h"
#include "CallReasonInfo.h"
#include "IJniEnabler.h"
#include "IJniMtcCallThread.h"
#include "IJniMtcServiceThread.h"
#include "ImsList.h"
#include "ImsTypeDef.h"
#include "IuMtcCall.h"
#include "IuMtcService.h"
#include "JniEnablerConnector.h"
#include "MtcDef.h"
#include "ServiceTrace.h"
#include "call/IMtcCallContext.h"
#include "call/MtcUiNotifier.h"
#include "call/ParticipantInfo.h"
#include "call/UpdatingInfo.h"
#include "helper/MtcSupplementaryService.h"
#include "media/IMtcMediaManager.h"

__IMS_TRACE_TAG_COM_MTC__;

PUBLIC
MtcUiNotifier::MtcUiNotifier(IN IMtcCallContext& objContext) :
        m_objContext(objContext)
{
}

PUBLIC
MtcUiNotifier::~MtcUiNotifier() {}

PUBLIC
void MtcUiNotifier::SendPreIncomingCallReceived()
{
    IMS_TRACE_I("SendPreIncomingCallReceived", 0, 0, 0);

    IJniMtcServiceThread* piServiceThread = m_objContext.GetService().GetJniServiceThread();
    if (piServiceThread == IMS_NULL)
    {
        IMS_TRACE_E(0, "JniMtcServiceThread is null", 0, 0, 0);
        return;
    }

    piServiceThread->OnPreIncomingCallReceived(m_objContext.GetCallKey());
}

PUBLIC
void MtcUiNotifier::SendIncomingCallReceived()
{
    IMS_TRACE_I("SendIncomingCallReceived", 0, 0, 0);

    IJniMtcCallThread* piThread = GetCallThread();
    if (piThread == IMS_NULL)
    {
        return;
    }

    piThread->OnIncomingCallReceived(m_objContext.GetCallKey(), m_objContext.CreateJniCallInfo(),
            m_objContext.GetMediaManager().GetMediaInfo(),
            m_objContext.GetSupplementaryService().GetServices(),
            m_objContext.GetParticipantInfo().GetOipType(),
            m_objContext.GetParticipantInfo().GetRemoteNumber());
}

PUBLIC
void MtcUiNotifier::SendIncomingCallRejected(IN const CallReasonInfo& objReason)
{
    IMS_TRACE_I("SendIncomingCallRejected", 0, 0, 0);

    IJniMtcServiceThread* piServiceThread = m_objContext.GetService().GetJniServiceThread();
    if (piServiceThread == IMS_NULL)
    {
        IMS_TRACE_E(0, "JniMtcServiceThread is null", 0, 0, 0);
        return;
    }

    piServiceThread->OnRejectedIncomingCall(m_objContext.CreateJniCallInfo(),
            m_objContext.GetMediaManager().GetMediaInfo(),
            m_objContext.GetSupplementaryService().GetServices(),
            m_objContext.GetParticipantInfo().GetOipType(),
            m_objContext.GetParticipantInfo().GetRemoteNumber(), objReason);
}

PUBLIC
void MtcUiNotifier::SendStarted()
{
    IMS_TRACE_I("SendStarted", 0, 0, 0);

    IJniMtcCallThread* piThread = GetCallThread();
    if (piThread == IMS_NULL)
    {
        return;
    }

    piThread->OnStarted(m_objContext.CreateJniCallInfo(),
            m_objContext.GetMediaManager().GetMediaInfo(),
            m_objContext.GetSupplementaryService().GetServices());
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
void MtcUiNotifier::SendProgressing()
{
    IMS_TRACE_I("SendProgressing", 0, 0, 0);

    IJniMtcCallThread* piThread = GetCallThread();
    if (piThread == IMS_NULL)
    {
        return;
    }

    MediaInfo objRefinedMediaInfo = m_objContext.GetMediaManager().GetMediaInfo();
    if (m_objContext.GetMediaManager().IsLocalTone())
    {
        objRefinedMediaInfo.eAudioDirection = DIRECTION_INACTIVE;
    }

    piThread->OnProgressing(m_objContext.CreateJniCallInfo(), objRefinedMediaInfo,
            m_objContext.GetSupplementaryService().GetServices());
}

PUBLIC
void MtcUiNotifier::SendHeld()
{
    IMS_TRACE_I("SendHeld", 0, 0, 0);

    IJniMtcCallThread* piThread = GetCallThread();
    if (piThread == IMS_NULL)
    {
        return;
    }

    piThread->OnHeld(m_objContext.CreateJniCallInfo(),
            m_objContext.GetMediaManager().GetMediaInfo(),
            m_objContext.GetSupplementaryService().GetServices());
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
void MtcUiNotifier::SendResumed()
{
    IMS_TRACE_I("SendResumed", 0, 0, 0);

    IJniMtcCallThread* piThread = GetCallThread();
    if (piThread == IMS_NULL)
    {
        return;
    }

    piThread->OnResumed(m_objContext.CreateJniCallInfo(),
            m_objContext.GetMediaManager().GetMediaInfo(),
            m_objContext.GetSupplementaryService().GetServices());
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
void MtcUiNotifier::SendHeldBy()
{
    IMS_TRACE_I("SendHeldBy", 0, 0, 0);

    IJniMtcCallThread* piThread = GetCallThread();
    if (piThread == IMS_NULL)
    {
        return;
    }

    piThread->OnHeldBy(m_objContext.CreateJniCallInfo(),
            m_objContext.GetMediaManager().GetMediaInfo(),
            m_objContext.GetSupplementaryService().GetServices());
}

PUBLIC
void MtcUiNotifier::SendResumedBy()
{
    IMS_TRACE_I("SendResumedBy", 0, 0, 0);

    IJniMtcCallThread* piThread = GetCallThread();
    if (piThread == IMS_NULL)
    {
        return;
    }

    piThread->OnResumedBy(m_objContext.CreateJniCallInfo(),
            m_objContext.GetUpdatingInfo().GetModifiedInfo(),
            m_objContext.GetSupplementaryService().GetServices());
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
void MtcUiNotifier::SendIncomingResume()
{
    IMS_TRACE_I("SendIncomingResume", 0, 0, 0);

    IJniMtcCallThread* piThread = GetCallThread();
    if (piThread == IMS_NULL)
    {
        return;
    }

    piThread->OnIncomingResume(m_objContext.CreateJniCallInfo(),
            m_objContext.GetMediaManager().GetMediaInfo(),
            m_objContext.GetSupplementaryService().GetServices());
}

PUBLIC
void MtcUiNotifier::SendIncomingUpdate(IN CallType eCallTypeToUpdate)
{
    IMS_TRACE_I("SendIncomingUpdate : %d", eCallTypeToUpdate, 0, 0);

    IJniMtcCallThread* piThread = GetCallThread();
    if (piThread == IMS_NULL)
    {
        return;
    }

    JniCallInfo objJniCallInfo = m_objContext.CreateJniCallInfo();
    objJniCallInfo.eCallType = eCallTypeToUpdate;

    piThread->OnIncomingUpdate(objJniCallInfo,
            m_objContext.GetUpdatingInfo().GetTargetCallType() == CallType::UNKNOWN
                    ? m_objContext.GetUpdatingInfo().GetModifiedInfo()
                    : m_objContext.GetUpdatingInfo().GetAlertingInfo(),
            m_objContext.GetSupplementaryService().GetServices());
}

PUBLIC
void MtcUiNotifier::SendUpdated()
{
    IMS_TRACE_I("SendUpdated", 0, 0, 0);

    IJniMtcCallThread* piThread = GetCallThread();
    if (piThread == IMS_NULL)
    {
        return;
    }

    piThread->OnUpdated(m_objContext.CreateJniCallInfo(),
            m_objContext.GetUpdatingInfo().GetModifiedMediaInfoWithOriginalAudioDir(),
            m_objContext.GetSupplementaryService().GetServices());
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
void MtcUiNotifier::SendUpdatedBy()
{
    IMS_TRACE_I("SendUpdatedBy", 0, 0, 0);

    IJniMtcCallThread* piThread = GetCallThread();
    if (piThread == IMS_NULL)
    {
        return;
    }

    piThread->OnUpdatedBy(m_objContext.CreateJniCallInfo(),
            m_objContext.GetMediaManager().GetMediaInfo(),
            m_objContext.GetSupplementaryService().GetServices());
}

PUBLIC
void MtcUiNotifier::SendNotifyInfo(
        IN IMS_UINT32 eType, IN const AString& strValue, IN IMS_SINT32 nValue, IN IMS_BOOL bValue)
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
void MtcUiNotifier::SendReplacedBy(IN IMS_SINTP /* nKey */, IN IMS_UINTP /* nType */)
{
    IMS_TRACE_I("SendReplacedBy", 0, 0, 0);

    IJniMtcCallThread* piThread = GetCallThread();
    if (piThread == IMS_NULL)
    {
        return;
    }
}

PUBLIC
void MtcUiNotifier::SendEctCompleted(IN IMS_RESULT nResult, IN const CallReasonInfo& objReason)
{
    IMS_TRACE_I("SendEctCompleted : Result[%d]", nResult, 0, 0);

    IJniMtcCallThread* piThread = GetCallThread();
    if (piThread == IMS_NULL)
    {
        return;
    }

    piThread->OnEctCompleted(nResult, objReason);
}

PUBLIC
void MtcUiNotifier::SendCallPushCompleted(
        IN IMS_RESULT /* nResult */, IN const CallReasonInfo& /* objReason */)
{
    IMS_TRACE_I("SendCallPushCompleted", 0, 0, 0);

    IJniMtcCallThread* piThread = GetCallThread();
    if (piThread == IMS_NULL)
    {
        return;
    }
}

PRIVATE
IJniMtcCallThread* MtcUiNotifier::GetCallThread() const
{
    IJniEnabler* piJniMtcCall = JniEnablerConnector::GetInstance().GetJniEnabler(
            m_objContext.GetSlotId(), EnablerType::MTC_CALL, m_objContext.GetCallKey());
    if (piJniMtcCall == IMS_NULL)
    {
        IMS_TRACE_D("GetCallThread : No JniMtcCall", 0, 0, 0);
        return IMS_NULL;
    }

    return reinterpret_cast<IJniMtcCallThread*>(piJniMtcCall->GetJniThread());
}
