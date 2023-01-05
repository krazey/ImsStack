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

#include "ImsStack.h"

namespace android
{

class ImsStackTest : public ::testing::Test
{
protected:
    virtual void SetUp() override
    {
        for (IMS_SINT32 i = 0; i < 10; ++i)
        {
            // 1 ~ 10
            m_objStack.Push(i + 1);
        }
    }

    virtual void TearDown() override
    {
        m_objStack.Clear();
        EXPECT_TRUE(m_objStack.IsEmpty());
    }

protected:
    ImsStack<IMS_SINT32> m_objStack;
};

TEST_F(ImsStackTest, Constructor)
{
    ImsStack<IMS_SINT32> objStack;
    EXPECT_TRUE(objStack.IsEmpty());
    EXPECT_EQ(objStack.GetSize(), 0);
}

TEST_F(ImsStackTest, CopyConstructor)
{
    ImsStack<IMS_SINT32> objNewStack(m_objStack);
    EXPECT_EQ(objNewStack.GetSize(), m_objStack.GetSize());
    EXPECT_EQ(objNewStack, m_objStack);
}

TEST_F(ImsStackTest, OperatorAssignment)
{
    ImsStack<IMS_SINT32> objNewStack;
    objNewStack = m_objStack;
    EXPECT_EQ(objNewStack.GetSize(), m_objStack.GetSize());
    EXPECT_EQ(objNewStack, m_objStack);
}

TEST_F(ImsStackTest, Equals)
{
    ImsStack<IMS_SINT32> objNewStack(m_objStack);
    EXPECT_TRUE(objNewStack.Equals(m_objStack));
    EXPECT_TRUE(objNewStack == m_objStack);

    objNewStack.Top() = 100;
    EXPECT_FALSE(objNewStack.Equals(m_objStack));
    EXPECT_FALSE(objNewStack == m_objStack);

    objNewStack.Clear();
    EXPECT_FALSE(objNewStack.Equals(m_objStack));
    EXPECT_FALSE(objNewStack == m_objStack);
}

TEST_F(ImsStackTest, Top)
{
    const ImsStack<IMS_SINT32>& objStack = m_objStack;
    EXPECT_EQ(objStack.Top(), 10);

    m_objStack.Top() = 100;
    EXPECT_EQ(objStack.Top(), 100);
}

TEST_F(ImsStackTest, Pop)
{
    EXPECT_EQ(m_objStack.Top(), 10);

    IMS_UINT32 nNewSize;
    IMS_UINT32 nOldSize;

    nOldSize = m_objStack.GetSize();
    m_objStack.Pop();
    nNewSize = m_objStack.GetSize();
    EXPECT_EQ(nNewSize, nOldSize - 1);
    EXPECT_EQ(m_objStack.Top(), 9);

    nOldSize = nNewSize;
    m_objStack.Pop();
    nNewSize = m_objStack.GetSize();
    EXPECT_EQ(nNewSize, nOldSize - 1);
    EXPECT_EQ(m_objStack.Top(), 8);
}

TEST_F(ImsStackTest, Push)
{
    EXPECT_EQ(m_objStack.Top(), 10);

    IMS_UINT32 nNewSize;
    IMS_UINT32 nOldSize;

    nOldSize = m_objStack.GetSize();
    m_objStack.Push(100);
    nNewSize = m_objStack.GetSize();
    EXPECT_EQ(nNewSize, nOldSize + 1);
    EXPECT_EQ(m_objStack.Top(), 100);

    nOldSize = nNewSize;
    m_objStack.Push(200);
    nNewSize = m_objStack.GetSize();
    EXPECT_EQ(nNewSize, nOldSize + 1);
    EXPECT_EQ(m_objStack.Top(), 200);
}

}  // namespace android
