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

#ifndef CALL_REASON_INFO_H_
#define CALL_REASON_INFO_H_

#include "AString.h"
#include "ImsTypeDef.h"

enum
{
    CODE_NONE = -1,

    // ref. ImsReasonInfo.java
    CODE_UNSPECIFIED = 0,
    CODE_LOCAL_ILLEGAL_ARGUMENT = 101,
    CODE_LOCAL_ILLEGAL_STATE = 102,
    CODE_LOCAL_INTERNAL_ERROR = 103,
    CODE_LOCAL_ENDED_BY_CONFERENCE_MERGE = 108,
    CODE_LOCAL_POWER_OFF = 111,
    CODE_LOCAL_LOW_BATTERY = 112,
    CODE_LOCAL_NETWORK_NO_SERVICE = 121,
    CODE_LOCAL_NETWORK_NO_LTE_COVERAGE = 122,
    CODE_LOCAL_SERVICE_UNAVAILABLE = 131,
    CODE_LOCAL_NOT_REGISTERED = 132,
    CODE_LOCAL_CALL_EXCEEDED = 141,
    CODE_LOCAL_CALL_BUSY = 142,
    CODE_LOCAL_CALL_DECLINE = 143,
    CODE_LOCAL_CALL_VCC_ON_PROGRESSING = 144,
    CODE_LOCAL_CALL_RESOURCE_RESERVATION_FAILED = 145,
    CODE_LOCAL_CALL_CS_RETRY_REQUIRED = 146,
    CODE_LOCAL_CALL_VOLTE_RETRY_REQUIRED = 147,
    CODE_LOCAL_CALL_TERMINATED = 148,
    CODE_LOCAL_HO_NOT_FEASIBLE = 149,
    CODE_TIMEOUT_1XX_WAITING = 201,
    CODE_TIMEOUT_NO_ANSWER = 202,
    CODE_TIMEOUT_NO_ANSWER_CALL_UPDATE = 203,
    CODE_CALL_BARRED = 240,
    CODE_SIP_REDIRECTED = 321,
    CODE_SIP_BAD_REQUEST = 331,
    CODE_SIP_FORBIDDEN = 332,
    CODE_SIP_NOT_FOUND = 333,
    CODE_SIP_NOT_SUPPORTED = 334,
    CODE_SIP_REQUEST_TIMEOUT = 335,
    CODE_SIP_TEMPRARILY_UNAVAILABLE = 336,  // typo in ImsCallReason
    CODE_SIP_BAD_ADDRESS = 337,
    CODE_SIP_BUSY = 338,
    CODE_SIP_REQUEST_CANCELLED = 339,
    CODE_SIP_NOT_ACCEPTABLE = 340,
    CODE_SIP_NOT_REACHABLE = 341,
    CODE_SIP_CLIENT_ERROR = 342,
    CODE_SIP_TRANSACTION_DOES_NOT_EXIST = 343,
    CODE_SIP_SERVER_INTERNAL_ERROR = 351,
    CODE_SIP_SERVICE_UNAVAILABLE = 352,
    CODE_SIP_SERVER_TIMEOUT = 353,
    CODE_SIP_SERVER_ERROR = 354,
    CODE_SIP_USER_REJECTED = 361,
    CODE_SIP_GLOBAL_ERROR = 362,
    CODE_SIP_METHOD_NOT_ALLOWED = 366,
    CODE_SIP_PROXY_AUTHENTICATION_REQUIRED = 367,
    CODE_SIP_REQUEST_ENTITY_TOO_LARGE = 368,
    CODE_SIP_REQUEST_URI_TOO_LARGE = 369,
    CODE_SIP_EXTENSION_REQUIRED = 370,
    CODE_SIP_INTERVAL_TOO_BRIEF = 371,
    CODE_SIP_CALL_OR_TRANS_DOES_NOT_EXIST = 372,
    CODE_SIP_LOOP_DETECTED = 373,
    CODE_SIP_TOO_MANY_HOPS = 374,
    CODE_SIP_AMBIGUOUS = 376,
    CODE_SIP_REQUEST_PENDING = 377,
    CODE_SIP_UNDECIPHERABLE = 378,
    CODE_MEDIA_INIT_FAILED = 401,
    CODE_MEDIA_NO_DATA = 402,
    CODE_MEDIA_NOT_ACCEPTABLE = 403,
    CODE_MEDIA_UNSPECIFIED = 404,
    CODE_USER_TERMINATED = 501,
    CODE_USER_NOANSWER = 502,
    CODE_USER_IGNORE = 503,
    CODE_USER_DECLINE = 504,
    CODE_LOW_BATTERY = 505,
    CODE_BLACKLISTED_CALL_ID = 506,
    CODE_USER_TERMINATED_BY_REMOTE = 510,
    CODE_USER_REJECTED_SESSION_MODIFICATION = 511,
    CODE_USER_CANCELLED_SESSION_MODIFICATION = 512,
    CODE_SESSION_MODIFICATION_FAILED = 1517,
    CODE_MULTIENDPOINT_NOT_SUPPORTED = 902,
    CODE_ANSWERED_ELSEWHERE = 1014,
    CODE_CALL_PULL_OUT_OF_SYNC = 1015,
    CODE_CALL_END_CAUSE_CALL_PULL = 1016,
    CODE_REJECTED_ELSEWHERE = 1017,
    CODE_SUPP_SVC_FAILED = 1201,
    CODE_SUPP_SVC_CANCELLED = 1202,
    CODE_SUPP_SVC_REINVITE_COLLISION = 1203,
    CODE_MAXIMUM_NUMBER_OF_CALLS_REACHED = 1403,
    CODE_REMOTE_CALL_DECLINE = 1404,
    CODE_WIFI_LOST = 1407,
    CODE_RADIO_INTERNAL_ERROR = 1502,
    CODE_NETWORK_RESP_TIMEOUT = 1503,
    CODE_ACCESS_CLASS_BLOCKED = 1512,
    CODE_SIP_ALTERNATE_EMERGENCY_CALL = 1514,
    CODE_REJECT_UNKNOWN = 1600,
    CODE_REJECT_ONGOING_CALL_WAITING_DISABLED = 1601,
    CODE_REJECT_CALL_ON_OTHER_SUB = 1602,
    CODE_REJECT_SERVICE_NOT_REGISTERED = 1604,
    CODE_REJECT_CALL_TYPE_NOT_ALLOWED = 1605,
    CODE_REJECT_ONGOING_E911_CALL = 1606,
    CODE_REJECT_ONGOING_CALL_SETUP = 1607,
    CODE_REJECT_MAX_CALL_LIMIT_REACHED = 1608,
    CODE_REJECT_UNSUPPORTED_SIP_HEADERS = 1609,
    CODE_REJECT_ONGOING_CALL_TRANSFER = 1611,
    CODE_REJECT_INTERNAL_ERROR = 1612,
    CODE_REJECT_QOS_FAILURE = 1613,
    CODE_REJECT_VT_TTY_NOT_ALLOWED = 1615,
    CODE_REJECT_ONGOING_CALL_UPGRADE = 1616,
    CODE_REJECT_ONGOING_CONFERENCE_CALL = 1618,
    CODE_REJECT_ONGOING_CS_CALL = 1621,

    // used by ImsStackNative only internally.
    CODE_INTERNAL_EARLYDIALOG_FORKED_TERMINATED = 10001,
    CODE_INTERNAL_REDIAL = 10002,
    CODE_INTERNAL_RRC_REJECT = 10003,
};

// CODE_LOCAL_CALL_CS_RETRY_REQUIRED
enum
{
    EXTRA_CODE_CALL_RETRY_NORMAL = 0,
    EXTRA_CODE_CALL_RETRY_SILENT_REDIAL = 1,
    EXTRA_CODE_CALL_RETRY_EMERGENCY = 2,
};

// CODE_INTERNAL_REDIAL
enum
{
    EXTRA_CODE_REDIAL_BY_RETRY_AFTER = 0,
    EXTRA_CODE_REDIAL_BY_REQUEST_TIMEOUT = 1,
    EXTRA_CODE_REDIAL_FOR_REDIRECTION = 2,
    EXTRA_CODE_REDIAL_FOR_SDP_CHANGE = 3,  // TODO: just full capa or reflect SDP contents
    EXTRA_CODE_REDIAL_AFTER_EPS_FALLBACK = 4,
    EXTRA_CODE_REDIAL_EMERGENCY_WITH_NEXT_PCSCF = 5,
    EXTRA_CODE_REDIAL_WITH_NEXT_PCSCF = 6,
};

// CODE_USER_TERMINATED
enum
{
    EXTRA_USER_TERMINATED_ECT = 0,
    EXTRA_USER_TERMINATED_AND_SIP_TIMEOUT = 1,
    EXTRA_USER_TERMINATED_AND_RTP_TIMEOUT = 2,
};

// CODE_SIP_NOT_ACCEPTABLE
enum
{
    EXTRA_CODE_NOT_ACCEPTABLE_BY_CALL_TYPE = 0,
    EXTRA_CODE_NOT_ACCEPTABLE_SIP_406 = 1,
    EXTRA_CODE_NOT_ACCEPTABLE_SIP_488 = 2,
    EXTRA_CODE_NOT_ACCEPTABLE_SIP_606 = 3,
};

// CODE_SIP_ and CODE_NETWORK_RESP_TIMEOUT
enum
{
    EXTRA_CODE_METHOD_INVITE = 0,
    EXTRA_CODE_METHOD_PRACK = 1,
    EXTRA_CODE_METHOD_UPDATE = 2,
};

enum
{
    EXTRA_CODE_EMERGENCYSERVICE_INVALID = -1,
    EXTRA_CODE_EMERGENCYSERVICE_GENERIC = 0,
    EXTRA_CODE_EMERGENCYSERVICE_AMBULANCE = 1,
    EXTRA_CODE_EMERGENCYSERVICE_ANIMAL_CONTROL = 2,
    EXTRA_CODE_EMERGENCYSERVICE_FIRE = 3,
    EXTRA_CODE_EMERGENCYSERVICE_GAS = 4,
    EXTRA_CODE_EMERGENCYSERVICE_MARINE = 5,
    EXTRA_CODE_EMERGENCYSERVICE_MOUNTAIN = 6,
    EXTRA_CODE_EMERGENCYSERVICE_PHYSICIAN = 7,
    EXTRA_CODE_EMERGENCYSERVICE_POISON = 8,
    EXTRA_CODE_EMERGENCYSERVICE_POLICE = 9,
    EXTRA_CODE_EMERGENCYSERVICE_COUNTRY_SPECIFIC = 10,
};

struct CallReasonInfo
{
public:
    CallReasonInfo(
            IN IMS_SINT32 _nCode, IN IMS_SINT32 _nExtraCode, IN const AString& _strExtraMessage) :
            nCode(_nCode),
            nExtraCode(_nExtraCode),
            strExtraMessage(_strExtraMessage)
    {
    }

    CallReasonInfo(IN IMS_SINT32 _nCode, IN IMS_SINT32 _nExtraCode) :
            CallReasonInfo(_nCode, _nExtraCode, AString::ConstNull())
    {
    }

    explicit CallReasonInfo(IN IMS_SINT32 _nCode) :
            CallReasonInfo(_nCode, -1)
    {
    }

    CallReasonInfo(IN const CallReasonInfo& objRhs) :
            nCode(objRhs.nCode),
            nExtraCode(objRhs.nExtraCode),
            strExtraMessage(objRhs.strExtraMessage)
    {
    }

    CallReasonInfo& operator=(const CallReasonInfo& objRhs)
    {
        if (this != &objRhs)
        {
            nCode = objRhs.nCode;
            nExtraCode = objRhs.nExtraCode;
            strExtraMessage = objRhs.strExtraMessage;
        }

        return *this;
    }

    IMS_BOOL operator==(const CallReasonInfo& objRhs) const
    {
        if (this == &objRhs)
        {
            return IMS_TRUE;
        }

        return nCode == objRhs.nCode && nExtraCode == objRhs.nExtraCode &&
                strExtraMessage.Equals(objRhs.strExtraMessage);
    }

    IMS_BOOL operator!=(const CallReasonInfo& objRhs) const { return !(*this == objRhs); }

    inline static IMS_BOOL IsTerminateRequired(IN IMS_SINT32 nCode)
    {
        switch (nCode)
        {
            case CODE_LOCAL_NOT_REGISTERED:
            case CODE_LOCAL_NETWORK_NO_SERVICE:
            case CODE_LOCAL_NETWORK_NO_LTE_COVERAGE:
            case CODE_LOCAL_CALL_VCC_ON_PROGRESSING:
                return IMS_FALSE;
            default:
                return IMS_TRUE;
        }
    }

    AString ToString() const
    {
        AString strOut;
        strOut.Sprintf("Code[%d] Extra[%d][%s]", nCode, nExtraCode, strExtraMessage.GetStr());

        return strOut;
    }

    CallReasonInfo ConvertFromInternal() const
    {
        switch (nCode)
        {
            case CODE_INTERNAL_EARLYDIALOG_FORKED_TERMINATED:
            case CODE_INTERNAL_REDIAL:
                // No scenario to be notified to the Java
                return CallReasonInfo(CODE_LOCAL_ILLEGAL_STATE);
            case CODE_INTERNAL_RRC_REJECT:
                return CallReasonInfo(CODE_LOCAL_NETWORK_NO_SERVICE);
            default:
                break;
        }
        return *this;
    }

    IMS_SINT32 nCode;
    IMS_SINT32 nExtraCode;
    AString strExtraMessage;
};

#define _TRACE_CR_(I) ((I).ToString().GetStr())

#endif
