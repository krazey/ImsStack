/*
    Author
    <table>
    date      author                    description
    --------  --------------            ----------
    20090908  toastops@                 Created
    20100321  hwangoo.park@             Change Register(...) for expires value
    </table>

    Description

*/

#include "ServiceMemory.h"
#include "ServiceTrace.h"
#include "ServicePhoneInfo.h"
#include "ServiceUtil.h"
#include "ServiceNetwork.h"
#include "TextParser.h"
#include "NATHelper.h"
#include "IDigestAka.h"
#include "private/ConfigurationManager.h"
#include "private/SubscriberConfig.h"
#include "Connector.h"
#include "ISipHeader.h"
#include "ISipMessage.h"
#include "ISipConnectionNotifier.h"
#include "ISipClientConnection.h"
#include "ISipGenericChallenge.h"
#include "ISipTransportHelper.h"
#include "Sip.h"
#include "SipHeaderName.h"
#include "SipDebug.h"
#include "SipParsingHelper.h"
#include "SipStatusCode.h"
#include "SipParameter.h"
#include "SipConfigProxy.h"
#include "SipFactory.h"
#include "base/IMS.h"
#include "base/SubscriberTracker.h"
#include "PAccessNetworkInfoHeader.h"
#include "SipTimerValuesHelper.h"
#include "SIPURNHelper.h"
#include "IRegistrationListener.h"
#include "IRegUserIdentityNotifier.h"
#include "IRegBindingStateListener.h"
#include "RegObserver.h"
#include "RegFlow.h"
#include "RegParameter.h"
#include "RegRefreshHelper.h"
#include "RegSubscription.h"
#include "IRegInfoRegistration.h"
#include "IRegInfoContact.h"
#include "RegInfoManager.h"
#include "RegInfo.h"
#include "RegBindingProxy.h"
#ifdef __IMS_TEST_AKA__
#include "AKAHelper.h"
#endif
#include "Registration.h"

__IMS_TRACE_TAG_REG__;

PUBLIC
Registration::Registration() :
        EngineActivity(),
        bDestroyed(IMS_FALSE),
        nState(STATE_CREATED),
        nSubState(SUB_STATE_IDLE),
        pRegFlow(IMS_NULL),
        pRegParam(IMS_NULL),
        objContacts(IMSList<RegContact*>()),
        piListener(IMS_NULL),
        piBindingStateListener(IMS_NULL),
        piUserIdNotifier(IMS_NULL),
        pRefreshHelper(IMS_NULL),
        piDigestAKA(IMS_NULL),
        // IMS_AUTH_NONCE_REUSE
        piGenericChallenge(IMS_NULL),
        piOngoingSCC(IMS_NULL),
        piNextRequest(IMS_NULL),
        piPreviousRequest(IMS_NULL),
        piPreviousResponse(IMS_NULL),
        // SIP_MESSAGE_MEDIATOR
        piMessageMediator(IMS_NULL),
        objObservers(IMSList<RegObserver*>()),
        bIsBehindNAT(IMS_FALSE),
        bIsWithinTrustDomain(IMS_TRUE),
        bActiveBindingsRestorationEnabled(IMS_FALSE),
        nRefCountForSCNEL(0)
{
    pStateTracker = new RegStateTracker();
}

PUBLIC VIRTUAL Registration::~Registration()
{
    if (piOngoingSCC != IMS_NULL)
    {
        piOngoingSCC->Close();
        piOngoingSCC = IMS_NULL;
    }

    ClearNextRequest();
    SetPreviousRequest(IMS_NULL);
    SetPreviousResponse(IMS_NULL);

    if (piDigestAKA != IMS_NULL)
    {
        piDigestAKA->Destroy();
        piDigestAKA = IMS_NULL;
    }

    // IMS_AUTH_NONCE_REUSE {
    SetAuthenticationChallenge(IMS_NULL);
    // }

    if (pRefreshHelper != IMS_NULL)
    {
        delete pRefreshHelper;
        pRefreshHelper = IMS_NULL;
    }

    if (pRegParam != IMS_NULL)
    {
        delete pRegParam;
        pRegParam = IMS_NULL;
    }

    RegBindingProxy::DestroyBinding(GetSlotId(), this);

    IMS_TRACE_D("Destructor :: Registration - %s",
            (pRegFlow != IMS_NULL) ? SipDebug::GetCharA1(pRegFlow->GetCallId().GetStr(), 8, '@')
                                   : "__UNKNOWN__",
            0, 0);

    if (pRegFlow != IMS_NULL)
    {
        delete pRegFlow;
        pRegFlow = IMS_NULL;
    }
}

/*

Remarks

*/
PUBLIC VIRTUAL IMS_BOOL Registration::Equals(IN CONST IRegistration* piReg) const
{
    const Registration* pReg = DYNAMIC_CAST(const Registration*, piReg);

    //---------------------------------------------------------------------------------------------

    if (pReg == IMS_NULL)
    {
        return IMS_FALSE;
    }

    return pRegFlow->GetRegKey().Equals(pReg->pRegFlow->GetRegKey());
}

/*

Remarks

*/
PUBLIC VIRTUAL IMS_BOOL Registration::IsBehindNAT() const
{
    //---------------------------------------------------------------------------------------------

    return bIsBehindNAT;
}

/*

Remarks

*/
PUBLIC VIRTUAL IMS_BOOL Registration::IsWithinTrustDomain() const
{
    //---------------------------------------------------------------------------------------------

    return bIsWithinTrustDomain && pStateTracker->IsWithinTrustDomain(GetSlotId());
}

/*

Remarks

*/
PUBLIC VIRTUAL const RegInfo* Registration::GetRegInfo() const
{
    //---------------------------------------------------------------------------------------------

    if (pRegFlow == IMS_NULL)
        return IMS_NULL;

    return RegInfoManager::GetInstance()->GetRegInfo(pRegFlow->GetRegKey());
}

/*

Remarks

*/
PUBLIC VIRTUAL const RegStateTracker* Registration::GetStateTracker() const
{
    //---------------------------------------------------------------------------------------------

    return pStateTracker.Get();
}

/*

Remarks
 MULTI_SUBS
 MULTI_REG_SIP_PROFILE
*/
PUBLIC
IMS_BOOL Registration::Create(IN IMS_UINT32 nFlowId, IN CONST SipAddress& objAOR,
        IN CONST AString& strSubsId /* = AString::ConstNull() */,
        IN SipProfile* pSIPProfile /* = IMS_NULL*/)
{
    //---------------------------------------------------------------------------------------------

    pRegFlow = new RegFlow(RegKey(GetSlotId(), nFlowId));

    if (pRegFlow == IMS_NULL)
    {
        IMS_TRACE_E(0, "Instantiating RegFlow(%d:%d) failed", GetSlotId(), nFlowId, 0);
        return IMS_FALSE;
    }

    pRegParam = new RegParameter(GetSlotId());

    if (pRegParam == IMS_NULL)
    {
        IMS_TRACE_E(0, "Instantiating RegParameter failed", 0, 0, 0);
        return IMS_FALSE;
    }

    if (!pRegParam->UpdateProfile(objAOR, strSubsId))
    {
        IMS_TRACE_E(0, "Updating a profile failed", 0, 0, 0);
        return IMS_FALSE;
    }

    pRefreshHelper = new RegRefreshHelper(this);

    if (pRefreshHelper == IMS_NULL)
    {
        IMS_TRACE_E(0, "Instantiating RefreshHelper failed", 0, 0, 0);
        return IMS_FALSE;
    }

    // MULTI_REG_SIP_PROFILE
    SetSIPProfile(pSIPProfile);

    pStateTracker->SetAOR(objAOR);
    pStateTracker->SetSubscriberId(strSubsId);

    return IMS_TRUE;
}

/*

Remarks

*/
PUBLIC
void Registration::Destroy()
{
    //---------------------------------------------------------------------------------------------

    bDestroyed = IMS_TRUE;

    IMS_TRACE_D("Registration :: Destroy() - SCNEL=%d", nRefCountForSCNEL, 0, 0);

    RegInfo* pRegInfo = RegInfoManager::GetInstance()->GetRegInfo(pRegFlow->GetRegKey());

    if (pRegInfo != IMS_NULL)
    {
        pRegInfo->RemoveListener(this);
    }

    DestroyAllContacts();
    UpdateBindingState(BINDING_DESTROY);

    if (pRefreshHelper != IMS_NULL)
    {
        if (pRefreshHelper->IsRequestPending())
        {
            pRefreshHelper->AbortConnection();
        }
    }

    if (piOngoingSCC != IMS_NULL)
    {
        piOngoingSCC->Close();
        piOngoingSCC = IMS_NULL;
    }

    if (piDigestAKA != IMS_NULL)
    {
        piDigestAKA->Destroy();
        piDigestAKA = IMS_NULL;
    }

    // SIP_MESSAGE_MEDIATOR
    SetSipMessageMediator(IMS_NULL);

    // NAT_REQ_UE_PUBLIC_IP
    if (NATHelper::IsNATResolverRequired())
    {
        NATHelper::GetInstance()->RemovePublicAddress(
                GetSlotId(), pRegFlow->GetRegKey().GetFlowId());
    }

    PostMessage(AMSG_DESTROY, 0, 0);
}

/*

Remarks

*/
PUBLIC
const IMSList<RegContact*>& Registration::GetAllContactsEx() const
{
    //---------------------------------------------------------------------------------------------

    return objContacts;
}

/*

Remarks

*/
PUBLIC
IMS_BOOL Registration::HasActiveBindings() const
{
    //---------------------------------------------------------------------------------------------

    for (IMS_UINT32 i = 0; i < objContacts.GetSize(); ++i)
    {
        RegContact* pContact = objContacts.GetAt(i);

        if (pContact->IsActiveBinding())
        {
            return IMS_TRUE;
        }
    }

    return IMS_FALSE;
}

/*

Remarks

*/
PUBLIC
IMS_BOOL Registration::IsAllBindingsRemoved() const
{
    //---------------------------------------------------------------------------------------------

    for (IMS_UINT32 i = 0; i < objContacts.GetSize(); ++i)
    {
        RegContact* pContact = objContacts.GetAt(i);

        if (!pContact->IsEmpty())
        {
            return IMS_FALSE;
        }
    }

    return IMS_TRUE;
}

/*

Remarks

*/
PRIVATE VIRTUAL IMS_BOOL Registration::DispatchMessage(IN IMSMSG& objMSG)
{
    //---------------------------------------------------------------------------------------------

    switch (objMSG.GetName())
    {
        case AMSG_REGISTRATION_STARTED:
            if (piListener != IMS_NULL)
            {
                piListener->Registration_Started();
            }
            return IMS_TRUE;

        case AMSG_REGISTRATION_START_FAILED:
            if (piListener != IMS_NULL)
            {
                piListener->Registration_StartFailed(LONG_TO_SINT(objMSG.nWparam));
            }
            return IMS_TRUE;

        case AMSG_REGISTRATION_UPDATED:
            if (piListener != IMS_NULL)
            {
                piListener->Registration_Updated();
            }
            return IMS_TRUE;

        case AMSG_REGISTRATION_UPDATE_FAILED:
            if (piListener != IMS_NULL)
            {
                piListener->Registration_UpdateFailed(LONG_TO_SINT(objMSG.nWparam));
            }
            return IMS_TRUE;

        case AMSG_REGISTRATION_REMOVED:
            if (piListener != IMS_NULL)
            {
                piListener->Registration_Removed();
            }
            return IMS_TRUE;

        case AMSG_REGISTRATION_TERMINATED:
            if (piListener != IMS_NULL)
            {
                piListener->Registration_Terminated(LONG_TO_SINT(objMSG.nWparam));
            }
            return IMS_TRUE;

        case AMSG_REGISTRATION_AKA_RESPONSE_RECEIVED:
            if (bDestroyed)
            {
                IMS_TRACE_D("Registration is already destroyed :: AKA_RESPONSE_RECEIVED", 0, 0, 0);
                return IMS_TRUE;
            }

            if (!CreateSA(objActiveCredential, objActiveSAKey))
            {
                IMS_TRACE_E(0, "Creating SA failed", 0, 0, 0);
                return IMS_TRUE;
            }

            if (!RespondToPendingChallenge(objActiveCredential))
            {
                RestoreSecurityHeaders();
            }
            return IMS_TRUE;

        case AMSG_REGISTRATION_BINDING_STATE_CHANGED:
            if (piBindingStateListener == IMS_NULL)
            {
                IMS_TRACE_D("IRegBindingStateListener is null", 0, 0, 0);
                return IMS_TRUE;
            }

            if (LONG_TO_INT(objMSG.nWparam) == BINDING_STATE_CALLER_CAPABILITY)
            {
                piBindingStateListener->RegBindingState_CallerCapabilityChanged();
            }
            return IMS_TRUE;

        default:
            break;
    }

    return EngineActivity::DispatchMessage(objMSG);
}

/*

Remarks

*/
PRIVATE VIRTUAL void Registration::ClientConnection_NotifyResponse(
        IN ISipClientConnection* piSCC, IN ISipClientConnection* /* piForkedSCC = IMS_NULL */)
{
    //---------------------------------------------------------------------------------------------

    if (piSCC == IMS_NULL)
        return;

    if (piSCC->Receive() != IMS_SUCCESS)
    {
        IMS_TRACE_E(0, "No message received", 0, 0, 0);
        return;
    }

    IMS_SINT32 nStatusCode = piSCC->GetStatusCode();

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

    SetPreviousResponse(piSCC->GetMessage());

    // Authentication challenged by S-CSCF
    if ((nStatusCode == SipStatusCode::SC_401) || (nStatusCode == SipStatusCode::SC_407))
    {
        // IMS_AUTH_NONCE_REUSE {
        SetAuthenticationChallenge(IMS_NULL);
        // }

        // Update the Security-xxx headers
        pRegParam->UpdateSecurityHeaders(piSCC->GetMessage());

        // Respond to the authentication challenge
        if (!RespondToChallenge(piSCC))
        {
            RestoreSecurityHeaders();
        }

        if (GetSubState() == SUB_STATE_IDLE)
        {
            piSCC->Close();
            piOngoingSCC = IMS_NULL;
        }

        // 3 Do not close SIPConnection in here
        return;
    }

    // Reset the flags after the transaction completed
    SetSubState(SUB_STATE_IDLE);

    piSCC->Close();
    piOngoingSCC = IMS_NULL;

    ISipMessage* piSIPMsg = GetPreviousResponse();

    // Handle the response except for 401/407
    if (SipStatusCode::IsFinalSuccess(nStatusCode))
    {
        // Update the Security-xxx headers
        pRegParam->UpdateSecurityHeaders(piSIPMsg);

        // Update the sequence number
        UpdateCSeqNumber(piSIPMsg);

        // Update the bindings
        IMS_RESULT nBindingUpdateResult = UpdateBindings(piSIPMsg);
        IMS_SINT32 nReason = REASON_NONE;

        if (nBindingUpdateResult == BINDING_UPDATE_NO_EXPIRES)
        {
            nReason = REASON_NO_EXPIRES;
        }

        // Update the refresh timer
        UpdateRefreshTimer();

        // Check the UA location
        CheckUALocation(piSIPMsg);

        // Store the persistent header information
        StorePersistentHeaders(piSIPMsg);

        // IMS_AUTH_NONCE_REUSE {
        SetNextNonce(piSIPMsg);
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
            UpdateBindingState(BINDING_RESULT_OK);
        else
            UpdateBindingState(BINDING_RESULT_NOK);

        CallListener(nPrevState, nPrevSubState, nReason);
    }
    else
    {
        // 140225, hwangoo.park
        // Do not clear the authentication challenge...
        // Initial registration required: 305 / 408 / 500 / 504 / 600 / txn timeout
        // // IMS_AUTH_NONCE_REUSE {
        // SetAuthenticationChallenge(IMS_NULL);
        // // }

        // 13.01.29, hwangoo.park
        // Do not check the status code to support re-use of Call-ID & CSeq number
        // when the registration is failed
        // if (nStatusCode == SipStatusCode::SC_423)
        {
            // Update the sequence number
            UpdateCSeqNumber(piSIPMsg);
        }

        if (nPrevState == STATE_INIT)
        {
            SetState(STATE_CREATED);
        }

        UpdateBindingState(BINDING_RESULT_NOK);

        CallListener(nPrevState, nPrevSubState, REASON_STATUS_CODE);
    }
}

/*

Remarks

*/
PRIVATE VIRTUAL void Registration::Error_NotifyError(
        IN ISipConnection* piSC, IN IMS_SINT32 nCode, IN CONST AString& strMessage)
{
    //---------------------------------------------------------------------------------------------

    (void)strMessage;

    IMS_TRACE_I("Registration::Error_NotifyError() - Code (%d), Message (%s)", nCode,
            strMessage.GetStr(), 0);

    if (piSC == IMS_NULL)
        return;

    piSC->Close();
    piOngoingSCC = IMS_NULL;

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

/*

Remarks

*/
PRIVATE VIRTUAL ISipMessage* Registration::GetNextRequest()
{
    //---------------------------------------------------------------------------------------------

    if (piNextRequest == IMS_NULL)
    {
        piNextRequest = SipParsingHelper::CreateMessage(ISipMessage::TYPE_REQUEST);
    }

    return piNextRequest;
}

/*

Remarks

*/
PRIVATE VIRTUAL ISipMessage* Registration::GetPreviousRequest() const
{
    //---------------------------------------------------------------------------------------------

    return piPreviousRequest;
}

/*

Remarks

*/
PRIVATE VIRTUAL ISipMessage* Registration::GetPreviousResponse() const
{
    //---------------------------------------------------------------------------------------------

    return piPreviousResponse;
}

/*

Remarks
 SIP_MESSAGE_MEDIATOR
*/
PRIVATE VIRTUAL void Registration::SetSipMessageMediator(IN IMessageMediator* piMediator)
{
    //---------------------------------------------------------------------------------------------

    piMessageMediator = piMediator;

    if (pRefreshHelper != IMS_NULL)
    {
        pRefreshHelper->SetMessageMediator(piMessageMediator);
    }
}

/*

Remarks

*/
PRIVATE VIRTUAL IMS_BOOL Registration::CreateBinding(
        IN CONST AString& strAppId, IN CONST AString& strServiceId)
{
    //---------------------------------------------------------------------------------------------

    return RegBindingProxy::CreateBinding(GetSlotId(), strAppId, strServiceId, this);
}

/*

Remarks

*/
PRIVATE VIRTUAL void Registration::DestroyBinding(
        IN CONST AString& strAppId, IN CONST AString& strServiceId)
{
    //---------------------------------------------------------------------------------------------

    RegBindingProxy::DestroyBinding(GetSlotId(), strAppId, strServiceId);
}

/*

Remarks

*/
PRIVATE VIRTUAL IRegContact* Registration::CreateContact(IN CONST IPAddress& objIPA,
        IN IMS_SINT32 nPort, IN IMS_SINT32 nExpiresPolicy /* = POLICY_EXPIRES_CONFIG */,
        IN IMS_UINT32 nExpiresValue /* = DEFAULT_EXPIRES */)
{
    //---------------------------------------------------------------------------------------------

    // If the contact address is already present in the list,
    // then just adds the service capability to the contact.
    for (IMS_UINT32 i = 0; i < objContacts.GetSize(); ++i)
    {
        RegContact* pTmpContact = objContacts.GetAt(i);

        if (objIPA.Equals(pTmpContact->GetIPAddress()) && (nPort == pTmpContact->GetPort()))
        {
            IMS_TRACE_D("RegContact (%s, %d) already exists", SipDebug::GetIp(objIPA), nPort, 0);
            return pTmpContact;
        }
    }

    SipProfile* pSIPProfile = pStateTracker->GetSIPProfile();

    // If not present, add a new Contact information
    RegContact* pNewContact = new RegContact(
            GetSlotId(), objIPA, nPort, this, pRegFlow->GetRegKey().GetFlowId(), pSIPProfile);

    if (pNewContact == IMS_NULL)
    {
        IMS_TRACE_E(0, "Creating a Contact (%s:%d) failed", SipDebug::GetIp(objIPA), nPort, 0);
        return IMS_NULL;
    }

    // Set user-info field
    pNewContact->SetAOR(pStateTracker->GetAOR());

    // Set "+sip.instance" parameter
    IMS_SINT32 nDeviceId = SipConfigProxy::GetDeviceId(GetSlotId(), pSIPProfile);

    if (nDeviceId != ISipConfig::DEVICE_ID_NONE)
    {
        AString strDeviceId;
        AString strInstance;

        if (nDeviceId == ISipConfig::DEVICE_ID_PREDEFINED)
        {
            strDeviceId = SipConfigProxy::GetPredefinedDeviceId(GetSlotId(), pSIPProfile);
        }
        else
        {
            strDeviceId = SIPURNHelper::GetURN(GetSlotId(), nDeviceId);
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
            if (SipConfigProxy::IsRegExpiresConfigured(GetSlotId(), pSIPProfile))
            {
                // Ignore the passed expiration (nExpiresValue)
                pNewContact->SetExpires(SipConfigProxy::GetRegExpires(GetSlotId(), pSIPProfile));
            }
        }
        break;

        case IRegistration::POLICY_EXPIRES_APP:
            pNewContact->SetExpires(nExpiresValue);
            break;

        default:
            break;
    }

    IMS_TRACE_D("RegContact (%s, %d) added", SipDebug::GetIp(objIPA), nPort, 0);

    if (!objContacts.Append(pNewContact))
    {
        delete pNewContact;
        return IMS_NULL;
    }

    ChoosePreferredContact();

    return pNewContact;
}

/*

Remarks

*/
PRIVATE VIRTUAL void Registration::DestroyAllContacts()
{
    //---------------------------------------------------------------------------------------------

    while (!objContacts.IsEmpty())
    {
        RegContact* pContact = objContacts.GetAt(0);

        // Notify the removal of contact to the RegBinding ...
        UpdateBindingState(BINDING_DESTROY_CONTACT);

        delete pContact;
        objContacts.RemoveAt(0);
    }

    pStateTracker->SetPreferredContact(IMS_NULL);
}

/*

Remarks

*/
PRIVATE VIRTUAL void Registration::DestroyContact(IN IRegContact* piContact)
{
    //---------------------------------------------------------------------------------------------

    for (IMS_UINT32 i = 0; i < objContacts.GetSize(); ++i)
    {
        RegContact* pContact = objContacts.GetAt(i);

        if (pContact == piContact)
        {
            // Notify the removal of contact to the RegBinding ...
            UpdateBindingState(BINDING_DESTROY_CONTACT);

            delete pContact;
            objContacts.RemoveAt(i);
            break;
        }
    }

    ChoosePreferredContact();
}

/*

Remarks

*/
PRIVATE VIRTUAL void Registration::DestroyContact(IN CONST IPAddress& objIPA, IN IMS_SINT32 nPort)
{
    //---------------------------------------------------------------------------------------------

    for (IMS_UINT32 i = 0; i < objContacts.GetSize(); ++i)
    {
        RegContact* pContact = objContacts.GetAt(i);

        if (objIPA.Equals(pContact->GetIPAddress()) && (nPort == pContact->GetPort()))
        {
            // Notify the removal of contact to the RegBinding ...
            UpdateBindingState(BINDING_DESTROY_CONTACT);

            delete pContact;
            objContacts.RemoveAt(i);
            break;
        }
    }

    ChoosePreferredContact();
}

/*

Remarks

*/
PRIVATE VIRTUAL const Credential* Registration::GetCredential() const
{
    //---------------------------------------------------------------------------------------------

    return &objActiveCredential;
}

/*

Remarks

*/
PRIVATE VIRTUAL const SipAddress& Registration::GetAOR() const
{
    //---------------------------------------------------------------------------------------------

    return pStateTracker->GetAOR();
}

/*

Remarks

*/
PRIVATE VIRTUAL const AStringArray& Registration::GetAssociatedURIs() const
{
    //---------------------------------------------------------------------------------------------

    return pStateTracker->GetAssociatedURIs();
}

/*

Remarks

*/
PRIVATE VIRTUAL const SipAddress& Registration::GetAuthorizedAOR() const
{
    //---------------------------------------------------------------------------------------------

    return pStateTracker->GetAuthorizedAOR();
}

/*

Remarks

*/
PRIVATE VIRTUAL IMSList<IRegContact*> Registration::GetAllContacts() const
{
    IMSList<IRegContact*> objIContacts;

    //---------------------------------------------------------------------------------------------

    for (IMS_UINT32 i = 0; i < objContacts.GetSize(); ++i)
    {
        RegContact* pContact = objContacts.GetAt(i);

        objIContacts.Append(pContact);
    }

    return objIContacts;
}

/*

Remarks

*/
PRIVATE VIRTUAL IRegContact* Registration::GetContact(
        IN CONST IPAddress& objIPA, IN IMS_SINT32 nPort) const
{
    //---------------------------------------------------------------------------------------------

    for (IMS_UINT32 i = 0; i < objContacts.GetSize(); ++i)
    {
        RegContact* pContact = objContacts.GetAt(i);

        if (objIPA.Equals(pContact->GetIPAddress()) && (nPort == pContact->GetPort()))
        {
            return pContact;
        }
    }

    return IMS_NULL;
}

/*

Remarks

*/
PRIVATE VIRTUAL IRegContact* Registration::GetPreferredContact() const
{
    //---------------------------------------------------------------------------------------------

    return const_cast<RegContact*>(pStateTracker->GetPreferredContact());
}

/*

Remarks

*/
PRIVATE VIRTUAL IRegParameter* Registration::GetParameter() const
{
    //---------------------------------------------------------------------------------------------

    return pRegParam;
}

/*

Remarks

*/
PRIVATE VIRTUAL const IPAddress& Registration::GetPublicIPAddress() const
{
    //---------------------------------------------------------------------------------------------

    return pStateTracker->GetPublicIPAddress();
}

/*

Remarks

*/
PRIVATE VIRTUAL const AStringArray& Registration::GetServiceRoutes() const
{
    //---------------------------------------------------------------------------------------------

    return pStateTracker->GetServiceRoutes();
}

/*

Remarks
 MULTI_REG_SIP_PROFILE
*/
PRIVATE VIRTUAL SipProfile* Registration::GetSIPProfile() const
{
    //---------------------------------------------------------------------------------------------

    return (!pStateTracker.IsNull()) ? pStateTracker->GetSIPProfile() : IMS_NULL;
}

/*

Remarks

*/
PRIVATE VIRTUAL IMS_SINT32 Registration::GetState() const
{
    //---------------------------------------------------------------------------------------------

    return nState;
}

/*

Remarks

*/
PRIVATE VIRTUAL IMS_BOOL Registration::IsBindingsUpdated() const
{
    //---------------------------------------------------------------------------------------------

    for (IMS_UINT32 i = 0; i < objContacts.GetSize(); ++i)
    {
        RegContact* pContact = objContacts.GetAt(i);

        if (pContact->IsBindingsUpdated())
        {
            IMS_TRACE_D("Registration bindings are updated", 0, 0, 0);
            return IMS_TRUE;
        }
    }

    return IMS_FALSE;
}

/*

Remarks

*/
PRIVATE VIRTUAL IMS_BOOL Registration::IsBindingsUpdating() const
{
    //---------------------------------------------------------------------------------------------

    return (GetSubState() != SUB_STATE_IDLE);
}

/*

Remarks

*/
PRIVATE VIRTUAL IMS_BOOL Registration::IsNetworkInterworkingRequired() const
{
    //---------------------------------------------------------------------------------------------

    // Emergency / normal registration
    return IMS_TRUE;
}

/*

Remarks

*/
PRIVATE VIRTUAL IMS_RESULT Registration::Register(IN IMS_SINT32 nExpires /* = (-1) */)
{
    IMS_SINT32 nCurrentState = GetState();
    IMS_SINT32 nCurrentSubState = GetSubState();

    //---------------------------------------------------------------------------------------------

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
        pRegFlow->UpdateCallId(pStateTracker->GetIPAddress());
    }

    ISipClientConnection* piSCC = CreateConnection(this);

    if (piSCC == IMS_NULL)
    {
        IMS_TRACE_E(0, "Instantiating a SIP connection failed", 0, 0, 0);
        return IMS_FAILURE;
    }

    piSCC->SetListener(this);
    piSCC->SetErrorListener(this);

    // Form REGISTER request
    ISipMessage* piSIPMsg = piSCC->GetMessage();

    // Copy the headers from the application
    if (piNextRequest != IMS_NULL)
    {
        // Set the headers and body parts if application already sets
        if (piSIPMsg->CopyHeadersAndBodyParts(piNextRequest) != IMS_SUCCESS)
        {
            IMS_TRACE_E(0, "Copying SIP headers and message body parts failed", 0, 0, 0);
            goto EXIT_Register;
        }
    }

    // Form the generic headers
    if (SetHeaders(piSCC) != IMS_SUCCESS)
    {
        IMS_TRACE_E(0, "Setting the generic headers failed", 0, 0, 0);
        goto EXIT_Register;
    }

    // Sets Contact headers
    if ((nExpires == 0) || IsAllBindingsRemoved())
    {
        if (SetContactNExpiresHeader(piSIPMsg, 0) != IMS_SUCCESS)
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
            for (IMS_UINT32 i = 0; i < objContacts.GetSize(); ++i)
            {
                RegContact* pContact = objContacts.GetAt(i);

                pContact->SetExpires(nExpires);
            }
        }

        if (SetContactNExpiresHeader(piSIPMsg, nExpires) != IMS_SUCCESS)
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
        SetNextAuthenticationInfo(piSCC);
    }
    // }

    // SIP_MESSAGE_MEDIATOR
    (void)AdjustMessage(piSCC->GetMessage(), IMessageMediator::MESSAGE_NORMAL);

    // Send a REGISTER request to the network
    if (piSCC->Send() != IMS_SUCCESS)
    {
        IMS_TRACE_E(0, "Sending REGISTER request failed", 0, 0, 0);
        goto EXIT_Register;
    }

    // Update the refresh data
    pRefreshHelper->UpdateOnMessageSent(piSCC);

    // Clear the next request
    ClearNextRequest();

    // Store the ongoing SIP client connection
    SetOngoingConnection(piSCC);

    // Update the last request/response message
    SetPreviousRequest(piSCC->GetMessage());
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

    IMS::SetLastError(IMSError::NO_ERROR);

    //// DEBUG
    SipDebug::Send(GetSlotId(), SipDebug::MSG_REQ, SipDebug::DIR_OUT,
            (GetSubState() == SUB_STATE_REGISTERING) ? SipMethod::REGISTER
                                                     : SipDebug::M_DEREGISTER);

    return IMS_SUCCESS;

EXIT_Register:
    SetSubState(SUB_STATE_IDLE);
    piSCC->Close();

    return IMS_FAILURE;
}

/*

Remarks

*/
PRIVATE VIRTUAL IMS_RESULT Registration::Deregister()
{
    //---------------------------------------------------------------------------------------------

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

    ISipClientConnection* piSCC = CreateConnection(this);

    if (piSCC == IMS_NULL)
    {
        IMS_TRACE_E(0, "Instantiating a SIP connection failed", 0, 0, 0);
        return IMS_FALSE;
    }

    piSCC->SetListener(this);
    piSCC->SetErrorListener(this);

    // Form REGISTER request
    ISipMessage* piSIPMsg = piSCC->GetMessage();

    // Copy the headers from the application
    if (piNextRequest != IMS_NULL)
    {
        // Set the headers and body parts if application already sets
        if (piSIPMsg->CopyHeadersAndBodyParts(piNextRequest) != IMS_SUCCESS)
        {
            IMS_TRACE_E(0, "Copying SIP headers and message body parts failed", 0, 0, 0);
            goto EXIT_Deregister;
        }
    }

    if (SetHeaders(piSCC) != IMS_SUCCESS)
    {
        IMS_TRACE_E(0, "Setting the generic headers failed", 0, 0, 0);
        goto EXIT_Deregister;
    }

    // Sets Contact headers
    if (SetContactNExpiresHeader(piSIPMsg, 0) != IMS_SUCCESS)
    {
        IMS_TRACE_E(0, "Setting Contact & Expires header failed", 0, 0, 0);
        goto EXIT_Deregister;
    }

    SetSubState(SUB_STATE_DEREGISTERING);

    // IMS_AUTH_NONCE_REUSE {
    // Set the credential information in case of re/de-registration
    SetNextAuthenticationInfo(piSCC);
    // }

    // SIP_MESSAGE_MEDIATOR
    (void)AdjustMessage(piSCC->GetMessage(), IMessageMediator::MESSAGE_NORMAL);

    // Send a REGISTER request to the network
    if (piSCC->Send() != IMS_SUCCESS)
    {
        IMS_TRACE_E(0, "Sending de-REGISTER request failed", 0, 0, 0);
        goto EXIT_Deregister;
    }

    // Update the refresh data
    pRefreshHelper->UpdateOnMessageSent(piSCC);

    // Clear the next request
    ClearNextRequest();

    // Store the ongoing SIP client connection
    SetOngoingConnection(piSCC);

    // Update the last request/response message
    SetPreviousRequest(piSCC->GetMessage());
    SetPreviousResponse(IMS_NULL);

    // Change the state
    UpdateBindingState(BINDING_DEREGISTERING);

    IMS::SetLastError(IMSError::NO_ERROR);

    //// DEBUG
    SipDebug::Send(GetSlotId(), SipDebug::MSG_REQ, SipDebug::DIR_OUT, SipDebug::M_DEREGISTER);

    return IMS_SUCCESS;

EXIT_Deregister:
    SetSubState(SUB_STATE_IDLE);
    piSCC->Close();

    return IMS_FAILURE;
}

/*

Remarks
 REG_RESTORATION_FOR_ACTIVE_BINDING
*/
PRIVATE VIRTUAL void Registration::RemoveActiveBindingsForcingly()
{
    //---------------------------------------------------------------------------------------------

    if (GetState() != STATE_ACTIVE)
    {
        IMS_TRACE_D("Registration :: No active bindings", 0, 0, 0);
        return;
    }

    if (GetSubState() != SUB_STATE_IDLE)
    {
        // FIXME: need to control the details

        SetSubState(SUB_STATE_IDLE);

        if (piOngoingSCC != IMS_NULL)
        {
            piOngoingSCC->Close();
            piOngoingSCC = IMS_NULL;
        }

        ClearNextRequest();
        SetPreviousRequest(IMS_NULL);
        SetPreviousResponse(IMS_NULL);
    }

    UpdateBindingState(BINDING_RESTORE);
}

/*

Remarks

*/
PRIVATE VIRTUAL void Registration::Restore()
{
    //---------------------------------------------------------------------------------------------

    IMS_TRACE_I("Registration::Restore - State (%s)", StateToString(GetState()), 0, 0);

    // 4 contact handling/ connection if present
    //  Reset the refresh timer
    pRefreshHelper->UpdateRefreshTimer(0);

    SetState(STATE_CREATED);
    SetSubState(SUB_STATE_IDLE);
    UpdateBindingState(BINDING_RESTORE);

    // Remove all the bindings if present
    for (IMS_UINT32 i = 0; i < objContacts.GetSize(); ++i)
    {
        RegContact* pContact = objContacts.GetAt(i);

        pContact->Restore();

        // IMS_IPSEC_UDP_ENC
        if (pRegParam->IsSecurityAssociationRequiredViaUDPEnc())
        {
            pContact->SetHostInfo(pContact->GetIPAddress());
        }
    }

    if (piOngoingSCC != IMS_NULL)
    {
        piOngoingSCC->Close();
        piOngoingSCC = IMS_NULL;
    }

    ClearNextRequest();
    SetPreviousRequest(IMS_NULL);
    SetPreviousResponse(IMS_NULL);

    pRegFlow->Restore();
    pRegParam->Restore();

    // IMS_AUTH_NONCE_REUSE {
    SetAuthenticationChallenge(IMS_NULL);
    // }

    // Abort the authentication procedure
    if (piDigestAKA != IMS_NULL)
    {
        piDigestAKA->Destroy();
        piDigestAKA = IMS_NULL;
    }

    // NAT_REQ_UE_PUBLIC_IP
    bIsBehindNAT = IMS_FALSE;
    pStateTracker->SetPublicIPAddress(IPAddress::NONE);

    if (NATHelper::IsNATResolverRequired())
    {
        NATHelper::GetInstance()->RemovePublicAddress(
                GetSlotId(), pRegFlow->GetRegKey().GetFlowId());
    }
}

/*

Remarks
 REG_RESTORATION_FOR_ACTIVE_BINDING
*/
PRIVATE VIRTUAL IMS_RESULT Registration::RestoreActiveBindings()
{
    //---------------------------------------------------------------------------------------------

    if (!bActiveBindingsRestorationEnabled)
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

/*

Remarks
 REG_RESTORATION_FOR_ACTIVE_BINDING
*/
PRIVATE VIRTUAL void Registration::SetActiveBindingsRestorationUsage(IN IMS_BOOL bEnabled)
{
    //---------------------------------------------------------------------------------------------

    bActiveBindingsRestorationEnabled = bEnabled;
}

/*

Remarks
 MULTI_SUBS
*/
PRIVATE VIRTUAL void Registration::SetAOR(
        IN CONST SipAddress& objAOR, IN CONST AString& strSubsId /* = AString::ConstNull() */)
{
    //---------------------------------------------------------------------------------------------

    pStateTracker->SetAOR(objAOR);
    pStateTracker->SetSubscriberId(strSubsId);

    // Update the profile
    pRegParam->UpdateProfile(objAOR, strSubsId);

    // Updates the user-info field in all Contacts
    const SipAddress& objTmpAOR = pStateTracker->GetAOR();

    for (IMS_UINT32 i = 0; i < objContacts.GetSize(); ++i)
    {
        RegContact* pContact = objContacts.GetAt(i);

        pContact->SetAOR(objTmpAOR);
    }
}

/*

Remarks

*/
PRIVATE VIRTUAL void Registration::SetListener(IN IRegistrationListener* piListener)
{
    //---------------------------------------------------------------------------------------------

    this->piListener = piListener;
}

/*

Remarks

*/
PRIVATE VIRTUAL void Registration::SetRefreshPolicy(IN IMS_SINT32 nPolicy,
        IN IMS_SINT32 nCriteriaInterval, IN IMS_SINT32 nValueEorLT, IN IMS_SINT32 nValueGT)
{
    //---------------------------------------------------------------------------------------------

    if (pRefreshHelper == IMS_NULL)
        return;

    switch (nPolicy)
    {
        case REFRESH_POLICY_NO_REFRESH:
            pRefreshHelper->SetPolicy(
                    RefreshHelper::POLICY_NO_REFRESH, nCriteriaInterval, nValueEorLT, nValueGT);
            break;

        case REFRESH_POLICY_SPEC:
            pRefreshHelper->SetPolicy(
                    RefreshHelper::POLICY_SPEC, nCriteriaInterval, nValueEorLT, nValueGT);
            break;

        case REFRESH_POLICY_REMAIN_TIME:
            pRefreshHelper->SetPolicy(
                    RefreshHelper::POLICY_REMAIN_TIME, nCriteriaInterval, nValueEorLT, nValueGT);
            break;

        case REFRESH_POLICY_RATIO:
            pRefreshHelper->SetPolicy(
                    RefreshHelper::POLICY_RATIO, nCriteriaInterval, nValueEorLT, nValueGT);
            break;

        default:
            IMS_TRACE_E(0, "Invalid refresh policy (%d)", nPolicy, 0, 0);
            break;
    }
}

/*

Remarks
 MULTI_REG_SIP_PROFILE
*/
PRIVATE VIRTUAL void Registration::SetSIPProfile(IN SipProfile* pProfile)
{
    //---------------------------------------------------------------------------------------------

    if (!pStateTracker.IsNull())
    {
        pStateTracker->SetSIPProfile(pProfile);
    }

    if (pRegParam != IMS_NULL)
    {
        pRegParam->UpdateSIPProfile(pProfile);
    }
}

/*

Remarks

*/
PRIVATE VIRTUAL void Registration::SetBindingStateListener(IN IRegBindingStateListener* piListener)
{
    //---------------------------------------------------------------------------------------------

    this->piBindingStateListener = piListener;
}

/*

Remarks

*/
PRIVATE VIRTUAL void Registration::SetFlagForWithinTrustDomain(IN IMS_BOOL bWithinTrustDomain)
{
    //---------------------------------------------------------------------------------------------

    if (bIsWithinTrustDomain != bWithinTrustDomain)
    {
        IMS_TRACE_I("WithinTrustDomain:: %s > %s", _TRACE_B_(bIsWithinTrustDomain),
                _TRACE_B_(bWithinTrustDomain), 0);

        bIsWithinTrustDomain = bWithinTrustDomain;
    }
}

/*

Remarks

*/
PRIVATE VIRTUAL void Registration::SetUserIdentityNotifier(
        IN IRegUserIdentityNotifier* piUserIdNotifier)
{
    //---------------------------------------------------------------------------------------------

    this->piUserIdNotifier = piUserIdNotifier;
}

/*

Remarks

*/
PRIVATE VIRTUAL void Registration::SetUserInfoForContactHeader(IN CONST AString& strUserInfo)
{
    //---------------------------------------------------------------------------------------------

    if (GetState() != STATE_ACTIVE)
    {
        IMS_TRACE_E(0, "Invalid state (%s)", StateToString(GetState()), 0, 0);
        return;
    }

    if (!pStateTracker.IsNull())
    {
        pStateTracker->SetUserInfoForContactHeader(strUserInfo);
    }
}

/*

Remarks

*/
PRIVATE VIRTUAL IRegSubscription* Registration::CreateSubscription(
        IN SipAddress* pResourceUri /* = IMS_NULL */)
{
    //---------------------------------------------------------------------------------------------

    if (GetState() != STATE_ACTIVE)
    {
        IMS_TRACE_E(0, "Invalid state (%s)", StateToString(GetState()), 0, 0);
        return IMS_NULL;
    }

    RegSubscription* pSubscription = new RegSubscription(pRegFlow->GetRegKey(), pStateTracker.Get(),
            pRefreshHelper->GetDuration(), pRegParam->GetSIPTimerValues());

    if (pSubscription == IMS_NULL)
    {
        return IMS_NULL;
    }

    SipAddress objResourceUri = GetAuthorizedAOR();

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
    RegInfo* pRegInfo = RegInfoManager::GetInstance()->GetRegInfo(pRegFlow->GetRegKey());

    if (pRegInfo != IMS_NULL)
    {
        pRegInfo->AddListener(this);
    }

    return pSubscription;
}

/*

Remarks

*/
PRIVATE VIRTUAL void Registration::ConnectionNotifierError_NotifyError(
        IN ISipConnectionNotifier* piSCN, IN IMS_SINT32 nCode, IN CONST AString& strMessage)
{
    //---------------------------------------------------------------------------------------------

    (void)piSCN;

    IMS_TRACE_D("ConnectionNotifierError_NotifyError :: code=%d, message=%s", nCode,
            strMessage.GetStr(), 0);

    if (GetState() != STATE_ACTIVE)
    {
        IMS_TRACE_D("Registration :: State is not in the ACTIVE; ignored ...", 0, 0, 0);
        return;
    }

    // REG_RESTORATION_FOR_ACTIVE_BINDING
    if (bActiveBindingsRestorationEnabled)
    {
        NetworkService* pNetworkService = NetworkService::GetNetworkService();
        INetworkConnection* piConnection =
                pNetworkService->FindConnection(pStateTracker->GetIPAddress());

        // Checks if the data connection is lost or not...
        // If the loss of data connection detects,
        // the error doesn't need to notify the application.
        if ((piConnection == IMS_NULL) ||
                ((piConnection != IMS_NULL) &&
                        (piConnection->GetState() != INetworkConnection::STATE_CONNECTED)))
        {
            IMS_TRACE_D("Registration :: Restoration(for active bindings) is enabled", 0, 0, 0);
            return;
        }
    }

    if (nCode == ISipConnectionNotifier::TRANSPORT_ERROR_TCP_CLIENT)
    {
        PostMessage(AMSG_REGISTRATION_TERMINATED, REASON_CLIENT_SOCKET_ERROR, 0);
    }
    else if (nCode == ISipConnectionNotifier::TRANSPORT_ERROR_TCP_SERVER)
    {
        PostMessage(AMSG_REGISTRATION_TERMINATED, REASON_SERVER_SOCKET_ERROR, 0);
    }
    else if (nCode == ISipConnectionNotifier::TRANSPORT_ERROR_UDP_SERVER)
    {
        PostMessage(AMSG_REGISTRATION_TERMINATED, REASON_SERVER_SOCKET_ERROR, 0);
    }
}

/*

Remarks

*/
PRIVATE VIRTUAL void Registration::AddObserver(IN RegObserver* pObserver)
{
    //---------------------------------------------------------------------------------------------

    for (IMS_UINT32 i = 0; i < objObservers.GetSize(); ++i)
    {
        RegObserver* pTmpObserver = objObservers.GetAt(i);

        if (pObserver == pTmpObserver)
            return;
    }

    objObservers.Append(pObserver);
}

/*

Remarks

*/
PRIVATE VIRTUAL void Registration::RemoveObserver(IN RegObserver* pObserver)
{
    //---------------------------------------------------------------------------------------------

    for (IMS_UINT32 i = 0; i < objObservers.GetSize(); ++i)
    {
        RegObserver* pTmpObserver = objObservers.GetAt(i);

        if (pObserver == pTmpObserver)
        {
            objObservers.RemoveAt(i);
            return;
        }
    }
}

/*

Remarks

*/
PRIVATE VIRTUAL IMS_SINT32 Registration::AddReferenceForSCNEL()
{
    //---------------------------------------------------------------------------------------------

#ifdef __IMS_DEBUG__
    IMS_TRACE_D("Registration :: SCNEL (add) - %d >> %d", nRefCountForSCNEL,
            (nRefCountForSCNEL + 1), 0);
#endif

    ++nRefCountForSCNEL;

    return nRefCountForSCNEL;
}

/*

Remarks

*/
PRIVATE VIRTUAL IMS_SINT32 Registration::RemoveReferenceForSCNEL()
{
    //---------------------------------------------------------------------------------------------

    if (nRefCountForSCNEL > 0)
    {
#ifdef __IMS_DEBUG__
        IMS_TRACE_D("Registration :: SCNEL (remove) - %d >> %d", nRefCountForSCNEL,
                (nRefCountForSCNEL - 1), 0);
#endif

        --nRefCountForSCNEL;

        if (nRefCountForSCNEL == 0)
        {
            IMS_TRACE_D("Registration :: SCNEL (0)", 0, 0, 0);
        }
    }

    return nRefCountForSCNEL;
}

/*

Remarks

*/
PRIVATE VIRTUAL void Registration::NotifyCallerCapabilityChanged()
{
    //---------------------------------------------------------------------------------------------

    PostMessage(AMSG_REGISTRATION_BINDING_STATE_CHANGED, BINDING_STATE_CALLER_CAPABILITY, 0);
}

/*

Remarks

*/
PRIVATE VIRTUAL void Registration::Refreshable_RefreshCompleted(
        IN ISipClientConnection* piSCC, IN IMS_SINT32 nCode /* = 0 */)
{
    IMS_SINT32 nStatusCode = piSCC->GetStatusCode();

    //---------------------------------------------------------------------------------------------

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

        SetPreviousResponse(piSCC->GetMessage());

        // Authentication challenged by S-CSCF
        if ((nStatusCode == SipStatusCode::SC_401) || (nStatusCode == SipStatusCode::SC_407))
        {
            // IMS_AUTH_NONCE_REUSE {
            SetAuthenticationChallenge(IMS_NULL);
            // }

            // Update the Security-xxx headers
            pRegParam->UpdateSecurityHeaders(piSCC->GetMessage());

            // Respond to the authentication challenge
            if (!RespondToChallenge(piSCC))
            {
                RestoreSecurityHeaders();
            }

            if (GetSubState() == SUB_STATE_IDLE)
            {
                pRefreshHelper->AbortConnection();
            }

            // 3 Do not close SIPConnection in here
            return;
        }

        // Reset the flags after the transaction completed
        SetSubState(SUB_STATE_IDLE);

        ISipMessage* piSIPMsg = GetPreviousResponse();

        if (SipStatusCode::IsFinalSuccess(nStatusCode))
        {
            // Update the Security-xxx headers
            pRegParam->UpdateSecurityHeaders(piSIPMsg);

            // Update the sequence number
            UpdateCSeqNumber(piSIPMsg);

            // Update the bindings
            IMS_RESULT nBindingUpdateResult = UpdateBindings(piSIPMsg);
            IMS_SINT32 nReason = REASON_NONE;

            if (nBindingUpdateResult == BINDING_UPDATE_NO_EXPIRES)
            {
                nReason = REASON_NO_EXPIRES;
            }

            // Update the refresh timer
            UpdateRefreshTimer();

            // Check the UA location
            CheckUALocation(piSIPMsg);

            // Store the persistent header information
            StorePersistentHeaders(piSIPMsg);

            // IMS_AUTH_NONCE_REUSE {
            SetNextNonce(piSIPMsg);
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
                UpdateBindingState(BINDING_RESULT_OK);
            else
                UpdateBindingState(BINDING_RESULT_NOK);

            CallListener(nPrevState, nPrevSubState, nReason);
        }
        else
        {
            // 140225, hwangoo.park
            // Do not clear the authentication challenge...
            // Initial registration required: 305 / 408 / 500 / 504 / txn timeout
            // // IMS_AUTH_NONCE_REUSE {
            // SetAuthenticationChallenge(IMS_NULL);
            // // }

            // 13.01.29, hwangoo.park
            // Do not check the status code to support re-use of Call-ID & CSeq number
            // when the registration is failed
            // if (nStatusCode == SipStatusCode::SC_423)
            {
                // Update the sequence number
                UpdateCSeqNumber(piSIPMsg);
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

/*

Remarks

*/
PRIVATE VIRTUAL IMS_BOOL Registration::Refreshable_RefreshStarted()
{
    IMS_BOOL bDoImplicitRefresh = IMS_TRUE;

    //---------------------------------------------------------------------------------------------

    IMS_TRACE_I("_____ REGISTRATION REFRESH STARTED ... State(%d)", nState, 0, 0);

    if (IsBindingsUpdating())
    {
        IMS_TRACE_D("Registration has been already requested by a service ...", 0, 0, 0);
        return IMS_TRUE;
    }

    if (piListener != IMS_NULL)
    {
        piListener->Registration_RefreshTimerExpired(bDoImplicitRefresh);
    }

    if (bDoImplicitRefresh)
    {
        return RegisterOnImplicitRefresh();
    }

    return IMS_TRUE;
}

/*

Remarks

*/
PRIVATE VIRTUAL void Registration::Refreshable_RefreshTerminated()
{
    //---------------------------------------------------------------------------------------------

    IMS_TRACE_I("_____ REGISTRATION REFRESH TERMINATED ...", 0, 0, 0);

    // IMS_AUTH_NONCE_REUSE {
    SetAuthenticationChallenge(IMS_NULL);
    // }

    SetState(STATE_TERMINATED);
    PostMessage(AMSG_REGISTRATION_TERMINATED, REASON_REFRESH_TIMEOUT, 0);
}

/*

Remarks

*/
PRIVATE VIRTUAL void Registration::RegCapabilityChange_ServiceAdded(
        IN CONST AString& strAppId, IN CONST AString& strServiceId)
{
    AStringArray objHeaders;

    //---------------------------------------------------------------------------------------------

    RegBindingProxy::QueryRegistrationHeaders(GetSlotId(), strAppId, strServiceId, objHeaders);

    // Adds extra headers
    if (!objHeaders.IsEmpty())
    {
        IRegParameter* piRegParameter = pRegParam;

        piRegParameter->AddExtraHeaders(objHeaders);
    }
}

/*

Remarks

*/
PRIVATE VIRTUAL void Registration::RegCapabilityChange_ServiceRemoved(
        IN CONST AString& strAppId, IN CONST AString& strServiceId)
{
    AStringArray objHeaders;

    //---------------------------------------------------------------------------------------------

    RegBindingProxy::QueryRegistrationHeaders(GetSlotId(), strAppId, strServiceId, objHeaders);

    // Removes extra headers
    if (!objHeaders.IsEmpty())
    {
        IRegParameter* piRegParameter = pRegParam;

        piRegParameter->RemoveExtraHeaders(objHeaders);
    }
}

/*

Remarks

*/
PRIVATE VIRTUAL void Registration::RegInfo_Updated(IN IMS_BOOL bStale /* = IMS_FALSE */)
{
    //---------------------------------------------------------------------------------------------

    (void)bStale;

    if (GetState() != STATE_ACTIVE)
    {
        return;
    }

    // Check the 'state' & 'event' reg info.
    RegInfo* pRegInfo = RegInfoManager::GetInstance()->GetRegInfo(pRegFlow->GetRegKey());
    IRegInfoRegistration* piRegInfoReg = IMS_NULL;

    if (pRegInfo == IMS_NULL)
    {
        return;
    }

    IMS_TRACE_D("RegInfo :: Updated", 0, 0, 0);

    const SipAddress& objAOR = pStateTracker->GetAuthorizedAOR();

    piRegInfoReg = pRegInfo->GetRegistration(objAOR);

    if (piRegInfoReg == IMS_NULL)
    {
        IMS_TRACE_E(0, "Cannot find the RegInfo (%s)",
                SipDebug::GetCharA1(objAOR.GetUser().GetStr(), 5), 0, 0);
        return;
    }

    IMS_BOOL bUpdateRefreshTimer = IMS_FALSE;
    IMS_UINT32 nShortenedExpires = 0;
    IMSList<IRegInfoContact*> objRegInfoContacts = piRegInfoReg->GetContacts();

    for (IMS_UINT32 i = 0; i < objRegInfoContacts.GetSize(); ++i)
    {
        const IRegInfoContact* piRegInfoContact = objRegInfoContacts.GetAt(i);

        if (piRegInfoContact == IMS_NULL)
            continue;

        IMS_SINT32 nState = piRegInfoContact->GetState();
        IMS_SINT32 nEvent = piRegInfoContact->GetEvent();
        IMS_UINT32 nExpires = piRegInfoContact->GetExpiresValue();

        if ((nState == IRegInfoContact::STATE_ACTIVE) &&
                (nEvent == IRegInfoContact::EVENT_SHORTENED) && (nExpires != 0))
        {
            for (IMS_UINT32 j = 0; j < objContacts.GetSize(); ++j)
            {
                RegContact* pContact = objContacts.GetAt(j);

                if (pContact == IMS_NULL)
                    continue;

                if (pContact->GetContactAddress().Equals(piRegInfoContact->GetURI()))
                {
                    bUpdateRefreshTimer = IMS_TRUE;

                    if (nShortenedExpires == 0)
                        nShortenedExpires = nExpires;
                    else if (nShortenedExpires > nExpires)
                        nShortenedExpires = nExpires;
                }
            }
        }
    }

    if (bUpdateRefreshTimer)
    {
        IMS_TRACE_D("RegInfo :: Active & Shortened (%d)", nShortenedExpires, 0, 0);
        pRefreshHelper->UpdateRefreshTimer(static_cast<IMS_SINT32>(nShortenedExpires));
    }
}

/*

Remarks

*/
PRIVATE VIRTUAL void Registration::RegInfo_UpdateFailed()
{
    //---------------------------------------------------------------------------------------------

    // 4 nothing to do in here
}

/*

Remarks

*/
PRIVATE VIRTUAL void Registration::DigestAka_OnResponse(IN const ByteArray& objRES,
        IN const ByteArray& objIK /* = ByteArray::ConstNull() */,
        IN const ByteArray& objCK /* = ByteArray::ConstNull() */)
{
    //---------------------------------------------------------------------------------------------

    IMS_TRACE_I("Registration :: AKA RES received", 0, 0, 0);

    if (GetSubState() == SUB_STATE_IDLE)
    {
        IMS_TRACE_E(0, "Invalid sub state (%d)", GetSubState(), 0, 0);
        return;
    }

    objActiveCredential = pRegParam->GetCredential();
    objActiveSAKey.SetIK(objIK);
    objActiveSAKey.SetCK(objCK);

    IMS_SINT32 nType = objActiveCredential.GetType();

    if (nType == Credential::TYPE_AKAv1_MD5)
    {
        objActiveCredential.SetAKAResponse(IMS_AKA::RESULT_OK, objRES);
    }
    else if (nType == Credential::TYPE_AKAv2_MD5)
    {
        objActiveCredential.SetAKAResponse(IMS_AKA::RESULT_OK, objRES, objIK, objCK);
    }

    PostMessage(AMSG_REGISTRATION_AKA_RESPONSE_RECEIVED, 0, 0);
}

/*

Remarks

*/
PRIVATE VIRTUAL void Registration::DigestAka_OnAutsFailed(IN const ByteArray& objAUTS)
{
    //---------------------------------------------------------------------------------------------

    IMS_TRACE_I("Registration :: AUTS failed", 0, 0, 0);

    if (GetSubState() == SUB_STATE_IDLE)
    {
        IMS_TRACE_E(0, "Invalid sub state (%d)", GetSubState(), 0, 0);
        return;
    }

    objActiveCredential = pRegParam->GetCredential();
    objActiveSAKey.SetIK(ByteArray::ConstNull());
    objActiveSAKey.SetCK(ByteArray::ConstNull());

    IMS_SINT32 nType = objActiveCredential.GetType();

    if (nType == Credential::TYPE_AKAv1_MD5)
    {
        objActiveCredential.SetAKAResponse(
                IMS_AKA::RESULT_NOK_SQN_SYNC_FAILED, ByteArray::ConstNull(), objAUTS);
    }
    else if (nType == Credential::TYPE_AKAv2_MD5)
    {
        objActiveCredential.SetAKAResponse(IMS_AKA::RESULT_NOK_SQN_SYNC_FAILED,
                ByteArray::ConstNull(), ByteArray::ConstNull(), ByteArray::ConstNull(), objAUTS);
    }

    PostMessage(AMSG_REGISTRATION_AKA_RESPONSE_RECEIVED, 0, 0);
}

/*

Remarks

*/
PRIVATE VIRTUAL void Registration::DigestAka_OnMacFailed()
{
    //---------------------------------------------------------------------------------------------

    IMS_TRACE_I("Registration :: MAC failed", 0, 0, 0);

    if (GetSubState() == SUB_STATE_IDLE)
    {
        IMS_TRACE_E(0, "Invalid sub state (%d)", GetSubState(), 0, 0);
        return;
    }

    objActiveCredential = pRegParam->GetCredential();
    objActiveSAKey.SetIK(ByteArray::ConstNull());
    objActiveSAKey.SetCK(ByteArray::ConstNull());

    IMS_SINT32 nType = objActiveCredential.GetType();

    if (nType == Credential::TYPE_AKAv1_MD5)
    {
        objActiveCredential.SetAKAResponse(IMS_AKA::RESULT_NOK_MAC_INVALID, ByteArray::ConstNull());
    }
    else if (nType == Credential::TYPE_AKAv2_MD5)
    {
        objActiveCredential.SetAKAResponse(IMS_AKA::RESULT_NOK_MAC_INVALID, ByteArray::ConstNull(),
                ByteArray::ConstNull(), ByteArray::ConstNull());
    }

    PostMessage(AMSG_REGISTRATION_AKA_RESPONSE_RECEIVED, 0, 0);
}

/*

Remarks
 SIP_MESSAGE_MEDIATOR
*/
PRIVATE
IMS_RESULT Registration::AdjustMessage(IN_OUT ISipMessage* piSIPMsg,
        IN IMS_SINT32 nMessage /* = IMessageMediator::MESSAGE_NORMAL */)
{
    if (piMessageMediator == IMS_NULL)
    {
        return IMS_FAILURE;
    }

    return piMessageMediator->MessageMediator_AdjustMessage(piSIPMsg, nMessage);
}

/*

Remarks

*/
PRIVATE
void Registration::CallListener(
        IN IMS_SINT32 nPrevState, IN IMS_SINT32 nPrevSubState, IN IMS_SINT32 nReason)
{
    //---------------------------------------------------------------------------------------------

    if (piListener == IMS_NULL)
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

        case STATE_TERMINATED:
            break;

        default:
            break;
    }
}

/*

Remarks

*/
PRIVATE
void Registration::CheckUALocation(IN ISipMessage* piSIPMsg)
{
    //---------------------------------------------------------------------------------------------

    if (piSIPMsg == IMS_NULL)
    {
        return;
    }

    AString strViaHdr = piSIPMsg->GetHeader(ISipHeader::VIA);
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
        if (bIsBehindNAT && pRegParam->IsSecurityAssociationPresent())
        {
            // Don't clear the flag.
        }
        else
        {
            bIsBehindNAT = IMS_FALSE;
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
        if (bIsBehindNAT && pRegParam->IsSecurityAssociationPresent())
        {
            // Don't clear the flag.
        }
        else
        {
            bIsBehindNAT = IMS_FALSE;
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
    IPAddress objSentBy(strHost);
    IPAddress objReceived(pParameter->GetValue());
    IMS_BOOL bIsPublicIPUpdateRequired = IMS_TRUE;
    IMS_BOOL bIsSentByPublicIP = IMS_FALSE;

    if (!IPAddress::NONE.Equals(pStateTracker->GetPublicIPAddress()) &&
            objSentBy.Equals(pStateTracker->GetPublicIPAddress()))
    {
        bIsSentByPublicIP = IMS_TRUE;
    }

    // NAT_REQ_UE_PUBLIC_IP
    if (bIsBehindNAT && pRegParam->IsSecurityAssociationPresent() && bIsSentByPublicIP)
    {
        bIsPublicIPUpdateRequired = IMS_FALSE;
    }
    else
    {
        if (objSentBy.IsIPv6Address() && objReceived.IsIPv6Address())
        {
            bIsBehindNAT = !objSentBy.Equals(objReceived);
        }
        else if (objSentBy.IsIPv4Address() && objReceived.IsIPv4Address())
        {
            bIsBehindNAT = !objSentBy.Equals(objReceived);
        }
        else
        {
            bIsBehindNAT = !strHost.Equals(pParameter->GetValue());
        }
    }

    piHeader->Destroy();

    IMS_TRACE_D("UA is %s behind on a NAT", (bIsBehindNAT) ? "located" : "not located", 0, 0);

    // NAT_REQ_UE_PUBLIC_IP
    if (bIsPublicIPUpdateRequired)
    {
        if (bIsBehindNAT)
        {
            pStateTracker->SetPublicIPAddress(objReceived);
        }
        else
        {
            pStateTracker->SetPublicIPAddress(IPAddress::NONE);
        }

        if (NATHelper::IsNATResolverRequired())
        {
            NATHelper::GetInstance()->SetPublicAddress(GetSlotId(),
                    pRegFlow->GetRegKey().GetFlowId(), objSentBy,
                    (bIsBehindNAT ? objReceived : IPAddress::NONE));
        }
    }
}

/*

Remarks

*/
PRIVATE
void Registration::ChoosePreferredContact()
{
    RegContact* pPreferredContact = IMS_NULL;

    //---------------------------------------------------------------------------------------------

    for (IMS_UINT32 i = 0; i < objContacts.GetSize(); ++i)
    {
        RegContact* pContact = objContacts.GetAt(i);

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

    pStateTracker->SetPreferredContact(pPreferredContact);
}

/*

Remarks

*/
PRIVATE
void Registration::ClearNextRequest()
{
    //---------------------------------------------------------------------------------------------

    if (piNextRequest != IMS_NULL)
    {
        piNextRequest->Destroy();
        piNextRequest = IMS_NULL;
    }
}

/*

Remarks

*/
PRIVATE
IDigestAka* Registration::CreateDigestAKA(IN CONST SubscriberConfig* pSubsConfig)
{
    //---------------------------------------------------------------------------------------------

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

/*

Remarks

*/
PRIVATE
IMS_BOOL Registration::CreateSA(IN CONST Credential& objCredential, IN CONST IMS_SA_KEY& objSAKey)
{
    IMS_SINT32 nAlgorithm = objCredential.GetType();

    //---------------------------------------------------------------------------------------------

    if ((nAlgorithm != Credential::TYPE_AKAv1_MD5) && (nAlgorithm != Credential::TYPE_AKAv2_MD5))
    {
        IMS_TRACE_D("Authentication algorithm(%d) is not AKA", nAlgorithm, 0, 0);
        return IMS_TRUE;
    }

    // Checks if the security association is required or not...
    if (!pRegParam->IsSecurityAssociationRequired())
    {
        IMS_TRACE_D("Security association is not required ...", 0, 0, 0);
        return IMS_TRUE;
    }

    if (piListener == IMS_NULL)
    {
        IMS_TRACE_D("Security association is not required ... ; no listener", 0, 0, 0);
        return IMS_TRUE;
    }

    IMS_BOOL bResultOfSA = IMS_TRUE;

    if (SipConfigProxy::IsIpSecConfigured(GetSlotId(), pStateTracker->GetSIPProfile()))
    {
        IMSList<SipSecurityHeader> objSecurityVerifys;

        pRegParam->SetSecurityVerifys(objSecurityVerifys);

        // Notifies the information which IK & CK will be used for this authentication.
        piListener->Registration_NotifyAKAResponse(objCredential.GetAKAResponse().nStatus,
                objSAKey.GetIK(), objSAKey.GetCK(), bResultOfSA);

        // If the result of AKA is not OK, clear the Security-Server headers
        if (!bResultOfSA || (objCredential.GetAKAResponse().nStatus != IMS_AKA::RESULT_OK))
        {
            RestoreSecurityHeaders();
        }

        // Updates the transport extension rules for SIP layer
        pRegParam->SetTransportExtForIPSec();
    }
    else
    {
        // Notifies the information which IK & CK will be used for this authentication.
        piListener->Registration_NotifyAKAResponse(objCredential.GetAKAResponse().nStatus,
                objSAKey.GetIK(), objSAKey.GetCK(), bResultOfSA);
    }

    return bResultOfSA;
}

/*

Remarks

*/
PRIVATE
IMS_SINT32 Registration::GetPortUC() const
{
    IMS_SINT32 nPortUC = pRegParam->GetProtectedPortUC();

    //---------------------------------------------------------------------------------------------

    // Check if the UE supports the Security Association Agreement.
    // If nPortUC returns Sip::PORT_UNSPECIFIED, then it does not support "sec-agree".
    if (nPortUC == Sip::PORT_UNSPECIFIED)
    {
        IMS_SINT32 nState = GetState();
        IMS_SINT32 nPrevState = GetSubState();

        if ((nState == STATE_CREATED) || ((nState == STATE_INIT) && (nPrevState != SUB_STATE_IDLE)))
        {
            return (pRegParam != IMS_NULL) ? pRegParam->GetPort() : Sip::PORT_5060;
        }

        return GetPortUS();
    }
    else if (nPortUC == 0)
    {
        if (!SipConfigProxy::IsIpSecConfigured(GetSlotId(), pStateTracker->GetSIPProfile()))
        {
            return GetPortUS();
        }
    }

    return nPortUC;
}

/*

Remarks

*/
PRIVATE
IMS_SINT32 Registration::GetPortUS() const
{
    const RegContact* pContact = pStateTracker->GetPreferredContact();

    //---------------------------------------------------------------------------------------------

    if (pContact == IMS_NULL)
    {
        if (pRegParam != IMS_NULL)
        {
            return pRegParam->GetPort();
        }

        return Sip::PORT_5060;
    }

    return pContact->GetPort();
}

/*

Remarks

*/
PRIVATE
const SubscriberConfig* Registration::GetSubsConfig() const
{
    const SubscriberConfig* pSubsConfig = IMS_NULL;
    const AString& strSubsId = pStateTracker->GetSubscriberId();
    ConfigurationManager* pConfigMngr = ConfigurationManager::GetInstance();

    //---------------------------------------------------------------------------------------------

    if (strSubsId.GetLength() > 0)
    {
        pSubsConfig = pConfigMngr->GetSubscriberConfig(strSubsId, GetSlotId());
    }
    else
    {
        const SipAddress& objAOR = pStateTracker->GetAOR();
        const AString& strId =
                SubscriberTracker::GetInstance()->GetSubscriberId(GetSlotId(), &objAOR);
        pSubsConfig = pConfigMngr->GetSubscriberConfig(strId, GetSlotId());
    }

    return pSubsConfig;
}

/*

Remarks

*/
PRIVATE
IMS_SINT32 Registration::GetSubState() const
{
    //---------------------------------------------------------------------------------------------

    return nSubState;
}

/*

Remarks

*/
PRIVATE
IMS_BOOL Registration::IsAkaSupported(IN CONST SubscriberConfig* pSubsConfig) const
{
    //---------------------------------------------------------------------------------------------

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

/*

Remarks

*/
PRIVATE
IMS_BOOL Registration::IsFlowControlRequired() const
{
    //---------------------------------------------------------------------------------------------

    if (pRegParam->GetFlowControlOption() == IRegParameter::FLOW_CONTROL_BY_PROVISION)
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
    const RegContact* pRegContact = pStateTracker->GetPreferredContact();

    if (SipConfigProxy::IsMultipleRegConfigured(GetSlotId(), pStateTracker->GetSIPProfile()) &&
            (piRequest != IMS_NULL) && (piResponse != IMS_NULL) && (pRegContact != IMS_NULL))
    {
        const AString OUTBOUND("outbound");

        if ((pRegContact->GetInstanceParameter() != IMS_NULL) &&
                (pRegContact->GetRegIdParameter() != IMS_NULL) &&
                piRequest->IsOptionSupported(OUTBOUND) && piResponse->IsOptionRequired(OUTBOUND))
        {
            return IMS_TRUE;
        }
    }

    return IMS_FALSE;
}

/*

Remarks

*/
PRIVATE
IMS_RESULT Registration::ReformContactHeader(IN_OUT ISipMessage* piSIPMsg)
{
    IMS_SINT32 nHCount = piSIPMsg->GetHeaderCount(ISipHeader::CONTACT_NORMAL);

    //---------------------------------------------------------------------------------------------

    while (nHCount > 0)
    {
        piSIPMsg->RemoveHeader(ISipHeader::CONTACT_NORMAL);
        --nHCount;
    }

    return SetContactNExpiresHeader(piSIPMsg);
}

/*

Remarks

*/
PRIVATE
IMS_BOOL Registration::RegisterOnImplicitRefresh()
{
    ISipClientConnection* piSCC = CreateConnection(this);

    //---------------------------------------------------------------------------------------------

    if (piSCC == IMS_NULL)
    {
        IMS_TRACE_E(0, "Creating a new SIP connection for registration refresh failed", 0, 0, 0);
        return IMS_FALSE;
    }

    // Form the generic headers
    if (SetHeaders(piSCC) != IMS_SUCCESS)
    {
        IMS_TRACE_E(0, "Setting the generic headers failed", 0, 0, 0);
        goto EXIT_RegisterOnImplicitRefresh;
    }

    // Sets Contact headers
    if (SetContactNExpiresHeader(piSCC->GetMessage()) != IMS_SUCCESS)
    {
        IMS_TRACE_E(0, "Setting Contact & Expires header failed", 0, 0, 0);
        goto EXIT_RegisterOnImplicitRefresh;
    }

    // IMS_AUTH_NONCE_REUSE {
    // Set the credential information in case of re/de-registration
    SetNextAuthenticationInfo(piSCC);
    // }

    // Send a refresh REGISTER request to the network
    if (pRefreshHelper->SendRefreshRequest(piSCC) != IMS_SUCCESS)
    {
        IMS_TRACE_E(0, "Sending a registration refresh request failed", 0, 0, 0);
        goto EXIT_RegisterOnImplicitRefresh;
    }

    SetSubState(SUB_STATE_REFRESHING);
    UpdateBindingState(BINDING_REGISTERING);

    // Update the last request/response message
    SetPreviousRequest(piSCC->GetMessage());
    SetPreviousResponse(IMS_NULL);

    //// DEBUG
    SipDebug::Send(GetSlotId(), SipDebug::MSG_REQ, SipDebug::DIR_OUT, SipMethod::REGISTER);

    return IMS_TRUE;

EXIT_RegisterOnImplicitRefresh:
    piSCC->Close();

    return IMS_FALSE;
}

/*

Remarks

*/
PRIVATE
IMS_BOOL Registration::RespondToChallenge(IN ISipClientConnection* piSCC)
{
    IMS_SINT32 nPrevState = GetState();
    IMS_SINT32 nPrevSubState = GetSubState();
    IMS_BOOL bResponseToChallenge = IMS_FALSE;
    IMS_SINT32 nAlgorithm = Credential::TYPE_MD5;
    ISipGenericChallenge* pChallenge = piSCC->GetAuthenticationChallenge();

    //---------------------------------------------------------------------------------------------

    // Reset the flags after the transaction completed
    SetSubState(SUB_STATE_IDLE);

    if (pChallenge != IMS_NULL)
    {
        if (piListener != IMS_NULL)
        {
            nAlgorithm = Credential::TranslateAlgorithm(pChallenge->GetAlgorithm());

            piListener->Registration_AuthenticationChallenged(nAlgorithm, bResponseToChallenge);
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
                    pStateTracker->GetSubscriberId().GetStr(), 0, 0);

            if (nPrevState == STATE_INIT)
            {
                SetState(STATE_CREATED);
            }

            UpdateBindingState(BINDING_RESULT_NOK);

            CallListener(nPrevState, nPrevSubState, REASON_INTERNAL_ERROR);
            return IMS_FALSE;
        }

        Credential& objCredential = pRegParam->GetCredential();

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
            if (piDigestAKA == IMS_NULL)
            {
                piDigestAKA = CreateDigestAKA(pSubsConfig);
            }

            if (piDigestAKA == IMS_NULL)
            {
                if (nPrevState == STATE_INIT)
                {
                    SetState(STATE_CREATED);
                }

                UpdateBindingState(BINDING_RESULT_NOK);

                CallListener(nPrevState, nPrevSubState, REASON_STATUS_CODE);
                return IMS_FALSE;
            }

            piDigestAKA->SetListener(this);

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

            if (piDigestAKA->GetAuthResponse(objChallenge) != IMS_SUCCESS)
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
#ifdef __IMS_TEST_AKA__
        else if (nAlgorithm == Credential::TYPE_AKAv1_MD5)
        {
            AKAHelper objAKAHelper;
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

            objAKAHelper.Authenticate(objCredential.GetPassword(), objChallenge);

            // Update the 'realm' parameter from WWW-Authenticate header
            objCredential.SetRealm(pChallenge->GetRealm());

            objActiveCredential = objCredential;
            objActiveSAKey.SetIK(objAKAHelper.GetIK());
            objActiveSAKey.SetCK(objAKAHelper.GetCK());

            objActiveCredential.SetAKAResponse(IMS_AKA::RESULT_OK, objAKAHelper.GetRES());

            // Set the sub-state and waits the response from ISIM module
            SetSubState(nPrevSubState);

            // IMS_AUTH_NONCE_REUSE {
            // Keep the authentication challenge for re/de-registration
            SetAuthenticationChallenge(pChallenge);
            // }

            PostMessage(AMSG_REGISTRATION_AKA_RESPONSE_RECEIVED, 0, 0);

            return IMS_TRUE;
        }
#endif
        else
        {
            // do nothing !!!
        }

        IMS_TRACE_D("RespondToChallenge :: Respond to MD5 authentication challenge ...", 0, 0, 0);

        // Overwrite the realm parameter if it required
        if (pRegParam->IsAuthRealmLenient() &&
                (!objCredential.GetRealm().Equals(pChallenge->GetRealm())))
        {
            IMS_TRACE_D("auth_realm_leniency is true; %s -> %s",
                    SipDebug::GetCharA1(objCredential.GetRealm().GetStr(), 4),
                    SipDebug::GetCharA2(pChallenge->GetRealm().GetStr(), 4), 0);

            objCredential.SetRealm(pChallenge->GetRealm());
        }

        if (piSCC->InitResubmissionRequest() != IMS_SUCCESS)
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
        if (piSCC->SetCredentials(objCredential) != IMS_SUCCESS)
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
        (void)AdjustMessage(piSCC->GetMessage(), IMessageMediator::MESSAGE_RESUBMIT);

        if (piSCC->Send() != IMS_SUCCESS)
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
            objActiveCredential = objCredential;

            // Keep the authentication challenge for re/de-registration
            SetAuthenticationChallenge(pChallenge);
        }
        // }

        // Update the last request/response message
        SetPreviousRequest(piSCC->GetMessage());
        SetPreviousResponse(IMS_NULL);

        // Remove the previous credential information for re-authentication
        piSCC->RemoveAllCredentials();
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

/*

Remarks

*/
PRIVATE
IMS_BOOL Registration::RespondToPendingChallenge(IN CONST Credential& objCredential)
{
    IMS_SINT32 nPrevState = GetState();
    IMS_SINT32 nPrevSubState = GetSubState();

    //---------------------------------------------------------------------------------------------

    IMS_TRACE_D("Start to respond to AKA authentication challenge ...", 0, 0, 0);

    if ((nPrevSubState == SUB_STATE_REGISTERING) || (nPrevSubState == SUB_STATE_DEREGISTERING))
    {
        if (piOngoingSCC == IMS_NULL)
        {
            if (nPrevState == STATE_INIT)
            {
                SetState(STATE_CREATED);
            }

            UpdateBindingState(BINDING_RESULT_NOK);

            CallListener(nPrevState, nPrevSubState, REASON_INTERNAL_ERROR);
            return IMS_FALSE;
        }

        if (piOngoingSCC->InitResubmissionRequest() != IMS_SUCCESS)
        {
            piOngoingSCC->Close();
            piOngoingSCC = IMS_NULL;

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
        IMS_SINT32 nPortUS = GetPortUS();
        IMS_SINT32 nPortFlowControl = Sip::PORT_UNSPECIFIED;
        IMS_SINT32 nTransportExt = pRegParam->GetTransportExtForRegOnly();

        if (nPrevState == STATE_INIT)
        {
            if (!pRegParam->IsSecurityAssociationPresent())
            {
                nPortUS = GetPortUC();
            }
            else
            {
                // Update the protected server port if SA established
                UpdateProtectedServerPortForContact(piOngoingSCC->GetMessage());
            }

            nPortFlowControl = pRegParam->GetPortFlowControl();
            nTransportExt |= pRegParam->GetTransportExt();
        }
        else
        {
            nPortFlowControl = pStateTracker->GetPortFlowControl();
            nTransportExt |= pStateTracker->GetTransportExt();
        }

        piOngoingSCC->SetTransportTuple(pStateTracker->GetIPAddress(), nPortUS, GetPortUC(),
                nPortFlowControl, nTransportExt);

        // IMS_IPSEC_UDP_ENC
        if ((nPrevSubState == SUB_STATE_REGISTERING) &&
                pRegParam->IsSecurityAssociationRequiredViaUDPEnc())
        {
            UpdateHostInfoInAllContacts();

            if (ReformContactHeader(piOngoingSCC->GetMessage()) != IMS_SUCCESS)
            {
                piOngoingSCC->Close();
                piOngoingSCC = IMS_NULL;

                if (nPrevState == STATE_INIT)
                {
                    SetState(STATE_CREATED);
                }

                UpdateBindingState(BINDING_RESULT_NOK);

                CallListener(nPrevState, nPrevSubState, REASON_INTERNAL_ERROR);
                return IMS_FALSE;
            }
        }

        if (pRegParam->FormRouteHeaders(piOngoingSCC, pStateTracker) != IMS_SUCCESS)
        {
            piOngoingSCC->Close();
            piOngoingSCC = IMS_NULL;

            if (nPrevState == STATE_INIT)
            {
                SetState(STATE_CREATED);
            }

            UpdateBindingState(BINDING_RESULT_NOK);

            CallListener(nPrevState, nPrevSubState, REASON_INTERNAL_ERROR);
            return IMS_FALSE;
        }

        if (pRegParam->FormSecurityHeaders(piOngoingSCC) != IMS_SUCCESS)
        {
            piOngoingSCC->Close();
            piOngoingSCC = IMS_NULL;

            if (nPrevState == STATE_INIT)
            {
                SetState(STATE_CREATED);
            }

            UpdateBindingState(BINDING_RESULT_NOK);

            CallListener(nPrevState, nPrevSubState, REASON_INTERNAL_ERROR);
            return IMS_FALSE;
        }

        if (piOngoingSCC->SetCredentials(objCredential) != IMS_SUCCESS)
        {
            piOngoingSCC->Close();
            piOngoingSCC = IMS_NULL;

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
                    GetSlotId(), pStateTracker->GetSIPProfile()))
        {
            ISipMessage* piSIPMsg = piOngoingSCC->GetMessage();
            PAccessNetworkInfoHeader::SetHeader(GetSlotId(), pStateTracker->GetIPAddress(),
                    pStateTracker->GetSIPProfile(), piSIPMsg);
        }
        else
        {
            if ((nPrevState == STATE_INIT) &&
                    (objCredential.GetAKAResponse().nStatus != IMS_AKA::RESULT_OK))
            {
                // Do not add P-Access-Network-Info header since the authentication is failed...
            }
            else
            {
                ISipMessage* piSIPMsg = piOngoingSCC->GetMessage();
                PAccessNetworkInfoHeader::SetHeader(GetSlotId(), pStateTracker->GetIPAddress(),
                        pStateTracker->GetSIPProfile(), piSIPMsg);
            }
        }

        // SIP_MESSAGE_MEDIATOR
        (void)AdjustMessage(piOngoingSCC->GetMessage(), IMessageMediator::MESSAGE_RESUBMIT);

        ISipTransportHelper* piTransHelper = SipFactory::GetTransportHelper(GetSlotId());

        if (piTransHelper != IMS_NULL)
        {
            piTransHelper->ApplyIpSecForServerSockets();
        }

        if (piOngoingSCC->Send() != IMS_SUCCESS)
        {
            piOngoingSCC->Close();
            piOngoingSCC = IMS_NULL;

            if (nPrevState == STATE_INIT)
            {
                SetState(STATE_CREATED);
            }

            UpdateBindingState(BINDING_RESULT_NOK);

            CallListener(nPrevState, nPrevSubState, REASON_INTERNAL_ERROR);
            return IMS_FALSE;
        }

        // Update the last request/response message
        SetPreviousRequest(piOngoingSCC->GetMessage());
        SetPreviousResponse(IMS_NULL);

        // Remove the previous credential information for re-authentication
        piOngoingSCC->RemoveAllCredentials();
    }
    // SUB_STATE_REFRESHING
    else
    {
        ISipClientConnection* piSCC = pRefreshHelper->GetConnection();

        if (piSCC == IMS_NULL)
        {
            UpdateBindingState(BINDING_RESULT_NOK);

            CallListener(nPrevState, nPrevSubState, REASON_INTERNAL_ERROR);
            return IMS_FALSE;
        }

        if (piSCC->InitResubmissionRequest() != IMS_SUCCESS)
        {
            pRefreshHelper->AbortConnection();

            UpdateBindingState(BINDING_RESULT_NOK);

            CallListener(nPrevState, nPrevSubState, REASON_INTERNAL_ERROR);
            return IMS_FALSE;
        }

        // RFC5626_FLOW_CONTROL
        // MULTI_REG_TRANSPORT
        IMS_SINT32 nTransportExt = pRegParam->GetTransportExtForRegOnly();

        nTransportExt |= pStateTracker->GetTransportExt();

        piSCC->SetTransportTuple(pStateTracker->GetIPAddress(), GetPortUS(), GetPortUC(),
                pStateTracker->GetPortFlowControl(), nTransportExt);

        if (pRegParam->FormRouteHeaders(piSCC, pStateTracker) != IMS_SUCCESS)
        {
            pRefreshHelper->AbortConnection();

            UpdateBindingState(BINDING_RESULT_NOK);

            CallListener(nPrevState, nPrevSubState, REASON_INTERNAL_ERROR);
            return IMS_FALSE;
        }

        if (pRegParam->FormSecurityHeaders(piSCC) != IMS_SUCCESS)
        {
            pRefreshHelper->AbortConnection();

            UpdateBindingState(BINDING_RESULT_NOK);

            CallListener(nPrevState, nPrevSubState, REASON_INTERNAL_ERROR);
            return IMS_FALSE;
        }

        if (piSCC->SetCredentials(objCredential) != IMS_SUCCESS)
        {
            pRefreshHelper->AbortConnection();

            UpdateBindingState(BINDING_RESULT_NOK);

            CallListener(nPrevState, nPrevSubState, REASON_INTERNAL_ERROR);
            return IMS_FALSE;
        }

        // Sets a new P-Access-Network-Info header on re-submitted message (2nd-REGISTER)
        ISipMessage* piSIPMsg = piSCC->GetMessage();
        PAccessNetworkInfoHeader::SetHeader(GetSlotId(), pStateTracker->GetIPAddress(),
                pStateTracker->GetSIPProfile(), piSIPMsg);

        // SIP_MESSAGE_MEDIATOR
        (void)AdjustMessage(piSCC->GetMessage(), IMessageMediator::MESSAGE_RESUBMIT);

        if (piSCC->Send() != IMS_SUCCESS)
        {
            pRefreshHelper->AbortConnection();

            UpdateBindingState(BINDING_RESULT_NOK);

            CallListener(nPrevState, nPrevSubState, REASON_INTERNAL_ERROR);
            return IMS_FALSE;
        }

        // Update the last request/response message
        SetPreviousRequest(piSCC->GetMessage());
        SetPreviousResponse(IMS_NULL);

        // Remove the previous credential information for re-authentication
        piSCC->RemoveAllCredentials();
    }

    IMS_TRACE_D("Terminate to respond to AKA authentication challenge ...", 0, 0, 0);

    return IMS_TRUE;
}

/*

Remarks

*/
PRIVATE
void Registration::RestoreSecurityHeaders()
{
    pRegParam->RemoveSecurityServers();

    if (GetState() == STATE_INIT)
    {
        pRegParam->RemovePreferredSecurityHeaders();
    }
    else
    {
        pRegParam->RestoreSecurityHeaders();
    }
}

// IMS_AUTH_NONCE_REUSE {
/*

Remarks

*/
PRIVATE
void Registration::SetAuthenticationChallenge(IN ISipGenericChallenge* piChallenge)
{
    //---------------------------------------------------------------------------------------------

    if (piChallenge == IMS_NULL)
    {
        if (piGenericChallenge != IMS_NULL)
        {
            piGenericChallenge->Destroy();
            piGenericChallenge = IMS_NULL;
        }

        return;
    }

    if (piGenericChallenge != IMS_NULL)
    {
        IMS_TRACE_D("Authentication challenge (%s) is already present",
                piGenericChallenge->GetNonce().GetStr(), 0, 0);
        return;
    }

    piGenericChallenge = piChallenge->Clone();
}
// }

/*

Remarks

*/
PRIVATE
IMS_RESULT Registration::SetContactNExpiresHeader(
        IN_OUT ISipMessage* piSIPMsg, IN IMS_SINT32 nExpires /* = (-1) */)
{
    //---------------------------------------------------------------------------------------------

    // de-REG
    if (nExpires == 0)
    {
        // Wildcard Contact header : remove all the bindings related to this IMPU.
        if (objContacts.IsEmpty())
        {
            if (piSIPMsg->SetHeader(ISipHeader::CONTACT_WILDCARD, "*") != IMS_SUCCESS)
            {
                IMS_TRACE_E(0, "Setting Contact header (using wildcard [*]) failed", 0, 0, 0);
                return IMS_FAILURE;
            }

            // If the Contact header uses a wildcard option,
            // the Expires header field needs to be set as 0.
            if (piSIPMsg->SetHeader(ISipHeader::EXPIRES_SEC, "0") != IMS_SUCCESS)
            {
                IMS_TRACE_E(0, "Setting Expires header failed", 0, 0, 0);
                return IMS_FAILURE;
            }
        }
        else
        {
            AString strContact;

            for (IMS_UINT32 i = 0; i < objContacts.GetSize(); ++i)
            {
                RegContact* pContact = objContacts.GetAt(i);

                strContact = pContact->ToString();

                // Adds a 'expires' parameter
                strContact.Append(TextParser::CHAR_SEMICOLON);
                strContact.Append(Sip::STR_EXPIRES);
                strContact.Append(TextParser::CHAR_EQUAL);
                strContact.Append('0');

                if (piSIPMsg->AddHeader(ISipHeader::CONTACT_NORMAL, strContact) != IMS_SUCCESS)
                {
                    IMS_TRACE_E(0, "Adding Contact header failed", 0, 0, 0);
                    return IMS_FAILURE;
                }
            }

            // If the Contact header uses a wildcard option,
            // the Expires header field needs to be set as 0.
            if (SetExpiresHeader(piSIPMsg, 0) != IMS_SUCCESS)
            {
                return IMS_FAILURE;
            }
        }
    }
    else
    {
        for (IMS_UINT32 i = 0; i < objContacts.GetSize(); ++i)
        {
            RegContact* pContact = objContacts.GetAt(i);

            if (piSIPMsg->AddHeader(ISipHeader::CONTACT_NORMAL, pContact->ToStringWithExpires()) !=
                    IMS_SUCCESS)
            {
                IMS_TRACE_E(0, "Adding Contact header failed", 0, 0, 0);
                return IMS_FAILURE;
            }
        }

        if (SetExpiresHeader(piSIPMsg, nExpires) != IMS_SUCCESS)
        {
            return IMS_FAILURE;
        }
    }

    return IMS_SUCCESS;
}

/*

Remarks

*/
PRIVATE
IMS_RESULT Registration::SetExpiresHeader(
        IN_OUT ISipMessage* piSIPMsg, IN IMS_SINT32 nExpires /* = (-1) */)
{
    //---------------------------------------------------------------------------------------------

    if (!SipConfigProxy::IsExpiresHeaderInRegRequired(GetSlotId(), pStateTracker->GetSIPProfile()))
    {
        // "expires" header parameter will be used to represent the registration expiration
        return IMS_SUCCESS;
    }

    if (nExpires != (-1))
    {
        AString strExpires;
        strExpires.Sprintf("%u", nExpires);
        if (piSIPMsg->SetHeader(ISipHeader::EXPIRES_SEC, strExpires) != IMS_SUCCESS)
        {
            IMS_TRACE_E(0, "Setting Expires header failed", 0, 0, 0);
            return IMS_FAILURE;
        }
    }
    else
    {
        RegContact* pPreferredContact = objContacts.IsEmpty() ? IMS_NULL : objContacts.GetAt(0);

        if ((pPreferredContact != IMS_NULL) &&
                (pPreferredContact->GetInitialExpires() !=
                        static_cast<IMS_UINT32>(RegContact::EXPIRES_NOT_SPECIFIED)))
        {
            AString strExpires;
            strExpires.Sprintf("%u", pPreferredContact->GetInitialExpires());
            if (piSIPMsg->SetHeader(ISipHeader::EXPIRES_SEC, strExpires) != IMS_SUCCESS)
            {
                IMS_TRACE_E(0, "Setting Expires header failed", 0, 0, 0);
                return IMS_FAILURE;
            }
        }
    }

    return IMS_SUCCESS;
}

/*

Remarks

*/
PRIVATE
IMS_RESULT Registration::SetHeaders(IN ISipClientConnection* piSCC)
{
    ISipMessage* piSIPMsg = piSCC->GetMessage();
    AString strCSeqHdr;

    //---------------------------------------------------------------------------------------------

    strCSeqHdr.Sprintf(
            "%lu %s", pRegFlow->IncreaseNGetCSeqValue(), SipMethod::ToName(SipMethod::REGISTER));

    if (piSIPMsg->SetHeader(ISipHeader::CSEQ, strCSeqHdr) != IMS_SUCCESS)
    {
        IMS_TRACE_E(0, "Setting CSeq header failed", 0, 0, 0);
        return IMS_FAILURE;
    }

    // Set Call-ID header
    if (piSIPMsg->SetHeader(ISipHeader::CALL_ID, pRegFlow->GetCallId()) != IMS_SUCCESS)
    {
        IMS_TRACE_E(0, "Setting Call-ID header failed", 0, 0, 0);
        return IMS_FAILURE;
    }

    // HEADER_REQ_SESSION-ID
    if (pRegFlow->GetSessionId().GetLength() > 0)
    {
        // Set Session-ID header
        if (piSIPMsg->SetHeader(ISipHeader::UNKNOWN, pRegFlow->GetSessionId(),
                    SipHeaderName::SESSION_ID) != IMS_SUCCESS)
        {
            IMS_TRACE_E(0, "Setting Session-ID failed", 0, 0, 0);
            return IMS_FAILURE;
        }
    }

    if (pRegParam->FormHeaders(piSCC, pStateTracker) != IMS_SUCCESS)
    {
        IMS_TRACE_E(0, "Forming REGISTER request failed", 0, 0, 0);
        return IMS_FAILURE;
    }

    if (pRegParam->FormSecurityHeaders(piSCC) != IMS_SUCCESS)
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
            PAccessNetworkInfoHeader::SetHeader(GetSlotId(), piContact->GetIPAddress(),
                    pStateTracker->GetSIPProfile(), piSIPMsg);
        }
        else
        {
            // Configuration for an initial REGISTER / SA is present on INIT
            if (SipConfigProxy::IsPanInfoInInitialRegRequired(
                        GetSlotId(), pStateTracker->GetSIPProfile()) ||
                    pRegParam->IsSecurityAssociationPresent())
            {
                PAccessNetworkInfoHeader::SetHeader(GetSlotId(), piContact->GetIPAddress(),
                        pStateTracker->GetSIPProfile(), piSIPMsg);
            }
        }
    }

    return IMS_SUCCESS;
}

// IMS_AUTH_NONCE_REUSE {
/*

Remarks

*/
PRIVATE
void Registration::SetNextAuthenticationInfo(IN_OUT ISipClientConnection*& piSCC)
{
    //---------------------------------------------------------------------------------------------

    if (piGenericChallenge != IMS_NULL)
    {
        piGenericChallenge->IncreaseNonceCount();

        piSCC->SetCredentials(objActiveCredential);
        piSCC->SetAuthenticationChallenge(piGenericChallenge);
    }
}

/*

Remarks
 SIP_DIGEST_AUTH_NONCE_REUSE
*/
PRIVATE
void Registration::SetNextNonce(IN ISipMessage* piSIPMsg)
{
    //---------------------------------------------------------------------------------------------

    if (piGenericChallenge == IMS_NULL)
    {
        return;
    }

    const SubscriberConfig* pSubsConfig = GetSubsConfig();

    if (IsAkaSupported(pSubsConfig))
    {
        // If IMS-AKA is required, the "nextnonce" field will be ignored.
        return;
    }

    AString strAuthenticationInfo =
            piSIPMsg->GetHeader(ISipHeader::UNKNOWN, 0, SipHeaderName::AUTHENTICATION_INFO);

    if (strAuthenticationInfo.GetLength() == 0)
    {
        return;
    }

    const IMS_CHAR NEXT_NONCE[] = "nextnonce";
    IMSList<AString> objTokens = strAuthenticationInfo.Split(TextParser::CHAR_COMMA);
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
            piGenericChallenge->SetNonce(strValue.Mid(1, strValue.GetLength() - 2));
        }
        else
        {
            piGenericChallenge->SetNonce(strValue);
        }
    }
}
// }

/*

Remarks

*/
PRIVATE
void Registration::SetOngoingConnection(IN ISipClientConnection* piSCC)
{
    //---------------------------------------------------------------------------------------------

    if (piOngoingSCC != IMS_NULL)
    {
        piOngoingSCC->Close();
        piOngoingSCC = IMS_NULL;
    }

    if (piSCC != IMS_NULL)
    {
        piOngoingSCC = piSCC;
    }
}

/*

Remarks

*/
PRIVATE
void Registration::SetPreviousRequest(IN ISipMessage* piSIPMsg)
{
    //---------------------------------------------------------------------------------------------

    if (piPreviousRequest != IMS_NULL)
    {
        piPreviousRequest->Destroy();
        piPreviousRequest = IMS_NULL;
    }

    if (piSIPMsg != IMS_NULL)
    {
        piPreviousRequest = piSIPMsg->Clone();
    }
}

/*

Remarks

*/
PRIVATE
void Registration::SetPreviousResponse(IN ISipMessage* piSIPMsg)
{
    //---------------------------------------------------------------------------------------------

    if (piPreviousResponse != IMS_NULL)
    {
        piPreviousResponse->Destroy();
        piPreviousResponse = IMS_NULL;
    }

    if (piSIPMsg != IMS_NULL)
    {
        piPreviousResponse = piSIPMsg->Clone();
    }
}

/*

Remarks

*/
PRIVATE
void Registration::SetState(IN IMS_SINT32 nState)
{
    //---------------------------------------------------------------------------------------------

    IMS_TRACE_I("Registration (%s) :: %s to %s",
            SipDebug::GetUri1(pStateTracker->GetAOR().GetUri()).GetStr(),
            StateToString(this->nState), StateToString(nState));

    this->nState = nState;
}

/*

Remarks

*/
PRIVATE
void Registration::SetSubState(IN IMS_SINT32 nSubState)
{
    //---------------------------------------------------------------------------------------------

    IMS_TRACE_I("Registration (Sub-State) :: %s to %s", SubStateToString(this->nSubState),
            SubStateToString(nSubState), 0);

    this->nSubState = nSubState;

    if (this->nSubState == SUB_STATE_IDLE)
    {
        // Release the registration flow
        IMS_UINT32 nSubscriber = RegFlow::NO_SUBSCRIBER;

        if (pRegFlow->IsReserved(&nSubscriber))
        {
            if (nSubscriber == GetName().GetHashCode())
            {
                pRegFlow->Release();
            }
        }
    }
}

/*

Remarks

*/
PRIVATE
void Registration::StorePersistentHeaders(IN CONST ISipMessage* piSIPMsg)
{
    //---------------------------------------------------------------------------------------------

    // IMS_IPSEC_UDP_ENC
    if (pRegParam->IsSecurityAssociationRequiredViaUDPEnc())
    {
        ChoosePreferredContact();
    }

    // Stores the public user identities
    AStringArray objTmpArray = piSIPMsg->GetHeaders(ISipHeader::P_ASSOCIATED_URI);

    // If P-Associated-URI does not contains in 200 OK,
    // the explicit registration AOR will be added.
    if (objTmpArray.IsEmpty())
    {
        objTmpArray.AddElement(pStateTracker->GetAOR().ToString());

        IMS_TRACE_D("P-Associated-URI is not present; AOR (%s)",
                SipDebug::GetUri1(objTmpArray.GetElementAt(0)).GetStr(), 0, 0);
    }

    if (piUserIdNotifier != IMS_NULL)
    {
        AStringArray objReorderedUserIds;

        if (piUserIdNotifier->RegUserIdentity_ReorderUserIdentities(
                    objTmpArray, objReorderedUserIds))
        {
            pStateTracker->SetAssociatedURIs(objReorderedUserIds);
        }
        else
        {
            pStateTracker->SetAssociatedURIs(objTmpArray);
        }
    }
    else
    {
        pStateTracker->SetAssociatedURIs(objTmpArray);
    }

    // Stores the Service-Route headers for the next outgoing request
    objTmpArray = piSIPMsg->GetHeaders(ISipHeader::SERVICE_ROUTE);

    // Update the service-routes if preloaded route set is present
    const AStringArray& objPreloadedRoutes = pRegParam->GetPreloadedRoutes();

    if (!objPreloadedRoutes.IsEmpty())
    {
        for (IMS_SINT32 i = objPreloadedRoutes.GetCount() - 1; i >= 0; --i)
        {
            objTmpArray.InsertElementAt(objPreloadedRoutes.GetElementAt(i), 0);
        }
    }

    pStateTracker->SetServiceRoutes(objTmpArray);

    // Path header fields
    objTmpArray = piSIPMsg->GetHeaders(ISipHeader::PATH);
    pStateTracker->SetPathHeaders(objTmpArray);

    // MULTI_REG_TRANSPORT
    pStateTracker->SetTransportExt(pRegParam->GetTransportExt());

    // RFC5626_FLOW_CONTROL
    // If IPSEC is supported and the flow control needs to be supported,
    // then the flow control port SHALL be a port_uc.
    if (IsFlowControlRequired())
    {
        pStateTracker->SetPortFlowControl(pRegParam->GetPortFlowControl());
    }
    else
    {
        pStateTracker->SetPortFlowControl(Sip::PORT_UNSPECIFIED);
    }

    // Updates the protected client /server port (uc / us)
    pStateTracker->SetPortUC(GetPortUC());
    pStateTracker->SetPortUS(GetPortUS());

    // Security-Client / Security-Verify headers
    pStateTracker->SetSecurityClients(pRegParam->GetSecurityClients());
    pStateTracker->SetSecurityVerifys(pRegParam->GetSecurityVerifys());
}

/*

Remarks

*/
PRIVATE
IMS_RESULT Registration::UpdateBindings(IN CONST ISipMessage* piSIPMsg)
{
    IMS_SINT32 nHeaderCount = piSIPMsg->GetHeaderCount(ISipHeader::CONTACT_ANY);
    AString strHeader;
    ISipHeader* piHeader;

    IMS_SINT32 nExpiresValue = (-1);
    IMSList<ISipHeader*> objHeaders;

    //---------------------------------------------------------------------------------------------

    // Extract the expiration value from Expires header field
    if (piSIPMsg->IsHeaderPresent(ISipHeader::EXPIRES_ANY))
    {
        strHeader = piSIPMsg->GetHeader(ISipHeader::EXPIRES_ANY);
        piHeader = SipParsingHelper::CreateHeader(ISipHeader::EXPIRES_ANY, strHeader);

        if (piHeader != IMS_NULL)
        {
            // 4 Make up for test equipment's fault (e.g. Anite)
            if (piHeader->GetValue().Contains(TextParser::CHAR_DQUOT))
            {
                IMS_BOOL bOK = IMS_FALSE;
                AString strValue = TextParser::TrimDQUOT(piHeader->GetValue());

                nExpiresValue = strValue.ToInt32(&bOK);

                if (!bOK)
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
        strHeader = piSIPMsg->GetHeader(ISipHeader::CONTACT_ANY, i);
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
            for (IMS_UINT32 j = 0; j < objContacts.GetSize(); ++j)
            {
                RegContact* pContact = objContacts.GetAt(j);

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
    for (IMS_UINT32 j = 0; j < objContacts.GetSize(); ++j)
    {
        RegContact* pContact = objContacts.GetAt(j);
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

    ////////////////////////////
    /// Remove the contact in the state, "STATE_TERMINATED"
#if 0
    for (IMS_UINT32 k = 0; k < objContacts.GetSize(); ++k)
    {
        RegContact *pContact = objContacts.GetAt(k);

        // State & Event : check this two field before removing Contact
        if (!pContact->IsActiveBinding() && pContact->IsEmpty())
        {
            objContacts.RemoveAt(k);
            --k;
        }
    }
#endif

    return nBindingUpdateResult;
}

/*

Remarks

*/
PRIVATE
void Registration::UpdateBindingState(IN IMS_SINT32 nState)
{
    //---------------------------------------------------------------------------------------------

    for (IMS_UINT32 i = 0; i < objObservers.GetSize(); ++i)
    {
        RegObserver* pObserver = objObservers.GetAt(i);

        if (pObserver == IMS_NULL)
            continue;

        pObserver->Update(nState);
    }
}

/*

Remarks
 IMS_IPSEC_UDP_ENC
*/
PRIVATE
void Registration::UpdateHostInfoInAllContacts()
{
    //---------------------------------------------------------------------------------------------

    if (pRegParam->IsSecurityAssociationRequiredViaUDPEnc())
    {
        for (IMS_UINT32 i = 0; i < objContacts.GetSize(); ++i)
        {
            RegContact* pContact = objContacts.GetAt(i);

            pContact->SetHostInfo(pStateTracker->GetPublicIPAddress());
        }
    }
}

/*

Remarks

*/
PRIVATE
void Registration::UpdateCSeqNumber(IN CONST ISipMessage* piSIPMsg)
{
    AString strCSeq = piSIPMsg->GetHeader(ISipHeader::CSEQ);
    IMS_SINT32 nPosOfSP = strCSeq.GetIndexOf(TextParser::CHAR_SP);

    //---------------------------------------------------------------------------------------------

    if (nPosOfSP == AString::NPOS)
    {
        // TODO:: select the proper value to increase sequence number when an error occurs
        // If CSeq header is invalid, the number will be increased as much as 5
        (void)pRegFlow->IncreaseNGetCSeqValue(5);
        return;
    }

    strCSeq = strCSeq.GetSubStr(0, nPosOfSP);

    IMS_BOOL bOK = IMS_FALSE;
    IMS_SINT32 nSeqNum = strCSeq.ToInt32(&bOK);

    if (!bOK)
    {
        // If the number is not converted, the number will be increased as much as 5
        (void)pRegFlow->IncreaseNGetCSeqValue(5);
        return;
    }

    IMS_TRACE_I("Registration - Sequence Number Changed (%d >> %d)",
            pRegFlow->IncreaseNGetCSeqValue(0), nSeqNum, 0);

    // Store the current sequence number (last one) from the last successful response.
    pRegFlow->SetCSeqValue(nSeqNum);
}

/*

Remarks

*/
PRIVATE
void Registration::UpdateProtectedServerPortForContact(IN_OUT ISipMessage* piSIPMsg)
{
    IRegContact* piRegContact = GetPreferredContact();

    if (piRegContact == IMS_NULL)
    {
        IMS_TRACE_D("No preferred contact", 0, 0, 0);
        return;
    }

    IMS_SINT32 nHCount = piSIPMsg->GetHeaderCount(ISipHeader::CONTACT_NORMAL);

    if (nHCount == 1)
    {
        AString strContact = piSIPMsg->GetHeader(ISipHeader::CONTACT_NORMAL);
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

                    piSIPMsg->RemoveHeader(ISipHeader::CONTACT_NORMAL);
                    piSIPMsg->SetHeader(
                            ISipHeader::CONTACT_NORMAL, piHeader->ToStringWithoutName());
                }
            }
            else
            {
                // Exceptional case
                IMS_TRACE_D("ReformContactHeader :: SipAddress is null", 0, 0, 0);
                ReformContactHeader(piSIPMsg);
            }

            piHeader->Destroy();
        }
        else
        {
            // Exceptional case
            IMS_TRACE_D("ReformContactHeader :: ISipHeader is null", 0, 0, 0);
            ReformContactHeader(piSIPMsg);
        }
    }
}

/*

Remarks

*/
PRIVATE
void Registration::UpdateRefreshTimer()
{
    IMS_UINT32 nExpirationValue = 0;
    IMS_UINT32 nAnchorContact = 0xFFFFFFFF;

    //---------------------------------------------------------------------------------------------

    // Select the minimum expiration value among the registered contacts
    // (State of contact is 'active')
    for (IMS_UINT32 i = 0; i < objContacts.GetSize(); ++i)
    {
        const RegContact* pContact = objContacts.GetAt(i);

        if (!pContact->IsActiveBinding())
            continue;

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
        const RegContact* pContact = objContacts.GetAt(nAnchorContact);

        pRefreshHelper->SetContactAddress(pContact->GetContactAddress());
    }

    pRefreshHelper->UpdateRefreshTimer(static_cast<IMS_SINT32>(nExpirationValue));
}

/*

Remarks

*/
PRIVATE GLOBAL ISipClientConnection* Registration::CreateConnection(IN Registration* pReg)
{
    AString strAOR = pReg->pStateTracker->GetAOR().ToString();
    const RegContact* pContact = pReg->pStateTracker->GetPreferredContact();

    //---------------------------------------------------------------------------------------------

    if (pContact == IMS_NULL)
    {
        IMS_TRACE_E(0, "No contacts for the registration (%s)", SipDebug::GetUri1(strAOR).GetStr(),
                0, 0);
        return IMS_NULL;
    }

    ISipClientConnection* piSCC = DYNAMIC_CAST(ISipClientConnection*, Connector::Open(strAOR));

    IMS_TRACE_D("CreateConnection - To (%s), Method (REGISTER)", SipDebug::GetUri1(strAOR).GetStr(),
            0, 0);

    if (piSCC == IMS_NULL)
    {
        IMS_TRACE_E(0, "Creating a SIP client connection failed", 0, 0, 0);
        return IMS_NULL;
    }

    // MULTI_REG_SIP_PROFILE
    piSCC->SetSipProfile(pReg->pStateTracker->GetSIPProfile());

    // Sets the transport tuples
    // RFC5626_FLOW_CONTROL
    // MULTI_REG_TRANSPORT
    IMS_SINT32 nTransportExt = pReg->pRegParam->GetTransportExtForRegOnly();

    if (pReg->GetState() == STATE_CREATED)
    {
        nTransportExt |= pReg->pRegParam->GetTransportExt();

        piSCC->SetTransportTuple(pContact->GetIPAddress(), pReg->GetPortUC(), pReg->GetPortUC(),
                pReg->pRegParam->GetPortFlowControl(), nTransportExt);
    }
    else
    {
        nTransportExt |= pReg->pStateTracker->GetTransportExt();

        piSCC->SetTransportTuple(pContact->GetIPAddress(), pReg->GetPortUS(),
                pReg->pStateTracker->GetPortUC(), pReg->pStateTracker->GetPortFlowControl(),
                nTransportExt);
    }

    // Sets the SIP transaction timers
    const SipTimerValues* pSIPTVs = pReg->pRegParam->GetSIPTimerValues();

    if (pSIPTVs != IMS_NULL)
    {
        piSCC->SetTransactionTimerValues(*pSIPTVs);
    }
    else
    {
        piSCC->SetTransactionTimerValues(SipTimerValuesHelper::GetValues(
                pReg->GetSlotId(), pReg->pStateTracker->GetSIPProfile()));
    }

    if (piSCC->InitRequest(SipMethod::ToName(SipMethod::REGISTER), IMS_NULL) != IMS_SUCCESS)
    {
        piSCC->Close();
        return IMS_NULL;
    }

    // Set From header
    if (piSCC->GetMessage()->SetHeader(ISipHeader::FROM, strAOR) != IMS_SUCCESS)
    {
        piSCC->Close();
        return IMS_NULL;
    }

    return piSCC;
}

/*

Remarks

*/
PRIVATE GLOBAL void Registration::DestroyAllHeaders(IN_OUT IMSList<ISipHeader*>& objHeaders)
{
    //---------------------------------------------------------------------------------------------

    if (objHeaders.IsEmpty())
        return;

    for (IMS_UINT32 i = 0; i < objHeaders.GetSize(); ++i)
    {
        ISipHeader* piHeader = objHeaders.GetAt(i);

        if (piHeader != IMS_NULL)
            piHeader->Destroy();
    }

    objHeaders.Clear();
}

/*

Remarks

*/
PRIVATE GLOBAL const IMS_CHAR* Registration::StateToString(IN IMS_SINT32 nState)
{
    //---------------------------------------------------------------------------------------------

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

/*

Remarks

*/
PRIVATE GLOBAL const IMS_CHAR* Registration::SubStateToString(IN IMS_SINT32 nSubState)
{
    //---------------------------------------------------------------------------------------------

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
