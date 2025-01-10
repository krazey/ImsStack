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
#include <gmock/gmock.h>

#include "ImsTypeDef.h"
#include "IMutex.h"
#include "manager/AosFactory.h"

class TestAosFactory : public AosFactory
{
    inline TestAosFactory() :
            AosFactory()
    {
    }

public:
    inline static AosFactory* GetAosFactory() { return AosFactory::m_gpAosFactory; }

    inline static void SetAosFactory(IN AosFactory* gpAosFactory)
    {
        AosFactory::m_gpAosFactory = gpAosFactory;
    }

    inline static IMutex* GetLock() { return AosFactory::m_gpiLock; }

    inline static void SetLock(IN IMutex* gpiLock) { AosFactory::m_gpiLock = gpiLock; }

    inline static ImsMap<IMS_SINT32, ImsAosManager*> GetManagers()
    {
        return AosFactory::m_objManagers;
    }

    inline static void SetManagers(IN const ImsMap<IMS_SINT32, ImsAosManager*>& objManagers)
    {
        AosFactory::m_objManagers = objManagers;
    }

    inline static void DeleteAosFactory()
    {
        if (GetAosFactory())
        {
            delete AosFactory::m_gpAosFactory;
        }
    }
};

class AosFactoryTest : public ::testing::Test
{
public:
    AosFactory* m_pOriginAosFactory;
    IMutex* m_pOriginLock;
    ImsMap<IMS_SINT32, ImsAosManager*> m_objOriginManagers;

protected:
    void SetUp() override
    {
        m_pOriginAosFactory = TestAosFactory::GetAosFactory();
        m_pOriginLock = TestAosFactory::GetLock();
        m_objOriginManagers = TestAosFactory::GetManagers();

        TestAosFactory::SetAosFactory(IMS_NULL);
        TestAosFactory::SetLock(IMS_NULL);
        TestAosFactory::GetManagers().Remove(IMS_SLOT_0);
    }

    void TearDown() override
    {
        ImsAosManager* pAoSMngr = TestAosFactory::GetManager(IMS_SLOT_0);
        if (pAoSMngr != null)
        {
            TestAosFactory::Stop(IMS_SLOT_0);
        }
        TestAosFactory::DeleteAosFactory();
        TestAosFactory::SetAosFactory(IMS_NULL);
        TestAosFactory::SetLock(IMS_NULL);

        TestAosFactory::SetAosFactory(m_pOriginAosFactory);
        TestAosFactory::SetLock(m_pOriginLock);
        TestAosFactory::SetManagers(m_objOriginManagers);
    }
};

TEST_F(AosFactoryTest, SucceedStart)
{
    // GIVEN
    EXPECT_EQ(TestAosFactory::GetAosFactory(), nullptr);
    EXPECT_EQ(TestAosFactory::GetLock(), nullptr);
    EXPECT_EQ(TestAosFactory::GetManager(), nullptr);
    IMS_SINT32 nManagers = TestAosFactory::GetManagers().GetSize();

    // WHEN
    TestAosFactory::Start(IMS_SLOT_0);

    // THEN
    EXPECT_NE(TestAosFactory::GetAosFactory(), nullptr);
    EXPECT_NE(TestAosFactory::GetLock(), nullptr);
    EXPECT_NE(TestAosFactory::GetManager(), nullptr);
    EXPECT_EQ(TestAosFactory::GetManagers().GetSize(), nManagers + 1);
}

TEST_F(AosFactoryTest, FailStartWhenDuplicate)
{
    // GIVEN
    EXPECT_EQ(TestAosFactory::GetAosFactory(), nullptr);
    EXPECT_EQ(TestAosFactory::GetLock(), nullptr);
    EXPECT_EQ(TestAosFactory::GetManager(), nullptr);
    IMS_SINT32 nManagers = TestAosFactory::GetManagers().GetSize();

    TestAosFactory::Start(IMS_SLOT_0);

    EXPECT_NE(TestAosFactory::GetAosFactory(), nullptr);
    EXPECT_NE(TestAosFactory::GetLock(), nullptr);
    EXPECT_NE(TestAosFactory::GetManager(), nullptr);
    EXPECT_EQ(TestAosFactory::GetManagers().GetSize(), nManagers + 1);

    // WHEN
    TestAosFactory::Start(IMS_SLOT_0);

    // THEN
    EXPECT_NE(TestAosFactory::GetAosFactory(), nullptr);
    EXPECT_NE(TestAosFactory::GetLock(), nullptr);
    EXPECT_NE(TestAosFactory::GetManager(), nullptr);
    EXPECT_EQ(TestAosFactory::GetManagers().GetSize(), nManagers + 1);
}

TEST_F(AosFactoryTest, SucceedStop)
{
    // GIVEN
    TestAosFactory::Start(IMS_SLOT_0);
    EXPECT_NE(TestAosFactory::GetAosFactory(), nullptr);
    EXPECT_NE(TestAosFactory::GetLock(), nullptr);
    EXPECT_NE(TestAosFactory::GetManager(), nullptr);
    IMS_SINT32 nManagers = TestAosFactory::GetManagers().GetSize();

    // WHEN
    TestAosFactory::Stop(IMS_SLOT_0);

    // THEN
    EXPECT_EQ(TestAosFactory::GetManager(), nullptr);
    EXPECT_EQ(TestAosFactory::GetManagers().GetSize(), nManagers - 1);
}

TEST_F(AosFactoryTest, FailStopWhenWithoutStart)
{
    // GIVEN
    EXPECT_EQ(TestAosFactory::GetAosFactory(), nullptr);
    EXPECT_EQ(TestAosFactory::GetLock(), nullptr);
    EXPECT_EQ(TestAosFactory::GetManager(), nullptr);

    // WHEN
    TestAosFactory::Stop(IMS_SLOT_0);

    // THEN
    EXPECT_EQ(TestAosFactory::GetManager(), nullptr);
}
