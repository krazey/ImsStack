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

#include "ImsTypeDef.h"
#include "IuMtsApp.h"
#include "JniMtsAppThread.h"
#include "MockIThread.h"
#include "MtsDef.h"
#include "PlatformContext.h"
#include "TestThreadService.h"
#include <binder/Parcel.h>
#include <gtest/gtest.h>

using ::testing::_;

#define MESSAGE_THREAD_SWITCHING 0

namespace android
{

MATCHER_P(IsSameMessageType, type, "")
{
    android::Parcel* pParcel = reinterpret_cast<android::Parcel*>(arg);
    IMS_UINT32 eType = pParcel->readInt32();
    return type == eType;
}

class JniMtsAppThreadTest : public ::testing::Test
{
public:
    inline JniMtsAppThreadTest() :
            pJniAppThread(IMS_NULL),
            pThreadService(new TestThreadService())
    {
        PlatformContext::GetInstance()->SetService(PlatformContext::SERVICE_THREAD, pThreadService);
        pThreadService->SetThread(&objMockThread);
        CreateJniMtsAppThread();
    }
    inline virtual ~JniMtsAppThreadTest()
    {
        delete pJniAppThread;
        PlatformContext::GetInstance()->SetService(PlatformContext::SERVICE_THREAD, IMS_NULL);
        delete pThreadService;
    }

public:
    Parcel objParcel;
    JniMtsAppThread* pJniAppThread;
    TestThreadService* pThreadService;
    MockIThread objMockThread;

protected:
    void CreateJniMtsAppThread()
    {
        pJniAppThread = new JniMtsAppThread();
        pJniAppThread->Start("", IMS_SLOT_0);
        Jni_SendDataToJava pfnSendDataToJava = reinterpret_cast<Jni_SendDataToJava>(0x01);
        pJniAppThread->SetCallback(0x02, pfnSendDataToJava);
    }
};

TEST_F(JniMtsAppThreadTest, ReportMoStatus)
{
    IMS_UINT32 eType = IuMtsApp::REPORT_MTS_MO_STATUS;

    EXPECT_CALL(objMockThread, PostMessageI(MESSAGE_THREAD_SWITCHING, _, IsSameMessageType(eType)))
            .Times(1);

    pJniAppThread->ReportMoStatus(MO_SUCCESS, SmsFormatType::SMSFORMAT_3GPP, 1, IMS_SLOT_0);
}

TEST_F(JniMtsAppThreadTest, ReportMtSms)
{
    IMS_UINT32 eType = IuMtsApp::REPORT_MTS_MT_SMS;
    ByteArray objRpData((IMS_BYTE)0x01);  // message type indicator(RP-MT-DATA)
    objRpData.Append((IMS_BYTE)0x03);     // message reference
    objRpData.Append((IMS_BYTE)0x0F);     // other required information elements

    EXPECT_CALL(objMockThread, PostMessageI(MESSAGE_THREAD_SWITCHING, _, IsSameMessageType(eType)))
            .Times(1);

    pJniAppThread->ReportMtSms(SmsFormatType::SMSFORMAT_3GPP, objRpData, IMS_SLOT_0);
}

}  // namespace android
