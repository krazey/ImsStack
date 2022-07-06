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
#include "MtsApp.h"

LOCAL IMS_SINT32 SLOT_ID = 0;

namespace android
{

class MtsAppTest : public ::testing::Test
{
public:
    MtsApp* pMtsApp;

protected:
    virtual void SetUp() override { pMtsApp = new MtsApp(SLOT_ID); }

    virtual void TearDown() override { delete pMtsApp; }
};

TEST_F(MtsAppTest, Constructor)
{
    ASSERT_NE(pMtsApp, nullptr);
}

TEST_F(MtsAppTest, CreateServiceAfterStart)
{
    pMtsApp->Start();
    ASSERT_NE(pMtsApp->GetMtsService(), nullptr);
}

TEST_F(MtsAppTest, CreateUtilsAfterStart)
{
    pMtsApp->Start();
    ASSERT_NE(pMtsApp->GetMtsDynamicLoader(), nullptr);
    ASSERT_NE(pMtsApp->GetMtsServiceState(), nullptr);
}

TEST_F(MtsAppTest, CreateMtsMessageControllerAfterStart)
{
    pMtsApp->Start();
    ASSERT_NE(pMtsApp->GetMtsMessageController(), nullptr);
}

TEST_F(MtsAppTest, CreateMtsCallTrackerAfterStart)
{
    pMtsApp->Start();
    ASSERT_NE(pMtsApp->GetMtsCallTracker(), nullptr);
}

}  // namespace android
