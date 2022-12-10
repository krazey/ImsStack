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

#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include "options/UceOptions.h"

#include "ServiceMessage.h"
#include "ServiceTimer.h"
#include "ServiceTrace.h"

__IMS_TRACE_TAG_USER_DECL__("UCE");

using ::testing::AnyNumber;
using ::testing::Return;

IMS_SINT32 OPTIONS_SIM_SLOT = 20;
IMS_UINT32 OPTIONS_KEY = 20;

class TestUceOptions : public UceOptions
{
public:
public:
    TestUceOptions() :
            UceOptions(AString("UceOptionsManager"), IMS_NULL, IMS_NULL, OPTIONS_KEY, IMS_TRUE,
                    OPTIONS_SIM_SLOT)
    {
    }
    virtual ~TestUceOptions() {}

    IMS_UINT32 GetKey() const { return m_nKey; }

    IMS_BOOL GetSendingRequest() const { return m_bIsSendingRequest; }

    void SetSendingRequest(IMS_BOOL value) { m_bIsSendingRequest = value; }
};

class UceOptionsTest : public ::testing::Test
{
public:
    TestUceOptions* pUceOptions;

protected:
    virtual void SetUp() override
    {
        pUceOptions = new TestUceOptions();
        ASSERT_TRUE(pUceOptions != nullptr);
    }

    virtual void TearDown() override
    {
        if (pUceOptions)
        {
            delete pUceOptions;
        }
    }
};

TEST_F(UceOptionsTest, AoSDisconnected)
{
    IMS_TRACE_D("AoSDisconnected", 0, 0, 0);
    pUceOptions->SetSendingRequest(IMS_TRUE);
    pUceOptions->AoSDisconnected();

    EXPECT_EQ(pUceOptions->GetKey(), 0);
    EXPECT_EQ(pUceOptions->GetSendingRequest(), IMS_FALSE);
}

TEST_F(UceOptionsTest, GetCapability)
{
    IMS_TRACE_D("GetCapability", 0, 0, 0);
    IMSList<AString> objContactList = IMSList<AString>();

    EXPECT_EQ(pUceOptions->GetCapability(objContactList), 0);

    objContactList.Append("urn%3Aurn-7%3A3gpp-application.ims.iari.rcse.dp");
    EXPECT_EQ(pUceOptions->GetCapability(objContactList), UceOptions::FEATURE_TAG_DP);

    objContactList.Append("urn%3Aurn-7%3A3gpp-service.ims.icsi.mmtel");
    objContactList.Append("video");
    EXPECT_EQ(pUceOptions->GetCapability(objContactList),
            UceOptions::FEATURE_TAG_DP | UceOptions::FEATURE_TAG_IPCALL_VOICE |
                    UceOptions::FEATURE_TAG_IPCALL_VIDEO);

    objContactList.Append("urn%3Aurn-7%3A3gpp-service.ims.icsi.oma.cpm.msg");
    objContactList.Append("urn%3Aurn-7%3A3gpp-service.ims.icsi.oma.cpm.largemsg");
    EXPECT_EQ(pUceOptions->GetCapability(objContactList),
            UceOptions::FEATURE_TAG_DP | UceOptions::FEATURE_TAG_IPCALL_VOICE |
                    UceOptions::FEATURE_TAG_IPCALL_VIDEO | UceOptions::FEATURE_TAG_PAGER_MESSAGING |
                    UceOptions::FEATURE_TAG_LARGE_MESSAGING);
}

TEST_F(UceOptionsTest, SetIARIFeatureTag)
{
    IMS_TRACE_D("SetIARIFeatureTag", 0, 0, 0);
    AString strIari = AString::ConstEmpty();
    pUceOptions->SetIARIFeatureTag(UceOptions::FEATURE_TAG_DP, strIari);
    EXPECT_STREQ(strIari.GetStr(), "urn%3Aurn-7%3A3gpp-application.ims.iari.rcse.dp");

    strIari = AString::ConstEmpty();
    pUceOptions->SetIARIFeatureTag(UceOptions::FEATURE_TAG_DP |
                    UceOptions::FEATURE_TAG_IPCALL_VOICE | UceOptions::FEATURE_TAG_IPCALL_VIDEO,
            strIari);
    EXPECT_STREQ(strIari.GetStr(), "urn%3Aurn-7%3A3gpp-application.ims.iari.rcse.dp");

    strIari = AString::ConstEmpty();
    pUceOptions->SetIARIFeatureTag(UceOptions::FEATURE_TAG_DP | UceOptions::FEATURE_TAG_SIMPLE_IM |
                    UceOptions::FEATURE_TAG_FT_HTTP,
            strIari);
    EXPECT_STREQ(strIari.GetStr(),
            "urn%3Aurn-7%3A3gpp-application.ims.iari.rcse.dp,urn%3Aurn-7%3A3gpp-application.ims."
            "iari.rcse.im,urn%3Aurn-7%3A3gpp-application.ims.iari.rcs.fthttp");
}

TEST_F(UceOptionsTest, SetICSIFeatureTag)
{
    IMS_TRACE_D("SetIARIFeatureTag", 0, 0, 0);
    AString strIcsi = AString::ConstEmpty();
    pUceOptions->SetICSIFeatureTag(UceOptions::FEATURE_TAG_IPCALL_VOICE, strIcsi);
    EXPECT_STREQ(strIcsi.GetStr(), "urn%3Aurn-7%3A3gpp-service.ims.icsi.mmtel");

    strIcsi = AString::ConstEmpty();
    pUceOptions->SetICSIFeatureTag(UceOptions::FEATURE_TAG_DP |
                    UceOptions::FEATURE_TAG_IPCALL_VOICE | UceOptions::FEATURE_TAG_IPCALL_VIDEO,
            strIcsi);
    EXPECT_STREQ(strIcsi.GetStr(), "urn%3Aurn-7%3A3gpp-service.ims.icsi.mmtel");

    strIcsi = AString::ConstEmpty();
    pUceOptions->SetICSIFeatureTag(UceOptions::FEATURE_TAG_CPM_CHAT |
                    UceOptions::FEATURE_TAG_IPCALL_VOICE | UceOptions::FEATURE_TAG_IPCALL_VIDEO,
            strIcsi);
    EXPECT_STREQ(strIcsi.GetStr(),
            "urn%3Aurn-7%3A3gpp-service.ims.icsi.mmtel,urn%3Aurn-7%3A3gpp-service.ims.icsi.oma.cpm."
            "session");
}

TEST_F(UceOptionsTest, SetNoTypeFeatureTag)
{
    IMS_TRACE_D("SetNoTypeFeatureTag", 0, 0, 0);
    AString strTag = AString::ConstEmpty();
    pUceOptions->SetNoTypeFeatureTag(UceOptions::FEATURE_TAG_IPCALL_VIDEO, strTag);
    EXPECT_STREQ(strTag.GetStr(), "video");

    strTag = AString::ConstEmpty();
    pUceOptions->SetNoTypeFeatureTag(
            UceOptions::FEATURE_TAG_DP | UceOptions::FEATURE_TAG_IPCALL_VIDEO, strTag);
    EXPECT_STREQ(strTag.GetStr(), "video");

    strTag = AString::ConstEmpty();
    pUceOptions->SetNoTypeFeatureTag(
            UceOptions::FEATURE_TAG_IPCALL_VIDEO | UceOptions::FEATURE_TAG_IS_BOT, strTag);
    EXPECT_STREQ(strTag.GetStr(), "video;+g.gsma.rcs.isbot");
}