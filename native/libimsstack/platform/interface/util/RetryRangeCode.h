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
#ifndef RETRY_RANGE_CODE_H_
#define RETRY_RANGE_CODE_H_

#include "RetryCode.h"

class RetryRangeCode : public RetryCode
{
public:
    inline RetryRangeCode() :
            RetryCode(),
            m_nMin(0),
            m_nMax(0)
    {
    }
    inline RetryRangeCode(IN IMS_SINT32 nMin, IN IMS_SINT32 nMax) :
            RetryCode(),
            m_nMin(nMin),
            m_nMax(nMax)
    {
    }
    inline RetryRangeCode(IN const RetryRangeCode& other) :
            RetryCode(other),
            m_nMin(other.m_nMin),
            m_nMax(other.m_nMax)
    {
    }
    inline virtual ~RetryRangeCode() {}

public:
    inline RetryRangeCode& operator=(IN const RetryRangeCode& other)
    {
        if (this != &other)
        {
            m_nMin = other.m_nMin;
            m_nMax = other.m_nMax;
        }

        return (*this);
    }

public:
    inline virtual IMS_BOOL IsIn(IN IMS_SINT32 nCode) const override
    {
        return ((nCode > m_nMin) && (nCode < m_nMax));
    }

private:
    // Its value is exclusive
    IMS_SINT32 m_nMin;
    IMS_SINT32 m_nMax;
};

#endif
