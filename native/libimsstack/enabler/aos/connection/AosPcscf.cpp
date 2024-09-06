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
#include "ServiceTrace.h"
#include "ServiceUtil.h"
#include "ServiceTimer.h"
#include "ServiceNetworkPolicy.h"

#include "IConfigurable.h"
#include "IConfiguration.h"
#include "CarrierConfig.h"
#include "Engine.h"
#include "ServerAddress.h"

#include "IRegistration.h"
#include "RegistrationManager.h"

#include "connection/AosPcscf.h"
#include "provider/AosProvider.h"
#include "provider/AosStaticProfile.h"
#include "provider/AosUtil.h"
#include "interface/IAosAppContext.h"
#include "interface/IAosConnection.h"
#include "interface/IAosNConfiguration.h"
#include "interface/IAosRegistration.h"
#include "interface/IAosSubscriber.h"

__IMS_TRACE_TAG_USER_DECL__("AOS");

#define APPPROFILE m_strTag.GetStr()

PUBLIC
AosPcscf::AosPcscf(IN IAosAppContext* piAppContext) :
        m_piAppContext(piAppContext),
        m_piListener(IMS_NULL),
        m_piDnsQueryRetryTimer(IMS_NULL),
        m_eRegType(AosRegistrationType::NORMAL),
        m_nSlotId(piAppContext->GetSlotId()),
        m_nChangedType(TYPE_CHANGED_DIFFERENT),
        m_bIsConfigured(IMS_FALSE),
        m_bIsDnsQueryRetry(IMS_FALSE),
        m_bOtherIpTypeRequired(IMS_FALSE),
        m_nCurrentPcscfIndex(0),
        m_nDiscoveryMethodIndex(0)
{
    m_strTag.Sprintf("%d:%s", m_nSlotId, m_piAppContext->GetProfileId().GetStr());

    IMS_TRACE_MEM("AOS_MEM", "AOS_M : [%s] AosPcscf = %" PFLS_u "/%" PFLS_x, APPPROFILE,
            sizeof(AosPcscf), this);
}

PUBLIC VIRTUAL AosPcscf::~AosPcscf()
{
    IMS_TRACE_MEM("AOS_MEM", "AOS_F : [%s] AosPcscf = %" PFLS_u "/%" PFLS_x, APPPROFILE,
            sizeof(AosPcscf), this);
}

PROTECTED VIRTUAL void AosPcscf::Init()
{
    A_IMS_TRACE_D(APPPROFILE, "Init", 0, 0, 0);

    m_eRegType = m_piAppContext->GetRegistration()->GetRegType();
}

PROTECTED VIRTUAL void AosPcscf::CleanUp()
{
    A_IMS_TRACE_D(APPPROFILE, "CleanUp", 0, 0, 0);

    m_piListener = IMS_NULL;

    CleanAll();
}

PUBLIC VIRTUAL void AosPcscf::Configure(IN IMS_UINT32 nIpVersion /* = IpAddress::UNKNOWN */)
{
    A_IMS_TRACE_I(APPPROFILE, "Configure :: nIpVersion(%d)", nIpVersion, 0, 0);

    CleanAll();

    if (IsLocalAddressTypeValid(nIpVersion))
    {
        if (!IsLocalAddressValid(nIpVersion))
        {
            A_IMS_TRACE_I(APPPROFILE, "Configure :: invalid explicit IP version", 0, 0, 0);
            return;
        }
        ProcessDiscovery(nIpVersion);
    }
    else
    {
        m_bOtherIpTypeRequired = IMS_TRUE;
        IMS_SINT32 nIpType = GetLocalIpVersion();
        ProcessDiscovery(nIpType);

        if (!IsConfigured() && (m_piDnsQueryRetryTimer == IMS_NULL))
        {
            IMS_SINT32 nOtherIpType =
                    (nIpType == IpAddress::IPV6) ? IpAddress::IPV4 : IpAddress::IPV6;
            if (IsLocalAddressValid(nOtherIpType))
            {
                ClearDiscoveryContents();
                ProcessDiscovery(nOtherIpType);
            }
        }
    }
}

PUBLIC VIRTUAL IMS_BOOL AosPcscf::IsConfigured() const
{
    A_IMS_TRACE_I(APPPROFILE, "IsConfigured :: (%s)", _TRACE_B_(m_bIsConfigured), 0, 0);
    return m_bIsConfigured;
}

PUBLIC VIRTUAL IMS_BOOL AosPcscf::IsAsyncDnsDiscovery() const
{
    return (m_eRegType == AosRegistrationType::RCS);
}

PUBLIC VIRTUAL IMS_BOOL AosPcscf::IsSinglePcoScheme()
{
    const ISubscriberConfig* piSubsConfig =
            GetSubscriberConfig((IsFakeDiscoverySchemes()) ? IAosSubscriber::FAKE : -1);

    if (piSubsConfig == IMS_NULL)
    {
        A_IMS_TRACE_D(APPPROFILE, "piSubsConfig is null", 0, 0, 0);
        return IMS_FALSE;
    }

    const ImsVector<IMS_SINT32>& objMethods = piSubsConfig->GetPcscfDiscoveryMethods();

    if (objMethods.IsEmpty())
    {
        return IMS_FALSE;
    }

    if (objMethods.GetSize() == 1)
    {
        if (objMethods.GetAt(0) == ISubscriberConfig::PCSCF_DISCOVERY_METHOD_PCO)
        {
            return IMS_TRUE;
        }
    }

    return IMS_FALSE;
}

PUBLIC VIRTUAL const AStringArray& AosPcscf::GetPcscfs()
{
    m_objCurrAddresses.RemoveAllElements();

    for (IMS_UINT32 nAt = 0; nAt < m_objPcscfList.GetSize(); nAt++)
    {
        Pcscf* pPcscf = m_objPcscfList.GetAt(nAt);
        if (pPcscf != IMS_NULL)
        {
            m_objCurrAddresses.AddElement(pPcscf->GetAddress());
        }
    }

    return m_objCurrAddresses;
}

PUBLIC VIRTUAL const ImsList<IMS_SINT32>& AosPcscf::GetPcscfsPorts()
{
    m_objCurrPorts.Clear();

    for (IMS_UINT32 nAt = 0; nAt < m_objPcscfList.GetSize(); nAt++)
    {
        Pcscf* pPcscf = m_objPcscfList.GetAt(nAt);
        if (pPcscf != IMS_NULL)
        {
            m_objCurrPorts.Append(pPcscf->GetPort());
        }
    }

    return m_objCurrPorts;
}

PUBLIC VIRTUAL void AosPcscf::UpdatePcscfs(IN const AStringArray& objPcscfs,
        IN ImsList<IMS_SINT32> objPorts /* = ImsList<IMS_SINT32>() */)
{
    ClearPcscfList();

    IMS_SINT32 nDefaultPort = GetDefaultPcscfPort();
    for (IMS_UINT32 nIndex = objPorts.GetSize(); nIndex < objPcscfs.GetCount(); nIndex++)
    {
        objPorts.Append(nDefaultPort);
    }

    for (IMS_UINT32 nAt = 0; nAt < objPcscfs.GetCount(); ++nAt)
    {
        IpAddress objIpa(objPcscfs.GetElementAt(nAt));
        IMS_SINT32 nPort = objPorts.GetAt(nAt);

        if (!IsSamePcscf(objIpa, nPort))
        {
            AddPcscf(objIpa.ToString(), nPort);
        }
    }
}

PUBLIC VIRTUAL IMS_BOOL AosPcscf::HasPcscf(IN IMS_SINT32 nIndex)
{
    if (nIndex <= (static_cast<IMS_SINT32>(m_objPcscfList.GetSize()) - 1))
    {
        return IMS_TRUE;
    }

    return IMS_FALSE;
}

PUBLIC VIRTUAL IMS_UINT32 AosPcscf::GetPcscfCount()
{
    return m_objPcscfList.GetSize();
}

PUBLIC VIRTUAL void AosPcscf::SetCurrentPcscfInvalid(
        IN IMS_BOOL bIsTimer /* = IMS_FALSE */, IN IMS_UINT32 nSeconds /* = 0 */)
{
    if (m_nCurrentPcscfIndex < m_objPcscfList.GetSize())
    {
        Pcscf* pPcscf = m_objPcscfList.GetAt(m_nCurrentPcscfIndex);
        if (pPcscf != IMS_NULL)
        {
            if (bIsTimer)
            {
                pPcscf->SetUnavailableWithDuration(nSeconds);
            }
            else
            {
                pPcscf->SetAvailable(IMS_FALSE);
            }
        }
    }
}

PUBLIC VIRTUAL void AosPcscf::RemoveCurrentPcscf()
{
    if (m_nCurrentPcscfIndex < m_objPcscfList.GetSize())
    {
        Pcscf* pPcscf = m_objPcscfList.GetAt(m_nCurrentPcscfIndex);
        if (pPcscf != IMS_NULL)
        {
            delete pPcscf;
        }

        m_objPcscfList.RemoveAt(m_nCurrentPcscfIndex);
    }
}

PUBLIC VIRTUAL void AosPcscf::SetAllPcscfValid()
{
    for (IMS_UINT32 nAt = 0; nAt < m_objPcscfList.GetSize(); nAt++)
    {
        Pcscf* pPcscf = m_objPcscfList.GetAt(nAt);
        if (pPcscf != IMS_NULL)
        {
            pPcscf->SetAvailable(IMS_TRUE);
        }
    }
}

PUBLIC VIRTUAL IMS_BOOL AosPcscf::IsAllPcscfTried()
{
    for (IMS_UINT32 nAt = 0; nAt < m_objPcscfList.GetSize(); nAt++)
    {
        Pcscf* pPcscf = m_objPcscfList.GetAt(nAt);
        if (pPcscf != IMS_NULL && !pPcscf->IsTried())
        {
            return IMS_FALSE;
        }
    }

    return IMS_TRUE;
}

PUBLIC VIRTUAL void AosPcscf::SetCurrentPcscfTried()
{
    if (m_nCurrentPcscfIndex < m_objPcscfList.GetSize())
    {
        Pcscf* pPcscf = m_objPcscfList.GetAt(m_nCurrentPcscfIndex);
        if (pPcscf != IMS_NULL)
        {
            pPcscf->SetTry(IMS_TRUE);
        }
    }
}

PUBLIC VIRTUAL void AosPcscf::ResetAllPcscfTried()
{
    for (IMS_UINT32 nAt = 0; nAt < m_objPcscfList.GetSize(); nAt++)
    {
        Pcscf* pPcscf = m_objPcscfList.GetAt(nAt);
        if (pPcscf != IMS_NULL)
        {
            pPcscf->SetTry(IMS_FALSE);
        }
    }
}

PUBLIC VIRTUAL IMS_UINT32 AosPcscf::GetCurrentPcscfTriedCount()
{
    if (m_nCurrentPcscfIndex < m_objPcscfList.GetSize())
    {
        Pcscf* pPcscf = m_objPcscfList.GetAt(m_nCurrentPcscfIndex);
        if (pPcscf != IMS_NULL)
        {
            return pPcscf->GetTriedCount();
        }
    }

    return 0;
}

PUBLIC VIRTUAL void AosPcscf::IncreaseCurrentPcscfTriedCount()
{
    if (!IsRegRetryCountPerPcscfConfigured() && !IsRegRetryCountOnSinglePcscfConfigured())
    {
        return;
    }

    if (m_nCurrentPcscfIndex < m_objPcscfList.GetSize())
    {
        Pcscf* pPcscf = m_objPcscfList.GetAt(m_nCurrentPcscfIndex);
        if (pPcscf != IMS_NULL)
        {
            pPcscf->IncreaseTriedCount();
        }
    }
}

PUBLIC VIRTUAL void AosPcscf::ResetCurrentPcscfTriedCount()
{
    if (!IsRegRetryCountPerPcscfConfigured() && !IsRegRetryCountOnSinglePcscfConfigured())
    {
        return;
    }

    if (m_nCurrentPcscfIndex < m_objPcscfList.GetSize())
    {
        Pcscf* pPcscf = m_objPcscfList.GetAt(m_nCurrentPcscfIndex);
        if (pPcscf != IMS_NULL)
        {
            pPcscf->ResetTriedCount();
        }
    }
}

PUBLIC VIRTUAL void AosPcscf::ResetAllPcscfTriedCount()
{
    if (!IsRegRetryCountPerPcscfConfigured() && !IsRegRetryCountOnSinglePcscfConfigured())
    {
        return;
    }

    for (IMS_UINT32 nAt = 0; nAt < m_objPcscfList.GetSize(); nAt++)
    {
        Pcscf* pPcscf = m_objPcscfList.GetAt(nAt);
        if (pPcscf != IMS_NULL)
        {
            pPcscf->ResetTriedCount();
        }
    }
}

PUBLIC VIRTUAL IMS_BOOL AosPcscf::GetCurrentPcscf(OUT AString& objPcscf, OUT IMS_UINT32& nPort)
{
    if ((m_objPcscfList.GetSize() - 1) < m_nCurrentPcscfIndex)
    {
        return IMS_FALSE;
    }

    Pcscf* pPcscf = m_objPcscfList.GetAt(m_nCurrentPcscfIndex);
    if (pPcscf == IMS_NULL)
    {
        return IMS_FALSE;
    }

    objPcscf = pPcscf->GetAddress();
    nPort = pPcscf->GetPort();

    if (nPort == 0)
    {
        nPort = 5060;
    }

    A_IMS_TRACE_I(APPPROFILE, "GetCurrentPcscf :: (%s, %d) at index(%d)",
            IMS_UTIL_SYS_PROP_IS_SERVER_INFO_HIDDEN_IN_LOG() ? "xxx" : objPcscf.GetStr(), nPort,
            m_nCurrentPcscfIndex);

    return IMS_TRUE;
}

PUBLIC VIRTUAL IMS_UINT32 AosPcscf::GetCurrentIndex() const
{
    return m_nCurrentPcscfIndex;
}

PUBLIC VIRTUAL IMS_BOOL AosPcscf::IsFirstPcscf()
{
    return (m_nCurrentPcscfIndex == 0) ? IMS_TRUE : IMS_FALSE;
}

PUBLIC VIRTUAL IMS_BOOL AosPcscf::GetFirstPcscf(OUT AString& objPcscf, OUT IMS_UINT32& nPort)
{
    if (m_objPcscfList.IsEmpty())
    {
        return IMS_FALSE;
    }

    Pcscf* pPcscf = m_objPcscfList.GetAt(0);
    if (pPcscf == IMS_NULL)
    {
        return IMS_FALSE;
    }

    objPcscf = pPcscf->GetAddress();
    nPort = pPcscf->GetPort();
    m_nCurrentPcscfIndex = 0;

    A_IMS_TRACE_I(APPPROFILE, "GetFirstPCSCF :: (%s, %d)",
            IMS_UTIL_SYS_PROP_IS_SERVER_INFO_HIDDEN_IN_LOG() ? "xxx" : objPcscf.GetStr(), nPort, 0);

    return IMS_TRUE;
}

PUBLIC VIRTUAL IMS_BOOL AosPcscf::HasNextPcscf()
{
    return (GetNextPcscfIndex() >= 0);
}

PUBLIC VIRTUAL IMS_SINT32 AosPcscf::GetNextPcscfIndex()
{
    if (m_objPcscfList.GetSize() == 0)
    {
        return -1;
    }

    for (IMS_UINT32 nAt = m_nCurrentPcscfIndex + 1; nAt < m_objPcscfList.GetSize(); nAt++)
    {
        Pcscf* pPcscf = m_objPcscfList.GetAt(nAt);
        if (pPcscf != IMS_NULL && pPcscf->IsAvailable())
        {
            return nAt;
        }
    }

    if (GET_N_CONFIG(m_nSlotId)->GetRegRetryDefaultPolicy() ==
            CarrierConfig::Assets::DEFAULT_RETRY_POLICY_CIRCULAR_NEXT_PCSCF)
    {
        for (int nAt = 0; nAt <= m_nCurrentPcscfIndex; nAt++)
        {
            Pcscf* pPcscf = m_objPcscfList.GetAt(nAt);
            if (pPcscf != IMS_NULL && pPcscf->IsAvailable())
            {
                return nAt;
            }
        }
    }

    return -1;
}

PUBLIC VIRTUAL IMS_BOOL AosPcscf::GetNextPcscf(OUT AString& objPcscf, OUT IMS_UINT32& nPort)
{
    IMS_SINT32 nNextPcscfIndex = GetNextPcscfIndex();

    if (nNextPcscfIndex < 0)
    {
        A_IMS_TRACE_I(APPPROFILE, "GetNextPcscf :: No PCSCF", 0, 0, 0);
        return IMS_FALSE;
    }

    Pcscf* pPcscf = m_objPcscfList.GetAt(nNextPcscfIndex);
    objPcscf = pPcscf->GetAddress();
    nPort = pPcscf->GetPort();
    m_nCurrentPcscfIndex = nNextPcscfIndex;

    A_IMS_TRACE_I(APPPROFILE, "GetNextPcscf :: (%s, %d) at index(%d)",
            IMS_UTIL_SYS_PROP_IS_SERVER_INFO_HIDDEN_IN_LOG() ? "xxx" : objPcscf.GetStr(), nPort,
            nNextPcscfIndex);

    return IMS_TRUE;
}

PUBLIC VIRTUAL void AosPcscf::SetFirstPcscfIndex()
{
    m_nCurrentPcscfIndex = 0;
}

PUBLIC VIRTUAL IMS_BOOL AosPcscf::CheckAndProcessChangeFromPco()
{
    A_IMS_TRACE_I(APPPROFILE, "CheckAndProcessChangeFromPco", 0, 0, 0);

    const AStringArray& objCurrPcscfs = GetPcscfs();
    AString strCurrPcscf;
    IMS_UINT32 nNa;
    if (!GetCurrentPcscf(strCurrPcscf, nNa))
    {
        return IMS_FALSE;
    }

    IpAddress objCurrPcscf;
    if (!objCurrPcscf.Parse(strCurrPcscf))
    {
        return IMS_FALSE;
    }

    AStringArray objNewPcscfs;
    if (GetChangedPcscfs(objNewPcscfs, objCurrPcscf.GetVersion()))
    {
        if (AosUtil::GetInstance()->IsListEqual(objCurrPcscfs, objNewPcscfs, IMS_TRUE))
        {
            return IMS_FALSE;
        }
        else
        {
            if (AosUtil::GetInstance()->IsStrExistInList(strCurrPcscf, objNewPcscfs, IMS_TRUE))
            {
                ProcessReorder(strCurrPcscf, objNewPcscfs);
                m_nChangedType = TYPE_CHANGED_REORDER;
            }
            else
            {
                UpdatePcscfs(objNewPcscfs, GetPcscfPort());
                m_nChangedType = TYPE_CHANGED_DIFFERENT;
            }

            SetFirstPcscfIndex();
            return IMS_TRUE;
        }
    }
    else
    {
        return IMS_FALSE;
    }
}

PUBLIC VIRTUAL IMS_UINT32 AosPcscf::GetChangedType()
{
    return m_nChangedType;
}

PUBLIC VIRTUAL void AosPcscf::SetListener(IN IAosPcscfListener* piListener)
{
    A_IMS_TRACE_D(APPPROFILE, "SetListener :: (%" PFLS_x ") is set", piListener, 0, 0);

    m_piListener = piListener;
}

PROTECTED
void AosPcscf::AddPcscf(IN const AString& strHost, IN IMS_SINT32 nPort)
{
    A_IMS_TRACE_D(APPPROFILE, "AddPcscf :: (%s) :: (%d)",
            IMS_UTIL_SYS_PROP_IS_SERVER_INFO_HIDDEN_IN_LOG() ? "xxx" : strHost.GetStr(), nPort, 0);

    Pcscf* pPcscf = new Pcscf(strHost, nPort);
    m_objPcscfList.Append(pPcscf);
}

PROTECTED
IMS_BOOL AosPcscf::GetChangedPcscfs(OUT AStringArray& objPcscfs, IN IMS_SINT32 nIpVersion)
{
    const AStringArray& objNewPcscfs = m_piAppContext->GetConnection()->GetPcscfAddress(nIpVersion);

    if (objNewPcscfs.GetCount() > 0)
    {
        IpAddress objIpa;

        for (int nAt = 0; nAt < objNewPcscfs.GetCount(); nAt++)
        {
            const AString& strCurrPcscf = objNewPcscfs.GetElementAt(nAt);

            if (!objIpa.Parse(strCurrPcscf))
            {
                continue;
            }

            if (objIpa.GetVersion() != nIpVersion)
            {
                continue;
            }

            if (objIpa.IsAnyAddress())
            {
                continue;
            }

            if (!AosUtil::GetInstance()->IsStrExistInList(strCurrPcscf, objPcscfs, IMS_TRUE))
            {
                objPcscfs.AddElement(strCurrPcscf);
            }
        }
    }
    else
    {
        return IMS_FALSE;
    }

    return (objPcscfs.GetCount() > 0);
}

PROTECTED
IMS_SINT32 AosPcscf::GetLocalIpVersion()
{
    const IpAddress& objIpa = m_piAppContext->GetConnection()->GetLocalAddress();
    IMS_SINT32 nIpVersion = objIpa.GetVersion();

    A_IMS_TRACE_I(APPPROFILE, "GetLocalIpVersion :: IP version (%s)",
            (nIpVersion == IpAddress::IPV4) ? "IPv4" : "IPv6", 0, 0);

    return nIpVersion;
}

PROTECTED
IMS_BOOL AosPcscf::IsLocalAddressValid(IN IMS_SINT32 nIpVersion)
{
    const IpAddress& objIpa = m_piAppContext->GetConnection()->GetLocalAddress(nIpVersion);
    return (!objIpa.IsAnyAddress());
}

PROTECTED
IMS_BOOL AosPcscf::IsLocalAddressTypeValid(IN IMS_SINT32 nIpVersion) const
{
    return (nIpVersion == IpAddress::IPV4 || nIpVersion == IpAddress::IPV6);
}

PROTECTED
IMS_BOOL AosPcscf::IsSamePcscf(IN const IpAddress& objPcscfAddress, IN IMS_SINT32 nPort)
{
    for (IMS_UINT32 nAt = 0; nAt < m_objPcscfList.GetSize(); ++nAt)
    {
        Pcscf* pPcscf = m_objPcscfList.GetAt(nAt);
        if (pPcscf != IMS_NULL)
        {
            IpAddress objIpa(pPcscf->GetAddress());

            if (objIpa.Equals(objPcscfAddress) && (pPcscf->GetPort() == nPort))
            {
                A_IMS_TRACE_D(APPPROFILE, "IsSamePcscf :: TRUE", 0, 0, 0);
                return IMS_TRUE;
            }
        }
    }

    return IMS_FALSE;
}

PROTECTED
void AosPcscf::SetConfigured(IN IMS_BOOL bConfigured)
{
    m_bIsConfigured = bConfigured;
}

PROTECTED VIRTUAL IMS_BOOL AosPcscf::IsFakeDiscoverySchemes() const
{
    return (m_eRegType == AosRegistrationType::EMERGENCY ||
            m_eRegType == AosRegistrationType::FAKE);
}

PROTECTED VIRTUAL void AosPcscf::ProcessDiscovery(IN IMS_SINT32 nIpVersion)
{
    A_IMS_TRACE_D(APPPROFILE, "ProcessDiscovery :: IPV(%d)", nIpVersion, 0, 0);

    IMS_SINT32 nDiscoveryMethod = -1;
    while (GetNextDiscoveryMethod(nDiscoveryMethod) == IMS_TRUE)
    {
        if (nDiscoveryMethod == ISubscriberConfig::PCSCF_DISCOVERY_METHOD_PCO)
        {
            if (GetFromPco(nIpVersion))
            {
                SetConfigured(IMS_TRUE);
                ClearDiscoveryContents();
                break;
            }
        }
        else if (nDiscoveryMethod == ISubscriberConfig::PCSCF_DISCOVERY_METHOD_CONFIG)
        {
            if (GetFromConf(nIpVersion))
            {
                SetConfigured(IMS_TRUE);
                ClearDiscoveryContents();
                break;
            }
            else
            {
                if (m_piDnsQueryRetryTimer != IMS_NULL)
                {
                    A_IMS_TRACE_I(APPPROFILE, "DNS query retry timer is running", 0, 0, 0);
                    break;
                }
            }
        }
    }

    PrintPcscfs();
}

PROTECTED VIRTUAL IMS_BOOL AosPcscf::GetNextDiscoveryMethod(OUT IMS_SINT32& nMethod)
{
    const ISubscriberConfig* piSubsConfig =
            GetSubscriberConfig((IsFakeDiscoverySchemes()) ? IAosSubscriber::FAKE : -1);
    if (piSubsConfig == IMS_NULL)
    {
        A_IMS_TRACE_D(APPPROFILE, "piSubsConfig is null", 0, 0, 0);
        return IMS_FALSE;
    }

    const ImsVector<IMS_SINT32>& objMethods = piSubsConfig->GetPcscfDiscoveryMethods();
    if (objMethods.IsEmpty())
    {
        return IMS_FALSE;
    }

    if (m_nDiscoveryMethodIndex < objMethods.GetSize())
    {
        nMethod = objMethods.GetAt(m_nDiscoveryMethodIndex);
        m_nDiscoveryMethodIndex++;
        return IMS_TRUE;
    }

    return IMS_FALSE;
}

PROTECTED VIRTUAL IMS_BOOL AosPcscf::GetFromPco(IN IMS_SINT32 nIpVersion)
{
    A_IMS_TRACE_I(APPPROFILE, "GetFromPco :: IPv(%d)", nIpVersion, 0, 0);

    const AStringArray& objCurrPcscfs =
            m_piAppContext->GetConnection()->GetPcscfAddress(nIpVersion);

    if (objCurrPcscfs.GetCount() > 0)
    {
        IpAddress objIpa;

        for (int nAt = 0; nAt < objCurrPcscfs.GetCount(); nAt++)
        {
            const AString& strCurrPcscf = objCurrPcscfs.GetElementAt(nAt);

            if (!objIpa.Parse(strCurrPcscf))
            {
                continue;
            }

            if (objIpa.GetVersion() != nIpVersion)
            {
                continue;
            }

            if (objIpa.IsAnyAddress())
            {
                continue;
            }

            IMS_SINT32 nPort = GetPcscfPort();
            if (!IsSamePcscf(objIpa, nPort))
            {
                AddPcscf(objIpa.ToString(), nPort);
            }
        }
    }
    else
    {
        A_IMS_TRACE_I(APPPROFILE, "no pcscf count", 0, 0, 0);
        return IMS_FALSE;
    }

    return (m_objPcscfList.GetSize() > 0);
}

PROTECTED VIRTUAL IMS_BOOL AosPcscf::GetFromConf(IN IMS_SINT32 nIpVersion)
{
    A_IMS_TRACE_I(APPPROFILE, "GetFromConf :: IPv(%d)", nIpVersion, 0, 0);

    const ISubscriberConfig* piSubsConfig =
            GetSubscriberConfig((IsFakeDiscoverySchemes()) ? IAosSubscriber::FAKE : -1);

    if (piSubsConfig == IMS_NULL)
    {
        A_IMS_TRACE_D(APPPROFILE, "piSubsConfig is null", 0, 0, 0);
        return IMS_FALSE;
    }

    const ImsVector<ServerAddress*>& objHosts = piSubsConfig->GetPcscfAddresses();

    if (objHosts.IsEmpty())
    {
        A_IMS_TRACE_I(APPPROFILE, "no pcscf count", 0, 0, 0);
        return IMS_FALSE;
    }

    for (IMS_UINT32 nAt = 0; nAt < objHosts.GetSize(); ++nAt)
    {
        const ServerAddress* pServAddr = objHosts.GetAt(nAt);
        const AString& strHost = pServAddr->GetAddress();
        IMS_SINT32 nPort = pServAddr->GetPort();

        if (strHost.GetLength() == 0)
        {
            continue;
        }

        if (nPort <= 0)
        {
            nPort = GetDefaultPcscfPort();
        }

        IpAddress objIpa;
        objIpa.Parse(strHost);

        // check the IP address or not
        if (objIpa.GetVersion() == nIpVersion)
        {
            if (!IsSamePcscf(objIpa, nPort))
            {
                // Add PCSCF
                AddPcscf(strHost, nPort);
            }
        }
        // process the DNS query if the address is not IP address
        else
        {
            if (!objIpa.IsUnknownAddress())
            {
                continue;
            }

            if (ProcessDnsQuery(strHost, nPort, nIpVersion) == IMS_FALSE)
            {
                A_IMS_TRACE_I(APPPROFILE, "GetHostByName is failed (IPv%d)", nIpVersion, 0, 0);
            }
        }
    }

    return (m_objPcscfList.GetSize() > 0);
}

PROTECTED VIRTUAL IMS_BOOL AosPcscf::ProcessDnsQuery(
        IN const AString& strHost, IN IMS_SINT32 nPort, IN IMS_SINT32 nIpVersion)
{
    ImsList<IpAddress> objIpas;

    A_IMS_TRACE_D(APPPROFILE, "DNS query :: host (%s)", strHost.GetStr(), 0, 0);
    if (m_piAppContext->GetConnection()->GetHostByName(strHost, objIpas, nIpVersion) == -1)
    {
        if (m_bIsDnsQueryRetry == IMS_FALSE)
        {
            RetryHost* pRetryHost = new RetryHost(strHost, nPort, nIpVersion);
            m_objRetryHostList.Append(pRetryHost);

            A_IMS_TRACE_I(APPPROFILE, "Fail for DNS query, start timer for 4sec", 0, 0, 0);
            StartTimer(TIMER_DNS_QUERY_RETRY, DNS_QEUERY_RETRY_WAITING_TIME_MILLS);
        }
        return IMS_FALSE;
    }

    for (IMS_UINT32 i = 0; i < objIpas.GetSize(); ++i)
    {
        const IpAddress& objCurrIpa = objIpas.GetAt(i);

        if (objCurrIpa.GetVersion() != nIpVersion)
        {
            continue;
        }

        if (objCurrIpa.IsAnyAddress())
        {
            continue;
        }

        if (!IsSamePcscf(objCurrIpa, nPort))
        {
            AddPcscf(objCurrIpa.ToString(), nPort);
        }
    }

    return IMS_TRUE;
}

PROTECTED VIRTUAL const ISubscriberConfig* AosPcscf::GetSubscriberConfig(
        IN IMS_SINT32 nType /* = -1 */)
{
    return m_piAppContext->GetSubscriber()->GetSubscriberConfig(nType);
}

PROTECTED VIRTUAL IMS_SINT32 AosPcscf::GetDefaultPcscfPort()
{
    return GET_N_CONFIG(m_nSlotId)->GetPcscfPort();
}

PROTECTED VIRTUAL void AosPcscf::CleanAll()
{
    SetConfigured(IMS_FALSE);
    ClearPcscfList();
    SetFirstPcscfIndex();
    ClearDiscoveryContents();
}

PROTECTED VIRTUAL void AosPcscf::ClearDiscoveryContents()
{
    ClearTimers();
    ClearRetryHostList();
    m_nDiscoveryMethodIndex = 0;
    m_bIsDnsQueryRetry = IMS_FALSE;
    m_bOtherIpTypeRequired = IMS_FALSE;
}

PROTECTED VIRTUAL void AosPcscf::ClearPcscfList()
{
    A_IMS_TRACE_I(APPPROFILE, "ClearPcscfList :: size(%d)", m_objPcscfList.GetSize(), 0, 0);

    for (IMS_UINT32 nAt = 0; nAt < m_objPcscfList.GetSize(); nAt++)
    {
        Pcscf* pPcscf = m_objPcscfList.GetAt(nAt);
        if (pPcscf != IMS_NULL)
        {
            delete pPcscf;
        }
    }

    m_objPcscfList.Clear();
}

PROTECTED VIRTUAL void AosPcscf::ClearRetryHostList()
{
    A_IMS_TRACE_I(APPPROFILE, "ClearRetryHostList :: size(%d)", m_objRetryHostList.GetSize(), 0, 0);

    for (IMS_UINT32 nAt = 0; nAt < m_objRetryHostList.GetSize(); nAt++)
    {
        RetryHost* pRetryHost = m_objRetryHostList.GetAt(nAt);
        if (pRetryHost != IMS_NULL)
        {
            delete pRetryHost;
        }
    }

    m_objRetryHostList.Clear();
}

PROTECTED
void AosPcscf::PrintPcscfs()
{
    if (Engine::GetConfiguration()->IsServerInfoHiddenInLog(m_nSlotId))
    {
        return;
    }

    AString strLog;
    AString strNumber;
    for (IMS_UINT32 nAt = 0; nAt < m_objPcscfList.GetSize(); nAt++)
    {
        Pcscf* pPcscf = m_objPcscfList.GetAt(nAt);
        if (pPcscf != IMS_NULL)
        {
            strLog.Append("[");
            strLog.Append(pPcscf->GetAddress().GetStr());
            strLog.Append("/");
            strLog.Append(strNumber.SetNumber(pPcscf->GetPort()));
            strLog.Append("]");
        }
    }

    A_IMS_TRACE_D(APPPROFILE, "pcscf list :: (%s)", strLog.GetStr(), 0, 0);
}

PROTECTED
void AosPcscf::ProcessDnsRetryTimerExpired()
{
    IMS_SINT32 nIpVersion = IpAddress::UNKNOWN;
    m_bIsDnsQueryRetry = IMS_TRUE;
    A_IMS_TRACE_I(APPPROFILE, "ProcessDnsRetryTimerExpired", 0, 0, 0);

    for (IMS_UINT32 nAt = 0; nAt < m_objRetryHostList.GetSize(); ++nAt)
    {
        RetryHost* pRetryHost = m_objRetryHostList.GetAt(nAt);
        if (pRetryHost != IMS_NULL)
        {
            nIpVersion = pRetryHost->GetIpVersion();
            ProcessDnsQuery(pRetryHost->GetHost(), pRetryHost->GetPort(), nIpVersion);
        }
    }

    if (m_objPcscfList.GetSize() > 0)
    {
        SetConfigured(IMS_TRUE);
        ClearDiscoveryContents();
        if (m_piListener != IMS_NULL)
        {
            m_piListener->Pcscf_NotifyResult(IMS_TRUE);
        }
    }
    else
    {
        ProcessDiscovery(nIpVersion);

        if (!IsConfigured() && m_bOtherIpTypeRequired)
        {
            IMS_SINT32 nOtherIpType =
                    (nIpVersion == IpAddress::IPV6) ? IpAddress::IPV4 : IpAddress::IPV6;
            if (IsLocalAddressValid(nOtherIpType))
            {
                ClearDiscoveryContents();
                ProcessDiscovery(nOtherIpType);
            }
        }
    }
}

PROTECTED VIRTUAL void AosPcscf::StartTimer(IN IMS_UINT32 nType, IN IMS_UINT32 nDuration)
{
    if (nDuration == 0)
    {
        return;
    }

    ITimer** ppiTimer = IMS_NULL;

    switch (nType)
    {
        case TIMER_DNS_QUERY_RETRY:
            ppiTimer = &m_piDnsQueryRetryTimer;
            break;

        default:
            return;
    }

    if (*ppiTimer != IMS_NULL)
    {
        StopTimer(nType);
    }

    *ppiTimer = AosUtil::GetInstance()->StartTimer(nDuration, this, TimerToString(nType));
}

PROTECTED VIRTUAL void AosPcscf::StopTimer(IN IMS_UINT32 nType)
{
    ITimer** ppiTimer = IMS_NULL;

    switch (nType)
    {
        case TIMER_DNS_QUERY_RETRY:
            ppiTimer = &m_piDnsQueryRetryTimer;
            break;

        default:
            return;
    }

    if (*ppiTimer == IMS_NULL)
    {
        return;
    }

    AosUtil::GetInstance()->StopTimer(*ppiTimer, TimerToString(nType));
}

PROTECTED VIRTUAL void AosPcscf::ClearTimers()
{
    if (m_piDnsQueryRetryTimer != IMS_NULL)
    {
        StopTimer(TIMER_DNS_QUERY_RETRY);
    }
}

PROTECTED VIRTUAL void AosPcscf::Timer_TimerExpired(IN ITimer* piTimer)
{
    if (piTimer == IMS_NULL)
    {
        return;
    }

    if (piTimer == m_piDnsQueryRetryTimer)
    {
        StopTimer(TIMER_DNS_QUERY_RETRY);
        ProcessDnsRetryTimerExpired();
        return;
    }
}

PROTECTED GLOBAL const IMS_CHAR* AosPcscf::TimerToString(IN IMS_UINT32 nType)
{
    switch (nType)
    {
        case TIMER_DNS_QUERY_RETRY:
            return "TIMER_DNS_QUERY_RETRY";

        default:
            return "__INVALID__";
    }
}

PRIVATE
IMS_SINT32 AosPcscf::GetPcscfPort()
{
    const ISubscriberConfig* piSubsConfig =
            GetSubscriberConfig((IsFakeDiscoverySchemes()) ? IAosSubscriber::FAKE : -1);

    if (piSubsConfig != IMS_NULL)
    {
        const ServerAddress* pServerAddress = piSubsConfig->GetPcscfAddress();
        if (pServerAddress != IMS_NULL)
            return pServerAddress->GetPort();
    }

    return GetDefaultPcscfPort();
}

PRIVATE
void AosPcscf::ProcessReorder(
        IN const AString& strCurrentPcscf, IN const AStringArray& objNewPcscfs)
{
    A_IMS_TRACE_D(APPPROFILE, "ProcessReorder", 0, 0, 0);

    AStringArray objUpdatePcscfs;
    IpAddress objIpa(strCurrentPcscf);

    IMS_SINT32 nCurrentIndex = 0;
    for (int nAt = 0; nAt < objNewPcscfs.GetCount(); nAt++)
    {
        const AString& strCurr = objNewPcscfs.GetElementAt(nAt);
        IpAddress objIpaCurr(strCurr);

        if (objIpaCurr.Equals(objIpa))
        {
            nCurrentIndex = nAt;
            objUpdatePcscfs.AddElement(strCurrentPcscf);
            break;
        }
    }

    for (int nAt = nCurrentIndex + 1; nAt < objNewPcscfs.GetCount(); ++nAt)
    {
        const AString& strCurr = objNewPcscfs.GetElementAt(nAt);
        objUpdatePcscfs.AddElement(strCurr);
    }

    for (int nAt = 0; nAt < nCurrentIndex; ++nAt)
    {
        const AString& strCurr = objNewPcscfs.GetElementAt(nAt);
        objUpdatePcscfs.AddElement(strCurr);
    }

    UpdatePcscfs(objUpdatePcscfs, GetPcscfPort());
}

PRIVATE
void AosPcscf::UpdatePcscfs(IN const AStringArray& objPcscfs, IN IMS_SINT32 nPort)
{
    ClearPcscfList();

    for (int nAt = 0; nAt < objPcscfs.GetCount(); ++nAt)
    {
        IpAddress objIpa(objPcscfs.GetElementAt(nAt));
        if (!IsSamePcscf(objIpa, nPort))
        {
            AddPcscf(objIpa.ToString(), nPort);
        }
    }
}

PRIVATE
IMS_BOOL AosPcscf::IsRegRetryCountOnSinglePcscfConfigured()
{
    return (GET_N_CONFIG(m_nSlotId)->GetRegRetryCountOnSinglePcscf() > 0);
}

PRIVATE
IMS_BOOL AosPcscf::IsRegRetryCountPerPcscfConfigured()
{
    return (GET_N_CONFIG(m_nSlotId)->GetRegRetryCountPerPcscf() > 0);
}