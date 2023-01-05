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
#include "SipDefLoggerUtil.h"
#include "ServiceTrace.h"

__IMS_TRACE_TAG_SIP__;

SipDefLoggerUtil::SipDefLoggerUtil() {}

SipDefLoggerUtil::~SipDefLoggerUtil() {}

void SipDefLoggerUtil::DumpLog(SIP_UINT32 nCategory, const SIP_CHAR* /*pszFile*/,
        SIP_UINT16 /*nLine*/, const SIP_CHAR* pszFormat, ...)
{
    if (TraceService::GetTraceService()->GetTrace()->IsTraceEnabled(
                nCategory, __IMS_TRACE_MODULE__))
    {
        va_list args;
        va_start(args, pszFormat);
        const SIP_CHAR* strTag = __IMS_TRACE_NAME__;
        SIP_UINT32 nModule = __IMS_TRACE_MODULE__;

        TraceService::GetTraceService()->GetTrace()->OutV(
                nCategory, strTag, (nModule), pszFormat, args);
        va_end(args);
    }
}
