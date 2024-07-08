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
#include "CallReasonInfo.h"
#include "IMtcService.h"
#include "ImsMap.h"
#include "ImsProcess.h"
#include "ImsTypeDef.h"
#include "IuMtcService.h"
#include "JniCallInfo.h"
#include "JniMtcServiceThread.h"
#include "MockIThread.h"
#include "MtcDef.h"
#include "PlatformContext.h"
#include "TestThreadService.h"
#include <binder/Parcel.h>
#include <gtest/gtest.h>

using ::testing::_;

// BaseServiceThread::MESSAGE_THREAD_SWITCHING = 0
#define MESSAGE_THREAD_SWITCHING 0

namespace android
{

MATCHER_P(IsSameMessageType, type, "")
{
    android::Parcel* pParcel = reinterpret_cast<android::Parcel*>(arg);
    IMS_UINT32 eType = pParcel->readInt32();
    return type == eType;
}

MATCHER_P2(IsSameMessageTypeAndState, type, state, "")
{
    android::Parcel* pParcel = reinterpret_cast<android::Parcel*>(arg);
    IMS_UINT32 eType = pParcel->readInt32();
    IMS_SINT32 eState = pParcel->readInt32();
    return type == eType && static_cast<IMS_SINT32>(state) == eState;
}

class JniMtcServiceThreadTest : public ::testing::Test
{
public:
    inline JniMtcServiceThreadTest() :
            pJniServiceThread(IMS_NULL),
            pThreadService(new TestThreadService())
    {
        PlatformContext::GetInstance()->SetService(PlatformContext::SERVICE_THREAD, pThreadService);
        pThreadService->SetThread(&objMockThread);
        CreateJniMtcServiceThread();
    }
    inline virtual ~JniMtcServiceThreadTest()
    {
        delete pJniServiceThread;
        PlatformContext::GetInstance()->SetService(PlatformContext::SERVICE_THREAD, IMS_NULL);
        delete pThreadService;
    }

public:
    Parcel objParcel;
    JniMtcServiceThread* pJniServiceThread;
    TestThreadService* pThreadService;
    MockIThread objMockThread;

protected:
    virtual void SetUp() override {}

    virtual void TearDown() override {}

    void CreateJniMtcServiceThread()
    {
        pJniServiceThread = new JniMtcServiceThread();
        pJniServiceThread->Start("", IMS_SLOT_0);
        Jni_SendDataToJava pfnSendDataToJava = reinterpret_cast<Jni_SendDataToJava>(0x01);
        pJniServiceThread->SetCallback(0x02, pfnSendDataToJava);
    }
};

TEST_F(JniMtcServiceThreadTest, OnServiceChanged)
{
    IMS_UINT32 eType = IuMtcService::SERVICE_CHANGED;
    IuMtcService::ServiceState eState = IuMtcService::ServiceState::SERVICE_UC;
    objParcel.writeInt32(eType);
    objParcel.setDataPosition(0);

    EXPECT_CALL(objMockThread,
            PostMessageI(MESSAGE_THREAD_SWITCHING, _, IsSameMessageTypeAndState(eType, eState)))
            .Times(1);

    pJniServiceThread->OnServiceChanged(eState, 0);
}

TEST_F(JniMtcServiceThreadTest, OnEmergencyServiceChanged)
{
    IMS_UINT32 eType = IuMtcService::E_SERVICE_CHANGED;
    IuMtcService::EmergencyServiceState eState = IuMtcService::EmergencyServiceState::OPENED;
    ServiceType eServiceType = ServiceType::EMERGENCY;
    objParcel.writeInt32(eType);
    objParcel.setDataPosition(0);

    EXPECT_CALL(objMockThread,
            PostMessageI(MESSAGE_THREAD_SWITCHING, _,
                    IsSameMessageTypeAndState(eType, static_cast<IMS_SINT32>(eState))))
            .Times(1);

    pJniServiceThread->OnEmergencyServiceChanged(eState, -1, eServiceType);
}

TEST_F(JniMtcServiceThreadTest, OnPreIncomingCallReceived)
{
    IMS_UINT32 eType = IuMtcService::PRE_INCOMING_CALL;
    objParcel.writeInt32(eType);
    objParcel.setDataPosition(0);

    EXPECT_CALL(objMockThread, PostMessageI(MESSAGE_THREAD_SWITCHING, _, IsSameMessageType(eType)))
            .Times(1);

    pJniServiceThread->OnPreIncomingCallReceived(1);
}

TEST_F(JniMtcServiceThreadTest, OnRejectedIncomingCall)
{
    IMS_UINT32 eType = IuMtcService::AUTO_REJECTED_CALL;
    objParcel.writeInt32(eType);
    objParcel.setDataPosition(0);

    EXPECT_CALL(objMockThread, PostMessageI(MESSAGE_THREAD_SWITCHING, _, IsSameMessageType(eType)))
            .Times(1);

    JniCallInfo objCallInfo;
    MediaInfo objMediaInfo;
    ImsMap<SuppType, SuppService*> objSuppServices;
    CallReasonInfo objReason(CODE_NONE);
    pJniServiceThread->OnRejectedIncomingCall(
            objCallInfo, objMediaInfo, objSuppServices, OipType::NONE, "", objReason);
}

TEST_F(JniMtcServiceThreadTest, OnJniReady)
{
    IMS_UINT32 eType = IuMtcService::JNI_READY;
    objParcel.writeInt32(eType);
    objParcel.setDataPosition(0);

    EXPECT_CALL(objMockThread, PostMessageI(MESSAGE_THREAD_SWITCHING, _, IsSameMessageType(eType)))
            .Times(1);

    pJniServiceThread->OnJniReady();
}

TEST_F(JniMtcServiceThreadTest, OnExternalCallsChanged)
{
    IMS_UINT32 eType = IuMtcService::EXTERNAL_CALLS_CHANGED;
    objParcel.writeInt32(eType);
    objParcel.setDataPosition(0);

    EXPECT_CALL(objMockThread, PostMessageI(MESSAGE_THREAD_SWITCHING, _, IsSameMessageType(eType)))
            .Times(1);

    ImsList<const JniExternalCall*> objJniExternalCall;
    pJniServiceThread->OnExternalCallsChanged(objJniExternalCall);
}

}  // namespace android
