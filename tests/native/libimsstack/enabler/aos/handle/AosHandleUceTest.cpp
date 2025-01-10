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

#include "handle/AosHandleUce.h"
#include "interface/IAosAppContext.h"

#include "interface/MockIAosAppContext.h"

using ::testing::AnyNumber;
using ::testing::Return;
using ::testing::ReturnRef;

class AosHandleUceTest : public ::testing::Test
{
public:
    AosHandleUce* m_pAosHandleUce;

    MockIAosAppContext m_objMockIAosAppContext;

protected:
    void SetUp() override
    {
        EXPECT_CALL(m_objMockIAosAppContext, GetSlotId())
                .Times(AnyNumber())
                .WillRepeatedly(Return(0));

        const AString strValue = AString("test");
        EXPECT_CALL(m_objMockIAosAppContext, GetProfileId())
                .Times(AnyNumber())
                .WillRepeatedly(ReturnRef(strValue));

        const AString strAppId = AString("ims.app.uce.test");
        const AString strServiceId = AString("ims.service.uce.test");
        const IMS_UINT32 nServiceType = -1;

        m_pAosHandleUce = new AosHandleUce(static_cast<IAosAppContext*>(&m_objMockIAosAppContext),
                strAppId, strServiceId, nServiceType);

        ASSERT_TRUE(m_pAosHandleUce != nullptr);
    }

    void TearDown() override
    {
        if (m_pAosHandleUce != nullptr)
        {
            delete m_pAosHandleUce;
            m_pAosHandleUce = nullptr;
        }
    }

    IMS_BOOL IsRegFeatureTagRequired() { return m_pAosHandleUce->m_bRegFeatureTagRequired; }
};

TEST_F(AosHandleUceTest, Constructor_Test)
{
    EXPECT_FALSE(IsRegFeatureTagRequired());
}
