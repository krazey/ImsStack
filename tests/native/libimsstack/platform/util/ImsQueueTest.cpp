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

#include "ImsQueue.h"

namespace android
{

class ImsQueueTest : public ::testing::Test
{
protected:
    virtual void SetUp() override
    {
        for (IMS_SINT32 i = 0; i < 10; ++i)
        {
            // 1 ~ 10
            m_objQueue.Push(i + 1);
        }
    }

    virtual void TearDown() override
    {
        m_objQueue.Clear();
        EXPECT_TRUE(m_objQueue.IsEmpty());
    }

protected:
    ImsQueue<IMS_SINT32> m_objQueue;
};

TEST_F(ImsQueueTest, Constructor)
{
    ImsQueue<IMS_SINT32> objQueue;
    EXPECT_TRUE(objQueue.IsEmpty());
    EXPECT_EQ(objQueue.GetSize(), 0);
}

TEST_F(ImsQueueTest, CopyConstructor)
{
    ImsQueue<IMS_SINT32> objNewQueue(m_objQueue);
    EXPECT_EQ(objNewQueue.GetSize(), m_objQueue.GetSize());
    EXPECT_EQ(objNewQueue, m_objQueue);
}

TEST_F(ImsQueueTest, OperatorAssignment)
{
    ImsQueue<IMS_SINT32> objNewQueue;
    objNewQueue = m_objQueue;
    EXPECT_EQ(objNewQueue.GetSize(), m_objQueue.GetSize());
    EXPECT_EQ(objNewQueue, m_objQueue);
}

TEST_F(ImsQueueTest, Equals)
{
    ImsQueue<IMS_SINT32> objNewQueue(m_objQueue);
    EXPECT_TRUE(objNewQueue.Equals(m_objQueue));
    EXPECT_TRUE(objNewQueue == m_objQueue);

    objNewQueue.GetBack() = 100;
    EXPECT_FALSE(objNewQueue.Equals(m_objQueue));
    EXPECT_FALSE(objNewQueue == m_objQueue);

    objNewQueue.Clear();
    EXPECT_FALSE(objNewQueue.Equals(m_objQueue));
    EXPECT_FALSE(objNewQueue == m_objQueue);
}

TEST_F(ImsQueueTest, GetBack)
{
    const ImsQueue<IMS_SINT32>& objQueue = m_objQueue;
    EXPECT_EQ(objQueue.GetBack(), 10);

    m_objQueue.GetBack() = 100;
    EXPECT_EQ(objQueue.GetBack(), 100);
}

TEST_F(ImsQueueTest, GetFront)
{
    const ImsQueue<IMS_SINT32>& objQueue = m_objQueue;
    EXPECT_EQ(objQueue.GetFront(), 1);

    m_objQueue.GetFront() = 100;
    EXPECT_EQ(objQueue.GetFront(), 100);
}

TEST_F(ImsQueueTest, Pop)
{
    EXPECT_EQ(m_objQueue.GetFront(), 1);

    IMS_UINT32 nNewSize;
    IMS_UINT32 nOldSize;

    nOldSize = m_objQueue.GetSize();
    m_objQueue.Pop();
    nNewSize = m_objQueue.GetSize();
    EXPECT_EQ(nNewSize, nOldSize - 1);
    EXPECT_EQ(m_objQueue.GetFront(), 2);

    nOldSize = nNewSize;
    m_objQueue.Pop();
    nNewSize = m_objQueue.GetSize();
    EXPECT_EQ(nNewSize, nOldSize - 1);
    EXPECT_EQ(m_objQueue.GetFront(), 3);
}

TEST_F(ImsQueueTest, Push)
{
    EXPECT_EQ(m_objQueue.GetBack(), 10);

    IMS_UINT32 nNewSize;
    IMS_UINT32 nOldSize;

    nOldSize = m_objQueue.GetSize();
    m_objQueue.Push(100);
    nNewSize = m_objQueue.GetSize();
    EXPECT_EQ(nNewSize, nOldSize + 1);
    EXPECT_EQ(m_objQueue.GetBack(), 100);

    nOldSize = nNewSize;
    m_objQueue.Push(200);
    nNewSize = m_objQueue.GetSize();
    EXPECT_EQ(nNewSize, nOldSize + 1);
    EXPECT_EQ(m_objQueue.GetBack(), 200);
}

}  // namespace android
