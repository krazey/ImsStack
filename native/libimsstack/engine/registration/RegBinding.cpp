/*
    Author
    <table>
    date      author                    description
    --------  --------------            ----------
    20100912  hwangoo.park@             Created
    </table>

    Description

*/

#include "ServiceMemory.h"
#include "ServiceTrace.h"
#include "Sip.h"
#include "SipDebug.h"
#include "ISipConnectionNotifier.h"
#include "util/SIPConnectionNotifierManager.h"
#include "SipConfigProxy.h"
#include "IRegistrationEx.h"
#include "IRegContact.h"
// REG_RESTORATION_FOR_ACTIVE_BINDING
#include "IRegParameter.h"
#include "RegInfo.h"
#include "RegStateTracker.h"
#include "IRegBindingListener.h"
#include "RegBinding.h"

__IMS_TRACE_TAG_REG__;

PUBLIC
RegBinding::RegBinding() :
        RegObserver(),
        piRegEx(IMS_NULL),
        piContact(IMS_NULL),
        nState(STATE_CREATED),
        piSCN(IMS_NULL),
        piListener(IMS_NULL),
        bDeregistrationNOK(IMS_FALSE)
{
}

PROTECTED VIRTUAL RegBinding::~RegBinding()
{
    DestroySIPConnectionNotifier();
}

PUBLIC
void RegBinding::Destroy()
{
    //---------------------------------------------------------------------------------------------

    if (piListener != IMS_NULL)
    {
        piListener->RegBinding_OnDestroy();
    }

    if (piRegEx != IMS_NULL)
    {
        piRegEx->RemoveObserver(this);
    }

    delete this;
}

PUBLIC
IMS_BOOL RegBinding::Create(IN IRegistrationEx* piRegEx)
{
    //---------------------------------------------------------------------------------------------

    this->piRegEx = piRegEx;

    if (this->piRegEx != IMS_NULL)
    {
        this->piRegEx->AddObserver(this);
    }

    SetState(STATE_INIT);

    if (piListener != IMS_NULL)
    {
        if (this->piRegEx != IMS_NULL)
        {
            const SipAddress& objAOR = this->piRegEx->GetStateTracker()->GetAOR();

            piListener->RegBinding_OnInit(&objAOR);
        }
        else
        {
            piListener->RegBinding_OnInit(IMS_NULL);
        }
    }

    return IMS_TRUE;
}

PUBLIC
IMS_BOOL RegBinding::IsSameContact(IN IRegContact* piContact) const
{
    //---------------------------------------------------------------------------------------------

    if (this->piContact != piContact)
    {
        return IMS_FALSE;
    }

    return IMS_TRUE;
}

PUBLIC
IMS_BOOL RegBinding::IsSameRegistration(IN IRegistrationEx* piRegEx) const
{
    //---------------------------------------------------------------------------------------------

    if (this->piRegEx != piRegEx)
    {
        return IMS_FALSE;
    }

    return IMS_TRUE;
}

PUBLIC
void RegBinding::QueryCapability(OUT CallerCapability*& pCapability) const
{
    //---------------------------------------------------------------------------------------------

    if (piListener == IMS_NULL)
    {
        pCapability = IMS_NULL;
        return;
    }

    piListener->RegBinding_OnQueryCapability(pCapability);
}

PUBLIC
void RegBinding::QueryRegistrationHeaders(OUT AStringArray& objHeaders) const
{
    //---------------------------------------------------------------------------------------------

    if (piListener == IMS_NULL)
    {
        return;
    }

    piListener->RegBinding_OnQueryRegistrationHeaders(objHeaders);
}

PUBLIC
void RegBinding::UpdateContact(IN IRegContact* piContact)
{
    //---------------------------------------------------------------------------------------------

    this->piContact = piContact;

    // RE_REG_BY_CAPABILITY_CHANGE
    // Checks if the reg-contact is already registered or not
    if ((this->piContact != IMS_NULL) && this->piContact->IsActiveBinding())
    {
        // Even if the UE fails the de-registration,
        // the application can restore the registration state to ACTIVE state.
        if (bDeregistrationNOK && (GetState() == STATE_TERMINATED))
        {
            bDeregistrationNOK = IMS_FALSE;

            SetState(STATE_ACTIVE);

            if (piListener != IMS_NULL)
            {
                piListener->RegBinding_OnActive();
            }
        }

        CreateSIPConnectionNotifier();
    }
}

PROTECTED VIRTUAL void RegBinding::Update(IN IMS_SINT32 nWhat)
{
    //---------------------------------------------------------------------------------------------

    if (piRegEx == IMS_NULL)
        return;

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
                if (piContact != IMS_NULL)
                {
                    SetState(STATE_ACTIVE_PENDING);
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

                if (piListener != IMS_NULL)
                {
                    piListener->RegBinding_OnActive();
                }

                CreateSIPConnectionNotifier();
            }
            else if (nState == STATE_ACTIVE_PENDING)
            {
                SetState(STATE_ACTIVE);

                if (piSCN != IMS_NULL)
                {
                    // RFC5626_FLOW_CONTROL
                    piSCN->UpdatePortFlowControl(GetPortFlowControl());
                    piSCN->UpdatePortUc(GetPortUC());
                }

                if (piListener != IMS_NULL)
                {
                    piListener->RegBinding_OnActive();
                }
            }
            else if (nState == STATE_ACTIVE_TERMINATING)
            {
                SetState(STATE_TERMINATED);

                if (piListener != IMS_NULL)
                {
                    piListener->RegBinding_OnTerminated();
                }

                DestroySIPConnectionNotifier();
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
                bDeregistrationNOK = IMS_TRUE;

                SetState(STATE_TERMINATED);

                if (piListener != IMS_NULL)
                {
                    piListener->RegBinding_OnTerminated();
                }

                DestroySIPConnectionNotifier();
            }
            else
            {
                IMS_TRACE_D("RESULT_NOK is ignored in %s", StateToString(nState), 0, 0);
            }
            break;

        case IRegistrationEx::BINDING_RESTORE:
            bDeregistrationNOK = IMS_FALSE;

            SetState(STATE_INIT);

            if (piListener != IMS_NULL)
            {
                piListener->RegBinding_OnTerminated();
            }

            DestroySIPConnectionNotifier();
            break;

        // REG_RESTORATION_FOR_ACTIVE_BINDING
        case IRegistrationEx::BINDING_RESTORE_ACTIVE_BINDINGS:
            if ((nState == STATE_INIT) || (nState == STATE_TERMINATED))
            {
                SetState(STATE_ACTIVE);
            }

            if ((piListener != IMS_NULL) && (GetState() == STATE_ACTIVE))
            {
                piListener->RegBinding_OnActive();
            }

            CreateSIPConnectionNotifier();
            RestoreTransportResourceForClientInitiatedConnection();
            RestoreTransportResourceForServerConnection();
            break;

        case IRegistrationEx::BINDING_DESTROY_CONTACT:
            SetState(STATE_TERMINATED);

            if (piListener != IMS_NULL)
            {
                piListener->RegBinding_OnTerminated();
            }

            DestroySIPConnectionNotifier();
            break;

        case IRegistrationEx::BINDING_DESTROY:
            SetState(STATE_TERMINATED);

            if (piListener != IMS_NULL)
            {
                piListener->RegBinding_OnTerminated();
            }

            DestroySIPConnectionNotifier();

            piRegEx = IMS_NULL;
            break;

        default:
            break;
    }
}

PROTECTED VIRTUAL const AStringArray& RegBinding::GetAssociatedURIs() const
{
    //---------------------------------------------------------------------------------------------

    if (!IsBindingActive())
    {
        return AStringArray::ConstNull();
    }

    return (piRegEx != IMS_NULL) ? piRegEx->GetStateTracker()->GetAssociatedURIs()
                                 : AStringArray::ConstNull();
}

PROTECTED VIRTUAL const SipAddress& RegBinding::GetAuthorizedAOR() const
{
    //---------------------------------------------------------------------------------------------

    if (!IsBindingActive())
    {
        return SipAddress::ConstNull();
    }

    return (piRegEx != IMS_NULL) ? piRegEx->GetStateTracker()->GetAuthorizedAOR()
                                 : SipAddress::ConstNull();
}

PROTECTED VIRTUAL const SipAddress& RegBinding::GetContactAddress() const
{
    //---------------------------------------------------------------------------------------------

    if (piRegEx == IMS_NULL)
    {
        return SipAddress::ConstNull();
    }

    if (piContact == IMS_NULL)
    {
        return SipAddress::ConstNull();
    }

    return piContact->GetContactAddress();
}

PROTECTED VIRTUAL const SipAddress* RegBinding::GetContactAddressForOutgoingMessage() const
{
    //---------------------------------------------------------------------------------------------

    if (piRegEx == IMS_NULL)
    {
        return IMS_NULL;
    }

    const RegStateTracker* pStateTracker = piRegEx->GetStateTracker();

    if (pStateTracker == IMS_NULL)
    {
        return IMS_NULL;
    }

    return pStateTracker->GetContactAddressForOutgoingMessage();
}

PROTECTED VIRTUAL const IPAddress& RegBinding::GetIPAddress() const
{
    //---------------------------------------------------------------------------------------------

    if (piRegEx == IMS_NULL)
    {
        return IPAddress::NONE;
    }

    if (piContact == IMS_NULL)
    {
        return IPAddress::NONE;
    }

    return piContact->GetIPAddress();
}

PROTECTED VIRTUAL const AStringArray& RegBinding::GetPathHeaders() const
{
    //---------------------------------------------------------------------------------------------

    if (!IsBindingActive())
    {
        return AStringArray::ConstNull();
    }

    return (piRegEx != IMS_NULL) ? piRegEx->GetStateTracker()->GetPathHeaders()
                                 : AStringArray::ConstNull();
}

PROTECTED VIRTUAL IMS_SINT32 RegBinding::GetPortFlowControl() const
{
    //---------------------------------------------------------------------------------------------

    return (piRegEx != IMS_NULL) ? piRegEx->GetStateTracker()->GetPortFlowControl()
                                 : Sip::PORT_UNSPECIFIED;
}

PROTECTED VIRTUAL IMS_SINT32 RegBinding::GetPortUC() const
{
    //---------------------------------------------------------------------------------------------

    return (piRegEx != IMS_NULL) ? piRegEx->GetStateTracker()->GetPortUC()
                                 : SipConfigProxy::GetPort(IMS_SLOT_0);
}

PROTECTED VIRTUAL IMS_SINT32 RegBinding::GetPortUS() const
{
    //---------------------------------------------------------------------------------------------

    return (piRegEx != IMS_NULL) ? piRegEx->GetStateTracker()->GetPortUS()
                                 : SipConfigProxy::GetPort(IMS_SLOT_0);
}

PROTECTED VIRTUAL const IRegInfo* RegBinding::GetRegInfo() const
{
    //---------------------------------------------------------------------------------------------

    return (piRegEx != IMS_NULL) ? piRegEx->GetRegInfo() : IMS_NULL;
}

PROTECTED VIRTUAL const AStringArray& RegBinding::GetSecurityClients() const
{
    //---------------------------------------------------------------------------------------------

    if (!IsBindingActive())
    {
        return AStringArray::ConstNull();
    }

    return (piRegEx != IMS_NULL) ? piRegEx->GetStateTracker()->GetSecurityClients()
                                 : AStringArray::ConstNull();
}

PROTECTED VIRTUAL const AStringArray& RegBinding::GetSecurityVerifys() const
{
    //---------------------------------------------------------------------------------------------

    if (!IsBindingActive())
    {
        return AStringArray::ConstNull();
    }

    return (piRegEx != IMS_NULL) ? piRegEx->GetStateTracker()->GetSecurityVerifys()
                                 : AStringArray::ConstNull();
}

PROTECTED VIRTUAL const AStringArray& RegBinding::GetServiceRoutes() const
{
    //---------------------------------------------------------------------------------------------

    if (!IsBindingActive())
    {
        return AStringArray::ConstNull();
    }

    return (piRegEx != IMS_NULL) ? piRegEx->GetStateTracker()->GetServiceRoutes()
                                 : AStringArray::ConstNull();
}

PROTECTED VIRTUAL SipProfile* RegBinding::GetSIPProfile() const
{
    //---------------------------------------------------------------------------------------------

    return (piRegEx != IMS_NULL) ? piRegEx->GetStateTracker()->GetSIPProfile() : IMS_NULL;
}

PROTECTED VIRTUAL IMS_SINT32 RegBinding::GetState() const
{
    //---------------------------------------------------------------------------------------------

    return nState;
}

PROTECTED VIRTUAL const AString& RegBinding::GetSubscriberId() const
{
    //---------------------------------------------------------------------------------------------

    return (piRegEx != IMS_NULL) ? piRegEx->GetStateTracker()->GetSubscriberId()
                                 : AString::ConstNull();
}

PROTECTED VIRTUAL IMS_SINT32 RegBinding::GetTransportExt() const
{
    //---------------------------------------------------------------------------------------------

    return (piRegEx != IMS_NULL) ? piRegEx->GetStateTracker()->GetTransportExt()
                                 : Sip::TRANSPORT_EXT_ANY;
}

PROTECTED VIRTUAL const SipParameter* RegBinding::GetInstanceParameter() const
{
    //---------------------------------------------------------------------------------------------

    if (piRegEx == IMS_NULL)
    {
        return IMS_NULL;
    }

    if (piContact == IMS_NULL)
    {
        return IMS_NULL;
    }

    return piContact->GetInstanceParameter();
}

PROTECTED VIRTUAL const SipAddress* RegBinding::GetPublicGRUU() const
{
    //---------------------------------------------------------------------------------------------

    if (piRegEx == IMS_NULL)
    {
        return IMS_NULL;
    }

    if (piContact == IMS_NULL)
    {
        return IMS_NULL;
    }

    return piContact->GetPublicGRUU();
}

PROTECTED VIRTUAL const SipAddress* RegBinding::GetTemporaryGRUU() const
{
    //---------------------------------------------------------------------------------------------

    if (piRegEx == IMS_NULL)
    {
        return IMS_NULL;
    }

    if (piContact == IMS_NULL)
    {
        return IMS_NULL;
    }

    return piContact->GetTemporaryGRUU();
}

PROTECTED VIRTUAL const IMSList<SipAddress*>& RegBinding::GetTemporaryGRUUs() const
{
    //---------------------------------------------------------------------------------------------

    if (piRegEx == IMS_NULL)
    {
        return SipAddress::ConstEmptyList();
    }

    if (piContact == IMS_NULL)
    {
        return SipAddress::ConstEmptyList();
    }

    return piContact->GetTemporaryGRUUs();
}

PROTECTED VIRTUAL IMS_BOOL RegBinding::IsBehindNAT() const
{
    //---------------------------------------------------------------------------------------------

    return (piRegEx != IMS_NULL) ? piRegEx->IsBehindNAT() : IMS_FALSE;
}

PROTECTED VIRTUAL IMS_BOOL RegBinding::IsWithinTrustDomain() const
{
    //---------------------------------------------------------------------------------------------

    return (piRegEx != IMS_NULL) ? piRegEx->IsWithinTrustDomain() : IMS_FALSE;
}

PROTECTED VIRTUAL void RegBinding::NotifyCallerCapabilityChanged()
{
    //---------------------------------------------------------------------------------------------

    if (piRegEx == IMS_NULL)
        return;

    piRegEx->NotifyCallerCapabilityChanged();
}

PROTECTED VIRTUAL void RegBinding::SetListener(IN IRegBindingListener* piListener)
{
    //---------------------------------------------------------------------------------------------

    this->piListener = piListener;
}

PRIVATE
void RegBinding::CreateSIPConnectionNotifier()
{
    SIPConnectionNotifierManager* pSCNMngr = SIPConnectionNotifierManager::GetInstance();

    //---------------------------------------------------------------------------------------------

    if (piRegEx == IMS_NULL)
    {
        IMS_TRACE_E(0, "No registration", 0, 0, 0);
        return;
    }

    if (piContact == IMS_NULL)
    {
        IMS_TRACE_E(0, "No contacts", 0, 0, 0);
        return;
    }

    // RE_REG_BY_CAPABILITY_CHANGE
    ISipConnectionNotifier* piTempSCN = piSCN;

    // MULTI_REG_TRANSPORT
    AString strParams = AString::ConstNull();
    IMS_SINT32 nTransportExt = piRegEx->GetStateTracker()->GetTransportExt();

    if (nTransportExt != Sip::TRANSPORT_EXT_ANY)
    {
        strParams.Sprintf("%s=%d", Sip::STR_TRANSPORT_EXT, nTransportExt);
    }

    // RFC5626_FLOW_CONTROL : GetPortFlowControl()
    piSCN = pSCNMngr->CreateConnectionNotifier(piContact->GetContactAddress().GetScheme(),
            piContact->GetIPAddress(), GetPortUS(), GetPortUC(), GetPortFlowControl(), strParams,
            piRegEx->GetStateTracker()->GetAuthorizedAOR());

    if ((piSCN != IMS_NULL) && (piSCN != piTempSCN))
    {
        if (piRegEx->AddReferenceForSCNEL() == 1)
        {
            piSCN->AddErrorListener(piRegEx);

            // RFC5626_FLOW_CONTROL
            if (Sip::IsPortSpecified(GetPortFlowControl()))
            {
                RestoreTransportResourceForClientInitiatedConnection();
            }
        }
    }

    // MULTI_REG_SIP_PROFILE
    if (piSCN != IMS_NULL)
    {
        piSCN->SetSipProfile(GetSIPProfile());
    }
    else
    {
        piRegEx->ConnectionNotifierError_NotifyError(
                piSCN, ISipConnectionNotifier::TRANSPORT_ERROR_UDP_SERVER, "SCN is not created");
    }

    // RE_REG_BY_CAPABILITY_CHANGE
    if (piTempSCN != IMS_NULL)
    {
        if (piSCN != piTempSCN)
        {
            piRegEx->RemoveReferenceForSCNEL();
        }

        SIPConnectionNotifierManager::GetInstance()->ReleaseConnectionNotifier(piTempSCN);
    }
}

PRIVATE
void RegBinding::DestroySIPConnectionNotifier()
{
    //---------------------------------------------------------------------------------------------

    if (piSCN == IMS_NULL)
    {
        return;
    }

    if ((piRegEx != IMS_NULL) && (piRegEx->RemoveReferenceForSCNEL() == 0))
    {
        piSCN->RemoveErrorListener(piRegEx);
    }

    SIPConnectionNotifierManager::GetInstance()->ReleaseConnectionNotifier(piSCN);
    piSCN = IMS_NULL;
}

PRIVATE
IMS_BOOL RegBinding::IsBindingActive() const
{
    IMS_SINT32 nState = GetState();

    //---------------------------------------------------------------------------------------------

    return (nState == STATE_ACTIVE) || (nState == STATE_ACTIVE_PENDING) ||
            (nState == STATE_ACTIVE_TERMINATING);
}

// REG_RESTORATION_FOR_ACTIVE_BINDING
PRIVATE
void RegBinding::RestoreTransportResourceForClientInitiatedConnection()
{
    //---------------------------------------------------------------------------------------------

    if ((piRegEx == IMS_NULL) || (piSCN == IMS_NULL))
    {
        return;
    }

    const IMS_SINT32 nTransportResource =
            ISipConnectionNotifier::TRANSPORT_CLIENT_INITIATED_CONNECTION;

    if (piSCN->IsTransportResourceReserved(nTransportResource))
    {
        return;
    }

    IRegParameter* piRegParam = piRegEx->GetParameter();

    if (piRegParam == IMS_NULL)
    {
        piRegEx->ConnectionNotifierError_NotifyError(piSCN,
                ISipConnectionNotifier::TRANSPORT_ERROR_TCP_CLIENT,
                "Restoration of TCP client connection is failed");
        return;
    }

    const SipAddress& objRoute = piRegParam->GetTopmostRouteAddress();
    IPAddress objPeerIP(objRoute.GetHost());

    if (piSCN->RestoreTransportResource(nTransportResource, objPeerIP, objRoute.GetPort()) !=
            IMS_SUCCESS)
    {
        piRegEx->ConnectionNotifierError_NotifyError(piSCN,
                ISipConnectionNotifier::TRANSPORT_ERROR_TCP_CLIENT,
                "Restoration of TCP client connection is failed");
    }
}

PRIVATE
void RegBinding::RestoreTransportResourceForServerConnection()
{
    //---------------------------------------------------------------------------------------------

    if ((piRegEx == IMS_NULL) || (piSCN == IMS_NULL))
    {
        return;
    }

    const IMS_SINT32 nTransportResource = ISipConnectionNotifier::TRANSPORT_SERVER_CONNECTION;

    if (piSCN->IsTransportResourceReserved(nTransportResource))
    {
        return;
    }

    if (piSCN->RestoreTransportResource(nTransportResource, IPAddress::NONE, 0) != IMS_SUCCESS)
    {
        piRegEx->ConnectionNotifierError_NotifyError(piSCN,
                ISipConnectionNotifier::TRANSPORT_ERROR_UDP_SERVER,
                "Restoration of server connection is failed");
    }
}

PRIVATE
void RegBinding::SetState(IN IMS_SINT32 nState)
{
    //---------------------------------------------------------------------------------------------

    IMS_TRACE_I("RegBinding (%s) :: %s to %s",
            SipDebug::GetUri1(GetAuthorizedAOR().GetUri()).GetStr(), StateToString(this->nState),
            StateToString(nState));

    this->nState = nState;
}

PRIVATE GLOBAL const IMS_CHAR* RegBinding::StateToString(IN IMS_SINT32 nState)
{
    //---------------------------------------------------------------------------------------------

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
