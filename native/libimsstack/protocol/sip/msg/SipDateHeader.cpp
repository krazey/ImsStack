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
#include "SipDebug.h"
#include "msg/SipDateHeader.h"
#include "msg/SipMsgUtil.h"
#include "platform/SipString.h"

const SIP_CHAR SipDateHeader::STR_GMT[] = "GMT";
const SIP_CHAR* SipDateHeader::WEEKDAY[] = {"Mon", "Tue", "Wed", "Thu", "Fri", "Sat", "Sun"};
const SIP_CHAR* SipDateHeader::MONTH[] = {
        "Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};

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
    objBuffer += WEEKDAY[m_eWkDay];
    objBuffer += COMMA;
    objBuffer += SPACE;

    AString strDateTime;
    strDateTime.Sprintf("%02u %s %4u", m_nDate, MONTH[m_eMonth], m_nYear);

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
    SipPf_Strcpy(*ppCurrPos, WEEKDAY[m_eWkDay]);
    SipEnc_UpdateCurrPos(ppCurrPos);

    **ppCurrPos = COMMA;
    (*ppCurrPos)++;

    **ppCurrPos = SPACE;
    (*ppCurrPos)++;

    SipPf_Sprintf(*ppCurrPos, "%02u %s %4u", m_nDate, MONTH[m_eMonth], m_nYear);
    SipEnc_UpdateCurrPos(ppCurrPos);

    **ppCurrPos = SPACE;
    (*ppCurrPos)++;

    SipPf_Sprintf(*ppCurrPos, "%02u:%02u:%02u", m_nHour, m_nMin, m_nSec);
    SipEnc_UpdateCurrPos(ppCurrPos);

    **ppCurrPos = SPACE;
    (*ppCurrPos)++;

    SipPf_Strcpy(*ppCurrPos, STR_GMT);
    SipEnc_UpdateCurrPos(ppCurrPos);

    return SIP_TRUE;
}

SIP_BOOL SipDateHeader::DecodeHdr(const SIP_CHAR* pStartPt, SIP_UINT32 nDecLen)
{
    /*Date: Thu, 21 Feb 2002 13:02:03 GMT*/

    if (nDecLen == SIP_ZERO)
    {
        SIP_DEBUG_WARNING(ESIPTRACE_MODDECODER, "Empty buffer", SIP_ZERO, SIP_ZERO);
        return SIP_FALSE;
    }

    const SIP_CHAR* pEndPt = pStartPt + nDecLen - SIP_ONE;
    const SIP_CHAR* pTempPos = SIP_NULL;

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

    m_eWkDay = GetWeekDayType(pszTempValue);
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

    if (m_nDate > MAX_DATE)
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

    m_eMonth = GetMonthType(pszTempValue);
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

    if (m_nHour >= MAX_HOUR)
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

    if (m_nMin >= MAX_MIN_SEC)
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
    if (m_nSec >= MAX_MIN_SEC)
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
    if ((nDate <= MAX_DATE) && (nDate > SIP_ZERO))
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
    const SIP_UINT16 YEAR_VAL = 1000;

    if (nYear >= YEAR_VAL)
    {
        m_nYear = nYear;
        return SIP_TRUE;
    }
    return SIP_FALSE;
}

SIP_BOOL SipDateHeader::SetHour(const SIP_UINT16 nHour)
{
    if (nHour < MAX_HOUR)
    {
        m_nHour = nHour;
        return SIP_TRUE;
    }
    return SIP_FALSE;
}

SIP_BOOL SipDateHeader::SetMinute(const SIP_UINT16 nMin)
{
    if (nMin < MAX_MIN_SEC)
    {
        m_nMin = nMin;
        return SIP_TRUE;
    }
    return SIP_FALSE;
}

SIP_BOOL SipDateHeader::SetSecond(const SIP_UINT16 nSec)
{
    if (nSec < MAX_MIN_SEC)
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

SIP_INT32 SipDateHeader::GetWeekDayType(SIP_CHAR* pszWeekDay)
{
    switch (pszWeekDay[0])
    {
        case 'M':
            if (SipPf_Strcmp(pszWeekDay, WEEKDAY[SipDateHeader::MONDAY]) == 0)
            {
                return SipDateHeader::MONDAY;
            }
            break;

        case 'T':
            if (SipPf_Strcmp(pszWeekDay, WEEKDAY[SipDateHeader::TUESDAY]) == 0)
            {
                return SipDateHeader::TUESDAY;
            }
            else if (SipPf_Strcmp(pszWeekDay, WEEKDAY[SipDateHeader::THURSDAY]) == 0)
            {
                return SipDateHeader::THURSDAY;
            }
            break;
        case 'W':
            if (SipPf_Strcmp(pszWeekDay, WEEKDAY[SipDateHeader::WEDNESDAY]) == 0)
            {
                return SipDateHeader::WEDNESDAY;
            }
            break;
        case 'F':
            if (SipPf_Strcmp(pszWeekDay, WEEKDAY[SipDateHeader::FRIDAY]) == 0)
            {
                return SipDateHeader::FRIDAY;
            }
            break;
        case 'S':
            if (SipPf_Strcmp(pszWeekDay, WEEKDAY[SipDateHeader::SATURDAY]) == 0)
            {
                return SipDateHeader::SATURDAY;
            }
            else if (SipPf_Strcmp(pszWeekDay, WEEKDAY[SipDateHeader::SUNDAY]) == 0)
            {
                return SipDateHeader::SUNDAY;
            }
            break;
        default:
            break;
    }
    return SipDateHeader::UNKNOWN_DAY;
}

SIP_INT32 SipDateHeader::GetMonthType(SIP_CHAR* pszMonth)
{
    switch (pszMonth[0])
    {
        case 'J':
            if (SipPf_Strcmp(pszMonth, MONTH[SipDateHeader::JANUARY]) == 0)
            {
                return SipDateHeader::JANUARY;
            }
            else if (SipPf_Strcmp(pszMonth, MONTH[SipDateHeader::JUNE]) == 0)
            {
                return SipDateHeader::JUNE;
            }
            else if (SipPf_Strcmp(pszMonth, MONTH[SipDateHeader::JULY]) == 0)
            {
                return SipDateHeader::JULY;
            }
            break;
        case 'F':
            if (SipPf_Strcmp(pszMonth, MONTH[SipDateHeader::FEBRUARY]) == 0)
            {
                return SipDateHeader::FEBRUARY;
            }
            break;
        case 'M':
            if (SipPf_Strcmp(pszMonth, MONTH[SipDateHeader::MARCH]) == 0)
            {
                return SipDateHeader::MARCH;
            }
            else if (SipPf_Strcmp(pszMonth, MONTH[SipDateHeader::MAY]) == 0)
            {
                return SipDateHeader::MAY;
            }
            break;
        case 'A':
            if (SipPf_Strcmp(pszMonth, MONTH[SipDateHeader::APRIL]) == 0)
            {
                return SipDateHeader::APRIL;
            }
            else if (SipPf_Strcmp(pszMonth, MONTH[SipDateHeader::AUGUST]) == 0)
            {
                return SipDateHeader::AUGUST;
            }
            break;
        case 'S':
            if (SipPf_Strcmp(pszMonth, MONTH[SipDateHeader::SEPTEMBER]) == 0)
            {
                return SipDateHeader::SEPTEMBER;
            }
            break;

        case 'O':
            if (SipPf_Strcmp(pszMonth, MONTH[SipDateHeader::OCTOBER]) == 0)
            {
                return SipDateHeader::OCTOBER;
            }
            break;
        case 'N':
            if (SipPf_Strcmp(pszMonth, MONTH[SipDateHeader::NOVEMBER]) == 0)
            {
                return SipDateHeader::NOVEMBER;
            }
            break;
        case 'D':
            if (SipPf_Strcmp(pszMonth, MONTH[SipDateHeader::DECEMBER]) == 0)
            {
                return SipDateHeader::DECEMBER;
            }
            break;

        default:
            break;
    }
    return SipDateHeader::UNKNOWN_MONTH;
}
