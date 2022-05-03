#include "ServiceMemory.h"
#include "ServiceTrace.h"

#include "ISession.h"
#include "ICoreService.h"
#include "call/IMtcCallContext.h"
#include "IReference.h"
#include "IMessage.h"
#include "ISipMessage.h"
#include "ISipHeader.h"
#include "SipStatusCode.h"
#include "SipHeaderName.h"

#include "IMtcService.h"
#include "utility/MessageUtil.h"
#include "configuration/ConfigDef.h"
#include "configuration/MtcConfigurationProxy.h"
#include "call/IMtcCallManager.h"
#include "call/IMtcCall.h"
#include "call/MtcSession.h"
#include "conferencecall/ConferenceDef.h"
#include "conferencecall/ConferenceUtils.h"
#include "conferencecall/ConferenceReference.h"
#include "conferencecall/IConferenceReference.h"
#include "conferencecall/UriFormatter.h"
#include "conferencecall/CallConnectionIdManager.h"
#include "helper/sipinterfaceholder/MtcSipInterfaceFactory.h"
#include "helper/sipinterfaceholder/ReferenceInterfaceHolder.h"
#include "conferencecall/ConferenceConfigurationWrapper.h"
#include "conferencecall/IConferenceReferenceListener.h"

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
        m_objConfUsers(IMSList<ConfUser*>()),
        m_piReference(IMS_NULL),
        m_bForceToTerminateInterface(IMS_FALSE)
{
    IMS_TRACE_I("+ConferenceReference", 0, 0, 0);
}

PUBLIC
ConferenceReference::ConferenceReference(IN IMtcContext& objContext, IN CallKey nConfCallKey,
        IN IMSList<ConfUser*>& objConfUsers, IN IConferenceReferenceListener& objListener) :
        m_objContext(objContext),
        m_nConfCallKey(nConfCallKey),
        m_objListener(objListener),
        m_nType(REFERENCE_TYPE_INVALID),
        m_pConfUser(IMS_NULL),
        m_objConfUsers(IMSList<ConfUser*>(objConfUsers)),
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

    if (ConferenceConfigurationWrapper::IsReferSubscriptionRequired())
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

    AString strSubState;
    MessageUtil::GetHeaderValue(piNotify, ISipHeader::SUBSCRIPTION_STATE, strSubState);
    IMS_SINT32 nStatusCode = MessageUtil::GetStatusCodeInNotify(piNotify);
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

    // TODO: how to get CallState through IMtcCallContext.
    if (m_objContext.GetCallManager().GetCallByCallKey(m_nConfCallKey)->GetState() ==
            IMtcCall::State::TERMINATING)
    {
        return IMS_FAILURE;
    }

    m_nType = REFERENCE_TYPE_INVITE;
    IMtcCall* pi1To1Call = m_pConfUser == IMS_NULL
            ? IMS_NULL
            : m_objContext.GetCallManager().GetCallByCallKey(
                      objConnectionIdManager.GetCallKey(m_pConfUser->nConnectionId));

    // 1. Form Refer-To URI
    GetReferToUri(strReferToUri, pi1To1Call);

    if (strReferToUri.GetLength() <= 0)
    {
        return IMS_FAILURE;
    }

    // 2. get IReference interface
    m_piReference = m_objContext.GetSipInterfaceFactory().GetIReferenceHolder()->GetIReference(
            &m_objContext.GetCallManager()
                     .GetCallByCallKey(m_nConfCallKey)
                     ->GetCallContext()
                     .GetSession()
                     ->GetISession(),
            strReferToUri, METHOD_INVITE);

    if (m_piReference == IMS_NULL)
    {
        return IMS_FAILURE;
    }
    m_piReference->SetListener(this);

    // 3. special handling for resource list.
    if (m_objConfUsers.GetSize() > 0)
    {
        IMessage* piReferMessage = m_piReference->GetNextRequest();
        // UriFormatter??

        AString strContentDisposition = "recipient-list";
        piReferMessage->AddHeader(SipHeaderName::CONTENT_DISPOSITION, strContentDisposition);

        MessageUtil::SetResourceListByConfUser(
                piReferMessage, strReferToUri, m_objConfUsers, IMS_FALSE, IMS_TRUE);
    }

    // 4. Set Replaces header
    AString strHeadersForReferTo;

    if (pi1To1Call != IMS_NULL)
    {
        SetReplaces(pi1To1Call);
        SetHeadersForReferTo(strHeadersForReferTo);
    }

    // 5. Set Referred-By header
    SetReferredByHeader();

    // 6. Send Refer
    // m_bImplicitSubscription should be set by conference configuration
    return m_piReference->ReferEx(
            ConferenceConfigurationWrapper::IsReferSubscriptionRequired(), strHeadersForReferTo);
}

PUBLIC VIRTUAL IMS_RESULT ConferenceReference::SendBye(
        IN AString strInvitedUri /* = AString::ConstEmpty()*/)
{
    IMS_TRACE_I("SendBye", 0, 0, 0);

    m_nType = REFERENCE_TYPE_BYE;
    AString strReferToUri;
    UriFormatter::GetReferToForBye(strReferToUri, m_pConfUser, strInvitedUri);

    m_piReference = m_objContext.GetSipInterfaceFactory().GetIReferenceHolder()->GetIReference(
            &m_objContext.GetCallManager()
                     .GetCallByCallKey(m_nConfCallKey)
                     ->GetCallContext()
                     .GetSession()
                     ->GetISession(),
            strReferToUri, METHOD_BYE);

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
    IMessage* piReferMessage = m_piReference->GetPreviousResponse(IMessage::REFERENCE_REFER);

    if (piReferMessage == IMS_NULL)
    {
        return SipStatusCode::SC_INVALID;
    }

    return piReferMessage->GetStatusCode();
}

PRIVATE
void ConferenceReference::GetReferToUri(OUT AString& strUri, IN IMtcCall* pi1to1Call) const
{
    // Send Refer with Session..
    if (pi1to1Call != IMS_NULL)
    {
        UriFormatter::GetReferToForInvite(strUri, pi1to1Call->GetCallContext());
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
        strUri = m_objContext.GetConfigurationProxy().GetStr(Feature::CONFERENCE_FACTORY_URI, 0);
    }
}

PRIVATE
void ConferenceReference::SetReplaces(IN IMtcCall* piCall)
{
    if (piCall == IMS_NULL)
    {
        return;
    }
    AString strSessionId;
    MessageUtil::GetSessionId(
            &(piCall->GetCallContext().GetSession()->GetISession()), strSessionId);
    m_piReference->SetReplaces(strSessionId);
}

PRIVATE
void ConferenceReference::SetReferredByHeader()
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

PRIVATE
void ConferenceReference::SetHeadersForReferTo(OUT AString& strHeadersForReferTo)
{
    strHeadersForReferTo = AString::ConstNull();

    if (ConferenceConfigurationWrapper::IsReferToExHeaderUsed())
    {
        strHeadersForReferTo = "Require=replaces";
    }
}
