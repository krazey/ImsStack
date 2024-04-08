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
    virtual void SetUp() override
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

    virtual void TearDown() override
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
    EXPECT_CALL(m_objMockIAosConnection, GetLocalAddress(_)).WillOnce(ReturnRef(IpAddress::ANY));
    EXPECT_CALL(m_objMockISubscriberConfig, GetPcscfDiscoveryMethods()).Times(0);

    m_pAosPcscf->Configure(IpAddress::IPV4);

    EXPECT_FALSE(m_pAosPcscf->GetOtherIpTypeRequired());
}

TEST_F(AosPcscfTest, ProcessDiscoveryForValidLocalAddressWhenConfigureWithIpVersion)
{
    EXPECT_CALL(m_objMockIAosConnection, GetLocalAddress(_))
            .WillOnce(ReturnRef(IpAddress::LOOPBACK));
    EXPECT_CALL(m_objMockISubscriberConfig, GetPcscfDiscoveryMethods()).Times(1);

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
    PreparePcscfPreset();

    AStringArray objGetPcscfs = m_pAosPcscf->GetPcscfs();

    for (int i = 0; i < objGetPcscfs.GetCount(); i++)
    {
        EXPECT_EQ(objGetPcscfs.GetElementAt(i), m_objPcscfAddressPreset.GetElementAt(i));
    }
}

TEST_F(AosPcscfTest, GetPcscfsPortsReturnsPcscfPortSet)
{
    PreparePcscfPreset();

    ImsList<IMS_SINT32> objGetPcscfsPorts = m_pAosPcscf->GetPcscfsPorts();

    for (int i = 0; i < objGetPcscfsPorts.GetSize(); i++)
    {
        EXPECT_EQ(objGetPcscfsPorts.GetAt(i), m_objPcscfPortPreset.GetAt(i));
    }
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
    PreparePcscfPreset();

    m_pAosPcscf->RemoveCurrentPcscf();

    m_objPcscfAddressPreset.RemoveElementAt(m_pAosPcscf->GetCurrentIndex());
    AStringArray objGetPcscfs = m_pAosPcscf->GetPcscfs();
    for (int i = 0; i < objGetPcscfs.GetCount(); i++)
    {
        EXPECT_EQ(objGetPcscfs.GetElementAt(i), m_objPcscfAddressPreset.GetElementAt(i));
    }
}

TEST_F(AosPcscfTest, UpdateAvailabilityOfPcscf)
{
    PreparePcscfPreset();
    IMS_UINT32 nUnavailableTimeSec = 5;
    Pcscf* pPcscf = m_pAosPcscf->GetPcscfList().GetAt(m_pAosPcscf->GetCurrentIndex());

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

TEST_F(AosPcscfTest, IncreaseCurrentPcscfTriedCount)
{
    EXPECT_CALL(m_objMockIAosNConfiguration, GetRegRetryCountPerPcscf())
            .Times(AnyNumber())
            .WillRepeatedly(Return(0));
    EXPECT_CALL(m_objMockIAosNConfiguration, GetRegRetryCountOnSinglePcscf())
            .Times(3)
            .WillOnce(Return(0))
            .WillRepeatedly(Return(2));
    PreparePcscfPreset();

    // do NOT increase if configured retry count is zero
    m_pAosPcscf->SetCurrentPcscfIndex(0);
    m_pAosPcscf->IncreaseCurrentPcscfTriedCount();
    EXPECT_EQ(m_pAosPcscf->GetCurrentPcscfTriedCount(), 0);

    // increase tried count of current pcscf
    m_pAosPcscf->IncreaseCurrentPcscfTriedCount();
    EXPECT_EQ(m_pAosPcscf->GetCurrentPcscfTriedCount(), 1);

    // do NOT handle invalid Pcscf index
    m_pAosPcscf->SetCurrentPcscfIndex(m_pAosPcscf->GetPcscfCount());
    m_pAosPcscf->IncreaseCurrentPcscfTriedCount();
    EXPECT_EQ(m_pAosPcscf->GetCurrentPcscfTriedCount(), 0);
}

TEST_F(AosPcscfTest, ResetCurrentPcscfTriedCount)
{
    EXPECT_CALL(m_objMockIAosNConfiguration, GetRegRetryCountPerPcscf())
            .Times(AnyNumber())
            .WillRepeatedly(Return(0));
    EXPECT_CALL(m_objMockIAosNConfiguration, GetRegRetryCountOnSinglePcscf())
            .WillOnce(Return(2))
            .WillOnce(Return(0))
            .WillOnce(Return(2));
    PreparePcscfPreset();

    // increase tried count of current pcscf
    m_pAosPcscf->SetCurrentPcscfIndex(0);
    m_pAosPcscf->IncreaseCurrentPcscfTriedCount();

    // do NOT reset if configured retry count is zero
    m_pAosPcscf->ResetCurrentPcscfTriedCount();
    EXPECT_EQ(m_pAosPcscf->GetCurrentPcscfTriedCount(), 1);

    // reset tried count of current pcscf
    m_pAosPcscf->ResetCurrentPcscfTriedCount();
    EXPECT_EQ(m_pAosPcscf->GetCurrentPcscfTriedCount(), 0);
}

TEST_F(AosPcscfTest, ResetAllPcscfTriedCount)
{
    EXPECT_CALL(m_objMockIAosNConfiguration, GetRegRetryCountPerPcscf())
            .Times(AnyNumber())
            .WillRepeatedly(Return(0));
    EXPECT_CALL(m_objMockIAosNConfiguration, GetRegRetryCountOnSinglePcscf())
            .Times(AnyNumber())
            .WillRepeatedly(Return(2));
    PreparePcscfPreset();
    AStringArray objPcscfs = m_pAosPcscf->GetPcscfs();

    for (IMS_UINT32 nIndex = 0; nIndex < objPcscfs.GetCount(); nIndex++)
    {
        m_pAosPcscf->SetCurrentPcscfIndex(nIndex);
        EXPECT_EQ(m_pAosPcscf->GetCurrentPcscfTriedCount(), 0);
        m_pAosPcscf->IncreaseCurrentPcscfTriedCount();
        EXPECT_EQ(m_pAosPcscf->GetCurrentPcscfTriedCount(), 1);
    }

    // do NOT reset if configured retry count is zero
    EXPECT_CALL(m_objMockIAosNConfiguration, GetRegRetryCountOnSinglePcscf())
            .Times(AnyNumber())
            .WillRepeatedly(Return(0));

    m_pAosPcscf->ResetAllPcscfTriedCount();

    for (IMS_UINT32 nIndex = 0; nIndex < objPcscfs.GetCount(); nIndex++)
    {
        m_pAosPcscf->SetCurrentPcscfIndex(nIndex);
        EXPECT_EQ(m_pAosPcscf->GetCurrentPcscfTriedCount(), 1);
    }

    // reset tried count of all pcscf
    EXPECT_CALL(m_objMockIAosNConfiguration, GetRegRetryCountOnSinglePcscf())
            .Times(AnyNumber())
            .WillRepeatedly(Return(2));

    m_pAosPcscf->ResetAllPcscfTriedCount();

    for (IMS_UINT32 nIndex = 0; nIndex < objPcscfs.GetCount(); nIndex++)
    {
        m_pAosPcscf->SetCurrentPcscfIndex(nIndex);
        EXPECT_EQ(m_pAosPcscf->GetCurrentPcscfTriedCount(), 0);
    }
}

TEST_F(AosPcscfTest, GetCurrentPcscf)
{
    AString objPcscfAddress;
    IMS_UINT32 nPcscfPort;

    // update current port as 5060 if it is 0
    m_pAosPcscf->AddPcscf(AString("0.0.0.1"), 0);
    m_pAosPcscf->SetCurrentPcscfIndex(0);
    EXPECT_TRUE(m_pAosPcscf->GetCurrentPcscf(objPcscfAddress, nPcscfPort));
    EXPECT_EQ(objPcscfAddress, AString("0.0.0.1"));
    EXPECT_EQ(nPcscfPort, 5060);

    // get current Pcscf successfully
    m_pAosPcscf->AddPcscf(AString("0.0.0.2"), 5061);
    m_pAosPcscf->SetCurrentPcscfIndex(1);
    EXPECT_TRUE(m_pAosPcscf->GetCurrentPcscf(objPcscfAddress, nPcscfPort));
    EXPECT_EQ(objPcscfAddress, AString("0.0.0.2"));
    EXPECT_EQ(nPcscfPort, 5061);

    // return false for Pcscf of index is null
    m_pAosPcscf->GetPcscfList().Append(nullptr);
    m_pAosPcscf->SetCurrentPcscfIndex(2);
    EXPECT_FALSE(m_pAosPcscf->GetCurrentPcscf(objPcscfAddress, nPcscfPort));

    // return false for Pcscf index larger than the Pcscf list size
    m_pAosPcscf->SetCurrentPcscfIndex(3);
    EXPECT_FALSE(m_pAosPcscf->GetCurrentPcscf(objPcscfAddress, nPcscfPort));
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

TEST_F(AosPcscfTest, GetFirstPcscf)
{
    AString objPcscfAddress;
    IMS_UINT32 nPcscfPort;

    // return false if Pcscf list is empty
    EXPECT_FALSE(m_pAosPcscf->GetFirstPcscf(objPcscfAddress, nPcscfPort));

    // return false if first Pcscf is null
    m_pAosPcscf->GetPcscfList().Append(nullptr);
    EXPECT_FALSE(m_pAosPcscf->GetFirstPcscf(objPcscfAddress, nPcscfPort));
    m_pAosPcscf->GetPcscfList().Clear();

    // get first Pcscf successfully
    m_pAosPcscf->AddPcscf(AString("0.0.0.1"), 5060);
    EXPECT_TRUE(m_pAosPcscf->GetFirstPcscf(objPcscfAddress, nPcscfPort));
    EXPECT_EQ(objPcscfAddress, AString("0.0.0.1"));
    EXPECT_EQ(nPcscfPort, 5060);
    EXPECT_EQ(m_pAosPcscf->GetCurrentPcscfIndex(), 0);
}

TEST_F(AosPcscfTest, HasNextPcscf)
{
    EXPECT_CALL(m_objMockIAosNConfiguration, GetRegRetryDefaultPolicy())
            .Times(2)
            .WillOnce(Return(CarrierConfig::Assets::DEFAULT_RETRY_POLICY_SPEC))
            .WillOnce(Return(CarrierConfig::Assets::DEFAULT_RETRY_POLICY_CIRCULAR_NEXT_PCSCF));

    // return false if Pcscf list is empty
    EXPECT_FALSE(m_pAosPcscf->HasNextPcscf());

    // return true If there is an available Pcscf after the current Pcscf index
    PreparePcscfPreset();
    m_pAosPcscf->SetCurrentPcscfIndex(0);
    EXPECT_TRUE(m_pAosPcscf->HasNextPcscf());

    // return false If there is no available Pcscf after the current Pcscf index
    m_pAosPcscf->SetCurrentPcscfIndex(m_pAosPcscf->GetPcscfCount());
    EXPECT_FALSE(m_pAosPcscf->HasNextPcscf());

    // if DEFAULT_RETRY_POLICY_CIRCULAR_NEXT_PCSCF is configured, check whole Pcscf list
    m_pAosPcscf->SetCurrentPcscfIndex(m_pAosPcscf->GetPcscfCount());
    EXPECT_TRUE(m_pAosPcscf->HasNextPcscf());
}

TEST_F(AosPcscfTest, GetNextPcscf)
{
    AString objPcscfAddress;
    IMS_UINT32 nPcscfPort;
    m_pAosPcscf->SetCurrentPcscfIndex(0);
    m_pAosPcscf->AddPcscf(AString("0.0.0.1"), 5060);
    m_pAosPcscf->AddPcscf(AString("0.0.0.2"), 5061);

    // return true if there is an available Pcscf after the current Pcscf index
    EXPECT_TRUE(m_pAosPcscf->GetNextPcscf(objPcscfAddress, nPcscfPort));
    EXPECT_EQ(objPcscfAddress, AString("0.0.0.2"));
    EXPECT_EQ(nPcscfPort, 5061);

    // return false if there is no available Pcscf after the current Pcscf index
    m_pAosPcscf->SetCurrentPcscfIndex(m_pAosPcscf->GetPcscfCount());
    EXPECT_FALSE(m_pAosPcscf->GetNextPcscf(objPcscfAddress, nPcscfPort));
}

TEST_F(AosPcscfTest, SetFirstPcscfIndex_UdateCurrentPcscIndexAsZero)
{
    m_pAosPcscf->SetCurrentPcscfIndex(3);
    m_pAosPcscf->SetFirstPcscfIndex();
    EXPECT_EQ(m_pAosPcscf->GetCurrentIndex(), 0);
}

TEST_F(AosPcscfTest, CheckAndProcessChangeFromPco_NotHandledCases)
{
    PreparePcscfPreset();

    // return false if fail to get current Pcscf
    m_pAosPcscf->SetCurrentPcscfIndex(m_pAosPcscf->GetPcscfCount());
    EXPECT_FALSE(m_pAosPcscf->CheckAndProcessChangeFromPco());

    // return false if current Pcscf address is invalid
    m_pAosPcscf->AddPcscf(AString("InvalidAddress"), 5060);
    EXPECT_FALSE(m_pAosPcscf->CheckAndProcessChangeFromPco());
    m_pAosPcscf->RemoveCurrentPcscf();

    // return false if Pcscf list is not changed
    m_pAosPcscf->SetCurrentPcscfIndex(0);
    EXPECT_CALL(m_objMockIAosConnection, GetPcscfAddress(_))
            .WillOnce(ReturnRef(m_objPcscfAddressPreset));
    EXPECT_FALSE(m_pAosPcscf->CheckAndProcessChangeFromPco());

    // return false if new Pcscf list is empty
    AStringArray objEmptyArray;
    EXPECT_CALL(m_objMockIAosConnection, GetPcscfAddress(_)).WillOnce(ReturnRef(objEmptyArray));
    EXPECT_FALSE(m_pAosPcscf->CheckAndProcessChangeFromPco());
}

TEST_F(AosPcscfTest, CheckAndProcessChangeFromPco_ReorderIfCurrentPcscfExist)
{
    PreparePcscfPreset();
    AString strNewPcscfs = "0.0.0.4, 0.0.0.2, 0.0.0.5";
    AStringArray objNewPcscfs = strNewPcscfs.Split(',');
    EXPECT_CALL(m_objMockIAosConnection, GetPcscfAddress(_))
            .Times(AnyNumber())
            .WillRepeatedly(ReturnRef(objNewPcscfs));

    m_pAosPcscf->SetCurrentPcscfIndex(1);
    EXPECT_TRUE(m_pAosPcscf->CheckAndProcessChangeFromPco());
    EXPECT_EQ(m_pAosPcscf->GetChangedType(), IAosPcscf::TYPE_CHANGED_REORDER);
    EXPECT_EQ(m_pAosPcscf->GetCurrentPcscfIndex(), 0);
}

TEST_F(AosPcscfTest, CheckAndProcessChangeFromPco_DifferentIfCurrentPcscfNotExist)
{
    PreparePcscfPreset();
    AString strNewPcscfs = "0.0.0.4, 0.0.0.5";
    AStringArray objNewPcscfs = strNewPcscfs.Split(',');
    EXPECT_CALL(m_objMockIAosConnection, GetPcscfAddress(_))
            .Times(AnyNumber())
            .WillRepeatedly(ReturnRef(objNewPcscfs));

    EXPECT_TRUE(m_pAosPcscf->CheckAndProcessChangeFromPco());
    EXPECT_EQ(m_pAosPcscf->GetChangedType(), IAosPcscf::TYPE_CHANGED_DIFFERENT);
    EXPECT_EQ(m_pAosPcscf->GetCurrentPcscfIndex(), 0);
}

TEST_F(AosPcscfTest, SetListener)
{
    EXPECT_CALL(objMockIAosPcscfListener, Pcscf_NotifyResult(IMS_TRUE)).Times(1);
    m_pAosPcscf->SetListener(&objMockIAosPcscfListener);
    m_pAosPcscf->GetListener()->Pcscf_NotifyResult(IMS_TRUE);
}

TEST_F(AosPcscfTest, GetChangedPcscfs)
{
    AString strNewPcscfs = "0.0.0.4, 0.0.0.0, 0.0.0.4, ::1, invalidAddress";
    AStringArray objPcscfPreset = strNewPcscfs.Split(',');
    AStringArray objChangedPcscfs;
    EXPECT_CALL(m_objMockIAosConnection, GetPcscfAddress(_))
            .Times(AnyNumber())
            .WillRepeatedly(ReturnRef(objPcscfPreset));

    // return true if there is available new Pcscf
    EXPECT_TRUE(m_pAosPcscf->GetChangedPcscfs(objChangedPcscfs, IpAddress::IPV4));
    EXPECT_EQ(objChangedPcscfs.GetCount(), 1);

    // return false if new Pcscf list is empty
    objPcscfPreset.RemoveAllElements();
    EXPECT_FALSE(m_pAosPcscf->GetChangedPcscfs(objChangedPcscfs, IpAddress::IPV4));
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

    EXPECT_TRUE(m_pAosPcscf->ProcessDnsQuery(AString("aaa.bbb.ccc"), 5060, IpAddress::IPV4));
    EXPECT_EQ(m_pAosPcscf->GetPcscfCount(), 1);
    EXPECT_EQ(m_pAosPcscf->GetPcscfs().GetElementAt(0), AString("0.0.0.4"));
}

TEST_F(AosPcscfTest, StartTimer)
{
    // re-start if the timer is already running
    EXPECT_CALL(m_objMockITimer, SetTimer(_, _)).Times(2);

    // start timer
    m_pAosPcscf->StartTimer(TestAosPcscf::TIMER_DNS_QUERY_RETRY, 10);

    // restart timer
    m_pAosPcscf->StartTimer(TestAosPcscf::TIMER_DNS_QUERY_RETRY, 10);
    m_pAosPcscf->StopTimer(TestAosPcscf::TIMER_DNS_QUERY_RETRY);
}

TEST_F(AosPcscfTest, StartTimer_FailureCases)
{
    EXPECT_CALL(m_objMockITimer, SetTimer(_, _)).Times(0);

    // do NOT start timer when duration is zero
    m_pAosPcscf->StartTimer(TestAosPcscf::TIMER_DNS_QUERY_RETRY, 0);

    // do NOT start timer for invalid timer type
    m_pAosPcscf->StartTimer(TestAosPcscf::TIMER_DNS_QUERY_RETRY + 100, 10);
}

TEST_F(AosPcscfTest, StopTimer)
{
    EXPECT_CALL(m_objMockITimer, KillTimer()).Times(1);

    // do NOT stop timer if there is no running timer
    m_pAosPcscf->StopTimer(TestAosPcscf::TIMER_DNS_QUERY_RETRY);

    // do NOT stop timer for invalid timer type
    m_pAosPcscf->StartTimer(TestAosPcscf::TIMER_DNS_QUERY_RETRY, 10);
    m_pAosPcscf->StopTimer(TestAosPcscf::TIMER_DNS_QUERY_RETRY + 100);

    // stop timer
    m_pAosPcscf->StopTimer(TestAosPcscf::TIMER_DNS_QUERY_RETRY);
}

TEST_F(AosPcscfTest, TimerExpired_FailureCases)
{
    EXPECT_CALL(m_objMockITimer, KillTimer()).Times(0);

    // do NOT handle timer expired event when the timer is null
    m_pAosPcscf->Timer_TimerExpired(nullptr);
}

TEST_F(AosPcscfTest, ProcessDnsRetryTimerExpired_RetrySucceeded)
{
    ImsList<IpAddress> objIpas;
    objIpas.Append(IpAddress(AString("0.0.0.4")));
    EXPECT_CALL(m_objMockIAosConnection, GetHostByNameInternal(_, _, _))
            .WillOnce(Return(-1))
            .WillOnce(DoAll(SetArgPointee<1>(objIpas), Return(1)));
    EXPECT_CALL(objMockIAosPcscfListener, Pcscf_NotifyResult(IMS_TRUE)).Times(1);
    m_pAosPcscf->SetListener(&objMockIAosPcscfListener);

    // DnsRetryTimer is started when the first DNS query fails
    EXPECT_FALSE(m_pAosPcscf->ProcessDnsQuery(AString("aaa.bbb.ccc"), 5060, IpAddress::IPV4));
    EXPECT_NE(m_pAosPcscf->GetDnsQueryRetryTimer(), nullptr);

    // invoke ProcessDnsRetryTimerExpired when the DnsRetryTimer is expired
    m_pAosPcscf->Timer_TimerExpired(m_pAosPcscf->GetDnsQueryRetryTimer());
    EXPECT_TRUE(m_pAosPcscf->IsConfigured());
    EXPECT_EQ(m_pAosPcscf->GetPcscfCount(), 1);
    EXPECT_EQ(m_pAosPcscf->GetPcscfs().GetElementAt(0), AString("0.0.0.4"));
}

TEST_F(AosPcscfTest, ProcessDnsRetryTimerExpired_RetryWithOtherIpType)
{
    EXPECT_CALL(m_objMockIAosConnection, GetHostByName(_, _, _))
            .Times(AnyNumber())
            .WillRepeatedly(Return(-1));
    EXPECT_CALL(m_objMockIAosConnection, GetLocalAddress(_))
            .WillOnce(ReturnRef(IpAddress::IPv6LOOPBACK));

    // DnsRetryTimer is started when the first DNS query fails
    EXPECT_FALSE(m_pAosPcscf->ProcessDnsQuery(AString("aaa.bbb.ccc"), 5060, IpAddress::IPV4));
    EXPECT_NE(m_pAosPcscf->GetDnsQueryRetryTimer(), nullptr);

    // when the DnsRetryTimer is expired perform PCSCF discovery again in the following order
    // 1. retry DNS query once more
    // 2. invoke ProcessDiscovery using IP type of RetryHost
    // 3. invoke ProcessDiscovery using other IP type
    EXPECT_CALL(m_objMockISubscriberConfig, GetPcscfDiscoveryMethods()).Times(2);
    m_pAosPcscf->SetOtherIpTypeRequired(true);
    m_pAosPcscf->Timer_TimerExpired(m_pAosPcscf->GetDnsQueryRetryTimer());
}

TEST_F(AosPcscfTest, TimerToStringForInvalidType)
{
    EXPECT_EQ(m_pAosPcscf->TimerToString(TestAosPcscf::TIMER_DNS_QUERY_RETRY + 100), "__INVALID__");
}