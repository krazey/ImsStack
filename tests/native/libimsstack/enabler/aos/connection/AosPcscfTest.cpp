/*
 * Copyright (C) 2021 The Android Open Source Project
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

class AosPcscfTest : public ::testing::Test
{
public:
    AosPcscf* pAosPcscf;
    AStringArray objPcscfAddressPreset;
    IMSList<IMS_SINT32> objPcscfPortPreset;
    MockIAosAppContext objMockIAosAppContext;
    MockIAosConnection objMockIAosConnection;
    MockIAosNConfiguration objMockIAosNConfiguration;
    MockIAosSubscriber objMockIAosSubscriber;
    MockISubscriberConfig objMockISubscriberConfig;

protected:
    virtual void SetUp() override
    {
        EXPECT_CALL(objMockIAosAppContext, GetSlotId())
                .Times(AnyNumber())
                .WillRepeatedly(Return(0));

        const AString strValue = AString("test");
        EXPECT_CALL(objMockIAosAppContext, GetProfileId())
                .Times(AnyNumber())
                .WillRepeatedly(ReturnRef(strValue));

        EXPECT_CALL(objMockIAosAppContext, GetConnection())
                .Times(AnyNumber())
                .WillRepeatedly(Return(&objMockIAosConnection));

        EXPECT_CALL(objMockIAosAppContext, GetSubscriber())
                .Times(AnyNumber())
                .WillRepeatedly(Return(&objMockIAosSubscriber));

        EXPECT_CALL(objMockIAosSubscriber, GetSubscriberConfig(_))
                .Times(AnyNumber())
                .WillRepeatedly(Return(&objMockISubscriberConfig));

        pAosPcscf = new AosPcscf(static_cast<IAosAppContext*>(&objMockIAosAppContext));
        ASSERT_TRUE(pAosPcscf != nullptr);
    }

    virtual void TearDown() override
    {
        if (pAosPcscf)
        {
            delete pAosPcscf;
        }
    }

    void Initializ() { pAosPcscf->Init(); }

    void CleanUp() { pAosPcscf->CleanUp(); }

    void SetRegType(IN AosRegistrationType RegType) { pAosPcscf->m_eRegType = RegType; }

    void SetNConfig(IN IAosNConfiguration* piNc) { pAosPcscf->m_piAosNConfig = piNc; }

    void SetConfigured(IN IMS_BOOL bConfigured) { pAosPcscf->SetConfigured(bConfigured); }

    void SetCurrentPcscfIndex(IN IMS_UINT32 nIndex) { pAosPcscf->m_nCurrentPcscfIndex = nIndex; }

    void SetPcscfs()
    {
        AString strPcscfs = "0.0.0.1, 0.0.0.2, 0.0.0.3";
        objPcscfAddressPreset = strPcscfs.Split(',');
        for (int i = 0; i < objPcscfAddressPreset.GetCount(); i++)
        {
            objPcscfPortPreset.Append(5060 + i);
            pAosPcscf->AddPcscf(objPcscfAddressPreset.GetElementAt(i), objPcscfPortPreset.GetAt(i));
        }
    }

    IMS_BOOL SetDnsQueryTimerExpired()
    {
        if (pAosPcscf->m_piDnsQueryRetryTimer != IMS_NULL)
        {
            pAosPcscf->Timer_TimerExpired(pAosPcscf->m_piDnsQueryRetryTimer);
            return IMS_TRUE;
        }

        return IMS_TRUE;
    }

    void NotifyResult() { pAosPcscf->m_piListener->Pcscf_NotifyResult(IMS_TRUE); }

    IMS_BOOL IsValidPcscf(IN IMS_UINT32 nIndex)
    {
        if (nIndex < pAosPcscf->m_objPcscfList.GetSize())
        {
            Pcscf* pPcscf = pAosPcscf->m_objPcscfList.GetAt(nIndex);
            return pPcscf->IsAvailable();
        }

        return IMS_FALSE;
    }

    IMS_BOOL IsTriedPcscf(IN IMS_UINT32 nIndex)
    {
        if (nIndex < pAosPcscf->m_objPcscfList.GetSize())
        {
            Pcscf* pPcscf = pAosPcscf->m_objPcscfList.GetAt(nIndex);
            return pPcscf->IsTried();
        }

        return IMS_FALSE;
    }
};

TEST_F(AosPcscfTest, Configure_GetFromPco)
{
    EXPECT_CALL(objMockIAosConnection, GetLocalAddress(_))
            .Times(2)
            .WillOnce(ReturnRef(IPAddress::ANY))
            .WillOnce(ReturnRef(IPAddress::LOOPBACK));

    EXPECT_CALL(objMockIAosConnection, GetPcscfAddress(_))
            .Times(AnyNumber())
            .WillRepeatedly(ReturnRef(objPcscfAddressPreset));

    IMSVector<IMS_SINT32> objDiscoveryMethods;
    objDiscoveryMethods.Add(ISubscriberConfig::PCSCF_DISCOVERY_METHOD_PCO);
    EXPECT_CALL(objMockISubscriberConfig, GetPcscfDiscoveryMethods())
            .Times(AnyNumber())
            .WillRepeatedly(ReturnRef(objDiscoveryMethods));
    EXPECT_CALL(objMockISubscriberConfig, GetPcscfAddress())
            .Times(AnyNumber())
            .WillRepeatedly(Return(nullptr));

    MockIAosNConfiguration objMockIAosNConfiguration;
    SetNConfig(&objMockIAosNConfiguration);
    EXPECT_CALL(objMockIAosNConfiguration, GetPcscfPort())
            .Times(AnyNumber())
            .WillRepeatedly(Return(5060));

    SetPcscfs();

    // valid local address type but invalid address
    pAosPcscf->Configure(IPAddress::IPV4);

    // valid local address type and valid address
    pAosPcscf->Configure(IPAddress::IPV4);
}

TEST_F(AosPcscfTest, Configure_GetFromConf)
{
    EXPECT_CALL(objMockIAosConnection, GetLocalAddress(_))
            .Times(1)
            .WillOnce(ReturnRef(IPAddress::LOOPBACK));

    IMSVector<IMS_SINT32> objDiscoveryMethods;
    objDiscoveryMethods.Add(ISubscriberConfig::PCSCF_DISCOVERY_METHOD_CONFIG);
    EXPECT_CALL(objMockISubscriberConfig, GetPcscfDiscoveryMethods())
            .Times(AnyNumber())
            .WillRepeatedly(ReturnRef(objDiscoveryMethods));

    const AString strAddress = AString("0.0.0.4");
    ServerAddress* pSa = new ServerAddress(strAddress, 5060);
    IMSVector<ServerAddress*> objPcscfAddresses;
    objPcscfAddresses.Add(pSa);
    EXPECT_CALL(objMockISubscriberConfig, GetPcscfAddresses())
            .Times(AnyNumber())
            .WillRepeatedly(ReturnRef(objPcscfAddresses));

    // unknown local address type
    pAosPcscf->Configure(IPAddress::UNKNOWN);

    if (pSa)
    {
        delete pSa;
    }
}

TEST_F(AosPcscfTest, Configure_ProcessDnsQuery)
{
    EXPECT_CALL(objMockIAosConnection, GetLocalAddress(_))
            .Times(AnyNumber())
            .WillRepeatedly(ReturnRef(IPAddress::LOOPBACK));

    IMSVector<IMS_SINT32> objDiscoveryMethods;
    objDiscoveryMethods.Add(ISubscriberConfig::PCSCF_DISCOVERY_METHOD_CONFIG);
    EXPECT_CALL(objMockISubscriberConfig, GetPcscfDiscoveryMethods())
            .Times(AnyNumber())
            .WillRepeatedly(ReturnRef(objDiscoveryMethods));

    const AString strAddress = AString("aaa.bbb.ccc");
    ServerAddress* pSa = new ServerAddress(strAddress, 5060);
    IMSVector<ServerAddress*> objPcscfAddresses;
    objPcscfAddresses.Add(pSa);
    EXPECT_CALL(objMockISubscriberConfig, GetPcscfAddresses())
            .Times(AnyNumber())
            .WillRepeatedly(ReturnRef(objPcscfAddresses));

    EXPECT_CALL(objMockIAosConnection, GetHostByName(_, _, _))
            .Times(AnyNumber())
            .WillRepeatedly(Return(-1));

    // unknown local address type
    pAosPcscf->Configure(IPAddress::UNKNOWN);

    EXPECT_TRUE(SetDnsQueryTimerExpired());

    if (pSa)
    {
        delete pSa;
    }
}

TEST_F(AosPcscfTest, IsConfigured)
{
    MockIAosRegistration objMockIAosRegistration;
    EXPECT_CALL(objMockIAosAppContext, GetRegistration())
            .Times(AnyNumber())
            .WillRepeatedly(Return(&objMockIAosRegistration));
    EXPECT_CALL(objMockIAosRegistration, GetRegType())
            .Times(AnyNumber())
            .WillRepeatedly(Return(AosRegistrationType::NORMAL));

    SetConfigured(IMS_TRUE);
    EXPECT_TRUE(pAosPcscf->IsConfigured());
    CleanUp();
    EXPECT_FALSE(pAosPcscf->IsConfigured());
}

TEST_F(AosPcscfTest, IsAsyncDnsDiscovery)
{
    MockIAosRegistration objMockIAosRegistration;
    EXPECT_CALL(objMockIAosAppContext, GetRegistration())
            .Times(AnyNumber())
            .WillRepeatedly(Return(&objMockIAosRegistration));
    EXPECT_CALL(objMockIAosRegistration, GetRegType())
            .Times(AnyNumber())
            .WillRepeatedly(Return(AosRegistrationType::NORMAL));

    SetRegType(AosRegistrationType::RCS);
    EXPECT_TRUE(pAosPcscf->IsAsyncDnsDiscovery());
    Initializ();
    EXPECT_FALSE(pAosPcscf->IsAsyncDnsDiscovery());
}

TEST_F(AosPcscfTest, IsSinglePcoScheme)
{
    IMSVector<IMS_SINT32> objDiscoveryMethods;
    objDiscoveryMethods.Add(ISubscriberConfig::PCSCF_DISCOVERY_METHOD_CONFIG);
    objDiscoveryMethods.Add(ISubscriberConfig::PCSCF_DISCOVERY_METHOD_PCO);
    EXPECT_CALL(objMockISubscriberConfig, GetPcscfDiscoveryMethods())
            .Times(AnyNumber())
            .WillRepeatedly(ReturnRef(objDiscoveryMethods));

    EXPECT_FALSE(pAosPcscf->IsSinglePcoScheme());
    objDiscoveryMethods.RemoveAt(0);
    EXPECT_TRUE(pAosPcscf->IsSinglePcoScheme());
}

TEST_F(AosPcscfTest, GetPcscfs)
{
    SetPcscfs();
    AStringArray objGetPcscfs = pAosPcscf->GetPcscfs();
    for (int i = 0; i < objGetPcscfs.GetCount(); i++)
    {
        EXPECT_EQ(objGetPcscfs.GetElementAt(i), objPcscfAddressPreset.GetElementAt(i));
    }
}

TEST_F(AosPcscfTest, GetPcscfsPorts)
{
    SetPcscfs();
    IMSList<IMS_SINT32> objGetPcscfsPorts = pAosPcscf->GetPcscfsPorts();
    for (int i = 0; i < objGetPcscfsPorts.GetSize(); i++)
    {
        EXPECT_EQ(objGetPcscfsPorts.GetAt(i), objPcscfPortPreset.GetAt(i));
    }
}

TEST_F(AosPcscfTest, UpdatePcscfs)
{
    MockIAosNConfiguration objMockIAosNConfiguration;
    SetNConfig(&objMockIAosNConfiguration);
    EXPECT_CALL(objMockIAosNConfiguration, GetPcscfPort())
            .Times(AnyNumber())
            .WillRepeatedly(Return(5060));

    SetPcscfs();
    AString strNewPcscfs = "0.0.0.3, 0.0.0.4";
    AStringArray objNewPcscfs = strNewPcscfs.Split(',');
    pAosPcscf->UpdatePcscfs(objNewPcscfs);

    AStringArray objGetPcscfs = pAosPcscf->GetPcscfs();
    for (int i = 0; i < objGetPcscfs.GetCount(); i++)
    {
        EXPECT_EQ(objGetPcscfs.GetElementAt(i), objNewPcscfs.GetElementAt(i));
    }
}

TEST_F(AosPcscfTest, HasPcscf)
{
    SetPcscfs();
    EXPECT_TRUE(pAosPcscf->HasPcscf(objPcscfAddressPreset.GetCount() - 1));
    EXPECT_FALSE(pAosPcscf->HasPcscf(objPcscfAddressPreset.GetCount()));
}

TEST_F(AosPcscfTest, GetPcscfCount)
{
    SetPcscfs();
    EXPECT_EQ(pAosPcscf->GetPcscfCount(), objPcscfAddressPreset.GetCount());
}

TEST_F(AosPcscfTest, RemoveCurrentPcscf)
{
    SetPcscfs();
    objPcscfAddressPreset.RemoveElementAt(pAosPcscf->GetCurrentIndex());
    pAosPcscf->RemoveCurrentPcscf();

    AStringArray objGetPcscfs = pAosPcscf->GetPcscfs();
    for (int i = 0; i < objGetPcscfs.GetCount(); i++)
    {
        EXPECT_EQ(objGetPcscfs.GetElementAt(i), objPcscfAddressPreset.GetElementAt(i));
    }
}

TEST_F(AosPcscfTest, ManageValidPcscf)
{
    SetPcscfs();
    EXPECT_TRUE(IsValidPcscf(pAosPcscf->GetCurrentIndex()));

    pAosPcscf->SetCurrentPcscfInvalid();
    EXPECT_FALSE(IsValidPcscf(pAosPcscf->GetCurrentIndex()));

    pAosPcscf->SetAllPcscfValid();
    EXPECT_TRUE(IsValidPcscf(pAosPcscf->GetCurrentIndex()));
}

TEST_F(AosPcscfTest, ManageTriedPcscf)
{
    SetPcscfs();
    AStringArray objPcscfs = pAosPcscf->GetPcscfs();
    for (int i = 0; i < objPcscfs.GetCount(); i++)
    {
        EXPECT_FALSE(IsTriedPcscf(i));
        SetCurrentPcscfIndex(i);
        pAosPcscf->SetCurrentPcscfTried();
        EXPECT_TRUE(IsTriedPcscf(i));
    }
    EXPECT_TRUE(pAosPcscf->IsAllPcscfTried());

    pAosPcscf->ResetAllPcscfTried();
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

    AStringArray objPcscfs = pAosPcscf->GetPcscfs();
    for (int i = 0; i < objPcscfs.GetCount(); i++)
    {
        SetCurrentPcscfIndex(i);
        EXPECT_TRUE(pAosPcscf->GetCurrentPcscf(objPcscf, nPort));
        EXPECT_EQ(objPcscf, objPcscfAddressPreset.GetElementAt(i));
        EXPECT_EQ(nPort, objPcscfPortPreset.GetAt(i));
    }

    SetCurrentPcscfIndex(objPcscfs.GetCount());
    EXPECT_FALSE(pAosPcscf->GetCurrentPcscf(objPcscf, nPort));
}

TEST_F(AosPcscfTest, GetCurrentIndex)
{
    SetPcscfs();
    AStringArray objPcscfs = pAosPcscf->GetPcscfs();
    for (int i = 0; i < objPcscfs.GetCount(); i++)
    {
        SetCurrentPcscfIndex(i);
        EXPECT_EQ(pAosPcscf->GetCurrentIndex(), i);
    }
}

TEST_F(AosPcscfTest, IsFirstPcscf)
{
    SetCurrentPcscfIndex(0);
    EXPECT_TRUE(pAosPcscf->IsFirstPcscf());
    SetCurrentPcscfIndex(1);
    EXPECT_FALSE(pAosPcscf->IsFirstPcscf());
}

TEST_F(AosPcscfTest, GetFirstPcscf)
{
    AString objPcscf;
    IMS_UINT32 nPort;

    EXPECT_FALSE(pAosPcscf->GetFirstPcscf(objPcscf, nPort));
    SetPcscfs();
    EXPECT_TRUE(pAosPcscf->GetFirstPcscf(objPcscf, nPort));
    EXPECT_EQ(objPcscf, objPcscfAddressPreset.GetElementAt(0));
    EXPECT_EQ(nPort, objPcscfPortPreset.GetAt(0));
}

TEST_F(AosPcscfTest, HasNextPcscf)
{
    MockIAosNConfiguration objMockIAosNConfiguration;
    SetNConfig(&objMockIAosNConfiguration);
    EXPECT_CALL(objMockIAosNConfiguration, GetRegRetryDefaultPolicy())
            .Times(AnyNumber())
            .WillRepeatedly(
                    Return(CarrierConfig::Assets::DEFAULT_RETRY_POLICY_CIRCULAR_NEXT_PCSCF));

    EXPECT_FALSE(pAosPcscf->HasNextPcscf());
    SetPcscfs();
    EXPECT_TRUE(pAosPcscf->HasNextPcscf());
}

TEST_F(AosPcscfTest, GetNextPcscfIndex)
{
    MockIAosNConfiguration objMockIAosNConfiguration;
    SetNConfig(&objMockIAosNConfiguration);
    EXPECT_CALL(objMockIAosNConfiguration, GetRegRetryDefaultPolicy())
            .Times(AnyNumber())
            .WillRepeatedly(
                    Return(CarrierConfig::Assets::DEFAULT_RETRY_POLICY_CIRCULAR_NEXT_PCSCF));

    EXPECT_LT(pAosPcscf->GetNextPcscfIndex(), 0);
    SetPcscfs();
    AStringArray objPcscfs = pAosPcscf->GetPcscfs();
    for (int i = 0; i < objPcscfs.GetCount(); i++)
    {
        SetCurrentPcscfIndex(i);
        EXPECT_GE(pAosPcscf->GetNextPcscfIndex(), 0);
    }
}

TEST_F(AosPcscfTest, GetNextPcscf)
{
    AString objPcscf;
    IMS_UINT32 nPort;
    MockIAosNConfiguration objMockIAosNConfiguration;
    SetNConfig(&objMockIAosNConfiguration);
    EXPECT_CALL(objMockIAosNConfiguration, GetRegRetryDefaultPolicy())
            .Times(AnyNumber())
            .WillRepeatedly(
                    Return(CarrierConfig::Assets::DEFAULT_RETRY_POLICY_CIRCULAR_NEXT_PCSCF));

    SetPcscfs();
    AStringArray objPcscfs = pAosPcscf->GetPcscfs();
    for (int i = 0; i < objPcscfs.GetCount(); i++)
    {
        EXPECT_TRUE(pAosPcscf->GetNextPcscf(objPcscf, nPort));
        EXPECT_EQ(objPcscf, objPcscfAddressPreset.GetElementAt(pAosPcscf->GetCurrentIndex()));
        EXPECT_EQ(nPort, objPcscfPortPreset.GetAt(pAosPcscf->GetCurrentIndex()));
    }
}

TEST_F(AosPcscfTest, SetFirstPcscfIndex)
{
    pAosPcscf->SetFirstPcscfIndex();
    EXPECT_EQ(pAosPcscf->GetCurrentIndex(), 0);
}

TEST_F(AosPcscfTest, CheckAndProcessChangeFromPco_Reorder)
{
    AString strNewPcscfs = "0.0.0.4, 0.0.0.1";
    AStringArray objNewPcscfs = strNewPcscfs.Split(',');
    EXPECT_CALL(objMockIAosConnection, GetPcscfAddress(_))
            .Times(AnyNumber())
            .WillRepeatedly(ReturnRef(objNewPcscfs));

    EXPECT_CALL(objMockISubscriberConfig, GetPcscfAddress())
            .Times(AnyNumber())
            .WillRepeatedly(Return(nullptr));

    MockIAosNConfiguration objMockIAosNConfiguration;
    SetNConfig(&objMockIAosNConfiguration);
    EXPECT_CALL(objMockIAosNConfiguration, GetPcscfPort())
            .Times(AnyNumber())
            .WillRepeatedly(Return(5060));

    SetPcscfs();
    EXPECT_TRUE(pAosPcscf->CheckAndProcessChangeFromPco());
    EXPECT_EQ(pAosPcscf->GetChangedType(), IAosPcscf::TYPE_CHANGED_REORDER);
}

TEST_F(AosPcscfTest, CheckAndProcessChangeFromPco_Different)
{
    AString strNewPcscfs = "0.0.0.4, 0.0.0.5";
    AStringArray objNewPcscfs = strNewPcscfs.Split(',');
    EXPECT_CALL(objMockIAosConnection, GetPcscfAddress(_))
            .Times(AnyNumber())
            .WillRepeatedly(ReturnRef(objNewPcscfs));

    EXPECT_CALL(objMockISubscriberConfig, GetPcscfAddress())
            .Times(AnyNumber())
            .WillRepeatedly(Return(nullptr));

    MockIAosNConfiguration objMockIAosNConfiguration;
    SetNConfig(&objMockIAosNConfiguration);
    EXPECT_CALL(objMockIAosNConfiguration, GetPcscfPort())
            .Times(AnyNumber())
            .WillRepeatedly(Return(5060));

    SetPcscfs();
    EXPECT_TRUE(pAosPcscf->CheckAndProcessChangeFromPco());
    EXPECT_EQ(pAosPcscf->GetChangedType(), IAosPcscf::TYPE_CHANGED_DIFFERENT);
}

TEST_F(AosPcscfTest, SetListener)
{
    MockIAosPcscfListener objMockIAosPcscfListener;
    EXPECT_CALL(objMockIAosPcscfListener, Pcscf_NotifyResult(_)).Times(1);

    pAosPcscf->SetListener(static_cast<IAosPcscfListener*>(&objMockIAosPcscfListener));
    NotifyResult();
}
