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
#include "ImsList.h"

namespace android
{

class ImsListTest : public ::testing::Test
{
protected:
    virtual void SetUp() override
    {
        for (IMS_SINT32 i = 0; i < 10; ++i)
        {
            // 1 ~ 10
            m_objList.Append(i + 1);
        }
    }

    virtual void TearDown() override
    {
        m_objList.Clear();
        EXPECT_TRUE(m_objList.IsEmpty());
    }

protected:
    ImsList<IMS_SINT32> m_objList;
};

TEST_F(ImsListTest, Constructor)
{
    ImsList<IMS_SINT32> objList;
    EXPECT_TRUE(objList.IsEmpty());
    EXPECT_EQ(objList.GetSize(), 0);
}

TEST_F(ImsListTest, CopyConstructor)
{
    ImsList<IMS_SINT32> objNewList(m_objList);
    EXPECT_EQ(objNewList.GetSize(), m_objList.GetSize());
    EXPECT_EQ(objNewList, m_objList);
}

TEST_F(ImsListTest, OperatorAssignment)
{
    ImsList<IMS_SINT32> objNewList;
    objNewList = m_objList;
    EXPECT_EQ(objNewList.GetSize(), m_objList.GetSize());
    EXPECT_EQ(objNewList, m_objList);
}

TEST_F(ImsListTest, Contains)
{
    EXPECT_TRUE(m_objList.Contains(1));
    EXPECT_TRUE(m_objList.Contains(10));
    EXPECT_FALSE(m_objList.Contains(11));

    ImsList<AString> objList;
    AString strDog("dog");
    AString strCat("cat");
    AString strLion("lion");
    objList.Append(strDog);
    objList.Append(strCat);

    EXPECT_TRUE(objList.Contains(strDog));
    EXPECT_TRUE(objList.Contains(strCat));
    EXPECT_FALSE(objList.Contains(strLion));

    ImsList<AString*> objPointerList;
    objPointerList.Append(&strDog);
    objPointerList.Append(&strCat);

    EXPECT_TRUE(objPointerList.Contains(&strDog));
    EXPECT_TRUE(objPointerList.Contains(&strCat));
    EXPECT_FALSE(objPointerList.Contains(&strLion));
}

TEST_F(ImsListTest, Equals)
{
    ImsList<IMS_SINT32> objNewList(m_objList);
    EXPECT_TRUE(objNewList.Equals(m_objList));
    EXPECT_TRUE(objNewList == m_objList);

    objNewList.SetAt(100, 3);
    EXPECT_FALSE(objNewList.Equals(m_objList));
    EXPECT_FALSE(objNewList == m_objList);

    objNewList.RemoveElementsAt(7, 3);
    EXPECT_FALSE(objNewList.Equals(m_objList));
    EXPECT_FALSE(objNewList == m_objList);

    objNewList.Clear();
    EXPECT_FALSE(objNewList.Equals(m_objList));
    EXPECT_FALSE(objNewList == m_objList);
}

TEST_F(ImsListTest, GetAt)
{
    IMS_SINT32 nTestIndex = 3;

    const ImsList<IMS_SINT32>& objList = m_objList;
    EXPECT_EQ(objList.GetAt(nTestIndex), m_objList.GetValueAt(nTestIndex));

    m_objList.GetAt(nTestIndex) = 200;
    EXPECT_EQ(objList.GetAt(nTestIndex), 200);
}

TEST_F(ImsListTest, InsertListAt)
{
    ImsList<IMS_SINT32> objList;
    ASSERT_TRUE(objList.Append(11));
    ASSERT_TRUE(objList.Append(12));
    ASSERT_TRUE(objList.Append(13));
    ASSERT_TRUE(objList.Append(14));
    ASSERT_TRUE(objList.Append(15));

    ASSERT_TRUE(m_objList.InsertListAt(objList, 0));
    ASSERT_TRUE(m_objList.InsertListAt(objList, 10));
    ASSERT_TRUE(m_objList.AppendList(objList));

    // 11 ~ 15, 1 ~ 5, 11 ~ 15, 6 ~ 10, 11 ~ 15
    const IMS_SINT32 nMergedCount = 25;
    // clang-format off
    const IMS_SINT32 nTestDatas[nMergedCount] = {
        11, 12, 13, 14, 15, 1, 2, 3, 4, 5, 11, 12, 13, 14, 15, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15
    };
    // clang-format on

    ASSERT_EQ(m_objList.GetSize(), nMergedCount);

    for (IMS_SINT32 i = 0; i < nMergedCount; ++i)
    {
        EXPECT_EQ(m_objList.GetAt(i), nTestDatas[i]);
    }
}

TEST_F(ImsListTest, Append)
{
    IMS_UINT32 nSize = m_objList.GetSize();
    ASSERT_TRUE(m_objList.Append(100));
    EXPECT_EQ(m_objList.GetSize(), nSize + 1);
    EXPECT_EQ(m_objList.GetAt(nSize), 100);
}

TEST_F(ImsListTest, Prepend)
{
    IMS_UINT32 nSize = m_objList.GetSize();
    ASSERT_TRUE(m_objList.Prepend(100));
    EXPECT_EQ(m_objList.GetSize(), nSize + 1);
    EXPECT_EQ(m_objList.GetAt(0), 100);
}

TEST_F(ImsListTest, InsertAt)
{
    EXPECT_EQ(m_objList.GetAt(0), 1);

    ASSERT_TRUE(m_objList.InsertAt(100, 0));
    EXPECT_EQ(m_objList.GetAt(0), 100);

    ASSERT_TRUE(m_objList.InsertAt(100, 5));
    EXPECT_EQ(m_objList.GetAt(5), 100);

    ASSERT_TRUE(m_objList.InsertAt(100, m_objList.GetSize()));
    EXPECT_EQ(m_objList.GetAt(m_objList.GetSize() - 1), 100);

    ASSERT_TRUE(m_objList.InsertAt(200, 0, 2));
    EXPECT_EQ(m_objList.GetAt(0), 200);
    EXPECT_EQ(m_objList.GetAt(1), 200);

    ASSERT_TRUE(m_objList.InsertAt(200, 5, 2));
    EXPECT_EQ(m_objList.GetAt(5), 200);
    EXPECT_EQ(m_objList.GetAt(6), 200);

    ASSERT_TRUE(m_objList.InsertAt(200, m_objList.GetSize(), 2));
    EXPECT_EQ(m_objList.GetAt(m_objList.GetSize() - 2), 200);
    EXPECT_EQ(m_objList.GetAt(m_objList.GetSize() - 1), 200);
}

TEST_F(ImsListTest, SetAt)
{
    EXPECT_EQ(m_objList.GetAt(3), 4);

    ASSERT_TRUE(m_objList.SetAt(100, 3));
    EXPECT_EQ(m_objList.GetAt(3), 100);

    ASSERT_FALSE(m_objList.SetAt(100, m_objList.GetSize()));
}

TEST_F(ImsListTest, RemoveAt)
{
    EXPECT_EQ(m_objList.GetSize(), 10);
    EXPECT_EQ(m_objList.GetAt(3), 4);

    ASSERT_TRUE(m_objList.RemoveAt(3));
    EXPECT_EQ(m_objList.GetSize(), 9);
    EXPECT_EQ(m_objList.GetAt(3), 5);

    ASSERT_TRUE(m_objList.RemoveElementsAt(3, 2));
    EXPECT_EQ(m_objList.GetSize(), 7);
    EXPECT_EQ(m_objList.GetAt(3), 7);

    ASSERT_TRUE(m_objList.RemoveElementsAt(0, m_objList.GetSize()));
    EXPECT_EQ(m_objList.GetSize(), 0);
}

TEST_F(ImsListTest, Remove)
{
    IMS_UINT32 nSize = m_objList.GetSize();
    EXPECT_TRUE(m_objList.Remove(1));
    EXPECT_EQ(nSize - 1, m_objList.GetSize());
    EXPECT_EQ(2, m_objList.GetAt(0));
    EXPECT_TRUE(m_objList.Remove(10));
    EXPECT_EQ(nSize - 2, m_objList.GetSize());
    EXPECT_EQ(9, m_objList.GetAt(m_objList.GetSize() - 1));
    EXPECT_FALSE(m_objList.Remove(11));
    EXPECT_EQ(nSize - 2, m_objList.GetSize());

    ImsList<AString> objList;
    AString strDog("dog");
    AString strCat("cat");
    AString strLion("lion");
    objList.Append(strDog);
    objList.Append(strCat);

    EXPECT_TRUE(objList.Remove(strDog));
    EXPECT_EQ(1, objList.GetSize());
    EXPECT_EQ(strCat, objList.GetAt(0));
    EXPECT_TRUE(objList.Remove(strCat));
    EXPECT_EQ(0, objList.GetSize());
    EXPECT_FALSE(objList.Remove(strLion));

    ImsList<AString*> objPointerList;
    objPointerList.Append(&strDog);
    objPointerList.Append(&strCat);

    EXPECT_TRUE(objPointerList.Remove(&strDog));
    EXPECT_EQ(1, objPointerList.GetSize());
    AString* pstrCat = objPointerList.GetAt(0);
    EXPECT_EQ(strCat, *pstrCat);
    EXPECT_TRUE(objPointerList.Remove(&strCat));
    EXPECT_EQ(0, objPointerList.GetSize());
    EXPECT_FALSE(objPointerList.Remove(&strLion));
}

}  // namespace android
