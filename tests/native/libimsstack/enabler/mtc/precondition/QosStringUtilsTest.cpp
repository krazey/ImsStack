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

#include "precondition/QosStringUtils.h"
#include <gtest/gtest.h>

class QosStringUtilsTest : public ::testing::Test
{
protected:
    virtual void SetUp() override {}

    virtual void TearDown() override {}
};

TEST_F(QosStringUtilsTest, ConvertSdpMediaType)
{
    IMS_SINT32 eSdpMediaType = SdpMedia::TYPE_INVALID;
    EXPECT_EQ(QosStringUtils::ConvertSdpMediaType(eSdpMediaType), "invalid");

    eSdpMediaType = SdpMedia::TYPE_AUDIO;
    EXPECT_EQ(QosStringUtils::ConvertSdpMediaType(eSdpMediaType), "audio");

    eSdpMediaType = SdpMedia::TYPE_VIDEO;
    EXPECT_EQ(QosStringUtils::ConvertSdpMediaType(eSdpMediaType), "video");

    eSdpMediaType = SdpMedia::TYPE_TEXT;
    EXPECT_EQ(QosStringUtils::ConvertSdpMediaType(eSdpMediaType), "text");

    eSdpMediaType = SdpMedia::TYPE_APPLICATION;
    EXPECT_EQ(QosStringUtils::ConvertSdpMediaType(eSdpMediaType), "application");

    eSdpMediaType = SdpMedia::TYPE_MESSAGE;
    EXPECT_EQ(QosStringUtils::ConvertSdpMediaType(eSdpMediaType), "message");

    eSdpMediaType = SdpMedia::TYPE_OTHER;
    EXPECT_EQ(QosStringUtils::ConvertSdpMediaType(eSdpMediaType), "other");

    eSdpMediaType = SdpMedia::TYPE_MAX;
    EXPECT_EQ(QosStringUtils::ConvertSdpMediaType(eSdpMediaType), "invalid max");

    eSdpMediaType = SdpMedia::TYPE_INVALID - 1;
    EXPECT_EQ(QosStringUtils::ConvertSdpMediaType(eSdpMediaType), "OUT_OF_RANGE");

    eSdpMediaType = SdpMedia::TYPE_MAX + 1;
    EXPECT_EQ(QosStringUtils::ConvertSdpMediaType(eSdpMediaType), "OUT_OF_RANGE");
}

TEST_F(QosStringUtilsTest, ConvertQosAttribute)
{
    IMS_SINT32 eAttrType = SdpAttribute::CURR;
    EXPECT_EQ(QosStringUtils::ConvertQosAttribute(eAttrType), "curr");

    eAttrType = SdpAttribute::DES;
    EXPECT_EQ(QosStringUtils::ConvertQosAttribute(eAttrType), "des");

    eAttrType = SdpAttribute::CONF;
    EXPECT_EQ(QosStringUtils::ConvertQosAttribute(eAttrType), "conf");

    eAttrType = SdpAttribute::CURR - 1;
    EXPECT_EQ(QosStringUtils::ConvertQosAttribute(eAttrType), "invalid");

    eAttrType = SdpAttribute::CONF + 1;
    EXPECT_EQ(QosStringUtils::ConvertQosAttribute(eAttrType), "invalid");
}

TEST_F(QosStringUtilsTest, ConvertQosDir)
{
    IMS_SINT32 eDirTag = SdpPrecondition::DIRECTION_INVALID;
    EXPECT_EQ(QosStringUtils::ConvertQosDir(eDirTag), "invalid");

    eDirTag = SdpPrecondition::DIRECTION_NONE;
    EXPECT_EQ(QosStringUtils::ConvertQosDir(eDirTag), "none");

    eDirTag = SdpPrecondition::DIRECTION_SEND;
    EXPECT_EQ(QosStringUtils::ConvertQosDir(eDirTag), "send");

    eDirTag = SdpPrecondition::DIRECTION_RECV;
    EXPECT_EQ(QosStringUtils::ConvertQosDir(eDirTag), "recv");

    eDirTag = SdpPrecondition::DIRECTION_SENDRECV;
    EXPECT_EQ(QosStringUtils::ConvertQosDir(eDirTag), "sendrecv");

    eDirTag = SdpPrecondition::DIRECTION_MAX;
    EXPECT_EQ(QosStringUtils::ConvertQosDir(eDirTag), "invalid max");

    eDirTag = SdpPrecondition::DIRECTION_INVALID - 1;
    EXPECT_EQ(QosStringUtils::ConvertQosDir(eDirTag), "OUT_OF_RANGE");

    eDirTag = SdpPrecondition::DIRECTION_MAX + 1;
    EXPECT_EQ(QosStringUtils::ConvertQosDir(eDirTag), "OUT_OF_RANGE");
}

TEST_F(QosStringUtilsTest, ConvertQosStrength)
{
    IMS_SINT32 eStrengthTag = SdpPrecondition::STRENGTH_INVALID;
    EXPECT_EQ(QosStringUtils::ConvertQosStrength(eStrengthTag), "invalid");

    eStrengthTag = SdpPrecondition::STRENGTH_MANDATORY;
    EXPECT_EQ(QosStringUtils::ConvertQosStrength(eStrengthTag), "mandatory");

    eStrengthTag = SdpPrecondition::STRENGTH_OPTIONAL;
    EXPECT_EQ(QosStringUtils::ConvertQosStrength(eStrengthTag), "optional");

    eStrengthTag = SdpPrecondition::STRENGTH_NONE;
    EXPECT_EQ(QosStringUtils::ConvertQosStrength(eStrengthTag), "none");

    eStrengthTag = SdpPrecondition::STRENGTH_FAILURE;
    EXPECT_EQ(QosStringUtils::ConvertQosStrength(eStrengthTag), "failure");

    eStrengthTag = SdpPrecondition::STRENGTH_UNKNOWN;
    EXPECT_EQ(QosStringUtils::ConvertQosStrength(eStrengthTag), "unknown");

    eStrengthTag = SdpPrecondition::STRENGTH_NOTUSED;
    EXPECT_EQ(QosStringUtils::ConvertQosStrength(eStrengthTag), "not used");

    eStrengthTag = SdpPrecondition::STRENGTH_MAX;
    EXPECT_EQ(QosStringUtils::ConvertQosStrength(eStrengthTag), "invalid max");

    eStrengthTag = SdpPrecondition::STRENGTH_INVALID - 1;
    EXPECT_EQ(QosStringUtils::ConvertQosStrength(eStrengthTag), "OUT_OF_RANGE");

    eStrengthTag = SdpPrecondition::STRENGTH_MAX + 1;
    EXPECT_EQ(QosStringUtils::ConvertQosStrength(eStrengthTag), "OUT_OF_RANGE");
}

TEST_F(QosStringUtilsTest, ConvertQosLossPolicy)
{
    QosLossPolicy ePolicy = QosLossPolicy::MAINTAIN;
    EXPECT_EQ(QosStringUtils::ConvertQosLossPolicy(ePolicy), "maintain");

    ePolicy = QosLossPolicy::MODIFY;
    EXPECT_EQ(QosStringUtils::ConvertQosLossPolicy(ePolicy), "modify");

    ePolicy = QosLossPolicy::RELEASE;
    EXPECT_EQ(QosStringUtils::ConvertQosLossPolicy(ePolicy), "release");
}

TEST_F(QosStringUtilsTest, ConvertQosStatus)
{
    QosStatus eStatus = QosStatus::IDLE;
    EXPECT_EQ(QosStringUtils::ConvertQosStatus(eStatus), "idle");

    eStatus = QosStatus::AVAILABLE;
    EXPECT_EQ(QosStringUtils::ConvertQosStatus(eStatus), "available");

    eStatus = QosStatus::LOST;
    EXPECT_EQ(QosStringUtils::ConvertQosStatus(eStatus), "lost");
}

TEST_F(QosStringUtilsTest, ConvertQosTimerType)
{
    QosTimerType eTimerType = QosTimerType::WAIT_AUDIO_DEDICATED_BEARER;
    EXPECT_EQ(QosStringUtils::ConvertQosTimerType(eTimerType), "wait audio dedicated bearer");

    eTimerType = QosTimerType::WAIT_AVAILABLE_AFTER_W2L_HANDOVER;
    EXPECT_EQ(QosStringUtils::ConvertQosTimerType(eTimerType), "wait available after w2l handover");

    eTimerType = QosTimerType::WAIT_VIDEO_TEXT_AVAILABLE;
    EXPECT_EQ(QosStringUtils::ConvertQosTimerType(eTimerType), "wait video text available");

    eTimerType = QosTimerType::GUARD_AFTER_LOST;
    EXPECT_EQ(QosStringUtils::ConvertQosTimerType(eTimerType), "guard after lost");

    eTimerType = QosTimerType::FORCE_AVAILABLE;
    EXPECT_EQ(QosStringUtils::ConvertQosTimerType(eTimerType), "force available");
}

TEST_F(QosStringUtilsTest, ConvertQosStatusType)
{
    IMS_SINT32 eStatusType = SdpPrecondition::STATUS_INVALID;
    EXPECT_EQ(QosStringUtils::ConvertQosStatusType(eStatusType), "invalid");

    eStatusType = SdpPrecondition::STATUS_E2E;
    EXPECT_EQ(QosStringUtils::ConvertQosStatusType(eStatusType), "e2e");

    eStatusType = SdpPrecondition::STATUS_LOCAL;
    EXPECT_EQ(QosStringUtils::ConvertQosStatusType(eStatusType), "local");

    eStatusType = SdpPrecondition::STATUS_REMOTE;
    EXPECT_EQ(QosStringUtils::ConvertQosStatusType(eStatusType), "remote");

    eStatusType = SdpPrecondition::STATUS_MAX;
    EXPECT_EQ(QosStringUtils::ConvertQosStatusType(eStatusType), "invalid max");

    eStatusType = SdpPrecondition::STATUS_INVALID - 1;
    EXPECT_EQ(QosStringUtils::ConvertQosStatusType(eStatusType), "OUT_OF_RANGE");

    eStatusType = SdpPrecondition::STATUS_MAX + 1;
    EXPECT_EQ(QosStringUtils::ConvertQosStatusType(eStatusType), "OUT_OF_RANGE");
}
