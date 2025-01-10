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

#include "interface/MockIAosAppContext.h"
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

// FIXME : It will be fixed when it is confirmed that it is possible to change MockAosIpsec.
class TestAosIpsecEx : public AosIpsec
{
public:
    inline TestAosIpsecEx(IN IAosIpsecListener* piListener, IN IMS_SINT32 nSlotId) :
            AosIpsec(piListener, nSlotId)
    {
    }

    inline void SetSaEstablishedForTest(IN IMS_BOOL bSaEstablished)
    {
        m_bSaEstablished = bSaEstablished;
    }
    inline void SetPolicy(IN IIpSecPolicy* piPolicy) { m_piPolicy = piPolicy; }
    inline INetworkIpSec* GetNetIpsec() { return m_piNetIpsec; }
    inline void SetNetIpsec(IN INetworkIpSec* piNetIpsec) { m_piNetIpsec = piNetIpsec; }
    inline IMS_BOOL GetIgnorePolicyExpired() { return m_bIgnorePolicyExpired; }
};

class TestAosIpsecHelper : public AosIpsecHelper
{
public:
    inline TestAosIpsecHelper(IN IRegContact* piRegContact, IN IRegParameter* piRegParameter,
            IN IAosAppContext* piAppContext, IN AString& strRegId) :
            AosIpsecHelper(piRegContact, piRegParameter, piAppContext, strRegId)
    {
    }

    FRIEND_TEST(AosIpsecHelperTest, Create);
    FRIEND_TEST(AosIpsecHelperTest, CreateOnChallenging);
    FRIEND_TEST(AosIpsecHelperTest, IsPcscfServerPortDifferent);
    FRIEND_TEST(AosIpsecHelperTest, UpdatePreloadedRoute);
    FRIEND_TEST(AosIpsecHelperTest, MakeSas);
    FRIEND_TEST(AosIpsecHelperTest, ProcessAuthChallenged);
    FRIEND_TEST(AosIpsecHelperTest, ProcessRegStarted);
    FRIEND_TEST(AosIpsecHelperTest, ProcessRegUpdated);
    FRIEND_TEST(AosIpsecHelperTest, ProcessRegUpdated2);
    FRIEND_TEST(AosIpsecHelperTest, IsEstablished);
    FRIEND_TEST(AosIpsecHelperTest, SetSecurityServerPortInRegContact);
    FRIEND_TEST(AosIpsecHelperTest, IgnoreCurrentPolicyExpired);
    FRIEND_TEST(AosIpsecHelperTest, SetPcscfPortnSpi);
    FRIEND_TEST(AosIpsecHelperTest, IPSecPolicyExpired);
    FRIEND_TEST(AosIpsecHelperTest, Destroy);

    inline void SetRegContact(IN IRegContact* piRegContact) { m_piRegContact = piRegContact; }
    inline void SetRegParameter(IN IRegParameter* piRegParameter)
    {
        m_piRegParameter = piRegParameter;
    }

    void SetIpsec(IN IMS_SINT32 nType, IN TestAosIpsecEx* pAosIpsec)
    {
        if (nType == NEW_IPSEC)
        {
            m_pNewIpsec = pAosIpsec;
        }
        else if (nType == CURR_IPSEC)
        {
            m_pCurrIpsec = pAosIpsec;
        }
        else  // OLD_IPSEC
        {
            m_pOldIpsec = pAosIpsec;
        }
    }

    TestAosIpsecEx* GetIpsec(IN IMS_SINT32 nType)
    {
        if (nType == NEW_IPSEC)
        {
            return static_cast<TestAosIpsecEx*>(m_pNewIpsec);
        }
        else if (nType == CURR_IPSEC)
        {
            return static_cast<TestAosIpsecEx*>(m_pCurrIpsec);
        }
        else  // OLD_IPSEC
        {
            return static_cast<TestAosIpsecEx*>(m_pOldIpsec);
        }
    }

    void SetUeIpsecInfo(IN IMS_SINT32 nUeInfo, IN IMS_SINT32 nWhere, IN IMS_UINT32 nValue)
    {
        if (nUeInfo == SPI)
        {
            if (nWhere == TYPE_CLIENT)
            {
                m_pUeIpsecInfo->nSpiC = nValue;
            }
            else
            {
                m_pUeIpsecInfo->nSpiS = nValue;
            }
        }
        else
        {
            if (nWhere == TYPE_CLIENT)
            {
                m_pUeIpsecInfo->nPortC = nValue;
            }
            else
            {
                m_pUeIpsecInfo->nPortS = nValue;
            }
        }
    }

    void SetIpsecSaEstablished(IN IMS_SINT32 nType, IN IMS_BOOL bSaEstablished)
    {
        if (bSaEstablished)
        {
            GetIpsec(nType)->SetSaEstablished();
        }
        else
        {
            GetIpsec(nType)->SetSaEstablishedForTest(IMS_FALSE);
        }
    }

    IIpSecPolicy* GetIpsecPolicy(IN IMS_SINT32 nType)
    {
        if (nType == NEW_IPSEC)
        {
            return m_pNewIpsec->GetPolicy();
        }
        else if (nType == CURR_IPSEC)
        {
            return m_pCurrIpsec->GetPolicy();
        }
        else
        {
            return m_pOldIpsec->GetPolicy();
        }
    }

    void SetIIpsecPolicy(IN IMS_SINT32 nType, IN IIpSecPolicy* piPolicy)
    {
        GetIpsec(nType)->SetPolicy(piPolicy);
    }

    INetworkIpSec* GetIpsecNetIpsec(IN IMS_SINT32 nType) { return GetIpsec(nType)->GetNetIpsec(); }

    void SetIIpsecNetIpsec(IN IMS_SINT32 nType, IN INetworkIpSec* piNetIpsec)
    {
        GetIpsec(nType)->SetNetIpsec(piNetIpsec);
    }

    void SetPcscfPortsAndSpisByType(IN IMS_SINT32 nType, IN IMS_UINT32 nPortC, IN IMS_UINT32 nPortS,
            IN IMS_UINT32 nSpiC, IN IMS_UINT32 nSpiS)
    {
        if (nType == NEW_IPSEC)
        {
            m_pNewIpsec->SetPcscfPortsAndSpis(nPortC, nPortS, nSpiC, nSpiS);
        }
        else if (nType == CURR_IPSEC)
        {
            m_pCurrIpsec->SetPcscfPortsAndSpis(nPortC, nPortS, nSpiC, nSpiS);
        }
        else  // OLD_IPSEC
        {
            m_pOldIpsec->SetPcscfPortsAndSpis(nPortC, nPortS, nSpiC, nSpiS);
        }
    }

    IMS_BOOL GetCurrentIgnorePolicyExpired()
    {
        return GetIpsec(CURR_IPSEC)->GetIgnorePolicyExpired();
    }

    void DestroyIpsec(IN IMS_SINT32 nType)
    {
        if (nType == NEW_IPSEC)
        {
            delete m_pNewIpsec;
        }
        else if (nType == CURR_IPSEC)
        {
            delete m_pCurrIpsec;
        }
        else
        {
            delete m_pOldIpsec;
        }
        SetIpsec(nType, IMS_NULL);
    }
};

class AosIpsecHelperTest : public ::testing::Test
{
public:
    TestAosIpsecHelper* m_pAosIpsecHelper;

    MockIAosAppContext m_objMockIAosAppContext;
    MockIRegContact m_objMockIRegContact;
    MockIRegParameter m_objMockIRegParameter;
    AosStaticProfile* m_pAosStaticProfile;

    IAosNConfiguration* m_pOriginAosNConfiguration;
    MockIAosNConfiguration m_objMockAosConfig;

    MockIAosIpsecListener m_objIAosIpsecListener;
    TestAosIpsecEx* m_pAosOldIpsec;
    TestAosIpsecEx* m_pAosCurrIpsec;
    TestAosIpsecEx* m_pAosNewIpsec;

    AString m_strRegId = AString("aos_normal_reg");
    const AString m_strAddr = AString("sip:1234@ims.google.com:5060");
    const AString m_strIpAddr1 = AString("10.168.219.102");
    const AString m_strIpAddr2 = AString("10.168.219.104");

protected:
    void SetUp() override
    {
        m_pAosStaticProfile = new AosStaticProfile();
        ON_CALL(m_objMockIAosAppContext, GetSlotId()).WillByDefault(Return(SLOT_ID));

        m_pAosIpsecHelper = new TestAosIpsecHelper(&m_objMockIRegContact, &m_objMockIRegParameter,
                &m_objMockIAosAppContext, m_strRegId);
        ASSERT_TRUE(m_pAosIpsecHelper != nullptr);
        m_pAosIpsecHelper->InitIpsec();

        m_pOriginAosNConfiguration = AosProvider::GetInstance()->GetNConfiguration();
        AosProvider::GetInstance()->SetNConfiguration(&m_objMockAosConfig, SLOT_ID);

        m_pAosOldIpsec = new TestAosIpsecEx(&m_objIAosIpsecListener, SLOT_ID);
        m_pAosCurrIpsec = new TestAosIpsecEx(&m_objIAosIpsecListener, SLOT_ID);
        m_pAosNewIpsec = new TestAosIpsecEx(&m_objIAosIpsecListener, SLOT_ID);
    }

    void TearDown() override
    {
        AosProvider::GetInstance()->SetNConfiguration(m_pOriginAosNConfiguration, SLOT_ID);

        m_pAosIpsecHelper->SetIpsec(OLD_IPSEC, IMS_NULL);
        m_pAosIpsecHelper->SetIpsec(CURR_IPSEC, IMS_NULL);
        m_pAosIpsecHelper->SetIpsec(NEW_IPSEC, IMS_NULL);

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

        if (m_pAosStaticProfile)
        {
            delete m_pAosStaticProfile;
        }
    }
};

TEST_F(AosIpsecHelperTest, Create)
{
    m_pAosIpsecHelper->SetRegContact(IMS_NULL);
    m_pAosIpsecHelper->SetRegParameter(IMS_NULL);
    EXPECT_FALSE(m_pAosIpsecHelper->Create(IMS_TRUE));

    m_pAosIpsecHelper->SetRegContact(&m_objMockIRegContact);
    m_pAosIpsecHelper->SetRegParameter(&m_objMockIRegParameter);

    // SetUePortnSpi() - GetValidUePort()
    IpAddress objIpAddr(m_strIpAddr1);
    EXPECT_CALL(m_objMockIRegContact, GetIpAddress()).Times(3).WillRepeatedly(ReturnRef(objIpAddr));

    EXPECT_CALL(m_objMockIRegContact, SetPort(_)).Times(AnyNumber());

    // SetSecurityClientHeader() return IMS_FALSE;
    ImsVector<IMS_SINT32> objAuthenticationAlgs;
    objAuthenticationAlgs.Clear();
    EXPECT_CALL(m_objMockAosConfig, GetIpsecAuthenticationAlgorithms())
            .Times(AnyNumber())
            .WillOnce(ReturnRef(objAuthenticationAlgs));
    EXPECT_FALSE(m_pAosIpsecHelper->Create(IMS_TRUE));
    m_pAosIpsecHelper->DestroyIpsec(NEW_IPSEC);

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
    m_pAosIpsecHelper->DestroyIpsec(NEW_IPSEC);

    EXPECT_TRUE(m_pAosIpsecHelper->Create(IMS_FALSE));

    m_pAosIpsecHelper->DestroyIpsec(NEW_IPSEC);
}

TEST_F(AosIpsecHelperTest, CreateOnChallenging)
{
    TestAosIpsecEx* pAosTestIpsec = new TestAosIpsecEx(&m_objIAosIpsecListener, SLOT_ID);
    m_pAosIpsecHelper->SetIpsec(NEW_IPSEC, pAosTestIpsec);
    TestAosIpsecEx* pAosTestIpsec2 = new TestAosIpsecEx(&m_objIAosIpsecListener, SLOT_ID);
    m_pAosIpsecHelper->SetIpsec(OLD_IPSEC, pAosTestIpsec2);

    m_pAosIpsecHelper->SetIpsec(CURR_IPSEC, IMS_NULL);

    m_pAosIpsecHelper->SetUeIpsecInfo(PORT, TYPE_CLIENT, 38002);
    m_pAosIpsecHelper->SetUeIpsecInfo(PORT, TYPE_SERVER, 39002);
    m_pAosIpsecHelper->SetUeIpsecInfo(SPI, TYPE_CLIENT, 12345678);
    m_pAosIpsecHelper->SetUeIpsecInfo(SPI, TYPE_SERVER, 87654321);

    m_pAosIpsecHelper->CreateOnChallenging();

    AosIpsec* pAosIpsec = m_pAosIpsecHelper->GetIpsec(NEW_IPSEC);
    EXPECT_EQ(pAosIpsec->GetUePort(AosIpsec::TYPE_CLIENT), 38002);
    EXPECT_EQ(pAosIpsec->GetUePort(AosIpsec::TYPE_SERVER), 39002);
    EXPECT_EQ(pAosIpsec->GetUeSpi(AosIpsec::TYPE_CLIENT), 12345678);
    EXPECT_EQ(pAosIpsec->GetUeSpi(AosIpsec::TYPE_SERVER), 87654321);

    m_pAosIpsecHelper->DestroyIpsec(NEW_IPSEC);
}

TEST_F(AosIpsecHelperTest, IsPcscfServerPortDifferent)
{
    m_pAosIpsecHelper->SetIpsec(NEW_IPSEC, IMS_NULL);
    EXPECT_FALSE(m_pAosIpsecHelper->IsPcscfServerPortDifferent());

    m_pAosIpsecHelper->SetIpsec(NEW_IPSEC, m_pAosNewIpsec);
    m_pAosIpsecHelper->SetIpsec(CURR_IPSEC, IMS_NULL);
    EXPECT_FALSE(m_pAosIpsecHelper->IsPcscfServerPortDifferent());

    m_pAosIpsecHelper->SetIpsec(CURR_IPSEC, m_pAosCurrIpsec);
    // For PortS of current ipsec
    m_pAosIpsecHelper->SetPcscfPortsAndSpisByType(CURR_IPSEC, 38002, 39002, 12345678, 87654321);
    // For PortS of new ipsec
    m_pAosIpsecHelper->SetPcscfPortsAndSpisByType(NEW_IPSEC, 38002, 39002, 12345678, 87654321);

    EXPECT_FALSE(m_pAosIpsecHelper->IsPcscfServerPortDifferent());

    // For PortS of new ipsec
    m_pAosIpsecHelper->SetPcscfPortsAndSpisByType(NEW_IPSEC, 38002, 39004, 12345678, 87654321);
    EXPECT_TRUE(m_pAosIpsecHelper->IsPcscfServerPortDifferent());
}

TEST_F(AosIpsecHelperTest, UpdatePreloadedRoute)
{
    m_pAosIpsecHelper->SetIpsec(NEW_IPSEC, IMS_NULL);
    EXPECT_FALSE(m_pAosIpsecHelper->UpdatePreloadedRoute(m_strIpAddr1));

    m_pAosIpsecHelper->SetIpsec(NEW_IPSEC, m_pAosNewIpsec);
    // For PortS of new ipsec
    m_pAosIpsecHelper->SetPcscfPortsAndSpisByType(NEW_IPSEC, 38002, 39002, 12345678, 87654321);
    EXPECT_CALL(m_objMockIRegParameter, RemoveAllPreloadedRoutes()).Times(1);
    EXPECT_CALL(
            m_objMockIRegParameter, AddPreloadedRoute(m_strIpAddr1, 39002, AString::ConstNull()))
            .Times(1);
    EXPECT_TRUE(m_pAosIpsecHelper->UpdatePreloadedRoute(m_strIpAddr1));
}

TEST_F(AosIpsecHelperTest, MakeSas)
{
    m_pAosIpsecHelper->SetIpsec(NEW_IPSEC, m_pAosNewIpsec);

    IpAddress objIpa(m_strIpAddr2);

    AString strIk = "";
    strIk.Append("integrity key");
    ByteArray objIk(strIk);

    AString strCk = "";
    strIk.Append("integrity key");
    ByteArray objCk(strCk);

    IIpSecPolicy* piOrigPolicy = m_pAosIpsecHelper->GetIpsecPolicy(NEW_IPSEC);
    MockIIpSecPolicy objMockIIpsecPolicy;
    m_pAosIpsecHelper->SetIIpsecPolicy(NEW_IPSEC, &objMockIIpsecPolicy);

    MockIIpSecSp objMockIIpSecSp;
    EXPECT_CALL(objMockIIpsecPolicy, CreateSp()).Times(12).WillRepeatedly(Return(&objMockIIpSecSp));

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
    EXPECT_CALL(objMockIIpsecPolicy, CreateSa()).Times(8).WillRepeatedly(Return(&objMockIIpSecSa));

    EXPECT_CALL(objMockIIpSecSa, SetSa(_, _, _, _, _, _, _, _, _, _, _)).Times(8);
    EXPECT_CALL(objMockIIpSecSa, DoneSa()).Times(8);

    // DeleteSamePolicy - _pOldIpsec == IMS_NULL
    m_pAosIpsecHelper->SetIpsec(OLD_IPSEC, IMS_NULL);

    // AddPolicy() - m_piPolicy can't be IMS_NULL
    // m_piPolicy is used in CreateSpforTcp() and CreateSpforUDP
    // SetIIpsecPolicy(NEW_IPSEC, IMS_NULL);
    INetworkIpSec* piOrigNetIpsec = m_pAosIpsecHelper->GetIpsecNetIpsec(NEW_IPSEC);
    MockINetworkIpSec objMockINetworkIpsec;
    m_pAosIpsecHelper->SetIIpsecNetIpsec(NEW_IPSEC, &objMockINetworkIpsec);
    EXPECT_CALL(objMockINetworkIpsec, AddPolicy(_))
            .Times(2)
            .WillOnce(Return(IMS_FALSE))
            .WillOnce(Return(IMS_TRUE));

    EXPECT_FALSE(m_pAosIpsecHelper->MakeSas(m_strIpAddr1, objIpa, objIk, objCk));

    EXPECT_CALL(m_objMockIRegContact, SetPort(_)).Times(1);

    EXPECT_TRUE(m_pAosIpsecHelper->MakeSas(m_strIpAddr1, objIpa, objIk, objCk));

    // return to origin variable
    m_pAosIpsecHelper->SetIIpsecPolicy(NEW_IPSEC, piOrigPolicy);
    m_pAosIpsecHelper->SetIIpsecNetIpsec(NEW_IPSEC, piOrigNetIpsec);
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

    TestAosIpsecEx* pAosTestIpsec = new TestAosIpsecEx(&m_objIAosIpsecListener, SLOT_ID);
    m_pAosIpsecHelper->SetIpsec(OLD_IPSEC, pAosTestIpsec);
    EXPECT_TRUE(m_pAosIpsecHelper->ProcessAuthChallenged(Credential::TYPE_AKAv1_MD5));
}

TEST_F(AosIpsecHelperTest, ProcessRegStarted)
{
    TestAosIpsecEx* pAosTestIpsec = new TestAosIpsecEx(&m_objIAosIpsecListener, SLOT_ID);
    m_pAosIpsecHelper->SetIpsec(OLD_IPSEC, pAosTestIpsec);
    m_pAosIpsecHelper->SetIpsec(CURR_IPSEC, m_pAosOldIpsec);
    m_pAosIpsecHelper->SetIpsec(NEW_IPSEC, m_pAosCurrIpsec);

    EXPECT_CALL(m_objMockIRegContact, GetExpires()).Times(3).WillRepeatedly(Return(1200));

    // ManagePolicyLifetime()
    IIpSecPolicy* piOrigPolicy = m_pAosIpsecHelper->GetIpsecPolicy(NEW_IPSEC);
    MockIIpSecPolicy objMockIIpsecPolicy;
    m_pAosIpsecHelper->SetIIpsecPolicy(NEW_IPSEC, &objMockIIpsecPolicy);
    EXPECT_CALL(objMockIIpsecPolicy, ManageLifetime(_)).Times(2);

    // DumpSas()
    INetworkIpSec* piOrigNetIpsec = m_pAosIpsecHelper->GetIpsecNetIpsec(NEW_IPSEC);
    MockINetworkIpSec objMockINetworkIpsec;
    m_pAosIpsecHelper->SetIIpsecNetIpsec(NEW_IPSEC, &objMockINetworkIpsec);

    EXPECT_CALL(objMockINetworkIpsec, DumpPolicy(_)).Times(2);

    // CloseUnsecureTCPSocket();
    // bUnsecureTcpSocketDestroyed = IMS_FALSE
    EXPECT_CALL(m_objMockAosConfig, IsUnsecureTcpSocketOnAccomplishingRegDestroyed())
            .Times(3)
            .WillOnce(Return(IMS_FALSE))
            .WillRepeatedly(Return(IMS_TRUE));

    m_pAosIpsecHelper->ProcessRegStarted();

    // return to origin variable
    m_pAosIpsecHelper->SetIIpsecPolicy(CURR_IPSEC, piOrigPolicy);
    m_pAosIpsecHelper->SetIIpsecNetIpsec(CURR_IPSEC, piOrigNetIpsec);

    // For 2nd ProcessRegStarted()
    m_pAosIpsecHelper->SetIpsec(OLD_IPSEC, IMS_NULL);
    m_pAosIpsecHelper->SetIpsec(CURR_IPSEC, m_pAosOldIpsec);
    m_pAosIpsecHelper->SetIpsec(NEW_IPSEC, m_pAosCurrIpsec);

    // bUnsecureTcpSocketDestroyed = IMS_TRUE
    MockIAosPcscf objMockIAosPcscf;
    EXPECT_CALL(objMockIAosPcscf, GetCurrentPcscf(_, _))
            .Times(2)
            .WillOnce(Return(IMS_FALSE))
            .WillOnce(Return(IMS_TRUE));

    EXPECT_CALL(m_objMockIAosAppContext, GetPcscf())
            .Times(2)
            .WillRepeatedly(Return(&objMockIAosPcscf));

    MockIAosConnection objMockIAosConnection;
    EXPECT_CALL(m_objMockIAosAppContext, GetConnection())
            .Times(1)
            .WillOnce(Return(&objMockIAosConnection));

    IpAddress ipAddr(m_strAddr);
    EXPECT_CALL(objMockIAosConnection, GetLocalAddress(_)).Times(1).WillOnce(ReturnRef(ipAddr));

    m_pAosIpsecHelper->ProcessRegStarted();

    // return to origin variable
    m_pAosIpsecHelper->SetIIpsecPolicy(CURR_IPSEC, piOrigPolicy);
    m_pAosIpsecHelper->SetIIpsecNetIpsec(CURR_IPSEC, piOrigNetIpsec);

    // For 3rd ProcessRegStarted()
    m_pAosIpsecHelper->SetIpsec(OLD_IPSEC, IMS_NULL);
    m_pAosIpsecHelper->SetIpsec(CURR_IPSEC, m_pAosOldIpsec);
    m_pAosIpsecHelper->SetIpsec(NEW_IPSEC, m_pAosCurrIpsec);

    piOrigPolicy = m_pAosIpsecHelper->GetIpsecPolicy(NEW_IPSEC);
    m_pAosIpsecHelper->SetIIpsecPolicy(NEW_IPSEC, &objMockIIpsecPolicy);
    piOrigNetIpsec = m_pAosIpsecHelper->GetIpsecNetIpsec(NEW_IPSEC);
    m_pAosIpsecHelper->SetIIpsecNetIpsec(NEW_IPSEC, &objMockINetworkIpsec);

    m_pAosIpsecHelper->ProcessRegStarted();

    // return to origin variable
    m_pAosIpsecHelper->SetIIpsecPolicy(CURR_IPSEC, piOrigPolicy);
    m_pAosIpsecHelper->SetIIpsecNetIpsec(CURR_IPSEC, piOrigNetIpsec);

    m_pAosIpsecHelper->SetIpsec(OLD_IPSEC, IMS_NULL);
    m_pAosIpsecHelper->SetIpsec(CURR_IPSEC, IMS_NULL);
    m_pAosIpsecHelper->SetIpsec(NEW_IPSEC, IMS_NULL);
}

TEST_F(AosIpsecHelperTest, ProcessRegUpdated)
{
    m_pAosIpsecHelper->SetIpsec(NEW_IPSEC, IMS_NULL);
    m_pAosIpsecHelper->ProcessRegUpdated();

    m_pAosIpsecHelper->SetIpsec(NEW_IPSEC, m_pAosNewIpsec);
    EXPECT_CALL(m_objMockIRegContact, GetExpires()).Times(1).WillRepeatedly(Return(1200));

    // m_pNewIpsec->IsSaEstablished() == IMS_FALSE
    m_pAosIpsecHelper->SetIpsecSaEstablished(NEW_IPSEC, IMS_FALSE);
    TestAosIpsecEx* pAosTestIpsec = new TestAosIpsecEx(&m_objIAosIpsecListener, SLOT_ID);
    m_pAosIpsecHelper->SetIpsec(NEW_IPSEC, pAosTestIpsec);
    m_pAosIpsecHelper->SetIpsec(CURR_IPSEC, m_pAosCurrIpsec);

    IIpSecPolicy* piOrigPolicy = m_pAosIpsecHelper->GetIpsecPolicy(CURR_IPSEC);
    MockIIpSecPolicy objMockIIpsecPolicy;
    m_pAosIpsecHelper->SetIIpsecPolicy(CURR_IPSEC, &objMockIIpsecPolicy);
    EXPECT_CALL(objMockIIpsecPolicy, ManageLifetime(_)).Times(1);

    m_pAosIpsecHelper->ProcessRegUpdated();

    // return to origin variable
    m_pAosIpsecHelper->SetIIpsecPolicy(CURR_IPSEC, piOrigPolicy);
}

TEST_F(AosIpsecHelperTest, ProcessRegUpdated2)
{
    EXPECT_CALL(m_objMockIRegContact, GetExpires()).Times(1).WillRepeatedly(Return(1200));

    TestAosIpsecEx* pAosTestIpsec = new TestAosIpsecEx(&m_objIAosIpsecListener, SLOT_ID);
    m_pAosIpsecHelper->SetIpsec(OLD_IPSEC, pAosTestIpsec);
    m_pAosIpsecHelper->SetIpsec(CURR_IPSEC, m_pAosOldIpsec);
    m_pAosIpsecHelper->SetIpsec(NEW_IPSEC, m_pAosCurrIpsec);

    // m_pNewIpsec->IsSaEstablished() == IMS_TRUE
    m_pAosIpsecHelper->SetIpsecSaEstablished(NEW_IPSEC, IMS_TRUE);

    // ManagePolicyLifetime()
    IIpSecPolicy* piOrigPolicy = m_pAosIpsecHelper->GetIpsecPolicy(NEW_IPSEC);
    MockIIpSecPolicy objMockIIpsecPolicy;
    m_pAosIpsecHelper->SetIIpsecPolicy(NEW_IPSEC, &objMockIIpsecPolicy);
    EXPECT_CALL(objMockIIpsecPolicy, ManageLifetime(_)).Times(1);

    // DumpSas()
    INetworkIpSec* piOrigNetIpsec = m_pAosIpsecHelper->GetIpsecNetIpsec(NEW_IPSEC);
    MockINetworkIpSec objMockINetworkIpsec;
    m_pAosIpsecHelper->SetIIpsecNetIpsec(NEW_IPSEC, &objMockINetworkIpsec);
    EXPECT_CALL(objMockINetworkIpsec, DumpPolicy(_)).Times(1);

    m_pAosIpsecHelper->ProcessRegUpdated();

    // return to origin variable
    m_pAosIpsecHelper->SetIIpsecPolicy(CURR_IPSEC, piOrigPolicy);
    m_pAosIpsecHelper->SetIIpsecNetIpsec(CURR_IPSEC, piOrigNetIpsec);
}

TEST_F(AosIpsecHelperTest, IsEstablished)
{
    m_pAosIpsecHelper->SetIpsec(OLD_IPSEC, IMS_NULL);
    m_pAosIpsecHelper->SetIpsec(CURR_IPSEC, IMS_NULL);
    m_pAosIpsecHelper->SetIpsec(NEW_IPSEC, IMS_NULL);
    EXPECT_FALSE(m_pAosIpsecHelper->IsEstablished());

    m_pAosIpsecHelper->SetIpsec(NEW_IPSEC, m_pAosNewIpsec);
    m_pAosIpsecHelper->SetIpsecSaEstablished(NEW_IPSEC, IMS_TRUE);
    EXPECT_TRUE(m_pAosIpsecHelper->IsEstablished());

    m_pAosIpsecHelper->SetIpsec(CURR_IPSEC, m_pAosCurrIpsec);
    m_pAosIpsecHelper->SetIpsecSaEstablished(CURR_IPSEC, IMS_FALSE);
    EXPECT_TRUE(m_pAosIpsecHelper->IsEstablished());

    m_pAosIpsecHelper->SetIpsecSaEstablished(CURR_IPSEC, IMS_TRUE);
    EXPECT_TRUE(m_pAosIpsecHelper->IsEstablished());

    m_pAosIpsecHelper->SetIpsec(OLD_IPSEC, m_pAosOldIpsec);
    m_pAosIpsecHelper->SetIpsecSaEstablished(OLD_IPSEC, IMS_TRUE);
    EXPECT_TRUE(m_pAosIpsecHelper->IsEstablished());
}

TEST_F(AosIpsecHelperTest, SetSecurityServerPortInRegContact)
{
    EXPECT_CALL(m_objMockAosConfig, IsSecurityServerPortInRegContactOfInitRegUsed())
            .Times(2)
            .WillOnce(Return(IMS_FALSE))
            .WillOnce(Return(IMS_TRUE));
    m_pAosIpsecHelper->SetSecurityServerPortInRegContact();

    m_pAosIpsecHelper->SetIpsec(NEW_IPSEC, m_pAosNewIpsec);
    EXPECT_CALL(m_objMockIRegContact, SetPort(_)).Times(1);

    m_pAosIpsecHelper->SetSecurityServerPortInRegContact();
}

TEST_F(AosIpsecHelperTest, IgnoreCurrentPolicyExpired)
{
    m_pAosIpsecHelper->SetIpsec(CURR_IPSEC, m_pAosCurrIpsec);
    m_pAosIpsecHelper->IgnoreCurrentPolicyExpired();
    EXPECT_TRUE(m_pAosIpsecHelper->GetCurrentIgnorePolicyExpired());
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
    m_pAosIpsecHelper->IPSecPolicyExpired(IMS_NULL);

    TestAosIpsecEx* pAosTestIpsec = new TestAosIpsecEx(&m_objIAosIpsecListener, SLOT_ID);

    m_pAosIpsecHelper->SetIpsec(OLD_IPSEC, IMS_NULL);
    m_pAosIpsecHelper->SetIpsec(CURR_IPSEC, IMS_NULL);
    m_pAosIpsecHelper->SetIpsec(NEW_IPSEC, IMS_NULL);
    m_pAosIpsecHelper->IPSecPolicyExpired(pAosTestIpsec);
}

TEST_F(AosIpsecHelperTest, Destroy)
{
    TestAosIpsecEx* pAosTestOldIpsec = new TestAosIpsecEx(&m_objIAosIpsecListener, SLOT_ID);
    TestAosIpsecEx* pAosTestCurrIpsec = new TestAosIpsecEx(&m_objIAosIpsecListener, SLOT_ID);
    TestAosIpsecEx* pAosTestNewIpsec = new TestAosIpsecEx(&m_objIAosIpsecListener, SLOT_ID);
    m_pAosIpsecHelper->SetIpsec(OLD_IPSEC, pAosTestOldIpsec);
    m_pAosIpsecHelper->SetIpsec(CURR_IPSEC, pAosTestCurrIpsec);
    m_pAosIpsecHelper->SetIpsec(NEW_IPSEC, pAosTestNewIpsec);

    m_pAosIpsecHelper->Destroy();
}