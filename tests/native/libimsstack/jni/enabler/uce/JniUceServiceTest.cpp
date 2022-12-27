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
#include "BaseService.h"
#include "JniEnablerConnector.h"
#include "JniUceService.h"
#include "MockIUceJni.h"
#include "IUUceService.h"
#include <binder/Parcel.h>
#include <gtest/gtest.h>

using ::testing::_;
using ::testing::Return;

namespace android
{

LOCAL IMS_SINT32 SLOT_ID = 0;

class TestJniUceService : public JniUceService
{
public:
    inline explicit TestJniUceService(
            IN Jni_SendDataToJava pfnSendDataToJava, IN IMS_SINT32 nSlotId = 0) :
            JniUceService(pfnSendDataToJava, nSlotId)
    {
    }

    inline virtual IMS_BOOL IsThreadSwitchingRequired(IN IMS_SINT32 /*nMsg*/) const override
    {
        return IMS_FALSE;
    }
};

class JniUceServiceTest : public ::testing::Test
{
public:
    MockIUceJni objMockIUceJni;
    Parcel objParcel;
    JniUceService* pJniService;

protected:
    virtual void SetUp() override
    {
        ON_CALL(objMockIUceJni, NotifyJniEnablerSet).WillByDefault(Return());
        JniEnablerConnector::GetInstance().SetNativeEnabler(
                SLOT_ID, EnablerType::UCE, &objMockIUceJni);

        pJniService = new TestJniUceService(reinterpret_cast<Jni_SendDataToJava>(0x01), SLOT_ID);
    }

    virtual void TearDown() override
    {
        delete pJniService;
        JniEnablerConnector::GetInstance().SetNativeEnabler(SLOT_ID, EnablerType::UCE, IMS_NULL);
    }
};

TEST_F(JniUceServiceTest, CreatesJniUceServiceThread)
{
    EXPECT_NE(nullptr, pJniService->GetJniThread());
}

TEST_F(JniUceServiceTest, SendPublishCmd)
{
    objParcel.writeInt32(IUUceService::UCE_SEND_PUBLISH_CMD);
    objParcel.writeInt32(0);
    objParcel.writeString16(android::String16(AString("pidfXml").GetStr()));
    objParcel.writeInt32(0);
    objParcel.writeInt32(0);
    objParcel.writeInt32(0);
    objParcel.setDataPosition(0);

    EXPECT_CALL(objMockIUceJni, SendPublishCmd(_, _, _, _, _)).Times(1);
    pJniService->SendData(objParcel);
}

TEST_F(JniUceServiceTest, SendSingleSubscribeCmd)
{
    objParcel.writeInt32(IUUceService::UCE_SEND_SINGLE_SUBSCRIBE_CMD);
    objParcel.writeInt32(0);
    objParcel.writeInt32(0);
    objParcel.writeString16(android::String16(AString("user").GetStr()));
    objParcel.setDataPosition(0);

    EXPECT_CALL(objMockIUceJni, SendSingleSubscribeCmd(_, _)).Times(1);

    pJniService->SendData(objParcel);
}

TEST_F(JniUceServiceTest, SendListSubscribeCmd)
{
    objParcel.writeInt32(IUUceService::UCE_SEND_LIST_SUBSCRIBE_CMD);
    objParcel.writeInt32(0);
    objParcel.writeInt32(1);
    objParcel.writeString16(android::String16(AString("user").GetStr()));
    objParcel.setDataPosition(0);

    EXPECT_CALL(objMockIUceJni, SendListSubscribeCmd(_, _)).Times(1);

    pJniService->SendData(objParcel);
}

TEST_F(JniUceServiceTest, SendOptionsCmd)
{
    objParcel.writeInt32(IUUceService::UCE_SEND_OPTIONS_CMD);
    objParcel.writeInt32(0);
    objParcel.writeString16(android::String16(AString("user").GetStr()));
    objParcel.setDataPosition(0);

    EXPECT_CALL(objMockIUceJni, SendOptionsCmd(_, _, _)).Times(1);

    pJniService->SendData(objParcel);
}

TEST_F(JniUceServiceTest, SendOptionsRespCmd)
{
    objParcel.writeInt32(IUUceService::UCE_SEND_OPTIONS_RESP_CMD);
    objParcel.writeInt32(0);
    objParcel.writeInt32(200);
    objParcel.writeString16(android::String16(AString("reason").GetStr()));
    objParcel.writeInt32(0);
    objParcel.setDataPosition(0);

    EXPECT_CALL(objMockIUceJni, SendOptionsRespCmd(_, _, _, _)).Times(1);

    pJniService->SendData(objParcel);
}

TEST_F(JniUceServiceTest, ImsRegistrationCheck)
{
    objParcel.writeInt32(IUUceService::UCE_GET_IMS_REGISTRATION_CMD);
    objParcel.setDataPosition(0);

    EXPECT_CALL(objMockIUceJni, ImsRegistrationCheck()).Times(1);

    pJniService->SendData(objParcel);
}

}  // namespace android
