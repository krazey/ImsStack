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
#include "ServiceMemory.h"
#include "ServiceTrace.h"

#include "FakeRegistration.h"
#include "IRegUserIdentityNotifier.h"
#include "IRegistrationListener.h"
#include "ISipConnectionNotifier.h"
#include "RegBindingProxy.h"
#include "RegFlow.h"
#include "RegObserver.h"
#include "Sip.h"
#include "SipConfigProxy.h"
#include "SipDebug.h"
#include "SipStatusCode.h"
#include "SipUrnHelper.h"
#include "base/Ims.h"

__IMS_TRACE_TAG_REG__;

PUBLIC
FakeRegistration::FakeRegistration() :
        EngineActivity(),
        m_nState(STATE_CREATED),
        m_nSubState(SUB_STATE_IDLE),
        m_pRegFlow(IMS_NULL),
        m_pRegParam(IMS_NULL),
        m_objContacts(ImsList<RegContact*>()),
        m_pStateTracker(new RegStateTracker()),
        m_piListener(IMS_NULL),
        m_piUserIdNotifier(IMS_NULL),
        m_objObservers(ImsList<RegObserver*>()),
        m_bIsWithinTrustDomain(IMS_TRUE),
        m_nRefCountForScnErrorListener(0)
{
}

PUBLIC VIRTUAL FakeRegistration::~FakeRegistration()
{
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

PUBLIC VIRTUAL IMS_BOOL FakeRegistration::Equals(IN const IRegistration* piReg) const
{
    const FakeRegistration* pReg = DYNAMIC_CAST(const FakeRegistration*, piReg);

    if (pReg == IMS_NULL)
    {
        return IMS_FALSE;
    }

    return m_pRegFlow->GetRegKey().Equals(pReg->m_pRegFlow->GetRegKey());
}

PUBLIC
IMS_BOOL FakeRegistration::Create(IN IMS_UINT32 nFlowId, IN const SipAddress& objAor,
        IN IMS_BOOL bEmergency, IN const AString& strSubsId /* = AString::ConstNull() */,
        IN SipProfile* pProfile /* = IMS_NULL*/)
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

    SetSipProfile(pProfile);

    m_pStateTracker->SetAor(objAor);
    m_pStateTracker->SetSubscriberId(strSubsId);
    m_pStateTracker->SetEmergencyRegistration(bEmergency);

    return IMS_TRUE;
}

PUBLIC
void FakeRegistration::Destroy()
{
    IMS_TRACE_D("Registration :: Destroy() - SCNEL=%d", m_nRefCountForScnErrorListener, 0, 0);

    DestroyAllContacts();
    UpdateBindingState(BINDING_DESTROY);

    PostMessage(AMSG_DESTROY, 0, 0);
}

PUBLIC
IMS_BOOL FakeRegistration::HasActiveBindings() const
{
    for (IMS_UINT32 i = 0; i < m_objContacts.GetSize(); ++i)
    {
        const RegContact* pContact = m_objContacts.GetAt(i);

        if (pContact->IsActiveBinding())
        {
            return IMS_TRUE;
        }
    }

    return IMS_FALSE;
}

PUBLIC
IMS_BOOL FakeRegistration::IsAllBindingsRemoved() const
{
    for (IMS_UINT32 i = 0; i < m_objContacts.GetSize(); ++i)
    {
        const RegContact* pContact = m_objContacts.GetAt(i);

        if (!pContact->IsEmpty())
        {
            return IMS_FALSE;
        }
    }

    return IMS_TRUE;
}

PRIVATE VIRTUAL IMS_BOOL FakeRegistration::DispatchMessage(IN ImsMessage& objMsg)
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
            break;

        case AMSG_REGISTRATION_RESPONSE_RECEIVED:
            NotifyResponse(LONG_TO_SINT(objMsg.nWparam));
            break;

        default:
            break;
    }

    return EngineActivity::DispatchMessage(objMsg);
}

PRIVATE VIRTUAL IMS_BOOL FakeRegistration::CreateBinding(
        IN const AString& strAppId, IN const AString& strServiceId)
{
    return RegBindingProxy::CreateBinding(GetSlotId(), strAppId, strServiceId, this);
}

PRIVATE VIRTUAL void FakeRegistration::DestroyBinding(
        IN const AString& strAppId, IN const AString& strServiceId)
{
    RegBindingProxy::DestroyBinding(GetSlotId(), strAppId, strServiceId);
}

PRIVATE VIRTUAL IRegContact* FakeRegistration::CreateContact(IN const IpAddress& objIpAddr,
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

    // If not present, add a new Contact information
    RegContact* pNewContact = new RegContact(GetSlotId(), objIpAddr, nPort,
            IRegContact::USER_INFO_PART_IMPU, this, (-1) /*pRegFlow->GetRegKey().GetFlowId()*/);

    if (pNewContact == IMS_NULL)
    {
        IMS_TRACE_E(0, "Creating a Contact (%s:%d) failed", SipDebug::GetIp(objIpAddr), nPort, 0);
        return IMS_NULL;
    }

    // Set user-info field
    pNewContact->SetAor(m_pStateTracker->GetAor());

    const SipProfile* pProfile = m_pStateTracker->GetSipProfile();
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

PRIVATE VIRTUAL void FakeRegistration::DestroyAllContacts()
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

PRIVATE VIRTUAL void FakeRegistration::DestroyContact(IN IRegContact* piContact)
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

PRIVATE VIRTUAL void FakeRegistration::DestroyContact(
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

PRIVATE VIRTUAL ImsList<IRegContact*> FakeRegistration::GetAllContacts() const
{
    ImsList<IRegContact*> objAllContacts;

    for (IMS_UINT32 i = 0; i < m_objContacts.GetSize(); ++i)
    {
        RegContact* pContact = m_objContacts.GetAt(i);

        objAllContacts.Append(pContact);
    }

    return objAllContacts;
}

PRIVATE VIRTUAL IRegContact* FakeRegistration::GetContact(
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

PRIVATE VIRTUAL IMS_BOOL FakeRegistration::IsBindingsUpdated() const
{
    for (IMS_UINT32 i = 0; i < m_objContacts.GetSize(); ++i)
    {
        const RegContact* pContact = m_objContacts.GetAt(i);

        if (pContact->IsBindingsUpdated())
        {
            IMS_TRACE_D("Registration bindings are updated", 0, 0, 0);
            return IMS_TRUE;
        }
    }

    return IMS_FALSE;
}

PRIVATE VIRTUAL IMS_RESULT FakeRegistration::Register(IN IMS_SINT32 nExpires /*= (-1)*/)
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

    // Sets Contact headers
    if ((nExpires == 0) || IsAllBindingsRemoved())
    {
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

        SetSubState(SUB_STATE_REGISTERING);
    }

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

    // 4 REG. OK
    PostMessage(AMSG_REGISTRATION_RESPONSE_RECEIVED, SipStatusCode::SC_200, 0);

    return IMS_SUCCESS;
}

PRIVATE VIRTUAL IMS_RESULT FakeRegistration::Deregister()
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

    SetSubState(SUB_STATE_DEREGISTERING);

    // Change the state
    UpdateBindingState(BINDING_DEREGISTERING);

    Ims::SetLastError(ImsError::NO_ERROR);

    // 4 de-REG. OK
    PostMessage(AMSG_REGISTRATION_RESPONSE_RECEIVED, SipStatusCode::SC_200, 0);

    return IMS_SUCCESS;
}

// REG_RESTORATION_FOR_ACTIVE_BINDING
PRIVATE VIRTUAL void FakeRegistration::RemoveActiveBindingsForcingly()
{
    // no-op
}

PRIVATE VIRTUAL void FakeRegistration::Restore()
{
    IMS_TRACE_I("Registration::Restore - State (%s)", StateToString(GetState()), 0, 0);

    // 4 contact handling/ connection if present

    SetState(STATE_CREATED);
    SetSubState(SUB_STATE_IDLE);
    UpdateBindingState(BINDING_RESTORE);

    // Remove all the bindings if present
    for (IMS_UINT32 i = 0; i < m_objContacts.GetSize(); ++i)
    {
        RegContact* pContact = m_objContacts.GetAt(i);

        pContact->Restore();
    }

    m_pRegFlow->Restore();
    m_pRegParam->Restore();
}

PRIVATE VIRTUAL void FakeRegistration::SetAor(
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

PRIVATE VIRTUAL void FakeRegistration::SetRefreshPolicy(IN IMS_SINT32 /*nPolicy*/,
        IN IMS_SINT32 /*nCriteriaInterval*/, IN IMS_SINT32 /*nValueEorLt*/,
        IN IMS_SINT32 /*nValueGt*/)
{
    IMS_TRACE_D("SetRefreshPolicy :: not implemented", 0, 0, 0);
}

PRIVATE VIRTUAL void FakeRegistration::SetSipProfile(IN SipProfile* pProfile)
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

PRIVATE VIRTUAL void FakeRegistration::SetBindingStateListener(
        IN IRegBindingStateListener* /*piListener*/)
{
    IMS_TRACE_D("SetBindingStateListener :: not implemented", 0, 0, 0);
}

PRIVATE VIRTUAL void FakeRegistration::SetFlagForWithinTrustDomain(IN IMS_BOOL bWithinTrustDomain)
{
    if (m_bIsWithinTrustDomain != bWithinTrustDomain)
    {
        IMS_TRACE_I("WithinTrustDomain:: %s > %s", _TRACE_B_(m_bIsWithinTrustDomain),
                _TRACE_B_(bWithinTrustDomain), 0);

        m_bIsWithinTrustDomain = bWithinTrustDomain;
    }
}

PRIVATE VIRTUAL void FakeRegistration::SetUserInfoForContactHeader(IN const AString& strUserInfo)
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

PRIVATE VIRTUAL void FakeRegistration::ConnectionNotifierError_NotifyError(
        IN ISipConnectionNotifier* piScn, IN IMS_SINT32 nCode, IN const AString& strMessage)
{
    (void)piScn;

    IMS_TRACE_D("ConnectionNotifierError_NotifyError :: code=%d, message=%s", nCode,
            strMessage.GetStr(), 0);

    if (GetState() != STATE_ACTIVE)
    {
        IMS_TRACE_D("FakeRegistration :: State is not in the ACTIVE; ignored ...", 0, 0, 0);
        return;
    }

    if (GetSubState() != SUB_STATE_IDLE)
    {
        IMS_TRACE_D("FakeRegistration :: Sub-State is not in the IDLE; ignored ...", 0, 0, 0);
        return;
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

PRIVATE VIRTUAL void FakeRegistration::AddObserver(IN RegObserver* pObserver)
{
    for (IMS_UINT32 i = 0; i < m_objObservers.GetSize(); ++i)
    {
        const RegObserver* pTmpObserver = m_objObservers.GetAt(i);

        if (pObserver == pTmpObserver)
        {
            return;
        }
    }

    m_objObservers.Append(pObserver);
}

PRIVATE VIRTUAL void FakeRegistration::RemoveObserver(IN RegObserver* pObserver)
{
    for (IMS_UINT32 i = 0; i < m_objObservers.GetSize(); ++i)
    {
        const RegObserver* pTmpObserver = m_objObservers.GetAt(i);

        if (pObserver == pTmpObserver)
        {
            m_objObservers.RemoveAt(i);
            return;
        }
    }
}

PRIVATE VIRTUAL IMS_SINT32 FakeRegistration::AddReferenceForScnErrorListener()
{
#ifdef __IMS_DEBUG__
    IMS_TRACE_D("Registration :: SCNEL (add) - %d >> %d", m_nRefCountForScnErrorListener,
            (m_nRefCountForScnErrorListener + 1), 0);
#endif

    ++m_nRefCountForScnErrorListener;

    return m_nRefCountForScnErrorListener;
}

PRIVATE VIRTUAL IMS_SINT32 FakeRegistration::RemoveReferenceForScnErrorListener()
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

PRIVATE VIRTUAL void FakeRegistration::RegCapabilityChange_ServiceAdded(
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

PRIVATE VIRTUAL void FakeRegistration::RegCapabilityChange_ServiceRemoved(
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

PRIVATE
void FakeRegistration::CallListener(
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
void FakeRegistration::ChoosePreferredContact()
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
IMS_SINT32 FakeRegistration::GetPortUc() const
{
    IMS_SINT32 nPortUc = m_pRegParam->GetProtectedPortUc();

    // Check if the UE supports the Security Association Agreement.
    // If nPortUC returns Sip::PORT_UNSPECIFIED, then it does not support "sec-agree".
    if (nPortUc == Sip::PORT_UNSPECIFIED)
    {
        return GetPortUs();
    }

    if (!SipConfigProxy::IsIpSecConfigured(GetSlotId(), m_pStateTracker->GetSipProfile()))
    {
        if (nPortUc == 0)
        {
            return GetPortUs();
        }
    }

    return nPortUc;
}

PRIVATE
IMS_SINT32 FakeRegistration::GetPortUs() const
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
void FakeRegistration::NotifyResponse(IN IMS_SINT32 nStatusCode)
{
    IMS_TRACE_I("Registration - %d response to REGISTER received", nStatusCode, 0, 0);

    if (SipStatusCode::Is1XX(nStatusCode))
    {
        return;
    }

    IMS_SINT32 nPrevState = GetState();
    IMS_SINT32 nPrevSubState = GetSubState();

    // Authentication challenged by S-CSCF
    if ((nStatusCode == SipStatusCode::SC_401) || (nStatusCode == SipStatusCode::SC_407))
    {
        PostMessage(AMSG_REGISTRATION_RESPONSE_RECEIVED, SipStatusCode::SC_200, 0);
        return;
    }

    // Reset the flags after the transaction completed
    SetSubState(SUB_STATE_IDLE);

    // Handle the response except for 401/407
    if (SipStatusCode::IsFinalSuccess(nStatusCode))
    {
        // Update the sequence number
        UpdateCSeqNumber();

        // Update the bindings
        IMS_RESULT nBindingUpdateResult = UpdateBindings(IsAllBindingsRemoved());
        IMS_SINT32 nReason = REASON_NONE;

        if (nBindingUpdateResult == BINDING_UPDATE_NO_EXPIRES)
        {
            nReason = REASON_NO_EXPIRES;
        }

        // Store the persistent header information
        StorePersistentHeaders();

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
        // Update the sequence number
        UpdateCSeqNumber();

        if (nPrevState == STATE_INIT)
        {
            SetState(STATE_CREATED);
        }

        UpdateBindingState(BINDING_RESULT_NOK);

        CallListener(nPrevState, nPrevSubState, REASON_STATUS_CODE);
    }
}

PRIVATE
void FakeRegistration::SetState(IN IMS_SINT32 nState)
{
    IMS_TRACE_I("Registration (%s) :: %s to %s",
            SipDebug::GetUri1(m_pStateTracker->GetAor().GetUri()).GetStr(), StateToString(m_nState),
            StateToString(nState));

    m_nState = nState;
}

PRIVATE
void FakeRegistration::SetSubState(IN IMS_SINT32 nSubState)
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
void FakeRegistration::StorePersistentHeaders()
{
    // Stores the public user identities
    AStringArray objTmpArray;

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

    // Stores the Service-Route from the preloaded route set
    m_pStateTracker->SetServiceRoutes(m_pRegParam->GetPreloadedRoutes());

    // Updates the protected client /server port (uc / us)
    m_pStateTracker->SetPortUc(GetPortUc());
    m_pStateTracker->SetPortUs(GetPortUs());

    // Security-Client / Security-Verify headers
    m_pStateTracker->SetSecurityClients(m_pRegParam->GetSecurityClients());
    m_pStateTracker->SetSecurityVerifys(m_pRegParam->GetSecurityVerifys());

    m_pStateTracker->AdjustRegistrationDedicatedParameters();
}

PRIVATE
IMS_RESULT FakeRegistration::UpdateBindings(IN IMS_BOOL bIsContactWildcard /*= IMS_FALSE*/)
{
    if (bIsContactWildcard)
    {
        // Only for de-registration procedure
        // So, all the registration contacts transit to "STATE_TERMINATED"
        for (IMS_UINT32 j = 0; j < m_objContacts.GetSize(); ++j)
        {
            RegContact* pContact = m_objContacts.GetAt(j);

            pContact->SetTerminated();
        }

        return BINDING_UPDATE_OK;
    }

    IMS_RESULT nBindingUpdateResult = BINDING_UPDATE_OK;
    IMS_BOOL bAtLeastOneContactUpdated = IMS_FALSE;

    // For each registration contact, updates the parameter ('expires')
    // If no match found, it transits to "STATE_TERMINATED"
    for (IMS_UINT32 j = 0; j < m_objContacts.GetSize(); ++j)
    {
        RegContact* pContact = m_objContacts.GetAt(j);
        IMS_SINT32 nUpdateResult = pContact->UpdateParameter(pContact->GetExpires());

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

    return nBindingUpdateResult;
}

PRIVATE
void FakeRegistration::UpdateBindingState(IN IMS_SINT32 nState)
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

PRIVATE
void FakeRegistration::UpdateCSeqNumber()
{
    // Increase the CSeq number after the transaction is completed
    IMS_SINT32 nSeqNum = m_pRegFlow->IncreaseNGetCSeqValue(0);

    IMS_TRACE_I("Registration - Sequence Number Changed (%d >> %d)", nSeqNum, (nSeqNum + 1), 0);

    // Store the current sequence number (last one) from the last successful response.
    m_pRegFlow->SetCSeqValue(nSeqNum + 1);
}

PRIVATE GLOBAL const IMS_CHAR* FakeRegistration::StateToString(IN IMS_SINT32 nState)
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

PRIVATE GLOBAL const IMS_CHAR* FakeRegistration::SubStateToString(IN IMS_SINT32 nSubState)
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
