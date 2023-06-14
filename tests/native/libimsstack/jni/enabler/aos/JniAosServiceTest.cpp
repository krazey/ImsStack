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
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include "JniEnablerConnector.h"

#include "IAosService.h"
#include "IIAosService.h"
#include "JniAosService.h"
#include "interface/IAosRegistrationControlListener.h"

#include "../../interface/aos/MockIAosService.h"

using ::testing::_;
using ::testing::AnyNumber;
using ::testing::Return;

using namespace android;

LOCAL IMS_SINT32 SLOT_ID = 0;

class TestJniAosService : public JniAosService
{
public:
    inline explicit TestJniAosService(
            IN Jni_SendDataToJava pfnSendDataToJava, IN IMS_SINT32 nSlotId = 0) :
            JniAosService(pfnSendDataToJava, nSlotId)
    {
    }

    inline virtual IMS_BOOL IsThreadSwitchingRequired(IN IMS_SINT32 /*nMsg*/) const override
    {
        return IMS_FALSE;
    }
};

class JniAosServiceTest : public ::testing::Test
{
public:
    MockIAosService m_objMockIAosService;
    JniAosService* m_pJniAosService;

protected:
    virtual void SetUp() override
    {
        EXPECT_CALL(m_objMockIAosService, NotifyJniEnablerSet())
                .Times(AnyNumber())
                .WillRepeatedly(Return());
        JniEnablerConnector::GetInstance().SetNativeEnabler(
                SLOT_ID, EnablerType::AOS_SERVICE, &m_objMockIAosService);

        m_pJniAosService =
                new TestJniAosService(reinterpret_cast<Jni_SendDataToJava>(0x01), SLOT_ID);
    }

    virtual void TearDown() override
    {
        if (m_pJniAosService)
        {
            delete m_pJniAosService;
        }

        JniEnablerConnector::GetInstance().SetNativeEnabler(
                SLOT_ID, EnablerType::AOS_SERVICE, IMS_NULL);
    }
};

TEST_F(JniAosServiceTest, SendData_REQUEST_REGISTRATION)
{
    Parcel objParcel;
    objParcel.writeInt32(IIAosService::J2N_REQUEST_REGISTRATION);
    objParcel.setDataPosition(0);

    EXPECT_CALL(m_objMockIAosService, UpdateSipDelegateRegistration()).Times(1);

    m_pJniAosService->SendData(objParcel);
}

TEST_F(JniAosServiceTest, SendData_REQUEST_DEREGISTRATION)
{
    Parcel objParcel;
    objParcel.writeInt32(IIAosService::J2N_REQUEST_DEREGISTRATION);
    objParcel.setDataPosition(0);

    EXPECT_CALL(m_objMockIAosService, TriggerSipDelegateDeregistration()).Times(1);

    m_pJniAosService->SendData(objParcel);
}

TEST_F(JniAosServiceTest, SendData_REQUEST_FULL_REGISTRATION)
{
    Parcel objParcel;
    objParcel.writeInt32(IIAosService::J2N_REQUEST_FULL_REGISTRATION);
    objParcel.writeInt32(488);
    objParcel.writeString16(String16("testSipReason"));
    objParcel.setDataPosition(0);

    EXPECT_CALL(m_objMockIAosService, TriggerFullNetworkRegistration(_, _)).Times(1);

    m_pJniAosService->SendData(objParcel);
}

TEST_F(JniAosServiceTest, SendData_REQUEST_CAPABILITIES_CHANGED)
{
    Parcel objParcel;
    objParcel.writeInt32(IIAosService::J2N_REQUEST_CAPABILITIES_CHANGED);
    objParcel.writeInt32(3);
    objParcel.writeInt32(static_cast<IMS_UINT32>(AosNetworkType::LTE));
    objParcel.writeInt32(static_cast<IMS_UINT32>(AosCapability::VOICE) |
            static_cast<IMS_UINT32>(AosCapability::VIDEO));
    objParcel.writeInt32(static_cast<IMS_UINT32>(AosNetworkType::IWLAN));
    objParcel.writeInt32(static_cast<IMS_UINT32>(AosCapability::VOICE) |
            static_cast<IMS_UINT32>(AosCapability::SMS));
    objParcel.writeInt32(static_cast<IMS_UINT32>(AosNetworkType::NR));
    objParcel.writeInt32(static_cast<IMS_UINT32>(AosCapability::VOICE) |
            static_cast<IMS_UINT32>(AosCapability::UT));
    objParcel.setDataPosition(0);

    EXPECT_CALL(m_objMockIAosService, NotifyCapabilitiesChanged(_)).Times(1);

    m_pJniAosService->SendData(objParcel);
}

TEST_F(JniAosServiceTest, SendData_REQUEST_CONTROL_REGISTRATION)
{
    Parcel objParcel;
    objParcel.writeInt32(IIAosService::J2N_REQUEST_CONTROL_REGISTRATION);
    objParcel.writeInt32(static_cast<IMS_UINT32>(AosRegRequestType::START));
    objParcel.writeInt32(static_cast<IMS_UINT32>(AosPcscfOrder::FIRST));
    objParcel.writeInt32(static_cast<IMS_UINT32>(AosControlCause::DATA));
    objParcel.setDataPosition(0);

    EXPECT_CALL(m_objMockIAosService, ControlRegistration(_, _, _)).Times(1);

    m_pJniAosService->SendData(objParcel);
}

TEST_F(JniAosServiceTest, SendData_NOTIFY_AIRPLANE_SETTING)
{
    Parcel objParcel;
    objParcel.writeInt32(IIAosService::J2N_NOTIFY_AIRPLANE_SETTING);
    objParcel.writeInt32(0);
    objParcel.setDataPosition(0);

    EXPECT_CALL(m_objMockIAosService, NotifyAirplaneSetting(_)).Times(1);

    m_pJniAosService->SendData(objParcel);
}

TEST_F(JniAosServiceTest, SendData_NOTIFY_DATA_ROAMING_SETTING)
{
    Parcel objParcel;
    objParcel.writeInt32(IIAosService::J2N_NOTIFY_DATA_ROAMING_SETTING);
    objParcel.writeInt32(0);
    objParcel.setDataPosition(0);

    EXPECT_CALL(m_objMockIAosService, NotifyDataRoamingSetting(_)).Times(1);

    m_pJniAosService->SendData(objParcel);
}

TEST_F(JniAosServiceTest, SendData_NOTIFY_MOBILE_DATA_SETTING)
{
    Parcel objParcel;
    objParcel.writeInt32(IIAosService::J2N_NOTIFY_MOBILE_DATA_SETTING);
    objParcel.writeInt32(0);
    objParcel.setDataPosition(0);

    EXPECT_CALL(m_objMockIAosService, NotifyMobileDataSetting(_)).Times(1);

    m_pJniAosService->SendData(objParcel);
}

TEST_F(JniAosServiceTest, SendData_NOTIFY_ROAMING_PREFERRED_VOICE_NETWORK)
{
    Parcel objParcel;
    objParcel.writeInt32(IIAosService::J2N_NOTIFY_ROAMING_PREFERRED_VOICE_NETWORK);
    objParcel.writeInt32(static_cast<IMS_UINT32>(RoamingPreferredVoiceNetwork::CELLULAR));
    objParcel.setDataPosition(0);

    EXPECT_CALL(m_objMockIAosService, NotifyRoamingPreferredVoiceNetwork(_)).Times(1);

    m_pJniAosService->SendData(objParcel);
}

TEST_F(JniAosServiceTest, SendData_NOTIFY_SERVICE_SETTING)
{
    Parcel objParcel;
    objParcel.writeInt32(IIAosService::J2N_NOTIFY_SERVICE_SETTING);
    objParcel.writeInt32(static_cast<IMS_UINT32>(ServiceSetting::OFF));
    objParcel.writeInt32(0);
    objParcel.setDataPosition(0);

    EXPECT_CALL(m_objMockIAosService, NotifyServiceSetting(_, _)).Times(1);

    m_pJniAosService->SendData(objParcel);
}

TEST_F(JniAosServiceTest, SendData_NOTIFY_TTY_SETTING)
{
    Parcel objParcel;
    objParcel.writeInt32(IIAosService::J2N_NOTIFY_TTY_SETTING);
    objParcel.writeInt32(0);
    objParcel.setDataPosition(0);

    EXPECT_CALL(m_objMockIAosService, NotifyTtySetting(_)).Times(1);

    m_pJniAosService->SendData(objParcel);
}

TEST_F(JniAosServiceTest, SendData_NOTIFY_VIDEO_SETTING)
{
    Parcel objParcel;
    objParcel.writeInt32(IIAosService::J2N_NOTIFY_VIDEO_SETTING);
    objParcel.writeInt32(0);
    objParcel.setDataPosition(0);

    EXPECT_CALL(m_objMockIAosService, NotifyVideoSetting(_)).Times(1);

    m_pJniAosService->SendData(objParcel);
}

TEST_F(JniAosServiceTest, SendData_NOTIFY_VOLTE_SETTING)
{
    Parcel objParcel;
    objParcel.writeInt32(IIAosService::J2N_NOTIFY_VOLTE_SETTING);
    objParcel.writeInt32(0);
    objParcel.setDataPosition(0);

    EXPECT_CALL(m_objMockIAosService, NotifyVolteSetting(_)).Times(1);

    m_pJniAosService->SendData(objParcel);
}

TEST_F(JniAosServiceTest, SendData_NOTIFY_WFC_SETTING)
{
    Parcel objParcel;
    objParcel.writeInt32(IIAosService::J2N_NOTIFY_WFC_SETTING);
    objParcel.writeInt32(0);
    objParcel.setDataPosition(0);

    EXPECT_CALL(m_objMockIAosService, NotifyWfcSetting(_)).Times(1);

    m_pJniAosService->SendData(objParcel);
}

TEST_F(JniAosServiceTest, SendData_NOTIFY_AOS_START)
{
    Parcel objParcel;
    objParcel.writeInt32(IIAosService::J2N_NOTIFY_AOS_START);
    objParcel.setDataPosition(0);

    EXPECT_CALL(m_objMockIAosService, NotifyAosStart()).Times(1);

    m_pJniAosService->SendData(objParcel);
}

TEST_F(JniAosServiceTest, SendData_NOTIFY_IPCAN_HANDOVER_FAILURE)
{
    Parcel objParcel;
    objParcel.writeInt32(IIAosService::J2N_NOTIFY_IPCAN_HANDOVER_FAILURE);
    objParcel.writeInt32(0);
    objParcel.writeInt32(1);
    objParcel.setDataPosition(0);

    EXPECT_CALL(m_objMockIAosService, NotifyIpcanHandoverFailure(_, _)).Times(1);

    m_pJniAosService->SendData(objParcel);
}

TEST_F(JniAosServiceTest, SendData_NOTIFY_ISIM_STATE)
{
    Parcel objParcel;
    objParcel.writeInt32(IIAosService::J2N_NOTIFY_ISIM_STATE);
    objParcel.writeInt32(static_cast<IMS_UINT32>(IsimState::LOADED));
    objParcel.setDataPosition(0);

    EXPECT_CALL(m_objMockIAosService, NotifyIsimState(_)).Times(1);

    m_pJniAosService->SendData(objParcel);
}

TEST_F(JniAosServiceTest, SendData_NOTIFY_LOCATION_INFO)
{
    Parcel objParcel;
    objParcel.writeInt32(IIAosService::J2N_NOTIFY_LOCATION_INFO);
    objParcel.writeInt32(static_cast<IMS_UINT32>(LocationInfo::FIXED));
    objParcel.setDataPosition(0);

    EXPECT_CALL(m_objMockIAosService, NotifyLocationInfo(_)).Times(1);

    m_pJniAosService->SendData(objParcel);
}

TEST_F(JniAosServiceTest, SendData_NOTIFY_MOBILE_DATA_LIMIT)
{
    Parcel objParcel;
    objParcel.writeInt32(IIAosService::J2N_NOTIFY_MOBILE_DATA_LIMIT);
    objParcel.writeInt32(0);
    objParcel.setDataPosition(0);

    EXPECT_CALL(m_objMockIAosService, NotifyMobileDataLimit(_)).Times(1);

    m_pJniAosService->SendData(objParcel);
}

TEST_F(JniAosServiceTest, SendData_NOTIFY_NETWORK_VIDEO_CAPABILITY)
{
    Parcel objParcel;
    objParcel.writeInt32(IIAosService::J2N_NOTIFY_NETWORK_VIDEO_CAPABILITY);
    objParcel.writeInt32(0);
    objParcel.setDataPosition(0);

    EXPECT_CALL(m_objMockIAosService, NotifyNetworkVideoCapability(_)).Times(1);

    m_pJniAosService->SendData(objParcel);
}

TEST_F(JniAosServiceTest, SendData_NOTIFY_PHONE_NUMBER_STATE)
{
    Parcel objParcel;
    objParcel.writeInt32(IIAosService::J2N_NOTIFY_PHONE_NUMBER_STATE);
    objParcel.writeInt32(0);
    objParcel.writeInt32(static_cast<IMS_UINT32>(PhoneNumberState::SIM_LOADED));
    objParcel.setDataPosition(0);

    EXPECT_CALL(m_objMockIAosService, NotifyPhoneNumberState(_, _)).Times(1);

    m_pJniAosService->SendData(objParcel);
}

TEST_F(JniAosServiceTest, SendData_NOTIFY_PLMN_CHANGED)
{
    Parcel objParcel;
    objParcel.writeInt32(IIAosService::J2N_NOTIFY_PLMN_CHANGED);
    objParcel.setDataPosition(0);

    EXPECT_CALL(m_objMockIAosService, NotifyPlmnChanged()).Times(1);

    m_pJniAosService->SendData(objParcel);
}

TEST_F(JniAosServiceTest, SendData_NOTIFY_POWER_OFF)
{
    Parcel objParcel;
    objParcel.writeInt32(IIAosService::J2N_NOTIFY_POWER_OFF);
    objParcel.setDataPosition(0);

    EXPECT_CALL(m_objMockIAosService, NotifyPowerOff()).Times(1);

    m_pJniAosService->SendData(objParcel);
}

TEST_F(JniAosServiceTest, SendData_NOTIFY_PRECISE_CALL_STATE)
{
    Parcel objParcel;
    objParcel.writeInt32(IIAosService::J2N_NOTIFY_PRECISE_CALL_STATE);
    objParcel.writeInt32(static_cast<IMS_UINT32>(PreciseCallState::ACTIVE));
    objParcel.setDataPosition(0);

    EXPECT_CALL(m_objMockIAosService, NotifyPreciseCallState(_)).Times(1);

    m_pJniAosService->SendData(objParcel);
}

TEST_F(JniAosServiceTest, SendData_J2N_NOTIFY_CARRIER_SIGNAL_PCO_VALUE_CHANGED)
{
    Parcel objParcel;
    objParcel.writeInt32(IIAosService::J2N_NOTIFY_CARRIER_SIGNAL_PCO_VALUE_CHANGED);
    objParcel.writeInt32(5);
    objParcel.setDataPosition(0);

    EXPECT_CALL(m_objMockIAosService, NotifyCarrierSignalPcoValueChanged(_)).Times(1);

    m_pJniAosService->SendData(objParcel);
}
