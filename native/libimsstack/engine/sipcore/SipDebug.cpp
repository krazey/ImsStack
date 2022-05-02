/*
    Author
    <table>
    date      author                    description
    --------  --------------            ----------
    20121116  hwangoo.park@             Created
    </table>

    Description
*/

#include "ServiceMemory.h"
#include "ServiceEvent.h"
#include "ServiceThread.h"
#include "ServiceUtil.h"
#include "SystemConfig.h"
#include "SIPStackHeaders.h"
#include "SipDebug.h"



PRIVATE GLOBAL
IMS_CHAR SIPDebug::acIP[SIPDebug::MAX_LOG_IP + 1] = { '\0', };
PRIVATE GLOBAL
IMS_CHAR SIPDebug::acLog1[SIPDebug::MAX_LOG_CHAR_ARRAY + 3 + 1] = { '\0', };
PRIVATE GLOBAL
IMS_CHAR SIPDebug::acLog2[SIPDebug::MAX_LOG_CHAR_ARRAY + 3 + 1] = { '\0', };
PRIVATE GLOBAL
AString SIPDebug::strLog1 = AString::ConstNull();
PRIVATE GLOBAL
AString SIPDebug::strLog2 = AString::ConstNull();

// For SIM2 for multi-SIM device
PRIVATE GLOBAL
IMS_CHAR SIPDebug::acIP_1[SIPDebug::MAX_LOG_IP + 1] = { '\0', };
PRIVATE GLOBAL
IMS_CHAR SIPDebug::acLog1_1[SIPDebug::MAX_LOG_CHAR_ARRAY + 3 + 1] = { '\0', };
PRIVATE GLOBAL
IMS_CHAR SIPDebug::acLog2_1[SIPDebug::MAX_LOG_CHAR_ARRAY + 3 + 1] = { '\0', };
PRIVATE GLOBAL
AString SIPDebug::strLog1_1 = AString::ConstNull();
PRIVATE GLOBAL
AString SIPDebug::strLog2_1 = AString::ConstNull();

/*

Remarks

*/
PUBLIC GLOBAL
void SIPDebug::Send(IN IMS_SINT32 nSlotId, IN IMS_SINT32 nMsgType,
        IN IMS_SINT32 nDirection, IN IMS_SINT32 nMethod, IN IMS_SINT32 nStatusCode /* = 0 */)
{
    // EVENT_IMS_DEBUG
    // wParam
    //    - HI : Category (SIP - 0, Media - 1, ...)
    //    - LO (high 1byte) : SIP (REQ - 0, RESP - 1)
    //    - LO (low 1byte) : DIR (OUT - 0, IN - 1)
    // lParam
    //    - HI : Method
    //    - LO : StatusCode

    //---------------------------------------------------------------------------------------------

    if (!CheckIfDebugRequired(nSlotId, nMsgType, nDirection, nMethod, nStatusCode))
    {
        return;
    }

    IMS_EVENT_SendEventForSlotId(IMS_EVENT_DEBUG,
            ((nMsgType | nDirection) & 0xFFFFFFFF),
            IMS_MAKEPARAM(nMethod, nStatusCode),
            nSlotId);
}

// Methods for logging based on release mode
/*

Remarks

*/
PUBLIC GLOBAL
void SIPDebug::InitLogging()
{
    IMS_MEM_Memset(acIP, 0x00, sizeof(acIP));
    IMS_MEM_Memset(acLog1, 0x00, sizeof(acLog1));
    IMS_MEM_Memset(acLog2, 0x00, sizeof(acLog2));

    strLog1 = AString::ConstNull();
    strLog2 = AString::ConstNull();

    // For SIM2 for multi-SIM device
    IMS_MEM_Memset(acIP_1, 0x00, sizeof(acIP_1));
    IMS_MEM_Memset(acLog1_1, 0x00, sizeof(acLog1_1));
    IMS_MEM_Memset(acLog2_1, 0x00, sizeof(acLog2_1));

    strLog1_1 = AString::ConstNull();
    strLog2_1 = AString::ConstNull();
}

/*

Remarks

*/
PUBLIC GLOBAL
const IMS_CHAR* SIPDebug::GetCharA1(IN CONST IMS_CHAR *pszValue, IN IMS_SINT32 nCount,
        IN CONST IMS_CHAR cDelimiter/* = 0 no delimiter */)
{
    if (nCount > MAX_LOG_CHAR_ARRAY)
    {
        nCount = MAX_LOG_CHAR_ARRAY;
    }

    IMS_CHAR* pszLog = (GetSimSlot() == IMS_SLOT_0) ? &acLog1[0] : &acLog1_1[0];

    return SIPStack::GetLogString(pszValue, pszLog, nCount + 3, cDelimiter);
}

/*

Remarks

*/
PUBLIC GLOBAL
const IMS_CHAR* SIPDebug::GetCharA2(IN CONST IMS_CHAR *pszValue, IN IMS_SINT32 nCount,
        IN CONST IMS_CHAR cDelimiter/* = 0 no delimiter */)
{
    if (nCount > MAX_LOG_CHAR_ARRAY)
    {
        nCount = MAX_LOG_CHAR_ARRAY;
    }

    IMS_CHAR* pszLog = (GetSimSlot() == IMS_SLOT_0) ? &acLog2[0] : &acLog2_1[0];

    return SIPStack::GetLogString(pszValue, pszLog, nCount + 3, cDelimiter);
}

/*

Remarks

*/
PUBLIC GLOBAL
const IMS_CHAR* SIPDebug::GetIP(IN CONST IPAddress &objIPA)
{
    //---------------------------------------------------------------------------------------------

    IMS_CHAR* pszLog = (GetSimSlot() == IMS_SLOT_0) ? &acIP[0] : &acIP_1[0];

    // fe80:xxx or 192.1xxx
    return SIPStack::GetLogString(objIPA.ToCharString(), pszLog, 8);
}

/*

Remarks

*/
PUBLIC GLOBAL
const IMS_CHAR* SIPDebug::GetIP(IN CONST AString &strIP)
{
    //---------------------------------------------------------------------------------------------

    IMS_CHAR* pszLog = (GetSimSlot() == IMS_SLOT_0) ? &acIP[0] : &acIP_1[0];

    // fe80:xxx or 192.1xxx
    return SIPStack::GetLogString(strIP.GetStr(), pszLog, 8);
}

/*

Remarks

*/
PUBLIC GLOBAL
const AString& SIPDebug::GetStr1(IN CONST AString &strValue, IN IMS_SINT32 nCount,
        IN CONST IMS_CHAR cDelimiter/* = 0 no delimiter */)
{
    //---------------------------------------------------------------------------------------------

    AString& strLog = (GetSimSlot() == IMS_SLOT_0) ? strLog1 : strLog1_1;

    return UtilService::GetLogString(strValue, strLog, nCount, cDelimiter);
}

/*

Remarks

*/
PUBLIC GLOBAL
const AString& SIPDebug::GetStr2(IN CONST AString &strValue, IN IMS_SINT32 nCount,
        IN CONST IMS_CHAR cDelimiter/* = 0 no delimiter */)
{
    //---------------------------------------------------------------------------------------------

    AString& strLog = (GetSimSlot() == IMS_SLOT_0) ? strLog2 : strLog2_1;

    return UtilService::GetLogString(strValue, strLog, nCount, cDelimiter);
}

/*

Remarks

*/
PUBLIC GLOBAL
const AString& SIPDebug::GetUri1(IN CONST AString &strValue)
{
    //---------------------------------------------------------------------------------------------

    AString& strLog = (GetSimSlot() == IMS_SLOT_0) ? strLog1 : strLog1_1;

    return UtilService::GetLogString(strValue, strLog, 10);
}

/*

Remarks

*/
PUBLIC GLOBAL
const AString& SIPDebug::GetUri2(IN CONST AString &strValue)
{
    //---------------------------------------------------------------------------------------------

    AString& strLog = (GetSimSlot() == IMS_SLOT_0) ? strLog2 : strLog2_1;

    return UtilService::GetLogString(strValue, strLog, 10);
}

/*

Remarks

*/
PRIVATE GLOBAL
IMS_BOOL SIPDebug::CheckIfDebugRequired(IN IMS_SINT32/* nSlotId*/, IN IMS_SINT32 nMsgType,
        IN IMS_SINT32 nDirection, IN IMS_SINT32 nMethod, IN IMS_SINT32 nStatusCode)
{
    IMS_BOOL bCheckDebugRequired = IMS_FALSE;

    if (bCheckDebugRequired)
    {
        (void) nStatusCode;

        if (nMethod == SIPMethod::REGISTER)
        {
            if ((nMsgType == MSG_RSP) && (nDirection == DIR_IN))
            {
                return IMS_TRUE;
            }
        }
        else if ((nMethod == SIPMethod::INVITE)
                || (nMethod == SIPMethod::BYE))
        {
            if (nMsgType == MSG_REQ)
            {
                return IMS_TRUE;
            }
        }

        // KT
        {
            return ((nMsgType == MSG_RSP) && (nDirection == DIR_IN));
        }
    }
    else
    {
        (void) nMsgType;
        (void) nDirection;
        (void) nMethod;
        (void) nStatusCode;
    }

    return IMS_FALSE;
}

/*

Remarks

*/
PRIVATE GLOBAL
IMS_SINT32 SIPDebug::GetSimSlot()
{
    if (SystemConfig::IsMultiSimEnabled())
    {
        return ThreadService::GetCurrentSlotId(IMS_SLOT_0);
    }

    return IMS_SLOT_0;
}
