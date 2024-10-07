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
#include <gtest/gtest.h>

#include "PlatformContext.h"

#include "ImsIdentity.h"
#include "ServerAddress.h"
#include "private/SubscriberConfig.h"

#include "MockISubscriberConfigListener.h"
#include "MockISubscriberInfoListener.h"
#include "TestConfigService.h"
#include "TestPhoneInfoService.h"
#include "TestUtilService.h"

using ::testing::_;
using ::testing::Eq;
using ::testing::Return;

namespace android
{

class SubscriberConfigTest : public ::testing::Test
{
public:
    inline SubscriberConfigTest() :
            m_nSipServerPort(5080),
            m_strConfName("subscriber"),
            m_nImpuRefIndex(0),
            m_strDomain("ims.mnc001.mcc001.3gppnetwork.org"),
            m_strImpi("1111@ims.mnc001.mcc001.3gppnetwork.org"),
            m_piIsimListener(IMS_NULL),
            m_pConfigBase(IMS_NULL),
            m_pSubscriberConfig(IMS_NULL)
    {
        m_objImsIdentityPriority.Add(CarrierConfig::Ims::IMS_IDENTITY_PRIORITY_ISIM);
        m_objPcscfDiscoveryMethods.Add(CarrierConfig::Ims::PCSCF_DISCOVERY_METHOD_PCO);

        m_objImpu.AddElement("sip:1111@ims.mnc001.mcc001.3gppnetwork.org");
        m_objImpu.AddElement("tel:1111");

        m_objPcscfs.AddElement("192.168.0.1");
        m_objPcscfs.AddElement("192.168.0.2");
    }

protected:
    TestConfigService m_objConfigService;
    TestPhoneInfoService m_objPhoneInfoService;
    TestUtilService m_objUtilService;
    MockISubscriberConfigListener m_objDefaultSubsConfigListener;
    MockISubscriberConfigListener m_objIsimProvisioningListener;
    MockISubscriberConfigListener m_objManualProvisioningListener;
    MockISubscriberInfoListener m_objSubsInfoListener;

    ImsVector<IMS_SINT32> m_objImsIdentityPriority;
    ImsVector<IMS_SINT32> m_objPcscfDiscoveryMethods;
    IMS_SINT32 m_nSipServerPort;
    AString m_strConfName;
    IMS_SINT32 m_nImpuRefIndex;
    AString m_strDomain;
    AString m_strImpi;
    AStringArray m_objImpu;
    AStringArray m_objPcscfs;
    IIsimListener* m_piIsimListener;
    ConfigBase* m_pConfigBase;
    SubscriberConfig* m_pSubscriberConfig;

protected:
    virtual void SetUp() override
    {
        PlatformContext::GetInstance()->SetService(
                PlatformContext::SERVICE_CONFIG, &m_objConfigService);
        PlatformContext::GetInstance()->SetService(
                PlatformContext::SERVICE_PHONE_INFO, &m_objPhoneInfoService);
        PlatformContext::GetInstance()->SetService(
                PlatformContext::SERVICE_UTIL, &m_objUtilService);

        SetUpPersistentProperties(IMS_SLOT_0);
        SetUpCarrierConfig();

        m_pSubscriberConfig = new SubscriberConfig(IMS_SLOT_0, m_strConfName);

        m_pSubscriberConfig->SetListener(&m_objDefaultSubsConfigListener);
        m_pSubscriberConfig->SetListener(
                &m_objIsimProvisioningListener, ISubscriberConfig::LISTEN_EVENT_ISIM_PROVISIONING);
        m_pSubscriberConfig->SetListener(&m_objManualProvisioningListener,
                ISubscriberConfig::LISTEN_EVENT_MANUAL_PROVISIONING);
        m_pSubscriberConfig->SetSubscriberInfoListener(&m_objSubsInfoListener);

        m_piIsimListener = static_cast<IIsimListener*>(m_pSubscriberConfig);
        m_pConfigBase = static_cast<ConfigBase*>(m_pSubscriberConfig);
    }

    virtual void TearDown() override
    {
        if (m_pSubscriberConfig != IMS_NULL)
        {
            m_pSubscriberConfig->RemoveListener(&m_objDefaultSubsConfigListener);
            m_pSubscriberConfig->RemoveListener(&m_objIsimProvisioningListener);
            m_pSubscriberConfig->RemoveListener(&m_objManualProvisioningListener);

            EXPECT_CALL(m_objConfigService.GetMockCarrierConfig(), RemoveListener(_)).Times(1);
            delete m_pSubscriberConfig;
            m_pSubscriberConfig = IMS_NULL;
        }
        PlatformContext::GetInstance()->SetService(PlatformContext::SERVICE_UTIL, IMS_NULL);
        PlatformContext::GetInstance()->SetService(PlatformContext::SERVICE_PHONE_INFO, IMS_NULL);
        PlatformContext::GetInstance()->SetService(PlatformContext::SERVICE_CONFIG, IMS_NULL);
    }

    void VerifyIsimInit()
    {
        MockIIsim& objIsim = m_objPhoneInfoService.GetMockIsim();
        EXPECT_CALL(objIsim, Init()).Times(1);
        EXPECT_CALL(objIsim, AddListener(_)).Times(1);
    }

    void VerifyIsimReadRecords(IMS_SINT32 nCount)
    {
        MockIIsim& objIsim = m_objPhoneInfoService.GetMockIsim();
        EXPECT_CALL(objIsim, GetImpi()).Times(nCount).WillRepeatedly(Return(m_strImpi));
        EXPECT_CALL(objIsim, GetImpu()).Times(nCount).WillRepeatedly(Return(m_objImpu));
        EXPECT_CALL(objIsim, GetHomeDomainName()).Times(nCount).WillRepeatedly(Return(m_strDomain));
    }

    void SetUpCarrierConfig()
    {
        MockICarrierConfig& objCc = m_objConfigService.GetMockCarrierConfig();

        ON_CALL(objCc, GetString(Eq(CarrierConfig::Ims::KEY_PHONE_CONTEXT_DOMAIN_NAME_STRING), _))
                .WillByDefault(Return(m_strDomain));

        ON_CALL(objCc, GetInt(Eq(CarrierConfig::Ims::KEY_SIP_SERVER_PORT_NUMBER_INT), _))
                .WillByDefault(Return(m_nSipServerPort));

        ON_CALL(objCc, GetInt(Eq(CarrierConfig::Ims::KEY_ISIM_INDEX_FOR_IMPU_INT), _))
                .WillByDefault(Return(m_nImpuRefIndex));

        ON_CALL(objCc, GetIntArray(Eq(CarrierConfig::Ims::KEY_IMS_IDENTITY_PRIORITY_INT_ARRAY)))
                .WillByDefault(Return(m_objImsIdentityPriority));

        ON_CALL(objCc, GetIntArray(Eq(CarrierConfig::Ims::KEY_PCSCF_DISCOVERY_METHOD_INT_ARRAY)))
                .WillByDefault(Return(m_objPcscfDiscoveryMethods));
    }

    void SetUpPersistentProperties(IMS_SINT32 nSlotId)
    {
        MockIImsPrivateProperty& objIpp = m_objUtilService.GetMockPrivateProperty();

        ON_CALL(objIpp,
                GetPersistent(Eq(ImsPrivateProperties::Persistent::KEY_CONFIG_HOME_DOMAIN_NAME),
                        Eq(nSlotId)))
                .WillByDefault(Return(m_strDomain));

        ON_CALL(objIpp,
                GetPersistent(Eq(ImsPrivateProperties::Persistent::KEY_CONFIG_IMPI), Eq(nSlotId)))
                .WillByDefault(Return(m_strImpi));

        ON_CALL(objIpp,
                GetPersistent(
                        Eq(ImsPrivateProperties::Persistent::KEY_CONFIG_IMPU_LIST), Eq(nSlotId)))
                .WillByDefault(Return(m_objImpu.ToString()));

        ON_CALL(objIpp,
                GetPersistent(Eq(ImsPrivateProperties::Persistent::KEY_CONFIG_PCSCF_ADDRESS_LIST),
                        Eq(nSlotId)))
                .WillByDefault(Return(m_objPcscfs.ToString()));
    }

    void VerifyStoreSubscriptionAttributes(IMS_SINT32 nCount)
    {
        IMS_SINT32 nSlotId = m_pSubscriberConfig->GetSlotId();
        MockIImsPrivateProperty& objIpp = m_objUtilService.GetMockPrivateProperty();

        EXPECT_CALL(objIpp,
                SetPersistentBoolean(
                        Eq(ImsPrivateProperties::Persistent::KEY_ISIM_ENABLED), _, Eq(nSlotId)))
                .Times(nCount);
        EXPECT_CALL(objIpp,
                SetPersistentBoolean(
                        Eq(ImsPrivateProperties::Persistent::KEY_USIM_ENABLED), _, Eq(nSlotId)))
                .Times(nCount);
    }

    void VerifyStoreSubscriberInfo(IMS_SINT32 nCount)
    {
        IMS_SINT32 nSlotId = m_pSubscriberConfig->GetSlotId();
        MockIImsPrivateProperty& objIpp = m_objUtilService.GetMockPrivateProperty();

        EXPECT_CALL(objIpp,
                SetPersistent(Eq(ImsPrivateProperties::Persistent::KEY_CONFIG_HOME_DOMAIN_NAME), _,
                        Eq(nSlotId)))
                .Times(nCount);
        EXPECT_CALL(objIpp,
                SetPersistent(
                        Eq(ImsPrivateProperties::Persistent::KEY_CONFIG_IMPI), _, Eq(nSlotId)))
                .Times(nCount);
        EXPECT_CALL(objIpp,
                SetPersistent(
                        Eq(ImsPrivateProperties::Persistent::KEY_CONFIG_IMPU_LIST), _, Eq(nSlotId)))
                .Times(nCount);
        EXPECT_CALL(objIpp,
                SetPersistent(
                        Eq(ImsPrivateProperties::Persistent::KEY_PRIMARY_IMPU), _, Eq(nSlotId)))
                .Times(nCount);
    }

    void VerifySubscriberInfo()
    {
        EXPECT_TRUE(m_pSubscriberConfig->IsProvisioningDone());
        EXPECT_EQ(m_pSubscriberConfig->GetCredential().GetRealm(), m_strDomain);
        EXPECT_EQ(m_pSubscriberConfig->GetCredential().GetUsername(), m_strImpi);
        EXPECT_EQ(m_pSubscriberConfig->GetHomeDomainName(), m_strDomain);
        EXPECT_EQ(m_pSubscriberConfig->GetIndexOfPrimaryPublicUserId(), m_nImpuRefIndex);
        EXPECT_EQ(m_pSubscriberConfig->GetPhoneContext(), m_strDomain);
        EXPECT_EQ(m_pSubscriberConfig->GetPrivateUserId(), m_strImpi);
        EXPECT_EQ(m_pSubscriberConfig->GetPublicUserId(), m_objImpu.GetElementAt(m_nImpuRefIndex));
        EXPECT_EQ(m_pSubscriberConfig->GetPublicUserIds().GetCount(), m_objImpu.GetCount());
        EXPECT_EQ(m_pSubscriberConfig->GetPublicUserId(IImsSubscriberInfo::IMPU_SIP),
                m_objImpu.GetElementAt(0));
        EXPECT_EQ(m_pSubscriberConfig->GetPublicUserId(IImsSubscriberInfo::IMPU_TEL),
                m_objImpu.GetElementAt(1));
        EXPECT_EQ(m_pSubscriberConfig->GetScscfAddress(), m_strDomain);
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

    IMS_BOOL bResult = m_pSubscriberConfig->Init();
    ASSERT_TRUE(bResult);

    ServerAddress* pServerAddress = m_pSubscriberConfig->GetPcscfAddress();
    ASSERT_NE(pServerAddress, nullptr);
    EXPECT_EQ(pServerAddress->GetAddress(), m_objPcscfs.GetElementAt(0));
    EXPECT_EQ(pServerAddress->GetPort(), m_nSipServerPort);

    EXPECT_EQ(m_pSubscriberConfig->GetPcscfAddresses().GetSize(), m_objPcscfs.GetCount());
    EXPECT_EQ(m_pSubscriberConfig->GetPcscfDiscoveryMethod(),
            ISubscriberConfig::PCSCF_DISCOVERY_METHOD_PCO);
    EXPECT_EQ(m_pSubscriberConfig->GetPcscfDiscoveryMethods().GetSize(), 1);
    EXPECT_EQ(m_pSubscriberConfig->GetSubscriberCount(), 1);
    ASSERT_NE(m_pSubscriberConfig->GetSubscriberInfo(), nullptr);
    EXPECT_TRUE(m_pSubscriberConfig->IsAkaSupported());
    EXPECT_FALSE(m_pSubscriberConfig->IsDebugOn());
    EXPECT_TRUE(m_pSubscriberConfig->IsServiceAllowed());
    EXPECT_TRUE(m_pSubscriberConfig->IsIsimSupported());
    EXPECT_FALSE(m_pSubscriberConfig->IsProvisioningDone());
    EXPECT_FALSE(m_pSubscriberConfig->IsUsimSupported());
    EXPECT_FALSE(m_pSubscriberConfig->IsTestMode());
    EXPECT_EQ(m_pSubscriberConfig->GetSubscriptionAttributes(),
            ISubscriberConfig::SUBSCRIPTION_ATTRIBUTE_IMS |
                    ISubscriberConfig::SUBSCRIPTION_ATTRIBUTE_ISIM);
    EXPECT_NE(m_pSubscriberConfig->GetConfigurable(), nullptr);
    EXPECT_TRUE(m_pSubscriberConfig->GetCredential().GetRealm().IsNull());
    EXPECT_TRUE(m_pSubscriberConfig->GetCredential().GetUsername().IsNull());
    EXPECT_TRUE(m_pSubscriberConfig->GetHomeDomainName().IsNull());
    EXPECT_EQ(m_pSubscriberConfig->GetIndexOfPrimaryPublicUserId(), m_nImpuRefIndex);
    EXPECT_EQ(m_pSubscriberConfig->GetPhoneContext(), m_strDomain);
    EXPECT_TRUE(m_pSubscriberConfig->GetPrivateUserId().IsNull());
    EXPECT_TRUE(m_pSubscriberConfig->GetPublicUserId().IsNull());
    EXPECT_TRUE(m_pSubscriberConfig->GetPublicUserIds().IsEmpty());
    EXPECT_TRUE(m_pSubscriberConfig->GetScscfAddress().IsNull());
    EXPECT_TRUE(m_pSubscriberConfig->IsAuthRealmLenient());
    EXPECT_NE(m_pSubscriberConfig->GetSubscriberInfoEx(), nullptr);
    EXPECT_EQ(m_pSubscriberConfig->GetConfName(), m_strConfName);
    EXPECT_EQ(m_pSubscriberConfig->GetId(), SubscriberConfig::GetDefaultId());
    EXPECT_TRUE(m_pSubscriberConfig->IsDefaultConfig());
}

TEST_F(SubscriberConfigTest, InitWhenUsimEnabled)
{
    m_objImsIdentityPriority.Clear();
    m_objImsIdentityPriority.Add(CarrierConfig::Ims::IMS_IDENTITY_PRIORITY_USIM);
    m_objPcscfDiscoveryMethods.Clear();
    m_objPcscfDiscoveryMethods.Add(CarrierConfig::Ims::PCSCF_DISCOVERY_METHOD_CONFIG);
    SetUpCarrierConfig();

    VerifyStoreSubscriptionAttributes(1);
    VerifyStoreSubscriberInfo(1);
    EXPECT_CALL(m_objConfigService.GetMockCarrierConfig(), AddListener(_)).Times(1);
    EXPECT_CALL(m_objSubsInfoListener, SubscriberInfo_UpdateImpu(_, _, _, _))
            .Times(m_objImpu.GetCount());

    IMS_BOOL bResult = m_pSubscriberConfig->Init();
    ASSERT_TRUE(bResult);

    ServerAddress* pServerAddress = m_pSubscriberConfig->GetPcscfAddress();
    ASSERT_NE(pServerAddress, nullptr);
    EXPECT_EQ(pServerAddress->GetAddress(), m_objPcscfs.GetElementAt(0));
    EXPECT_EQ(pServerAddress->GetPort(), m_nSipServerPort);

    EXPECT_EQ(m_pSubscriberConfig->GetPcscfAddresses().GetSize(), m_objPcscfs.GetCount());
    EXPECT_EQ(m_pSubscriberConfig->GetPcscfDiscoveryMethod(),
            ISubscriberConfig::PCSCF_DISCOVERY_METHOD_CONFIG);
    EXPECT_EQ(m_pSubscriberConfig->GetPcscfDiscoveryMethods().GetSize(), 1);
    EXPECT_EQ(m_pSubscriberConfig->GetSubscriberCount(), 1);
    ASSERT_NE(m_pSubscriberConfig->GetSubscriberInfo(), nullptr);
    EXPECT_TRUE(m_pSubscriberConfig->IsAkaSupported());
    EXPECT_FALSE(m_pSubscriberConfig->IsDebugOn());
    EXPECT_TRUE(m_pSubscriberConfig->IsServiceAllowed());
    EXPECT_FALSE(m_pSubscriberConfig->IsIsimSupported());
    EXPECT_TRUE(m_pSubscriberConfig->IsProvisioningDone());
    EXPECT_TRUE(m_pSubscriberConfig->IsUsimSupported());
    EXPECT_FALSE(m_pSubscriberConfig->IsTestMode());
    EXPECT_EQ(m_pSubscriberConfig->GetSubscriptionAttributes(),
            ISubscriberConfig::SUBSCRIPTION_ATTRIBUTE_IMS |
                    ISubscriberConfig::SUBSCRIPTION_ATTRIBUTE_USIM);
    EXPECT_NE(m_pSubscriberConfig->GetConfigurable(), nullptr);
    EXPECT_TRUE(m_pSubscriberConfig->IsAuthRealmLenient());
    EXPECT_NE(m_pSubscriberConfig->GetSubscriberInfoEx(), nullptr);
    EXPECT_EQ(m_pSubscriberConfig->GetConfName(), m_strConfName);
    EXPECT_EQ(m_pSubscriberConfig->GetId(), SubscriberConfig::GetDefaultId());
    EXPECT_TRUE(m_pSubscriberConfig->IsDefaultConfig());

    VerifySubscriberInfo();
}

TEST_F(SubscriberConfigTest, OnIsimLoaded)
{
    VerifyIsimInit();
    VerifyIsimReadRecords(1);
    VerifyStoreSubscriptionAttributes(2);
    VerifyStoreSubscriberInfo(1);
    EXPECT_CALL(m_objSubsInfoListener, SubscriberInfo_UpdateImpu(_, _, _, _))
            .Times(m_objImpu.GetCount());
    EXPECT_CALL(m_objDefaultSubsConfigListener, SubscriberConfig_InitCompleted()).Times(1);
    EXPECT_CALL(m_objIsimProvisioningListener, SubscriberConfig_InitCompleted()).Times(1);
    EXPECT_CALL(m_objManualProvisioningListener, SubscriberConfig_InitCompleted()).Times(0);

    IMS_BOOL bResult = m_pSubscriberConfig->Init();
    ASSERT_TRUE(bResult);

    StartSubscriberConfig();
    m_piIsimListener->Isim_OnStateChanged(IIsim::STATE_LOADED);

    VerifySubscriberInfo();
}

TEST_F(SubscriberConfigTest, OnIsimRefreshed)
{
    VerifyIsimInit();
    VerifyIsimReadRecords(2);
    VerifyStoreSubscriptionAttributes(3);
    VerifyStoreSubscriberInfo(2);
    EXPECT_CALL(m_objSubsInfoListener, SubscriberInfo_UpdateImpu(_, _, _, _))
            .Times(m_objImpu.GetCount() * 2);
    EXPECT_CALL(m_objDefaultSubsConfigListener, SubscriberConfig_InitCompleted()).Times(1);
    EXPECT_CALL(m_objIsimProvisioningListener, SubscriberConfig_InitCompleted()).Times(1);
    EXPECT_CALL(m_objManualProvisioningListener, SubscriberConfig_InitCompleted()).Times(0);
    EXPECT_CALL(m_objDefaultSubsConfigListener, SubscriberConfig_RefreshStarted()).Times(1);
    EXPECT_CALL(m_objIsimProvisioningListener, SubscriberConfig_RefreshStarted()).Times(1);
    EXPECT_CALL(m_objDefaultSubsConfigListener, SubscriberConfig_RefreshCompleted()).Times(1);
    EXPECT_CALL(m_objIsimProvisioningListener, SubscriberConfig_RefreshCompleted()).Times(1);

    IMS_BOOL bResult = m_pSubscriberConfig->Init();
    ASSERT_TRUE(bResult);

    StartSubscriberConfig();
    m_piIsimListener->Isim_OnStateChanged(IIsim::STATE_LOADED);
    m_piIsimListener->Isim_OnStateChanged(IIsim::STATE_REFRESHING);
    m_piIsimListener->Isim_OnStateChanged(IIsim::STATE_LOADED);

    VerifySubscriberInfo();
}

TEST_F(SubscriberConfigTest, OnIsimNotPresent)
{
    VerifyIsimInit();
    VerifyIsimReadRecords(0);
    EXPECT_CALL(m_objDefaultSubsConfigListener,
            SubscriberConfig_NotifyError(Eq(ISubscriberConfig::ERROR_NO_ISIM_APPLICATION)))
            .Times(1);
    EXPECT_CALL(m_objIsimProvisioningListener,
            SubscriberConfig_NotifyError(Eq(ISubscriberConfig::ERROR_NO_ISIM_APPLICATION)))
            .Times(1);
    EXPECT_CALL(m_objManualProvisioningListener, SubscriberConfig_NotifyError(_)).Times(0);

    IMS_BOOL bResult = m_pSubscriberConfig->Init();
    ASSERT_TRUE(bResult);

    StartSubscriberConfig();
    m_piIsimListener->Isim_OnStateChanged(IIsim::STATE_NOT_PRESENT);

    EXPECT_TRUE(m_pSubscriberConfig->IsProvisioningDone());
}

TEST_F(SubscriberConfigTest, EnableIsim)
{
    m_objImsIdentityPriority.Clear();
    m_objImsIdentityPriority.Add(CarrierConfig::Ims::IMS_IDENTITY_PRIORITY_USIM);
    SetUpCarrierConfig();
    VerifyStoreSubscriptionAttributes(2);
    VerifyStoreSubscriberInfo(2);
    // USIM(2) & REMOVE_ALL & ISIM(2)
    EXPECT_CALL(m_objSubsInfoListener, SubscriberInfo_UpdateImpu(_, _, _, _))
            .Times(m_objImpu.GetCount() * 2 + 1);

    IMS_BOOL bResult = m_pSubscriberConfig->Init();
    ASSERT_TRUE(bResult);

    StartSubscriberConfig();

    EXPECT_FALSE(m_pSubscriberConfig->IsIsimSupported());
    EXPECT_TRUE(m_pSubscriberConfig->IsUsimSupported());
    EXPECT_TRUE(m_pSubscriberConfig->IsProvisioningDone());
    VerifySubscriberInfo();

    VerifyIsimInit();
    EXPECT_CALL(m_objDefaultSubsConfigListener, SubscriberConfig_RefreshStarted()).Times(1);
    EXPECT_CALL(m_objIsimProvisioningListener, SubscriberConfig_RefreshStarted()).Times(1);

    m_pSubscriberConfig->EnableIsim();

    EXPECT_TRUE(m_pSubscriberConfig->IsIsimSupported());
    EXPECT_FALSE(m_pSubscriberConfig->IsUsimSupported());
    EXPECT_FALSE(m_pSubscriberConfig->IsProvisioningDone());

    VerifyIsimReadRecords(1);
    EXPECT_CALL(m_objDefaultSubsConfigListener, SubscriberConfig_RefreshCompleted()).Times(1);
    EXPECT_CALL(m_objIsimProvisioningListener, SubscriberConfig_RefreshCompleted()).Times(1);

    m_piIsimListener->Isim_OnStateChanged(IIsim::STATE_LOADED);

    EXPECT_TRUE(m_pSubscriberConfig->IsIsimSupported());
    EXPECT_FALSE(m_pSubscriberConfig->IsUsimSupported());
    EXPECT_TRUE(m_pSubscriberConfig->IsProvisioningDone());
    VerifySubscriberInfo();
}

TEST_F(SubscriberConfigTest, UpdateSubscriberInfo)
{
    VerifyIsimInit();
    VerifyStoreSubscriptionAttributes(2);
    VerifyStoreSubscriberInfo(1);
    // REMOVE_ALL & ADD
    EXPECT_CALL(m_objSubsInfoListener, SubscriberInfo_UpdateImpu(_, _, _, _)).Times(2);
    EXPECT_CALL(m_objDefaultSubsConfigListener, SubscriberConfig_RefreshStarted()).Times(1);
    EXPECT_CALL(m_objDefaultSubsConfigListener, SubscriberConfig_RefreshCompleted()).Times(1);
    EXPECT_CALL(m_objIsimProvisioningListener, SubscriberConfig_InitCompleted()).Times(0);
    EXPECT_CALL(m_objIsimProvisioningListener, SubscriberConfig_RefreshStarted()).Times(0);
    EXPECT_CALL(m_objIsimProvisioningListener, SubscriberConfig_RefreshCompleted()).Times(0);
    EXPECT_CALL(m_objManualProvisioningListener, SubscriberConfig_RefreshStarted()).Times(1);
    EXPECT_CALL(m_objManualProvisioningListener, SubscriberConfig_RefreshCompleted()).Times(1);

    IMS_BOOL bResult = m_pSubscriberConfig->Init();
    ASSERT_TRUE(bResult);

    StartSubscriberConfig();
    m_piIsimListener->Isim_OnStateChanged(IIsim::STATE_NOT_PRESENT);

    EXPECT_TRUE(m_pSubscriberConfig->IsIsimSupported());
    EXPECT_FALSE(m_pSubscriberConfig->IsUsimSupported());
    EXPECT_TRUE(m_pSubscriberConfig->IsProvisioningDone());

    m_pSubscriberConfig->UpdateSubscriberInfo(m_strDomain, m_strImpi, m_objImpu.GetElementAt(0));

    EXPECT_FALSE(m_pSubscriberConfig->IsIsimSupported());
    EXPECT_TRUE(m_pSubscriberConfig->IsUsimSupported());
    EXPECT_TRUE(m_pSubscriberConfig->IsProvisioningDone());

    EXPECT_EQ(m_pSubscriberConfig->GetCredential().GetRealm(), m_strDomain);
    EXPECT_EQ(m_pSubscriberConfig->GetCredential().GetUsername(), m_strImpi);
    EXPECT_EQ(m_pSubscriberConfig->GetHomeDomainName(), m_strDomain);
    EXPECT_EQ(m_pSubscriberConfig->GetIndexOfPrimaryPublicUserId(), 0);
    EXPECT_EQ(m_pSubscriberConfig->GetPhoneContext(), m_strDomain);
    EXPECT_EQ(m_pSubscriberConfig->GetPrivateUserId(), m_strImpi);
    EXPECT_EQ(m_pSubscriberConfig->GetPublicUserId(), m_objImpu.GetElementAt(0));
    EXPECT_EQ(m_pSubscriberConfig->GetPublicUserIds().GetCount(), 1);
    EXPECT_EQ(m_pSubscriberConfig->GetPublicUserId(IImsSubscriberInfo::IMPU_SIP),
            m_objImpu.GetElementAt(0));
    EXPECT_TRUE(m_pSubscriberConfig->GetPublicUserId(IImsSubscriberInfo::IMPU_TEL).IsEmpty());
    EXPECT_EQ(m_pSubscriberConfig->GetScscfAddress(), m_strDomain);
}

TEST_F(SubscriberConfigTest, UpdateSubscriberInfoWithIsimEnabled)
{
    VerifyIsimInit();
    VerifyStoreSubscriptionAttributes(2);
    VerifyStoreSubscriberInfo(1);
    // REMOVE_ALL & ADD
    EXPECT_CALL(m_objSubsInfoListener, SubscriberInfo_UpdateImpu(_, _, _, _)).Times(2);
    EXPECT_CALL(m_objDefaultSubsConfigListener, SubscriberConfig_RefreshStarted()).Times(1);
    EXPECT_CALL(m_objDefaultSubsConfigListener, SubscriberConfig_RefreshCompleted()).Times(1);
    EXPECT_CALL(m_objIsimProvisioningListener, SubscriberConfig_InitCompleted()).Times(0);
    EXPECT_CALL(m_objIsimProvisioningListener, SubscriberConfig_RefreshStarted()).Times(0);
    EXPECT_CALL(m_objIsimProvisioningListener, SubscriberConfig_RefreshCompleted()).Times(0);
    EXPECT_CALL(m_objManualProvisioningListener, SubscriberConfig_RefreshStarted()).Times(1);
    EXPECT_CALL(m_objManualProvisioningListener, SubscriberConfig_RefreshCompleted()).Times(1);

    IMS_BOOL bResult = m_pSubscriberConfig->Init();
    ASSERT_TRUE(bResult);

    StartSubscriberConfig();
    m_piIsimListener->Isim_OnStateChanged(IIsim::STATE_NOT_PRESENT);

    EXPECT_TRUE(m_pSubscriberConfig->IsIsimSupported());
    EXPECT_FALSE(m_pSubscriberConfig->IsUsimSupported());
    EXPECT_TRUE(m_pSubscriberConfig->IsProvisioningDone());

    m_pSubscriberConfig->UpdateSubscriberInfo(
            m_strDomain, m_strImpi, m_objImpu.GetElementAt(0), IMS_TRUE);

    EXPECT_TRUE(m_pSubscriberConfig->IsIsimSupported());
    EXPECT_FALSE(m_pSubscriberConfig->IsUsimSupported());
    EXPECT_TRUE(m_pSubscriberConfig->IsProvisioningDone());

    EXPECT_EQ(m_pSubscriberConfig->GetCredential().GetRealm(), m_strDomain);
    EXPECT_EQ(m_pSubscriberConfig->GetCredential().GetUsername(), m_strImpi);
    EXPECT_EQ(m_pSubscriberConfig->GetHomeDomainName(), m_strDomain);
    EXPECT_EQ(m_pSubscriberConfig->GetIndexOfPrimaryPublicUserId(), 0);
    EXPECT_EQ(m_pSubscriberConfig->GetPhoneContext(), m_strDomain);
    EXPECT_EQ(m_pSubscriberConfig->GetPrivateUserId(), m_strImpi);
    EXPECT_EQ(m_pSubscriberConfig->GetPublicUserId(), m_objImpu.GetElementAt(0));
    EXPECT_EQ(m_pSubscriberConfig->GetPublicUserIds().GetCount(), 1);
    EXPECT_EQ(m_pSubscriberConfig->GetPublicUserId(IImsSubscriberInfo::IMPU_SIP),
            m_objImpu.GetElementAt(0));
    EXPECT_TRUE(m_pSubscriberConfig->GetPublicUserId(IImsSubscriberInfo::IMPU_TEL).IsEmpty());
    EXPECT_EQ(m_pSubscriberConfig->GetScscfAddress(), m_strDomain);
}

TEST_F(SubscriberConfigTest, RefreshWhenIsimEnabled)
{
    VerifyIsimInit();
    VerifyIsimReadRecords(1);
    VerifyStoreSubscriptionAttributes(2);
    VerifyStoreSubscriberInfo(1);
    // REMOVE_ALL & count of IMPU.
    EXPECT_CALL(m_objSubsInfoListener, SubscriberInfo_UpdateImpu(_, _, _, _)).Times(3);
    EXPECT_CALL(m_objDefaultSubsConfigListener, SubscriberConfig_InitCompleted()).Times(1);
    EXPECT_CALL(m_objIsimProvisioningListener, SubscriberConfig_InitCompleted()).Times(1);
    EXPECT_CALL(m_objManualProvisioningListener, SubscriberConfig_InitCompleted()).Times(0);
    MockIIsim& objIsim = m_objPhoneInfoService.GetMockIsim();
    EXPECT_CALL(objIsim, IsLoadCompleted()).Times(1).WillOnce(Return(IMS_TRUE));

    StartSubscriberConfig();
    m_pSubscriberConfig->Refresh();

    VerifySubscriberInfo();
}

TEST_F(SubscriberConfigTest, RefreshWhenUsimEnabled)
{
    m_objImsIdentityPriority.Clear();
    m_objImsIdentityPriority.Add(CarrierConfig::Ims::IMS_IDENTITY_PRIORITY_USIM);
    SetUpCarrierConfig();
    VerifyStoreSubscriptionAttributes(1);
    VerifyStoreSubscriberInfo(1);
    // REMOVE_ALL & count of IMPU.
    EXPECT_CALL(m_objSubsInfoListener, SubscriberInfo_UpdateImpu(_, _, _, _)).Times(3);
    EXPECT_CALL(m_objDefaultSubsConfigListener, SubscriberConfig_InitCompleted()).Times(1);
    EXPECT_CALL(m_objIsimProvisioningListener, SubscriberConfig_InitCompleted()).Times(0);
    EXPECT_CALL(m_objManualProvisioningListener, SubscriberConfig_InitCompleted()).Times(1);

    m_pSubscriberConfig->Refresh();

    VerifySubscriberInfo();
}

TEST_F(SubscriberConfigTest, Update)
{
    IConfigurable* piConfigurable = m_pSubscriberConfig->GetConfigurable();

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

    // ISIM
    EXPECT_TRUE(m_pSubscriberConfig->IsIsimSupported());
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

    // USIM
    m_objImsIdentityPriority.Clear();
    m_objImsIdentityPriority.Add(CarrierConfig::Ims::IMS_IDENTITY_PRIORITY_USIM);
    SetUpCarrierConfig();

    EXPECT_CALL(m_objSubsInfoListener, SubscriberInfo_UpdateImpu(_, _, _, _)).Times(7);
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

    VerifyStoreSubscriptionAttributes(2);
    VerifyStoreSubscriberInfo(2);
    EXPECT_CALL(m_objDefaultSubsConfigListener, SubscriberConfig_RefreshStarted()).Times(1);
    EXPECT_CALL(m_objDefaultSubsConfigListener, SubscriberConfig_RefreshCompleted()).Times(1);
    EXPECT_CALL(m_objManualProvisioningListener, SubscriberConfig_RefreshStarted()).Times(1);
    EXPECT_CALL(m_objManualProvisioningListener, SubscriberConfig_RefreshCompleted()).Times(1);

    EXPECT_TRUE(piConfigurable->Update(IConfigurable::CP_I_SUBSCRIBER_ALL));
    EXPECT_TRUE(piConfigurable->Update(IConfigurable::CP_I_WRITE_PROVISIONING_SUBSCRIBER));
    EXPECT_TRUE(m_pSubscriberConfig->IsProvisioningDone());
    EXPECT_FALSE(m_pSubscriberConfig->IsTestMode());
    EXPECT_TRUE(piConfigurable->Update(IConfigurable::CP_I_END_SUBSCRIBER));
    EXPECT_FALSE(piConfigurable->Update(IConfigurable::CP_I_MAX));
}

TEST_F(SubscriberConfigTest, CarrierConfigChanged)
{
    VerifyIsimInit();
    VerifyIsimReadRecords(1);

    IMS_BOOL bResult = m_pSubscriberConfig->Init();
    ASSERT_TRUE(bResult);
    StartSubscriberConfig();
    m_piIsimListener->Isim_OnStateChanged(IIsim::STATE_LOADED);

    m_nImpuRefIndex = 1;
    SetUpCarrierConfig();
    IMS_SINT32 nSlotId = m_pSubscriberConfig->GetSlotId();
    EXPECT_CALL(m_objUtilService.GetMockPrivateProperty(),
            SetPersistent(Eq(ImsPrivateProperties::Persistent::KEY_PRIMARY_IMPU),
                    Eq(m_objImpu.GetElementAt(m_nImpuRefIndex)), Eq(nSlotId)))
            .Times(1);

    ICarrierConfigListener* piCarrierConfigListener =
            static_cast<ICarrierConfigListener*>(m_pSubscriberConfig);
    // No actions because SIM slot is not matched.
    piCarrierConfigListener->CarrierConfig_NotifyConfigChanged(nSlotId + 1);

    piCarrierConfigListener->CarrierConfig_NotifyConfigChanged(nSlotId);

    EXPECT_EQ(m_pSubscriberConfig->GetIndexOfPrimaryPublicUserId(), m_nImpuRefIndex);
    EXPECT_EQ(m_pSubscriberConfig->GetPhoneContext(), m_strDomain);
}

TEST_F(SubscriberConfigTest, FakeSubscriber)
{
    VerifyStoreSubscriptionAttributes(0);
    VerifyStoreSubscriberInfo(0);

    const AString& strImpu = ImsIdentity::GetAnonymousUserId();
    AString strTemp;
    AString strDomain;
    AString strImpi;
    strImpu.SplitF('@', strTemp, strDomain, IMS_FALSE);
    strImpu.SplitF(':', strTemp, strImpi, IMS_FALSE);

    AString strPublicUserId;
    strPublicUserId.Sprintf("\"Anonymous\" <%s>", strImpu.GetStr());

    AString strId("fake");
    AString strConfName = "subscriber_" + strId;
    SubscriberConfig objSubscriberConfig(IMS_SLOT_0, strConfName);

    ASSERT_TRUE(objSubscriberConfig.GetSubscriberInfo(1) == nullptr);
    ASSERT_TRUE(objSubscriberConfig.GetSubscriberInfo() == nullptr);

    IMS_BOOL bResult = objSubscriberConfig.Init();
    ASSERT_TRUE(bResult);

    EXPECT_EQ(objSubscriberConfig.GetConfName(), strConfName);
    EXPECT_EQ(objSubscriberConfig.GetId(), strId);

    EXPECT_TRUE(objSubscriberConfig.IsProvisioningDone());
    EXPECT_EQ(objSubscriberConfig.GetCredential().GetRealm(), strDomain);
    EXPECT_EQ(objSubscriberConfig.GetCredential().GetUsername(), strImpi);
    EXPECT_EQ(objSubscriberConfig.GetHomeDomainName(), strDomain);
    EXPECT_EQ(objSubscriberConfig.GetIndexOfPrimaryPublicUserId(), 0);
    EXPECT_EQ(objSubscriberConfig.GetPhoneContext(), strDomain);
    EXPECT_EQ(objSubscriberConfig.GetPrivateUserId(), strImpi);
    EXPECT_EQ(objSubscriberConfig.GetPublicUserId(), strPublicUserId);
    EXPECT_EQ(objSubscriberConfig.GetPublicUserIds().GetCount(), 1);
    EXPECT_EQ(objSubscriberConfig.GetPublicUserId(IImsSubscriberInfo::IMPU_SIP), strPublicUserId);
    EXPECT_TRUE(objSubscriberConfig.GetPublicUserId(IImsSubscriberInfo::IMPU_TEL).IsEmpty());
    EXPECT_EQ(objSubscriberConfig.GetScscfAddress(), strDomain);
}

}  // namespace android
