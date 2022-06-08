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

#include "SipError.h"
#include "SipManager.h"
#include "SipPrivate.h"

namespace android
{

class SipPrivateTest : public ::testing::Test
{
protected:
    virtual void SetUp() override
    {
        SipManager::GetInstance();
        m_nDefaultEncodingOptions = SipPrivate::OPTIONS_E | SipPrivate::OPT_E_FULLFORM;
        SipPrivate::Init(IMS_SLOT_0, m_nDefaultEncodingOptions);
    }

    virtual void TearDown() override {}

protected:
    IMS_SINT32 m_nDefaultEncodingOptions;
};

TEST_F(SipPrivateTest, Init)
{
    EXPECT_EQ(0, SipPrivate::GetLastError());
    EXPECT_EQ(m_nDefaultEncodingOptions, SipPrivate::GetEncodingOptions());

    IMS_SINT32 nEncodingOptions = SipPrivate::OPTIONS_E | SipPrivate::OPT_E_SHORTFORM;

    SipPrivate::Init(IMS_SLOT_0, nEncodingOptions);

    EXPECT_EQ(nEncodingOptions, SipPrivate::GetEncodingOptions());
}

TEST_F(SipPrivateTest, SetLastError)
{
    IMS_SINT32 nSipError = SipError::ILLEGAL_ARGUMENT;
    SipPrivate::SetLastError(nSipError);
    EXPECT_EQ(nSipError, SipPrivate::GetLastError());

    nSipError = SipError::NO_ERROR;
    SipPrivate::SetLastError(nSipError);
    EXPECT_EQ(nSipError, SipPrivate::GetLastError());
}

}  // namespace android
