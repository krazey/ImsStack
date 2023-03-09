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
#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "ByteArray.h"
#include "ImsNew.h"
#include "ImsStrLib.h"

namespace android
{

class ByteArrayTest : public ::testing::Test
{
protected:
    virtual void SetUp() override {}
    virtual void TearDown() override {}
};

TEST_F(ByteArrayTest, Constructor)
{
    ByteArray objBa;

    EXPECT_TRUE(objBa.IsNull());
}

TEST_F(ByteArrayTest, ConstructorWithByte)
{
    const IMS_BYTE byte = 5;
    ByteArray objBa(byte);

    EXPECT_EQ(objBa, byte);
}

TEST_F(ByteArrayTest, ConstructorWithCharPointer)
{
    const IMS_CHAR* pValue = "Hello";
    ByteArray objBa(pValue);

    EXPECT_EQ(objBa.GetLength(), IMS_StrLen(pValue));
    EXPECT_STREQ(objBa.ToString().GetStr(), pValue);
}

TEST_F(ByteArrayTest, ConstructorWithAString)
{
    const AString strValue = "Hello";
    ByteArray objBa(strValue);

    EXPECT_EQ(objBa.GetLength(), strValue.GetLength());
    EXPECT_EQ(objBa.ToString(), strValue);
}

TEST_F(ByteArrayTest, ConstructorWithBytePointer)
{
    const IMS_BYTE baValue[] = {'H', 'e', 'l', 'l', 'o'};
    IMS_SINT32 nValueLength = sizeof(baValue);
    ByteArray objBa(baValue, nValueLength);

    EXPECT_EQ(objBa.GetLength(), nValueLength);
    EXPECT_EQ(objBa.ToString(), AString("Hello", nValueLength));
}

TEST_F(ByteArrayTest, CopyConstructor)
{
    const ByteArray objBa(AString("Hello"));
    // This is a test to verify the copy constructor.
    // NOLINTNEXTLINE(performance-unnecessary-copy-initialization)
    ByteArray objCopiedBa(objBa);

    EXPECT_EQ(objBa, objCopiedBa);
}

TEST_F(ByteArrayTest, AssignmentOperator)
{
    const ByteArray objValue(AString("Hello"));
    ByteArray objBa;

    objBa = objValue;

    EXPECT_EQ(objBa, objValue);

    IMS_BYTE byte = '!';
    objBa = byte;

    EXPECT_EQ(objBa, byte);
}

TEST_F(ByteArrayTest, CompoundAssignmentOperator)
{
    const ByteArray objExpectedBa(AString("Hello!"));
    const ByteArray objValue(AString("Hello"));
    ByteArray objBa;

    objBa += objValue;

    EXPECT_EQ(objBa, objValue);

    IMS_BYTE byte = '!';
    objBa += byte;

    EXPECT_EQ(objBa, objExpectedBa);
}

TEST_F(ByteArrayTest, ArrayIndexOperator)
{
    const IMS_BYTE baValue[] = {'H', 'e', 'l', 'l', 'o', '!'};
    ByteArray objBa(baValue, sizeof(baValue));

    EXPECT_EQ(objBa.GetLength(), sizeof(baValue));

    for (IMS_SINT32 i = 0; i < objBa.GetLength(); ++i)
    {
        EXPECT_EQ(baValue[i], static_cast<IMS_BYTE>(objBa[i]));
    }

    for (IMS_UINT32 i = 0; i < objBa.GetLength(); ++i)
    {
        EXPECT_EQ(baValue[i], static_cast<IMS_BYTE>(objBa[i]));
    }

    const ByteArray& objConstBa = objBa;

    for (IMS_SINT32 i = 0; i < objConstBa.GetLength(); ++i)
    {
        EXPECT_EQ(baValue[i], objConstBa[i]);
    }

    for (IMS_UINT32 i = 0; i < objConstBa.GetLength(); ++i)
    {
        EXPECT_EQ(baValue[i], objConstBa[i]);
    }
}

TEST_F(ByteArrayTest, EqualOperator)
{
    const IMS_BYTE baValue1[] = {'1'};
    const IMS_BYTE baValue2[] = {'2'};
    const ByteArray objValue(AString("Hello"));
    // This is a test to verify the copy constructor.
    // NOLINTNEXTLINE(performance-unnecessary-copy-initialization)
    const ByteArray objBa(objValue);
    const ByteArray objBa1(baValue1, sizeof(baValue1));

    EXPECT_EQ(objBa, objBa);
    EXPECT_EQ(objBa, objValue);
    EXPECT_NE(objBa, ByteArray::ConstNull());
    EXPECT_NE(objBa, objBa1);
    EXPECT_NE(objBa, baValue1[0]);

    EXPECT_EQ(objBa1, baValue1[0]);
    EXPECT_NE(objBa1, baValue2[0]);
}

TEST_F(ByteArrayTest, PlusOperator)
{
    const IMS_BYTE baLastValue[] = {'!'};
    const IMS_BYTE baFirstValue[] = {'H'};
    const ByteArray objExpectedBa(AString("Hello!"));
    const ByteArray objValue1(AString("Hello"));
    const ByteArray objValue2(AString("!"));
    const ByteArray objValue3(AString("ello!"));
    ByteArray objBa;

    objBa = objValue1 + objValue2;

    EXPECT_EQ(objBa, objExpectedBa);

    objBa = objValue1 + baLastValue[0];

    EXPECT_EQ(objBa, objExpectedBa);

    objBa = baFirstValue[0] + objValue3;

    EXPECT_EQ(objBa, objExpectedBa);
}

TEST_F(ByteArrayTest, Append)
{
    const IMS_BYTE baValue[] = {'H', 'e', 'l', 'l', 'o'};
    const IMS_BYTE baAppendedValue[] = {'w', 'o', 'r', 'l', 'd'};
    const ByteArray objExpectedBa(AString("Hello,world~!"));
    ByteArray objBa(baValue, sizeof(baValue));

    EXPECT_EQ(objBa.GetLength(), sizeof(baValue));

    objBa.Append(',');
    objBa.Append(baAppendedValue, sizeof(baAppendedValue));
    ByteArray objAppendedValue(AString("~!"));
    objBa.Append(objAppendedValue);

    EXPECT_EQ(objBa, objExpectedBa);
}

TEST_F(ByteArrayTest, Erase)
{
    const IMS_BYTE baValue[] = {'H', 'e', 'l', 'l', 'o'};
    IMS_SINT32 nValueLength = sizeof(baValue);
    ByteArray objBa(baValue, nValueLength);

    EXPECT_EQ(objBa.GetLength(), nValueLength);

    objBa.Erase(4, -1);

    EXPECT_EQ(objBa.GetLength(), nValueLength);

    objBa.Erase(5, 1);

    EXPECT_EQ(objBa.GetLength(), nValueLength);

    objBa.Erase(-1, 1);

    EXPECT_EQ(objBa.GetLength(), nValueLength);

    objBa.Erase(4, 1);

    EXPECT_EQ(objBa.GetLength(), nValueLength - 1);

    objBa.Erase(1, 2);

    EXPECT_EQ(objBa.GetLength(), nValueLength - 3);

    const ByteArray& objConstBa = objBa;
    EXPECT_EQ(objConstBa[0], 'H');
    EXPECT_EQ(objConstBa[1], 'l');
}

TEST_F(ByteArrayTest, Prepend)
{
    const IMS_BYTE baValue[] = {'~', '!'};
    const IMS_BYTE baPrependedValue[] = {'w', 'o', 'r', 'l', 'd'};
    const ByteArray objExpectedBa(AString("Hello,world~!"));
    ByteArray objBa(baValue, sizeof(baValue));

    EXPECT_EQ(objBa.GetLength(), sizeof(baValue));

    objBa.Prepend(baPrependedValue, sizeof(baPrependedValue));
    objBa.Prepend(',');
    ByteArray objPrependedValue(AString("Hello"));
    objBa.Prepend(objPrependedValue);

    EXPECT_EQ(objBa, objExpectedBa);
}

TEST_F(ByteArrayTest, Attach)
{
    const IMS_BYTE baValue[] = {'H', 'e', 'l', 'l', 'o', '!'};
    IMS_SINT32 nValueLength = sizeof(baValue);
    const ByteArray objExpectedBa(baValue, nValueLength);
    ByteArray objBa;

    objBa.Attach(baValue, nValueLength);

    EXPECT_EQ(objBa, objExpectedBa);
}

TEST_F(ByteArrayTest, AttachAndAssignment)
{
    const IMS_BYTE baValue[] = {'H', 'e', 'l', 'l', 'o', '!'};
    IMS_SINT32 nValueLength = sizeof(baValue);
    const ByteArray objExpectedBa(AString("Hello!"));
    ByteArray objBa;

    objBa.Attach(baValue, nValueLength);

    EXPECT_EQ(objBa, objExpectedBa);

    ByteArray objNewBa;
    objNewBa = objBa;

    EXPECT_EQ(objNewBa, objExpectedBa);
}

TEST_F(ByteArrayTest, GetSubData)
{
    const IMS_BYTE baValue[] = {'H', 'e', 'l', 'l', 'o', ',', 'w', 'o', 'r', 'l', 'd', '~', '!'};
    IMS_SINT32 nValueLength = sizeof(baValue);
    ByteArray objBa(baValue, nValueLength);

    EXPECT_EQ(objBa.GetLength(), nValueLength);

    ByteArray objSubBa = objBa.GetSubData(nValueLength + 1);

    EXPECT_TRUE(objSubBa.IsNull());

    objSubBa = objBa.GetSubData(-1);

    EXPECT_EQ(objBa, objSubBa);

    objSubBa = objBa.GetSubData(6, 5);
    const ByteArray objConstSubBa(AString("world"));

    EXPECT_EQ(objSubBa, objConstSubBa);
}

TEST_F(ByteArrayTest, Equals)
{
    const IMS_BYTE baValue1[] = {'1'};
    const IMS_BYTE baValue2[] = {'2'};
    const ByteArray objValue(AString("Hello"));
    // This is a test to verify the copy constructor.
    // NOLINTNEXTLINE(performance-unnecessary-copy-initialization)
    const ByteArray objBa(objValue);
    const ByteArray objBa1(baValue1, sizeof(baValue1));

    EXPECT_TRUE(objBa.Equals(objBa));
    EXPECT_TRUE(objBa.Equals(objValue));
    EXPECT_FALSE(objBa.Equals(ByteArray::ConstNull()));
    EXPECT_FALSE(objBa.Equals(objBa1));
    EXPECT_FALSE(objBa.Equals(baValue1[0]));

    EXPECT_TRUE(objBa1.Equals(baValue1[0]));
    EXPECT_FALSE(objBa1.Equals(baValue2[0]));
}

TEST_F(ByteArrayTest, ToString)
{
    const AString strValue("Hello");
    ByteArray objBa(strValue);

    EXPECT_EQ(objBa.GetLength(), strValue.GetLength());
    EXPECT_EQ(objBa.ToString(), strValue);
}

TEST_F(ByteArrayTest, ToHexString)
{
    const AString strValue("123");
    const AString strHexValue("313233");
    ByteArray objBa(strValue);

    EXPECT_EQ(objBa.GetLength(), strValue.GetLength());
    EXPECT_EQ(objBa.ToHexString(), strHexValue);
}

TEST_F(ByteArrayTest, ByteRef)
{
    const IMS_SINT32 nIndex0 = 0;
    const IMS_UINT32 nIndex4 = 4;
    ByteArray objBa(AString("12345"));

    objBa[nIndex0] = 0x30;

    EXPECT_EQ(objBa, ByteArray(AString("02345")));

    ByteArray::ByteRef objByteRef = objBa[nIndex4];
    objByteRef = 0x36;

    EXPECT_EQ(objBa, ByteArray(AString("02346")));

    ByteArray::ByteRef objByteRef2(objBa, 2);

    EXPECT_EQ(static_cast<IMS_BYTE>(objByteRef2), static_cast<const ByteArray&>(objBa)[2]);

    ByteArray::ByteRef objByteRef3(objBa, objBa.GetLength() + 1);

    EXPECT_EQ(static_cast<IMS_BYTE>(objByteRef3), 0);
}

}  // namespace android
