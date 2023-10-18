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
#ifndef INTERFACE_SYSTEM_TIME_H_
#define INTERFACE_SYSTEM_TIME_H_

#include "AString.h"

struct ImsDate
{
    IMS_SINT32 nYear;
    IMS_SINT32 nMonth;
    IMS_SINT32 nDay;
    IMS_SINT32 nDayOfWeek;
};

struct ImsTime
{
    IMS_UINT32 nHour;
    IMS_UINT32 nMinute;
    IMS_UINT32 nSecond;
    IMS_UINT32 nMillisecond;
};

struct ImsGmTime
{
    IMS_SINT32 nYear;
    IMS_SINT32 nMonth;
    IMS_SINT32 nDay;
    IMS_UINT32 nHour;
    IMS_UINT32 nMinute;
    IMS_UINT32 nSecond;
};

class ISystemTime
{
protected:
    virtual ~ISystemTime() = default;

public:
    virtual ImsDate GetDate() const = 0;
    virtual void GetDate(OUT IMS_SINT32& nYear, OUT IMS_SINT32& nMonth, OUT IMS_SINT32& nDay,
            OUT IMS_SINT32& nDayOfWeek) const = 0;

    virtual ImsTime GetLocalTime() const = 0;
    virtual void GetLocalTime(
            OUT IMS_UINT32& nHour, OUT IMS_UINT32& nMinute, OUT IMS_UINT32& nSecond) const = 0;

    virtual ImsGmTime GetGmTime() const = 0;
    virtual void GetGmTime(OUT IMS_SINT32& nYear, OUT IMS_SINT32& nMonth, OUT IMS_SINT32& nDay,
            OUT IMS_UINT32& nHour, OUT IMS_UINT32& nMinute, OUT IMS_UINT32& nSecond) const = 0;

    virtual IMS_UINT32 GetRandom(IN IMS_UINT32 nRange = 0) const = 0;

    virtual IMS_UINT32 GetTickCount() const = 0;

    virtual IMS_UINT32 GetTimeInSeconds() const = 0;

    virtual IMS_UINT32 GetTimeInMicroSeconds() const = 0;

    virtual IMS_UINT32 GetTimeInMilliSeconds() const = 0;

    virtual AString GetTimeString() const = 0;

    virtual AString GetTimeStringEx() const = 0;

    virtual AString GetGmTimeString() const = 0;

    // "YYYY-MM-DDTHH:MM:SSZ" or "YYYY-MM-DDTHH:MM:SS+09:00"
    virtual AString GetUtcFormat(IN IMS_BOOL bNumOffset = IMS_FALSE) const = 0;

    virtual void Sleep(IN IMS_UINT32 nMilliSeconds) = 0;

    virtual IMS_SLONG GetDiffGmTime(IN const AString& strBegin, IN const AString& strEnd,
            IN IMS_BOOL bSummerTime = IMS_FALSE) = 0;
};

#endif
