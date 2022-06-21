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

#include "app/MockAosAppContext.h"

#include "interface/IAosAppContext.h"
#include "provider/AosStaticProfile.h"
#include "registration/AosRegistration.h"

using ::testing::_;
using ::testing::AnyNumber;
using ::testing::Return;
using ::testing::ReturnNull;
using ::testing::ReturnRef;

const IMS_SINT32 SLOT_ID = 0;

class AosRegistrationTest : public ::testing::Test
{
public:
    AosRegistration* pAosRegistration;

    AosStaticProfile* pAosStaticProfile;
    MockAosAppContext* pMockAosAppContext;

protected:
    virtual void SetUp() override
    {
        pAosStaticProfile = new AosStaticProfile();
        pMockAosAppContext = new MockAosAppContext(pAosStaticProfile);

        EXPECT_CALL(*pMockAosAppContext, GetSlotId()).WillRepeatedly(Return(SLOT_ID));
        EXPECT_CALL(*pMockAosAppContext, GetStaticProfile())
                .Times(AnyNumber())
                .WillRepeatedly(Return(pAosStaticProfile));

        pAosRegistration = new AosRegistration(static_cast<IAosAppContext*>(pMockAosAppContext),
                pAosStaticProfile->GetRegistrationId());
    }

    virtual void TearDown() override
    {
        if (pAosStaticProfile)
        {
            delete pAosStaticProfile;
        }

        if (pMockAosAppContext)
        {
            delete pMockAosAppContext;
        }

        if (pAosRegistration)
        {
            delete pAosRegistration;
        }
    }

    void SetMode(IN IMS_UINT32 nMode) { pAosRegistration->m_nRegMode = nMode; }
};

TEST_F(AosRegistrationTest, CheckMode)
{
    SetMode(IAosRegistration::MODE_NORMAL);
    EXPECT_EQ(pAosRegistration->GetMode(), IAosRegistration::MODE_NORMAL);

    SetMode(IAosRegistration::MODE_FAKE);
    EXPECT_EQ(pAosRegistration->GetMode(), IAosRegistration::MODE_FAKE);
}