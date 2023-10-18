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

#include "SipError.h"
#include "SipManager.h"
#include "SipPrivate.h"

namespace android
{

class SipErrorTest : public ::testing::Test
{
public:
    inline SipErrorTest() :
            m_pThreadService(new TestThreadService())
    {
        PlatformContext::GetInstance()->SetService(
                PlatformContext::SERVICE_THREAD, m_pThreadService);
    }
    inline virtual ~SipErrorTest()
    {
        PlatformContext::GetInstance()->SetService(PlatformContext::SERVICE_THREAD, IMS_NULL);

        if (m_pThreadService != IMS_NULL)
        {
            delete m_pThreadService;
        }
    }

protected:
    virtual void SetUp() override { SipManager::GetInstance(); }

    virtual void TearDown() override {}

protected:
    TestThreadService* m_pThreadService;
};

TEST_F(SipErrorTest, GetLastError)
{
    // clang-format off
    const IMS_SINT32 nSipErrors[] = {
        SipError::NO_ERROR,
        SipError::GENERAL_ERROR,
        SipError::CONNECTION_NOT_FOUND,
        SipError::TRANSPORT_NOT_SUPPORTED,
        SipError::DIALOG_UNAVAILABLE,
        SipError::UNKNOWN_CONTENT_TYPE,
        SipError::INVALID_STATE,
        SipError::INVALID_OPERATION,
        SipError::TRANSACTION_UNAVAILABLE,
        SipError::INVALID_MESSAGE,
        SipError::ALREADY_RESPONDED,
        SipError::CSEQ_VALUE_EXCEEDED,
        SipError::CSEQ_VALUE_MISMATCHED,
        SipError::DIALOG_NOT_EXIST,
        SipError::INVALID_SIP_ADDRESS,
        SipError::LOCAL_TAG_MISMATCH,
        SipError::LOOP_DETECTED,
        SipError::NO_MESSAGE,
        SipError::PORT_ALREADY_RESERVED,
        SipError::REQUEST_OUT_OF_ORDER,
        SipError::SECOND_REQUEST_FAILED,
        SipError::TRANSACTION_NOT_EXIST,
        SipError::TRANSACTION_TIMER_EXPIRED,
        SipError::URI_SCHEME_NOT_SUPPORTED,
        SipError::VIA_PROTOCOL_MISMATCH,
        SipError::VIA_ADDRESS_MISMATCH,
        SipError::AUTHENTICATION_FAILED,
        SipError::ILLEGAL_ARGUMENT,
        SipError::LIST_OPERATION_FAILED,
        SipError::NO_MEMORY,
        SipError::PARSING_ERROR,
        SipError::TIMER_ERROR,
        SipError::TRANSPORT_ERROR,
        0
    };
    // clang-format on

    IMS_SINT32 i = 0;

    while (nSipErrors[i] != 0)
    {
        SipPrivate::SetLastError(nSipErrors[i]);
        EXPECT_EQ(nSipErrors[i], SipError::GetLastError());
        ++i;
    }
}

}  // namespace android
