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
    AosIpsecHelper* m_pAosIpsecHelper;

    MockIRegContact m_objMockIRegContact;
    MockIRegParameter m_objMockIRegParameter;
    AosStaticProfile* m_pAosStaticProfile;
    MockAosAppContext* m_pMockAosAppContext;
    AString* pRegId;

    IAosNConfiguration* m_pOriginAosNConfiguration;
    MockIAosNConfiguration m_objMockAosConfig;

    MockIAosIpsecListener m_objIAosIpsecListener;
    AosIpsec* m_pAosOldIpsec;
    AosIpsec* m_pAosCurrIpsec;
    AosIpsec* m_pAosNewIpsec;

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
        TYPE_SERVER = 1,
        TYPE_CLIENT
    };

protected:
    virtual void SetUp() override
    {
        m_pAosStaticProfile = new AosStaticProfile();
        m_pMockAosAppContext = new MockAosAppContext(m_pAosStaticProfile);
        EXPECT_CALL(*m_pMockAosAppContext, GetSlotId()).WillRepeatedly(Return(SLOT_ID));

        pRegId = new AString(strRegId);

        m_pAosIpsecHelper = new AosIpsecHelper(static_cast<IRegContact*>(&m_objMockIRegContact),
                static_cast<IRegParameter*>(&m_objMockIRegParameter),
                static_cast<IAosAppContext*>(m_pMockAosAppContext), *pRegId);
        ASSERT_TRUE(m_pAosIpsecHelper != nullptr);
        m_pAosIpsecHelper->InitIpsec();

        m_pOriginAosNConfiguration = AosProvider::GetInstance()->GetNConfiguration();
        AosProvider::GetInstance()->SetNConfiguration(
                static_cast<IAosNConfiguration*>(&m_objMockAosConfig), SLOT_ID);

        m_pAosOldIpsec =
                new AosIpsec(static_cast<IAosIpsecListener*>(&m_objIAosIpsecListener), SLOT_ID);
        m_pAosCurrIpsec =
                new AosIpsec(static_cast<IAosIpsecListener*>(&m_objIAosIpsecListener), SLOT_ID);
        m_pAosNewIpsec =
                new AosIpsec(static_cast<IAosIpsecListener*>(&m_objIAosIpsecListener), SLOT_ID);
    }

    virtual void TearDown() override
    {
        AosProvider::GetInstance()->SetNConfiguration(m_pOriginAosNConfiguration, SLOT_ID);

        SetIpsec(OLD_IPSEC, IMS_NULL);
        SetIpsec(CURR_IPSEC, IMS_NULL);
        SetIpsec(NEW_IPSEC, IMS_NULL);

        if (m_pAosOldIpsec)
        {
            delete m_pAosOldIpsec;
        }
        if (m_pAosCurrIpsec)
        {
            delete m_pAosCurrIpsec;
        }
        if (m_pAosNewIpsec)
        {
            delete m_pAosNewIpsec;
        }

        if (m_pAosIpsecHelper)
        {
            delete m_pAosIpsecHelper;
        }

        if (m_pMockAosAppContext)
        {
            delete m_pMockAosAppContext;
        }

        if (m_pAosStaticProfile)
        {
            delete m_pAosStaticProfile;
        }

        if (pRegId)
        {
            delete pRegId;
        }
    }

    void SetRegContact(IN IRegContact* piRegContact)
    {
        m_pAosIpsecHelper->m_piRegContact = piRegContact;
    }

    void SetRegParameter(IN IRegParameter* piRegParameter)
    {
        m_pAosIpsecHelper->m_piRegParameter = piRegParameter;
    }

    void CallIPSecPolicyExpired(IN AosIpsec* pAosIpsec)
    {
        m_pAosIpsecHelper->IPSecPolicyExpired(pAosIpsec);
    }

    void SetUeIpsecInfo(IN IMS_SINT32 nUeInfo, IN IMS_SINT32 nWhere, IN IMS_UINT32 nValue)
    {
        if (nUeInfo == SPI)
        {
            if (nWhere == TYPE_CLIENT)
            {
                m_pAosIpsecHelper->m_pUeIpsecInfo->nSpiC = nValue;
            }
            else
            {
                m_pAosIpsecHelper->m_pUeIpsecInfo->nSpiS = nValue;
            }
        }
        else
        {
            if (nWhere == TYPE_CLIENT)
            {
                m_pAosIpsecHelper->m_pUeIpsecInfo->nPortC = nValue;
            }
            else
            {
                m_pAosIpsecHelper->m_pUeIpsecInfo->nPortS = nValue;
            }
        }
    }

    void SetIpsec(IN IMS_SINT32 nType, IN AosIpsec* pAosIpsec)
    {
        if (nType == NEW_IPSEC)
        {
            m_pAosIpsecHelper->m_pNewIpsec = pAosIpsec;
        }
        else if (nType == CURR_IPSEC)
        {
            m_pAosIpsecHelper->m_pCurrIpsec = pAosIpsec;
        }
        else  // OLD_IPSEC
        {
            m_pAosIpsecHelper->m_pOldIpsec = pAosIpsec;
        }
    }

    AosIpsec* GetIpsec(IN IMS_SINT32 nType)
    {
        if (nType == NEW_IPSEC)
        {
            return m_pAosIpsecHelper->m_pNewIpsec;
        }
        else if (nType == CURR_IPSEC)
        {
            return m_pAosIpsecHelper->m_pCurrIpsec;
        }
        else  // OLD_IPSEC
        {
            return m_pAosIpsecHelper->m_pOldIpsec;
        }
    }

    void SetIpsecSaEstablished(IN IMS_SINT32 nType, IN IMS_BOOL bSaEstablished)
    {
        if (nType == NEW_IPSEC)
        {
            if (bSaEstablished)
            {
                m_pAosIpsecHelper->m_pNewIpsec->SetSaEstablished();
            }
            else
            {
                m_pAosIpsecHelper->m_pNewIpsec->m_bSaEstablished = IMS_FALSE;
            }
        }
        else if (nType == CURR_IPSEC)
        {
            if (bSaEstablished)
            {
                m_pAosIpsecHelper->m_pCurrIpsec->SetSaEstablished();
            }
            else
            {
                m_pAosIpsecHelper->m_pCurrIpsec->m_bSaEstablished = IMS_FALSE;
            }
        }
        else
        {
            if (bSaEstablished)
            {
                m_pAosIpsecHelper->m_pOldIpsec->SetSaEstablished();
            }
            else
            {
                m_pAosIpsecHelper->m_pOldIpsec->m_bSaEstablished = IMS_FALSE;
            }
        }
    }

    IIpSecPolicy* GetIpsecPolicy(IN IMS_SINT32 nType)
    {
        if (nType == NEW_IPSEC)
        {
            return m_pAosIpsecHelper->m_pNewIpsec->m_piPolicy;
        }
        else if (nType == CURR_IPSEC)
        {
            return m_pAosIpsecHelper->m_pCurrIpsec->m_piPolicy;
        }
        else
        {
            return m_pAosIpsecHelper->m_pOldIpsec->m_piPolicy;
        }
    }

    void SetIIpsecPolicy(IN IMS_SINT32 nType, IN IIpSecPolicy* piPolicy)
    {
        if (nType == NEW_IPSEC)
        {
            m_pAosIpsecHelper->m_pNewIpsec->m_piPolicy = piPolicy;
        }
        else if (nType == CURR_IPSEC)
        {
            m_pAosIpsecHelper->m_pCurrIpsec->m_piPolicy = piPolicy;
        }
        else
        {
            m_pAosIpsecHelper->m_pOldIpsec->m_piPolicy = piPolicy;
        }
    }

    INetworkIpSec* GetIpsecNetIpsec(IN IMS_SINT32 nType)
    {
        if (nType == NEW_IPSEC)
        {
            return m_pAosIpsecHelper->m_pNewIpsec->m_piNetIpsec;
        }
        else if (nType == CURR_IPSEC)
        {
            return m_pAosIpsecHelper->m_pCurrIpsec->m_piNetIpsec;
        }
        else
        {
            return m_pAosIpsecHelper->m_pOldIpsec->m_piNetIpsec;
        }
    }

    void SetIIpsecNetIpsec(IN IMS_SINT32 nType, IN INetworkIpSec* piNetIpsec)
    {
        if (nType == NEW_IPSEC)
        {
            m_pAosIpsecHelper->m_pNewIpsec->m_piNetIpsec = piNetIpsec;
        }
        else if (nType == CURR_IPSEC)
        {
            m_pAosIpsecHelper->m_pCurrIpsec->m_piNetIpsec = piNetIpsec;
        }
        else  // OLD_IPSEC
        {
            m_pAosIpsecHelper->m_pOldIpsec->m_piNetIpsec = piNetIpsec;
        }
    }

    void SetPcscf(IN IMS_SINT32 nType, IN IMS_SINT32 nPcscfInfo, IN IMS_SINT32 nWhere,
            IN IMS_UINT32 nValue)
    {
        if (nType == NEW_IPSEC)
        {
            if (nPcscfInfo == PORT)
            {
                if (nWhere == TYPE_CLIENT)
                {
                    m_pAosIpsecHelper->m_pNewIpsec->m_pPcscfInfo->nPortC = nValue;
                }
                else  // TYPE_SERVER
                {
                    m_pAosIpsecHelper->m_pNewIpsec->m_pPcscfInfo->nPortS = nValue;
                }
            }
            else  // SPI
            {
                if (nWhere == TYPE_CLIENT)
                {
                    m_pAosIpsecHelper->m_pNewIpsec->m_pPcscfInfo->nSpiC = nValue;
                }
                else  // TYPE_SERVER
                {
                    m_pAosIpsecHelper->m_pNewIpsec->m_pPcscfInfo->nSpiS = nValue;
                }
            }
        }
        else if (nType == CURR_IPSEC)
        {
            if (nPcscfInfo == PORT)
            {
                if (nWhere == TYPE_CLIENT)
                {
                    m_pAosIpsecHelper->m_pCurrIpsec->m_pPcscfInfo->nPortC = nValue;
                }
                else  // TYPE_SERVER
                {
                    m_pAosIpsecHelper->m_pCurrIpsec->m_pPcscfInfo->nPortS = nValue;
                }
            }
            else  // SPI
            {
                if (nWhere == TYPE_CLIENT)
                {
                    m_pAosIpsecHelper->m_pCurrIpsec->m_pPcscfInfo->nSpiC = nValue;
                }
                else  // TYPE_SERVER
                {
                    m_pAosIpsecHelper->m_pCurrIpsec->m_pPcscfInfo->nSpiS = nValue;
                }
            }
        }
        else  // OLD_IPSEC
        {
            if (nPcscfInfo == PORT)
            {
                if (nWhere == TYPE_CLIENT)
                {
                    m_pAosIpsecHelper->m_pOldIpsec->m_pPcscfInfo->nPortC = nValue;
                }
                else  // TYPE_SERVER
                {
                    m_pAosIpsecHelper->m_pOldIpsec->m_pPcscfInfo->nPortS = nValue;
                }
            }
            else  // SPI
            {
                if (nWhere == TYPE_CLIENT)
                {
                    m_pAosIpsecHelper->m_pOldIpsec->m_pPcscfInfo->nSpiC = nValue;
                }
                else  // TYPE_SERVER
                {
                    m_pAosIpsecHelper->m_pOldIpsec->m_pPcscfInfo->nSpiS = nValue;
                }
            }
        }
    }

    IMS_BOOL GetCurrentIgnorePolicyExpired()
    {
        return m_pAosIpsecHelper->m_pCurrIpsec->m_bIgnorePolicyExpired;
    }

    void DestroyIpsec(IN IMS_SINT32 nType)
    {
        if (nType == NEW_IPSEC)
        {
            delete m_pAosIpsecHelper->m_pNewIpsec;
        }
        else if (nType == CURR_IPSEC)
        {
            delete m_pAosIpsecHelper->m_pCurrIpsec;
        }
        else
        {
            delete m_pAosIpsecHelper->m_pOldIpsec;
        }
        SetIpsec(nType, IMS_NULL);
    }

    void CallDestroy() { m_pAosIpsecHelper->Destroy(); }
};

TEST_F(AosIpsecHelperTest, Create)
{
    SetRegContact(IMS_NULL);
    SetRegParameter(IMS_NULL);
    EXPECT_FALSE(m_pAosIpsecHelper->Create(IMS_TRUE));

    SetRegContact(static_cast<IRegContact*>(&m_objMockIRegContact));
    SetRegParameter(static_cast<IRegParameter*>(&m_objMockIRegParameter));

    // SetUePortnSpi() - GetValidUePort()
    IpAddress objIpAddr(IPADDR1);
    EXPECT_CALL(m_objMockIRegContact, GetIpAddress()).Times(3).WillRepeatedly(ReturnRef(objIpAddr));

    EXPECT_CALL(m_objMockIRegContact, SetPort(_)).Times(AnyNumber());

    // SetSecurityClientHeader() return IMS_FALSE;
    ImsVector<IMS_SINT32> objAuthenticationAlgs;
    objAuthenticationAlgs.Clear();
    EXPECT_CALL(m_objMockAosConfig, GetIpsecAuthenticationAlgorithms())
            .Times(AnyNumber())
            .WillOnce(ReturnRef(objAuthenticationAlgs));
    EXPECT_FALSE(m_pAosIpsecHelper->Create(IMS_TRUE));
    DestroyIpsec(NEW_IPSEC);

    // SetSecurityClientHeader() return IMS_TRUE;
    objAuthenticationAlgs.Clear();
    objAuthenticationAlgs.Add(0);
    objAuthenticationAlgs.Add(1);

    EXPECT_CALL(m_objMockAosConfig, GetIpsecAuthenticationAlgorithms())
            .Times(AnyNumber())
            .WillRepeatedly(ReturnRef(objAuthenticationAlgs));

    ImsVector<IMS_SINT32> objEncryptionAlgs;
    objEncryptionAlgs.Clear();
    objEncryptionAlgs.Add(0);
    objEncryptionAlgs.Add(1);
    objEncryptionAlgs.Add(2);

    EXPECT_CALL(m_objMockAosConfig, GetIpsecEncryptionAlgorithms())
            .Times(2)
            .WillRepeatedly(ReturnRef(objEncryptionAlgs));

    EXPECT_CALL(m_objMockIRegParameter, RemoveSecurityClients()).Times(2);
    EXPECT_CALL(m_objMockIRegParameter, AddSecurityClient(_)).Times(12);

    // SetSecurityServerPortInRegistration()
    EXPECT_CALL(m_objMockAosConfig, IsSecurityServerPortInInitRegUsed())
            .Times(2)
            .WillRepeatedly(Return(IMS_TRUE));

    EXPECT_CALL(m_objMockIRegParameter, SetPort(_)).Times(2);

    // SetSecurityServerPortInRegContact - call if bInitial == IMS_TRUE
    EXPECT_CALL(m_objMockAosConfig, IsSecurityServerPortInRegContactOfInitRegUsed())
            .Times(1)
            .WillOnce(Return(IMS_TRUE));

    // create m_pNewIpsec
    EXPECT_TRUE(m_pAosIpsecHelper->Create(IMS_TRUE));
    DestroyIpsec(NEW_IPSEC);

    EXPECT_TRUE(m_pAosIpsecHelper->Create(IMS_FALSE));

    DestroyIpsec(NEW_IPSEC);
}

TEST_F(AosIpsecHelperTest, CreateOnChallenging)
{
    AosIpsec* pAosTestIpsec =
            new AosIpsec(static_cast<IAosIpsecListener*>(&m_objIAosIpsecListener), SLOT_ID);
    SetIpsec(NEW_IPSEC, pAosTestIpsec);
    AosIpsec* pAosTestIpsec2 =
            new AosIpsec(static_cast<IAosIpsecListener*>(&m_objIAosIpsecListener), SLOT_ID);
    SetIpsec(OLD_IPSEC, pAosTestIpsec2);

    SetIpsec(CURR_IPSEC, IMS_NULL);

    SetUeIpsecInfo(PORT, TYPE_CLIENT, 38002);
    SetUeIpsecInfo(PORT, TYPE_SERVER, 39002);
    SetUeIpsecInfo(SPI, TYPE_CLIENT, 12345678);
    SetUeIpsecInfo(SPI, TYPE_SERVER, 87654321);

    m_pAosIpsecHelper->CreateOnChallenging();

    EXPECT_EQ(GetIpsec(NEW_IPSEC)->GetUePort(AosIpsec::TYPE_CLIENT), 38002);
    EXPECT_EQ(GetIpsec(NEW_IPSEC)->GetUePort(AosIpsec::TYPE_SERVER), 39002);
    EXPECT_EQ(GetIpsec(NEW_IPSEC)->GetUeSpi(AosIpsec::TYPE_CLIENT), 12345678);
    EXPECT_EQ(GetIpsec(NEW_IPSEC)->GetUeSpi(AosIpsec::TYPE_SERVER), 87654321);

    DestroyIpsec(NEW_IPSEC);
}

TEST_F(AosIpsecHelperTest, IsPcscfServerPortDifferent)
{
    SetIpsec(NEW_IPSEC, IMS_NULL);
    EXPECT_FALSE(m_pAosIpsecHelper->IsPcscfServerPortDifferent());

    SetIpsec(NEW_IPSEC, m_pAosNewIpsec);
    SetIpsec(CURR_IPSEC, IMS_NULL);
    EXPECT_FALSE(m_pAosIpsecHelper->IsPcscfServerPortDifferent());

    SetIpsec(CURR_IPSEC, m_pAosCurrIpsec);

    SetPcscf(CURR_IPSEC, PORT, TYPE_SERVER, 39002);
    SetPcscf(NEW_IPSEC, PORT, TYPE_SERVER, 39002);

    EXPECT_FALSE(m_pAosIpsecHelper->IsPcscfServerPortDifferent());

    SetPcscf(NEW_IPSEC, PORT, TYPE_SERVER, 39004);
    EXPECT_TRUE(m_pAosIpsecHelper->IsPcscfServerPortDifferent());
}

TEST_F(AosIpsecHelperTest, UpdatePreloadedRoute)
{
    AString strPcscf = IPADDR1;

    SetIpsec(NEW_IPSEC, IMS_NULL);
    EXPECT_FALSE(m_pAosIpsecHelper->UpdatePreloadedRoute(strPcscf));

    SetIpsec(NEW_IPSEC, m_pAosNewIpsec);
    SetPcscf(NEW_IPSEC, PORT, TYPE_SERVER, 39002);
    EXPECT_CALL(m_objMockIRegParameter, RemoveAllPreloadedRoutes()).Times(1);
    EXPECT_CALL(m_objMockIRegParameter, AddPreloadedRoute(strPcscf, 39002, AString::ConstNull()))
            .Times(1);
    EXPECT_TRUE(m_pAosIpsecHelper->UpdatePreloadedRoute(strPcscf));
}

TEST_F(AosIpsecHelperTest, MakeSas)
{
    SetIpsec(NEW_IPSEC, m_pAosNewIpsec);

    AString strPcscf = IPADDR1;
    IpAddress objIpa(IPADDR2);

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

    EXPECT_FALSE(m_pAosIpsecHelper->MakeSas(strPcscf, objIpa, objIk, objCk));

    EXPECT_CALL(m_objMockIRegContact, SetPort(_)).Times(1);

    EXPECT_TRUE(m_pAosIpsecHelper->MakeSas(strPcscf, objIpa, objIk, objCk));

    // return to origin variable
    SetIIpsecPolicy(NEW_IPSEC, origpiPolicy);
    SetIIpsecNetIpsec(NEW_IPSEC, origpiNetIpsec);
}

TEST_F(AosIpsecHelperTest, ProcessAuthChallenged)
{
    ImsList<SipSecurityHeader> objSecuServerHs;
    objSecuServerHs.Clear();

    EXPECT_CALL(m_objMockIRegParameter, GetSecurityServers()).WillOnce(ReturnRef(objSecuServerHs));

    EXPECT_FALSE(m_pAosIpsecHelper->ProcessAuthChallenged(Credential::TYPE_AKAv1_MD5));

    // compare to Algorithm
    SipSecurityHeader objSecurityH;
    objSecuServerHs.Append(objSecurityH);
    EXPECT_CALL(m_objMockIRegParameter, GetSecurityServers())
            .WillRepeatedly(ReturnRef(objSecuServerHs));

    EXPECT_FALSE(m_pAosIpsecHelper->ProcessAuthChallenged(Credential::TYPE_AKAv2_MD5));

    // check bOldSaRemovedOnEstablisingSa
    EXPECT_CALL(m_objMockAosConfig, IsOldSaOnEstablishingSaRemoved)
            .WillOnce(Return(IMS_FALSE))
            .WillOnce(Return(IMS_TRUE));
    EXPECT_TRUE(m_pAosIpsecHelper->ProcessAuthChallenged(Credential::TYPE_AKAv1_MD5));

    AosIpsec* pAosTestIpsec =
            new AosIpsec(static_cast<IAosIpsecListener*>(&m_objIAosIpsecListener), SLOT_ID);
    SetIpsec(OLD_IPSEC, pAosTestIpsec);
    EXPECT_TRUE(m_pAosIpsecHelper->ProcessAuthChallenged(Credential::TYPE_AKAv1_MD5));
}

TEST_F(AosIpsecHelperTest, ProcessRegStarted)
{
    AosIpsec* pAosTestIpsec =
            new AosIpsec(static_cast<IAosIpsecListener*>(&m_objIAosIpsecListener), SLOT_ID);
    SetIpsec(OLD_IPSEC, pAosTestIpsec);
    SetIpsec(CURR_IPSEC, m_pAosOldIpsec);
    SetIpsec(NEW_IPSEC, m_pAosCurrIpsec);

    EXPECT_CALL(m_objMockIRegContact, GetExpires()).Times(3).WillRepeatedly(Return(1200));

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
    // bUnsecureTcpSocketDestroyed = IMS_FALSE
    EXPECT_CALL(m_objMockAosConfig, IsUnsecureTcpSocketOnAccomplishingRegDestroyed())
            .Times(3)
            .WillOnce(Return(IMS_FALSE))
            .WillRepeatedly(Return(IMS_TRUE));

    m_pAosIpsecHelper->ProcessRegStarted();

    // return to origin variable
    SetIIpsecPolicy(CURR_IPSEC, origpiPolicy);
    SetIIpsecNetIpsec(CURR_IPSEC, origpiNetIpsec);

    // For 2nd ProcessRegStarted()
    SetIpsec(OLD_IPSEC, IMS_NULL);
    SetIpsec(CURR_IPSEC, m_pAosOldIpsec);
    SetIpsec(NEW_IPSEC, m_pAosCurrIpsec);

    // bUnsecureTcpSocketDestroyed = IMS_TRUE
    MockIAosPcscf objMockIAosPcscf;
    EXPECT_CALL(objMockIAosPcscf, GetCurrentPcscf(_, _))
            .Times(2)
            .WillOnce(Return(IMS_FALSE))
            .WillOnce(Return(IMS_TRUE));

    EXPECT_CALL(*m_pMockAosAppContext, GetPcscf())
            .Times(2)
            .WillRepeatedly(Return(&objMockIAosPcscf));

    MockIAosConnection objMockIAosConnection;
    EXPECT_CALL(*m_pMockAosAppContext, GetConnection())
            .Times(1)
            .WillOnce(Return(&objMockIAosConnection));

    IpAddress ipAddr(ADDRESS1);
    EXPECT_CALL(objMockIAosConnection, GetLocalAddress(_)).Times(1).WillOnce(ReturnRef(ipAddr));

    m_pAosIpsecHelper->ProcessRegStarted();

    // return to origin variable
    SetIIpsecPolicy(CURR_IPSEC, origpiPolicy);
    SetIIpsecNetIpsec(CURR_IPSEC, origpiNetIpsec);

    // For 3rd ProcessRegStarted()
    SetIpsec(OLD_IPSEC, IMS_NULL);
    SetIpsec(CURR_IPSEC, m_pAosOldIpsec);
    SetIpsec(NEW_IPSEC, m_pAosCurrIpsec);

    origpiPolicy = GetIpsecPolicy(NEW_IPSEC);
    SetIIpsecPolicy(NEW_IPSEC, static_cast<IIpSecPolicy*>(&objMockIIpsecPolicy));
    origpiNetIpsec = GetIpsecNetIpsec(NEW_IPSEC);
    SetIIpsecNetIpsec(NEW_IPSEC, static_cast<INetworkIpSec*>(&objMockINetworkIpsec));

    m_pAosIpsecHelper->ProcessRegStarted();

    // return to origin variable
    SetIIpsecPolicy(CURR_IPSEC, origpiPolicy);
    SetIIpsecNetIpsec(CURR_IPSEC, origpiNetIpsec);

    SetIpsec(OLD_IPSEC, IMS_NULL);
    SetIpsec(CURR_IPSEC, IMS_NULL);
    SetIpsec(NEW_IPSEC, IMS_NULL);
}

TEST_F(AosIpsecHelperTest, ProcessRegUpdated)
{
    SetIpsec(NEW_IPSEC, IMS_NULL);
    m_pAosIpsecHelper->ProcessRegUpdated();

    SetIpsec(NEW_IPSEC, m_pAosNewIpsec);
    EXPECT_CALL(m_objMockIRegContact, GetExpires()).Times(1).WillRepeatedly(Return(1200));

    // m_pNewIpsec->IsSaEstablished() == IMS_FALSE
    SetIpsecSaEstablished(NEW_IPSEC, IMS_FALSE);
    AosIpsec* pAosTestIpsec =
            new AosIpsec(static_cast<IAosIpsecListener*>(&m_objIAosIpsecListener), SLOT_ID);
    SetIpsec(NEW_IPSEC, pAosTestIpsec);
    SetIpsec(CURR_IPSEC, m_pAosCurrIpsec);

    IIpSecPolicy* origpiPolicy = GetIpsecPolicy(CURR_IPSEC);
    MockIIpSecPolicy objMockIIpsecPolicy;
    SetIIpsecPolicy(CURR_IPSEC, static_cast<IIpSecPolicy*>(&objMockIIpsecPolicy));
    EXPECT_CALL(objMockIIpsecPolicy, ManageLifetime(_)).Times(1);

    m_pAosIpsecHelper->ProcessRegUpdated();

    // return to origin variable
    SetIIpsecPolicy(CURR_IPSEC, origpiPolicy);
}

TEST_F(AosIpsecHelperTest, ProcessRegUpdated2)
{
    EXPECT_CALL(m_objMockIRegContact, GetExpires()).Times(1).WillRepeatedly(Return(1200));

    AosIpsec* pAosTestIpsec =
            new AosIpsec(static_cast<IAosIpsecListener*>(&m_objIAosIpsecListener), SLOT_ID);
    SetIpsec(OLD_IPSEC, pAosTestIpsec);
    SetIpsec(CURR_IPSEC, m_pAosOldIpsec);
    SetIpsec(NEW_IPSEC, m_pAosCurrIpsec);

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

    m_pAosIpsecHelper->ProcessRegUpdated();

    // return to origin variable
    SetIIpsecPolicy(CURR_IPSEC, origpiPolicy);
    SetIIpsecNetIpsec(CURR_IPSEC, origpiNetIpsec);
}

TEST_F(AosIpsecHelperTest, IsEstablished)
{
    SetIpsec(OLD_IPSEC, IMS_NULL);
    SetIpsec(CURR_IPSEC, IMS_NULL);
    SetIpsec(NEW_IPSEC, IMS_NULL);
    EXPECT_FALSE(m_pAosIpsecHelper->IsEstablished());

    SetIpsec(NEW_IPSEC, m_pAosNewIpsec);
    SetIpsecSaEstablished(NEW_IPSEC, IMS_TRUE);
    EXPECT_TRUE(m_pAosIpsecHelper->IsEstablished());

    SetIpsec(CURR_IPSEC, m_pAosCurrIpsec);
    SetIpsecSaEstablished(CURR_IPSEC, IMS_FALSE);
    EXPECT_TRUE(m_pAosIpsecHelper->IsEstablished());

    SetIpsecSaEstablished(CURR_IPSEC, IMS_TRUE);
    EXPECT_TRUE(m_pAosIpsecHelper->IsEstablished());

    SetIpsec(OLD_IPSEC, m_pAosOldIpsec);
    SetIpsecSaEstablished(OLD_IPSEC, IMS_TRUE);
    EXPECT_TRUE(m_pAosIpsecHelper->IsEstablished());
}

TEST_F(AosIpsecHelperTest, SetSecurityServerPortInRegContact)
{
    EXPECT_CALL(m_objMockAosConfig, IsSecurityServerPortInRegContactOfInitRegUsed())
            .Times(2)
            .WillOnce(Return(IMS_FALSE))
            .WillOnce(Return(IMS_TRUE));
    m_pAosIpsecHelper->SetSecurityServerPortInRegContact();

    SetIpsec(NEW_IPSEC, m_pAosNewIpsec);
    EXPECT_CALL(m_objMockIRegContact, SetPort(_)).Times(1);

    m_pAosIpsecHelper->SetSecurityServerPortInRegContact();
}

TEST_F(AosIpsecHelperTest, IgnoreCurrentPolicyExpired)
{
    SetIpsec(CURR_IPSEC, m_pAosCurrIpsec);
    m_pAosIpsecHelper->IgnoreCurrentPolicyExpired();
    EXPECT_TRUE(GetCurrentIgnorePolicyExpired());
}

TEST_F(AosIpsecHelperTest, SetPcscfPortnSpi)
{
    ImsList<SipSecurityHeader> objSecuServerH;
    objSecuServerH.Clear();

    EXPECT_CALL(m_objMockIRegParameter, GetSecurityServers()).WillOnce(ReturnRef(objSecuServerH));
    EXPECT_FALSE(m_pAosIpsecHelper->SetPcscfPortnSpi());
}

TEST_F(AosIpsecHelperTest, IPSecPolicyExpired)
{
    CallIPSecPolicyExpired(IMS_NULL);

    AosIpsec* pAosTestIpsec =
            new AosIpsec(static_cast<IAosIpsecListener*>(&m_objIAosIpsecListener), SLOT_ID);

    SetIpsec(OLD_IPSEC, IMS_NULL);
    SetIpsec(CURR_IPSEC, IMS_NULL);
    SetIpsec(NEW_IPSEC, IMS_NULL);
    CallIPSecPolicyExpired(pAosTestIpsec);
}

TEST_F(AosIpsecHelperTest, Destroy)
{
    AosIpsec* pAosTestOldIpsec =
            new AosIpsec(static_cast<IAosIpsecListener*>(&m_objIAosIpsecListener), SLOT_ID);
    AosIpsec* pAosTestCurrIpsec =
            new AosIpsec(static_cast<IAosIpsecListener*>(&m_objIAosIpsecListener), SLOT_ID);
    AosIpsec* pAosTestNewIpsec =
            new AosIpsec(static_cast<IAosIpsecListener*>(&m_objIAosIpsecListener), SLOT_ID);
    SetIpsec(OLD_IPSEC, pAosTestOldIpsec);
    SetIpsec(CURR_IPSEC, pAosTestCurrIpsec);
    SetIpsec(NEW_IPSEC, pAosTestNewIpsec);

    CallDestroy();
}