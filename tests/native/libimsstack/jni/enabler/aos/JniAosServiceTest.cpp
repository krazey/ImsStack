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

#include "../../interface/aos/MockIAosService.h"

using ::testing::_;
using ::testing::NiceMock;

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
    JniAosService* m_pJniAosService;
    Parcel m_objParcel;
    NiceMock<MockIAosService> m_objMockIAosService;

protected:
    virtual void SetUp() override
    {
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

TEST_F(JniAosServiceTest, ShouldInvokeAosServiceWhenRequestRegistration)
{
    // GIVEN
    m_objParcel.writeInt32(IIAosService::J2N_REQUEST_REGISTRATION);
    m_objParcel.setDataPosition(0);

    EXPECT_CALL(m_objMockIAosService, UpdateSipDelegateRegistration());

    // WHEN
    m_pJniAosService->SendData(m_objParcel);

    // THEN : GIVEN conditions should be met.
}

TEST_F(JniAosServiceTest, ShouldInvokeAosServiceWhenRequestDeregistration)
{
    // GIVEN
    m_objParcel.writeInt32(IIAosService::J2N_REQUEST_DEREGISTRATION);
    m_objParcel.setDataPosition(0);

    EXPECT_CALL(m_objMockIAosService, TriggerSipDelegateDeregistration());

    // WHEN
    m_pJniAosService->SendData(m_objParcel);

    // THEN : GIVEN conditions should be met.
}

TEST_F(JniAosServiceTest, ShouldInvokeAosServiceWhenRequestFullRegistration)
{
    // GIVEN
    m_objParcel.writeInt32(IIAosService::J2N_REQUEST_FULL_REGISTRATION);
    m_objParcel.setDataPosition(0);

    EXPECT_CALL(m_objMockIAosService, TriggerFullNetworkRegistration(_, _));

    // WHEN
    m_pJniAosService->SendData(m_objParcel);

    // THEN : GIVEN conditions should be met.
}

TEST_F(JniAosServiceTest, ShouldInvokeAosServiceWhenRequestCapabilitiesChanged)
{
    // GIVEN
    m_objParcel.writeInt32(IIAosService::J2N_REQUEST_CAPABILITIES_CHANGED);
    m_objParcel.setDataPosition(0);

    EXPECT_CALL(m_objMockIAosService, NotifyCapabilitiesChanged(_));

    // WHEN
    m_pJniAosService->SendData(m_objParcel);

    // THEN : GIVEN conditions should be met.
}

TEST_F(JniAosServiceTest, ShouldInvokeAosServiceWhenRequestControlRegistration)
{
    // GIVEN
    m_objParcel.writeInt32(IIAosService::J2N_REQUEST_CONTROL_REGISTRATION);
    m_objParcel.setDataPosition(0);

    EXPECT_CALL(m_objMockIAosService, ControlRegistration(_, _, _));

    // WHEN
    m_pJniAosService->SendData(m_objParcel);

    // THEN : GIVEN conditions should be met.
}

TEST_F(JniAosServiceTest, ShouldInvokeAosServiceWhenNotifyAirplaneSetting)
{
    // GIVEN
    m_objParcel.writeInt32(IIAosService::J2N_NOTIFY_AIRPLANE_SETTING);
    m_objParcel.setDataPosition(0);

    EXPECT_CALL(m_objMockIAosService, NotifyAirplaneSetting(_));

    // WHEN
    m_pJniAosService->SendData(m_objParcel);

    // THEN : GIVEN conditions should be met.
}

TEST_F(JniAosServiceTest, ShouldInvokeAosServiceWhenNotifyDataRoamingSetting)
{
    // GIVEN
    m_objParcel.writeInt32(IIAosService::J2N_NOTIFY_DATA_ROAMING_SETTING);
    m_objParcel.setDataPosition(0);

    EXPECT_CALL(m_objMockIAosService, NotifyDataRoamingSetting(_));

    // WHEN
    m_pJniAosService->SendData(m_objParcel);

    // THEN : GIVEN conditions should be met.
}

TEST_F(JniAosServiceTest, ShouldInvokeAosServiceWhenNotifyMobileDataSetting)
{
    // GIVEN
    m_objParcel.writeInt32(IIAosService::J2N_NOTIFY_MOBILE_DATA_SETTING);
    m_objParcel.setDataPosition(0);

    EXPECT_CALL(m_objMockIAosService, NotifyMobileDataSetting(_));

    // WHEN
    m_pJniAosService->SendData(m_objParcel);

    // THEN : GIVEN conditions should be met.
}

TEST_F(JniAosServiceTest, ShouldInvokeAosServiceWhenNotifyRoamingPreferredVoiceNetwork)
{
    // GIVEN
    m_objParcel.writeInt32(IIAosService::J2N_NOTIFY_ROAMING_PREFERRED_VOICE_NETWORK);
    m_objParcel.setDataPosition(0);

    EXPECT_CALL(m_objMockIAosService, NotifyRoamingPreferredVoiceNetwork(_));

    // WHEN
    m_pJniAosService->SendData(m_objParcel);

    // THEN : GIVEN conditions should be met.
}

TEST_F(JniAosServiceTest, ShouldInvokeAosServiceWhenNotifyServiceSetting)
{
    // GIVEN
    m_objParcel.writeInt32(IIAosService::J2N_NOTIFY_SERVICE_SETTING);
    m_objParcel.setDataPosition(0);

    EXPECT_CALL(m_objMockIAosService, NotifyServiceSetting(_, _));

    // WHEN
    m_pJniAosService->SendData(m_objParcel);

    // THEN : GIVEN conditions should be met.
}

TEST_F(JniAosServiceTest, ShouldInvokeAosServiceWhenNotifyTtySetting)
{
    // GIVEN
    m_objParcel.writeInt32(IIAosService::J2N_NOTIFY_TTY_SETTING);
    m_objParcel.setDataPosition(0);

    EXPECT_CALL(m_objMockIAosService, NotifyTtySetting(_));

    // WHEN
    m_pJniAosService->SendData(m_objParcel);

    // THEN : GIVEN conditions should be met.
}

TEST_F(JniAosServiceTest, ShouldInvokeAosServiceWhenNotifyVideoSetting)
{
    // GIVEN
    m_objParcel.writeInt32(IIAosService::J2N_NOTIFY_VIDEO_SETTING);
    m_objParcel.setDataPosition(0);

    EXPECT_CALL(m_objMockIAosService, NotifyVideoSetting(_));

    // WHEN
    m_pJniAosService->SendData(m_objParcel);

    // THEN : GIVEN conditions should be met.
}

TEST_F(JniAosServiceTest, ShouldInvokeAosServiceWhenNotifyVolteSetting)
{
    // GIVEN
    m_objParcel.writeInt32(IIAosService::J2N_NOTIFY_VOLTE_SETTING);
    m_objParcel.setDataPosition(0);

    EXPECT_CALL(m_objMockIAosService, NotifyVolteSetting(_));

    // WHEN
    m_pJniAosService->SendData(m_objParcel);

    // THEN : GIVEN conditions should be met.
}

TEST_F(JniAosServiceTest, ShouldInvokeAosServiceWhenNotifyWfcSetting)
{
    // GIVEN
    m_objParcel.writeInt32(IIAosService::J2N_NOTIFY_WFC_SETTING);
    m_objParcel.setDataPosition(0);

    EXPECT_CALL(m_objMockIAosService, NotifyWfcSetting(_));

    // WHEN
    m_pJniAosService->SendData(m_objParcel);

    // THEN : GIVEN conditions should be met.
}

TEST_F(JniAosServiceTest, ShouldInvokeAosServiceWhenNotifyAosStart)
{
    // GIVEN
    m_objParcel.writeInt32(IIAosService::J2N_NOTIFY_AOS_START);
    m_objParcel.setDataPosition(0);

    EXPECT_CALL(m_objMockIAosService, NotifyAosStart());

    // WHEN
    m_pJniAosService->SendData(m_objParcel);

    // THEN : GIVEN conditions should be met.
}

TEST_F(JniAosServiceTest, ShouldInvokeAosServiceWhenNotifyIpcanHandoverFailure)
{
    // GIVEN
    m_objParcel.writeInt32(IIAosService::J2N_NOTIFY_IPCAN_HANDOVER_FAILURE);
    m_objParcel.setDataPosition(0);

    EXPECT_CALL(m_objMockIAosService, NotifyIpcanHandoverFailure(_, _));

    // WHEN
    m_pJniAosService->SendData(m_objParcel);

    // THEN : GIVEN conditions should be met.
}

TEST_F(JniAosServiceTest, ShouldInvokeAosServiceWhenNotifyIsimState)
{
    // GIVEN
    m_objParcel.writeInt32(IIAosService::J2N_NOTIFY_ISIM_STATE);
    m_objParcel.setDataPosition(0);

    EXPECT_CALL(m_objMockIAosService, NotifyIsimState(_));

    // WHEN
    m_pJniAosService->SendData(m_objParcel);

    // THEN : GIVEN conditions should be met.
}

TEST_F(JniAosServiceTest, ShouldInvokeAosServiceWhenNotifyLocationInfo)
{
    // GIVEN
    m_objParcel.writeInt32(IIAosService::J2N_NOTIFY_LOCATION_INFO);
    m_objParcel.setDataPosition(0);

    EXPECT_CALL(m_objMockIAosService, NotifyLocationInfo(_));

    // WHEN
    m_pJniAosService->SendData(m_objParcel);

    // THEN : GIVEN conditions should be met.
}

TEST_F(JniAosServiceTest, ShouldInvokeAosServiceWhenNotifyMobileDataLimit)
{
    // GIVEN
    m_objParcel.writeInt32(IIAosService::J2N_NOTIFY_MOBILE_DATA_LIMIT);
    m_objParcel.setDataPosition(0);

    EXPECT_CALL(m_objMockIAosService, NotifyMobileDataLimit(_));

    // WHEN
    m_pJniAosService->SendData(m_objParcel);

    // THEN : GIVEN conditions should be met.
}

TEST_F(JniAosServiceTest, ShouldInvokeAosServiceWhenNotifyNetworkVideoCapability)
{
    // GIVEN
    m_objParcel.writeInt32(IIAosService::J2N_NOTIFY_NETWORK_VIDEO_CAPABILITY);
    m_objParcel.setDataPosition(0);

    EXPECT_CALL(m_objMockIAosService, NotifyNetworkVideoCapability(_));

    // WHEN
    m_pJniAosService->SendData(m_objParcel);

    // THEN : GIVEN conditions should be met.
}

TEST_F(JniAosServiceTest, ShouldInvokeAosServiceWhenNotifyPhoneNumberState)
{
    // GIVEN
    m_objParcel.writeInt32(IIAosService::J2N_NOTIFY_PHONE_NUMBER_STATE);
    m_objParcel.setDataPosition(0);

    EXPECT_CALL(m_objMockIAosService, NotifyPhoneNumberState(_, _));

    // WHEN
    m_pJniAosService->SendData(m_objParcel);

    // THEN : GIVEN conditions should be met.
}

TEST_F(JniAosServiceTest, ShouldInvokeAosServiceWhenNotifyPlmnChanged)
{
    // GIVEN
    m_objParcel.writeInt32(IIAosService::J2N_NOTIFY_PLMN_CHANGED);
    m_objParcel.setDataPosition(0);

    EXPECT_CALL(m_objMockIAosService, NotifyPlmnChanged());

    // WHEN
    m_pJniAosService->SendData(m_objParcel);

    // THEN : GIVEN conditions should be met.
}

TEST_F(JniAosServiceTest, ShouldInvokeAosServiceWhenNotifyPowerOff)
{
    // GIVEN
    m_objParcel.writeInt32(IIAosService::J2N_NOTIFY_POWER_OFF);
    m_objParcel.setDataPosition(0);

    EXPECT_CALL(m_objMockIAosService, NotifyPowerOff());

    // WHEN
    m_pJniAosService->SendData(m_objParcel);

    // THEN : GIVEN conditions should be met.
}

TEST_F(JniAosServiceTest, ShouldInvokeAosServiceWhenNotifyPreciseCallState)
{
    // GIVEN
    m_objParcel.writeInt32(IIAosService::J2N_NOTIFY_PRECISE_CALL_STATE);
    m_objParcel.setDataPosition(0);

    EXPECT_CALL(m_objMockIAosService, NotifyPreciseCallState(_));

    // WHEN
    m_pJniAosService->SendData(m_objParcel);

    // THEN : GIVEN conditions should be met.
}

TEST_F(JniAosServiceTest, ShouldInvokeAosServiceWhenNotifyCarrierSignalPcoValueChanged)
{
    // GIVEN
    m_objParcel.writeInt32(IIAosService::J2N_NOTIFY_CARRIER_SIGNAL_PCO_VALUE_CHANGED);
    m_objParcel.setDataPosition(0);

    EXPECT_CALL(m_objMockIAosService, NotifyCarrierSignalPcoValueChanged(_));

    // WHEN
    m_pJniAosService->SendData(m_objParcel);

    // THEN : GIVEN conditions should be met.
}

TEST_F(JniAosServiceTest, ShouldInvokeAosServiceWhenNotifyEmergencyCallbackModeChanged)
{
    // GIVEN
    m_objParcel.writeInt32(IIAosService::J2N_NOTIFY_EMERGENCY_CALLBACK_MODE_CHANGED);
    m_objParcel.setDataPosition(0);

    EXPECT_CALL(m_objMockIAosService, NotifyEmergencyCallbackModeChanged(_, _, _));

    // WHEN
    m_pJniAosService->SendData(m_objParcel);

    // THEN : GIVEN conditions should be met.
}
