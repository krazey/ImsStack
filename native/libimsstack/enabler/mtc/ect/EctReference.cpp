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

#include "ICoreService.h"
#include "IMessage.h"
#include "IMtcContext.h"
#include "IMtcService.h"
#include "IReference.h"
#include "ISession.h"
#include "ISipHeader.h"
#include "ISipMessage.h"
#include "ServiceMemory.h"
#include "ServiceTrace.h"
#include "SipStatusCode.h"
#include "call/IMtcCall.h"
#include "call/IMtcCallContext.h"
#include "call/IMtcCallManager.h"
#include "call/IMtcSession.h"
#include "dialingplan/IMtcDialingPlan.h"
#include "ect/EctReference.h"
#include "ect/IEctReferenceListener.h"
#include "helper/sipinterfaceholder/IMtcSipInterfaceFactory.h"
#include "helper/sipinterfaceholder/ReferenceInterfaceHolder.h"
#include "utility/IMessageUtils.h"
// TODO: move UriFormatter to common part.
#include "conferencecall/UriFormatter.h"

__IMS_TRACE_TAG_COM_MTC__;

LOCAL const IMS_CHAR METHOD_INVITE[] = "INVITE";

PUBLIC
EctReference::EctReference(IN IMtcContext& objContext, IN CallKey nTransfereeKey,
        IN IEctReferenceListener& objListener) :
        m_objContext(objContext),
        m_nTransfereeKey(nTransfereeKey),
        m_objListener(objListener),
        m_piReference(IMS_NULL)
{
    IMS_TRACE_I("+EctReference", 0, 0, 0);
}

PUBLIC EctReference::~EctReference()
{
    IMS_TRACE_I("~EctReference", 0, 0, 0);
    m_objContext.GetSipInterfaceFactory().GetIReferenceHolder()->ReleaseIReference(m_piReference);
}

PUBLIC VIRTUAL void EctReference::ReferenceDelivered(IN IReference* piReference)
{
    IMS_TRACE_I("ReferenceDelivered", 0, 0, 0);

    IMessage* piReferMessage = piReference->GetPreviousResponse(IMessage::REFERENCE_REFER);

    if (piReferMessage == IMS_NULL)
    {
        return m_objListener.OnReferenceStarted();
    }

    IMS_SINT32 nStatusCode = piReferMessage->GetStatusCode();

    if (SipStatusCode::IsFinalSuccess(nStatusCode))
    {
        return m_objListener.OnReferenceStarted();
    }
    else
    {
        IMS_TRACE_I("ReferenceDelivered : but response is failure", 0, 0, 0);
        return m_objListener.OnReferenceStartFailed();
    }
}

PUBLIC VIRTUAL void EctReference::ReferenceDeliveryFailed(IN IReference* piReference)
{
    IMS_TRACE_I("ReferenceDeliveryFailed", 0, 0, 0);

    (void)piReference;
    m_objListener.OnReferenceStartFailed();
}

PUBLIC VIRTUAL void EctReference::ReferenceNotify(IN IReference* piReference, IN IMessage* piNotify)
{
    (void)piReference;

    AString strSubState =
            m_objContext.GetMessageUtils().GetHeaderValue(piNotify, ISipHeader::SUBSCRIPTION_STATE);
    IMS_SINT32 nStatusCode = m_objContext.GetMessageUtils().GetStatusCodeInNotify(piNotify);
    IMS_TRACE_I(
            "ReferenceNotify : state[%s] status-code[%d]", strSubState.GetStr(), nStatusCode, 0);

    m_objListener.OnReferenceUpdated(nStatusCode);
}

PUBLIC VIRTUAL void EctReference::ReferenceTerminated(IN IReference* piReference)
{
    IMS_TRACE_I("ReferenceTerminated", 0, 0, 0);

    (void)piReference;
}

PUBLIC VIRTUAL IMS_RESULT EctReference::SendInvite(IN CallKey nTransferTargetKey)
{
    IMS_TRACE_I("SendInvite", 0, 0, 0);

    if (m_objContext.GetCallManager().GetCallByCallKey(m_nTransfereeKey)->GetState() ==
            IMtcCall::State::TERMINATING)
    {
        return IMS_FAILURE;
    }

    IMtcCall* piTransferTargetCall =
            m_objContext.GetCallManager().GetCallByCallKey(nTransferTargetKey);

    return SendRefer(GetReferToUri(piTransferTargetCall), piTransferTargetCall);
}

PUBLIC VIRTUAL IMS_RESULT EctReference::SendInvite(IN const AString& strTransferTarget)
{
    IMS_TRACE_I("SendInvite", 0, 0, 0);

    if (m_objContext.GetCallManager().GetCallByCallKey(m_nTransfereeKey)->GetState() ==
            IMtcCall::State::TERMINATING)
    {
        return IMS_FAILURE;
    }

    return SendRefer(GetReferToUri(strTransferTarget), IMS_NULL);
}

PUBLIC VIRTUAL IMS_UINT32 EctReference::GetResponseCode() const
{
    IMessage* piReferMessage = m_piReference->GetPreviousResponse(IMessage::REFERENCE_REFER);

    if (piReferMessage == IMS_NULL)
    {
        return SipStatusCode::SC_INVALID;
    }

    return piReferMessage->GetStatusCode();
}

PRIVATE
IMS_RESULT EctReference::SendRefer(
        IN const AString& strReferToUri, IN IMtcCall* piTransferTargetCall)
{
    if (strReferToUri.GetLength() <= 0)
    {
        IMS_TRACE_E(0, "no Refer-To URI", 0, 0, 0);
        return IMS_FAILURE;
    }

    IMtcCall* piTransfereeCall = m_objContext.GetCallManager().GetCallByCallKey(m_nTransfereeKey);
    if (piTransfereeCall->GetKey() == IMtcCall::CALL_KEY_INVALID)
    {
        IMS_TRACE_E(0, "transferee call is NullCall", 0, 0, 0);
        return IMS_FAILURE;
    }

    m_piReference = m_objContext.GetSipInterfaceFactory().GetIReferenceHolder()->GetIReference(
            &m_objContext.GetCallManager()
                     .GetCallByCallKey(m_nTransfereeKey)
                     ->GetCallContext()
                     .GetSession()
                     ->GetISession(),
            strReferToUri, METHOD_INVITE);

    if (m_piReference == IMS_NULL)
    {
        IMS_TRACE_E(0, "piReference is null", 0, 0, 0);
        return IMS_FAILURE;
    }
    m_piReference->SetListener(this);

    if (piTransferTargetCall != IMS_NULL)
    {
        SetReplaces(piTransferTargetCall);
    }
    SetReferredByHeader();

    return m_piReference->ReferEx(IMS_TRUE, "Require=replaces");
}

PRIVATE
AString EctReference::GetReferToUri(IN IMtcCall* piTransferTargetCall)
{
    AString strUri;
    return UriFormatter::GetReferToForInvite(strUri, piTransferTargetCall->GetCallContext());
}

PRIVATE
AString EctReference::GetReferToUri(IN const AString& strTransferTarget) const
{
    return m_objContext.GetDialingPlan().GetToUri(strTransferTarget, CallInfo(), Scheme::SIP);
}

PRIVATE
void EctReference::SetReplaces(IN IMtcCall* piTransferTargetCall)
{
    if (piTransferTargetCall == IMS_NULL)
    {
        return;
    }
    AString strSessionId = m_objContext.GetMessageUtils().GetSessionId(
            &(piTransferTargetCall->GetCallContext().GetSession()->GetISession()));
    m_piReference->SetReplaces(strSessionId);
}

PRIVATE
void EctReference::SetReferredByHeader()
{
    AString strLocalUri =
            m_objContext.GetServiceByType(ServiceType::NORMAL)->GetICoreService()->GetLocalUserId();
    if (strLocalUri.GetLength() <= 0)
    {
        return;
    }

    IMessage* piMessage = m_piReference->GetNextRequest();
    if (piMessage == IMS_NULL)
    {
        return;
    }

    ISipMessage* piSipMessage = piMessage->GetMessage();
    piSipMessage->AddHeader(ISipHeader::REFERRED_BY, strLocalUri);
}
