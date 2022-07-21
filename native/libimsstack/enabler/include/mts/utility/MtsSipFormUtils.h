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

#ifndef MTS_SIP_FORM_UTILS_H_
#define MTS_SIP_FORM_UTILS_H_

#include "AString.h"
#include "MtsDef.h"

class MtsDialingPlan;

class MtsSipFormUtils final
{
public:
    MtsSipFormUtils(IN IMS_SINT32 nSlotId);
    ~MtsSipFormUtils();

    IMS_BOOL FormDestination(IN const AString& strTargetAddress, IN const IMS_BOOL bIsAckorError,
            IN const AString& strLastIpSmgw, OUT AString& strDest);
    AString FormContentTypeEnumToStr(IN SmsFormatType nType);
    SmsFormatType FormContentTypeStrToEnum(IN const AString& strContentType);
    IMS_BOOL IsTelUrlParam(IN const AString& strParam) const;
    IMS_BOOL IsNumberFormat(IN const AString& strDial) const;
    IMS_BOOL IsIpAddress(IN const AString& strIp) const;
    IMS_SINT32 CheckScheme(IN const AString& strScheme) const;

private:
    IMS_SINT32 GetRequestUriType();
    IMS_BOOL IsVisualSeparator(IN const IMS_CHAR ch) const;

public:
    enum
    {
        SCHEME_UNKNOWN = -1,
        SCHEME_TEL,
        SCHEME_SIP,
        SCHEME_SIPS
    };

private:
    MtsDialingPlan* m_pMtsDialingPlan;
    IMS_SINT32 m_nSlotId;
};

#endif
