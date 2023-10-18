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

#include "ImsStrLib.h"
#include "ServiceTrace.h"
#include "utility/MtsSmUtils.h"

__IMS_TRACE_TAG_COM_MTS__;

PUBLIC
MtsSmUtils::MtsSmUtils()
{
    IMS_TRACE_I("+MtsSmUtils", 0, 0, 0);
}

PUBLIC
MtsSmUtils::~MtsSmUtils()
{
    IMS_TRACE_I("~MtsSmUtils", 0, 0, 0);
}

PUBLIC
IMS_SINT32 MtsSmUtils::GetRpMr(IN const IMS_BYTE* pbyContent)
{
    if (pbyContent == IMS_NULL)
    {
        return (-1);
    }

    return static_cast<IMS_SINT32>(pbyContent[1]);
}

PUBLIC
IMS_SINT32 MtsSmUtils::GetRpMr(IN const ByteArray& objContent)
{
    if (objContent.GetLength() < 2)
    {
        return (-1);
    }

    return GetRpMr(objContent.GetData());
}

PUBLIC
IMS_SINT32 MtsSmUtils::GetMti(IN SmsFormatType eSmsFormat, IN const IMS_BYTE* pbyContent)
{
    if (pbyContent != IMS_NULL)
    {
        if (eSmsFormat == SmsFormatType::SMSFORMAT_3GPP)
        {
            return static_cast<IMS_SINT32>(pbyContent[0] & 0x07);
        }
        else if (eSmsFormat == SmsFormatType::SMSFORMAT_3GPP2)
        {
            return static_cast<IMS_SINT32>(pbyContent[0]);
        }
    }

    return (-1);
}

PUBLIC
IMS_SINT32 MtsSmUtils::GetMti(IN SmsFormatType eSmsFormat, IN const ByteArray& objContent)
{
    if (objContent.GetLength() == 0)
    {
        return (-1);
    }

    return GetMti(eSmsFormat, objContent.GetData());
}
