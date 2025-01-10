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
#include "MockITimer.h"
#include "PlatformContext.h"
#include "ServerAddress.h"
#include "TestTimerService.h"

#include "interface/MockIAosAppContext.h"
#include "interface/MockIAosConnection.h"
#include "interface/MockIAosNConfiguration.h"
#include "interface/MockIAosPcscf.h"
#include "interface/MockIAosSubscriber.h"
#include "interface/MockIAosRegistration.h"
#include "provider/AosProvider.h"
#include "../../../config/interface/common/MockISubscriberConfig.h"

using ::testing::_;
using ::testing::AnyNumber;
using ::testing::DoAll;
using ::testing::Return;
using ::testing::ReturnRef;
using ::testing::SetArgPointee;

#define DECLARE_USING(Base)             \
    using Base::AddPcscf;               \
    using Base::GetChangedPcscfs;       \
    using Base::SetConfigured;          \
    using Base::ProcessDiscovery;       \
    using Base::GetNextDiscoveryMethod; \
    using Base::GetFromConf;            \
    using Base::ProcessDnsQuery;        \
    using Base::CleanAll;               \
    using Base::StartTimer;             \
    using Base::StopTimer;              \
    using Base::Timer_TimerExpired;     \
    using Base::TimerToString;

const IMS_SINT32 SLOT_ID = 0;
const AString PROFILE_ID = AString("test");
const IMS_SINT32 DEFAULT_PORT = 5060;

class TestAosPcscf : public AosPcscf
{
public:
    DECLARE_USING(AosPcscf)

    explicit inline TestAosPcscf(IN IAosAppContext* piAppContext) :
            AosPcscf(piAppContext)
    {
    }

    inline IAosPcscfListener* GetListener() { return m_piListener; }
    inline ITimer* GetDnsQueryRetryTimer() { return m_piDnsQueryRetryTimer; }
    inline void SetRegType(IN AosRegistrationType eType) { m_eRegType = eType; }
    inline AosRegistrationType GetRegType() { return m_eRegType; }
    inline void SetCurrentPcscfIndex(IN IMS_UINT32 nIndex) { m_nCurrentPcscfIndex = nIndex; }
    inline IMS_UINT32 GetCurrentPcscfIndex() { return m_nCurrentPcscfIndex; }
    inline void SetDiscoveryMethodIndex(IN IMS_UINT32 nIndex) { m_nDiscoveryMethodIndex = nIndex; }
    inline void SetOtherIpTypeRequired(IN IMS_BOOL bRequired)
    {
        m_bOtherIpTypeRequired = bRequired;
    }
    inline IMS_BOOL GetOtherIpTypeRequired() { return m_bOtherIpTypeRequired; }
    inline ImsList<Pcscf*> GetPcscfList() { return m_objPcscfList; }
    void AddRetryHost()
    {
        RetryHost* pRetryHost = new RetryHost(AString("RetryHost"), 5060, IpAddress::IPV4);
        m_objRetryHostList.Append(pRetryHost);
    }
};

class AosPcscfTest : public ::testing::Test
{
public:
    inline AosPcscfTest() :
            m_pAosPcscf(IMS_NULL),
            m_objMockITimer(m_objTimerService.GetMockTimer())
    {
        PlatformContext::GetInstance()->SetService(
                PlatformContext::SERVICE_TIMER, &m_objTimerService);

        m_piAosNConfiguration = AosProvider::GetInstance()->GetNConfiguration(SLOT_ID);
        AosProvider::GetInstance()->SetNConfiguration(&m_objMockIAosNConfiguration, SLOT_ID);
    }
    inline virtual ~AosPcscfTest()
    {
        AosProvider::GetInstance()->SetNConfiguration(m_piAosNConfiguration, SLOT_ID);
        PlatformContext::GetInstance()->SetService(PlatformContext::SERVICE_TIMER, IMS_NULL);
    }

    TestAosPcscf* m_pAosPcscf;
    TestTimerService m_objTimerService;

    AStringArray m_objPcscfAddressPreset;
    ImsList<IMS_SINT32> m_objPcscfPortPreset;
    ImsVector<ServerAddress*> m_objConfiguredPcscfs;
    IAosNConfiguration* m_piAosNConfiguration;
    ImsVector<IMS_SINT32> m_objEmptyDiscoveryMethods;

    MockIAosAppContext m_objMockIAosAppContext;
    MockIAosConnection m_objMockIAosConnection;
    MockIAosNConfiguration m_objMockIAosNConfiguration;
    MockIAosPcscfListener objMockIAosPcscfListener;
    MockIAosRegistration objMockIAosRegistration;
    MockIAosSubscriber m_objMockIAosSubscriber;
    MockISubscriberConfig m_objMockISubscriberConfig;
    MockITimer& m_objMockITimer;

protected:
    void SetUp() override
    {
        ON_CALL(m_objMockIAosAppContext, GetProfileId()).WillByDefault(ReturnRef(PROFILE_ID));
        ON_CALL(m_objMockIAosAppContext, GetSlotId()).WillByDefault(Return(SLOT_ID));
        ON_CALL(m_objMockIAosAppContext, GetConnection())
                .WillByDefault(Return(&m_objMockIAosConnection));
        ON_CALL(m_objMockIAosAppContext, GetRegistration())
                .WillByDefault(Return(&objMockIAosRegistration));
        ON_CALL(m_objMockIAosAppContext, GetSubscriber())
                .WillByDefault(Return(&m_objMockIAosSubscriber));
        ON_CALL(m_objMockIAosSubscriber, GetSubscriberConfig(_))
                .WillByDefault(Return(&m_objMockISubscriberConfig));
        ON_CALL(m_objMockIAosNConfiguration, GetPcscfPort()).WillByDefault(Return(DEFAULT_PORT));
        ON_CALL(m_objMockIAosNConfiguration, GetRegRetryCountPerPcscf()).WillByDefault(Return(0));
        ON_CALL(m_objMockIAosNConfiguration, GetRegRetryCountOnSinglePcscf())
                .WillByDefault(Return(2));
        ON_CALL(m_objMockISubscriberConfig, GetPcscfDiscoveryMethods())
                .WillByDefault(ReturnRef(m_objEmptyDiscoveryMethods));

        m_pAosPcscf = new TestAosPcscf(&m_objMockIAosAppContext);
        ASSERT_TRUE(m_pAosPcscf != nullptr);
    }

    void TearDown() override
    {
        m_objPcscfAddressPreset.RemoveAllElements();
        m_objPcscfPortPreset.Clear();

        if (m_pAosPcscf)
        {
            m_pAosPcscf->CleanAll();
            delete m_pAosPcscf;
            m_pAosPcscf = IMS_NULL;
        }
    }

    void PreparePcscfPreset()
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

    void PrepareConfiguredPcscfs(IN AString strAddresses)
    {
        AStringArray objPcscfAddress = strAddresses.Split(',');
        for (int i = 0; i < objPcscfAddress.GetCount(); i++)
        {
            ServerAddress* pSa = new ServerAddress(objPcscfAddress.GetElementAt(i), 0);
            m_objConfiguredPcscfs.Add(pSa);
        }
    }

    void ClearConfiguredPcscfs()
    {
        for (int i = 0; i < m_objConfiguredPcscfs.GetSize(); i++)
        {
            ServerAddress* pSa = m_objConfiguredPcscfs.GetAt(i);
            if (pSa)
            {
                delete pSa;
            }
        }
        m_objConfiguredPcscfs.Clear();
    }
};

TEST_F(AosPcscfTest, UpdateRegTypeWhenInit)
{
    EXPECT_CALL(objMockIAosRegistration, GetRegType())
            .WillRepeatedly(Return(AosRegistrationType::FAKE));

    m_pAosPcscf->Init();

    EXPECT_EQ(m_pAosPcscf->GetRegType(), AosRegistrationType::FAKE);
}

TEST_F(AosPcscfTest, RemoveListenerWhenCleanUp)
{
    m_pAosPcscf->SetListener(&objMockIAosPcscfListener);
    m_pAosPcscf->SetConfigured(IMS_TRUE);

    m_pAosPcscf->CleanUp();

    EXPECT_EQ(m_pAosPcscf->GetListener(), nullptr);
    EXPECT_FALSE(m_pAosPcscf->IsConfigured());
}

TEST_F(AosPcscfTest, DoNotProcessDiscoveryForInvalidLocalAddressWhenConfigureWithIpVersion)
{
    ON_CALL(m_objMockIAosConnection, GetLocalAddress(_)).WillByDefault(ReturnRef(IpAddress::ANY));

    EXPECT_CALL(m_objMockISubscriberConfig, GetPcscfDiscoveryMethods()).Times(0);

    m_pAosPcscf->Configure(IpAddress::IPV4);

    EXPECT_FALSE(m_pAosPcscf->GetOtherIpTypeRequired());
}

TEST_F(AosPcscfTest, ProcessDiscoveryForValidLocalAddressWhenConfigureWithIpVersion)
{
    ON_CALL(m_objMockIAosConnection, GetLocalAddress(_))
            .WillByDefault(ReturnRef(IpAddress::LOOPBACK));

    EXPECT_CALL(m_objMockISubscriberConfig, GetPcscfDiscoveryMethods());

    m_pAosPcscf->Configure(IpAddress::IPV4);

    EXPECT_FALSE(m_pAosPcscf->GetOtherIpTypeRequired());
}

TEST_F(AosPcscfTest, TryProcessDiscoveryForEachIpVerionWhenConfigureWithoutIpVersion)
{
    ImsVector<IMS_SINT32> objDiscoveryMethods;
    objDiscoveryMethods.Add(ISubscriberConfig::PCSCF_DISCOVERY_METHOD_PCO);
    ON_CALL(m_objMockISubscriberConfig, GetPcscfDiscoveryMethods())
            .WillByDefault(ReturnRef(objDiscoveryMethods));
    ON_CALL(m_objMockIAosConnection, GetLocalAddress(IpAddress::UNKNOWN))
            .WillByDefault(ReturnRef(IpAddress::LOOPBACK));
    ON_CALL(m_objMockIAosConnection, GetLocalAddress(IpAddress::IPV6))
            .WillByDefault(ReturnRef(IpAddress::IPv6LOOPBACK));

    AStringArray objEmptyPcscfSet;
    EXPECT_CALL(m_objMockIAosConnection, GetPcscfAddress(IpAddress::IPV4))
            .WillOnce(ReturnRef(objEmptyPcscfSet));
    EXPECT_CALL(m_objMockIAosConnection, GetPcscfAddress(IpAddress::IPV6))
            .WillOnce(ReturnRef(objEmptyPcscfSet));

    m_pAosPcscf->Configure();
}

TEST_F(AosPcscfTest, IsConfiguredReturnsWhetherPcscfIsConfigured)
{
    m_pAosPcscf->SetConfigured(IMS_TRUE);
    EXPECT_TRUE(m_pAosPcscf->IsConfigured());

    m_pAosPcscf->SetConfigured(IMS_FALSE);
    EXPECT_FALSE(m_pAosPcscf->IsConfigured());
}

TEST_F(AosPcscfTest, IsAsyncDnsDiscoveryReturnsWhetherRegTypeIsRcs)
{
    m_pAosPcscf->SetRegType(AosRegistrationType::NORMAL);
    EXPECT_FALSE(m_pAosPcscf->IsAsyncDnsDiscovery());

    m_pAosPcscf->SetRegType(AosRegistrationType::RCS);
    EXPECT_TRUE(m_pAosPcscf->IsAsyncDnsDiscovery());
}

TEST_F(AosPcscfTest, IsSinglePcoSchemeReturnsFalseIfSubscriberIsInvalid)
{
    m_pAosPcscf->SetRegType(AosRegistrationType::FAKE);
    ImsVector<IMS_SINT32> objDiscoveryMethods;
    objDiscoveryMethods.Add(ISubscriberConfig::PCSCF_DISCOVERY_METHOD_PCO);
    ON_CALL(m_objMockISubscriberConfig, GetPcscfDiscoveryMethods())
            .WillByDefault(ReturnRef(objDiscoveryMethods));
    ON_CALL(m_objMockIAosSubscriber, GetSubscriberConfig(IAosSubscriber::FAKE))
            .WillByDefault(Return(nullptr));

    IMS_BOOL bResult = m_pAosPcscf->IsSinglePcoScheme();

    EXPECT_FALSE(bResult);
}

TEST_F(AosPcscfTest, IsSinglePcoSchemeReturnsFalseIfTwoDiscoveryMethodExist)
{
    ImsVector<IMS_SINT32> objDiscoveryMethods;
    objDiscoveryMethods.Add(ISubscriberConfig::PCSCF_DISCOVERY_METHOD_CONFIG);
    objDiscoveryMethods.Add(ISubscriberConfig::PCSCF_DISCOVERY_METHOD_PCO);
    ON_CALL(m_objMockISubscriberConfig, GetPcscfDiscoveryMethods())
            .WillByDefault(ReturnRef(objDiscoveryMethods));

    IMS_BOOL bResult = m_pAosPcscf->IsSinglePcoScheme();

    EXPECT_FALSE(bResult);
}

TEST_F(AosPcscfTest, IsSinglePcoSchemeReturnsFalseIfNoDiscoveryMethodExist)
{
    ImsVector<IMS_SINT32> objDiscoveryMethods;
    ON_CALL(m_objMockISubscriberConfig, GetPcscfDiscoveryMethods())
            .WillByDefault(ReturnRef(objDiscoveryMethods));

    IMS_BOOL bResult = m_pAosPcscf->IsSinglePcoScheme();

    EXPECT_FALSE(bResult);
}

TEST_F(AosPcscfTest, IsSinglePcoSchemeReturnsTrueIfOneDiscoveryMethodExist)
{
    ImsVector<IMS_SINT32> objDiscoveryMethods;
    objDiscoveryMethods.Add(ISubscriberConfig::PCSCF_DISCOVERY_METHOD_PCO);
    ON_CALL(m_objMockISubscriberConfig, GetPcscfDiscoveryMethods())
            .WillByDefault(ReturnRef(objDiscoveryMethods));

    IMS_BOOL bResult = m_pAosPcscf->IsSinglePcoScheme();

    EXPECT_TRUE(bResult);
}

TEST_F(AosPcscfTest, GetPcscfsReturnsPcscfAddressSet)
{
    m_pAosPcscf->AddPcscf(AString("0.0.0.1"), 5060);
    m_pAosPcscf->AddPcscf(AString("0.0.0.2"), 5061);
    m_pAosPcscf->AddPcscf(AString("0.0.0.3"), 5062);

    AStringArray objGetPcscfs = m_pAosPcscf->GetPcscfs();

    EXPECT_EQ(objGetPcscfs.GetCount(), 3);
    EXPECT_EQ(objGetPcscfs.GetElementAt(0), AString("0.0.0.1"));
    EXPECT_EQ(objGetPcscfs.GetElementAt(1), AString("0.0.0.2"));
    EXPECT_EQ(objGetPcscfs.GetElementAt(2), AString("0.0.0.3"));
}

TEST_F(AosPcscfTest, GetPcscfsPortsReturnsPcscfPortSet)
{
    m_pAosPcscf->AddPcscf(AString("0.0.0.1"), 5060);
    m_pAosPcscf->AddPcscf(AString("0.0.0.2"), 5061);
    m_pAosPcscf->AddPcscf(AString("0.0.0.3"), 5062);

    ImsList<IMS_SINT32> objGetPcscfsPorts = m_pAosPcscf->GetPcscfsPorts();

    EXPECT_EQ(objGetPcscfsPorts.GetSize(), 3);
    EXPECT_EQ(objGetPcscfsPorts.GetAt(0), 5060);
    EXPECT_EQ(objGetPcscfsPorts.GetAt(1), 5061);
    EXPECT_EQ(objGetPcscfsPorts.GetAt(2), 5062);
}

TEST_F(AosPcscfTest, ReplaceWithNewPcscfsWhenUpdatePcscfs)
{
    PreparePcscfPreset();
    AString strNewPcscfs = "0.0.0.4, 0.0.0.5, 0.0.0.4, 0.0.0.5";
    AStringArray objNewPcscfs = strNewPcscfs.Split(',');
    ImsList<IMS_SINT32> objNewPorts;
    objNewPorts.Append(5000);

    m_pAosPcscf->UpdatePcscfs(objNewPcscfs, objNewPorts);

    AStringArray objGetPcscfs = m_pAosPcscf->GetPcscfs();
    ImsList<IMS_SINT32> objGetPcscfsPorts = m_pAosPcscf->GetPcscfsPorts();
    // Pcscf that has same address and port would added only once
    EXPECT_EQ(objGetPcscfs.GetCount(), 3);
    EXPECT_EQ(objGetPcscfs.GetElementAt(0), objNewPcscfs.GetElementAt(0));
    EXPECT_EQ(objGetPcscfsPorts.GetAt(0), objNewPorts.GetAt(0));
    EXPECT_EQ(objGetPcscfs.GetElementAt(1), objNewPcscfs.GetElementAt(1));
    // If the port number is smaller than the address number, fill in the default port.
    EXPECT_EQ(objGetPcscfsPorts.GetAt(1), DEFAULT_PORT);
    EXPECT_EQ(objGetPcscfs.GetElementAt(2), objNewPcscfs.GetElementAt(2));
    EXPECT_EQ(objGetPcscfsPorts.GetAt(2), DEFAULT_PORT);
}

TEST_F(AosPcscfTest, HasPcscfReturnsWhetherPcscfOfReceivedIndexExist)
{
    PreparePcscfPreset();

    EXPECT_TRUE(m_pAosPcscf->HasPcscf(m_objPcscfAddressPreset.GetCount() - 1));
    EXPECT_FALSE(m_pAosPcscf->HasPcscf(m_objPcscfAddressPreset.GetCount()));
}

TEST_F(AosPcscfTest, GetPcscfCountReturnsSizeOfPcscfList)
{
    PreparePcscfPreset();

    IMS_UINT32 nExpectedCount = m_objPcscfAddressPreset.GetCount();
    EXPECT_EQ(m_pAosPcscf->GetPcscfCount(), nExpectedCount);
}

TEST_F(AosPcscfTest, RemovePcscfOfCurrentIndexWhenRemoveCurrentPcscf)
{
    m_pAosPcscf->AddPcscf(AString("0.0.0.1"), 5060);
    m_pAosPcscf->AddPcscf(AString("0.0.0.2"), 5061);
    m_pAosPcscf->AddPcscf(AString("0.0.0.3"), 5062);
    m_pAosPcscf->SetCurrentPcscfIndex(1);

    m_pAosPcscf->RemoveCurrentPcscf();

    AStringArray objGetPcscfs = m_pAosPcscf->GetPcscfs();
    EXPECT_EQ(objGetPcscfs.GetCount(), 2);
    EXPECT_EQ(objGetPcscfs.GetElementAt(0), AString("0.0.0.1"));
    EXPECT_EQ(objGetPcscfs.GetElementAt(1), AString("0.0.0.3"));
}

TEST_F(AosPcscfTest, UpdateAvailabilityOfPcscf)
{
    IMS_UINT32 nUnavailableTimeSec = 5;
    IMS_UINT32 nCurrentPcscfIndex = 0;
    m_pAosPcscf->AddPcscf(AString("0.0.0.1"), 5060);
    m_pAosPcscf->SetCurrentPcscfIndex(nCurrentPcscfIndex);
    Pcscf* pPcscf = m_pAosPcscf->GetPcscfList().GetAt(nCurrentPcscfIndex);

    // Set invalid with timer
    m_pAosPcscf->SetCurrentPcscfInvalid(IMS_TRUE, nUnavailableTimeSec);
    EXPECT_FALSE(pPcscf->IsAvailable());

    m_pAosPcscf->SetAllPcscfValid();
    EXPECT_TRUE(pPcscf->IsAvailable());

    // Set invalid without timer
    m_pAosPcscf->SetCurrentPcscfInvalid();
    EXPECT_FALSE(pPcscf->IsAvailable());
}

TEST_F(AosPcscfTest, UpdateWhetherPcscfWasTried)
{
    PreparePcscfPreset();
    AStringArray objPcscfs = m_pAosPcscf->GetPcscfs();

    EXPECT_FALSE(m_pAosPcscf->IsAllPcscfTried());
    for (IMS_UINT32 nIndex = 0; nIndex < objPcscfs.GetCount(); nIndex++)
    {
        Pcscf* pPcscf = m_pAosPcscf->GetPcscfList().GetAt(nIndex);
        EXPECT_FALSE(pPcscf->IsTried());

        m_pAosPcscf->SetCurrentPcscfIndex(nIndex);
        m_pAosPcscf->SetCurrentPcscfTried();
        EXPECT_TRUE(pPcscf->IsTried());
    }
    EXPECT_TRUE(m_pAosPcscf->IsAllPcscfTried());

    m_pAosPcscf->ResetAllPcscfTried();
    for (IMS_UINT32 nIndex = 0; nIndex < objPcscfs.GetCount(); nIndex++)
    {
        Pcscf* pPcscf = m_pAosPcscf->GetPcscfList().GetAt(nIndex);
        EXPECT_FALSE(pPcscf->IsTried());
    }
}

TEST_F(AosPcscfTest, DoNotIncreaseCurrentPcscfTriedCountIfConfiguredRetryCountIsZero)
{
    m_pAosPcscf->AddPcscf(AString("0.0.0.1"), 5060);
    m_pAosPcscf->SetCurrentPcscfIndex(0);
    ON_CALL(m_objMockIAosNConfiguration, GetRegRetryCountOnSinglePcscf()).WillByDefault(Return(0));

    m_pAosPcscf->IncreaseCurrentPcscfTriedCount();

    EXPECT_EQ(m_pAosPcscf->GetCurrentPcscfTriedCount(), 0);
}

TEST_F(AosPcscfTest, IncreaseCurrentPcscfTriedCountIfConfiguredRetryCountIsNotZero)
{
    m_pAosPcscf->AddPcscf(AString("0.0.0.1"), 5060);
    m_pAosPcscf->SetCurrentPcscfIndex(0);
    ON_CALL(m_objMockIAosNConfiguration, GetRegRetryCountOnSinglePcscf()).WillByDefault(Return(2));

    m_pAosPcscf->IncreaseCurrentPcscfTriedCount();

    EXPECT_EQ(m_pAosPcscf->GetCurrentPcscfTriedCount(), 1);
}

TEST_F(AosPcscfTest, DoNotHandleInvalidPcscfIndexWhenIncreaseCurrentPcscfTriedCount)
{
    m_pAosPcscf->AddPcscf(AString("0.0.0.1"), 5060);
    m_pAosPcscf->SetCurrentPcscfIndex(1);

    m_pAosPcscf->IncreaseCurrentPcscfTriedCount();

    EXPECT_EQ(m_pAosPcscf->GetCurrentPcscfTriedCount(), 0);
}

TEST_F(AosPcscfTest, DoNotResetCurrentPcscfTriedCountIfConfiguredRetryCountIsZero)
{
    // set tried count of current pcscf as 1
    m_pAosPcscf->AddPcscf(AString("0.0.0.1"), 5060);
    m_pAosPcscf->SetCurrentPcscfIndex(0);
    m_pAosPcscf->IncreaseCurrentPcscfTriedCount();
    ON_CALL(m_objMockIAosNConfiguration, GetRegRetryCountOnSinglePcscf()).WillByDefault(Return(0));

    m_pAosPcscf->ResetCurrentPcscfTriedCount();

    EXPECT_EQ(m_pAosPcscf->GetCurrentPcscfTriedCount(), 1);
}

TEST_F(AosPcscfTest, ResetCurrentPcscfTriedCountIfConfiguredRetryCountIsNotZero)
{
    // set tried count of current pcscf as 1
    m_pAosPcscf->AddPcscf(AString("0.0.0.1"), 5060);
    m_pAosPcscf->SetCurrentPcscfIndex(0);
    m_pAosPcscf->IncreaseCurrentPcscfTriedCount();
    ON_CALL(m_objMockIAosNConfiguration, GetRegRetryCountOnSinglePcscf()).WillByDefault(Return(2));

    m_pAosPcscf->ResetCurrentPcscfTriedCount();

    EXPECT_EQ(m_pAosPcscf->GetCurrentPcscfTriedCount(), 0);
}

TEST_F(AosPcscfTest, DoNotResetAllPcscfTriedCountIfConfiguredRetryCountIsZero)
{
    // set tried count of current pcscf as 1
    m_pAosPcscf->AddPcscf(AString("0.0.0.1"), 5060);
    m_pAosPcscf->AddPcscf(AString("0.0.0.2"), 5061);
    m_pAosPcscf->SetCurrentPcscfIndex(0);
    m_pAosPcscf->IncreaseCurrentPcscfTriedCount();
    m_pAosPcscf->SetCurrentPcscfIndex(1);
    m_pAosPcscf->IncreaseCurrentPcscfTriedCount();
    ON_CALL(m_objMockIAosNConfiguration, GetRegRetryCountOnSinglePcscf()).WillByDefault(Return(0));

    m_pAosPcscf->ResetAllPcscfTriedCount();

    m_pAosPcscf->SetCurrentPcscfIndex(0);
    EXPECT_EQ(m_pAosPcscf->GetCurrentPcscfTriedCount(), 1);
    m_pAosPcscf->SetCurrentPcscfIndex(1);
    EXPECT_EQ(m_pAosPcscf->GetCurrentPcscfTriedCount(), 1);
}

TEST_F(AosPcscfTest, ResetAllPcscfTriedCountIfConfiguredRetryCountIsNotZero)
{
    // set tried count of current pcscf as 1
    m_pAosPcscf->AddPcscf(AString("0.0.0.1"), 5060);
    m_pAosPcscf->AddPcscf(AString("0.0.0.2"), 5061);
    m_pAosPcscf->SetCurrentPcscfIndex(0);
    m_pAosPcscf->IncreaseCurrentPcscfTriedCount();
    m_pAosPcscf->SetCurrentPcscfIndex(1);
    m_pAosPcscf->IncreaseCurrentPcscfTriedCount();
    ON_CALL(m_objMockIAosNConfiguration, GetRegRetryCountOnSinglePcscf()).WillByDefault(Return(2));

    m_pAosPcscf->ResetAllPcscfTriedCount();

    m_pAosPcscf->SetCurrentPcscfIndex(0);
    EXPECT_EQ(m_pAosPcscf->GetCurrentPcscfTriedCount(), 0);
    m_pAosPcscf->SetCurrentPcscfIndex(1);
    EXPECT_EQ(m_pAosPcscf->GetCurrentPcscfTriedCount(), 0);
}

TEST_F(AosPcscfTest, GetCurrentPcscfReturnsFalseIfCurrentPcscfIndexIsInvalid)
{
    m_pAosPcscf->AddPcscf(AString("0.0.0.1"), 5060);
    m_pAosPcscf->SetCurrentPcscfIndex(1);

    AString objPcscfAddress;
    IMS_UINT32 nPcscfPort;
    IMS_BOOL bResult = m_pAosPcscf->GetCurrentPcscf(objPcscfAddress, nPcscfPort);

    EXPECT_FALSE(bResult);
}

TEST_F(AosPcscfTest, GetCurrentPcscfReturnsTrueIfCurrentPcscfIsAvailable)
{
    m_pAosPcscf->AddPcscf(AString("0.0.0.1"), 5060);
    m_pAosPcscf->SetCurrentPcscfIndex(0);

    AString objPcscfAddress;
    IMS_UINT32 nPcscfPort;
    IMS_BOOL bResult = m_pAosPcscf->GetCurrentPcscf(objPcscfAddress, nPcscfPort);

    EXPECT_TRUE(bResult);
    EXPECT_EQ(objPcscfAddress, AString("0.0.0.1"));
    EXPECT_EQ(nPcscfPort, 5060);
}

TEST_F(AosPcscfTest, GetCurrentIndexReturnsCurrentPcscfIndex)
{
    PreparePcscfPreset();
    AStringArray objPcscfs = m_pAosPcscf->GetPcscfs();
    for (IMS_UINT32 nIndex = 0; nIndex < objPcscfs.GetCount(); nIndex++)
    {
        m_pAosPcscf->SetCurrentPcscfIndex(nIndex);
        EXPECT_EQ(m_pAosPcscf->GetCurrentIndex(), nIndex);
    }
}

TEST_F(AosPcscfTest, IsFirstPcscfReturnsWhetherCurrentPcscfIndexIsZero)
{
    m_pAosPcscf->SetCurrentPcscfIndex(0);
    EXPECT_TRUE(m_pAosPcscf->IsFirstPcscf());

    m_pAosPcscf->SetCurrentPcscfIndex(1);
    EXPECT_FALSE(m_pAosPcscf->IsFirstPcscf());
}

TEST_F(AosPcscfTest, GetFirstPcscfReturnsFalseIfPcscfListIsEmpty)
{
    AString objPcscfAddress;
    IMS_UINT32 nPcscfPort;
    IMS_BOOL bResult = m_pAosPcscf->GetFirstPcscf(objPcscfAddress, nPcscfPort);

    EXPECT_FALSE(bResult);
}

TEST_F(AosPcscfTest, GetFirstPcscfReturnsTrueIfFirstPcscfIsAvailable)
{
    m_pAosPcscf->AddPcscf(AString("0.0.0.1"), 5060);

    AString objPcscfAddress;
    IMS_UINT32 nPcscfPort;
    IMS_BOOL bResult = m_pAosPcscf->GetFirstPcscf(objPcscfAddress, nPcscfPort);

    EXPECT_TRUE(bResult);
    EXPECT_EQ(objPcscfAddress, AString("0.0.0.1"));
    EXPECT_EQ(nPcscfPort, 5060);
}

TEST_F(AosPcscfTest, HasNextPcscfReturnsFalseIfPcscfListIsEmpty)
{
    IMS_BOOL bResult = m_pAosPcscf->HasNextPcscf();

    EXPECT_FALSE(bResult);
}

TEST_F(AosPcscfTest, HasNextPcscfReturnsFalseIfNoAvailablePcscfAfterCurrentPcscfIndex)
{
    m_pAosPcscf->AddPcscf(AString("0.0.0.1"), 5060);
    m_pAosPcscf->SetCurrentPcscfIndex(0);
    ON_CALL(m_objMockIAosNConfiguration, GetRegRetryDefaultPolicy())
            .WillByDefault(Return(CarrierConfig::Ims::DEFAULT_RETRY_POLICY_SPEC));

    IMS_BOOL bResult = m_pAosPcscf->HasNextPcscf();

    EXPECT_FALSE(bResult);
}

TEST_F(AosPcscfTest, HasNextPcscfReturnsTrueIfAvailablePcscfExistAfterCurrentPcscfIndex)
{
    m_pAosPcscf->AddPcscf(AString("0.0.0.1"), 5060);
    m_pAosPcscf->AddPcscf(AString("0.0.0.2"), 5061);
    m_pAosPcscf->SetCurrentPcscfIndex(0);
    ON_CALL(m_objMockIAosNConfiguration, GetRegRetryDefaultPolicy())
            .WillByDefault(Return(CarrierConfig::Ims::DEFAULT_RETRY_POLICY_SPEC));

    IMS_BOOL bResult = m_pAosPcscf->HasNextPcscf();

    EXPECT_TRUE(bResult);
}

TEST_F(AosPcscfTest, HasNextPcscfReturnsTrueIfAvailablePcscfExistInPcscfList)
{
    m_pAosPcscf->AddPcscf(AString("0.0.0.1"), 5060);
    m_pAosPcscf->AddPcscf(AString("0.0.0.2"), 5061);
    m_pAosPcscf->SetCurrentPcscfIndex(1);
    ON_CALL(m_objMockIAosNConfiguration, GetRegRetryDefaultPolicy())
            .WillByDefault(Return(CarrierConfig::Ims::DEFAULT_RETRY_POLICY_CIRCULAR_NEXT_PCSCF));

    IMS_BOOL bResult = m_pAosPcscf->HasNextPcscf();

    EXPECT_TRUE(bResult);
}

TEST_F(AosPcscfTest, GetNextPcscfReturnsFalseIfNotExist)
{
    m_pAosPcscf->AddPcscf(AString("0.0.0.1"), 5060);
    m_pAosPcscf->SetCurrentPcscfIndex(0);
    ON_CALL(m_objMockIAosNConfiguration, GetRegRetryDefaultPolicy())
            .WillByDefault(Return(CarrierConfig::Ims::DEFAULT_RETRY_POLICY_SPEC));

    AString objPcscfAddress;
    IMS_UINT32 nPcscfPort;
    IMS_BOOL bResult = m_pAosPcscf->GetNextPcscf(objPcscfAddress, nPcscfPort);

    EXPECT_FALSE(bResult);
}

TEST_F(AosPcscfTest, GetNextPcscfReturnsTrueIfExist)
{
    m_pAosPcscf->AddPcscf(AString("0.0.0.1"), 5060);
    m_pAosPcscf->AddPcscf(AString("0.0.0.2"), 5061);
    m_pAosPcscf->SetCurrentPcscfIndex(1);
    ON_CALL(m_objMockIAosNConfiguration, GetRegRetryDefaultPolicy())
            .WillByDefault(Return(CarrierConfig::Ims::DEFAULT_RETRY_POLICY_CIRCULAR_NEXT_PCSCF));

    AString objPcscfAddress;
    IMS_UINT32 nPcscfPort;
    IMS_BOOL bResult = m_pAosPcscf->GetNextPcscf(objPcscfAddress, nPcscfPort);

    EXPECT_TRUE(bResult);
}

TEST_F(AosPcscfTest, UpdateCurrentPcscIndexAsZeroWhenSetFirstPcscfIndex)
{
    m_pAosPcscf->SetCurrentPcscfIndex(3);

    m_pAosPcscf->SetFirstPcscfIndex();

    EXPECT_EQ(m_pAosPcscf->GetCurrentIndex(), 0);
}

TEST_F(AosPcscfTest, CheckAndProcessChangeFromPcoReturnsFalseIfFailToGetCurrentPcscf)
{
    PreparePcscfPreset();
    m_pAosPcscf->SetCurrentPcscfIndex(m_pAosPcscf->GetPcscfCount());

    IMS_BOOL bResult = m_pAosPcscf->CheckAndProcessChangeFromPco();

    EXPECT_FALSE(bResult);
}

TEST_F(AosPcscfTest, CheckAndProcessChangeFromPcoReturnsFalseIfCurrentPcscfAddressIsInvalid)
{
    m_pAosPcscf->AddPcscf(AString("InvalidAddress"), 5060);
    m_pAosPcscf->SetCurrentPcscfIndex(0);

    IMS_BOOL bResult = m_pAosPcscf->CheckAndProcessChangeFromPco();

    EXPECT_FALSE(bResult);
}

TEST_F(AosPcscfTest, CheckAndProcessChangeFromPcoReturnsFalseIfPcscfListIsNotChanged)
{
    PreparePcscfPreset();
    ON_CALL(m_objMockIAosConnection, GetPcscfAddress(_))
            .WillByDefault(ReturnRef(m_objPcscfAddressPreset));

    IMS_BOOL bResult = m_pAosPcscf->CheckAndProcessChangeFromPco();

    EXPECT_FALSE(bResult);
}

TEST_F(AosPcscfTest, CheckAndProcessChangeFromPcoReturnsFalseIfNewPcscfListIsEmpty)
{
    PreparePcscfPreset();
    AStringArray objEmptyArray;
    ON_CALL(m_objMockIAosConnection, GetPcscfAddress(_)).WillByDefault(ReturnRef(objEmptyArray));

    IMS_BOOL bResult = m_pAosPcscf->CheckAndProcessChangeFromPco();

    EXPECT_FALSE(bResult);
}

TEST_F(AosPcscfTest, ReorderPcscfsIfCurrentPcscfExistWhenCheckAndProcessChangeFromPco)
{
    PreparePcscfPreset();
    m_pAosPcscf->SetCurrentPcscfIndex(1);
    AString strNewPcscfs = "0.0.0.4, 0.0.0.2, 0.0.0.5";
    AStringArray objNewPcscfs = strNewPcscfs.Split(',');
    ON_CALL(m_objMockIAosConnection, GetPcscfAddress(_)).WillByDefault(ReturnRef(objNewPcscfs));

    m_pAosPcscf->CheckAndProcessChangeFromPco();

    EXPECT_EQ(m_pAosPcscf->GetChangedType(), IAosPcscf::TYPE_CHANGED_REORDER);
    EXPECT_EQ(m_pAosPcscf->GetCurrentPcscfIndex(), 0);
}

TEST_F(AosPcscfTest, UpdatePcscfsIfCurrentPcscfNotExistWhenCheckAndProcessChangeFromPco)
{
    PreparePcscfPreset();
    AString strNewPcscfs = "0.0.0.4, 0.0.0.5";
    AStringArray objNewPcscfs = strNewPcscfs.Split(',');
    ON_CALL(m_objMockIAosConnection, GetPcscfAddress(_)).WillByDefault(ReturnRef(objNewPcscfs));

    m_pAosPcscf->CheckAndProcessChangeFromPco();

    EXPECT_EQ(m_pAosPcscf->GetChangedType(), IAosPcscf::TYPE_CHANGED_DIFFERENT);
    EXPECT_EQ(m_pAosPcscf->GetCurrentPcscfIndex(), 0);
}

TEST_F(AosPcscfTest, GetChangedPcscfsReturnsFalseIfNewPcscfListIsEmpty)
{
    AStringArray objEmptyArray;
    ON_CALL(m_objMockIAosConnection, GetPcscfAddress(_)).WillByDefault(ReturnRef(objEmptyArray));

    AStringArray objChangedPcscfs;
    IMS_BOOL bResult = m_pAosPcscf->GetChangedPcscfs(objChangedPcscfs, IpAddress::IPV4);

    EXPECT_FALSE(bResult);
}

TEST_F(AosPcscfTest, GetChangedPcscfsReturnsTrueIfAvailablePcscfExist)
{
    AString strNewPcscfs = "0.0.0.4, 0.0.0.0, 0.0.0.4, ::1, invalidAddress";
    AStringArray objNewPcscfs = strNewPcscfs.Split(',');
    ON_CALL(m_objMockIAosConnection, GetPcscfAddress(_)).WillByDefault(ReturnRef(objNewPcscfs));

    AStringArray objChangedPcscfs;
    IMS_BOOL bResult = m_pAosPcscf->GetChangedPcscfs(objChangedPcscfs, IpAddress::IPV4);

    EXPECT_TRUE(bResult);
    EXPECT_EQ(objChangedPcscfs.GetCount(), 1);
}

TEST_F(AosPcscfTest, ConfigurePcscfWhenProcessDiscoveryUsingPcoMethod)
{
    ImsVector<IMS_SINT32> objDiscoveryMethods;
    objDiscoveryMethods.Add(ISubscriberConfig::PCSCF_DISCOVERY_METHOD_PCO);
    ON_CALL(m_objMockISubscriberConfig, GetPcscfDiscoveryMethods())
            .WillByDefault(ReturnRef(objDiscoveryMethods));
    AString strPcscfs = "0.0.0.4, 0.0.0.0, 0.0.0.4, ::1, invalidAddress";
    AStringArray objPcscfPreset = strPcscfs.Split(',');
    ON_CALL(m_objMockIAosConnection, GetPcscfAddress(IpAddress::IPV4))
            .WillByDefault(ReturnRef(objPcscfPreset));

    m_pAosPcscf->ProcessDiscovery(IpAddress::IPV4);

    EXPECT_TRUE(m_pAosPcscf->IsConfigured());
    EXPECT_EQ(m_pAosPcscf->GetPcscfCount(), 1);
}

TEST_F(AosPcscfTest, ConfigurePcscfWhenProcessDiscoveryUsingConfigMethod)
{
    PrepareConfiguredPcscfs(AString(", 0.0.0.4, 0.0.0.4, ::1, hostName"));
    ImsVector<IMS_SINT32> objDiscoveryMethods;
    objDiscoveryMethods.Add(ISubscriberConfig::PCSCF_DISCOVERY_METHOD_CONFIG);
    ON_CALL(m_objMockISubscriberConfig, GetPcscfDiscoveryMethods())
            .WillByDefault(ReturnRef(objDiscoveryMethods));

    EXPECT_CALL(m_objMockISubscriberConfig, GetPcscfAddresses())
            .WillOnce(ReturnRef(m_objConfiguredPcscfs));

    m_pAosPcscf->ProcessDiscovery(IpAddress::IPV4);

    EXPECT_TRUE(m_pAosPcscf->IsConfigured());
    EXPECT_EQ(m_pAosPcscf->GetPcscfCount(), 1);
    ClearConfiguredPcscfs();
}

TEST_F(AosPcscfTest, TriggerDnsQueryRetryIfFailToQueryDnsWhenProcessDiscoveryUsingConfigMethod)
{
    PrepareConfiguredPcscfs(AString("hostName"));
    ImsVector<IMS_SINT32> objDiscoveryMethods;
    objDiscoveryMethods.Add(ISubscriberConfig::PCSCF_DISCOVERY_METHOD_CONFIG);
    ON_CALL(m_objMockISubscriberConfig, GetPcscfDiscoveryMethods())
            .WillByDefault(ReturnRef(objDiscoveryMethods));

    EXPECT_CALL(m_objMockISubscriberConfig, GetPcscfAddresses())
            .WillOnce(ReturnRef(m_objConfiguredPcscfs));
    EXPECT_CALL(m_objMockIAosConnection, GetHostByName(_, _, _)).WillOnce(Return(-1));

    m_pAosPcscf->ProcessDiscovery(IpAddress::IPV4);

    EXPECT_FALSE(m_pAosPcscf->IsConfigured());
    EXPECT_NE(m_pAosPcscf->GetDnsQueryRetryTimer(), nullptr);
    ClearConfiguredPcscfs();
}

TEST_F(AosPcscfTest, GetNextDiscoveryMethodReturnsFalseIfSubscriberConfigIsNull)
{
    m_pAosPcscf->SetRegType(AosRegistrationType::FAKE);
    ON_CALL(m_objMockIAosSubscriber, GetSubscriberConfig(IAosSubscriber::FAKE))
            .WillByDefault(Return(nullptr));

    IMS_SINT32 nDiscoveryMethod;
    IMS_BOOL bResult = m_pAosPcscf->GetNextDiscoveryMethod(nDiscoveryMethod);

    EXPECT_FALSE(bResult);
}

TEST_F(AosPcscfTest, GetNextDiscoveryMethodReturnsFalseIfNoDiscoveryMethodExist)
{
    ImsVector<IMS_SINT32> objEmptyDiscoveryMethods;
    ON_CALL(m_objMockISubscriberConfig, GetPcscfDiscoveryMethods())
            .WillByDefault(ReturnRef(objEmptyDiscoveryMethods));

    IMS_SINT32 nDiscoveryMethod;
    IMS_BOOL bResult = m_pAosPcscf->GetNextDiscoveryMethod(nDiscoveryMethod);

    EXPECT_FALSE(bResult);
}

TEST_F(AosPcscfTest, GetNextDiscoveryMethodReturnsTrueIfDiscoveryMethodExist)
{
    ImsVector<IMS_SINT32> objDiscoveryMethods;
    objDiscoveryMethods.Add(ISubscriberConfig::PCSCF_DISCOVERY_METHOD_CONFIG);
    ON_CALL(m_objMockISubscriberConfig, GetPcscfDiscoveryMethods())
            .WillByDefault(ReturnRef(objDiscoveryMethods));

    IMS_SINT32 nDiscoveryMethod;
    IMS_BOOL bResult = m_pAosPcscf->GetNextDiscoveryMethod(nDiscoveryMethod);

    EXPECT_TRUE(bResult);
}

TEST_F(AosPcscfTest, GetNextDiscoveryMethodReturnsFalseIfNoMoreDiscoveryMethodExist)
{
    ImsVector<IMS_SINT32> objDiscoveryMethods;
    objDiscoveryMethods.Add(ISubscriberConfig::PCSCF_DISCOVERY_METHOD_CONFIG);
    ON_CALL(m_objMockISubscriberConfig, GetPcscfDiscoveryMethods())
            .WillByDefault(ReturnRef(objDiscoveryMethods));
    m_pAosPcscf->SetDiscoveryMethodIndex(1);

    IMS_SINT32 nDiscoveryMethod;
    IMS_BOOL bResult = m_pAosPcscf->GetNextDiscoveryMethod(nDiscoveryMethod);

    EXPECT_FALSE(bResult);
}

TEST_F(AosPcscfTest, GetFromConfReturnsFalseIfSubscriberConfigIsNull)
{
    ON_CALL(m_objMockIAosSubscriber, GetSubscriberConfig(_)).WillByDefault(Return(nullptr));

    IMS_BOOL bResult = m_pAosPcscf->GetFromConf(IpAddress::IPV4);

    EXPECT_FALSE(bResult);
}

TEST_F(AosPcscfTest, GetFromConfReturnsFalseIfPreconfiguredPcscfListIsEmpty)
{
    ON_CALL(m_objMockIAosSubscriber, GetSubscriberConfig(_))
            .WillByDefault(Return(&m_objMockISubscriberConfig));
    ImsVector<ServerAddress*> objEmptyPcscfList;
    ON_CALL(m_objMockISubscriberConfig, GetPcscfAddresses())
            .WillByDefault(ReturnRef(objEmptyPcscfList));

    IMS_BOOL bResult = m_pAosPcscf->GetFromConf(IpAddress::IPV4);

    EXPECT_FALSE(bResult);
}

TEST_F(AosPcscfTest, ProcessDnsQueryReturnsTrueIfSucceed)
{
    ImsList<IpAddress> objIpas;
    objIpas.Append(IpAddress(AString("0.0.0.4")));
    objIpas.Append(IpAddress(AString("fe80::1")));
    objIpas.Append(IpAddress::ANY);
    objIpas.Append(IpAddress(AString("0.0.0.4")));
    EXPECT_CALL(m_objMockIAosConnection, GetHostByNameInternal(_, _, _))
            .WillOnce(DoAll(SetArgPointee<1>(objIpas), Return(1)));

    IMS_BOOL bResult = m_pAosPcscf->ProcessDnsQuery(AString("aaa.bbb.ccc"), 5060, IpAddress::IPV4);

    EXPECT_TRUE(bResult);
    EXPECT_EQ(m_pAosPcscf->GetPcscfCount(), 1);
    EXPECT_EQ(m_pAosPcscf->GetPcscfs().GetElementAt(0), AString("0.0.0.4"));
}

TEST_F(AosPcscfTest, ProcessDnsQueryReturnsFalseIfFail)
{
    ON_CALL(m_objMockIAosConnection, GetHostByName(_, _, _)).WillByDefault(Return(-1));

    IMS_BOOL bResult = m_pAosPcscf->ProcessDnsQuery(AString("aaa.bbb.ccc"), 5060, IpAddress::IPV4);

    EXPECT_FALSE(bResult);
}

TEST_F(AosPcscfTest, DoNotStartIfDurationIsZeroWhenStartTimer)
{
    EXPECT_CALL(m_objMockITimer, SetTimer(_, _)).Times(0);

    m_pAosPcscf->StartTimer(TestAosPcscf::TIMER_DNS_QUERY_RETRY, 0);
}

TEST_F(AosPcscfTest, DoNotStartIfTypeIsInvalidWhenStartTimer)
{
    EXPECT_CALL(m_objMockITimer, SetTimer(_, _)).Times(0);

    m_pAosPcscf->StartTimer(TestAosPcscf::TIMER_DNS_QUERY_RETRY + 100, 10);
}

TEST_F(AosPcscfTest, RestartIfAlreadyRunningWhenStartTimer)
{
    EXPECT_CALL(m_objMockITimer, SetTimer(_, _)).Times(2);
    EXPECT_CALL(m_objMockITimer, KillTimer()).Times(2);

    m_pAosPcscf->StartTimer(TestAosPcscf::TIMER_DNS_QUERY_RETRY, 10);
    m_pAosPcscf->StartTimer(TestAosPcscf::TIMER_DNS_QUERY_RETRY, 10);

    m_pAosPcscf->StopTimer(TestAosPcscf::TIMER_DNS_QUERY_RETRY);
}

TEST_F(AosPcscfTest, DoNotStopIfTypeIsInvalidWhenStopTimer)
{
    EXPECT_CALL(m_objMockITimer, KillTimer()).Times(0);

    m_pAosPcscf->StopTimer(TestAosPcscf::TIMER_DNS_QUERY_RETRY + 100);
}

TEST_F(AosPcscfTest, DoNotStopIfNotRunningWhenStopTimer)
{
    EXPECT_CALL(m_objMockITimer, KillTimer()).Times(0);

    m_pAosPcscf->StopTimer(TestAosPcscf::TIMER_DNS_QUERY_RETRY);
}

TEST_F(AosPcscfTest, SucceedToStopWhenStopTimer)
{
    m_pAosPcscf->StartTimer(TestAosPcscf::TIMER_DNS_QUERY_RETRY, 10);

    EXPECT_CALL(m_objMockITimer, KillTimer()).Times(1);

    m_pAosPcscf->StopTimer(TestAosPcscf::TIMER_DNS_QUERY_RETRY);
}

TEST_F(AosPcscfTest, DoNothingIfTimerIsNullWhenTimerExpired)
{
    EXPECT_CALL(m_objMockITimer, KillTimer()).Times(0);

    m_pAosPcscf->Timer_TimerExpired(IMS_NULL);
}

TEST_F(AosPcscfTest, NotifyResultIfSucceedToRetryDnsQueryWhenDnsRetryTimerExpired)
{
    m_pAosPcscf->SetListener(&objMockIAosPcscfListener);
    m_pAosPcscf->AddRetryHost();
    m_pAosPcscf->StartTimer(TestAosPcscf::TIMER_DNS_QUERY_RETRY, 10);

    ImsList<IpAddress> objIpas;
    objIpas.Append(IpAddress(AString("0.0.0.4")));
    EXPECT_CALL(m_objMockIAosConnection, GetHostByNameInternal(_, _, _))
            .WillOnce(DoAll(SetArgPointee<1>(objIpas), Return(1)));
    EXPECT_CALL(objMockIAosPcscfListener, Pcscf_NotifyResult(IMS_TRUE));

    m_pAosPcscf->Timer_TimerExpired(m_pAosPcscf->GetDnsQueryRetryTimer());
}

TEST_F(AosPcscfTest, TriggerDiscoveryUsingEachIpTypeIfFailToDnsQueryAgainWhenDnsRetryTimerExpired)
{
    m_pAosPcscf->SetOtherIpTypeRequired(true);
    m_pAosPcscf->AddRetryHost();
    m_pAosPcscf->StartTimer(TestAosPcscf::TIMER_DNS_QUERY_RETRY, 10);
    ON_CALL(m_objMockIAosConnection, GetHostByName(_, _, _)).WillByDefault(Return(-1));
    ON_CALL(m_objMockIAosConnection, GetLocalAddress(_))
            .WillByDefault(ReturnRef(IpAddress::IPv6LOOPBACK));

    EXPECT_CALL(m_objMockISubscriberConfig, GetPcscfDiscoveryMethods())
            .Times(2)
            .WillRepeatedly(ReturnRef(m_objEmptyDiscoveryMethods));

    m_pAosPcscf->Timer_TimerExpired(m_pAosPcscf->GetDnsQueryRetryTimer());
}

TEST_F(AosPcscfTest, TimerToStringForInvalidType)
{
    EXPECT_STREQ(
            m_pAosPcscf->TimerToString(TestAosPcscf::TIMER_DNS_QUERY_RETRY + 100), "__INVALID__");
}