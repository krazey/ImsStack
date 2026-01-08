/*
 * Copyright (C) 2023 The Android Open Source Project
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

#include "ImsUuid.h"
#include "PlatformContext.h"
#include "TestUtilService.h"

using ::testing::_;
using ::testing::Invoke;
using ::testing::Unused;

namespace android
{

class ImsUuidTest : public ::testing::Test
{
public:
    inline ImsUuidTest() :
            m_strUuid("7d444840-9dc0-11d1-b245-5ffdce74fad2")
    {
    }

protected:
    TestUtilService m_objUtilService;
    AString m_strUuid;

protected:
    virtual void SetUp() override
    {
        PlatformContext::GetInstance()->SetService(
                PlatformContext::SERVICE_UTIL, &m_objUtilService);

        ON_CALL(m_objUtilService.GetMockSystemUtil(), GetUuid)
                .WillByDefault(Invoke(
                        [&](Unused, AString& strUuid, Unused)
                        {
                            strUuid = m_strUuid;
                        }));
    }

    virtual void TearDown() override
    {
        PlatformContext::GetInstance()->SetService(PlatformContext::SERVICE_UTIL, IMS_NULL);
    }
};

TEST_F(ImsUuidTest, GetUuid)
{
    AString strUuid = ImsUuid::GetUuid(ImsUuid::VERSION_1);
    EXPECT_EQ(strUuid, m_strUuid);

    strUuid = ImsUuid::GetUuid(ImsUuid::VERSION_3, "device-id");
    EXPECT_EQ(strUuid, m_strUuid);

    strUuid = ImsUuid::GetUuid(ImsUuid::VERSION_4);
    EXPECT_EQ(strUuid, m_strUuid);

    strUuid = ImsUuid::GetUuid(2);
    EXPECT_TRUE(strUuid.IsNull());

    strUuid = ImsUuid::GetUuid(3, AString::ConstNull());
    EXPECT_TRUE(strUuid.IsNull());

    strUuid = ImsUuid::GetUuid(3, AString::ConstEmpty());
    EXPECT_TRUE(strUuid.IsNull());
}

}  // namespace android
