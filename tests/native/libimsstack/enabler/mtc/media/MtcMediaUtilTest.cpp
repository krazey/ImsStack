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

#include "IMtcService.h"
#include "INetworkWatcher.h"
#include "ImsTypeDef.h"
#include "MediaDef.h"
#include "MockIMtcService.h"
#include "MtcDef.h"
#include "call/IMtcCall.h"
#include "media/MtcMediaUtil.h"
#include <gtest/gtest.h>

using ::testing::AnyNumber;
using ::testing::Return;

namespace android
{

class MtcMediaUtilTest : public ::testing::Test
{
protected:
    virtual void SetUp() override {}

    virtual void TearDown() override {}
};

TEST_F(MtcMediaUtilTest, GetCallTypeFromMediaTypes)
{
    IMS_UINT32 eMediaTypes = MEDIATYPE_NONE;
    EXPECT_EQ(MtcMediaUtil::GetCallTypeFromMediaTypes(eMediaTypes), CallType::UNKNOWN);

    eMediaTypes = MEDIATYPE_AUDIO;
    EXPECT_EQ(MtcMediaUtil::GetCallTypeFromMediaTypes(eMediaTypes), CallType::VOIP);

    eMediaTypes = MEDIATYPE_VIDEO;
    EXPECT_EQ(MtcMediaUtil::GetCallTypeFromMediaTypes(eMediaTypes), CallType::UNKNOWN);

    eMediaTypes = MEDIATYPE_TEXT;
    EXPECT_EQ(MtcMediaUtil::GetCallTypeFromMediaTypes(eMediaTypes), CallType::UNKNOWN);

    eMediaTypes = MEDIATYPE_AUDIO | MEDIATYPE_TEXT;
    EXPECT_EQ(MtcMediaUtil::GetCallTypeFromMediaTypes(eMediaTypes), CallType::RTT);

    eMediaTypes = MEDIATYPE_AUDIO | MEDIATYPE_VIDEO;
    EXPECT_EQ(MtcMediaUtil::GetCallTypeFromMediaTypes(eMediaTypes), CallType::VT);

    eMediaTypes = MEDIATYPE_VIDEO | MEDIATYPE_TEXT;
    EXPECT_EQ(MtcMediaUtil::GetCallTypeFromMediaTypes(eMediaTypes), CallType::UNKNOWN);

    eMediaTypes = MEDIATYPE_AUDIO | MEDIATYPE_VIDEO | MEDIATYPE_TEXT;
    EXPECT_EQ(MtcMediaUtil::GetCallTypeFromMediaTypes(eMediaTypes), CallType::VIDEO_RTT);
}

TEST_F(MtcMediaUtilTest, GetCallTypeFromMediaContents)
{
    MEDIA_CONTENT_TYPE eMediaContents = MEDIA_TYPE_INVALID;
    EXPECT_EQ(MtcMediaUtil::GetCallTypeFromMediaContents(eMediaContents), CallType::UNKNOWN);

    eMediaContents = MEDIA_TYPE_AUDIO;
    EXPECT_EQ(MtcMediaUtil::GetCallTypeFromMediaContents(eMediaContents), CallType::VOIP);

    eMediaContents = MEDIA_TYPE_VIDEO;
    EXPECT_EQ(MtcMediaUtil::GetCallTypeFromMediaContents(eMediaContents), CallType::UNKNOWN);

    eMediaContents = MEDIA_TYPE_AUDIOVIDEO;
    EXPECT_EQ(MtcMediaUtil::GetCallTypeFromMediaContents(eMediaContents), CallType::VT);

    eMediaContents = MEDIA_TYPE_TEXT;
    EXPECT_EQ(MtcMediaUtil::GetCallTypeFromMediaContents(eMediaContents), CallType::UNKNOWN);

    eMediaContents = MEDIA_TYPE_AUDIOTEXT;
    EXPECT_EQ(MtcMediaUtil::GetCallTypeFromMediaContents(eMediaContents), CallType::RTT);

    eMediaContents = MEDIA_TYPE_VIDEOTEXT;
    EXPECT_EQ(MtcMediaUtil::GetCallTypeFromMediaContents(eMediaContents), CallType::UNKNOWN);

    eMediaContents = MEDIA_TYPE_AUDIOVIDEOTEXT;
    EXPECT_EQ(MtcMediaUtil::GetCallTypeFromMediaContents(eMediaContents), CallType::VIDEO_RTT);

    eMediaContents = MEDIA_TYPE_NOTUSED;
    EXPECT_EQ(MtcMediaUtil::GetCallTypeFromMediaContents(eMediaContents), CallType::UNKNOWN);
}

TEST_F(MtcMediaUtilTest, GetMediaTypesFromCallType)
{
    CallType eCallType = CallType::UNKNOWN;
    EXPECT_EQ(MtcMediaUtil::GetMediaTypesFromCallType(eCallType), MEDIATYPE_NONE);

    eCallType = CallType::VOIP;
    EXPECT_EQ(MtcMediaUtil::GetMediaTypesFromCallType(eCallType), MEDIATYPE_AUDIO);

    eCallType = CallType::VT;
    EXPECT_EQ(
            MtcMediaUtil::GetMediaTypesFromCallType(eCallType), MEDIATYPE_AUDIO | MEDIATYPE_VIDEO);

    eCallType = CallType::RTT;
    EXPECT_EQ(MtcMediaUtil::GetMediaTypesFromCallType(eCallType), MEDIATYPE_AUDIO | MEDIATYPE_TEXT);

    eCallType = CallType::VIDEO_RTT;
    EXPECT_EQ(MtcMediaUtil::GetMediaTypesFromCallType(eCallType),
            MEDIATYPE_AUDIO | MEDIATYPE_VIDEO | MEDIATYPE_TEXT);
}

TEST_F(MtcMediaUtilTest, GetMediaTypesFromMediaContents)
{
    MEDIA_CONTENT_TYPE eMediaContents = MEDIA_TYPE_INVALID;
    EXPECT_EQ(MtcMediaUtil::GetMediaTypesFromMediaContents(eMediaContents), MEDIATYPE_NONE);

    eMediaContents = MEDIA_TYPE_AUDIO;
    EXPECT_EQ(MtcMediaUtil::GetMediaTypesFromMediaContents(eMediaContents), MEDIATYPE_AUDIO);

    eMediaContents = MEDIA_TYPE_VIDEO;
    EXPECT_EQ(MtcMediaUtil::GetMediaTypesFromMediaContents(eMediaContents), MEDIATYPE_VIDEO);

    eMediaContents = MEDIA_TYPE_AUDIOVIDEO;
    EXPECT_EQ(MtcMediaUtil::GetMediaTypesFromMediaContents(eMediaContents),
            MEDIATYPE_AUDIO | MEDIATYPE_VIDEO);

    eMediaContents = MEDIA_TYPE_TEXT;
    EXPECT_EQ(MtcMediaUtil::GetMediaTypesFromMediaContents(eMediaContents), MEDIATYPE_TEXT);

    eMediaContents = MEDIA_TYPE_AUDIOTEXT;
    EXPECT_EQ(MtcMediaUtil::GetMediaTypesFromMediaContents(eMediaContents),
            MEDIATYPE_AUDIO | MEDIATYPE_TEXT);

    eMediaContents = MEDIA_TYPE_VIDEOTEXT;
    EXPECT_EQ(MtcMediaUtil::GetMediaTypesFromMediaContents(eMediaContents),
            MEDIATYPE_VIDEO | MEDIATYPE_TEXT);

    eMediaContents = MEDIA_TYPE_AUDIOVIDEOTEXT;
    EXPECT_EQ(MtcMediaUtil::GetMediaTypesFromMediaContents(eMediaContents),
            MEDIATYPE_AUDIO | MEDIATYPE_VIDEO | MEDIATYPE_TEXT);

    eMediaContents = MEDIA_TYPE_NOTUSED;
    EXPECT_EQ(MtcMediaUtil::GetMediaTypesFromMediaContents(eMediaContents), MEDIATYPE_NONE);
}

TEST_F(MtcMediaUtilTest, GetMediaContentsFromMediaTypes)
{
    IMS_UINT32 eMediaTypes = MEDIATYPE_NONE;
    EXPECT_EQ(MtcMediaUtil::GetMediaContentsFromMediaTypes(eMediaTypes), MEDIA_TYPE_INVALID);

    eMediaTypes = MEDIATYPE_AUDIO;
    EXPECT_EQ(MtcMediaUtil::GetMediaContentsFromMediaTypes(eMediaTypes), MEDIA_TYPE_AUDIO);

    eMediaTypes = MEDIATYPE_VIDEO;
    EXPECT_EQ(MtcMediaUtil::GetMediaContentsFromMediaTypes(eMediaTypes), MEDIA_TYPE_VIDEO);

    eMediaTypes = MEDIATYPE_TEXT;
    EXPECT_EQ(MtcMediaUtil::GetMediaContentsFromMediaTypes(eMediaTypes), MEDIA_TYPE_TEXT);

    eMediaTypes = MEDIATYPE_AUDIO | MEDIATYPE_TEXT;
    EXPECT_EQ(MtcMediaUtil::GetMediaContentsFromMediaTypes(eMediaTypes), MEDIA_TYPE_AUDIOTEXT);

    eMediaTypes = MEDIATYPE_AUDIO | MEDIATYPE_VIDEO;
    EXPECT_EQ(MtcMediaUtil::GetMediaContentsFromMediaTypes(eMediaTypes), MEDIA_TYPE_AUDIOVIDEO);

    eMediaTypes = MEDIATYPE_VIDEO | MEDIATYPE_TEXT;
    EXPECT_EQ(MtcMediaUtil::GetMediaContentsFromMediaTypes(eMediaTypes), MEDIA_TYPE_VIDEOTEXT);

    eMediaTypes = MEDIATYPE_VIDEO | MEDIATYPE_TEXT | MEDIATYPE_AUDIO;
    EXPECT_EQ(MtcMediaUtil::GetMediaContentsFromMediaTypes(eMediaTypes), MEDIA_TYPE_AUDIOVIDEOTEXT);
}

TEST_F(MtcMediaUtilTest, GetMediaContentsFromCallType)
{
    CallType eCallType = CallType::UNKNOWN;
    EXPECT_EQ(MtcMediaUtil::GetMediaContentsFromCallType(eCallType), MEDIA_TYPE_INVALID);

    eCallType = CallType::VOIP;
    EXPECT_EQ(MtcMediaUtil::GetMediaContentsFromCallType(eCallType), MEDIA_TYPE_AUDIO);

    eCallType = CallType::VT;
    EXPECT_EQ(MtcMediaUtil::GetMediaContentsFromCallType(eCallType), MEDIA_TYPE_AUDIOVIDEO);

    eCallType = CallType::RTT;
    EXPECT_EQ(MtcMediaUtil::GetMediaContentsFromCallType(eCallType), MEDIA_TYPE_AUDIOTEXT);

    eCallType = CallType::VIDEO_RTT;
    EXPECT_EQ(MtcMediaUtil::GetMediaContentsFromCallType(eCallType), MEDIA_TYPE_AUDIOVIDEOTEXT);
}

TEST_F(MtcMediaUtilTest, GetMediaServiceType)
{
    ServiceType eServiceType = ServiceType::UNKNOWN;
    EXPECT_EQ(MtcMediaUtil::GetMediaServiceType(eServiceType), MEDIA_SERVICE_DEFAULT);

    eServiceType = ServiceType::NORMAL;
    EXPECT_EQ(MtcMediaUtil::GetMediaServiceType(eServiceType), MEDIA_SERVICE_DEFAULT);

    eServiceType = ServiceType::EMERGENCY;
    EXPECT_EQ(MtcMediaUtil::GetMediaServiceType(eServiceType), MEDIA_SERVICE_EMERGENCY);
}

TEST_F(MtcMediaUtilTest, GetMediaNetworkType)
{
    MockIMtcService objMtcService;

    EXPECT_CALL(objMtcService, IsWlanIpCanType)
            .Times(AnyNumber())
            .WillOnce(Return(IMS_TRUE))
            .WillRepeatedly(Return(IMS_FALSE));

    IMS_SINT32 eType = INetworkWatcher::RADIOTECH_TYPE_LTE;
    EXPECT_EQ(MtcMediaUtil::GetMediaNetworkType(&objMtcService, eType), MEDIA_NETWORK_WIFI);
    EXPECT_EQ(MtcMediaUtil::GetMediaNetworkType(&objMtcService, eType), MEDIA_NETWORK_LTE);

    eType = INetworkWatcher::RADIOTECH_TYPE_HSPAP;
    EXPECT_EQ(MtcMediaUtil::GetMediaNetworkType(&objMtcService, eType), MEDIA_NETWORK_HSPA_PLUS);

    eType = INetworkWatcher::RADIOTECH_TYPE_UMTS;
    EXPECT_EQ(MtcMediaUtil::GetMediaNetworkType(&objMtcService, eType), MEDIA_NETWORK_HSPA);

    eType = INetworkWatcher::RADIOTECH_TYPE_HSPA;
    EXPECT_EQ(MtcMediaUtil::GetMediaNetworkType(&objMtcService, eType), MEDIA_NETWORK_HSPA);

    eType = INetworkWatcher::RADIOTECH_TYPE_HSDPA;
    EXPECT_EQ(MtcMediaUtil::GetMediaNetworkType(&objMtcService, eType), MEDIA_NETWORK_HSPA);

    eType = INetworkWatcher::RADIOTECH_TYPE_HSUPA;
    EXPECT_EQ(MtcMediaUtil::GetMediaNetworkType(&objMtcService, eType), MEDIA_NETWORK_HSPA);

    eType = INetworkWatcher::RADIOTECH_TYPE_CDMA;
    EXPECT_EQ(MtcMediaUtil::GetMediaNetworkType(&objMtcService, eType), MEDIA_NETWORK_HSPA);

    eType = INetworkWatcher::RADIOTECH_TYPE_EHRPD;
    EXPECT_EQ(MtcMediaUtil::GetMediaNetworkType(&objMtcService, eType), MEDIA_NETWORK_EHRPD);

    eType = INetworkWatcher::RADIOTECH_TYPE_LTE_CA;
    EXPECT_EQ(MtcMediaUtil::GetMediaNetworkType(&objMtcService, eType), MEDIA_NETWORK_LTE);
}

TEST_F(MtcMediaUtilTest, GetGttModeFromTextQuality)
{
    IMS_UINT32 eTextQuality = TEXT_QUALITY_NONE;
    EXPECT_EQ(MtcMediaUtil::GetGttModeFromTextQuality(eTextQuality), GTT_MODE_INVALID);

    eTextQuality = TEXT_QUALITY_T140;
    EXPECT_EQ(MtcMediaUtil::GetGttModeFromTextQuality(eTextQuality), GTT_MODE_FULL);

    eTextQuality = TEXT_QUALITY_T140_RED;
    EXPECT_EQ(MtcMediaUtil::GetGttModeFromTextQuality(eTextQuality), GTT_MODE_FULL);

    eTextQuality = TEXT_QUALITY_NOTUSED;
    EXPECT_EQ(MtcMediaUtil::GetGttModeFromTextQuality(eTextQuality), GTT_MODE_INVALID);

    const IMS_UINT32 eInvalidTextQuality = -1;
    EXPECT_EQ(MtcMediaUtil::GetGttModeFromTextQuality(eInvalidTextQuality), GTT_MODE_INVALID);
}

TEST_F(MtcMediaUtilTest, MediaTypesToStringReturnsConvertedStringOfMediaTypes)
{
    IMS_UINT32 eMediaTypes = MEDIATYPE_NONE;
    EXPECT_TRUE(MtcMediaUtil::MediaTypesToString(eMediaTypes).Equals(""));

    eMediaTypes = MEDIATYPE_AUDIO;
    EXPECT_TRUE(MtcMediaUtil::MediaTypesToString(eMediaTypes).Contains("audio"));

    eMediaTypes |= MEDIATYPE_VIDEO;
    EXPECT_TRUE(MtcMediaUtil::MediaTypesToString(eMediaTypes).Contains("video"));

    eMediaTypes |= MEDIATYPE_TEXT;
    EXPECT_TRUE(MtcMediaUtil::MediaTypesToString(eMediaTypes).Contains("text"));
}

TEST_F(MtcMediaUtilTest, StringToMediaTypesReturnsMediaTypesFromString)
{
    IMS_UINT32 eMediaTypes = MEDIATYPE_NONE;
    AString strMediaTypes;
    EXPECT_EQ(MtcMediaUtil::StringToMediaTypes(strMediaTypes), eMediaTypes);

    strMediaTypes.Append("audio");
    eMediaTypes |= MEDIATYPE_AUDIO;
    EXPECT_EQ(MtcMediaUtil::StringToMediaTypes(strMediaTypes), eMediaTypes);

    strMediaTypes.Append("video");
    eMediaTypes |= MEDIATYPE_VIDEO;
    EXPECT_EQ(MtcMediaUtil::StringToMediaTypes(strMediaTypes), eMediaTypes);

    strMediaTypes.Append("text");
    eMediaTypes |= MEDIATYPE_TEXT;
    EXPECT_EQ(MtcMediaUtil::StringToMediaTypes(strMediaTypes), eMediaTypes);
}

}  // namespace android
