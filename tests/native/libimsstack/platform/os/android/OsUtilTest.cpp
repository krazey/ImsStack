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
 * WITHout WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "ByteArray.h"
#include "MockISystem.h"
#include "OsUtil.h"
#include "PlatformContext.h"

using ::testing::_;
using ::testing::AnyOf;
using ::testing::Invoke;
using ::testing::Unused;

namespace android
{

class OsUtilTest : public ::testing::Test
{
protected:
    MockISystem m_objSystem;
    OsUtil* m_pOsUtil;

protected:
    virtual void SetUp() override
    {
        PlatformContext::GetInstance()->SetSystem(&m_objSystem);

        m_pOsUtil = new OsUtil();
        ASSERT_TRUE(m_pOsUtil != nullptr);
    }

    virtual void TearDown() override
    {
        if (m_pOsUtil != IMS_NULL)
        {
            delete m_pOsUtil;
            m_pOsUtil = IMS_NULL;
        }
        PlatformContext::GetInstance()->SetSystem(IMS_NULL);
    }
};

TEST_F(OsUtilTest, IsServerInfoHiddenInLog)
{
    EXPECT_FALSE(m_pOsUtil->IsServerInfoHiddenInLog());
    EXPECT_TRUE(m_pOsUtil->IsDebugMode());
    m_pOsUtil->SetDebugOn(IMS_TRUE);
    EXPECT_FALSE(m_pOsUtil->IsServerInfoHiddenInLog());
    m_pOsUtil->SetDebugOn(IMS_FALSE);
    EXPECT_FALSE(m_pOsUtil->IsServerInfoHiddenInLog());
}

TEST_F(OsUtilTest, SystemUtil)
{
    ON_CALL(m_objSystem, GetUuid)
            .WillByDefault(Invoke(
                    [&](Unused, Unused, AString& strUuid)
                    {
                        strUuid = "7d444840-9dc0-11d1-b245-5ffdce74fad2";
                    }));
    AString strUuid;
    ISystemUtil* piSystemUtil = static_cast<ISystemUtil*>(m_pOsUtil);
    piSystemUtil->GetUuid(1, strUuid);
    const AString strDefaultUuid("7d444840-9dc0-11d1-b245-5ffdce74fad2");
    EXPECT_EQ(strUuid, strDefaultUuid);
}

TEST_F(OsUtilTest, SystemProperty)
{
    AString strUser("user");
    AString strUserDebug("userdebug");
    AString strEng("eng");

    ISystemProperty* piSystemProperty = static_cast<ISystemProperty*>(m_pOsUtil);
    EXPECT_THAT(piSystemProperty->Get("ro.build.type"), AnyOf(strUser, strUserDebug, strEng));
}

TEST_F(OsUtilTest, Zlib)
{
    IZLib* piZlib = static_cast<IZLib*>(m_pOsUtil);
    ByteArray objContent("Text to compress");
    ByteArray objOut;
    EXPECT_TRUE(piZlib->Compress(objContent, objOut));

    ByteArray objUnCompressedOut;
    EXPECT_TRUE(piZlib->Uncompress(objOut, objUnCompressedOut));
}

}  // namespace android
