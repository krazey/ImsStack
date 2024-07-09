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

#include "IRegBindingListener.h"
#include "IRegContact.h"
#include "IRegParameter.h"
#include "IRegistrationEx.h"
#include "ISipConnectionNotifier.h"
#include "RegBinding.h"
#include "RegInfo.h"
#include "RegStateTracker.h"
#include "RegistrationContext.h"
#include "Sip.h"
#include "SipConfigProxy.h"
#include "SipDebug.h"
#include "util/ISipConnectionNotifierManager.h"

__IMS_TRACE_TAG_REG__;

PUBLIC
RegBinding::RegBinding() :
        RegObserver(),
        m_piRegEx(IMS_NULL),
        m_piContact(IMS_NULL),
        m_nState(STATE_CREATED),
        m_piScn(IMS_NULL),
        m_piListener(IMS_NULL),
        m_bDeregistrationFailed(IMS_FALSE)
{
}

PROTECTED VIRTUAL RegBinding::~RegBinding()
{
    DestroySipConnectionNotifier();
}

PUBLIC
void RegBinding::Destroy()
{
    if (m_piListener != IMS_NULL)
    {
        m_piListener->RegBinding_OnDestroy();
    }

    if (m_piRegEx != IMS_NULL)
    {
        m_piRegEx->RemoveObserver(this);
    }

    delete this;
}

PUBLIC
IMS_BOOL RegBinding::Create(IN IRegistrationEx* piRegEx)
{
    m_piRegEx = piRegEx;

    if (m_piRegEx != IMS_NULL)
    {
        m_piRegEx->AddObserver(this);
    }

    SetState(STATE_INIT);

    if (m_piListener != IMS_NULL)
    {
        if (m_piRegEx != IMS_NULL)
        {
            const SipAddress& objAor = m_piRegEx->GetStateTracker()->GetAor();

            m_piListener->RegBinding_OnInit(&objAor);
        }
        else
        {
            m_piListener->RegBinding_OnInit(IMS_NULL);
        }
    }

    return IMS_TRUE;
}

PUBLIC
void RegBinding::QueryCapability(OUT CallerCapability*& pCapability) const
{
    if (m_piListener == IMS_NULL)
    {
        pCapability = IMS_NULL;
        return;
    }

    m_piListener->RegBinding_OnQueryCapability(pCapability);
}

PUBLIC
void RegBinding::QueryRegistrationHeaders(OUT AStringArray& objHeaders) const
{
    if (m_piListener == IMS_NULL)
    {
        return;
    }

    m_piListener->RegBinding_OnQueryRegistrationHeaders(objHeaders);
}

PUBLIC
void RegBinding::UpdateContact(IN IRegContact* piContact)
{
    m_piContact = piContact;

    // RE_REG_BY_CAPABILITY_CHANGE
    // Checks if the reg-contact is already registered or not
    if ((m_piContact != IMS_NULL) && m_piContact->IsActiveBinding())
    {
        // Even if the UE fails the de-registration,
        // the application can restore the registration state to ACTIVE state.
        if (m_bDeregistrationFailed && (GetState() == STATE_TERMINATED))
        {
            m_bDeregistrationFailed = IMS_FALSE;

            SetState(STATE_ACTIVE);

            if (m_piListener != IMS_NULL)
            {
                m_piListener->RegBinding_OnActive();
            }
        }

        CreateSipConnectionNotifier();
    }
}

PROTECTED VIRTUAL void RegBinding::Update(IN IMS_SINT32 nWhat)
{
    if (m_piRegEx == IMS_NULL)
    {
        return;
    }

    IMS_SINT32 nState = GetState();

    switch (nWhat)
    {
        case IRegistrationEx::BINDING_REGISTERING:
            if (nState == STATE_INIT)
            {
                SetState(STATE_INIT_PENDING);
            }
            else if (nState == STATE_ACTIVE)
            {
                if (m_piContact != IMS_NULL)
                {
                    SetState(STATE_ACTIVE_PENDING);

                    if (m_piRegEx != IMS_NULL && m_piRegEx->IsActiveBindingsRestorationEnabled())
                    {
                        // If the server connection was lost by some reasons and re-REGISTER
                        // request is being sent, this silently restores the server connection
                        // using the connection that was used by the REGISTER transaction.
                        RestoreTransportResourceForServerConnection(IMS_FALSE);
                    }
                }
                else
                {
                    SetState(STATE_ACTIVE_TERMINATING);
                }
            }
            else
            {
                IMS_TRACE_D("REGISTERING is ignored in %s", StateToString(nState), 0, 0);
            }
            break;
        case IRegistrationEx::BINDING_DEREGISTERING:
            if (nState == STATE_ACTIVE)
            {
                SetState(STATE_ACTIVE_TERMINATING);
            }
            else
            {
                IMS_TRACE_D("DEREGISTERING is ignored in %s", StateToString(nState), 0, 0);
            }
            break;
        case IRegistrationEx::BINDING_RESULT_OK:
            if (nState == STATE_INIT_PENDING)
            {
                SetState(STATE_ACTIVE);

                if (m_piListener != IMS_NULL)
                {
                    m_piListener->RegBinding_OnActive();
                }

                CreateSipConnectionNotifier();
            }
            else if (nState == STATE_ACTIVE_PENDING)
            {
                SetState(STATE_ACTIVE);

                if (m_piScn != IMS_NULL)
                {
                    // RFC5626_FLOW_CONTROL
                    m_piScn->UpdatePortFlowControl(GetPortFlowControl());
                    m_piScn->UpdatePortUc(GetPortUc());
                }

                if (m_piListener != IMS_NULL)
                {
                    m_piListener->RegBinding_OnActive();
                }
            }
            else if (nState == STATE_ACTIVE_TERMINATING)
            {
                SetState(STATE_TERMINATED);

                if (m_piListener != IMS_NULL)
                {
                    m_piListener->RegBinding_OnTerminated();
                }

                DestroySipConnectionNotifier();
            }
            else
            {
                IMS_TRACE_D("RESULT_OK is ignored in %s", StateToString(nState), 0, 0);
            }
            break;
        case IRegistrationEx::BINDING_RESULT_NOK:
            if (nState == STATE_INIT_PENDING)
            {
                SetState(STATE_INIT);
            }
            else if (nState == STATE_ACTIVE_PENDING)
            {
                SetState(STATE_ACTIVE);
            }
            else if (nState == STATE_ACTIVE_TERMINATING)
            {
                m_bDeregistrationFailed = IMS_TRUE;

                SetState(STATE_TERMINATED);

                if (m_piListener != IMS_NULL)
                {
                    m_piListener->RegBinding_OnTerminated();
                }

                DestroySipConnectionNotifier();
            }
            else
            {
                IMS_TRACE_D("RESULT_NOK is ignored in %s", StateToString(nState), 0, 0);
            }
            break;
        case IRegistrationEx::BINDING_RESTORE:
            m_bDeregistrationFailed = IMS_FALSE;

            SetState(STATE_INIT);

            if (m_piListener != IMS_NULL)
            {
                m_piListener->RegBinding_OnTerminated();
            }

            DestroySipConnectionNotifier();
            break;
        // REG_RESTORATION_FOR_ACTIVE_BINDING
        case IRegistrationEx::BINDING_RESTORE_ACTIVE_BINDINGS:
            if ((nState == STATE_INIT) || (nState == STATE_TERMINATED))
            {
                SetState(STATE_ACTIVE);
            }

            if ((m_piListener != IMS_NULL) && (GetState() == STATE_ACTIVE))
            {
                m_piListener->RegBinding_OnActive();
            }

            CreateSipConnectionNotifier();
            RestoreTransportResourceForClientInitiatedConnection();
            RestoreTransportResourceForServerConnection();
            break;
        case IRegistrationEx::BINDING_DESTROY_CONTACT:
            SetState(STATE_TERMINATED);

            if (m_piListener != IMS_NULL)
            {
                m_piListener->RegBinding_OnTerminated();
            }

            DestroySipConnectionNotifier();
            break;
        case IRegistrationEx::BINDING_DESTROY:
            SetState(STATE_TERMINATED);

            if (m_piListener != IMS_NULL)
            {
                m_piListener->RegBinding_OnTerminated();
            }

            DestroySipConnectionNotifier();

            m_piRegEx = IMS_NULL;
            break;
        default:
            break;
    }
}

PROTECTED VIRTUAL const AStringArray& RegBinding::GetAssociatedUris() const
{
    if (!IsBindingActive())
    {
        return AStringArray::ConstNull();
    }

    return (m_piRegEx != IMS_NULL) ? m_piRegEx->GetStateTracker()->GetAssociatedUris()
                                   : AStringArray::ConstNull();
}

PROTECTED VIRTUAL const SipAddress& RegBinding::GetAuthorizedAor() const
{
    if (!IsBindingActive())
    {
        return SipAddress::ConstNull();
    }

    return (m_piRegEx != IMS_NULL) ? m_piRegEx->GetStateTracker()->GetAuthorizedAor()
                                   : SipAddress::ConstNull();
}

PROTECTED VIRTUAL const SipAddress& RegBinding::GetContactAddress() const
{
    if (m_piRegEx == IMS_NULL)
    {
        return SipAddress::ConstNull();
    }

    if (m_piContact == IMS_NULL)
    {
        return SipAddress::ConstNull();
    }

    return m_piContact->GetContactAddress();
}

PROTECTED VIRTUAL const SipAddress* RegBinding::GetContactAddressForOutgoingMessage() const
{
    if (m_piRegEx == IMS_NULL)
    {
        return IMS_NULL;
    }

    const RegStateTracker* pStateTracker = m_piRegEx->GetStateTracker();

    if (pStateTracker == IMS_NULL)
    {
        return IMS_NULL;
    }

    return pStateTracker->GetContactAddressForOutgoingMessage();
}

PROTECTED VIRTUAL const IpAddress& RegBinding::GetIpAddress() const
{
    if (m_piRegEx == IMS_NULL)
    {
        return IpAddress::NONE;
    }

    if (m_piContact == IMS_NULL)
    {
        return IpAddress::NONE;
    }

    return m_piContact->GetIpAddress();
}

PROTECTED VIRTUAL const AStringArray& RegBinding::GetPathHeaders() const
{
    if (!IsBindingActive())
    {
        return AStringArray::ConstNull();
    }

    return (m_piRegEx != IMS_NULL) ? m_piRegEx->GetStateTracker()->GetPathHeaders()
                                   : AStringArray::ConstNull();
}

PROTECTED VIRTUAL IMS_SINT32 RegBinding::GetPortFlowControl() const
{
    return (m_piRegEx != IMS_NULL) ? m_piRegEx->GetStateTracker()->GetPortFlowControl()
                                   : Sip::PORT_UNSPECIFIED;
}

PROTECTED VIRTUAL IMS_SINT32 RegBinding::GetPortUc() const
{
    return (m_piRegEx != IMS_NULL) ? m_piRegEx->GetStateTracker()->GetPortUc()
                                   : SipConfigProxy::GetPort(IMS_SLOT_0);
}

PROTECTED VIRTUAL IMS_SINT32 RegBinding::GetPortUs() const
{
    return (m_piRegEx != IMS_NULL) ? m_piRegEx->GetStateTracker()->GetPortUs()
                                   : SipConfigProxy::GetPort(IMS_SLOT_0);
}

PROTECTED VIRTUAL const IRegInfo* RegBinding::GetRegInfo() const
{
    return (m_piRegEx != IMS_NULL) ? m_piRegEx->GetRegInfo() : IMS_NULL;
}

PROTECTED VIRTUAL const AStringArray& RegBinding::GetSecurityClients() const
{
    if (!IsBindingActive())
    {
        return AStringArray::ConstNull();
    }

    return (m_piRegEx != IMS_NULL) ? m_piRegEx->GetStateTracker()->GetSecurityClients()
                                   : AStringArray::ConstNull();
}

PROTECTED VIRTUAL const AStringArray& RegBinding::GetSecurityVerifys() const
{
    if (!IsBindingActive())
    {
        return AStringArray::ConstNull();
    }

    return (m_piRegEx != IMS_NULL) ? m_piRegEx->GetStateTracker()->GetSecurityVerifys()
                                   : AStringArray::ConstNull();
}

PROTECTED VIRTUAL const AStringArray& RegBinding::GetServiceRoutes() const
{
    if (!IsBindingActive())
    {
        return AStringArray::ConstNull();
    }

    return (m_piRegEx != IMS_NULL) ? m_piRegEx->GetStateTracker()->GetServiceRoutes()
                                   : AStringArray::ConstNull();
}

PROTECTED VIRTUAL SipProfile* RegBinding::GetSipProfile() const
{
    return (m_piRegEx != IMS_NULL) ? m_piRegEx->GetStateTracker()->GetSipProfile() : IMS_NULL;
}

PROTECTED VIRTUAL const AString& RegBinding::GetSubscriberId() const
{
    return (m_piRegEx != IMS_NULL) ? m_piRegEx->GetStateTracker()->GetSubscriberId()
                                   : AString::ConstNull();
}

PROTECTED VIRTUAL IMS_SINT32 RegBinding::GetTransportExt() const
{
    return (m_piRegEx != IMS_NULL) ? m_piRegEx->GetStateTracker()->GetTransportExt()
                                   : Sip::TRANSPORT_EXT_ANY;
}

PROTECTED VIRTUAL const SipParameter* RegBinding::GetInstanceParameter() const
{
    if (m_piRegEx == IMS_NULL)
    {
        return IMS_NULL;
    }

    if (m_piContact == IMS_NULL)
    {
        return IMS_NULL;
    }

    return m_piContact->GetInstanceParameter();
}

PROTECTED VIRTUAL const SipAddress* RegBinding::GetPublicGruu() const
{
    if (m_piRegEx == IMS_NULL)
    {
        return IMS_NULL;
    }

    if (m_piContact == IMS_NULL)
    {
        return IMS_NULL;
    }

    return m_piContact->GetPublicGruu();
}

PROTECTED VIRTUAL const SipAddress* RegBinding::GetTemporaryGruu() const
{
    if (m_piRegEx == IMS_NULL)
    {
        return IMS_NULL;
    }

    if (m_piContact == IMS_NULL)
    {
        return IMS_NULL;
    }

    return m_piContact->GetTemporaryGruu();
}

PROTECTED VIRTUAL const ImsList<SipAddress*>& RegBinding::GetTemporaryGruus() const
{
    if (m_piRegEx == IMS_NULL)
    {
        return SipAddress::ConstEmptyList();
    }

    if (m_piContact == IMS_NULL)
    {
        return SipAddress::ConstEmptyList();
    }

    return m_piContact->GetTemporaryGruus();
}

PROTECTED VIRTUAL IMS_BOOL RegBinding::IsBehindNat() const
{
    return (m_piRegEx != IMS_NULL) ? m_piRegEx->IsBehindNat() : IMS_FALSE;
}

PROTECTED VIRTUAL IMS_BOOL RegBinding::IsWithinTrustDomain() const
{
    return (m_piRegEx != IMS_NULL) ? m_piRegEx->IsWithinTrustDomain() : IMS_FALSE;
}

PROTECTED VIRTUAL void RegBinding::NotifyCallerCapabilityChanged()
{
    if (m_piRegEx == IMS_NULL)
    {
        return;
    }

    m_piRegEx->NotifyCallerCapabilityChanged();
}

PROTECTED VIRTUAL void RegBinding::SetListener(IN IRegBindingListener* piListener)
{
    m_piListener = piListener;
}

PRIVATE
void RegBinding::CreateSipConnectionNotifier()
{
    ISipConnectionNotifierManager* piScnManager =
            RegistrationContext::GetInstance()->GetSipConnectionNotifierManager();

    if (m_piRegEx == IMS_NULL)
    {
        IMS_TRACE_E(0, "No registration", 0, 0, 0);
        return;
    }

    if (m_piContact == IMS_NULL)
    {
        IMS_TRACE_E(0, "No contacts", 0, 0, 0);
        return;
    }

    // RE_REG_BY_CAPABILITY_CHANGE
    ISipConnectionNotifier* piTempScn = m_piScn;

    // MULTI_REG_TRANSPORT
    AString strParams = AString::ConstNull();
    IMS_SINT32 nTransportExt = m_piRegEx->GetStateTracker()->GetTransportExt();

    if (nTransportExt != Sip::TRANSPORT_EXT_ANY)
    {
        strParams.Sprintf("%s=%d", Sip::STR_TRANSPORT_EXT, nTransportExt);
    }

    // RFC5626_FLOW_CONTROL : GetPortFlowControl()
    m_piScn = piScnManager->CreateConnectionNotifier(m_piContact->GetContactAddress().GetScheme(),
            m_piContact->GetIpAddress(), GetPortUs(), GetPortUc(), GetPortFlowControl(), strParams,
            m_piRegEx->GetStateTracker()->GetAuthorizedAor());

    if ((m_piScn != IMS_NULL) && (m_piScn != piTempScn))
    {
        if (m_piRegEx->AddReferenceForScnErrorListener() == 1)
        {
            m_piScn->AddErrorListener(m_piRegEx);

            // RFC5626_FLOW_CONTROL
            if (Sip::IsPortSpecified(GetPortFlowControl()))
            {
                RestoreTransportResourceForClientInitiatedConnection();
            }
        }
    }

    // MULTI_REG_SIP_PROFILE
    if (m_piScn != IMS_NULL)
    {
        m_piScn->SetSipProfile(GetSipProfile());
    }
    else
    {
        m_piRegEx->ConnectionNotifierError_NotifyError(
                m_piScn, ISipConnectionNotifier::TRANSPORT_ERROR_UDP_SERVER, "SCN is not created");
    }

    // RE_REG_BY_CAPABILITY_CHANGE
    if (piTempScn != IMS_NULL)
    {
        if (m_piScn != piTempScn)
        {
            m_piRegEx->RemoveReferenceForScnErrorListener();
        }

        piScnManager->ReleaseConnectionNotifier(piTempScn);
    }
}

PRIVATE
void RegBinding::DestroySipConnectionNotifier()
{
    if (m_piScn == IMS_NULL)
    {
        return;
    }

    if ((m_piRegEx != IMS_NULL) && (m_piRegEx->RemoveReferenceForScnErrorListener() == 0))
    {
        m_piScn->RemoveErrorListener(m_piRegEx);
    }

    RegistrationContext::GetInstance()
            ->GetSipConnectionNotifierManager()
            ->ReleaseConnectionNotifier(m_piScn);
    m_piScn = IMS_NULL;
}

PRIVATE
IMS_BOOL RegBinding::IsBindingActive() const
{
    IMS_SINT32 nState = GetState();

    return (nState == STATE_ACTIVE) || (nState == STATE_ACTIVE_PENDING) ||
            (nState == STATE_ACTIVE_TERMINATING);
}

// REG_RESTORATION_FOR_ACTIVE_BINDING
PRIVATE
void RegBinding::RestoreTransportResourceForClientInitiatedConnection()
{
    if ((m_piRegEx == IMS_NULL) || (m_piScn == IMS_NULL))
    {
        return;
    }

    const IMS_SINT32 nTransportResource =
            ISipConnectionNotifier::TRANSPORT_CLIENT_INITIATED_CONNECTION;

    if (m_piScn->IsTransportResourceReserved(nTransportResource))
    {
        return;
    }

    IRegParameter* piRegParam = m_piRegEx->GetParameter();

    if (piRegParam == IMS_NULL)
    {
        m_piRegEx->ConnectionNotifierError_NotifyError(m_piScn,
                ISipConnectionNotifier::TRANSPORT_ERROR_TCP_CLIENT,
                "Restoration of TCP client connection is failed");
        return;
    }

    const SipAddress& objRoute = piRegParam->GetTopmostRouteAddress();
    IpAddress objPeerIpAddr(objRoute.GetHost());

    if (m_piScn->RestoreTransportResource(nTransportResource, objPeerIpAddr, objRoute.GetPort()) !=
            IMS_SUCCESS)
    {
        m_piRegEx->ConnectionNotifierError_NotifyError(m_piScn,
                ISipConnectionNotifier::TRANSPORT_ERROR_TCP_CLIENT,
                "Restoration of TCP client connection is failed");
    }
}

PRIVATE
void RegBinding::RestoreTransportResourceForServerConnection(
        IN IMS_BOOL bNotifyError /*= IMS_TRUE*/)
{
    if ((m_piRegEx == IMS_NULL) || (m_piScn == IMS_NULL))
    {
        return;
    }

    const IMS_SINT32 nTransportResource = ISipConnectionNotifier::TRANSPORT_SERVER_CONNECTION;

    if (m_piScn->IsTransportResourceReserved(nTransportResource))
    {
        return;
    }

    if (m_piScn->RestoreTransportResource(nTransportResource, IpAddress::NONE, 0) != IMS_SUCCESS)
    {
        if (bNotifyError)
        {
            m_piRegEx->ConnectionNotifierError_NotifyError(m_piScn,
                    ISipConnectionNotifier::TRANSPORT_ERROR_UDP_SERVER,
                    "Restoration of server connection is failed");
        }
    }
}

PRIVATE
void RegBinding::SetState(IN IMS_SINT32 nState)
{
    IMS_TRACE_I("RegBinding (%s) :: %s to %s",
            SipDebug::GetUri1(GetAuthorizedAor().GetUri()).GetStr(), StateToString(m_nState),
            StateToString(nState));

    m_nState = nState;
}

PRIVATE GLOBAL const IMS_CHAR* RegBinding::StateToString(IN IMS_SINT32 nState)
{
    switch (nState)
    {
        case STATE_CREATED:
            return "STATE_CREATED";
        case STATE_INIT:
            return "STATE_INIT";
        case STATE_INIT_PENDING:
            return "STATE_INIT_PENDING";
        case STATE_ACTIVE:
            return "STATE_ACTIVE";
        case STATE_ACTIVE_PENDING:
            return "STATE_ACTIVE_PENDING";
        case STATE_ACTIVE_TERMINATING:
            return "STATE_ACTIVE_TERMINATING";
        case STATE_TERMINATED:
            return "STATE_TERMINATED";
        default:
            return "__INVALID__";
    }
}
