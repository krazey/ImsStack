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

#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include "app/MockAosAppContext.h"
#include "interface/MockIAosConnection.h"
#include "interface/MockIAosNConfiguration.h"
#include "interface/MockIAosPcscf.h"
#include "registration/MockAosIpsec.h"
#include "../../../engine/interface/registration/MockIRegContact.h"
#include "../../../engine/interface/registration/MockIRegParameter.h"
#include "../../../platform/interface/MockIIpSecPolicy.h"
#include "../../../platform/interface/MockIIpSecSa.h"
#include "../../../platform/interface/MockIIpSecSp.h"
#include "../../../platform/interface/MockINetworkIpSec.h"

#include "Credential.h"
#include "ImsIpSecType.h"
#include "provider/AosProvider.h"
#include "registration/AosIpsecHelper.h"

using ::testing::_;
using ::testing::AnyNumber;
using ::testing::Return;
using ::testing::ReturnRef;

const IMS_SINT32 SLOT_ID = 0;

class AosIpsecHelperTest : public ::testing::Test
{
public:
    AosIpsecHelper* pAosIpsecHelper;

    MockIRegContact objMockIRegContact;
    MockIRegParameter objMockIRegParameter;
    AosStaticProfile* pAosStaticProfile;
    MockAosAppContext* pMockAosAppContext;
    AString* pRegId;

    IAosNConfiguration* pOriginAosNConfiguration;
    MockIAosNConfiguration objMockAosConfig;

    MockIAosIpsecListener objIAosIpsecListener;
    AosIpsec* pAosOldIpsec;
    AosIpsec* pAosCurrIpsec;
    AosIpsec* pAosNewIpsec;

    const AString strRegId = "aos_normal_reg";
    const AString ADDRESS1 = "sip:1234@ims.google.com:5060";

    const AString IPADDR1 = "10.168.219.102";
    const AString IPADDR2 = "10.168.219.104";

    enum
    {
        OLD_IPSEC = 0,
        CURR_IPSEC,
        NEW_IPSEC
    };

    enum
    {
        PORT = 1,
        SPI
    };

    enum
    {
        SERVER = 1,
        CLIENT
    };

protected:
    virtual void SetUp() override
    {
        pAosStaticProfile = new AosStaticProfile();
        pMockAosAppContext = new MockAosAppContext(pAosStaticProfile);
        EXPECT_CALL(*pMockAosAppContext, GetSlotId()).WillRepeatedly(Return(SLOT_ID));

        pRegId = new AString(strRegId);

        pAosIpsecHelper = new AosIpsecHelper(static_cast<IRegContact*>(&objMockIRegContact),
                static_cast<IRegParameter*>(&objMockIRegParameter),
                static_cast<IAosAppContext*>(pMockAosAppContext), *pRegId);
        ASSERT_TRUE(pAosIpsecHelper != nullptr);
        pAosIpsecHelper->InitIpsec();

        pOriginAosNConfiguration = AosProvider::GetInstance()->GetNConfiguration();
        AosProvider::GetInstance()->SetNConfiguration(
                static_cast<IAosNConfiguration*>(&objMockAosConfig), SLOT_ID);

        pAosOldIpsec =
                new AosIpsec(static_cast<IAosIpsecListener*>(&objIAosIpsecListener), SLOT_ID);
        pAosCurrIpsec =
                new AosIpsec(static_cast<IAosIpsecListener*>(&objIAosIpsecListener), SLOT_ID);
        pAosNewIpsec =
                new AosIpsec(static_cast<IAosIpsecListener*>(&objIAosIpsecListener), SLOT_ID);
    }

    virtual void TearDown() override
    {
        AosProvider::GetInstance()->SetNConfiguration(pOriginAosNConfiguration, SLOT_ID);

        SetIpsec(OLD_IPSEC, IMS_NULL);
        SetIpsec(CURR_IPSEC, IMS_NULL);
        SetIpsec(NEW_IPSEC, IMS_NULL);

        if (pAosOldIpsec)
        {
            delete pAosOldIpsec;
        }
        if (pAosCurrIpsec)
        {
            delete pAosCurrIpsec;
        }
        if (pAosNewIpsec)
        {
            delete pAosNewIpsec;
        }

        if (pRegId)
        {
            delete pRegId;
        }

        if (pAosStaticProfile)
        {
            delete pAosStaticProfile;
        }

        if (pMockAosAppContext)
        {
            delete pMockAosAppContext;
        }

        if (pAosIpsecHelper)
        {
            delete pAosIpsecHelper;
        }
    }

    void SetRegContact(IN IRegContact* piRegContact)
    {
        pAosIpsecHelper->m_piRegContact = piRegContact;
    }

    void SetRegParameter(IN IRegParameter* piRegParameter)
    {
        pAosIpsecHelper->m_piRegParameter = piRegParameter;
    }

    void CallIPSecPolicyExpired(IN AosIpsec* pAosIpsec)
    {
        pAosIpsecHelper->IPSecPolicyExpired(pAosIpsec);
    }

    void SetIpsec(IN IMS_SINT32 nType, IN AosIpsec* pAosIpsec)
    {
        if (nType == NEW_IPSEC)
        {
            pAosIpsecHelper->m_pNewIpsec = pAosIpsec;
        }
        else if (nType == CURR_IPSEC)
        {
            pAosIpsecHelper->m_pCurrIpsec = pAosIpsec;
        }
        else  // OLD_IPSEC
        {
            pAosIpsecHelper->m_pOldIpsec = pAosIpsec;
        }
    }

    void SetIpsecSaEstablished(IN IMS_SINT32 nType, IN IMS_BOOL bSaEstablished)
    {
        if (nType == NEW_IPSEC)
        {
            if (bSaEstablished)
            {
                pAosIpsecHelper->m_pNewIpsec->SetSaEstablished();
            }
            else
            {
                pAosIpsecHelper->m_pNewIpsec->m_bSaEstablished = IMS_FALSE;
            }
        }
        else if (nType == CURR_IPSEC)
        {
            if (bSaEstablished)
            {
                pAosIpsecHelper->m_pCurrIpsec->SetSaEstablished();
            }
            else
            {
                pAosIpsecHelper->m_pCurrIpsec->m_bSaEstablished = IMS_FALSE;
            }
        }
        else
        {
            if (bSaEstablished)
            {
                pAosIpsecHelper->m_pOldIpsec->SetSaEstablished();
            }
            else
            {
                pAosIpsecHelper->m_pOldIpsec->m_bSaEstablished = IMS_FALSE;
            }
        }
    }

    IIpSecPolicy* GetIpsecPolicy(IN IMS_SINT32 nType)
    {
        if (nType == NEW_IPSEC)
        {
            return pAosIpsecHelper->m_pNewIpsec->m_piPolicy;
        }
        else if (nType == CURR_IPSEC)
        {
            return pAosIpsecHelper->m_pCurrIpsec->m_piPolicy;
        }
        else
        {
            return pAosIpsecHelper->m_pOldIpsec->m_piPolicy;
        }
    }

    void SetIIpsecPolicy(IN IMS_SINT32 nType, IN IIpSecPolicy* piPolicy)
    {
        if (nType == NEW_IPSEC)
        {
            pAosIpsecHelper->m_pNewIpsec->m_piPolicy = piPolicy;
        }
        else if (nType == CURR_IPSEC)
        {
            pAosIpsecHelper->m_pCurrIpsec->m_piPolicy = piPolicy;
        }
        else
        {
            pAosIpsecHelper->m_pOldIpsec->m_piPolicy = piPolicy;
        }
    }

    INetworkIpSec* GetIpsecNetIpsec(IN IMS_SINT32 nType)
    {
        if (nType == NEW_IPSEC)
        {
            return pAosIpsecHelper->m_pNewIpsec->m_piNetIpsec;
        }
        else if (nType == CURR_IPSEC)
        {
            return pAosIpsecHelper->m_pCurrIpsec->m_piNetIpsec;
        }
        else
        {
            return pAosIpsecHelper->m_pOldIpsec->m_piNetIpsec;
        }
    }

    void SetIIpsecNetIpsec(IN IMS_SINT32 nType, IN INetworkIpSec* piNetIpsec)
    {
        if (nType == NEW_IPSEC)
        {
            pAosIpsecHelper->m_pNewIpsec->m_piNetIpsec = piNetIpsec;
        }
        else if (nType == CURR_IPSEC)
        {
            pAosIpsecHelper->m_pCurrIpsec->m_piNetIpsec = piNetIpsec;
        }
        else  // OLD_IPSEC
        {
            pAosIpsecHelper->m_pOldIpsec->m_piNetIpsec = piNetIpsec;
        }
    }

    void SetPcscf(IN IMS_SINT32 nType, IN IMS_SINT32 nPcscfInfo, IN IMS_SINT32 nWhere,
            IN IMS_UINT32 nValue)
    {
        if (nType == NEW_IPSEC)
        {
            if (nPcscfInfo == PORT)
            {
                if (nWhere == CLIENT)
                {
                    pAosIpsecHelper->m_pNewIpsec->m_pPcscfInfo->nPortC = nValue;
                }
                else  // SERVER
                {
                    pAosIpsecHelper->m_pNewIpsec->m_pPcscfInfo->nPortS = nValue;
                }
            }
            else  // SPI
            {
                if (nWhere == CLIENT)
                {
                    pAosIpsecHelper->m_pNewIpsec->m_pPcscfInfo->nSpiC = nValue;
                }
                else  // SERVER
                {
                    pAosIpsecHelper->m_pNewIpsec->m_pPcscfInfo->nSpiS = nValue;
                }
            }
        }
        else if (nType == CURR_IPSEC)
        {
            if (nPcscfInfo == PORT)
            {
                if (nWhere == CLIENT)
                {
                    pAosIpsecHelper->m_pCurrIpsec->m_pPcscfInfo->nPortC = nValue;
                }
                else  // SERVER
                {
                    pAosIpsecHelper->m_pCurrIpsec->m_pPcscfInfo->nPortS = nValue;
                }
            }
            else  // SPI
            {
                if (nWhere == CLIENT)
                {
                    pAosIpsecHelper->m_pCurrIpsec->m_pPcscfInfo->nSpiC = nValue;
                }
                else  // SERVER
                {
                    pAosIpsecHelper->m_pCurrIpsec->m_pPcscfInfo->nSpiS = nValue;
                }
            }
        }
        else  // OLD_IPSEC
        {
            if (nPcscfInfo == PORT)
            {
                if (nWhere == CLIENT)
                {
                    pAosIpsecHelper->m_pOldIpsec->m_pPcscfInfo->nPortC = nValue;
                }
                else  // SERVER
                {
                    pAosIpsecHelper->m_pOldIpsec->m_pPcscfInfo->nPortS = nValue;
                }
            }
            else  // SPI
            {
                if (nWhere == CLIENT)
                {
                    pAosIpsecHelper->m_pOldIpsec->m_pPcscfInfo->nSpiC = nValue;
                }
                else  // SERVER
                {
                    pAosIpsecHelper->m_pOldIpsec->m_pPcscfInfo->nSpiS = nValue;
                }
            }
        }
    }

    IMS_BOOL GetCurrentIgnorePolicyExpired()
    {
        return pAosIpsecHelper->m_pCurrIpsec->m_bIgnorePolicyExpired;
    }

    void DestroyIpsec(IN IMS_SINT32 nType)
    {
        if (nType == NEW_IPSEC)
        {
            delete pAosIpsecHelper->m_pNewIpsec;
        }
        else if (nType == CURR_IPSEC)
        {
            delete pAosIpsecHelper->m_pCurrIpsec;
        }
        else
        {
            delete pAosIpsecHelper->m_pOldIpsec;
        }
        SetIpsec(nType, IMS_NULL);
    }

    void CallDestroy() { pAosIpsecHelper->Destroy(); }
};

TEST_F(AosIpsecHelperTest, Create)
{
    SetRegContact(IMS_NULL);
    SetRegParameter(IMS_NULL);
    EXPECT_FALSE(pAosIpsecHelper->Create(IMS_TRUE));

    SetRegContact(static_cast<IRegContact*>(&objMockIRegContact));
    SetRegParameter(static_cast<IRegParameter*>(&objMockIRegParameter));

    // SetUePortnSpi() - GetValidUePort()
    IPAddress objIpAddr(IPADDR1);
    EXPECT_CALL(objMockIRegContact, GetIpAddress()).Times(3).WillRepeatedly(ReturnRef(objIpAddr));

    // SetSecurityClientHeader() return IMS_FALSE;
    IMSVector<IMS_SINT32> objAuthenticationAlgs;
    objAuthenticationAlgs.Clear();
    EXPECT_CALL(objMockAosConfig, GetIpsecAuthenticationAlgorithms())
            .Times(AnyNumber())
            .WillOnce(ReturnRef(objAuthenticationAlgs));
    EXPECT_FALSE(pAosIpsecHelper->Create(IMS_TRUE));

    // SetSecurityClientHeader() return IMS_TRUE;
    objAuthenticationAlgs.Clear();
    objAuthenticationAlgs.Add(0);
    objAuthenticationAlgs.Add(1);

    EXPECT_CALL(objMockAosConfig, GetIpsecAuthenticationAlgorithms())
            .Times(AnyNumber())
            .WillRepeatedly(ReturnRef(objAuthenticationAlgs));

    IMSVector<IMS_SINT32> objEncryptionAlgs;
    objEncryptionAlgs.Clear();
    objEncryptionAlgs.Add(0);
    objEncryptionAlgs.Add(1);
    objEncryptionAlgs.Add(2);

    EXPECT_CALL(objMockAosConfig, GetIpsecEncryptionAlgorithms())
            .Times(2)
            .WillRepeatedly(ReturnRef(objEncryptionAlgs));

    // SetSecurityServerPortInRegistration()
    EXPECT_CALL(objMockAosConfig, IsSecurityServerPortInInitialRegistrationUsed())
            .Times(2)
            .WillOnce(Return(IMS_TRUE));

    // SetSecurityServerPortInRegContact - call if bInitial == IMS_TRUE
    EXPECT_CALL(objMockAosConfig, IsSecurityServerPortInRegContactOfInitialRegistrationUsed())
            .Times(1)
            .WillOnce(Return(IMS_TRUE));

    // create m_pNewIpsec
    EXPECT_TRUE(pAosIpsecHelper->Create(IMS_TRUE));

    EXPECT_TRUE(pAosIpsecHelper->Create(IMS_FALSE));

    DestroyIpsec(NEW_IPSEC);
}

TEST_F(AosIpsecHelperTest, CreateOnChallenging)
{
    AosIpsec* pAosTestIpsec =
            new AosIpsec(static_cast<IAosIpsecListener*>(&objIAosIpsecListener), SLOT_ID);
    SetIpsec(NEW_IPSEC, pAosTestIpsec);
    AosIpsec* pAosTestIpsec2 =
            new AosIpsec(static_cast<IAosIpsecListener*>(&objIAosIpsecListener), SLOT_ID);
    SetIpsec(OLD_IPSEC, pAosTestIpsec2);

    SetIpsec(CURR_IPSEC, IMS_NULL);

    pAosIpsecHelper->CreateOnChallenging();

    DestroyIpsec(NEW_IPSEC);
}

TEST_F(AosIpsecHelperTest, IsPcscfServerPortDifferent)
{
    SetIpsec(NEW_IPSEC, IMS_NULL);
    EXPECT_FALSE(pAosIpsecHelper->IsPcscfServerPortDifferent());

    SetIpsec(NEW_IPSEC, pAosNewIpsec);
    SetIpsec(CURR_IPSEC, IMS_NULL);
    EXPECT_FALSE(pAosIpsecHelper->IsPcscfServerPortDifferent());

    SetIpsec(CURR_IPSEC, pAosCurrIpsec);

    SetPcscf(CURR_IPSEC, PORT, SERVER, 39002);
    SetPcscf(NEW_IPSEC, PORT, SERVER, 39002);

    EXPECT_FALSE(pAosIpsecHelper->IsPcscfServerPortDifferent());

    SetPcscf(NEW_IPSEC, PORT, SERVER, 39004);
    EXPECT_TRUE(pAosIpsecHelper->IsPcscfServerPortDifferent());
}

TEST_F(AosIpsecHelperTest, UpdatePreloadedRoute)
{
    AString strPcscf = IPADDR1;

    SetIpsec(NEW_IPSEC, IMS_NULL);
    EXPECT_FALSE(pAosIpsecHelper->UpdatePreloadedRoute(strPcscf));

    SetIpsec(NEW_IPSEC, pAosNewIpsec);
    SetPcscf(NEW_IPSEC, PORT, SERVER, 39002);
    EXPECT_CALL(objMockIRegParameter, RemoveAllPreloadedRoutes()).Times(1);
    EXPECT_CALL(objMockIRegParameter, AddPreloadedRoute(strPcscf, 39002, AString::ConstNull()))
            .Times(1);
    EXPECT_TRUE(pAosIpsecHelper->UpdatePreloadedRoute(strPcscf));
}

TEST_F(AosIpsecHelperTest, MakeSas)
{
    SetIpsec(NEW_IPSEC, pAosNewIpsec);

    AString strPcscf = IPADDR1;
    IPAddress objIpa(IPADDR2);

    AString strIk = "";
    strIk.Append("integrity key");
    ByteArray objIk(strIk);

    AString strCk = "";
    strIk.Append("integrity key");
    ByteArray objCk(strCk);

    IIpSecPolicy* origpiPolicy = GetIpsecPolicy(NEW_IPSEC);
    MockIIpSecPolicy objMockIIpsecPolicy;
    SetIIpsecPolicy(NEW_IPSEC, static_cast<IIpSecPolicy*>(&objMockIIpsecPolicy));

    MockIIpSecSp objMockIIpSecSp;
    EXPECT_CALL(objMockIIpsecPolicy, CreateSp())
            .Times(12)
            .WillRepeatedly(Return(static_cast<IIpSecSp*>(&objMockIIpSecSp)));

    // UDP - CreateSps(IpSecType::TRANS_PROTOCOL_UDP);
    EXPECT_CALL(objMockIIpSecSp,
            SetTransportInfo(_, _, _, _, IpSecType::TRANS_PROTOCOL_UDP, IpSecType::ACTION_APPLY,
                    IpSecType::DIRECTION_OUTBOUND, _, _))
            .Times(2);
    EXPECT_CALL(objMockIIpSecSp,
            SetTransportInfo(_, _, _, _, IpSecType::TRANS_PROTOCOL_UDP, IpSecType::ACTION_PERMIT,
                    IpSecType::DIRECTION_INBOUND, _, _))
            .Times(2);

    // TCP - CreateSps(IpSecType::TRANS_PROTOCOL_TCP);
    EXPECT_CALL(objMockIIpSecSp,
            SetTransportInfo(_, _, _, _, IpSecType::TRANS_PROTOCOL_TCP, IpSecType::ACTION_APPLY,
                    IpSecType::DIRECTION_OUTBOUND, _, _))
            .Times(4);
    EXPECT_CALL(objMockIIpSecSp,
            SetTransportInfo(_, _, _, _, IpSecType::TRANS_PROTOCOL_TCP, IpSecType::ACTION_PERMIT,
                    IpSecType::DIRECTION_INBOUND, _, _))
            .Times(4);

    // UDP TCP - SA_DIR_US_PC, SA_DIR_UC_PS, SA_DIR_PS_UC, SA_DIR_PC_UC
    EXPECT_CALL(objMockIIpSecSp, SetSecurityAlgorithmInfo(_, _, _)).Times(12);
    EXPECT_CALL(objMockIIpSecSp, DoneSp()).Times(12);

    // CreateSa
    MockIIpSecSa objMockIIpSecSa;
    EXPECT_CALL(objMockIIpsecPolicy, CreateSa())
            .Times(8)
            .WillRepeatedly(Return(static_cast<IIpSecSa*>(&objMockIIpSecSa)));

    EXPECT_CALL(objMockIIpSecSa, SetSa(_, _, _, _, _, _, _, _, _, _, _)).Times(8);
    EXPECT_CALL(objMockIIpSecSa, DoneSa()).Times(8);

    // DeleteSamePolicy - _pOldIpsec == IMS_NULL
    SetIpsec(OLD_IPSEC, IMS_NULL);

    // AddPolicy() - m_piPolicy can't be IMS_NULL
    // m_piPolicy is used in CreateSpforTcp() and CreateSpforUDP
    // SetIIpsecPolicy(NEW_IPSEC, IMS_NULL);
    INetworkIpSec* origpiNetIpsec = GetIpsecNetIpsec(NEW_IPSEC);
    MockINetworkIpSec objMockINetworkIpsec;
    SetIIpsecNetIpsec(NEW_IPSEC, static_cast<INetworkIpSec*>(&objMockINetworkIpsec));
    EXPECT_CALL(objMockINetworkIpsec, AddPolicy(_))
            .Times(2)
            .WillOnce(Return(IMS_FALSE))
            .WillOnce(Return(IMS_TRUE));

    EXPECT_FALSE(pAosIpsecHelper->MakeSas(strPcscf, objIpa, objIk, objCk));

    EXPECT_CALL(objMockIRegContact, SetPort(_)).Times(1);

    EXPECT_TRUE(pAosIpsecHelper->MakeSas(strPcscf, objIpa, objIk, objCk));

    // return to origin variable
    SetIIpsecPolicy(NEW_IPSEC, origpiPolicy);
    SetIIpsecNetIpsec(NEW_IPSEC, origpiNetIpsec);
}

TEST_F(AosIpsecHelperTest, ProcessAuthChallenged)
{
    IMSList<SipSecurityHeader> objSecuServerH;
    objSecuServerH.Clear();

    EXPECT_CALL(objMockIRegParameter, GetSecurityServers()).WillOnce(ReturnRef(objSecuServerH));

    EXPECT_FALSE(pAosIpsecHelper->ProcessAuthChallenged(Credential::TYPE_AKAv1_MD5));
}

TEST_F(AosIpsecHelperTest, ProcessRegStarted)
{
    AosIpsec* pAosTestIpsec =
            new AosIpsec(static_cast<IAosIpsecListener*>(&objIAosIpsecListener), SLOT_ID);
    SetIpsec(OLD_IPSEC, pAosTestIpsec);
    SetIpsec(CURR_IPSEC, pAosOldIpsec);
    SetIpsec(NEW_IPSEC, pAosCurrIpsec);

    EXPECT_CALL(objMockIRegContact, GetExpires()).Times(2).WillRepeatedly(Return(1200));

    // ManagePolicyLifetime()
    IIpSecPolicy* origpiPolicy = GetIpsecPolicy(NEW_IPSEC);
    MockIIpSecPolicy objMockIIpsecPolicy;
    SetIIpsecPolicy(NEW_IPSEC, static_cast<IIpSecPolicy*>(&objMockIIpsecPolicy));
    EXPECT_CALL(objMockIIpsecPolicy, ManageLifetime(_)).Times(2);

    // DumpSas()
    INetworkIpSec* origpiNetIpsec = GetIpsecNetIpsec(NEW_IPSEC);
    MockINetworkIpSec objMockINetworkIpsec;
    SetIIpsecNetIpsec(NEW_IPSEC, static_cast<INetworkIpSec*>(&objMockINetworkIpsec));

    EXPECT_CALL(objMockINetworkIpsec, DumpPolicy(_)).Times(2);

    // CloseUnsecureTCPSocket();
    MockIAosPcscf objMockIAosPcscf;
    EXPECT_CALL(objMockIAosPcscf, GetCurrentPcscf(_, _))
            .Times(2)
            .WillOnce(Return(IMS_FALSE))
            .WillOnce(Return(IMS_TRUE));

    EXPECT_CALL(*pMockAosAppContext, GetPcscf()).Times(2).WillRepeatedly(Return(&objMockIAosPcscf));

    MockIAosConnection objMockIAosConnection;
    EXPECT_CALL(*pMockAosAppContext, GetConnection())
            .Times(1)
            .WillOnce(Return(&objMockIAosConnection));

    IPAddress ipAddr(ADDRESS1);
    EXPECT_CALL(objMockIAosConnection, GetLocalAddress(_)).Times(1).WillOnce(ReturnRef(ipAddr));

    pAosIpsecHelper->ProcessRegStarted();

    // return to origin variable
    SetIIpsecPolicy(CURR_IPSEC, origpiPolicy);
    SetIIpsecNetIpsec(CURR_IPSEC, origpiNetIpsec);

    // For 2nd ProcessRegStarted()
    SetIpsec(OLD_IPSEC, IMS_NULL);
    SetIpsec(CURR_IPSEC, pAosOldIpsec);
    SetIpsec(NEW_IPSEC, pAosCurrIpsec);

    origpiPolicy = GetIpsecPolicy(NEW_IPSEC);
    SetIIpsecPolicy(NEW_IPSEC, static_cast<IIpSecPolicy*>(&objMockIIpsecPolicy));
    origpiNetIpsec = GetIpsecNetIpsec(NEW_IPSEC);
    SetIIpsecNetIpsec(NEW_IPSEC, static_cast<INetworkIpSec*>(&objMockINetworkIpsec));

    pAosIpsecHelper->ProcessRegStarted();

    // return to origin variable
    SetIIpsecPolicy(CURR_IPSEC, origpiPolicy);
    SetIIpsecNetIpsec(CURR_IPSEC, origpiNetIpsec);

    SetIpsec(OLD_IPSEC, IMS_NULL);
    SetIpsec(CURR_IPSEC, IMS_NULL);
    SetIpsec(NEW_IPSEC, IMS_NULL);
}

TEST_F(AosIpsecHelperTest, ProcessRegStartFailed)
{
    // CloseUnsecureTCPSocket();
    MockIAosPcscf objMockIAosPcscf;
    EXPECT_CALL(objMockIAosPcscf, GetCurrentPcscf(_, _)).Times(1).WillOnce(Return(IMS_FALSE));

    EXPECT_CALL(*pMockAosAppContext, GetPcscf()).Times(1).WillOnce(Return(&objMockIAosPcscf));

    pAosIpsecHelper->ProcessRegStartFailed();
}

TEST_F(AosIpsecHelperTest, ProcessRegUpdated)
{
    SetIpsec(NEW_IPSEC, IMS_NULL);
    pAosIpsecHelper->ProcessRegUpdated();

    SetIpsec(NEW_IPSEC, pAosNewIpsec);
    EXPECT_CALL(objMockIRegContact, GetExpires()).Times(1).WillRepeatedly(Return(1200));

    // m_pNewIpsec->IsSaEstablished() == IMS_FALSE
    SetIpsecSaEstablished(NEW_IPSEC, IMS_FALSE);
    AosIpsec* pAosTestIpsec =
            new AosIpsec(static_cast<IAosIpsecListener*>(&objIAosIpsecListener), SLOT_ID);
    SetIpsec(NEW_IPSEC, pAosTestIpsec);
    SetIpsec(CURR_IPSEC, pAosCurrIpsec);

    IIpSecPolicy* origpiPolicy = GetIpsecPolicy(CURR_IPSEC);
    MockIIpSecPolicy objMockIIpsecPolicy;
    SetIIpsecPolicy(CURR_IPSEC, static_cast<IIpSecPolicy*>(&objMockIIpsecPolicy));
    EXPECT_CALL(objMockIIpsecPolicy, ManageLifetime(_)).Times(1);

    pAosIpsecHelper->ProcessRegUpdated();

    // return to origin variable
    SetIIpsecPolicy(CURR_IPSEC, origpiPolicy);
}

TEST_F(AosIpsecHelperTest, ProcessRegUpdated2)
{
    EXPECT_CALL(objMockIRegContact, GetExpires()).Times(1).WillRepeatedly(Return(1200));

    AosIpsec* pAosTestIpsec =
            new AosIpsec(static_cast<IAosIpsecListener*>(&objIAosIpsecListener), SLOT_ID);
    SetIpsec(OLD_IPSEC, pAosTestIpsec);
    SetIpsec(CURR_IPSEC, pAosOldIpsec);
    SetIpsec(NEW_IPSEC, pAosCurrIpsec);

    // m_pNewIpsec->IsSaEstablished() == IMS_TRUE
    SetIpsecSaEstablished(NEW_IPSEC, IMS_TRUE);

    // ManagePolicyLifetime()
    IIpSecPolicy* origpiPolicy = GetIpsecPolicy(NEW_IPSEC);
    MockIIpSecPolicy objMockIIpsecPolicy;
    SetIIpsecPolicy(NEW_IPSEC, static_cast<IIpSecPolicy*>(&objMockIIpsecPolicy));
    EXPECT_CALL(objMockIIpsecPolicy, ManageLifetime(_)).Times(1);

    // DumpSas()
    INetworkIpSec* origpiNetIpsec = GetIpsecNetIpsec(NEW_IPSEC);
    MockINetworkIpSec objMockINetworkIpsec;
    SetIIpsecNetIpsec(NEW_IPSEC, static_cast<INetworkIpSec*>(&objMockINetworkIpsec));
    EXPECT_CALL(objMockINetworkIpsec, DumpPolicy(_)).Times(1);

    pAosIpsecHelper->ProcessRegUpdated();

    // return to origin variable
    SetIIpsecPolicy(CURR_IPSEC, origpiPolicy);
    SetIIpsecNetIpsec(CURR_IPSEC, origpiNetIpsec);
}

TEST_F(AosIpsecHelperTest, IsEstablished)
{
    SetIpsec(OLD_IPSEC, IMS_NULL);
    SetIpsec(CURR_IPSEC, IMS_NULL);
    SetIpsec(NEW_IPSEC, IMS_NULL);
    EXPECT_FALSE(pAosIpsecHelper->IsEstablished());

    SetIpsec(NEW_IPSEC, pAosNewIpsec);
    SetIpsecSaEstablished(NEW_IPSEC, IMS_TRUE);
    EXPECT_TRUE(pAosIpsecHelper->IsEstablished());

    SetIpsec(CURR_IPSEC, pAosCurrIpsec);
    SetIpsecSaEstablished(CURR_IPSEC, IMS_FALSE);
    EXPECT_TRUE(pAosIpsecHelper->IsEstablished());

    SetIpsecSaEstablished(CURR_IPSEC, IMS_TRUE);
    EXPECT_TRUE(pAosIpsecHelper->IsEstablished());

    SetIpsec(OLD_IPSEC, pAosOldIpsec);
    SetIpsecSaEstablished(OLD_IPSEC, IMS_TRUE);
    EXPECT_TRUE(pAosIpsecHelper->IsEstablished());
}

TEST_F(AosIpsecHelperTest, SetSecurityServerPortInRegContact)
{
    EXPECT_CALL(objMockAosConfig, IsSecurityServerPortInRegContactOfInitialRegistrationUsed())
            .Times(2)
            .WillOnce(Return(IMS_FALSE))
            .WillOnce(Return(IMS_TRUE));
    pAosIpsecHelper->SetSecurityServerPortInRegContact();

    SetIpsec(NEW_IPSEC, pAosNewIpsec);
    EXPECT_CALL(objMockIRegContact, SetPort(_)).Times(1);

    pAosIpsecHelper->SetSecurityServerPortInRegContact();
}

TEST_F(AosIpsecHelperTest, IgnoreCurrentPolicyExpired)
{
    SetIpsec(CURR_IPSEC, pAosCurrIpsec);
    pAosIpsecHelper->IgnoreCurrentPolicyExpired();
    EXPECT_TRUE(GetCurrentIgnorePolicyExpired());
}

TEST_F(AosIpsecHelperTest, SetPcscfPortnSpi)
{
    IMSList<SipSecurityHeader> objSecuServerH;
    objSecuServerH.Clear();

    EXPECT_CALL(objMockIRegParameter, GetSecurityServers()).WillOnce(ReturnRef(objSecuServerH));
    EXPECT_FALSE(pAosIpsecHelper->SetPcscfPortnSpi());
}

TEST_F(AosIpsecHelperTest, IPSecPolicyExpired)
{
    CallIPSecPolicyExpired(IMS_NULL);

    AosIpsec* pAosTestIpsec =
            new AosIpsec(static_cast<IAosIpsecListener*>(&objIAosIpsecListener), SLOT_ID);

    SetIpsec(OLD_IPSEC, IMS_NULL);
    SetIpsec(CURR_IPSEC, IMS_NULL);
    SetIpsec(NEW_IPSEC, IMS_NULL);
    CallIPSecPolicyExpired(pAosTestIpsec);
}

TEST_F(AosIpsecHelperTest, Destroy)
{
    AosIpsec* pAosTestOldIpsec =
            new AosIpsec(static_cast<IAosIpsecListener*>(&objIAosIpsecListener), SLOT_ID);
    AosIpsec* pAosTestCurrIpsec =
            new AosIpsec(static_cast<IAosIpsecListener*>(&objIAosIpsecListener), SLOT_ID);
    AosIpsec* pAosTestNewIpsec =
            new AosIpsec(static_cast<IAosIpsecListener*>(&objIAosIpsecListener), SLOT_ID);
    SetIpsec(OLD_IPSEC, pAosTestOldIpsec);
    SetIpsec(CURR_IPSEC, pAosTestCurrIpsec);
    SetIpsec(NEW_IPSEC, pAosTestNewIpsec);

    CallDestroy();
}