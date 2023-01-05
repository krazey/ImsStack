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
#include "NatHelper.h"
#include "ServiceMemory.h"
#include "SystemConfig.h"

PRIVATE
NatHelper::NatHelper() :
        m_ppBindings(IMS_NULL)
{
    IMS_SINT32 nSimCount = SystemConfig::GetSupportedSimCount();

    m_ppBindings = new ImsList<IpBinding>*[nSimCount];

    for (IMS_SINT32 i = 0; i < nSimCount; i++)
    {
        m_ppBindings[i] = new ImsList<IpBinding>();
    }
}

PRIVATE
NatHelper::~NatHelper()
{
    if (m_ppBindings != IMS_NULL)
    {
        IMS_SINT32 nSimCount = SystemConfig::GetSupportedSimCount();

        for (IMS_SINT32 i = 0; i < nSimCount; ++i)
        {
            if (m_ppBindings[i] != IMS_NULL)
            {
                delete m_ppBindings[i];
            }
        }

        delete[] m_ppBindings;
    }
}

PUBLIC
void NatHelper::Clear(IN IMS_SINT32 nSlotId)
{
    ImsList<IpBinding>* pBindings = GetIpBindings(nSlotId);

    if (pBindings != IMS_NULL)
    {
        pBindings->Clear();
    }
}

PUBLIC
IpAddress NatHelper::GetPrivateAddress(IN IMS_SINT32 nSlotId, IN const IpAddress& objPublicIp) const
{
    ImsList<IpBinding>* pBindings = GetIpBindings(nSlotId);

    if (pBindings == IMS_NULL)
    {
        return IpAddress::NONE;
    }

    for (IMS_UINT32 i = 0; i < pBindings->GetSize(); ++i)
    {
        const IpBinding& objIpBinding = pBindings->GetAt(i);

        if (objPublicIp.Equals(objIpBinding.GetPublicIp()))
        {
            return objIpBinding.GetPrivateIp();
        }
    }

    return IpAddress::NONE;
}

PUBLIC
IpAddress NatHelper::GetPublicAddress(IN IMS_SINT32 nSlotId, IN const IpAddress& objPrivateIp) const
{
    ImsList<IpBinding>* pBindings = GetIpBindings(nSlotId);

    if (pBindings == IMS_NULL)
    {
        return IpAddress::NONE;
    }

    for (IMS_UINT32 i = 0; i < pBindings->GetSize(); ++i)
    {
        const IpBinding& objIpBinding = pBindings->GetAt(i);

        if (objPrivateIp.Equals(objIpBinding.GetPrivateIp()))
        {
            return objIpBinding.GetPublicIp();
        }
    }

    return IpAddress::NONE;
}

PUBLIC
IMS_BOOL NatHelper::IsBehindNat(
        IN IMS_SINT32 nSlotId, IN const IpAddress& objPrivateIp /*= IpAddress::NONE*/) const
{
    ImsList<IpBinding>* pBindings = GetIpBindings(nSlotId);

    if (pBindings == IMS_NULL)
    {
        return IMS_FALSE;
    }

    for (IMS_UINT32 i = 0; i < pBindings->GetSize(); ++i)
    {
        const IpBinding& objIpBinding = pBindings->GetAt(i);
        const IpAddress& objPublicIp = objIpBinding.GetPublicIp();

        if (!objPrivateIp.Equals(IpAddress::NONE) &&
                !objPrivateIp.Equals(objIpBinding.GetPrivateIp()))
        {
            continue;
        }

        if (!objPublicIp.Equals(IpAddress::NONE) && !objPublicIp.Equals(IpAddress::IPv6NONE))
        {
            return IMS_TRUE;
        }
    }

    return IMS_FALSE;
}

PUBLIC
void NatHelper::RemovePublicAddress(IN IMS_SINT32 nSlotId, IN IMS_SINT32 nId)
{
    ImsList<IpBinding>* pBindings = GetIpBindings(nSlotId);

    if (pBindings == IMS_NULL)
    {
        return;
    }

    for (IMS_UINT32 i = 0; i < pBindings->GetSize();)
    {
        const IpBinding& objIpBinding = pBindings->GetAt(i);

        if (nId == objIpBinding.GetId())
        {
            pBindings->RemoveAt(i);
        }
        else
        {
            ++i;
        }
    }
}

PUBLIC
void NatHelper::RemovePublicAddress(IN IMS_SINT32 nSlotId, IN const IpAddress& objPrivateIp)
{
    ImsList<IpBinding>* pBindings = GetIpBindings(nSlotId);

    if (pBindings == IMS_NULL)
    {
        return;
    }

    for (IMS_UINT32 i = 0; i < pBindings->GetSize(); ++i)
    {
        const IpBinding& objIpBinding = pBindings->GetAt(i);

        if (objPrivateIp.Equals(objIpBinding.GetPrivateIp()))
        {
            pBindings->RemoveAt(i);
            break;
        }
    }
}

PUBLIC
void NatHelper::SetPublicAddress(IN IMS_SINT32 nSlotId, IN IMS_SINT32 nId,
        IN const IpAddress& objPrivateIp, IN const IpAddress& objPublicIp)
{
    RemoveIpBinding(nSlotId, nId, objPrivateIp);

    if (!objPublicIp.Equals(IpAddress::NONE) && !objPublicIp.Equals(IpAddress::IPv6NONE))
    {
        ImsList<IpBinding>* pBindings = GetIpBindings(nSlotId);

        if (pBindings != IMS_NULL)
        {
            pBindings->Append(IpBinding(nId, objPrivateIp, objPublicIp));
        }
    }
}

PUBLIC GLOBAL NatHelper* NatHelper::GetInstance()
{
    static NatHelper* s_pNatHelper = IMS_NULL;

    if (s_pNatHelper == IMS_NULL)
    {
        s_pNatHelper = new NatHelper();
    }

    return s_pNatHelper;
}

PUBLIC GLOBAL IMS_BOOL NatHelper::IsNatResolverRequired()
{
    // FIXME: SKT only requires, but it's not used in the moment.
    return IMS_FALSE;
}

PRIVATE
void NatHelper::RemoveIpBinding(
        IN IMS_SINT32 nSlotId, IN IMS_SINT32 nId, IN const IpAddress& objPrivateIp)
{
    ImsList<IpBinding>* pBindings = GetIpBindings(nSlotId);

    if (pBindings == IMS_NULL)
    {
        return;
    }

    for (IMS_UINT32 i = 0; i < pBindings->GetSize(); ++i)
    {
        const IpBinding& objIpBinding = pBindings->GetAt(i);

        if ((nId == objIpBinding.GetId()) && objPrivateIp.Equals(objIpBinding.GetPrivateIp()))
        {
            pBindings->RemoveAt(i);
            break;
        }
    }
}

PRIVATE
ImsList<NatHelper::IpBinding>* NatHelper::GetIpBindings(IN IMS_SINT32 nSlotId) const
{
    if ((nSlotId < IMS_SLOT_0) || (nSlotId >= SystemConfig::GetSupportedSimCount()))
    {
        nSlotId = IMS_SLOT_0;
    }

    return m_ppBindings[nSlotId];
}
