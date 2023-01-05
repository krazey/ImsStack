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

#include "SipTimerValues.h"

namespace android
{

class SipTimerValuesTest : public ::testing::Test
{
protected:
    virtual void SetUp() override {}

    virtual void TearDown() override {}
};

TEST_F(SipTimerValuesTest, ConstructorNAssignmentOperator)
{
    // clang-format off
    const IMS_SINT32 nTimerTypes[] = {
        SipTimerValues::TIMER_T1,
        SipTimerValues::TIMER_T2,
        SipTimerValues::TIMER_B,
        SipTimerValues::TIMER_D,
        SipTimerValues::TIMER_F,
        SipTimerValues::TIMER_H,
        SipTimerValues::TIMER_I,
        SipTimerValues::TIMER_J,
        SipTimerValues::TIMER_K,
        SipTimerValues::TIMER_ALL
    };
    // clang-format on

    SipTimerValues objStv1;
    IMS_SINT32 i = 0;

    // No flags
    while (nTimerTypes[i] != SipTimerValues::TIMER_ALL)
    {
        EXPECT_EQ(0, objStv1.GetValue(nTimerTypes[i]));
        ++i;
    }

    SipTimerValues objStv2(objStv1);

    // No flags
    i = 0;
    while (nTimerTypes[i] != SipTimerValues::TIMER_ALL)
    {
        EXPECT_EQ(0, objStv2.GetValue(nTimerTypes[i]));
        ++i;
    }

    SipTimerValues objStv3 = SipTimerValues::CreateTimerValues(500, 4000);
    SipTimerValues objStv4(objStv3);

    // All flags
    i = 0;
    while (nTimerTypes[i] != SipTimerValues::TIMER_ALL)
    {
        EXPECT_EQ(objStv3.GetValue(nTimerTypes[i]), objStv4.GetValue(nTimerTypes[i]));
        ++i;
    }
}

TEST_F(SipTimerValuesTest, GetValue)
{
    // clang-format off
    const IMS_SINT32 nTimerTypes[] = {
        SipTimerValues::TIMER_T1,
        SipTimerValues::TIMER_T2,
        SipTimerValues::TIMER_B,
        SipTimerValues::TIMER_D,
        SipTimerValues::TIMER_F,
        SipTimerValues::TIMER_H,
        SipTimerValues::TIMER_I,
        SipTimerValues::TIMER_J,
        SipTimerValues::TIMER_K,
        SipTimerValues::TIMER_ALL
    };

    const IMS_SINT32 nTimerValuesForRfc3261[] = {
        500,
        4000,
        32000,
        32000,
        32000,
        32000,
        5000,
        32000,
        5000,
        0
    };

    const IMS_SINT32 nTimerValuesFor3gpp[] = {
        2000,
        16000,
        128000,
        128000,
        128000,
        128000,
        17000,
        128000,
        17000,
        0
    };
    // clang-format on

    IMS_SINT32 i;

    // RFC 3261
    SipTimerValues objStvForRfc3261 = SipTimerValues::CreateTimerValues(500, 4000);

    i = 0;
    while (nTimerTypes[i] != SipTimerValues::TIMER_ALL)
    {
        EXPECT_EQ(nTimerValuesForRfc3261[i], objStvForRfc3261.GetValue(nTimerTypes[i]));
        ++i;
    }

    // 3GPP
    SipTimerValues objStvFor3gpp = SipTimerValues::CreateTimerValues(2000, 16000);

    i = 0;
    while (nTimerTypes[i] != SipTimerValues::TIMER_ALL)
    {
        EXPECT_EQ(nTimerValuesFor3gpp[i], objStvFor3gpp.GetValue(nTimerTypes[i]));
        ++i;
    }
}

TEST_F(SipTimerValuesTest, IsSet)
{
    // clang-format off
    const IMS_SINT32 nTimerTypes[] = {
        SipTimerValues::TIMER_T1,
        SipTimerValues::TIMER_T2,
        SipTimerValues::TIMER_B,
        SipTimerValues::TIMER_D,
        SipTimerValues::TIMER_F,
        SipTimerValues::TIMER_H,
        SipTimerValues::TIMER_I,
        SipTimerValues::TIMER_J,
        SipTimerValues::TIMER_K,
        SipTimerValues::TIMER_ALL
    };
    // clang-format on

    IMS_SINT32 i;

    // All flags
    SipTimerValues objStv1 = SipTimerValues::CreateTimerValues(500, 4000);

    i = 0;
    while (nTimerTypes[i] != SipTimerValues::TIMER_ALL)
    {
        EXPECT_EQ(IMS_TRUE, objStv1.IsSet(nTimerTypes[i]));
        ++i;
    }

    // No flags
    SipTimerValues objStv2;

    i = 0;
    while (nTimerTypes[i] != SipTimerValues::TIMER_ALL)
    {
        EXPECT_EQ(IMS_FALSE, objStv2.IsSet(nTimerTypes[i]));
        ++i;
    }
}

TEST_F(SipTimerValuesTest, SetValue)
{
    // clang-format off
    const IMS_SINT32 nTimerTypes[] = {
        SipTimerValues::TIMER_T1,
        SipTimerValues::TIMER_T2,
        SipTimerValues::TIMER_B,
        SipTimerValues::TIMER_D,
        SipTimerValues::TIMER_F,
        SipTimerValues::TIMER_H,
        SipTimerValues::TIMER_I,
        SipTimerValues::TIMER_J,
        SipTimerValues::TIMER_K,
        SipTimerValues::TIMER_ALL
    };

    const IMS_SINT32 nTimerValues[] = {
        500,
        4000,
        32000,
        32000,
        32000,
        32000,
        5000,
        32000,
        5000,
        0
    };
    // clang-format on

    IMS_SINT32 i;

    SipTimerValues objStv1;

    i = 0;
    while (nTimerTypes[i] != SipTimerValues::TIMER_ALL)
    {
        objStv1.SetValue(nTimerTypes[i], nTimerValues[i]);
        ++i;
    }

    i = 0;
    while (nTimerTypes[i] != SipTimerValues::TIMER_ALL)
    {
        EXPECT_EQ(nTimerValues[i], objStv1.GetValue(nTimerTypes[i]));
        ++i;
    }

    i = 0;
    while (nTimerTypes[i] != SipTimerValues::TIMER_ALL)
    {
        EXPECT_EQ(IMS_TRUE, objStv1.IsSet(nTimerTypes[i]));
        ++i;
    }

    SipTimerValues objStv2;

    objStv2.SetValue(SipTimerValues::TIMER_T1, 2000);
    objStv2.SetValue(SipTimerValues::TIMER_T2, 16000);
    objStv2.SetValue(SipTimerValues::TIMER_F, 128000);
    objStv2.SetValue(SipTimerValues::TIMER_K, 17000);

    // clang-format off
    const IMS_BOOL bExpectedValues2[] = {
        IMS_TRUE,
        IMS_TRUE,
        IMS_FALSE,
        IMS_FALSE,
        IMS_TRUE,
        IMS_FALSE,
        IMS_FALSE,
        IMS_FALSE,
        IMS_TRUE,
        IMS_FALSE
    };

    const IMS_SINT32 nExpectedValues2[] = {
        2000,
        16000,
        0,
        0,
        128000,
        0,
        0,
        0,
        17000,
        0
    };
    // clang-format on

    i = 0;
    while (nTimerTypes[i] != SipTimerValues::TIMER_ALL)
    {
        EXPECT_EQ(bExpectedValues2[i], objStv2.IsSet(nTimerTypes[i]));
        ++i;
    }

    i = 0;
    while (nTimerTypes[i] != SipTimerValues::TIMER_ALL)
    {
        EXPECT_EQ(nExpectedValues2[i], objStv2.GetValue(nTimerTypes[i]));
        ++i;
    }

    SipTimerValues objStv3 = SipTimerValues::CreateTimerValues(2000, 16000);

    objStv3.SetValue(SipTimerValues::TIMER_T1, 3000);
    objStv3.SetValue(SipTimerValues::TIMER_B, 16000);
    objStv3.SetValue(SipTimerValues::TIMER_F, 8000);
    objStv3.SetValue(SipTimerValues::TIMER_K, 5000);

    // clang-format off
    const IMS_SINT32 nExpectedValues3[] = {
        3000,
        16000,
        16000,
        128000,
        8000,
        128000,
        17000,
        128000,
        5000,
        0
    };
    // clang-format on

    i = 0;
    while (nTimerTypes[i] != SipTimerValues::TIMER_ALL)
    {
        EXPECT_EQ(IMS_TRUE, objStv3.IsSet(nTimerTypes[i]));
        ++i;
    }

    i = 0;
    while (nTimerTypes[i] != SipTimerValues::TIMER_ALL)
    {
        EXPECT_EQ(nExpectedValues3[i], objStv3.GetValue(nTimerTypes[i]));
        ++i;
    }
}

}  // namespace android
