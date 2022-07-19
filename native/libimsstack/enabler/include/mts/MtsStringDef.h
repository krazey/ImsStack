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

#ifndef MTS_STRING_DEF_H_
#define MTS_STRING_DEF_H_

#include "IMSTypeDef.h"
#include "MtsDef.h"

class MtsStringDef
{
public:
    inline static const IMS_CHAR* PS_SmsFormatType(IN SmsFormatType eSmsFormat)
    {
        switch (eSmsFormat)
        {
            case SmsFormatType::SMSFORMAT_3GPP:
                return "3GPP";
            case SmsFormatType::SMSFORMAT_3GPP2:
                return "3GPP2";
            default:
                return "invalid";
        }
    }
};

#ifndef PS_SmsFormatType
#define PS_SmsFormatType(A) MtsStringDef::PS_SmsFormatType(A)
#endif

#endif
