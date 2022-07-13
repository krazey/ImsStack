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
#include "utility/MtsStrName.h"

namespace android
{

class MtsStrNameTest : public ::testing::Test
{
public:
    MtsStrName* pMtsStrName;

protected:
    virtual void SetUp() override { pMtsStrName = new MtsStrName(); }

    virtual void TearDown() override { delete pMtsStrName; }
};

TEST_F(MtsStrNameTest, Constructor)
{
    ASSERT_NE(pMtsStrName, nullptr);
}

TEST_F(MtsStrNameTest, GetMtsAppId)
{
    EXPECT_STREQ(pMtsStrName->GetMtsAppId().GetStr(), "ims.app.mts");
}

TEST_F(MtsStrNameTest, GetMtsServiceId)
{
    EXPECT_STREQ(pMtsStrName->GetMtsServiceId().GetStr(), "ims.service.mts");
}

TEST_F(MtsStrNameTest, GetMtsConnectorName)
{
    EXPECT_STREQ(pMtsStrName->GetMtsConnectorName().GetStr(),
            "imscore://ims.app.mts;serviceId=ims.service.mts");
}

}  // namespace android
