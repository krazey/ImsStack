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

#ifndef MTS_DEF_H_
#define MTS_DEF_H_

#include "ByteArray.h"
#include "ImsTypeDef.h"

enum class SmsFormatType;

class SmsSendRequestInfo
{
public:
    SmsSendRequestInfo(IN SmsFormatType eInitSmsFormat, IN ByteArray* pInitContent,
            IN const AString& strInitAddress, IN IMS_SINT32 nInitSeqId,
            IN IMS_BOOL bInitEmergencyNumber, IN IMS_UINT32 nInitRetryCount) :
            eSmsFormat(eInitSmsFormat),
            pContent(pInitContent),
            strAddress(strInitAddress),
            nSeqId(nInitSeqId),
            bEmergencyNumber(bInitEmergencyNumber),
            nRetryCount(nInitRetryCount)
    {
    }

    SmsFormatType eSmsFormat;
    ByteArray* pContent;
    AString strAddress;
    IMS_SINT32 nSeqId;
    IMS_BOOL bEmergencyNumber;
    IMS_UINT32 nRetryCount;
};

enum class MtsTimerType
{
    TIMER_UNKNOWN = 0,
    TIMER_SMS_CALLBACK_MODE = 1,
    TIMER_RETRY_AFTER = 2,
};

enum class MtsServiceType
{
    NORMAL = 0,
    EMERGENCY,
};

enum class MtsTrafficStartResult
{
    TRAFFIC_READY = 0,
    TRAFFIC_AWAITING_SETUP = 1,
    TRAFFIC_NOT_ALLOWED = 2,
    TRAFFIC_NOT_FOUND = 3,
    TRAFFIC_UNKNOWN = 4,
};

enum class MtsTransactionType
{
    MESSAGE_TYPE_SEND = 1,
    MESSAGE_TYPE_RECEIVE = 2,
    MESSAGE_TYPE_INVALID = 3,
};

enum class SmsFormatType
{
    SMSFORMAT_INVALID = 0,
    SMSFORMAT_3GPP = 1,
    SMSFORMAT_3GPP2 = 2,
};

enum
{
    SMS_MTI_NONE = -1
};

class MtsRegRecoveryPolicy
{
public:
    enum Policy : IMS_SINT32
    {
        MTS_REG_RECOVERY_POLICY_NONE = -1,
        REGISTER_START,
        REGISTER_START_WITH_WLAN,
        REGISTER_REFRESH,
        REGISTER_STOP,
        REGISTER_REINITIATE,
        E_REGISTER_FAKE_WITH_NEXT_PCSCF,
        PCSCF_NEXT,
        PCSCF_NEXT_WITH_DISCOVERY,
        IPSEC_DISABLED,
        RETRY_COUNT_INCREASE,
        RETRY_COUNT_INCREASE_WITH_INITIAL_REGISTRATION,
        UPDATE_SIP_DELEGATE_REGISTRATION,
        TRIGGER_SIP_DELEGATE_DEREGISTRATION,
        TRIGGER_FULL_NETWORK_REGISTRATION,
        PLMN_BLOCK_WITH_TIMEOUT,
        E_REGISTER_FAKE_WITH_SAME_PCSCF
    };
};

// RP Data Unit type in 3GPP SMS
enum
{
    SMS_3GPP_MTI_RP_DATA_FROM_MS = 0,
    SMS_3GPP_MTI_RP_DATA_FROM_N = 1,
    SMS_3GPP_MTI_RP_ACK_FROM_MS = 2,
    SMS_3GPP_MTI_RP_ACK_FROM_N = 3,
    SMS_3GPP_MTI_RP_ERROR_FROM_MS = 4,
    SMS_3GPP_MTI_RP_ERROR_FROM_N = 5,
    SMS_3GPP_MTI_RP_SMMA = 6
};

// Bearer Data Unit type in 3GPP2 SMS
enum
{
    SMS_3GPP2_MTI_POINT_TO_POINT = 0,
    SMS_3GPP2_MTI_BROADCAST = 1,
    SMS_3GPP2_MTI_ACKNOWLEDGE = 2
};

enum
{
    URI_SCHEME_UNKNOWN = -1,
    URI_SCHEME_TEL,
    URI_SCHEME_SIP,
    URI_SCHEME_SIPS
};

enum
{
    MTS_RADIO_GUARD_TIMER_MS = 2000,
    MTS_RADIO_EXTENDED_GUARD_TIMER_MS = 8000
};

// Call type
enum
{
    CALL_TYPE_CS = 0,
    CALL_TYPE_NORMAL,
    CALL_TYPE_EMERGENCY
};

// Call state
enum
{
    CALL_STATE_UNKNOWN = -1,
    CALL_STATE_IDLE = 0,
    CALL_STATE_TERMINATING = 1,
    CALL_STATE_RINGBACK = 2,
    CALL_STATE_RINGING = 3,
    CALL_STATE_ALERTING = 4,
    CALL_STATE_OFFHOOK = 5
};

enum
{
    SESSION_TYPE_NONE = 0x00000000,
    SESSION_TYPE_VOIP = 0x00000001,
    SESSION_TYPE_VS = 0x00000002,
    SESSION_TYPE_VT = 0x00000004
};

// State of Service
enum
{
    STATE_INIT = 0,
    STATE_READY,
    STATE_LIMITED,
    STATE_NOTREADY
};

// SIP Header values
enum
{
    CONTENT_TRANSFER_ENCODING_BINARY
};

#endif
