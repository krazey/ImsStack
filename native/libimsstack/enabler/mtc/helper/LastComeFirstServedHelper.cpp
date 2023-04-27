/*
 * Copyright (C) 2023 The Android Open Source Project
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
#include "IMtcContext.h"
#include "INetworkWatcher.h"
#include "ServiceTrace.h"
#include "call/IMtcCall.h"
#include "call/IMtcCallContext.h"
#include "call/IMtcSession.h"
#include "call/MtcCallManager.h"
#include "configuration/MtcConfigurationProxy.h"
#include "core/IMessage.h"
#include "core/ISession.h"
#include "helper/IPassiveTimerHolder.h"
#include "helper/LastComeFirstServedHelper.h"
#include "precondition/IMtcPreconditionManager.h"

__IMS_TRACE_TAG_COM_MTC__;

PUBLIC
LastComeFirstServedHelper::LastComeFirstServedHelper(IN IMtcContext& objContext) :
        m_objContext(objContext),
        m_bPreExistingIncomingCallOnNr(IMS_FALSE)
{
    IMS_TRACE_I("+LastComeFirstServedHelper", 0, 0, 0);
}

PUBLIC VIRTUAL LastComeFirstServedHelper::~LastComeFirstServedHelper()
{
    IMS_TRACE_I("~LastComeFirstServedHelper", 0, 0, 0);
}

PUBLIC GLOBAL IMS_BOOL LastComeFirstServedHelper::IsSupported(
        IN const MtcConfigurationProxy& objConfigurationProxy)
{
    return objConfigurationProxy.GetInt(Feature::PRE_ALERTING_TIMER) > 0;
}

PUBLIC VIRTUAL void LastComeFirstServedHelper::OnCallReceived(IN CallKey nIncomingCallKey)
{
    if (!IsNormalCall(nIncomingCallKey))
    {
        return;
    }

    IMS_TRACE_D("OnCallReceived", 0, 0, 0);
    IMtcCall* piPreExistingIncomingCall = GetPreExistingIncomingCall();
    if (piPreExistingIncomingCall != IMS_NULL)
    {
        if (!IsNormalCall(piPreExistingIncomingCall->GetKey()))
        {
            return;
        }

        if (!m_bPreExistingIncomingCallOnNr ||
                m_objContext.GetPassiveTimerHolder().IsActive(
                        IPassiveTimerHolder::Type::PRE_ALERTING_GUARD))
        {
            return;
        }

        IMS_TRACE_D("OnCallReceived Reject pre-existing incoming call", 0, 0, 0);
        piPreExistingIncomingCall->Reject(
                CallReasonInfo(GetRejectReasonCode(piPreExistingIncomingCall->GetCallContext())));
    }

    IMS_BOOL bOnNr = GetCallContext(nIncomingCallKey).GetService().IsNr();
    SetNetworkInfoOfIncomingCall(bOnNr);
    if (bOnNr)
    {
        StartPreAlertingGuardTimer();
    }
}

PRIVATE
IMS_BOOL LastComeFirstServedHelper::IsNormalCall(IN CallKey nKey) const
{
    CallInfo& objCallInfo = GetCallContext(nKey).GetCallInfo();
    return (!objCallInfo.bEmergency && !objCallInfo.bUssi);
}

PRIVATE
IMtcCallContext& LastComeFirstServedHelper::GetCallContext(IN CallKey nKey) const
{
    return m_objContext.GetCallManager().GetCallByCallKey(nKey)->GetCallContext();
}

PRIVATE
IMtcCall* LastComeFirstServedHelper::GetPreExistingIncomingCall() const
{
    ImsList<IMtcCall*> objCalls =
            m_objContext.GetCallManager().GetCallsByState(IMtcCall::State::INCOMING);
    // There can be only one call in Incoming State.
    return (objCalls.GetSize() > 0) ? objCalls.GetAt(0) : IMS_NULL;
}

PRIVATE
IMS_SINT32 LastComeFirstServedHelper::GetRejectReasonCode(IN IMtcCallContext& objCallContext) const
{
    ISession& objISession = objCallContext.GetSession()->GetISession();
    if (objISession.GetPreviousRequest(IMessage::SESSION_PRACK) != IMS_NULL ||
            objCallContext.GetPreconditionManager().IsDedicatedBearerAllocated(
                    &objISession, MEDIATYPE_AUDIO))
    {
        return CODE_REJECT_ONGOING_CALL_SETUP;
    }

    return CODE_REJECT_QOS_FAILURE;
}

PRIVATE
void LastComeFirstServedHelper::StartPreAlertingGuardTimer() const
{
    IMS_TRACE_D("StartPreAlertingGuardTimer", 0, 0, 0);
    IMS_SINT32 nPreAlertingTime =
            m_objContext.GetConfigurationProxy().GetInt(Feature::PRE_ALERTING_TIMER);
    m_objContext.GetPassiveTimerHolder().AddTimer(
            IPassiveTimerHolder::Type::PRE_ALERTING_GUARD, nPreAlertingTime, IMS_TRUE);
}
