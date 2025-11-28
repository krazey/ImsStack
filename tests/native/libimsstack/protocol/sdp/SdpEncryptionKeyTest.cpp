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

#include "AString.h"

#include "Sdp.h"
#include "SdpEncryptionKey.h"

namespace android
{

class TestEncryptionKey
{
public:
    inline TestEncryptionKey() :
            m_strSdpFullLine(AString::ConstNull()),
            m_strSdpLine(AString::ConstNull()),
            m_nMethod(SdpEncryptionKey::METHOD_INVALID),
            m_strKey(AString::ConstNull())
    {
    }
    inline TestEncryptionKey(const AString& strSdpFullLine, const AString& strSdpLine,
            IMS_SINT32 nMethod, const AString& strKey) :
            m_strSdpFullLine(strSdpFullLine),
            m_strSdpLine(strSdpLine),
            m_nMethod(nMethod),
            m_strKey(strKey)
    {
    }
    inline TestEncryptionKey(const TestEncryptionKey& other) :
            m_strSdpFullLine(other.m_strSdpFullLine),
            m_strSdpLine(other.m_strSdpLine),
            m_nMethod(other.m_nMethod),
            m_strKey(other.m_strKey)
    {
    }
    inline ~TestEncryptionKey() {}

    inline TestEncryptionKey& operator=(const TestEncryptionKey& other)
    {
        if (this != &other)
        {
            m_strSdpFullLine = other.m_strSdpFullLine;
            m_strSdpLine = other.m_strSdpLine;
            m_nMethod = other.m_nMethod;
            m_strKey = other.m_strKey;
        }

        return *this;
    }

public:
    AString m_strSdpFullLine;
    AString m_strSdpLine;
    IMS_SINT32 m_nMethod;
    AString m_strKey;
};

class SdpEncryptionKeyTest : public ::testing::Test
{
public:
    SdpEncryptionKeyTest();

protected:
    TestEncryptionKey m_objTestKey;
    ImsVector<TestEncryptionKey> m_objTestEncryptionKeys;
};

SdpEncryptionKeyTest::SdpEncryptionKeyTest() :
        m_objTestKey(TestEncryptionKey("k=base64:U0RQIGVuY3J5cHRpb24ga2V5IQ==\r\n",
                "base64:U0RQIGVuY3J5cHRpb24ga2V5IQ==", SdpEncryptionKey::METHOD_BASE64,
                "U0RQIGVuY3J5cHRpb24ga2V5IQ=="))
{
    // clang-format off
    m_objTestEncryptionKeys.Add(m_objTestKey);
    m_objTestEncryptionKeys.Add(TestEncryptionKey(
            "k=clear:SDP encryption key!\r\n",
            "clear:SDP encryption key!",
            SdpEncryptionKey::METHOD_CLEAR,
            "SDP encryption key!"));
    m_objTestEncryptionKeys.Add(TestEncryptionKey(
            "k=uri:https://www.example.com/key\r\n",
            "uri:https://www.example.com/key",
            SdpEncryptionKey::METHOD_URI,
            "https://www.example.com/key"));
    m_objTestEncryptionKeys.Add(TestEncryptionKey(
            "k=prompt\r\n",
            "prompt",
            SdpEncryptionKey::METHOD_PROMPT,
            AString::ConstNull()));
    // clang-format on
}

TEST_F(SdpEncryptionKeyTest, Constructor)
{
    SdpEncryptionKey objEncryptionKey;
    EXPECT_EQ(objEncryptionKey.GetMethod(), SdpEncryptionKey::METHOD_INVALID);
    EXPECT_EQ(objEncryptionKey.GetKey(), AString::ConstNull());
    EXPECT_EQ(objEncryptionKey.GetValue(), AString::ConstNull());
}

TEST_F(SdpEncryptionKeyTest, CopyConstructor)
{
    SdpEncryptionKey objEncryptionKey;
    ASSERT_TRUE(objEncryptionKey.SetValue(m_objTestKey.m_nMethod, m_objTestKey.m_strKey));

    SdpEncryptionKey objNewEncryptionKey(objEncryptionKey);
    EXPECT_EQ(objNewEncryptionKey.GetMethod(), objEncryptionKey.GetMethod());
    EXPECT_EQ(objNewEncryptionKey.GetKey(), objEncryptionKey.GetKey());
    EXPECT_EQ(objNewEncryptionKey.GetValue(), objEncryptionKey.GetValue());
}

TEST_F(SdpEncryptionKeyTest, OperatorAssignment)
{
    SdpEncryptionKey objEncryptionKey;
    ASSERT_TRUE(objEncryptionKey.SetValue(m_objTestKey.m_nMethod, m_objTestKey.m_strKey));

    SdpEncryptionKey objNewEncryptionKey;
    objNewEncryptionKey = objEncryptionKey;
    EXPECT_EQ(objNewEncryptionKey.GetMethod(), m_objTestKey.m_nMethod);
    EXPECT_EQ(objNewEncryptionKey.GetKey(), m_objTestKey.m_strKey);
}

TEST_F(SdpEncryptionKeyTest, Decode)
{
    SdpEncryptionKey objEncryptionKey;
    IMS_UINT32 nTestCount = m_objTestEncryptionKeys.GetSize();

    for (IMS_UINT32 i = 0; i < nTestCount; ++i)
    {
        const TestEncryptionKey& objTestKey = m_objTestEncryptionKeys.GetAt(i);

        ASSERT_TRUE(objEncryptionKey.Decode(objTestKey.m_strSdpLine));
        EXPECT_EQ(objEncryptionKey.GetValue(), objTestKey.m_strSdpLine);
        EXPECT_EQ(objEncryptionKey.GetMethod(), objTestKey.m_nMethod);
        EXPECT_EQ(objEncryptionKey.GetKey(), objTestKey.m_strKey);
    }

    EXPECT_FALSE(objEncryptionKey.Decode(AString::ConstNull()));
    EXPECT_FALSE(objEncryptionKey.Decode(AString::ConstEmpty()));
    EXPECT_FALSE(objEncryptionKey.Decode("prompt:test-key"));
    EXPECT_FALSE(objEncryptionKey.Decode("test:test-key"));
}

TEST_F(SdpEncryptionKeyTest, Encode)
{
    SdpEncryptionKey objEncryptionKey;
    AString strEncoded = objEncryptionKey.Encode();
    EXPECT_EQ(strEncoded, AString::ConstNull());

    IMS_UINT32 nTestCount = m_objTestEncryptionKeys.GetSize();

    for (IMS_UINT32 i = 0; i < nTestCount; ++i)
    {
        const TestEncryptionKey& objTestKey = m_objTestEncryptionKeys.GetAt(i);

        ASSERT_TRUE(objEncryptionKey.SetValue(objTestKey.m_nMethod, objTestKey.m_strKey));
        strEncoded = objEncryptionKey.Encode();
        EXPECT_EQ(objTestKey.m_strSdpFullLine, strEncoded);
    }
}

TEST_F(SdpEncryptionKeyTest, SetValue)
{
    SdpEncryptionKey objEncryptionKey;
    IMS_UINT32 nTestCount = m_objTestEncryptionKeys.GetSize();

    for (IMS_UINT32 i = 0; i < nTestCount; ++i)
    {
        const TestEncryptionKey& objTestKey = m_objTestEncryptionKeys.GetAt(i);

        ASSERT_TRUE(objEncryptionKey.SetValue(objTestKey.m_nMethod, objTestKey.m_strKey));
        EXPECT_EQ(objEncryptionKey.GetValue(), objTestKey.m_strSdpLine);
        EXPECT_EQ(objEncryptionKey.GetMethod(), objTestKey.m_nMethod);
        EXPECT_EQ(objEncryptionKey.GetKey(), objTestKey.m_strKey);
    }

    EXPECT_FALSE(
            objEncryptionKey.SetValue(SdpEncryptionKey::METHOD_INVALID, m_objTestKey.m_strKey));
    EXPECT_FALSE(objEncryptionKey.SetValue(SdpEncryptionKey::METHOD_MAX, m_objTestKey.m_strKey));

    for (IMS_UINT32 i = 0; i < nTestCount; ++i)
    {
        const TestEncryptionKey& objTestKey = m_objTestEncryptionKeys.GetAt(i);

        if (objTestKey.m_nMethod == SdpEncryptionKey::METHOD_PROMPT)
        {
            ASSERT_FALSE(objEncryptionKey.SetValue(objTestKey.m_nMethod, "test-key"));
        }
        else
        {
            ASSERT_FALSE(objEncryptionKey.SetValue(objTestKey.m_nMethod, AString::ConstNull()));
            ASSERT_FALSE(objEncryptionKey.SetValue(objTestKey.m_nMethod, AString::ConstEmpty()));
        }
    }
}

}  // namespace android
