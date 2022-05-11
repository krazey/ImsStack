#include "msg/SipDateHeader.h"
#include "platform/sip_pf_string.h"
#include "SipTrace.h"
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

/*****************************************************************************
 * Function name      : sipGetWeekDayType
 *
 * Description        :
 *
 * Preconditions      :
 *
 * Side Effects          : none
 *****************************************************************************/
SIP_INT32 sipGetWeekDayType(SIP_CHAR* pszWeekDay)
{
    switch (pszWeekDay[0])
    {
        case 'M':
        case 'm':
            if (SipPf_Strnicmp(pszWeekDay, gaszWeekday[SipDateHeader::MONDAY], MAX_WEEK_LEN) == 0)
            {
                return SipDateHeader::MONDAY;
            }
            break;

        case 'T':
        case 't':
            if (SipPf_Strnicmp(pszWeekDay, gaszWeekday[SipDateHeader::TUESDAY], MAX_WEEK_LEN) == 0)
            {
                return SipDateHeader::TUESDAY;
            }
            else if (SipPf_Strnicmp(
                             pszWeekDay, gaszWeekday[SipDateHeader::THURSDAY], MAX_WEEK_LEN) == 0)
            {
                return SipDateHeader::THURSDAY;
            }
            break;
        case 'W':
        case 'w':
            if (SipPf_Strnicmp(pszWeekDay, gaszWeekday[SipDateHeader::WEDNESDAY], MAX_WEEK_LEN) ==
                    0)
            {
                return SipDateHeader::WEDNESDAY;
            }
            break;
        case 'F':
        case 'f':
            if (SipPf_Strnicmp(pszWeekDay, gaszWeekday[SipDateHeader::FRIDAY], MAX_WEEK_LEN) == 0)
            {
                return SipDateHeader::FRIDAY;
            }
            break;
        case 'S':
        case 's':
            if (SipPf_Strnicmp(pszWeekDay, gaszWeekday[SipDateHeader::SATURDAY], MAX_WEEK_LEN) == 0)
            {
                return SipDateHeader::SATURDAY;
            }
            else if (SipPf_Strnicmp(pszWeekDay, gaszWeekday[SipDateHeader::SUNDAY], MAX_WEEK_LEN) ==
                    0)
            {
                return SipDateHeader::SUNDAY;
            }
            break;
        default:
            break;
    }
    return SipDateHeader::UNKNOWN_DAY;
}
/*****************************************************************************
 * Function name      : sipGetMonthType
 *
 * Description        :
 *
 * Preconditions      :
 *
 * Side Effects          : none
 *****************************************************************************/
SIP_INT32 sipGetMonthType(SIP_CHAR* pszMonth)
{
    switch (pszMonth[0])
    {
        case 'J':
        case 'j':
            if (SipPf_Strnicmp(pszMonth, gaszMonth[SipDateHeader::JANUARY], MAX_MONTH_LEN) == 0)
            {
                return SipDateHeader::JANUARY;
            }
            else if (SipPf_Strnicmp(pszMonth, gaszMonth[SipDateHeader::JUNE], MAX_MONTH_LEN) == 0)
            {
                return SipDateHeader::JUNE;
            }
            else if (SipPf_Strnicmp(pszMonth, gaszMonth[SipDateHeader::JULY], MAX_MONTH_LEN) == 0)
            {
                return SipDateHeader::JULY;
            }
            break;
        case 'F':
        case 'f':
            if (SipPf_Strnicmp(pszMonth, gaszMonth[SipDateHeader::FEBRUARY], MAX_MONTH_LEN) == 0)
            {
                return SipDateHeader::FEBRUARY;
            }
            break;
        case 'M':
        case 'm':
            if (SipPf_Strnicmp(pszMonth, gaszMonth[SipDateHeader::MARCH], MAX_MONTH_LEN) == 0)
            {
                return SipDateHeader::MARCH;
            }
            else if (SipPf_Strnicmp(pszMonth, gaszMonth[SipDateHeader::MAY], MAX_MONTH_LEN) == 0)
            {
                return SipDateHeader::MAY;
            }
            break;

        case 'A':
        case 'a':
            if (SipPf_Strnicmp(pszMonth, gaszMonth[SipDateHeader::APRIL], MAX_MONTH_LEN) == 0)
            {
                return SipDateHeader::APRIL;
            }
            else if (SipPf_Strnicmp(pszMonth, gaszMonth[SipDateHeader::AUGUST], MAX_MONTH_LEN) == 0)
            {
                return SipDateHeader::AUGUST;
            }
            break;
        case 's':
        case 'S':
            if (SipPf_Strnicmp(pszMonth, gaszMonth[SipDateHeader::SEPTEMBER], MAX_MONTH_LEN) == 0)
            {
                return SipDateHeader::SEPTEMBER;
            }
            break;

        case 'O':
        case 'o':
            if (SipPf_Strnicmp(pszMonth, gaszMonth[SipDateHeader::OCTOBER], MAX_MONTH_LEN) == 0)
            {
                return SipDateHeader::OCTOBER;
            }
            break;
        case 'N':
        case 'n':
            if (SipPf_Strnicmp(pszMonth, gaszMonth[SipDateHeader::NOVEMBER], MAX_MONTH_LEN) == 0)
            {
                return SipDateHeader::NOVEMBER;
            }
            break;
        case 'D':
        case 'd':
            if (SipPf_Strnicmp(pszMonth, gaszMonth[SipDateHeader::DECEMBER], MAX_MONTH_LEN) == 0)
            {
                return SipDateHeader::DECEMBER;
            }
            break;

        default:
            break;
    }
    return SipDateHeader::UNKNOWN_MONTH;
}

/******************************************************************************
 * Function name      : SipDateHeader::SipDateHeader
 *
 * Description     :
 *
 * Preconditions      :
 *
 * Side Effects      : none
 *****************************************************************************/
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

/******************************************************************************
 * Function name      : SipDateHeader::~SipDateHeader
 *
 * Description     :
 *
 * Preconditions      :
 *
 * Side Effects      : none
 *****************************************************************************/
SipDateHeader::~SipDateHeader() {}

/******************************************************************************
 * Function name      : SipDateHeader::~SipDateHeader
 *
 * Description     :
 *
 * Preconditions      :
 *
 * Side Effects      : none
 *****************************************************************************/
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

/******************************************************************************
 * Function name      : SipDateHeader::EncodeHdr
 *
 * Description     :
 *
 * Preconditions      :
 *
 * Side Effects      : none
 *****************************************************************************/
SIP_BOOL SipDateHeader::EncodeHdr(SIP_CHAR** ppCurrPos, SIP_BOOL /*bParams = SIP_TRUE*/)
{
    if (EncodeDate(ppCurrPos) == SIP_FALSE)
    {
        SIP_DEBUG_WARNING(ESIPTRACE_MODENCODER, "Date Encoding failed", SIP_ZERO, SIP_ZERO);
        return SIP_FALSE;
    }

    return SIP_TRUE;
}

SIP_BOOL SipDateHeader::EncodeWeek(SIP_CHAR** ppCurrPos)
{
    SipPf_Strcpy(*ppCurrPos, gaszWeekday[m_eWkDay]);
    SipEnc_UpdateCurrPos(ppCurrPos);
    return SIP_TRUE;
}

SIP_BOOL SipDateHeader::EncodeTime(SIP_CHAR** ppCurrPos)
{
    SipPf_Sprintf(*ppCurrPos, (SIP_CHAR*)"%02u:%02u:%02u", m_nHour, m_nMin, m_nSec);
    SipEnc_UpdateCurrPos(ppCurrPos);

    return SIP_TRUE;
}

SIP_BOOL SipDateHeader::EncodeDate(SIP_CHAR** ppCurrPos)
{
    SIP_CHAR* pMsgCurrPtr = *ppCurrPos;

    // Date: Thu, 21 Feb 2002 13:02:03 GMT
    EncodeWeek(&pMsgCurrPtr);

    pMsgCurrPtr[0] = COMMA;
    pMsgCurrPtr++;

    pMsgCurrPtr[0] = SPACE;
    pMsgCurrPtr++;

    SipPf_Sprintf(pMsgCurrPtr, (SIP_CHAR*)"%02u %s %4u", m_nDate, gaszMonth[m_eMonth], m_nYear);
    SipEnc_UpdateCurrPos(&pMsgCurrPtr);

    pMsgCurrPtr[0] = SPACE;
    pMsgCurrPtr++;

    EncodeTime(&pMsgCurrPtr);

    pMsgCurrPtr[0] = SPACE;
    pMsgCurrPtr++;

    SipPf_Strcpy(pMsgCurrPtr, STR_GMT);
    SipEnc_UpdateCurrPos(&pMsgCurrPtr);

    *ppCurrPos = pMsgCurrPtr;
    return SIP_TRUE;
}

/******************************************************************************
 * Function name      : SipDateHeader::DecodeHdr
 *
 * Description     :
 *
 * Preconditions      :
 *
 * Side Effects      : none
 *****************************************************************************/
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

    if (sipFindPreDelimiter(pStartPt, pEndPt, &pTempPos, COMMA) == SIP_FALSE)
    {
        SIP_DEBUG_WARNING(ESIPTRACE_MODDECODER, "Invalid Date Format", SIP_ZERO, SIP_ZERO);
        return SIP_FALSE;
    }

    SIP_CHAR* pszTempValue = sipCreateString(pStartPt, pTempPos);
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

    if (sipFindPreDelimiter(pStartPt, pEndPt, &pTempPos, SPACE) == SIP_FALSE)
    {
        SIP_DEBUG_WARNING(ESIPTRACE_MODDECODER, "Invalid Date Format", SIP_ZERO, SIP_ZERO);
        return SIP_FALSE;
    }

    /*Get the value of date*/
    pszTempValue = sipCreateString(pStartPt, pTempPos);
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

    if (sipFindPreDelimiter(pStartPt, pEndPt, &pTempPos, SPACE) == SIP_FALSE)
    {
        SIP_DEBUG_WARNING(ESIPTRACE_MODDECODER, "Invalid Date Format", SIP_ZERO, SIP_ZERO);
        return SIP_FALSE;
    }

    pszTempValue = sipCreateString(pStartPt, pTempPos);
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

    if (sipFindPreDelimiter(pStartPt, pEndPt, &pTempPos, SPACE) == SIP_FALSE)
    {
        SIP_DEBUG_WARNING(ESIPTRACE_MODDECODER, "Invalid Date Format", SIP_ZERO, SIP_ZERO);
        return SIP_FALSE;
    }
    pszTempValue = sipCreateString(pStartPt, pTempPos);
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

    if (sipFindPreDelimiter(pStartPt, pEndPt, &pTempPos, COLON) == SIP_FALSE)
    {
        SIP_DEBUG_WARNING(ESIPTRACE_MODDECODER, "Invalid Date Format", SIP_ZERO, SIP_ZERO);
        return SIP_FALSE;
    }
    pszTempValue = sipCreateString(pStartPt, pTempPos);
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

    if (sipFindPreDelimiter(pStartPt, pEndPt, &pTempPos, COLON) == SIP_FALSE)
    {
        SIP_DEBUG_WARNING(ESIPTRACE_MODDECODER, "Invalid Date Format", SIP_ZERO, SIP_ZERO);
        return SIP_FALSE;
    }
    pszTempValue = sipCreateString(pStartPt, pTempPos);
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
    if (sipFindPreDelimiter(pStartPt, pEndPt, &pTempPos, SPACE) == SIP_FALSE)
    {
        SIP_DEBUG_WARNING(ESIPTRACE_MODDECODER, "Invalid Date Format", SIP_ZERO, SIP_ZERO);
        return SIP_FALSE;
    }
    pszTempValue = sipCreateString(pStartPt, pTempPos);
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
    pTempPos = SIP_NULL;
    SIP_CHAR* pszTempGMT = sipCreateString(pStartPt, pEndPt);
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

/******************************************************************************
 * Function name      : SipDateHeader::SetDate
 *
 * Description     :
 *
 * Preconditions      :
 *
 * Side Effects      : none
 *****************************************************************************/
SIP_BOOL SipDateHeader::SetDate(const SIP_UINT16 nDate)
{
    if ((nDate <= DATE_VAL) && (nDate > SIP_ZERO))
    {
        m_nDate = nDate;
        return SIP_TRUE;
    }
    return SIP_FALSE;
}

/******************************************************************************
 * Function name      : SipDateHeader::SetMonth
 *
 * Description     :
 *
 * Preconditions      :
 *
 * Side Effects      : none
 *****************************************************************************/
SIP_BOOL SipDateHeader::SetMonth(SIP_INT32 eMonth)
{
    if ((eMonth >= SipDateHeader::JANUARY) && (eMonth < SipDateHeader::UNKNOWN_MONTH))
    {
        m_eMonth = eMonth;
        return SIP_TRUE;
    }
    return SIP_FALSE;
}

/******************************************************************************
 * Function name      : SipDateHeader::SetYear
 *
 * Description     :
 *
 * Preconditions      :
 *
 * Side Effects      : none
 *****************************************************************************/
SIP_BOOL SipDateHeader::SetYear(const SIP_UINT16 nYear)
{
    if (nYear >= YEAR_VAL)
    {
        m_nYear = nYear;
        return SIP_TRUE;
    }
    return SIP_FALSE;
}

/******************************************************************************
 * Function name      : SipDateHeader::SetHour
 *
 * Description     :
 *
 * Preconditions      :
 *
 * Side Effects      : none
 *****************************************************************************/
SIP_BOOL SipDateHeader::SetHour(const SIP_UINT16 nHour)
{
    if (nHour <= TIME_VAL)
    {
        m_nHour = nHour;
        return SIP_TRUE;
    }
    return SIP_FALSE;
}

/******************************************************************************
 * Function name      : SipDateHeader::SetMinute
 *
 * Description     :
 *
 * Preconditions      :
 *
 * Side Effects      : none
 *****************************************************************************/
SIP_BOOL SipDateHeader::SetMinute(const SIP_UINT16 nMin)
{
    if (nMin <= TIME_VAL)
    {
        m_nMin = nMin;
        return SIP_TRUE;
    }
    return SIP_FALSE;
}

/******************************************************************************
 * Function name      : SipDateHeader::SetSecond
 *
 * Description     :
 *
 * Preconditions      :
 *
 * Side Effects      : none
 *****************************************************************************/
SIP_BOOL SipDateHeader::SetSecond(const SIP_UINT16 nSec)
{
    if (nSec <= TIME_VAL)
    {
        m_nSec = nSec;
        return SIP_TRUE;
    }
    return SIP_FALSE;
}

/******************************************************************************
 * Function name      : SipDateHeader::SetWkDay
 *
 * Description     :
 *
 * Preconditions      :
 *
 * Side Effects      : none
 *****************************************************************************/
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
