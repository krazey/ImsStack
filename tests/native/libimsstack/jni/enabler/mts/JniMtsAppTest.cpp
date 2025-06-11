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

#include "IuMtsApp.h"
#include "JniEnablerConnector.h"
#include "JniMtsApp.h"
#include "MockIMtsService.h"
#include "MockIMtsJni.h"
#include <binder/Parcel.h>
#include <gtest/gtest.h>

using ::testing::_;
using ::testing::Return;

namespace android
{

LOCAL IMS_SINT32 SLOT_ID = 0;

class TestJniMtsApp : public JniMtsApp
{
public:
    inline explicit TestJniMtsApp(
            IN Jni_SendDataToJava pfnSendDataToJava, IN IMS_SINT32 nSlotId = 0) :
            JniMtsApp(pfnSendDataToJava, nSlotId)
    {
    }

    inline virtual IMS_BOOL IsThreadSwitchingRequired(IN IMS_SINT32 /*nMsg*/) const override
    {
        return IMS_FALSE;
    }
};

class JniMtsAppTest : public ::testing::Test
{
public:
    MockIMtsJni objMockMtsJni;
    Parcel objParcel;
    JniMtsApp* pJniApp;

protected:
    virtual void SetUp() override
    {
        ON_CALL(objMockMtsJni, NotifyJniEnablerSet).WillByDefault(Return());
        JniEnablerConnector::GetInstance().SetNativeEnabler(
                SLOT_ID, EnablerType::MTS, &objMockMtsJni);

        pJniApp = new TestJniMtsApp(reinterpret_cast<Jni_SendDataToJava>(0x01), SLOT_ID);
    }

    virtual void TearDown() override
    {
        delete pJniApp;
        JniEnablerConnector::GetInstance().SetNativeEnabler(SLOT_ID, EnablerType::MTS, IMS_NULL);
    }
};

TEST_F(JniMtsAppTest, CreatesJniMtsAppThread)
{
    EXPECT_NE(nullptr, pJniApp->GetJniThread());
}

TEST_F(JniMtsAppTest, SendDataMoSms3gpp)
{
    objParcel.writeInt32(IuMtsApp::NOTI_MTSENABLER_SEND_MO_SMS);
    objParcel.writeInt32(SMSFORMAT_3GPP);
    objParcel.setDataPosition(0);

    EXPECT_CALL(objMockMtsJni, SendMoSmsByServiceType(SmsFormatType::SMSFORMAT_3GPP, _, _, _, _, _))
            .Times(1);

    pJniApp->SendData(objParcel);
}

TEST_F(JniMtsAppTest, SendDataMoSms3gpp2)
{
    objParcel.writeInt32(IuMtsApp::NOTI_MTSENABLER_SEND_MO_SMS);
    objParcel.writeInt32(SMSFORMAT_3GPP2);
    objParcel.setDataPosition(0);

    EXPECT_CALL(
            objMockMtsJni, SendMoSmsByServiceType(SmsFormatType::SMSFORMAT_3GPP2, _, _, _, _, _))
            .Times(1);

    pJniApp->SendData(objParcel);
}

}  // namespace android
