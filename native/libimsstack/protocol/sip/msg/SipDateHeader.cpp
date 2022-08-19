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
#include "msg/SipDateHeader.h"
#include "platform/sip_pf_string.h"
#include "sip_debug.h"
#include "msg/sip_msgutil.h"
#include "platform/sip_pf_string.h"

#define MAX_WEEK_LEN  3
#define MAX_MONTH_LEN 3
#define STR_GMT       "GMT"
#define DATE_VAL      31
#define YEAR_VAL      1000

const SIP_UINT16 TIME_VAL = 60;
const SIP_UINT16 HOUR_VAL = 24;
const SIP_CHAR gaszWeekday[][MAX_WEEK_LEN + SIP_ONE] = {
        "Mon", "Tue", "Wed", "Thu", "Fri", "Sat", "Sun"};
const SIP_CHAR gaszMonth[][MAX_MONTH_LEN + SIP_ONE] = {
        "Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};

SIP_INT32 sipGetWeekDayType(SIP_CHAR* pszWeekDay)
{
    switch (pszWeekDay[0])
    {
        case 'M':
            if (SipPf_Strcmp(pszWeekDay, gaszWeekday[SipDateHeader::MONDAY]) == 0)
            {
                return SipDateHeader::MONDAY;
            }
            break;

        case 'T':
            if (SipPf_Strcmp(pszWeekDay, gaszWeekday[SipDateHeader::TUESDAY]) == 0)
            {
                return SipDateHeader::TUESDAY;
            }
            else if (SipPf_Strcmp(pszWeekDay, gaszWeekday[SipDateHeader::THURSDAY]) == 0)
            {
                return SipDateHeader::THURSDAY;
            }
            break;
        case 'W':
            if (SipPf_Strcmp(pszWeekDay, gaszWeekday[SipDateHeader::WEDNESDAY]) == 0)
            {
                return SipDateHeader::WEDNESDAY;
            }
            break;
        case 'F':
            if (SipPf_Strcmp(pszWeekDay, gaszWeekday[SipDateHeader::FRIDAY]) == 0)
            {
                return SipDateHeader::FRIDAY;
            }
            break;
        case 'S':
            if (SipPf_Strcmp(pszWeekDay, gaszWeekday[SipDateHeader::SATURDAY]) == 0)
            {
                return SipDateHeader::SATURDAY;
            }
            else if (SipPf_Strcmp(pszWeekDay, gaszWeekday[SipDateHeader::SUNDAY]) == 0)
            {
                return SipDateHeader::SUNDAY;
            }
            break;
        default:
            break;
    }
    return SipDateHeader::UNKNOWN_DAY;
}

SIP_INT32 sipGetMonthType(SIP_CHAR* pszMonth)
{
    switch (pszMonth[0])
    {
        case 'J':
            if (SipPf_Strcmp(pszMonth, gaszMonth[SipDateHeader::JANUARY]) == 0)
            {
                return SipDateHeader::JANUARY;
            }
            else if (SipPf_Strcmp(pszMonth, gaszMonth[SipDateHeader::JUNE]) == 0)
            {
                return SipDateHeader::JUNE;
            }
            else if (SipPf_Strcmp(pszMonth, gaszMonth[SipDateHeader::JULY]) == 0)
            {
                return SipDateHeader::JULY;
            }
            break;
        case 'F':
            if (SipPf_Strcmp(pszMonth, gaszMonth[SipDateHeader::FEBRUARY]) == 0)
            {
                return SipDateHeader::FEBRUARY;
            }
            break;
        case 'M':
            if (SipPf_Strcmp(pszMonth, gaszMonth[SipDateHeader::MARCH]) == 0)
            {
                return SipDateHeader::MARCH;
            }
            else if (SipPf_Strcmp(pszMonth, gaszMonth[SipDateHeader::MAY]) == 0)
            {
                return SipDateHeader::MAY;
            }
            break;
        case 'A':
            if (SipPf_Strcmp(pszMonth, gaszMonth[SipDateHeader::APRIL]) == 0)
            {
                return SipDateHeader::APRIL;
            }
            else if (SipPf_Strcmp(pszMonth, gaszMonth[SipDateHeader::AUGUST]) == 0)
            {
                return SipDateHeader::AUGUST;
            }
            break;
        case 'S':
            if (SipPf_Strcmp(pszMonth, gaszMonth[SipDateHeader::SEPTEMBER]) == 0)
            {
                return SipDateHeader::SEPTEMBER;
            }
            break;

        case 'O':
            if (SipPf_Strcmp(pszMonth, gaszMonth[SipDateHeader::OCTOBER]) == 0)
            {
                return SipDateHeader::OCTOBER;
            }
            break;
        case 'N':
            if (SipPf_Strcmp(pszMonth, gaszMonth[SipDateHeader::NOVEMBER]) == 0)
            {
                return SipDateHeader::NOVEMBER;
            }
            break;
        case 'D':
            if (SipPf_Strcmp(pszMonth, gaszMonth[SipDateHeader::DECEMBER]) == 0)
            {
                return SipDateHeader::DECEMBER;
            }
            break;

        default:
            break;
    }
    return SipDateHeader::UNKNOWN_MONTH;
}

SipDateHeader::SipDateHeader() :
        SipHeaderBase(SipHeaderBase::DATE),
        m_nDate(SIP_ZERO),
        m_eMonth(SipDateHeader::UNKNOWN_MONTH),
        m_nYear(SIP_ZERO),
        m_nHour(SIP_ZERO),
        m_nMin(SIP_ZERO),
        m_nSec(SIP_ZERO),
        m_eWkDay(SipDateHeader::UNKNOWN_DAY)
{
}

SipDateHeader::~SipDateHeader() {}

SipDateHeader::SipDateHeader(const SipDateHeader& objHeader) :
        SipHeaderBase(objHeader),
        m_nDate(objHeader.m_nDate),
        m_eMonth(objHeader.m_eMonth),
        m_nYear(objHeader.m_nYear),
        m_nHour(objHeader.m_nHour),
        m_nMin(objHeader.m_nMin),
        m_nSec(objHeader.m_nSec),
        m_eWkDay(objHeader.m_eWkDay)
{
}

SIP_BOOL SipDateHeader::Encode(AStringBuffer& objBuffer, SIP_BOOL /*bParams*/) const
{
    if (IsValidHeader() == SIP_FALSE)
    {
        SIP_DEBUG_WARNING(
                ESIPTRACE_MODENCODER, "EncodeHdr: invalid date Values ", SIP_ZERO, SIP_ZERO);
        return SIP_FALSE;
    }

    // Date: Thu, 21 Feb 2002 13:02:03 GMT
    objBuffer += gaszWeekday[m_eWkDay];
    objBuffer += COMMA;
    objBuffer += SPACE;

    AString strDateTime;
    strDateTime.Sprintf("%02u %s %4u", m_nDate, gaszMonth[m_eMonth], m_nYear);

    objBuffer += strDateTime;
    objBuffer += SPACE;

    strDateTime.Sprintf("%02u:%02u:%02u", m_nHour, m_nMin, m_nSec);

    objBuffer += strDateTime;
    objBuffer += SPACE;
    objBuffer += STR_GMT;

    return SIP_TRUE;
}

SIP_BOOL SipDateHeader::EncodeHdr(SIP_CHAR** ppCurrPos, SIP_BOOL /*bParams = SIP_TRUE*/)
{
    if (IsValidHeader() == SIP_FALSE)
    {
        SIP_DEBUG_WARNING(
                ESIPTRACE_MODENCODER, "EncodeHdr: invalid date Values ", SIP_ZERO, SIP_ZERO);
        return SIP_FALSE;
    }

    // Date: Thu, 21 Feb 2002 13:02:03 GMT
    SipPf_Strcpy(*ppCurrPos, gaszWeekday[m_eWkDay]);
    SipEnc_UpdateCurrPos(ppCurrPos);

    **ppCurrPos = COMMA;
    (*ppCurrPos)++;

    **ppCurrPos = SPACE;
    (*ppCurrPos)++;

    SipPf_Sprintf(*ppCurrPos, (SIP_CHAR*)"%02u %s %4u", m_nDate, gaszMonth[m_eMonth], m_nYear);
    SipEnc_UpdateCurrPos(ppCurrPos);

    **ppCurrPos = SPACE;
    (*ppCurrPos)++;

    SipPf_Sprintf(*ppCurrPos, (SIP_CHAR*)"%02u:%02u:%02u", m_nHour, m_nMin, m_nSec);
    SipEnc_UpdateCurrPos(ppCurrPos);

    **ppCurrPos = SPACE;
    (*ppCurrPos)++;

    SipPf_Strcpy(*ppCurrPos, STR_GMT);
    SipEnc_UpdateCurrPos(ppCurrPos);

    return SIP_TRUE;
}

SIP_BOOL SipDateHeader::DecodeHdr(SIP_CHAR* pStartPt, SIP_UINT32 nDecLen)
{
    /*Date: Thu, 21 Feb 2002 13:02:03 GMT*/

    if (nDecLen == SIP_ZERO)
    {
        SIP_DEBUG_WARNING(ESIPTRACE_MODDECODER, "Empty buffer", SIP_ZERO, SIP_ZERO);
        return SIP_FALSE;
    }

    SIP_CHAR* pEndPt = pStartPt + nDecLen - SIP_ONE;
    SIP_CHAR* pTempPos = SIP_NULL;

    if (SipFindPreDelimiter(pStartPt, pEndPt, &pTempPos, COMMA) == SIP_FALSE)
    {
        SIP_DEBUG_WARNING(ESIPTRACE_MODDECODER, "Invalid Date Format", SIP_ZERO, SIP_ZERO);
        return SIP_FALSE;
    }

    SIP_CHAR* pszTempValue = SipCreateString(pStartPt, pTempPos);
    if (pszTempValue == SIP_NULL)
    {
        SIP_DEBUG_WARNING(ESIPTRACE_MODDECODER, "Memory Allocation failed", SIP_ZERO, SIP_ZERO);
        return SIP_FALSE;
    }

    m_eWkDay = sipGetWeekDayType(pszTempValue);
    if (m_eWkDay == SipDateHeader::UNKNOWN_DAY)
    {
        SIP_DEBUG_WARNING(ESIPTRACE_MODDECODER, "Invalid week day", SIP_ZERO, SIP_ZERO);
        delete[] pszTempValue;
        return SIP_FALSE;
    }
    delete[] pszTempValue;

    if (*(pTempPos + SIP_TWO) != SPACE)
    {
        SIP_DEBUG_WARNING(ESIPTRACE_MODDECODER, "Invalid Date Format", SIP_ZERO, SIP_ZERO);
        return SIP_FALSE;
    }

    /*Skip comma and a space and point to start of date */
    pStartPt = pTempPos + SIP_THREE;
    pTempPos = SIP_NULL;

    if (SipFindPreDelimiter(pStartPt, pEndPt, &pTempPos, SPACE) == SIP_FALSE)
    {
        SIP_DEBUG_WARNING(ESIPTRACE_MODDECODER, "Invalid Date Format", SIP_ZERO, SIP_ZERO);
        return SIP_FALSE;
    }

    /*Get the value of date*/
    pszTempValue = SipCreateString(pStartPt, pTempPos);
    if (pszTempValue == SIP_NULL)
    {
        SIP_DEBUG_WARNING(ESIPTRACE_MODDECODER, "Memory Allocation failed", SIP_ZERO, SIP_ZERO);
        return SIP_FALSE;
    }

    m_nDate = SipPf_Atoi(pszTempValue);
    delete[] pszTempValue;

    if (m_nDate > DATE_VAL)
    {
        SIP_DEBUG_WARNING(ESIPTRACE_MODDECODER, "Invalid Date", SIP_ZERO, SIP_ZERO);
        return SIP_FALSE;
    }

    /*Update the start point to the start of Month*/
    pStartPt = pTempPos + SIP_TWO;
    pTempPos = SIP_NULL;

    if (SipFindPreDelimiter(pStartPt, pEndPt, &pTempPos, SPACE) == SIP_FALSE)
    {
        SIP_DEBUG_WARNING(ESIPTRACE_MODDECODER, "Invalid Date Format", SIP_ZERO, SIP_ZERO);
        return SIP_FALSE;
    }

    pszTempValue = SipCreateString(pStartPt, pTempPos);
    if (pszTempValue == SIP_NULL)
    {
        SIP_DEBUG_WARNING(ESIPTRACE_MODDECODER, "Memory Allocation failed", SIP_ZERO, SIP_ZERO);
        return SIP_FALSE;
    }

    m_eMonth = sipGetMonthType(pszTempValue);
    if (m_eMonth == SipDateHeader::UNKNOWN_MONTH)
    {
        SIP_DEBUG_WARNING(ESIPTRACE_MODDECODER, "Invalid month", SIP_ZERO, SIP_ZERO);
        delete[] pszTempValue;
        return SIP_FALSE;
    }
    delete[] pszTempValue;

    /*Update the start point to the start of year*/
    pStartPt = pTempPos + SIP_TWO;
    pTempPos = SIP_NULL;

    if (SipFindPreDelimiter(pStartPt, pEndPt, &pTempPos, SPACE) == SIP_FALSE)
    {
        SIP_DEBUG_WARNING(ESIPTRACE_MODDECODER, "Invalid Date Format", SIP_ZERO, SIP_ZERO);
        return SIP_FALSE;
    }

    pszTempValue = SipCreateString(pStartPt, pTempPos);
    if (pszTempValue == SIP_NULL)
    {
        SIP_DEBUG_WARNING(ESIPTRACE_MODDECODER, "Memory Allocation failed", SIP_ZERO, SIP_ZERO);
        return SIP_FALSE;
    }

    SIP_UINT16 nTempLen = SipPf_Strlen(pszTempValue);
    if (nTempLen != SIP_FOUR)
    {
        SIP_DEBUG_WARNING(ESIPTRACE_MODDECODER, "Invalid Year", SIP_ZERO, SIP_ZERO);
        delete[] pszTempValue;
        return SIP_FALSE;
    }

    m_nYear = SipPf_Atoi(pszTempValue);
    delete[] pszTempValue;

    /*Update the start point to the start of Time(Hour)*/
    pStartPt = pTempPos + SIP_TWO;
    pTempPos = SIP_NULL;

    if (SipFindPreDelimiter(pStartPt, pEndPt, &pTempPos, COLON) == SIP_FALSE)
    {
        SIP_DEBUG_WARNING(ESIPTRACE_MODDECODER, "Invalid Date Format", SIP_ZERO, SIP_ZERO);
        return SIP_FALSE;
    }

    pszTempValue = SipCreateString(pStartPt, pTempPos);
    if (pszTempValue == SIP_NULL)
    {
        SIP_DEBUG_WARNING(ESIPTRACE_MODDECODER, "Memory Allocation failed", SIP_ZERO, SIP_ZERO);
        return SIP_FALSE;
    }

    m_nHour = SipPf_Atoi(pszTempValue);
    delete[] pszTempValue;

    if (m_nHour >= HOUR_VAL)
    {
        SIP_DEBUG_WARNING(ESIPTRACE_MODDECODER, "Invalid Hour", SIP_ZERO, SIP_ZERO);
        return SIP_FALSE;
    }

    /*Update the start point to the start of Time(Min)*/
    pStartPt = pTempPos + SIP_TWO;
    pTempPos = SIP_NULL;

    if (SipFindPreDelimiter(pStartPt, pEndPt, &pTempPos, COLON) == SIP_FALSE)
    {
        SIP_DEBUG_WARNING(ESIPTRACE_MODDECODER, "Invalid Date Format", SIP_ZERO, SIP_ZERO);
        return SIP_FALSE;
    }

    pszTempValue = SipCreateString(pStartPt, pTempPos);
    if (pszTempValue == SIP_NULL)
    {
        SIP_DEBUG_WARNING(ESIPTRACE_MODDECODER, "Memory Allocation fail", SIP_ZERO, SIP_ZERO);
        return SIP_FALSE;
    }
    m_nMin = SipPf_Atoi(pszTempValue);
    delete[] pszTempValue;

    if (m_nMin >= TIME_VAL)
    {
        SIP_DEBUG_WARNING(ESIPTRACE_MODDECODER, "Invalid Minute Value", SIP_ZERO, SIP_ZERO);
        return SIP_FALSE;
    }

    /*Update the start point to the start of Time(Sec)*/
    pStartPt = pTempPos + SIP_TWO;
    pTempPos = SIP_NULL;

    /*Check validity of Sec*/
    if (SipFindPreDelimiter(pStartPt, pEndPt, &pTempPos, SPACE) == SIP_FALSE)
    {
        SIP_DEBUG_WARNING(ESIPTRACE_MODDECODER, "Invalid Date Format", SIP_ZERO, SIP_ZERO);
        return SIP_FALSE;
    }

    pszTempValue = SipCreateString(pStartPt, pTempPos);
    if (pszTempValue == SIP_NULL)
    {
        SIP_DEBUG_WARNING(ESIPTRACE_MODDECODER, "Memory Allocation fail", SIP_ZERO, SIP_ZERO);
        return SIP_FALSE;
    }

    m_nSec = SipPf_Atoi(pszTempValue);
    delete[] pszTempValue;
    if (m_nSec >= TIME_VAL)
    {
        SIP_DEBUG_WARNING(ESIPTRACE_MODDECODER, "Invalid Second Value", SIP_ZERO, SIP_ZERO);
        return SIP_FALSE;
    }

    /*Check for GMT*/
    pStartPt = pTempPos + SIP_TWO;

    SIP_CHAR* pszTempGMT = SipCreateString(pStartPt, pEndPt);
    if ((pszTempGMT != SIP_NULL) && (SipPf_Strcmp(pszTempGMT, "GMT") != 0))
    {
        SIP_DEBUG_WARNING(ESIPTRACE_MODDECODER, "GMT not matching", SIP_ZERO, SIP_ZERO);
        delete[] pszTempGMT;
        return SIP_FALSE;
    }
    else if (pszTempGMT != SIP_NULL)
    {
        delete[] pszTempGMT;
    }

    return SIP_TRUE;
}

SIP_BOOL SipDateHeader::SetDate(const SIP_UINT16 nDate)
{
    if ((nDate <= DATE_VAL) && (nDate > SIP_ZERO))
    {
        m_nDate = nDate;
        return SIP_TRUE;
    }
    return SIP_FALSE;
}

SIP_BOOL SipDateHeader::SetMonth(SIP_INT32 eMonth)
{
    if ((eMonth >= SipDateHeader::JANUARY) && (eMonth < SipDateHeader::UNKNOWN_MONTH))
    {
        m_eMonth = eMonth;
        return SIP_TRUE;
    }
    return SIP_FALSE;
}

SIP_BOOL SipDateHeader::SetYear(const SIP_UINT16 nYear)
{
    if (nYear >= YEAR_VAL)
    {
        m_nYear = nYear;
        return SIP_TRUE;
    }
    return SIP_FALSE;
}

SIP_BOOL SipDateHeader::SetHour(const SIP_UINT16 nHour)
{
    if (nHour < HOUR_VAL)
    {
        m_nHour = nHour;
        return SIP_TRUE;
    }
    return SIP_FALSE;
}

SIP_BOOL SipDateHeader::SetMinute(const SIP_UINT16 nMin)
{
    if (nMin < TIME_VAL)
    {
        m_nMin = nMin;
        return SIP_TRUE;
    }
    return SIP_FALSE;
}

SIP_BOOL SipDateHeader::SetSecond(const SIP_UINT16 nSec)
{
    if (nSec < TIME_VAL)
    {
        m_nSec = nSec;
        return SIP_TRUE;
    }
    return SIP_FALSE;
}

SIP_BOOL SipDateHeader::SetWkDay(SIP_INT32 eWkDay)
{
    if ((eWkDay >= SipDateHeader::MONDAY) && (eWkDay < SipDateHeader::UNKNOWN_DAY))
    {
        m_eWkDay = eWkDay;
        return SIP_TRUE;
    }
    return SIP_FALSE;
}

SipHeaderBase* SipDateHeader::GetNewObj(SIP_INT32 /*eHdr*/, SipHeaderBase* pHeader)
{
    if (pHeader != SIP_NULL)
    {
        return new SipDateHeader(*reinterpret_cast<SipDateHeader*>(pHeader));
    }
    return new SipDateHeader();
}
