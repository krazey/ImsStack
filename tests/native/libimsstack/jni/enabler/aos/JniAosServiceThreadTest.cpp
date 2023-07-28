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

TEST_F(JniAosServiceThreadTest, NotifyRegistered)
{
    ImsList<AString> objFeatureTags = ImsList<AString>();
    objFeatureTags.Append(AString("featureTag1"));
    objFeatureTags.Append(AString("featureTag2"));
    objFeatureTags.Append(AString("featureTag3"));
    EXPECT_THAT(m_pJniAosServiceThread->NotifyRegistered(0, 0, objFeatureTags),
            AnyOf(IMS_TRUE, IMS_FALSE));
}

TEST_F(JniAosServiceThreadTest, NotifyRegistering)
{
    ImsList<AString> objFeatureTags = ImsList<AString>();
    objFeatureTags.Append(AString("featureTag1"));
    objFeatureTags.Append(AString("featureTag2"));
    objFeatureTags.Append(AString("featureTag3"));
    EXPECT_THAT(m_pJniAosServiceThread->NotifyRegistering(0, 0, objFeatureTags),
            AnyOf(IMS_TRUE, IMS_FALSE));
}

TEST_F(JniAosServiceThreadTest, NotifyDeregistered)
{
    EXPECT_THAT(m_pJniAosServiceThread->NotifyDeregistered(0, 0), AnyOf(IMS_TRUE, IMS_FALSE));
}

TEST_F(JniAosServiceThreadTest, NotifyTechnologyChangeFailed)
{
    EXPECT_THAT(
            m_pJniAosServiceThread->NotifyTechnologyChangeFailed(0, 0), AnyOf(IMS_TRUE, IMS_FALSE));
}

TEST_F(JniAosServiceThreadTest, NotifyAssociatedUriChanged)
{
    ImsList<AString> objUris = ImsList<AString>();
    objUris.Append(AString("uri1"));
    objUris.Append(AString("uri2"));
    objUris.Append(AString("uri3"));
    EXPECT_THAT(m_pJniAosServiceThread->NotifyAssociatedUriChanged(objUris),
            AnyOf(IMS_TRUE, IMS_FALSE));
}

TEST_F(JniAosServiceThreadTest, NotifyCapabilitiesUpdateFailed)
{
    EXPECT_THAT(m_pJniAosServiceThread->NotifyCapabilitiesUpdateFailed(0, 0, 0),
            AnyOf(IMS_TRUE, IMS_FALSE));
}

TEST_F(JniAosServiceThreadTest, NotifyAosIsimState)
{
    EXPECT_THAT(m_pJniAosServiceThread->NotifyAosIsimState(0), AnyOf(IMS_TRUE, IMS_FALSE));
}

TEST_F(JniAosServiceThreadTest, NotifyRegEventState)
{
    ImsList<AString> objImpus;
    EXPECT_THAT(
            m_pJniAosServiceThread->NotifyRegEventState(200, objImpus), AnyOf(IMS_TRUE, IMS_FALSE));
}

TEST_F(JniAosServiceThreadTest, RequestPhoneNumberRetry)
{
    EXPECT_THAT(m_pJniAosServiceThread->RequestPhoneNumberRetry(0), AnyOf(IMS_TRUE, IMS_FALSE));
}

TEST_F(JniAosServiceThreadTest, RequestWifiService)
{
    EXPECT_THAT(m_pJniAosServiceThread->RequestWifiService(IMS_TRUE), AnyOf(IMS_TRUE, IMS_FALSE));
}