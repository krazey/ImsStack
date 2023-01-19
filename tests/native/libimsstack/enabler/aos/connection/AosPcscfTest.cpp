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

#include "connection/AosPcscf.h"
#include "ServerAddress.h"

#include "interface/MockIAosAppContext.h"
#include "interface/MockIAosConnection.h"
#include "interface/MockIAosNConfiguration.h"
#include "interface/MockIAosPcscf.h"
#include "interface/MockIAosSubscriber.h"
#include "interface/MockIAosRegistration.h"
#include "../../../config/interface/common/MockISubscriberConfig.h"

using ::testing::_;
using ::testing::AnyNumber;
using ::testing::Return;
using ::testing::ReturnRef;

const IMS_SINT32 SLOT_ID = 0;

class AosPcscfTest : public ::testing::Test
{
public:
    AosPcscf* m_pAosPcscf;
    AStringArray m_objPcscfAddressPreset;
    IMSList<IMS_SINT32> m_objPcscfPortPreset;

    MockIAosAppContext m_objMockIAosAppContext;
    MockIAosConnection m_objMockIAosConnection;
    MockIAosNConfiguration m_objMockIAosNConfiguration;
    MockIAosSubscriber m_objMockIAosSubscriber;
    MockISubscriberConfig m_objMockISubscriberConfig;

protected:
    virtual void SetUp() override
    {
        EXPECT_CALL(m_objMockIAosAppContext, GetSlotId())
                .Times(AnyNumber())
                .WillRepeatedly(Return(SLOT_ID));

        const AString strValue = AString("test");
        EXPECT_CALL(m_objMockIAosAppContext, GetProfileId())
                .Times(AnyNumber())
                .WillRepeatedly(ReturnRef(strValue));

        EXPECT_CALL(m_objMockIAosAppContext, GetConnection())
                .Times(AnyNumber())
                .WillRepeatedly(Return(&m_objMockIAosConnection));

        EXPECT_CALL(m_objMockIAosAppContext, GetSubscriber())
                .Times(AnyNumber())
                .WillRepeatedly(Return(&m_objMockIAosSubscriber));

        EXPECT_CALL(m_objMockIAosSubscriber, GetSubscriberConfig(-1))
                .Times(AnyNumber())
                .WillRepeatedly(Return(&m_objMockISubscriberConfig));

        m_pAosPcscf = new AosPcscf(static_cast<IAosAppContext*>(&m_objMockIAosAppContext));
        ASSERT_TRUE(m_pAosPcscf != nullptr);
    }

    virtual void TearDown() override
    {
        if (m_pAosPcscf)
        {
            delete m_pAosPcscf;
        }
    }

    void Initialize() { m_pAosPcscf->Init(); }

    void CleanUp() { m_pAosPcscf->CleanUp(); }

    void SetRegType(IN AosRegistrationType RegType) { m_pAosPcscf->m_eRegType = RegType; }

    void SetNConfig(IN IAosNConfiguration* piNc) { m_pAosPcscf->m_piAosNConfig = piNc; }

    void SetConfigured(IN IMS_BOOL bConfigured) { m_pAosPcscf->SetConfigured(bConfigured); }

    void SetCurrentPcscfIndex(IN IMS_UINT32 nIndex) { m_pAosPcscf->m_nCurrentPcscfIndex = nIndex; }

    void SetPcscfs()
    {
        AString strPcscfs = "0.0.0.1, 0.0.0.2, 0.0.0.3";
        m_objPcscfAddressPreset = strPcscfs.Split(',');
        for (int i = 0; i < m_objPcscfAddressPreset.GetCount(); i++)
        {
            m_objPcscfPortPreset.Append(5060 + i);
            m_pAosPcscf->AddPcscf(
                    m_objPcscfAddressPreset.GetElementAt(i), m_objPcscfPortPreset.GetAt(i));
        }
    }

    IMS_BOOL SetDnsQueryTimerExpired()
    {
        if (m_pAosPcscf->m_piDnsQueryRetryTimer != IMS_NULL)
        {
            m_pAosPcscf->Timer_TimerExpired(m_pAosPcscf->m_piDnsQueryRetryTimer);
            return IMS_TRUE;
        }
        else
        {
            return IMS_FALSE;
        }
    }

    void NotifyResult() { m_pAosPcscf->m_piListener->Pcscf_NotifyResult(IMS_TRUE); }

    IMS_BOOL IsValidPcscf(IN IMS_UINT32 nIndex)
    {
        if (nIndex < m_pAosPcscf->m_objPcscfList.GetSize())
        {
            Pcscf* pPcscf = m_pAosPcscf->m_objPcscfList.GetAt(nIndex);
            return pPcscf->IsAvailable();
        }

        return IMS_FALSE;
    }

    IMS_BOOL IsTriedPcscf(IN IMS_UINT32 nIndex)
    {
        if (nIndex < m_pAosPcscf->m_objPcscfList.GetSize())
        {
            Pcscf* pPcscf = m_pAosPcscf->m_objPcscfList.GetAt(nIndex);
            return pPcscf->IsTried();
        }

        return IMS_FALSE;
    }
};

TEST_F(AosPcscfTest, Configure_GetFromPco)
{
    EXPECT_CALL(m_objMockIAosConnection, GetLocalAddress(_))
            .Times(2)
            .WillOnce(ReturnRef(IPAddress::ANY))
            .WillOnce(ReturnRef(IPAddress::LOOPBACK));

    EXPECT_CALL(m_objMockIAosConnection, GetPcscfAddress(_))
            .Times(AnyNumber())
            .WillRepeatedly(ReturnRef(m_objPcscfAddressPreset));

    IMSVector<IMS_SINT32> objDiscoveryMethods;
    objDiscoveryMethods.Add(ISubscriberConfig::PCSCF_DISCOVERY_METHOD_PCO);
    EXPECT_CALL(m_objMockISubscriberConfig, GetPcscfDiscoveryMethods())
            .Times(AnyNumber())
            .WillRepeatedly(ReturnRef(objDiscoveryMethods));
    EXPECT_CALL(m_objMockISubscriberConfig, GetPcscfAddress())
            .Times(AnyNumber())
            .WillRepeatedly(Return(nullptr));

    MockIAosNConfiguration m_objMockIAosNConfiguration;
    SetNConfig(&m_objMockIAosNConfiguration);
    EXPECT_CALL(m_objMockIAosNConfiguration, GetPcscfPort())
            .Times(AnyNumber())
            .WillRepeatedly(Return(5060));

    SetPcscfs();

    // valid local address type but invalid address
    m_pAosPcscf->Configure(IPAddress::IPV4);
    EXPECT_FALSE(m_pAosPcscf->IsConfigured());

    // valid local address type and valid address
    m_pAosPcscf->Configure(IPAddress::IPV4);
    EXPECT_TRUE(m_pAosPcscf->IsConfigured());
}

TEST_F(AosPcscfTest, Configure_GetFromConf)
{
    EXPECT_CALL(m_objMockIAosConnection, GetLocalAddress(_))
            .Times(1)
            .WillOnce(ReturnRef(IPAddress::LOOPBACK));

    IMSVector<IMS_SINT32> objDiscoveryMethods;
    objDiscoveryMethods.Add(ISubscriberConfig::PCSCF_DISCOVERY_METHOD_CONFIG);
    EXPECT_CALL(m_objMockISubscriberConfig, GetPcscfDiscoveryMethods())
            .Times(AnyNumber())
            .WillRepeatedly(ReturnRef(objDiscoveryMethods));

    const AString strAddress = AString("0.0.0.4");
    ServerAddress* pSa = new ServerAddress(strAddress, 5060);
    IMSVector<ServerAddress*> objPcscfAddresses;
    objPcscfAddresses.Add(pSa);
    EXPECT_CALL(m_objMockISubscriberConfig, GetPcscfAddresses())
            .Times(AnyNumber())
            .WillRepeatedly(ReturnRef(objPcscfAddresses));

    // unknown local address type
    m_pAosPcscf->Configure(IPAddress::UNKNOWN);
    EXPECT_TRUE(m_pAosPcscf->IsConfigured());

    if (pSa)
    {
        delete pSa;
    }
}

TEST_F(AosPcscfTest, Configure_GetFromConf_TryOtherIpType)
{
    EXPECT_CALL(m_objMockIAosConnection, GetLocalAddress(_))
            .Times(AnyNumber())
            .WillRepeatedly(ReturnRef(IPAddress::IPv6LOOPBACK));

    IMSVector<IMS_SINT32> objDiscoveryMethods;
    objDiscoveryMethods.Add(ISubscriberConfig::PCSCF_DISCOVERY_METHOD_CONFIG);
    EXPECT_CALL(m_objMockISubscriberConfig, GetPcscfDiscoveryMethods())
            .Times(AnyNumber())
            .WillRepeatedly(ReturnRef(objDiscoveryMethods));

    const AString strAddress = AString("0.0.0.4");
    ServerAddress* pSa = new ServerAddress(strAddress, 5060);
    IMSVector<ServerAddress*> objPcscfAddresses;
    objPcscfAddresses.Add(pSa);
    EXPECT_CALL(m_objMockISubscriberConfig, GetPcscfAddresses())
            .Times(AnyNumber())
            .WillRepeatedly(ReturnRef(objPcscfAddresses));

    // unknown local address type
    m_pAosPcscf->Configure(IPAddress::UNKNOWN);
    EXPECT_TRUE(m_pAosPcscf->IsConfigured());

    if (pSa)
    {
        delete pSa;
    }
}

TEST_F(AosPcscfTest, Configure_ProcessDnsQuery)
{
    EXPECT_CALL(m_objMockIAosConnection, GetLocalAddress(_))
            .Times(AnyNumber())
            .WillRepeatedly(ReturnRef(IPAddress::LOOPBACK));

    IMSVector<IMS_SINT32> objDiscoveryMethods;
    objDiscoveryMethods.Add(ISubscriberConfig::PCSCF_DISCOVERY_METHOD_CONFIG);
    EXPECT_CALL(m_objMockISubscriberConfig, GetPcscfDiscoveryMethods())
            .Times(AnyNumber())
            .WillRepeatedly(ReturnRef(objDiscoveryMethods));

    const AString strAddress = AString("aaa.bbb.ccc");
    ServerAddress* pSa = new ServerAddress(strAddress, 5060);
    IMSVector<ServerAddress*> objPcscfAddresses;
    objPcscfAddresses.Add(pSa);
    EXPECT_CALL(m_objMockISubscriberConfig, GetPcscfAddresses())
            .Times(AnyNumber())
            .WillRepeatedly(ReturnRef(objPcscfAddresses));

    EXPECT_CALL(m_objMockIAosConnection, GetHostByName(_, _, _))
            .Times(AnyNumber())
            .WillRepeatedly(Return(-1));

    // unknown local address type
    m_pAosPcscf->Configure(IPAddress::UNKNOWN);
    EXPECT_FALSE(m_pAosPcscf->IsConfigured());
    EXPECT_TRUE(SetDnsQueryTimerExpired());

    if (pSa)
    {
        delete pSa;
    }
}

TEST_F(AosPcscfTest, FailToFindDiscoveryMethod)
{
    EXPECT_CALL(m_objMockIAosConnection, GetLocalAddress(_))
            .Times(AnyNumber())
            .WillRepeatedly(ReturnRef(IPAddress::LOOPBACK));
    EXPECT_CALL(m_objMockIAosSubscriber, GetSubscriberConfig(IAosSubscriber::FAKE))
            .Times(2)
            .WillOnce(Return(nullptr))
            .WillOnce(Return(&m_objMockISubscriberConfig));
    IMSVector<IMS_SINT32> objDiscoveryMethods;
    EXPECT_CALL(m_objMockISubscriberConfig, GetPcscfDiscoveryMethods())
            .Times(AnyNumber())
            .WillRepeatedly(ReturnRef(objDiscoveryMethods));

    SetRegType(AosRegistrationType::FAKE);
    m_pAosPcscf->Configure(IPAddress::IPV4);
    m_pAosPcscf->Configure(IPAddress::IPV4);

    EXPECT_FALSE(m_pAosPcscf->IsConfigured());
}

TEST_F(AosPcscfTest, IsConfigured)
{
    MockIAosRegistration objMockIAosRegistration;
    EXPECT_CALL(m_objMockIAosAppContext, GetRegistration())
            .Times(AnyNumber())
            .WillRepeatedly(Return(&objMockIAosRegistration));
    EXPECT_CALL(objMockIAosRegistration, GetRegType())
            .Times(AnyNumber())
            .WillRepeatedly(Return(AosRegistrationType::NORMAL));

    SetConfigured(IMS_TRUE);
    EXPECT_TRUE(m_pAosPcscf->IsConfigured());
    CleanUp();
    EXPECT_FALSE(m_pAosPcscf->IsConfigured());
}

TEST_F(AosPcscfTest, IsAsyncDnsDiscovery)
{
    MockIAosRegistration objMockIAosRegistration;
    EXPECT_CALL(m_objMockIAosAppContext, GetRegistration())
            .Times(AnyNumber())
            .WillRepeatedly(Return(&objMockIAosRegistration));
    EXPECT_CALL(objMockIAosRegistration, GetRegType())
            .Times(AnyNumber())
            .WillRepeatedly(Return(AosRegistrationType::NORMAL));

    SetRegType(AosRegistrationType::RCS);
    EXPECT_TRUE(m_pAosPcscf->IsAsyncDnsDiscovery());
    Initialize();
    EXPECT_FALSE(m_pAosPcscf->IsAsyncDnsDiscovery());
}

TEST_F(AosPcscfTest, IsSinglePcoScheme_EmptyDiscoveryMethod)
{
    IMSVector<IMS_SINT32> objDiscoveryMethods;
    objDiscoveryMethods.Add(ISubscriberConfig::PCSCF_DISCOVERY_METHOD_PCO);
    EXPECT_CALL(m_objMockISubscriberConfig, GetPcscfDiscoveryMethods())
            .Times(AnyNumber())
            .WillRepeatedly(ReturnRef(objDiscoveryMethods));

    objDiscoveryMethods.RemoveAt(0);
    EXPECT_FALSE(m_pAosPcscf->IsSinglePcoScheme());
}

TEST_F(AosPcscfTest, IsSinglePcoScheme_InvalidSubscriber)
{
    EXPECT_CALL(m_objMockIAosSubscriber, GetSubscriberConfig(IAosSubscriber::FAKE))
            .WillOnce(Return(nullptr));
    IMSVector<IMS_SINT32> objDiscoveryMethods;
    objDiscoveryMethods.Add(ISubscriberConfig::PCSCF_DISCOVERY_METHOD_PCO);
    EXPECT_CALL(m_objMockISubscriberConfig, GetPcscfDiscoveryMethods())
            .Times(AnyNumber())
            .WillRepeatedly(ReturnRef(objDiscoveryMethods));

    SetRegType(AosRegistrationType::FAKE);
    EXPECT_FALSE(m_pAosPcscf->IsSinglePcoScheme());
}

TEST_F(AosPcscfTest, IsSinglePcoScheme)
{
    IMSVector<IMS_SINT32> objDiscoveryMethods;
    objDiscoveryMethods.Add(ISubscriberConfig::PCSCF_DISCOVERY_METHOD_CONFIG);
    objDiscoveryMethods.Add(ISubscriberConfig::PCSCF_DISCOVERY_METHOD_PCO);
    EXPECT_CALL(m_objMockISubscriberConfig, GetPcscfDiscoveryMethods())
            .Times(AnyNumber())
            .WillRepeatedly(ReturnRef(objDiscoveryMethods));

    EXPECT_FALSE(m_pAosPcscf->IsSinglePcoScheme());
    objDiscoveryMethods.RemoveAt(0);
    EXPECT_TRUE(m_pAosPcscf->IsSinglePcoScheme());
}

TEST_F(AosPcscfTest, GetPcscfs)
{
    SetPcscfs();
    AStringArray objGetPcscfs = m_pAosPcscf->GetPcscfs();
    for (int i = 0; i < objGetPcscfs.GetCount(); i++)
    {
        EXPECT_EQ(objGetPcscfs.GetElementAt(i), m_objPcscfAddressPreset.GetElementAt(i));
    }
}

TEST_F(AosPcscfTest, GetPcscfsPorts)
{
    SetPcscfs();
    IMSList<IMS_SINT32> objGetPcscfsPorts = m_pAosPcscf->GetPcscfsPorts();
    for (int i = 0; i < objGetPcscfsPorts.GetSize(); i++)
    {
        EXPECT_EQ(objGetPcscfsPorts.GetAt(i), m_objPcscfPortPreset.GetAt(i));
    }
}

TEST_F(AosPcscfTest, UpdatePcscfs)
{
    MockIAosNConfiguration m_objMockIAosNConfiguration;
    SetNConfig(&m_objMockIAosNConfiguration);
    EXPECT_CALL(m_objMockIAosNConfiguration, GetPcscfPort())
            .Times(AnyNumber())
            .WillRepeatedly(Return(5060));

    SetPcscfs();
    AString strNewPcscfs = "0.0.0.3, 0.0.0.4";
    AStringArray objNewPcscfs = strNewPcscfs.Split(',');
    m_pAosPcscf->UpdatePcscfs(objNewPcscfs, m_objPcscfPortPreset);

    AStringArray objGetPcscfs = m_pAosPcscf->GetPcscfs();
    for (int i = 0; i < objGetPcscfs.GetCount(); i++)
    {
        EXPECT_EQ(objGetPcscfs.GetElementAt(i), objNewPcscfs.GetElementAt(i));
    }
}

TEST_F(AosPcscfTest, HasPcscf)
{
    SetPcscfs();
    EXPECT_TRUE(m_pAosPcscf->HasPcscf(m_objPcscfAddressPreset.GetCount() - 1));
    EXPECT_FALSE(m_pAosPcscf->HasPcscf(m_objPcscfAddressPreset.GetCount()));
}

TEST_F(AosPcscfTest, GetPcscfCount)
{
    SetPcscfs();
    EXPECT_EQ(m_pAosPcscf->GetPcscfCount(), m_objPcscfAddressPreset.GetCount());
}

TEST_F(AosPcscfTest, RemoveCurrentPcscf)
{
    SetPcscfs();
    m_objPcscfAddressPreset.RemoveElementAt(m_pAosPcscf->GetCurrentIndex());
    m_pAosPcscf->RemoveCurrentPcscf();

    AStringArray objGetPcscfs = m_pAosPcscf->GetPcscfs();
    for (int i = 0; i < objGetPcscfs.GetCount(); i++)
    {
        EXPECT_EQ(objGetPcscfs.GetElementAt(i), m_objPcscfAddressPreset.GetElementAt(i));
    }
}

TEST_F(AosPcscfTest, ManageValidPcscf)
{
    IMS_UINT32 nUnavailableTimeSec = 5;

    SetPcscfs();
    EXPECT_TRUE(IsValidPcscf(m_pAosPcscf->GetCurrentIndex()));

    m_pAosPcscf->SetCurrentPcscfInvalid(IMS_TRUE, nUnavailableTimeSec);
    EXPECT_FALSE(IsValidPcscf(m_pAosPcscf->GetCurrentIndex()));

    m_pAosPcscf->SetAllPcscfValid();
    EXPECT_TRUE(IsValidPcscf(m_pAosPcscf->GetCurrentIndex()));
}

TEST_F(AosPcscfTest, ManageTriedPcscf)
{
    SetPcscfs();
    AStringArray objPcscfs = m_pAosPcscf->GetPcscfs();
    for (int i = 0; i < objPcscfs.GetCount(); i++)
    {
        EXPECT_FALSE(m_pAosPcscf->IsAllPcscfTried());
        EXPECT_FALSE(IsTriedPcscf(i));
        SetCurrentPcscfIndex(i);
        m_pAosPcscf->SetCurrentPcscfTried();
        EXPECT_TRUE(IsTriedPcscf(i));
    }
    EXPECT_TRUE(m_pAosPcscf->IsAllPcscfTried());

    m_pAosPcscf->ResetAllPcscfTried();
    for (int i = 0; i < objPcscfs.GetCount(); i++)
    {
        EXPECT_FALSE(IsTriedPcscf(i));
    }
}

TEST_F(AosPcscfTest, GetCurrentPcscf)
{
    AString objPcscf;
    IMS_UINT32 nPort;
    SetPcscfs();

    AStringArray objPcscfs = m_pAosPcscf->GetPcscfs();
    for (int i = 0; i < objPcscfs.GetCount(); i++)
    {
        SetCurrentPcscfIndex(i);
        EXPECT_TRUE(m_pAosPcscf->GetCurrentPcscf(objPcscf, nPort));
        EXPECT_EQ(objPcscf, m_objPcscfAddressPreset.GetElementAt(i));
        EXPECT_EQ(nPort, m_objPcscfPortPreset.GetAt(i));
    }

    SetCurrentPcscfIndex(objPcscfs.GetCount());
    EXPECT_FALSE(m_pAosPcscf->GetCurrentPcscf(objPcscf, nPort));
}

TEST_F(AosPcscfTest, GetCurrentIndex)
{
    SetPcscfs();
    AStringArray objPcscfs = m_pAosPcscf->GetPcscfs();
    for (int i = 0; i < objPcscfs.GetCount(); i++)
    {
        SetCurrentPcscfIndex(i);
        EXPECT_EQ(m_pAosPcscf->GetCurrentIndex(), i);
    }
}

TEST_F(AosPcscfTest, IsFirstPcscf)
{
    SetCurrentPcscfIndex(0);
    EXPECT_TRUE(m_pAosPcscf->IsFirstPcscf());
    SetCurrentPcscfIndex(1);
    EXPECT_FALSE(m_pAosPcscf->IsFirstPcscf());
}

TEST_F(AosPcscfTest, GetFirstPcscf)
{
    AString objPcscf;
    IMS_UINT32 nPort;

    EXPECT_FALSE(m_pAosPcscf->GetFirstPcscf(objPcscf, nPort));
    SetPcscfs();
    EXPECT_TRUE(m_pAosPcscf->GetFirstPcscf(objPcscf, nPort));
    EXPECT_EQ(objPcscf, m_objPcscfAddressPreset.GetElementAt(0));
    EXPECT_EQ(nPort, m_objPcscfPortPreset.GetAt(0));
}

TEST_F(AosPcscfTest, HasNextPcscf)
{
    MockIAosNConfiguration m_objMockIAosNConfiguration;
    SetNConfig(&m_objMockIAosNConfiguration);
    EXPECT_CALL(m_objMockIAosNConfiguration, GetRegRetryDefaultPolicy())
            .Times(AnyNumber())
            .WillRepeatedly(
                    Return(CarrierConfig::Assets::DEFAULT_RETRY_POLICY_CIRCULAR_NEXT_PCSCF));

    EXPECT_FALSE(m_pAosPcscf->HasNextPcscf());
    SetPcscfs();
    EXPECT_TRUE(m_pAosPcscf->HasNextPcscf());
}

TEST_F(AosPcscfTest, GetNextPcscfIndex)
{
    MockIAosNConfiguration m_objMockIAosNConfiguration;
    SetNConfig(&m_objMockIAosNConfiguration);
    EXPECT_CALL(m_objMockIAosNConfiguration, GetRegRetryDefaultPolicy())
            .Times(AnyNumber())
            .WillRepeatedly(
                    Return(CarrierConfig::Assets::DEFAULT_RETRY_POLICY_CIRCULAR_NEXT_PCSCF));

    EXPECT_LT(m_pAosPcscf->GetNextPcscfIndex(), 0);
    SetPcscfs();
    AStringArray objPcscfs = m_pAosPcscf->GetPcscfs();
    for (int i = 0; i < objPcscfs.GetCount(); i++)
    {
        SetCurrentPcscfIndex(i);
        EXPECT_GE(m_pAosPcscf->GetNextPcscfIndex(), 0);
    }
}

TEST_F(AosPcscfTest, GetNextPcscf)
{
    AString objPcscf;
    IMS_UINT32 nPort;
    MockIAosNConfiguration m_objMockIAosNConfiguration;
    SetNConfig(&m_objMockIAosNConfiguration);
    EXPECT_CALL(m_objMockIAosNConfiguration, GetRegRetryDefaultPolicy())
            .Times(AnyNumber())
            .WillRepeatedly(
                    Return(CarrierConfig::Assets::DEFAULT_RETRY_POLICY_CIRCULAR_NEXT_PCSCF));

    EXPECT_FALSE(m_pAosPcscf->GetNextPcscf(objPcscf, nPort));
    SetPcscfs();
    AStringArray objPcscfs = m_pAosPcscf->GetPcscfs();
    for (int i = 0; i < objPcscfs.GetCount(); i++)
    {
        EXPECT_TRUE(m_pAosPcscf->GetNextPcscf(objPcscf, nPort));
        EXPECT_EQ(objPcscf, m_objPcscfAddressPreset.GetElementAt(m_pAosPcscf->GetCurrentIndex()));
        EXPECT_EQ(nPort, m_objPcscfPortPreset.GetAt(m_pAosPcscf->GetCurrentIndex()));
    }
}

TEST_F(AosPcscfTest, SetFirstPcscfIndex)
{
    m_pAosPcscf->SetFirstPcscfIndex();
    EXPECT_EQ(m_pAosPcscf->GetCurrentIndex(), 0);
}

TEST_F(AosPcscfTest, CheckAndProcessChangeFromPco_Reorder)
{
    AString strNewPcscfs = "0.0.0.4, 0.0.0.1";
    AStringArray objNewPcscfs = strNewPcscfs.Split(',');
    EXPECT_CALL(m_objMockIAosConnection, GetPcscfAddress(_))
            .Times(AnyNumber())
            .WillRepeatedly(ReturnRef(objNewPcscfs));

    EXPECT_CALL(m_objMockISubscriberConfig, GetPcscfAddress())
            .Times(AnyNumber())
            .WillRepeatedly(Return(nullptr));

    MockIAosNConfiguration m_objMockIAosNConfiguration;
    SetNConfig(&m_objMockIAosNConfiguration);
    EXPECT_CALL(m_objMockIAosNConfiguration, GetPcscfPort())
            .Times(AnyNumber())
            .WillRepeatedly(Return(5060));

    SetPcscfs();
    EXPECT_TRUE(m_pAosPcscf->CheckAndProcessChangeFromPco());
    EXPECT_EQ(m_pAosPcscf->GetChangedType(), IAosPcscf::TYPE_CHANGED_REORDER);
}

TEST_F(AosPcscfTest, CheckAndProcessChangeFromPco_Different)
{
    AString strNewPcscfs = "0.0.0.4, 0.0.0.5";
    AStringArray objNewPcscfs = strNewPcscfs.Split(',');
    EXPECT_CALL(m_objMockIAosConnection, GetPcscfAddress(_))
            .Times(AnyNumber())
            .WillRepeatedly(ReturnRef(objNewPcscfs));

    EXPECT_CALL(m_objMockISubscriberConfig, GetPcscfAddress())
            .Times(AnyNumber())
            .WillRepeatedly(Return(nullptr));

    MockIAosNConfiguration m_objMockIAosNConfiguration;
    SetNConfig(&m_objMockIAosNConfiguration);
    EXPECT_CALL(m_objMockIAosNConfiguration, GetPcscfPort())
            .Times(AnyNumber())
            .WillRepeatedly(Return(5060));

    SetPcscfs();
    EXPECT_TRUE(m_pAosPcscf->CheckAndProcessChangeFromPco());
    EXPECT_EQ(m_pAosPcscf->GetChangedType(), IAosPcscf::TYPE_CHANGED_DIFFERENT);
}

TEST_F(AosPcscfTest, SetListener)
{
    MockIAosPcscfListener objMockIAosPcscfListener;
    EXPECT_CALL(objMockIAosPcscfListener, Pcscf_NotifyResult(_)).Times(1);

    m_pAosPcscf->SetListener(static_cast<IAosPcscfListener*>(&objMockIAosPcscfListener));
    NotifyResult();
}

TEST_F(AosPcscfTest, Get_Reset_IncreaseCurrentPcscfTriedCount)
{
    MockIAosNConfiguration m_objMockIAosNConfiguration;
    SetNConfig(&m_objMockIAosNConfiguration);
    EXPECT_CALL(m_objMockIAosNConfiguration, GetRegRetryCountPerPcscf())
            .Times(AnyNumber())
            .WillRepeatedly(Return(2));

    SetPcscfs();
    SetCurrentPcscfIndex(0);

    EXPECT_EQ(m_pAosPcscf->GetCurrentPcscfTriedCount(), 0);

    m_pAosPcscf->IncreaseCurrentPcscfTriedCount();
    EXPECT_EQ(m_pAosPcscf->GetCurrentPcscfTriedCount(), 1);

    m_pAosPcscf->IncreaseCurrentPcscfTriedCount();
    EXPECT_EQ(m_pAosPcscf->GetCurrentPcscfTriedCount(), 2);

    m_pAosPcscf->ResetCurrentPcscfTriedCount();
    EXPECT_EQ(m_pAosPcscf->GetCurrentPcscfTriedCount(), 0);

    SetCurrentPcscfIndex(3);
    EXPECT_EQ(m_pAosPcscf->GetCurrentPcscfTriedCount(), 0);

    m_pAosPcscf->IncreaseCurrentPcscfTriedCount();
    EXPECT_EQ(m_pAosPcscf->GetCurrentPcscfTriedCount(), 0);

    m_pAosPcscf->IncreaseCurrentPcscfTriedCount();
    EXPECT_EQ(m_pAosPcscf->GetCurrentPcscfTriedCount(), 0);
}

TEST_F(AosPcscfTest, ResetAllPcscfTriedCount)
{
    MockIAosNConfiguration m_objMockIAosNConfiguration;
    SetNConfig(&m_objMockIAosNConfiguration);
    EXPECT_CALL(m_objMockIAosNConfiguration, GetRegRetryCountPerPcscf())
            .Times(AnyNumber())
            .WillRepeatedly(Return(2));

    SetPcscfs();

    SetCurrentPcscfIndex(0);
    m_pAosPcscf->IncreaseCurrentPcscfTriedCount();
    EXPECT_EQ(m_pAosPcscf->GetCurrentPcscfTriedCount(), 1);

    SetCurrentPcscfIndex(1);
    m_pAosPcscf->IncreaseCurrentPcscfTriedCount();
    EXPECT_EQ(m_pAosPcscf->GetCurrentPcscfTriedCount(), 1);

    SetCurrentPcscfIndex(2);
    m_pAosPcscf->IncreaseCurrentPcscfTriedCount();
    EXPECT_EQ(m_pAosPcscf->GetCurrentPcscfTriedCount(), 1);

    m_pAosPcscf->ResetAllPcscfTriedCount();

    SetCurrentPcscfIndex(0);
    EXPECT_EQ(m_pAosPcscf->GetCurrentPcscfTriedCount(), 0);

    SetCurrentPcscfIndex(1);
    EXPECT_EQ(m_pAosPcscf->GetCurrentPcscfTriedCount(), 0);

    SetCurrentPcscfIndex(2);
    EXPECT_EQ(m_pAosPcscf->GetCurrentPcscfTriedCount(), 0);
}