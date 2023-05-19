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

#include "MtcFactory.h"
#include <gtest/gtest.h>

namespace android
{

class TestMtcFactory : public MtcFactory
{
public:
    IMS_BOOL Has(IN IMS_SINT32 nSlotId) { return m_objMtcApps.GetIndexOfKey(nSlotId) >= 0; }
};

class MtcFactoryTest : public ::testing::Test
{
protected:
    virtual void SetUp() override {}

    virtual void TearDown() override {}
};

TEST_F(MtcFactoryTest, GetInstanceReturnsSameInstance)
{
    EXPECT_EQ(MtcFactory::GetInstance(), MtcFactory::GetInstance());
}

TEST_F(MtcFactoryTest, StartCreatesNewApp)
{
    TestMtcFactory objFactory;

    EXPECT_FALSE(objFactory.Has(0));
    EXPECT_FALSE(objFactory.Has(1));

    objFactory.Start(1);

    EXPECT_FALSE(objFactory.Has(0));
    EXPECT_TRUE(objFactory.Has(1));
}

TEST_F(MtcFactoryTest, StartTwiceDoesNothing)
{
    TestMtcFactory objFactory;

    objFactory.Start(0);
    objFactory.Start(0);

    EXPECT_TRUE(objFactory.Has(0));
}

TEST_F(MtcFactoryTest, StopDestroysApp)
{
    TestMtcFactory objFactory;
    objFactory.Start(0);
    objFactory.Start(1);

    objFactory.Stop(1);

    EXPECT_TRUE(objFactory.Has(0));
    EXPECT_FALSE(objFactory.Has(1));
}

TEST_F(MtcFactoryTest, StopTwiceDoesNothing)
{
    TestMtcFactory objFactory;
    objFactory.Start(0);

    objFactory.Stop(0);
    objFactory.Stop(0);

    EXPECT_FALSE(objFactory.Has(0));
}

}  // namespace android
