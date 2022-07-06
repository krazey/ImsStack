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

class MtsDialingPlan;

class MtsSipFormUtils final
{
public:
    MtsSipFormUtils(IN IMS_SINT32 nSlotId);
    ~MtsSipFormUtils();

    static MtsSipFormUtils* GetInstance(IN IMS_SINT32 nSlotId);
    IMS_BOOL FormDestination(IN const AString& strTargetAddress, IN const IMS_BOOL bIsAckorError,
            IN const AString& strLastIpSmgw, OUT AString& strDest);
    AString FormContentTypeEnumToStr(IN IMS_UINT32 nType);
    IMS_UINT32 FormContentTypeStrToEnum(IN const AString& strContentType);
    IMS_SINT32 GetSlotId();
    AString ValidateAndUpdatePsi();
    IMS_BOOL IsTelUrlParam(IN const AString& strParam) const;
    IMS_BOOL IsNumberFormat(IN const AString& strDial) const;
    IMS_BOOL IsIpAddress(IN const AString& strIp) const;
    IMS_SINT32 CheckScheme(IN const AString& strScheme) const;

private:
    MtsDialingPlan* GetDialingPlan(IN IMS_SINT32 nSlotId);
    IMS_SINT32 GetRequestUriType();
    static IMS_BOOL IsVisualSeparator(IN IMS_CHAR ch);

public:
    enum
    {
        SCHEME_UNKNOWN = -1,
        SCHEME_TEL,
        SCHEME_SIP,
        SCHEME_SIPS
    };

public:
    IMS_UINT32 m_nMtsFormat;

private:
    MtsDialingPlan* m_pMtsDialingPlan;
    AString m_strPsi;
    IMS_SINT32 m_nSlotId;
};

#endif
