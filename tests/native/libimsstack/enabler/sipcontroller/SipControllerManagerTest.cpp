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
#include "SipControllerManager.h"

namespace android
{
const IMS_SINT32 SLOT_ID = 0;
const AString STR_NAME = "SipControllerThread";

class SipControllerManagerTest : public ::testing::Test
{
public:
    SipControllerManager* pManager;

protected:
    virtual void SetUp() override { pManager = new SipControllerManager(SLOT_ID, STR_NAME); }

    virtual void TearDown() override { delete pManager; }
};

TEST_F(SipControllerManagerTest, Constructor)
{
    ASSERT_NE(pManager, nullptr);
}

TEST_F(SipControllerManagerTest, CreateRegService)
{
    ASSERT_NE(pManager->GetRegService(), nullptr);
}

TEST_F(SipControllerManagerTest, CreateConfService)
{
    ASSERT_NE(pManager->GetConfService(), nullptr);
}

TEST_F(SipControllerManagerTest, CreateMsgService)
{
    ASSERT_NE(pManager->GetMsgService(), nullptr);
}
}  // namespace android
