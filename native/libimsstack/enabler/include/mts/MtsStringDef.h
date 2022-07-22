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

    inline static const IMS_CHAR* PS_MtiStringFrom3gpp(IN const IMS_SINT32 nMti)
    {
        switch (nMti)
        {
            case SMS_3GPP_MTI_RP_DATA_FROM_MS:
                return "SMS_3GPP_MTI_RP_DATA_FROM_MS";
            case SMS_3GPP_MTI_RP_DATA_FROM_N:
                return "SMS_3GPP_MTI_RP_DATA_FROM_N";
            case SMS_3GPP_MTI_RP_ACK_FROM_MS:
                return "SMS_3GPP_MTI_RP_ACK_FROM_MS";
            case SMS_3GPP_MTI_RP_ACK_FROM_N:
                return "SMS_3GPP_MTI_RP_ACK_FROM_N";
            case SMS_3GPP_MTI_RP_ERROR_FROM_MS:
                return "SMS_3GPP_MTI_RP_ERROR_FROM_MS";
            case SMS_3GPP_MTI_RP_ERROR_FROM_N:
                return "SMS_3GPP_MTI_RP_ERROR_FROM_N";
            case SMS_3GPP_MTI_RP_SMMA:
                return "SMS_3GPP_MTI_RP_SMMA";
            default:
                return "SMS 3GPP MTI INFO INVALID";
        }
    }

    inline static const IMS_CHAR* PS_MtiStringFrom3gpp2(IN const IMS_SINT32 nMti)
    {
        switch (nMti)
        {
            case SMS_3GPP2_MTI_POINT_TO_POINT:
                return "SMS_3GPP2_MTI_POINT_TO_POINT";
            case SMS_3GPP2_MTI_BROADCAST:
                return "SMS_3GPP2_MTI_BROADCAST";
            case SMS_3GPP2_MTI_ACKNOWLEDGE:
                return "SMS_3GPP2_MTI_ACKNOWLEDGE";
            default:
                return "SMS 3GPP2 MTI INFO INVALID";
        }
    }

    inline static const IMS_CHAR* PS_MoStatus(IN const IMS_SINT32 nReason)
    {
        switch (nReason)
        {
            case MO_SUCCESS:
                return "MO_SUCCESS";
            case MO_IMS_TEMP_FAILURE:
                return "MO_IMS_TEMP_FAILURE";
            case MO_IMS_PERM_FAILURE:
                return "MO_IMS_PERM_FAILURE";
            case MO_IMS_LIMITEDSMSSVCREGI:
                return "MO_IMS_LIMITEDSMSSVCREGI";
            case MO_RETRY_CS:
                return "MO_RETRY_CS";
            case MO_RETRY_CS_OR_SGS:
                return "MO_RETRY_CS_OR_SGS";
            default:
                return "MO_INVALID";
        }
    }

    inline static const IMS_CHAR* PS_CallState(IN const IMS_UINT32 nState)
    {
        switch (nState)
        {
            case CALL_STATE_IDLE:
                return "CALL_STATE_IDLE";
            case CALL_STATE_TERMINATING:
                return "CALL_STATE_TERMINATING";
            case CALL_STATE_RINGBACK:
                return "CALL_STATE_RINGBACK";
            case CALL_STATE_RINGING:
                return "CALL_STATE_RINGING";
            case CALL_STATE_ALERTING:
                return "CALL_STATE_ALERTING";
            case CALL_STATE_OFFHOOK:
                return "CALL_STATE_OFFHOOK";
            default:
                return "__INVALID__";
        }
    }
};

#ifndef PS_SmsFormatType
#define PS_SmsFormatType(A) MtsStringDef::PS_SmsFormatType(A)
#endif

#ifndef PS_MtiStringFrom3gpp
#define PS_MtiStringFrom3gpp(A) MtsStringDef::PS_MtiStringFrom3gpp(A)
#endif

#ifndef PS_MtiStringFrom3gpp2
#define PS_MtiStringFrom3gpp2(A) MtsStringDef::PS_MtiStringFrom3gpp2(A)
#endif

#ifndef PS_MoStatus
#define PS_MoStatus(A) MtsStringDef::PS_MoStatus(A)
#endif

#ifndef PS_CallState
#define PS_CallState(A) MtsStringDef::PS_CallState(A)
#endif

#endif
