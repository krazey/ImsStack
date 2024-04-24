/*
 * Copyright (C) 2023 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHout WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "MockISubscriberConfigListener.h"
#include "MockISubscriberInfoListener.h"
#include "PlatformContext.h"
#include "ServerAddress.h"
#include "TestConfigService.h"
#include "TestPhoneInfoService.h"
#include "TestThreadService.h"
#include "TestTimerService.h"
#include "TestUtilService.h"
#include "private/SubscriberConfig.h"

using ::testing::_;
using ::testing::AnyNumber;
using ::testing::Eq;
using ::testing::Invoke;
using ::testing::Return;
using ::testing::Unused;

namespace android
{

class SubscriberConfigTest : public ::testing::Test
{
public:
    inline SubscriberConfigTest() :
            m_nImpuRefIndex(1),
            m_strDomain("ims.mnc001.mcc001.3gppnetwork.org"),
            m_strImpi("1111@ims.mnc001.mcc001.3gppnetwork.org"),
            m_pSubscriberConfig(IMS_NULL),
            m_piIsimListener(IMS_NULL),
            m_pConfigBase(IMS_NULL)
    {
        m_objImpu.AddElement("sip:1111@ims.mnc001.mcc001.3gppnetwork.org");
        m_objImpu.AddElement("tel:1111");

        m_objPcscfs.AddElement("192.168.0.1");
        m_objPcscfs.AddElement("192.168.0.2");
    }

public:
    IMS_SINT32 m_nImpuRefIndex;
    AString m_strDomain;
    AString m_strImpi;
    AStringArray m_objImpu;
    AStringArray m_objPcscfs;

    MockISubscriberConfigListener m_objSubscriberConfigListener;
    MockIIsim m_objIsim;
    MockISubscriberInfoListener m_objSubInfoListener;

    SubscriberConfig* m_pSubscriberConfig;
    IIsimListener* m_piIsimListener;
    ConfigBase* m_pConfigBase;

    TestPhoneInfoService m_objPhoneInfoService;
    TestConfigService m_objConfigService;
    TestTimerService m_objTimerService;
    TestUtilService m_objUtilService;

protected:
    virtual void SetUp() override
    {
        PlatformContext::GetInstance()->SetService(
                PlatformContext::SERVICE_UTIL, &m_objUtilService);
        PlatformContext::GetInstance()->SetService(
                PlatformContext::SERVICE_TIMER, &m_objTimerService);
        PlatformContext::GetInstance()->SetService(
                PlatformContext::SERVICE_CONFIG, &m_objConfigService);
        PlatformContext::GetInstance()->SetService(
                PlatformContext::SERVICE_PHONE_INFO, &m_objPhoneInfoService);

        m_objPhoneInfoService.SetIsim(&m_objIsim);

        m_pSubscriberConfig = new SubscriberConfig(IMS_SLOT_0, "subscriber");
        ISubscriberConfig* piSubConfig = static_cast<ISubscriberConfig*>(m_pSubscriberConfig);
        piSubConfig->SetListener(&m_objSubscriberConfigListener);
        m_piIsimListener = static_cast<IIsimListener*>(m_pSubscriberConfig);
        m_pConfigBase = static_cast<ConfigBase*>(m_pSubscriberConfig);
        m_pSubscriberConfig->SetSubscriberInfoListener(&m_objSubInfoListener);
        SetUpPersistentProperties();

        EXPECT_CALL(m_objConfigService.GetMockCarrierConfig(),
                GetString(Eq(CarrierConfig::Ims::KEY_PHONE_CONTEXT_DOMAIN_NAME_STRING), _))
                .Times(AnyNumber())
                .WillRepeatedly(Return(m_strDomain));

        EXPECT_CALL(m_objConfigService.GetMockCarrierConfig(),
                GetInt(Eq(CarrierConfig::Ims::KEY_SIP_SERVER_PORT_NUMBER_INT), _))
                .Times(AnyNumber())
                .WillRepeatedly(Return(5060));

        EXPECT_CALL(m_objConfigService.GetMockCarrierConfig(),
                GetInt(Eq(CarrierConfig::Ims::KEY_ISIM_INDEX_FOR_IMPU_INT), _))
                .Times(AnyNumber())
                .WillRepeatedly(Return(m_nImpuRefIndex));

        ImsVector<IMS_SINT32> objImsIdentity;
        objImsIdentity.Add(CarrierConfig::Ims::IMS_IDENTITY_PRIORITY_ISIM);

        EXPECT_CALL(m_objConfigService.GetMockCarrierConfig(),
                GetIntArray(Eq(CarrierConfig::Ims::KEY_IMS_IDENTITY_PRIORITY_INT_ARRAY)))
                .Times(AnyNumber())
                .WillRepeatedly(Return(objImsIdentity));

        ImsVector<IMS_SINT32> objPcscfs;
        EXPECT_CALL(m_objConfigService.GetMockCarrierConfig(),
                GetIntArray(Eq(CarrierConfig::Ims::KEY_PCSCF_DISCOVERY_METHOD_INT_ARRAY)))
                .Times(AnyNumber())
                .WillRepeatedly(Return(objPcscfs));
    }

    virtual void TearDown() override
    {
        if (m_pSubscriberConfig != IMS_NULL)
        {
            ISubscriberConfig* piSubConfig = static_cast<ISubscriberConfig*>(m_pSubscriberConfig);
            piSubConfig->RemoveListener(&m_objSubscriberConfigListener);

            EXPECT_CALL(m_objConfigService.GetMockCarrierConfig(), RemoveListener(_)).Times(1);
            delete m_pSubscriberConfig;
            m_pSubscriberConfig = IMS_NULL;
        }
        PlatformContext::GetInstance()->SetService(PlatformContext::SERVICE_UTIL, IMS_NULL);
        PlatformContext::GetInstance()->SetService(PlatformContext::SERVICE_TIMER, IMS_NULL);
        PlatformContext::GetInstance()->SetService(PlatformContext::SERVICE_CONFIG, IMS_NULL);
        PlatformContext::GetInstance()->SetService(PlatformContext::SERVICE_PHONE_INFO, IMS_NULL);
    }

    void SetUpIsimInit()
    {
        EXPECT_CALL(m_objIsim, Init()).Times(1);
        EXPECT_CALL(m_objIsim, AddListener(_)).Times(1);
    }

    void SetUpIsimRecords(IMS_SINT32 nCount)
    {
        EXPECT_CALL(m_objIsim, GetImpi()).Times(nCount).WillRepeatedly(Return(m_strImpi));
        EXPECT_CALL(m_objIsim, GetImpu()).Times(nCount).WillRepeatedly(Return(m_objImpu));
        EXPECT_CALL(m_objIsim, GetHomeDomainName())
                .Times(nCount)
                .WillRepeatedly(Return(m_strDomain));
    }

    void SetUpPersistentProperties()
    {
        ON_CALL(m_objUtilService.GetMockPrivateProperty(),
                GetPersistent(Eq(ImsPrivateProperties::Persistent::KEY_CONFIG_HOME_DOMAIN_NAME),
                        Eq(IMS_SLOT_0)))
                .WillByDefault(Return(m_strDomain));

        ON_CALL(m_objUtilService.GetMockPrivateProperty(),
                GetPersistent(
                        Eq(ImsPrivateProperties::Persistent::KEY_CONFIG_IMPI), Eq(IMS_SLOT_0)))
                .WillByDefault(Return(m_strImpi));

        ON_CALL(m_objUtilService.GetMockPrivateProperty(),
                GetPersistent(
                        Eq(ImsPrivateProperties::Persistent::KEY_CONFIG_IMPU_LIST), Eq(IMS_SLOT_0)))
                .WillByDefault(Return(m_objImpu.ToString()));

        ON_CALL(m_objUtilService.GetMockPrivateProperty(),
                GetPersistent(Eq(ImsPrivateProperties::Persistent::KEY_CONFIG_PCSCF_ADDRESS_LIST),
                        Eq(IMS_SLOT_0)))
                .WillByDefault(Return(m_objPcscfs.ToString()));

        EXPECT_CALL(m_objUtilService.GetMockPrivateProperty(), SetPersistentBoolean(_, _, _))
                .Times(AnyNumber());
    }

    void StartSubscriberConfig()
    {
        IAsyncConfig* piAsyncConfig = static_cast<IAsyncConfig*>(m_pSubscriberConfig);
        piAsyncConfig->HandleMessage(IAsyncConfig::ACMSG_START, 0, 0);
    }
};

TEST_F(SubscriberConfigTest, Init)
{
    EXPECT_CALL(m_objConfigService.GetMockCarrierConfig(), AddListener(_)).Times(1);
    EXPECT_TRUE(m_pSubscriberConfig->Init());

    ASSERT_TRUE(m_pSubscriberConfig->GetSubscriberInfo() != nullptr);
    EXPECT_TRUE(m_pSubscriberConfig->IsAkaSupported());
    EXPECT_TRUE(m_pSubscriberConfig->IsIsimSupported());
    EXPECT_EQ(m_pSubscriberConfig->GetHomeDomainName(), AString::ConstNull());
    EXPECT_EQ(m_pSubscriberConfig->GetPhoneContext(), m_strDomain);
    EXPECT_EQ(m_pSubscriberConfig->GetIndexOfPrimaryPublicUserId(), 1);
    ASSERT_TRUE(m_pSubscriberConfig->GetPrivateUserId().IsNull());
    ASSERT_TRUE(m_pSubscriberConfig->GetPublicUserId().IsNull());
    const AStringArray& objPublicUserIds = m_pSubscriberConfig->GetPublicUserIds();
    EXPECT_EQ(objPublicUserIds.GetCount(), 0);
}

TEST_F(SubscriberConfigTest, UpdateIsimRecords)
{
    EXPECT_TRUE(m_pSubscriberConfig->Init());
    // Two public user identities.
    EXPECT_CALL(m_objSubInfoListener, SubscriberInfo_UpdateImpu(_, _, _, _)).Times(2);
    SetUpIsimInit();
    SetUpIsimRecords(1);

    StartSubscriberConfig();
    m_piIsimListener->Isim_OnStateChanged(IIsim::STATE_LOADED);

    EXPECT_EQ(m_strImpi, m_pSubscriberConfig->GetPrivateUserId());
    EXPECT_EQ(m_strDomain, m_pSubscriberConfig->GetHomeDomainName());
    EXPECT_EQ(m_objImpu.GetElementAt(0),
            m_pSubscriberConfig->GetPublicUserId(IImsSubscriberInfo::IMPU_SIP));
    EXPECT_EQ(m_objImpu.GetElementAt(1),
            m_pSubscriberConfig->GetPublicUserId(IImsSubscriberInfo::IMPU_TEL));
}

TEST_F(SubscriberConfigTest, IsimRefresh)
{
    EXPECT_TRUE(m_pSubscriberConfig->Init());
    EXPECT_CALL(m_objSubscriberConfigListener, SubscriberConfig_RefreshStarted()).Times(1);
    EXPECT_CALL(m_objSubscriberConfigListener, SubscriberConfig_RefreshCompleted()).Times(1);
    // Two public user identities and ISIM state changed twice.
    EXPECT_CALL(m_objSubInfoListener, SubscriberInfo_UpdateImpu(_, _, _, _)).Times(4);
    SetUpIsimInit();
    SetUpIsimRecords(2);

    StartSubscriberConfig();
    m_piIsimListener->Isim_OnStateChanged(IIsim::STATE_LOADED);
    m_piIsimListener->Isim_OnStateChanged(IIsim::STATE_REFRESHING);
    m_piIsimListener->Isim_OnStateChanged(IIsim::STATE_LOADED);
}

TEST_F(SubscriberConfigTest, RefreshWhenIsimEnabled)
{
    EXPECT_CALL(m_objSubscriberConfigListener, SubscriberConfig_InitCompleted()).Times(1);
    // SUBSCRIBER_INFO_REMOVE_ALL and count of IMPU.
    EXPECT_CALL(m_objSubInfoListener, SubscriberInfo_UpdateImpu(_, _, _, _)).Times(3);
    EXPECT_CALL(m_objIsim, IsLoadCompleted()).Times(1).WillOnce(Return(IMS_TRUE));
    SetUpIsimInit();
    SetUpIsimRecords(1);

    StartSubscriberConfig();
    m_pSubscriberConfig->Refresh();

    EXPECT_EQ(m_pSubscriberConfig->GetSubscriberCount(), 1);
    ASSERT_TRUE(m_pSubscriberConfig->GetSubscriberInfoEx(0) != nullptr);
    EXPECT_EQ(m_pSubscriberConfig->GetHomeDomainName(), m_strDomain);
    EXPECT_EQ(m_pSubscriberConfig->GetPrivateUserId(), m_strImpi);
    EXPECT_EQ(m_pSubscriberConfig->GetPublicUserId(), m_objImpu.GetElementAt(m_nImpuRefIndex));
    EXPECT_EQ(m_pSubscriberConfig->GetPublicUserIds().GetElementAt(0), m_objImpu.GetElementAt(0));
    EXPECT_EQ(m_pSubscriberConfig->GetPublicUserIds().GetElementAt(1), m_objImpu.GetElementAt(1));
    EXPECT_EQ(m_pSubscriberConfig->GetScscfAddress(), m_strDomain);
    EXPECT_TRUE(m_pSubscriberConfig->IsAuthRealmLenient());
}

TEST_F(SubscriberConfigTest, RefreshWhenIsimDisabled)
{
    ImsVector<IMS_SINT32> objImsIdentity;
    objImsIdentity.Add(CarrierConfig::Ims::IMS_IDENTITY_PRIORITY_USIM);
    EXPECT_CALL(m_objConfigService.GetMockCarrierConfig(),
            GetIntArray(Eq(CarrierConfig::Ims::KEY_IMS_IDENTITY_PRIORITY_INT_ARRAY)))
            .Times(1)
            .WillOnce(Return(objImsIdentity));
    EXPECT_CALL(m_objSubscriberConfigListener, SubscriberConfig_InitCompleted()).Times(1);
    // SUBSCRIBER_INFO_REMOVE_ALL and count of IMPU.
    EXPECT_CALL(m_objSubInfoListener, SubscriberInfo_UpdateImpu(_, _, _, _)).Times(3);

    m_pSubscriberConfig->Refresh();

    EXPECT_EQ(m_pSubscriberConfig->GetSubscriberCount(), 1);
    ASSERT_TRUE(m_pSubscriberConfig->GetSubscriberInfoEx(0) != nullptr);
    EXPECT_EQ(m_pSubscriberConfig->GetHomeDomainName(), m_strDomain);
    EXPECT_EQ(m_pSubscriberConfig->GetPrivateUserId(), m_strImpi);
    EXPECT_EQ(m_pSubscriberConfig->GetPublicUserId(), m_objImpu.GetElementAt(0));
    EXPECT_EQ(m_pSubscriberConfig->GetPublicUserIds().GetElementAt(0), m_objImpu.GetElementAt(0));
    EXPECT_EQ(m_pSubscriberConfig->GetPublicUserIds().GetElementAt(1), m_objImpu.GetElementAt(1));
    EXPECT_EQ(m_pSubscriberConfig->GetScscfAddress(), m_strDomain);
    EXPECT_TRUE(m_pSubscriberConfig->IsAuthRealmLenient());
}

TEST_F(SubscriberConfigTest, Update)
{
    ISubscriberConfig* piSubConfig = static_cast<ISubscriberConfig*>(m_pSubscriberConfig);
    IConfigurable* piConfigurable = piSubConfig->GetConfigurable();

    // SubscriberInfo is null
    EXPECT_TRUE(piConfigurable->Update(IConfigurable::CP_I_START_SUBSCRIBER));
    EXPECT_TRUE(piConfigurable->Update(
            IConfigurable::CP_I_SUBSCRIPTION_ATTRIBUTE_USIM, AString("true")));
    EXPECT_TRUE(piConfigurable->Update(
            IConfigurable::CP_I_SUBSCRIPTION_ATTRIBUTE_ISIM, AString("false")));
    EXPECT_FALSE(piConfigurable->Update(IConfigurable::CP_I_HOME_DOMAIN_NAME));
    EXPECT_FALSE(piConfigurable->Update(IConfigurable::CP_I_IMPI));
    EXPECT_FALSE(piConfigurable->Update(IConfigurable::CP_I_IMPU_PRIMARY_REF_INDEX));
    EXPECT_FALSE(piConfigurable->Update(IConfigurable::CP_I_IMPU_1));
    EXPECT_FALSE(piConfigurable->Update(IConfigurable::CP_I_PHONE_CONTEXT));
    EXPECT_FALSE(piConfigurable->Update(IConfigurable::CP_I_AUTH_USERNAME));
    EXPECT_FALSE(piConfigurable->Update(IConfigurable::CP_I_AUTH_PASSWORD));
    EXPECT_FALSE(piConfigurable->Update(IConfigurable::CP_I_AUTH_REALM));
    EXPECT_FALSE(piConfigurable->Update(IConfigurable::CP_I_AUTH_ALGORITHM));
    EXPECT_FALSE(piConfigurable->Update(IConfigurable::CP_I_SERVER_SCSCF));
    EXPECT_TRUE(
            piConfigurable->Update(IConfigurable::CP_I_PCSCF_DISCOVERY_METHODS, AString("conf")));
    EXPECT_FALSE(piConfigurable->Update(IConfigurable::CP_I_PCSCF_ADDRESS_3));

    EXPECT_TRUE(piConfigurable->Update(IConfigurable::CP_I_SUBSCRIPTION_ATTRIBUTE_ALL));
    EXPECT_TRUE(piConfigurable->Update(IConfigurable::CP_I_SUBSCRIPTION_ATTRIBUTE_ISIM));
    EXPECT_TRUE(piConfigurable->Update(
            IConfigurable::CP_I_SUBSCRIPTION_ATTRIBUTE_ISIM, AString("true")));
    EXPECT_TRUE(piConfigurable->Update(
            IConfigurable::CP_I_SUBSCRIPTION_ATTRIBUTE_USIM, AString("true")));
    EXPECT_TRUE(piConfigurable->Update(IConfigurable::CP_I_SUBSCRIPTION_ATTRIBUTE_USIM));

    ASSERT_TRUE(m_pSubscriberConfig->GetSubscriberInfo() == nullptr);
    EXPECT_TRUE(m_pSubscriberConfig->GetHomeDomainName().IsNull());
    EXPECT_TRUE(m_pSubscriberConfig->GetPhoneContext().IsNull());
    EXPECT_EQ(m_pSubscriberConfig->GetIndexOfPrimaryPublicUserId(), -1);
    ASSERT_TRUE(m_pSubscriberConfig->GetPrivateUserId().IsNull());
    ASSERT_TRUE(m_pSubscriberConfig->GetPublicUserId().IsNull());
    AStringArray objPublicUserIds = m_pSubscriberConfig->GetPublicUserIds();
    EXPECT_EQ(objPublicUserIds.GetCount(), 0);
    EXPECT_TRUE(m_pSubscriberConfig->GetScscfAddress().IsNull());
    EXPECT_FALSE(m_pSubscriberConfig->IsAuthRealmLenient());

    // ISIM Info
    EXPECT_TRUE(m_pSubscriberConfig->IsIsimSupported());
    // Read From Persistent property
    EXPECT_TRUE(m_pSubscriberConfig->Load());

    EXPECT_TRUE(piConfigurable->Update(
            IConfigurable::CP_I_SUBSCRIPTION_ATTRIBUTE_ISIM, AString("true")));
    EXPECT_FALSE(piConfigurable->Update(IConfigurable::CP_I_HOME_DOMAIN_NAME));
    EXPECT_FALSE(piConfigurable->Update(IConfigurable::CP_I_IMPI));
    EXPECT_FALSE(piConfigurable->Update(IConfigurable::CP_I_IMPU_1));
    EXPECT_FALSE(piConfigurable->Update(IConfigurable::CP_I_PHONE_CONTEXT));
    EXPECT_FALSE(piConfigurable->Update(IConfigurable::CP_I_AUTH_USERNAME));
    EXPECT_FALSE(piConfigurable->Update(IConfigurable::CP_I_AUTH_REALM));
    EXPECT_FALSE(piConfigurable->Update(IConfigurable::CP_I_AUTH_ALGORITHM));
    EXPECT_FALSE(piConfigurable->Update(IConfigurable::CP_I_SERVER_SCSCF));

    // USIM info
    ImsVector<IMS_SINT32> objImsIdentity;
    objImsIdentity.Add(CarrierConfig::Ims::IMS_IDENTITY_PRIORITY_USIM);

    EXPECT_CALL(m_objConfigService.GetMockCarrierConfig(),
            GetIntArray(Eq(CarrierConfig::Ims::KEY_IMS_IDENTITY_PRIORITY_INT_ARRAY)))
            .Times(AnyNumber())
            .WillRepeatedly(Return(objImsIdentity));

    EXPECT_CALL(m_objSubInfoListener, SubscriberInfo_UpdateImpu(_, _, _, _)).Times(5);

    EXPECT_TRUE(piConfigurable->Update(
            IConfigurable::CP_I_SUBSCRIPTION_ATTRIBUTE_ISIM, AString("false")));
    EXPECT_TRUE(piConfigurable->Update(
            IConfigurable::CP_I_SUBSCRIPTION_ATTRIBUTE_USIM, AString("true")));
    EXPECT_TRUE(m_pSubscriberConfig->Load());
    EXPECT_FALSE(m_pSubscriberConfig->IsIsimSupported());
    EXPECT_TRUE(m_pSubscriberConfig->IsUsimSupported());

    EXPECT_TRUE(piConfigurable->Update(IConfigurable::CP_I_HOME_DOMAIN_NAME));
    EXPECT_TRUE(piConfigurable->Update(IConfigurable::CP_I_HOME_DOMAIN_NAME, m_strDomain));

    EXPECT_TRUE(piConfigurable->Update(IConfigurable::CP_I_IMPI));
    EXPECT_TRUE(piConfigurable->Update(IConfigurable::CP_I_IMPI, m_strImpi));

    EXPECT_TRUE(piConfigurable->Update(IConfigurable::CP_I_IMPU_PRIMARY_REF_INDEX));
    EXPECT_TRUE(piConfigurable->Update(IConfigurable::CP_I_IMPU_PRIMARY_REF_INDEX, AString("1")));

    EXPECT_TRUE(piConfigurable->Update(IConfigurable::CP_I_IMPU_0));
    EXPECT_FALSE(piConfigurable->Update(IConfigurable::CP_I_IMPU_9));
    EXPECT_TRUE(piConfigurable->Update(IConfigurable::CP_I_IMPU_0, m_objImpu.GetElementAt(0)));

    EXPECT_TRUE(piConfigurable->Update(IConfigurable::CP_I_PHONE_CONTEXT));
    EXPECT_TRUE(piConfigurable->Update(IConfigurable::CP_I_PHONE_CONTEXT, m_strDomain));

    EXPECT_TRUE(piConfigurable->Update(IConfigurable::CP_I_AUTH_USERNAME));
    EXPECT_TRUE(
            piConfigurable->Update(IConfigurable::CP_I_AUTH_USERNAME, AString("auth-username")));

    EXPECT_TRUE(piConfigurable->Update(IConfigurable::CP_I_AUTH_PASSWORD));
    EXPECT_TRUE(piConfigurable->Update(IConfigurable::CP_I_AUTH_PASSWORD, AString("password")));

    EXPECT_TRUE(piConfigurable->Update(IConfigurable::CP_I_AUTH_REALM));
    EXPECT_TRUE(piConfigurable->Update(IConfigurable::CP_I_AUTH_REALM, AString("auth-realm")));

    EXPECT_TRUE(piConfigurable->Update(IConfigurable::CP_I_AUTH_ALGORITHM));
    EXPECT_TRUE(piConfigurable->Update(IConfigurable::CP_I_AUTH_ALGORITHM, AString("AKAv1-MD5")));
    EXPECT_TRUE(m_pSubscriberConfig->IsAkaSupported());
    EXPECT_TRUE(piConfigurable->Update(IConfigurable::CP_I_AUTH_ALGORITHM, AString("AKAv2-MD5")));
    EXPECT_TRUE(m_pSubscriberConfig->IsAkaSupported());
    EXPECT_TRUE(piConfigurable->Update(IConfigurable::CP_I_AUTH_ALGORITHM, AString("auth")));
    EXPECT_FALSE(m_pSubscriberConfig->IsAkaSupported());

    EXPECT_TRUE(piConfigurable->Update(IConfigurable::CP_I_SERVER_SCSCF));
    EXPECT_TRUE(piConfigurable->Update(IConfigurable::CP_I_SERVER_SCSCF, m_strDomain));

    EXPECT_TRUE(piConfigurable->Update(IConfigurable::CP_I_PCSCF_DISCOVERY_METHODS));
    EXPECT_TRUE(
            piConfigurable->Update(IConfigurable::CP_I_PCSCF_DISCOVERY_METHODS, AString("conf")));

    EXPECT_TRUE(piConfigurable->Update(IConfigurable::CP_I_PCSCF_ADDRESS_0));
    EXPECT_FALSE(piConfigurable->Update(IConfigurable::CP_I_PCSCF_ADDRESS_6));
    EXPECT_TRUE(piConfigurable->Update(IConfigurable::CP_I_PCSCF_ADDRESS_1, AString("pcscf.com")));

    EXPECT_TRUE(piConfigurable->Update(IConfigurable::CP_I_PCSCF_PORT_0));
    EXPECT_FALSE(piConfigurable->Update(IConfigurable::CP_I_PCSCF_PORT_7));
    EXPECT_TRUE(piConfigurable->Update(IConfigurable::CP_I_PCSCF_PORT_1, AString("4465")));
    EXPECT_TRUE(piConfigurable->Update(IConfigurable::CP_I_PCSCF_ALL));

    EXPECT_CALL(m_objSubscriberConfigListener, SubscriberConfig_RefreshStarted()).Times(1);
    EXPECT_CALL(m_objSubscriberConfigListener, SubscriberConfig_RefreshCompleted()).Times(1);
    EXPECT_TRUE(piConfigurable->Update(IConfigurable::CP_I_SUBSCRIBER_ALL));

    EXPECT_CALL(m_objUtilService.GetMockPrivateProperty(), SetPersistent(_, _, _))
            .Times(AnyNumber());
    EXPECT_TRUE(piConfigurable->Update(IConfigurable::CP_I_WRITE_PROVISIONING_SUBSCRIBER));
    EXPECT_TRUE(m_pSubscriberConfig->IsProvisioningDone());
    EXPECT_FALSE(m_pSubscriberConfig->IsTestMode());
    EXPECT_TRUE(piConfigurable->Update(IConfigurable::CP_I_END_SUBSCRIBER));
    EXPECT_FALSE(piConfigurable->Update(IConfigurable::CP_I_MAX));
}

TEST_F(SubscriberConfigTest, CarrierConfigChanged)
{
    ImsVector<IMS_SINT32> objImsIdentityUsim;
    objImsIdentityUsim.Add(CarrierConfig::Ims::IMS_IDENTITY_PRIORITY_USIM);
    ImsVector<IMS_SINT32> objImsIdentityIsim;
    objImsIdentityIsim.Add(CarrierConfig::Ims::IMS_IDENTITY_PRIORITY_ISIM);
    EXPECT_CALL(m_objConfigService.GetMockCarrierConfig(),
            GetIntArray(Eq(CarrierConfig::Ims::KEY_IMS_IDENTITY_PRIORITY_INT_ARRAY)))
            .WillOnce(Return(objImsIdentityIsim))   // Init
            .WillOnce(Return(objImsIdentityUsim))   // CarrierConfig_NotifyConfigChanged
            .WillOnce(Return(objImsIdentityUsim))   // UpdateAllConfigs
            .WillOnce(Return(objImsIdentityIsim))   // CarrierConfig_NotifyConfigChanged
            .WillOnce(Return(objImsIdentityIsim));  // UpdateAllConfigs
    EXPECT_TRUE(m_pSubscriberConfig->Init());
    EXPECT_CALL(m_objSubscriberConfigListener, SubscriberConfig_RefreshStarted()).Times(2);
    EXPECT_CALL(m_objSubscriberConfigListener, SubscriberConfig_RefreshCompleted()).Times(2);
    EXPECT_CALL(m_objIsim, IsLoadCompleted()).Times(1).WillOnce(Return(IMS_TRUE));
    SetUpIsimInit();
    SetUpIsimRecords(2);

    StartSubscriberConfig();
    m_piIsimListener->Isim_OnStateChanged(IIsim::STATE_LOADED);

    ICarrierConfigListener* piCarrierConfigListener =
            static_cast<ICarrierConfigListener*>(m_pSubscriberConfig);
    // SIM slot mismatched.
    piCarrierConfigListener->CarrierConfig_NotifyConfigChanged(IMS_SLOT_1);

    // ISIM disabled and carrier configuration changed.
    piCarrierConfigListener->CarrierConfig_NotifyConfigChanged(IMS_SLOT_0);

    EXPECT_FALSE(m_pSubscriberConfig->IsIsimSupported());
    EXPECT_TRUE(m_pSubscriberConfig->IsProvisioningDone());

    // ISIM enabled and carrier configuration changed.
    piCarrierConfigListener->CarrierConfig_NotifyConfigChanged(IMS_SLOT_0);

    EXPECT_TRUE(m_pSubscriberConfig->IsIsimSupported());
    EXPECT_TRUE(m_pSubscriberConfig->IsProvisioningDone());
}

TEST_F(SubscriberConfigTest, FakeSubscriber)
{
    SubscriberConfig objSubscriberConfig(IMS_SLOT_0, "subscriber_fake");

    ASSERT_TRUE(objSubscriberConfig.GetSubscriberInfo(1) == nullptr);
    ASSERT_TRUE(objSubscriberConfig.GetSubscriberInfo() == nullptr);

    EXPECT_TRUE(objSubscriberConfig.Init());
}

}  // namespace android
