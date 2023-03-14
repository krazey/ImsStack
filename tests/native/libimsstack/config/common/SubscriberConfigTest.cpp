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

#include "ImsEventDef.h"
#include "ImsTypeDef.h"
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

        EXPECT_CALL(m_objConfigService.GetMockCarrierConfig(),
                GetString(Eq(CarrierConfig::Ims::KEY_PHONE_CONTEXT_DOMAIN_NAME_STRING), _))
                .Times(AnyNumber())
                .WillRepeatedly(Return(AString("phone-context")));

        m_pSubscriberConfig = new SubscriberConfig(IMS_SLOT_0, "subscriber");

        ISubscriberConfig* piSubConfig = static_cast<ISubscriberConfig*>(m_pSubscriberConfig);
        piSubConfig->SetListener(&m_objSubscriberConfigListener);
        m_piIsimListener = static_cast<IIsimListener*>(m_pSubscriberConfig);
        m_pConfigBase = static_cast<ConfigBase*>(m_pSubscriberConfig);
        m_pSubscriberConfig->SetSubscriberInfoListener(&m_objSubInfoListener);

        AString strPcscfs = "10.34.14.4, 10.34.14.5";
        ON_CALL(m_objUtilService.GetMockPrivateProperty(),
                GetPersistent(Eq(ImsPrivateProperties::Persistent::KEY_CONFIG_PCSCF_ADDRESS_LIST),
                        Eq(IMS_SLOT_0)))
                .WillByDefault(Return(strPcscfs));

        EXPECT_CALL(m_objUtilService.GetMockPrivateProperty(), SetPersistentBoolean(_, _, _))
                .Times(AnyNumber());

        EXPECT_CALL(m_objConfigService.GetMockCarrierConfig(),
                GetInt(Eq(CarrierConfig::Ims::KEY_SIP_SERVER_PORT_NUMBER_INT), _))
                .Times(AnyNumber())
                .WillRepeatedly(Return(5060));

        EXPECT_CALL(m_objConfigService.GetMockCarrierConfig(),
                GetInt(Eq(CarrierConfig::Ims::KEY_ISIM_INDEX_FOR_IMPU_INT), _))
                .Times(AnyNumber())
                .WillRepeatedly(Return(1));

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
};

TEST_F(SubscriberConfigTest, Init)
{
    EXPECT_CALL(m_objConfigService.GetMockCarrierConfig(), AddListener(_)).Times(1);

    EXPECT_TRUE(m_pSubscriberConfig->Init());

    ASSERT_TRUE(m_pSubscriberConfig->GetSubscriberInfo() != nullptr);
    EXPECT_TRUE(m_pSubscriberConfig->IsAkaSupported());
    EXPECT_TRUE(m_pSubscriberConfig->IsIsimSupported());
    EXPECT_EQ(m_pSubscriberConfig->GetHomeDomainName(), AString::ConstNull());
    EXPECT_EQ(m_pSubscriberConfig->GetPhoneContext(), AString("phone-context"));
    EXPECT_EQ(m_pSubscriberConfig->GetIndexOfPrimaryPublicUserId(), 1);
    ASSERT_TRUE(m_pSubscriberConfig->GetPrivateUserId().IsNull());
    ASSERT_TRUE(m_pSubscriberConfig->GetPublicUserId().IsNull());
    AStringArray objPublicUserIds = m_pSubscriberConfig->GetPublicUserIds();
    EXPECT_EQ(objPublicUserIds.GetCount(), 0);
}

TEST_F(SubscriberConfigTest, InitProvisioning)
{
    EXPECT_CALL(m_objIsim, Init()).Times(1).WillOnce(Return(IMS_SUCCESS));

    IAsyncConfig* piAsyncConfig = static_cast<IAsyncConfig*>(m_pSubscriberConfig);
    piAsyncConfig->HandleMessage(IAsyncConfig::ACMSG_START, 0, 0);

    EXPECT_CALL(m_objIsim, Init()).Times(2).WillOnce(Return(IMS_FAILURE));

    // Isim Init failed so retry
    ITimerListener* piTimerListener;
    EXPECT_CALL(m_objTimerService.GetMockTimer(), SetTimer(_, _))
            .Times(1)
            .WillOnce(Invoke(
                    [&](Unused, ITimerListener* pListener)
                    {
                        piTimerListener = pListener;
                        return 1;
                    }));
    piAsyncConfig->HandleMessage(IAsyncConfig::ACMSG_START, 0, 0);

    piTimerListener->Timer_TimerExpired(static_cast<ITimer*>(&m_objTimerService.GetMockTimer()));
}

TEST_F(SubscriberConfigTest, StartProvisioning)
{
    EXPECT_TRUE(m_pSubscriberConfig->Init());

    m_piIsimListener->Isim_OnStateChanged(IIsim::STATE_IDLE);

    EXPECT_CALL(m_objIsim, Init()).Times(2).WillRepeatedly(Return(IMS_SUCCESS));
    EXPECT_CALL(m_objIsim, IsReady()).Times(AnyNumber()).WillRepeatedly(Return(IMS_TRUE));
    EXPECT_CALL(m_objIsim, AddListener(_)).Times(2);
    EXPECT_CALL(m_objIsim, GetState()).Times(6).WillRepeatedly(Return(IIsim::STATE_INIT));

    IAsyncConfig* piAsyncConfig = static_cast<IAsyncConfig*>(m_pSubscriberConfig);
    piAsyncConfig->HandleMessage(IAsyncConfig::ACMSG_START, 0, 0);

    // Isim Start failed so retry
    EXPECT_CALL(m_objIsim, Start(_)).Times(6).WillRepeatedly(Return(IMS_FAILURE));
    ITimerListener* piTimerListener;
    EXPECT_CALL(m_objTimerService.GetMockTimer(), SetTimer(_, _))
            .Times(6)
            .WillRepeatedly(Invoke(
                    [&](Unused, ITimerListener* pListener)
                    {
                        piTimerListener = pListener;
                        return 1;
                    }));

    m_piIsimListener->Isim_OnStateChanged(IIsim::STATE_INIT);
    ASSERT_TRUE(piTimerListener != nullptr);
    piTimerListener->Timer_TimerExpired(&m_objTimerService.GetMockTimer());
    piTimerListener->Timer_TimerExpired(&m_objTimerService.GetMockTimer());
    piTimerListener->Timer_TimerExpired(&m_objTimerService.GetMockTimer());
    piTimerListener->Timer_TimerExpired(&m_objTimerService.GetMockTimer());
    piTimerListener->Timer_TimerExpired(&m_objTimerService.GetMockTimer());

    EXPECT_CALL(m_objIsim, Start(_)).Times(AnyNumber()).WillRepeatedly(Return(IMS_SUCCESS));
    piTimerListener->Timer_TimerExpired(&m_objTimerService.GetMockTimer());

    EXPECT_CALL(m_objIsim, GetState())
            .Times(AnyNumber())
            .WillRepeatedly(Return(IIsim::STATE_READY));
    EXPECT_CALL(m_objIsim, GetImpi()).Times(1).WillOnce(Return(IMS_FAILURE));

    // Isim Init : start provisioning but GetImpi is failed
    EXPECT_CALL(m_objSubscriberConfigListener, SubscriberConfig_NotifyError(_)).Times(1);
    m_piIsimListener->Isim_OnStateChanged(IIsim::STATE_INIT);

    // Isim Init : start provisioning but GetImpi is success
    EXPECT_CALL(m_objIsim, GetImpi()).Times(AnyNumber()).WillRepeatedly(Return(IMS_SUCCESS));
    m_piIsimListener->Isim_OnStateChanged(IIsim::STATE_INIT);

    // Received Impi
    ByteArray objRecord("1111@test.ims.com");

    m_piIsimListener->Isim_OnImpi(objRecord);
    EXPECT_EQ(m_pSubscriberConfig->GetPrivateUserId(), AString("1111@test.ims.com"));

    // Isim Init : start provisioning but GetImpu is failed
    EXPECT_CALL(m_objIsim, GetImpu()).Times(1).WillOnce(Return(IMS_FAILURE));
    EXPECT_CALL(m_objSubscriberConfigListener, SubscriberConfig_NotifyError(_)).Times(1);
    m_piIsimListener->Isim_OnStateChanged(IIsim::STATE_INIT);

    // Isim Init : start provisioning but GetImpu is success
    EXPECT_CALL(m_objIsim, GetImpu()).Times(AnyNumber()).WillRepeatedly(Return(IMS_SUCCESS));
    m_piIsimListener->Isim_OnStateChanged(IIsim::STATE_INIT);

    // While provisioning is going on calling Isim state changed to refreshing
    EXPECT_CALL(m_objSubscriberConfigListener, SubscriberConfig_RefreshStarted()).Times(2);
    m_piIsimListener->Isim_OnStateChanged(IIsim::STATE_REFRESHING);

    // Received Impi
    ByteArray objImpiRecord("sip:1111@test.ims.com");
    ImsList<ByteArray> objValues;
    objValues.Append(objImpiRecord);

    m_piIsimListener->Isim_OnImpu(objValues);
    EXPECT_EQ(m_pSubscriberConfig->GetPublicUserId(), AString("sip:1111@test.ims.com"));

    // Isim Init : start provisioning but GetHomeDomainName is failed
    EXPECT_CALL(m_objIsim, GetHomeDomainName()).Times(1).WillOnce(Return(IMS_FAILURE));
    EXPECT_CALL(m_objSubscriberConfigListener, SubscriberConfig_NotifyError(_)).Times(1);
    m_piIsimListener->Isim_OnStateChanged(IIsim::STATE_INIT);

    // Isim Init : start provisioning but GetHomeDomainName is success
    EXPECT_CALL(m_objIsim, GetHomeDomainName())
            .Times(AnyNumber())
            .WillRepeatedly(Return(IMS_SUCCESS));
    m_piIsimListener->Isim_OnStateChanged(IIsim::STATE_READY);

    // Received HomeDomain
    EXPECT_CALL(m_objSubscriberConfigListener, SubscriberConfig_InitCompleted()).Times(1);
    ByteArray objHomeDomainRecord("test.ims.com");

    m_piIsimListener->Isim_OnHomeDomainName(objHomeDomainRecord);
    EXPECT_EQ(m_pSubscriberConfig->GetHomeDomainName(), AString("test.ims.com"));

    // Calling INIT after provisioned
    EXPECT_CALL(m_objSubInfoListener, SubscriberInfo_UpdateImpu(_, _, _, _)).Times(2);
    m_piIsimListener->Isim_OnStateChanged(IIsim::STATE_INIT);
    m_pSubscriberConfig->Refresh();
}

TEST_F(SubscriberConfigTest, ReadIsimRecord)
{
    EXPECT_TRUE(m_pSubscriberConfig->Init());

    EXPECT_CALL(m_objIsim, Init()).Times(1).WillOnce(Return(IMS_SUCCESS));
    EXPECT_CALL(m_objIsim, IsReady()).Times(AnyNumber()).WillRepeatedly(Return(IMS_TRUE));
    EXPECT_CALL(m_objIsim, AddListener(_)).Times(1);
    EXPECT_CALL(m_objIsim, GetState()).Times(2).WillRepeatedly(Return(IIsim::STATE_INIT));

    IAsyncConfig* piAsyncConfig = static_cast<IAsyncConfig*>(m_pSubscriberConfig);
    piAsyncConfig->HandleMessage(IAsyncConfig::ACMSG_START, 0, 0);

    ImsList<ByteArray> objValues;
    m_piIsimListener->Isim_OnField(IIsim::FIELD_IST, objValues);

    ByteArray objIstRecord("80000000000000");
    objValues.Append(objIstRecord);
    m_piIsimListener->Isim_OnField(IIsim::FIELD_IST, objValues);

    EXPECT_CALL(m_objIsim, GetState())
            .Times(AnyNumber())
            .WillRepeatedly(Return(IIsim::STATE_READY));
    EXPECT_CALL(m_objIsim, GetImpi()).Times(2).WillRepeatedly(Return(IIsim::RESULT_NO_RECORDS));
    EXPECT_CALL(m_objIsim, GetImpu()).Times(2).WillRepeatedly(Return(IIsim::RESULT_NO_RECORDS));
    EXPECT_CALL(m_objIsim, GetHomeDomainName())
            .Times(2)
            .WillRepeatedly(Return(IIsim::RESULT_NO_RECORDS));

    AString strPcscf("1.1.1.5");
    ByteArray objPcscfRecord(strPcscf);
    objValues.Clear();
    objValues.Append(objPcscfRecord);

    m_piIsimListener->Isim_OnField(IIsim::FIELD_PCSCF_ADDRESS, objValues);

    ServerAddress* pSa = m_pSubscriberConfig->GetPcscfAddress();
    EXPECT_EQ(pSa->GetAddress(), strPcscf);
    ImsVector<ServerAddress*> objPcscfs;
    objPcscfs = m_pSubscriberConfig->GetPcscfAddresses();
    EXPECT_FALSE(objPcscfs.IsEmpty());
    EXPECT_EQ(objPcscfs.GetSize(), 2);
    EXPECT_EQ((objPcscfs.GetAt(0))->GetAddress(), strPcscf);
    EXPECT_TRUE((objPcscfs.GetAt(1))->GetAddress().IsNull());

    objValues.Clear();
    objValues.Append(ByteArray::ConstNull());
    m_piIsimListener->Isim_OnField(IIsim::FIELD_PCSCF_ADDRESS, objValues);

    pSa = m_pSubscriberConfig->GetPcscfAddress();
    EXPECT_TRUE(pSa->GetAddress().IsNull());
    objPcscfs = m_pSubscriberConfig->GetPcscfAddresses();
    EXPECT_FALSE(objPcscfs.IsEmpty());
    EXPECT_EQ(objPcscfs.GetSize(), 2);
    EXPECT_TRUE((objPcscfs.GetAt(0))->GetAddress().IsNull());
    EXPECT_TRUE((objPcscfs.GetAt(1))->GetAddress().IsNull());

    // Calling REFRESHED after provisioned
    EXPECT_CALL(m_objSubInfoListener, SubscriberInfo_UpdateImpu(_, _, _, _)).Times(1);
    m_piIsimListener->Isim_OnStateChanged(IIsim::STATE_REFRESHED);
}

TEST_F(SubscriberConfigTest, IsimRefresh)
{
    EXPECT_TRUE(m_pSubscriberConfig->Init());

    EXPECT_CALL(m_objIsim, Init()).Times(1).WillOnce(Return(IMS_SUCCESS));
    EXPECT_CALL(m_objIsim, IsReady()).Times(AnyNumber()).WillRepeatedly(Return(IMS_TRUE));
    EXPECT_CALL(m_objIsim, AddListener(_)).Times(1);
    EXPECT_CALL(m_objIsim, Start(_)).Times(AnyNumber()).WillRepeatedly(Return(IMS_SUCCESS));
    EXPECT_CALL(m_objIsim, GetState())
            .Times(AnyNumber())
            .WillRepeatedly(Return(IIsim::STATE_READY));

    IAsyncConfig* piAsyncConfig = static_cast<IAsyncConfig*>(m_pSubscriberConfig);
    piAsyncConfig->HandleMessage(IAsyncConfig::ACMSG_START, 0, 0);

    EXPECT_CALL(m_objSubscriberConfigListener, SubscriberConfig_RefreshStarted()).Times(3);
    EXPECT_CALL(m_objSubInfoListener, SubscriberInfo_UpdateImpu(_, _, _, _)).Times(1);

    m_piIsimListener->Isim_OnStateChanged(IIsim::STATE_REFRESHING);
    m_piIsimListener->Isim_OnStateChanged(IIsim::STATE_INIT);

    EXPECT_CALL(m_objIsim, GetImpi()).Times(2).WillRepeatedly(Return(IIsim::RESULT_NO_RECORDS));
    EXPECT_CALL(m_objIsim, GetImpu()).Times(2).WillRepeatedly(Return(IIsim::RESULT_NO_RECORDS));
    EXPECT_CALL(m_objIsim, GetHomeDomainName())
            .Times(2)
            .WillRepeatedly(Return(IIsim::RESULT_NO_RECORDS));

    // Isim in Refresing state, provisioning done with Impi, Impu, HomeDomain no records
    m_piIsimListener->Isim_OnStateChanged(IIsim::STATE_REFRESHING);
    m_piIsimListener->Isim_OnStateChanged(IIsim::STATE_INIT);
    m_piIsimListener->Isim_OnError(IIsim::ERROR_REFRESH_ERROR);

    EXPECT_CALL(m_objSubscriberConfigListener, SubscriberConfig_RefreshCompleted()).Times(1);
    m_piIsimListener->Isim_OnStateChanged(IIsim::STATE_REFRESHED);
}

TEST_F(SubscriberConfigTest, IsimError)
{
    EXPECT_CALL(m_objSubscriberConfigListener, SubscriberConfig_NotifyError(_)).Times(2);

    m_piIsimListener->Isim_OnError(IIsim::ERROR_REFRESH_REG_FAILED);
    m_piIsimListener->Isim_OnError(IIsim::ERROR_INTERFACE_CHANNEL_ERROR);
    m_piIsimListener->Isim_OnError(IIsim::ERROR_REFRESH_ERROR);
    m_piIsimListener->Isim_OnError(IIsim::ERROR_READ_DENIED);
    m_piIsimListener->Isim_OnError(IIsim::ERROR_CARD_REMOVED);
    m_piIsimListener->Isim_OnError(IIsim::ERROR_NO_ISIM_APPLICATION);
    MockISubscriberConfigListener objSubscriberConfigListener;
    ISubscriberConfig* piSubConfig = static_cast<ISubscriberConfig*>(m_pSubscriberConfig);
    EXPECT_CALL(objSubscriberConfigListener, SubscriberConfig_NotifyError(_)).Times(1);
    piSubConfig->SetListener(&objSubscriberConfigListener);
    piSubConfig->RemoveListener(&objSubscriberConfigListener);
}

TEST_F(SubscriberConfigTest, Refresh)
{
    EXPECT_CALL(m_objConfigService.GetMockCarrierConfig(), GetInt(_, _)).Times(1);

    ImsVector<IMS_SINT32> objImsIdentity;
    objImsIdentity.Add(CarrierConfig::Ims::IMS_IDENTITY_PRIORITY_USIM);

    EXPECT_CALL(m_objConfigService.GetMockCarrierConfig(),
            GetIntArray(CarrierConfig::Ims::KEY_IMS_IDENTITY_PRIORITY_INT_ARRAY))
            .Times(1)
            .WillOnce(Return(objImsIdentity));

    ON_CALL(m_objUtilService.GetMockPrivateProperty(),
            GetPersistent(Eq(ImsPrivateProperties::Persistent::KEY_CONFIG_HOME_DOMAIN_NAME),
                    Eq(IMS_SLOT_0)))
            .WillByDefault(Return(AString("test2.ims.com")));

    ON_CALL(m_objUtilService.GetMockPrivateProperty(),
            GetPersistent(Eq(ImsPrivateProperties::Persistent::KEY_CONFIG_IMPI), Eq(IMS_SLOT_0)))
            .WillByDefault(Return(AString("1111@test2.ims.com")));

    ON_CALL(m_objUtilService.GetMockPrivateProperty(),
            GetPersistent(
                    Eq(ImsPrivateProperties::Persistent::KEY_CONFIG_IMPU_LIST), Eq(IMS_SLOT_0)))
            .WillByDefault(Return(AString("sip:1112@test2.ims.com, tel:1112")));

    EXPECT_CALL(m_objSubInfoListener, SubscriberInfo_UpdateImpu(_, _, _, _)).Times(1);

    // Refresh with Usim
    m_pSubscriberConfig->Refresh();

    EXPECT_EQ(m_pSubscriberConfig->GetSubscriberCount(), 1);
    ASSERT_TRUE(m_pSubscriberConfig->GetSubscriberInfoEx(0) != nullptr);
    EXPECT_EQ(m_pSubscriberConfig->GetHomeDomainName(), AString("test2.ims.com"));
    EXPECT_EQ(m_pSubscriberConfig->GetPrivateUserId(), AString("1111@test2.ims.com"));
    EXPECT_EQ(m_pSubscriberConfig->GetPublicUserId(), AString("sip:1112@test2.ims.com"));
    AStringArray strPublicUserIds;
    strPublicUserIds.AddElement("sip:1112@test2.ims.com");
    strPublicUserIds.AddElement("tel:1112");
    EXPECT_EQ(m_pSubscriberConfig->GetPublicUserIds().GetElementAt(0),
            strPublicUserIds.GetElementAt(0));
    EXPECT_EQ(m_pSubscriberConfig->GetPublicUserIds().GetElementAt(1),
            strPublicUserIds.GetElementAt(1));
    EXPECT_EQ(m_pSubscriberConfig->GetScscfAddress(), AString("test2.ims.com"));
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

    EXPECT_CALL(m_objSubInfoListener, SubscriberInfo_UpdateImpu(_, _, _, _)).Times(3);

    EXPECT_TRUE(piConfigurable->Update(
            IConfigurable::CP_I_SUBSCRIPTION_ATTRIBUTE_ISIM, AString("false")));
    EXPECT_TRUE(piConfigurable->Update(
            IConfigurable::CP_I_SUBSCRIPTION_ATTRIBUTE_USIM, AString("true")));
    EXPECT_TRUE(m_pSubscriberConfig->Load());
    EXPECT_FALSE(m_pSubscriberConfig->IsIsimSupported());
    EXPECT_TRUE(m_pSubscriberConfig->IsUsimSupported());

    EXPECT_TRUE(piConfigurable->Update(IConfigurable::CP_I_HOME_DOMAIN_NAME));
    EXPECT_TRUE(
            piConfigurable->Update(IConfigurable::CP_I_HOME_DOMAIN_NAME, AString("two.tmus.com")));

    EXPECT_TRUE(piConfigurable->Update(IConfigurable::CP_I_IMPI));
    EXPECT_TRUE(piConfigurable->Update(IConfigurable::CP_I_IMPI, AString("1111@test2.ims.com")));

    EXPECT_TRUE(piConfigurable->Update(IConfigurable::CP_I_IMPU_PRIMARY_REF_INDEX));
    EXPECT_TRUE(piConfigurable->Update(IConfigurable::CP_I_IMPU_PRIMARY_REF_INDEX, AString("1")));

    EXPECT_TRUE(piConfigurable->Update(IConfigurable::CP_I_IMPU_0));
    EXPECT_FALSE(piConfigurable->Update(IConfigurable::CP_I_IMPU_9));
    EXPECT_TRUE(
            piConfigurable->Update(IConfigurable::CP_I_IMPU_0, AString("sip:1111@two.tmus.com")));

    EXPECT_TRUE(piConfigurable->Update(IConfigurable::CP_I_PHONE_CONTEXT));
    EXPECT_TRUE(
            piConfigurable->Update(IConfigurable::CP_I_PHONE_CONTEXT, AString("phone-context")));

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
    EXPECT_TRUE(piConfigurable->Update(IConfigurable::CP_I_SERVER_SCSCF, AString("scscf.com")));

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

TEST_F(SubscriberConfigTest, ConfigChanged)
{
    MockISubscriberConfigListener objSubscriberConfigListener;
    ISubscriberConfig* piSubConfig = static_cast<ISubscriberConfig*>(m_pSubscriberConfig);

    piSubConfig->SetListener(&objSubscriberConfigListener);
    m_pSubscriberConfig->SetSubscriberInfoListener(&m_objSubInfoListener);
    m_piIsimListener = static_cast<IIsimListener*>(m_pSubscriberConfig);

    EXPECT_TRUE(m_pSubscriberConfig->Init());

    EXPECT_CALL(m_objIsim, Init()).Times(1).WillOnce(Return(IMS_SUCCESS));
    EXPECT_CALL(m_objIsim, IsReady()).Times(AnyNumber()).WillRepeatedly(Return(IMS_TRUE));
    EXPECT_CALL(m_objIsim, AddListener(_)).Times(1);
    EXPECT_CALL(m_objIsim, Start(_)).Times(AnyNumber()).WillRepeatedly(Return(IMS_SUCCESS));
    EXPECT_CALL(m_objIsim, GetState())
            .Times(AnyNumber())
            .WillRepeatedly(Return(IIsim::STATE_READY));

    IAsyncConfig* piAsyncConfig = static_cast<IAsyncConfig*>(m_pSubscriberConfig);
    piAsyncConfig->HandleMessage(IAsyncConfig::ACMSG_START, 0, 0);

    EXPECT_CALL(m_objIsim, GetImpi()).Times(2).WillRepeatedly(Return(IIsim::RESULT_NO_RECORDS));
    EXPECT_CALL(m_objIsim, GetImpu()).Times(2).WillRepeatedly(Return(IIsim::RESULT_NO_RECORDS));
    EXPECT_CALL(m_objIsim, GetHomeDomainName())
            .Times(2)
            .WillRepeatedly(Return(IIsim::RESULT_NO_RECORDS));

    m_piIsimListener->Isim_OnStateChanged(IIsim::STATE_READY);
    // Calling carrier config changed as USIM
    ICarrierConfigListener* piCarrierConfigListener =
            static_cast<ICarrierConfigListener*>(m_pSubscriberConfig);
    piCarrierConfigListener->CarrierConfig_NotifyConfigChanged(IMS_SLOT_1);

    EXPECT_CALL(m_objSubscriberConfigListener, SubscriberConfig_RefreshStarted()).Times(1);
    EXPECT_CALL(m_objSubInfoListener, SubscriberInfo_UpdateImpu(_, _, _, _)).Times(2);

    piCarrierConfigListener->CarrierConfig_NotifyConfigChanged(IMS_SLOT_0);

    // USIM info
    ImsVector<IMS_SINT32> objImsIdentity;
    objImsIdentity.Add(CarrierConfig::Ims::IMS_IDENTITY_PRIORITY_USIM);

    EXPECT_CALL(m_objConfigService.GetMockCarrierConfig(),
            GetIntArray(Eq(CarrierConfig::Ims::KEY_IMS_IDENTITY_PRIORITY_INT_ARRAY)))
            .Times(AnyNumber())
            .WillRepeatedly(Return(objImsIdentity));

    piCarrierConfigListener->CarrierConfig_NotifyConfigChanged(IMS_SLOT_0);

    // Calling Isim records for USIM supported carrier
    ImsList<ByteArray> objValues;
    EXPECT_CALL(m_objSubscriberConfigListener, SubscriberConfig_InitCompleted()).Times(0);
    m_piIsimListener->Isim_OnField(IIsim::FIELD_PCSCF_ADDRESS, objValues);
    m_piIsimListener->Isim_OnHomeDomainName(ByteArray::ConstNull());
    m_piIsimListener->Isim_OnImpi(ByteArray::ConstNull());
    m_piIsimListener->Isim_OnImpu(objValues);
    EXPECT_CALL(m_objSubscriberConfigListener, SubscriberConfig_NotifyError(_)).Times(0);
    m_piIsimListener->Isim_OnError(IIsim::ERROR_NO_ISIM_APPLICATION);
    m_piIsimListener->Isim_OnStateChanged(IIsim::STATE_IDLE);

    piSubConfig->RemoveListener(&objSubscriberConfigListener);
}

TEST_F(SubscriberConfigTest, FakeSubscriber)
{
    SubscriberConfig objSubscriberConfig(IMS_SLOT_0, "subscriber_fake");

    ASSERT_TRUE(objSubscriberConfig.GetSubscriberInfo(1) == nullptr);
    ASSERT_TRUE(objSubscriberConfig.GetSubscriberInfo() == nullptr);

    EXPECT_TRUE(objSubscriberConfig.Init());
}

}  // namespace android
