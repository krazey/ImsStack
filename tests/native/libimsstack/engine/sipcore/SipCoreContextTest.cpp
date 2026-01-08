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

#include "SipCoreContext.h"

#include "MockISipCoreContext.h"
#include "TestSipProtocol.h"

using ::testing::Return;

namespace android
{

class SipCoreContextTest : public ::testing::Test
{
public:
    inline SipCoreContextTest() :
            m_pSipCoreContext(IMS_NULL)
    {
    }

protected:
    SipCoreContext* m_pSipCoreContext;

protected:
    void SetUp() override { m_pSipCoreContext = SipCoreContext::GetInstance(); }

    void TearDown() override { SipCoreContext::DestroyInstance(); }
};

TEST_F(SipCoreContextTest, Accessors)
{
    EXPECT_NE(m_pSipCoreContext->GetSipProtocol(), nullptr);
}

TEST_F(SipCoreContextTest, AccessorsWithExternalSipCoreContext)
{
    TestSipProtocol objSipProtocol;
    MockISipCoreContext objSipCoreContext;
    EXPECT_CALL(objSipCoreContext, GetSipProtocol()).Times(1).WillOnce(Return(&objSipProtocol));
    m_pSipCoreContext->SetSipCoreContext(&objSipCoreContext);

    EXPECT_EQ(m_pSipCoreContext->GetSipProtocol(), &objSipProtocol);
}

}  // namespace android
