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
#ifndef AOS_APP_REQ_TYPE_H_
#define AOS_APP_REQ_TYPE_H_

class AosAppRequest
{
public:
    // --------- Get Request [START] ---------
    enum
    {
        STATE_NONE = 0,

        // AosRegMode class
        STATE_REGISTRATION_MODE,

        // AosReason class
        STATE_IMS_DISCONNECTED_REASON,
        STATE_IMS_SUSPENDED_REASON,

        /*
            AosImsService class
            if state is available, enabler can request regi command or etc ..
        */
        STATE_IMS_SERVICE,

        STATE_IMS_NET_POLICY,

        STATE_REG_NW_TYPE,

        STATE_PCSCF_PORT,

        STATE_LOCAL_PORT,

        STATE_PCSCF_ADDRESS,

        STATE_LOCAL_ADDRESS,

        STATE_ASSOCIATED_URI,

        STATE_IPCAN_TYPE,

        // PATH Header value
        STATE_PATH,

        // Service-Route Header value
        STATE_SERVICE_ROUTE,

        STATE_LAST_PATH,

        STATE_SUPPORTED,

        STATE_PROTECTED,

        STATE_SUPPORT_CALLING_NUMBER_VERIFICATION,

        // Operator Specific state 100 ~
        STATE_OPERATOR = 100,
    };
    // --------- Get Request [END] ---------

    // --------- Command Request [START] ---------
    enum
    {
        COMMAND_REGISTER = 100,
        COMMAND_REGISTER_RECOVERY,
        COMMAND_REGISTER_STOP,
        COMMAND_REGISTER_NEXT_PCSCF,
        COMMAND_REGISTER_REFRESH,

        COMMAND_SET_CALL_FALLBACK = 110,

        COMMAND_SET_PUBLISH_STARTED = 120,
        COMMAND_SET_PUBLISH_TERMINATED,

        COMMAND_SET_UNBLOCK_AC_INCOMPLTED = 130,
        COMMAND_SET_BLOCK_AC_INCOMPLTED,
        COMMAND_SET_AC_CONFIGURED,
        COMMAND_SET_SERVICE_FEATURE,
        COMMAND_FAKE_E_REGISTER,

        /*
            for RCSNA, it triggers re-Register because of 403 response to CPM INVITE.
        */
        COMMAND_HANDLE_INSTANTANEOUS_OFFLINE = 140,

        COMMAND_NO_RTP_PING_CHECK = 150,

        COMMAND_REGINFO_UPDATE = 160,

        COMMAND_SERVICE_CONTROL = 170,

        // SMS to 911 for VZW
        COMMAND_SCBM = 180,
        COMMAND_REGISTER_INFO,

        // Operator Specific Command 200 ~
        COMMAND_OPERATOR = 200
    };
    // --------- Command Request [END] ---------
};

class AosImsService
{
public:
    enum
    {
        AVAILABLE = 0,
        UNAVAILABLE,
        PENDING,
        FORBIDDEN,
        UNSUBSCRIBED
    };
};

class AosPcscfRecovery
{
public:
    enum
    {
        DEFAULT_RULE = 0,
        DATA_RECOVERY
    };
};

class AosRegMode
{
public:
    enum
    {
        UNKNOWN = 0,
        NORMAL,
        ADMIN,
        INTERNAL,
        NOUICC
    };
};

class AosRegRecoveryType
{
public:
    enum
    {
        UNKNOWN = 0,
        REDIAL_TO1XRTT,
        SERVER_OUTAGE,
        SERVER_OUTAGE_IMPLICITLY,
        UNPROTECTED_IPSEC,
        KEEP_DATA_CONNECTION,
        RECOVER_AFTER_CSFB,
        PCSCF_CHANGE,
        PDN_RECONNECT,
        SELECT_CELLULAR_NETWORK
    };
};

class AosRegType
{
public:
    enum
    {
        TYPE_IPCAN_MOBILE = 0,
        TYPE_IPCAN_WLAN = 1111
    };
};

class AosRegProtectedType
{
public:
    enum
    {
        REG_UNPROTECTED = 0,
        REG_PROTECTED
    };
};

class AosSupportability
{
public:
    enum
    {
        NOT_SUPPORTED = 0,
        SUPPORTED
    };
};

#endif  // AOS_APP_REQ_TYPE_H_
