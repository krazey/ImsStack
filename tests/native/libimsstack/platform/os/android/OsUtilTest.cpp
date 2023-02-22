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

#include "ImsTypeDef.h"
#include "MockISystem.h"
#include "PlatformContext.h"
#include "OsUtil.h"

using ::testing::_;
using ::testing::Invoke;
using ::testing::Unused;

namespace android
{

class OsUtilTest : public ::testing::Test
{
public:
    MockISystem m_objMockSystem;
    ISystem* m_piDefaultSystem;

    OsUtil* m_pOsUtil;

protected:
    virtual void SetUp() override
    {
        m_piDefaultSystem = PlatformContext::GetInstance()->SetSystem(&m_objMockSystem);

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
        PlatformContext::GetInstance()->SetSystem(m_piDefaultSystem);
    }
};

TEST_F(OsUtilTest, InitializeReadOnlyProperties)
{
    m_pOsUtil->InitializeReadOnlyProperties();
    EXPECT_EQ(m_pOsUtil->IsUserMode(), IMS_FALSE);

    m_pOsUtil->SetDebugOn(IMS_TRUE);
    EXPECT_EQ(m_pOsUtil->IsDebugMode(), IMS_TRUE);
}

TEST_F(OsUtilTest, IsServerInfoHiddenInLog)
{
    EXPECT_EQ(m_pOsUtil->IsServerInfoHiddenInLog(), IMS_FALSE);
    EXPECT_EQ(m_pOsUtil->IsDebugMode(), IMS_TRUE);
    m_pOsUtil->SetDebugOn(IMS_TRUE);
    EXPECT_EQ(m_pOsUtil->IsServerInfoHiddenInLog(), IMS_FALSE);
    m_pOsUtil->SetDebugOn(IMS_FALSE);
    EXPECT_EQ(m_pOsUtil->IsServerInfoHiddenInLog(), IMS_FALSE);
}

TEST_F(OsUtilTest, SystemUtil)
{
    ISystemUtil* piSystemUtil = static_cast<ISystemUtil*>(m_pOsUtil);
    AString strOutValue("Digest");
    EXPECT_CALL(m_objMockSystem, GetDigestSha1(_, _))
            .Times(1)
            .WillOnce(Invoke(
                    [strOutValue](Unused, AString& strOut)
                    {
                        strOut = strOutValue;
                        return 1;
                    }));

    AString strValue("Certificate");
    AString strOut;
    piSystemUtil->DigestSha1(strValue, strOut);
    EXPECT_EQ(strOut, strOutValue);

    AString strUid("00000000-0000-0000-0000-000000000000");
    EXPECT_EQ(piSystemUtil->GetUuid(), strUid);
}

TEST_F(OsUtilTest, SystemProperty)
{
    ISystemProperty* piSystemProperty = static_cast<ISystemProperty*>(m_pOsUtil);
    AString strOut("userdebug");
    EXPECT_EQ(piSystemProperty->Get("ro.build.type"), strOut);
    EXPECT_EQ(piSystemProperty->Set("ro.build.type", "user"), IMS_TRUE);
    EXPECT_EQ(piSystemProperty->Set("ro.build.type", strOut), IMS_TRUE);
    EXPECT_EQ(piSystemProperty->Get("ro.build.type"), strOut);
}

TEST_F(OsUtilTest, Zlib)
{
    IZLib* piZlib = static_cast<IZLib*>(m_pOsUtil);
    ByteArray objContent("Text to compress");
    ByteArray objOut;
    EXPECT_EQ(piZlib->Compress(objContent, objOut), IMS_TRUE);

    ByteArray objUnCompressedOut;
    EXPECT_EQ(piZlib->Uncompress(objOut, objUnCompressedOut), IMS_TRUE);
}

}  // namespace android
