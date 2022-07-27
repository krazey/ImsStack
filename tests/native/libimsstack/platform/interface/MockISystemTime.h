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
#ifndef MOCK_I_SYSTEM_TIME_H_
#define MOCK_I_SYSTEM_TIME_H_

#include <gmock/gmock.h>

#include "ISystemTime.h"

class MockISystemTime : public ISystemTime
{
public:
    inline MockISystemTime() {}
    inline virtual ~MockISystemTime() {}

    MOCK_METHOD(ImsDate, GetDate, (), (const, override));
    MOCK_METHOD(void, GetDate,
            (OUT IMS_SINT32 & nYear, OUT IMS_SINT32& nMonth, OUT IMS_SINT32& nDay,
                    OUT IMS_SINT32& nDayOfWeek),
            (const, override));
    MOCK_METHOD(ImsTime, GetLocalTime, (), (const, override));
    MOCK_METHOD(void, GetLocalTime,
            (OUT IMS_UINT32 & nHour, OUT IMS_UINT32& nMinute, OUT IMS_UINT32& nSecond),
            (const, override));
    MOCK_METHOD(ImsGmTime, GetGmTime, (), (const, override));
    MOCK_METHOD(void, GetGmTime,
            (OUT IMS_SINT32 & nYear, OUT IMS_SINT32& nMonth, OUT IMS_SINT32& nDay,
                    OUT IMS_UINT32& nHour, OUT IMS_UINT32& nMinute, OUT IMS_UINT32& nSecond),
            (const, override));
    MOCK_METHOD(
            IMS_UINT32, GetRandom, (IN IMS_BOOL bSeed, IN IMS_UINT32 nRange), (const, override));
    MOCK_METHOD(IMS_UINT32, GetTickCount, (), (const, override));
    MOCK_METHOD(IMS_UINT32, GetTimeInSeconds, (), (const, override));
    MOCK_METHOD(IMS_UINT32, GetTimeInMicroSeconds, (), (const, override));
    MOCK_METHOD(IMS_UINT32, GetTimeInMilliSeconds, (), (const, override));
    MOCK_METHOD(AString, GetTimeString, (), (const, override));
    MOCK_METHOD(AString, GetTimeStringEx, (), (const, override));
    MOCK_METHOD(AString, GetGmTimeString, (), (const, override));
    // "YYYY-MM-DDTHH:MM:SSZ" or "YYYY-MM-DDTHH:MM:SS+09:00"
    MOCK_METHOD(AString, GetUtcFormat, (IN IMS_BOOL bNumOffset), (const, override));
    MOCK_METHOD(void, Sleep, (IN IMS_UINT32 nMilliSeconds), (override));
    MOCK_METHOD(IMS_SLONG, GetDiffGmTime,
            (IN const AString& strBegin, IN const AString& strEnd, IN IMS_BOOL bSummerTime),
            (override));
};

#endif
