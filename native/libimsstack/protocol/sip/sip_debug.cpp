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
#include "sip_pf_datatypes.h"
#include "platform/sip_pf_memory.h"
#include "platform/sip_pf_string.h"

#include "sip_debug.h"
#include "SipUtil.h"

void SIP_DEBUG_LOG(SIP_UINT32 nCategory, SIP_CHAR* pszFilename, SIP_INT32 nLine,
    SIP_CHAR* pszFormat,...)
{
    SipUtil* pUtil = SipUtil_GetInstance();

    if (pUtil == SIP_NULL)
    {
        return;
    }

    SIP_CHAR szTemp[DEBUG_MSG_MAX_SIZE + 1] = {SIP_ZERO};

    va_list args;
    va_start(args, pszFormat);
    vsnprintf(szTemp, DEBUG_MSG_MAX_SIZE, pszFormat, args);
    va_end(args);

    SIP_CHAR* pszTempFilename = SipPf_Strdup(pszFilename);

    const SIP_CHAR* pTemp = (pszTempFilename != SIP_NULL) ?\
            SipPf_StripFileName(pszTempFilename) : "xxx";

    SIP_CHAR szFrmtString[DEBUG_MSG_MAX_SIZE + 1] = {SIP_ZERO};
    SipPf_Snprintf(szFrmtString, DEBUG_MSG_MAX_SIZE, "[%s:%d] %s", pTemp, nLine, szTemp);

    pUtil->GetLogger()->DumpLog(nCategory, SIP_NULL, nLine, szFrmtString);

    if (pszTempFilename != SIP_NULL)
    {
        delete[] pszTempFilename;
    }
}
