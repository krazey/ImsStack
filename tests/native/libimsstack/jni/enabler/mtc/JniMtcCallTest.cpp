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
#include "BaseThread.h"
#include "EnablerUtils.h"
#include "ImsProcess.h"
#include "IuMtcCall.h"
#include "JniEnablerConnector.h"
#include "JniMtcCall.h"
#include "MockIMtcCallController.h"
#include "MockIThread.h"
#include "PlatformContext.h"
#include "TestThreadService.h"
#include "call/IMtcCall.h"
#include <binder/Parcel.h>
#include <gtest/gtest.h>

using ::testing::_;
using ::testing::Return;

namespace android
{

MATCHER_P(IsSameMessageType, type, "")
{
    android::Parcel* pParcel = reinterpret_cast<android::Parcel*>(arg);
    IMS_UINT32 eType = pParcel->readInt32();
    return type == eType;
}

MATCHER_P(IsSameImsMessage, type, "")
{
    return type == arg.nMSG;
}

LOCAL const IMS_SINT32 SLOT_ID = 0;

class TestJniMtcCall : public JniMtcCall
{
public:
    inline explicit TestJniMtcCall(
            IN Jni_SendDataToJava pfnSendDataToJava, IN IMS_SINT32 nSlotId = 0) :
            JniMtcCall(pfnSendDataToJava, nSlotId)
    {
    }

    inline virtual IMS_BOOL IsThreadSwitchingRequired(IN IMS_SINT32 /*nMsg*/) const override
    {
        return IMS_FALSE;
    }
};

class JniMtcCallTest : public ::testing::Test
{
public:
    MockIMtcCallController objMockController;
    MockIThread objMockThread;
    TestThreadService* pThreadService;
    Parcel objParcel;
    JniMtcCall* pJniCall;

protected:
    virtual void SetUp() override
    {
        ON_CALL(objMockController, NotifyJniEnablerSet).WillByDefault(Return());
        JniEnablerConnector::GetInstance().SetNativeEnabler(
                SLOT_ID, EnablerType::MTC_CALL, &objMockController);

        pThreadService = new TestThreadService();
        PlatformContext::GetInstance()->SetService(PlatformContext::SERVICE_THREAD, pThreadService);
        pThreadService->SetThread(&objMockThread);

        // EnablerThread
        auto fnEntry = []() -> BaseThread*
        {
            return new BaseThread();
        };
        ImsProcess::GetInstance()->LoadThread(
                EnablerUtils::GetEnablerThreadName(SLOT_ID), fnEntry, 0);

        pJniCall = new TestJniMtcCall(reinterpret_cast<Jni_SendDataToJava>(0x01), SLOT_ID);
    }

    virtual void TearDown() override
    {
        delete pJniCall;
        JniEnablerConnector::GetInstance().SetNativeEnabler(
                SLOT_ID, EnablerType::MTC_CALL, IMS_NULL);

        ImsProcess::GetInstance()->UnloadAppThread(EnablerUtils::GetEnablerThreadName(SLOT_ID));
        PlatformContext::GetInstance()->SetService(PlatformContext::SERVICE_THREAD, IMS_NULL);
        delete pThreadService;
    }
};

TEST_F(JniMtcCallTest, CreatesJniMtcCallThread)
{
    EXPECT_NE(nullptr, pJniCall->GetJniThread());
}

TEST_F(JniMtcCallTest, DestructorInvokesDetach)
{
    EXPECT_CALL(objMockController, Detach(IMtcCall::CALL_KEY_INVALID));
}

TEST_F(JniMtcCallTest, DestructorDoesNotInvokeDetachIfNativeEnablerIsNull)
{
    JniEnablerConnector::GetInstance().SetNativeEnabler(SLOT_ID, EnablerType::MTC_CALL, IMS_NULL);
    EXPECT_CALL(objMockController, Detach(_)).Times(0);
}

TEST_F(JniMtcCallTest, DestroyPostsMessageDestroy)
{
    EXPECT_CALL(objMockThread, PostMessageI(IsSameImsMessage(-1)));
    pJniCall->Destroy();
}

TEST_F(JniMtcCallTest, SendDataOpenInvokesOpenAndAttach)
{
    CallKey nValidKey = 1;
    objParcel.writeInt32(IuMtcCall::OPEN);
    objParcel.setDataPosition(0);

    EXPECT_CALL(objMockController, Open(_, _)).WillOnce(Return(nValidKey));
    EXPECT_CALL(objMockController, Attach(nValidKey));

    pJniCall->SendData(objParcel);
}

TEST_F(JniMtcCallTest, SendDataOpenDoesNotInvokeAttachIfCallKeyIsInvalid)
{
    CallKey nInvalidKey = 0;
    objParcel.writeInt32(IuMtcCall::OPEN);
    objParcel.setDataPosition(0);

    EXPECT_CALL(objMockController, Open(_, _)).WillOnce(Return(nInvalidKey));
    EXPECT_CALL(objMockController, Attach(_)).Times(0);

    pJniCall->SendData(objParcel);
}

TEST_F(JniMtcCallTest, SendDataAttach)
{
    objParcel.writeInt32(IuMtcCall::ATTACH);
    objParcel.setDataPosition(0);

    EXPECT_CALL(objMockController, Attach(_)).Times(1);

    pJniCall->SendData(objParcel);
}

TEST_F(JniMtcCallTest, SendDataStart)
{
    // set m_nCallKey to 1
    CallKey nValidKey = 1;
    objParcel.writeInt32(IuMtcCall::OPEN);
    objParcel.setDataPosition(0);
    ON_CALL(objMockController, Open(_, _)).WillByDefault(Return(nValidKey));
    pJniCall->SendData(objParcel);

    objParcel.setDataPosition(0);
    objParcel.writeInt32(IuMtcCall::START);
    objParcel.setDataPosition(0);

    EXPECT_CALL(objMockController, Start(_, _, _, _, _)).Times(1);

    pJniCall->SendData(objParcel);
}

TEST_F(JniMtcCallTest, SendDataStartDoesNotInvokeStartIfCallKeyIsInvalid)
{
    // set m_nCallKey to 0
    CallKey nInvalidKey = 0;
    objParcel.writeInt32(IuMtcCall::OPEN);
    objParcel.setDataPosition(0);
    ON_CALL(objMockController, Open(_, _)).WillByDefault(Return(nInvalidKey));

    pJniCall->SendData(objParcel);

    // BaseServiceThread::MESSAGE_THREAD_SWITCHING = 0
    EXPECT_CALL(objMockThread, PostMessageI(0, _, IsSameMessageType(IuMtcCall::START_FAILED)))
            .Times(1);

    objParcel.setDataPosition(0);
    objParcel.writeInt32(IuMtcCall::START);
    objParcel.setDataPosition(0);

    EXPECT_CALL(objMockController, Start(_, _, _, _, _)).Times(0);

    pJniCall->SendData(objParcel);
}

TEST_F(JniMtcCallTest, SendDataUserAlert)
{
    objParcel.writeInt32(IuMtcCall::USER_ALERT);
    objParcel.setDataPosition(0);

    EXPECT_CALL(objMockController, HandleUserAlert(_)).Times(1);

    pJniCall->SendData(objParcel);
}

TEST_F(JniMtcCallTest, SendDataAccept)
{
    objParcel.writeInt32(IuMtcCall::ACCEPT);
    objParcel.setDataPosition(0);

    EXPECT_CALL(objMockController, Accept(_, _, _)).Times(1);

    pJniCall->SendData(objParcel);
}

TEST_F(JniMtcCallTest, SendDataReject)
{
    objParcel.writeInt32(IuMtcCall::REJECT);
    objParcel.setDataPosition(0);

    EXPECT_CALL(objMockController, Reject(_, _)).Times(1);

    pJniCall->SendData(objParcel);
}

TEST_F(JniMtcCallTest, SendDataHold)
{
    objParcel.writeInt32(IuMtcCall::HOLD);
    objParcel.setDataPosition(0);

    EXPECT_CALL(objMockController, Hold(_, _)).Times(1);

    pJniCall->SendData(objParcel);
}

TEST_F(JniMtcCallTest, SendDataResume)
{
    objParcel.writeInt32(IuMtcCall::RESUME);
    objParcel.setDataPosition(0);

    EXPECT_CALL(objMockController, Resume(_, _)).Times(1);

    pJniCall->SendData(objParcel);
}

TEST_F(JniMtcCallTest, SendDataTerminate)
{
    objParcel.writeInt32(IuMtcCall::TERMINATE);
    objParcel.setDataPosition(0);

    EXPECT_CALL(objMockController, Terminate(_, _)).Times(1);

    pJniCall->SendData(objParcel);
}

TEST_F(JniMtcCallTest, SendDataUpdate)
{
    objParcel.writeInt32(IuMtcCall::UPDATE);
    objParcel.setDataPosition(0);

    EXPECT_CALL(objMockController, Update(_, _, _)).Times(1);

    pJniCall->SendData(objParcel);
}

TEST_F(JniMtcCallTest, SendDataAcceptUpdate)
{
    objParcel.writeInt32(IuMtcCall::ACCEPT_UPDATE);
    objParcel.setDataPosition(0);

    EXPECT_CALL(objMockController, AcceptUpdate(_, _, _)).Times(1);

    pJniCall->SendData(objParcel);
}

TEST_F(JniMtcCallTest, SendDataRejectUpdate)
{
    objParcel.writeInt32(IuMtcCall::REJECT_UPDATE);
    objParcel.setDataPosition(0);

    EXPECT_CALL(objMockController, RejectUpdate(_, _)).Times(1);

    pJniCall->SendData(objParcel);
}

TEST_F(JniMtcCallTest, SendDataCancelUpdate)
{
    objParcel.writeInt32(IuMtcCall::CANCEL_UPDATE);
    objParcel.setDataPosition(0);

    EXPECT_CALL(objMockController, CancelUpdate(_, _)).Times(1);

    pJniCall->SendData(objParcel);
}

TEST_F(JniMtcCallTest, SendDataAcceptResume)
{
    objParcel.writeInt32(IuMtcCall::ACCEPT_RESUME);
    objParcel.setDataPosition(0);

    EXPECT_CALL(objMockController, AcceptResume(_, _, _)).Times(1);

    pJniCall->SendData(objParcel);
}

TEST_F(JniMtcCallTest, SendDataRejectResume)
{
    objParcel.writeInt32(IuMtcCall::REJECT_RESUME);
    objParcel.setDataPosition(0);

    EXPECT_CALL(objMockController, RejectResume(_, _)).Times(1);

    pJniCall->SendData(objParcel);
}

// TODO: deprecated
TEST_F(JniMtcCallTest, SendDataSendUssd)
{
    objParcel.writeInt32(IuMtcCall::SEND_USSD);
    objParcel.setDataPosition(0);

    EXPECT_CALL(objMockController, SendUssd(_, _)).Times(1);

    pJniCall->SendData(objParcel);
}

TEST_F(JniMtcCallTest, SendDataStartConf)
{
    objParcel.writeInt32(IuMtcCall::STARTCONF);
    objParcel.setDataPosition(0);
    // TODO: implement logic
    /*
        EXPECT_CALL(objMockController, (_))
                .Times(1);
    */
    pJniCall->SendData(objParcel);
}

TEST_F(JniMtcCallTest, SendDataConfMerge)
{
    objParcel.writeInt32(IuMtcCall::CONF_MERGE);
    objParcel.setDataPosition(0);

    EXPECT_CALL(objMockController, MergeToConference(_, _)).Times(1);

    pJniCall->SendData(objParcel);
}

TEST_F(JniMtcCallTest, SendDataConfExpand)
{
    objParcel.writeInt32(IuMtcCall::CONF_EXPAND);
    objParcel.setDataPosition(0);
    // TODO: implement logic
    /*
        EXPECT_CALL(objMockController, (_))
                .Times(1);
    */
    pJniCall->SendData(objParcel);
}

TEST_F(JniMtcCallTest, SendDataConfJoin)
{
    objParcel.writeInt32(IuMtcCall::CONF_JOIN);
    objParcel.setDataPosition(0);

    EXPECT_CALL(objMockController, AddToConference(_, _)).Times(1);

    pJniCall->SendData(objParcel);
}

TEST_F(JniMtcCallTest, SendDataConfDrop)
{
    objParcel.writeInt32(IuMtcCall::CONF_DROP);
    objParcel.setDataPosition(0);

    EXPECT_CALL(objMockController, RemoveFromConference(_, _)).Times(1);

    pJniCall->SendData(objParcel);
}

TEST_F(JniMtcCallTest, SendDataEctStart)
{
    objParcel.writeInt32(IuMtcCall::ECT_START);
    objParcel.setDataPosition(0);

    EXPECT_CALL(objMockController, Transfer(_, _)).Times(1);

    pJniCall->SendData(objParcel);
}

TEST_F(JniMtcCallTest, SendDataEctStartBlind)
{
    objParcel.writeInt32(IuMtcCall::ECT_START_BLIND);
    objParcel.setDataPosition(0);

    EXPECT_CALL(objMockController, Transfer(_, _)).Times(1);

    pJniCall->SendData(objParcel);
}

TEST_F(JniMtcCallTest, SendDataOpenDoesNotInvokeOpenIfNativeEnablerIsNull)
{
    JniEnablerConnector::GetInstance().SetNativeEnabler(SLOT_ID, EnablerType::MTC_CALL, IMS_NULL);

    EXPECT_CALL(objMockController, Open(_, _)).Times(0);
    EXPECT_CALL(objMockController, Attach(_)).Times(0);

    pJniCall->SendData(objParcel);
}

}  // namespace android
