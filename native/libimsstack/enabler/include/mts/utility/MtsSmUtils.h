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

#ifndef MTS_SM_UTIL_H_
#define MTS_SM_UTIL_H_

#include "ByteArray.h"
#include "MtsDef.h"

class MtsSmUtils final
{
public:
    MtsSmUtils();
    ~MtsSmUtils();

    IMS_SINT32 GetRpMr(IN const IMS_BYTE* pbySmsData);
    IMS_SINT32 GetRpMr(IN const ByteArray& objSmsData);
    IMS_SINT32 GetMti(IN SmsFormatType eSmsFormat, IN const IMS_BYTE* objSms);
    IMS_SINT32 GetMti(IN SmsFormatType eSmsFormat, IN const ByteArray& objSmsData);
    void PrintSmsDataBurst(IN const ByteArray& objSmsData);
    const IMS_CHAR* GetMtiStringFrom3gpp(IN const IMS_SINT32 nMti);
    const IMS_CHAR* GetMtiStringFrom3gpp2(IN const IMS_SINT32 nMti);

public:
    enum
    {
        MTS_SMS_TRX_TYPE_SEND = 1,
        MTS_SMS_TRX_TYPE_RECEIVE,
        MTS_SMS_TRX_TYPE_INVALID
    };

    enum
    {
        MTS_SMS_MTI_NONE = -1
    };

    // RP Data Unit type in 3GPP SMS
    enum
    {
        MTS_3GPP_MTI_RP_DATA_From_MS = 0,
        MTS_3GPP_MTI_RP_DATA_From_N = 1,
        MTS_3GPP_MTI_RP_ACK_From_MS = 2,
        MTS_3GPP_MTI_RP_ACK_From_N = 3,
        MTS_3GPP_MTI_RP_ERROR_From_MS = 4,
        MTS_3GPP_MTI_RP_ERROR_From_N = 5,
        MTS_3GPP_MTI_RP_SMMA = 6
    };

    // Bearer Data Unit type in 3GPP2 SMS
    enum
    {
        MTS_3GPP2_MTI_SMS_POINT_TO_POINT = 0,
        MTS_3GPP2_MTI_SMS_BROADCAST = 1,
        MTS_3GPP2_MTI_SMS_ACKNOWLEDGE = 2
    };
};

#endif
