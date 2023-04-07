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

#include "IuMtsService.h"
#include "JniEnablerConnector.h"
#include "JniMtsService.h"
#include "MockIMtsService.h"
#include <binder/Parcel.h>
#include <gtest/gtest.h>

using ::testing::_;
using ::testing::Return;

namespace android
{

LOCAL IMS_SINT32 SLOT_ID = 0;

class TestJniMtsService : public JniMtsService
{
public:
    inline explicit TestJniMtsService(
            IN Jni_SendDataToJava pfnSendDataToJava, IN IMS_SINT32 nSlotId = 0) :
            JniMtsService(pfnSendDataToJava, nSlotId)
    {
    }

    inline virtual IMS_BOOL IsThreadSwitchingRequired(IN IMS_SINT32 /*nMsg*/) const override
    {
        return IMS_FALSE;
    }
};

class JniMtsServiceTest : public ::testing::Test
{
public:
    MockIMtsService objMockService;
    Parcel objParcel;
    JniMtsService* pJniService;

protected:
    virtual void SetUp() override
    {
        ON_CALL(objMockService, NotifyJniEnablerSet).WillByDefault(Return());
        JniEnablerConnector::GetInstance().SetNativeEnabler(
                SLOT_ID, EnablerType::MTS_SERVICE, &objMockService);

        pJniService = new TestJniMtsService(reinterpret_cast<Jni_SendDataToJava>(0x01), SLOT_ID);
    }

    virtual void TearDown() override
    {
        delete pJniService;
        JniEnablerConnector::GetInstance().SetNativeEnabler(
                SLOT_ID, EnablerType::MTS_SERVICE, IMS_NULL);
    }
};

TEST_F(JniMtsServiceTest, CreatesJniMtsServiceThread)
{
    EXPECT_NE(nullptr, pJniService->GetJniThread());
}

TEST_F(JniMtsServiceTest, SendDataMoSms3gpp)
{
    objParcel.writeInt32(IuMtsService::NOTI_MTSENABLER_SEND_MO_SMS);
    objParcel.writeInt32(SMSFORMAT_3GPP);
    objParcel.setDataPosition(0);

    EXPECT_CALL(objMockService, SendMoSms(SmsFormatType::SMSFORMAT_3GPP, _, _, _, _)).Times(1);

    pJniService->SendData(objParcel);
}

TEST_F(JniMtsServiceTest, SendDataMoSms3gpp2)
{
    objParcel.writeInt32(IuMtsService::NOTI_MTSENABLER_SEND_MO_SMS);
    objParcel.writeInt32(SMSFORMAT_3GPP2);
    objParcel.setDataPosition(0);

    EXPECT_CALL(objMockService, SendMoSms(SmsFormatType::SMSFORMAT_3GPP2, _, _, _, _)).Times(1);

    pJniService->SendData(objParcel);
}

TEST_F(JniMtsServiceTest, SendDataMtResult)
{
    objParcel.writeInt32(IuMtsService::NOTI_MTSENABLER_SEND_MT_RESULT);
    objParcel.setDataPosition(0);

    EXPECT_CALL(objMockService, SendMtResult(_)).Times(1);

    pJniService->SendData(objParcel);
}

}  // namespace android
