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
#include "call/MockIMtcCallContext.h"
#include "media/MtcMediaManager.h"

class MtcMediaManagerTest : public ::testing::Test
{
public:
    MockIMtcCallContext objContext;
    MtcMediaManager* pMediaManager;

protected:
    virtual void SetUp() override { pMediaManager = new MtcMediaManager(objContext); }

    virtual void TearDown() override { delete pMediaManager; }
};

TEST_F(MtcMediaManagerTest, GetStateReturnsIdleInitially)
{
    EXPECT_EQ(MediaState::IDLE, pMediaManager->GetState());
    EXPECT_EQ(MediaState::IDLE, pMediaManager->GetOldState());
}

TEST_F(MtcMediaManagerTest, TerminateSetsStateToTerminating)
{
    pMediaManager->Terminate();

    EXPECT_EQ(MediaState::TERMINATING, pMediaManager->GetState());
}
