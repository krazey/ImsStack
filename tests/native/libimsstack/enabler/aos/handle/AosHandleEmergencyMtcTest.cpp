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

#include "handle/AosHandleEmergencyMtc.h"
#include "interface/IAosAppContext.h"

#include "interface/MockIAosAppContext.h"

using ::testing::_;
using ::testing::AnyNumber;
using ::testing::Return;
using ::testing::ReturnRef;

class AosHandleEmergencyMtcTest : public ::testing::Test
{
public:
    AosHandleEmergencyMtc* m_pAosHandleEmergencyMtc;

    MockIAosAppContext m_objMockIAosAppContext;

protected:
    virtual void SetUp() override
    {
        EXPECT_CALL(m_objMockIAosAppContext, GetSlotId())
                .Times(AnyNumber())
                .WillRepeatedly(Return(0));

        const AString strValue = AString("test");
        EXPECT_CALL(m_objMockIAosAppContext, GetProfileId())
                .Times(AnyNumber())
                .WillRepeatedly(ReturnRef(strValue));

        const AString strAppId = AString("ims.app.mtc.emergency.test");
        const AString strServiceId = AString("ims.service.mtc.emergency.test");
        const IMS_UINT32 nServiceType = -1;

        m_pAosHandleEmergencyMtc =
                new AosHandleEmergencyMtc(static_cast<IAosAppContext*>(&m_objMockIAosAppContext),
                        strAppId, strServiceId, nServiceType);

        ASSERT_TRUE(m_pAosHandleEmergencyMtc != nullptr);
    }

    virtual void TearDown() override
    {
        if (m_pAosHandleEmergencyMtc != nullptr)
        {
            delete m_pAosHandleEmergencyMtc;
            m_pAosHandleEmergencyMtc = nullptr;
        }
    }
};

TEST_F(AosHandleEmergencyMtcTest, Constructor_Test) {}

TEST_F(AosHandleEmergencyMtcTest, Destructor_Test) {}