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
#ifndef RETRY_SINGLE_CODE_H_
#define RETRY_SINGLE_CODE_H_

#include "RetryCode.h"

class RetrySingleCode : public RetryCode
{
public:
    inline RetrySingleCode() :
            RetryCode(),
            m_nCode(0)
    {
    }
    inline explicit RetrySingleCode(IN IMS_SINT32 nCode) :
            RetryCode(),
            m_nCode(nCode)
    {
    }
    inline RetrySingleCode(IN const RetrySingleCode& other) :
            RetryCode(other),
            m_nCode(other.m_nCode)
    {
    }
    inline virtual ~RetrySingleCode() {}

public:
    inline RetrySingleCode& operator=(IN const RetrySingleCode& other)
    {
        if (this != &other)
        {
            m_nCode = other.m_nCode;
        }

        return (*this);
    }

public:
    inline virtual IMS_BOOL IsIn(IN IMS_SINT32 nCode) const override { return (m_nCode == nCode); }

private:
    IMS_SINT32 m_nCode;
};

#endif
