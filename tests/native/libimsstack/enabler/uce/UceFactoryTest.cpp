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
#include "UceFactory.h"

class TestUceFactory : public UceFactory
{
    inline TestUceFactory() :
            UceFactory()
    {
    }

public:
    inline static UceFactory* GetUceFactory() { return UceFactory::m_gpUceFactory; }

    inline static void SetUceFactory(IN UceFactory* gpUceFactory)
    {
        UceFactory::m_gpUceFactory = gpUceFactory;
    }

    inline static IMSMap<IMS_SINT32, UceApp*> GetUceManagers()
    {
        return UceFactory::m_objUceManagers;
    }

    inline static void SetUceManagers(IN const IMSMap<IMS_SINT32, UceApp*>& objManagers)
    {
        UceFactory::m_objUceManagers = objManagers;
    }

    inline static void DeleteUceFactory()
    {
        if (GetUceFactory())
        {
            delete UceFactory::m_gpUceFactory;
        }
    }
};

class UceFactoryTest : public ::testing::Test
{
public:
    UceFactory* m_pUceFactory;
    IMSMap<IMS_SINT32, UceApp*> m_objUceManagers;

protected:
    virtual void SetUp() override
    {
        m_pUceFactory = TestUceFactory::GetUceFactory();
        m_objUceManagers = TestUceFactory::GetUceManagers();

        TestUceFactory::SetUceFactory(IMS_NULL);
        TestUceFactory::GetUceManagers().Remove(IMS_SLOT_0);
    }

    virtual void TearDown() override
    {
        TestUceFactory::DeleteUceFactory();
        TestUceFactory::SetUceFactory(IMS_NULL);
        TestUceFactory::GetUceManagers().Remove(IMS_SLOT_0);

        TestUceFactory::SetUceFactory(m_pUceFactory);
        TestUceFactory::SetUceManagers(m_objUceManagers);
    }
};

TEST_F(UceFactoryTest, Start)
{
    EXPECT_EQ(TestUceFactory::GetUceFactory(), nullptr);
    EXPECT_EQ(TestUceFactory::GetUceApp(), nullptr);
    IMS_SINT32 nManagers = TestUceFactory::GetUceManagers().GetSize();

    // Test1 : First Start
    TestUceFactory::Start(IMS_SLOT_0);
    EXPECT_NE(TestUceFactory::GetUceFactory(), nullptr);
    EXPECT_NE(TestUceFactory::GetUceApp(), nullptr);
    EXPECT_EQ(TestUceFactory::GetUceManagers().GetSize(), nManagers + 1);

    // Test2 : Duplicate Start
    TestUceFactory::Start(IMS_SLOT_0);
    EXPECT_NE(TestUceFactory::GetUceFactory(), nullptr);
    EXPECT_NE(TestUceFactory::GetUceApp(), nullptr);
    EXPECT_EQ(TestUceFactory::GetUceManagers().GetSize(), nManagers + 1);
}

TEST_F(UceFactoryTest, Stop)
{
    EXPECT_EQ(TestUceFactory::GetUceFactory(), nullptr);
    EXPECT_EQ(TestUceFactory::GetUceApp(), nullptr);

    // Test1 : Stop without Start
    TestUceFactory::Stop(IMS_SLOT_0);
    EXPECT_NE(TestUceFactory::GetUceFactory(), nullptr);
    EXPECT_EQ(TestUceFactory::GetUceApp(), nullptr);

    // Test2 : Stop after Start
    TestUceFactory::Start(IMS_SLOT_0);
    EXPECT_NE(TestUceFactory::GetUceFactory(), nullptr);
    EXPECT_NE(TestUceFactory::GetUceApp(), nullptr);
    IMS_SINT32 nManagers = TestUceFactory::GetUceManagers().GetSize();

    TestUceFactory::Stop(IMS_SLOT_0);
    EXPECT_NE(TestUceFactory::GetUceFactory(), nullptr);
    EXPECT_EQ(TestUceFactory::GetUceApp(), nullptr);
    EXPECT_EQ(TestUceFactory::GetUceManagers().GetSize(), nManagers - 1);
}