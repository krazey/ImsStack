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
#include "ServiceEvent.h"
#include "ServiceMemory.h"
#include "ServiceThread.h"
#include "ServiceUtil.h"
#include "SystemConfig.h"

#include "SipDebug.h"
#include "SIPStackHeaders.h"

PRIVATE GLOBAL
IMS_CHAR SipDebug::acIpAddr[SipDebug::MAX_LOG_IP + 1] = { '\0', };
PRIVATE GLOBAL
IMS_CHAR SipDebug::acLog1[SipDebug::MAX_LOG_CHAR_ARRAY + 3 + 1] = { '\0', };
PRIVATE GLOBAL
IMS_CHAR SipDebug::acLog2[SipDebug::MAX_LOG_CHAR_ARRAY + 3 + 1] = { '\0', };
PRIVATE GLOBAL
AString SipDebug::strLog1 = AString::ConstNull();
PRIVATE GLOBAL
AString SipDebug::strLog2 = AString::ConstNull();

// For SIM2 for multi-SIM device
PRIVATE GLOBAL
IMS_CHAR SipDebug::acIpAddr_1[SipDebug::MAX_LOG_IP + 1] = { '\0', };
PRIVATE GLOBAL
IMS_CHAR SipDebug::acLog1_1[SipDebug::MAX_LOG_CHAR_ARRAY + 3 + 1] = { '\0', };
PRIVATE GLOBAL
IMS_CHAR SipDebug::acLog2_1[SipDebug::MAX_LOG_CHAR_ARRAY + 3 + 1] = { '\0', };
PRIVATE GLOBAL
AString SipDebug::strLog1_1 = AString::ConstNull();
PRIVATE GLOBAL
AString SipDebug::strLog2_1 = AString::ConstNull();

PUBLIC GLOBAL
void SipDebug::Send(IN IMS_SINT32 nSlotId, IN IMS_SINT32 nMsgType,
        IN IMS_SINT32 nDirection, IN IMS_SINT32 nMethod, IN IMS_SINT32 nStatusCode/* = 0*/)
{
    // EVENT_IMS_DEBUG
    // wParam
    //    - HI : Category (SIP - 0, Media - 1, ...)
    //    - LO (high 1byte) : SIP (REQ - 0, RESP - 1)
    //    - LO (low 1byte) : DIR (OUT - 0, IN - 1)
    // lParam
    //    - HI : Method
    //    - LO : StatusCode

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
PUBLIC GLOBAL
void SipDebug::InitLogging()
{
    IMS_MEM_Memset(acIpAddr, 0x00, sizeof(acIpAddr));
    IMS_MEM_Memset(acLog1, 0x00, sizeof(acLog1));
    IMS_MEM_Memset(acLog2, 0x00, sizeof(acLog2));

    strLog1 = AString::ConstNull();
    strLog2 = AString::ConstNull();

    // For SIM2 for multi-SIM device
    IMS_MEM_Memset(acIpAddr_1, 0x00, sizeof(acIpAddr_1));
    IMS_MEM_Memset(acLog1_1, 0x00, sizeof(acLog1_1));
    IMS_MEM_Memset(acLog2_1, 0x00, sizeof(acLog2_1));

    strLog1_1 = AString::ConstNull();
    strLog2_1 = AString::ConstNull();
}

PUBLIC GLOBAL
const IMS_CHAR* SipDebug::GetCharA1(IN const IMS_CHAR* pszValue, IN IMS_SINT32 nCount,
        IN const IMS_CHAR cDelimiter/* = 0 no delimiter*/)
{
    if (nCount > MAX_LOG_CHAR_ARRAY)
    {
        nCount = MAX_LOG_CHAR_ARRAY;
    }

    IMS_CHAR* pszLog = (GetSimSlot() == IMS_SLOT_0) ? &acLog1[0] : &acLog1_1[0];

    return SIPStack::GetLogString(pszValue, pszLog, nCount + 3, cDelimiter);
}

PUBLIC GLOBAL
const IMS_CHAR* SipDebug::GetCharA2(IN const IMS_CHAR* pszValue, IN IMS_SINT32 nCount,
        IN const IMS_CHAR cDelimiter/* = 0 no delimiter*/)
{
    if (nCount > MAX_LOG_CHAR_ARRAY)
    {
        nCount = MAX_LOG_CHAR_ARRAY;
    }

    IMS_CHAR* pszLog = (GetSimSlot() == IMS_SLOT_0) ? &acLog2[0] : &acLog2_1[0];

    return SIPStack::GetLogString(pszValue, pszLog, nCount + 3, cDelimiter);
}

PUBLIC GLOBAL
const IMS_CHAR* SipDebug::GetIp(IN const IPAddress& objIpAddr)
{
    IMS_CHAR* pszLog = (GetSimSlot() == IMS_SLOT_0) ? &acIpAddr[0] : &acIpAddr_1[0];

    // fe80:xxx or 192.1xxx
    return SIPStack::GetLogString(objIpAddr.ToCharString(), pszLog, 8);
}

PUBLIC GLOBAL
const IMS_CHAR* SipDebug::GetIp(IN const AString& strIpAddr)
{
    IMS_CHAR* pszLog = (GetSimSlot() == IMS_SLOT_0) ? &acIpAddr[0] : &acIpAddr_1[0];

    // fe80:xxx or 192.1xxx
    return SIPStack::GetLogString(strIpAddr.GetStr(), pszLog, 8);
}

PUBLIC GLOBAL
const AString& SipDebug::GetStr1(IN const AString& strValue, IN IMS_SINT32 nCount,
        IN const IMS_CHAR cDelimiter/* = 0 no delimiter*/)
{
    AString& strLog = (GetSimSlot() == IMS_SLOT_0) ? strLog1 : strLog1_1;

    return UtilService::GetLogString(strValue, strLog, nCount, cDelimiter);
}

PUBLIC GLOBAL
const AString& SipDebug::GetStr2(IN const AString& strValue, IN IMS_SINT32 nCount,
        IN const IMS_CHAR cDelimiter/* = 0 no delimiter*/)
{
    AString& strLog = (GetSimSlot() == IMS_SLOT_0) ? strLog2 : strLog2_1;

    return UtilService::GetLogString(strValue, strLog, nCount, cDelimiter);
}

PUBLIC GLOBAL
const AString& SipDebug::GetUri1(IN const AString& strValue)
{
    AString& strLog = (GetSimSlot() == IMS_SLOT_0) ? strLog1 : strLog1_1;

    return UtilService::GetLogString(strValue, strLog, 10);
}

PUBLIC GLOBAL
const AString& SipDebug::GetUri2(IN const AString& strValue)
{
    AString& strLog = (GetSimSlot() == IMS_SLOT_0) ? strLog2 : strLog2_1;

    return UtilService::GetLogString(strValue, strLog, 10);
}

PRIVATE GLOBAL
IMS_BOOL SipDebug::CheckIfDebugRequired(IN IMS_SINT32/* nSlotId*/, IN IMS_SINT32 nMsgType,
        IN IMS_SINT32 nDirection, IN IMS_SINT32 nMethod, IN IMS_SINT32 nStatusCode)
{
    IMS_BOOL bCheckDebugRequired = IMS_FALSE;

    if (bCheckDebugRequired)
    {
        (void) nStatusCode;

        if (nMethod == SipMethod::REGISTER)
        {
            if ((nMsgType == MSG_RSP) && (nDirection == DIR_IN))
            {
                return IMS_TRUE;
            }
        }
        else if ((nMethod == SipMethod::INVITE) || (nMethod == SipMethod::BYE))
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

PRIVATE GLOBAL
IMS_SINT32 SipDebug::GetSimSlot()
{
    if (SystemConfig::IsMultiSimEnabled())
    {
        return ThreadService::GetCurrentSlotId(IMS_SLOT_0);
    }

    return IMS_SLOT_0;
}
