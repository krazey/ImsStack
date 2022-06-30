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

#include "registration/MockAosIpsec.h"

#include "registration/AosIpsec.h"

const IMS_SINT32 SLOT_ID = 0;

class AosIpsecTest : public ::testing::Test
{
public:
    AosIpsec* pAosIpsec;

    MockIAosIpsecListener objIAosIpsecListener;

protected:
    virtual void SetUp() override
    {
        pAosIpsec = new AosIpsec(static_cast<IAosIpsecListener*>(&objIAosIpsecListener), SLOT_ID);
        ASSERT_TRUE(pAosIpsec != nullptr);
    }

    virtual void TearDown() override
    {
        if (pAosIpsec)
        {
            delete pAosIpsec;
        }
    }
};

TEST_F(AosIpsecTest, CreateUePort)
{
    EXPECT_GE(39000, pAosIpsec->CreateUePort());
    EXPECT_LE(38001, pAosIpsec->CreateUePort());
}

TEST_F(AosIpsecTest, CreateUeSpi)
{
    EXPECT_LE(1000000000, pAosIpsec->CreateUeSpi());
}