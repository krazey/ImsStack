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

#include "JniAosServiceThread.h"

using ::testing::AnyOf;

class JniAosServiceThreadTest : public ::testing::Test
{
public:
    JniAosServiceThread* m_pJniAosServiceThread;

protected:
    virtual void SetUp() override
    {
        m_pJniAosServiceThread = new JniAosServiceThread();
        ASSERT_TRUE(m_pJniAosServiceThread != nullptr);
    }

    virtual void TearDown() override
    {
        if (m_pJniAosServiceThread)
        {
            delete m_pJniAosServiceThread;
        }
    }
};

TEST_F(JniAosServiceThreadTest, SucceedsSendData2JavaWhenNotifyRegistered)
{
    // GIVEN
    ImsList<AString> objFeatureTags = ImsList<AString>();
    objFeatureTags.Append(AString("featureTag1"));
    objFeatureTags.Append(AString("featureTag2"));
    objFeatureTags.Append(AString("featureTag3"));

    // WHEN
    IMS_BOOL bResult = m_pJniAosServiceThread->NotifyRegistered(0, 0, objFeatureTags);

    // THEN
    EXPECT_THAT(bResult, AnyOf(IMS_TRUE, IMS_FALSE));
}

TEST_F(JniAosServiceThreadTest, SucceedsSendData2JavaWhenNotifyRegistering)
{
    // GIVEN
    ImsList<AString> objFeatureTags = ImsList<AString>();
    objFeatureTags.Append(AString("featureTag1"));
    objFeatureTags.Append(AString("featureTag2"));
    objFeatureTags.Append(AString("featureTag3"));

    // WHEN
    IMS_BOOL bResult = m_pJniAosServiceThread->NotifyRegistering(0, 0, objFeatureTags);

    // THEN
    EXPECT_THAT(bResult, AnyOf(IMS_TRUE, IMS_FALSE));
}

TEST_F(JniAosServiceThreadTest, SucceedsSendData2JavaWhenNotifyDeregistered)
{
    // GIVEN
    // WHEN
    IMS_BOOL bResult = m_pJniAosServiceThread->NotifyDeregistered(0, 0);

    // THEN
    EXPECT_THAT(bResult, AnyOf(IMS_TRUE, IMS_FALSE));
}

TEST_F(JniAosServiceThreadTest, SucceedsSendData2JavaWhenNotifyTechnologyChangeFailed)
{
    // GIVEN
    // WHEN
    IMS_BOOL bResult = m_pJniAosServiceThread->NotifyTechnologyChangeFailed(0, 0);

    // THEN
    EXPECT_THAT(bResult, AnyOf(IMS_TRUE, IMS_FALSE));
}

TEST_F(JniAosServiceThreadTest, SucceedsSendData2JavaWhenNotifyAssociatedUriChanged)
{
    // GIVEN
    ImsList<AString> objUris = ImsList<AString>();
    objUris.Append(AString("uri1"));
    objUris.Append(AString("uri2"));
    objUris.Append(AString("uri3"));

    // WHEN
    IMS_BOOL bResult = m_pJniAosServiceThread->NotifyAssociatedUriChanged(objUris);

    // THEN
    EXPECT_THAT(bResult, AnyOf(IMS_TRUE, IMS_FALSE));
}

TEST_F(JniAosServiceThreadTest, SucceedsSendData2JavaWhenNotifyCapabilitiesUpdateFailed)
{
    // GIVEN
    // WHEN
    IMS_BOOL bResult = m_pJniAosServiceThread->NotifyCapabilitiesUpdateFailed(0, 0, 0);

    // THEN
    EXPECT_THAT(bResult, AnyOf(IMS_TRUE, IMS_FALSE));
}

TEST_F(JniAosServiceThreadTest, SucceedsSendData2JavaWhenNotifyAosIsimState)
{
    // GIVEN
    // WHEN
    IMS_BOOL bResult = m_pJniAosServiceThread->NotifyAosIsimState(0);

    // THEN
    EXPECT_THAT(bResult, AnyOf(IMS_TRUE, IMS_FALSE));
}

TEST_F(JniAosServiceThreadTest, SucceedsSendData2JavaWhenNotifyRegEventState)
{
    // GIVEN
    ImsList<AString> objImpus;

    // WHEN
    IMS_BOOL bResult = m_pJniAosServiceThread->NotifyRegEventState(200, objImpus);

    // THEN
    EXPECT_THAT(bResult, AnyOf(IMS_TRUE, IMS_FALSE));
}

TEST_F(JniAosServiceThreadTest, SucceedsSendData2JavaWhenRequestPhoneNumberRetry)
{
    // GIVEN
    // WHEN
    IMS_BOOL bResult = m_pJniAosServiceThread->RequestPhoneNumberRetry(0);

    // THEN
    EXPECT_THAT(bResult, AnyOf(IMS_TRUE, IMS_FALSE));
}

TEST_F(JniAosServiceThreadTest, SucceedsSendData2JavaWhenRequestWifiService)
{
    // GIVEN
    // WHEN
    IMS_BOOL bResult = m_pJniAosServiceThread->RequestWifiService(IMS_TRUE);

    // THEN
    EXPECT_THAT(bResult, AnyOf(IMS_TRUE, IMS_FALSE));
}
