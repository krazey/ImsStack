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
#include <limits>
#include <random>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

#include "OsSystemTime.h"

#define STRUCT_TM_INIT                                                                   \
    {                                                                                    \
        .tm_sec = 0, .tm_min = 0, .tm_hour = 0, .tm_mday = 0, .tm_mon = 0, .tm_year = 0, \
        .tm_wday = 0, .tm_yday = 0, .tm_isdst = 0, .tm_gmtoff = 0,                       \
    }

// clang-format off
PRIVATE GLOBAL const IMS_CHAR* OsSystemTime::MONTH[] = {
        "Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec",
};

PRIVATE GLOBAL const IMS_CHAR* OsSystemTime::DAY_OF_WEEK[] = {
        "Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat",
};
// clang-format on

PUBLIC
OsSystemTime::OsSystemTime() {}

PUBLIC VIRTUAL OsSystemTime::~OsSystemTime() {}

PUBLIC VIRTUAL ImsDate OsSystemTime::GetDate() const
{
    ImsDate stDate;

    GetDate(stDate.nYear, stDate.nMonth, stDate.nDay, stDate.nDayOfWeek);

    return stDate;
}

PUBLIC VIRTUAL void OsSystemTime::GetDate(OUT IMS_SINT32& nYear, OUT IMS_SINT32& nMonth,
        OUT IMS_SINT32& nDay, OUT IMS_SINT32& nDayOfWeek) const
{
    time_t now = 0;
    struct tm stTm = STRUCT_TM_INIT;

    time(&now);
    struct tm* pstTm = localtime_r(&now, &stTm);

    if (pstTm != IMS_NULL)
    {
        nYear = (pstTm->tm_year) + 1900;
        nMonth = pstTm->tm_mon + 1;
        nDay = pstTm->tm_mday;
        nDayOfWeek = (pstTm->tm_wday) - 1;
    }
}

PUBLIC VIRTUAL ImsTime OsSystemTime::GetLocalTime() const
{
    IMS_UINT32 nHour = 0;
    IMS_UINT32 nMinute = 0;
    IMS_UINT32 nSecond = 0;
    ImsTime stTime = {0, 0, 0, 0};

    /* Get the local time */
    GetLocalTime(nHour, nMinute, nSecond);

    stTime.nHour = nHour;
    stTime.nMinute = nMinute;
    stTime.nSecond = nSecond;
    stTime.nMillisecond = GetTimeInMilliSeconds();

    return stTime;
}

PUBLIC VIRTUAL void OsSystemTime::GetLocalTime(
        OUT IMS_UINT32& nHour, OUT IMS_UINT32& nMinute, OUT IMS_UINT32& nSecond) const
{
    time_t now = 0;
    struct tm stTm = STRUCT_TM_INIT;

    time(&now);
    struct tm* pstTm = localtime_r(&now, &stTm);

    if (pstTm != IMS_NULL)
    {
        nHour = pstTm->tm_hour;
        nMinute = pstTm->tm_min;
        nSecond = pstTm->tm_sec;
    }
}

PUBLIC VIRTUAL ImsGmTime OsSystemTime::GetGmTime() const
{
    ImsGmTime stGmTime = {0, 0, 0, 0, 0, 0};
    time_t now = 0;
    struct tm stTm = STRUCT_TM_INIT;

    time(&now);
    struct tm* pstTm = gmtime_r(&now, &stTm);

    if (pstTm == IMS_NULL)
    {
        return stGmTime;
    }

    stGmTime.nYear = (pstTm->tm_year) + 1900;
    stGmTime.nMonth = pstTm->tm_mon + 1;
    stGmTime.nDay = pstTm->tm_mday;
    stGmTime.nHour = pstTm->tm_hour;
    stGmTime.nMinute = pstTm->tm_min;
    stGmTime.nSecond = pstTm->tm_sec;

    return stGmTime;
}

PUBLIC VIRTUAL void OsSystemTime::GetGmTime(OUT IMS_SINT32& nYear, OUT IMS_SINT32& nMonth,
        OUT IMS_SINT32& nDay, OUT IMS_UINT32& nHour, OUT IMS_UINT32& nMinute,
        OUT IMS_UINT32& nSecond) const
{
    time_t now = 0;
    struct tm stTm = STRUCT_TM_INIT;

    time(&now);
    struct tm* pstTm = gmtime_r(&now, &stTm);

    if (pstTm == IMS_NULL)
    {
        nYear = 0;
        nMonth = 0;
        nDay = 0;
        nHour = 0;
        nMinute = 0;
        nSecond = 0;
    }
    else
    {
        nYear = (pstTm->tm_year) + 1900;
        nMonth = pstTm->tm_mon + 1;
        nDay = pstTm->tm_mday;
        nHour = pstTm->tm_hour;
        nMinute = pstTm->tm_min;
        nSecond = pstTm->tm_sec;
    }
}

PUBLIC VIRTUAL IMS_UINT32 OsSystemTime::GetRandom(IN IMS_UINT32 nRange /*= 0*/) const
{
    std::random_device rd;
    std::mt19937 rng(rd());
    std::uniform_int_distribution<IMS_UINT32> dist(0, std::numeric_limits<IMS_SINT32>::max());

    if (nRange == 0)
    {
        return dist(rng);
    }
    else
    {
        return dist(rng) % nRange;
    }
}

PUBLIC VIRTUAL IMS_UINT32 OsSystemTime::GetTickCount() const
{
    struct timeval stTimeval = {0, 0};

    gettimeofday(&stTimeval, NULL);

    return stTimeval.tv_usec;
}

PUBLIC VIRTUAL IMS_UINT32 OsSystemTime::GetTimeInSeconds() const
{
    struct timeval stTimeval = {0, 0};

    gettimeofday(&stTimeval, NULL);

    return stTimeval.tv_sec;
}

PUBLIC VIRTUAL IMS_UINT32 OsSystemTime::GetTimeInMicroSeconds() const
{
    /* Get the current time and calculate it in micro-seconds */
    struct timeval stTimeval = {0, 0};

    gettimeofday(&stTimeval, NULL);

    return stTimeval.tv_usec;
}

PUBLIC VIRTUAL IMS_UINT32 OsSystemTime::GetTimeInMilliSeconds() const
{
    /* Get the current time and calculate it in milli-seconds */
    struct timeval stTimeval = {0, 0};

    gettimeofday(&stTimeval, NULL);

    return (stTimeval.tv_usec / 1000);
}

PUBLIC VIRTUAL AString OsSystemTime::GetTimeString() const
{
    time_t now = 0;
    struct tm stTm = STRUCT_TM_INIT;
    AString strTime;

    time(&now);
    struct tm* pTm = localtime_r(&now, &stTm);

    /*
     * Convert the local time to the formatted time string.
     * For example, Wed Jan 02 02:03:55 1980\0 (25 characters including '\0').
     */
    if (pTm != IMS_NULL)
    {
        strTime.Sprintf("%s %s %2.2d %2.2d:%2.2d:%2.2d %4d",
                ((pTm->tm_wday >= 0) && (pTm->tm_wday < 7)) ? DAY_OF_WEEK[pTm->tm_wday] : "null",
                ((pTm->tm_mon >= 0) && (pTm->tm_mon < 12)) ? MONTH[pTm->tm_mon] : "null",
                pTm->tm_mday, pTm->tm_hour, pTm->tm_min, pTm->tm_sec, (pTm->tm_year) + 1900);
    }

    return strTime;
}

PUBLIC VIRTUAL AString OsSystemTime::GetTimeStringEx() const
{
    AString strTime;
    time_t now = 0;

    time(&now);
    struct tm* pTm = localtime(&now);

    /*
     * Convert the local time to the formatted time string.
     * For example, YYYYMMDDHHMMSS
     */
    if (pTm != IMS_NULL)
    {
        strTime.Sprintf("%4d%2.2d%2.2d%2.2d%2.2d%2.2d", pTm->tm_year + 1900, pTm->tm_mon + 1,
                pTm->tm_mday, pTm->tm_hour, pTm->tm_min, pTm->tm_sec);
    }

    return strTime;
}

PUBLIC VIRTUAL AString OsSystemTime::GetGmTimeString() const
{
    time_t now = 0;
    struct tm stTm = STRUCT_TM_INIT;
    AString strTime;

    time(&now);
    struct tm* pstTm = gmtime_r(&now, &stTm);

    if (pstTm != IMS_NULL)
    {
        // Example : 1985-04-12T23:20:50.52Z (RFC3339)
        IMS_UINT32 nMilliSec = GetTimeInMilliSeconds();
        AString strDot = ".";
        AString strMilliSec = "";
        strMilliSec.SetNumber(nMilliSec);

        if (strMilliSec.GetLength() > 2)
        {
            strMilliSec = strMilliSec.Left(2);
        }
        else if (strMilliSec.GetLength() == 1)
        {
            strMilliSec.Prepend('0');
        }

        if (strMilliSec.GetLength() <= 0)
        {
            strMilliSec = "00";
        }

        strTime.Sprintf("%d-%2.2d-%2.2dT%2.2d:%2.2d:%2.2d%s%sZ", pstTm->tm_year + 1900,
                pstTm->tm_mon + 1, pstTm->tm_mday, pstTm->tm_hour, pstTm->tm_min, pstTm->tm_sec,
                strDot.GetStr(), strMilliSec.GetStr());
    }

    return strTime;
}

PUBLIC VIRTUAL AString OsSystemTime::GetUtcFormat(IN IMS_BOOL bNumOffset /*= IMS_FALSE*/) const
{
    time_t now = 0;
    struct tm stTm = STRUCT_TM_INIT;
    AString strUtcFormat(AString::ConstEmpty());

    time(&now);

    if (!bNumOffset)
    {
        struct tm* pstTm = gmtime_r(&now, &stTm);

        if (pstTm != IMS_NULL)
        {
            strUtcFormat.Sprintf("%d-%2.2d-%2.2dT%2.2d:%2.2d:%2.2dZ", pstTm->tm_year + 1900,
                    pstTm->tm_mon + 1, pstTm->tm_mday, pstTm->tm_hour, pstTm->tm_min,
                    pstTm->tm_sec);
        }
    }
    else
    {
        struct tm* pstTm = localtime_r(&now, &stTm);

        if (pstTm != IMS_NULL)
        {
            IMS_SLONG nGmtOffset = pstTm->tm_gmtoff;
            IMS_SLONG nTempOffset = nGmtOffset;
            IMS_SINT32 nHH = 0;
            IMS_SINT32 nMM = 0;

            if (nTempOffset < 0)
            {
                nTempOffset *= (-1);
            }

            if (nTempOffset != 0)
            {
                // Converts a second to a minute
                nTempOffset = nTempOffset / 60;

                nHH = LONG_TO_INT(nTempOffset / 60);
                nMM = LONG_TO_INT(nTempOffset % 60);
            }

            strUtcFormat.Sprintf("%d-%2.2d-%2.2dT%2.2d:%2.2d:%2.2d%s%2.2d:%2.2d",
                    pstTm->tm_year + 1900, pstTm->tm_mon + 1, pstTm->tm_mday, pstTm->tm_hour,
                    pstTm->tm_min, pstTm->tm_sec, (nGmtOffset < 0) ? "-" : "+", nHH, nMM);
        }
    }

    return strUtcFormat;
}

PUBLIC VIRTUAL void OsSystemTime::Sleep(IN IMS_UINT32 nMilliSeconds)
{
    if (nMilliSeconds == 0)
    {
        return;
    }

    // Convert milli-seconds to micro-seconds
    usleep(1000 * nMilliSeconds);
}

PUBLIC VIRTUAL IMS_SLONG OsSystemTime::GetDiffGmTime(IN const AString& strBegin,
        IN const AString& strEnd, IN IMS_BOOL bSummerTime /*= IMS_FALSE*/)
{
    struct tm tmBegin = STRUCT_TM_INIT;
    strptime(strBegin.GetStr(), "%Y-%m-%dT%H:%M:%SZ", &tmBegin);
    tmBegin.tm_isdst = bSummerTime == IMS_TRUE ? 1 : 0;

    time_t tBegin = mktime(&tmBegin);

    struct tm tmEnd = STRUCT_TM_INIT;
    strptime(strEnd.GetStr(), "%Y-%m-%dT%H:%M:%SZ", &tmEnd);
    tmEnd.tm_isdst = bSummerTime == IMS_TRUE ? 1 : 0;
    time_t tEnd = mktime(&tmEnd);

    return difftime(tBegin, tEnd);
}
