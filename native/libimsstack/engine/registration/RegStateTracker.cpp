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

#include "RegContact.h"
#include "RegStateTracker.h"
#include "SipConfigProxy.h"

__IMS_TRACE_TAG_REG__;

PUBLIC
RegStateTracker::RegStateTracker() :
        RcObject(),
        m_strSubsId(AString::ConstNull()),
        m_pAuthorizedAor(IMS_NULL),
        m_objIpAddress(IpAddress::NONE),
        m_objPublicIpAddress(IpAddress::IPv6NONE),
        m_pContactAddressForOutgoingMessage(IMS_NULL),
        m_pPreferredContact(IMS_NULL),
        m_nTransportExt(0),
        m_nPortFlowControl(0),
        m_nPortUc(0),
        m_nPortUs(0),
        m_pSipProfile(IMS_NULL)
{
}

PUBLIC
RegStateTracker::RegStateTracker(IN const RegStateTracker& other) :
        RcObject(other),
        m_strSubsId(other.m_strSubsId),
        m_pAuthorizedAor(IMS_NULL),
        m_objIpAddress(other.m_objIpAddress),
        m_objPublicIpAddress(other.m_objPublicIpAddress),
        m_pContactAddressForOutgoingMessage(IMS_NULL),
        m_pPreferredContact(other.m_pPreferredContact),
        m_nTransportExt(other.m_nTransportExt),
        m_nPortFlowControl(other.m_nPortFlowControl),
        m_nPortUc(other.m_nPortUc),
        m_nPortUs(other.m_nPortUs),
        m_pSipProfile(other.m_pSipProfile)
{
    if (other.m_pAuthorizedAor != IMS_NULL)
    {
        m_pAuthorizedAor = new SipAddress(*(other.m_pAuthorizedAor));
    }

    if (other.m_pContactAddressForOutgoingMessage != IMS_NULL)
    {
        m_pContactAddressForOutgoingMessage =
                new SipAddress(*(other.m_pContactAddressForOutgoingMessage));
    }
}

PUBLIC VIRTUAL RegStateTracker::~RegStateTracker()
{
    if (m_pAuthorizedAor != IMS_NULL)
    {
        delete m_pAuthorizedAor;
    }

    if (m_pContactAddressForOutgoingMessage != IMS_NULL)
    {
        delete m_pContactAddressForOutgoingMessage;
    }

    IMS_TRACE_D("Destructor :: RegStateTracker", 0, 0, 0);
}

PUBLIC
const SipAddress& RegStateTracker::GetAuthorizedAor() const
{
    if (m_pAuthorizedAor == IMS_NULL)
    {
        return m_objAor;
    }

    return (*m_pAuthorizedAor);
}

PUBLIC
const SipAddress& RegStateTracker::GetContactAddress() const
{
    if (m_pPreferredContact != IMS_NULL)
    {
        return m_pPreferredContact->GetContactAddress();
    }

    return m_objPreferredContactAddress;
}

PUBLIC
IMS_BOOL RegStateTracker::IsWithinTrustDomain(IN IMS_SINT32 nSlotId) const
{
    return SipConfigProxy::IsTrustDomainConfigured(nSlotId, GetSipProfile());
}

PRIVATE
void RegStateTracker::SetAssociatedUris(IN const AStringArray& objAssociatedUris)
{
    m_objAssociatedUris = objAssociatedUris;

    // The topmost user identity is an authorized & registered explicitly by the network
    if (m_objAssociatedUris.IsEmpty())
    {
        if (m_pAuthorizedAor != IMS_NULL)
        {
            delete m_pAuthorizedAor;
            m_pAuthorizedAor = IMS_NULL;
        }
    }
    else
    {
        const AString& strImpu = m_objAssociatedUris.GetFirstElement();

        if (m_pAuthorizedAor != IMS_NULL)
        {
            delete m_pAuthorizedAor;
            m_pAuthorizedAor = IMS_NULL;
        }

        m_pAuthorizedAor = new SipAddress();

        if (m_pAuthorizedAor != IMS_NULL)
        {
            if (!m_pAuthorizedAor->Create(strImpu))
            {
                IMS_TRACE_E(0, "Creating an authorized AOR failed", 0, 0, 0);
            }
        }
        else
        {
            IMS_TRACE_E(0, "Creating an authorized AOR failed", 0, 0, 0);
        }
    }
}

PRIVATE
void RegStateTracker::SetPreferredContact(IN RegContact* pContact)
{
    m_pPreferredContact = pContact;

    if (m_pPreferredContact != IMS_NULL)
    {
        m_objIpAddress = m_pPreferredContact->GetIpAddress();
        m_objPreferredContactAddress = m_pPreferredContact->GetContactAddress();
    }
}

PRIVATE
void RegStateTracker::SetSecurityClients(IN const ImsList<SipSecurityHeader>& objClients)
{
    m_objSecurityClients.RemoveAllElements();

    for (IMS_UINT32 i = 0; i < objClients.GetSize(); ++i)
    {
        const SipSecurityHeader& objHeader = objClients.GetAt(i);

        m_objSecurityClients.AddElement(objHeader.ToString());
    }
}

PRIVATE
void RegStateTracker::SetSecurityVerifys(IN const ImsList<SipSecurityHeader>& objVerifys)
{
    m_objSecurityVerifys.RemoveAllElements();

    for (IMS_UINT32 i = 0; i < objVerifys.GetSize(); ++i)
    {
        const SipSecurityHeader& objHeader = objVerifys.GetAt(i);

        m_objSecurityVerifys.AddElement(objHeader.ToString());
    }
}

PRIVATE
void RegStateTracker::SetUserInfoForContactHeader(IN const AString& strUserInfo)
{
    if (m_pContactAddressForOutgoingMessage != IMS_NULL)
    {
        delete m_pContactAddressForOutgoingMessage;
        m_pContactAddressForOutgoingMessage = IMS_NULL;
    }

    if (strUserInfo.IsNull())
    {
        return;
    }

    m_pContactAddressForOutgoingMessage = new SipAddress(GetContactAddress());

    if (m_pContactAddressForOutgoingMessage != IMS_NULL)
    {
        m_pContactAddressForOutgoingMessage->SetUser(
                strUserInfo.IsEmpty() ? AString::ConstNull() : strUserInfo);
    }
}
