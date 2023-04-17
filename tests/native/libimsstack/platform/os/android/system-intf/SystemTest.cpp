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
#include <gmock/gmock.h>

#include "IImsRadio.h"
#include "IIpcan.h"
#include "ImsEventDef.h"
#include "ImsIpSecType.h"
#include "ImsPrivateProperties.h"
#include "ImsTypeDef.h"
#include "IPhoneInfoLocation.h"
#include "IPhoneInfoSubscriber.h"
#include "MockISystemListener.h"
#include "MockSystemCallback.h"
#include "OsParcel.h"
#include "ServiceNetworkPolicy.h"
#include "System.h"
#include "SystemConstants.h"
#include "device/OsIsim.h"
#include "device/OsUsim.h"
#include "network/OsNetworkConstants.h"

using ::testing::_;
using ::testing::AnyNumber;
using ::testing::Invoke;
using ::testing::Return;
using ::testing::Unused;

namespace android
{

class SystemTest : public ::testing::Test
{
public:
    int EVENT_CONNECTION_SETUP_PREPARED = 2;

    MockISystemListener m_objMockISystemListener;
    MockSystemCallback m_objMockSystemCallback;

    System* m_pSystem;

protected:
    virtual void SetUp() override
    {
        m_pSystem = System::GetInstance();
        ASSERT_TRUE(m_pSystem != nullptr);
        m_pSystem->SetCallback(&m_objMockSystemCallback);
    }

    virtual void TearDown() override
    {
        if (m_pSystem != IMS_NULL)
        {
            m_pSystem->Destroy();
        }
    }
};

TEST_F(SystemTest, NotifyData)
{
    android::Parcel out;
    android::Parcel in;

    /* Call Notify Data with invalid CATEGORY*/
    in.writeInt32(0);
    in.writeInt32(0);
    in.setDataPosition(0);

    m_pSystem->NotifyData(in, out);
    out.setDataPosition(0);

    EXPECT_EQ(0, out.readInt32());

    /* Call Notify Data for CATEGORY_NETWORK*/
    in.setDataPosition(0);
    out.setDataPosition(0);
    in.writeInt32(0);
    in.writeInt32(SystemConstants::NOTIFY_SERVICE_STATE_CHANGED);
    in.writeInt32(STATE_IN_SERVICE);
    in.setDataPosition(0);

    m_pSystem->NotifyData(in, out);
    out.setDataPosition(0);

    EXPECT_EQ(1, out.readInt32());

    /* Call Notify Data for CATEGORY_WIFI*/
    in.setDataPosition(0);
    out.setDataPosition(0);
    in.writeInt32(0);
    in.writeInt32(SystemConstants::NOTIFY_WIFI_STATE_CHANGED);
    in.writeInt32(WIFI_STATE_DISABLED);
    in.setDataPosition(0);

    m_pSystem->NotifyData(in, out);
    out.setDataPosition(0);

    EXPECT_EQ(1, out.readInt32());

    /* Call Notify Data for CATEGORY_CALL*/
    in.setDataPosition(0);
    out.setDataPosition(0);
    in.writeInt32(0);
    in.writeInt32(SystemConstants::NOTIFY_VOICE_CALL_STATE_CHANGED);
    in.writeInt32(2);
    in.setDataPosition(0);

    m_pSystem->NotifyData(in, out);
    out.setDataPosition(0);

    EXPECT_EQ(1, out.readInt32());

    /* Call Notify Data for CATEGORY_POWER*/
    in.setDataPosition(0);
    out.setDataPosition(0);
    in.writeInt32(0);
    in.writeInt32(SystemConstants::NOTIFY_BATTERY_LEVEL_CHANGED);
    in.writeInt32(15);
    in.setDataPosition(0);

    m_pSystem->NotifyData(in, out);
    out.setDataPosition(0);

    EXPECT_EQ(1, out.readInt32());

    /* Call Notify Data for CATEGORY_ALARM*/
    in.setDataPosition(0);
    out.setDataPosition(0);
    in.writeInt32(0);
    in.writeInt32(SystemConstants::NOTIFY_ALARM_EXPIRED);
    in.writeInt64(154536453574651);
    in.setDataPosition(0);

    m_pSystem->NotifyData(in, out);
    out.setDataPosition(0);

    EXPECT_EQ(1, out.readInt32());

    /* Call Notify Data for CATEGORY_CONFIG*/
    in.setDataPosition(0);
    out.setDataPosition(0);
    in.writeInt32(0);
    in.writeInt32(SystemConstants::NOTIFY_CONFIGURATION_CHANGED);
    in.writeInt32(0);
    in.setDataPosition(0);

    m_pSystem->NotifyData(in, out);
    out.setDataPosition(0);

    EXPECT_EQ(1, out.readInt32());

    /* Call Notify Data for CATEGORY_EVENT*/
    in.setDataPosition(0);
    out.setDataPosition(0);
    in.writeInt32(0);
    in.writeInt32(SystemConstants::NOTIFY_EVENT);
    in.writeInt32(IMS_EVENT_ROAMING_STATE);
    in.writeInt32(IMS_ROAMING_STATE_ON);
    in.writeInt32(IMS_ROAMING_STATE_ON);
    in.setDataPosition(0);

    m_pSystem->NotifyData(in, out);
    out.setDataPosition(0);

    EXPECT_EQ(1, out.readInt32());

    in.setDataPosition(0);
    out.setDataPosition(0);
    in.writeInt32(0);
    in.writeInt32(SystemConstants::NOTIFY_ISIM_EVENT);
    in.writeInt32(OsIsim::NOTIFICATION_ISIM_STATE_CHANGED);
    in.writeString16(String16("LOADED"));
    in.setDataPosition(0);

    m_pSystem->NotifyData(in, out);
    out.setDataPosition(0);

    EXPECT_EQ(1, out.readInt32());

    in.setDataPosition(0);
    out.setDataPosition(0);
    in.writeInt32(0);
    in.writeInt32(SystemConstants::NOTIFY_USIM_EVENT);
    in.writeInt32(OsUsim::NOTIFICATION_USIM_AUTH);
    in.writeInt32(87);
    in.writeString16(String16("response"));
    in.writeInt64(657657576575);
    in.setDataPosition(0);

    m_pSystem->NotifyData(in, out);
    out.setDataPosition(0);

    EXPECT_EQ(1, out.readInt32());

    /* Call Notify Data for CATEGORY_RADIO*/
    in.setDataPosition(0);
    out.setDataPosition(0);
    in.writeInt32(0);
    in.writeInt32(SystemConstants::NOTIFY_RADIO_EVENT);
    in.writeInt32(EVENT_CONNECTION_SETUP_PREPARED);
    in.writeInt32(17);
    in.setDataPosition(0);

    m_pSystem->NotifyData(in, out);
    out.setDataPosition(0);

    EXPECT_EQ(1, out.readInt32());
}

TEST_F(SystemTest, AddAndRemoveListener)
{
    m_pSystem->AddListener(SystemConstants::CATEGORY_NETWORK, null, 0);
    m_pSystem->AddListener(SystemConstants::CATEGORY_NETWORK, &m_objMockISystemListener, 0);

    android::Parcel in;
    android::Parcel out;

    /* Call CATEGORY_NETWORK listener*/
    in.setDataPosition(0);
    out.setDataPosition(0);
    in.writeInt32(0);
    in.writeInt32(SystemConstants::NOTIFY_SERVICE_STATE_CHANGED);
    in.writeInt32(STATE_IN_SERVICE);
    in.setDataPosition(0);

    m_pSystem->NotifyData(in, out);
    out.setDataPosition(0);

    EXPECT_EQ(1, out.readInt32());

    in.setDataPosition(0);
    out.setDataPosition(0);
    in.writeInt32(0);
    in.writeInt32(SystemConstants::NOTIFY_DATA_CONNECTION_STATE_CHANGED);
    in.writeInt32(NetworkPolicy::APN_IMS);
    in.writeInt32(DATA_CONNECTED);
    in.setDataPosition(0);

    m_pSystem->NotifyData(in, out);

    out.setDataPosition(0);
    EXPECT_EQ(1, out.readInt32());

    in.setDataPosition(0);
    out.setDataPosition(0);
    in.writeInt32(0);
    in.writeInt32(SystemConstants::NOTIFY_DATA_CONNECTION_IPCAN_CHANGED);
    in.writeInt32(NetworkPolicy::APN_IMS);
    in.writeInt32(IIpcan::CATEGORY_MOBILE);
    in.setDataPosition(0);

    m_pSystem->NotifyData(in, out);
    out.setDataPosition(0);

    EXPECT_EQ(1, out.readInt32());

    in.setDataPosition(0);
    out.setDataPosition(0);
    in.writeInt32(0);
    in.writeInt32(SystemConstants::NOTIFY_AIRPLANE_MODE_CHANGED);
    in.writeInt32(0);
    in.setDataPosition(0);

    m_pSystem->NotifyData(in, out);
    out.setDataPosition(0);

    EXPECT_EQ(1, out.readInt32());

    in.setDataPosition(0);
    out.setDataPosition(0);
    in.writeInt32(0);
    in.writeInt32(SystemConstants::NOTIFY_NETWORK_TYPE_CHANGED);
    in.writeInt32(RADIO_TECH_LTE);
    in.setDataPosition(0);

    m_pSystem->NotifyData(in, out);
    out.setDataPosition(0);

    EXPECT_EQ(1, out.readInt32());

    in.setDataPosition(0);
    out.setDataPosition(0);
    in.writeInt32(0);
    in.writeInt32(SystemConstants::NOTIFY_VOICE_NETWORK_TYPE_CHANGED);
    in.writeInt32(RADIO_TECH_LTE);
    in.setDataPosition(0);

    m_pSystem->NotifyData(in, out);
    out.setDataPosition(0);

    EXPECT_EQ(1, out.readInt32());

    in.setDataPosition(0);
    out.setDataPosition(0);
    in.writeInt32(0);
    in.writeInt32(SystemConstants::NOTIFY_DATA_CONNECTION_FAILED);
    in.writeInt32(NetworkPolicy::APN_IMS);
    in.setDataPosition(0);

    m_pSystem->NotifyData(in, out);
    out.setDataPosition(0);

    EXPECT_EQ(1, out.readInt32());

    in.setDataPosition(0);
    out.setDataPosition(0);
    in.writeInt32(0);
    in.writeInt32(SystemConstants::NOTIFY_DATA_CONNECTION_FAILED);
    in.writeInt32(NetworkPolicy::APN_IMS);
    in.setDataPosition(0);

    m_pSystem->NotifyData(in, out);
    out.setDataPosition(0);

    EXPECT_EQ(1, out.readInt32());

    in.setDataPosition(0);
    out.setDataPosition(0);
    in.writeInt32(0);
    in.writeInt32(SystemConstants::CATEGORY_NETWORK + 2);
    in.writeInt32(0);
    in.setDataPosition(0);

    m_pSystem->NotifyData(in, out);
    out.setDataPosition(0);

    EXPECT_EQ(1, out.readInt32());
    EXPECT_CALL(m_objMockISystemListener, System_NotifyEvent(_, _, _)).Times(AnyNumber());

    /* Call CATEGORY_WIFI listener*/
    m_pSystem->AddListener(SystemConstants::CATEGORY_WIFI, &m_objMockISystemListener, 0);

    in.setDataPosition(0);
    out.setDataPosition(0);
    in.writeInt32(0);
    in.writeInt32(SystemConstants::NOTIFY_WIFI_STATE_CHANGED);
    in.writeInt32(WIFI_STATE_DISABLED);
    in.setDataPosition(0);

    m_pSystem->NotifyData(in, out);
    out.setDataPosition(0);

    EXPECT_EQ(1, out.readInt32());

    in.setDataPosition(0);
    out.setDataPosition(0);
    in.writeInt32(0);
    in.writeInt32(SystemConstants::NOTIFY_WIFI_CONNECTION_STATE_CHANGED);
    in.writeInt32(WIFI_CONNECTION_STATE_CONNECTED);
    in.setDataPosition(0);

    m_pSystem->NotifyData(in, out);
    out.setDataPosition(0);

    EXPECT_EQ(1, out.readInt32());

    in.setDataPosition(0);
    out.setDataPosition(0);
    in.writeInt32(0);
    in.writeInt32(SystemConstants::CATEGORY_WIFI + 1);
    in.writeInt32(0);
    in.setDataPosition(0);

    m_pSystem->NotifyData(in, out);
    out.setDataPosition(0);

    EXPECT_EQ(1, out.readInt32());
    EXPECT_CALL(m_objMockISystemListener, System_NotifyEvent(_, _, _)).Times(AnyNumber());

    /* Call CATEGORY_CALL listener*/
    m_pSystem->AddListener(SystemConstants::CATEGORY_CALL, &m_objMockISystemListener, 0);

    in.setDataPosition(0);
    out.setDataPosition(0);
    in.writeInt32(0);
    in.writeInt32(SystemConstants::NOTIFY_VOICE_CALL_STATE_CHANGED);
    in.writeInt32(2);
    in.setDataPosition(0);

    m_pSystem->NotifyData(in, out);
    out.setDataPosition(0);

    EXPECT_EQ(1, out.readInt32());
    EXPECT_CALL(m_objMockISystemListener, System_NotifyEvent(_, _, _)).Times(AnyNumber());

    /* Call CATEGORY_POWER listener*/
    m_pSystem->AddListener(SystemConstants::CATEGORY_POWER, &m_objMockISystemListener, 0);

    in.setDataPosition(0);
    out.setDataPosition(0);
    in.writeInt32(0);
    in.writeInt32(SystemConstants::NOTIFY_BATTERY_LEVEL_CHANGED);
    in.writeInt32(15);
    in.setDataPosition(0);

    m_pSystem->NotifyData(in, out);
    out.setDataPosition(0);

    EXPECT_EQ(1, out.readInt32());
    EXPECT_CALL(m_objMockISystemListener, System_NotifyEvent(_, _, _)).Times(AnyNumber());

    /* Call CATEGORY_ALARM listener*/
    m_pSystem->AddListener(SystemConstants::CATEGORY_ALARM, &m_objMockISystemListener, 0);

    in.setDataPosition(0);
    out.setDataPosition(0);
    in.writeInt32(0);
    in.writeInt32(SystemConstants::NOTIFY_ALARM_EXPIRED);
    in.writeInt64(154536453574651);
    in.setDataPosition(0);

    m_pSystem->NotifyData(in, out);
    out.setDataPosition(0);

    EXPECT_EQ(1, out.readInt32());
    EXPECT_CALL(m_objMockISystemListener, System_NotifyEvent(_, _, _)).Times(AnyNumber());

    /* Call CATEGORY_CONFIG listener*/
    m_pSystem->AddListener(SystemConstants::CATEGORY_CONFIG, &m_objMockISystemListener, 0);

    in.setDataPosition(0);
    out.setDataPosition(0);
    in.writeInt32(0);
    in.writeInt32(SystemConstants::NOTIFY_CONFIGURATION_CHANGED);
    in.writeInt32(10);
    in.setDataPosition(0);

    m_pSystem->NotifyData(in, out);
    out.setDataPosition(0);

    EXPECT_EQ(1, out.readInt32());
    EXPECT_CALL(m_objMockISystemListener, System_NotifyEvent(_, _, _)).Times(AnyNumber());

    /* Call CATEGORY_EVENT listener*/
    m_pSystem->AddListener(SystemConstants::CATEGORY_EVENT, &m_objMockISystemListener, 0);

    in.setDataPosition(0);
    out.setDataPosition(0);
    in.writeInt32(0);
    in.writeInt32(SystemConstants::NOTIFY_EVENT);
    in.writeInt32(IMS_EVENT_ROAMING_STATE);
    in.writeInt32(IMS_ROAMING_STATE_ON);
    in.writeInt32(IMS_ROAMING_STATE_ON);
    in.setDataPosition(0);

    m_pSystem->NotifyData(in, out);
    out.setDataPosition(0);

    EXPECT_EQ(1, out.readInt32());
    EXPECT_CALL(m_objMockISystemListener, System_NotifyEvent(_, _, _)).Times(AnyNumber());

    /* Call CATEGORY_USIM listener*/
    m_pSystem->AddListener(SystemConstants::CATEGORY_USIM, &m_objMockISystemListener, 0);

    in.setDataPosition(0);
    out.setDataPosition(0);
    in.writeInt32(0);
    in.writeInt32(SystemConstants::NOTIFY_USIM_EVENT);
    in.writeInt32(OsUsim::NOTIFICATION_USIM_AUTH);
    in.writeInt32(87);
    in.writeString16(String16("response"));
    in.writeInt64(657657576575);
    in.setDataPosition(0);

    m_pSystem->NotifyData(in, out);
    out.setDataPosition(0);

    EXPECT_EQ(1, out.readInt32());
    EXPECT_CALL(m_objMockISystemListener, System_NotifyEvent(_, _, _)).Times(AnyNumber());

    m_pSystem->AddListener(SystemConstants::CATEGORY_RADIO, &m_objMockISystemListener, 0);

    /* Call Notify Data for CATEGORY_RADIO*/
    in.setDataPosition(0);
    out.setDataPosition(0);
    in.writeInt32(0);
    in.writeInt32(SystemConstants::NOTIFY_RADIO_EVENT);
    in.writeInt32(EVENT_CONNECTION_SETUP_PREPARED);
    in.writeInt32(17);
    in.setDataPosition(0);

    m_pSystem->NotifyData(in, out);
    out.setDataPosition(0);

    EXPECT_EQ(1, out.readInt32());
    EXPECT_CALL(m_objMockISystemListener, System_NotifyEvent(_, _, _)).Times(AnyNumber());

    // Testing RemoveListener
    m_pSystem->RemoveListener(SystemConstants::CATEGORY_NETWORK, null, 0);
    m_pSystem->RemoveListener(SystemConstants::CATEGORY_NETWORK, &m_objMockISystemListener, 0);

    /* Call CATEGORY_NETWORK listener*/
    in.setDataPosition(0);
    out.setDataPosition(0);
    in.writeInt32(0);
    in.writeInt32(SystemConstants::NOTIFY_SERVICE_STATE_CHANGED);
    in.writeInt32(STATE_IN_SERVICE);
    in.setDataPosition(0);

    m_pSystem->NotifyData(in, out);
    out.setDataPosition(0);

    EXPECT_EQ(1, out.readInt32());
    EXPECT_CALL(m_objMockISystemListener, System_NotifyEvent(_, _, _)).Times(0);

    /* Call CATEGORY_WIFI listener*/
    m_pSystem->RemoveListener(SystemConstants::CATEGORY_WIFI, &m_objMockISystemListener, 0);

    in.setDataPosition(0);
    out.setDataPosition(0);
    in.writeInt32(0);
    in.writeInt32(SystemConstants::NOTIFY_WIFI_STATE_CHANGED);
    in.writeInt32(WIFI_STATE_DISABLED);
    in.setDataPosition(0);

    m_pSystem->NotifyData(in, out);
    out.setDataPosition(0);

    EXPECT_EQ(1, out.readInt32());
    EXPECT_CALL(m_objMockISystemListener, System_NotifyEvent(_, _, _)).Times(0);

    /* Call CATEGORY_CALL listener*/
    m_pSystem->RemoveListener(SystemConstants::CATEGORY_CALL, &m_objMockISystemListener, 0);

    in.setDataPosition(0);
    out.setDataPosition(0);
    in.writeInt32(0);
    in.writeInt32(SystemConstants::NOTIFY_VOICE_CALL_STATE_CHANGED);
    in.writeInt32(2);
    in.setDataPosition(0);

    m_pSystem->NotifyData(in, out);
    out.setDataPosition(0);

    EXPECT_EQ(1, out.readInt32());
    EXPECT_CALL(m_objMockISystemListener, System_NotifyEvent(_, _, _)).Times(0);

    /* Call CATEGORY_POWER listener*/
    m_pSystem->RemoveListener(SystemConstants::CATEGORY_POWER, &m_objMockISystemListener, 0);

    in.setDataPosition(0);
    out.setDataPosition(0);
    in.writeInt32(0);
    in.writeInt32(SystemConstants::NOTIFY_BATTERY_LEVEL_CHANGED);
    in.writeInt32(15);
    in.setDataPosition(0);

    m_pSystem->NotifyData(in, out);
    out.setDataPosition(0);

    EXPECT_EQ(1, out.readInt32());
    EXPECT_CALL(m_objMockISystemListener, System_NotifyEvent(_, _, _)).Times(0);

    /* Call CATEGORY_ALARM listener*/
    m_pSystem->RemoveListener(SystemConstants::CATEGORY_ALARM, &m_objMockISystemListener, 0);

    in.setDataPosition(0);
    out.setDataPosition(0);
    in.writeInt32(0);
    in.writeInt32(SystemConstants::NOTIFY_ALARM_EXPIRED);
    in.writeInt64(154536453574651);
    in.setDataPosition(0);

    m_pSystem->NotifyData(in, out);
    out.setDataPosition(0);

    EXPECT_EQ(1, out.readInt32());
    EXPECT_CALL(m_objMockISystemListener, System_NotifyEvent(_, _, _)).Times(0);

    /* Call CATEGORY_CONFIG listener*/
    m_pSystem->RemoveListener(SystemConstants::CATEGORY_CONFIG, &m_objMockISystemListener, 0);

    in.setDataPosition(0);
    out.setDataPosition(0);
    in.writeInt32(0);
    in.writeInt32(SystemConstants::NOTIFY_CONFIGURATION_CHANGED);
    in.writeInt32(10);
    in.setDataPosition(0);

    m_pSystem->NotifyData(in, out);
    out.setDataPosition(0);

    EXPECT_EQ(1, out.readInt32());
    EXPECT_CALL(m_objMockISystemListener, System_NotifyEvent(_, _, _)).Times(0);

    /* Call CATEGORY_EVENT listener*/
    m_pSystem->RemoveListener(SystemConstants::CATEGORY_EVENT, &m_objMockISystemListener, 0);

    in.setDataPosition(0);
    out.setDataPosition(0);
    in.writeInt32(0);
    in.writeInt32(SystemConstants::NOTIFY_EVENT);
    in.writeInt32(IMS_EVENT_ROAMING_STATE);
    in.writeInt32(IMS_ROAMING_STATE_ON);
    in.writeInt32(IMS_ROAMING_STATE_ON);
    in.setDataPosition(0);

    m_pSystem->NotifyData(in, out);
    out.setDataPosition(0);

    EXPECT_EQ(1, out.readInt32());
    EXPECT_CALL(m_objMockISystemListener, System_NotifyEvent(_, _, _)).Times(0);

    /* Call CATEGORY_USIM listener*/
    m_pSystem->RemoveListener(SystemConstants::CATEGORY_USIM, &m_objMockISystemListener, 0);

    in.setDataPosition(0);
    out.setDataPosition(0);
    in.writeInt32(0);
    in.writeInt32(SystemConstants::NOTIFY_USIM_EVENT);
    in.writeInt32(OsUsim::NOTIFICATION_USIM_AUTH);
    in.writeInt32(87);
    in.writeString16(String16("response"));
    in.writeInt64(657657576575);
    in.setDataPosition(0);

    m_pSystem->NotifyData(in, out);
    out.setDataPosition(0);

    EXPECT_EQ(1, out.readInt32());
    EXPECT_CALL(m_objMockISystemListener, System_NotifyEvent(_, _, _)).Times(0);

    /* Call CATEGORY_RADIO listener*/
    m_pSystem->RemoveListener(SystemConstants::CATEGORY_RADIO, &m_objMockISystemListener, 0);

    in.setDataPosition(0);
    out.setDataPosition(0);
    in.writeInt32(0);
    in.writeInt32(SystemConstants::NOTIFY_RADIO_EVENT);
    in.writeInt32(EVENT_CONNECTION_SETUP_PREPARED);
    in.writeInt32(17);
    in.setDataPosition(0);

    m_pSystem->NotifyData(in, out);
    out.setDataPosition(0);

    EXPECT_EQ(1, out.readInt32());
    EXPECT_CALL(m_objMockISystemListener, System_NotifyEvent(_, _, _)).Times(0);
}

TEST_F(SystemTest, GetPowerLevel)
{
    m_pSystem->SetCallback(IMS_NULL);

    EXPECT_EQ(m_pSystem->GetPowerLevel(), 0);

    m_pSystem->SetCallback(&m_objMockSystemCallback);

    EXPECT_CALL(m_objMockSystemCallback, SendDataToJava(_, _, _))
            .Times(AnyNumber())
            .WillRepeatedly(Return(1));
    EXPECT_EQ(m_pSystem->GetPowerLevel(), 0);
}

TEST_F(SystemTest, GetDeviceId)
{
    AString strDeviceId;
    m_pSystem->SetCallback(IMS_NULL);

    EXPECT_EQ(m_pSystem->GetDeviceId(strDeviceId, 0), 0);

    m_pSystem->SetCallback(&m_objMockSystemCallback);

    EXPECT_CALL(m_objMockSystemCallback, SendDataToJava(_, _, _))
            .Times(AnyNumber())
            .WillRepeatedly(Return(1));
    EXPECT_EQ(m_pSystem->GetDeviceId(strDeviceId, 0), 1);

    EXPECT_CALL(m_objMockSystemCallback, SendDataToJava(_, _, _))
            .Times(AnyNumber())
            .WillRepeatedly(Return(0));
    EXPECT_EQ(m_pSystem->GetDeviceId(strDeviceId, 0), 0);
}

TEST_F(SystemTest, GetDeviceSoftwareVersion)
{
    AString strSv;

    EXPECT_CALL(m_objMockSystemCallback, SendDataToJava(_, _, _))
            .Times(AnyNumber())
            .WillRepeatedly(Return(1));
    EXPECT_EQ(m_pSystem->GetDeviceSoftwareVersion(strSv, 0), 1);
}

TEST_F(SystemTest, GetExternalStoragePath)
{
    AString strExternalStoragePath;

    EXPECT_CALL(m_objMockSystemCallback, SendDataToJava(_, _, _))
            .Times(AnyNumber())
            .WillRepeatedly(Return(1));
    EXPECT_EQ(m_pSystem->GetExternalStoragePath(strExternalStoragePath), 1);

    EXPECT_CALL(m_objMockSystemCallback, SendDataToJava(_, _, _))
            .Times(AnyNumber())
            .WillRepeatedly(Return(0));
    EXPECT_EQ(m_pSystem->GetExternalStoragePath(strExternalStoragePath), 0);
}

TEST_F(SystemTest, GetPhoneNumber)
{
    AString strPhoneNumber;

    EXPECT_CALL(m_objMockSystemCallback, SendDataToJava(_, _, _))
            .Times(AnyNumber())
            .WillRepeatedly(Return(1));
    EXPECT_EQ(m_pSystem->GetPhoneNumber(strPhoneNumber, 0), 1);

    EXPECT_CALL(m_objMockSystemCallback, SendDataToJava(_, _, _))
            .Times(AnyNumber())
            .WillRepeatedly(Return(0));
    EXPECT_EQ(m_pSystem->GetPhoneNumber(strPhoneNumber, 0), 0);
}

TEST_F(SystemTest, GetSubscriberId)
{
    AString strImsi;

    EXPECT_CALL(m_objMockSystemCallback, SendDataToJava(_, _, _))
            .Times(AnyNumber())
            .WillRepeatedly(Return(1));
    EXPECT_EQ(m_pSystem->GetSubscriberId(strImsi, 0), 1);

    EXPECT_CALL(m_objMockSystemCallback, SendDataToJava(_, _, _))
            .Times(AnyNumber())
            .WillRepeatedly(Return(0));
    EXPECT_EQ(m_pSystem->GetSubscriberId(strImsi, 0), 0);
}

TEST_F(SystemTest, GetMcc)
{
    AString strMcc;

    EXPECT_CALL(m_objMockSystemCallback, SendDataToJava(_, _, _))
            .Times(AnyNumber())
            .WillRepeatedly(Return(1));
    EXPECT_EQ(m_pSystem->GetMcc(strMcc, 0), 1);

    EXPECT_CALL(m_objMockSystemCallback, SendDataToJava(_, _, _))
            .Times(AnyNumber())
            .WillRepeatedly(Return(0));
    EXPECT_EQ(m_pSystem->GetMcc(strMcc, 0), 0);
}

TEST_F(SystemTest, GetMnc)
{
    AString strMnc;

    EXPECT_CALL(m_objMockSystemCallback, SendDataToJava(_, _, _))
            .Times(AnyNumber())
            .WillRepeatedly(Return(1));
    EXPECT_EQ(m_pSystem->GetMnc(strMnc, 0), 1);

    EXPECT_CALL(m_objMockSystemCallback, SendDataToJava(_, _, _))
            .Times(AnyNumber())
            .WillRepeatedly(Return(0));
    EXPECT_EQ(m_pSystem->GetMnc(strMnc, 0), 0);
}

TEST_F(SystemTest, GetOperator)
{
    AString strOperator;

    EXPECT_CALL(m_objMockSystemCallback, SendDataToJava(_, _, _))
            .Times(AnyNumber())
            .WillRepeatedly(Return(1));
    EXPECT_EQ(m_pSystem->GetOperator(strOperator, 0), 1);

    EXPECT_CALL(m_objMockSystemCallback, SendDataToJava(_, _, _))
            .Times(AnyNumber())
            .WillRepeatedly(Return(0));
    EXPECT_EQ(m_pSystem->GetOperator(strOperator, 0), 0);
}

TEST_F(SystemTest, GetCountry)
{
    AString strCountry;

    EXPECT_CALL(m_objMockSystemCallback, SendDataToJava(_, _, _))
            .Times(AnyNumber())
            .WillRepeatedly(Return(1));
    EXPECT_EQ(m_pSystem->GetCountry(strCountry, 0), 1);

    EXPECT_CALL(m_objMockSystemCallback, SendDataToJava(_, _, _))
            .Times(AnyNumber())
            .WillRepeatedly(Return(0));
    EXPECT_EQ(m_pSystem->GetCountry(strCountry, 0), 0);
}

TEST_F(SystemTest, GetNetworkCountry)
{
    AString strCountry;

    EXPECT_CALL(m_objMockSystemCallback, SendDataToJava(_, _, _))
            .Times(AnyNumber())
            .WillRepeatedly(Return(1));
    EXPECT_EQ(m_pSystem->GetNetworkCountry(strCountry, 0), 1);

    EXPECT_CALL(m_objMockSystemCallback, SendDataToJava(_, _, _))
            .Times(AnyNumber())
            .WillRepeatedly(Return(0));
    EXPECT_EQ(m_pSystem->GetNetworkCountry(strCountry, 0), 0);
}

TEST_F(SystemTest, GetEmergencyNumberListFromSim)
{
    AString strEnlFromSim;

    EXPECT_CALL(m_objMockSystemCallback, SendDataToJava(_, _, _))
            .Times(AnyNumber())
            .WillRepeatedly(Return(1));
    EXPECT_EQ(m_pSystem->GetEmergencyNumberListFromSim(strEnlFromSim, 0), 1);

    EXPECT_CALL(m_objMockSystemCallback, SendDataToJava(_, _, _))
            .Times(AnyNumber())
            .WillRepeatedly(Return(0));
    EXPECT_EQ(m_pSystem->GetEmergencyNumberListFromSim(strEnlFromSim, 0), 0);
}

TEST_F(SystemTest, GetEmergencyPriorityFromModem)
{
    m_pSystem->SetCallback(IMS_NULL);

    EXPECT_EQ(m_pSystem->GetEmergencyPriorityFromModem(0), 1);

    m_pSystem->SetCallback(&m_objMockSystemCallback);

    EXPECT_CALL(m_objMockSystemCallback, SendDataToJava(_, _, _))
            .Times(AnyNumber())
            .WillRepeatedly(Return(1));
    EXPECT_EQ(m_pSystem->GetEmergencyPriorityFromModem(0), 0);
}

TEST_F(SystemTest, IsUiccGbaSupported)
{
    EXPECT_CALL(m_objMockSystemCallback, SendDataToJava(_, _, _))
            .Times(AnyNumber())
            .WillRepeatedly(Return(1));
    EXPECT_EQ(m_pSystem->IsUiccGbaSupported(0), 0);
}

TEST_F(SystemTest, GetIsimState)
{
    AString strIsimState(AString::ConstNull());

    EXPECT_EQ(m_pSystem->GetIsimState(0), strIsimState);

    AString strValue("LOADED");
    String16 str16(strValue.GetStr(), strValue.GetLength());

    EXPECT_CALL(m_objMockSystemCallback, SendDataToJava(_, _, _))
            .Times(1)
            .WillOnce(Invoke(
                    [str16](Unused, android::Parcel& out, Unused)
                    {
                        out.writeString16(str16);
                        out.setDataPosition(0);
                        return 1;
                    }));
    EXPECT_EQ(m_pSystem->GetIsimState(0), strValue);
}

TEST_F(SystemTest, ReadIsimFileAttributes)
{
    m_pSystem->SetCallback(IMS_NULL);

    EXPECT_EQ(m_pSystem->ReadIsimFileAttributes(OsIsim::EF_ID_IMPU, 0), IMS_FAILURE);

    m_pSystem->SetCallback(&m_objMockSystemCallback);

    EXPECT_CALL(m_objMockSystemCallback, SendDataToJava(_, _, _))
            .Times(AnyNumber())
            .WillRepeatedly(Return(1));
    EXPECT_EQ(m_pSystem->ReadIsimFileAttributes(OsIsim::EF_ID_IMPI, 0), IMS_SUCCESS);

    EXPECT_CALL(m_objMockSystemCallback, SendDataToJava(_, _, _))
            .Times(AnyNumber())
            .WillRepeatedly(Return(0));
    EXPECT_EQ(m_pSystem->ReadIsimFileAttributes(OsIsim::EF_ID_IMPU, 0), IMS_FAILURE);
}

TEST_F(SystemTest, ReadIsimRecord)
{
    m_pSystem->SetCallback(IMS_NULL);

    EXPECT_EQ(m_pSystem->ReadIsimRecord(OsIsim::EF_ID_IMPI, 1, 0), IMS_FAILURE);

    m_pSystem->SetCallback(&m_objMockSystemCallback);

    EXPECT_CALL(m_objMockSystemCallback, SendDataToJava(_, _, _))
            .Times(AnyNumber())
            .WillRepeatedly(Return(1));
    EXPECT_EQ(m_pSystem->ReadIsimRecord(OsIsim::EF_ID_IMPU, 1, 0), IMS_SUCCESS);

    EXPECT_CALL(m_objMockSystemCallback, SendDataToJava(_, _, _))
            .Times(AnyNumber())
            .WillRepeatedly(Return(0));
    EXPECT_EQ(m_pSystem->ReadIsimRecord(OsIsim::EF_ID_DOMAIN, 1, 0), IMS_FAILURE);
}

TEST_F(SystemTest, RequestIsimAuthentication)
{
    AString strNonce("NjY4NWE2N2E=");
    m_pSystem->SetCallback(IMS_NULL);

    EXPECT_EQ(m_pSystem->RequestIsimAuthentication(strNonce, 1, 0), IMS_FAILURE);

    m_pSystem->SetCallback(&m_objMockSystemCallback);

    EXPECT_CALL(m_objMockSystemCallback, SendDataToJava(_, _, _))
            .Times(AnyNumber())
            .WillRepeatedly(Return(1));
    EXPECT_EQ(m_pSystem->RequestIsimAuthentication(strNonce, 1, 0), IMS_SUCCESS);

    EXPECT_CALL(m_objMockSystemCallback, SendDataToJava(_, _, _))
            .Times(AnyNumber())
            .WillRepeatedly(Return(0));
    EXPECT_EQ(m_pSystem->RequestIsimAuthentication(strNonce, 1, 0), IMS_FAILURE);
}

TEST_F(SystemTest, RequestUsimAuthentication)
{
    AString strNonce("NjY4NWE2N2E=");
    m_pSystem->SetCallback(IMS_NULL);

    EXPECT_EQ(m_pSystem->RequestUsimAuthentication(strNonce, 1, 0), IMS_FAILURE);

    m_pSystem->SetCallback(&m_objMockSystemCallback);

    EXPECT_CALL(m_objMockSystemCallback, SendDataToJava(_, _, _))
            .Times(AnyNumber())
            .WillRepeatedly(Return(1));
    EXPECT_EQ(m_pSystem->RequestUsimAuthentication(strNonce, 1, 0), IMS_SUCCESS);

    EXPECT_CALL(m_objMockSystemCallback, SendDataToJava(_, _, _))
            .Times(AnyNumber())
            .WillRepeatedly(Return(0));
    EXPECT_EQ(m_pSystem->RequestUsimAuthentication(strNonce, 1, 0), IMS_FAILURE);
}

TEST_F(SystemTest, GetCallState)
{
    EXPECT_CALL(m_objMockSystemCallback, SendDataToJava(_, _, _))
            .Times(AnyNumber())
            .WillRepeatedly(Return(1));
    EXPECT_EQ(m_pSystem->GetCallState(0), 0);
}

TEST_F(SystemTest, IsEmergencyNumber)
{
    AString strNumber;
    m_pSystem->SetCallback(IMS_NULL);

    EXPECT_EQ(m_pSystem->IsEmergencyNumber(strNumber, 0), 0);

    m_pSystem->SetCallback(&m_objMockSystemCallback);

    EXPECT_CALL(m_objMockSystemCallback, SendDataToJava(_, _, _))
            .Times(AnyNumber())
            .WillRepeatedly(Return(1));
    EXPECT_EQ(m_pSystem->IsEmergencyNumber(strNumber, 0), 0);
}

TEST_F(SystemTest, GetTtyMode)
{
    EXPECT_CALL(m_objMockSystemCallback, SendDataToJava(_, _, _))
            .Times(AnyNumber())
            .WillRepeatedly(Return(1));
    EXPECT_EQ(m_pSystem->GetTtyMode(0), 0);
}

TEST_F(SystemTest, GetRttMode)
{
    EXPECT_CALL(m_objMockSystemCallback, SendDataToJava(_, _, _))
            .Times(AnyNumber())
            .WillRepeatedly(Return(1));
    EXPECT_EQ(m_pSystem->GetRttMode(0), 0);
}

TEST_F(SystemTest, GetCallStateInOtherSlot)
{
    EXPECT_CALL(m_objMockSystemCallback, SendDataToJava(_, _, _))
            .Times(AnyNumber())
            .WillRepeatedly(Return(1));
    EXPECT_EQ(m_pSystem->GetCallStateInOtherSlot(0), 0);
}

TEST_F(SystemTest, GetDeviceName)
{
    AString strName;

    EXPECT_CALL(m_objMockSystemCallback, SendDataToJava(_, _, _))
            .Times(AnyNumber())
            .WillRepeatedly(Return(1));
    EXPECT_EQ(m_pSystem->GetDeviceName(strName), 1);

    EXPECT_CALL(m_objMockSystemCallback, SendDataToJava(_, _, _))
            .Times(AnyNumber())
            .WillRepeatedly(Return(0));
    EXPECT_EQ(m_pSystem->GetDeviceName(strName), 0);
}

TEST_F(SystemTest, GetDigestSha1)
{
    AString strIn("/files/client.pem");
    AString strOut;

    m_pSystem->SetCallback(IMS_NULL);

    EXPECT_EQ(m_pSystem->GetDigestSha1(strIn, strOut), 0);

    m_pSystem->SetCallback(&m_objMockSystemCallback);

    EXPECT_CALL(m_objMockSystemCallback, SendDataToJava(_, _, _))
            .Times(AnyNumber())
            .WillRepeatedly(Return(1));
    EXPECT_EQ(m_pSystem->GetDigestSha1(strIn, strOut), 1);

    EXPECT_CALL(m_objMockSystemCallback, SendDataToJava(_, _, _))
            .Times(AnyNumber())
            .WillRepeatedly(Return(0));
    EXPECT_EQ(m_pSystem->GetDigestSha1(strIn, strOut), 0);
}

TEST_F(SystemTest, GetNetworkType)
{
    EXPECT_CALL(m_objMockSystemCallback, SendDataToJava(_, _, _))
            .Times(AnyNumber())
            .WillRepeatedly(Return(1));
    EXPECT_EQ(m_pSystem->GetNetworkType(0), 0);
}

TEST_F(SystemTest, GetVoiceNetworkType)
{
    EXPECT_CALL(m_objMockSystemCallback, SendDataToJava(_, _, _))
            .Times(AnyNumber())
            .WillRepeatedly(Return(1));
    EXPECT_EQ(m_pSystem->GetVoiceNetworkType(0), 0);
}

TEST_F(SystemTest, GetRoamingState)
{
    EXPECT_CALL(m_objMockSystemCallback, SendDataToJava(_, _, _))
            .Times(AnyNumber())
            .WillRepeatedly(Return(1));
    EXPECT_EQ(m_pSystem->GetRoamingState(0), 0);
}

TEST_F(SystemTest, GetVoiceRoamingType)
{
    EXPECT_CALL(m_objMockSystemCallback, SendDataToJava(_, _, _))
            .Times(AnyNumber())
            .WillRepeatedly(Return(1));
    EXPECT_EQ(m_pSystem->GetVoiceRoamingType(0), 0);
}

TEST_F(SystemTest, GetDataRoamingType)
{
    EXPECT_CALL(m_objMockSystemCallback, SendDataToJava(_, _, _))
            .Times(AnyNumber())
            .WillRepeatedly(Return(1));
    EXPECT_EQ(m_pSystem->GetDataRoamingType(0), 0);
}

TEST_F(SystemTest, GetServiceState)
{
    EXPECT_CALL(m_objMockSystemCallback, SendDataToJava(_, _, _))
            .Times(AnyNumber())
            .WillRepeatedly(Return(1));
    EXPECT_EQ(m_pSystem->GetServiceState(0), 0);
}

TEST_F(SystemTest, GetVoiceServiceState)
{
    EXPECT_CALL(m_objMockSystemCallback, SendDataToJava(_, _, _))
            .Times(AnyNumber())
            .WillRepeatedly(Return(1));
    EXPECT_EQ(m_pSystem->GetVoiceServiceState(0), 0);

    EXPECT_CALL(m_objMockSystemCallback, SendDataToJava(_, _, _))
            .Times(AnyNumber())
            .WillRepeatedly(Return(0));
    EXPECT_EQ(m_pSystem->GetVoiceServiceState(0), 0);
}

TEST_F(SystemTest, GetAccessNetworkInfo)
{
    AStringArray strOut;
    IMS_SINT32 nNetworkType;

    m_pSystem->SetCallback(IMS_NULL);

    EXPECT_EQ(m_pSystem->GetAccessNetworkInfo(IIpcan::TYPE_LTE, nNetworkType, strOut, 0), 0);

    AString strValue("10.229.89.23");
    String16 str16(strValue.GetStr(), strValue.GetLength());

    m_pSystem->SetCallback(&m_objMockSystemCallback);

    EXPECT_CALL(m_objMockSystemCallback, SendDataToJava(_, _, _))
            .Times(1)
            .WillOnce(Invoke(
                    [str16](Unused, android::Parcel& out, Unused)
                    {
                        out.writeInt32(IIpcan::TYPE_NR);
                        out.writeInt32(1);
                        out.writeString16(str16);
                        out.setDataPosition(0);
                        return 1;
                    }));
    EXPECT_EQ(m_pSystem->GetAccessNetworkInfo(IIpcan::TYPE_LTE, nNetworkType, strOut, 0), 1);
    EXPECT_EQ(strOut.GetFirstElement(), strValue);
    EXPECT_EQ(nNetworkType, IIpcan::TYPE_NR);

    EXPECT_CALL(m_objMockSystemCallback, SendDataToJava(_, _, _))
            .Times(AnyNumber())
            .WillRepeatedly(Return(0));
    EXPECT_EQ(m_pSystem->GetAccessNetworkInfo(IIpcan::TYPE_NR, nNetworkType, strOut, 0), 0);
}

TEST_F(SystemTest, GetLastAccessNetworkInfo)
{
    IMS_SINT32 nNetworkType = IIpcan::TYPE_LTE;
    AString strOut;

    m_pSystem->SetCallback(IMS_NULL);

    EXPECT_EQ(m_pSystem->GetLastAccessNetworkInfo(nNetworkType, 0).GetFirstElement(), strOut);
    EXPECT_CALL(m_objMockSystemCallback, SendDataToJava(_, _, _)).Times(0);

    AString strAddress("10.221.45.75");
    String16 str16(strAddress.GetStr(), strAddress.GetLength());

    m_pSystem->SetCallback(&m_objMockSystemCallback);

    EXPECT_CALL(m_objMockSystemCallback, SendDataToJava(_, _, _))
            .Times(1)
            .WillOnce(Invoke(
                    [str16](Unused, android::Parcel& out, Unused)
                    {
                        out.writeInt32(1);
                        out.writeString16(str16);
                        out.setDataPosition(0);
                        return 1;
                    }));
    EXPECT_EQ(m_pSystem->GetLastAccessNetworkInfo(nNetworkType, 0).GetFirstElement(), strAddress);

    EXPECT_CALL(m_objMockSystemCallback, SendDataToJava(_, _, _))
            .Times(AnyNumber())
            .WillRepeatedly(Return(0));
    EXPECT_EQ(m_pSystem->GetLastAccessNetworkInfo(nNetworkType, 0).GetFirstElement(), strOut);
}

TEST_F(SystemTest, GetMocnPlmnInfo)
{
    EXPECT_CALL(m_objMockSystemCallback, SendDataToJava(_, _, _))
            .Times(AnyNumber())
            .WillRepeatedly(Return(1));
    EXPECT_EQ(m_pSystem->GetMocnPlmnInfo(0), 0);
}

TEST_F(SystemTest, GetMtu)
{
    m_pSystem->SetCallback(IMS_NULL);

    EXPECT_EQ(m_pSystem->GetMtu(NetworkPolicy::APN_IMS, 0), 0);

    m_pSystem->SetCallback(&m_objMockSystemCallback);

    EXPECT_CALL(m_objMockSystemCallback, SendDataToJava(_, _, _))
            .Times(AnyNumber())
            .WillRepeatedly(Return(1));
    EXPECT_EQ(m_pSystem->GetMtu(NetworkPolicy::APN_IMS, 0), 0);

    EXPECT_CALL(m_objMockSystemCallback, SendDataToJava(_, _, _))
            .Times(AnyNumber())
            .WillRepeatedly(Return(0));
    EXPECT_EQ(m_pSystem->GetMtu(NetworkPolicy::APN_IMS, 0), 0);
}

TEST_F(SystemTest, BindSocket)
{
    m_pSystem->SetCallback(IMS_NULL);

    EXPECT_EQ(m_pSystem->BindSocket(NetworkPolicy::APN_IMS, 120, 0), 0);

    m_pSystem->SetCallback(&m_objMockSystemCallback);

    EXPECT_CALL(m_objMockSystemCallback, SendDataToJava(_, _, _))
            .Times(AnyNumber())
            .WillRepeatedly(Return(1));
    EXPECT_EQ(m_pSystem->BindSocket(NetworkPolicy::APN_IMS, 120, 0), 0);

    EXPECT_CALL(m_objMockSystemCallback, SendDataToJava(_, _, _))
            .Times(AnyNumber())
            .WillRepeatedly(Return(0));
    EXPECT_EQ(m_pSystem->BindSocket(NetworkPolicy::APN_IMS, 120, 0), 0);
}

TEST_F(SystemTest, IsImsEmergencyCallSupported)
{
    EXPECT_CALL(m_objMockSystemCallback, SendDataToJava(_, _, _))
            .Times(AnyNumber())
            .WillRepeatedly(Return(1));
    EXPECT_EQ(m_pSystem->IsImsEmergencyCallSupported(0), 0);
}

TEST_F(SystemTest, IsLteEmergencyOnly)
{
    EXPECT_CALL(m_objMockSystemCallback, SendDataToJava(_, _, _))
            .Times(AnyNumber())
            .WillRepeatedly(Return(1));
    EXPECT_EQ(m_pSystem->IsLteEmergencyOnly(0), 0);
}

TEST_F(SystemTest, IsEmergencyAttachSupported)
{
    EXPECT_CALL(m_objMockSystemCallback, SendDataToJava(_, _, _))
            .Times(AnyNumber())
            .WillRepeatedly(Return(1));
    EXPECT_EQ(m_pSystem->IsEmergencyAttachSupported(0), 0);
}

TEST_F(SystemTest, IsMobileDataEnabled)
{
    EXPECT_CALL(m_objMockSystemCallback, SendDataToJava(_, _, _))
            .Times(AnyNumber())
            .WillRepeatedly(Return(1));
    EXPECT_EQ(m_pSystem->IsMobileDataEnabled(0), 0);
}

TEST_F(SystemTest, IsImsVoiceCallSupported)
{
    EXPECT_CALL(m_objMockSystemCallback, SendDataToJava(_, _, _))
            .Times(AnyNumber())
            .WillRepeatedly(Return(1));
    EXPECT_EQ(m_pSystem->IsImsVoiceCallSupported(0), 0);
}

TEST_F(SystemTest, ActivateDataConnection)
{
    m_pSystem->SetCallback(IMS_NULL);

    EXPECT_EQ(m_pSystem->ActivateDataConnection(NetworkPolicy::APN_IMS, 0), 0);

    m_pSystem->SetCallback(&m_objMockSystemCallback);

    EXPECT_CALL(m_objMockSystemCallback, SendDataToJava(_, _, _))
            .Times(AnyNumber())
            .WillRepeatedly(Return(1));
    EXPECT_EQ(m_pSystem->ActivateDataConnection(NetworkPolicy::APN_IMS, 0), 0);

    EXPECT_CALL(m_objMockSystemCallback, SendDataToJava(_, _, _))
            .Times(AnyNumber())
            .WillRepeatedly(Return(0));
    EXPECT_EQ(m_pSystem->ActivateDataConnection(NetworkPolicy::APN_IMS, 0), 0);
}

TEST_F(SystemTest, DeactivateDataConnection)
{
    m_pSystem->SetCallback(IMS_NULL);

    EXPECT_EQ(m_pSystem->DeactivateDataConnection(NetworkPolicy::APN_IMS, 0), 0);

    m_pSystem->SetCallback(&m_objMockSystemCallback);

    EXPECT_CALL(m_objMockSystemCallback, SendDataToJava(_, _, _))
            .Times(AnyNumber())
            .WillRepeatedly(Return(1));
    EXPECT_EQ(m_pSystem->DeactivateDataConnection(NetworkPolicy::APN_IMS, 0), 0);

    EXPECT_CALL(m_objMockSystemCallback, SendDataToJava(_, _, _))
            .Times(AnyNumber())
            .WillRepeatedly(Return(0));
    EXPECT_EQ(m_pSystem->DeactivateDataConnection(NetworkPolicy::APN_IMS, 0), 0);
}

TEST_F(SystemTest, GetApnName)
{
    AString strOut;

    m_pSystem->SetCallback(IMS_NULL);

    EXPECT_EQ(m_pSystem->GetApnName(NetworkPolicy::APN_IMS, 0), strOut);

    AString strValue("IMS");
    String16 str16(strValue.GetStr(), strValue.GetLength());

    m_pSystem->SetCallback(&m_objMockSystemCallback);

    EXPECT_CALL(m_objMockSystemCallback, SendDataToJava(_, _, _))
            .Times(1)
            .WillOnce(Invoke(
                    [str16](Unused, android::Parcel& out, Unused)
                    {
                        out.writeString16(str16);
                        out.setDataPosition(0);
                        return 1;
                    }));
    EXPECT_EQ(m_pSystem->GetApnName(NetworkPolicy::APN_IMS, 0), strValue);

    EXPECT_CALL(m_objMockSystemCallback, SendDataToJava(_, _, _))
            .Times(AnyNumber())
            .WillRepeatedly(Return(0));
    EXPECT_EQ(m_pSystem->GetApnName(NetworkPolicy::APN_IMS, 0), strOut);
}

TEST_F(SystemTest, GetDataConnectionState)
{
    EXPECT_CALL(m_objMockSystemCallback, SendDataToJava(_, _, _))
            .Times(AnyNumber())
            .WillRepeatedly(Return(1));
    EXPECT_EQ(m_pSystem->GetDataConnectionState(NetworkPolicy::APN_IMS, 0), 0);

    EXPECT_CALL(m_objMockSystemCallback, SendDataToJava(_, _, _))
            .Times(AnyNumber())
            .WillRepeatedly(Return(0));
    EXPECT_EQ(m_pSystem->GetDataConnectionState(NetworkPolicy::APN_IMS, 0), 0);
}

TEST_F(SystemTest, GetHostByName)
{
    AString strHost("hostName");
    AString strOut;

    EXPECT_EQ(m_pSystem->GetHostByName(strHost, 0, NetworkPolicy::APN_IMS, 0).GetFirstElement(),
            strOut);
    EXPECT_CALL(m_objMockSystemCallback, SendDataToJava(_, _, _)).Times(0);

    AString strAddress("10.221.45.75");
    String16 str16(strAddress.GetStr(), strAddress.GetLength());

    EXPECT_CALL(m_objMockSystemCallback, SendDataToJava(_, _, _))
            .Times(1)
            .WillOnce(Invoke(
                    [str16](Unused, android::Parcel& out, Unused)
                    {
                        out.writeInt32(1);
                        out.writeString16(str16);
                        out.setDataPosition(0);
                        return 1;
                    }));
    EXPECT_EQ(m_pSystem->GetHostByName(strHost, 0, NetworkPolicy::APN_IMS, 0).GetFirstElement(),
            strAddress);

    EXPECT_CALL(m_objMockSystemCallback, SendDataToJava(_, _, _))
            .Times(AnyNumber())
            .WillRepeatedly(Return(0));
    EXPECT_EQ(m_pSystem->GetHostByName(strHost, 0, NetworkPolicy::APN_IMS, 0).GetFirstElement(),
            strOut);
}

TEST_F(SystemTest, GetIfaceId)
{
    EXPECT_CALL(m_objMockSystemCallback, SendDataToJava(_, _, _))
            .Times(AnyNumber())
            .WillRepeatedly(Return(1));
    EXPECT_EQ(m_pSystem->GetIfaceId(NetworkPolicy::APN_IMS, 0), 0);
}

TEST_F(SystemTest, GetIfaceName)
{
    AString strOut;

    m_pSystem->SetCallback(IMS_NULL);

    EXPECT_EQ(m_pSystem->GetIfaceName(NetworkPolicy::APN_IMS, 0), strOut);

    AString strValue("IMS");
    String16 str16(strValue.GetStr(), strValue.GetLength());

    m_pSystem->SetCallback(&m_objMockSystemCallback);

    EXPECT_CALL(m_objMockSystemCallback, SendDataToJava(_, _, _))
            .Times(1)
            .WillOnce(Invoke(
                    [str16](Unused, android::Parcel& out, Unused)
                    {
                        out.writeString16(str16);
                        out.setDataPosition(0);
                        return 1;
                    }));
    EXPECT_EQ(m_pSystem->GetIfaceName(NetworkPolicy::APN_IMS, 0), strValue);

    EXPECT_CALL(m_objMockSystemCallback, SendDataToJava(_, _, _))
            .Times(AnyNumber())
            .WillRepeatedly(Return(0));
    EXPECT_EQ(m_pSystem->GetIfaceName(NetworkPolicy::APN_IMS, 0), strOut);
}

TEST_F(SystemTest, GetIpcanCategory)
{
    EXPECT_CALL(m_objMockSystemCallback, SendDataToJava(_, _, _))
            .Times(AnyNumber())
            .WillRepeatedly(Return(1));
    EXPECT_EQ(m_pSystem->GetIpcanCategory(NetworkPolicy::APN_IMS, 0), 0);
}

TEST_F(SystemTest, GetLocalAddress)
{
    AString strOut;

    m_pSystem->SetCallback(IMS_NULL);

    EXPECT_EQ(m_pSystem->GetLocalAddress(NetworkPolicy::APN_IMS, 0, 0), strOut);

    AString strValue("10.221.67.98");
    String16 str16(strValue.GetStr(), strValue.GetLength());

    m_pSystem->SetCallback(&m_objMockSystemCallback);

    EXPECT_CALL(m_objMockSystemCallback, SendDataToJava(_, _, _))
            .Times(1)
            .WillOnce(Invoke(
                    [str16](Unused, android::Parcel& out, Unused)
                    {
                        out.writeString16(str16);
                        out.setDataPosition(0);
                        return 1;
                    }));
    EXPECT_EQ(m_pSystem->GetLocalAddress(NetworkPolicy::APN_IMS, 0, 0), strValue);

    EXPECT_CALL(m_objMockSystemCallback, SendDataToJava(_, _, _))
            .Times(AnyNumber())
            .WillRepeatedly(Return(0));
    EXPECT_EQ(m_pSystem->GetLocalAddress(NetworkPolicy::APN_IMS, 0, 0), strOut);
}

TEST_F(SystemTest, GetPcscfAddresses)
{
    AString strOut;

    m_pSystem->SetCallback(IMS_NULL);

    EXPECT_EQ(m_pSystem->GetPcscfAddresses(NetworkPolicy::APN_IMS, 0, 0).GetFirstElement(), strOut);
    EXPECT_CALL(m_objMockSystemCallback, SendDataToJava(_, _, _)).Times(0);

    AString strAddress("10.221.45.75");
    String16 str16(strAddress.GetStr(), strAddress.GetLength());

    m_pSystem->SetCallback(&m_objMockSystemCallback);

    EXPECT_CALL(m_objMockSystemCallback, SendDataToJava(_, _, _))
            .Times(1)
            .WillOnce(Invoke(
                    [str16](Unused, android::Parcel& out, Unused)
                    {
                        out.writeInt32(1);
                        out.writeString16(str16);
                        out.setDataPosition(0);
                        return 1;
                    }));
    EXPECT_EQ(m_pSystem->GetPcscfAddresses(NetworkPolicy::APN_IMS, 0, 0).GetFirstElement(),
            strAddress);

    EXPECT_CALL(m_objMockSystemCallback, SendDataToJava(_, _, _))
            .Times(AnyNumber())
            .WillRepeatedly(Return(0));
    EXPECT_EQ(m_pSystem->GetPcscfAddresses(NetworkPolicy::APN_IMS, 0, 0).GetFirstElement(), strOut);
}

TEST_F(SystemTest, GetWifiBssId)
{
    AString strValue("d8:c7:c8:cc:43:24");
    String16 str16(strValue.GetStr(), strValue.GetLength());

    EXPECT_CALL(m_objMockSystemCallback, SendDataToJava(_, _, _))
            .Times(1)
            .WillOnce(Invoke(
                    [str16](Unused, android::Parcel& out, Unused)
                    {
                        out.writeString16(str16);
                        out.setDataPosition(0);
                        return 1;
                    }));
    EXPECT_EQ(m_pSystem->GetWifiBssId(), strValue);
}

TEST_F(SystemTest, GetWifiConnectionState)
{
    EXPECT_CALL(m_objMockSystemCallback, SendDataToJava(_, _, _))
            .Times(AnyNumber())
            .WillRepeatedly(Return(1));
    EXPECT_EQ(m_pSystem->GetWifiConnectionState(), 0);
}

TEST_F(SystemTest, GetWifiState)
{
    EXPECT_CALL(m_objMockSystemCallback, SendDataToJava(_, _, _))
            .Times(AnyNumber())
            .WillRepeatedly(Return(1));
    EXPECT_EQ(m_pSystem->GetWifiState(), 0);
}

TEST_F(SystemTest, GetWifiSsId)
{
    AString strValue("linksys001");
    String16 str16(strValue.GetStr(), strValue.GetLength());

    EXPECT_CALL(m_objMockSystemCallback, SendDataToJava(_, _, _))
            .Times(1)
            .WillOnce(Invoke(
                    [str16](Unused, android::Parcel& out, Unused)
                    {
                        out.writeString16(str16);
                        out.setDataPosition(0);
                        return 1;
                    }));
    EXPECT_EQ(m_pSystem->GetWifiSsId(), strValue);
}

TEST_F(SystemTest, SetAlarm)
{
    m_pSystem->SetCallback(IMS_NULL);

    EXPECT_EQ(m_pSystem->SetAlarm(1000, INT64_TO_UINTP(232432423)), 0);

    m_pSystem->SetCallback(&m_objMockSystemCallback);

    EXPECT_CALL(m_objMockSystemCallback, SendDataToJava(_, _, _))
            .Times(AnyNumber())
            .WillRepeatedly(Return(1));
    EXPECT_EQ(m_pSystem->SetAlarm(2000, INT64_TO_UINTP(232432423334)), 0);

    EXPECT_CALL(m_objMockSystemCallback, SendDataToJava(_, _, _))
            .Times(AnyNumber())
            .WillRepeatedly(Return(0));
    EXPECT_EQ(m_pSystem->SetAlarm(2000, INT64_TO_UINTP(232432423334)), 0);
}

TEST_F(SystemTest, KillAlarm)
{
    m_pSystem->SetCallback(IMS_NULL);

    EXPECT_EQ(m_pSystem->KillAlarm(INT64_TO_UINTP(232432423)), 0);

    m_pSystem->SetCallback(&m_objMockSystemCallback);

    EXPECT_CALL(m_objMockSystemCallback, SendDataToJava(_, _, _))
            .Times(AnyNumber())
            .WillRepeatedly(Return(1));
    EXPECT_EQ(m_pSystem->KillAlarm(INT64_TO_UINTP(232432423334)), 0);

    EXPECT_CALL(m_objMockSystemCallback, SendDataToJava(_, _, _))
            .Times(AnyNumber())
            .WillRepeatedly(Return(0));
    EXPECT_EQ(m_pSystem->KillAlarm(INT64_TO_UINTP(232432423334)), 0);
}

TEST_F(SystemTest, GetPreference)
{
    AString strFileName("impu_list");
    AString strKey("imsi");
    AString strValue;

    m_pSystem->SetCallback(IMS_NULL);

    EXPECT_EQ(m_pSystem->GetPreference(strFileName, strKey, PREFERENCE_VALUE_LONG, 0, strValue), 0);

    m_pSystem->SetCallback(&m_objMockSystemCallback);

    EXPECT_CALL(m_objMockSystemCallback, SendDataToJava(_, _, _))
            .Times(AnyNumber())
            .WillRepeatedly(Return(1));
    EXPECT_EQ(m_pSystem->GetPreference(strFileName, strKey, PREFERENCE_VALUE_LONG, 0, strValue), 1);

    EXPECT_CALL(m_objMockSystemCallback, SendDataToJava(_, _, _))
            .Times(AnyNumber())
            .WillRepeatedly(Return(0));
    EXPECT_EQ(m_pSystem->GetPreference(strFileName, strKey, PREFERENCE_VALUE_LONG, 0, strValue), 0);
}

TEST_F(SystemTest, SetPreference)
{
    AString strFileName("impu_list");
    AString strKey("imsi");
    AString strValue("234159556188095");

    m_pSystem->SetCallback(IMS_NULL);

    EXPECT_EQ(m_pSystem->SetPreference(strFileName, strKey, PREFERENCE_VALUE_LONG, strValue, 0), 0);

    m_pSystem->SetCallback(&m_objMockSystemCallback);

    EXPECT_CALL(m_objMockSystemCallback, SendDataToJava(_, _, _))
            .Times(AnyNumber())
            .WillRepeatedly(Return(1));
    EXPECT_EQ(m_pSystem->SetPreference(strFileName, strKey, PREFERENCE_VALUE_LONG, strValue, 0), 0);

    EXPECT_CALL(m_objMockSystemCallback, SendDataToJava(_, _, _))
            .Times(AnyNumber())
            .WillRepeatedly(Return(0));
    EXPECT_EQ(m_pSystem->SetPreference(strFileName, strKey, PREFERENCE_VALUE_LONG, strValue, 0), 0);
}

TEST_F(SystemTest, GetPrivateProperty)
{
    AString strOut;
    AString strKey(ImsPrivateProperties::Persistent::KEY_TEST_LOG_OPTIONS);

    m_pSystem->SetCallback(IMS_NULL);

    EXPECT_EQ(m_pSystem->GetPrivateProperty(IMS_TRUE, strKey, 0), strOut);

    AString strValue("0x0001000F");
    String16 str16(strValue.GetStr(), strValue.GetLength());

    m_pSystem->SetCallback(&m_objMockSystemCallback);

    EXPECT_CALL(m_objMockSystemCallback, SendDataToJava(_, _, _))
            .Times(1)
            .WillOnce(Invoke(
                    [str16](Unused, android::Parcel& out, Unused)
                    {
                        out.writeString16(str16);
                        out.setDataPosition(0);
                        return 1;
                    }));
    EXPECT_EQ(m_pSystem->GetPrivateProperty(IMS_TRUE, strKey, 0), strValue);

    EXPECT_CALL(m_objMockSystemCallback, SendDataToJava(_, _, _))
            .Times(AnyNumber())
            .WillRepeatedly(Return(0));
    EXPECT_EQ(m_pSystem->GetPrivateProperty(IMS_TRUE, strKey, 0), strOut);
}

TEST_F(SystemTest, SetPrivateProperty)
{
    AString strKey(ImsPrivateProperties::Persistent::KEY_TEST_LOG_OPTIONS);
    AString strValue("0x00010000");

    m_pSystem->SetCallback(IMS_NULL);

    EXPECT_EQ(m_pSystem->SetPrivateProperty(IMS_TRUE, strKey, strValue, 0), 0);

    m_pSystem->SetCallback(&m_objMockSystemCallback);

    EXPECT_CALL(m_objMockSystemCallback, SendDataToJava(_, _, _))
            .Times(AnyNumber())
            .WillRepeatedly(Return(1));

    EXPECT_EQ(m_pSystem->SetPrivateProperty(IMS_TRUE, strKey, strValue, 0), 1);

    EXPECT_CALL(m_objMockSystemCallback, SendDataToJava(_, _, _))
            .Times(AnyNumber())
            .WillRepeatedly(Return(0));
    EXPECT_EQ(m_pSystem->SetPrivateProperty(IMS_TRUE, strKey, strValue, 0), 0);
}

TEST_F(SystemTest, GetCarrierConfig)
{
    OsParcel objConfig;

    m_pSystem->SetCallback(IMS_NULL);

    EXPECT_EQ(m_pSystem->GetCarrierConfig(0, objConfig), IMS_FALSE);

    m_pSystem->SetCallback(&m_objMockSystemCallback);

    EXPECT_CALL(m_objMockSystemCallback, SendDataToJava(_, _, _))
            .Times(AnyNumber())
            .WillRepeatedly(Return(1));

    EXPECT_EQ(m_pSystem->GetCarrierConfig(0, objConfig), 0);

    EXPECT_CALL(m_objMockSystemCallback, SendDataToJava(_, _, _))
            .Times(AnyNumber())
            .WillRepeatedly(Return(0));
    EXPECT_EQ(m_pSystem->GetCarrierConfig(0, objConfig), IMS_FALSE);
}

TEST_F(SystemTest, SendEvent)
{
    m_pSystem->SetCallback(IMS_NULL);

    EXPECT_EQ(m_pSystem->SendEvent(SystemConstants::NOTIFY_USIM_EVENT, 0, 0, 0), 0);

    m_pSystem->SetCallback(&m_objMockSystemCallback);

    EXPECT_CALL(m_objMockSystemCallback, SendDataToJava(_, _, _))
            .Times(AnyNumber())
            .WillRepeatedly(Return(1));

    EXPECT_EQ(m_pSystem->SendEvent(SystemConstants::NOTIFY_USIM_EVENT, 0, 0, 0), 0);

    EXPECT_CALL(m_objMockSystemCallback, SendDataToJava(_, _, _))
            .Times(AnyNumber())
            .WillRepeatedly(Return(0));
    EXPECT_EQ(m_pSystem->SendEvent(SystemConstants::NOTIFY_USIM_EVENT, 0, 0, 0), 0);
}

TEST_F(SystemTest, SetEvent)
{
    EXPECT_CALL(m_objMockSystemCallback, SendDataToJava(_, _, _))
            .Times(AnyNumber())
            .WillRepeatedly(Return(1));
    EXPECT_EQ(m_pSystem->SetEvent(SystemConstants::NOTIFY_USIM_EVENT, 0), 0);
}

TEST_F(SystemTest, ResetEvent)
{
    EXPECT_CALL(m_objMockSystemCallback, SendDataToJava(_, _, _))
            .Times(AnyNumber())
            .WillRepeatedly(Return(1));
    EXPECT_EQ(m_pSystem->ResetEvent(SystemConstants::NOTIFY_USIM_EVENT, 0), 0);
}

TEST_F(SystemTest, IsWifiCallingEnabled)
{
    EXPECT_CALL(m_objMockSystemCallback, SendDataToJava(_, _, _))
            .Times(AnyNumber())
            .WillRepeatedly(Return(1));
    EXPECT_EQ(m_pSystem->IsWifiCallingEnabled(0), 0);
}

TEST_F(SystemTest, GetWifiCallingPreferences)
{
    EXPECT_CALL(m_objMockSystemCallback, SendDataToJava(_, _, _))
            .Times(AnyNumber())
            .WillRepeatedly(Return(1));
    EXPECT_EQ(m_pSystem->GetWifiCallingPreferences(0), 0);
}

TEST_F(SystemTest, IsWifiCallingProvisioned)
{
    EXPECT_CALL(m_objMockSystemCallback, SendDataToJava(_, _, _))
            .Times(AnyNumber())
            .WillRepeatedly(Return(1));
    EXPECT_EQ(m_pSystem->IsWifiCallingProvisioned(0), 0);
}

TEST_F(SystemTest, GetWifiCallingAddressId)
{
    AString strValue("10:221:59:89");
    String16 str16(strValue.GetStr(), strValue.GetLength());

    EXPECT_CALL(m_objMockSystemCallback, SendDataToJava(_, _, _))
            .Times(1)
            .WillOnce(Invoke(
                    [str16](Unused, android::Parcel& out, Unused)
                    {
                        out.writeString16(str16);
                        out.setDataPosition(0);
                        return 1;
                    }));
    EXPECT_EQ(m_pSystem->GetWifiCallingAddressId(0), strValue);
}

TEST_F(SystemTest, StartListeningForLocation)
{
    EXPECT_CALL(m_objMockSystemCallback, SendDataToJava(_, _, _))
            .Times(AnyNumber())
            .WillRepeatedly(Return(1));
    EXPECT_EQ(m_pSystem->StartListeningForLocation(30, 0), 0);
}

TEST_F(SystemTest, StopListeningForLocation)
{
    EXPECT_CALL(m_objMockSystemCallback, SendDataToJava(_, _, _))
            .Times(AnyNumber())
            .WillRepeatedly(Return(1));
    m_pSystem->StopListeningForLocation(0);
}

TEST_F(SystemTest, GetLastKnownLocation)
{
    AStringArray objLocationInfo;

    m_pSystem->SetCallback(IMS_NULL);

    EXPECT_EQ(m_pSystem->GetLastKnownLocation(
                      objLocationInfo, ILocationInfo::LOCATION_POSITION_N_COUNTRY, 0),
            0);

    AString strValue("12.971599");
    String16 str16(strValue.GetStr(), strValue.GetLength());

    m_pSystem->SetCallback(&m_objMockSystemCallback);

    EXPECT_CALL(m_objMockSystemCallback, SendDataToJava(_, _, _))
            .Times(1)
            .WillOnce(Invoke(
                    [str16](Unused, android::Parcel& out, Unused)
                    {
                        out.writeInt32(1);
                        out.writeString16(str16);
                        out.setDataPosition(0);
                        return 1;
                    }));
    EXPECT_EQ(m_pSystem->GetLastKnownLocation(
                      objLocationInfo, ILocationInfo::LOCATION_POSITION_N_COUNTRY, 0),
            1);
    EXPECT_EQ(objLocationInfo.GetFirstElement(), strValue);

    EXPECT_CALL(m_objMockSystemCallback, SendDataToJava(_, _, _))
            .Times(AnyNumber())
            .WillRepeatedly(Return(1));
    EXPECT_EQ(m_pSystem->GetLastKnownLocation(
                      objLocationInfo, ILocationInfo::LOCATION_POSITION_N_COUNTRY, 0),
            0);

    EXPECT_CALL(m_objMockSystemCallback, SendDataToJava(_, _, _))
            .Times(AnyNumber())
            .WillRepeatedly(Return(0));
    EXPECT_EQ(m_pSystem->GetLastKnownLocation(
                      objLocationInfo, ILocationInfo::LOCATION_POSITION_N_COUNTRY, 0),
            0);
}

TEST_F(SystemTest, StartInstantLocationUpdate)
{
    EXPECT_CALL(m_objMockSystemCallback, SendDataToJava(_, _, _))
            .Times(AnyNumber())
            .WillRepeatedly(Return(1));
    EXPECT_EQ(m_pSystem->StartInstantLocationUpdate(0), 0);
}

TEST_F(SystemTest, StartImsTraffic)
{
    m_pSystem->SetCallback(IMS_NULL);

    EXPECT_EQ(m_pSystem->StartImsTraffic(10, IImsRadio::TRAFFIC_TYPE_VIDEO,
                      IImsRadio::ACCESS_NETWORK_TYPE_EUTRAN, IImsRadio::DIRECTION_MO, 0),
            -1);

    m_pSystem->SetCallback(&m_objMockSystemCallback);

    EXPECT_CALL(m_objMockSystemCallback, SendDataToJava(_, _, _))
            .Times(AnyNumber())
            .WillRepeatedly(Return(1));

    EXPECT_EQ(m_pSystem->StartImsTraffic(10, IImsRadio::TRAFFIC_TYPE_VIDEO,
                      IImsRadio::ACCESS_NETWORK_TYPE_EUTRAN, IImsRadio::DIRECTION_MO, 0),
            0);

    EXPECT_CALL(m_objMockSystemCallback, SendDataToJava(_, _, _))
            .Times(AnyNumber())
            .WillRepeatedly(Return(0));
    EXPECT_EQ(m_pSystem->StartImsTraffic(10, IImsRadio::TRAFFIC_TYPE_VIDEO,
                      IImsRadio::ACCESS_NETWORK_TYPE_EUTRAN, IImsRadio::DIRECTION_MO, 0),
            -1);
}

TEST_F(SystemTest, StopImsTraffic)
{
    EXPECT_CALL(m_objMockSystemCallback, SendDataToJava(_, _, _))
            .Times(AnyNumber())
            .WillRepeatedly(Return(1));
    m_pSystem->StopImsTraffic(10, 0);
}

TEST_F(SystemTest, TriggerEpsFallback)
{
    EXPECT_CALL(m_objMockSystemCallback, SendDataToJava(_, _, _))
            .Times(AnyNumber())
            .WillRepeatedly(Return(1));
    EXPECT_EQ(m_pSystem->TriggerEpsFallback(IImsRadio::EPSFB_REASON_NO_NETWORK_TRIGGER, 0), 0);
}

TEST_F(SystemTest, AddIpSecSaParameter)
{
    ByteArray objIk("12ae555f002f93cfd70a4a6488f40ce8000000");
    ByteArray objCk("2711c778ca05da14de3ce192");

    IpSecSaParameter objIpSecSaParameter(1, IpSecType::SECURITY_PROTOCOL_ESP,
            IpSecType::INTEGRITY_ALGORITHM_HMAC_SHA_1_96, IpSecType::ENCRYPTION_ALGORITHM_AES_CBC,
            objIk, objCk);
    IpAddress srcIp("10.128.120.179");
    IpAddress remoteIp("10.140.6.68");
    IpSecSaParameter::Policy objPolicy(1102393271, IpSecType::DIRECTION_ANY,
            IpSecType::MODE_TRANSPORT, IpSecType::TRANS_PROTOCOL_TCP, srcIp, 32009, remoteIp,
            32001);
    objIpSecSaParameter.AddPolicy(objPolicy);

    EXPECT_CALL(m_objMockSystemCallback, SendDataToJava(_, _, _))
            .Times(AnyNumber())
            .WillRepeatedly(Return(1));
    EXPECT_EQ(m_pSystem->AddIpSecSaParameter(objIpSecSaParameter, 0), 0);

    EXPECT_CALL(m_objMockSystemCallback, SendDataToJava(_, _, _))
            .Times(AnyNumber())
            .WillRepeatedly(Return(0));
    EXPECT_EQ(m_pSystem->AddIpSecSaParameter(objIpSecSaParameter, 0), 0);
}

TEST_F(SystemTest, RemoveIpSecSaParameter)
{
    m_pSystem->SetCallback(IMS_NULL);
    m_pSystem->RemoveIpSecSaParameter(1, 0);
    m_pSystem->SetCallback(&m_objMockSystemCallback);

    EXPECT_CALL(m_objMockSystemCallback, SendDataToJava(_, _, _))
            .Times(AnyNumber())
            .WillRepeatedly(Return(1));

    m_pSystem->RemoveIpSecSaParameter(1, 0);

    EXPECT_CALL(m_objMockSystemCallback, SendDataToJava(_, _, _))
            .Times(AnyNumber())
            .WillRepeatedly(Return(0));
    m_pSystem->RemoveIpSecSaParameter(1, 0);
}

TEST_F(SystemTest, ApplyIpSecSa)
{
    m_pSystem->SetCallback(IMS_NULL);

    EXPECT_EQ(m_pSystem->ApplyIpSecSa(1, 1102393271, 125, 0), 0);

    m_pSystem->SetCallback(&m_objMockSystemCallback);

    EXPECT_CALL(m_objMockSystemCallback, SendDataToJava(_, _, _))
            .Times(AnyNumber())
            .WillRepeatedly(Return(1));
    EXPECT_EQ(m_pSystem->ApplyIpSecSa(1, 1102393271, 119, 0), 0);

    EXPECT_CALL(m_objMockSystemCallback, SendDataToJava(_, _, _))
            .Times(AnyNumber())
            .WillRepeatedly(Return(0));
    EXPECT_EQ(m_pSystem->ApplyIpSecSa(1, -755290214, 125, 0), 0);
}

TEST_F(SystemTest, RemoveIpSecSa)
{
    m_pSystem->SetCallback(IMS_NULL);
    m_pSystem->RemoveIpSecSa(1, 1102393271, 123, 0);
    m_pSystem->SetCallback(&m_objMockSystemCallback);

    EXPECT_CALL(m_objMockSystemCallback, SendDataToJava(_, _, _))
            .Times(AnyNumber())
            .WillRepeatedly(Return(1));

    m_pSystem->RemoveIpSecSa(1, -992000057, 110, 0);

    EXPECT_CALL(m_objMockSystemCallback, SendDataToJava(_, _, _))
            .Times(AnyNumber())
            .WillRepeatedly(Return(0));
    m_pSystem->RemoveIpSecSa(1, 1102393271, 124, 0);
}
}  // namespace android
