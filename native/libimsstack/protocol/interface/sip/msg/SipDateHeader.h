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
#ifndef __SIP_DATE_HEADER_H__
#define __SIP_DATE_HEADER_H__

#include "msg/SipHeaderBase.h"

class SipDateHeader : public SipHeaderBase
{
public:
    /*Enumration for week day*/
    enum
    {
        MONDAY,
        TUESDAY,
        WEDNESDAY,
        THURSDAY,
        FRIDAY,
        SATURDAY,
        SUNDAY,
        UNKNOWN_DAY
    };

    /*Enumration for month*/
    enum
    {
        JANUARY,
        FEBRUARY,
        MARCH,
        APRIL,
        MAY,
        JUNE,
        JULY,
        AUGUST,
        SEPTEMBER,
        OCTOBER,
        NOVEMBER,
        DECEMBER,
        UNKNOWN_MONTH
    };

private:
    /*Date */
    SIP_UINT16 m_nDate;
    SIP_INT32 m_eMonth;
    SIP_UINT16 m_nYear;

    /*Time*/
    SIP_UINT16 m_nHour;
    SIP_UINT16 m_nMin;
    SIP_UINT16 m_nSec;

    SIP_INT32 m_eWkDay;

public:
    /*constructor*/
    SipDateHeader();

    SipDateHeader(const SipDateHeader& objHeader);
    static SipHeaderBase* GetNewObj(SIP_INT32 eHeaderType, SipHeaderBase* pHeader);

    /*virtual methods*/
    SIP_BOOL Encode(AStringBuffer& objBuffer, SIP_BOOL bParams) const override;
    /*Function for encoding of headers*/
    SIP_BOOL EncodeHdr(SIP_CHAR** ppCurrPos, SIP_BOOL bParams = SIP_TRUE) override;

    /*Function for decoding of headers*/
    SIP_BOOL DecodeHdr(SIP_CHAR* pStartPt, SIP_UINT32 nDecLen) override;

    /*set methods*/
    SIP_BOOL SetDate(const SIP_UINT16 nDate);

    SIP_BOOL SetMonth(SIP_INT32 eMonth);

    SIP_BOOL SetYear(const SIP_UINT16 nYear);

    SIP_BOOL SetHour(const SIP_UINT16 nHour);

    SIP_BOOL SetMinute(const SIP_UINT16 nMin);

    SIP_BOOL SetSecond(const SIP_UINT16 nSec);

    SIP_BOOL SetWkDay(SIP_INT32 eWkDay);

    /*Get methods*/
    inline SIP_UINT16 GetDate() const { return m_nDate; }

    inline SIP_INT32 GetMonth() const { return m_eMonth; }

    inline SIP_UINT16 GetYear() const { return m_nYear; }

    inline SIP_UINT16 GetHour() const { return m_nHour; }

    inline SIP_UINT16 GetMinute() const { return m_nMin; }

    inline SIP_UINT16 GetSecond() const { return m_nSec; }

    inline SIP_INT32 GetWkDay() const { return m_eWkDay; }

    inline SIP_BOOL IsValidHeader() const override
    {
        return ((m_nDate == SIP_ZERO) || (m_eMonth == UNKNOWN_MONTH) || (m_nYear == SIP_ZERO) ||
                       (m_eWkDay == UNKNOWN_DAY))
                ? SIP_FALSE
                : SIP_TRUE;
    }

private:
    ~SipDateHeader();
};
#endif  //__SIP_DATE_HEADER_H__
