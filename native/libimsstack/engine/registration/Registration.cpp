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
#include "IDigestAka.h"
#include "NatHelper.h"
#include "ServiceMemory.h"
#include "ServiceNetwork.h"
#include "ServicePhoneInfo.h"
#include "ServiceTrace.h"
#include "ServiceUtil.h"
#include "TextParser.h"

#include "private/ConfigurationManager.h"
#include "private/SubscriberConfig.h"

#include "Connector.h"
#include "IRegBindingStateListener.h"
#include "IRegInfoContact.h"
#include "IRegInfoManager.h"
#include "IRegInfoRegistration.h"
#include "IRegUserIdentityNotifier.h"
#include "IRegistrationListener.h"
#include "ISipClientConnection.h"
#include "ISipConnectionNotifier.h"
#include "ISipHeader.h"
#include "ISipGenericChallenge.h"
#include "ISipMessage.h"
#include "ISipTransportHelper.h"
#include "ImsCoreContext.h"
#include "PAccessNetworkInfoHeader.h"
#include "RegBindingProxy.h"
#include "RegFlow.h"
#include "RegInfo.h"
#include "RegObserver.h"
#include "RegRefreshHelper.h"
#include "RegSubscription.h"
#include "Registration.h"
#include "Sip.h"
#include "SipConfigProxy.h"
#include "SipDebug.h"
#include "SipFactory.h"
#include "SipParameter.h"
#include "SipParsingHelper.h"
#include "SipStatusCode.h"
#include "SipTimerValuesHelper.h"
#include "SipUrnHelper.h"
#include "base/Ims.h"
#include "base/SubscriberTracker.h"

__IMS_TRACE_TAG_REG__;

PUBLIC
Registration::Registration() :
        EngineActivity(),
        m_bDestroyed(IMS_FALSE),
        m_nState(STATE_CREATED),
        m_nSubState(SUB_STATE_IDLE),
        m_pRegFlow(IMS_NULL),
        m_pRegParam(IMS_NULL),
        m_objContacts(ImsList<RegContact*>()),
        m_pStateTracker(new RegStateTracker()),
        m_piListener(IMS_NULL),
        m_piBindingStateListener(IMS_NULL),
        m_piUserIdNotifier(IMS_NULL),
        m_pRefreshHelper(IMS_NULL),
        m_piDigestAka(IMS_NULL),
        m_piGenericChallenge(IMS_NULL),
        m_piOngoingScc(IMS_NULL),
        m_piNextRequest(IMS_NULL),
        m_piPreviousRequest(IMS_NULL),
        m_piPreviousResponse(IMS_NULL),
        m_piMessageMediator(IMS_NULL),
        m_objObservers(ImsList<RegObserver*>()),
        m_bIsBehindNat(IMS_FALSE),
        m_bIsWithinTrustDomain(IMS_TRUE),
        m_bActiveBindingsRestorationEnabled(IMS_FALSE),
        m_nRefCountForScnErrorListener(0)
{
}

PUBLIC VIRTUAL Registration::~Registration()
{
    if (m_piOngoingScc != IMS_NULL)
    {
        m_piOngoingScc->Close();
        m_piOngoingScc = IMS_NULL;
    }

    ClearNextRequest();
    SetPreviousRequest(IMS_NULL);
    SetPreviousResponse(IMS_NULL);

    if (m_piDigestAka != IMS_NULL)
    {
        m_piDigestAka->Destroy();
        m_piDigestAka = IMS_NULL;
    }

    // IMS_AUTH_NONCE_REUSE {
    SetAuthenticationChallenge(IMS_NULL);
    // }

    if (m_pRefreshHelper != IMS_NULL)
    {
        delete m_pRefreshHelper;
        m_pRefreshHelper = IMS_NULL;
    }

    if (m_pRegParam != IMS_NULL)
    {
        delete m_pRegParam;
        m_pRegParam = IMS_NULL;
    }

    RegBindingProxy::DestroyBinding(GetSlotId(), this);

    IMS_TRACE_D("Destructor :: Registration - %s",
            (m_pRegFlow != IMS_NULL) ? SipDebug::GetCharA1(m_pRegFlow->GetCallId().GetStr(), 8, '@')
                                     : "__UNKNOWN__",
            0, 0);

    if (m_pRegFlow != IMS_NULL)
    {
        delete m_pRegFlow;
        m_pRegFlow = IMS_NULL;
    }
}

PUBLIC VIRTUAL IMS_BOOL Registration::Equals(IN const IRegistration* piReg) const
{
    const Registration* pReg = DYNAMIC_CAST(const Registration*, piReg);

    if (pReg == IMS_NULL)
    {
        return IMS_FALSE;
    }

    return m_pRegFlow->GetRegKey().Equals(pReg->m_pRegFlow->GetRegKey());
}

PUBLIC VIRTUAL const RegInfo* Registration::GetRegInfo() const
{
    if (m_pRegFlow == IMS_NULL)
    {
        return IMS_NULL;
    }

    return ImsCoreContext::GetInstance()->GetRegInfoManager()->GetRegInfo(m_pRegFlow->GetRegKey());
}

PUBLIC
IMS_BOOL Registration::Create(IN IMS_UINT32 nFlowId, IN const SipAddress& objAor,
        IN const AString& strSubsId /*= AString::ConstNull()*/,
        IN SipProfile* pProfile /*= IMS_NULL*/)
{
    m_pRegFlow = new RegFlow(RegKey(GetSlotId(), nFlowId));

    if (m_pRegFlow == IMS_NULL)
    {
        IMS_TRACE_E(0, "Instantiating RegFlow(%d:%d) failed", GetSlotId(), nFlowId, 0);
        return IMS_FALSE;
    }

    m_pRegParam = new RegParameter(GetSlotId());

    if (m_pRegParam == IMS_NULL)
    {
        IMS_TRACE_E(0, "Instantiating RegParameter failed", 0, 0, 0);
        return IMS_FALSE;
    }

    if (!m_pRegParam->UpdateProfile(objAor, strSubsId))
    {
        IMS_TRACE_E(0, "Updating a profile failed", 0, 0, 0);
        return IMS_FALSE;
    }

    m_pRefreshHelper = new RegRefreshHelper(this);

    if (m_pRefreshHelper == IMS_NULL)
    {
        IMS_TRACE_E(0, "Instantiating RefreshHelper failed", 0, 0, 0);
        return IMS_FALSE;
    }

    SetSipProfile(pProfile);

    m_pStateTracker->SetAor(objAor);
    m_pStateTracker->SetSubscriberId(strSubsId);

    return IMS_TRUE;
}

PUBLIC
void Registration::Destroy()
{
    m_bDestroyed = IMS_TRUE;

    IMS_TRACE_D("Registration :: Destroy() - SCNEL=%d", m_nRefCountForScnErrorListener, 0, 0);

    RegInfo* pRegInfo =
            ImsCoreContext::GetInstance()->GetRegInfoManager()->GetRegInfo(m_pRegFlow->GetRegKey());

    if (pRegInfo != IMS_NULL)
    {
        pRegInfo->RemoveListener(this);
    }

    DestroyAllContacts();
    UpdateBindingState(BINDING_DESTROY);

    if (m_pRefreshHelper != IMS_NULL)
    {
        if (m_pRefreshHelper->IsRequestPending())
        {
            m_pRefreshHelper->AbortConnection();
        }
    }

    if (m_piOngoingScc != IMS_NULL)
    {
        m_piOngoingScc->Close();
        m_piOngoingScc = IMS_NULL;
    }

    if (m_piDigestAka != IMS_NULL)
    {
        m_piDigestAka->Destroy();
        m_piDigestAka = IMS_NULL;
    }

    SetSipMessageMediator(IMS_NULL);

    // NAT_REQ_UE_PUBLIC_IP
    if (NatHelper::IsNatResolverRequired())
    {
        NatHelper::GetInstance()->RemovePublicAddress(
                GetSlotId(), m_pRegFlow->GetRegKey().GetFlowId());
    }

    PostMessage(AMSG_DESTROY, 0, 0);
}

PUBLIC
IMS_BOOL Registration::HasActiveBindings() const
{
    for (IMS_UINT32 i = 0; i < m_objContacts.GetSize(); ++i)
    {
        RegContact* pContact = m_objContacts.GetAt(i);

        if (pContact->IsActiveBinding())
        {
            return IMS_TRUE;
        }
    }

    return IMS_FALSE;
}

PUBLIC
IMS_BOOL Registration::IsAllBindingsRemoved() const
{
    for (IMS_UINT32 i = 0; i < m_objContacts.GetSize(); ++i)
    {
        RegContact* pContact = m_objContacts.GetAt(i);

        if (!pContact->IsEmpty())
        {
            return IMS_FALSE;
        }
    }

    return IMS_TRUE;
}

PRIVATE VIRTUAL IMS_BOOL Registration::DispatchMessage(IN ImsMessage& objMsg)
{
    switch (objMsg.GetName())
    {
        case AMSG_REGISTRATION_STARTED:
            if (m_piListener != IMS_NULL)
            {
                m_piListener->Registration_Started();
            }
            return IMS_TRUE;
        case AMSG_REGISTRATION_START_FAILED:
            if (m_piListener != IMS_NULL)
            {
                m_piListener->Registration_StartFailed(LONG_TO_SINT(objMsg.nWparam));
            }
            return IMS_TRUE;
        case AMSG_REGISTRATION_UPDATED:
            if (m_piListener != IMS_NULL)
            {
                m_piListener->Registration_Updated();
            }
            return IMS_TRUE;
        case AMSG_REGISTRATION_UPDATE_FAILED:
            if (m_piListener != IMS_NULL)
            {
                m_piListener->Registration_UpdateFailed(LONG_TO_SINT(objMsg.nWparam));
            }
            return IMS_TRUE;
        case AMSG_REGISTRATION_REMOVED:
            if (m_piListener != IMS_NULL)
            {
                m_piListener->Registration_Removed();
            }
            return IMS_TRUE;
        case AMSG_REGISTRATION_TERMINATED:
            if (m_piListener != IMS_NULL)
            {
                m_piListener->Registration_Terminated(LONG_TO_SINT(objMsg.nWparam));
            }
            return IMS_TRUE;
        case AMSG_REGISTRATION_AKA_RESPONSE_RECEIVED:
            if (m_bDestroyed)
            {
                IMS_TRACE_D("Registration is already destroyed :: AKA_RESPONSE_RECEIVED", 0, 0, 0);
                return IMS_TRUE;
            }

            if (!CreateSa(m_objActiveCredential, m_objActiveSaKey))
            {
                IMS_TRACE_E(0, "Creating SA failed", 0, 0, 0);
                return IMS_TRUE;
            }

            if (!RespondToPendingChallenge(m_objActiveCredential))
            {
                RestoreSecurityHeaders();
            }
            return IMS_TRUE;
        case AMSG_REGISTRATION_BINDING_STATE_CHANGED:
            if (m_piBindingStateListener == IMS_NULL)
            {
                IMS_TRACE_D("IRegBindingStateListener is null", 0, 0, 0);
                return IMS_TRUE;
            }

            if (LONG_TO_INT(objMsg.nWparam) == BINDING_STATE_CALLER_CAPABILITY)
            {
                m_piBindingStateListener->RegBindingState_CallerCapabilityChanged();
            }
            return IMS_TRUE;
        default:
            break;
    }

    return EngineActivity::DispatchMessage(objMsg);
}

PRIVATE VIRTUAL void Registration::ClientConnection_NotifyResponse(
        IN ISipClientConnection* piScc, IN ISipClientConnection* /*piForkedScc = IMS_NULL*/)
{
    if (piScc == IMS_NULL)
    {
        return;
    }

    if (piScc->Receive() != IMS_SUCCESS)
    {
        IMS_TRACE_E(0, "No message received", 0, 0, 0);
        return;
    }

    IMS_SINT32 nStatusCode = piScc->GetStatusCode();

    IMS_TRACE_I("Registration - %d response to REGISTER received", nStatusCode, 0, 0);

    if (SipStatusCode::Is1XX(nStatusCode))
    {
        return;
    }

    IMS_SINT32 nPrevState = GetState();
    IMS_SINT32 nPrevSubState = GetSubState();

    //// DEBUG
    SipDebug::Send(GetSlotId(), SipDebug::MSG_RSP, SipDebug::DIR_IN,
            (nPrevSubState == SUB_STATE_REGISTERING) ? SipMethod::REGISTER : SipDebug::M_DEREGISTER,
            nStatusCode);

    SetPreviousResponse(piScc->GetMessage());

    // Authentication challenged by S-CSCF
    if ((nStatusCode == SipStatusCode::SC_401) || (nStatusCode == SipStatusCode::SC_407))
    {
        // IMS_AUTH_NONCE_REUSE {
        SetAuthenticationChallenge(IMS_NULL);
        // }

        // Update the Security-xxx headers
        m_pRegParam->UpdateSecurityHeaders(piScc->GetMessage());

        // Respond to the authentication challenge
        if (!RespondToChallenge(piScc))
        {
            RestoreSecurityHeaders();
        }

        if (GetSubState() == SUB_STATE_IDLE)
        {
            piScc->Close();
            m_piOngoingScc = IMS_NULL;
        }

        // 3 Do not close SipConnection in here
        return;
    }

    // Reset the flags after the transaction completed
    SetSubState(SUB_STATE_IDLE);

    piScc->Close();
    m_piOngoingScc = IMS_NULL;

    ISipMessage* piSipMsg = GetPreviousResponse();

    // Handle the response except for 401/407
    if (SipStatusCode::IsFinalSuccess(nStatusCode))
    {
        // Update the Security-xxx headers
        m_pRegParam->UpdateSecurityHeaders(piSipMsg);

        // Update the sequence number
        UpdateCSeqNumber(piSipMsg);

        // Update the bindings
        IMS_RESULT nBindingUpdateResult = UpdateBindings(piSipMsg);
        IMS_SINT32 nReason = REASON_NONE;

        if (nBindingUpdateResult == BINDING_UPDATE_NO_EXPIRES)
        {
            nReason = REASON_NO_EXPIRES;
        }

        // Update the refresh timer
        UpdateRefreshTimer();

        // Check the UA location
        CheckUaLocation(piSipMsg);

        // Store the persistent header information
        StorePersistentHeaders(piSipMsg);

        // IMS_AUTH_NONCE_REUSE {
        SetNextNonce(piSipMsg);
        // }

        // Go to "ACTIVE" state
        if (HasActiveBindings())
        {
            SetState(STATE_ACTIVE);
        }
        else
        {
            nReason = REASON_NO_ACTIVE_BINDINGS;

            if (nPrevState == STATE_INIT)
            {
                SetState(STATE_CREATED);
            }
            else
            {
                // IMS_AUTH_NONCE_REUSE {
                SetAuthenticationChallenge(IMS_NULL);
                // }
                SetState(STATE_TERMINATED);
            }
        }

        if (nBindingUpdateResult == BINDING_UPDATE_OK)
        {
            UpdateBindingState(BINDING_RESULT_OK);
        }
        else
        {
            UpdateBindingState(BINDING_RESULT_NOK);
        }

        CallListener(nPrevState, nPrevSubState, nReason);
    }
    else
    {
        // Do not clear the authentication challenge...
        // Initial registration required: 305 / 408 / 500 / 504 / 600 / txn timeout
        // // IMS_AUTH_NONCE_REUSE {
        // SetAuthenticationChallenge(IMS_NULL);
        // // }

        // Do not check the status code to support re-use of Call-ID & CSeq number
        // when the registration is failed
        // if (nStatusCode == SipStatusCode::SC_423)
        {
            // Update the sequence number
            UpdateCSeqNumber(piSipMsg);
        }

        if (nPrevState == STATE_INIT)
        {
            SetState(STATE_CREATED);
        }

        UpdateBindingState(BINDING_RESULT_NOK);

        CallListener(nPrevState, nPrevSubState, REASON_STATUS_CODE);
    }
}

PRIVATE VIRTUAL void Registration::Error_NotifyError(
        IN ISipConnection* piSc, IN IMS_SINT32 nCode, IN const AString& strMessage)
{
    (void)strMessage;

    IMS_TRACE_I("Registration::Error_NotifyError() - Code (%d), Message (%s)", nCode,
            strMessage.GetStr(), 0);

    if (piSc == IMS_NULL)
    {
        return;
    }

    piSc->Close();
    m_piOngoingScc = IMS_NULL;

    IMS_SINT32 nPrevState = GetState();
    IMS_SINT32 nPrevSubState = GetSubState();

    SetSubState(SUB_STATE_IDLE);

    if (nPrevSubState == SUB_STATE_DEREGISTERING)
    {
        // IMS_AUTH_NONCE_REUSE {
        SetAuthenticationChallenge(IMS_NULL);
        // }
        SetState(STATE_TERMINATED);
    }

    UpdateBindingState(BINDING_RESULT_NOK);

    if (nCode == SipError::TRANSACTION_TIMER_EXPIRED)
    {
        CallListener(nPrevState, nPrevSubState, REASON_TRANSACTION_TIMEOUT);
    }
    else if (nCode == SipError::TRANSPORT_ERROR)
    {
        CallListener(nPrevState, nPrevSubState, REASON_CLIENT_SOCKET_ERROR);
    }
    else
    {
        CallListener(nPrevState, nPrevSubState, REASON_INTERNAL_ERROR);
    }

    //// DEBUG
    if (nCode == SipError::TRANSACTION_TIMER_EXPIRED)
    {
        SipDebug::Send(GetSlotId(), SipDebug::MSG_RSP, SipDebug::DIR_IN,
                (nPrevSubState == SUB_STATE_REGISTERING) ? SipMethod::REGISTER
                                                         : SipDebug::M_DEREGISTER,
                0);
    }
}

PRIVATE VIRTUAL ISipMessage* Registration::GetNextRequest()
{
    if (m_piNextRequest == IMS_NULL)
    {
        m_piNextRequest = SipParsingHelper::CreateMessage(ISipMessage::TYPE_REQUEST);
    }

    return m_piNextRequest;
}

PRIVATE VIRTUAL void Registration::SetSipMessageMediator(IN IMessageMediator* piMediator)
{
    m_piMessageMediator = piMediator;

    if (m_pRefreshHelper != IMS_NULL)
    {
        m_pRefreshHelper->SetMessageMediator(m_piMessageMediator);
    }
}

PRIVATE VIRTUAL IMS_BOOL Registration::CreateBinding(
        IN const AString& strAppId, IN const AString& strServiceId)
{
    return RegBindingProxy::CreateBinding(GetSlotId(), strAppId, strServiceId, this);
}

PRIVATE VIRTUAL void Registration::DestroyBinding(
        IN const AString& strAppId, IN const AString& strServiceId)
{
    RegBindingProxy::DestroyBinding(GetSlotId(), strAppId, strServiceId);
}

PRIVATE VIRTUAL IRegContact* Registration::CreateContact(IN const IpAddress& objIpAddr,
        IN IMS_SINT32 nPort, IN IMS_SINT32 nExpiresPolicy /*= POLICY_EXPIRES_CONFIG*/,
        IN IMS_UINT32 nExpiresValue /*= DEFAULT_EXPIRES*/)
{
    // If the contact address is already present in the list,
    // then just adds the service capability to the contact.
    for (IMS_UINT32 i = 0; i < m_objContacts.GetSize(); ++i)
    {
        RegContact* pTmpContact = m_objContacts.GetAt(i);

        if (objIpAddr.Equals(pTmpContact->GetIpAddress()) && (nPort == pTmpContact->GetPort()))
        {
            IMS_TRACE_D("RegContact (%s, %d) already exists", SipDebug::GetIp(objIpAddr), nPort, 0);
            return pTmpContact;
        }
    }

    SipProfile* pProfile = m_pStateTracker->GetSipProfile();

    // If not present, add a new Contact information
    RegContact* pNewContact = new RegContact(
            GetSlotId(), objIpAddr, nPort, this, m_pRegFlow->GetRegKey().GetFlowId(), pProfile);

    if (pNewContact == IMS_NULL)
    {
        IMS_TRACE_E(0, "Creating a Contact (%s:%d) failed", SipDebug::GetIp(objIpAddr), nPort, 0);
        return IMS_NULL;
    }

    // Set user-info field
    pNewContact->SetAor(m_pStateTracker->GetAor());

    // Set "+sip.instance" parameter
    IMS_SINT32 nDeviceId = SipConfigProxy::GetDeviceId(GetSlotId(), pProfile);

    if (nDeviceId != ISipConfig::DEVICE_ID_NONE)
    {
        AString strDeviceId;
        AString strInstance;

        if (nDeviceId == ISipConfig::DEVICE_ID_PREDEFINED)
        {
            strDeviceId = SipConfigProxy::GetPredefinedDeviceId(GetSlotId(), pProfile);
        }
        else
        {
            strDeviceId = SipUrnHelper::GetUrn(GetSlotId(), nDeviceId);
        }

        strInstance.Append('"');
        strInstance.Append('<');
        strInstance.Append(strDeviceId);
        strInstance.Append('>');
        strInstance.Append('"');

        static_cast<IRegContact*>(pNewContact)
                ->AddHeaderParameter(Sip::STR_SIP_INSTANCE, strInstance);
    }

    // Set the regiration expiration value
    switch (nExpiresPolicy)
    {
        case IRegistration::POLICY_EXPIRES_CONFIG:
        {
            if (SipConfigProxy::IsRegExpiresConfigured(GetSlotId(), pProfile))
            {
                // Ignore the passed expiration (nExpiresValue)
                pNewContact->SetExpires(SipConfigProxy::GetRegExpires(GetSlotId(), pProfile));
            }
            break;
        }
        case IRegistration::POLICY_EXPIRES_APP:
            pNewContact->SetExpires(nExpiresValue);
            break;

        default:
            break;
    }

    IMS_TRACE_D("RegContact (%s, %d) added", SipDebug::GetIp(objIpAddr), nPort, 0);

    if (!m_objContacts.Append(pNewContact))
    {
        delete pNewContact;
        return IMS_NULL;
    }

    ChoosePreferredContact();

    return pNewContact;
}

PRIVATE VIRTUAL void Registration::DestroyAllContacts()
{
    while (!m_objContacts.IsEmpty())
    {
        RegContact* pContact = m_objContacts.GetAt(0);

        // Notify the removal of contact to the RegBinding ...
        UpdateBindingState(BINDING_DESTROY_CONTACT);

        delete pContact;
        m_objContacts.RemoveAt(0);
    }

    m_pStateTracker->SetPreferredContact(IMS_NULL);
}

PRIVATE VIRTUAL void Registration::DestroyContact(IN IRegContact* piContact)
{
    for (IMS_UINT32 i = 0; i < m_objContacts.GetSize(); ++i)
    {
        RegContact* pContact = m_objContacts.GetAt(i);

        if (pContact == piContact)
        {
            // Notify the removal of contact to the RegBinding ...
            UpdateBindingState(BINDING_DESTROY_CONTACT);

            delete pContact;
            m_objContacts.RemoveAt(i);
            break;
        }
    }

    ChoosePreferredContact();
}

PRIVATE VIRTUAL void Registration::DestroyContact(
        IN const IpAddress& objIpAddr, IN IMS_SINT32 nPort)
{
    for (IMS_UINT32 i = 0; i < m_objContacts.GetSize(); ++i)
    {
        RegContact* pContact = m_objContacts.GetAt(i);

        if (objIpAddr.Equals(pContact->GetIpAddress()) && (nPort == pContact->GetPort()))
        {
            // Notify the removal of contact to the RegBinding ...
            UpdateBindingState(BINDING_DESTROY_CONTACT);

            delete pContact;
            m_objContacts.RemoveAt(i);
            break;
        }
    }

    ChoosePreferredContact();
}

PRIVATE VIRTUAL ImsList<IRegContact*> Registration::GetAllContacts() const
{
    ImsList<IRegContact*> objAllContacts;

    for (IMS_UINT32 i = 0; i < m_objContacts.GetSize(); ++i)
    {
        RegContact* pContact = m_objContacts.GetAt(i);

        objAllContacts.Append(pContact);
    }

    return objAllContacts;
}

PRIVATE VIRTUAL IRegContact* Registration::GetContact(
        IN const IpAddress& objIpAddr, IN IMS_SINT32 nPort) const
{
    for (IMS_UINT32 i = 0; i < m_objContacts.GetSize(); ++i)
    {
        RegContact* pContact = m_objContacts.GetAt(i);

        if (objIpAddr.Equals(pContact->GetIpAddress()) && (nPort == pContact->GetPort()))
        {
            return pContact;
        }
    }

    return IMS_NULL;
}

PRIVATE VIRTUAL IMS_BOOL Registration::IsBindingsUpdated() const
{
    for (IMS_UINT32 i = 0; i < m_objContacts.GetSize(); ++i)
    {
        RegContact* pContact = m_objContacts.GetAt(i);

        if (pContact->IsBindingsUpdated())
        {
            IMS_TRACE_D("Registration bindings are updated", 0, 0, 0);
            return IMS_TRUE;
        }
    }

    return IMS_FALSE;
}

PRIVATE VIRTUAL IMS_RESULT Registration::Register(IN IMS_SINT32 nExpires /*= (-1)*/)
{
    IMS_SINT32 nCurrentState = GetState();
    IMS_SINT32 nCurrentSubState = GetSubState();

    if ((nCurrentState == STATE_INIT) || (nCurrentState == STATE_TERMINATED))
    {
        IMS_TRACE_E(0, "REGISTER request can't be sent in INIT or TERMINATED state", 0, 0, 0);
        return IMS_FAILURE;
    }

    if (nCurrentSubState != SUB_STATE_IDLE)
    {
        IMS_TRACE_E(
                0, "REGISTER request can't be sent when another transaction is ongoing", 0, 0, 0);
        return IMS_FAILURE;
    }

    if ((nCurrentState == STATE_CREATED) && IsAllBindingsRemoved())
    {
        IMS_TRACE_E(0, "There is no contacts or caller capabilities for the registration", 0, 0, 0);
        return IMS_FAILURE;
    }

    // Updates Call-ID
    if (nCurrentState == STATE_CREATED)
    {
        m_pRegFlow->UpdateCallId(m_pStateTracker->GetIpAddress());
    }

    ISipClientConnection* piScc = CreateConnection(this);

    if (piScc == IMS_NULL)
    {
        IMS_TRACE_E(0, "Instantiating a SIP connection failed", 0, 0, 0);
        return IMS_FAILURE;
    }

    piScc->SetListener(this);
    piScc->SetErrorListener(this);

    // Form REGISTER request
    ISipMessage* piSipMsg = piScc->GetMessage();

    // Copy the headers from the application
    if (m_piNextRequest != IMS_NULL)
    {
        // Set the headers and body parts if application already sets
        if (piSipMsg->CopyHeadersAndBodyParts(m_piNextRequest) != IMS_SUCCESS)
        {
            IMS_TRACE_E(0, "Copying SIP headers and message body parts failed", 0, 0, 0);
            goto EXIT_Register;
        }
    }

    // Form the generic headers
    if (SetHeaders(piScc) != IMS_SUCCESS)
    {
        IMS_TRACE_E(0, "Setting the generic headers failed", 0, 0, 0);
        goto EXIT_Register;
    }

    // Sets Contact headers
    if ((nExpires == 0) || IsAllBindingsRemoved())
    {
        if (SetContactNExpiresHeader(piSipMsg, 0) != IMS_SUCCESS)
        {
            IMS_TRACE_E(0, "Setting Contact & Expires header failed", 0, 0, 0);
            goto EXIT_Register;
        }

        SetSubState(SUB_STATE_DEREGISTERING);
    }
    else
    {
        if (nExpires != (-1))
        {
            // Updates the expires value for this registration (e.g. 423 response)
            for (IMS_UINT32 i = 0; i < m_objContacts.GetSize(); ++i)
            {
                RegContact* pContact = m_objContacts.GetAt(i);

                pContact->SetExpires(nExpires);
            }
        }

        if (SetContactNExpiresHeader(piSipMsg, nExpires) != IMS_SUCCESS)
        {
            IMS_TRACE_E(0, "Setting Contact & Expires header failed", 0, 0, 0);
            goto EXIT_Register;
        }

        SetSubState(SUB_STATE_REGISTERING);
    }

    // IMS_AUTH_NONCE_REUSE {
    if ((nCurrentState == STATE_CREATED) || (nCurrentState == STATE_ACTIVE))
    {
        // Set the credential information in case of re/de-registration
        SetNextAuthenticationInfo(piScc);
    }
    // }

    // SIP_MESSAGE_MEDIATOR
    (void)AdjustMessage(piScc->GetMessage(), IMessageMediator::MESSAGE_NORMAL);

    // Send a REGISTER request to the network
    if (piScc->Send() != IMS_SUCCESS)
    {
        IMS_TRACE_E(0, "Sending REGISTER request failed", 0, 0, 0);
        goto EXIT_Register;
    }

    // Update the refresh data
    m_pRefreshHelper->UpdateOnMessageSent(piScc);

    // Clear the next request
    ClearNextRequest();

    // Store the ongoing SIP client connection
    SetOngoingConnection(piScc);

    // Update the last request/response message
    SetPreviousRequest(piScc->GetMessage());
    SetPreviousResponse(IMS_NULL);

    // Change the state
    if (nCurrentState == STATE_CREATED)
    {
        SetState(STATE_INIT);
    }

    if (GetSubState() == SUB_STATE_REGISTERING)
    {
        UpdateBindingState(BINDING_REGISTERING);
    }
    else
    {
        UpdateBindingState(BINDING_DEREGISTERING);
    }

    Ims::SetLastError(ImsError::NO_ERROR);

    //// DEBUG
    SipDebug::Send(GetSlotId(), SipDebug::MSG_REQ, SipDebug::DIR_OUT,
            (GetSubState() == SUB_STATE_REGISTERING) ? SipMethod::REGISTER
                                                     : SipDebug::M_DEREGISTER);

    return IMS_SUCCESS;

EXIT_Register:
    SetSubState(SUB_STATE_IDLE);
    piScc->Close();

    return IMS_FAILURE;
}

PRIVATE VIRTUAL IMS_RESULT Registration::Deregister()
{
    if (GetState() != STATE_ACTIVE)
    {
        IMS_TRACE_E(
                0, "de-REGISTER request can't be sent in non-ACTIVE state (%d)", GetState(), 0, 0);
        return IMS_FAILURE;
    }

    if (GetSubState() != SUB_STATE_IDLE)
    {
        IMS_TRACE_E(0, "de-REGISTER request can't be sent when another transaction is ongoing", 0,
                0, 0);
        return IMS_FAILURE;
    }

    ISipClientConnection* piScc = CreateConnection(this);

    if (piScc == IMS_NULL)
    {
        IMS_TRACE_E(0, "Instantiating a SIP connection failed", 0, 0, 0);
        return IMS_FALSE;
    }

    piScc->SetListener(this);
    piScc->SetErrorListener(this);

    // Form REGISTER request
    ISipMessage* piSipMsg = piScc->GetMessage();

    // Copy the headers from the application
    if (m_piNextRequest != IMS_NULL)
    {
        // Set the headers and body parts if application already sets
        if (piSipMsg->CopyHeadersAndBodyParts(m_piNextRequest) != IMS_SUCCESS)
        {
            IMS_TRACE_E(0, "Copying SIP headers and message body parts failed", 0, 0, 0);
            goto EXIT_Deregister;
        }
    }

    if (SetHeaders(piScc) != IMS_SUCCESS)
    {
        IMS_TRACE_E(0, "Setting the generic headers failed", 0, 0, 0);
        goto EXIT_Deregister;
    }

    // Sets Contact headers
    if (SetContactNExpiresHeader(piSipMsg, 0) != IMS_SUCCESS)
    {
        IMS_TRACE_E(0, "Setting Contact & Expires header failed", 0, 0, 0);
        goto EXIT_Deregister;
    }

    SetSubState(SUB_STATE_DEREGISTERING);

    // IMS_AUTH_NONCE_REUSE {
    // Set the credential information in case of re/de-registration
    SetNextAuthenticationInfo(piScc);
    // }

    // SIP_MESSAGE_MEDIATOR
    (void)AdjustMessage(piScc->GetMessage(), IMessageMediator::MESSAGE_NORMAL);

    // Send a REGISTER request to the network
    if (piScc->Send() != IMS_SUCCESS)
    {
        IMS_TRACE_E(0, "Sending de-REGISTER request failed", 0, 0, 0);
        goto EXIT_Deregister;
    }

    // Update the refresh data
    m_pRefreshHelper->UpdateOnMessageSent(piScc);

    // Clear the next request
    ClearNextRequest();

    // Store the ongoing SIP client connection
    SetOngoingConnection(piScc);

    // Update the last request/response message
    SetPreviousRequest(piScc->GetMessage());
    SetPreviousResponse(IMS_NULL);

    // Change the state
    UpdateBindingState(BINDING_DEREGISTERING);

    Ims::SetLastError(ImsError::NO_ERROR);

    //// DEBUG
    SipDebug::Send(GetSlotId(), SipDebug::MSG_REQ, SipDebug::DIR_OUT, SipDebug::M_DEREGISTER);

    return IMS_SUCCESS;

EXIT_Deregister:
    SetSubState(SUB_STATE_IDLE);
    piScc->Close();

    return IMS_FAILURE;
}

// REG_RESTORATION_FOR_ACTIVE_BINDING
PRIVATE VIRTUAL void Registration::RemoveActiveBindingsForcingly()
{
    if (GetState() != STATE_ACTIVE)
    {
        IMS_TRACE_D("Registration :: No active bindings", 0, 0, 0);
        return;
    }

    if (GetSubState() != SUB_STATE_IDLE)
    {
        // FIXME: need to control the details

        SetSubState(SUB_STATE_IDLE);

        if (m_piOngoingScc != IMS_NULL)
        {
            m_piOngoingScc->Close();
            m_piOngoingScc = IMS_NULL;
        }

        ClearNextRequest();
        SetPreviousRequest(IMS_NULL);
        SetPreviousResponse(IMS_NULL);
    }

    UpdateBindingState(BINDING_RESTORE);
}

PRIVATE VIRTUAL void Registration::Restore()
{
    IMS_TRACE_I("Registration::Restore - State (%s)", StateToString(GetState()), 0, 0);

    // 4 contact handling/ connection if present
    //  Reset the refresh timer
    m_pRefreshHelper->UpdateRefreshTimer(0);

    SetState(STATE_CREATED);
    SetSubState(SUB_STATE_IDLE);
    UpdateBindingState(BINDING_RESTORE);

    // Remove all the bindings if present
    for (IMS_UINT32 i = 0; i < m_objContacts.GetSize(); ++i)
    {
        RegContact* pContact = m_objContacts.GetAt(i);

        pContact->Restore();

        // IMS_IPSEC_UDP_ENC
        if (m_pRegParam->IsSecurityAssociationRequiredViaUdpEnc())
        {
            pContact->SetHostInfo(pContact->GetIpAddress());
        }
    }

    if (m_piOngoingScc != IMS_NULL)
    {
        m_piOngoingScc->Close();
        m_piOngoingScc = IMS_NULL;
    }

    ClearNextRequest();
    SetPreviousRequest(IMS_NULL);
    SetPreviousResponse(IMS_NULL);

    m_pRegFlow->Restore();
    m_pRegParam->Restore();

    // IMS_AUTH_NONCE_REUSE {
    SetAuthenticationChallenge(IMS_NULL);
    // }

    // Abort the authentication procedure
    if (m_piDigestAka != IMS_NULL)
    {
        m_piDigestAka->Destroy();
        m_piDigestAka = IMS_NULL;
    }

    // NAT_REQ_UE_PUBLIC_IP
    m_bIsBehindNat = IMS_FALSE;
    m_pStateTracker->SetPublicIpAddress(IpAddress::NONE);

    if (NatHelper::IsNatResolverRequired())
    {
        NatHelper::GetInstance()->RemovePublicAddress(
                GetSlotId(), m_pRegFlow->GetRegKey().GetFlowId());
    }
}

// REG_RESTORATION_FOR_ACTIVE_BINDING
PRIVATE VIRTUAL IMS_RESULT Registration::RestoreActiveBindings()
{
    if (!m_bActiveBindingsRestorationEnabled)
    {
        IMS_TRACE_D("Registration :: Restoration(for active bindings) is disabled", 0, 0, 0);
        return IMS_FAILURE;
    }

    if (GetState() != STATE_ACTIVE)
    {
        IMS_TRACE_D("Registration :: No active bindings", 0, 0, 0);
        return IMS_FAILURE;
    }

    UpdateBindingState(BINDING_RESTORE_ACTIVE_BINDINGS);

    return IMS_SUCCESS;
}

PRIVATE VIRTUAL void Registration::SetAor(
        IN const SipAddress& objAor, IN const AString& strSubsId /*= AString::ConstNull()*/)
{
    m_pStateTracker->SetAor(objAor);
    m_pStateTracker->SetSubscriberId(strSubsId);

    // Update the profile
    m_pRegParam->UpdateProfile(objAor, strSubsId);

    // Updates the user-info field in all Contacts
    const SipAddress& objTmpAor = m_pStateTracker->GetAor();

    for (IMS_UINT32 i = 0; i < m_objContacts.GetSize(); ++i)
    {
        RegContact* pContact = m_objContacts.GetAt(i);

        pContact->SetAor(objTmpAor);
    }
}

PRIVATE VIRTUAL void Registration::SetRefreshPolicy(IN IMS_SINT32 nPolicy,
        IN IMS_SINT32 nCriteriaInterval, IN IMS_SINT32 nValueEorLt, IN IMS_SINT32 nValueGt)
{
    if (m_pRefreshHelper == IMS_NULL)
    {
        return;
    }

    switch (nPolicy)
    {
        case REFRESH_POLICY_NO_REFRESH:
            m_pRefreshHelper->SetPolicy(
                    RefreshHelper::POLICY_NO_REFRESH, nCriteriaInterval, nValueEorLt, nValueGt);
            break;
        case REFRESH_POLICY_SPEC:
            m_pRefreshHelper->SetPolicy(
                    RefreshHelper::POLICY_SPEC, nCriteriaInterval, nValueEorLt, nValueGt);
            break;
        case REFRESH_POLICY_REMAIN_TIME:
            m_pRefreshHelper->SetPolicy(
                    RefreshHelper::POLICY_REMAIN_TIME, nCriteriaInterval, nValueEorLt, nValueGt);
            break;
        case REFRESH_POLICY_RATIO:
            m_pRefreshHelper->SetPolicy(
                    RefreshHelper::POLICY_RATIO, nCriteriaInterval, nValueEorLt, nValueGt);
            break;
        default:
            IMS_TRACE_E(0, "Invalid refresh policy (%d)", nPolicy, 0, 0);
            break;
    }
}

PRIVATE VIRTUAL void Registration::SetSipProfile(IN SipProfile* pProfile)
{
    if (!m_pStateTracker.IsNull())
    {
        m_pStateTracker->SetSipProfile(pProfile);
    }

    if (m_pRegParam != IMS_NULL)
    {
        m_pRegParam->UpdateSipProfile(pProfile);
    }
}

PRIVATE VIRTUAL void Registration::SetFlagForWithinTrustDomain(IN IMS_BOOL bWithinTrustDomain)
{
    if (m_bIsWithinTrustDomain != bWithinTrustDomain)
    {
        IMS_TRACE_I("WithinTrustDomain:: %s > %s", _TRACE_B_(m_bIsWithinTrustDomain),
                _TRACE_B_(bWithinTrustDomain), 0);

        m_bIsWithinTrustDomain = bWithinTrustDomain;
    }
}

PRIVATE VIRTUAL void Registration::SetUserInfoForContactHeader(IN const AString& strUserInfo)
{
    if (GetState() != STATE_ACTIVE)
    {
        IMS_TRACE_E(0, "Invalid state (%s)", StateToString(GetState()), 0, 0);
        return;
    }

    if (!m_pStateTracker.IsNull())
    {
        m_pStateTracker->SetUserInfoForContactHeader(strUserInfo);
    }
}

PRIVATE VIRTUAL IRegSubscription* Registration::CreateSubscription(
        IN SipAddress* pResourceUri /*= IMS_NULL*/)
{
    if (GetState() != STATE_ACTIVE)
    {
        IMS_TRACE_E(0, "Invalid state (%s)", StateToString(GetState()), 0, 0);
        return IMS_NULL;
    }

    RegSubscription* pSubscription =
            new RegSubscription(m_pRegFlow->GetRegKey(), m_pStateTracker.Get(),
                    m_pRefreshHelper->GetDuration(), m_pRegParam->GetSipTimerValues());

    if (pSubscription == IMS_NULL)
    {
        return IMS_NULL;
    }

    SipAddress objResourceUri = GetAuthorizedAor();

    if (pResourceUri == IMS_NULL)
    {
        pResourceUri = &objResourceUri;
    }

    // Initialize the subscription
    if (!pSubscription->InitMethod(pResourceUri->ToString(), AString::ConstNull(), (*pResourceUri)))
    {
        pSubscription->DestroyEx();

        IMS_TRACE_E(0, "Initializing RegSubscription failed", 0, 0, 0);
        return IMS_NULL;
    }

    //// Register the listener to obtain the reginfo ...
    RegInfo* pRegInfo =
            ImsCoreContext::GetInstance()->GetRegInfoManager()->GetRegInfo(m_pRegFlow->GetRegKey());

    if (pRegInfo != IMS_NULL)
    {
        pRegInfo->AddListener(this);
    }

    return pSubscription;
}

PRIVATE VIRTUAL void Registration::ConnectionNotifierError_NotifyError(
        IN ISipConnectionNotifier* piScn, IN IMS_SINT32 nCode, IN const AString& strMessage)
{
    (void)piScn;

    IMS_TRACE_D("ConnectionNotifierError_NotifyError :: code=%d, message=%s", nCode,
            strMessage.GetStr(), 0);

    if (GetState() != STATE_ACTIVE)
    {
        IMS_TRACE_D("Registration :: State is not in the ACTIVE; ignored ...", 0, 0, 0);
        return;
    }

    // REG_RESTORATION_FOR_ACTIVE_BINDING
    if (m_bActiveBindingsRestorationEnabled)
    {
        NetworkService* pNetworkService = NetworkService::GetNetworkService();
        INetworkConnection* piConnection =
                pNetworkService->FindConnection(m_pStateTracker->GetIpAddress());

        // Checks if the data connection is lost or not...
        // If the loss of data connection detects,
        // the error doesn't need to notify the application.
        if ((piConnection == IMS_NULL) ||
                (piConnection->GetState() != INetworkConnection::STATE_CONNECTED))
        {
            IMS_TRACE_D("Registration :: Restoration(for active bindings) is enabled", 0, 0, 0);
            return;
        }
    }

    if (nCode == ISipConnectionNotifier::TRANSPORT_ERROR_TCP_CLIENT)
    {
        PostMessage(AMSG_REGISTRATION_TERMINATED, REASON_CLIENT_SOCKET_ERROR, 0);
    }
    else if (nCode == ISipConnectionNotifier::TRANSPORT_ERROR_TCP_SERVER ||
            nCode == ISipConnectionNotifier::TRANSPORT_ERROR_UDP_SERVER)
    {
        PostMessage(AMSG_REGISTRATION_TERMINATED, REASON_SERVER_SOCKET_ERROR, 0);
    }
}

PRIVATE VIRTUAL void Registration::AddObserver(IN RegObserver* pObserver)
{
    for (IMS_UINT32 i = 0; i < m_objObservers.GetSize(); ++i)
    {
        RegObserver* pTmpObserver = m_objObservers.GetAt(i);

        if (pObserver == pTmpObserver)
        {
            return;
        }
    }

    m_objObservers.Append(pObserver);
}

PRIVATE VIRTUAL void Registration::RemoveObserver(IN RegObserver* pObserver)
{
    for (IMS_UINT32 i = 0; i < m_objObservers.GetSize(); ++i)
    {
        RegObserver* pTmpObserver = m_objObservers.GetAt(i);

        if (pObserver == pTmpObserver)
        {
            m_objObservers.RemoveAt(i);
            return;
        }
    }
}

PRIVATE VIRTUAL IMS_SINT32 Registration::AddReferenceForScnErrorListener()
{
#ifdef __IMS_DEBUG__
    IMS_TRACE_D("Registration :: SCNEL (add) - %d >> %d", m_nRefCountForScnErrorListener,
            (m_nRefCountForScnErrorListener + 1), 0);
#endif

    ++m_nRefCountForScnErrorListener;

    return m_nRefCountForScnErrorListener;
}

PRIVATE VIRTUAL IMS_SINT32 Registration::RemoveReferenceForScnErrorListener()
{
    if (m_nRefCountForScnErrorListener > 0)
    {
#ifdef __IMS_DEBUG__
        IMS_TRACE_D("Registration :: SCNEL (remove) - %d >> %d", m_nRefCountForScnErrorListener,
                (m_nRefCountForScnErrorListener - 1), 0);
#endif

        --m_nRefCountForScnErrorListener;

        if (m_nRefCountForScnErrorListener == 0)
        {
            IMS_TRACE_D("Registration :: SCNEL (0)", 0, 0, 0);
        }
    }

    return m_nRefCountForScnErrorListener;
}

PRIVATE VIRTUAL void Registration::NotifyCallerCapabilityChanged()
{
    PostMessage(AMSG_REGISTRATION_BINDING_STATE_CHANGED, BINDING_STATE_CALLER_CAPABILITY, 0);
}

PRIVATE VIRTUAL void Registration::Refreshable_RefreshCompleted(
        IN ISipClientConnection* piScc, IN IMS_SINT32 nCode /*= 0*/)
{
    IMS_SINT32 nStatusCode = piScc->GetStatusCode();

    IMS_TRACE_I("_____ REGISTRATION REFRESH COMPLETED ... Refresh Code (%d, %d)", nCode,
            nStatusCode, 0);

    if (SipStatusCode::Is1XX(nStatusCode))
    {
        return;
    }

    IMS_SINT32 nPrevState = GetState();
    IMS_SINT32 nPrevSubState = GetSubState();

    // In case, the session refresh request is successfully done.
    if (nCode == 0)
    {
        //// DEBUG
        SipDebug::Send(
                GetSlotId(), SipDebug::MSG_RSP, SipDebug::DIR_IN, SipMethod::REGISTER, nStatusCode);

        SetPreviousResponse(piScc->GetMessage());

        // Authentication challenged by S-CSCF
        if ((nStatusCode == SipStatusCode::SC_401) || (nStatusCode == SipStatusCode::SC_407))
        {
            // IMS_AUTH_NONCE_REUSE {
            SetAuthenticationChallenge(IMS_NULL);
            // }

            // Update the Security-xxx headers
            m_pRegParam->UpdateSecurityHeaders(piScc->GetMessage());

            // Respond to the authentication challenge
            if (!RespondToChallenge(piScc))
            {
                RestoreSecurityHeaders();
            }

            if (GetSubState() == SUB_STATE_IDLE)
            {
                m_pRefreshHelper->AbortConnection();
            }

            // 3 Do not close SipConnection in here
            return;
        }

        // Reset the flags after the transaction completed
        SetSubState(SUB_STATE_IDLE);

        ISipMessage* piSipMsg = GetPreviousResponse();

        if (SipStatusCode::IsFinalSuccess(nStatusCode))
        {
            // Update the Security-xxx headers
            m_pRegParam->UpdateSecurityHeaders(piSipMsg);

            // Update the sequence number
            UpdateCSeqNumber(piSipMsg);

            // Update the bindings
            IMS_RESULT nBindingUpdateResult = UpdateBindings(piSipMsg);
            IMS_SINT32 nReason = REASON_NONE;

            if (nBindingUpdateResult == BINDING_UPDATE_NO_EXPIRES)
            {
                nReason = REASON_NO_EXPIRES;
            }

            // Update the refresh timer
            UpdateRefreshTimer();

            // Check the UA location
            CheckUaLocation(piSipMsg);

            // Store the persistent header information
            StorePersistentHeaders(piSipMsg);

            // IMS_AUTH_NONCE_REUSE {
            SetNextNonce(piSipMsg);
            // }

            // Go to "ACTIVE" state
            if (HasActiveBindings())
            {
                SetState(STATE_ACTIVE);
            }
            else
            {
                nReason = REASON_NO_ACTIVE_BINDINGS;

                // IMS_AUTH_NONCE_REUSE {
                SetAuthenticationChallenge(IMS_NULL);
                // }
                SetState(STATE_TERMINATED);
            }

            if (nBindingUpdateResult == BINDING_UPDATE_OK)
            {
                UpdateBindingState(BINDING_RESULT_OK);
            }
            else
            {
                UpdateBindingState(BINDING_RESULT_NOK);
            }

            CallListener(nPrevState, nPrevSubState, nReason);
        }
        else
        {
            // Do not clear the authentication challenge...
            // Initial registration required: 305 / 408 / 500 / 504 / txn timeout
            // // IMS_AUTH_NONCE_REUSE {
            // SetAuthenticationChallenge(IMS_NULL);
            // // }

            // Do not check the status code to support re-use of Call-ID & CSeq number
            // when the registration is failed
            // if (nStatusCode == SipStatusCode::SC_423)
            {
                // Update the sequence number
                UpdateCSeqNumber(piSipMsg);
            }

            UpdateBindingState(BINDING_RESULT_NOK);
            CallListener(nPrevState, nPrevSubState, REASON_STATUS_CODE);
        }
    }
    // The session refresh request is timed out.
    else if (nCode == RefreshHelper::TRANSACTION_TIMEOUT)
    {
        // 140225, hwangoo.park
        // Do not clear the authentication challenge...
        // // IMS_AUTH_NONCE_REUSE {
        // SetAuthenticationChallenge(IMS_NULL);
        // // }

        // Reset the flags after the transaction completed
        SetSubState(SUB_STATE_IDLE);

        UpdateBindingState(BINDING_RESULT_NOK);
        CallListener(nPrevState, nPrevSubState, REASON_TRANSACTION_TIMEOUT);

        //// DEBUG
        SipDebug::Send(GetSlotId(), SipDebug::MSG_RSP, SipDebug::DIR_IN, SipMethod::REGISTER, 0);
    }
    else
    {
        // Reset the flags after the transaction completed
        SetSubState(SUB_STATE_IDLE);

        UpdateBindingState(BINDING_RESULT_NOK);

        if (nCode == SipError::TRANSPORT_ERROR)
        {
            CallListener(nPrevState, nPrevSubState, REASON_CLIENT_SOCKET_ERROR);
        }
        else
        {
            CallListener(nPrevState, nPrevSubState, REASON_INTERNAL_ERROR);
        }

        //// DEBUG
        SipDebug::Send(GetSlotId(), SipDebug::MSG_RSP, SipDebug::DIR_IN, SipMethod::REGISTER, -1);
    }
}

PRIVATE VIRTUAL IMS_BOOL Registration::Refreshable_RefreshStarted()
{
    IMS_BOOL bDoImplicitRefresh = IMS_TRUE;

    IMS_TRACE_I("_____ REGISTRATION REFRESH STARTED ... State(%d)", m_nState, 0, 0);

    if (IsBindingsUpdating())
    {
        IMS_TRACE_D("Registration has been already requested by a service ...", 0, 0, 0);
        return IMS_TRUE;
    }

    if (m_piListener != IMS_NULL)
    {
        m_piListener->Registration_RefreshTimerExpired(bDoImplicitRefresh);
    }

    if (bDoImplicitRefresh)
    {
        return RegisterOnImplicitRefresh();
    }

    return IMS_TRUE;
}

PRIVATE VIRTUAL void Registration::Refreshable_RefreshTerminated()
{
    IMS_TRACE_I("_____ REGISTRATION REFRESH TERMINATED ...", 0, 0, 0);

    // IMS_AUTH_NONCE_REUSE {
    SetAuthenticationChallenge(IMS_NULL);
    // }

    SetState(STATE_TERMINATED);
    PostMessage(AMSG_REGISTRATION_TERMINATED, REASON_REFRESH_TIMEOUT, 0);
}

PRIVATE VIRTUAL void Registration::RegCapabilityChange_ServiceAdded(
        IN const AString& strAppId, IN const AString& strServiceId)
{
    AStringArray objHeaders;
    RegBindingProxy::QueryRegistrationHeaders(GetSlotId(), strAppId, strServiceId, objHeaders);

    // Adds extra headers
    if (!objHeaders.IsEmpty())
    {
        IRegParameter* piRegParameter = m_pRegParam;

        piRegParameter->AddExtraHeaders(objHeaders);
    }
}

PRIVATE VIRTUAL void Registration::RegCapabilityChange_ServiceRemoved(
        IN const AString& strAppId, IN const AString& strServiceId)
{
    AStringArray objHeaders;
    RegBindingProxy::QueryRegistrationHeaders(GetSlotId(), strAppId, strServiceId, objHeaders);

    // Removes extra headers
    if (!objHeaders.IsEmpty())
    {
        IRegParameter* piRegParameter = m_pRegParam;

        piRegParameter->RemoveExtraHeaders(objHeaders);
    }
}

PRIVATE VIRTUAL void Registration::RegInfo_Updated(IN IMS_BOOL bStale /*= IMS_FALSE*/)
{
    (void)bStale;

    if (GetState() != STATE_ACTIVE)
    {
        return;
    }

    // Check the 'state' & 'event' reg info.
    RegInfo* pRegInfo =
            ImsCoreContext::GetInstance()->GetRegInfoManager()->GetRegInfo(m_pRegFlow->GetRegKey());
    IRegInfoRegistration* piRegInfoReg = IMS_NULL;

    if (pRegInfo == IMS_NULL)
    {
        return;
    }

    IMS_TRACE_D("RegInfo :: Updated", 0, 0, 0);

    const SipAddress& objAor = m_pStateTracker->GetAuthorizedAor();

    piRegInfoReg = pRegInfo->GetRegistration(objAor);

    if (piRegInfoReg == IMS_NULL)
    {
        IMS_TRACE_E(0, "Cannot find the RegInfo (%s)",
                SipDebug::GetCharA1(objAor.GetUser().GetStr(), 5), 0, 0);
        return;
    }

    IMS_BOOL bUpdateRefreshTimer = IMS_FALSE;
    IMS_UINT32 nShortenedExpires = 0;
    ImsList<IRegInfoContact*> objRegInfoContacts = piRegInfoReg->GetContacts();

    for (IMS_UINT32 i = 0; i < objRegInfoContacts.GetSize(); ++i)
    {
        const IRegInfoContact* piRegInfoContact = objRegInfoContacts.GetAt(i);

        if (piRegInfoContact == IMS_NULL)
        {
            continue;
        }

        IMS_SINT32 nState = piRegInfoContact->GetState();
        IMS_SINT32 nEvent = piRegInfoContact->GetEvent();
        IMS_UINT32 nExpires = piRegInfoContact->GetExpiresValue();

        if ((nState == IRegInfoContact::STATE_ACTIVE) &&
                (nEvent == IRegInfoContact::EVENT_SHORTENED) && (nExpires != 0))
        {
            for (IMS_UINT32 j = 0; j < m_objContacts.GetSize(); ++j)
            {
                RegContact* pContact = m_objContacts.GetAt(j);

                if (pContact == IMS_NULL)
                {
                    continue;
                }

                if (pContact->GetContactAddress().Equals(piRegInfoContact->GetUri()))
                {
                    bUpdateRefreshTimer = IMS_TRUE;

                    if (nShortenedExpires == 0 || nShortenedExpires > nExpires)
                    {
                        nShortenedExpires = nExpires;
                    }
                }
            }
        }
    }

    if (bUpdateRefreshTimer)
    {
        IMS_TRACE_D("RegInfo :: Active & Shortened (%d)", nShortenedExpires, 0, 0);
        m_pRefreshHelper->UpdateRefreshTimer(static_cast<IMS_SINT32>(nShortenedExpires));
    }
}

PRIVATE VIRTUAL void Registration::DigestAka_OnResponse(IN const ByteArray& objRes,
        IN const ByteArray& objIk /*= ByteArray::ConstNull()*/,
        IN const ByteArray& objCk /*= ByteArray::ConstNull()*/)
{
    IMS_TRACE_I("Registration :: AKA RES received", 0, 0, 0);

    if (GetSubState() == SUB_STATE_IDLE)
    {
        IMS_TRACE_E(0, "Invalid sub state (%d)", GetSubState(), 0, 0);
        return;
    }

    m_objActiveCredential = m_pRegParam->GetCredential();
    m_objActiveSaKey.SetIk(objIk);
    m_objActiveSaKey.SetCk(objCk);

    IMS_SINT32 nType = m_objActiveCredential.GetType();

    if (nType == Credential::TYPE_AKAv1_MD5)
    {
        m_objActiveCredential.SetAkaResponse(ImsAkaParam::RESULT_OK, objRes);
    }
    else if (nType == Credential::TYPE_AKAv2_MD5)
    {
        m_objActiveCredential.SetAkaResponse(ImsAkaParam::RESULT_OK, objRes, objIk, objCk);
    }

    PostMessage(AMSG_REGISTRATION_AKA_RESPONSE_RECEIVED, 0, 0);
}

PRIVATE VIRTUAL void Registration::DigestAka_OnAutsFailed(IN const ByteArray& objAuts)
{
    IMS_TRACE_I("Registration :: AUTS failed", 0, 0, 0);

    if (GetSubState() == SUB_STATE_IDLE)
    {
        IMS_TRACE_E(0, "Invalid sub state (%d)", GetSubState(), 0, 0);
        return;
    }

    m_objActiveCredential = m_pRegParam->GetCredential();
    m_objActiveSaKey.SetIk(ByteArray::ConstNull());
    m_objActiveSaKey.SetCk(ByteArray::ConstNull());

    IMS_SINT32 nType = m_objActiveCredential.GetType();

    if (nType == Credential::TYPE_AKAv1_MD5)
    {
        m_objActiveCredential.SetAkaResponse(
                ImsAkaParam::RESULT_NOK_SQN_SYNC_FAILED, ByteArray::ConstNull(), objAuts);
    }
    else if (nType == Credential::TYPE_AKAv2_MD5)
    {
        m_objActiveCredential.SetAkaResponse(ImsAkaParam::RESULT_NOK_SQN_SYNC_FAILED,
                ByteArray::ConstNull(), ByteArray::ConstNull(), ByteArray::ConstNull(), objAuts);
    }

    PostMessage(AMSG_REGISTRATION_AKA_RESPONSE_RECEIVED, 0, 0);
}

PRIVATE VIRTUAL void Registration::DigestAka_OnMacFailed()
{
    IMS_TRACE_I("Registration :: MAC failed", 0, 0, 0);

    if (GetSubState() == SUB_STATE_IDLE)
    {
        IMS_TRACE_E(0, "Invalid sub state (%d)", GetSubState(), 0, 0);
        return;
    }

    m_objActiveCredential = m_pRegParam->GetCredential();
    m_objActiveSaKey.SetIk(ByteArray::ConstNull());
    m_objActiveSaKey.SetCk(ByteArray::ConstNull());

    IMS_SINT32 nType = m_objActiveCredential.GetType();

    if (nType == Credential::TYPE_AKAv1_MD5)
    {
        m_objActiveCredential.SetAkaResponse(
                ImsAkaParam::RESULT_NOK_MAC_INVALID, ByteArray::ConstNull());
    }
    else if (nType == Credential::TYPE_AKAv2_MD5)
    {
        m_objActiveCredential.SetAkaResponse(ImsAkaParam::RESULT_NOK_MAC_INVALID,
                ByteArray::ConstNull(), ByteArray::ConstNull(), ByteArray::ConstNull());
    }

    PostMessage(AMSG_REGISTRATION_AKA_RESPONSE_RECEIVED, 0, 0);
}

PRIVATE
IMS_RESULT Registration::AdjustMessage(
        IN_OUT ISipMessage* piSipMsg, IN IMS_SINT32 nMessage /*= IMessageMediator::MESSAGE_NORMAL*/)
{
    if (m_piMessageMediator == IMS_NULL)
    {
        return IMS_FAILURE;
    }

    return m_piMessageMediator->MessageMediator_AdjustMessage(piSipMsg, nMessage);
}

PRIVATE
void Registration::CallListener(
        IN IMS_SINT32 nPrevState, IN IMS_SINT32 nPrevSubState, IN IMS_SINT32 nReason)
{
    if (m_piListener == IMS_NULL)
    {
        IMS_TRACE_E(0, "NO LISTENER REGISTERED", 0, 0, 0);
        return;
    }

    switch (nPrevState)
    {
        case STATE_INIT:
            if (nPrevSubState == SUB_STATE_REGISTERING)
            {
                if (nReason == REASON_NONE)
                {
                    PostMessage(AMSG_REGISTRATION_STARTED, nReason, 0);
                }
                else
                {
                    PostMessage(AMSG_REGISTRATION_START_FAILED, nReason, 0);
                }
            }
            break;
        case STATE_ACTIVE:
            if ((nPrevSubState == SUB_STATE_REGISTERING) || (nPrevSubState == SUB_STATE_REFRESHING))
            {
                if (nReason == REASON_NONE)
                {
                    PostMessage(AMSG_REGISTRATION_UPDATED, nReason, 0);
                }
                else
                {
                    PostMessage(AMSG_REGISTRATION_UPDATE_FAILED, nReason, 0);
                }
            }
            else if (nPrevSubState == SUB_STATE_DEREGISTERING)
            {
                PostMessage(AMSG_REGISTRATION_REMOVED, 0, 0);
            }
            break;
        case STATE_TERMINATED:  // FALL-THROUGH
        default:
            break;
    }
}

PRIVATE
void Registration::CheckUaLocation(IN ISipMessage* piSipMsg)
{
    if (piSipMsg == IMS_NULL)
    {
        return;
    }

    AString strViaHdr = piSipMsg->GetHeader(ISipHeader::VIA);
    ISipHeader* piHeader = SipParsingHelper::CreateHeader(ISipHeader::VIA, strViaHdr);

    if (piHeader == IMS_NULL)
    {
        return;
    }

    // Check if the 'received' parameter is present or not
    const SipParameter* pParameter = piHeader->GetParameter(Sip::STR_RECEIVED);

    if (pParameter == IMS_NULL)
    {
        IMS_TRACE_D("The 'received' parameter does not exist", 0, 0, 0);

        piHeader->Destroy();

        // NAT_REQ_UE_PUBLIC_IP
        // FIXME: Is it required to check "sent-by"?
        if (m_bIsBehindNat && m_pRegParam->IsSecurityAssociationPresent())
        {
            // Don't clear the flag.
        }
        else
        {
            m_bIsBehindNat = IMS_FALSE;
        }
        return;
    }

    // SIP/2.0/UDP sent-by(host+port)
    const AString& strBody = piHeader->GetValue();
    IMS_SINT32 nIndex = strBody.GetIndexOf(' ');

    if (nIndex < 0)
    {
        IMS_TRACE_E(0, "There is no space character in Via header (%s)",
                SipDebug::GetCharA1(strBody.GetStr(), 17), 0, 0);

        piHeader->Destroy();

        // NAT_REQ_UE_PUBLIC_IP
        // FIXME: Is it required to check "sent-by"?
        if (m_bIsBehindNat && m_pRegParam->IsSecurityAssociationPresent())
        {
            // Don't clear the flag.
        }
        else
        {
            m_bIsBehindNat = IMS_FALSE;
        }
        return;
    }

    AString strHostNPort = strBody.GetSubStr(nIndex + 1);
    AString strHost;

    // We need to take care of '[', ']' while extracting the host from IPv6 reference
    if ((nIndex = strHostNPort.GetIndexOf(TextParser::CHAR_LSBRACKET)) != AString::NPOS)
    {
        // Strip the '[' , ']' before resolving the address.
        IMS_SINT32 nEndOfHost = strHostNPort.GetIndexOf(TextParser::CHAR_RSBRACKET);

        strHost = strHostNPort.GetSubStr(nIndex + 1, nEndOfHost - nIndex - 1);
    }
    else
    {
        nIndex = strHostNPort.GetIndexOf(TextParser::CHAR_COLON);

        if (nIndex != AString::NPOS)
        {
            strHost = strHostNPort.GetSubStr(0, nIndex);
        }
        else
        {
            strHost = strHostNPort;
        }
    }

    // Compares the sent-by address & the address of 'received' parameter
    IpAddress objSentBy(strHost);
    IpAddress objReceived(pParameter->GetValue());
    IMS_BOOL bIsPublicIpUpdateRequired = IMS_TRUE;
    IMS_BOOL bIsSentByPublicIp = IMS_FALSE;

    if (!IpAddress::NONE.Equals(m_pStateTracker->GetPublicIpAddress()) &&
            objSentBy.Equals(m_pStateTracker->GetPublicIpAddress()))
    {
        bIsSentByPublicIp = IMS_TRUE;
    }

    // NAT_REQ_UE_PUBLIC_IP
    if (m_bIsBehindNat && m_pRegParam->IsSecurityAssociationPresent() && bIsSentByPublicIp)
    {
        bIsPublicIpUpdateRequired = IMS_FALSE;
    }
    else
    {
        if ((objSentBy.IsIPv6Address() && objReceived.IsIPv6Address()) ||
                (objSentBy.IsIPv4Address() && objReceived.IsIPv4Address()))
        {
            m_bIsBehindNat = !objSentBy.Equals(objReceived);
        }
        else
        {
            m_bIsBehindNat = !strHost.Equals(pParameter->GetValue());
        }
    }

    piHeader->Destroy();

    IMS_TRACE_D("UA is %s behind on a NAT", (m_bIsBehindNat) ? "located" : "not located", 0, 0);

    // NAT_REQ_UE_PUBLIC_IP
    if (bIsPublicIpUpdateRequired)
    {
        if (m_bIsBehindNat)
        {
            m_pStateTracker->SetPublicIpAddress(objReceived);
        }
        else
        {
            m_pStateTracker->SetPublicIpAddress(IpAddress::NONE);
        }

        if (NatHelper::IsNatResolverRequired())
        {
            NatHelper::GetInstance()->SetPublicAddress(GetSlotId(),
                    m_pRegFlow->GetRegKey().GetFlowId(), objSentBy,
                    (m_bIsBehindNat ? objReceived : IpAddress::NONE));
        }
    }
}

PRIVATE
void Registration::ChoosePreferredContact()
{
    RegContact* pPreferredContact = IMS_NULL;

    for (IMS_UINT32 i = 0; i < m_objContacts.GetSize(); ++i)
    {
        RegContact* pContact = m_objContacts.GetAt(i);

        if (pPreferredContact == IMS_NULL)
        {
            pPreferredContact = pContact;
        }
        else
        {
            const AString& strNew = pContact->GetPreference();
            const AString& strOld = pPreferredContact->GetPreference();

            if (AString::Compare(strNew.GetStr(), strOld.GetStr()) > 0)
            {
                pPreferredContact = pContact;
            }
        }
    }

    m_pStateTracker->SetPreferredContact(pPreferredContact);
}

PRIVATE
void Registration::ClearNextRequest()
{
    if (m_piNextRequest != IMS_NULL)
    {
        m_piNextRequest->Destroy();
        m_piNextRequest = IMS_NULL;
    }
}

PRIVATE
IDigestAka* Registration::CreateDigestAka(IN const SubscriberConfig* pSubsConfig)
{
    if (pSubsConfig == IMS_NULL)
    {
        return IMS_NULL;
    }

    PhoneInfoService* pPhoneInfoService = PhoneInfoService::GetPhoneInfoService();

    if (pSubsConfig->IsIsimSupported())
    {
        IIsim* piIsim = pPhoneInfoService->GetIsim(GetSlotId());

        if (piIsim != IMS_NULL)
        {
            return piIsim->CreateDigestAka();
        }
    }
    else
    {
        IUsim* piUsim = pPhoneInfoService->GetUsim(GetSlotId());

        if (piUsim != IMS_NULL)
        {
            return piUsim->CreateDigestAka();
        }
    }

    return IMS_NULL;
}

PRIVATE
IMS_BOOL Registration::CreateSa(IN const Credential& objCredential, IN const ImsSaKey& objSaKey)
{
    IMS_SINT32 nAlgorithm = objCredential.GetType();

    if ((nAlgorithm != Credential::TYPE_AKAv1_MD5) && (nAlgorithm != Credential::TYPE_AKAv2_MD5))
    {
        IMS_TRACE_D("Authentication algorithm(%d) is not AKA", nAlgorithm, 0, 0);
        return IMS_TRUE;
    }

    // Checks if the security association is required or not...
    if (!m_pRegParam->IsSecurityAssociationRequired())
    {
        IMS_TRACE_D("Security association is not required ...", 0, 0, 0);
        return IMS_TRUE;
    }

    if (m_piListener == IMS_NULL)
    {
        IMS_TRACE_D("Security association is not required ... ; no listener", 0, 0, 0);
        return IMS_TRUE;
    }

    IMS_BOOL bResultOfSa = IMS_TRUE;

    if (SipConfigProxy::IsIpSecConfigured(GetSlotId(), m_pStateTracker->GetSipProfile()))
    {
        ImsList<SipSecurityHeader> objSecurityVerifys;

        m_pRegParam->SetSecurityVerifys(objSecurityVerifys);

        // Notifies the information which IK & CK will be used for this authentication.
        m_piListener->Registration_NotifyAkaResponse(objCredential.GetAkaResponse().m_nStatus,
                objSaKey.GetIk(), objSaKey.GetCk(), bResultOfSa);

        // If the result of AKA is not OK, clear the Security-Server headers
        if (!bResultOfSa || (objCredential.GetAkaResponse().m_nStatus != ImsAkaParam::RESULT_OK))
        {
            RestoreSecurityHeaders();
        }

        // Updates the transport extension rules for SIP layer
        m_pRegParam->SetTransportExtForIpSec();
    }
    else
    {
        // Notifies the information which IK & CK will be used for this authentication.
        m_piListener->Registration_NotifyAkaResponse(objCredential.GetAkaResponse().m_nStatus,
                objSaKey.GetIk(), objSaKey.GetCk(), bResultOfSa);
    }

    return bResultOfSa;
}

PRIVATE
IMS_SINT32 Registration::GetPortUc() const
{
    IMS_SINT32 nPortUc = m_pRegParam->GetProtectedPortUc();

    // Check if the UE supports the Security Association Agreement.
    // If nPortUC returns Sip::PORT_UNSPECIFIED, then it does not support "sec-agree".
    if (nPortUc == Sip::PORT_UNSPECIFIED)
    {
        IMS_SINT32 nState = GetState();
        IMS_SINT32 nPrevState = GetSubState();

        if ((nState == STATE_CREATED) || ((nState == STATE_INIT) && (nPrevState != SUB_STATE_IDLE)))
        {
            return (m_pRegParam != IMS_NULL) ? m_pRegParam->GetPort() : Sip::PORT_5060;
        }

        return GetPortUs();
    }
    else if (nPortUc == 0)
    {
        if (!SipConfigProxy::IsIpSecConfigured(GetSlotId(), m_pStateTracker->GetSipProfile()))
        {
            return GetPortUs();
        }
    }

    return nPortUc;
}

PRIVATE
IMS_SINT32 Registration::GetPortUs() const
{
    const RegContact* pContact = m_pStateTracker->GetPreferredContact();

    if (pContact == IMS_NULL)
    {
        if (m_pRegParam != IMS_NULL)
        {
            return m_pRegParam->GetPort();
        }

        return Sip::PORT_5060;
    }

    return pContact->GetPort();
}

PRIVATE
const SubscriberConfig* Registration::GetSubsConfig() const
{
    const SubscriberConfig* pSubsConfig = IMS_NULL;
    const AString& strSubsId = m_pStateTracker->GetSubscriberId();
    ConfigurationManager* pConfigMngr = ConfigurationManager::GetInstance();

    if (strSubsId.GetLength() > 0)
    {
        pSubsConfig = pConfigMngr->GetSubscriberConfig(strSubsId, GetSlotId());
    }
    else
    {
        const SipAddress& objAor = m_pStateTracker->GetAor();
        const AString& strId =
                SubscriberTracker::GetInstance()->GetSubscriberId(GetSlotId(), &objAor);
        pSubsConfig = pConfigMngr->GetSubscriberConfig(strId, GetSlotId());
    }

    return pSubsConfig;
}

PRIVATE
IMS_BOOL Registration::IsAkaSupported(IN const SubscriberConfig* pSubsConfig) const
{
    if (pSubsConfig == IMS_NULL)
    {
        return IMS_FALSE;
    }

    if (pSubsConfig->IsIsimSupported())
    {
        return IMS_TRUE;
    }
    else
    {
        IUsim* piUsim = PhoneInfoService::GetPhoneInfoService()->GetUsim(GetSlotId());

        if (piUsim != IMS_NULL)
        {
            const Credential& objCredential = pSubsConfig->GetCredential();

            if ((objCredential.GetType() == Credential::TYPE_AKAv1_MD5) ||
                    (objCredential.GetType() == Credential::TYPE_AKAv2_MD5))
            {
                return IMS_TRUE;
            }
        }
    }

    return IMS_FALSE;
}

PRIVATE
IMS_BOOL Registration::IsFlowControlRequired() const
{
    if (m_pRegParam->GetFlowControlOption() == IRegParameter::FLOW_CONTROL_BY_PROVISION)
    {
        return IMS_TRUE;
    }

    // - Multiple registration supports
    // - "+sip.instance" & "reg-id" header field in Contact header
    // - "outbound" option tag in Supported header
    // - "outbound" option tag in Require header in 200 OK (to REGISTER)
    // If all the conditions are true, the provisioned flow control port is selected.
    // Otherwise, Sip::PORT_UNSPECIFIED is selected.
    ISipMessage* piRequest = GetPreviousRequest();
    ISipMessage* piResponse = GetPreviousResponse();
    const RegContact* pRegContact = m_pStateTracker->GetPreferredContact();

    if (SipConfigProxy::IsMultipleRegConfigured(GetSlotId(), m_pStateTracker->GetSipProfile()) &&
            (piRequest != IMS_NULL) && (piResponse != IMS_NULL) && (pRegContact != IMS_NULL))
    {
        const AString strOutbound("outbound");

        if ((pRegContact->GetInstanceParameter() != IMS_NULL) &&
                (pRegContact->GetRegIdParameter() != IMS_NULL) &&
                piRequest->IsOptionSupported(strOutbound) &&
                piResponse->IsOptionRequired(strOutbound))
        {
            return IMS_TRUE;
        }
    }

    return IMS_FALSE;
}

PRIVATE
IMS_RESULT Registration::ReformContactHeader(IN_OUT ISipMessage* piSipMsg)
{
    IMS_SINT32 nHCount = piSipMsg->GetHeaderCount(ISipHeader::CONTACT_NORMAL);

    while (nHCount > 0)
    {
        piSipMsg->RemoveHeader(ISipHeader::CONTACT_NORMAL);
        --nHCount;
    }

    return SetContactNExpiresHeader(piSipMsg);
}

PRIVATE
IMS_BOOL Registration::RegisterOnImplicitRefresh()
{
    ISipClientConnection* piScc = CreateConnection(this);

    if (piScc == IMS_NULL)
    {
        IMS_TRACE_E(0, "Creating a new SIP connection for registration refresh failed", 0, 0, 0);
        return IMS_FALSE;
    }

    // Form the generic headers
    if (SetHeaders(piScc) != IMS_SUCCESS)
    {
        IMS_TRACE_E(0, "Setting the generic headers failed", 0, 0, 0);
        goto EXIT_RegisterOnImplicitRefresh;
    }

    // Sets Contact headers
    if (SetContactNExpiresHeader(piScc->GetMessage()) != IMS_SUCCESS)
    {
        IMS_TRACE_E(0, "Setting Contact & Expires header failed", 0, 0, 0);
        goto EXIT_RegisterOnImplicitRefresh;
    }

    // IMS_AUTH_NONCE_REUSE {
    // Set the credential information in case of re/de-registration
    SetNextAuthenticationInfo(piScc);
    // }

    // Send a refresh REGISTER request to the network
    if (m_pRefreshHelper->SendRefreshRequest(piScc) != IMS_SUCCESS)
    {
        IMS_TRACE_E(0, "Sending a registration refresh request failed", 0, 0, 0);
        goto EXIT_RegisterOnImplicitRefresh;
    }

    SetSubState(SUB_STATE_REFRESHING);
    UpdateBindingState(BINDING_REGISTERING);

    // Update the last request/response message
    SetPreviousRequest(piScc->GetMessage());
    SetPreviousResponse(IMS_NULL);

    //// DEBUG
    SipDebug::Send(GetSlotId(), SipDebug::MSG_REQ, SipDebug::DIR_OUT, SipMethod::REGISTER);

    return IMS_TRUE;

EXIT_RegisterOnImplicitRefresh:
    piScc->Close();

    return IMS_FALSE;
}

PRIVATE
IMS_BOOL Registration::RespondToChallenge(IN ISipClientConnection* piScc)
{
    IMS_SINT32 nPrevState = GetState();
    IMS_SINT32 nPrevSubState = GetSubState();
    IMS_BOOL bResponseToChallenge = IMS_FALSE;
    IMS_SINT32 nAlgorithm = Credential::TYPE_MD5;
    ISipGenericChallenge* pChallenge = piScc->GetAuthenticationChallenge();

    // Reset the flags after the transaction completed
    SetSubState(SUB_STATE_IDLE);

    if (pChallenge != IMS_NULL)
    {
        if (m_piListener != IMS_NULL)
        {
            nAlgorithm = Credential::TranslateAlgorithm(pChallenge->GetAlgorithm());

            m_piListener->Registration_AuthenticationChallenged(nAlgorithm, bResponseToChallenge);
        }
    }
    else
    {
        IMS_TRACE_E(0, "No authentication challenge", 0, 0, 0);
    }

    if (bResponseToChallenge)
    {
        const SubscriberConfig* pSubsConfig = GetSubsConfig();

        if (pSubsConfig == IMS_NULL)
        {
            IMS_TRACE_E(0, "SubscriberConfig is null; subsId=%s",
                    m_pStateTracker->GetSubscriberId().GetStr(), 0, 0);

            if (nPrevState == STATE_INIT)
            {
                SetState(STATE_CREATED);
            }

            UpdateBindingState(BINDING_RESULT_NOK);

            CallListener(nPrevState, nPrevSubState, REASON_INTERNAL_ERROR);
            return IMS_FALSE;
        }

        Credential& objCredential = m_pRegParam->GetCredential();

        objCredential.SetType(nAlgorithm);

        // Checks if ISIM interaction needs or not
        // Even though AKA authentication needs, it will not support if ISIM is not supported
        if ((IsAkaSupported(pSubsConfig) &&
                    ((nAlgorithm == Credential::TYPE_AKAv1_MD5) ||
                            (nAlgorithm == Credential::TYPE_AKAv2_MD5))))
        {
            // Update the 'realm' parameter from WWW-Authenticate header
            objCredential.SetRealm(pChallenge->GetRealm());

            // Request the RES to the ISIM module
            if (m_piDigestAka == IMS_NULL)
            {
                m_piDigestAka = CreateDigestAka(pSubsConfig);
            }

            if (m_piDigestAka == IMS_NULL)
            {
                if (nPrevState == STATE_INIT)
                {
                    SetState(STATE_CREATED);
                }

                UpdateBindingState(BINDING_RESULT_NOK);

                CallListener(nPrevState, nPrevSubState, REASON_STATUS_CODE);
                return IMS_FALSE;
            }

            m_piDigestAka->SetListener(this);

            // Decoding base64 encoded nonce value and pass the decoded value to the ISIM
            AString strNonce = AString::FromBase64(pChallenge->GetNonce());
            const IMS_BYTE* pbyNonce = reinterpret_cast<const IMS_BYTE*>(strNonce.GetStr());
            ByteArray objChallenge;

            // Length of RAND (1) + RAND (16)
            objChallenge.Append(0x10);
            objChallenge.Append(&pbyNonce[0], 16);
            // Length of AUTN (1) + AUTN (16)
            objChallenge.Append(0x10);
            objChallenge.Append(&pbyNonce[16], 16);

            // Set the sub-state and waits the response from ISIM module
            SetSubState(nPrevSubState);

            if (m_piDigestAka->GetAuthResponse(objChallenge) != IMS_SUCCESS)
            {
                SetSubState(SUB_STATE_IDLE);

                if (nPrevState == STATE_INIT)
                {
                    SetState(STATE_CREATED);
                }

                UpdateBindingState(BINDING_RESULT_NOK);

                CallListener(nPrevState, nPrevSubState, REASON_STATUS_CODE);
                return IMS_FALSE;
            }

            IMS_TRACE_D(
                    "RespondToChallenge :: Waiting for AKA authentication response ...", 0, 0, 0);

            // IMS_AUTH_NONCE_REUSE {
            // Keep the authentication challenge for re/de-registration
            SetAuthenticationChallenge(pChallenge);
            // }

            return IMS_TRUE;
        }
        else
        {
            // do nothing !!!
        }

        IMS_TRACE_D("RespondToChallenge :: Respond to MD5 authentication challenge ...", 0, 0, 0);

        // Overwrite the realm parameter if it required
        if (m_pRegParam->IsAuthRealmLenient() &&
                (!objCredential.GetRealm().Equals(pChallenge->GetRealm())))
        {
            IMS_TRACE_D("auth_realm_leniency is true; %s -> %s",
                    SipDebug::GetCharA1(objCredential.GetRealm().GetStr(), 4),
                    SipDebug::GetCharA2(pChallenge->GetRealm().GetStr(), 4), 0);

            objCredential.SetRealm(pChallenge->GetRealm());
        }

        if (piScc->InitResubmissionRequest() != IMS_SUCCESS)
        {
            if (nPrevState == STATE_INIT)
            {
                SetState(STATE_CREATED);
            }

            UpdateBindingState(BINDING_RESULT_NOK);

            CallListener(nPrevState, nPrevSubState, REASON_INTERNAL_ERROR);
            return IMS_FALSE;
        }

        // Authentication algorithm will be MD5
        if (piScc->SetCredentials(objCredential) != IMS_SUCCESS)
        {
            if (nPrevState == STATE_INIT)
            {
                SetState(STATE_CREATED);
            }

            UpdateBindingState(BINDING_RESULT_NOK);

            CallListener(nPrevState, nPrevSubState, REASON_INTERNAL_ERROR);
            return IMS_FALSE;
        }

        // SIP_MESSAGE_MEDIATOR
        (void)AdjustMessage(piScc->GetMessage(), IMessageMediator::MESSAGE_RESUBMIT);

        if (piScc->Send() != IMS_SUCCESS)
        {
            if (nPrevState == STATE_INIT)
            {
                SetState(STATE_CREATED);
            }

            UpdateBindingState(BINDING_RESULT_NOK);

            CallListener(nPrevState, nPrevSubState, REASON_INTERNAL_ERROR);
            return IMS_FALSE;
        }

        SetSubState(nPrevSubState);

        // IMS_AUTH_NONCE_REUSE {
        // In case of SIP digest security algorithm, we don't need to reuse the nonce value.
        // So, we will re-use the nonce only and only if 'qop' parameter exists.
        if (pChallenge->GetQop().GetLength() > 0)
        {
            // Set the active credential
            m_objActiveCredential = objCredential;

            // Keep the authentication challenge for re/de-registration
            SetAuthenticationChallenge(pChallenge);
        }
        // }

        // Update the last request/response message
        SetPreviousRequest(piScc->GetMessage());
        SetPreviousResponse(IMS_NULL);

        // Remove the previous credential information for re-authentication
        piScc->RemoveAllCredentials();
    }
    else
    {
        if (nPrevState == STATE_INIT)
        {
            SetState(STATE_CREATED);
        }

        if (nPrevSubState != SUB_STATE_REFRESHING)
        {
            UpdateBindingState(BINDING_RESULT_NOK);
        }

        CallListener(nPrevState, nPrevSubState, REASON_STATUS_CODE);
        return IMS_FALSE;
    }

    return IMS_TRUE;
}

PRIVATE
IMS_BOOL Registration::RespondToPendingChallenge(IN const Credential& objCredential)
{
    IMS_SINT32 nPrevState = GetState();
    IMS_SINT32 nPrevSubState = GetSubState();

    IMS_TRACE_D("Start to respond to AKA authentication challenge ...", 0, 0, 0);

    if ((nPrevSubState == SUB_STATE_REGISTERING) || (nPrevSubState == SUB_STATE_DEREGISTERING))
    {
        if (m_piOngoingScc == IMS_NULL)
        {
            if (nPrevState == STATE_INIT)
            {
                SetState(STATE_CREATED);
            }

            UpdateBindingState(BINDING_RESULT_NOK);

            CallListener(nPrevState, nPrevSubState, REASON_INTERNAL_ERROR);
            return IMS_FALSE;
        }

        if (m_piOngoingScc->InitResubmissionRequest() != IMS_SUCCESS)
        {
            m_piOngoingScc->Close();
            m_piOngoingScc = IMS_NULL;

            if (nPrevState == STATE_INIT)
            {
                SetState(STATE_CREATED);
            }

            UpdateBindingState(BINDING_RESULT_NOK);

            CallListener(nPrevState, nPrevSubState, REASON_INTERNAL_ERROR);
            return IMS_FALSE;
        }

        // RFC5626_FLOW_CONTROL
        // MULTI_REG_TRANSPORT
        IMS_SINT32 nPortUs = GetPortUs();
        IMS_SINT32 nPortFlowControl;
        IMS_SINT32 nTransportExt = m_pRegParam->GetTransportExtForRegOnly();

        if (nPrevState == STATE_INIT)
        {
            if (!m_pRegParam->IsSecurityAssociationPresent())
            {
                nPortUs = GetPortUc();
            }
            else
            {
                // Update the protected server port if SA established
                UpdateProtectedServerPortForContact(m_piOngoingScc->GetMessage());
            }

            nPortFlowControl = m_pRegParam->GetPortFlowControl();
            nTransportExt |= m_pRegParam->GetTransportExt();
        }
        else
        {
            nPortFlowControl = m_pStateTracker->GetPortFlowControl();
            nTransportExt |= m_pStateTracker->GetTransportExt();
        }

        m_piOngoingScc->SetTransportTuple(m_pStateTracker->GetIpAddress(), nPortUs, GetPortUc(),
                nPortFlowControl, nTransportExt);

        // IMS_IPSEC_UDP_ENC
        if ((nPrevSubState == SUB_STATE_REGISTERING) &&
                m_pRegParam->IsSecurityAssociationRequiredViaUdpEnc())
        {
            UpdateHostInfoInAllContacts();

            if (ReformContactHeader(m_piOngoingScc->GetMessage()) != IMS_SUCCESS)
            {
                m_piOngoingScc->Close();
                m_piOngoingScc = IMS_NULL;

                if (nPrevState == STATE_INIT)
                {
                    SetState(STATE_CREATED);
                }

                UpdateBindingState(BINDING_RESULT_NOK);

                CallListener(nPrevState, nPrevSubState, REASON_INTERNAL_ERROR);
                return IMS_FALSE;
            }
        }

        if (m_pRegParam->FormRouteHeaders(m_piOngoingScc, m_pStateTracker) != IMS_SUCCESS)
        {
            m_piOngoingScc->Close();
            m_piOngoingScc = IMS_NULL;

            if (nPrevState == STATE_INIT)
            {
                SetState(STATE_CREATED);
            }

            UpdateBindingState(BINDING_RESULT_NOK);

            CallListener(nPrevState, nPrevSubState, REASON_INTERNAL_ERROR);
            return IMS_FALSE;
        }

        if (m_pRegParam->FormSecurityHeaders(m_piOngoingScc) != IMS_SUCCESS)
        {
            m_piOngoingScc->Close();
            m_piOngoingScc = IMS_NULL;

            if (nPrevState == STATE_INIT)
            {
                SetState(STATE_CREATED);
            }

            UpdateBindingState(BINDING_RESULT_NOK);

            CallListener(nPrevState, nPrevSubState, REASON_INTERNAL_ERROR);
            return IMS_FALSE;
        }

        if (m_piOngoingScc->SetCredentials(objCredential) != IMS_SUCCESS)
        {
            m_piOngoingScc->Close();
            m_piOngoingScc = IMS_NULL;

            if (nPrevState == STATE_INIT)
            {
                SetState(STATE_CREATED);
            }

            UpdateBindingState(BINDING_RESULT_NOK);

            CallListener(nPrevState, nPrevSubState, REASON_INTERNAL_ERROR);
            return IMS_FALSE;
        }

        // Sets a new P-Access-Network-Info header on re-submitted message (2nd-REGISTER)
        if (SipConfigProxy::IsPanInfoInInitialRegRequired(
                    GetSlotId(), m_pStateTracker->GetSipProfile()))
        {
            ISipMessage* piSipMsg = m_piOngoingScc->GetMessage();
            PAccessNetworkInfoHeader::SetHeader(GetSlotId(), m_pStateTracker->GetIpAddress(),
                    m_pStateTracker->GetSipProfile(), piSipMsg);
        }
        else
        {
            if ((nPrevState == STATE_INIT) &&
                    (objCredential.GetAkaResponse().m_nStatus != ImsAkaParam::RESULT_OK))
            {
                // Do not add P-Access-Network-Info header since the authentication is failed...
            }
            else
            {
                ISipMessage* piSipMsg = m_piOngoingScc->GetMessage();
                PAccessNetworkInfoHeader::SetHeader(GetSlotId(), m_pStateTracker->GetIpAddress(),
                        m_pStateTracker->GetSipProfile(), piSipMsg);
            }
        }

        // SIP_MESSAGE_MEDIATOR
        (void)AdjustMessage(m_piOngoingScc->GetMessage(), IMessageMediator::MESSAGE_RESUBMIT);

        ISipTransportHelper* piTransHelper = SipFactory::GetTransportHelper(GetSlotId());

        if (piTransHelper != IMS_NULL)
        {
            piTransHelper->ApplyIpSecForServerSockets();
        }

        if (m_piOngoingScc->Send() != IMS_SUCCESS)
        {
            m_piOngoingScc->Close();
            m_piOngoingScc = IMS_NULL;

            if (nPrevState == STATE_INIT)
            {
                SetState(STATE_CREATED);
            }

            UpdateBindingState(BINDING_RESULT_NOK);

            CallListener(nPrevState, nPrevSubState, REASON_INTERNAL_ERROR);
            return IMS_FALSE;
        }

        // Update the last request/response message
        SetPreviousRequest(m_piOngoingScc->GetMessage());
        SetPreviousResponse(IMS_NULL);

        // Remove the previous credential information for re-authentication
        m_piOngoingScc->RemoveAllCredentials();
    }
    // SUB_STATE_REFRESHING
    else
    {
        ISipClientConnection* piScc = m_pRefreshHelper->GetConnection();

        if (piScc == IMS_NULL)
        {
            UpdateBindingState(BINDING_RESULT_NOK);

            CallListener(nPrevState, nPrevSubState, REASON_INTERNAL_ERROR);
            return IMS_FALSE;
        }

        if (piScc->InitResubmissionRequest() != IMS_SUCCESS)
        {
            m_pRefreshHelper->AbortConnection();

            UpdateBindingState(BINDING_RESULT_NOK);

            CallListener(nPrevState, nPrevSubState, REASON_INTERNAL_ERROR);
            return IMS_FALSE;
        }

        // RFC5626_FLOW_CONTROL
        IMS_SINT32 nTransportExt = m_pRegParam->GetTransportExtForRegOnly();

        nTransportExt |= m_pStateTracker->GetTransportExt();

        piScc->SetTransportTuple(m_pStateTracker->GetIpAddress(), GetPortUs(), GetPortUc(),
                m_pStateTracker->GetPortFlowControl(), nTransportExt);

        if (m_pRegParam->FormRouteHeaders(piScc, m_pStateTracker) != IMS_SUCCESS)
        {
            m_pRefreshHelper->AbortConnection();

            UpdateBindingState(BINDING_RESULT_NOK);

            CallListener(nPrevState, nPrevSubState, REASON_INTERNAL_ERROR);
            return IMS_FALSE;
        }

        if (m_pRegParam->FormSecurityHeaders(piScc) != IMS_SUCCESS)
        {
            m_pRefreshHelper->AbortConnection();

            UpdateBindingState(BINDING_RESULT_NOK);

            CallListener(nPrevState, nPrevSubState, REASON_INTERNAL_ERROR);
            return IMS_FALSE;
        }

        if (piScc->SetCredentials(objCredential) != IMS_SUCCESS)
        {
            m_pRefreshHelper->AbortConnection();

            UpdateBindingState(BINDING_RESULT_NOK);

            CallListener(nPrevState, nPrevSubState, REASON_INTERNAL_ERROR);
            return IMS_FALSE;
        }

        // Sets a new P-Access-Network-Info header on re-submitted message (2nd-REGISTER)
        ISipMessage* piSipMsg = piScc->GetMessage();
        PAccessNetworkInfoHeader::SetHeader(GetSlotId(), m_pStateTracker->GetIpAddress(),
                m_pStateTracker->GetSipProfile(), piSipMsg);

        // SIP_MESSAGE_MEDIATOR
        (void)AdjustMessage(piScc->GetMessage(), IMessageMediator::MESSAGE_RESUBMIT);

        if (piScc->Send() != IMS_SUCCESS)
        {
            m_pRefreshHelper->AbortConnection();

            UpdateBindingState(BINDING_RESULT_NOK);

            CallListener(nPrevState, nPrevSubState, REASON_INTERNAL_ERROR);
            return IMS_FALSE;
        }

        // Update the last request/response message
        SetPreviousRequest(piScc->GetMessage());
        SetPreviousResponse(IMS_NULL);

        // Remove the previous credential information for re-authentication
        piScc->RemoveAllCredentials();
    }

    IMS_TRACE_D("Terminate to respond to AKA authentication challenge ...", 0, 0, 0);

    return IMS_TRUE;
}

PRIVATE
void Registration::RestoreSecurityHeaders()
{
    m_pRegParam->RemoveSecurityServers();

    if (GetState() == STATE_INIT)
    {
        m_pRegParam->RemovePreferredSecurityHeaders();
    }
    else
    {
        m_pRegParam->RestoreSecurityHeaders();
    }
}

// IMS_AUTH_NONCE_REUSE {
PRIVATE
void Registration::SetAuthenticationChallenge(IN ISipGenericChallenge* piChallenge)
{
    if (piChallenge == IMS_NULL)
    {
        if (m_piGenericChallenge != IMS_NULL)
        {
            m_piGenericChallenge->Destroy();
            m_piGenericChallenge = IMS_NULL;
        }

        return;
    }

    if (m_piGenericChallenge != IMS_NULL)
    {
        IMS_TRACE_D("Authentication challenge (%s) is already present",
                m_piGenericChallenge->GetNonce().GetStr(), 0, 0);
        return;
    }

    m_piGenericChallenge = piChallenge->Clone();
}
// }

PRIVATE
IMS_RESULT Registration::SetContactNExpiresHeader(
        IN_OUT ISipMessage* piSipMsg, IN IMS_SINT32 nExpires /*= (-1)*/)
{
    // de-REG
    if (nExpires == 0)
    {
        // Wildcard Contact header : remove all the bindings related to this IMPU.
        if (m_objContacts.IsEmpty())
        {
            if (piSipMsg->SetHeader(ISipHeader::CONTACT_WILDCARD, "*") != IMS_SUCCESS)
            {
                IMS_TRACE_E(0, "Setting Contact header (using wildcard [*]) failed", 0, 0, 0);
                return IMS_FAILURE;
            }

            // If the Contact header uses a wildcard option,
            // the Expires header field needs to be set as 0.
            if (piSipMsg->SetHeader(ISipHeader::EXPIRES_SEC, "0") != IMS_SUCCESS)
            {
                IMS_TRACE_E(0, "Setting Expires header failed", 0, 0, 0);
                return IMS_FAILURE;
            }
        }
        else
        {
            AString strContact;

            for (IMS_UINT32 i = 0; i < m_objContacts.GetSize(); ++i)
            {
                RegContact* pContact = m_objContacts.GetAt(i);

                strContact = pContact->ToString();

                // Adds a 'expires' parameter
                strContact.Append(TextParser::CHAR_SEMICOLON);
                strContact.Append(Sip::STR_EXPIRES);
                strContact.Append(TextParser::CHAR_EQUAL);
                strContact.Append('0');

                if (piSipMsg->AddHeader(ISipHeader::CONTACT_NORMAL, strContact) != IMS_SUCCESS)
                {
                    IMS_TRACE_E(0, "Adding Contact header failed", 0, 0, 0);
                    return IMS_FAILURE;
                }
            }

            // If the Contact header uses a wildcard option,
            // the Expires header field needs to be set as 0.
            if (SetExpiresHeader(piSipMsg, 0) != IMS_SUCCESS)
            {
                return IMS_FAILURE;
            }
        }
    }
    else
    {
        for (IMS_UINT32 i = 0; i < m_objContacts.GetSize(); ++i)
        {
            RegContact* pContact = m_objContacts.GetAt(i);

            if (piSipMsg->AddHeader(ISipHeader::CONTACT_NORMAL, pContact->ToStringWithExpires()) !=
                    IMS_SUCCESS)
            {
                IMS_TRACE_E(0, "Adding Contact header failed", 0, 0, 0);
                return IMS_FAILURE;
            }
        }

        if (SetExpiresHeader(piSipMsg, nExpires) != IMS_SUCCESS)
        {
            return IMS_FAILURE;
        }
    }

    return IMS_SUCCESS;
}

PRIVATE
IMS_RESULT Registration::SetExpiresHeader(
        IN_OUT ISipMessage* piSipMsg, IN IMS_SINT32 nExpires /*= (-1)*/)
{
    if (!SipConfigProxy::IsExpiresHeaderInRegRequired(
                GetSlotId(), m_pStateTracker->GetSipProfile()))
    {
        // "expires" header parameter will be used to represent the registration expiration
        return IMS_SUCCESS;
    }

    if (nExpires != (-1))
    {
        AString strExpires;
        strExpires.Sprintf("%u", nExpires);

        if (piSipMsg->SetHeader(ISipHeader::EXPIRES_SEC, strExpires) != IMS_SUCCESS)
        {
            IMS_TRACE_E(0, "Setting Expires header failed", 0, 0, 0);
            return IMS_FAILURE;
        }
    }
    else
    {
        RegContact* pPreferredContact = m_objContacts.IsEmpty() ? IMS_NULL : m_objContacts.GetAt(0);

        if ((pPreferredContact != IMS_NULL) &&
                (pPreferredContact->GetInitialExpires() !=
                        static_cast<IMS_UINT32>(RegContact::EXPIRES_NOT_SPECIFIED)))
        {
            AString strExpires;
            strExpires.Sprintf("%u", pPreferredContact->GetInitialExpires());

            if (piSipMsg->SetHeader(ISipHeader::EXPIRES_SEC, strExpires) != IMS_SUCCESS)
            {
                IMS_TRACE_E(0, "Setting Expires header failed", 0, 0, 0);
                return IMS_FAILURE;
            }
        }
    }

    return IMS_SUCCESS;
}

PRIVATE
IMS_RESULT Registration::SetHeaders(IN ISipClientConnection* piScc)
{
    ISipMessage* piSipMsg = piScc->GetMessage();
    AString strCSeqHdr;

    strCSeqHdr.Sprintf(
            "%lu %s", m_pRegFlow->IncreaseNGetCSeqValue(), SipMethod::ToName(SipMethod::REGISTER));

    if (piSipMsg->SetHeader(ISipHeader::CSEQ, strCSeqHdr) != IMS_SUCCESS)
    {
        IMS_TRACE_E(0, "Setting CSeq header failed", 0, 0, 0);
        return IMS_FAILURE;
    }

    // Set Call-ID header
    if (piSipMsg->SetHeader(ISipHeader::CALL_ID, m_pRegFlow->GetCallId()) != IMS_SUCCESS)
    {
        IMS_TRACE_E(0, "Setting Call-ID header failed", 0, 0, 0);
        return IMS_FAILURE;
    }

    // HEADER_REQ_SESSION-ID
    if (m_pRegFlow->GetSessionId().GetLength() > 0)
    {
        // Set Session-ID header
        if (piSipMsg->SetHeader(ISipHeader::SESSION_ID, m_pRegFlow->GetSessionId()) != IMS_SUCCESS)
        {
            IMS_TRACE_E(0, "Setting Session-ID failed", 0, 0, 0);
            return IMS_FAILURE;
        }
    }

    if (m_pRegParam->FormHeaders(piScc, m_pStateTracker) != IMS_SUCCESS)
    {
        IMS_TRACE_E(0, "Forming REGISTER request failed", 0, 0, 0);
        return IMS_FAILURE;
    }

    if (m_pRegParam->FormSecurityHeaders(piScc) != IMS_SUCCESS)
    {
        IMS_TRACE_E(0, "Forming security headers failed", 0, 0, 0);
        return IMS_FAILURE;
    }

    const IRegContact* piContact = GetPreferredContact();

    // Set P-Access-Network-Info header
    if (piContact != IMS_NULL)
    {
        // re-REG / de-REG
        if (GetState() == STATE_ACTIVE)
        {
            PAccessNetworkInfoHeader::SetHeader(GetSlotId(), piContact->GetIpAddress(),
                    m_pStateTracker->GetSipProfile(), piSipMsg);
        }
        else
        {
            // Configuration for an initial REGISTER / SA is present on INIT
            if (SipConfigProxy::IsPanInfoInInitialRegRequired(
                        GetSlotId(), m_pStateTracker->GetSipProfile()) ||
                    m_pRegParam->IsSecurityAssociationPresent())
            {
                PAccessNetworkInfoHeader::SetHeader(GetSlotId(), piContact->GetIpAddress(),
                        m_pStateTracker->GetSipProfile(), piSipMsg);
            }
        }
    }

    return IMS_SUCCESS;
}

// IMS_AUTH_NONCE_REUSE {
PRIVATE
void Registration::SetNextAuthenticationInfo(IN_OUT ISipClientConnection*& piScc)
{
    if (m_piGenericChallenge != IMS_NULL)
    {
        m_piGenericChallenge->IncreaseNonceCount();

        piScc->SetCredentials(m_objActiveCredential);
        piScc->SetAuthenticationChallenge(m_piGenericChallenge);
    }
}

// SIP_DIGEST_AUTH_NONCE_REUSE
PRIVATE
void Registration::SetNextNonce(IN ISipMessage* piSipMsg)
{
    if (m_piGenericChallenge == IMS_NULL)
    {
        return;
    }

    const SubscriberConfig* pSubsConfig = GetSubsConfig();

    if (IsAkaSupported(pSubsConfig))
    {
        // If IMS-AKA is required, the "nextnonce" field will be ignored.
        return;
    }

    AString strAuthenticationInfo = piSipMsg->GetHeader(ISipHeader::AUTHENTICATION_INFO);

    if (strAuthenticationInfo.GetLength() == 0)
    {
        return;
    }

    const IMS_CHAR NEXT_NONCE[] = "nextnonce";
    ImsList<AString> objTokens = strAuthenticationInfo.Split(TextParser::CHAR_COMMA);
    AString strName;
    AString strValue;
    IMS_BOOL bNextNoncePresent = IMS_FALSE;

    for (IMS_UINT32 i = 0; i < objTokens.GetSize(); ++i)
    {
        const AString& strToken = objTokens.GetAt(i);
        IMS_SINT32 nCount = strToken.SplitF(TextParser::CHAR_EQUAL, strName, strValue);

        if ((nCount == 2) && strName.EqualsIgnoreCase(NEXT_NONCE))
        {
            bNextNoncePresent = IMS_TRUE;
            break;
        }
    }

    if (bNextNoncePresent)
    {
        if (strValue.StartsWith(TextParser::CHAR_DQUOT))
        {
            m_piGenericChallenge->SetNonce(strValue.Mid(1, strValue.GetLength() - 2));
        }
        else
        {
            m_piGenericChallenge->SetNonce(strValue);
        }
    }
}
// }

PRIVATE
void Registration::SetOngoingConnection(IN ISipClientConnection* piScc)
{
    if (m_piOngoingScc != IMS_NULL)
    {
        m_piOngoingScc->Close();
        m_piOngoingScc = IMS_NULL;
    }

    if (piScc != IMS_NULL)
    {
        m_piOngoingScc = piScc;
    }
}

PRIVATE
void Registration::SetPreviousRequest(IN ISipMessage* piSipMsg)
{
    if (m_piPreviousRequest != IMS_NULL)
    {
        m_piPreviousRequest->Destroy();
        m_piPreviousRequest = IMS_NULL;
    }

    if (piSipMsg != IMS_NULL)
    {
        m_piPreviousRequest = piSipMsg->Clone();
    }
}

PRIVATE
void Registration::SetPreviousResponse(IN ISipMessage* piSipMsg)
{
    if (m_piPreviousResponse != IMS_NULL)
    {
        m_piPreviousResponse->Destroy();
        m_piPreviousResponse = IMS_NULL;
    }

    if (piSipMsg != IMS_NULL)
    {
        m_piPreviousResponse = piSipMsg->Clone();
    }
}

PRIVATE
void Registration::SetState(IN IMS_SINT32 nState)
{
    IMS_TRACE_I("Registration (%s) :: %s to %s",
            SipDebug::GetUri1(m_pStateTracker->GetAor().GetUri()).GetStr(), StateToString(m_nState),
            StateToString(nState));

    m_nState = nState;
}

PRIVATE
void Registration::SetSubState(IN IMS_SINT32 nSubState)
{
    IMS_TRACE_I("Registration (Sub-State) :: %s to %s", SubStateToString(m_nSubState),
            SubStateToString(nSubState), 0);

    m_nSubState = nSubState;

    if (m_nSubState == SUB_STATE_IDLE)
    {
        // Release the registration flow
        IMS_UINT32 nSubscriber = RegFlow::NO_SUBSCRIBER;

        if (m_pRegFlow->IsReserved(&nSubscriber))
        {
            if (nSubscriber == GetName().GetHashCode())
            {
                m_pRegFlow->Release();
            }
        }
    }
}

PRIVATE
void Registration::StorePersistentHeaders(IN const ISipMessage* piSipMsg)
{
    // IMS_IPSEC_UDP_ENC
    if (m_pRegParam->IsSecurityAssociationRequiredViaUdpEnc())
    {
        ChoosePreferredContact();
    }

    // Stores the public user identities
    AStringArray objTmpArray = piSipMsg->GetHeaders(ISipHeader::P_ASSOCIATED_URI);

    // If P-Associated-URI does not contains in 200 OK,
    // the explicit registration AOR will be added.
    if (objTmpArray.IsEmpty())
    {
        objTmpArray.AddElement(m_pStateTracker->GetAor().ToString());

        IMS_TRACE_D("P-Associated-URI is not present; AOR (%s)",
                SipDebug::GetUri1(objTmpArray.GetElementAt(0)).GetStr(), 0, 0);
    }

    if (m_piUserIdNotifier != IMS_NULL)
    {
        AStringArray objReorderedUserIds;

        if (m_piUserIdNotifier->RegUserIdentity_ReorderUserIdentities(
                    objTmpArray, objReorderedUserIds))
        {
            m_pStateTracker->SetAssociatedUris(objReorderedUserIds);
        }
        else
        {
            m_pStateTracker->SetAssociatedUris(objTmpArray);
        }
    }
    else
    {
        m_pStateTracker->SetAssociatedUris(objTmpArray);
    }

    // Stores the Service-Route headers for the next outgoing request
    objTmpArray = piSipMsg->GetHeaders(ISipHeader::SERVICE_ROUTE);

    // Update the service-routes if preloaded route set is present
    const AStringArray& objPreloadedRoutes = m_pRegParam->GetPreloadedRoutes();

    if (!objPreloadedRoutes.IsEmpty())
    {
        for (IMS_SINT32 i = objPreloadedRoutes.GetCount() - 1; i >= 0; --i)
        {
            objTmpArray.InsertElementAt(objPreloadedRoutes.GetElementAt(i), 0);
        }
    }

    m_pStateTracker->SetServiceRoutes(objTmpArray);

    // Path header fields
    objTmpArray = piSipMsg->GetHeaders(ISipHeader::PATH);
    m_pStateTracker->SetPathHeaders(objTmpArray);

    m_pStateTracker->SetTransportExt(m_pRegParam->GetTransportExt());

    // RFC5626_FLOW_CONTROL
    // If IPSEC is supported and the flow control needs to be supported,
    // then the flow control port SHALL be a port_uc.
    if (IsFlowControlRequired())
    {
        m_pStateTracker->SetPortFlowControl(m_pRegParam->GetPortFlowControl());
    }
    else
    {
        m_pStateTracker->SetPortFlowControl(Sip::PORT_UNSPECIFIED);
    }

    // Updates the protected client /server port (uc / us)
    m_pStateTracker->SetPortUc(GetPortUc());
    m_pStateTracker->SetPortUs(GetPortUs());

    // Security-Client / Security-Verify headers
    m_pStateTracker->SetSecurityClients(m_pRegParam->GetSecurityClients());
    m_pStateTracker->SetSecurityVerifys(m_pRegParam->GetSecurityVerifys());
}

PRIVATE
IMS_RESULT Registration::UpdateBindings(IN const ISipMessage* piSipMsg)
{
    IMS_SINT32 nHeaderCount = piSipMsg->GetHeaderCount(ISipHeader::CONTACT_ANY);
    AString strHeader;
    ISipHeader* piHeader;

    IMS_SINT32 nExpiresValue = (-1);
    ImsList<ISipHeader*> objHeaders;

    // Extract the expiration value from Expires header field
    if (piSipMsg->IsHeaderPresent(ISipHeader::EXPIRES_ANY))
    {
        strHeader = piSipMsg->GetHeader(ISipHeader::EXPIRES_ANY);
        piHeader = SipParsingHelper::CreateHeader(ISipHeader::EXPIRES_ANY, strHeader);

        if (piHeader != IMS_NULL)
        {
            // 4 Make up for test equipment's fault (e.g. Anite)
            if (piHeader->GetValue().Contains(TextParser::CHAR_DQUOT))
            {
                IMS_BOOL bOk = IMS_FALSE;
                AString strValue = TextParser::TrimDquot(piHeader->GetValue());

                nExpiresValue = strValue.ToInt32(&bOk);

                if (!bOk)
                {
                    nExpiresValue = (-1);
                }
            }
            else
            {
                nExpiresValue = piHeader->GetValueInt();
            }

            piHeader->Destroy();
        }
    }

    // Process more operation for Contact headers
    for (IMS_SINT32 i = 0; i < nHeaderCount; ++i)
    {
        strHeader = piSipMsg->GetHeader(ISipHeader::CONTACT_ANY, i);
        piHeader = SipParsingHelper::CreateHeader(ISipHeader::CONTACT_ANY, strHeader);

        if (piHeader == IMS_NULL)
        {
            continue;
        }

        if (!objHeaders.Append(piHeader))
        {
            piHeader->Destroy();
            continue;
        }
    }

    IMS_TRACE_D("Contact header field :: count=%d", objHeaders.GetSize(), 0, 0);

    if (objHeaders.GetSize() == 1)
    {
        piHeader = objHeaders.GetAt(0);

        if (piHeader->GetType() == ISipHeader::CONTACT_WILDCARD)
        {
            // Only for de-registration procedure
            // So, all the registration contacts transit to "STATE_TERMINATED"
            for (IMS_UINT32 j = 0; j < m_objContacts.GetSize(); ++j)
            {
                RegContact* pContact = m_objContacts.GetAt(j);

                pContact->SetTerminated();
            }

            DestroyAllHeaders(objHeaders);

            return BINDING_UPDATE_OK;
        }
    }

    IMS_RESULT nBindingUpdateResult = BINDING_UPDATE_OK;
    IMS_BOOL bAtLeastOneContactUpdated = IMS_FALSE;

    // For each registration contact, updates the parameter ('expires')
    // If no match found, it transits to "STATE_TERMINATED"
    for (IMS_UINT32 j = 0; j < m_objContacts.GetSize(); ++j)
    {
        RegContact* pContact = m_objContacts.GetAt(j);
        IMS_SINT32 nUpdateResult = pContact->UpdateParameter(objHeaders, nExpiresValue);

        if (nUpdateResult == RegContact::UPDATE_OK)
        {
            bAtLeastOneContactUpdated = IMS_TRUE;
        }
        else if (nUpdateResult == RegContact::UPDATE_NO_EXPIRES)
        {
            nBindingUpdateResult = BINDING_UPDATE_NO_EXPIRES;
        }
    }

    // If the Contact is updated for at least one contact, the result will be reset.
    if (bAtLeastOneContactUpdated)
    {
        nBindingUpdateResult = BINDING_UPDATE_OK;
    }

    // Destroy the local resource
    DestroyAllHeaders(objHeaders);

    return nBindingUpdateResult;
}

PRIVATE
void Registration::UpdateBindingState(IN IMS_SINT32 nState)
{
    for (IMS_UINT32 i = 0; i < m_objObservers.GetSize(); ++i)
    {
        RegObserver* pObserver = m_objObservers.GetAt(i);

        if (pObserver == IMS_NULL)
        {
            continue;
        }

        pObserver->Update(nState);
    }
}

// IMS_IPSEC_UDP_ENC
PRIVATE
void Registration::UpdateHostInfoInAllContacts()
{
    if (m_pRegParam->IsSecurityAssociationRequiredViaUdpEnc())
    {
        for (IMS_UINT32 i = 0; i < m_objContacts.GetSize(); ++i)
        {
            RegContact* pContact = m_objContacts.GetAt(i);

            pContact->SetHostInfo(m_pStateTracker->GetPublicIpAddress());
        }
    }
}

PRIVATE
void Registration::UpdateCSeqNumber(IN const ISipMessage* piSipMsg)
{
    AString strCSeq = piSipMsg->GetHeader(ISipHeader::CSEQ);
    IMS_SINT32 nPosOfSp = strCSeq.GetIndexOf(TextParser::CHAR_SP);

    if (nPosOfSp == AString::NPOS)
    {
        // TODO:: select the proper value to increase sequence number when an error occurs
        // If CSeq header is invalid, the number will be increased as much as 5
        (void)m_pRegFlow->IncreaseNGetCSeqValue(5);
        return;
    }

    strCSeq = strCSeq.GetSubStr(0, nPosOfSp);

    IMS_BOOL bOk = IMS_FALSE;
    IMS_SINT32 nSeqNum = strCSeq.ToInt32(&bOk);

    if (!bOk)
    {
        // If the number is not converted, the number will be increased as much as 5
        (void)m_pRegFlow->IncreaseNGetCSeqValue(5);
        return;
    }

    IMS_TRACE_I("Registration - Sequence Number Changed (%d >> %d)",
            m_pRegFlow->IncreaseNGetCSeqValue(0), nSeqNum, 0);

    // Store the current sequence number (last one) from the last successful response.
    m_pRegFlow->SetCSeqValue(nSeqNum);
}

PRIVATE
void Registration::UpdateProtectedServerPortForContact(IN_OUT ISipMessage* piSipMsg)
{
    IRegContact* piRegContact = GetPreferredContact();

    if (piRegContact == IMS_NULL)
    {
        IMS_TRACE_D("No preferred contact", 0, 0, 0);
        return;
    }

    IMS_SINT32 nHCount = piSipMsg->GetHeaderCount(ISipHeader::CONTACT_NORMAL);

    if (nHCount == 1)
    {
        AString strContact = piSipMsg->GetHeader(ISipHeader::CONTACT_NORMAL);
        ISipHeader* piHeader =
                SipParsingHelper::CreateHeader(ISipHeader::CONTACT_NORMAL, strContact);

        if (piHeader != IMS_NULL)
        {
            SipAddress* pAddress = const_cast<SipAddress*>(piHeader->GetSipAddress());

            if (pAddress != IMS_NULL)
            {
                IMS_SINT32 nPort = piRegContact->GetPort();

                if (pAddress->GetPort() != nPort)
                {
                    IMS_TRACE_D("RegContact :: port is changed; %d >> %d", pAddress->GetPort(),
                            nPort, 0);

                    pAddress->SetPort(nPort);

                    piSipMsg->RemoveHeader(ISipHeader::CONTACT_NORMAL);
                    piSipMsg->SetHeader(
                            ISipHeader::CONTACT_NORMAL, piHeader->ToStringWithoutName());
                }
            }
            else
            {
                // Exceptional case
                IMS_TRACE_D("ReformContactHeader :: SipAddress is null", 0, 0, 0);
                ReformContactHeader(piSipMsg);
            }

            piHeader->Destroy();
        }
        else
        {
            // Exceptional case
            IMS_TRACE_D("ReformContactHeader :: ISipHeader is null", 0, 0, 0);
            ReformContactHeader(piSipMsg);
        }
    }
}

PRIVATE
void Registration::UpdateRefreshTimer()
{
    IMS_UINT32 nExpirationValue = 0;
    IMS_UINT32 nAnchorContact = 0xFFFFFFFF;

    // Select the minimum expiration value among the registered contacts
    // (State of contact is 'active')
    for (IMS_UINT32 i = 0; i < m_objContacts.GetSize(); ++i)
    {
        const RegContact* pContact = m_objContacts.GetAt(i);

        if (!pContact->IsActiveBinding())
        {
            continue;
        }

        IMS_UINT32 nTmpVal = pContact->GetExpires();

        if (nTmpVal != 0)
        {
            if (nExpirationValue == 0)
            {
                nExpirationValue = nTmpVal;
                nAnchorContact = i;
            }
            else
            {
                if (nTmpVal < nExpirationValue)
                {
                    nExpirationValue = nTmpVal;
                    nAnchorContact = i;
                }
            }
        }
    }

    if (nAnchorContact != 0xFFFFFFFF)
    {
        const RegContact* pContact = m_objContacts.GetAt(nAnchorContact);

        m_pRefreshHelper->SetContactAddress(pContact->GetContactAddress());
    }

    m_pRefreshHelper->UpdateRefreshTimer(static_cast<IMS_SINT32>(nExpirationValue));
}

PRIVATE GLOBAL ISipClientConnection* Registration::CreateConnection(IN Registration* pReg)
{
    AString strAor = pReg->m_pStateTracker->GetAor().ToString();
    const RegContact* pContact = pReg->m_pStateTracker->GetPreferredContact();

    if (pContact == IMS_NULL)
    {
        IMS_TRACE_E(0, "No contacts for the registration (%s)", SipDebug::GetUri1(strAor).GetStr(),
                0, 0);
        return IMS_NULL;
    }

    ISipClientConnection* piScc = DYNAMIC_CAST(ISipClientConnection*, Connector::Open(strAor));

    IMS_TRACE_D("CreateConnection - To (%s), Method (REGISTER)", SipDebug::GetUri1(strAor).GetStr(),
            0, 0);

    if (piScc == IMS_NULL)
    {
        IMS_TRACE_E(0, "Creating a SIP client connection failed", 0, 0, 0);
        return IMS_NULL;
    }

    piScc->SetSipProfile(pReg->m_pStateTracker->GetSipProfile());

    // Sets the transport tuples
    // RFC5626_FLOW_CONTROL
    IMS_SINT32 nTransportExt = pReg->m_pRegParam->GetTransportExtForRegOnly();

    if (pReg->GetState() == STATE_CREATED)
    {
        nTransportExt |= pReg->m_pRegParam->GetTransportExt();

        piScc->SetTransportTuple(pContact->GetIpAddress(), pReg->GetPortUc(), pReg->GetPortUc(),
                pReg->m_pRegParam->GetPortFlowControl(), nTransportExt);
    }
    else
    {
        nTransportExt |= pReg->m_pStateTracker->GetTransportExt();

        piScc->SetTransportTuple(pContact->GetIpAddress(), pReg->GetPortUs(),
                pReg->m_pStateTracker->GetPortUc(), pReg->m_pStateTracker->GetPortFlowControl(),
                nTransportExt);
    }

    // Sets the SIP transaction timers
    const SipTimerValues* pSipTimerValues = pReg->m_pRegParam->GetSipTimerValues();

    if (pSipTimerValues != IMS_NULL)
    {
        piScc->SetTransactionTimerValues(*pSipTimerValues);
    }
    else
    {
        piScc->SetTransactionTimerValues(SipTimerValuesHelper::GetValues(
                pReg->GetSlotId(), pReg->m_pStateTracker->GetSipProfile()));
    }

    if (piScc->InitRequest(SipMethod::ToName(SipMethod::REGISTER), IMS_NULL) != IMS_SUCCESS)
    {
        piScc->Close();
        return IMS_NULL;
    }

    // Set From header
    if (piScc->GetMessage()->SetHeader(ISipHeader::FROM, strAor) != IMS_SUCCESS)
    {
        piScc->Close();
        return IMS_NULL;
    }

    return piScc;
}

PRIVATE GLOBAL void Registration::DestroyAllHeaders(IN_OUT ImsList<ISipHeader*>& objHeaders)
{
    if (objHeaders.IsEmpty())
    {
        return;
    }

    for (IMS_UINT32 i = 0; i < objHeaders.GetSize(); ++i)
    {
        ISipHeader* piHeader = objHeaders.GetAt(i);

        if (piHeader != IMS_NULL)
        {
            piHeader->Destroy();
        }
    }

    objHeaders.Clear();
}

PRIVATE GLOBAL const IMS_CHAR* Registration::StateToString(IN IMS_SINT32 nState)
{
    switch (nState)
    {
        case STATE_CREATED:
            return "STATE_CREATED";
        case STATE_INIT:
            return "STATE_INIT";
        case STATE_ACTIVE:
            return "STATE_ACTIVE";
        case STATE_TERMINATED:
            return "STATE_TERMINATED";
        default:
            return "__INVALID__";
    }
}

PRIVATE GLOBAL const IMS_CHAR* Registration::SubStateToString(IN IMS_SINT32 nSubState)
{
    switch (nSubState)
    {
        case SUB_STATE_IDLE:
            return "SUB_STATE_IDLE";
        case SUB_STATE_REGISTERING:
            return "SUB_STATE_REGISTERING";
        case SUB_STATE_REFRESHING:
            return "SUB_STATE_REFRESHING";
        case SUB_STATE_DEREGISTERING:
            return "SUB_STATE_DEREGISTERING";
        default:
            return "__INVALID__";
    }
}
