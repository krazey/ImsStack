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

#include "Sip.h"

namespace android
{

class SipTest : public ::testing::Test
{
protected:
    virtual void SetUp() override {}

    virtual void TearDown() override {}
};

TEST_F(SipTest, IsPortSpecified)
{
    // valid values
    EXPECT_EQ(IMS_TRUE, Sip::IsPortSpecified(Sip::PORT_5060));
    EXPECT_EQ(IMS_TRUE, Sip::IsPortSpecified(Sip::PORT_5061));
    EXPECT_EQ(IMS_TRUE, Sip::IsPortSpecified(Sip::PORT_U_ENC));

    // boundary values
    EXPECT_EQ(IMS_FALSE, Sip::IsPortSpecified(0));
    EXPECT_EQ(IMS_FALSE, Sip::IsPortSpecified(Sip::PORT_UNSPECIFIED));

    // greater than the boundary values
    EXPECT_EQ(IMS_FALSE, Sip::IsPortSpecified(-1));
    EXPECT_EQ(IMS_FALSE, Sip::IsPortSpecified(65536));
}

}  // namespace android
