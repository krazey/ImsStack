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
#include "IuMtcService.h"
#include "JniEnablerConnector.h"
#include "JniMtcService.h"
#include "MockIMtcService.h"
#include <binder/Parcel.h>
#include <gtest/gtest.h>

using ::testing::_;
using ::testing::Return;

namespace android
{

LOCAL IMS_SINT32 SLOT_ID = 0;

class TestJniMtcService : public JniMtcService
{
public:
    inline explicit TestJniMtcService(
            IN Jni_SendDataToJava pfnSendDataToJava, IN IMS_SINT32 nSlotId = 0) :
            JniMtcService(pfnSendDataToJava, nSlotId)
    {
    }

    inline virtual IMS_BOOL IsThreadSwitchingRequired(IN IMS_SINT32 /*nMsg*/) const override
    {
        return IMS_FALSE;
    }
};

class JniMtcServiceTest : public ::testing::Test
{
public:
    MockIMtcService objMockService;
    Parcel objParcel;
    JniMtcService* pJniService;

protected:
    virtual void SetUp() override
    {
        ON_CALL(objMockService, NotifyJniEnablerSet).WillByDefault(Return());
        JniEnablerConnector::GetInstance().SetNativeEnabler(
                SLOT_ID, EnablerType::MTC_SERVICE, &objMockService);

        pJniService = new TestJniMtcService(reinterpret_cast<Jni_SendDataToJava>(0x01), SLOT_ID);
    }

    virtual void TearDown() override
    {
        delete pJniService;
        JniEnablerConnector::GetInstance().SetNativeEnabler(
                SLOT_ID, EnablerType::MTC_SERVICE, IMS_NULL);
    }
};

TEST_F(JniMtcServiceTest, CreatesJniMtcServiceThread)
{
    EXPECT_NE(nullptr, pJniService->GetJniThread());
}

TEST_F(JniMtcServiceTest, SendDataUpdateSrvccState)
{
    objParcel.writeInt32(IuMtcService::SRVCC_STATE_CHANGED);
    objParcel.setDataPosition(0);

    EXPECT_CALL(objMockService, UpdateSrvccState(_)).Times(1);

    pJniService->SendData(objParcel);
}

TEST_F(JniMtcServiceTest, SendDataSetTbcw)
{
    objParcel.writeInt32(IuMtcService::SET_TERMINAL_BASED_CALL_WAITING);
    objParcel.setDataPosition(0);

    EXPECT_CALL(objMockService, SetTerminalBasedCallWaiting(_)).Times(1);

    pJniService->SendData(objParcel);
}

TEST_F(JniMtcServiceTest, SendDataOpenEmergencyService)
{
    objParcel.writeInt32(IuMtcService::OPEN_EMERGENCY_SERVICE);
    objParcel.writeInt32(static_cast<IMS_SINT32>(ServiceType::EMERGENCY));
    objParcel.setDataPosition(0);

    EXPECT_CALL(objMockService, OpenEmergencyService(ServiceType::EMERGENCY)).Times(1);

    pJniService->SendData(objParcel);
}

}  // namespace android
