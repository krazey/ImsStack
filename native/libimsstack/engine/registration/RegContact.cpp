/*
    Author
    <table>
    date      author                    description
    --------  --------------            ----------
    20090908  toastops@                 Created
    </table>

    Description

*/

#include "ServiceMemory.h"
#include "ServiceTrace.h"
#include "Feature.h"
#include "Configuration.h"
#include "Sip.h"
#include "SipFeatures.h"
#include "SipDebug.h"
#include "ISipHeader.h"
#include "ISipClientConnection.h"
#include "SipParameter.h"
#include "SipConfigProxy.h"
#include "util/CallerCapability.h"
#include "RegBindingProxy.h"
#include "IRegCapabilityChangeListener.h"
#include "RegContact.h"

__IMS_TRACE_TAG_REG__;

PUBLIC
RegContact::RegContact(IN IMS_SINT32 nSlotId, IN CONST IPAddress& objIPA_, IN IMS_SINT32 nPort_,
        IN IRegCapabilityChangeListener* piListener_, IN IMS_SINT32 nRegId_ /* = (-1) */,
        IN CONST SipProfile* pSIPProfile /* = IMS_NULL*/) :
        ImsSlot(nSlotId),
        nState(STATE_CREATED),
        pAOR(IMS_NULL),
        objIPA(objIPA_),
        nPolicyUserInfo(POLICY_USER_INFO_IMPU),
        pInstanceParameter(IMS_NULL),
        pRegIdParameter(IMS_NULL),
        pPubGRUU(IMS_NULL),
        pTempGRUU(IMS_NULL),
        objTempGRUUs(IMSList<SipAddress*>()),
        bFlag_BindingsUpdateTracker(IMS_TRUE),
        bFlag_AllCapabilitiesByConfig(IMS_TRUE),
        pAllCapabilities(new CallerCapability(0xFFFFFFFF)),
        pExtraCapabilities(new CallerCapability(0xFFFFFFFF)),
        objCallerCapabilities(IMSList<CallerCapability*>()),
        piCapabilityChangeListener(piListener_),
        pRegisteredCapabilities(IMS_NULL),
        nInitialExpires(EXPIRES_NOT_SPECIFIED),
        nNetworkProvisionedExpires(EXPIRES_NOT_SPECIFIED)
{
    objContactAddress.SetScheme(Sip::STR_SIP);
    objContactAddress.SetHost(objIPA_.ToString());
    objContactAddress.SetPort(nPort_);

    if (nRegId_ > 0)
    {
        if (SipConfigProxy::IsMultipleRegConfigured(GetSlotId(), pSIPProfile))
        {
            AString strRegId;
            strRegId.SetNumber(nRegId_);

            pRegIdParameter = new SipParameter(Sip::STR_REG_ID, strRegId);
        }
        else
        {
            IMS_TRACE_D("Multiple registration is not configured; reg-id=%d", nRegId_, 0, 0);
        }
    }
}

PUBLIC VIRTUAL RegContact::~RegContact()
{
    RegBindingProxy::UnbindContact(GetSlotId(), this);

    if (pAOR != IMS_NULL)
    {
        delete pAOR;
    }

    if (pInstanceParameter != IMS_NULL)
    {
        delete pInstanceParameter;
        pInstanceParameter = IMS_NULL;
    }

    if (pRegIdParameter != IMS_NULL)
    {
        delete pRegIdParameter;
        pRegIdParameter = IMS_NULL;
    }

    DestroyGRUU();

    objCallerCapabilities.Clear();

    if (pAllCapabilities != IMS_NULL)
    {
        delete pAllCapabilities;
    }

    if (pExtraCapabilities != IMS_NULL)
    {
        delete pExtraCapabilities;
    }

    if (pRegisteredCapabilities != IMS_NULL)
    {
        delete pRegisteredCapabilities;
    }

    RemoveAllHeaderParameters();

    IMS_TRACE_D("Destructor :: RegContact (%s, %d)", SipDebug::GetIp(objIPA),
            objContactAddress.GetPort(), 0);
}

/*

Remarks

*/
PUBLIC VIRTUAL const SipAddress& RegContact::GetContactAddress() const
{
    //---------------------------------------------------------------------------------------------

    return objContactAddress;
}

/*

Remarks

*/
PUBLIC VIRTUAL IMS_UINT32 RegContact::GetExpires() const
{
    //---------------------------------------------------------------------------------------------

    if (nState != STATE_ACTIVE)
    {
        return nInitialExpires;
    }

    return nNetworkProvisionedExpires;
}

/*

Remarks

*/
PUBLIC VIRTUAL const IPAddress& RegContact::GetIPAddress() const
{
    //---------------------------------------------------------------------------------------------

    return objIPA;
}

/*

Remarks

*/
PUBLIC VIRTUAL IMS_SINT32 RegContact::GetPort() const
{
    //---------------------------------------------------------------------------------------------

    return objContactAddress.GetPort();
}

/*

Remarks

*/
PUBLIC VIRTUAL const IMSList<SipParameter*>& RegContact::GetHeaderParameters() const
{
    //---------------------------------------------------------------------------------------------

    return objHeaderParams;
}

/*

Remarks

*/
PUBLIC VIRTUAL const SipParameter* RegContact::GetInstanceParameter() const
{
    //---------------------------------------------------------------------------------------------

    return pInstanceParameter;
}

/*

Remarks
 MULTIPLE_REGISTRATION
*/
PUBLIC VIRTUAL const SipParameter* RegContact::GetRegIdParameter() const
{
    //---------------------------------------------------------------------------------------------

    return pRegIdParameter;
}

/*

Remarks

*/
PUBLIC VIRTUAL const SipAddress* RegContact::GetPublicGRUU() const
{
    //---------------------------------------------------------------------------------------------

    return pPubGRUU;
}

/*

Remarks

*/
PUBLIC VIRTUAL const SipAddress* RegContact::GetTemporaryGRUU() const
{
    //---------------------------------------------------------------------------------------------

    return pTempGRUU;
}

/*

Remarks

*/
PUBLIC VIRTUAL const IMSList<SipAddress*>& RegContact::GetTemporaryGRUUs() const
{
    //---------------------------------------------------------------------------------------------

    return objTempGRUUs;
}

/*

Remarks

*/
PUBLIC VIRTUAL IMS_BOOL RegContact::IsActiveBinding() const
{
    //---------------------------------------------------------------------------------------------

    if (nState != STATE_ACTIVE)
        return IMS_FALSE;

    return IMS_TRUE;
}

/*

Remarks

*/
PUBLIC VIRTUAL IMS_BOOL RegContact::IsEmpty() const
{
    //---------------------------------------------------------------------------------------------

    return (objCallerCapabilities.IsEmpty());
}

/*

Remarks

*/
PUBLIC
void RegContact::DestroyGRUU()
{
    //---------------------------------------------------------------------------------------------

    if (pPubGRUU != IMS_NULL)
    {
        delete pPubGRUU;
        pPubGRUU = IMS_NULL;
    }

    if (pTempGRUU != IMS_NULL)
    {
        delete pTempGRUU;
        pTempGRUU = IMS_NULL;
    }

    if (!objTempGRUUs.IsEmpty())
    {
        for (IMS_UINT32 i = 0; i < objTempGRUUs.GetSize(); ++i)
        {
            SipAddress* pAddress = objTempGRUUs.GetAt(i);

            if (pAddress != IMS_NULL)
            {
                delete pAddress;
            }
        }

        objTempGRUUs.Clear();
    }
}

/*

Remarks

*/
PUBLIC
IMS_UINT32 RegContact::GetInitialExpires() const
{
    //---------------------------------------------------------------------------------------------

    return nInitialExpires;
}

/*

Remarks

*/
PUBLIC
const AString& RegContact::GetPreference() const
{
    //---------------------------------------------------------------------------------------------

    for (IMS_UINT32 i = 0; i < objHeaderParams.GetSize(); ++i)
    {
        const SipParameter* pParameter = objHeaderParams.GetAt(i);

        if (pParameter->GetName().EqualsIgnoreCase('q'))
        {
            return pParameter->GetValue();
        }
    }

    return AString::ConstNull();
}

/*

Remarks

*/
PUBLIC
IMS_SINT32 RegContact::GetState() const
{
    //---------------------------------------------------------------------------------------------

    return nState;
}

/*

Remarks

*/
PUBLIC
IMS_BOOL RegContact::IsBindingsUpdated() const
{
    //---------------------------------------------------------------------------------------------

    return bFlag_BindingsUpdateTracker;
}

/*

Remarks

*/
PUBLIC
IMS_BOOL RegContact::IsExpirationValueSpecified() const
{
    //---------------------------------------------------------------------------------------------

    return (nInitialExpires != EXPIRES_NOT_SPECIFIED);
}

/*

Remarks

*/
PUBLIC
void RegContact::Restore()
{
    //---------------------------------------------------------------------------------------------

    bFlag_BindingsUpdateTracker = IMS_FALSE;

    SetState(STATE_CREATED);

    DestroyGRUU();
}

/*

Remarks

*/
PUBLIC
void RegContact::SetAOR(IN CONST SipAddress& objAOR)
{
    //---------------------------------------------------------------------------------------------

    if (pAOR != IMS_NULL)
    {
        delete pAOR;
        pAOR = IMS_NULL;
    }

    pAOR = new SipAddress(objAOR);

    if ((nPolicyUserInfo == POLICY_USER_INFO_IMPU) && (pAOR != IMS_NULL))
    {
        const SipAddress::UserInfoPart* pUIPart = pAOR->GetUserInfoPart();

        if (pUIPart == IMS_NULL)
        {
            objContactAddress.SetUser(pAOR->GetUser());
        }
        else
        {
            objContactAddress.SetUser(pUIPart->GetUser());
        }
    }
}

/*

Remarks

*/
PUBLIC
void RegContact::SetExpires(IN IMS_UINT32 nValue)
{
    //---------------------------------------------------------------------------------------------

    nInitialExpires = nValue;
}

/*

Remarks
 IMS_IPSEC_UDP_ENC
*/
PUBLIC
void RegContact::SetHostInfo(IN CONST IPAddress& objIP)
{
    //---------------------------------------------------------------------------------------------

    objContactAddress.SetHost(objIP.ToString());
}

/*

Remarks

*/
PUBLIC
void RegContact::SetTerminated()
{
    //---------------------------------------------------------------------------------------------

    bFlag_BindingsUpdateTracker = IMS_FALSE;

    SetState(STATE_TERMINATED);
}

/*

Remarks

*/
PUBLIC
AString RegContact::ToString() const
{
    AStringBuffer objStringBuffer(256);

    //---------------------------------------------------------------------------------------------

    FormContact(IMS_FALSE, objStringBuffer);

    return static_cast<const AStringBuffer&>(objStringBuffer).GetString();
}

/*

Remarks

*/
PUBLIC
AString RegContact::ToStringWithExpires() const
{
    AStringBuffer objStringBuffer(256);

    //---------------------------------------------------------------------------------------------

    FormContact(IMS_TRUE, objStringBuffer);

    return static_cast<const AStringBuffer&>(objStringBuffer).GetString();
}

/*

Remarks

*/
PUBLIC
IMS_SINT32 RegContact::UpdateParameter(IN IMS_SINT32 nExpiresValue)
{
    //---------------------------------------------------------------------------------------------

    bFlag_BindingsUpdateTracker = IMS_FALSE;

    if (nExpiresValue > 0)
    {
        nNetworkProvisionedExpires = nExpiresValue;

        SetState(STATE_ACTIVE);
    }
    else
    {
        SetTerminated();
        return UPDATE_NO_EXPIRES;
    }

    return UPDATE_OK;
}

/*

Remarks

*/
PUBLIC
IMS_SINT32 RegContact::UpdateParameter(
        IN CONST IMSList<ISipHeader*>& objContactHeaders, IN IMS_SINT32 nExpiresValue)
{
    //---------------------------------------------------------------------------------------------

    bFlag_BindingsUpdateTracker = IMS_FALSE;

    for (IMS_UINT32 i = 0; i < objContactHeaders.GetSize(); ++i)
    {
        const ISipHeader* piHeader = objContactHeaders.GetAt(i);
        const SipAddress* pAddress = piHeader->GetSipAddress();

        if (pAddress == IMS_NULL)
            continue;

        // Find the matched contact address
        IMS_BOOL bSameContact = pAddress->Equals(objContactAddress);

        // If "transport" parameter is included in Contact header and it needs to be
        // ignored when comparing reg-binding, URI comparison should be done
        // without "transport" parameter.
        if (!bSameContact && SipFeatures::IsTransportParameterIgnoredForRegBinding(GetSlotId()))
        {
            const AString strTransport("transport");

            if ((pAddress->GetParameter(strTransport) != IMS_NULL) &&
                    (objContactAddress.GetParameter(strTransport) == IMS_NULL))
            {
                SipAddress objAddress(*pAddress);
                objAddress.RemoveParameter(strTransport);

                bSameContact = objAddress.Equals(objContactAddress);

                if (bSameContact)
                {
                    IMS_TRACE_D("LV :: Contact is same w/o \"transport\" parameter", 0, 0, 0);
                }
            }
        }

        if (bSameContact)
        {
            UpdateRegisteredCapabilities(piHeader);
            UpdateGRUU(piHeader);

            // Extract 'expires' parameter
            const SipParameter* pParameter = piHeader->GetParameter(Sip::STR_EXPIRES);

            if (pParameter != IMS_NULL)
            {
                IMS_BOOL bOK = IMS_FALSE;

                nNetworkProvisionedExpires = pParameter->GetValue().ToInt32(&bOK);

                if (bOK)
                {
                    if (nNetworkProvisionedExpires > 0)
                    {
                        SetState(STATE_ACTIVE);
                        return UPDATE_OK;
                    }
                    else
                    {
                        SetTerminated();
                        return UPDATE_NO_EXPIRES;
                    }
                }
                else
                {
                    if (nExpiresValue > 0)
                    {
                        nNetworkProvisionedExpires = nExpiresValue;

                        SetState(STATE_ACTIVE);
                        return UPDATE_OK;
                    }
                    else
                    {
                        SetTerminated();
                        return UPDATE_NO_EXPIRES;
                    }
                }
            }
            else
            {
                if (nExpiresValue > 0)
                {
                    nNetworkProvisionedExpires = nExpiresValue;
                    SetState(STATE_ACTIVE);
                    return UPDATE_OK;
                }
                else
                {
                    SetTerminated();
                    return UPDATE_NO_EXPIRES;
                }
            }

            // 4 break ??
            break;
        }
        else
        {
            IMS_TRACE_D("Not matched contact (%s, %s)",
                    SipDebug::GetUri1(pAddress->GetUri()).GetStr(),
                    SipDebug::GetUri2(objContactAddress.GetUri()).GetStr(), 0);
        }
    }

    // If any matched address does not exist, we consider that this contact is terminated.
    SetTerminated();

    return UPDATE_OK;
}

/*

Remarks

*/
PRIVATE VIRTUAL IMS_BOOL RegContact::AddHeaderParameter(
        IN CONST AString& strName, IN CONST AString& strValue /* = AString::ConstNull() */)
{
    //---------------------------------------------------------------------------------------------

    if (strName.StartsWith('+') && strName.EqualsIgnoreCase(Sip::STR_SIP_INSTANCE))
    {
        if (pInstanceParameter != IMS_NULL)
        {
            pInstanceParameter->SetValue(strValue);
        }
        else
        {
            pInstanceParameter = new SipParameter(strName, strValue);

            if (pInstanceParameter == IMS_NULL)
            {
                return IMS_FALSE;
            }
        }

        return IMS_TRUE;
    }

    if (strName.EqualsIgnoreCase(Sip::STR_REG_ID))
    {
        if (pRegIdParameter != IMS_NULL)
        {
            pRegIdParameter->SetValue(strValue);
        }
        else
        {
            pRegIdParameter = new SipParameter(strName, strValue);

            if (pRegIdParameter == IMS_NULL)
            {
                return IMS_FALSE;
            }
        }

        return IMS_TRUE;
    }

    for (IMS_UINT32 i = 0; i < objHeaderParams.GetSize(); ++i)
    {
        SipParameter* pParameter = objHeaderParams.GetAt(i);

        if (strName.EqualsIgnoreCase(pParameter->GetName()))
        {
            if (!strValue.IsNULL())
            {
                pParameter->AddValues(strValue);
            }

            return IMS_TRUE;
        }
    }

    SipParameter* pParameter = new SipParameter(strName, strValue);

    if (pParameter == IMS_NULL)
    {
        return IMS_FALSE;
    }

    if (!objHeaderParams.Append(pParameter))
    {
        delete pParameter;
        return IMS_FALSE;
    }

    return IMS_TRUE;
}

/*

Remarks

*/
PRIVATE VIRTUAL IMS_BOOL RegContact::AddUriParameter(
        IN CONST AString& strName, IN CONST AString& strValue /* = AString::ConstNull() */)
{
    //---------------------------------------------------------------------------------------------

    if (objContactAddress.AddParameter(strName, strValue) != IMS_SUCCESS)
    {
        IMS_TRACE_E(0, "Adding uri parameter in Contact (%s)",
                SipDebug::GetUri1(objContactAddress.ToString()).GetStr(), 0, 0);
        return IMS_FALSE;
    }

    return IMS_TRUE;
}

/*

Remarks

*/
PRIVATE VIRTUAL void RegContact::RemoveAllHeaderParameters()
{
    //---------------------------------------------------------------------------------------------

    if (!objHeaderParams.IsEmpty())
    {
        for (IMS_UINT32 i = 0; i < objHeaderParams.GetSize(); ++i)
        {
            SipParameter* pParameter = objHeaderParams.GetAt(i);

            if (pParameter != IMS_NULL)
            {
                delete pParameter;
            }
        }

        objHeaderParams.Clear();
    }
}

/*

Remarks

*/
PRIVATE VIRTUAL void RegContact::RemoveHeaderParameter(
        IN CONST AString& strName, IN CONST AString& strValue /* = AString::ConstNull() */)
{
    //---------------------------------------------------------------------------------------------

    if (strName.StartsWith('+') && strName.EqualsIgnoreCase(Sip::STR_SIP_INSTANCE))
    {
        if (pInstanceParameter != IMS_NULL)
        {
            delete pInstanceParameter;
            pInstanceParameter = IMS_NULL;
        }

        return;
    }

    if (strName.EqualsIgnoreCase(Sip::STR_REG_ID))
    {
        if (pRegIdParameter != IMS_NULL)
        {
            delete pRegIdParameter;
            pRegIdParameter = IMS_NULL;
        }

        return;
    }

    for (IMS_UINT32 i = 0; i < objHeaderParams.GetSize(); ++i)
    {
        SipParameter* pParameter = objHeaderParams.GetAt(i);

        if (strName.EqualsIgnoreCase(pParameter->GetName()))
        {
            if (!strValue.IsNULL())
            {
                pParameter->RemoveValue(strValue);

                if (pParameter->GetValues().IsEmpty())
                {
                    objHeaderParams.RemoveAt(i);
                    delete pParameter;
                    return;
                }
            }
            else
            {
                objHeaderParams.RemoveAt(i);
                delete pParameter;
                return;
            }
        }
    }
}

/*

Remarks

*/
PRIVATE VIRTUAL void RegContact::RemoveUriParameter(
        IN CONST AString& strName, IN CONST AString& strValue /* = AString::ConstNull() */)
{
    //---------------------------------------------------------------------------------------------

    (void)strValue;

    objContactAddress.RemoveParameter(strName);
}

/*

Remarks

*/
PRIVATE VIRTUAL void RegContact::SetDisplayName(IN CONST AString& strDisplayName)
{
    //---------------------------------------------------------------------------------------------

    objContactAddress.SetDisplayName(strDisplayName);
}

/*

Remarks

*/
PRIVATE VIRTUAL void RegContact::SetListener(IN IRegContactListener* piListener)
{
    //---------------------------------------------------------------------------------------------

    (void)piListener;
}

/*

Remarks

*/
PRIVATE VIRTUAL void RegContact::SetPolicyForCallerCapability(IN IMS_BOOL bCapsByApp)
{
    //---------------------------------------------------------------------------------------------

    bFlag_AllCapabilitiesByConfig = (bCapsByApp) ? IMS_FALSE : IMS_TRUE;
}

/*

Remarks

*/
PRIVATE VIRTUAL void RegContact::SetPort(IN IMS_SINT32 nPort)
{
    //---------------------------------------------------------------------------------------------

    objContactAddress.SetPort(nPort);
}

/*

Remarks

*/
PRIVATE VIRTUAL void RegContact::SetUserInfo(IN IMS_SINT32 nPolicy /* = POLICY_USER_INFO_IMPU */,
        IN CONST AString& strUserInfo /* = AString::ConstNull() */)
{
    //---------------------------------------------------------------------------------------------

    nPolicyUserInfo = nPolicy;

    if (nPolicyUserInfo == POLICY_USER_INFO_APP)
    {
        objContactAddress.SetUser(strUserInfo);
    }
    else if (nPolicyUserInfo == POLICY_USER_INFO_IMPU)
    {
        if (pAOR != IMS_NULL)
        {
            objContactAddress.SetUser(pAOR->GetUser());
        }
    }
    else
    {
        objContactAddress.SetUser(AString::ConstNull());
    }
}

/*

Remarks

*/
PRIVATE VIRTUAL IMS_BOOL RegContact::AddExtraCapability(
        IN CONST AString& strName, IN CONST AString& strValue)
{
    //---------------------------------------------------------------------------------------------

    if (pExtraCapabilities == IMS_NULL)
    {
        return IMS_FALSE;
    }

    FeatureSet objFeatureSet(strName, strValue);

    if (strValue.GetLength() == 0)
    {
        objFeatureSet.Add(strName);
    }

    pExtraCapabilities->AddFeature(&objFeatureSet);

    return IMS_TRUE;
}

/*

Remarks

*/
PRIVATE VIRTUAL void RegContact::RemoveExtraCapability(
        IN CONST AString& strName, IN CONST AString& strValue)
{
    //---------------------------------------------------------------------------------------------

    if (pExtraCapabilities == IMS_NULL)
    {
        return;
    }

    if (strName.IsNULL() && strValue.IsNULL())
    {
        pExtraCapabilities->Clear();
        return;
    }

    FeatureSet objFeatureSet(strName, strValue);

    if (strValue.GetLength() == 0)
    {
        objFeatureSet.Add(strName);
    }

    pExtraCapabilities->RemoveFeature(&objFeatureSet);
}

/*

Remarks

*/
PRIVATE VIRTUAL IMS_BOOL RegContact::AddService(
        IN CONST AString& strAppId, IN CONST AString& strServiceId)
{
    CallerCapability* pCC = IMS_NULL;

    //---------------------------------------------------------------------------------------------

    RegBindingProxy::QueryCapability(GetSlotId(), strAppId, strServiceId, pCC);

    if (pCC == IMS_NULL)
    {
        IMS_TRACE_D("No caller capabilities (%s, %s)", strAppId.GetStr(), strServiceId.GetStr(), 0);
        return IMS_FALSE;
    }

    if (!RegBindingProxy::BindContact(GetSlotId(), strAppId, strServiceId, this))
    {
        IMS_TRACE_D("Binding Contact failed (%s, %s)", strAppId.GetStr(), strServiceId.GetStr(), 0);
        return IMS_FALSE;
    }

    AddCallerCapability(pCC);

    if (piCapabilityChangeListener != IMS_NULL)
    {
        piCapabilityChangeListener->RegCapabilityChange_ServiceAdded(strAppId, strServiceId);
    }

    return IMS_TRUE;
}

/*

Remarks

*/
PRIVATE VIRTUAL void RegContact::RemoveService(
        IN CONST AString& strAppId, IN CONST AString& strServiceId)
{
    CallerCapability* pCC = IMS_NULL;

    //---------------------------------------------------------------------------------------------

    RegBindingProxy::QueryCapability(GetSlotId(), strAppId, strServiceId, pCC);

    if (pCC == IMS_NULL)
    {
        IMS_TRACE_D("No caller capabilities (%s, %s)", strAppId.GetStr(), strServiceId.GetStr(), 0);
    }

    RegBindingProxy::UnbindContact(GetSlotId(), strAppId, strServiceId);

    RemoveCallerCapability(pCC);

    if (piCapabilityChangeListener != IMS_NULL)
    {
        piCapabilityChangeListener->RegCapabilityChange_ServiceRemoved(strAppId, strServiceId);
    }
}

/*

Remarks

*/
PRIVATE VIRTUAL IMS_BOOL RegContact::IsServiceRegistered(
        IN const AString& strAppId, IN const AString& strServiceId) const
{
    CallerCapability* pCC = IMS_NULL;

    RegBindingProxy::QueryCapability(GetSlotId(), strAppId, strServiceId, pCC);

    if (pCC == IMS_NULL)
    {
        return IMS_FALSE;
    }

    if (!IsActiveBinding())
    {
        return IMS_FALSE;
    }

    if (pRegisteredCapabilities == IMS_NULL)
    {
        return IMS_TRUE;
    }

    if (pRegisteredCapabilities->IsEmpty())
    {
        return IMS_TRUE;
    }

    const IAppConfig* piAppConfig =
            Configuration::GetInstance()->GetAppConfig(strAppId, GetSlotId());
    const ICoreServiceConfig* piServiceConfig =
            (piAppConfig != IMS_NULL) ? piAppConfig->GetCoreServiceConfig(strServiceId) : IMS_NULL;

    if (piServiceConfig == IMS_NULL)
    {
        return IMS_TRUE;
    }

    RCPtr<Feature> pFeature = IMS_NULL;

    if (piServiceConfig->IsIARISupported())
    {
        const ServiceIdentifier& objIARI = piServiceConfig->GetIARI();
        pFeature = new Feature(Feature::OTHER_G_3GPP_IARI_REF, objIARI.GetName());

        if (!pRegisteredCapabilities->HasFeature(pFeature.Get()))
        {
            return IMS_FALSE;
        }

        pFeature = IMS_NULL;
    }

    const IMSList<ServiceIdentifier>& objICSIs = piServiceConfig->GetICSIs();

    for (IMS_UINT32 i = 0; i < objICSIs.GetSize(); i++)
    {
        const ServiceIdentifier& objICSI = objICSIs.GetAt(i);
        pFeature = new Feature(Feature::OTHER_G_3GPP_ICSI_REF, objICSI.GetName());

        if (!pRegisteredCapabilities->HasFeature(pFeature.Get()))
        {
            return IMS_FALSE;
        }

        pFeature = IMS_NULL;
    }

    const IMSList<ServiceIdentifier>& objFeatureTags = piServiceConfig->GetFeatureTags();

    for (IMS_UINT32 i = 0; i < objFeatureTags.GetSize(); i++)
    {
        const ServiceIdentifier& objFeatureTag = objFeatureTags.GetAt(i);
        pFeature = new Feature(objFeatureTag.GetName());

        if (!pRegisteredCapabilities->HasFeature(pFeature.Get()))
        {
            return IMS_FALSE;
        }

        pFeature = IMS_NULL;
    }

    IMS_TRACE_D("ServiceRegistered :: [%s-%s]", strAppId.GetStr(), strServiceId.GetStr(), 0);

    return IMS_TRUE;
}

/*

Remarks

*/
PRIVATE VIRTUAL IMS_BOOL RegContact::IsFeatureRegistered(
        IN const AString& strFTName, IN const AString& strFTValue /* = AString::ConstNull()*/) const
{
    if (!IsActiveBinding())
    {
        return IMS_FALSE;
    }

    if (strFTName.GetLength() == 0)
    {
        return IMS_TRUE;
    }

    if (pRegisteredCapabilities == IMS_NULL)
    {
        return IMS_TRUE;
    }

    if (pRegisteredCapabilities->IsEmpty())
    {
        return IMS_TRUE;
    }

    RCPtr<Feature> pFeature = new Feature(strFTName, strFTValue);

    if (!pRegisteredCapabilities->HasFeature(pFeature.Get()))
    {
        return IMS_FALSE;
    }

    if (strFTValue.GetLength() == 0)
    {
        IMS_TRACE_D("FeatureRegistered :: [%s]", strFTName.GetStr(), 0, 0);
    }
    else
    {
        IMS_TRACE_D("FeatureRegistered :: [%s=%s]", strFTName.GetStr(), strFTValue.GetStr(), 0);
    }

    return IMS_TRUE;
}

/*

Remarks

*/
PRIVATE VIRTUAL void RegContact::RecalculateCallerCapabilities()
{
    //---------------------------------------------------------------------------------------------

    pAllCapabilities->Clear();

    for (IMS_UINT32 i = 0; i < objCallerCapabilities.GetSize(); ++i)
    {
        pAllCapabilities->AddFeatures(objCallerCapabilities.GetAt(i));
    }
}

/*

Remarks

*/
PRIVATE
void RegContact::FormContact(IN IMS_BOOL bExpiresRequired, OUT AStringBuffer& objSB) const
{
    //---------------------------------------------------------------------------------------------

    objSB.Append(objContactAddress.ToString());

    if (bFlag_AllCapabilitiesByConfig)
    {
        if (!pAllCapabilities->IsEmpty() && pExtraCapabilities->IsEmpty())
        {
            objSB.Append(TextParser::CHAR_SEMICOLON);
            objSB.Append(pAllCapabilities->ToString());
        }
        else if (pAllCapabilities->IsEmpty() && !pExtraCapabilities->IsEmpty())
        {
            objSB.Append(TextParser::CHAR_SEMICOLON);
            objSB.Append(pExtraCapabilities->ToString());
        }
        else if (!pAllCapabilities->IsEmpty() && !pExtraCapabilities->IsEmpty())
        {
            CallerCapability* pCC = new CallerCapability(0);

            pCC->AddFeatures(pAllCapabilities);
            pCC->AddFeatures(pExtraCapabilities);

            objSB.Append(TextParser::CHAR_SEMICOLON);
            objSB.Append(pCC->ToString());

            delete pCC;
        }
    }
    else
    {
        if (!pExtraCapabilities->IsEmpty())
        {
            objSB.Append(TextParser::CHAR_SEMICOLON);
            objSB.Append(pExtraCapabilities->ToString());
        }
    }

    // Adds a 'expires' parameter
    if (bExpiresRequired)
    {
        if (nInitialExpires != EXPIRES_NOT_SPECIFIED)
        {
            AString strExpires;
            strExpires.Sprintf("%lu", nInitialExpires);

            objSB.Append(TextParser::CHAR_SEMICOLON);
            objSB.Append(Sip::STR_EXPIRES);
            objSB.Append(TextParser::CHAR_EQUAL);
            objSB.Append(strExpires);
        }
    }

    if (!objHeaderParams.IsEmpty())
    {
        for (IMS_UINT32 i = 0; i < objHeaderParams.GetSize(); ++i)
        {
            const SipParameter* pParameter = objHeaderParams.GetAt(i);

            if (pParameter == IMS_NULL)
                continue;

            objSB.Append(TextParser::CHAR_SEMICOLON);
            objSB.Append(pParameter->ToString());
        }
    }

    if (pInstanceParameter != IMS_NULL)
    {
        objSB.Append(TextParser::CHAR_SEMICOLON);
        objSB.Append(pInstanceParameter->ToString());
    }

    if (pRegIdParameter != IMS_NULL)
    {
        objSB.Append(TextParser::CHAR_SEMICOLON);
        objSB.Append(pRegIdParameter->ToString());
    }
}

/*

Remarks

*/
PRIVATE
IMS_BOOL RegContact::AddCallerCapability(IN CONST CallerCapability* pCC)
{
    //---------------------------------------------------------------------------------------------

    if (pAllCapabilities == IMS_NULL)
    {
        return IMS_FALSE;
    }

    if (!RegisterServiceCapability(pCC))
    {
        return IMS_FALSE;
    }

    IMS_BOOL bIsChanged = pAllCapabilities->AddFeatures(pCC);

    if (!bFlag_BindingsUpdateTracker)
    {
        bFlag_BindingsUpdateTracker = bIsChanged;
    }

    return IMS_TRUE;
}

/*

Remarks

*/
PRIVATE
IMS_BOOL RegContact::RemoveCallerCapability(IN CONST CallerCapability* pCC)
{
    //---------------------------------------------------------------------------------------------

    if (pAllCapabilities == IMS_NULL)
    {
        return IMS_FALSE;
    }

    UnregisterServiceCapability(pCC);

    IMS_BOOL bIsChanged = pAllCapabilities->RemoveFeatures(pCC);

    if (!bFlag_BindingsUpdateTracker)
    {
        bFlag_BindingsUpdateTracker = bIsChanged;
    }

    return IMS_TRUE;
}

/*

Remarks

*/
PRIVATE
IMS_BOOL RegContact::RegisterServiceCapability(IN CONST CallerCapability* pCC)
{
    //---------------------------------------------------------------------------------------------

    // First, checks if the service is already bound or not
    for (IMS_UINT32 i = 0; i < objCallerCapabilities.GetSize(); ++i)
    {
        const CallerCapability* pExCC = objCallerCapabilities.GetAt(i);

        if (pExCC->Equals(pCC))
        {
            return IMS_TRUE;
        }
    }

    // If not present, add a new service capability
    return objCallerCapabilities.Append(const_cast<CallerCapability*>(pCC));
}

/*

Remarks

*/
PRIVATE
void RegContact::UnregisterServiceCapability(IN CONST CallerCapability* pCC)
{
    //---------------------------------------------------------------------------------------------

    // First, checks if the service is already bound or not
    for (IMS_UINT32 i = 0; i < objCallerCapabilities.GetSize(); ++i)
    {
        const CallerCapability* pExCC = objCallerCapabilities.GetAt(i);

        if (pExCC->Equals(pCC))
        {
            objCallerCapabilities.RemoveAt(i);
        }
    }
}

/*

Remarks

*/
PRIVATE
void RegContact::SetState(IN IMS_SINT32 nState)
{
    //---------------------------------------------------------------------------------------------

    IMS_TRACE_I("RegContact (%s) :: %s to %s",
            SipDebug::GetUri1(objContactAddress.GetUri()).GetStr(), StateToString(this->nState),
            StateToString(nState));

    this->nState = nState;
}

/*

Remarks

*/
PRIVATE
void RegContact::UpdateGRUU(IN CONST ISipHeader* piHeader)
{
    //---------------------------------------------------------------------------------------------

    if (piHeader == IMS_NULL)
    {
        return;
    }

    // Destroys all the previous GRUU identities
    // DestroyGRUU();

    // 'pub-gruu' parameter
    const SipParameter* pParameter = piHeader->GetParameter("pub-gruu");

    if (pParameter != IMS_NULL)
    {
        // Destroy the previous public GRUU
        if (pPubGRUU != IMS_NULL)
        {
            delete pPubGRUU;
            pPubGRUU = IMS_NULL;
        }

        const AString& strValue = pParameter->GetValue();

        IMS_TRACE_D("pub-gruu (%s)", SipDebug::GetUri1(strValue).GetStr(), 0, 0);

        if (strValue.StartsWith(TextParser::CHAR_DQUOT))
        {
            pPubGRUU = new SipAddress(strValue.GetSubStr(1, strValue.GetLength() - 2));
        }
        else
        {
            pPubGRUU = new SipAddress(strValue);
        }

        if (pPubGRUU == IMS_NULL)
        {
            IMS_TRACE_E(0, "Creating a pub-gruu (%s) failed", SipDebug::GetUri1(strValue).GetStr(),
                    0, 0);
        }
    }

    // 'temp-gruu' parameter
    pParameter = piHeader->GetParameter("temp-gruu");

    if (pParameter != IMS_NULL)
    {
        // Destroy the previous temporary GRUUs
        if (pTempGRUU != IMS_NULL)
        {
            delete pTempGRUU;
            pTempGRUU = IMS_NULL;
        }

        if (!objTempGRUUs.IsEmpty())
        {
            for (IMS_UINT32 i = 0; i < objTempGRUUs.GetSize(); ++i)
            {
                SipAddress* pAddress = objTempGRUUs.GetAt(i);

                if (pAddress != IMS_NULL)
                {
                    delete pAddress;
                }
            }

            objTempGRUUs.Clear();
        }

        const AString& strValue = pParameter->GetValue();

        IMS_TRACE_D("temp-gruu (%s)", SipDebug::GetUri1(strValue).GetStr(), 0, 0);

        if (strValue.StartsWith(TextParser::CHAR_DQUOT))
        {
            pTempGRUU = new SipAddress(strValue.GetSubStr(1, strValue.GetLength() - 2));
        }
        else
        {
            pTempGRUU = new SipAddress(strValue);
        }

        if (pTempGRUU == IMS_NULL)
        {
            IMS_TRACE_E(0, "Creating a temp-gruu (%s) failed", SipDebug::GetUri1(strValue).GetStr(),
                    0, 0);
        }
        else
        {
            objTempGRUUs.Append(new SipAddress(*pTempGRUU));
        }
    }
}

/*

Remarks

*/
PRIVATE
void RegContact::UpdateRegisteredCapabilities(IN const ISipHeader* piHeader)
{
    if (pRegisteredCapabilities != IMS_NULL)
    {
        delete pRegisteredCapabilities;
        pRegisteredCapabilities = IMS_NULL;
    }

    const IMSList<SipParameter*>& objParameters = piHeader->GetParameters();

    if (objParameters.IsEmpty())
    {
        return;
    }

    CallerCapability* pCC = new CallerCapability(0);

    for (IMS_UINT32 i = 0; i < objParameters.GetSize(); i++)
    {
        const SipParameter* pParameter = objParameters.GetAt(i);

        if (pParameter == IMS_NULL)
        {
            continue;
        }

        const AString& strName = pParameter->GetName();

        if (!Feature::IsFeatureTag(strName) || strName.EqualsIgnoreCase(Sip::STR_SIP_INSTANCE))
        {
            continue;
        }

        RCPtr<Feature> pFeature = IMS_NULL;

        if (pParameter->IsNameOnly())
        {
            pFeature = new Feature(strName, AString::ConstNull());
            pCC->AddFeature(pFeature.Get());
            continue;
        }

        const AStringArray& objValues = pParameter->GetValues();

        for (IMS_SINT32 j = 0; j < objValues.GetCount(); j++)
        {
            pFeature = new Feature(strName, objValues.GetElementAt(j));
            pCC->AddFeature(pFeature.Get());
        }
    }

    if (pCC->IsEmpty())
    {
        delete pCC;
        return;
    }

    pRegisteredCapabilities = pCC;

    IMS_TRACE_I("RegisteredCapabilities=[%s]", pRegisteredCapabilities->ToString().GetStr(), 0, 0);
}

/*

Remarks

*/
PRIVATE GLOBAL const IMS_CHAR* RegContact::StateToString(IN IMS_SINT32 nState)
{
    //---------------------------------------------------------------------------------------------

    switch (nState)
    {
        case STATE_CREATED:
            return "STATE_CREATED";
        case STATE_ACTIVE:
            return "STATE_ACTIVE";
        case STATE_TERMINATED:
            return "STATE_TERMINATED";
        default:
            return "__INVALID__";
    }
}
