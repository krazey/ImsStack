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

#include "PlatformContext.h"
#include "TestThreadService.h"

#include "base/Ims.h"
#include "base/ImsError.h"

using ::testing::Return;

namespace android
{

class ImsErrorTest : public ::testing::Test
{
public:
    inline ImsErrorTest() :
            m_pThreadService(new TestThreadService())
    {
        PlatformContext::GetInstance()->SetService(
                PlatformContext::SERVICE_THREAD, m_pThreadService);
    }
    inline virtual ~ImsErrorTest()
    {
        PlatformContext::GetInstance()->SetService(PlatformContext::SERVICE_THREAD, IMS_NULL);

        if (m_pThreadService != IMS_NULL)
        {
            delete m_pThreadService;
        }
    }

protected:
    virtual void SetUp() override { Ims::Init(); }

    virtual void TearDown() override {}

protected:
    TestThreadService* m_pThreadService;
};

TEST_F(ImsErrorTest, GetLastError)
{
    // Initial error code
    EXPECT_EQ(ImsError::NO_ERROR, ImsError::GetLastError());

    // clang-format off
    const IMS_SINT32 nImsErrors[] = {
        ImsError::GENERAL_ERROR,
        ImsError::SERVICE_CLOSED,
        ImsError::ILLEGAL_STATE,
        ImsError::ILLEGAL_ARGUMENT,
        ImsError::PROFILE_MISSED,
        ImsError::PROFILE_CORRUPTED,
        ImsError::AUTHENTICATION_FAILED,
        ImsError::REGISTRATION_REQUEST_TIMEOUT,
        ImsError::REGISTRATION_FAILED_RESPONSE,
        ImsError::CONNECTION_NOT_FOUND,
        ImsError::INVALID_OPERATION,
        ImsError::REFRESH_FAILED,
        ImsError::NO_MESSAGE,
        ImsError::NO_MEMORY,
        ImsError::NOT_FOUND,
        ImsError::ALREADY_EXISTS,
        ImsError::LIST_OPERATION_FAILED,
        ImsError::PARSING_ERROR,
        ImsError::NO_SIP_CONNECTION,
        0
    };
    // clang-format on

    IMS_SINT32 i = 0;

    while (nImsErrors[i] != 0)
    {
        Ims::SetLastError(nImsErrors[i]);
        EXPECT_EQ(nImsErrors[i], ImsError::GetLastError());
        ++i;
    }
}

TEST_F(ImsErrorTest, GetLastErrorString)
{
    // Initial error code
    EXPECT_STREQ(ImsError::GetString(ImsError::NO_ERROR), ImsError::GetLastErrorString());

    // clang-format off
    const IMS_SINT32 nImsErrors[] = {
        ImsError::GENERAL_ERROR,
        ImsError::SERVICE_CLOSED,
        ImsError::ILLEGAL_STATE,
        ImsError::ILLEGAL_ARGUMENT,
        ImsError::PROFILE_MISSED,
        ImsError::PROFILE_CORRUPTED,
        ImsError::AUTHENTICATION_FAILED,
        ImsError::REGISTRATION_REQUEST_TIMEOUT,
        ImsError::REGISTRATION_FAILED_RESPONSE,
        ImsError::CONNECTION_NOT_FOUND,
        ImsError::INVALID_OPERATION,
        ImsError::REFRESH_FAILED,
        ImsError::NO_MESSAGE,
        ImsError::NO_MEMORY,
        ImsError::NOT_FOUND,
        ImsError::ALREADY_EXISTS,
        ImsError::LIST_OPERATION_FAILED,
        ImsError::PARSING_ERROR,
        ImsError::NO_SIP_CONNECTION,
        0
    };
    // clang-format on

    IMS_SINT32 i = 0;

    while (nImsErrors[i] != 0)
    {
        Ims::SetLastError(nImsErrors[i]);
        EXPECT_STREQ(ImsError::GetString(nImsErrors[i]), ImsError::GetLastErrorString());
        ++i;
    }
}

}  // namespace android
