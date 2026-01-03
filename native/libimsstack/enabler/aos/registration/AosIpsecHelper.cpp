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
#include "ServiceNetwork.h"
#include "ISocket.h"
#include "IRegContact.h"
#include "IRegParameter.h"
#include "Credential.h"
#include "ImsIpSecType.h"
#include "ISipTransportHelper.h"
#include "SipFactory.h"
#include "interface/IAosAppContext.h"
#include "interface/IAosNConfiguration.h"
#include "interface/IAosConnection.h"
#include "interface/IAosPcscf.h"
#include "provider/AosProvider.h"
#include "provider/AosUtil.h"
#include "registration/AosIpsecHelper.h"

__IMS_TRACE_TAG_AOS__;

#define REGID m_strTag.GetStr()

PUBLIC
AosIpsecHelper::AosIpsecHelper(IN IRegContact* piRegContact, IN IRegParameter* piRegParameter,
        IN IAosAppContext* piAppContext, IN AString& strRegId) :
        m_piRegContact(piRegContact),
        m_piRegParameter(piRegParameter),
        m_piContext(piAppContext),
        m_strRegId(strRegId),
        m_pOldIpsec(IMS_NULL),
        m_pCurrIpsec(IMS_NULL),
        m_pNewIpsec(IMS_NULL),
        m_pUeIpsecInfo(IMS_NULL)
{
    m_strTag.Sprintf("%d:%s", m_piContext->GetSlotId(), m_strRegId.GetStr());

    IMS_TRACE_MEM("AOS_MEM", "AOS_M : [%s] AosIpsecHelper = %" PFLS_u "/%" PFLS_x, REGID,
            sizeof(AosIpsecHelper), this);

    m_pUeIpsecInfo = new UeIpsecInfo();
};

PUBLIC VIRTUAL AosIpsecHelper::~AosIpsecHelper()
{
    IMS_TRACE_MEM("AOS_MEM", "AOS_F : [%s] AosIpsecHelper = %" PFLS_u "/%" PFLS_x, REGID,
            sizeof(AosIpsecHelper), this);

    if (m_piContext != IMS_NULL)
    {
        // Set Socket Operation related to IPSEC
        AosUtil::GetInstance()->SetSocketOptionLinger(-1, m_piContext->GetSlotId());
        AosUtil::GetInstance()->SetSocketOptionShutDown(2 /*both*/, m_piContext->GetSlotId());
    }

    if (m_pUeIpsecInfo != IMS_NULL)
    {
        delete m_pUeIpsecInfo;
    }

    Destroy();
}

PUBLIC VIRTUAL IMS_BOOL AosIpsecHelper::Create(IN IMS_BOOL bInitial)
{
    A_IMS_TRACE_I(REGID, "Create", 0, 0, 0);

    if (m_piRegContact == IMS_NULL || m_piRegParameter == IMS_NULL)
    {
        A_IMS_TRACE_I(REGID, "contact is null", 0, 0, 0);
        return IMS_FALSE;
    }

    if (m_pNewIpsec != IMS_NULL)
    {
        delete m_pNewIpsec;
        m_pNewIpsec = IMS_NULL;
    }

    m_pNewIpsec = new AosIpsec(this, m_piContext->GetSlotId());

    SetUePortnSpi(bInitial);

    m_piRegContact->SetPort(static_cast<IMS_SINT32>(m_pNewIpsec->GetUePort(AosIpsec::TYPE_SERVER)));

    if (!SetSecurityClientHeader())
    {
        return IMS_FALSE;
    }

    SetSecurityServerPortInRegistration();

    // Set default port for first un-protected REGISTER message and not secure port
    if (bInitial == IMS_TRUE)
    {
        m_piRegContact->SetPort(AosUtil::GetInstance()->GetLocalPort(m_piContext->GetSlotId()));
        SetSecurityServerPortInRegContact();
    }

    return IMS_TRUE;
}

PUBLIC VIRTUAL void AosIpsecHelper::CreateOnChallenging()
{
    A_IMS_TRACE_I(REGID, "CreateOnChallenging", 0, 0, 0);

    if (m_pNewIpsec != IMS_NULL)
    {
        delete m_pNewIpsec;
        m_pNewIpsec = IMS_NULL;
    }

    // remove the old SAs when making a new re-challenge.
    if (m_pOldIpsec != IMS_NULL)
    {
        CloseSecureTCPSocket(m_pOldIpsec);

        delete m_pOldIpsec;
        m_pOldIpsec = IMS_NULL;
    }

    m_pNewIpsec = new AosIpsec(this, m_piContext->GetSlotId());

    // set UE Port & SPI using previous values
    m_pNewIpsec->SetUePortsAndSpis(m_pUeIpsecInfo->nPortC, m_pUeIpsecInfo->nPortS,
            m_pUeIpsecInfo->nSpiC, m_pUeIpsecInfo->nSpiS);
}

PUBLIC VIRTUAL IMS_BOOL AosIpsecHelper::SetPcscfPortnSpi()
{
    A_IMS_TRACE_I(REGID, "SetPcscfPortnSpi", 0, 0, 0);

    // get Security Server Header
    const ImsList<SipSecurityHeader>& objSecuServerH = m_piRegParameter->GetSecurityServers();

    if (objSecuServerH.IsEmpty() == IMS_TRUE)
    {
        A_IMS_TRACE_I(REGID, "security header is empty", 0, 0, 0);
        return IMS_FALSE;
    }

    const SipSecurityHeader* pPreferredSSH = m_piRegParameter->GetPreferredSecurityServer();

    if (pPreferredSSH == IMS_NULL)
    {
        A_IMS_TRACE_I(REGID, "pPreferredSSH is null", 0, 0, 0);
        return IMS_FALSE;
    }

    // set Ports & SPIs
    m_pNewIpsec->SetPcscfPortsAndSpis(pPreferredSSH->GetPortC(), pPreferredSSH->GetPortS(),
            pPreferredSSH->GetSpiC(), pPreferredSSH->GetSpiS());

    A_IMS_TRACE_D(REGID, "security-server, proto (%d), alg(%d) , size(%d)",
            pPreferredSSH->GetProtocol(), pPreferredSSH->GetAlgorithm(), objSecuServerH.GetSize());

    m_pNewIpsec->SetSecurityAlgorithm(pPreferredSSH->GetProtocol(), pPreferredSSH->GetAlgorithm(),
            pPreferredSSH->GetEncryptionAlgorithm());

    return IMS_TRUE;
}

PUBLIC VIRTUAL IMS_BOOL AosIpsecHelper::IsPcscfServerPortDifferent()
{
    if (m_pNewIpsec == IMS_NULL)
    {
        return IMS_FALSE;
    }

    if (m_pCurrIpsec == IMS_NULL)
    {
        return IMS_FALSE;
    }

    IMS_UINT32 currPcscfPort = (m_pCurrIpsec->GetPcscfPort(AosIpsec::TYPE_SERVER));
    IMS_UINT32 newPcscfPort = (m_pNewIpsec->GetPcscfPort(AosIpsec::TYPE_SERVER));

    A_IMS_TRACE_I(REGID, "PCSCF Server port curr (%d), new (%d)", currPcscfPort, newPcscfPort, 0);

    return (currPcscfPort != newPcscfPort);
}

PUBLIC VIRTUAL IMS_BOOL AosIpsecHelper::UpdatePreloadedRoute(IN const AString& strPcscf)
{
    if (m_pNewIpsec == IMS_NULL)
    {
        return IMS_FALSE;
    }

    m_piRegParameter->RemoveAllPreloadedRoutes();
    m_piRegParameter->AddPreloadedRoute(
            strPcscf, static_cast<IMS_SINT32>(m_pNewIpsec->GetPcscfPort(AosIpsec::TYPE_SERVER)));

    return IMS_TRUE;
}

PUBLIC VIRTUAL IMS_BOOL AosIpsecHelper::MakeSas(IN const AString& strPcscf,
        IN const IpAddress& objIpa, IN const ByteArray& objIk, IN const ByteArray& objCk)
{
    A_IMS_TRACE_D(REGID, "MakeSas", 0, 0, 0);

    IpAddress objPcscfIpa;
    objPcscfIpa.Parse(strPcscf);

    // set Local & P-CSCF IPs
    m_pNewIpsec->SetIps(objIpa, objPcscfIpa);

    // set IK & CK Keys
    m_pNewIpsec->SetKeys(objIk, objCk);

    // create 2 SPs for UDP
    m_pNewIpsec->CreateSps(IpSecType::TRANS_PROTOCOL_UDP);

    // create 4 SPs for TCP
    m_pNewIpsec->CreateSps(IpSecType::TRANS_PROTOCOL_TCP);

    // create security associations
    m_pNewIpsec->CreateSas();

    // remove same IPSec
    DeleteSamePolicy();

    // add Policy to IPSec lib
    if (!m_pNewIpsec->AddPolicy())
    {
        A_IMS_TRACE_I(REGID, "add policy is failed", 0, 0, 0);
        return IMS_FALSE;
    }

    // Set SA establishment
    m_pNewIpsec->SetSaEstablished();

    // Set protected port after establishing SA
    m_piRegContact->SetPort(static_cast<IMS_SINT32>(m_pNewIpsec->GetUePort(AosIpsec::TYPE_SERVER)));

    return IMS_TRUE;
}

PUBLIC VIRTUAL IMS_BOOL AosIpsecHelper::ProcessAuthChallenged(IN IMS_SINT32 nAlgorithm)
{
    A_IMS_TRACE_I(REGID, "ProcessAuthChallenged", 0, 0, 0);

    if (CheckSecurityServerHeader() == IMS_FALSE)
    {
        return IMS_FALSE;
    }

    if (nAlgorithm != Credential::TYPE_AKAv1_MD5)
    {
        return IMS_FALSE;
    }

    // if it requires to remove the old SAs when making a new SA.
    IMS_BOOL bOldSaRemovedOnEstablisingSa =
            GET_N_CONFIG(m_piContext->GetSlotId())->IsOldSaOnEstablishingSaRemoved();
    if (bOldSaRemovedOnEstablisingSa == IMS_TRUE)
    {
        if (m_pOldIpsec != IMS_NULL)
        {
            delete m_pOldIpsec;
            m_pOldIpsec = IMS_NULL;
        }
    }

    return IMS_TRUE;
}

PUBLIC VIRTUAL void AosIpsecHelper::ProcessRegStarted()
{
    /*
        This is to prevent a memory leakage and crash!!
        If registration refresh had failed, registration will be recovered after some time.
        Even though the refresh failed, we have an ipsec established m_pCurrIpsec is pointing to.
        When the recovery is done, ProcessRegStarted() is invoked.
        It is the case we have m_pCurrIpsec is not null. Always be care when
    */
    if (m_pOldIpsec != IMS_NULL)
    {
        delete m_pOldIpsec;
        m_pOldIpsec = IMS_NULL;
    }

    m_pOldIpsec = m_pCurrIpsec;
    m_pCurrIpsec = m_pNewIpsec;
    m_pNewIpsec = IMS_NULL;

    // set SA life timer
    IMS_UINT32 nExpireTime = m_piRegContact->GetExpires();
    A_IMS_TRACE_I(REGID, "ProcessRegStarted :: nExpireTime (%d)", nExpireTime, 0, 0);

    // set SA lifetime
    m_pCurrIpsec->ManagePolicyLifetime(nExpireTime * 1000 + IPSEC_UPDATE_GUARD_LIFE_TIME_MILLIS);
    m_pCurrIpsec->DumpSas();

    CloseUnsecureTCPSocket();
}

PUBLIC VIRTUAL IMS_BOOL AosIpsecHelper::ProcessRegUpdated()
{
    if (m_pNewIpsec == IMS_NULL)
    {
        A_IMS_TRACE_I(REGID, "new ipsec is null", 0, 0, 0);
        return IMS_FALSE;
    }

    IMS_UINT32 nExpireTime = m_piRegContact->GetExpires();
    A_IMS_TRACE_I(REGID, "ProcessRegUpdated :: nExpireTime in Refresh (%d)", nExpireTime, 0, 0);

    // check to update Pcscf SA Info if UE is received Security-Server Header or not.
    if (m_pNewIpsec->IsSaEstablished())
    {
        A_IMS_TRACE_I(REGID, "established new SA set , replace current SA", 0, 0, 0);

        // UE can delete the old IPsec when an IPsec is newly established.
        if (m_pOldIpsec != IMS_NULL)
        {
            CloseSecureTCPSocket(m_pOldIpsec);

            delete m_pOldIpsec;
            m_pOldIpsec = IMS_NULL;
        }

        m_pOldIpsec = m_pCurrIpsec;  // old SA
        m_pCurrIpsec = m_pNewIpsec;  // new SA
        m_pNewIpsec = IMS_NULL;

        // set SA lifetime
        m_pCurrIpsec->ManagePolicyLifetime(
                nExpireTime * 1000 + IPSEC_UPDATE_GUARD_LIFE_TIME_MILLIS);

        /*
            Maintain the lifetime of the old SA.
            1, UE may have a pending SIP transactions on the old SA.
            2, UE shall maintain the old SA until PCSCF stars a transaction on the new one.
            m_pOldIpsec->ManagePolicyLifetime(IPSEC_UPDATE_GUARD_LIFE_TIME_MILLIS);
            set Dump DBs
        */
        m_pCurrIpsec->DumpSas();
    }
    else
    {
        // keep current SA
        A_IMS_TRACE_I(REGID, "don't received 401 challenge, so keep current SA", 0, 0, 0);

        // delete New SA
        delete m_pNewIpsec;
        m_pNewIpsec = IMS_NULL;

        // update lifetime
        m_pCurrIpsec->ManagePolicyLifetime(
                nExpireTime * 1000 + IPSEC_UPDATE_GUARD_LIFE_TIME_MILLIS);
    }

    return IMS_TRUE;
}

PUBLIC VIRTUAL void AosIpsecHelper::InitIpsec()
{
    // set socket operation related to IPSEC
    AosUtil::GetInstance()->SetSocketOptionLinger(0, m_piContext->GetSlotId());
    AosUtil::GetInstance()->SetSocketOptionShutDown(3 /*no shutdown*/, m_piContext->GetSlotId());
}

PUBLIC VIRTUAL IMS_BOOL AosIpsecHelper::IsEstablished()
{
    if ((m_pOldIpsec != IMS_NULL) && m_pOldIpsec->IsSaEstablished())
    {
        return IMS_TRUE;
    }

    if ((m_pCurrIpsec != IMS_NULL) && m_pCurrIpsec->IsSaEstablished())
    {
        return IMS_TRUE;
    }

    if ((m_pNewIpsec != IMS_NULL) && m_pNewIpsec->IsSaEstablished())
    {
        return IMS_TRUE;
    }

    return IMS_FALSE;
}

PUBLIC VIRTUAL void AosIpsecHelper::SetSecurityServerPortInRegContact()
{
    IMS_BOOL bServerPortInRegContact =
            GET_N_CONFIG(m_piContext->GetSlotId())->IsSecurityServerPortInRegContactOfInitRegUsed();
    if (bServerPortInRegContact == IMS_TRUE)
    {
        A_IMS_TRACE_I(REGID, "SetSecurityServerPortInRegContact", 0, 0, 0);
        m_piRegContact->SetPort(
                static_cast<IMS_SINT32>(m_pNewIpsec->GetUePort(AosIpsec::TYPE_SERVER)));
    }
}

PUBLIC VIRTUAL void AosIpsecHelper::IgnoreCurrentPolicyExpired()
{
    if (m_pCurrIpsec != IMS_NULL)
    {
        m_pCurrIpsec->IgnorePolicyLifetime();
    }
}

PROTECTED VIRTUAL void AosIpsecHelper::SetSecurityServerPortInRegistration()
{
    IMS_BOOL bServerPortInRegistration =
            GET_N_CONFIG(m_piContext->GetSlotId())->IsSecurityServerPortInInitRegUsed();
    if (bServerPortInRegistration == IMS_TRUE)
    {
        m_piRegParameter->SetPort(
                static_cast<IMS_SINT32>(m_pNewIpsec->GetUePort(AosIpsec::TYPE_SERVER)));
    }
}

PROTECTED VIRTUAL IMS_UINT32 AosIpsecHelper::FindAvailableUePort(
        IN UePortType ePortType, OUT IMS_UINT32& nUePort)
{
    IMS_UINT32 nLowerPort = 0;
    IMS_UINT32 nUpperPort = 0;

    if (ePortType == UePortType::CLIENT_PORT)
    {
        ImsVector<IMS_SINT32>& objPortRange =
                GET_N_CONFIG(m_piContext->GetSlotId())->GetIpsecUeClientPortRange();
        if (objPortRange.GetSize() >= 2)
        {
            nLowerPort = objPortRange.GetAt(0);
            nUpperPort = objPortRange.GetAt(1);
        }
        else
        {
            A_IMS_TRACE_I(REGID, "Client port range not in config, using defaults", 0, 0, 0);
            nLowerPort = AosIpsec::UE_PORT_LOWER;
            nUpperPort = AosIpsec::UE_PORT_UPPER;
        }
    }
    else  // UePortType::SERVER_PORT
    {
        ImsVector<IMS_SINT32>& objPortRange =
                GET_N_CONFIG(m_piContext->GetSlotId())->GetIpsecUeServerPortRange();
        if (objPortRange.GetSize() >= 2)
        {
            nLowerPort = objPortRange.GetAt(0);
            nUpperPort = objPortRange.GetAt(1);
        }
        else
        {
            A_IMS_TRACE_I(REGID, "Server port range not in config, using default logic", 0, 0, 0);
            nLowerPort = AosIpsec::UE_PORT_LOWER + AosIpsec::PORTS_INTERVAL;
            nUpperPort = AosIpsec::UE_PORT_UPPER + AosIpsec::PORTS_INTERVAL;
        }
    }

    return GetValidUePort(nLowerPort, nUpperPort, nUePort);
}

PROTECTED VIRTUAL void AosIpsecHelper::SetUePortnSpi(IN IMS_BOOL bInitial)
{
    static IMS_UINT32 nLastClientPort = 0;
    static IMS_UINT32 nLastServerPort = 0;

    m_pUeIpsecInfo->nPortC = FindAvailableUePort(UePortType::CLIENT_PORT, nLastClientPort);

    // ONLY create PortS in initial registration
    if (bInitial)
    {
        m_pUeIpsecInfo->nPortS = FindAvailableUePort(UePortType::SERVER_PORT, nLastServerPort);
    }

    m_pUeIpsecInfo->nSpiC = m_pNewIpsec->CreateUeSpi();
    m_pUeIpsecInfo->nSpiS = m_pUeIpsecInfo->nSpiC + 1;

    A_IMS_TRACE_I(REGID, "SetUePortnSpi :: nPortC (%d) , nPortS (%d) , nSpiC (%x)",
            m_pUeIpsecInfo->nPortC, m_pUeIpsecInfo->nPortS, m_pUeIpsecInfo->nSpiC);

    m_pNewIpsec->SetUePortsAndSpis(m_pUeIpsecInfo->nPortC, m_pUeIpsecInfo->nPortS,
            m_pUeIpsecInfo->nSpiC, m_pUeIpsecInfo->nSpiS);
}

PROTECTED VIRTUAL IMS_BOOL AosIpsecHelper::SetSecurityClientHeader()
{
    const ImsVector<IMS_SINT32>& objAuthenticationAlgs =
            GET_N_CONFIG(m_piContext->GetSlotId())->GetIpsecAuthenticationAlgorithms();
    if (objAuthenticationAlgs.GetSize() == 0)
    {
        A_IMS_TRACE_I(REGID, "ipsec integrity algorithm is invalid", 0, 0, 0);
        return IMS_FALSE;
    }

    ImsVector<IMS_SINT32>& objEncryptionAlgs =
            GET_N_CONFIG(m_piContext->GetSlotId())->GetIpsecEncryptionAlgorithms();

    if (objEncryptionAlgs.GetSize() == 0)
    {
        objEncryptionAlgs.Push(IpSecType::ENCRYPTION_ALGORITHM_NO);
    }

    m_piRegParameter->RemoveSecurityClients();

    // create Security-Client header with integrity and confidentiality algorithm combinations.
    for (IMS_UINT32 nIAlgIdx = 0; nIAlgIdx < objAuthenticationAlgs.GetSize(); nIAlgIdx++)
    {
        for (IMS_UINT32 nEAlgIdx = 0; nEAlgIdx < objEncryptionAlgs.GetSize(); nEAlgIdx++)
        {
            SipSecurityHeader objSecurityClientH;

            m_pNewIpsec->MakeSecurityClientH(objSecurityClientH);

            objSecurityClientH.SetAlgorithm(m_pNewIpsec->GetAuthAlgoForSecurityHeader(
                    objAuthenticationAlgs.GetAt(nIAlgIdx)));

            objSecurityClientH.SetEncryptionAlgorithm(
                    m_pNewIpsec->GetEncrAlgoForSecurityHeader(objEncryptionAlgs.GetAt(nEAlgIdx)));

            m_piRegParameter->AddSecurityClient(objSecurityClientH);
        }
    }

    return IMS_TRUE;
}

PROTECTED VIRTUAL IMS_BOOL AosIpsecHelper::CheckSecurityServerHeader()
{
    A_IMS_TRACE_I(REGID, "CheckSecurityServerHeader", 0, 0, 0);

    const ImsList<SipSecurityHeader>& objSecuServerH = m_piRegParameter->GetSecurityServers();

    if (!objSecuServerH.IsEmpty())
    {
        return IMS_TRUE;
    }

    return IMS_FALSE;
}

PROTECTED VIRTUAL void AosIpsecHelper::ProcessPolicyExpired(IN AosIpsec* pIpsec)
{
    A_IMS_TRACE_I(REGID, "ProcessPolicyExpired", 0, 0, 0);

    // ONLY set ipsec class to null because of AosIpsec class delets itself
    if (m_pOldIpsec == pIpsec)
    {
        m_pOldIpsec = IMS_NULL;
    }
    else if (m_pCurrIpsec == pIpsec)
    {
        m_pCurrIpsec = IMS_NULL;
    }
    else if (m_pNewIpsec == pIpsec)
    {
        m_pNewIpsec = IMS_NULL;
    }
    else
    {
        A_IMS_TRACE_D(REGID, "no ipsec class", 0, 0, 0);
    }
}

PROTECTED VIRTUAL void AosIpsecHelper::IPSecPolicyExpired(IN AosIpsec* pIpsec)
{
    A_IMS_TRACE_I(REGID, "IPSecPolicyExpired", 0, 0, 0);

    CloseSecureTCPSocket(pIpsec);

    ProcessPolicyExpired(pIpsec);
}

PROTECTED
void AosIpsecHelper::Destroy()
{
    A_IMS_TRACE_I(REGID, "Destroy", 0, 0, 0);

    if (m_pOldIpsec != IMS_NULL)
    {
        CloseSecureTCPSocket(m_pOldIpsec);

        delete m_pOldIpsec;
        m_pOldIpsec = IMS_NULL;
    }

    if (m_pCurrIpsec != IMS_NULL)
    {
        CloseSecureTCPSocket(m_pCurrIpsec);

        delete m_pCurrIpsec;
        m_pCurrIpsec = IMS_NULL;
    }

    if (m_pNewIpsec != IMS_NULL)
    {
        CloseSecureTCPSocket(m_pNewIpsec);

        delete m_pNewIpsec;
        m_pNewIpsec = IMS_NULL;
    }
}

PRIVATE
void AosIpsecHelper::CloseUnsecureTCPSocket()
{
    IMS_BOOL bUnsecureTcpSocketDestroyed =
            GET_N_CONFIG(m_piContext->GetSlotId())
                    ->IsUnsecureTcpSocketOnAccomplishingRegDestroyed();
    if (bUnsecureTcpSocketDestroyed == IMS_FALSE)
    {
        return;
    }

    AString objPcscf;
    IMS_UINT32 nPort;

    if (m_piContext->GetPcscf()->GetCurrentPcscf(objPcscf, nPort) == IMS_FALSE)
    {
        A_IMS_TRACE_I(REGID, "Failed to get PCSCF", 0, 0, 0);
        return;
    }

    IpAddress objIpaPcscf(objPcscf);
    IpAddress objIpaLocal = m_piContext->GetConnection()->GetLocalAddress(objIpaPcscf.GetVersion());

    SipFactory::GetTransportHelper(m_piContext->GetSlotId())
            ->DestroyTcpSocket(objIpaLocal, 0, objIpaPcscf, nPort);
}

PRIVATE
void AosIpsecHelper::CloseSecureTCPSocket(IN AosIpsec* pIpsec)
{
    if (pIpsec == IMS_NULL)
    {
        return;
    }

    const IpAddress& objUeIpa = pIpsec->GetUeIpa();
    IMS_UINT32 nUeCPort = pIpsec->GetUePort(AosIpsec::TYPE_CLIENT);
    const IpAddress& objPcscfIpAddr = pIpsec->GetPcscfIpa();
    IMS_UINT32 nPcscfCPort = pIpsec->GetPcscfPort(AosIpsec::TYPE_CLIENT);
    IMS_UINT32 nPcscfSPort = pIpsec->GetPcscfPort(AosIpsec::TYPE_SERVER);

    // Disconnect TCP client connection
    IMS_TRACE_D("UE Port(%d), PCSCF (%s, %d)", nUeCPort, objPcscfIpAddr.ToString().GetStr(),
            nPcscfSPort);

    SipFactory::GetTransportHelper(m_piContext->GetSlotId())
            ->DestroyTcpSocket(objUeIpa, nUeCPort, objPcscfIpAddr, nPcscfSPort);

    // Disconnect TCP server connection if being not used for other IPSec
    IMS_SINT32 nRefCnt = 0;

    if (m_pOldIpsec != IMS_NULL)
    {
        if (nPcscfCPort == m_pOldIpsec->GetPcscfPort(AosIpsec::TYPE_CLIENT) &&
                objPcscfIpAddr.Equals(m_pOldIpsec->GetPcscfIpa()))
        {
            nRefCnt++;
        }
    }

    if (m_pCurrIpsec != IMS_NULL)
    {
        if (nPcscfCPort == m_pCurrIpsec->GetPcscfPort(AosIpsec::TYPE_CLIENT) &&
                objPcscfIpAddr.Equals(m_pCurrIpsec->GetPcscfIpa()))
        {
            nRefCnt++;
        }
    }

    if (m_pNewIpsec != IMS_NULL)
    {
        if (nPcscfCPort == m_pNewIpsec->GetPcscfPort(AosIpsec::TYPE_CLIENT) &&
                objPcscfIpAddr.Equals(m_pNewIpsec->GetPcscfIpa()))
        {
            nRefCnt++;
        }
    }

    IMS_TRACE_I("Reference Count (%d)", nRefCnt, 0, 0);

    if (nRefCnt <= 1)
    {
        SipFactory::GetTransportHelper(m_piContext->GetSlotId())
                ->DestroyTcpSocket(objUeIpa, 0, objPcscfIpAddr, nPcscfCPort, IMS_TRUE);
    }
}

PRIVATE
IMS_UINT32 AosIpsecHelper::GetValidUePort(
        IN IMS_UINT32 nLowerPort, IN IMS_UINT32 nUpperPort, OUT IMS_UINT32& nUePort)
{
    if (nLowerPort >= nUpperPort)
    {
        A_IMS_TRACE_D(REGID, "Invalid port range [%u, %u)", nLowerPort, nUpperPort, 0);
        return 0;
    }

    if (nUePort < nLowerPort - 1 || nUePort >= nUpperPort)
    {
        nUePort = nLowerPort - 1;
    }

    IMS_UINT32 nRangeSize = nUpperPort - nLowerPort;
    IMS_UINT32 nMaxCount =
            (nRangeSize < IPSEC_PORT_RANGE_MAX_COUNT) ? nRangeSize : IPSEC_PORT_RANGE_MAX_COUNT;
    const IpAddress& objUeIPA = m_piRegContact->GetIpAddress();
    NetworkService* pNetworkService = NetworkService::GetNetworkService();

    for (IMS_UINT32 i = 0; i < nMaxCount; i++)
    {
        nUePort++;
        if (nUePort >= nUpperPort)
        {
            nUePort = nLowerPort;
        }

        if (pNetworkService->CheckIpAndPortAvailability(objUeIPA, nUePort, ISocket::TYPE_STREAM) &&
                pNetworkService->CheckIpAndPortAvailability(objUeIPA, nUePort, ISocket::TYPE_DGRAM))
        {
            return nUePort;
        }
    }

    A_IMS_TRACE_D(REGID, "No valid UE port in range [%u, %u)", nLowerPort, nUpperPort, 0);
    return nUePort;
}

PRIVATE
void AosIpsecHelper::DeleteSamePolicy()
{
    if (m_pOldIpsec == IMS_NULL || m_pNewIpsec == IMS_NULL)
    {
        return;
    }

    if (m_pOldIpsec->GetPcscfPort(AosIpsec::TYPE_CLIENT) ==
            m_pNewIpsec->GetPcscfPort(AosIpsec::TYPE_CLIENT))
    {
        A_IMS_TRACE_I(REGID, "remove same IPSec policy", 0, 0, 0);
        CloseSecureTCPSocket(m_pOldIpsec);
        delete m_pOldIpsec;
        m_pOldIpsec = IMS_NULL;
    }
}
