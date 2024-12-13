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

class AoSAppRequest
{
public:
    // --------- Get Request [START] ---------
    enum
    {
        STATE_NONE = 0,

        // AoSRegMode class
        STATE_REGISTRATION_MODE,

        // AosReason class
        STATE_IMS_DISCONNECTED_REASON,
        STATE_IMS_SUSPENDED_REASON,

        /*
            AoSIMSService class
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

class AoSSCBMInfo
{
public:
    enum
    {
        STARTED = 0,
        TERMINATED,
        TERMINATED_ECALL,
        TERMINATED_ESMS,
    };
};

class AoSRegisterInfo
{
public:
    enum
    {
        ECALL_INIT = 0,
        ECALL_DONE,
        ESMS_INIT,
        ESMS_DONE
    };
};

class AoSIMSService
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

class AoSIMSRetryCounter
{
public:
    enum
    {
        RESET = 0,
        INCREASE,
        DECREASE
    };
};

class AoSPCSCFRecovery
{
public:
    enum
    {
        DEFAULT_RULE = 0,
        DATA_RECOVERY
    };
};

class AoSRegMode
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

class AoSRegRecoveryType
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

class AoSRegType
{
public:
    enum
    {
        TYPE_IPCAN_MOBILE = 0,
        TYPE_IPCAN_WLAN = 1111
    };
};

class AoSServiceControlParam
{
public:
    enum
    {
        /*
         * Enablers (i.e. UC) realize that WiFi calling is not allowed
         * in this location (country). AoS blocks WiFi calling service
         * until UE moves to another country.
         */
        VOWIFI_BLOCKED_LOCATION = 1
    };
};

class AoSNetWorkType
{
public:
    enum
    {
        NW_TYPE_NONE = (0x00000000),
        NW_TYPE_EPDG = (0x00010000),
        NW_TYPE_AMPS = (0x00020000),
        NW_TYPE_CDMA = (0x00040000),
        NW_TYPE_GSM = (0x00080000),
        NW_TYPE_HDR = (0x00100000),
        NW_TYPE_WCDMA = (0x00200000),
        NW_TYPE_GPS = (0x00400000),
        NW_TYPE_EDGE = (0x00800000),
        NW_TYPE_WLAN = (0x01000000),
        NW_TYPE_CDMA1X = (0x02000000),
        NW_TYPE_EVDODO = (0x04000000),
        NW_TYPE_EVDORA = (0x08000000),
        NW_TYPE_EHRPD = (0x10000000),
        NW_TYPE_LTE = (0x20000000),
        NW_TYPE_HSPA = (0x40000000),
        NW_TYPE_NR = (0x80000000),
        NW_TYPE__MAX
    };
};

class AoSRegProtectedType
{
public:
    enum
    {
        REG_UNPROTECTED = 0,
        REG_PROTECTED
    };
};

class AoSSupportability
{
public:
    enum
    {
        NOT_SUPPORTED = 0,
        SUPPORTED
    };
};

class AoSFakeERegType
{
public:
    enum
    {
        TYPE_SAME_PCSCF = 0,
        TYPE_NEXT_PCSCF
    };
};

class AoSRegFeatureType
{
public:
    enum
    {
        TYPE_RCS_CHATBOT = (0x00000001),
        TYPE_RCS_XBOT = (0x00000002),
        TYPE_RCS_CHATBOT_SA = (0x00000004),
        TYPE_RCS_HTTPFT = (0x00000008),
        TYPE_RCS_GEOPUSH = (0x00000010),
        TYPE_RCS_FTSMS = (0x00000020),
        TYPE_RCS_GEOSMS = (0x00000040),
        TYPE_RCS_PRESENCE = (0x00000080),
        TYPE_RCS_MOBILITY = (0x00000100),
        TYPE_RCS_TELEPHONY_CS = (0x00000200),
        TYPE_RCS_CALL_COMPOSER = (0x00000400),
        TYPE_RCS_SHARED_MAP = (0x00000800),
        TYPE_RCS_SHARED_SKETCH = (0x00001000),
        TYPE_RCS_CALL_UNANSWERED = (0x00002000),
        TYPE_RCS_CPM_SESSION = (0x00004000),
        TYPE_RCS_CPM_FT = (0x00008000),
        TYPE_RCS_CPM_MSG = (0x00010000),
        TYPE_RCS_CPM_LARGEMSG = (0x00020000),
        TYPE_RCS_CPM_SYSMSG = (0x00040000),
        TYPE_RCS_CANCEL_MSG = (0x00080000)
    };
};

#endif  // AOS_APP_REQ_TYPE_H_
