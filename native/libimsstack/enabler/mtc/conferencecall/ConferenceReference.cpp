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

#include "CarrierConfig.h"
#include "ICoreService.h"
#include "IMessage.h"
#include "IMtcService.h"
#include "IReference.h"
#include "ISession.h"
#include "ISipHeader.h"
#include "ISipMessage.h"
#include "ServiceMemory.h"
#include "ServiceTrace.h"
#include "SipHeaderName.h"
#include "SipStatusCode.h"
#include "call/CallConnectionIdManager.h"
#include "call/IMtcCall.h"
#include "call/IMtcCallContext.h"
#include "call/IMtcCallManager.h"
#include "call/IMtcSession.h"
#include "conferencecall/ConferenceConfigurationHelper.h"
#include "conferencecall/ConferenceDef.h"
#include "conferencecall/ConferenceReference.h"
#include "conferencecall/ConferenceUtils.h"
#include "conferencecall/IConferenceReference.h"
#include "conferencecall/IConferenceReferenceListener.h"
#include "conferencecall/UriFormatter.h"
#include "configuration/ConfigDef.h"
#include "configuration/MtcConfigurationProxy.h"
#include "helper/sipinterfaceholder/IMtcSipInterfaceFactory.h"
#include "helper/sipinterfaceholder/ReferenceInterfaceHolder.h"
#include "utility/IMessageUtils.h"

__IMS_TRACE_TAG_COM_MTC__;

PUBLIC GLOBAL const IMS_CHAR ConferenceReference::METHOD_INVITE[] = "INVITE";
const IMS_CHAR ConferenceReference::METHOD_BYE[] = "BYE";

PUBLIC
ConferenceReference::ConferenceReference(IN IMtcContext& objContext, IN CallKey nConfCallKey,
        IN ConfUser* pConfUser, IN IConferenceReferenceListener& objListener) :
        m_objContext(objContext),
        m_nConfCallKey(nConfCallKey),
        m_objListener(objListener),
        m_nType(REFERENCE_TYPE_INVALID),
        m_pConfUser(pConfUser),
        m_objConfUsers(ImsList<ConfUser*>()),
        m_piReference(IMS_NULL),
        m_bForceToTerminateInterface(IMS_FALSE)
{
    IMS_TRACE_I("+ConferenceReference", 0, 0, 0);
}

PUBLIC
ConferenceReference::ConferenceReference(IN IMtcContext& objContext, IN CallKey nConfCallKey,
        IN ImsList<ConfUser*>& objConfUsers, IN IConferenceReferenceListener& objListener) :
        m_objContext(objContext),
        m_nConfCallKey(nConfCallKey),
        m_objListener(objListener),
        m_nType(REFERENCE_TYPE_INVALID),
        m_pConfUser(IMS_NULL),
        m_objConfUsers(ImsList<ConfUser*>(objConfUsers)),
        m_piReference(IMS_NULL),
        m_bForceToTerminateInterface(IMS_FALSE)
{
    IMS_TRACE_I("+ConferenceReference", 0, 0, 0);
}

PUBLIC VIRTUAL ConferenceReference::~ConferenceReference()
{
    IMS_TRACE_I("~ConferenceReference", 0, 0, 0);
    m_objContext.GetSipInterfaceFactory().GetIReferenceHolder()->ReleaseIReference(
            m_piReference, m_bForceToTerminateInterface);

    m_objConfUsers.Clear();
}

PUBLIC VIRTUAL void ConferenceReference::ReferenceDelivered(IN IReference* piReference)
{
    IMS_TRACE_I("ReferenceDelivered", 0, 0, 0);

    if (ConferenceConfigurationHelper::IsReferSubscriptionRequired(
                m_objContext.GetConfigurationProxy()))
    {
        return m_objListener.OnReferenceStarted(this);
    }

    IMessage* piReferMessage = piReference->GetPreviousResponse(IMessage::REFERENCE_REFER);

    if (piReferMessage == IMS_NULL)
    {
        return m_objListener.OnReferenceStarted(this);
    }

    IMS_SINT32 nStatusCode = piReferMessage->GetStatusCode();

    if (SipStatusCode::IsFinalSuccess(nStatusCode))
    {
        return m_objListener.OnReferenceStarted(this);
    }
    else
    {
        // TODO: is this reachable?
        IMS_TRACE_I("ReferenceDelivered : but response is failure", 0, 0, 0);
        return m_objListener.OnReferenceStartFailed(this);
    }
}

PUBLIC VIRTUAL void ConferenceReference::ReferenceDeliveryFailed(IN IReference* piReference)
{
    IMS_TRACE_I("ReferenceDeliveryFailed", 0, 0, 0);

    (void)piReference;
    m_objListener.OnReferenceStartFailed(this);
}

PUBLIC VIRTUAL void ConferenceReference::ReferenceNotify(
        IN IReference* piReference, IN IMessage* piNotify)
{
    (void)piReference;

    AString strSubState =
            m_objContext.GetMessageUtils().GetHeaderValue(piNotify, ISipHeader::SUBSCRIPTION_STATE);
    IMS_SINT32 nStatusCode = m_objContext.GetMessageUtils().GetStatusCodeInNotify(piNotify);
    IMS_TRACE_I(
            "ReferenceNotify : state[%s] status-code[%d]", strSubState.GetStr(), nStatusCode, 0);

    ReferSubscriptionState eState = ReferSubscriptionState::INVALID;
    if (strSubState.Equals("active"))
    {
        eState = ReferSubscriptionState::ACTIVE;
    }
    else if (strSubState.Equals("terminated"))
    {
        eState = ReferSubscriptionState::TERMINATED;
    }

    m_objListener.OnReferenceUpdated(this, nStatusCode, eState);
}

PUBLIC VIRTUAL void ConferenceReference::ReferenceTerminated(IN IReference* piReference)
{
    IMS_TRACE_I("ReferenceTerminated", 0, 0, 0);

    (void)piReference;
    m_objListener.OnReferenceUpdated(
            this, SipStatusCode::SC_INVALID, ReferSubscriptionState::TERMINATED);
    // do not add logic here. instance will be destroyed before here.
}

PUBLIC VIRTUAL IMS_RESULT ConferenceReference::SendInvite(
        OUT AString& strReferToUri, IN CallConnectionIdManager& objConnectionIdManager)
{
    IMS_TRACE_I("SendInvite", 0, 0, 0);

    IMtcCall* piConfCall = GetConferenceCall();
    if (piConfCall == IMS_NULL)
    {
        // TODO: check for KR conference call cases.
        return IMS_FAILURE;
    }

    if (piConfCall->GetState() == IMtcCall::State::TERMINATING)
    {
        return IMS_FAILURE;
    }

    m_nType = REFERENCE_TYPE_INVITE;

    if (m_pConfUser)
    {
        return SendInviteForSingleUser(
                strReferToUri, objConnectionIdManager.GetCallKey(m_pConfUser->nConnectionId));
    }
    else if (m_objConfUsers.GetSize() > 0)
    {
        return SendInviteForMultipleUser(strReferToUri);
    }
    else
    {
        return IMS_FAILURE;
    }
}

PUBLIC VIRTUAL IMS_RESULT ConferenceReference::SendBye(
        IN AString strInvitedUri /* = AString::ConstEmpty()*/)
{
    IMS_TRACE_I("SendBye", 0, 0, 0);

    m_nType = REFERENCE_TYPE_BYE;
    AString strReferToUri;
    UriFormatter::GetReferToForBye(
            strReferToUri, m_objContext.GetConfigurationProxy(), m_pConfUser, strInvitedUri);

    m_piReference = GetIReference(strReferToUri, METHOD_BYE);
    if (m_piReference == IMS_NULL)
    {
        IMS_TRACE_I("SendBye : m_piReference is null", 0, 0, 0);
        return IMS_FAILURE;
    }

    m_piReference->SetListener(this);

    SetReferredByHeader();

    return m_piReference->Refer(IMS_TRUE);
}

PUBLIC VIRTUAL IMS_UINT32 ConferenceReference::GetType() const
{
    IMS_TRACE_I("GetType : [%d]", m_nType, 0, 0);

    return m_nType;
}

PUBLIC VIRTUAL IMS_UINT32 ConferenceReference::GetResponseCode() const
{
    if (m_piReference == IMS_NULL)
    {
        return SipStatusCode::SC_INVALID;
    }

    IMessage* piReferMessage = m_piReference->GetPreviousResponse(IMessage::REFERENCE_REFER);
    if (piReferMessage == IMS_NULL)
    {
        return SipStatusCode::SC_INVALID;
    }

    return piReferMessage->GetStatusCode();
}

PRIVATE
IMS_RESULT ConferenceReference::SendInviteForSingleUser(
        OUT AString& strReferToUri, IN CallKey n1To1Key)
{
    IMtcCall* pi1To1Call = m_objContext.GetCallManager().GetCallByCallKey(n1To1Key);
    if (pi1To1Call->GetKey() == IMtcCall::CALL_KEY_INVALID)
    {
        pi1To1Call = IMS_NULL;
    }

    IMS_TRACE_D("SendInviteForSingleUser key[%d]", n1To1Key, 0, 0);

    // 1. Form Refer-To URI
    GetReferToUri(strReferToUri, pi1To1Call);
    if (strReferToUri.GetLength() <= 0)
    {
        return IMS_FAILURE;
    }

    // 2. get IReference interface
    m_piReference = GetIReference(strReferToUri, METHOD_INVITE);
    if (m_piReference == IMS_NULL)
    {
        return IMS_FAILURE;
    }
    m_piReference->SetListener(this);

    // 3. Set Replaces header
    AString strHeadersForReferTo;
    SetReplaces(pi1To1Call);
    SetHeadersForReferTo(strHeadersForReferTo);

    // 4. Set Referred-By header
    SetReferredByHeader();

    // 5. Send Refer
    return m_piReference->ReferEx(ConferenceConfigurationHelper::IsReferSubscriptionRequired(
                                          m_objContext.GetConfigurationProxy()),
            strHeadersForReferTo);
}

PRIVATE
IMS_RESULT ConferenceReference::SendInviteForMultipleUser(OUT AString& strReferToUri)
{
    // 1. Form Refer-To URI
    GetReferToUri(strReferToUri, IMS_NULL);
    if (strReferToUri.GetLength() <= 0)
    {
        return IMS_FAILURE;
    }

    // 2. get IReference interface
    m_piReference = GetIReference(strReferToUri, METHOD_INVITE);
    if (m_piReference == IMS_NULL)
    {
        return IMS_FAILURE;
    }
    m_piReference->SetListener(this);

    // 3. resource-list
    IMessage* piReferMessage = m_piReference->GetNextRequest();
    AString strContentDisposition = "recipient-list";
    piReferMessage->AddHeader(SipHeaderName::CONTENT_DISPOSITION, strContentDisposition);

    m_objContext.GetMessageUtils().SetResourceList(
            piReferMessage, m_objContext, strReferToUri, m_objConfUsers, IMS_FALSE, IMS_FALSE);

    // 4. Set Referred-By header
    SetReferredByHeader();

    // 5. Send Refer
    return m_piReference->ReferEx(ConferenceConfigurationHelper::IsReferSubscriptionRequired(
            m_objContext.GetConfigurationProxy()));
}

PRIVATE
void ConferenceReference::GetReferToUri(OUT AString& strUri, IN IMtcCall* pi1To1Call) const
{
    // Send Refer with Session..
    if (pi1To1Call != IMS_NULL)
    {
        UriFormatter::GetReferToForInvite(strUri, pi1To1Call->GetCallContext());
    }
    // Send Refer with Target number - single refer
    else if (m_pConfUser != IMS_NULL)
    {
        UriFormatter::GetReferToForInvite(strUri,
                m_objContext.GetCallManager().GetCallByCallKey(m_nConfCallKey)->GetCallContext(),
                m_pConfUser);
    }
    // Send Refer with Target number - multiple refer with resource list
    else if (m_objConfUsers.GetSize() > 0)
    {
        strUri = m_objContext.GetConfigurationProxy().GetString(
                ConfigVoice::KEY_CONFERENCE_FACTORY_URI_STRING);
    }
}

PRIVATE
void ConferenceReference::SetReplaces(IN IMtcCall* pi1To1Call)
{
    if (pi1To1Call == IMS_NULL)
    {
        return;
    }

    AString strSessionId = m_objContext.GetMessageUtils().GetSessionId(
            &(pi1To1Call->GetCallContext().GetSession()->GetISession()));
    m_piReference->SetReplaces(strSessionId);
}

PRIVATE
void ConferenceReference::SetReferredByHeader()
{
    IMtcService* piMtcService = m_objContext.GetServiceByType(ServiceType::NORMAL);
    if (piMtcService == IMS_NULL)
    {
        return;
    }

    AString strLocalUri = piMtcService->GetICoreService()->GetLocalUserId();
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

PRIVATE
void ConferenceReference::SetHeadersForReferTo(OUT AString& strHeadersForReferTo)
{
    strHeadersForReferTo = AString::ConstNull();

    if (ConferenceConfigurationHelper::IsReferToExHeaderUsed(m_objContext.GetConfigurationProxy()))
    {
        strHeadersForReferTo = "Require=replaces";
    }
}

PRIVATE
IMtcCall* ConferenceReference::GetConferenceCall()
{
    IMtcCall* piCall = m_objContext.GetCallManager().GetCallByCallKey(m_nConfCallKey);
    if (piCall->GetKey() == IMtcCall::CALL_KEY_INVALID)
    {
        return IMS_NULL;
    }

    return piCall;
}

PRIVATE
IReference* ConferenceReference::GetIReference(
        IN const AString& strInvitedUri, IN const AString& strMethod)
{
    IMtcCall* piCall = GetConferenceCall();
    if (piCall == IMS_NULL)
    {
        return IMS_NULL;
    }

    IMtcSession* pMtcSession = piCall->GetCallContext().GetSession();
    if (pMtcSession == IMS_NULL)
    {
        IMS_TRACE_D("GetIReference null", 0, 0, 0);
        return IMS_NULL;
    }

    IMS_TRACE_D("GetIReference", 0, 0, 0);
    return m_objContext.GetSipInterfaceFactory().GetIReferenceHolder()->GetIReference(
            &pMtcSession->GetISession(), strInvitedUri, strMethod);
}
