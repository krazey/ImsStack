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
#include "BaseServiceThread.h"
#include "CallReasonInfo.h"
#include "ImsProcess.h"
#include "IuMtcCall.h"
#include "JniCallInfo.h"
#include "JniMtcCallThread.h"
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

class JniMtcCallThreadTest : public ::testing::Test
{
public:
    inline JniMtcCallThreadTest() :
            pJniCallThread(IMS_NULL),
            pThreadService(new TestThreadService()),
            objReason(CODE_UNSPECIFIED)
    {
        PlatformContext::GetInstance()->SetService(PlatformContext::SERVICE_THREAD, pThreadService);
        pThreadService->SetThread(&objMockThread);
        CreateJniMtcCallThread();
    }
    inline virtual ~JniMtcCallThreadTest()
    {
        objSuppServices.Clear();

        delete pJniCallThread;
        PlatformContext::GetInstance()->SetService(PlatformContext::SERVICE_THREAD, IMS_NULL);
        delete pThreadService;
    }

public:
    Parcel objParcel;
    JniMtcCallThread* pJniCallThread;
    TestThreadService* pThreadService;
    MockIThread objMockThread;
    CallReasonInfo objReason;
    JniCallInfo objCallInfo;
    MediaInfo objMediaInfo;
    ImsMap<SuppType, SuppService*> objSuppServices;

protected:
    virtual void SetUp() override {}

    virtual void TearDown() override {}

    void CreateJniMtcCallThread()
    {
        pJniCallThread = new JniMtcCallThread();
        pJniCallThread->Start("", IMS_SLOT_0);
        Jni_SendDataToJava pfnSendDataToJava = reinterpret_cast<Jni_SendDataToJava>(0x01);
        pJniCallThread->SetCallback(0x02, pfnSendDataToJava);
    }
};

TEST_F(JniMtcCallThreadTest, OnStarted)
{
    IMS_UINT32 eType = IuMtcCall::STARTED;
    objParcel.writeInt32(eType);
    objParcel.setDataPosition(0);

    EXPECT_CALL(objMockThread, PostMessageI(MESSAGE_THREAD_SWITCHING, _, IsSameMessageType(eType)))
            .Times(1);

    pJniCallThread->OnStarted(objCallInfo, objMediaInfo, objSuppServices);
}

TEST_F(JniMtcCallThreadTest, OnStartFailed)
{
    IMS_UINT32 eType = IuMtcCall::START_FAILED;
    objParcel.writeInt32(eType);
    objParcel.setDataPosition(0);

    EXPECT_CALL(objMockThread, PostMessageI(MESSAGE_THREAD_SWITCHING, _, IsSameMessageType(eType)))
            .Times(1);

    pJniCallThread->OnStartFailed(objReason);
}

TEST_F(JniMtcCallThreadTest, OnInitiating)
{
    IMS_UINT32 eType = IuMtcCall::INITIATING;
    objParcel.writeInt32(eType);
    objParcel.setDataPosition(0);

    EXPECT_CALL(objMockThread, PostMessageI(MESSAGE_THREAD_SWITCHING, _, IsSameMessageType(eType)))
            .Times(1);

    pJniCallThread->OnInitiating(objCallInfo, objMediaInfo);
}

TEST_F(JniMtcCallThreadTest, OnProgressing)
{
    IMS_UINT32 eType = IuMtcCall::PROGRESSING;
    objParcel.writeInt32(eType);
    objParcel.setDataPosition(0);

    EXPECT_CALL(objMockThread, PostMessageI(MESSAGE_THREAD_SWITCHING, _, IsSameMessageType(eType)))
            .Times(1);

    pJniCallThread->OnProgressing(objCallInfo, objMediaInfo, objSuppServices);
}

TEST_F(JniMtcCallThreadTest, OnHeld)
{
    IMS_UINT32 eType = IuMtcCall::HELD;
    objParcel.writeInt32(eType);
    objParcel.setDataPosition(0);

    EXPECT_CALL(objMockThread, PostMessageI(MESSAGE_THREAD_SWITCHING, _, IsSameMessageType(eType)))
            .Times(1);

    pJniCallThread->OnHeld(objCallInfo, objMediaInfo, objSuppServices);
}

TEST_F(JniMtcCallThreadTest, OnHoldFailed)
{
    IMS_UINT32 eType = IuMtcCall::HOLD_FAILED;
    objParcel.writeInt32(eType);
    objParcel.setDataPosition(0);

    EXPECT_CALL(objMockThread, PostMessageI(MESSAGE_THREAD_SWITCHING, _, IsSameMessageType(eType)))
            .Times(1);

    pJniCallThread->OnHoldFailed(objReason);
}

TEST_F(JniMtcCallThreadTest, OnResumed)
{
    IMS_UINT32 eType = IuMtcCall::RESUMED;
    objParcel.writeInt32(eType);
    objParcel.setDataPosition(0);

    EXPECT_CALL(objMockThread, PostMessageI(MESSAGE_THREAD_SWITCHING, _, IsSameMessageType(eType)))
            .Times(1);

    pJniCallThread->OnResumed(objCallInfo, objMediaInfo, objSuppServices);
}

TEST_F(JniMtcCallThreadTest, OnResumeFailed)
{
    IMS_UINT32 eType = IuMtcCall::RESUME_FAILED;
    objParcel.writeInt32(eType);
    objParcel.setDataPosition(0);

    EXPECT_CALL(objMockThread, PostMessageI(MESSAGE_THREAD_SWITCHING, _, IsSameMessageType(eType)))
            .Times(1);

    pJniCallThread->OnResumeFailed(objReason);
}

TEST_F(JniMtcCallThreadTest, OnHeldBy)
{
    IMS_UINT32 eType = IuMtcCall::HELD_BY;
    objParcel.writeInt32(eType);
    objParcel.setDataPosition(0);

    EXPECT_CALL(objMockThread, PostMessageI(MESSAGE_THREAD_SWITCHING, _, IsSameMessageType(eType)))
            .Times(1);

    pJniCallThread->OnHeldBy(objCallInfo, objMediaInfo, objSuppServices);
}

TEST_F(JniMtcCallThreadTest, OnResumedBy)
{
    IMS_UINT32 eType = IuMtcCall::RESUMED_BY;
    objParcel.writeInt32(eType);
    objParcel.setDataPosition(0);

    EXPECT_CALL(objMockThread, PostMessageI(MESSAGE_THREAD_SWITCHING, _, IsSameMessageType(eType)))
            .Times(1);

    pJniCallThread->OnResumedBy(objCallInfo, objMediaInfo, objSuppServices);
}

TEST_F(JniMtcCallThreadTest, OnTerminated)
{
    IMS_UINT32 eType = IuMtcCall::TERMINATED;
    objParcel.writeInt32(eType);
    objParcel.setDataPosition(0);

    EXPECT_CALL(objMockThread, PostMessageI(MESSAGE_THREAD_SWITCHING, _, IsSameMessageType(eType)))
            .Times(1);

    pJniCallThread->OnTerminated(objReason);
}

TEST_F(JniMtcCallThreadTest, OnIncomingResume)
{
    IMS_UINT32 eType = IuMtcCall::INCOMING_RESUME;
    objParcel.writeInt32(eType);
    objParcel.setDataPosition(0);

    EXPECT_CALL(objMockThread, PostMessageI(MESSAGE_THREAD_SWITCHING, _, IsSameMessageType(eType)))
            .Times(1);

    pJniCallThread->OnIncomingResume(objCallInfo, objMediaInfo, objSuppServices);
}

TEST_F(JniMtcCallThreadTest, OnUpdated)
{
    IMS_UINT32 eType = IuMtcCall::UPDATED;
    objParcel.writeInt32(eType);
    objParcel.setDataPosition(0);

    EXPECT_CALL(objMockThread, PostMessageI(MESSAGE_THREAD_SWITCHING, _, IsSameMessageType(eType)))
            .Times(1);

    pJniCallThread->OnUpdated(objCallInfo, objMediaInfo, objSuppServices);
}

TEST_F(JniMtcCallThreadTest, OnUpdateFailed)
{
    IMS_UINT32 eType = IuMtcCall::UPDATE_FAILED;
    objParcel.writeInt32(eType);
    objParcel.setDataPosition(0);

    EXPECT_CALL(objMockThread, PostMessageI(MESSAGE_THREAD_SWITCHING, _, IsSameMessageType(eType)))
            .Times(1);

    pJniCallThread->OnUpdateFailed(objReason);
}

TEST_F(JniMtcCallThreadTest, OnUpdatedBy)
{
    IMS_UINT32 eType = IuMtcCall::UPDATED_BY;
    objParcel.writeInt32(eType);
    objParcel.setDataPosition(0);

    EXPECT_CALL(objMockThread, PostMessageI(MESSAGE_THREAD_SWITCHING, _, IsSameMessageType(eType)))
            .Times(1);

    pJniCallThread->OnUpdatedBy(objCallInfo, objMediaInfo, objSuppServices);
}

TEST_F(JniMtcCallThreadTest, OnMerged)
{
    IMS_UINT32 eType = IuMtcCall::CONF_MERGED;
    objParcel.writeInt32(eType);
    objParcel.setDataPosition(0);

    EXPECT_CALL(objMockThread, PostMessageI(MESSAGE_THREAD_SWITCHING, _, IsSameMessageType(eType)))
            .Times(1);

    ImsList<ConfUser*> objUsers;
    pJniCallThread->OnMerged(objCallInfo, objMediaInfo, objSuppServices, objUsers);
}

TEST_F(JniMtcCallThreadTest, OnMergeFailed)
{
    IMS_UINT32 eType = IuMtcCall::CONF_MERGEFAILED;
    objParcel.writeInt32(eType);
    objParcel.setDataPosition(0);

    EXPECT_CALL(objMockThread, PostMessageI(MESSAGE_THREAD_SWITCHING, _, IsSameMessageType(eType)))
            .Times(1);

    pJniCallThread->OnMergeFailed(objReason);
}

TEST_F(JniMtcCallThreadTest, OnConferenceParticipantAdded)
{
    IMS_UINT32 eType = IuMtcCall::CONF_JOINED;
    objParcel.writeInt32(eType);
    objParcel.setDataPosition(0);

    EXPECT_CALL(objMockThread, PostMessageI(MESSAGE_THREAD_SWITCHING, _, IsSameMessageType(eType)))
            .Times(1);

    pJniCallThread->OnConferenceParticipantAdded();
}

TEST_F(JniMtcCallThreadTest, OnConferenceParticipantAddFailed)
{
    IMS_UINT32 eType = IuMtcCall::CONF_JOINED;
    objParcel.writeInt32(eType);
    objParcel.setDataPosition(0);

    EXPECT_CALL(objMockThread, PostMessageI(MESSAGE_THREAD_SWITCHING, _, IsSameMessageType(eType)))
            .Times(1);

    pJniCallThread->OnConferenceParticipantAddFailed(objReason);
}

TEST_F(JniMtcCallThreadTest, OnConferenceParticipantRemoved)
{
    IMS_UINT32 eType = IuMtcCall::CONF_DROPPED;
    objParcel.writeInt32(eType);
    objParcel.setDataPosition(0);

    EXPECT_CALL(objMockThread, PostMessageI(MESSAGE_THREAD_SWITCHING, _, IsSameMessageType(eType)))
            .Times(1);

    pJniCallThread->OnConferenceParticipantRemoved();
}

TEST_F(JniMtcCallThreadTest, OnConferenceParticipantRemoveFailed)
{
    IMS_UINT32 eType = IuMtcCall::CONF_DROPPED;
    objParcel.writeInt32(eType);
    objParcel.setDataPosition(0);

    EXPECT_CALL(objMockThread, PostMessageI(MESSAGE_THREAD_SWITCHING, _, IsSameMessageType(eType)))
            .Times(1);

    pJniCallThread->OnConferenceParticipantRemoveFailed(objReason);
}

TEST_F(JniMtcCallThreadTest, OnConferenceInfoChanged)
{
    IMS_UINT32 eType = IuMtcCall::CONF_NOTIFY_CONF_INFO;
    objParcel.writeInt32(eType);
    objParcel.setDataPosition(0);

    EXPECT_CALL(objMockThread, PostMessageI(MESSAGE_THREAD_SWITCHING, _, IsSameMessageType(eType)))
            .Times(1);

    pJniCallThread->OnConferenceInfoChanged("", "", 2, 6, "");
}

TEST_F(JniMtcCallThreadTest, OnConferenceParticipantsInfoChanged)
{
    IMS_UINT32 eType = IuMtcCall::CONF_NOTIFY_USERS_INFO;
    objParcel.writeInt32(eType);
    objParcel.setDataPosition(0);

    EXPECT_CALL(objMockThread, PostMessageI(MESSAGE_THREAD_SWITCHING, _, IsSameMessageType(eType)))
            .Times(1);

    ImsList<ConfUser*> objUsers;
    pJniCallThread->OnConferenceParticipantsInfoChanged(objUsers);
}

TEST_F(JniMtcCallThreadTest, OnEctCompleted)
{
    IMS_UINT32 eType = IuMtcCall::ECT_COMPLETED;
    objParcel.writeInt32(eType);
    objParcel.setDataPosition(0);

    EXPECT_CALL(objMockThread, PostMessageI(MESSAGE_THREAD_SWITCHING, _, IsSameMessageType(eType)))
            .Times(1);

    pJniCallThread->OnEctCompleted(IMS_SUCCESS, objReason);
}

TEST_F(JniMtcCallThreadTest, OnIncomingCallReceived)
{
    IMS_UINT32 eType = IuMtcCall::INCOMING_CALL_RECEIVED;
    objParcel.writeInt32(eType);
    objParcel.setDataPosition(0);

    EXPECT_CALL(objMockThread, PostMessageI(MESSAGE_THREAD_SWITCHING, _, IsSameMessageType(eType)))
            .Times(1);

    ImsList<ConfUser*> objUsers;
    pJniCallThread->OnIncomingCallReceived(
            1, objCallInfo, objMediaInfo, objSuppServices, OipType::NONE, "");
}

TEST_F(JniMtcCallThreadTest, OnInformationNotificationReceived)
{
    IMS_UINT32 eType = IuMtcCall::NOTIFY_INFO;
    objParcel.writeInt32(eType);
    objParcel.setDataPosition(0);

    EXPECT_CALL(objMockThread, PostMessageI(MESSAGE_THREAD_SWITCHING, _, IsSameMessageType(eType)))
            .Times(1);

    ImsList<ConfUser*> objUsers;
    pJniCallThread->OnInformationNotificationReceived(0, "", 0, 0);
}

}  // namespace android
