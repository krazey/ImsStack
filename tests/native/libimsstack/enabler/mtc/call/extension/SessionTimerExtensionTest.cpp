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

#include "call/MockIMtcCallContext.h"
#include "call/extension/IMtcExtension.h"
#include "call/extension/SessionTimerExtension.h"
#include <gmock/gmock.h>
#include <gtest/gtest.h>

class SessionTimerExtensionTest : public ::testing::Test
{
public:
    MockIMtcCallContext objContext;
    SessionTimerExtension* pExtension;
};

TEST_F(SessionTimerExtensionTest, Clone)
{
    pExtension = new SessionTimerExtension(objContext);
    IMtcExtension* pCopiedExtension = pExtension->Clone();

    EXPECT_STREQ(pExtension->GetOptionTag().GetStr(), pCopiedExtension->GetOptionTag().GetStr());
    EXPECT_EQ(pExtension->IsAvailableOnRemote(), pCopiedExtension->IsAvailableOnRemote());

    delete pCopiedExtension;
    delete pExtension;
}
