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

#include "ImsVector.h"

#include "SdpAttribute.h"

namespace android
{

class TestAttribute
{
public:
    inline TestAttribute() :
            m_strSdpFullLine(AString::ConstNull()),
            m_strSdpLine(AString::ConstNull()),
            m_nAttrName(SdpAttribute::ATTRIBUTE_INVALID),
            m_strAttrName(AString::ConstNull()),
            m_strAttrValue(AString::ConstNull())
    {
    }
    inline TestAttribute(const AString& strSdpFullLine, const AString& strSdpLine,
            IMS_SINT32 nAttrName, const AString& strAttrName, const AString& strAttrValue) :
            m_strSdpFullLine(strSdpFullLine),
            m_strSdpLine(strSdpLine),
            m_nAttrName(nAttrName),
            m_strAttrName(strAttrName),
            m_strAttrValue(strAttrValue)
    {
    }
    inline TestAttribute(const TestAttribute& other) :
            m_strSdpFullLine(other.m_strSdpFullLine),
            m_strSdpLine(other.m_strSdpLine),
            m_nAttrName(other.m_nAttrName),
            m_strAttrName(other.m_strAttrName),
            m_strAttrValue(other.m_strAttrValue)
    {
    }
    inline ~TestAttribute() {}

    inline TestAttribute& operator=(const TestAttribute& other)
    {
        if (this != &other)
        {
            m_strSdpFullLine = other.m_strSdpFullLine;
            m_strSdpLine = other.m_strSdpLine;
            m_nAttrName = other.m_nAttrName;
            m_strAttrName = other.m_strAttrName;
            m_strAttrValue = other.m_strAttrValue;
        }

        return *this;
    }

public:
    AString m_strSdpFullLine;  // including "a="
    AString m_strSdpLine;      // excluding "a="
    IMS_SINT32 m_nAttrName;    // attribute-name as enum
    AString m_strAttrName;     // attribute-name as string
    AString m_strAttrValue;    // attribute-value
};

class SdpAttributeTest : public ::testing::Test
{
public:
    SdpAttributeTest();

protected:
    TestAttribute m_objTestAttr;
    ImsVector<TestAttribute> m_objNormalTestAttributes;
    ImsVector<TestAttribute> m_objAbnormalTestAttributes;
    ImsVector<TestAttribute> m_objNotEqualsTestAttributes1;
    ImsVector<TestAttribute> m_objNotEqualsTestAttributes2;
};

SdpAttributeTest::SdpAttributeTest()
{
    // clang-format off
    m_objTestAttr = TestAttribute(
            "a=rtpmap:100 AMR/8000/1\r\n", "rtpmap:100 AMR/8000/1",
            SdpAttribute::RTPMAP, "rtpmap", "100 AMR/8000/1");

    m_objNormalTestAttributes.Add(TestAttribute(
            "a=rtpmap:100 AMR/8000/1\r\n", "rtpmap:100 AMR/8000/1",
            SdpAttribute::RTPMAP, "rtpmap", "100 AMR/8000/1"));
    m_objNormalTestAttributes.Add(TestAttribute(
            "a=fmtp:105 octet-align=1;max-red=220\r\n", "fmtp:105 octet-align=1;max-red=220",
            SdpAttribute::FMTP, "fmtp", "105 octet-align=1;max-red=220"));
    m_objNormalTestAttributes.Add(TestAttribute(
            "a=sendrecv\r\n", "sendrecv",
            SdpAttribute::SENDRECV, "sendrecv", AString::ConstEmpty()));
    m_objNormalTestAttributes.Add(TestAttribute(
            "a=attrname:attrvalue\r\n", "attrname:attrvalue",
            SdpAttribute::ATTRIBUTE_OTHER, "attrname", "attrvalue"));
    m_objNormalTestAttributes.Add(TestAttribute(
            "a=attrname\r\n", "attrname",
            SdpAttribute::ATTRIBUTE_OTHER, "attrname", AString::ConstEmpty()));
    m_objNormalTestAttributes.Add(TestAttribute(
            "a=accept-types:message/cpim text/plain\r\n", "accept-types:message/cpim text/plain",
            SdpAttribute::ACCEPT_TYPES, "accept-types", "message/cpim text/plain"));

    m_objAbnormalTestAttributes.Add(TestAttribute(
            "a=attrname:attr\nvalue\r\n", "attrname:attr\nvalue",
            SdpAttribute::ATTRIBUTE_OTHER, "attrname", "attr\nvalue"));
    m_objAbnormalTestAttributes.Add(TestAttribute(
            "a=attr name\r\n", "attr name",
            SdpAttribute::ATTRIBUTE_OTHER, "attr name", AString::ConstEmpty()));
    m_objAbnormalTestAttributes.Add(TestAttribute(
            "a=mid:1 2\r\n", "mid:1 2",
            SdpAttribute::MID, "mid", "1 2"));
    m_objAbnormalTestAttributes.Add(TestAttribute(
            "a=curr:qos local\r sendrecv\r\n", "curr:qos local\r sendrecv",
            SdpAttribute::CURR, "curr", "qos local\r sendrecv"));

    m_objNotEqualsTestAttributes1.Add(TestAttribute(
            "a=rtpmap:100 AMR/8000/1\r\n", "rtpmap:100 AMR/8000/1",
            SdpAttribute::RTPMAP, "rtpmap", "100 AMR/8000/1"));
    m_objNotEqualsTestAttributes1.Add(TestAttribute(
            "a=fmtp:105 octet-align=1;max-red=220\r\n", "fmtp:105 octet-align=1;max-red=220",
            SdpAttribute::FMTP, "fmtp", "105 octet-align=1;max-red=220"));
    m_objNotEqualsTestAttributes1.Add(TestAttribute(
            "a=sendrecv\r\n", "sendrecv",
            SdpAttribute::SENDRECV, "sendrecv", AString::ConstEmpty()));
    m_objNotEqualsTestAttributes1.Add(TestAttribute(
            "a=sendrecv\r\n", "sendrecv",
            SdpAttribute::SENDRECV, "sendrecv", AString::ConstEmpty()));
    m_objNotEqualsTestAttributes1.Add(TestAttribute(
            "a=attrname:attrvalue\r\n", "attrname:attrvalue",
            SdpAttribute::ATTRIBUTE_OTHER, "attrname", "attrvalue"));
    m_objNotEqualsTestAttributes1.Add(TestAttribute(
            "a=attrname\r\n", "attrname",
            SdpAttribute::ATTRIBUTE_OTHER, "attrname", AString::ConstEmpty()));

    m_objNotEqualsTestAttributes2.Add(TestAttribute(
            "a=rtpmap:101 AMR-WB/16000/1\r\n", "rtpmap:101 AMR-WB/16000/1",
            SdpAttribute::RTPMAP, "rtpmap", "101 AMR-WB/16000/1"));
    m_objNotEqualsTestAttributes2.Add(TestAttribute(
            "a=rtpmap:100 AMR/8000/1\r\n", "rtpmap:100 AMR/8000/1",
            SdpAttribute::RTPMAP, "rtpmap", "100 AMR/8000/1"));
    m_objNotEqualsTestAttributes2.Add(TestAttribute(
            "a=fmtp:105 octet-align=1;max-red=220\r\n", "fmtp:105 octet-align=1;max-red=220",
            SdpAttribute::FMTP, "fmtp", "105 octet-align=1;max-red=220"));
    m_objNotEqualsTestAttributes2.Add(TestAttribute(
            "a=inactive\r\n", "inactive",
            SdpAttribute::INACTIVE, "inactive", AString::ConstEmpty()));
    m_objNotEqualsTestAttributes2.Add(TestAttribute(
            "a=attrname2:attrvalue\r\n", "attrname2:attrvalue",
            SdpAttribute::ATTRIBUTE_OTHER, "attrname2", "attrvalue"));
    m_objNotEqualsTestAttributes2.Add(TestAttribute(
            "a=attrname2\r\n", "attrname2",
            SdpAttribute::ATTRIBUTE_OTHER, "attrname", AString::ConstEmpty()));
    // clang-format on
}

TEST_F(SdpAttributeTest, Constructor)
{
    SdpAttribute objAttr;
    EXPECT_EQ(SdpAttribute::ATTRIBUTE_INVALID, objAttr.GetAttribute());
    EXPECT_EQ(objAttr.GetAttributeValue(), AString::ConstEmpty());
    EXPECT_EQ(objAttr.GetAttributeName(), AString::ConstNull());
    EXPECT_EQ(objAttr.GetValue(), AString::ConstNull());
}

TEST_F(SdpAttributeTest, CopyConstructor)
{
    SdpAttribute objAttr;
    ASSERT_TRUE(objAttr.SetValue(
            m_objTestAttr.m_nAttrName, m_objTestAttr.m_strAttrValue, m_objTestAttr.m_strAttrName));

    SdpAttribute objNewAttr(objAttr);
    EXPECT_TRUE(objNewAttr.Equals(&objAttr));
}

TEST_F(SdpAttributeTest, OperatorAssignment)
{
    SdpAttribute objAttr;
    ASSERT_TRUE(objAttr.SetValue(
            m_objTestAttr.m_nAttrName, m_objTestAttr.m_strAttrValue, m_objTestAttr.m_strAttrName));

    SdpAttribute objNewAttr;
    objNewAttr = objAttr;
    EXPECT_TRUE(objNewAttr.Equals(&objAttr));
}

TEST_F(SdpAttributeTest, Decode)
{
    SdpAttribute objAttr;
    IMS_UINT32 nTestCount = m_objNormalTestAttributes.GetSize();

    for (IMS_UINT32 i = 0; i < nTestCount; ++i)
    {
        const TestAttribute& objTestAttr = m_objNormalTestAttributes.GetAt(i);

        ASSERT_TRUE(objAttr.Decode(objTestAttr.m_strSdpLine));
        EXPECT_EQ(objAttr.GetAttribute(), objTestAttr.m_nAttrName);
        EXPECT_EQ(objAttr.GetAttributeValue(), objTestAttr.m_strAttrValue);
        EXPECT_EQ(objAttr.GetAttributeName(), objTestAttr.m_strAttrName);
        EXPECT_EQ(objAttr.GetValue(), objTestAttr.m_strSdpLine);
    }

    nTestCount = m_objAbnormalTestAttributes.GetSize();

    for (IMS_UINT32 i = 0; i < nTestCount; ++i)
    {
        const TestAttribute& objTestAttr = m_objAbnormalTestAttributes.GetAt(i);

        ASSERT_FALSE(objAttr.Decode(objTestAttr.m_strSdpLine));
    }

    EXPECT_FALSE(objAttr.Decode(AString::ConstNull()));
    EXPECT_FALSE(objAttr.Decode(AString::ConstEmpty()));
}

TEST_F(SdpAttributeTest, Encode)
{
    SdpAttribute objAttr;
    AString strEncoded = objAttr.Encode();
    EXPECT_EQ(strEncoded, AString::ConstNull());

    IMS_UINT32 nTestCount = m_objNormalTestAttributes.GetSize();

    for (IMS_UINT32 i = 0; i < nTestCount; ++i)
    {
        const TestAttribute& objTestAttr = m_objNormalTestAttributes.GetAt(i);

        ASSERT_TRUE(objAttr.SetValue(
                objTestAttr.m_nAttrName, objTestAttr.m_strAttrValue, objTestAttr.m_strAttrName));
        strEncoded = objAttr.Encode();
        EXPECT_EQ(strEncoded, objTestAttr.m_strSdpFullLine);
    }
}

TEST_F(SdpAttributeTest, Equals)
{
    SdpAttribute objAttr1;
    SdpAttribute objAttr2;
    IMS_UINT32 nTestCount = m_objNormalTestAttributes.GetSize();

    for (IMS_UINT32 i = 0; i < nTestCount; ++i)
    {
        const TestAttribute& objTestAttr = m_objNormalTestAttributes.GetAt(i);

        ASSERT_TRUE(objAttr1.Decode(objTestAttr.m_strSdpLine));
        ASSERT_TRUE(objAttr2.Decode(objTestAttr.m_strSdpLine));
        EXPECT_TRUE(objAttr1.Equals(&objAttr2));

        SdpAttribute objAttr3(objAttr1);
        SdpAttribute objAttr4 = objAttr2;
        EXPECT_TRUE(objAttr3.Equals(&objAttr4));
    }

    ASSERT_TRUE(objAttr1.Decode(m_objTestAttr.m_strSdpLine));
    ASSERT_TRUE(objAttr2.SetValue(
            m_objTestAttr.m_nAttrName, m_objTestAttr.m_strAttrValue, m_objTestAttr.m_strAttrName));
    EXPECT_TRUE(objAttr1.Equals(&objAttr2));

    ASSERT_EQ(m_objNotEqualsTestAttributes1.GetSize(), m_objNotEqualsTestAttributes2.GetSize());
    nTestCount = m_objNotEqualsTestAttributes1.GetSize();

    for (IMS_UINT32 i = 0; i < nTestCount; ++i)
    {
        const TestAttribute& objTestAttr1 = m_objNotEqualsTestAttributes1.GetAt(i);
        const TestAttribute& objTestAttr2 = m_objNotEqualsTestAttributes2.GetAt(i);

        ASSERT_TRUE(objAttr1.Decode(objTestAttr1.m_strSdpLine));
        ASSERT_TRUE(objAttr2.Decode(objTestAttr2.m_strSdpLine));
        EXPECT_FALSE(objAttr1.Equals(&objAttr2));

        SdpAttribute objAttr3(objAttr1);
        SdpAttribute objAttr4 = objAttr2;
        EXPECT_FALSE(objAttr3.Equals(&objAttr4));
    }
}

TEST_F(SdpAttributeTest, SetValue)
{
    SdpAttribute objAttr;
    IMS_UINT32 nTestCount = m_objNormalTestAttributes.GetSize();

    for (IMS_UINT32 i = 0; i < nTestCount; ++i)
    {
        const TestAttribute& objTestAttr = m_objNormalTestAttributes.GetAt(i);

        ASSERT_TRUE(objAttr.SetValue(
                objTestAttr.m_nAttrName, objTestAttr.m_strAttrValue, objTestAttr.m_strAttrName));
        EXPECT_EQ(objAttr.GetAttribute(), objTestAttr.m_nAttrName);
        EXPECT_EQ(objAttr.GetAttributeValue(), objTestAttr.m_strAttrValue);
        EXPECT_EQ(objAttr.GetAttributeName(), objTestAttr.m_strAttrName);
        EXPECT_EQ(objAttr.GetValue(), objTestAttr.m_strSdpLine);
    }

    nTestCount = m_objAbnormalTestAttributes.GetSize();

    for (IMS_UINT32 i = 0; i < nTestCount; ++i)
    {
        const TestAttribute& objTestAttr = m_objAbnormalTestAttributes.GetAt(i);

        ASSERT_FALSE(objAttr.SetValue(
                objTestAttr.m_nAttrName, objTestAttr.m_strAttrValue, objTestAttr.m_strAttrName));
    }
}

}  // namespace android
