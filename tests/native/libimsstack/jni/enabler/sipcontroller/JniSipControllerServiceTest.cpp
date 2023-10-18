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
#include "BaseService.h"
#include "IURcsMessageService.h"
#include "JniEnablerConnector.h"
#include "JniSipControllerService.h"
#include "MockISipControllerService.h"
#include <binder/Parcel.h>
#include <gtest/gtest.h>

using ::testing::_;
using ::testing::Return;

namespace android
{
LOCAL IMS_SINT32 SLOT_ID = 0;
class TestJniSipControllerService : public JniSipControllerService
{
public:
    inline explicit TestJniSipControllerService(
            IN Jni_SendDataToJava pfnSendDataToJava, IN IMS_SINT32 nSlotId = 0) :
            JniSipControllerService(pfnSendDataToJava, nSlotId)
    {
    }

    inline virtual IMS_BOOL IsThreadSwitchingRequired(IN IMS_SINT32 /*nMsg*/) const override
    {
        return IMS_FALSE;
    }
};

class JniSipControllerServiceTest : public ::testing::Test
{
public:
    Parcel objParcel;
    MockISipControllerService objMockJni;
    JniSipControllerService* pJniService;

protected:
    virtual void SetUp() override
    {
        ON_CALL(objMockJni, NotifyJniEnablerSet).WillByDefault(Return());
        JniEnablerConnector::GetInstance().SetNativeEnabler(
                SLOT_ID, EnablerType::SIP_DELEGATE, &objMockJni);
        pJniService = new TestJniSipControllerService(
                reinterpret_cast<Jni_SendDataToJava>(0x01), SLOT_ID);
    }

    virtual void TearDown() override
    {
        delete pJniService;
        JniEnablerConnector::GetInstance().SetNativeEnabler(
                SLOT_ID, EnablerType::SIP_DELEGATE, IMS_NULL);
    }
};

TEST_F(JniSipControllerServiceTest, CreatesJniSipControllerServiceThread)
{
    EXPECT_NE(nullptr, pJniService->GetJniThread());
}

TEST_F(JniSipControllerServiceTest, SendMessage)
{
    objParcel.writeInt32(IUSncService::SEND_MESSAGE_CMD);
    objParcel.writeString16(android::String16(AString("startline").GetStr()));
    objParcel.writeString16(android::String16(AString("headersection").GetStr()));
    objParcel.writeInt32(0);
    objParcel.writeString16(android::String16(AString("content").GetStr()));
    objParcel.writeString16(android::String16(AString("method").GetStr()));
    objParcel.writeString16(android::String16(AString("from").GetStr()));
    objParcel.writeString16(android::String16(AString("to").GetStr()));
    objParcel.writeInt32(0);
    objParcel.setDataPosition(0);
    EXPECT_CALL(objMockJni, SendMessage(_)).Times(1);
    pJniService->SendData(objParcel);
}

TEST_F(JniSipControllerServiceTest, CloseSession)
{
    objParcel.writeInt32(IUSncService::CLOSE_SESSION_CMD);
    objParcel.writeString16(android::String16(AString("CallId").GetStr()));
    objParcel.setDataPosition(0);
    EXPECT_CALL(objMockJni, CloseSession(_)).Times(1);
    pJniService->SendData(objParcel);
}

TEST_F(JniSipControllerServiceTest, NotifyMessageReceiveError)
{
    objParcel.writeInt32(IUSncService::NOTIFY_MESSAGE_RECEIVE_ERROR_CMD);
    objParcel.writeString16(android::String16(AString("viaTransactionId").GetStr()));
    objParcel.setDataPosition(0);
    EXPECT_CALL(objMockJni, NotifyMessageReceiveError(_)).Times(1);
    pJniService->SendData(objParcel);
}

TEST_F(JniSipControllerServiceTest, UpdateDelegateRegistration)
{
    objParcel.writeInt32(IUSncControl::UPDATE_SIPREGISTRATION_CMD);
    objParcel.writeInt32(1);
    objParcel.writeString16(android::String16(AString("featuretag").GetStr()));
    objParcel.setDataPosition(0);
    EXPECT_CALL(objMockJni, UpdateDelegateRegistration(_)).Times(1);
    pJniService->SendData(objParcel);
}

TEST_F(JniSipControllerServiceTest, TriggerDelegateDeregistration)
{
    objParcel.writeInt32(IUSncControl::TRIGGER_SIPDEREGISTRATION_CMD);
    objParcel.setDataPosition(0);
    EXPECT_CALL(objMockJni, TriggerDelegateDeregistration()).Times(1);
    pJniService->SendData(objParcel);
}
}  // namespace android
