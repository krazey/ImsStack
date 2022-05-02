/*
    Author
    <table>
    date      author                    description
    --------  --------------            ----------
    20130227  hwangoo.park@             Created
    </table>

    Description

*/

#include "ServiceMemory.h"
#include "ServiceTrace.h"
#include "Sip.h"
#include "SipStatusCode.h"
#include "ISipConnectionNotifier.h"
#include "SipDebug.h"
#include "SipConfigProxy.h"
#include "base/IMS.h"
#include "SIPURNHelper.h"
#include "IRegistrationListener.h"
#include "IRegUserIdentityNotifier.h"
#include "RegObserver.h"
#include "RegFlow.h"
#include "RegParameter.h"
#include "RegBindingProxy.h"
#include "FakeRegistration.h"

__IMS_TRACE_TAG_REG__;



PUBLIC
FakeRegistration::FakeRegistration()
    : EngineActivity()
    , nState(STATE_CREATED)
    , nSubState(SUB_STATE_IDLE)
    , pRegFlow(IMS_NULL)
    , pRegParam(IMS_NULL)
    , objContacts(IMSList<RegContact*>())
    , pStateTracker(IMS_NULL)
    , piListener(IMS_NULL)
    , piUserIdNotifier(IMS_NULL)
    , objObservers(IMSList<RegObserver*>())
    , bIsWithinTrustDomain(IMS_TRUE)
    , nRefCountForSCNEL(0)
{
    pStateTracker = new RegStateTracker();
}

PUBLIC VIRTUAL
FakeRegistration::~FakeRegistration()
{
    if (pRegParam != IMS_NULL)
    {
        delete pRegParam;
        pRegParam = IMS_NULL;
    }

    RegBindingProxy::DestroyBinding(GetSlotId(), this);

    IMS_TRACE_D("Destructor :: Registration - %s",
            (pRegFlow != IMS_NULL) ? SIPDebug::GetCharA1(
                    pRegFlow->GetCallId().GetStr(), 8, '@') : "__UNKNOWN__", 0, 0);

    if (pRegFlow != IMS_NULL)
    {
        delete pRegFlow;
        pRegFlow = IMS_NULL;
    }
}

/*

Remarks

*/
PUBLIC VIRTUAL
IMS_BOOL FakeRegistration::Equals(IN CONST IRegistration *piReg) const
{
    const FakeRegistration *pReg = DYNAMIC_CAST(const FakeRegistration*, piReg);

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
PUBLIC VIRTUAL
IMS_BOOL FakeRegistration::IsBehindNAT() const
{
    //---------------------------------------------------------------------------------------------

    return IMS_FALSE;
}

/*

Remarks

*/
PUBLIC VIRTUAL
IMS_BOOL FakeRegistration::IsWithinTrustDomain() const
{
    //---------------------------------------------------------------------------------------------

    return bIsWithinTrustDomain;
}

/*

Remarks

*/
PUBLIC VIRTUAL
const RegInfo* FakeRegistration::GetRegInfo() const
{
    //---------------------------------------------------------------------------------------------

    return IMS_NULL;
}

/*

Remarks

*/
PUBLIC VIRTUAL
const RegStateTracker* FakeRegistration::GetStateTracker() const
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
IMS_BOOL FakeRegistration::Create(IN IMS_UINT32 nFlowId, IN CONST SIPAddress &objAOR,
        IN CONST AString &strSubsId /* = AString::ConstNull() */,
        IN SIPProfile *pSIPProfile/* = IMS_NULL*/)
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
void FakeRegistration::Destroy()
{
    //---------------------------------------------------------------------------------------------

    IMS_TRACE_D("Registration :: Destroy() - SCNEL=%d", nRefCountForSCNEL, 0, 0);

    DestroyAllContacts();
    UpdateBindingState(BINDING_DESTROY);

    PostMessage(AMSG_DESTROY, 0, 0);
}

/*

Remarks

*/
PUBLIC
const IMSList<RegContact*>& FakeRegistration::GetAllContactsEx() const
{
    //---------------------------------------------------------------------------------------------

    return objContacts;
}

/*

Remarks

*/
PUBLIC
IMS_BOOL FakeRegistration::HasActiveBindings() const
{
    //---------------------------------------------------------------------------------------------

    for (IMS_UINT32 i = 0; i < objContacts.GetSize(); ++i)
    {
        RegContact *pContact = objContacts.GetAt(i);

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
IMS_BOOL FakeRegistration::IsAllBindingsRemoved() const
{
    //---------------------------------------------------------------------------------------------

    for (IMS_UINT32 i = 0; i < objContacts.GetSize(); ++i)
    {
        RegContact *pContact = objContacts.GetAt(i);

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
PRIVATE VIRTUAL
IMS_BOOL FakeRegistration::DispatchMessage(IN IMSMSG &objMSG)
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
        break;

    case AMSG_REGISTRATION_RESPONSE_RECEIVED:
        NotifyResponse(LONG_TO_SINT(objMSG.nWparam));
        break;

    default:
        break;
    }

    return EngineActivity::DispatchMessage(objMSG);
}

/*

Remarks

*/
PRIVATE VIRTUAL
ISIPMessage* FakeRegistration::GetNextRequest()
{
    //---------------------------------------------------------------------------------------------

    return IMS_NULL;
}

/*

Remarks

*/
PRIVATE VIRTUAL
ISIPMessage* FakeRegistration::GetPreviousRequest() const
{
    //---------------------------------------------------------------------------------------------

    return IMS_NULL;
}

/*

Remarks

*/
PRIVATE VIRTUAL
ISIPMessage* FakeRegistration::GetPreviousResponse() const
{
    //---------------------------------------------------------------------------------------------

    return IMS_NULL;
}

/*

Remarks
 SIP_MESSAGE_MEDIATOR
*/
PRIVATE VIRTUAL
void FakeRegistration::SetSipMessageMediator(IN IMessageMediator * /* piMediator */)
{
    // no-op
}

/*

Remarks

*/
PRIVATE VIRTUAL
IMS_BOOL FakeRegistration::CreateBinding(IN CONST AString &strAppId,
        IN CONST AString &strServiceId)
{
    //---------------------------------------------------------------------------------------------

    return RegBindingProxy::CreateBinding(GetSlotId(), strAppId, strServiceId, this);
}

/*

Remarks

*/
PRIVATE VIRTUAL
void FakeRegistration::DestroyBinding(IN CONST AString &strAppId, IN CONST AString &strServiceId)
{
    //---------------------------------------------------------------------------------------------

    RegBindingProxy::DestroyBinding(GetSlotId(), strAppId, strServiceId);
}

/*

Remarks

*/
PRIVATE VIRTUAL
IRegContact* FakeRegistration::CreateContact(IN CONST IPAddress &objIPA, IN IMS_SINT32 nPort,
        IN IMS_SINT32 nExpiresPolicy /* = POLICY_EXPIRES_CONFIG */,
        IN IMS_UINT32 nExpiresValue /* = DEFAULT_EXPIRES */)
{
    //---------------------------------------------------------------------------------------------

    // If the contact address is already present in the list,
    // then just adds the service capability to the contact.
    for (IMS_UINT32 i = 0; i < objContacts.GetSize(); ++i)
    {
        RegContact *pTmpContact = objContacts.GetAt(i);

        if (objIPA.Equals(pTmpContact->GetIPAddress())
                && (nPort == pTmpContact->GetPort()))
        {
            IMS_TRACE_D("RegContact (%s, %d) already exists", SIPDebug::GetIP(objIPA), nPort, 0);
            return pTmpContact;
        }
    }

    SIPProfile *pSIPProfile = pStateTracker->GetSIPProfile();

    // If not present, add a new Contact information
    RegContact *pNewContact = new RegContact(GetSlotId(),
            objIPA, nPort, this , (-1) /*pRegFlow->GetRegKey().GetFlowId()*/, pSIPProfile);

    if (pNewContact == IMS_NULL)
    {
        IMS_TRACE_E(0, "Creating a Contact (%s:%d) failed", SIPDebug::GetIP(objIPA), nPort, 0);
        return IMS_NULL;
    }

    // Set user-info field
    pNewContact->SetAOR(pStateTracker->GetAOR());

    // Set "+sip.instance" parameter
    IMS_SINT32 nDeviceId = SIPConfigProxy::GetDeviceId(GetSlotId(), pSIPProfile);

    if (nDeviceId != ISipConfig::DEVICE_ID_NONE)
    {
        AString strDeviceId;
        AString strInstance;

        if (nDeviceId == ISipConfig::DEVICE_ID_PREDEFINED)
        {
            strDeviceId = SIPConfigProxy::GetPredefinedDeviceId(GetSlotId(), pSIPProfile);
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

        static_cast<IRegContact*>(pNewContact)->AddHeaderParameter(
                SIP::STR_SIP_INSTANCE, strInstance);
    }

    // Set the regiration expiration value
    switch (nExpiresPolicy)
    {
    case IRegistration::POLICY_EXPIRES_CONFIG:
        {
            if (SIPConfigProxy::IsRegExpiresConfigured(GetSlotId(), pSIPProfile))
            {
                // Ignore the passed expiration (nExpiresValue)
                pNewContact->SetExpires(SIPConfigProxy::GetRegExpires(GetSlotId(), pSIPProfile));
            }
        }
        break;

    case IRegistration::POLICY_EXPIRES_APP:
        pNewContact->SetExpires(nExpiresValue);
        break;
    }

    IMS_TRACE_D("RegContact (%s, %d) added", SIPDebug::GetIP(objIPA), nPort, 0);

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
PRIVATE VIRTUAL
void FakeRegistration::DestroyAllContacts()
{
    //---------------------------------------------------------------------------------------------

    while (!objContacts.IsEmpty())
    {
        RegContact *pContact = objContacts.GetAt(0);

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
PRIVATE VIRTUAL
void FakeRegistration::DestroyContact(IN IRegContact *piContact)
{
    //---------------------------------------------------------------------------------------------

    for (IMS_UINT32 i = 0; i < objContacts.GetSize(); ++i)
    {
        RegContact *pContact = objContacts.GetAt(i);

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
PRIVATE VIRTUAL
void FakeRegistration::DestroyContact(IN CONST IPAddress &objIPA, IN IMS_SINT32 nPort)
{
    //---------------------------------------------------------------------------------------------

    for (IMS_UINT32 i = 0; i < objContacts.GetSize(); ++i)
    {
        RegContact *pContact = objContacts.GetAt(i);

        if (objIPA.Equals(pContact->GetIPAddress())
                && (nPort == pContact->GetPort()))
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
PRIVATE VIRTUAL
const Credential* FakeRegistration::GetCredential() const
{
    //---------------------------------------------------------------------------------------------

    return IMS_NULL;
}

/*

Remarks

*/
PRIVATE VIRTUAL
const SIPAddress& FakeRegistration::GetAOR() const
{
    //---------------------------------------------------------------------------------------------

    return pStateTracker->GetAOR();
}

/*

Remarks

*/
PRIVATE VIRTUAL
const AStringArray& FakeRegistration::GetAssociatedURIs() const
{
    //---------------------------------------------------------------------------------------------

    return pStateTracker->GetAssociatedURIs();
}

/*

Remarks

*/
PRIVATE VIRTUAL
const SIPAddress& FakeRegistration::GetAuthorizedAOR() const
{
    //---------------------------------------------------------------------------------------------

    return pStateTracker->GetAuthorizedAOR();
}

/*

Remarks

*/
PRIVATE VIRTUAL
IMSList<IRegContact*> FakeRegistration::GetAllContacts() const
{
    IMSList<IRegContact*> objIContacts;

    //---------------------------------------------------------------------------------------------

    for (IMS_UINT32 i = 0; i < objContacts.GetSize(); ++i)
    {
        RegContact *pContact = objContacts.GetAt(i);

        objIContacts.Append(pContact);
    }

    return objIContacts;
}

/*

Remarks

*/
PRIVATE VIRTUAL
IRegContact* FakeRegistration::GetContact(IN CONST IPAddress &objIPA, IN IMS_SINT32 nPort) const
{
    //---------------------------------------------------------------------------------------------

    for (IMS_UINT32 i = 0; i < objContacts.GetSize(); ++i)
    {
        RegContact *pContact = objContacts.GetAt(i);

        if (objIPA.Equals(pContact->GetIPAddress())
                && (nPort == pContact->GetPort()))
        {
            return pContact;
        }
    }

    return IMS_NULL;
}

/*

Remarks

*/
PRIVATE VIRTUAL
IRegContact* FakeRegistration::GetPreferredContact() const
{
    //---------------------------------------------------------------------------------------------

    return const_cast<RegContact*>(pStateTracker->GetPreferredContact());
}

/*

Remarks

*/
PRIVATE VIRTUAL
IRegParameter* FakeRegistration::GetParameter() const
{
    //---------------------------------------------------------------------------------------------

    return pRegParam;
}

/*

Remarks

*/
PRIVATE VIRTUAL
const IPAddress& FakeRegistration::GetPublicIPAddress() const
{
    //---------------------------------------------------------------------------------------------

    return IPAddress::NONE;
}

/*

Remarks

*/
PRIVATE VIRTUAL
const AStringArray& FakeRegistration::GetServiceRoutes() const
{
    //---------------------------------------------------------------------------------------------

    return pStateTracker->GetServiceRoutes();
}

/*

Remarks
 MULTI_REG_SIP_PROFILE
*/
PRIVATE VIRTUAL
SIPProfile* FakeRegistration::GetSIPProfile() const
{
    //---------------------------------------------------------------------------------------------

    return (!pStateTracker.IsNull()) ? pStateTracker->GetSIPProfile() : IMS_NULL;
}

/*

Remarks

*/
PRIVATE VIRTUAL
IMS_SINT32 FakeRegistration::GetState() const
{
    //---------------------------------------------------------------------------------------------

    return nState;
}

/*

Remarks

*/
PRIVATE VIRTUAL
IMS_BOOL FakeRegistration::IsBindingsUpdated() const
{
    //---------------------------------------------------------------------------------------------

    for (IMS_UINT32 i = 0; i < objContacts.GetSize(); ++i)
    {
        RegContact *pContact = objContacts.GetAt(i);

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
PRIVATE VIRTUAL
IMS_BOOL FakeRegistration::IsBindingsUpdating() const
{
    //---------------------------------------------------------------------------------------------

    return (GetSubState() != SUB_STATE_IDLE);
}

/*

Remarks

*/
PRIVATE VIRTUAL
IMS_BOOL FakeRegistration::IsNetworkInterworkingRequired() const
{
    //---------------------------------------------------------------------------------------------

    // Emergency registration w/o UICC or w/ invalid UICC
    return IMS_FALSE;
}

/*

Remarks

*/
PRIVATE VIRTUAL
IMS_RESULT FakeRegistration::Register(IN IMS_SINT32 nExpires /* = (-1) */)
{
    IMS_SINT32 nCurrentState = GetState();
    IMS_SINT32 nCurrentSubState = GetSubState();

    //---------------------------------------------------------------------------------------------

    if ((nCurrentState == STATE_INIT)
            || (nCurrentState == STATE_TERMINATED))
    {
        IMS_TRACE_E(0, "REGISTER request can't be sent in INIT or TERMINATED state", 0, 0, 0);
        return IMS_FAILURE;
    }

    if (nCurrentSubState != SUB_STATE_IDLE)
    {
        IMS_TRACE_E(0, "REGISTER request can't be sent when another transaction is ongoing",
                0, 0, 0);
        return IMS_FAILURE;
    }

    if ((nCurrentState == STATE_CREATED) && IsAllBindingsRemoved())
    {
        IMS_TRACE_E(0, "There is no contacts or caller capabilities for the registration",
                0, 0, 0);
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
            for (IMS_UINT32 i = 0; i < objContacts.GetSize(); ++i)
            {
                RegContact *pContact = objContacts.GetAt(i);

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

    IMS::SetLastError(IMSError::NO_ERROR);

    //4 REG. OK
    PostMessage(AMSG_REGISTRATION_RESPONSE_RECEIVED, SIPStatusCode::SC_200, 0);

    return IMS_SUCCESS;
}

/*

Remarks

*/
PRIVATE VIRTUAL
IMS_RESULT FakeRegistration::Deregister()
{
    //---------------------------------------------------------------------------------------------

    if (GetState() != STATE_ACTIVE)
    {
        IMS_TRACE_E(0, "de-REGISTER request can't be sent in non-ACTIVE state (%d)",
                GetState(), 0, 0);
        return IMS_FAILURE;
    }

    if (GetSubState() != SUB_STATE_IDLE)
    {
        IMS_TRACE_E(0, "de-REGISTER request can't be sent when another transaction is ongoing",
                0, 0, 0);
        return IMS_FAILURE;
    }

    SetSubState(SUB_STATE_DEREGISTERING);

    // Change the state
    UpdateBindingState(BINDING_DEREGISTERING);

    IMS::SetLastError(IMSError::NO_ERROR);

    //4 de-REG. OK
    PostMessage(AMSG_REGISTRATION_RESPONSE_RECEIVED, SIPStatusCode::SC_200, 0);

    return IMS_SUCCESS;
}

/*

Remarks
 REG_RESTORATION_FOR_ACTIVE_BINDING
*/
PRIVATE VIRTUAL
void FakeRegistration::RemoveActiveBindingsForcingly()
{
    //---------------------------------------------------------------------------------------------

    // no-op
}

/*

Remarks

*/
PRIVATE VIRTUAL
void FakeRegistration::Restore()
{
    //---------------------------------------------------------------------------------------------

    IMS_TRACE_I("Registration::Restore - State (%s)", StateToString(GetState()), 0, 0);

    //4 contact handling/ connection if present

    SetState(STATE_CREATED);
    SetSubState(SUB_STATE_IDLE);
    UpdateBindingState(BINDING_RESTORE);

    // Remove all the bindings if present
    for (IMS_UINT32 i = 0; i < objContacts.GetSize(); ++i)
    {
        RegContact *pContact = objContacts.GetAt(i);

        pContact->Restore();
    }

    pRegFlow->Restore();
    pRegParam->Restore();
}

/*

Remarks
 REG_RESTORATION_FOR_ACTIVE_BINDING
*/
PRIVATE VIRTUAL
IMS_RESULT FakeRegistration::RestoreActiveBindings()
{
    //---------------------------------------------------------------------------------------------

    // no-op

    return IMS_SUCCESS;
}

/*

Remarks
 REG_RESTORATION_FOR_ACTIVE_BINDING
*/
PRIVATE VIRTUAL
void FakeRegistration::SetActiveBindingsRestorationUsage(IN IMS_BOOL /* bEnabled */)
{
    //---------------------------------------------------------------------------------------------

    // no-op
}

/*

Remarks
 MULTI_SUBS
*/
PRIVATE VIRTUAL
void FakeRegistration::SetAOR(IN CONST SIPAddress &objAOR,
        IN CONST AString &strSubsId /* = AString::ConstNull() */)
{
    //---------------------------------------------------------------------------------------------

    pStateTracker->SetAOR(objAOR);
    pStateTracker->SetSubscriberId(strSubsId);

    // Update the profile
    pRegParam->UpdateProfile(objAOR, strSubsId);

    // Updates the user-info field in all Contacts
    const SIPAddress &objTmpAOR = pStateTracker->GetAOR();

    for (IMS_UINT32 i = 0; i < objContacts.GetSize(); ++i)
    {
        RegContact *pContact = objContacts.GetAt(i);

        pContact->SetAOR(objTmpAOR);
    }
}

/*

Remarks

*/
PRIVATE VIRTUAL
void FakeRegistration::SetListener(IN IRegistrationListener *piListener)
{
    //---------------------------------------------------------------------------------------------

    this->piListener = piListener;
}

/*

Remarks

*/
PRIVATE VIRTUAL
void FakeRegistration::SetRefreshPolicy(IN IMS_SINT32 /* nPolicy */,
        IN IMS_SINT32 /* nCriteriaInterval */, IN IMS_SINT32 /* nValueEorLT */,
        IN IMS_SINT32 /* nValueGT */)
{
    //---------------------------------------------------------------------------------------------

    IMS_TRACE_D("SetRefreshPolicy :: not implemented", 0, 0, 0);
}

/*

Remarks
 MULTI_REG_SIP_PROFILE
*/
PRIVATE VIRTUAL
void FakeRegistration::SetSIPProfile(IN SIPProfile *pProfile)
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
PRIVATE VIRTUAL
void FakeRegistration::SetBindingStateListener(IN IRegBindingStateListener * /* piListener */)
{
    //---------------------------------------------------------------------------------------------

    IMS_TRACE_D("SetBindingStateListener :: not implemented", 0, 0, 0);
}

/*

Remarks

*/
PRIVATE VIRTUAL
void FakeRegistration::SetFlagForWithinTrustDomain(IN IMS_BOOL bWithinTrustDomain)
{
    //---------------------------------------------------------------------------------------------

    if (bIsWithinTrustDomain != bWithinTrustDomain)
    {
        IMS_TRACE_I("WithinTrustDomain:: %s > %s",
                _TRACE_B_(bIsWithinTrustDomain), _TRACE_B_(bWithinTrustDomain), 0);

        bIsWithinTrustDomain = bWithinTrustDomain;
    }
}

/*

Remarks

*/
PRIVATE VIRTUAL
void FakeRegistration::SetUserIdentityNotifier(IN IRegUserIdentityNotifier *piUserIdNotifier)
{
    //---------------------------------------------------------------------------------------------

    this->piUserIdNotifier = piUserIdNotifier;
}

/*

Remarks

*/
PRIVATE VIRTUAL
void FakeRegistration::SetUserInfoForContactHeader(IN CONST AString &strUserInfo)
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
PRIVATE VIRTUAL
IRegSubscription* FakeRegistration::CreateSubscription(
        IN SIPAddress * /* pResourceUri = IMS_NULL */)
{
    //---------------------------------------------------------------------------------------------

    return IMS_NULL;
}

/*

Remarks

*/
PRIVATE VIRTUAL
void FakeRegistration::ConnectionNotifierError_NotifyError(IN ISIPConnectionNotifier *piSCN,
        IN IMS_SINT32 nCode, IN CONST AString &strMessage)
{
    //---------------------------------------------------------------------------------------------

    (void) piSCN;

    IMS_TRACE_D("ConnectionNotifierError_NotifyError :: code=%d, message=%s",
            nCode, strMessage.GetStr(), 0);

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

    if (nCode == ISIPConnectionNotifier::TRANSPORT_ERROR_TCP_CLIENT)
    {
        PostMessage(AMSG_REGISTRATION_TERMINATED, REASON_CLIENT_SOCKET_ERROR, 0);
    }
    else if (nCode == ISIPConnectionNotifier::TRANSPORT_ERROR_TCP_SERVER)
    {
        PostMessage(AMSG_REGISTRATION_TERMINATED, REASON_SERVER_SOCKET_ERROR, 0);
    }
    else if (nCode == ISIPConnectionNotifier::TRANSPORT_ERROR_UDP_SERVER)
    {
        PostMessage(AMSG_REGISTRATION_TERMINATED, REASON_SERVER_SOCKET_ERROR, 0);
    }
}

/*

Remarks

*/
PRIVATE VIRTUAL
void FakeRegistration::AddObserver(IN RegObserver *pObserver)
{
    //---------------------------------------------------------------------------------------------

    for (IMS_UINT32 i = 0; i < objObservers.GetSize(); ++i)
    {
        RegObserver *pTmpObserver = objObservers.GetAt(i);

        if (pObserver == pTmpObserver)
            return;
    }

    objObservers.Append(pObserver);
}

/*

Remarks

*/
PRIVATE VIRTUAL
void FakeRegistration::RemoveObserver(IN RegObserver *pObserver)
{
    //---------------------------------------------------------------------------------------------

    for (IMS_UINT32 i = 0; i < objObservers.GetSize(); ++i)
    {
        RegObserver *pTmpObserver = objObservers.GetAt(i);

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
PRIVATE VIRTUAL
IMS_SINT32 FakeRegistration::AddReferenceForSCNEL()
{
    //---------------------------------------------------------------------------------------------

#ifdef __IMS_DEBUG__
    IMS_TRACE_D("Registration :: SCNEL (add) - %d >> %d",
            nRefCountForSCNEL, (nRefCountForSCNEL + 1), 0);
#endif

    ++nRefCountForSCNEL;

    return nRefCountForSCNEL;
}

/*

Remarks

*/
PRIVATE VIRTUAL
IMS_SINT32 FakeRegistration::RemoveReferenceForSCNEL()
{
    //---------------------------------------------------------------------------------------------

    if (nRefCountForSCNEL > 0)
    {
#ifdef __IMS_DEBUG__
        IMS_TRACE_D("Registration :: SCNEL (remove) - %d >> %d",
                nRefCountForSCNEL, (nRefCountForSCNEL - 1), 0);
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
PRIVATE VIRTUAL
void FakeRegistration::NotifyCallerCapabilityChanged()
{
    //---------------------------------------------------------------------------------------------

    // No operations...
}

/*

Remarks

*/
PRIVATE VIRTUAL
void FakeRegistration::RegCapabilityChange_ServiceAdded(IN CONST AString &strAppId,
        IN CONST AString &strServiceId)
{
    AStringArray objHeaders;

    //---------------------------------------------------------------------------------------------

    RegBindingProxy::QueryRegistrationHeaders(GetSlotId(), strAppId, strServiceId, objHeaders);

    // Adds extra headers
    if (!objHeaders.IsEmpty())
    {
        IRegParameter *piRegParameter = pRegParam;

        piRegParameter->AddExtraHeaders(objHeaders);
    }
}

/*

Remarks

*/
PRIVATE VIRTUAL
void FakeRegistration::RegCapabilityChange_ServiceRemoved(IN CONST AString &strAppId,
        IN CONST AString &strServiceId)
{
    AStringArray objHeaders;

    //---------------------------------------------------------------------------------------------

    RegBindingProxy::QueryRegistrationHeaders(GetSlotId(), strAppId, strServiceId, objHeaders);

    // Removes extra headers
    if (!objHeaders.IsEmpty())
    {
        IRegParameter *piRegParameter = pRegParam;

        piRegParameter->RemoveExtraHeaders(objHeaders);
    }
}

/*

Remarks

*/
PRIVATE
void FakeRegistration::CallListener(IN IMS_SINT32 nPrevState, IN IMS_SINT32 nPrevSubState,
        IN IMS_SINT32 nReason)
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
        if ((nPrevSubState == SUB_STATE_REGISTERING)
            || (nPrevSubState == SUB_STATE_REFRESHING))
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
void FakeRegistration::ChoosePreferredContact()
{
    RegContact *pPreferredContact = IMS_NULL;

    //---------------------------------------------------------------------------------------------

    for (IMS_UINT32 i = 0; i < objContacts.GetSize(); ++i)
    {
        RegContact *pContact = objContacts.GetAt(i);

        if (pPreferredContact == IMS_NULL)
        {
            pPreferredContact = pContact;
        }
        else
        {
            const AString &strNew = pContact->GetPreference();
            const AString &strOld = pPreferredContact->GetPreference();

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
IMS_SINT32 FakeRegistration::GetPortUC() const
{
    IMS_SINT32 nPortUC = pRegParam->GetProtectedPortUC();

    //---------------------------------------------------------------------------------------------

    // Check if the UE supports the Security Association Agreement.
    // If nPortUC returns SIP::PORT_UNSPECIFIED, then it does not support "sec-agree".
    if (nPortUC == SIP::PORT_UNSPECIFIED)
    {
        return GetPortUS();
    }

    if (!SIPConfigProxy::IsIpSecConfigured(GetSlotId(), pStateTracker->GetSIPProfile()))
    {
        if (nPortUC == 0)
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
IMS_SINT32 FakeRegistration::GetPortUS() const
{
    const RegContact *pContact = pStateTracker->GetPreferredContact();

    //---------------------------------------------------------------------------------------------

    if (pContact == IMS_NULL)
    {
        if (pRegParam != IMS_NULL)
        {
            return pRegParam->GetPort();
        }

        return SIP::PORT_5060;
    }

    return pContact->GetPort();
}

/*

Remarks

*/
PRIVATE
IMS_SINT32 FakeRegistration::GetSubState() const
{
    //---------------------------------------------------------------------------------------------

    return nSubState;
}

/*

Remarks

*/
PRIVATE
void FakeRegistration::NotifyResponse(IN IMS_SINT32 nStatusCode)
{
    //---------------------------------------------------------------------------------------------

    IMS_TRACE_I("Registration - %d response to REGISTER received", nStatusCode, 0, 0);

    if (SIPStatusCode::Is1XX(nStatusCode))
    {
        return;
    }

    IMS_SINT32 nPrevState = GetState();
    IMS_SINT32 nPrevSubState = GetSubState();

    // Authentication challenged by S-CSCF
    if ((nStatusCode == SIPStatusCode::SC_401)
            || (nStatusCode == SIPStatusCode::SC_407))
    {
        PostMessage(AMSG_REGISTRATION_RESPONSE_RECEIVED, SIPStatusCode::SC_200, 0);
        return;
    }

    // Reset the flags after the transaction completed
    SetSubState(SUB_STATE_IDLE);

    // Handle the response except for 401/407
    if (SIPStatusCode::IsFinalSuccess(nStatusCode))
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
            UpdateBindingState(BINDING_RESULT_OK);
        else
            UpdateBindingState(BINDING_RESULT_NOK);

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

/*

Remarks

*/
PRIVATE
void FakeRegistration::SetState(IN IMS_SINT32 nState)
{
    //---------------------------------------------------------------------------------------------

    IMS_TRACE_I("Registration (%s) :: %s to %s",
            SIPDebug::GetUri1(pStateTracker->GetAOR().GetURI()).GetStr(),
            StateToString(this->nState), StateToString(nState));

    this->nState = nState;
}

/*

Remarks

*/
PRIVATE
void FakeRegistration::SetSubState(IN IMS_SINT32 nSubState)
{
    //---------------------------------------------------------------------------------------------

    IMS_TRACE_I("Registration (Sub-State) :: %s to %s",
            SubStateToString(this->nSubState), SubStateToString(nSubState), 0);

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
void FakeRegistration::StorePersistentHeaders()
{
    //---------------------------------------------------------------------------------------------

    // Stores the public user identities
    AStringArray objTmpArray;

    // If P-Associated-URI does not contains in 200 OK,
    // the explicit registration AOR will be added.
    if (objTmpArray.IsEmpty())
    {
        objTmpArray.AddElement(pStateTracker->GetAOR().ToString());

        IMS_TRACE_D("P-Associated-URI is not present; AOR (%s)",
                SIPDebug::GetUri1(objTmpArray.GetElementAt(0)).GetStr(), 0, 0);
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

    // Stores the Service-Route from the preloaded route set
    pStateTracker->SetServiceRoutes(pRegParam->GetPreloadedRoutes());

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
IMS_RESULT FakeRegistration::UpdateBindings(IN IMS_BOOL bIsContactWildcard /* = IMS_FALSE */)
{
    //---------------------------------------------------------------------------------------------

    if (bIsContactWildcard)
    {
        // Only for de-registration procedure
        // So, all the registration contacts transit to "STATE_TERMINATED"
        for (IMS_UINT32 j = 0; j < objContacts.GetSize(); ++j)
        {
            RegContact *pContact = objContacts.GetAt(j);

            pContact->SetTerminated();
        }

        return BINDING_UPDATE_OK;
    }

    IMS_RESULT nBindingUpdateResult = BINDING_UPDATE_OK;
    IMS_BOOL bAtLeastOneContactUpdated = IMS_FALSE;

    // For each registration contact, updates the parameter ('expires')
    // If no match found, it transits to "STATE_TERMINATED"
    for (IMS_UINT32 j = 0; j < objContacts.GetSize(); ++j)
    {
        RegContact *pContact = objContacts.GetAt(j);
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
void FakeRegistration::UpdateBindingState(IN IMS_SINT32 nState)
{
    //---------------------------------------------------------------------------------------------

    for (IMS_UINT32 i = 0; i < objObservers.GetSize(); ++i)
    {
        RegObserver *pObserver = objObservers.GetAt(i);

        if (pObserver == IMS_NULL)
            continue;

        pObserver->Update(nState);
    }
}

/*

Remarks

*/
PRIVATE
void FakeRegistration::UpdateCSeqNumber()
{
    //---------------------------------------------------------------------------------------------

    // Increase the CSeq number after the transaction is completed
    IMS_SINT32 nSeqNum = pRegFlow->IncreaseNGetCSeqValue(0);

    IMS_TRACE_I("Registration - Sequence Number Changed (%d >> %d)", nSeqNum, (nSeqNum + 1), 0);

    // Store the current sequence number (last one) from the last successful response.
    pRegFlow->SetCSeqValue(nSeqNum + 1);
}

/*

Remarks

*/
PRIVATE GLOBAL
const IMS_CHAR* FakeRegistration::StateToString(IN IMS_SINT32 nState)
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
PRIVATE GLOBAL
const IMS_CHAR* FakeRegistration::SubStateToString(IN IMS_SINT32 nSubState)
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
