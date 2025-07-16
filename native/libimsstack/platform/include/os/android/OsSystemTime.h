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
#ifndef OS_SYSTEM_TIME_H_
#define OS_SYSTEM_TIME_H_

#include "ISystemTime.h"

class OsSystemTime : public ISystemTime
{
public:
    OsSystemTime();
    ~OsSystemTime() override;

public:
    ImsDate GetDate() const override;
    void GetDate(OUT IMS_SINT32& nYear, OUT IMS_SINT32& nMonth, OUT IMS_SINT32& nDay,
            OUT IMS_SINT32& nDayOfWeek) const override;

    ImsTime GetLocalTime() const override;
    void GetLocalTime(
            OUT IMS_UINT32& nHour, OUT IMS_UINT32& nMinute, OUT IMS_UINT32& nSecond) const override;

    ImsGmTime GetGmTime() const override;
    void GetGmTime(OUT IMS_SINT32& nYear, OUT IMS_SINT32& nMonth, OUT IMS_SINT32& nDay,
            OUT IMS_UINT32& nHour, OUT IMS_UINT32& nMinute, OUT IMS_UINT32& nSecond) const override;

    IMS_UINT32 GetRandom(IN IMS_UINT32 nRange = 0) const override;
    IMS_UINT32 GetTickCount() const override;
    IMS_UINT32 GetTimeInSeconds() const override;
    IMS_UINT32 GetTimeInMicroSeconds() const override;
    IMS_UINT32 GetTimeInMilliSeconds() const override;
    AString GetTimeString() const override;
    AString GetTimeStringEx() const override;

    AString GetGmTimeString() const override;
    AString GetUtcFormat(IN IMS_BOOL bNumOffset = IMS_FALSE) const override;

    void Sleep(IN IMS_UINT32 nMilliSeconds) override;

    IMS_SLONG GetDiffGmTime(IN const AString& strBegin, IN const AString& strEnd,
            IN IMS_BOOL bSummerTime = IMS_FALSE) override;

public:
    static const IMS_CHAR* MONTH[];
    static const IMS_CHAR* DAY_OF_WEEK[];
};

#endif
