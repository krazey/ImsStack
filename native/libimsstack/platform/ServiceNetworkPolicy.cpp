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
#include "PlatformContext.h"
#include "ServiceMemory.h"
#include "ServiceNetworkPolicy.h"
#include "SystemConfig.h"

class NetworkPolicyHolder
{
public:
    NetworkPolicyHolder();
    ~NetworkPolicyHolder();

    NetworkPolicyHolder(IN const NetworkPolicyHolder&) = delete;
    NetworkPolicyHolder& operator=(IN const NetworkPolicyHolder&) = delete;

public:
    void InitPolicies();
    IMS_BOOL AddPolicy(IN const NetworkPolicy& objPolicy);
    const NetworkPolicy* GetPolicy(IN const AString& strName) const;
    const NetworkPolicy* GetPolicy(IN IMS_SINT32 nApnType) const;
    void RemovePolicy(IN const AString& strName);
    void RemoveAllPolicies();

private:
    IMSList<NetworkPolicy*> m_objPolicys;
};

PUBLIC
NetworkPolicyHolder::NetworkPolicyHolder() :
        m_objPolicys(IMSList<NetworkPolicy*>())
{
    InitPolicies();
}

PUBLIC
NetworkPolicyHolder::~NetworkPolicyHolder()
{
    while (!m_objPolicys.IsEmpty())
    {
        NetworkPolicy* pPolicy = m_objPolicys.GetAt(0);

        if (pPolicy != IMS_NULL)
        {
            delete pPolicy;
        }

        m_objPolicys.RemoveAt(0);
    }
}

PUBLIC
void NetworkPolicyHolder::InitPolicies()
{
    // "ims"
    NetworkPolicy* pPolicy = new NetworkPolicy(IMS_TRUE, "mobile_ims", NetworkPolicy::APN_IMS);
    m_objPolicys.Append(pPolicy);

    // "emergency"
    pPolicy = new NetworkPolicy(IMS_FALSE, "mobile_emergency", NetworkPolicy::APN_EMERGENCY);
    m_objPolicys.Append(pPolicy);

    // "internet"
    pPolicy = new NetworkPolicy(IMS_FALSE, "mobile_internet", NetworkPolicy::APN_INTERNET);
    m_objPolicys.Append(pPolicy);

    // "wifi"
    pPolicy = new NetworkPolicy(IMS_FALSE, "wifi", NetworkPolicy::APN_WIFI);
    m_objPolicys.Append(pPolicy);
}

PUBLIC
IMS_BOOL NetworkPolicyHolder::AddPolicy(IN const NetworkPolicy& objPolicy)
{
    NetworkPolicy* pNewPolicy = new NetworkPolicy(objPolicy);

    if (pNewPolicy == IMS_NULL)
    {
        return IMS_FALSE;
    }

    if (!m_objPolicys.Append(pNewPolicy))
    {
        delete pNewPolicy;

        return IMS_FALSE;
    }

    return IMS_TRUE;
}

PUBLIC
const NetworkPolicy* NetworkPolicyHolder::GetPolicy(IN const AString& strName) const
{
    for (IMS_UINT32 i = 0; i < m_objPolicys.GetSize(); ++i)
    {
        const NetworkPolicy* pPolicy = m_objPolicys.GetAt(i);

        if (strName.Equals(pPolicy->GetName()))
        {
            return pPolicy;
        }
    }

    return IMS_NULL;
}

PUBLIC
const NetworkPolicy* NetworkPolicyHolder::GetPolicy(IN IMS_SINT32 nApnType) const
{
    if (nApnType == NetworkPolicy::APN_NONE)
    {
        for (IMS_UINT32 i = 0; i < m_objPolicys.GetSize(); ++i)
        {
            const NetworkPolicy* pPolicy = m_objPolicys.GetAt(i);

            if (pPolicy->IsPrimary())
            {
                return pPolicy;
            }
        }
    }
    else
    {
        for (IMS_UINT32 i = 0; i < m_objPolicys.GetSize(); ++i)
        {
            const NetworkPolicy* pPolicy = m_objPolicys.GetAt(i);

            if (pPolicy->GetApnType() == nApnType)
            {
                return pPolicy;
            }
        }
    }

    return IMS_NULL;
}

PUBLIC
void NetworkPolicyHolder::RemovePolicy(IN const AString& strName)
{
    for (IMS_UINT32 i = 0; i < m_objPolicys.GetSize(); ++i)
    {
        NetworkPolicy* pPolicy = m_objPolicys.GetAt(i);

        if (strName.Equals(pPolicy->GetName()))
        {
            m_objPolicys.RemoveAt(i);
            delete pPolicy;
            return;
        }
    }
}

PUBLIC
void NetworkPolicyHolder::RemoveAllPolicies()
{
    for (IMS_UINT32 i = 0; i < m_objPolicys.GetSize(); ++i)
    {
        NetworkPolicy* pPolicy = m_objPolicys.GetAt(i);

        if (pPolicy != IMS_NULL)
        {
            delete pPolicy;
        }
    }

    m_objPolicys.Clear();
}

class NetworkServicePolicyPrivate
{
public:
    NetworkServicePolicyPrivate();
    ~NetworkServicePolicyPrivate();

    NetworkServicePolicyPrivate(IN const NetworkServicePolicyPrivate&) = delete;
    NetworkServicePolicyPrivate& operator=(IN const NetworkServicePolicyPrivate&) = delete;

public:
    inline NetworkPolicyHolder* GetHolder(IN IMS_SINT32 nSlotId) const
    {
        if ((nSlotId < IMS_SLOT_0) || (nSlotId >= SystemConfig::GetMaxSimSlot()))
        {
            nSlotId = IMS_SLOT_0;
        }

        return m_ppHolder[nSlotId];
    }

private:
    NetworkPolicyHolder** m_ppHolder;
};

PUBLIC
NetworkServicePolicyPrivate::NetworkServicePolicyPrivate() :
        m_ppHolder(IMS_NULL)
{
    IMS_SINT32 nSimCount = SystemConfig::GetMaxSimSlot();

    m_ppHolder = new NetworkPolicyHolder*[nSimCount];

    for (IMS_SINT32 i = 0; i < nSimCount; ++i)
    {
        m_ppHolder[i] = new NetworkPolicyHolder();
    }
}

PUBLIC
NetworkServicePolicyPrivate::~NetworkServicePolicyPrivate()
{
    if (m_ppHolder != IMS_NULL)
    {
        IMS_SINT32 nSimCount = SystemConfig::GetMaxSimSlot();

        for (IMS_SINT32 i = 0; i < nSimCount; ++i)
        {
            if (m_ppHolder[i] != IMS_NULL)
            {
                delete m_ppHolder[i];
            }
        }

        delete[] m_ppHolder;
    }
}

PUBLIC
NetworkPolicy::NetworkPolicy(IN IMS_BOOL bPrimary /* = IMS_FALSE */) :
        m_bPrimary(bPrimary),
        m_strName(AString::ConstEmpty()),
        m_nApnType(APN_NONE)
{
}

PUBLIC
NetworkPolicy::NetworkPolicy(
        IN IMS_BOOL bPrimary, IN const AString& strName, IN IMS_SINT32 nApnType) :
        m_bPrimary(bPrimary),
        m_strName(strName),
        m_nApnType(nApnType)
{
}

PUBLIC
NetworkPolicy::NetworkPolicy(IN const NetworkPolicy& other) :
        m_bPrimary(other.m_bPrimary),
        m_strName(other.m_strName),
        m_nApnType(other.m_nApnType)
{
}

PUBLIC
NetworkPolicy::~NetworkPolicy() {}

PUBLIC
NetworkPolicy& NetworkPolicy::operator=(IN const NetworkPolicy& other)
{
    if (this != &other)
    {
        m_bPrimary = other.m_bPrimary;
        m_strName = other.m_strName;
        m_nApnType = other.m_nApnType;
    }

    return (*this);
}

PUBLIC GLOBAL IMS_BOOL NetworkPolicy::IsMobilePolicy(IN const AString& strName)
{
    return strName.StartsWith("mobile");
}

PUBLIC GLOBAL IMS_BOOL NetworkPolicy::IsMobilePolicy(IN IMS_SINT32 nApnType)
{
    return (nApnType == APN_IMS) || (nApnType == APN_EMERGENCY) || (nApnType == APN_INTERNET);
}

PUBLIC GLOBAL IMS_BOOL NetworkPolicy::IsWiFiPolicy(IN const AString& strName)
{
    return strName.StartsWith("wifi");
}

PUBLIC GLOBAL IMS_BOOL NetworkPolicy::IsWiFiPolicy(IN IMS_SINT32 nApnType)
{
    return (nApnType == APN_WIFI);
}

PRIVATE
NetworkServicePolicy::NetworkServicePolicy() :
        m_pPrivate(new NetworkServicePolicyPrivate())
{
}

PRIVATE
NetworkServicePolicy::~NetworkServicePolicy()
{
    if (m_pPrivate != IMS_NULL)
    {
        delete m_pPrivate;
    }
}

PUBLIC
IMS_BOOL NetworkServicePolicy::AddPolicy(
        IN const AString& strName, IN const NetworkPolicy& objPolicy, IN IMS_SINT32 nSlotId)
{
    NetworkPolicyHolder* pHolder = m_pPrivate->GetHolder(nSlotId);

    if (pHolder->GetPolicy(strName) != IMS_NULL)
    {
        // It already exists
        return IMS_TRUE;
    }

    return pHolder->AddPolicy(objPolicy);
}

PUBLIC
const NetworkPolicy* NetworkServicePolicy::GetPolicy(
        IN const AString& strName, IN IMS_SINT32 nSlotId) const
{
    NetworkPolicyHolder* pHolder = m_pPrivate->GetHolder(nSlotId);
    return pHolder->GetPolicy(strName);
}

PUBLIC
const NetworkPolicy* NetworkServicePolicy::GetPolicy(
        IN IMS_SINT32 nApnType, IN IMS_SINT32 nSlotId) const
{
    NetworkPolicyHolder* pHolder = m_pPrivate->GetHolder(nSlotId);
    return pHolder->GetPolicy(nApnType);
}

PUBLIC
void NetworkServicePolicy::RemovePolicy(IN const AString& strName, IN IMS_SINT32 nSlotId)
{
    NetworkPolicyHolder* pHolder = m_pPrivate->GetHolder(nSlotId);
    pHolder->RemovePolicy(strName);
}

PUBLIC
void NetworkServicePolicy::RemoveAllPolicies(IN IMS_SINT32 nSlotId)
{
    NetworkPolicyHolder* pHolder = m_pPrivate->GetHolder(nSlotId);
    pHolder->RemoveAllPolicies();
}

PUBLIC GLOBAL NetworkServicePolicy* NetworkServicePolicy::GetInstance()
{
    return DYNAMIC_CAST(NetworkServicePolicy*,
            PlatformContext::GetInstance()->GetService(PlatformContext::SERVICE_NETWORK_POLICY));
}
