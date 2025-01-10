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

#include "IMtsService.h"
#include "IMtsServiceState.h"
#include "MtsApp.h"
#include <gtest/gtest.h>

namespace android
{

const IMS_SINT32 SLOT_ID = 0;

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

TEST_F(MtsAppTest, GetSlotId)
{
    EXPECT_EQ(pMtsApp->GetSlotId(), SLOT_ID);
}

TEST_F(MtsAppTest, StartAndStop)
{
    pMtsApp->Start();

    pMtsApp->Stop();
    EXPECT_EQ(pMtsApp->GetService().GetIMtsServiceState()->GetImsRegConnected(), IMS_FALSE);
}

}  // namespace android
