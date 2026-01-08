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
#include "CallReasonInfo.h"
#include "ImsList.h"
#include "ImsTypeDef.h"
#include "JniCallInfo.h"
#include "JniMtcUtils.h"
#include "MtcDef.h"
#include "call/IMtcCall.h"
#include "conferencecall/ConferenceDef.h"
#include <binder/Parcel.h>
#include <gtest/gtest.h>

namespace android
{

class JniMtcUtilsTest : public ::testing::Test
{
protected:
    virtual void SetUp() override {}

    virtual void TearDown() override {}
};

TEST_F(JniMtcUtilsTest, ConvertString)
{
    AString strAnyInputString("Any Input String");
    Parcel objParcel;
    objParcel.writeString16(android::String16(strAnyInputString.GetStr()));
    objParcel.setDataPosition(0);

    AString strOutput;
    JniMtcUtils::ConvertString(objParcel.readString16(), strOutput);

    EXPECT_STREQ(strAnyInputString.GetStr(), strOutput.GetStr());
}

TEST_F(JniMtcUtilsTest, ReadCallType)
{
    CallType eAnyType = CallType::VOIP;
    Parcel objParcel;
    objParcel.writeInt32(static_cast<IMS_SINT32>(eAnyType));
    objParcel.setDataPosition(0);

    EXPECT_EQ(eAnyType, JniMtcUtils::ReadCallType(objParcel));
}

TEST_F(JniMtcUtilsTest, ReadServiceType)
{
    ServiceType eAnyType = ServiceType::NORMAL;
    Parcel objParcel;
    objParcel.writeInt32(static_cast<IMS_SINT32>(eAnyType));
    objParcel.setDataPosition(0);

    EXPECT_EQ(eAnyType, JniMtcUtils::ReadServiceType(objParcel));
}

TEST_F(JniMtcUtilsTest, WtiteAndReadCallInfo)
{
    // Not all the params are compared.
    CallType eAnyType = CallType::VOIP;
    JniCallInfo objAnyJniCallInfo;
    objAnyJniCallInfo.eCallType = eAnyType;
    objAnyJniCallInfo.eEmergencyType = EmergencyType::EMERGENCY_ROUTING;
    objAnyJniCallInfo.bOffline = IMS_TRUE;
    objAnyJniCallInfo.bUssi = IMS_TRUE;

    Parcel objParcel;
    JniMtcUtils::WriteCallInfoToParcel(objAnyJniCallInfo, objParcel);
    objParcel.setDataPosition(0);

    JniCallInfo objConvertedJniCallInfo = JniMtcUtils::ReadCallInfo(objParcel);
    EXPECT_EQ(objConvertedJniCallInfo, objAnyJniCallInfo);
}

TEST_F(JniMtcUtilsTest, WtiteAndReadMediaInfo)
{
    MediaInfo objAnyMediaInfo;
    objAnyMediaInfo.eAudioQuality = AUDIO_QUALITY_EVS;
    objAnyMediaInfo.eVideoQuality = VIDEO_QUALITY_VGA_PR;
    objAnyMediaInfo.eAudioDirection = DIRECTION_SEND_RECEIVE;
    objAnyMediaInfo.eVideoDirection = DIRECTION_SEND_RECEIVE;
    objAnyMediaInfo.eTextDirection = DIRECTION_SEND_RECEIVE;
    objAnyMediaInfo.eGttMode = GTT_MODE_FULL;

    Parcel objParcel;
    JniMtcUtils::WriteMediaInfoToParcel(objAnyMediaInfo, objParcel);
    objParcel.setDataPosition(0);

    MediaInfo objConvertedMediaInfo = JniMtcUtils::ReadMediaInfo(objParcel, objConvertedMediaInfo);
    EXPECT_EQ(objConvertedMediaInfo, objAnyMediaInfo);
}

TEST_F(JniMtcUtilsTest, WriteAudioCodecAttributesToParcel)
{
    const float nBitrateKbps = 64.0f;
    const float nBitrateStartKbps = 32.0f;
    const float nBitrateEndKbps = 128.0f;
    const float nBandwidthKhz = 16.0f;
    const float nBandwidthStartKhz = 8.0f;
    const float nBandwidthEndKhz = 24.0f;

    AudioCodecAttributes objAudioCodecAttributes(nBitrateKbps, nBitrateStartKbps, nBitrateEndKbps,
            nBandwidthKhz, nBandwidthStartKhz, nBandwidthEndKhz);

    Parcel parcel;
    JniMtcUtils::WriteAudioCodecAttributesToParcel(objAudioCodecAttributes, parcel);

    parcel.setDataPosition(0);
    EXPECT_EQ(nBitrateKbps, parcel.readFloat());
    EXPECT_EQ(nBitrateStartKbps, parcel.readFloat());
    EXPECT_EQ(nBitrateEndKbps, parcel.readFloat());
    EXPECT_EQ(nBandwidthKhz, parcel.readFloat());
    EXPECT_EQ(nBandwidthStartKhz, parcel.readFloat());
    EXPECT_EQ(nBandwidthEndKhz, parcel.readFloat());
}

TEST_F(JniMtcUtilsTest, WtiteAndReadSuppServices)
{
    ImsList<SuppService*> objSuppServices;
    SuppService objAnySuppService1;
    objAnySuppService1.nType = static_cast<IMS_SINT32>(SuppType::CNAP);
    objAnySuppService1.strValue = "anyValue1";
    objAnySuppService1.nValue = 1;
    objAnySuppService1.bValue = IMS_TRUE;

    SuppService objAnySuppService2;
    objAnySuppService2.nType = static_cast<IMS_SINT32>(SuppType::CALL_PULL);
    objAnySuppService2.strValue = "anyValue2";
    objAnySuppService2.nValue = 2;
    objAnySuppService2.bValue = IMS_FALSE;

    objSuppServices.Append(&objAnySuppService1);
    objSuppServices.Append(&objAnySuppService2);

    Parcel objParcel;
    JniMtcUtils::WriteSuppServicesToParcel(objSuppServices, objParcel);
    objParcel.setDataPosition(0);

    ImsList<SuppService*> objConvertedSuppServices =
            JniMtcUtils::ReadSupplementaryService(objParcel);
    EXPECT_EQ(objSuppServices.GetSize(), objConvertedSuppServices.GetSize());
    EXPECT_EQ(*objSuppServices.GetAt(0), *objConvertedSuppServices.GetAt(0));
    EXPECT_EQ(*objSuppServices.GetAt(1), *objConvertedSuppServices.GetAt(1));

    objSuppServices.Clear();

    delete objConvertedSuppServices.GetAt(0);
    delete objConvertedSuppServices.GetAt(1);
    objConvertedSuppServices.Clear();
}

TEST_F(JniMtcUtilsTest, WtiteAndReadCofnerenceParticipants)
{
    ImsList<ConfUser*> objUsers;
    ConfUser objAnyUser1;
    objAnyUser1.nConnectionId = 1;
    objAnyUser1.strTarget = "anyTarget1";
    objAnyUser1.strUserEntity = "anyUserEntity1";
    objAnyUser1.strEpEntity = "anyEpEntity1";
    objAnyUser1.strDisplayName = "anyDisplayName1";
    objAnyUser1.eStatus = STATUS_CONNECTED;
    objAnyUser1.eStatusCode = 200;
    objAnyUser1.eCcType = COPYCONTROLTYPE_CC;
    objAnyUser1.bAnonymize = IMS_TRUE;

    ConfUser objAnyUser2;
    objAnyUser2.nConnectionId = 2;
    objAnyUser2.strTarget = "anyTarget2";
    objAnyUser2.strUserEntity = "anyUserEntity2";
    objAnyUser2.strEpEntity = "anyEpEntity2";
    objAnyUser2.strDisplayName = "anyDisplayName2";
    objAnyUser2.eStatus = STATUS_DISCONNECTED;
    objAnyUser2.eStatusCode = 486;
    objAnyUser2.eCcType = COPYCONTROLTYPE_TO;
    objAnyUser2.bAnonymize = IMS_FALSE;

    objUsers.Append(&objAnyUser1);
    objUsers.Append(&objAnyUser2);

    Parcel objParcel;
    JniMtcUtils::WriteConfUsersToParcel(objUsers, objParcel);
    objParcel.setDataPosition(0);

    ImsList<ConfUser*> objConvertedUsers = JniMtcUtils::ReadConferenceParticipants(objParcel);
    EXPECT_EQ(objConvertedUsers.GetSize(), objUsers.GetSize());
    EXPECT_EQ(*objUsers.GetAt(0), *objConvertedUsers.GetAt(0));
    EXPECT_EQ(*objUsers.GetAt(1), *objConvertedUsers.GetAt(1));

    objUsers.Clear();

    delete objConvertedUsers.GetAt(0);
    delete objConvertedUsers.GetAt(1);
    objConvertedUsers.Clear();
}

TEST_F(JniMtcUtilsTest, WriteCallReasonInfo)
{
    AString strAnyExtra("anyString");
    const CallReasonInfo objAnyReason(CODE_USER_TERMINATED, EXTRA_USER_TERMINATED_ECT, "anyString");
    Parcel objParcel;
    JniMtcUtils::WriteCallReasonInfoToParcel(objAnyReason, objParcel);
    objParcel.setDataPosition(0);

    EXPECT_EQ(CODE_USER_TERMINATED, objParcel.readInt32());
    EXPECT_EQ(EXTRA_USER_TERMINATED_ECT, objParcel.readInt32());
    AString strOutExtra;
    JniMtcUtils::ConvertString(objParcel.readString16(), strOutExtra);
    EXPECT_STREQ(strAnyExtra.GetStr(), strOutExtra.GetStr());
}

}  // namespace android
