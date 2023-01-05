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
#ifndef RETRY_CONDITION_H_
#define RETRY_CONDITION_H_

#include "ImsList.h"

class RetryCode;

class RetryCondition
{
public:
    inline RetryCondition() {}
    inline ~RetryCondition() {}

    RetryCondition(IN const RetryCondition&) = delete;
    RetryCondition& operator=(IN const RetryCondition&) = delete;

public:
    IMS_BOOL Add(IN IMS_SINT32 nCode);
    IMS_BOOL Add(IN IMS_SINT32 nMinCode, IN IMS_SINT32 nMaxCode);

private:
    IMS_BOOL Verify(IN IMS_SINT32 nCode) const;

private:
    friend class RetryTaskHelper;

    ImsList<RetryCode*> m_objRetryCodes;
};

#endif
