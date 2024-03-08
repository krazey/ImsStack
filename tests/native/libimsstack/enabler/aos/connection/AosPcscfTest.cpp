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

class TestAosPcscf : public AosPcscf
{
public:
    AStringArray m_objPcscfAddressPreset;
    ImsList<IMS_SINT32> m_objPcscfPortPreset;

public:
    DECLARE_USING(AosPcscf)

    TestAosPcscf(IN IAosAppContext* piAppContext) :
            AosPcscf(piAppContext)
    {
    }

    inline IAosPcscfListener* GetListener() { return m_piListener; }
    inline ITimer* GetDnsQueryRetryTimer() { return m_piDnsQueryRetryTimer; }
    inline void SetRegType(IN AosRegistrationType eType) { m_eRegType = eType; }
    inline AosRegistrationType GetRegType() { return m_eRegType; }
    inline void SetCurrentPcscfIndex(IN IMS_UINT32 nIndex) { m_nCurrentPcscfIndex = nIndex; }
    inline IMS_UINT32 GetCurrentPcscfIndex() { return m_nCurrentPcscfIndex; }
    inline void SetOtherIpTypeRequired(IN IMS_BOOL bRequired)
    {
        m_bOtherIpTypeRequired = bRequired;
    }
    inline IMS_BOOL GetOtherIpTypeRequired() { return m_bOtherIpTypeRequired; }
    inline ImsList<Pcscf*> GetcscfList() { return m_objPcscfList; }

    void PreparePcscfPreset()
    {
        AString strPcscfs = "0.0.0.1, 0.0.0.2, 0.0.0.3";
        m_objPcscfAddressPreset = strPcscfs.Split(',');
        for (int i = 0; i < m_objPcscfAddressPreset.GetCount(); i++)
        {
            m_objPcscfPortPreset.Append(5060 + i);
            AddPcscf(m_objPcscfAddressPreset.GetElementAt(i), m_objPcscfPortPreset.GetAt(i));
        }
    }

    IMS_BOOL IsValidPcscf(IN IMS_UINT32 nIndex)
    {
        if (nIndex < m_objPcscfList.GetSize())
        {
            Pcscf* pPcscf = m_objPcscfList.GetAt(nIndex);
            return pPcscf->IsAvailable();
        }

        return IMS_FALSE;
    }

    IMS_BOOL IsTriedPcscf(IN IMS_UINT32 nIndex)
    {
        if (nIndex < m_objPcscfList.GetSize())
        {
            Pcscf* pPcscf = m_objPcscfList.GetAt(nIndex);
            return pPcscf->IsTried();
        }

        return IMS_FALSE;
    }
};

class AosPcscfTest : public ::testing::Test
{
public:
    inline AosPcscfTest() :
            m_objMockITimer(m_objTimerService.GetMockTimer())
    {
        PlatformContext::GetInstance()->SetService(
                PlatformContext::SERVICE_TIMER, &m_objTimerService);
    }
    inline virtual ~AosPcscfTest()
    {
        PlatformContext::GetInstance()->SetService(PlatformContext::SERVICE_TIMER, IMS_NULL);
    }

    TestAosPcscf* m_pTestAosPcscf;
    TestTimerService m_objTimerService;

    IAosNConfiguration* m_piAosNConfiguration;
    ImsVector<IMS_SINT32> m_objDiscoveryMethods;

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
        m_piAosNConfiguration = AosProvider::GetInstance()->GetNConfiguration();
        AosProvider::GetInstance()->SetNConfiguration(&m_objMockIAosNConfiguration, SLOT_ID);

        ON_CALL(m_objMockIAosAppContext, GetProfileId()).WillByDefault(ReturnRef(PROFILE_ID));
        ON_CALL(m_objMockIAosAppContext, GetSlotId()).WillByDefault(Return(SLOT_ID));
        ON_CALL(m_objMockIAosNConfiguration, GetPcscfPort()).WillByDefault(Return(5060));
        ON_CALL(m_objMockIAosAppContext, GetConnection())
                .WillByDefault(Return(&m_objMockIAosConnection));
        ON_CALL(m_objMockIAosAppContext, GetRegistration())
                .WillByDefault(Return(&objMockIAosRegistration));
        ON_CALL(m_objMockIAosAppContext, GetSubscriber())
                .WillByDefault(Return(&m_objMockIAosSubscriber));
        ON_CALL(m_objMockIAosSubscriber, GetSubscriberConfig(_))
                .WillByDefault(Return(&m_objMockISubscriberConfig));
        ON_CALL(m_objMockISubscriberConfig, GetPcscfDiscoveryMethods())
                .WillByDefault(ReturnRef(m_objDiscoveryMethods));

        m_pTestAosPcscf = new TestAosPcscf(&m_objMockIAosAppContext);
    }

    virtual void TearDown() override
    {
        AosProvider::GetInstance()->SetNConfiguration(m_piAosNConfiguration, SLOT_ID);
        m_objDiscoveryMethods.Clear();

        if (m_pTestAosPcscf)
        {
            delete m_pTestAosPcscf;
        }
    }
};

TEST_F(AosPcscfTest, Init_UpdateRegType)
{
    EXPECT_CALL(objMockIAosRegistration, GetRegType())
            .WillRepeatedly(Return(AosRegistrationType::FAKE));

    m_pTestAosPcscf->Init();

    EXPECT_EQ(m_pTestAosPcscf->GetRegType(), AosRegistrationType::FAKE);
}

TEST_F(AosPcscfTest, CleanUp_ResetVariable)
{
    m_pTestAosPcscf->SetListener(&objMockIAosPcscfListener);
    m_pTestAosPcscf->SetConfigured(IMS_TRUE);

    m_pTestAosPcscf->CleanUp();

    EXPECT_EQ(m_pTestAosPcscf->GetListener(), nullptr);
    EXPECT_FALSE(m_pTestAosPcscf->IsConfigured());
}

TEST_F(AosPcscfTest, ConfigureWithIpVersion_DoNotProcessDiscoveryForInvalidLocalAddress)
{
    EXPECT_CALL(m_objMockIAosConnection, GetLocalAddress(_)).WillOnce(ReturnRef(IpAddress::ANY));
    EXPECT_CALL(m_objMockISubscriberConfig, GetPcscfDiscoveryMethods()).Times(0);

    m_pTestAosPcscf->Configure(IpAddress::IPV4);

    EXPECT_FALSE(m_pTestAosPcscf->GetOtherIpTypeRequired());
}

TEST_F(AosPcscfTest, ConfigureWithIpVersion_ProcessDiscoveryForValidLocalAddress)
{
    EXPECT_CALL(m_objMockIAosConnection, GetLocalAddress(_))
            .WillOnce(ReturnRef(IpAddress::LOOPBACK));
    EXPECT_CALL(m_objMockISubscriberConfig, GetPcscfDiscoveryMethods()).Times(1);

    m_pTestAosPcscf->Configure(IpAddress::IPV4);

    EXPECT_FALSE(m_pTestAosPcscf->GetOtherIpTypeRequired());
}

TEST_F(AosPcscfTest, ConfigureWithoutIpVersion_TryProcessDiscoveryForEachIpVerion)
{
    AStringArray objEmptyPcscfSet;
    EXPECT_CALL(m_objMockIAosConnection, GetLocalAddress(IpAddress::UNKNOWN))
            .WillOnce(ReturnRef(IpAddress::LOOPBACK));
    EXPECT_CALL(m_objMockIAosConnection, GetLocalAddress(IpAddress::IPV6))
            .WillOnce(ReturnRef(IpAddress::IPv6LOOPBACK));
    EXPECT_CALL(m_objMockIAosConnection, GetPcscfAddress(IpAddress::IPV4))
            .WillOnce(ReturnRef(objEmptyPcscfSet));
    EXPECT_CALL(m_objMockIAosConnection, GetPcscfAddress(IpAddress::IPV6))
            .WillOnce(ReturnRef(objEmptyPcscfSet));
    m_objDiscoveryMethods.Add(ISubscriberConfig::PCSCF_DISCOVERY_METHOD_PCO);

    m_pTestAosPcscf->Configure();
}

TEST_F(AosPcscfTest, SetAndCheckConfigured)
{
    m_pTestAosPcscf->SetConfigured(IMS_TRUE);
    EXPECT_TRUE(m_pTestAosPcscf->IsConfigured());

    m_pTestAosPcscf->SetConfigured(IMS_FALSE);
    EXPECT_FALSE(m_pTestAosPcscf->IsConfigured());
}

TEST_F(AosPcscfTest, IsAsyncDnsDiscoveryWhenRcsRegType)
{
    m_pTestAosPcscf->SetRegType(AosRegistrationType::NORMAL);
    EXPECT_FALSE(m_pTestAosPcscf->IsAsyncDnsDiscovery());

    m_pTestAosPcscf->SetRegType(AosRegistrationType::RCS);
    EXPECT_TRUE(m_pTestAosPcscf->IsAsyncDnsDiscovery());
}

TEST_F(AosPcscfTest, IsNotSinglePcoSchemeWhenInvalidSubscriber)
{
    EXPECT_CALL(m_objMockIAosSubscriber, GetSubscriberConfig(IAosSubscriber::FAKE))
            .WillOnce(Return(nullptr));
    m_objDiscoveryMethods.Add(ISubscriberConfig::PCSCF_DISCOVERY_METHOD_PCO);
    m_pTestAosPcscf->SetRegType(AosRegistrationType::FAKE);

    EXPECT_FALSE(m_pTestAosPcscf->IsSinglePcoScheme());
}

TEST_F(AosPcscfTest, IsNotSinglePcoSchemeWhenTwoDiscoveryMethod)
{
    m_objDiscoveryMethods.Add(ISubscriberConfig::PCSCF_DISCOVERY_METHOD_CONFIG);
    m_objDiscoveryMethods.Add(ISubscriberConfig::PCSCF_DISCOVERY_METHOD_PCO);
    EXPECT_FALSE(m_pTestAosPcscf->IsSinglePcoScheme());
}

TEST_F(AosPcscfTest, IsNotSinglePcoSchemeWhenEmptyDiscoveryMethod)
{
    m_objDiscoveryMethods.Clear();
    EXPECT_FALSE(m_pTestAosPcscf->IsSinglePcoScheme());
}

TEST_F(AosPcscfTest, IsSinglePcoSchemeWhenSinglePcoDiscoveryMethod)
{
    m_objDiscoveryMethods.Add(ISubscriberConfig::PCSCF_DISCOVERY_METHOD_PCO);
    EXPECT_TRUE(m_pTestAosPcscf->IsSinglePcoScheme());
}

TEST_F(AosPcscfTest, GetPcscfs_ReturnPcscfAddressSet)
{
    m_pTestAosPcscf->PreparePcscfPreset();
    AStringArray objGetPcscfs = m_pTestAosPcscf->GetPcscfs();
    for (int i = 0; i < objGetPcscfs.GetCount(); i++)
    {
        EXPECT_EQ(objGetPcscfs.GetElementAt(i),
                m_pTestAosPcscf->m_objPcscfAddressPreset.GetElementAt(i));
    }
}

TEST_F(AosPcscfTest, GetPcscfsPorts_ReturnPcscfPortSet)
{
    m_pTestAosPcscf->PreparePcscfPreset();
    ImsList<IMS_SINT32> objGetPcscfsPorts = m_pTestAosPcscf->GetPcscfsPorts();
    for (int i = 0; i < objGetPcscfsPorts.GetSize(); i++)
    {
        EXPECT_EQ(objGetPcscfsPorts.GetAt(i), m_pTestAosPcscf->m_objPcscfPortPreset.GetAt(i));
    }
}

TEST_F(AosPcscfTest, UpdatePcscfs_ReplaceWithNewPcscfs)
{
    AString strNewPcscfs = "0.0.0.4, 0.0.0.5, 0.0.0.4, 0.0.0.5";
    AStringArray objNewPcscfs = strNewPcscfs.Split(',');
    ImsList<IMS_SINT32> objNewPorts;
    objNewPorts.Append(5000);

    m_pTestAosPcscf->UpdatePcscfs(objNewPcscfs, objNewPorts);

    AStringArray objGetPcscfs = m_pTestAosPcscf->GetPcscfs();
    ImsList<IMS_SINT32> objGetPcscfsPorts = m_pTestAosPcscf->GetPcscfsPorts();

    // last new Pcscf is ignored because same address and port is in the list
    EXPECT_EQ(objGetPcscfs.GetCount(), 3);
    for (int i = 0; i < objGetPcscfs.GetCount(); i++)
    {
        EXPECT_EQ(objGetPcscfs.GetElementAt(i), objNewPcscfs.GetElementAt(i));
        if (i < objNewPorts.GetSize())
        {
            EXPECT_EQ(objGetPcscfsPorts.GetAt(i), 5000);
        }
        else
        {
            EXPECT_EQ(objGetPcscfsPorts.GetAt(i), 5060);
        }
    }
}

TEST_F(AosPcscfTest, HasPcscf_ReturnWhetherPcscfExist)
{
    m_pTestAosPcscf->PreparePcscfPreset();
    EXPECT_TRUE(m_pTestAosPcscf->HasPcscf(m_pTestAosPcscf->m_objPcscfAddressPreset.GetCount() - 1));
    EXPECT_FALSE(m_pTestAosPcscf->HasPcscf(m_pTestAosPcscf->m_objPcscfAddressPreset.GetCount()));
}

TEST_F(AosPcscfTest, GetPcscfCount)
{
    m_pTestAosPcscf->PreparePcscfPreset();
    EXPECT_EQ(
            m_pTestAosPcscf->GetPcscfCount(), m_pTestAosPcscf->m_objPcscfAddressPreset.GetCount());
}

TEST_F(AosPcscfTest, RemoveCurrentPcscf)
{
    m_pTestAosPcscf->PreparePcscfPreset();
    m_pTestAosPcscf->m_objPcscfAddressPreset.RemoveElementAt(m_pTestAosPcscf->GetCurrentIndex());

    m_pTestAosPcscf->RemoveCurrentPcscf();

    AStringArray objGetPcscfs = m_pTestAosPcscf->GetPcscfs();
    for (int i = 0; i < objGetPcscfs.GetCount(); i++)
    {
        EXPECT_EQ(objGetPcscfs.GetElementAt(i),
                m_pTestAosPcscf->m_objPcscfAddressPreset.GetElementAt(i));
    }
}

TEST_F(AosPcscfTest, ManageValidPcscf)
{
    IMS_UINT32 nUnavailableTimeSec = 5;

    m_pTestAosPcscf->PreparePcscfPreset();
    EXPECT_TRUE(m_pTestAosPcscf->IsValidPcscf(m_pTestAosPcscf->GetCurrentIndex()));

    // Set with timer
    m_pTestAosPcscf->SetCurrentPcscfInvalid(IMS_TRUE, nUnavailableTimeSec);
    EXPECT_FALSE(m_pTestAosPcscf->IsValidPcscf(m_pTestAosPcscf->GetCurrentIndex()));

    m_pTestAosPcscf->SetAllPcscfValid();
    EXPECT_TRUE(m_pTestAosPcscf->IsValidPcscf(m_pTestAosPcscf->GetCurrentIndex()));

    // Set without timer
    m_pTestAosPcscf->SetCurrentPcscfInvalid();
    EXPECT_FALSE(m_pTestAosPcscf->IsValidPcscf(m_pTestAosPcscf->GetCurrentIndex()));
}

TEST_F(AosPcscfTest, ManageTriedPcscf)
{
    m_pTestAosPcscf->PreparePcscfPreset();
    AStringArray objPcscfs = m_pTestAosPcscf->GetPcscfs();

    EXPECT_FALSE(m_pTestAosPcscf->IsAllPcscfTried());
    for (IMS_UINT32 nIndex = 0; nIndex < objPcscfs.GetCount(); nIndex++)
    {
        m_pTestAosPcscf->SetCurrentPcscfIndex(nIndex);
        EXPECT_FALSE(m_pTestAosPcscf->IsTriedPcscf(nIndex));
        m_pTestAosPcscf->SetCurrentPcscfTried();
        EXPECT_TRUE(m_pTestAosPcscf->IsTriedPcscf(nIndex));
    }
    EXPECT_TRUE(m_pTestAosPcscf->IsAllPcscfTried());

    m_pTestAosPcscf->ResetAllPcscfTried();
    for (IMS_UINT32 nIndex = 0; nIndex < objPcscfs.GetCount(); nIndex++)
    {
        EXPECT_FALSE(m_pTestAosPcscf->IsTriedPcscf(nIndex));
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
    m_pTestAosPcscf->PreparePcscfPreset();

    // do NOT increase if configured retry count is zero
    m_pTestAosPcscf->SetCurrentPcscfIndex(0);
    m_pTestAosPcscf->IncreaseCurrentPcscfTriedCount();
    EXPECT_EQ(m_pTestAosPcscf->GetCurrentPcscfTriedCount(), 0);

    // increase tried count of current pcscf
    m_pTestAosPcscf->IncreaseCurrentPcscfTriedCount();
    EXPECT_EQ(m_pTestAosPcscf->GetCurrentPcscfTriedCount(), 1);

    // do NOT handle invalid Pcscf index
    m_pTestAosPcscf->SetCurrentPcscfIndex(m_pTestAosPcscf->GetPcscfCount());
    m_pTestAosPcscf->IncreaseCurrentPcscfTriedCount();
    EXPECT_EQ(m_pTestAosPcscf->GetCurrentPcscfTriedCount(), 0);
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
    m_pTestAosPcscf->PreparePcscfPreset();

    // increase tried count of current pcscf
    m_pTestAosPcscf->SetCurrentPcscfIndex(0);
    m_pTestAosPcscf->IncreaseCurrentPcscfTriedCount();

    // do NOT reset if configured retry count is zero
    m_pTestAosPcscf->ResetCurrentPcscfTriedCount();
    EXPECT_EQ(m_pTestAosPcscf->GetCurrentPcscfTriedCount(), 1);

    // reset tried count of current pcscf
    m_pTestAosPcscf->ResetCurrentPcscfTriedCount();
    EXPECT_EQ(m_pTestAosPcscf->GetCurrentPcscfTriedCount(), 0);
}

TEST_F(AosPcscfTest, ResetAllPcscfTriedCount)
{
    EXPECT_CALL(m_objMockIAosNConfiguration, GetRegRetryCountPerPcscf())
            .Times(AnyNumber())
            .WillRepeatedly(Return(0));
    EXPECT_CALL(m_objMockIAosNConfiguration, GetRegRetryCountOnSinglePcscf())
            .Times(AnyNumber())
            .WillRepeatedly(Return(2));
    m_pTestAosPcscf->PreparePcscfPreset();
    AStringArray objPcscfs = m_pTestAosPcscf->GetPcscfs();

    for (IMS_UINT32 nIndex = 0; nIndex < objPcscfs.GetCount(); nIndex++)
    {
        m_pTestAosPcscf->SetCurrentPcscfIndex(nIndex);
        EXPECT_EQ(m_pTestAosPcscf->GetCurrentPcscfTriedCount(), 0);
        m_pTestAosPcscf->IncreaseCurrentPcscfTriedCount();
        EXPECT_EQ(m_pTestAosPcscf->GetCurrentPcscfTriedCount(), 1);
    }

    // do NOT reset if configured retry count is zero
    EXPECT_CALL(m_objMockIAosNConfiguration, GetRegRetryCountOnSinglePcscf())
            .Times(AnyNumber())
            .WillRepeatedly(Return(0));

    m_pTestAosPcscf->ResetAllPcscfTriedCount();

    for (IMS_UINT32 nIndex = 0; nIndex < objPcscfs.GetCount(); nIndex++)
    {
        m_pTestAosPcscf->SetCurrentPcscfIndex(nIndex);
        EXPECT_EQ(m_pTestAosPcscf->GetCurrentPcscfTriedCount(), 1);
    }

    // reset tried count of all pcscf
    EXPECT_CALL(m_objMockIAosNConfiguration, GetRegRetryCountOnSinglePcscf())
            .Times(AnyNumber())
            .WillRepeatedly(Return(2));

    m_pTestAosPcscf->ResetAllPcscfTriedCount();

    for (IMS_UINT32 nIndex = 0; nIndex < objPcscfs.GetCount(); nIndex++)
    {
        m_pTestAosPcscf->SetCurrentPcscfIndex(nIndex);
        EXPECT_EQ(m_pTestAosPcscf->GetCurrentPcscfTriedCount(), 0);
    }
}

TEST_F(AosPcscfTest, GetCurrentPcscf)
{
    AString objPcscfAddress;
    IMS_UINT32 nPcscfPort;

    // update current port as 5060 if it is 0
    m_pTestAosPcscf->AddPcscf(AString("0.0.0.1"), 0);
    m_pTestAosPcscf->SetCurrentPcscfIndex(0);
    EXPECT_TRUE(m_pTestAosPcscf->GetCurrentPcscf(objPcscfAddress, nPcscfPort));
    EXPECT_EQ(objPcscfAddress, AString("0.0.0.1"));
    EXPECT_EQ(nPcscfPort, 5060);

    // get current Pcscf successfully
    m_pTestAosPcscf->AddPcscf(AString("0.0.0.2"), 5061);
    m_pTestAosPcscf->SetCurrentPcscfIndex(1);
    EXPECT_TRUE(m_pTestAosPcscf->GetCurrentPcscf(objPcscfAddress, nPcscfPort));
    EXPECT_EQ(objPcscfAddress, AString("0.0.0.2"));
    EXPECT_EQ(nPcscfPort, 5061);

    // return false for Pcscf of index is null
    m_pTestAosPcscf->GetcscfList().Append(nullptr);
    m_pTestAosPcscf->SetCurrentPcscfIndex(2);
    EXPECT_FALSE(m_pTestAosPcscf->GetCurrentPcscf(objPcscfAddress, nPcscfPort));

    // return false for Pcscf index larger than the Pcscf list size
    m_pTestAosPcscf->SetCurrentPcscfIndex(3);
    EXPECT_FALSE(m_pTestAosPcscf->GetCurrentPcscf(objPcscfAddress, nPcscfPort));
}

TEST_F(AosPcscfTest, GetCurrentIndex)
{
    m_pTestAosPcscf->PreparePcscfPreset();
    AStringArray objPcscfs = m_pTestAosPcscf->GetPcscfs();
    for (IMS_UINT32 nIndex = 0; nIndex < objPcscfs.GetCount(); nIndex++)
    {
        m_pTestAosPcscf->SetCurrentPcscfIndex(nIndex);
        EXPECT_EQ(m_pTestAosPcscf->GetCurrentIndex(), nIndex);
    }
}

TEST_F(AosPcscfTest, IsFirstPcscf)
{
    m_pTestAosPcscf->SetCurrentPcscfIndex(0);
    EXPECT_TRUE(m_pTestAosPcscf->IsFirstPcscf());

    m_pTestAosPcscf->SetCurrentPcscfIndex(1);
    EXPECT_FALSE(m_pTestAosPcscf->IsFirstPcscf());
}

TEST_F(AosPcscfTest, GetFirstPcscf)
{
    AString objPcscfAddress;
    IMS_UINT32 nPcscfPort;

    // return false if Pcscf list is empty
    EXPECT_FALSE(m_pTestAosPcscf->GetFirstPcscf(objPcscfAddress, nPcscfPort));

    // return false if first Pcscf is null
    m_pTestAosPcscf->GetcscfList().Append(nullptr);
    EXPECT_FALSE(m_pTestAosPcscf->GetFirstPcscf(objPcscfAddress, nPcscfPort));
    m_pTestAosPcscf->GetcscfList().Clear();

    // get first Pcscf successfully
    m_pTestAosPcscf->AddPcscf(AString("0.0.0.1"), 5060);
    EXPECT_TRUE(m_pTestAosPcscf->GetFirstPcscf(objPcscfAddress, nPcscfPort));
    EXPECT_EQ(objPcscfAddress, AString("0.0.0.1"));
    EXPECT_EQ(nPcscfPort, 5060);
    EXPECT_EQ(m_pTestAosPcscf->GetCurrentPcscfIndex(), 0);
}

TEST_F(AosPcscfTest, HasNextPcscf)
{
    EXPECT_CALL(m_objMockIAosNConfiguration, GetRegRetryDefaultPolicy())
            .Times(2)
            .WillOnce(Return(CarrierConfig::Assets::DEFAULT_RETRY_POLICY_SPEC))
            .WillOnce(Return(CarrierConfig::Assets::DEFAULT_RETRY_POLICY_CIRCULAR_NEXT_PCSCF));

    // return false if Pcscf list is empty
    EXPECT_FALSE(m_pTestAosPcscf->HasNextPcscf());

    // return true If there is an available Pcscf after the current Pcscf index
    m_pTestAosPcscf->PreparePcscfPreset();
    m_pTestAosPcscf->SetCurrentPcscfIndex(0);
    EXPECT_TRUE(m_pTestAosPcscf->HasNextPcscf());

    // return false If there is no available Pcscf after the current Pcscf index
    m_pTestAosPcscf->SetCurrentPcscfIndex(m_pTestAosPcscf->GetPcscfCount());
    EXPECT_FALSE(m_pTestAosPcscf->HasNextPcscf());

    // if DEFAULT_RETRY_POLICY_CIRCULAR_NEXT_PCSCF is configured, check whole Pcscf list
    m_pTestAosPcscf->SetCurrentPcscfIndex(m_pTestAosPcscf->GetPcscfCount());
    EXPECT_TRUE(m_pTestAosPcscf->HasNextPcscf());
}

TEST_F(AosPcscfTest, GetNextPcscf)
{
    AString objPcscfAddress;
    IMS_UINT32 nPcscfPort;
    m_pTestAosPcscf->SetCurrentPcscfIndex(0);
    m_pTestAosPcscf->AddPcscf(AString("0.0.0.1"), 5060);
    m_pTestAosPcscf->AddPcscf(AString("0.0.0.2"), 5061);

    // return true if there is an available Pcscf after the current Pcscf index
    EXPECT_TRUE(m_pTestAosPcscf->GetNextPcscf(objPcscfAddress, nPcscfPort));
    EXPECT_EQ(objPcscfAddress, AString("0.0.0.2"));
    EXPECT_EQ(nPcscfPort, 5061);

    // return false if there is no available Pcscf after the current Pcscf index
    m_pTestAosPcscf->SetCurrentPcscfIndex(m_pTestAosPcscf->GetPcscfCount());
    EXPECT_FALSE(m_pTestAosPcscf->GetNextPcscf(objPcscfAddress, nPcscfPort));
}

TEST_F(AosPcscfTest, SetFirstPcscfIndex_UdateCurrentPcscIndexAsZero)
{
    m_pTestAosPcscf->SetCurrentPcscfIndex(3);
    m_pTestAosPcscf->SetFirstPcscfIndex();
    EXPECT_EQ(m_pTestAosPcscf->GetCurrentIndex(), 0);
}

TEST_F(AosPcscfTest, CheckAndProcessChangeFromPco_NotHandledCases)
{
    m_pTestAosPcscf->PreparePcscfPreset();

    // return false if fail to get current Pcscf
    m_pTestAosPcscf->SetCurrentPcscfIndex(m_pTestAosPcscf->GetPcscfCount());
    EXPECT_FALSE(m_pTestAosPcscf->CheckAndProcessChangeFromPco());

    // return false if current Pcscf address is invalid
    m_pTestAosPcscf->AddPcscf(AString("InvalidAddress"), 5060);
    EXPECT_FALSE(m_pTestAosPcscf->CheckAndProcessChangeFromPco());
    m_pTestAosPcscf->RemoveCurrentPcscf();

    // return false if Pcscf list is not changed
    m_pTestAosPcscf->SetCurrentPcscfIndex(0);
    EXPECT_CALL(m_objMockIAosConnection, GetPcscfAddress(_))
            .WillOnce(ReturnRef(m_pTestAosPcscf->m_objPcscfAddressPreset));
    EXPECT_FALSE(m_pTestAosPcscf->CheckAndProcessChangeFromPco());

    // return false if new Pcscf list is empty
    AStringArray objEmptyArray;
    EXPECT_CALL(m_objMockIAosConnection, GetPcscfAddress(_)).WillOnce(ReturnRef(objEmptyArray));
    EXPECT_FALSE(m_pTestAosPcscf->CheckAndProcessChangeFromPco());
}

TEST_F(AosPcscfTest, CheckAndProcessChangeFromPco_ReorderIfCurrentPcscfExist)
{
    m_pTestAosPcscf->PreparePcscfPreset();
    AString strNewPcscfs = "0.0.0.4, 0.0.0.2, 0.0.0.5";
    AStringArray objNewPcscfs = strNewPcscfs.Split(',');
    EXPECT_CALL(m_objMockIAosConnection, GetPcscfAddress(_))
            .Times(AnyNumber())
            .WillRepeatedly(ReturnRef(objNewPcscfs));

    m_pTestAosPcscf->SetCurrentPcscfIndex(1);
    EXPECT_TRUE(m_pTestAosPcscf->CheckAndProcessChangeFromPco());
    EXPECT_EQ(m_pTestAosPcscf->GetChangedType(), IAosPcscf::TYPE_CHANGED_REORDER);
    EXPECT_EQ(m_pTestAosPcscf->GetCurrentPcscfIndex(), 0);
}

TEST_F(AosPcscfTest, CheckAndProcessChangeFromPco_DifferentIfCurrentPcscfNotExist)
{
    m_pTestAosPcscf->PreparePcscfPreset();
    AString strNewPcscfs = "0.0.0.4, 0.0.0.5";
    AStringArray objNewPcscfs = strNewPcscfs.Split(',');
    EXPECT_CALL(m_objMockIAosConnection, GetPcscfAddress(_))
            .Times(AnyNumber())
            .WillRepeatedly(ReturnRef(objNewPcscfs));

    EXPECT_TRUE(m_pTestAosPcscf->CheckAndProcessChangeFromPco());
    EXPECT_EQ(m_pTestAosPcscf->GetChangedType(), IAosPcscf::TYPE_CHANGED_DIFFERENT);
    EXPECT_EQ(m_pTestAosPcscf->GetCurrentPcscfIndex(), 0);
}

TEST_F(AosPcscfTest, SetListener)
{
    EXPECT_CALL(objMockIAosPcscfListener, Pcscf_NotifyResult(IMS_TRUE)).Times(1);
    m_pTestAosPcscf->SetListener(&objMockIAosPcscfListener);
    m_pTestAosPcscf->GetListener()->Pcscf_NotifyResult(IMS_TRUE);
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
    EXPECT_TRUE(m_pTestAosPcscf->GetChangedPcscfs(objChangedPcscfs, IpAddress::IPV4));
    EXPECT_EQ(objChangedPcscfs.GetCount(), 1);

    // return false if new Pcscf list is empty
    objPcscfPreset.RemoveAllElements();
    EXPECT_FALSE(m_pTestAosPcscf->GetChangedPcscfs(objChangedPcscfs, IpAddress::IPV4));
}

TEST_F(AosPcscfTest, ProcessDiscovery_PcoMethod)
{
    m_objDiscoveryMethods.Add(ISubscriberConfig::PCSCF_DISCOVERY_METHOD_PCO);
    AString strAddress = AString("0.0.0.4");
    ServerAddress* pSa = new ServerAddress(strAddress, 5060);
    EXPECT_CALL(m_objMockISubscriberConfig, GetPcscfAddress())
            .Times(AnyNumber())
            .WillRepeatedly(Return(pSa));
    AString strPcscfs = "0.0.0.4, 0.0.0.0, 0.0.0.4, ::1, invalidAddress";
    AStringArray objPcscfPreset = strPcscfs.Split(',');
    EXPECT_CALL(m_objMockIAosConnection, GetPcscfAddress(IpAddress::IPV4))
            .WillOnce(ReturnRef(objPcscfPreset));

    m_pTestAosPcscf->ProcessDiscovery(IpAddress::IPV4);
    EXPECT_TRUE(m_pTestAosPcscf->IsConfigured());
    EXPECT_EQ(m_pTestAosPcscf->GetPcscfCount(), 1);
}

TEST_F(AosPcscfTest, ProcessDiscovery_ConfigMethod)
{
    m_objDiscoveryMethods.Add(ISubscriberConfig::PCSCF_DISCOVERY_METHOD_CONFIG);
    ImsVector<ServerAddress*> objPcscfAddresses;
    AString strAddresses = AString(", 0.0.0.4, 0.0.0.4, ::1, hostName");
    AStringArray objPcscfAddress = strAddresses.Split(',');
    for (int i = 0; i < objPcscfAddress.GetCount(); i++)
    {
        ServerAddress* pSa = new ServerAddress(objPcscfAddress.GetElementAt(i), 0);
        objPcscfAddresses.Add(pSa);
    }
    EXPECT_CALL(m_objMockISubscriberConfig, GetPcscfAddresses())
            .Times(AnyNumber())
            .WillRepeatedly(ReturnRef(objPcscfAddresses));
    EXPECT_CALL(m_objMockIAosConnection, GetHostByName(_, _, _))
            .Times(AnyNumber())
            .WillRepeatedly(Return(-1));

    // success to configure Pcscf in SubscriberConfig
    m_pTestAosPcscf->ProcessDiscovery(IpAddress::IPV4);
    EXPECT_TRUE(m_pTestAosPcscf->IsConfigured());
    EXPECT_EQ(m_pTestAosPcscf->GetPcscfCount(), 1);
    m_pTestAosPcscf->CleanAll();

    // start TIMER_DNS_QUERY_RETRY when fail to DNS query
    objPcscfAddresses.Clear();
    objPcscfAddresses.Add(new ServerAddress(AString("hostName"), 0));
    m_pTestAosPcscf->ProcessDiscovery(IpAddress::IPV4);
    EXPECT_FALSE(m_pTestAosPcscf->IsConfigured());
    m_pTestAosPcscf->CleanAll();
}

TEST_F(AosPcscfTest, GetNextDiscoveryMethod)
{
    IMS_SINT32 nDiscoveryMethod;
    m_pTestAosPcscf->SetRegType(AosRegistrationType::FAKE);
    EXPECT_CALL(m_objMockIAosSubscriber, GetSubscriberConfig(IAosSubscriber::FAKE))
            .Times(AnyNumber())
            .WillOnce(Return(nullptr))
            .WillRepeatedly(Return(&m_objMockISubscriberConfig));

    // return false if SubscriberConfig is null
    EXPECT_FALSE(m_pTestAosPcscf->GetNextDiscoveryMethod(nDiscoveryMethod));

    // return false if there is no configured method
    EXPECT_FALSE(m_pTestAosPcscf->GetNextDiscoveryMethod(nDiscoveryMethod));

    // return true if there is configured method
    m_objDiscoveryMethods.Add(ISubscriberConfig::PCSCF_DISCOVERY_METHOD_CONFIG);
    EXPECT_TRUE(m_pTestAosPcscf->GetNextDiscoveryMethod(nDiscoveryMethod));

    // return false if there is no more configured method
    EXPECT_FALSE(m_pTestAosPcscf->GetNextDiscoveryMethod(nDiscoveryMethod));
}

TEST_F(AosPcscfTest, GetFromConf_FailureCases)
{
    ImsVector<ServerAddress*> objPcscfAddresses;
    EXPECT_CALL(m_objMockIAosSubscriber, GetSubscriberConfig(_))
            .WillOnce(Return(nullptr))
            .WillOnce(Return(&m_objMockISubscriberConfig));
    EXPECT_CALL(m_objMockISubscriberConfig, GetPcscfAddresses())
            .Times(AnyNumber())
            .WillRepeatedly(ReturnRef(objPcscfAddresses));

    // return false if SubscriberConfig is null
    EXPECT_FALSE(m_pTestAosPcscf->GetFromConf(IpAddress::IPV4));

    // return false if pre-configured Pcscf list is empty
    EXPECT_FALSE(m_pTestAosPcscf->GetFromConf(IpAddress::IPV4));
}

TEST_F(AosPcscfTest, ProcessDnsQuery)
{
    ImsList<IpAddress> objIpas;
    objIpas.Append(IpAddress(AString("0.0.0.4")));
    objIpas.Append(IpAddress(AString("fe80::1")));
    objIpas.Append(IpAddress::ANY);
    objIpas.Append(IpAddress(AString("0.0.0.4")));
    EXPECT_CALL(m_objMockIAosConnection, GetHostByNameInternal(_, _, _))
            .WillOnce(DoAll(SetArgPointee<1>(objIpas), Return(1)));

    EXPECT_TRUE(m_pTestAosPcscf->ProcessDnsQuery(AString("aaa.bbb.ccc"), 5060, IpAddress::IPV4));
    EXPECT_EQ(m_pTestAosPcscf->GetPcscfCount(), 1);
    EXPECT_EQ(m_pTestAosPcscf->GetPcscfs().GetElementAt(0), AString("0.0.0.4"));
}

TEST_F(AosPcscfTest, StartTimer)
{
    // re-start if the timer is already running
    EXPECT_CALL(m_objMockITimer, SetTimer(_, _)).Times(2);

    // start timer
    m_pTestAosPcscf->StartTimer(TestAosPcscf::TIMER_DNS_QUERY_RETRY, 10);

    // restart timer
    m_pTestAosPcscf->StartTimer(TestAosPcscf::TIMER_DNS_QUERY_RETRY, 10);
    m_pTestAosPcscf->StopTimer(TestAosPcscf::TIMER_DNS_QUERY_RETRY);
}

TEST_F(AosPcscfTest, StartTimer_FailureCases)
{
    EXPECT_CALL(m_objMockITimer, SetTimer(_, _)).Times(0);

    // do NOT start timer when duration is zero
    m_pTestAosPcscf->StartTimer(TestAosPcscf::TIMER_DNS_QUERY_RETRY, 0);

    // do NOT start timer for invalid timer type
    m_pTestAosPcscf->StartTimer(TestAosPcscf::TIMER_DNS_QUERY_RETRY + 100, 10);
}

TEST_F(AosPcscfTest, StopTimer)
{
    EXPECT_CALL(m_objMockITimer, KillTimer()).Times(1);

    // do NOT stop timer if there is no running timer
    m_pTestAosPcscf->StopTimer(TestAosPcscf::TIMER_DNS_QUERY_RETRY);

    // do NOT stop timer for invalid timer type
    m_pTestAosPcscf->StartTimer(TestAosPcscf::TIMER_DNS_QUERY_RETRY, 10);
    m_pTestAosPcscf->StopTimer(TestAosPcscf::TIMER_DNS_QUERY_RETRY + 100);

    // stop timer
    m_pTestAosPcscf->StopTimer(TestAosPcscf::TIMER_DNS_QUERY_RETRY);
}

TEST_F(AosPcscfTest, TimerExpired_FailureCases)
{
    EXPECT_CALL(m_objMockITimer, KillTimer()).Times(0);

    // do NOT handle timer expired event when the timer is null
    m_pTestAosPcscf->Timer_TimerExpired(nullptr);
}

TEST_F(AosPcscfTest, ProcessDnsRetryTimerExpired_RetrySucceeded)
{
    ImsList<IpAddress> objIpas;
    objIpas.Append(IpAddress(AString("0.0.0.4")));
    EXPECT_CALL(m_objMockIAosConnection, GetHostByNameInternal(_, _, _))
            .WillOnce(Return(-1))
            .WillOnce(DoAll(SetArgPointee<1>(objIpas), Return(1)));
    EXPECT_CALL(objMockIAosPcscfListener, Pcscf_NotifyResult(IMS_TRUE)).Times(1);
    m_pTestAosPcscf->SetListener(&objMockIAosPcscfListener);

    // DnsRetryTimer is started when the first DNS query fails
    EXPECT_FALSE(m_pTestAosPcscf->ProcessDnsQuery(AString("aaa.bbb.ccc"), 5060, IpAddress::IPV4));
    EXPECT_NE(m_pTestAosPcscf->GetDnsQueryRetryTimer(), nullptr);

    // invoke ProcessDnsRetryTimerExpired when the DnsRetryTimer is expired
    m_pTestAosPcscf->Timer_TimerExpired(m_pTestAosPcscf->GetDnsQueryRetryTimer());
    EXPECT_TRUE(m_pTestAosPcscf->IsConfigured());
    EXPECT_EQ(m_pTestAosPcscf->GetPcscfCount(), 1);
    EXPECT_EQ(m_pTestAosPcscf->GetPcscfs().GetElementAt(0), AString("0.0.0.4"));
}

TEST_F(AosPcscfTest, ProcessDnsRetryTimerExpired_RetryWithOtherIpType)
{
    EXPECT_CALL(m_objMockIAosConnection, GetHostByName(_, _, _))
            .Times(AnyNumber())
            .WillRepeatedly(Return(-1));
    EXPECT_CALL(m_objMockIAosConnection, GetLocalAddress(_))
            .WillOnce(ReturnRef(IpAddress::IPv6LOOPBACK));

    // DnsRetryTimer is started when the first DNS query fails
    EXPECT_FALSE(m_pTestAosPcscf->ProcessDnsQuery(AString("aaa.bbb.ccc"), 5060, IpAddress::IPV4));
    EXPECT_NE(m_pTestAosPcscf->GetDnsQueryRetryTimer(), nullptr);

    // when the DnsRetryTimer is expired perform PCSCF discovery again in the following order
    // 1. retry DNS query once more
    // 2. invoke ProcessDiscovery using IP type of RetryHost
    // 3. invoke ProcessDiscovery using other IP type
    EXPECT_CALL(m_objMockISubscriberConfig, GetPcscfDiscoveryMethods()).Times(2);
    m_pTestAosPcscf->SetOtherIpTypeRequired(true);
    m_pTestAosPcscf->Timer_TimerExpired(m_pTestAosPcscf->GetDnsQueryRetryTimer());
}

TEST_F(AosPcscfTest, TimerToStringForInvalidType)
{
    EXPECT_EQ(m_pTestAosPcscf->TimerToString(TestAosPcscf::TIMER_DNS_QUERY_RETRY + 100),
            "__INVALID__");
}