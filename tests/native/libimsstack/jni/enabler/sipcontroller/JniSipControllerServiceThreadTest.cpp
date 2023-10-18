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
#include "AString.h"
#include "ImsProcess.h"
#include "ImsTypeDef.h"
#include "ISipControllerService.h"
#include "IURcsMessageService.h"
#include "JniSipControllerServiceThread.h"
#include "MockIThread.h"
#include "PlatformContext.h"
#include "TestThreadService.h"
#include <binder/Parcel.h>
#include <gtest/gtest.h>

using ::testing::_;

#define MESSAGE_THREAD_SWITCHING 0

namespace android
{
MATCHER_P3(IsRegistrationUpdated, type, featureCount, regState, "")
{
    android::Parcel* pParcel = reinterpret_cast<android::Parcel*>(arg);
    IMS_UINT32 eType = pParcel->readInt32();
    IMS_UINT32 data1 = pParcel->readInt32();
    IMS_UINT32 data2 = pParcel->readInt32();
    return type == eType && featureCount == data1 && regState == data2;
}

class JniSipControllerServiceThreadTest : public ::testing::Test
{
public:
    inline JniSipControllerServiceThreadTest() :
            pServiceThread(IMS_NULL),
            pThreadService(new TestThreadService())
    {
        PlatformContext::GetInstance()->SetService(PlatformContext::SERVICE_THREAD, pThreadService);
        pThreadService->SetThread(&objMockThread);
        CreateServiceThread();
    }
    inline virtual ~JniSipControllerServiceThreadTest()
    {
        delete pServiceThread;
        PlatformContext::GetInstance()->SetService(PlatformContext::SERVICE_THREAD, IMS_NULL);
        delete pThreadService;
    }

public:
    JniSipControllerServiceThread* pServiceThread;
    TestThreadService* pThreadService;
    MockIThread objMockThread;

protected:
    virtual void SetUp() override {}

    virtual void TearDown() override {}

    void CreateServiceThread()
    {
        pServiceThread = new JniSipControllerServiceThread();
        pServiceThread->Start("", IMS_SLOT_0);
        Jni_SendDataToJava pfnSendDataToJava = reinterpret_cast<Jni_SendDataToJava>(0x01);
        pServiceThread->SetCallback(0x02, pfnSendDataToJava);
    }
};

TEST_F(JniSipControllerServiceThreadTest, OnRegistrationUpdated)
{
    IMS_UINT32 featureCount = 1;
    IMS_UINT32 regState = 3;
    AString featureTag = "featureTag";

    IMS_UINT32 eType = IUSncControl::ONREGISTRATION_UPDATED_IND;
    Parcel objParcel;
    objParcel.writeInt32(eType);
    objParcel.writeInt32(featureCount);
    objParcel.writeInt32(regState);
    objParcel.writeString16(android::String16(featureTag.GetStr()));
    objParcel.setDataPosition(0);

    IUSncFeatureTagsParam* pParam = new IUSncFeatureTagsParam();
    pParam->m_nFeatureCount = featureCount;
    pParam->m_nRegState = regState;
    pParam->m_objFeatureTags.AddElement(featureTag.GetStr());
    IMS_UINTP nParam = reinterpret_cast<IMS_UINTP>(pParam);

    EXPECT_CALL(objMockThread,
            PostMessageI(MESSAGE_THREAD_SWITCHING, _,
                    IsRegistrationUpdated(eType, featureCount, regState)))
            .Times(1);

    pServiceThread->OnRegistrationUpdated(nParam);
}
}  // namespace android
