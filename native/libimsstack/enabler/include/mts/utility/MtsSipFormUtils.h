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
    explicit MtsSipFormUtils(IN IMS_SINT32 nSlotId);
    ~MtsSipFormUtils();
    MtsSipFormUtils(IN const MtsSipFormUtils&) = delete;
    MtsSipFormUtils& operator=(IN const MtsSipFormUtils&) = delete;

    IMS_BOOL FormDestination(IN const AString& strTargetAddress, IN const IMS_BOOL bIsAckorError,
            IN const AString& strLastIpSmgw, OUT AString& strDest);
    static AString FormContentTypeEnumToStr(IN SmsFormatType nType);
    static SmsFormatType FormContentTypeStrToEnum(IN const AString& strContentType);
    static IMS_BOOL IsTelUrlParam(IN const AString& strParam);
    static IMS_BOOL IsNumberFormat(IN const AString& strDial);
    static IMS_BOOL IsIpAddress(IN const AString& strIp);
    static IMS_SINT32 CheckScheme(IN const AString& strTargetAddress);

private:
    IMS_SINT32 GetRequestUriType();
    static IMS_BOOL IsVisualSeparator(IN const IMS_CHAR ch);

    MtsDialingPlan* m_pMtsDialingPlan;
    IMS_SINT32 m_nSlotId;
};

#endif
