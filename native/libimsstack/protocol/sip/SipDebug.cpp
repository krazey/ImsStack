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
#include "SipUtil.h"
#include "platform/SipString.h"

// NOLINTNEXTLINE(cert-dcl50-cpp)
SIP_VOID SIP_DEBUG_LOG(SIP_UINT32 nCategory, const SIP_CHAR* pszFilename, SIP_INT32 nLine,
        const SIP_CHAR* pszFormat, ...)
{
    SIP_CHAR szTemp[DEBUG_MSG_MAX_SIZE + 1] = {SIP_ZERO};

    va_list args;
    va_start(args, pszFormat);
    vsnprintf(szTemp, DEBUG_MSG_MAX_SIZE, pszFormat, args);
    va_end(args);

    SIP_CHAR szFrmtString[DEBUG_MSG_MAX_SIZE + 1] = {SIP_ZERO};
    SipPf_Snprintf(szFrmtString, DEBUG_MSG_MAX_SIZE, "%s", szTemp);

    SipUtil::GetInstance()->GetLogger()->DumpLog(nCategory, pszFilename, nLine, szFrmtString);
}

SIP_VOID SIP_ASSERT_LOG(const SIP_CHAR* pszCondition, const SIP_CHAR* pszModule, SIP_UINT16 nLine)
{
    SipUtil::GetInstance()->GetLogger()->DumpAssertLog(pszCondition, pszModule, nLine);
}
