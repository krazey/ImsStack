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

#include "IImsRadio.h"
#include "INetworkWatcher.h"
#include "IIpcan.h"
#include "ImsTypeDef.h"
#include "IuMtsService.h"
#include "MtsDef.h"

class MtsStringDef
{
public:
    inline static const IMS_CHAR* PS_AccessNetworkType(IN IMS_UINT32 nAccessNetworkType)
    {
        switch (nAccessNetworkType)
        {
            case IImsRadio::ACCESS_NETWORK_TYPE_UTRAN:
                return "UTRAN";
            case IImsRadio::ACCESS_NETWORK_TYPE_EUTRAN:
                return "EUTRAN";
            case IImsRadio::ACCESS_NETWORK_TYPE_NGRAN:
                return "NGRAN";
            case IImsRadio::ACCESS_NETWORK_TYPE_IWLAN:
                return "IWLAN";
            default:
                return "__INVALID__";
        }
    }

    inline static const IMS_CHAR* PS_Ipcan(IN const IMS_UINT32 nIpcan)
    {
        switch (nIpcan)
        {
            case IIpcan::CATEGORY_MOBILE:
                return "CATEGORY_MOBILE";
            case IIpcan::CATEGORY_WLAN:
                return "CATEGORY_WLAN";
            case IIpcan::CATEGORY_ANY:
                return "CATEGORY_ANY";
            default:
                return "__INVALID__";
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
                return "__INVALID__";
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
                return "__INVALID__";
        }
    }

    inline static const IMS_CHAR* PS_MoStatus(IN const IMS_SINT32 nReason)
    {
        switch (nReason)
        {
            case MO_SUCCESS:
                return "MO_SUCCESS";
            case MO_ERROR_GENERIC:
                return "MO_ERROR_GENERIC";
            case MO_ERROR_RETRY:
                return "MO_ERROR_RETRY";
            default:
                return "__INVALID__";
        }
    }

    inline static const IMS_CHAR* PS_RadioTechType(IN IMS_SINT32 nRadioTechType)
    {
        switch (nRadioTechType)
        {
            case INetworkWatcher::RADIOTECH_TYPE_UNKNOWN:
                return "UNKNOWN";
            case INetworkWatcher::RADIOTECH_TYPE_GPRS:
                return "GPRS";
            case INetworkWatcher::RADIOTECH_TYPE_EDGE:
                return "EDGE";
            case INetworkWatcher::RADIOTECH_TYPE_UMTS:
                return "UMTS";
            case INetworkWatcher::RADIOTECH_TYPE_CDMA:
                return "CDMA";
            case INetworkWatcher::RADIOTECH_TYPE_EVDO_0:
                return "EVDO_0";
            case INetworkWatcher::RADIOTECH_TYPE_EVDO_A:
                return "EVDO_A";
            case INetworkWatcher::RADIOTECH_TYPE_1xRTT:
                return "1xRTT";
            case INetworkWatcher::RADIOTECH_TYPE_HSDPA:
                return "HSDPA";
            case INetworkWatcher::RADIOTECH_TYPE_HSUPA:
                return "HSUPA";
            case INetworkWatcher::RADIOTECH_TYPE_HSPA:
                return "HSPA";
            case INetworkWatcher::RADIOTECH_TYPE_IDEN:
                return "IDEN";
            case INetworkWatcher::RADIOTECH_TYPE_EVDO_B:
                return "EVDO_B";
            case INetworkWatcher::RADIOTECH_TYPE_LTE:
                return "LTE";
            case INetworkWatcher::RADIOTECH_TYPE_EHRPD:
                return "EHRPD";
            case INetworkWatcher::RADIOTECH_TYPE_HSPAP:
                return "HSPAP";
            case INetworkWatcher::RADIOTECH_TYPE_GSM:
                return "GSM";
            case INetworkWatcher::RADIOTECH_TYPE_TD_SCDMA:
                return "TD_SCDMA";
            case INetworkWatcher::RADIOTECH_TYPE_IWLAN:
                return "IWLAN";
            case INetworkWatcher::RADIOTECH_TYPE_LTE_CA:
                return "LTE_CA";
            case INetworkWatcher::RADIOTECH_TYPE_NR:
                return "NR";
            case INetworkWatcher::RADIOTECH_TYPE_MAX:
                return "MAX";
            default:
                return "__INVALID__";
        }
    }

    inline static const IMS_CHAR* PS_ServiceState(IN IMS_SINT32 nState)
    {
        switch (nState)
        {
            case STATE_INIT:
                return "STATE_INIT";
            case STATE_READY:
                return "STATE_READY";
            case STATE_LIMITED:
                return "STATE_LIMITED";
            case STATE_NOTREADY:
                return "STATE_NOTREADY";
            default:
                return "__INVALID__";
        }
    }

    inline static const IMS_CHAR* PS_SmsFormatType(IN SmsFormatType eSmsFormat)
    {
        switch (eSmsFormat)
        {
            case SmsFormatType::SMSFORMAT_3GPP:
                return "3GPP";
            case SmsFormatType::SMSFORMAT_3GPP2:
                return "3GPP2";
            default:
                return "__INVALID__";
        }
    }

    inline static const IMS_CHAR* PS_TrafficDirection(IN IMS_UINT32 nTrafficDirection)
    {
        switch (nTrafficDirection)
        {
            case IImsRadio::DIRECTION_MO:
                return "MO";
            case IImsRadio::DIRECTION_MT:
                return "MT";
            default:
                return "__INVALID__";
        }
    }

    inline static const IMS_CHAR* PS_TrafficType(IN IMS_UINT32 nTrafficType)
    {
        switch (nTrafficType)
        {
            case IImsRadio::TRAFFIC_TYPE_SMS:
                return "SMS";
            case IImsRadio::TRAFFIC_TYPE_EMERGENCY_SMS:
                return "EMERGENCY_SMS";
            default:
                return "__INVALID__";
        }
    }

    inline static const IMS_CHAR* PS_SipHeaderValue(IN IMS_UINT32 nSipHeaderValue)
    {
        switch (nSipHeaderValue)
        {
            case CONTENT_TRANSFER_ENCODING_BINARY:
                return "binary";
            default:
                return "__INVALID__";
        }
    }
};

#ifndef PS_AccessNetworkType
#define PS_AccessNetworkType(A) MtsStringDef::PS_AccessNetworkType(A)
#endif

#ifndef PS_Ipcan
#define PS_Ipcan(A) MtsStringDef::PS_Ipcan(A)
#endif

#ifndef PS_MoStatus
#define PS_MoStatus(A) MtsStringDef::PS_MoStatus(A)
#endif

#ifndef PS_MtiStringFrom3gpp
#define PS_MtiStringFrom3gpp(A) MtsStringDef::PS_MtiStringFrom3gpp(A)
#endif

#ifndef PS_MtiStringFrom3gpp2
#define PS_MtiStringFrom3gpp2(A) MtsStringDef::PS_MtiStringFrom3gpp2(A)
#endif

#ifndef PS_RadioTechType
#define PS_RadioTechType(A) MtsStringDef::PS_RadioTechType(A)
#endif

#ifndef PS_ServiceState
#define PS_ServiceState(A) MtsStringDef::PS_ServiceState(A)
#endif

#ifndef PS_SmsFormatType
#define PS_SmsFormatType(A) MtsStringDef::PS_SmsFormatType(A)
#endif

#ifndef PS_TrafficDirection
#define PS_TrafficDirection(A) MtsStringDef::PS_TrafficDirection(A)
#endif

#ifndef PS_TrafficType
#define PS_TrafficType(A) MtsStringDef::PS_TrafficType(A)
#endif

#ifndef PS_SipHeaderValue
#define PS_SipHeaderValue(A) MtsStringDef::PS_SipHeaderValue(A)
#endif

#endif
