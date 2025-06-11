/*
 * Copyright (C) 2025 The Android Open Source Project
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

#include "media/MtcMediaStringUtils.h"

#include <gtest/gtest.h>

namespace android
{

class MtcMediaStringUtilsTest : public ::testing::Test
{
protected:
    virtual void SetUp() override {}

    virtual void TearDown() override {}
};

TEST_F(MtcMediaStringUtilsTest, ConvertReportType)
{
    IMS_UINT32 eReportType = REPORT_TYPE::REPORT_SUCCESS;
    EXPECT_EQ(MtcMediaStringUtils::ConvertReportType(eReportType), "success");

    eReportType = REPORT_TYPE::REPORT_FAILURE;
    EXPECT_EQ(MtcMediaStringUtils::ConvertReportType(eReportType), "failure");

    eReportType = REPORT_TYPE::REPORT_DATA_RECEIVE_FAILED;
    EXPECT_EQ(MtcMediaStringUtils::ConvertReportType(eReportType), "data receive failed");

    eReportType = REPORT_TYPE::REPORT_DATA_RECEIVE_STARTED;
    EXPECT_EQ(MtcMediaStringUtils::ConvertReportType(eReportType), "data receive started");

    eReportType = REPORT_TYPE::REPORT_QOS;
    EXPECT_EQ(MtcMediaStringUtils::ConvertReportType(eReportType), "qos");

    eReportType = REPORT_TYPE::REPORT_CHECK_RADIO_CONNECTION;
    EXPECT_EQ(MtcMediaStringUtils::ConvertReportType(eReportType), "check radio condition");

    eReportType = REPORT_TYPE::REPORT_NW_TONE_RTP_RECEIVE_STARTED;
    EXPECT_EQ(MtcMediaStringUtils::ConvertReportType(eReportType), "network tone receive started");

    eReportType = REPORT_TYPE::REPORT_NW_TONE_RTP_RECEIVE_FAILED;
    EXPECT_EQ(MtcMediaStringUtils::ConvertReportType(eReportType), "network tone receive failed");

    eReportType = REPORT_TYPE::REPORT_RECEIVED_DTMF_EVENT;
    EXPECT_EQ(MtcMediaStringUtils::ConvertReportType(eReportType), "dtmf event received");

    eReportType = REPORT_TYPE::REPORT_MEDIA_DETACH;
    EXPECT_EQ(MtcMediaStringUtils::ConvertReportType(eReportType), "media detach");

    eReportType = REPORT_TYPE::REPORT_TRIGGER_ANBR_QUERY;
    EXPECT_EQ(MtcMediaStringUtils::ConvertReportType(eReportType), "trigger anbr query");

    eReportType = REPORT_TYPE::REPORT_ANBR_NEGOTIATION_RESULT;
    EXPECT_EQ(MtcMediaStringUtils::ConvertReportType(eReportType), "anbr negotiation result");

    eReportType = REPORT_TYPE::REPORT_NOTUSED;
    EXPECT_EQ(MtcMediaStringUtils::ConvertReportType(eReportType), "not used");

    eReportType = REPORT_TYPE::REPORT_NOTUSED + 1;
    EXPECT_EQ(MtcMediaStringUtils::ConvertReportType(eReportType), "OUT_OF_RANGE");

    eReportType = REPORT_TYPE::REPORT_SUCCESS - 1;
    EXPECT_EQ(MtcMediaStringUtils::ConvertReportType(eReportType), "OUT_OF_RANGE");
}

TEST_F(MtcMediaStringUtilsTest, ConvertContentType)
{
    IMS_UINT32 eContentType = MEDIA_TYPE_INVALID;
    EXPECT_EQ(MtcMediaStringUtils::ConvertContentType(eContentType), "invalid");

    eContentType = MEDIA_TYPE_AUDIO;
    EXPECT_EQ(MtcMediaStringUtils::ConvertContentType(eContentType), "audio");

    eContentType = MEDIA_TYPE_VIDEO;
    EXPECT_EQ(MtcMediaStringUtils::ConvertContentType(eContentType), "video");

    eContentType = MEDIA_TYPE_AUDIOVIDEO;
    EXPECT_EQ(MtcMediaStringUtils::ConvertContentType(eContentType), "audio & video");

    eContentType = MEDIA_TYPE_TEXT;
    EXPECT_EQ(MtcMediaStringUtils::ConvertContentType(eContentType), "text");

    eContentType = MEDIA_TYPE_AUDIOTEXT;
    EXPECT_EQ(MtcMediaStringUtils::ConvertContentType(eContentType), "audio & text");

    eContentType = MEDIA_TYPE_VIDEOTEXT;
    EXPECT_EQ(MtcMediaStringUtils::ConvertContentType(eContentType), "video & text");

    eContentType = MEDIA_TYPE_AUDIOVIDEOTEXT;
    EXPECT_EQ(MtcMediaStringUtils::ConvertContentType(eContentType), "audio & video & text");

    eContentType = MEDIA_TYPE_NOTUSED;
    EXPECT_EQ(MtcMediaStringUtils::ConvertContentType(eContentType), "not used");

    eContentType = MEDIA_TYPE_INVALID - 1;
    EXPECT_EQ(MtcMediaStringUtils::ConvertContentType(eContentType), "OUT_OF_RANGE");

    eContentType = MEDIA_TYPE_NOTUSED + 1;
    EXPECT_EQ(MtcMediaStringUtils::ConvertContentType(eContentType), "OUT_OF_RANGE");
}

TEST_F(MtcMediaStringUtilsTest, ConvertErrorType)
{
    IMS_SINT32 eErrorType = RtpError::NO_ERROR;
    EXPECT_EQ(MtcMediaStringUtils::ConvertErrorType(eErrorType), "no error");

    eErrorType = RtpError::INVALID_PARAM;
    EXPECT_EQ(MtcMediaStringUtils::ConvertErrorType(eErrorType), "invalid param");

    eErrorType = RtpError::NOT_READY;
    EXPECT_EQ(MtcMediaStringUtils::ConvertErrorType(eErrorType), "not ready");

    eErrorType = RtpError::NO_MEMORY;
    EXPECT_EQ(MtcMediaStringUtils::ConvertErrorType(eErrorType), "no memory");

    eErrorType = RtpError::NO_RESOURCES;
    EXPECT_EQ(MtcMediaStringUtils::ConvertErrorType(eErrorType), "no resources");

    eErrorType = RtpError::PORT_UNAVAILABLE;
    EXPECT_EQ(MtcMediaStringUtils::ConvertErrorType(eErrorType), "port unavailable");

    eErrorType = RtpError::REQUEST_NOT_SUPPORTED;
    EXPECT_EQ(MtcMediaStringUtils::ConvertErrorType(eErrorType), "request not supported");

    eErrorType = RtpError::RESPONSE_WAIT_TIMEOUT;
    EXPECT_EQ(MtcMediaStringUtils::ConvertErrorType(eErrorType), "response wait timeout");

    eErrorType = RtpError::NO_ERROR - 1;
    EXPECT_EQ(MtcMediaStringUtils::ConvertErrorType(eErrorType), "OUT_OF_RANGE");

    eErrorType = RtpError::RESPONSE_WAIT_TIMEOUT + 1;
    EXPECT_EQ(MtcMediaStringUtils::ConvertErrorType(eErrorType), "OUT_OF_RANGE");
}

TEST_F(MtcMediaStringUtilsTest, ConvertNegoType)
{
    MediaNego::MediaNegoResult eNegoType = MediaNego::MediaNegoResult::NO_ERROR;
    EXPECT_EQ(MtcMediaStringUtils::ConvertNegoType(eNegoType), "no error");

    eNegoType = MediaNego::MediaNegoResult::ERROR_INVALID_DESCRIPTOR;
    EXPECT_EQ(MtcMediaStringUtils::ConvertNegoType(eNegoType), "invalid descriptor");

    eNegoType = MediaNego::MediaNegoResult::ERROR_NO_CODEC_MATCHED;
    EXPECT_EQ(MtcMediaStringUtils::ConvertNegoType(eNegoType), "no codec matched");

    eNegoType = MediaNego::MediaNegoResult::ERROR_IP_MISMATCH;
    EXPECT_EQ(MtcMediaStringUtils::ConvertNegoType(eNegoType), "ip mismatch");

    eNegoType = MediaNego::MediaNegoResult::ERROR_NO_AUDIO;
    EXPECT_EQ(MtcMediaStringUtils::ConvertNegoType(eNegoType), "no audio");

    eNegoType = MediaNego::MediaNegoResult::ERROR_NO_VIDEO;
    EXPECT_EQ(MtcMediaStringUtils::ConvertNegoType(eNegoType), "no video");

    eNegoType = MediaNego::MediaNegoResult::ERROR_NO_TEXT;
    EXPECT_EQ(MtcMediaStringUtils::ConvertNegoType(eNegoType), "no text");
}

TEST_F(MtcMediaStringUtilsTest, ConvertPemType)
{
    PemType ePemType = PemType::NONE;
    EXPECT_EQ(MtcMediaStringUtils::ConvertPemType(ePemType), "none");

    ePemType = PemType::SENDRECV;
    EXPECT_EQ(MtcMediaStringUtils::ConvertPemType(ePemType), "sendrecv");

    ePemType = PemType::SENDONLY;
    EXPECT_EQ(MtcMediaStringUtils::ConvertPemType(ePemType), "sendonly");

    ePemType = PemType::RECVONLY;
    EXPECT_EQ(MtcMediaStringUtils::ConvertPemType(ePemType), "recvonly");

    ePemType = PemType::INACTIVE;
    EXPECT_EQ(MtcMediaStringUtils::ConvertPemType(ePemType), "inactive");
}

TEST_F(MtcMediaStringUtilsTest, ConvertQuality)
{
    IMS_SINT32 eCodecType = MEDIA_QUALITY_NONE - 1;
    EXPECT_EQ(MtcMediaStringUtils::ConvertQuality(eCodecType), "OUT_OF_RANGE");

    eCodecType = MEDIA_QUALITY_NONE;
    EXPECT_EQ(MtcMediaStringUtils::ConvertQuality(eCodecType), "none");

    eCodecType = MEDIA_QUALITY_NONE + 1;
    EXPECT_EQ(MtcMediaStringUtils::ConvertQuality(eCodecType), "valid");

    eCodecType = MEDIA_QUALITY_NOTUSED - 1;
    EXPECT_EQ(MtcMediaStringUtils::ConvertQuality(eCodecType), "valid");

    eCodecType = MEDIA_QUALITY_NOTUSED;
    EXPECT_EQ(MtcMediaStringUtils::ConvertQuality(eCodecType), "none");

    eCodecType = MEDIA_QUALITY_NOTUSED + 1;
    EXPECT_EQ(MtcMediaStringUtils::ConvertQuality(eCodecType), "OUT_OF_RANGE");
}

TEST_F(MtcMediaStringUtilsTest, ConvertNegoState)
{
    NEGO_STATE eNegoState = STATE_IDLE;
    EXPECT_EQ(MtcMediaStringUtils::ConvertNegoState(eNegoState), "idle");

    eNegoState = STATE_OFFER_RECEIVED;
    EXPECT_EQ(MtcMediaStringUtils::ConvertNegoState(eNegoState), "offer received");

    eNegoState = STATE_OFFER_SENT;
    EXPECT_EQ(MtcMediaStringUtils::ConvertNegoState(eNegoState), "offer sent");

    eNegoState = STATE_NEGOTIATED;
    EXPECT_EQ(MtcMediaStringUtils::ConvertNegoState(eNegoState), "negotiated");

    eNegoState = STATE_NOTUSED;
    EXPECT_EQ(MtcMediaStringUtils::ConvertNegoState(eNegoState), "not used");
}

TEST_F(MtcMediaStringUtilsTest, ConvertDirection)
{
    IMS_SINT32 eDirection = MEDIA_DIRECTION_INVALID;
    EXPECT_EQ(MtcMediaStringUtils::ConvertDirection(eDirection), "invalid");

    eDirection = MEDIA_DIRECTION_INACTIVE;
    EXPECT_EQ(MtcMediaStringUtils::ConvertDirection(eDirection), "inactive");

    eDirection = MEDIA_DIRECTION_RECEIVE;
    EXPECT_EQ(MtcMediaStringUtils::ConvertDirection(eDirection), "receive");

    eDirection = MEDIA_DIRECTION_SEND;
    EXPECT_EQ(MtcMediaStringUtils::ConvertDirection(eDirection), "send");

    eDirection = MEDIA_DIRECTION_SEND_RECEIVE;
    EXPECT_EQ(MtcMediaStringUtils::ConvertDirection(eDirection), "sendrecv");

    eDirection = MEDIA_DIRECTION_INVALID - 1;
    EXPECT_EQ(MtcMediaStringUtils::ConvertDirection(eDirection), "OUT_OF_RANGE");

    eDirection = MEDIA_DIRECTION_SEND_RECEIVE + 1;
    EXPECT_EQ(MtcMediaStringUtils::ConvertDirection(eDirection), "OUT_OF_RANGE");
}

TEST_F(MtcMediaStringUtilsTest, ConvertProtocolType)
{
    IMS_SINT32 eProtocolType = MEDIA_PROTOCOL_NONE;
    EXPECT_EQ(MtcMediaStringUtils::ConvertProtocolType(eProtocolType), "none");

    eProtocolType = MEDIA_PROTOCOL_ANY;
    EXPECT_EQ(MtcMediaStringUtils::ConvertProtocolType(eProtocolType), "any");

    eProtocolType = MEDIA_PROTOCOL_RTP;
    EXPECT_EQ(MtcMediaStringUtils::ConvertProtocolType(eProtocolType), "rtp");

    eProtocolType = MEDIA_PROTOCOL_RTCP;
    EXPECT_EQ(MtcMediaStringUtils::ConvertProtocolType(eProtocolType), "rtcp");

    eProtocolType = MEDIA_PROTOCOL_NO_CHANGE;
    EXPECT_EQ(MtcMediaStringUtils::ConvertProtocolType(eProtocolType), "no change");

    eProtocolType = MEDIA_PROTOCOL_NONE - 1;
    EXPECT_EQ(MtcMediaStringUtils::ConvertProtocolType(eProtocolType), "OUT_OF_RANGE");

    eProtocolType = MEDIA_PROTOCOL_NO_CHANGE + 1;
    EXPECT_EQ(MtcMediaStringUtils::ConvertProtocolType(eProtocolType), "OUT_OF_RANGE");
}

}  // namespace android
