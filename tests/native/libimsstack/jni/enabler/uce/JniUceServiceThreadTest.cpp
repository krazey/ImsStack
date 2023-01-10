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
#include "AString.h"
#include "IUceJni.h"
#include "IUce.h"
#include "ImsProcess.h"
#include "ImsTypeDef.h"
#include "JniUceServiceThread.h"
#include "MockIThread.h"
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

MATCHER_P2(IsSameMessageTypeAndData, type, data, "")
{
    android::Parcel* pParcel = reinterpret_cast<android::Parcel*>(arg);
    IMS_UINT32 eType = pParcel->readInt32();
    IMS_UINT32 data1 = pParcel->readInt32();
    return type == eType && data == data1;
}

MATCHER_P3(IsNotifyImsRegistered, type, service, network, "")
{
    android::Parcel* pParcel = reinterpret_cast<android::Parcel*>(arg);
    IMS_UINT32 eType = pParcel->readInt32();
    IMS_UINT32 data1 = pParcel->readInt64();
    IMS_SINT32 data2 = pParcel->readInt32();
    return type == eType && service == data1 && network == data2;
}

MATCHER_P5(IsPublishResponseInd, type, key, capability, responseCode, reasonHeaderCause, "")
{
    android::Parcel* pParcel = reinterpret_cast<android::Parcel*>(arg);

    IMS_UINT32 eType = pParcel->readInt32();
    IMS_UINT32 data1 = pParcel->readInt32();
    IMS_UINT32 data2 = pParcel->readInt64();
    IMS_UINT32 data3 = pParcel->readInt32();
    pParcel->readString16();
    IMS_UINT32 data4 = pParcel->readInt32();
    return type == eType && key == data1 && capability == data2 && responseCode == data3 &&
            reasonHeaderCause == data4;
}

MATCHER_P4(IsPublishUpdatedInd, type, capability, responseCode, reasonHeaderCause, "")
{
    android::Parcel* pParcel = reinterpret_cast<android::Parcel*>(arg);

    IMS_UINT32 eType = pParcel->readInt32();
    IMS_UINT32 data1 = pParcel->readInt64();
    IMS_UINT32 data2 = pParcel->readInt32();
    pParcel->readString16();
    IMS_UINT32 data3 = pParcel->readInt32();

    return type == eType && capability == data1 && responseCode == data2 &&
            reasonHeaderCause == data3;
}

MATCHER_P3(IsErrorInd, type, key, error, "")
{
    android::Parcel* pParcel = reinterpret_cast<android::Parcel*>(arg);
    IMS_UINT32 eType = pParcel->readInt32();
    IMS_UINT32 data1 = pParcel->readInt32();
    IMS_UINT32 data2 = pParcel->readInt32();
    return type == eType && key == data1 && error == data2;
}

MATCHER_P4(IsSubscribeResponseInd, type, key, responseCode, reasonHeaderCause, "")
{
    android::Parcel* pParcel = reinterpret_cast<android::Parcel*>(arg);

    IMS_UINT32 eType = pParcel->readInt32();
    IMS_UINT32 data1 = pParcel->readInt32();
    IMS_UINT32 data2 = pParcel->readInt32();
    pParcel->readString16();
    IMS_UINT32 data3 = pParcel->readInt32();

    return type == eType && key == data1 && responseCode == data2 && reasonHeaderCause == data3;
}

MATCHER_P3(IsSubscribeTerminatedInd, type, key, retryAfterMillsecond, "")
{
    android::Parcel* pParcel = reinterpret_cast<android::Parcel*>(arg);

    IMS_UINT32 eType = pParcel->readInt32();
    IMS_UINT32 data1 = pParcel->readInt32();
    pParcel->readString16();
    IMS_UINT32 data2 = pParcel->readInt32();
    return type == eType && key == data1 && retryAfterMillsecond == data2;
}

MATCHER_P4(IsOptionsResponseInd, type, key, responseCode, theirCaps, "")
{
    android::Parcel* pParcel = reinterpret_cast<android::Parcel*>(arg);

    IMS_UINT32 eType = pParcel->readInt32();
    IMS_UINT32 data1 = pParcel->readInt32();
    IMS_UINT32 data2 = pParcel->readInt32();
    pParcel->readString16();
    IMS_UINT32 data3 = pParcel->readInt64();
    return type == eType && key == data1 && responseCode == data2 && theirCaps == data3;
}

MATCHER_P3(IsOptionsReceivedInd, type, key, remoteCaps, "")
{
    android::Parcel* pParcel = reinterpret_cast<android::Parcel*>(arg);

    IMS_UINT32 eType = pParcel->readInt32();
    IMS_UINT32 data1 = pParcel->readInt32();
    pParcel->readString16();
    IMS_UINT32 data2 = pParcel->readInt64();
    return type == eType && key == data1 && remoteCaps == data2;
}

class JniUceServiceThreadTest : public ::testing::Test
{
public:
    inline JniUceServiceThreadTest() :
            pJniServiceThread(IMS_NULL),
            pThreadService(new TestThreadService())
    {
        PlatformContext::GetInstance()->SetService(PlatformContext::SERVICE_THREAD, pThreadService);
        pThreadService->SetThread(&objMockThread);
        CreateJniUcecServiceThread();
    }
    inline virtual ~JniUceServiceThreadTest()
    {
        delete pJniServiceThread;
        PlatformContext::GetInstance()->SetService(PlatformContext::SERVICE_THREAD, IMS_NULL);
        delete pThreadService;
    }

public:
    Parcel objParcel;
    JniUceServiceThread* pJniServiceThread;
    TestThreadService* pThreadService;
    MockIThread objMockThread;

protected:
    virtual void SetUp() override {}

    virtual void TearDown() override {}

    void CreateJniUcecServiceThread()
    {
        pJniServiceThread = new JniUceServiceThread();
        pJniServiceThread->Start("", IMS_SLOT_0);
        Jni_SendDataToJava pfnSendDataToJava = reinterpret_cast<Jni_SendDataToJava>(0x01);
        pJniServiceThread->SetCallback(0x02, pfnSendDataToJava);
    }
};

TEST_F(JniUceServiceThreadTest, NotifyImsDeregistered)
{
    IMS_UINT32 eType = IUUceService::UCE_IMS_AGENT_DISCONNECTED_IND;
    objParcel.writeInt32(eType);
    objParcel.setDataPosition(0);

    EXPECT_CALL(objMockThread, PostMessageI(MESSAGE_THREAD_SWITCHING, _, IsSameMessageType(eType)))
            .Times(1);

    pJniServiceThread->NotifyImsDeregistered();
}

TEST_F(JniUceServiceThreadTest, NotifyImsRegistered)
{
    IMS_UINT32 eType = IUUceService::UCE_IMS_AGENT_CONNECTED_IND;
    IMS_UINT32 registeredService = 10;
    IMS_SINT32 registeredNetwork = 10;
    objParcel.writeInt32(eType);
    objParcel.writeInt64(registeredNetwork);
    objParcel.writeInt32(registeredNetwork);
    objParcel.setDataPosition(0);

    EXPECT_CALL(objMockThread,
            PostMessageI(MESSAGE_THREAD_SWITCHING, _,
                    IsNotifyImsRegistered(eType, registeredService, registeredNetwork)))
            .Times(1);

    pJniServiceThread->NotifyImsRegistered(registeredService, registeredNetwork);
}

TEST_F(JniUceServiceThreadTest, PublishResponseInd)
{
    IMS_UINT32 eType = IUUceService::UCE_PUBLISH_RESPONSE_IND;
    IMS_UINT32 key = 10;
    IMS_UINT32 responseCode = 200;
    IMS_UINT32 capability = 10;
    AString reason = AString("reason");
    IMS_UINT32 reasonHeaderCause = 3;
    AString reasonHeaderText = AString("text");
    AString etag = AString("etag");
    IMS_UINT32 needToRetry = 0;

    objParcel.writeInt32(eType);
    objParcel.writeInt32(key);
    objParcel.writeInt64(capability);
    objParcel.writeInt32(responseCode);
    objParcel.writeString16(android::String16(reason.GetStr()));
    objParcel.writeInt32(reasonHeaderCause);
    objParcel.writeString16(android::String16(reasonHeaderText.GetStr()));
    objParcel.writeString16(android::String16(etag.GetStr()));
    objParcel.writeInt32(needToRetry);
    objParcel.setDataPosition(0);

    EXPECT_CALL(objMockThread,
            PostMessageI(MESSAGE_THREAD_SWITCHING, _,
                    IsPublishResponseInd(eType, key, capability, responseCode, reasonHeaderCause)))
            .Times(1);

    pJniServiceThread->PublishResponseInd(key, responseCode, capability, reason, reasonHeaderCause,
            reasonHeaderText, etag, needToRetry);
}

TEST_F(JniUceServiceThreadTest, PublishUpdatedInd)
{
    IMS_UINT32 eType = IUUceService::UCE_PUBLISH_UPDATED_IND;
    IMS_UINT32 capability = 10;
    IMS_SINT32 responseCode = 200;
    AString reason = AString("reason");
    IMS_SINT32 reasonHeaderCause = 3;
    AString reasonHeaderText = AString("text");
    AString etag = AString("etag");
    IMS_UINT32 needToRetry = 0;

    objParcel.writeInt32(eType);
    objParcel.writeInt64(capability);
    objParcel.writeInt32(responseCode);
    objParcel.writeString16(android::String16(reason.GetStr()));
    objParcel.writeInt32(reasonHeaderCause);
    objParcel.writeString16(android::String16(reasonHeaderText.GetStr()));
    objParcel.writeString16(android::String16(etag.GetStr()));
    objParcel.writeInt32(needToRetry);
    objParcel.setDataPosition(0);

    EXPECT_CALL(objMockThread,
            PostMessageI(MESSAGE_THREAD_SWITCHING, _,
                    IsPublishUpdatedInd(eType, capability, responseCode, reasonHeaderCause)))
            .Times(1);

    pJniServiceThread->PublishUpdatedInd(capability, responseCode, reason, reasonHeaderCause,
            reasonHeaderText, etag, needToRetry);
}

TEST_F(JniUceServiceThreadTest, PublishErrorInd)
{
    IMS_UINT32 eType = IUUceService::UCE_PUBLISH_CMD_ERROR_IND;
    IMS_UINT32 key = 10;
    IMS_UINT32 commandError = 10;

    objParcel.writeInt32(eType);
    objParcel.writeInt32(key);
    objParcel.writeInt32(commandError);
    objParcel.setDataPosition(0);

    EXPECT_CALL(objMockThread,
            PostMessageI(MESSAGE_THREAD_SWITCHING, _, IsErrorInd(eType, key, commandError)))
            .Times(1);

    pJniServiceThread->PublishErrorInd(key, commandError);
}

TEST_F(JniUceServiceThreadTest, UnPublishedInd)
{
    IMS_UINT32 eType = IUUceService::UCE_UNPUBLISHED_IND;

    objParcel.writeInt32(eType);
    objParcel.setDataPosition(0);

    EXPECT_CALL(objMockThread, PostMessageI(MESSAGE_THREAD_SWITCHING, _, IsSameMessageType(eType)))
            .Times(1);

    pJniServiceThread->UnPublishedInd();
}

TEST_F(JniUceServiceThreadTest, SubscribeResponseInd)
{
    IMS_UINT32 eType = IUUceService::UCE_SUBSCRIBE_RESPONSE_IND;
    IMS_UINT32 key = 10;
    IMS_SINT32 responseCode = 200;
    AString reason = AString("reason");
    IMS_SINT32 reasonHeaderCause = 3;
    AString reasonHeaderText = AString("text");

    objParcel.writeInt32(eType);
    objParcel.writeInt32(key);
    objParcel.writeInt32(responseCode);
    objParcel.writeString16(android::String16(reason.GetStr()));
    objParcel.writeInt32(reasonHeaderCause);
    objParcel.writeString16(android::String16(reasonHeaderText.GetStr()));
    objParcel.setDataPosition(0);

    EXPECT_CALL(objMockThread,
            PostMessageI(MESSAGE_THREAD_SWITCHING, _,
                    IsSubscribeResponseInd(eType, key, responseCode, reasonHeaderCause)))
            .Times(1);

    pJniServiceThread->SubscribeResponseInd(
            key, responseCode, reason, reasonHeaderCause, reasonHeaderText);
}

TEST_F(JniUceServiceThreadTest, NotifyInd)
{
    IMS_UINT32 eType = IUUceService::UCE_PRESENCE_NOTIFY_IND;
    IMS_UINT32 key = 10;
    IMS_UINT32 count = 0;
    IMSList<AString> pidfXmls;

    objParcel.writeInt32(eType);
    objParcel.writeInt32(key);
    objParcel.writeInt32(count);
    objParcel.setDataPosition(0);

    EXPECT_CALL(objMockThread,
            PostMessageI(MESSAGE_THREAD_SWITCHING, _, IsSameMessageTypeAndData(eType, key)))
            .Times(1);

    pJniServiceThread->NotifyInd(key, count, pidfXmls);
}

TEST_F(JniUceServiceThreadTest, SubscribeErrorInd)
{
    IMS_UINT32 eType = IUUceService::UCE_SUBSCRIBE_CMD_ERROR_IND;
    IMS_UINT32 key = 10;
    IMS_UINT32 commandError = 10;

    objParcel.writeInt32(eType);
    objParcel.writeInt32(key);
    objParcel.writeInt32(commandError);
    objParcel.setDataPosition(0);

    EXPECT_CALL(objMockThread,
            PostMessageI(MESSAGE_THREAD_SWITCHING, _, IsErrorInd(eType, key, commandError)))
            .Times(1);

    pJniServiceThread->SubscribeErrorInd(key, commandError);
}

TEST_F(JniUceServiceThreadTest, SubscribeResourceTerminatedInd)
{
    IMS_UINT32 eType = IUUceService::UCE_SUBSCRIBE_RESOURCE_TERMINATED_IND;
    IMS_UINT32 key = 10;
    IMS_UINT32 count = 0;
    IMSList<IUceTerminatedReason*> terminateContacts;

    objParcel.writeInt32(eType);
    objParcel.writeInt32(key);
    objParcel.writeInt32(count);
    objParcel.setDataPosition(0);

    EXPECT_CALL(objMockThread,
            PostMessageI(MESSAGE_THREAD_SWITCHING, _, IsSameMessageTypeAndData(eType, key)))
            .Times(1);

    pJniServiceThread->SubscribeResourceTerminatedInd(key, count, terminateContacts);
}

TEST_F(JniUceServiceThreadTest, SubscribeTerminatedInd)
{
    IMS_UINT32 eType = IUUceService::UCE_SUBSCRIBE_TERMINATED_IND;
    IMS_UINT32 key = 10;
    AString reason = AString("reason");
    IMS_UINT32 retryAfterMillsecond = 10;

    objParcel.writeInt32(eType);
    objParcel.writeInt32(key);
    objParcel.writeString16(android::String16(reason.GetStr()));
    objParcel.writeInt32(retryAfterMillsecond);
    objParcel.setDataPosition(0);

    EXPECT_CALL(objMockThread,
            PostMessageI(MESSAGE_THREAD_SWITCHING, _,
                    IsSubscribeTerminatedInd(eType, key, retryAfterMillsecond)))
            .Times(1);

    pJniServiceThread->SubscribeTerminatedInd(key, reason, retryAfterMillsecond);
}

TEST_F(JniUceServiceThreadTest, OptionsResponseInd)
{
    IMS_UINT32 eType = IUUceService::UCE_OPTIONS_RESPONSE_IND;
    IMS_UINT32 key = 10;
    IMS_UINT32 responseCode = 200;
    AString reason = AString("reason");
    IMS_UINT32 theirCaps = 10;

    objParcel.writeInt32(eType);
    objParcel.writeInt32(key);
    objParcel.writeInt32(responseCode);
    objParcel.writeString16(android::String16(reason.GetStr()));
    objParcel.writeInt64(theirCaps);
    objParcel.setDataPosition(0);

    EXPECT_CALL(objMockThread,
            PostMessageI(MESSAGE_THREAD_SWITCHING, _,
                    IsOptionsResponseInd(eType, key, responseCode, theirCaps)))
            .Times(1);

    pJniServiceThread->OptionsResponseInd(key, responseCode, reason, theirCaps);
}

TEST_F(JniUceServiceThreadTest, OptionsErrorInd)
{
    IMS_UINT32 eType = IUUceService::UCE_OPTIONS_CMD_ERROR_IND;
    IMS_UINT32 key = 10;
    IMS_UINT32 commandError = 10;

    objParcel.writeInt32(eType);
    objParcel.writeInt32(key);
    objParcel.writeInt32(commandError);
    objParcel.setDataPosition(0);

    EXPECT_CALL(objMockThread,
            PostMessageI(MESSAGE_THREAD_SWITCHING, _, IsErrorInd(eType, key, commandError)))
            .Times(1);

    pJniServiceThread->OptionsErrorInd(key, commandError);
}

TEST_F(JniUceServiceThreadTest, OptionsReceivedInd)
{
    IMS_UINT32 eType = IUUceService::UCE_OPTIONS_RECEIVED_IND;
    IMS_UINT32 key = 10;
    AString remote = AString("remote");
    IMS_UINT32 remoteCaps = 10;

    objParcel.writeInt32(eType);
    objParcel.writeInt32(key);
    objParcel.writeString16(android::String16(remote.GetStr()));
    objParcel.writeInt64(remoteCaps);
    objParcel.setDataPosition(0);

    EXPECT_CALL(objMockThread,
            PostMessageI(MESSAGE_THREAD_SWITCHING, _, IsOptionsReceivedInd(eType, key, remoteCaps)))
            .Times(1);

    pJniServiceThread->OptionsReceivedInd(key, remote, remoteCaps);
}

TEST_F(JniUceServiceThreadTest, NotifyImsRegiRefreshed)
{
    IMS_UINT32 eType = IUUceService::UCE_IMS_AGENT_REFRESHED_IND;
    IMS_SINT32 registeredNetwork = 10;

    objParcel.writeInt32(eType);
    objParcel.writeInt32(registeredNetwork);

    objParcel.setDataPosition(0);

    EXPECT_CALL(objMockThread,
            PostMessageI(MESSAGE_THREAD_SWITCHING, _,
                    IsSameMessageTypeAndData(eType, registeredNetwork)))
            .Times(1);

    pJniServiceThread->NotifyImsRegiRefreshed(registeredNetwork);
}

TEST_F(JniUceServiceThreadTest, NotifyNetworkChanged)
{
    IMS_UINT32 eType = IUUceService::UCE_NETWORK_CHANGED;
    IMS_SINT32 changedNetwork = 10;

    objParcel.writeInt32(eType);
    objParcel.writeInt32(changedNetwork);
    objParcel.setDataPosition(0);

    EXPECT_CALL(objMockThread,
            PostMessageI(
                    MESSAGE_THREAD_SWITCHING, _, IsSameMessageTypeAndData(eType, changedNetwork)))
            .Times(1);

    pJniServiceThread->NotifyNetworkChanged(changedNetwork);
}

}  // namespace android
