/*
 * Copyright (C) 2023 The Android Open Source Project
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
#ifndef MOCK_I_DIGEST_AKA_LISTENER_H_
#define MOCK_I_DIGEST_AKA_LISTENER_H_

#include <gmock/gmock.h>

#include "IDigestAkaListener.h"

class MockIDigestAkaListener : public IDigestAkaListener
{
public:
    inline MockIDigestAkaListener() = default;
    inline ~MockIDigestAkaListener() = default;

    MOCK_METHOD(void, DigestAka_OnResponse,
            (IN const ByteArray& objRes, IN const ByteArray& objIk, IN const ByteArray& objCk),
            (override));

    MOCK_METHOD(void, DigestAka_OnAutsFailed, (IN const ByteArray& objAuts), (override));
    MOCK_METHOD(void, DigestAka_OnMacFailed, (), (override));
};

#endif