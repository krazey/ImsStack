/*
    Author
    <table>
    date      author                    description
    --------  --------------            ----------
    20090819  YR@                       Created
    </table>

    Description

*/

#ifndef _INTERFACE_IMS_SYSTEM_H_
#define _INTERFACE_IMS_SYSTEM_H_

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
public:
    virtual ImsDate GetDate() const = 0;
    virtual void GetDate(OUT IMS_SINT32 &nYear, OUT IMS_SINT32 &nMonth,
            OUT IMS_SINT32 &nDay, OUT IMS_SINT32 &nDayOfWeek) const = 0;

    virtual ImsTime GetLocalTime() const = 0;
    virtual void GetLocalTime(OUT IMS_UINT32 &nHour, OUT IMS_UINT32 &nMinute,
            OUT IMS_UINT32 &nSecond) const = 0;

    virtual ImsGmTime GetGmTime() const = 0;
    virtual void GetGmTime(OUT IMS_SINT32 &nYear,
            OUT IMS_SINT32 &nMonth, OUT IMS_SINT32 &nDay,
            OUT IMS_UINT32 &nHour, OUT IMS_UINT32 &nMinute, OUT IMS_UINT32 &nSecond) const = 0;

    virtual IMS_UINT32 GetRandom(IN IMS_BOOL bSeed = IMS_TRUE, IN IMS_UINT32 nRange = 0) const = 0;

    virtual IMS_UINT32 GetTickCount() const = 0;

    virtual IMS_UINT32 GetTimeInSeconds() const = 0;

    virtual IMS_UINT32 GetTimeInMicroSeconds() const = 0;

    virtual IMS_UINT32 GetTimeInMilliSeconds() const = 0;

    virtual AString GetTimeString() const = 0;

    // LGU+ Knight
    virtual AString GetTimeStringEx() const = 0;

    virtual AString GetGmTimeString() const = 0;

    // "YYYY-MM-DDTHH:MM:SSZ" or "YYYY-MM-DDTHH:MM:SS+09:00"
    virtual AString GetUtcFormat(IN IMS_BOOL bNumOffset = IMS_FALSE) const = 0;

    virtual void Sleep(IN IMS_UINT32 nMilliSeconds) = 0;

    virtual IMS_SLONG GetDiffGmTime(IN const AString& strBegin,
            IN const AString& strEnd, IN IMS_BOOL bSummerTime = IMS_FALSE) = 0;
};

#endif // _INTERFACE_IMS_SYSTEM_H_
