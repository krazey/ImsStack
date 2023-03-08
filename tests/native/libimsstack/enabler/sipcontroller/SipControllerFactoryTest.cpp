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
#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include "ImsTypeDef.h"
#include "SipControllerFactory.h"

class TestSipControllerFactory : public SipControllerFactory
{
    inline TestSipControllerFactory() :
            SipControllerFactory()
    {
    }

public:
    inline static SipControllerFactory* GetFactory() { return SipControllerFactory::m_gpFactory; }

    inline static void SetFactory(IN SipControllerFactory* gpFactory)
    {
        SipControllerFactory::m_gpFactory = gpFactory;
    }

    inline static ImsMap<IMS_SINT32, SipControllerManager*> GetManagers()
    {
        return SipControllerFactory::m_objManagers;
    }

    inline static void SetManagers(IN const ImsMap<IMS_SINT32, SipControllerManager*>& objManagers)
    {
        SipControllerFactory::m_objManagers = objManagers;
    }

    inline static void DeleteFactory()
    {
        if (GetFactory())
        {
            delete SipControllerFactory::m_gpFactory;
        }
    }
};

class SipControllerFactoryTest : public ::testing::Test
{
public:
    SipControllerFactory* m_pFactory;
    ImsMap<IMS_SINT32, SipControllerManager*> m_objManagers;

protected:
    virtual void SetUp() override
    {
        m_pFactory = TestSipControllerFactory::GetFactory();
        m_objManagers = TestSipControllerFactory::GetManagers();

        TestSipControllerFactory::SetFactory(IMS_NULL);
        TestSipControllerFactory::GetManagers().Remove(IMS_SLOT_0);
    }

    virtual void TearDown() override
    {
        TestSipControllerFactory::DeleteFactory();
        TestSipControllerFactory::SetFactory(IMS_NULL);
        TestSipControllerFactory::GetManagers().Remove(IMS_SLOT_0);

        TestSipControllerFactory::SetFactory(m_pFactory);
        TestSipControllerFactory::SetManagers(m_objManagers);
    }
};

TEST_F(SipControllerFactoryTest, Start)
{
    EXPECT_EQ(TestSipControllerFactory::GetFactory(), nullptr);
    EXPECT_EQ(TestSipControllerFactory::GetSipControllerManager(), nullptr);
    IMS_SINT32 nManagers = TestSipControllerFactory::GetManagers().GetSize();

    // Test1 : First Start
    TestSipControllerFactory::Start(IMS_SLOT_0);
    EXPECT_NE(TestSipControllerFactory::GetFactory(), nullptr);
    EXPECT_NE(TestSipControllerFactory::GetSipControllerManager(), nullptr);
    EXPECT_EQ(TestSipControllerFactory::GetManagers().GetSize(), nManagers + 1);

    // Test2 : Duplicate Start
    TestSipControllerFactory::Start(IMS_SLOT_0);
    EXPECT_NE(TestSipControllerFactory::GetFactory(), nullptr);
    EXPECT_NE(TestSipControllerFactory::GetSipControllerManager(), nullptr);
    EXPECT_EQ(TestSipControllerFactory::GetManagers().GetSize(), nManagers + 1);
}

TEST_F(SipControllerFactoryTest, Stop)
{
    EXPECT_EQ(TestSipControllerFactory::GetFactory(), nullptr);
    EXPECT_EQ(TestSipControllerFactory::GetSipControllerManager(), nullptr);

    // Test1 : Stop without Start
    TestSipControllerFactory::Stop(IMS_SLOT_0);
    EXPECT_NE(TestSipControllerFactory::GetFactory(), nullptr);
    EXPECT_EQ(TestSipControllerFactory::GetSipControllerManager(), nullptr);

    // Test2 : Stop after Start
    TestSipControllerFactory::Start(IMS_SLOT_0);
    EXPECT_NE(TestSipControllerFactory::GetFactory(), nullptr);
    EXPECT_NE(TestSipControllerFactory::GetSipControllerManager(), nullptr);
    IMS_SINT32 nManagers = TestSipControllerFactory::GetManagers().GetSize();

    TestSipControllerFactory::Stop(IMS_SLOT_0);
    EXPECT_NE(TestSipControllerFactory::GetFactory(), nullptr);
    EXPECT_EQ(TestSipControllerFactory::GetSipControllerManager(), nullptr);
    EXPECT_EQ(TestSipControllerFactory::GetManagers().GetSize(), nManagers - 1);
}