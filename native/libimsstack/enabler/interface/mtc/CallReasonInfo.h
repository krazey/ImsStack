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
#include "IMSTypeDef.h"

struct CallReasonInfo
{
public:
    CallReasonInfo(IN IMS_SINT32 _nCode, IN IMS_SINT32 _nExtraCode, IN AString _strExtraMessage) :
            nCode(_nCode),
            nExtraCode(_nExtraCode),
            strExtraMessage(_strExtraMessage)
    {
    }

    CallReasonInfo(IN IMS_SINT32 _nCode, IN IMS_SINT32 _nExtraCode) :
            CallReasonInfo(_nCode, _nExtraCode, AString::ConstNull())
    {
    }

    CallReasonInfo(IN IMS_SINT32 _nCode) :
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

    AString ToString() const
    {
        AString strOut;
        strOut.Sprintf("Code[%d] Extra[%d][%s]", nCode, nExtraCode, strExtraMessage.GetStr());

        return strOut;
    }

    IMS_SINT32 nCode;
    IMS_SINT32 nExtraCode;
    AString strExtraMessage;
};

enum
{
    CODE_NONE = 0,
    CODE_UNSPECIFIED = 1,

    CODE_LOCAL_SERVICE_UNAVAILABLE = 101,
    CODE_LOCAL_NOT_REGISTERED = 102,
    CODE_LOCAL_POWER_OFF = 103,
    CODE_LOCAL_LOW_BATTERY = 104,
    CODE_LOW_BATTERY = 105,
    CODE_LOCAL_CALL_END_UNSPECIFIED = 106,

    CODE_LOCAL_NETWORK_NO_SERVICE = 201,
    CODE_LOCAL_NETWORK_NO_LTE_COVERAGE = 202,
    CODE_WIFI_LOST = 203,
    CODE_LOCAL_NETWORK_ROAMING = 204,
    CODE_LOCAL_NETWORK_IP_CHANGED = 205,
    CODE_CALL_BARRED = 206,
    CODE_ACCESS_CLASS_BLOCKED = 207,

    CODE_USER_TERMINATED = 301,
    CODE_USER_TERMINATED_BY_REMOTE = 302,
    CODE_USER_DECLINE = 303,
    CODE_USER_NOANSWER = 304,
    CODE_USER_IGNORE = 305,

    CODE_LOCAL_CALL_CS_RETRY_REQUIRED = 401,
    CODE_LOCAL_CALL_VOLTE_RETRY_REQUIRED = 402,
    CODE_NO_CSFB_IN_CS_ROAM = 403,

    CODE_ANSWERED_ELSEWHERE = 501,
    CODE_REJECTED_ELSEWHERE = 502,
    CODE_MAXIMUM_NUMBER_OF_CALLS_REACHED = 503,
    CODE_CALL_END_CAUSE_CALL_PULL = 504,

    CODE_MEDIA_INIT_FAILED = 601,
    CODE_MEDIA_NOT_ACCEPTABLE = 602,
    CODE_MEDIA_NO_DATA = 603,
    CODE_MEDIA_UNSPECIFIED = 604,

    CODE_SESSION_INTERNAL_ERROR = 701,
    CODE_LOCAL_ENDED_BY_CONFERENCE_MERGE = 702,
    CODE_TIMEOUT_1XX_WAITING = 703,
    CODE_TIMEOUT_NO_ANSWER = 704,
    CODE_TIMEOUT_NO_ANSWER_CALL_UPDATE = 705,
    CODE_NETWORK_RESP_TIMEOUT = 706,

    CODE_SIP_SERVER_ERROR = 801,
    CODE_SIP_FORBIDDEN = 802,
    CODE_SIP_NOT_FOUND = 803,
    CODE_SIP_NOT_SUPPORTED = 804,
    CODE_SIP_TEMPORARILY_UNAVAILABLE = 805,
    CODE_SIP_BAD_ADDRESS = 806,
    CODE_SIP_BUSY = 807,
    CODE_SIP_USER_REJECTED = 808,
    CODE_SIP_REQUEST_CANCELLED = 809,
    CODE_SIP_BAD_REQUEST = 810,
    CODE_SIP_NOT_ACCEPTABLE = 811,
    CODE_SIP_REQUEST_TIMEOUT = 812,
    CODE_SIP_NOT_REACHABLE = 813,
    CODE_SIP_REDIRECTED = 814,
    CODE_SIP_CLIENT_ERROR = 815,
    CODE_SIP_SERVICE_UNAVAILABLE = 816,
    CODE_SIP_REQUEST_PENDING = 817,

    CODE_REJECT_ONGOING_CALL_WAITING_DISABLED = 901,
    CODE_REJECT_VT_TTY_NOT_ALLOWED = 902,
    CODE_REJECT_ONGOING_E911_CALL = 903,
    CODE_REJECT_MAX_CALL_LIMIT_REACHED = 904,
    CODE_LOCAL_CALL_BUSY = 905,
    CODE_REJECT_UNSUPPORTED_SIP_HEADERS = 906,
    CODE_REJECT_ONGOING_CALL_SETUP = 907,
    CODE_REJECT_ONGOING_CS_CALL = 908,
    CODE_REJECT_CALL_ON_OTHER_SUB = 909,
    CODE_LOCAL_CALL_EXCEEDED = 910,
    CODE_LOCAL_CALL_RESOURCE_RESERVATION_FAILED = 911,
    CODE_REJECT_QOS_FAILURE = 912,

    CODE_EARLYDIALOG_FORKED_TERMINATED_INTERNALONLY = 1001,
    CODE_RETRY_AFTER_INTERNALONLY = 1002,
    CODE_REJECT_ONGOING_CALL_UPDATE = 1003,
};

// CODE_LOCAL_CALL_CS_RETRY_REQUIRED
enum
{
    EXTRA_CODE_CALL_RETRY_NORMAL = 0,
    EXTRA_CODE_CALL_RETRY_SILENT_REDIAL = 1,
    EXTRA_CODE_CALL_RETRY_EMERGENCY = 2,
};

// CODE_USER_TERMINATED
enum
{
    EXTRA_USER_TERMINATED_ECT = 0,
    EXTRA_USER_TERMINATED_AND_SIP_TIMEOUT = 1,
    EXTRA_USER_TERMINATED_AND_RTP_TIMEOUT = 2,
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

enum
{
    EXTRA_CODE_NOT_ACCEPTABLE_SIP_406 = 1,
    EXTRA_CODE_NOT_ACCEPTABLE_SIP_488 = 2,
    EXTRA_CODE_NOT_ACCEPTABLE_SIP_606 = 3,
};

#define _TRACE_CR_(I) (I.ToString().GetStr())

#endif
