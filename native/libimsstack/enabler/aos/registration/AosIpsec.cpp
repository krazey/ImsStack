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
#include "ServiceNetwork.h"
#include "ServiceSystemTime.h"
#include "registration/AosIpsec.h"
#include "INetworkIpSec.h"
#include "IIpSecSp.h"
#include "IIpSecSa.h"
#include "IIpSecPolicy.h"
#include "ImsIpSecType.h"

__IMS_TRACE_TAG_AOS__;

#define AOSTAG m_strTag.GetStr()

IMS_UINT32 gRandUePort = AosIpsec::UE_PORT_LOWER;
IMS_UINT32 gRandPcscfPort = AosIpsec::PCSCF_PORT_LOWER;

PUBLIC
AosIpsec::AosIpsec(IN IAosIpsecListener* piListener, IN IMS_SINT32 nSlotId) :
        m_piNetIpsec(IMS_NULL),
        m_piPolicy(IMS_NULL),
        m_pPcscfInfo(new PcscfIpsecInfo()),
        m_bSaEstablished(IMS_FALSE),
        m_bIgnorePolicyExpired(IMS_FALSE),
        m_piListener(piListener),
        m_pUeInfo(new UeIpsecInfo()),
        m_nSecuProto(IpSecType::SECURITY_PROTOCOL_ESP),
        m_nAuthAlgo(IpSecType::INTEGRITY_ALGORITHM_HMAC_SHA_1_96),
        m_nEncrAlgo(IpSecType::ENCRYPTION_ALGORITHM_NO),
        m_nMode(IpSecType::MODE_TRANSPORT),
        m_bAddPolicy(IMS_FALSE),
        m_nSlotId(nSlotId)
{
    IMS_TRACE_MEM("AOS_MEM", "AOS_M : AosIpsec = %" PFLS_u "/%" PFLS_x, sizeof(AosIpsec), this, 0);

    m_strTag.Sprintf("%d", m_nSlotId);

    m_piNetIpsec = NetworkService::GetNetworkService()->GetIpSec(m_nSlotId);

    if (m_piNetIpsec == IMS_NULL)
    {
        A_IMS_TRACE_D(AOSTAG, "INetIPSec is null", 0, 0, 0);
        return;
    }

    m_piPolicy = m_piNetIpsec->CreatePolicy();

    if (m_piPolicy == IMS_NULL)
    {
        A_IMS_TRACE_D(AOSTAG, "IIPSecPolicy is null", 0, 0, 0);
        return;
    }

    m_piPolicy->SetListener(this);
}

PUBLIC VIRTUAL AosIpsec::~AosIpsec()
{
    IMS_TRACE_MEM("AOS_MEM", "AOS_F : AosIpsec = %" PFLS_u "/%" PFLS_x, sizeof(AosIpsec), this, 0);

    if (m_pUeInfo != IMS_NULL)
    {
        delete m_pUeInfo;
    }

    if (m_pPcscfInfo != IMS_NULL)
    {
        delete m_pPcscfInfo;
    }

    if (m_bAddPolicy == IMS_TRUE)
    {
        m_piNetIpsec->DeletePolicy(m_piPolicy);
    }

    m_piNetIpsec->DestroyPolicy(m_piPolicy);
    m_piPolicy = IMS_NULL;
    m_piNetIpsec = IMS_NULL;
}

PUBLIC
void AosIpsec::IpSecPolicy_OnSecurityAssociationExpired(IN IIpSecPolicy* piPolicy)
{
    A_IMS_TRACE_I(AOSTAG, "Expires IPSec Policy", 0, 0, 0);

    if (piPolicy != m_piPolicy)
    {
        A_IMS_TRACE_D(AOSTAG, "Policy is mismatch", 0, 0, 0);
        return;
    }

    if (m_bIgnorePolicyExpired)
    {
        A_IMS_TRACE_I(AOSTAG, "ExpiredSAs :: ignore expired policy", 0, 0, 0);
        return;
    }

    if (m_piListener != IMS_NULL)
    {
        m_piListener->IPSecPolicyExpired(this);
    }

    delete this;
}

PUBLIC
IMS_UINT32 AosIpsec::CreateUePort()
{
    IMS_UINT32 nPort = 0;
    // 1. Crate UE Port
    if (gRandUePort < UE_PORT_UPPER)
    {
        nPort = ++gRandUePort;
    }
    else
    {
        gRandUePort = UE_PORT_LOWER;
        nPort = gRandUePort;
    }

    A_IMS_TRACE_I(AOSTAG, "Create UE Port (%d)", nPort, 0, 0);

    return nPort;
}

PUBLIC
IMS_UINT32 AosIpsec::CreateUeSpi()
{
    static IMS_UINT32 nSpi = IMS_SYS_GetRandom0();
    nSpi = nSpi + SPI_VALUE_TO_BE_INCREASED;

    if (nSpi < SPI_MIN)
    {
        nSpi += SPI_MIN;
    }

    A_IMS_TRACE_I(AOSTAG, "Create UE Spi (%x)", nSpi, 0, 0);

    return nSpi;
}

PUBLIC
void AosIpsec::CreateSps(IN IMS_UINT32 nType)
{
    A_IMS_TRACE_I(AOSTAG, "CreateSps :: type (%d)", nType, 0, 0);

    if (IpSecType::TRANS_PROTOCOL_TCP == nType)
    {
        CreateSpforTcp(TCP_CLIENT_OUTBOUND);
        CreateSpforTcp(TCP_CLIENT_INBOUND);
        CreateSpforTcp(TCP_SERVER_OUTBOUND);
        CreateSpforTcp(TCP_SERVER_INBOUND);
    }
    else  // UDP case
    {
        CreateSpforUdp(DIRECTION_OUTBOUND);
        CreateSpforUdp(DIRECTION_INBOUND);
    }
}

PUBLIC
void AosIpsec::CreateSas()
{
    CreateSa(SA_DIR_US_PC);
    CreateSa(SA_DIR_UC_PS);
    CreateSa(SA_DIR_PS_UC);
    CreateSa(SA_DIR_PC_US);
}

PUBLIC
IMS_BOOL AosIpsec::AddPolicy()
{
    A_IMS_TRACE_I(AOSTAG, "Add Policy to IPSEC lib", 0, 0, 0);

    if (m_piPolicy == IMS_NULL)
    {
        A_IMS_TRACE_D(AOSTAG, "m_piPolicy is null", 0, 0, 0);
        return IMS_FALSE;
    }

    m_bAddPolicy = m_piNetIpsec->AddPolicy(m_piPolicy);

    return m_bAddPolicy;
}

PUBLIC
void AosIpsec::SetSaEstablished()
{
    A_IMS_TRACE_I(AOSTAG, "Sa is established", 0, 0, 0);

    m_bSaEstablished = IMS_TRUE;
}

PUBLIC
IMS_BOOL AosIpsec::IsSaEstablished()
{
    A_IMS_TRACE_I(AOSTAG, "SA  %s", (m_bSaEstablished) ? "ESTABLISHED" : "NOT ESTABLISHED", 0, 0);

    return m_bSaEstablished;
}

PUBLIC
void AosIpsec::SetIps(IN const IpAddress& objLocalIpa, IN const IpAddress& objPcscfIpa)
{
    A_IMS_TRACE_D(AOSTAG, "Set IPs Local (%s) , P-CSCF (%s)", objLocalIpa.ToString().GetStr(),
            objPcscfIpa.ToString().GetStr(), 0);

    m_pUeInfo->objIpa = objLocalIpa;
    m_pPcscfInfo->objIpa = objPcscfIpa;
}

PUBLIC
void AosIpsec::SetKeys(IN const ByteArray& objAuthKey, IN const ByteArray& objEncrKey)
{
    A_IMS_TRACE_D(AOSTAG, "SetKeys :: (IK , CK)", 0, 0, 0);
    m_pUeInfo->objIk = objAuthKey;
    m_pUeInfo->objCk = objEncrKey;
}

PUBLIC
void AosIpsec::SetSecurityAlgorithm(
        IN IMS_UINT32 nSecuAlog, IN IMS_UINT32 nAuthAlgo, IN IMS_UINT32 nEncrAlgo)
{
    A_IMS_TRACE_I(AOSTAG, "SetSecurityAlgorithm :: Securtiy (%d) , Auth (%d) , Encr (%d)",
            nSecuAlog, nAuthAlgo, nEncrAlgo);

    this->m_nSecuProto = (nSecuAlog == IpSecType::SECURITY_PROTOCOL_AH)
            ? IpSecType::SECURITY_PROTOCOL_AH
            : IpSecType::SECURITY_PROTOCOL_ESP;
    this->m_nAuthAlgo = nAuthAlgo;
    this->m_nEncrAlgo = nEncrAlgo;

    if (nEncrAlgo == SipSecurityHeader::EALG_UNSPECIFIED)
    {
        this->m_nEncrAlgo = IpSecType::ENCRYPTION_ALGORITHM_NO;
    }
}

PUBLIC
void AosIpsec::SetUePortsAndSpis(
        IN IMS_UINT32 nPortC, IN IMS_UINT32 nPortS, IN IMS_UINT32 nSpiC, IN IMS_UINT32 nSpiS)
{
    A_IMS_TRACE_D(AOSTAG, "Set Ports and Spis in UE", 0, 0, 0);

    m_pUeInfo->nPortC = nPortC;
    m_pUeInfo->nPortS = nPortS;
    m_pUeInfo->nSpiC = nSpiC;
    m_pUeInfo->nSpiS = nSpiS;
}

PUBLIC
void AosIpsec::SetPcscfPortsAndSpis(
        IN IMS_UINT32 nPortC, IN IMS_UINT32 nPortS, IN IMS_UINT32 nSpiC, IN IMS_UINT32 nSpiS)
{
    A_IMS_TRACE_D(AOSTAG, "Set Ports and Spis in P-CSCF", 0, 0, 0);

    m_pPcscfInfo->nPortC = nPortC;
    m_pPcscfInfo->nPortS = nPortS;
    m_pPcscfInfo->nSpiC = nSpiC;
    m_pPcscfInfo->nSpiS = nSpiS;
}

PUBLIC
void AosIpsec::MakeSecurityClientH(
        IN SipSecurityHeader& objSecuH, IN IMS_BOOL bSpi3gpp /* = IMS_TRUE */)
{
    // 1. Set Algorithm
    objSecuH.SetAlgorithm(m_nAuthAlgo);
    objSecuH.SetEncryptionAlgorithm(m_nEncrAlgo);
    objSecuH.SetMode(m_nMode);
    objSecuH.SetProtocol(m_nSecuProto);

    // 2. Set Ports
    objSecuH.SetPort(m_pUeInfo->nPortC, m_pUeInfo->nPortS);

    // 3. Set SPIs
    objSecuH.SetSpi(m_pUeInfo->nSpiC, m_pUeInfo->nSpiS, bSpi3gpp);
}

PUBLIC
void AosIpsec::IgnorePolicyLifetime()
{
    m_bIgnorePolicyExpired = IMS_TRUE;
}

PUBLIC
void AosIpsec::ManagePolicyLifetime(IN IMS_UINT32 nDuration)
{
    A_IMS_TRACE_I(AOSTAG, "Manage policy lifetime (%d)", nDuration, 0, 0);

    m_piPolicy->ManageLifetime(nDuration);
}

PUBLIC
IIpSecPolicy* AosIpsec::GetPolicy()
{
    if (m_piPolicy != IMS_NULL)
    {
        return m_piPolicy;
    }
    else
    {
        A_IMS_TRACE_D(AOSTAG, "m_piNetIpsec is null", 0, 0, 0);
        return IMS_NULL;
    }
}

PUBLIC
const IpAddress& AosIpsec::GetUeIpa() const
{
    return m_pUeInfo->objIpa;
}

PUBLIC
IMS_UINT32 AosIpsec::GetUePort(IN IMS_UINT32 nType)
{
    IMS_UINT32 nPort = 0;
    if (nType == TYPE_CLIENT)
    {
        nPort = m_pUeInfo->nPortC;
    }
    else if (nType == TYPE_SERVER)
    {
        nPort = m_pUeInfo->nPortS;
    }

    A_IMS_TRACE_I(
            AOSTAG, "GetUePort :: type (%s) , port (%d)", (nType) ? "SERVER" : "CLIENT", nPort, 0);

    return nPort;
}

PUBLIC
IMS_UINT32 AosIpsec::GetUeSpi(IN IMS_UINT32 nType)
{
    IMS_UINT32 nSpi = 0;

    if (nType == TYPE_CLIENT)
    {
        nSpi = m_pUeInfo->nSpiC;
    }
    else if (nType == TYPE_SERVER)
    {
        nSpi = m_pUeInfo->nSpiS;
    }

    A_IMS_TRACE_I(
            AOSTAG, "GetUeSpi :: type (%s) , spi (%x)", (nType) ? "SERVER" : "CLIENT", nSpi, 0);

    return nSpi;
}

PUBLIC
IMS_UINT32 AosIpsec::GetPcscfPort(IN IMS_UINT32 nType)
{
    IMS_UINT32 nPort = 0;

    if (nType == TYPE_CLIENT)
    {
        nPort = m_pPcscfInfo->nPortC;
    }
    else if (nType == TYPE_SERVER)
    {
        nPort = m_pPcscfInfo->nPortS;
    }

    A_IMS_TRACE_I(AOSTAG, "GetPcscfPort :: type (%s) , port (%d)", (nType) ? "SERVER" : "CLIENT",
            nPort, 0);

    return nPort;
}

PUBLIC
IMS_UINT32 AosIpsec::GetPcscfSpi(IN IMS_UINT32 nType)
{
    IMS_UINT32 nSpi = 0;

    if (nType == TYPE_CLIENT)
    {
        nSpi = m_pPcscfInfo->nSpiC;
    }
    else if (nType == TYPE_SERVER)
    {
        nSpi = m_pPcscfInfo->nSpiS;
    }

    A_IMS_TRACE_I(
            AOSTAG, "GetPcscfSpi :: type (%s) , spi (%d)", (nType) ? "SERVER" : "CLIENT", nSpi, 0);

    return nSpi;
}

PUBLIC
const IpAddress& AosIpsec::GetPcscfIpa() const
{
    return m_pPcscfInfo->objIpa;
}

PUBLIC
void AosIpsec::DumpSas()
{
    A_IMS_TRACE_D(AOSTAG, "Display dump to IPSec Lib", 0, 0, 0);
    m_piNetIpsec->DumpPolicy(m_piPolicy);
}

PRIVATE
void AosIpsec::CreateSpforUdp(IN IMS_UINT32 nDir)
{
    A_IMS_TRACE_D(AOSTAG, "CreateSpforUdp :: Direction (%d)", nDir, 0, 0);

    IIpSecSp* piSp = m_piPolicy->CreateSp();

    if (nDir == DIRECTION_OUTBOUND)
    {
        piSp->SetTransportInfo(m_pUeInfo->objIpa, m_pUeInfo->nPortC, m_pPcscfInfo->objIpa,
                m_pPcscfInfo->nPortS, IpSecType::TRANS_PROTOCOL_UDP, IpSecType::ACTION_APPLY,
                IpSecType::DIRECTION_OUTBOUND, m_pPcscfInfo->nSpiS, m_nMode);

        piSp->SetSecurityAlgorithmInfo(m_nSecuProto, m_nAuthAlgo, m_nEncrAlgo);
    }
    else  // INBOUND
    {
        piSp->SetTransportInfo(m_pPcscfInfo->objIpa, m_pPcscfInfo->nPortC, m_pUeInfo->objIpa,
                m_pUeInfo->nPortS, IpSecType::TRANS_PROTOCOL_UDP, IpSecType::ACTION_PERMIT,
                IpSecType::DIRECTION_INBOUND, m_pUeInfo->nSpiS, m_nMode);

        piSp->SetSecurityAlgorithmInfo(m_nSecuProto, m_nAuthAlgo, m_nEncrAlgo);
    }

    piSp->DoneSp();
}

PRIVATE
void AosIpsec::CreateSpforTcp(IN IMS_UINT32 nType)
{
    A_IMS_TRACE_D(AOSTAG, "CreateSpforTcp :: type (%d)", nType, 0, 0);

    IIpSecSp* piSp = m_piPolicy->CreateSp();

    if (nType == TCP_CLIENT_OUTBOUND)
    {
        // Ue-PortC -> Pcscf-PortS
        piSp->SetTransportInfo(m_pUeInfo->objIpa, m_pUeInfo->nPortC, m_pPcscfInfo->objIpa,
                m_pPcscfInfo->nPortS, IpSecType::TRANS_PROTOCOL_TCP, IpSecType::ACTION_APPLY,
                IpSecType::DIRECTION_OUTBOUND, m_pPcscfInfo->nSpiS, m_nMode);
    }
    else if (nType == TCP_CLIENT_INBOUND)
    {
        // Ue-PortC <- Pcscf-PortS
        piSp->SetTransportInfo(m_pPcscfInfo->objIpa, m_pPcscfInfo->nPortS, m_pUeInfo->objIpa,
                m_pUeInfo->nPortC, IpSecType::TRANS_PROTOCOL_TCP, IpSecType::ACTION_PERMIT,
                IpSecType::DIRECTION_INBOUND, m_pUeInfo->nSpiC, m_nMode);
    }
    else if (nType == TCP_SERVER_OUTBOUND)
    {
        // Ue-PortS -> Pcscf-PortC
        piSp->SetTransportInfo(m_pUeInfo->objIpa, m_pUeInfo->nPortS, m_pPcscfInfo->objIpa,
                m_pPcscfInfo->nPortC, IpSecType::TRANS_PROTOCOL_TCP, IpSecType::ACTION_APPLY,
                IpSecType::DIRECTION_OUTBOUND, m_pPcscfInfo->nSpiC, m_nMode);
    }
    else if (nType == TCP_SERVER_INBOUND)
    {
        // Ue-PortS <- Pcscf-PortC
        piSp->SetTransportInfo(m_pPcscfInfo->objIpa, m_pPcscfInfo->nPortC, m_pUeInfo->objIpa,
                m_pUeInfo->nPortS, IpSecType::TRANS_PROTOCOL_TCP, IpSecType::ACTION_PERMIT,
                IpSecType::DIRECTION_INBOUND, m_pUeInfo->nSpiS, m_nMode);
    }
    else
    {
        A_IMS_TRACE_D(AOSTAG, "nType is wrong", 0, 0, 0);
    }

    piSp->SetSecurityAlgorithmInfo(m_nSecuProto, m_nAuthAlgo, m_nEncrAlgo);
    piSp->DoneSp();
}

PRIVATE
void AosIpsec::CreateSa(IN IMS_UINT32 nType)
{
    A_IMS_TRACE_D(AOSTAG, "CreateSA :: type (%d)", nType, 0, 0);

    /*
        four SADs
        1. Uc ---> Ps
        2. Us ---> Pc
        3. Us <--- Pc
        4. Uc <--- Ps
    */

    IIpSecSa* piSa = m_piPolicy->CreateSa();

    IpAddress objSrcIPA;
    IMS_UINT32 nSrcPort = 0;
    IpAddress objDestIPA;
    IMS_UINT32 nDestPort = 0;
    IMS_UINT32 nSpi = 0;

    if (nType == SA_DIR_UC_PS)
    {
        objSrcIPA = m_pUeInfo->objIpa;
        nSrcPort = m_pUeInfo->nPortC;
        objDestIPA = m_pPcscfInfo->objIpa;
        nDestPort = m_pPcscfInfo->nPortS;
        nSpi = m_pPcscfInfo->nSpiS;
    }
    else if (nType == SA_DIR_US_PC)
    {
        objSrcIPA = m_pUeInfo->objIpa;
        nSrcPort = m_pUeInfo->nPortS;
        objDestIPA = m_pPcscfInfo->objIpa;
        nDestPort = m_pPcscfInfo->nPortC;
        nSpi = m_pPcscfInfo->nSpiC;
    }
    else if (nType == SA_DIR_PC_US)
    {
        objSrcIPA = m_pPcscfInfo->objIpa;
        nSrcPort = m_pPcscfInfo->nPortC;
        objDestIPA = m_pUeInfo->objIpa;
        nDestPort = m_pUeInfo->nPortS;
        nSpi = m_pUeInfo->nSpiS;
    }
    else if (nType == SA_DIR_PS_UC)
    {
        objSrcIPA = m_pPcscfInfo->objIpa;
        nSrcPort = m_pPcscfInfo->nPortS;
        objDestIPA = m_pUeInfo->objIpa;
        nDestPort = m_pUeInfo->nPortC;
        nSpi = m_pUeInfo->nSpiC;
    }
    else
    {
        A_IMS_TRACE_D(AOSTAG, "SAD's type is invalid", 0, 0, 0);
        return;
    }

    piSa->SetSa(objSrcIPA, nSrcPort, objDestIPA, nDestPort, m_nSecuProto, nSpi, m_nMode,
            m_nAuthAlgo, m_nEncrAlgo, m_pUeInfo->objIk, m_pUeInfo->objCk);

    piSa->DoneSa();
}

PRIVATE
IMS_UINT32 AosIpsec::GetIntegrityAlgorithm()
{
    A_IMS_TRACE_I(AOSTAG, "GetIntegrityAlgorithm :: (%d)", m_nAuthAlgo, 0, 0);

    return m_nAuthAlgo;
}
