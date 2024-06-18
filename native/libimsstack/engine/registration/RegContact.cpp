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

#include "Feature.h"

#include "Engine.h"
#include "IConfiguration.h"
#include "IRegCapabilityChangeListener.h"
#include "ISipClientConnection.h"
#include "ISipHeader.h"
#include "RegBindingProxy.h"
#include "RegContact.h"
#include "Sip.h"
#include "SipConfigProxy.h"
#include "SipDebug.h"
#include "SipFeatures.h"
#include "SipParameter.h"

__IMS_TRACE_TAG_REG__;

PUBLIC
RegContact::RegContact(IN IMS_SINT32 nSlotId, IN const IpAddress& objIpAddr, IN IMS_SINT32 nPort,
        IN IRegCapabilityChangeListener* piListener, IN IMS_SINT32 nRegId /*= (-1)*/,
        IN const SipProfile* pProfile /*= IMS_NULL*/) :
        ImsSlot(nSlotId),
        m_nState(STATE_CREATED),
        m_pAor(IMS_NULL),
        m_objIpAddr(objIpAddr),
        m_nPolicyUserInfo(POLICY_USER_INFO_IMPU),
        m_pInstanceParameter(IMS_NULL),
        m_pRegIdParameter(IMS_NULL),
        m_pPubGruu(IMS_NULL),
        m_pTempGruu(IMS_NULL),
        m_objTempGruus(ImsList<SipAddress*>()),
        m_bBindingsUpdateTracker(IMS_TRUE),
        m_bAllCapabilitiesByConfig(IMS_TRUE),
        m_pAllCapabilities(new CallerCapability(0xFFFFFFFF)),
        m_pExtraCapabilities(new CallerCapability(0xFFFFFFFF)),
        m_objCallerCapabilities(ImsList<CallerCapability*>()),
        m_piCapabilityChangeListener(piListener),
        m_pRegisteredCapabilities(IMS_NULL),
        m_nInitialExpires(EXPIRES_NOT_SPECIFIED),
        m_nNetworkProvisionedExpires(EXPIRES_NOT_SPECIFIED)
{
    m_objContactAddress.SetScheme(Sip::STR_SIP);
    m_objContactAddress.SetHost(objIpAddr.ToString());
    m_objContactAddress.SetPort(nPort);

    if (nRegId > 0)
    {
        if (SipConfigProxy::IsMultipleRegConfigured(GetSlotId(), pProfile))
        {
            AString strRegId;
            strRegId.SetNumber(nRegId);

            m_pRegIdParameter = new SipParameter(Sip::STR_REG_ID, strRegId);
        }
        else
        {
            IMS_TRACE_D("Multiple registration is not configured; reg-id=%d", nRegId, 0, 0);
        }
    }
}

PUBLIC VIRTUAL RegContact::~RegContact()
{
    RegBindingProxy::UnbindContact(GetSlotId(), this);

    if (m_pAor != IMS_NULL)
    {
        delete m_pAor;
    }

    if (m_pInstanceParameter != IMS_NULL)
    {
        delete m_pInstanceParameter;
        m_pInstanceParameter = IMS_NULL;
    }

    if (m_pRegIdParameter != IMS_NULL)
    {
        delete m_pRegIdParameter;
        m_pRegIdParameter = IMS_NULL;
    }

    DestroyGruu();

    m_objCallerCapabilities.Clear();

    if (m_pAllCapabilities != IMS_NULL)
    {
        delete m_pAllCapabilities;
    }

    if (m_pExtraCapabilities != IMS_NULL)
    {
        delete m_pExtraCapabilities;
    }

    if (m_pRegisteredCapabilities != IMS_NULL)
    {
        delete m_pRegisteredCapabilities;
    }

    RemoveAllHeaderParameters();

    IMS_TRACE_D("Destructor :: RegContact (%s, %d)", SipDebug::GetIp(m_objIpAddr),
            m_objContactAddress.GetPort(), 0);
}

PUBLIC VIRTUAL IMS_UINT32 RegContact::GetExpires() const
{
    if (m_nState != STATE_ACTIVE)
    {
        return m_nInitialExpires;
    }

    return m_nNetworkProvisionedExpires;
}

PUBLIC VIRTUAL IMS_BOOL RegContact::IsActiveBinding() const
{
    if (m_nState != STATE_ACTIVE)
    {
        return IMS_FALSE;
    }

    return IMS_TRUE;
}

PUBLIC
void RegContact::DestroyGruu()
{
    if (m_pPubGruu != IMS_NULL)
    {
        delete m_pPubGruu;
        m_pPubGruu = IMS_NULL;
    }

    if (m_pTempGruu != IMS_NULL)
    {
        delete m_pTempGruu;
        m_pTempGruu = IMS_NULL;
    }

    if (!m_objTempGruus.IsEmpty())
    {
        for (IMS_UINT32 i = 0; i < m_objTempGruus.GetSize(); ++i)
        {
            SipAddress* pAddress = m_objTempGruus.GetAt(i);

            if (pAddress != IMS_NULL)
            {
                delete pAddress;
            }
        }

        m_objTempGruus.Clear();
    }
}

PUBLIC
const AString& RegContact::GetPreference() const
{
    for (IMS_UINT32 i = 0; i < m_objHeaderParams.GetSize(); ++i)
    {
        const SipParameter* pParameter = m_objHeaderParams.GetAt(i);

        if (pParameter->GetName().EqualsIgnoreCase('q'))
        {
            return pParameter->GetValue();
        }
    }

    return AString::ConstNull();
}

PUBLIC
void RegContact::Restore()
{
    m_bBindingsUpdateTracker = IMS_FALSE;

    SetState(STATE_CREATED);

    DestroyGruu();
}

PUBLIC
void RegContact::SetAor(IN const SipAddress& objAor)
{
    if (m_pAor != IMS_NULL)
    {
        delete m_pAor;
        m_pAor = IMS_NULL;
    }

    m_pAor = new SipAddress(objAor);

    if ((m_nPolicyUserInfo == POLICY_USER_INFO_IMPU) && (m_pAor != IMS_NULL))
    {
        const SipAddress::UserInfoPart* pUserInfoPart = m_pAor->GetUserInfoPart();

        if (pUserInfoPart == IMS_NULL)
        {
            m_objContactAddress.SetUser(m_pAor->GetUser());
        }
        else
        {
            m_objContactAddress.SetUser(pUserInfoPart->GetUser());
        }
    }
}

PUBLIC
void RegContact::SetTerminated()
{
    m_bBindingsUpdateTracker = IMS_FALSE;

    SetState(STATE_TERMINATED);
}

PUBLIC
AString RegContact::ToString() const
{
    AStringBuffer objStringBuffer(256);

    FormContact(IMS_FALSE, objStringBuffer);

    return static_cast<const AStringBuffer&>(objStringBuffer).GetString();
}

PUBLIC
AString RegContact::ToStringWithExpires() const
{
    AStringBuffer objStringBuffer(256);

    FormContact(IMS_TRUE, objStringBuffer);

    return static_cast<const AStringBuffer&>(objStringBuffer).GetString();
}

PUBLIC
IMS_SINT32 RegContact::UpdateParameter(IN IMS_SINT32 nExpiresValue)
{
    m_bBindingsUpdateTracker = IMS_FALSE;

    if (nExpiresValue > 0)
    {
        m_nNetworkProvisionedExpires = nExpiresValue;
        SetState(STATE_ACTIVE);
    }
    else
    {
        SetTerminated();
        return UPDATE_NO_EXPIRES;
    }

    return UPDATE_OK;
}

PUBLIC
IMS_SINT32 RegContact::UpdateParameter(
        IN const ImsList<ISipHeader*>& objContactHeaders, IN IMS_SINT32 nExpiresValue)
{
    m_bBindingsUpdateTracker = IMS_FALSE;

    for (IMS_UINT32 i = 0; i < objContactHeaders.GetSize(); ++i)
    {
        const ISipHeader* piHeader = objContactHeaders.GetAt(i);
        const SipAddress* pAddress = piHeader->GetSipAddress();

        if (pAddress == IMS_NULL)
        {
            continue;
        }

        // Find the matched contact address
        IMS_BOOL bSameContact = pAddress->Equals(m_objContactAddress);

        // If "transport" parameter is included in Contact header and it needs to be
        // ignored when comparing reg-binding, URI comparison should be done
        // without "transport" parameter.
        if (!bSameContact && SipFeatures::IsTransportParameterIgnoredForRegBinding(GetSlotId()))
        {
            const AString strTransport("transport");

            if ((pAddress->GetParameter(strTransport) != IMS_NULL) &&
                    (m_objContactAddress.GetParameter(strTransport) == IMS_NULL))
            {
                SipAddress objAddress(*pAddress);
                objAddress.RemoveParameter(strTransport);

                bSameContact = objAddress.Equals(m_objContactAddress);

                if (bSameContact)
                {
                    IMS_TRACE_D("LV :: Contact is same w/o \"transport\" parameter", 0, 0, 0);
                }
            }
        }

        if (bSameContact)
        {
            UpdateRegisteredCapabilities(piHeader);
            UpdateGruu(piHeader);

            // Extract 'expires' parameter
            const SipParameter* pParameter = piHeader->GetParameter(Sip::STR_EXPIRES);

            if (pParameter != IMS_NULL)
            {
                IMS_BOOL bOk = IMS_FALSE;

                m_nNetworkProvisionedExpires = pParameter->GetValue().ToInt32(&bOk);

                if (bOk)
                {
                    if (m_nNetworkProvisionedExpires > 0)
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
                        m_nNetworkProvisionedExpires = nExpiresValue;

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
                    m_nNetworkProvisionedExpires = nExpiresValue;
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
                    SipDebug::GetUri2(m_objContactAddress.GetUri()).GetStr(), 0);
        }
    }

    // If any matched address does not exist, we consider that this contact is terminated.
    SetTerminated();

    return UPDATE_OK;
}

PRIVATE VIRTUAL IMS_BOOL RegContact::AddHeaderParameter(
        IN const AString& strName, IN const AString& strValue /*= AString::ConstNull()*/)
{
    if (strName.StartsWith('+') && strName.EqualsIgnoreCase(Sip::STR_SIP_INSTANCE))
    {
        if (m_pInstanceParameter != IMS_NULL)
        {
            m_pInstanceParameter->SetValue(strValue);
        }
        else
        {
            m_pInstanceParameter = new SipParameter(strName, strValue);

            if (m_pInstanceParameter == IMS_NULL)
            {
                return IMS_FALSE;
            }
        }

        return IMS_TRUE;
    }

    if (strName.EqualsIgnoreCase(Sip::STR_REG_ID))
    {
        if (m_pRegIdParameter != IMS_NULL)
        {
            m_pRegIdParameter->SetValue(strValue);
        }
        else
        {
            m_pRegIdParameter = new SipParameter(strName, strValue);

            if (m_pRegIdParameter == IMS_NULL)
            {
                return IMS_FALSE;
            }
        }

        return IMS_TRUE;
    }

    for (IMS_UINT32 i = 0; i < m_objHeaderParams.GetSize(); ++i)
    {
        SipParameter* pParameter = m_objHeaderParams.GetAt(i);

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

    if (!m_objHeaderParams.Append(pParameter))
    {
        delete pParameter;
        return IMS_FALSE;
    }

    return IMS_TRUE;
}

PRIVATE VIRTUAL IMS_BOOL RegContact::AddUriParameter(
        IN const AString& strName, IN const AString& strValue /*= AString::ConstNull()*/)
{
    if (m_objContactAddress.AddParameter(strName, strValue) != IMS_SUCCESS)
    {
        IMS_TRACE_E(0, "Adding uri parameter in Contact (%s)",
                SipDebug::GetUri1(m_objContactAddress.ToString()).GetStr(), 0, 0);
        return IMS_FALSE;
    }

    return IMS_TRUE;
}

PRIVATE VIRTUAL void RegContact::RemoveAllHeaderParameters()
{
    if (!m_objHeaderParams.IsEmpty())
    {
        for (IMS_UINT32 i = 0; i < m_objHeaderParams.GetSize(); ++i)
        {
            SipParameter* pParameter = m_objHeaderParams.GetAt(i);

            if (pParameter != IMS_NULL)
            {
                delete pParameter;
            }
        }

        m_objHeaderParams.Clear();
    }
}

PRIVATE VIRTUAL void RegContact::RemoveHeaderParameter(
        IN const AString& strName, IN const AString& strValue /*= AString::ConstNull()*/)
{
    if (strName.StartsWith('+') && strName.EqualsIgnoreCase(Sip::STR_SIP_INSTANCE))
    {
        if (m_pInstanceParameter != IMS_NULL)
        {
            delete m_pInstanceParameter;
            m_pInstanceParameter = IMS_NULL;
        }

        return;
    }

    if (strName.EqualsIgnoreCase(Sip::STR_REG_ID))
    {
        if (m_pRegIdParameter != IMS_NULL)
        {
            delete m_pRegIdParameter;
            m_pRegIdParameter = IMS_NULL;
        }

        return;
    }

    for (IMS_UINT32 i = 0; i < m_objHeaderParams.GetSize(); ++i)
    {
        SipParameter* pParameter = m_objHeaderParams.GetAt(i);

        if (strName.EqualsIgnoreCase(pParameter->GetName()))
        {
            if (!strValue.IsNULL())
            {
                pParameter->RemoveValue(strValue);

                if (pParameter->GetValues().IsEmpty())
                {
                    m_objHeaderParams.RemoveAt(i);
                    delete pParameter;
                    return;
                }
            }
            else
            {
                m_objHeaderParams.RemoveAt(i);
                delete pParameter;
                return;
            }
        }
    }
}

PRIVATE VIRTUAL void RegContact::RemoveUriParameter(
        IN const AString& strName, IN const AString& strValue /*= AString::ConstNull()*/)
{
    (void)strValue;

    m_objContactAddress.RemoveParameter(strName);
}

PRIVATE VIRTUAL void RegContact::SetUserInfo(IN IMS_SINT32 nPolicy /*= POLICY_USER_INFO_IMPU*/,
        IN const AString& strUserInfo /*= AString::ConstNull()*/)
{
    m_nPolicyUserInfo = nPolicy;

    if (m_nPolicyUserInfo == POLICY_USER_INFO_APP)
    {
        m_objContactAddress.SetUser(strUserInfo);
    }
    else if (m_nPolicyUserInfo == POLICY_USER_INFO_IMPU)
    {
        if (m_pAor != IMS_NULL)
        {
            m_objContactAddress.SetUser(m_pAor->GetUser());
        }
    }
    else
    {
        m_objContactAddress.SetUser(AString::ConstNull());
    }
}

PRIVATE VIRTUAL IMS_BOOL RegContact::AddExtraCapability(
        IN const AString& strName, IN const AString& strValue)
{
    if (m_pExtraCapabilities == IMS_NULL)
    {
        return IMS_FALSE;
    }

    FeatureSet objFeatureSet(strName, strValue);

    if (strValue.GetLength() == 0)
    {
        objFeatureSet.Add(strName);
    }

    m_pExtraCapabilities->AddFeature(&objFeatureSet);

    return IMS_TRUE;
}

PRIVATE VIRTUAL void RegContact::RemoveExtraCapability(
        IN const AString& strName, IN const AString& strValue)
{
    if (m_pExtraCapabilities == IMS_NULL)
    {
        return;
    }

    if (strName.IsNULL() && strValue.IsNULL())
    {
        m_pExtraCapabilities->Clear();
        return;
    }

    FeatureSet objFeatureSet(strName, strValue);

    if (strValue.GetLength() == 0)
    {
        objFeatureSet.Add(strName);
    }

    m_pExtraCapabilities->RemoveFeature(&objFeatureSet);
}

PRIVATE VIRTUAL IMS_BOOL RegContact::AddService(
        IN const AString& strAppId, IN const AString& strServiceId)
{
    CallerCapability* pCc = IMS_NULL;
    RegBindingProxy::QueryCapability(GetSlotId(), strAppId, strServiceId, pCc);

    if (pCc == IMS_NULL)
    {
        IMS_TRACE_D("No caller capabilities (%s, %s)", strAppId.GetStr(), strServiceId.GetStr(), 0);
        return IMS_FALSE;
    }

    if (!RegBindingProxy::BindContact(GetSlotId(), strAppId, strServiceId, this))
    {
        IMS_TRACE_D("Binding Contact failed (%s, %s)", strAppId.GetStr(), strServiceId.GetStr(), 0);
        return IMS_FALSE;
    }

    AddCallerCapability(pCc);

    if (m_piCapabilityChangeListener != IMS_NULL)
    {
        m_piCapabilityChangeListener->RegCapabilityChange_ServiceAdded(strAppId, strServiceId);
    }

    return IMS_TRUE;
}

PRIVATE VIRTUAL void RegContact::RemoveService(
        IN const AString& strAppId, IN const AString& strServiceId)
{
    CallerCapability* pCc = IMS_NULL;
    RegBindingProxy::QueryCapability(GetSlotId(), strAppId, strServiceId, pCc);

    if (pCc == IMS_NULL)
    {
        IMS_TRACE_D("No caller capabilities (%s, %s)", strAppId.GetStr(), strServiceId.GetStr(), 0);
    }

    RegBindingProxy::UnbindContact(GetSlotId(), strAppId, strServiceId);

    RemoveCallerCapability(pCc);

    if (m_piCapabilityChangeListener != IMS_NULL)
    {
        m_piCapabilityChangeListener->RegCapabilityChange_ServiceRemoved(strAppId, strServiceId);
    }
}

PRIVATE VIRTUAL IMS_BOOL RegContact::IsServiceRegistered(
        IN const AString& strAppId, IN const AString& strServiceId) const
{
    CallerCapability* pCc = IMS_NULL;
    RegBindingProxy::QueryCapability(GetSlotId(), strAppId, strServiceId, pCc);

    if (pCc == IMS_NULL)
    {
        return IMS_FALSE;
    }

    if (!IsActiveBinding())
    {
        return IMS_FALSE;
    }

    if (m_pRegisteredCapabilities == IMS_NULL)
    {
        return IMS_TRUE;
    }

    if (m_pRegisteredCapabilities->IsEmpty())
    {
        return IMS_TRUE;
    }

    const IAppConfig* piAppConfig = Engine::GetConfiguration()->GetAppConfig(strAppId, GetSlotId());
    const ICoreServiceConfig* piServiceConfig =
            (piAppConfig != IMS_NULL) ? piAppConfig->GetCoreServiceConfig(strServiceId) : IMS_NULL;

    if (piServiceConfig == IMS_NULL)
    {
        return IMS_TRUE;
    }

    RcPtr<Feature> pFeature = IMS_NULL;

    if (piServiceConfig->IsIariSupported())
    {
        const ServiceIdentifier& objIari = piServiceConfig->GetIari();
        pFeature = new Feature(Feature::OTHER_G_3GPP_IARI_REF, objIari.GetName());

        if (!m_pRegisteredCapabilities->HasFeature(pFeature.Get()))
        {
            return IMS_FALSE;
        }

        pFeature = IMS_NULL;
    }

    const ImsList<ServiceIdentifier>& objIcsis = piServiceConfig->GetIcsis();

    for (IMS_UINT32 i = 0; i < objIcsis.GetSize(); i++)
    {
        const ServiceIdentifier& objIcsi = objIcsis.GetAt(i);
        pFeature = new Feature(Feature::OTHER_G_3GPP_ICSI_REF, objIcsi.GetName());

        if (!m_pRegisteredCapabilities->HasFeature(pFeature.Get()))
        {
            return IMS_FALSE;
        }

        pFeature = IMS_NULL;
    }

    const ImsList<ServiceIdentifier>& objFeatureTags = piServiceConfig->GetFeatureTags();

    for (IMS_UINT32 i = 0; i < objFeatureTags.GetSize(); i++)
    {
        const ServiceIdentifier& objFeatureTag = objFeatureTags.GetAt(i);
        pFeature = new Feature(objFeatureTag.GetName());

        if (!m_pRegisteredCapabilities->HasFeature(pFeature.Get()))
        {
            return IMS_FALSE;
        }

        pFeature = IMS_NULL;
    }

    IMS_TRACE_D("ServiceRegistered :: [%s-%s]", strAppId.GetStr(), strServiceId.GetStr(), 0);

    return IMS_TRUE;
}

PRIVATE VIRTUAL IMS_BOOL RegContact::IsFeatureRegistered(
        IN const AString& strFtName, IN const AString& strFtValue /*= AString::ConstNull()*/) const
{
    if (!IsActiveBinding())
    {
        return IMS_FALSE;
    }

    if (strFtName.GetLength() == 0)
    {
        return IMS_TRUE;
    }

    if (m_pRegisteredCapabilities == IMS_NULL)
    {
        return IMS_TRUE;
    }

    if (m_pRegisteredCapabilities->IsEmpty())
    {
        return IMS_TRUE;
    }

    RcPtr<Feature> pFeature = new Feature(strFtName, strFtValue);

    if (!m_pRegisteredCapabilities->HasFeature(pFeature.Get()))
    {
        return IMS_FALSE;
    }

    if (strFtValue.GetLength() == 0)
    {
        IMS_TRACE_D("FeatureRegistered :: [%s]", strFtName.GetStr(), 0, 0);
    }
    else
    {
        IMS_TRACE_D("FeatureRegistered :: [%s=%s]", strFtName.GetStr(), strFtValue.GetStr(), 0);
    }

    return IMS_TRUE;
}

PRIVATE VIRTUAL void RegContact::RecalculateCallerCapabilities()
{
    m_pAllCapabilities->Clear();

    for (IMS_UINT32 i = 0; i < m_objCallerCapabilities.GetSize(); ++i)
    {
        m_pAllCapabilities->AddFeatures(m_objCallerCapabilities.GetAt(i));
    }
}

PRIVATE
void RegContact::FormContact(IN IMS_BOOL bExpiresRequired, OUT AStringBuffer& objSb) const
{
    objSb.Append(m_objContactAddress.ToString());

    if (m_bAllCapabilitiesByConfig)
    {
        if (!m_pAllCapabilities->IsEmpty() && m_pExtraCapabilities->IsEmpty())
        {
            objSb.Append(TextParser::CHAR_SEMICOLON);
            objSb.Append(m_pAllCapabilities->ToString());
        }
        else if (m_pAllCapabilities->IsEmpty() && !m_pExtraCapabilities->IsEmpty())
        {
            objSb.Append(TextParser::CHAR_SEMICOLON);
            objSb.Append(m_pExtraCapabilities->ToString());
        }
        else if (!m_pAllCapabilities->IsEmpty() && !m_pExtraCapabilities->IsEmpty())
        {
            CallerCapability* pCc = new CallerCapability(0);

            pCc->AddFeatures(m_pAllCapabilities);
            pCc->AddFeatures(m_pExtraCapabilities);

            objSb.Append(TextParser::CHAR_SEMICOLON);
            objSb.Append(pCc->ToString());

            delete pCc;
        }
    }
    else
    {
        if (!m_pExtraCapabilities->IsEmpty())
        {
            objSb.Append(TextParser::CHAR_SEMICOLON);
            objSb.Append(m_pExtraCapabilities->ToString());
        }
    }

    // Adds a 'expires' parameter
    if (bExpiresRequired)
    {
        if (m_nInitialExpires != EXPIRES_NOT_SPECIFIED)
        {
            AString strExpires;
            strExpires.Sprintf("%lu", m_nInitialExpires);

            objSb.Append(TextParser::CHAR_SEMICOLON);
            objSb.Append(Sip::STR_EXPIRES);
            objSb.Append(TextParser::CHAR_EQUAL);
            objSb.Append(strExpires);
        }
    }

    if (!m_objHeaderParams.IsEmpty())
    {
        for (IMS_UINT32 i = 0; i < m_objHeaderParams.GetSize(); ++i)
        {
            const SipParameter* pParameter = m_objHeaderParams.GetAt(i);

            if (pParameter == IMS_NULL)
                continue;

            objSb.Append(TextParser::CHAR_SEMICOLON);
            objSb.Append(pParameter->ToString());
        }
    }

    if (m_pInstanceParameter != IMS_NULL)
    {
        objSb.Append(TextParser::CHAR_SEMICOLON);
        objSb.Append(m_pInstanceParameter->ToString());
    }

    if (m_pRegIdParameter != IMS_NULL)
    {
        objSb.Append(TextParser::CHAR_SEMICOLON);
        objSb.Append(m_pRegIdParameter->ToString());
    }
}

PRIVATE
IMS_BOOL RegContact::AddCallerCapability(IN const CallerCapability* pCc)
{
    if (m_pAllCapabilities == IMS_NULL)
    {
        return IMS_FALSE;
    }

    if (!RegisterServiceCapability(pCc))
    {
        return IMS_FALSE;
    }

    IMS_BOOL bIsChanged = m_pAllCapabilities->AddFeatures(pCc);

    if (!m_bBindingsUpdateTracker)
    {
        m_bBindingsUpdateTracker = bIsChanged;
    }

    return IMS_TRUE;
}

PRIVATE
IMS_BOOL RegContact::RemoveCallerCapability(IN const CallerCapability* pCc)
{
    if (m_pAllCapabilities == IMS_NULL)
    {
        return IMS_FALSE;
    }

    UnregisterServiceCapability(pCc);

    IMS_BOOL bIsChanged = m_pAllCapabilities->RemoveFeatures(pCc);

    if (!m_bBindingsUpdateTracker)
    {
        m_bBindingsUpdateTracker = bIsChanged;
    }

    return IMS_TRUE;
}

PRIVATE
IMS_BOOL RegContact::RegisterServiceCapability(IN const CallerCapability* pCc)
{
    // First, checks if the service is already bound or not
    for (IMS_UINT32 i = 0; i < m_objCallerCapabilities.GetSize(); ++i)
    {
        const CallerCapability* pTmpCc = m_objCallerCapabilities.GetAt(i);

        if (pTmpCc->Equals(pCc))
        {
            return IMS_TRUE;
        }
    }

    // If not present, add a new service capability
    return m_objCallerCapabilities.Append(const_cast<CallerCapability*>(pCc));
}

PRIVATE
void RegContact::UnregisterServiceCapability(IN const CallerCapability* pCc)
{
    // First, checks if the service is already bound or not
    for (IMS_UINT32 i = 0; i < m_objCallerCapabilities.GetSize(); ++i)
    {
        const CallerCapability* pTmpCc = m_objCallerCapabilities.GetAt(i);

        if (pTmpCc->Equals(pCc))
        {
            m_objCallerCapabilities.RemoveAt(i);
        }
    }
}

PRIVATE
void RegContact::SetState(IN IMS_SINT32 nState)
{
    IMS_TRACE_I("RegContact (%s) :: %s to %s",
            SipDebug::GetUri1(m_objContactAddress.GetUri()).GetStr(), StateToString(m_nState),
            StateToString(nState));

    m_nState = nState;
}

PRIVATE
void RegContact::UpdateGruu(IN const ISipHeader* piHeader)
{
    if (piHeader == IMS_NULL)
    {
        return;
    }

    // Destroys all the previous GRUU identities
    // DestroyGruu();

    // 'pub-gruu' parameter
    const SipParameter* pParameter = piHeader->GetParameter("pub-gruu");

    if (pParameter != IMS_NULL)
    {
        // Destroy the previous public GRUU
        if (m_pPubGruu != IMS_NULL)
        {
            delete m_pPubGruu;
            m_pPubGruu = IMS_NULL;
        }

        const AString& strValue = pParameter->GetValue();

        IMS_TRACE_D("pub-gruu (%s)", SipDebug::GetUri1(strValue).GetStr(), 0, 0);

        if (strValue.StartsWith(TextParser::CHAR_DQUOT))
        {
            m_pPubGruu = new SipAddress(strValue.GetSubStr(1, strValue.GetLength() - 2));
        }
        else
        {
            m_pPubGruu = new SipAddress(strValue);
        }

        if (m_pPubGruu == IMS_NULL)
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
        if (m_pTempGruu != IMS_NULL)
        {
            delete m_pTempGruu;
            m_pTempGruu = IMS_NULL;
        }

        if (!m_objTempGruus.IsEmpty())
        {
            for (IMS_UINT32 i = 0; i < m_objTempGruus.GetSize(); ++i)
            {
                SipAddress* pAddress = m_objTempGruus.GetAt(i);

                if (pAddress != IMS_NULL)
                {
                    delete pAddress;
                }
            }

            m_objTempGruus.Clear();
        }

        const AString& strValue = pParameter->GetValue();

        IMS_TRACE_D("temp-gruu (%s)", SipDebug::GetUri1(strValue).GetStr(), 0, 0);

        if (strValue.StartsWith(TextParser::CHAR_DQUOT))
        {
            m_pTempGruu = new SipAddress(strValue.GetSubStr(1, strValue.GetLength() - 2));
        }
        else
        {
            m_pTempGruu = new SipAddress(strValue);
        }

        if (m_pTempGruu == IMS_NULL)
        {
            IMS_TRACE_E(0, "Creating a temp-gruu (%s) failed", SipDebug::GetUri1(strValue).GetStr(),
                    0, 0);
        }
        else
        {
            m_objTempGruus.Append(new SipAddress(*m_pTempGruu));
        }
    }
}

PRIVATE
void RegContact::UpdateRegisteredCapabilities(IN const ISipHeader* piHeader)
{
    if (m_pRegisteredCapabilities != IMS_NULL)
    {
        delete m_pRegisteredCapabilities;
        m_pRegisteredCapabilities = IMS_NULL;
    }

    const ImsList<SipParameter*>& objParameters = piHeader->GetParameters();

    if (objParameters.IsEmpty())
    {
        return;
    }

    CallerCapability* pCc = new CallerCapability(0);

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

        RcPtr<Feature> pFeature = IMS_NULL;

        if (pParameter->IsNameOnly())
        {
            pFeature = new Feature(strName, AString::ConstNull());
            pCc->AddFeature(pFeature.Get());
            continue;
        }

        const AStringArray& objValues = pParameter->GetValues();

        for (IMS_SINT32 j = 0; j < objValues.GetCount(); j++)
        {
            pFeature = new Feature(strName, objValues.GetElementAt(j));
            pCc->AddFeature(pFeature.Get());
        }
    }

    if (pCc->IsEmpty())
    {
        delete pCc;
        return;
    }

    m_pRegisteredCapabilities = pCc;

    IMS_TRACE_I(
            "RegisteredCapabilities=[%s]", m_pRegisteredCapabilities->ToString().GetStr(), 0, 0);
}

PRIVATE GLOBAL const IMS_CHAR* RegContact::StateToString(IN IMS_SINT32 nState)
{
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
