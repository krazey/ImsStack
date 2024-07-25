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
#ifndef __SIP_DEF_LOGGER_UTIL_H__
#define __SIP_DEF_LOGGER_UTIL_H__

#include "ISipLoggerUtil.h"
#include "SipDatatypes.h"

class SipDefLoggerUtil : public ISipLoggerUtil
{
public:
    SipDefLoggerUtil();
    ~SipDefLoggerUtil();

public:
    void DumpLog(SIP_UINT32 nCategory, const SIP_CHAR* pszFile, SIP_UINT16 nLine,
            const SIP_CHAR* pszFormat, ...) override;
};

#endif  //__SIP_DEF_LOGGER_UTIL_H__
