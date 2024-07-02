/*
 * Copyright (C) 2024 The Android Open Source Project
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

#include "CoreContext.h"

#include "MockICoreContext.h"
#include "TestImsCoreProtocol.h"

using ::testing::Return;

namespace android
{

class CoreContextTest : public ::testing::Test
{
public:
    inline CoreContextTest() :
            m_pCoreContext(IMS_NULL)
    {
    }

protected:
    CoreContext* m_pCoreContext;

protected:
    void SetUp() override { m_pCoreContext = CoreContext::GetInstance(); }

    void TearDown() override { CoreContext::DestroyInstance(); }
};

TEST_F(CoreContextTest, Accessors)
{
    EXPECT_NE(m_pCoreContext->GetImsCoreProtocol(), nullptr);
}

TEST_F(CoreContextTest, AccessorsWithExternalCoreContext)
{
    TestImsCoreProtocol objImsCoreProtocol;
    MockICoreContext objCoreContext;
    EXPECT_CALL(objCoreContext, GetImsCoreProtocol())
            .Times(1)
            .WillOnce(Return(&objImsCoreProtocol));
    m_pCoreContext->SetCoreContext(&objCoreContext);

    EXPECT_EQ(m_pCoreContext->GetImsCoreProtocol(), &objImsCoreProtocol);
}

}  // namespace android
