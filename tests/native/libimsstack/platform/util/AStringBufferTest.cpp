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

#include "AStringBuffer.h"

namespace android
{

class AStringBufferTest : public ::testing::Test
{
protected:
    virtual void SetUp() override {}
    virtual void TearDown() override {}
};

TEST_F(AStringBufferTest, Constructor)
{
    AStringBuffer objAStringBuffer;

    EXPECT_TRUE(objAStringBuffer.GetString().IsNull());
}

TEST_F(AStringBufferTest, ConstructorWithCapacity)
{
    AStringBuffer objAStringBuffer(10);

    EXPECT_EQ(objAStringBuffer.GetCapacity(), 10);
    EXPECT_EQ(objAStringBuffer.GetLength(), 0);
}

TEST_F(AStringBufferTest, CopyConstructor)
{
    AStringBuffer objAStringBuffer(10);
    AStringBuffer objCopiedAStringBufferTest(objAStringBuffer);

    EXPECT_EQ(objCopiedAStringBufferTest.GetCapacity(), 10);
    EXPECT_EQ(objCopiedAStringBufferTest.GetLength(), 0);

    objAStringBuffer.SetCapacity(40);

    EXPECT_EQ(objAStringBuffer.GetCapacity(), 40);
    EXPECT_EQ(objCopiedAStringBufferTest.GetCapacity(), 10);
    EXPECT_TRUE(objAStringBuffer.IsEmpty());
}

TEST_F(AStringBufferTest, AssignmentOperatorForAStringBuffer)
{
    const AString strValue("Hello");
    AStringBuffer objAStringBuffer(strValue);

    AStringBuffer objCopiedAStringBufferTest;

    objCopiedAStringBufferTest = objAStringBuffer;

    EXPECT_EQ(objCopiedAStringBufferTest.GetString(), strValue);
}

TEST_F(AStringBufferTest, AssignmentOperatorForChar)
{
    AStringBuffer objAStringBuffer;
    objAStringBuffer = '~';

    EXPECT_STREQ(objAStringBuffer.GetCharString(), "~");
}

TEST_F(AStringBufferTest, AssignmentOperatorForCharPointer)
{
    const IMS_CHAR* pValue = "1234";
    AStringBuffer objAStringBuffer;
    objAStringBuffer = pValue;

    EXPECT_STREQ(objAStringBuffer.GetCharString(), pValue);
}

TEST_F(AStringBufferTest, AssignmentOperatorForAString)
{
    AString strValue("1234");
    AStringBuffer objAStringBuffer;
    objAStringBuffer = strValue;

    EXPECT_EQ(objAStringBuffer.GetString(), strValue);
}

TEST_F(AStringBufferTest, AdditionAssignmentForAStringBuffer)
{
    AStringBuffer objAStringBuffer1(10);
    objAStringBuffer1 = "Hello";

    AStringBuffer objStringBufferTest2;
    objStringBufferTest2 = " World";

    objAStringBuffer1 += objStringBufferTest2;

    EXPECT_EQ(objAStringBuffer1.GetString(), AString("Hello World"));
    EXPECT_EQ(objAStringBuffer1.GetLength(), 11);
}

TEST_F(AStringBufferTest, AdditionAssignmentForChar)
{
    AStringBuffer objAStringBuffer1(10);
    objAStringBuffer1 = "Hello";

    objAStringBuffer1 += ' ';
    objAStringBuffer1 += 'W';
    objAStringBuffer1 += 'o';
    objAStringBuffer1 += 'r';
    objAStringBuffer1 += 'l';
    objAStringBuffer1 += 'd';

    EXPECT_EQ(objAStringBuffer1.GetString(), AString("Hello World"));
}

TEST_F(AStringBufferTest, AdditionAssignmentForCharPointer)
{
    const IMS_CHAR* pValue = " World";
    AStringBuffer objAStringBuffer;
    objAStringBuffer = "Hello";

    objAStringBuffer += pValue;

    EXPECT_STREQ(objAStringBuffer.GetCharString(), "Hello World");
}

TEST_F(AStringBufferTest, AdditionAssignmentForAString)
{
    AString strValue(" World");
    AStringBuffer objAStringBuffer;
    objAStringBuffer = "Hello";

    objAStringBuffer += strValue;

    EXPECT_EQ(objAStringBuffer.GetString(), AString("Hello World"));
}

TEST_F(AStringBufferTest, AdditionAssignmentForSignedInt16)
{
    AStringBuffer objAStringBuffer;
    objAStringBuffer = "Minus One : ";

    IMS_SINT16 nSignedInt16 = -1;

    objAStringBuffer += nSignedInt16;

    EXPECT_EQ(objAStringBuffer.GetString(), AString("Minus One : -1"));
}

TEST_F(AStringBufferTest, AdditionAssignmentForUnsignedInt16)
{
    AStringBuffer objAStringBuffer;
    objAStringBuffer = "Twenty One : ";

    IMS_UINT16 nUnsignedInt16 = 21;

    objAStringBuffer += nUnsignedInt16;

    EXPECT_EQ(objAStringBuffer.GetString(), AString("Twenty One : 21"));
}

TEST_F(AStringBufferTest, AdditionAssignmentForSignedInt32)
{
    AStringBuffer objAStringBuffer;
    objAStringBuffer = "Minus One Million : ";

    IMS_SINT32 nSignedInt32 = -1000000;

    objAStringBuffer += nSignedInt32;

    EXPECT_EQ(objAStringBuffer.GetString(), AString("Minus One Million : -1000000"));
}

TEST_F(AStringBufferTest, AdditionAssignmentForUnsignedInt32)
{
    AStringBuffer objAStringBuffer;
    objAStringBuffer = "Twenty One Millions : ";

    IMS_UINT32 nUnsignedInt32 = 21000000;

    objAStringBuffer += nUnsignedInt32;

    EXPECT_EQ(objAStringBuffer.GetString(), AString("Twenty One Millions : 21000000"));
}

TEST_F(AStringBufferTest, AdditionAssignmentForSignedLong)
{
    AStringBuffer objAStringBuffer;
    objAStringBuffer = "Minus One Billion : ";

    IMS_SLONG nSignedLong = -1000000000;

    objAStringBuffer += nSignedLong;

    EXPECT_EQ(objAStringBuffer.GetString(), AString("Minus One Billion : -1000000000"));
}

TEST_F(AStringBufferTest, AdditionAssignmentForUnsignedLong)
{
    AStringBuffer objAStringBuffer;
    objAStringBuffer = "Three Billions : ";

    IMS_ULONG nUnsignedInt16 = 3000000000;

    objAStringBuffer += nUnsignedInt16;

    EXPECT_EQ(objAStringBuffer.GetString(), AString("Three Billions : 3000000000"));
}

TEST_F(AStringBufferTest, ArraySubscriptForReturnChar)
{
    AStringBuffer objAStringBuffer;
    objAStringBuffer = "Hello";

    IMS_CHAR cCharValue = objAStringBuffer[2];

    EXPECT_EQ(cCharValue, 'l');
}

TEST_F(AStringBufferTest, ArraySubscript_SignedIndexForReturnChar)
{
    AStringBuffer objAStringBuffer;
    objAStringBuffer = "Hello";

    IMS_SINT32 nIndex = 2;

    const IMS_CHAR cCharValue = objAStringBuffer[nIndex];

    EXPECT_EQ(cCharValue, 'l');
}

TEST_F(AStringBufferTest, ArraySubscriptForSignedIndexAndReturnAStringCharRef)
{
    AStringBuffer objAStringBuffer;
    objAStringBuffer = "Hello";

    IMS_SINT32 nIndex = 2;

    AString::CharRef objCharRef = objAStringBuffer[nIndex];

    EXPECT_EQ(objCharRef, 'l');
}

TEST_F(AStringBufferTest, ArraySubscriptForUnsignedIndexAndReturnReturnChar)
{
    AStringBuffer objAStringBuffer;
    objAStringBuffer = "Hello";

    IMS_UINT32 nIndex = 2;

    const IMS_CHAR cCharValue = objAStringBuffer[nIndex];

    EXPECT_EQ(cCharValue, 'l');
}

TEST_F(AStringBufferTest, ArraySubscriptForUnsignedIndexAndReturnAStringCharRef)
{
    AStringBuffer objAStringBuffer;
    objAStringBuffer = "Hello";

    IMS_UINT32 nIndex = 2;

    AString::CharRef objCharRef = objAStringBuffer[nIndex];

    EXPECT_EQ(objCharRef, 'l');
}

TEST_F(AStringBufferTest, GetCharStringConstScope)
{
    const AString strValue("Hello World");
    const AStringBuffer objAStringBuffer(strValue);

    EXPECT_STREQ(objAStringBuffer.GetCharString(), strValue.GetStr());
}

TEST_F(AStringBufferTest, GetStringConstScope)
{
    const AString strValue("Hello World");
    const AStringBuffer objAStringBuffer(strValue);

    EXPECT_EQ(objAStringBuffer.GetString(), strValue);
}

TEST_F(AStringBufferTest, Chop)
{
    AStringBuffer objAStringBuffer(AString("Hello World"));

    objAStringBuffer.Chop(6);

    EXPECT_STREQ(objAStringBuffer.GetCharString(), "Hello");

    objAStringBuffer.Chop(20);

    EXPECT_STREQ(objAStringBuffer.GetCharString(), "");
}

TEST_F(AStringBufferTest, Truncate)
{
    AStringBuffer objAStringBuffer(AString("Hello World"));

    objAStringBuffer.Truncate(6);

    EXPECT_STREQ(objAStringBuffer.GetCharString(), "Hello ");
}

TEST_F(AStringBufferTest, Erase)
{
    AStringBuffer objAStringBuffer(AString("Hello World"));

    objAStringBuffer.Erase(5, 1);

    EXPECT_STREQ(objAStringBuffer.GetCharString(), "HelloWorld");
}

TEST_F(AStringBufferTest, FillChar)
{
    AStringBuffer objAStringBuffer(10);

    objAStringBuffer.Fill('G', 5);

    EXPECT_STREQ(objAStringBuffer.GetCharString(), "GGGGG");
}

TEST_F(AStringBufferTest, InsertChar)
{
    AStringBuffer objAStringBuffer(AString("Helo"));

    objAStringBuffer.Insert(2, 'l');

    EXPECT_STREQ(objAStringBuffer.GetCharString(), "Hello");
}

TEST_F(AStringBufferTest, InsertCharPointer)
{
    AStringBuffer objAStringBuffer(AString("Hello Program"));

    const IMS_CHAR* pValue = " World";

    objAStringBuffer.Insert(5, pValue);

    EXPECT_STREQ(objAStringBuffer.GetCharString(), "Hello World Program");
}

TEST_F(AStringBufferTest, InsertAString)
{
    AStringBuffer objAStringBuffer(AString("Hello Program"));

    AString strValue(" World");

    objAStringBuffer.Insert(5, strValue);

    EXPECT_STREQ(objAStringBuffer.GetCharString(), "Hello World Program");
}

TEST_F(AStringBufferTest, InsertSignedInt16)
{
    AStringBuffer objAStringBuffer(AString("Converting and inserting signed int16  in to string"));

    IMS_SINT16 nValue = -222;

    objAStringBuffer.Insert(38, nValue);

    EXPECT_STREQ(objAStringBuffer.GetCharString(),
            "Converting and inserting signed int16 -222 in to string");
}

TEST_F(AStringBufferTest, InsertUnsignedInt16)
{
    AStringBuffer objAStringBuffer(
            AString("Converting and inserting unsigned int16  in to string"));

    IMS_UINT16 nValue = 222;

    objAStringBuffer.Insert(40, nValue);

    EXPECT_STREQ(objAStringBuffer.GetCharString(),
            "Converting and inserting unsigned int16 222 in to string");
}

TEST_F(AStringBufferTest, InsertSignedInt32)
{
    AStringBuffer objAStringBuffer(AString("Converting and inserting signed int32  in to string"));

    IMS_SINT32 nValue = -222;

    objAStringBuffer.Insert(38, nValue);

    EXPECT_STREQ(objAStringBuffer.GetCharString(),
            "Converting and inserting signed int32 -222 in to string");
}

TEST_F(AStringBufferTest, InsertUnsignedInt32)
{
    AStringBuffer objAStringBuffer(
            AString("Converting and inserting unsigned int32  in to string"));

    IMS_UINT32 nValue = 222;

    objAStringBuffer.Insert(40, nValue);

    EXPECT_STREQ(objAStringBuffer.GetCharString(),
            "Converting and inserting unsigned int32 222 in to string");
}

TEST_F(AStringBufferTest, InsertSignedLong)
{
    AStringBuffer objAStringBuffer(AString("Converting and inserting signed long  in to string"));

    IMS_SLONG nValue = -222;

    objAStringBuffer.Insert(37, nValue);

    EXPECT_STREQ(objAStringBuffer.GetCharString(),
            "Converting and inserting signed long -222 in to string");
}

TEST_F(AStringBufferTest, InsertUnsignedLong)
{
    AStringBuffer objAStringBuffer(AString("Converting and inserting unsigned long  in to string"));

    IMS_ULONG nValue = 222;

    objAStringBuffer.Insert(39, nValue);

    EXPECT_STREQ(objAStringBuffer.GetCharString(),
            "Converting and inserting unsigned long 222 in to string");
}

TEST_F(AStringBufferTest, PrependChar)
{
    AStringBuffer objAStringBuffer(AString("ello"));

    objAStringBuffer.Prepend('H');

    EXPECT_STREQ(objAStringBuffer.GetCharString(), "Hello");
}

TEST_F(AStringBufferTest, PrependCharPointer)
{
    AStringBuffer objAStringBuffer(AString("World Program"));

    const IMS_CHAR* pValue = "Hello ";

    objAStringBuffer.Prepend(pValue);

    EXPECT_STREQ(objAStringBuffer.GetCharString(), "Hello World Program");
}

TEST_F(AStringBufferTest, PrependAString)
{
    AStringBuffer objAStringBuffer(AString("World Program"));

    AString strValue("Hello ");

    objAStringBuffer.Prepend(strValue);

    EXPECT_STREQ(objAStringBuffer.GetCharString(), "Hello World Program");
}

TEST_F(AStringBufferTest, ReplaceCharPointer)
{
    AStringBuffer objAStringBuffer(AString("Hello World, Good Morning!!"));

    const IMS_CHAR* pValue = "Google";

    objAStringBuffer.Replace(6, 5, pValue);

    EXPECT_STREQ(objAStringBuffer.GetCharString(), "Hello Google, Good Morning!!");
}

TEST_F(AStringBufferTest, ReplaceAString)
{
    AStringBuffer objAStringBuffer(AString("Hello World, Good Morning!!"));

    AString strValue("Google");

    objAStringBuffer.Replace(6, 5, strValue);

    EXPECT_STREQ(objAStringBuffer.GetCharString(), "Hello Google, Good Morning!!");
}

TEST_F(AStringBufferTest, ReplaceCharWithCharPointerString)
{
    AStringBuffer objAStringBuffer(AString("hello G, hi G, hello G"));

    const IMS_CHAR* pValue = "Google";

    objAStringBuffer.Replace('G', pValue);

    EXPECT_STREQ(objAStringBuffer.GetCharString(), "hello Google, hi Google, hello Google");
}

TEST_F(AStringBufferTest, ReplaceCharWithAStringString)
{
    AStringBuffer objAStringBuffer(AString("hello G, hi G, hello G"));

    AString strValue("Google");

    objAStringBuffer.Replace('G', strValue);

    EXPECT_STREQ(objAStringBuffer.GetCharString(), "hello Google, hi Google, hello Google");
}

TEST_F(AStringBufferTest, ReplaceCharPointerStringWithCharPointerString)
{
    AStringBuffer objAStringBuffer(AString("hello Google, hi Google, hello Google"));

    const IMS_CHAR* pOldValue = "Google";
    const IMS_CHAR* pNewValue = "World";

    objAStringBuffer.Replace(pOldValue, pNewValue);

    EXPECT_STREQ(objAStringBuffer.GetCharString(), "hello World, hi World, hello World");
}

TEST_F(AStringBufferTest, ReplaceCharPointerStringWithAStringString)
{
    AStringBuffer objAStringBuffer(AString("hello Google, hi Google, hello Google"));

    const IMS_CHAR* pOldValue = "Google";
    AString strNewValue("World");

    objAStringBuffer.Replace(pOldValue, strNewValue);

    EXPECT_STREQ(objAStringBuffer.GetCharString(), "hello World, hi World, hello World");
}

TEST_F(AStringBufferTest, ReplaceAStringStringWithCharPointerString)
{
    AStringBuffer objAStringBuffer(AString("hello Google, hi Google, hello Google"));

    AString strOldValue("Google");
    const IMS_CHAR* pNewValue = "World";

    objAStringBuffer.Replace(strOldValue, pNewValue);

    EXPECT_STREQ(objAStringBuffer.GetCharString(), "hello World, hi World, hello World");
}

TEST_F(AStringBufferTest, ReplaceAStringStringWithAStringString)
{
    AStringBuffer objAStringBuffer(AString("hello world, hi world, hello world"));

    AString strOldValue("world");
    AString strNewValue("WORLD");

    objAStringBuffer.Replace(strOldValue, strNewValue);

    EXPECT_STREQ(objAStringBuffer.GetCharString(), "hello WORLD, hi WORLD, hello WORLD");
}

TEST_F(AStringBufferTest, AdditionOfAStringBufferAndAStringBuffer)
{
    AStringBuffer objAStringBuffer1(AString("hello "));
    AStringBuffer objAStringBuffer2(AString("world"));

    const AStringBuffer objCombinedStringBuffer = objAStringBuffer1 + objAStringBuffer2;

    EXPECT_STREQ(objCombinedStringBuffer.GetCharString(), "hello world");
}

TEST_F(AStringBufferTest, AdditionOfAStringBufferAndCharPointer)
{
    AStringBuffer objAStringBuffer1(AString("hello "));
    const char* pBuffer2 = "world";

    const AStringBuffer objCombinedStringBuffer = objAStringBuffer1 + pBuffer2;

    EXPECT_STREQ(objCombinedStringBuffer.GetCharString(), "hello world");
}

TEST_F(AStringBufferTest, AdditionOfAStringBufferAndChar)
{
    AStringBuffer objAStringBuffer1(AString("hello "));

    const AStringBuffer objCombinedStringBuffer = objAStringBuffer1 + 'G';

    EXPECT_STREQ(objCombinedStringBuffer.GetCharString(), "hello G");
}

TEST_F(AStringBufferTest, AdditionOfCharPointerAndAStringBuffer)
{
    const char* pBuffer1 = "hello ";
    AStringBuffer objAStringBuffer2(AString("world"));

    const AStringBuffer objCombinedStringBuffer = pBuffer1 + objAStringBuffer2;

    EXPECT_STREQ(objCombinedStringBuffer.GetCharString(), "hello world");
}

TEST_F(AStringBufferTest, AdditionOfCharAndAStringBuffer)
{
    AStringBuffer objAStringBuffer(AString("ox"));

    const AStringBuffer objCombinedStringBuffer = 'f' + objAStringBuffer;

    EXPECT_STREQ(objCombinedStringBuffer.GetCharString(), "fox");
}

}  // namespace android