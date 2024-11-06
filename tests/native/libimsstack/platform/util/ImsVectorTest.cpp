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
#include "ImsVector.h"

namespace android
{

class ImsVectorTest : public ::testing::Test
{
protected:
    virtual void SetUp() override
    {
        for (IMS_SINT32 i = 0; i < 10; ++i)
        {
            // 1 ~ 10
            m_objVector.Add(i + 1);
        }
    }

    virtual void TearDown() override
    {
        m_objVector.Clear();
        EXPECT_TRUE(m_objVector.IsEmpty());
    }

protected:
    ImsVector<IMS_SINT32> m_objVector;
};

TEST_F(ImsVectorTest, Constructor)
{
    ImsVector<IMS_SINT32> objVector;
    EXPECT_TRUE(objVector.IsEmpty());
    EXPECT_EQ(objVector.GetSize(), 0);
}

TEST_F(ImsVectorTest, CopyConstructor)
{
    ImsVector<IMS_SINT32> objNewVector(m_objVector);
    EXPECT_EQ(objNewVector.GetSize(), m_objVector.GetSize());
    EXPECT_EQ(objNewVector, m_objVector);
}

TEST_F(ImsVectorTest, OperatorAssignment)
{
    ImsVector<IMS_SINT32> objNewVector;
    objNewVector = m_objVector;
    EXPECT_EQ(objNewVector.GetSize(), m_objVector.GetSize());
    EXPECT_EQ(objNewVector, m_objVector);
}

TEST_F(ImsVectorTest, Contains)
{
    EXPECT_TRUE(m_objVector.Contains(1));
    EXPECT_TRUE(m_objVector.Contains(10));
    EXPECT_FALSE(m_objVector.Contains(11));

    ImsVector<AString> objVector;
    AString strDog("dog");
    AString strCat("cat");
    AString strLion("lion");
    objVector.Add(strDog);
    objVector.Add(strCat);

    EXPECT_TRUE(objVector.Contains(strDog));
    EXPECT_TRUE(objVector.Contains(strCat));
    EXPECT_FALSE(objVector.Contains(strLion));

    ImsVector<AString*> objPointerVector;
    objPointerVector.Add(&strDog);
    objPointerVector.Add(&strCat);

    EXPECT_TRUE(objPointerVector.Contains(&strDog));
    EXPECT_TRUE(objPointerVector.Contains(&strCat));
    EXPECT_FALSE(objPointerVector.Contains(&strLion));
}

TEST_F(ImsVectorTest, Equals)
{
    ImsVector<IMS_SINT32> objNewVector(m_objVector);
    EXPECT_TRUE(objNewVector.Equals(m_objVector));
    EXPECT_TRUE(objNewVector == m_objVector);

    objNewVector.ReplaceAt(100, 3);
    EXPECT_FALSE(objNewVector.Equals(m_objVector));
    EXPECT_FALSE(objNewVector == m_objVector);

    objNewVector.RemoveElementsAt(7, 3);
    EXPECT_FALSE(objNewVector.Equals(m_objVector));
    EXPECT_FALSE(objNewVector == m_objVector);

    objNewVector.Clear();
    EXPECT_FALSE(objNewVector.Equals(m_objVector));
    EXPECT_FALSE(objNewVector == m_objVector);
}

TEST_F(ImsVectorTest, SetCapacity)
{
    ImsVector<IMS_SINT32> objVector;
    IMS_UINT32 nOldCapacity = objVector.GetCapacity();
    IMS_UINT32 nNewCapacity = objVector.SetCapacity(objVector.GetSize() + 16);
    EXPECT_EQ(nNewCapacity, objVector.GetSize() + 16);
    EXPECT_GT(nNewCapacity, nOldCapacity);

    nOldCapacity = m_objVector.GetCapacity();
    nNewCapacity = m_objVector.SetCapacity(m_objVector.GetSize() - 1);
    EXPECT_EQ(nOldCapacity, nNewCapacity);
}

TEST_F(ImsVectorTest, GetArrayConst)
{
    const IMS_SINT32* pData = m_objVector.GetArrayConst();

    for (IMS_UINT32 i = 0; i < m_objVector.GetSize(); ++i)
    {
        EXPECT_EQ(pData[i], m_objVector.GetAt(i));
    }
}

TEST_F(ImsVectorTest, GetArray)
{
    IMS_SINT32* pData = m_objVector.GetArray();

    for (IMS_UINT32 i = 0; i < m_objVector.GetSize(); ++i)
    {
        EXPECT_EQ(pData[i], m_objVector.GetAt(i));
    }

    pData[3] = 100;
    EXPECT_EQ(m_objVector.GetAt(3), 100);
}

TEST_F(ImsVectorTest, GetAt)
{
    IMS_SINT32 nTestIndex = 3;

    // const operations
    const ImsVector<IMS_SINT32>& objVector = m_objVector;
    EXPECT_EQ(objVector.GetAt(nTestIndex), objVector[nTestIndex]);
    EXPECT_EQ(objVector.GetAt(objVector.GetSize() - 1), objVector.Top());

    // non-const operations
    EXPECT_EQ(m_objVector.GetAt(nTestIndex), m_objVector.GetValueAt(nTestIndex));
    EXPECT_EQ(m_objVector.GetAt(m_objVector.GetSize() - 1), m_objVector.Top());

    m_objVector.Top() = 100;
    EXPECT_EQ(m_objVector.GetAt(m_objVector.GetSize() - 1), 100);
    EXPECT_EQ(m_objVector.Top(), 100);

    m_objVector.GetAt(nTestIndex) = 200;
    EXPECT_EQ(m_objVector.GetAt(nTestIndex), 200);
}

TEST_F(ImsVectorTest, InsertVectorAt)
{
    ImsVector<IMS_SINT32> objVector;
    objVector.Add(11);
    objVector.Add(12);
    objVector.Add(13);
    objVector.Add(14);
    objVector.Add(15);

    m_objVector.InsertVectorAt(objVector, 0);
    m_objVector.InsertVectorAt(objVector, 10);
    m_objVector.AppendVector(objVector);

    // 11 ~ 15, 1 ~ 5, 11 ~ 15, 6 ~ 10, 11 ~ 15
    const IMS_SINT32 nMergedCount = 25;
    // clang-format off
    const IMS_SINT32 nTestDatas[nMergedCount] = {
        11, 12, 13, 14, 15, 1, 2, 3, 4, 5, 11, 12, 13, 14, 15, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15
    };
    // clang-format on

    ASSERT_EQ(m_objVector.GetSize(), nMergedCount);

    for (IMS_SINT32 i = 0; i < nMergedCount; ++i)
    {
        EXPECT_EQ(m_objVector.GetAt(i), nTestDatas[i]);
    }
}

TEST_F(ImsVectorTest, Push)
{
    IMS_UINT32 nNewSize;
    IMS_UINT32 nOldSize;

    nOldSize = m_objVector.GetSize();
    m_objVector.Push();
    nNewSize = m_objVector.GetSize();
    EXPECT_EQ(nNewSize, nOldSize + 1);
    EXPECT_EQ(m_objVector.Top(), 0);

    nOldSize = nNewSize;
    m_objVector.Pop();
    nNewSize = m_objVector.GetSize();
    EXPECT_EQ(nNewSize, nOldSize - 1);
}

TEST_F(ImsVectorTest, Add)
{
    IMS_SLONG nSize = static_cast<IMS_SLONG>(m_objVector.GetSize());
    IMS_SLONG nIndex = m_objVector.Add();
    EXPECT_EQ(m_objVector.GetSize(), nSize + 1);
    EXPECT_EQ(m_objVector.GetAt(nIndex), 0);

    nIndex = m_objVector.Add(100);
    EXPECT_EQ(m_objVector.GetSize(), nSize + 2);
    EXPECT_EQ(m_objVector.GetAt(nIndex), 100);
}

TEST_F(ImsVectorTest, InsertAt)
{
    EXPECT_EQ(m_objVector.GetAt(0), 1);

    m_objVector.InsertAt(0);
    EXPECT_EQ(m_objVector.GetAt(0), 0);
    m_objVector.InsertAt(100, 0);
    EXPECT_EQ(m_objVector.GetAt(0), 100);

    m_objVector.InsertAt(5);
    EXPECT_EQ(m_objVector.GetAt(5), 0);
    m_objVector.InsertAt(100, 5);
    EXPECT_EQ(m_objVector.GetAt(5), 100);

    m_objVector.InsertAt(m_objVector.GetSize());
    EXPECT_EQ(m_objVector.GetAt(m_objVector.GetSize() - 1), 0);
    m_objVector.InsertAt(100, m_objVector.GetSize());
    EXPECT_EQ(m_objVector.GetAt(m_objVector.GetSize() - 1), 100);

    m_objVector.InsertAt(static_cast<IMS_UINT32>(0), 2);
    EXPECT_EQ(m_objVector.GetAt(0), 0);
    EXPECT_EQ(m_objVector.GetAt(1), 0);
    m_objVector.InsertAt(200, 0, 2);
    EXPECT_EQ(m_objVector.GetAt(0), 200);
    EXPECT_EQ(m_objVector.GetAt(1), 200);

    m_objVector.InsertAt(static_cast<IMS_UINT32>(5), 2);
    EXPECT_EQ(m_objVector.GetAt(5), 0);
    EXPECT_EQ(m_objVector.GetAt(6), 0);
    m_objVector.InsertAt(200, 5, 2);
    EXPECT_EQ(m_objVector.GetAt(5), 200);
    EXPECT_EQ(m_objVector.GetAt(6), 200);

    m_objVector.InsertAt(static_cast<IMS_UINT32>(m_objVector.GetSize()), 2);
    EXPECT_EQ(m_objVector.GetAt(m_objVector.GetSize() - 2), 0);
    EXPECT_EQ(m_objVector.GetAt(m_objVector.GetSize() - 1), 0);
    m_objVector.InsertAt(200, m_objVector.GetSize(), 2);
    EXPECT_EQ(m_objVector.GetAt(m_objVector.GetSize() - 2), 200);
    EXPECT_EQ(m_objVector.GetAt(m_objVector.GetSize() - 1), 200);
}

TEST_F(ImsVectorTest, ReplaceAt)
{
    EXPECT_EQ(m_objVector.GetAt(3), 4);

    EXPECT_EQ(m_objVector.ReplaceAt(3), 3);
    EXPECT_EQ(m_objVector.GetAt(3), 0);

    EXPECT_EQ(m_objVector.ReplaceAt(100, 3), 3);
    EXPECT_EQ(m_objVector.GetAt(3), 100);

    EXPECT_EQ(m_objVector.ReplaceAt(m_objVector.GetSize()), -1);
    EXPECT_EQ(m_objVector.ReplaceAt(100, m_objVector.GetSize()), -1);
}

TEST_F(ImsVectorTest, RemoveAt)
{
    EXPECT_EQ(m_objVector.GetSize(), 10);
    EXPECT_EQ(m_objVector.GetAt(3), 4);

    m_objVector.RemoveAt(3);
    EXPECT_EQ(m_objVector.GetSize(), 9);
    EXPECT_EQ(m_objVector.GetAt(3), 5);

    m_objVector.RemoveElementsAt(3, 2);
    EXPECT_EQ(m_objVector.GetSize(), 7);
    EXPECT_EQ(m_objVector.GetAt(3), 7);

    m_objVector.RemoveElementsAt(0, m_objVector.GetSize());
    EXPECT_EQ(m_objVector.GetSize(), 0);
}

TEST_F(ImsVectorTest, Remove)
{
    IMS_UINT32 nSize = m_objVector.GetSize();
    EXPECT_TRUE(m_objVector.Remove(1));
    EXPECT_EQ(nSize - 1, m_objVector.GetSize());
    EXPECT_EQ(2, m_objVector.GetAt(0));
    EXPECT_TRUE(m_objVector.Remove(10));
    EXPECT_EQ(nSize - 2, m_objVector.GetSize());
    EXPECT_EQ(9, m_objVector.GetAt(m_objVector.GetSize() - 1));
    EXPECT_FALSE(m_objVector.Remove(11));
    EXPECT_EQ(nSize - 2, m_objVector.GetSize());

    ImsVector<AString> objVector;
    AString strDog("dog");
    AString strCat("cat");
    AString strLion("lion");
    objVector.Add(strDog);
    objVector.Add(strCat);

    EXPECT_TRUE(objVector.Remove(strDog));
    EXPECT_EQ(1, objVector.GetSize());
    EXPECT_EQ(strCat, objVector.GetAt(0));
    EXPECT_TRUE(objVector.Remove(strCat));
    EXPECT_EQ(0, objVector.GetSize());
    EXPECT_FALSE(objVector.Remove(strLion));

    ImsVector<AString*> objPointerVector;
    objPointerVector.Add(&strDog);
    objPointerVector.Add(&strCat);

    EXPECT_TRUE(objPointerVector.Remove(&strDog));
    EXPECT_EQ(1, objPointerVector.GetSize());
    AString* pstrCat = objPointerVector.GetAt(0);
    EXPECT_EQ(strCat, *pstrCat);
    EXPECT_TRUE(objPointerVector.Remove(&strCat));
    EXPECT_EQ(0, objPointerVector.GetSize());
    EXPECT_FALSE(objPointerVector.Remove(&strLion));
}

TEST_F(ImsVectorTest, Shrink)
{
    ImsVector<IMS_SINT32> objVector;
    IMS_UINT32 nCapacity = m_objVector.GetCapacity();

    for (IMS_SINT32 i = 0; i < 50; ++i)
    {
        m_objVector.Add(i + 11);
    }

    EXPECT_GT(m_objVector.GetCapacity(), nCapacity);

    nCapacity = m_objVector.GetCapacity();
    m_objVector.RemoveElementsAt(30, 30);
    m_objVector.Shrink();
    EXPECT_LT(m_objVector.GetCapacity(), nCapacity);

    nCapacity = m_objVector.GetCapacity();
    m_objVector.Clear();
    m_objVector.Shrink();
    EXPECT_LT(m_objVector.GetCapacity(), nCapacity);
}

}  // namespace android
