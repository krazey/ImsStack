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
#include "common/ImsIdentity.h"

namespace android
{

class ImsIdentityTest : public ::testing::Test
{
protected:
    virtual void SetUp() override {}

    virtual void TearDown() override {}
};

TEST_F(ImsIdentityTest, CreateSipUserIdWithPhoneWithPhoneContext)
{
    const IMS_SINT32 SLOT_ID = 0;
    AString strDialString = "#SomeDialString";
    AString strPhoneContext = "some.Phone.Context";
    AString strExpectedSipUri =
            "sip:%23SomeDialString;phone-context=some.Phone.Context@;user=phone";

    AString strUri = ImsIdentity::CreateSipUserIdWithPhone(strDialString, SLOT_ID, strPhoneContext);

    EXPECT_EQ(strExpectedSipUri, strUri);
}

TEST_F(ImsIdentityTest, CreateSipUserIdWithPhoneWithoutPhoneContext)
{
    const IMS_SINT32 SLOT_ID = 0;
    AString strDialString = "#SomeDialString";
    AString strExpectedSipUri = "sip:%23SomeDialString;phone-context=@;user=phone";

    AString strUri = ImsIdentity::CreateSipUserIdWithPhone(strDialString, SLOT_ID);

    EXPECT_EQ(strExpectedSipUri, strUri);
}

}  // namespace android
