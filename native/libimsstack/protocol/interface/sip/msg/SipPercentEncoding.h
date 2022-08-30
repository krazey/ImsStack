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
#ifndef __SIP_PERCENT_ENCODING_H__
#define __SIP_PERCENT_ENCODING_H__

#include "SipDatatypes.h"

class SipPercentEncoding
{
public:
    static SIP_CHAR* DoPercentDecoding(SIP_CHAR* pszString);
    static SIP_CHAR* DoPercentDecoding(SIP_CHAR* pszString, SIP_INT32* pFinalLength);
    static SIP_CHAR* DoPerEnc_UserAndHeader(SIP_CHAR* pszString, SIP_CHAR* pType);
    static SIP_CHAR* DoPerEnc_Password(SIP_CHAR* pszString);
    static SIP_CHAR* DoPerEnc_Host(SIP_CHAR* pszString);
    static SIP_CHAR* DoPerEnc_Param(SIP_CHAR* pszName, SIP_CHAR* pszValue);
    static SIP_CHAR* DoPerEnc_TokenParam(SIP_CHAR* pszString);

    static SIP_CHAR* DoPerEnc_MddrParam(SIP_CHAR* pszString);
    static SIP_CHAR* DoPerEnc_TtlParam(SIP_CHAR* pszString);

    static SIP_CHAR* DoPerEnc_OtherParam(SIP_CHAR* pszString);
};

#endif  //__SIP_PERCENT_ENCODING_H__
