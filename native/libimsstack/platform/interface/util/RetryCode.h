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
#ifndef RETRY_CODE_H_
#define RETRY_CODE_H_

#include "ImsTypeDef.h"

class RetryCode
{
public:
    inline RetryCode() {}
    inline RetryCode(IN const RetryCode&) {}
    inline virtual ~RetryCode() {}

public:
    inline RetryCode& operator=(IN const RetryCode&) { return (*this); }

public:
    virtual IMS_BOOL IsIn(IN IMS_SINT32 nCode) const = 0;
};

#endif
