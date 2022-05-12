package com.android.imsstack.enabler.mtc;

public class IUMtcCall {

    public static final int EVENT_U2I        = 1200;
    public static final int EVENT_I2U        = 1300;

    // Event : UI to IMS
    public static final int START               = (EVENT_U2I + 1);
    public static final int STARTCONF           = (EVENT_U2I + 2);
    public static final int USER_ALERT          = (EVENT_U2I + 3);
    public static final int ACCEPT              = (EVENT_U2I + 4);
    public static final int REJECT              = (EVENT_U2I + 5);
    public static final int HOLD                = (EVENT_U2I + 6);
    public static final int RESUME              = (EVENT_U2I + 7);
    public static final int SEND_DTMF           = (EVENT_U2I + 8);
    public static final int TERMINATE           = (EVENT_U2I + 9);
    public static final int UPDATE              = (EVENT_U2I + 10);
    public static final int ACCEPT_UPDATE       = (EVENT_U2I + 11);
    public static final int REJECT_UPDATE       = (EVENT_U2I + 12);
    public static final int CANCEL_UPDATE       = (EVENT_U2I + 13);
    public static final int ACCEPT_RESUME       = (EVENT_U2I + 14);
    public static final int REJECT_RESUME       = (EVENT_U2I + 15);
    // Google_IMS_IF :: USSD {
    public static final int SEND_USSD           = (EVENT_U2I + 16);
    // Google_IMS_IF :: USSD }
    public static final int REQUEST_ECT         = (EVENT_U2I + 61);
    public static final int REQUEST_CALL_PUSH    = (EVENT_U2I + 62);
    public static final int CANCEL_CALL_PUSH    = (EVENT_U2I + 63);
    public static final int REQUEST_ECT_BLIND   = (EVENT_U2I + 64);
    public static final int ATTACH              = (EVENT_U2I + 98);
    public static final int OPEN                = (EVENT_U2I + 99);

    // Event : IMS to UI
    public static final int STARTED                 = (EVENT_I2U + 1);
    public static final int START_FAILED            = (EVENT_I2U + 2);
    public static final int PROGRESSING             = (EVENT_I2U + 3);
    public static final int HELD                    = (EVENT_I2U + 4);
    public static final int HOLD_FAILED             = (EVENT_I2U + 5);
    public static final int HELD_BY                 = (EVENT_I2U + 6);
    public static final int RESUMED                 = (EVENT_I2U + 7);
    public static final int RESUME_FAILED           = (EVENT_I2U + 8);
    public static final int RESUMED_BY              = (EVENT_I2U + 9);
    public static final int TERMINATED              = (EVENT_I2U + 10);
    public static final int INCOMING_UPDATE         = (EVENT_I2U + 11);
    public static final int UPDATED                 = (EVENT_I2U + 12);
    public static final int UPDATE_FAILED           = (EVENT_I2U + 13);
    public static final int UPDATED_BY              = (EVENT_I2U + 14);
    public static final int NOTIFY_INFO             = (EVENT_I2U + 15);
    public static final int INCOMING_RESUME         = (EVENT_I2U + 16);
    public static final int SET_PROPERTY            = (EVENT_I2U + 17);
    public static final int INCOMING_CALL_RECEIVED  = (EVENT_I2U + 18);

    public static final int ECT_COMPLETED           = (EVENT_I2U + 61);
    public static final int REPLACED_BY             = (EVENT_I2U + 62);
    public static final int CALL_PUSH_COMPLETED     = (EVENT_I2U + 63);

    public static final int CODEC_INFO_UPDATED = (EVENT_I2U + 91);

    // Call Type
    public static final int VOLTE_CALL_TYPE_NORMAL = 0;
    public static final int VOLTE_CALL_TYPE_EMERGENCY = 1;
    public static final int VOLTE_CALL_TYPE_OFFLINE_REG_RECOVERY = 2;
    public static final int VOLTE_CALL_TYPE_OFFLINE_REG_REGRESSION = 3;
    public static final int VOLTE_CALL_TYPE_NORMAL_WIFI = 4;
    public static final int VOLTE_CALL_TYPE_EMERGENCY_WIFI = 5;
    public static final int VOLTE_CALL_TYPE_USSI = 6;

    // Call Type
    public static final int CALLTYPE_VOIP        = 1;
    public static final int CALLTYPE_VT          = 2;
    public static final int CALLTYPE_RTT         = 3;
    public static final int CALLTYPE_VIDEO_RTT   = 4;

    // Service Type
    public static final int SERVICETYPE_NONE            = 0;
    public static final int SERVICETYPE_NORMAL          = (0x00000001);
    public static final int SERVICETYPE_EMERGENCY       = (0x00000002);

    // Property AGOODUC - WILL REMOVED
    public static final int PROPERTY_IS_CONF                = 0;
    public static final int PROPERTY_IS_VMS                    = 1;
    public static final int PROPERTY_IS_EMERGENCY            = 2;
    public static final int PROPERTY_IS_MMC                    = 3;
    public static final int PROPERTY_ENABLE_CONF                = 50;

    // Info Type
    public static final int INFO_TYPE_NONE                    = 0;
    public static final int INFO_TYPE_MYNUMBER                = 1;    // String
    public static final int INFO_TYPE_BALANCE                = 2;    // String
    public static final int INFO_TYPE_MINUTES                = 3;    // String
    public static final int INFO_TYPE_TEXTUSAGE                = 4;    // String
    public static final int INFO_TYPE_PREPAID                = 5;    // String
    public static final int INFO_TYPE_SPCODE_REQ_SUCCESS    = 7;    // String
    public static final int INFO_TYPE_SPCODE_REQ_FAIL        = 8;    // String
    public static final int INFO_TYPE_SPCODE_CHECK_SUCCESS    = 9;    // String
    public static final int INFO_TYPE_SPCODE_CHECK_FAIL        = 10;    // String
    // Google_IMS_IF :: USSD {
    public static final int INFO_TYPE_USSD                   = 11; // int & String
    // Google_IMS_IF :: USSD }
    public static final int INFO_TYPE_MEDIA_VIDEO_LOWEST_BIT_RATE = 12; // no value
    public static final int INFO_TYPE_MEDIA_VIDEO_NO_DATA = 13; // no value
    public static final int INFO_TYPE_MEDIA_DTMF_RECEIVED = 14; // String & int

    // Types for REPLACED_BY
    public static final int REPLACED_BY_TYPE_ECT                = 0;

    public static final int VOLTE_CALL_STATE_IDLE = 0;
    public static final int VOLTE_CALL_STATE_TERMINATING = 1;
    public static final int VOLTE_CALL_STATE_RINGBACK = 2;
    public static final int VOLTE_CALL_STATE_RINGING = 3;
    public static final int VOLTE_CALL_STATE_ALERTING = 4;
    public static final int VOLTE_CALL_STATE_OFFHOOK = 5;

    public class SpecialCode
    {
        public static final int SPCODE_NONE                            = 0;
        public static final int SPCODE_USSD                         = 1;
        public static final int SPCODE_PCFALL_REGI                     = 2;
        public static final int SPCODE_PCFALL_ACTIVE                = 3;
        public static final int SPCODE_PCFALL_DEACTIVE                = 4;
        public static final int SPCODE_PCFALL_EARASE                = 5;
        public static final int SPCODE_PCFALL_CHECK                    = 6;
        public static final int SPCODE_PCFONLY_REGI                    = 7;
        public static final int SPCODE_PCFONLY_ACTIVE                = 8;
        public static final int SPCODE_PCFONLY_DEACTIVE                = 9;
        public static final int SPCODE_PCFONLY_EARASE                = 10;
        public static final int SPCODE_PCFONLY_CHECK                = 11;
        public static final int SPCODE_CF_DELAY_REGI                = 12;
        public static final int SPCODE_CF_DELAY_ACTIVE                = 13;
        public static final int SPCODE_CF_DELAY_DEACTIVE             = 14;
        public static final int SPCODE_CF_DELAY_EARASE                 = 15;
        public static final int SPCODE_CF_DELAY_CHECK                 = 16;
        public static final int SPCODE_CLIP_ACTIVE                    = 17;
        public static final int SPCODE_CLIP_DEACTIVE                = 18;
        public static final int SPCODE_CLIP_CHECK                    = 19;
        public static final int SPCODE_CLIR_ACTIVE                    = 20;
        public static final int SPCODE_CLIR_DEACTIVE                 = 21;
        public static final int SPCODE_CLIR_CHECK                    = 22;
        public static final int SPCODE_CW_ACTIVE                    = 23;
        public static final int SPCODE_CW_DEACTIVE                    = 24;
        public static final int SPCODE_CW_CHECK                        = 25;
        public static final int SPCODE_CFU_REGI                     = 26;
        public static final int SPCODE_CFU_ACTIVE                     = 27;
        public static final int SPCODE_CFU_DEACTIVE                    = 28;
        public static final int SPCODE_CFU_EARASE                    = 29;
        public static final int SPCODE_CFU_CHECK                    = 30;
        public static final int SPCODE_CFNRY_REGI                     = 31;
        public static final int SPCODE_CFNRY_ACTIVE                    = 32;
        public static final int SPCODE_CFNRY_DEACTIVE                = 33;
        public static final int SPCODE_CFNRY_EARASE                    = 34;
        public static final int SPCODE_CFNRY_CHECK                    = 35;
        public static final int SPCODE_CFNRC_REGI                     = 36;
        public static final int SPCODE_CFNRC_ACTIVE                    = 37;
        public static final int SPCODE_CFNRC_DEACTIVE                = 38;
        public static final int SPCODE_CFNRC_EARASE                    = 39;
        public static final int SPCODE_CFNRC_CHECK                    = 40;
        public static final int SPCODE_CFB_REGI                        = 41;
        public static final int SPCODE_CFB_ACTIVE                     = 42;
        public static final int SPCODE_CFB_DEACTIVE                    = 43;
        public static final int SPCODE_CFB_EARASE                    = 44;
        public static final int SPCODE_CFB_CHECK                    = 45;
        public static final int SPCODE_IMEI_CHECK                    = 46;
    };

    public class Fail_Reason
    {
        // Reason
        public static final int FAIL_REASON_SESSION_NONE                = 0;
        public static final int FAIL_REASON_SESSION_UNKNOWN                = 1;

        public static final int FAIL_REASON_SERVICE_UNAVAILABLE         = 101;
        public static final int FAIL_REASON_SERVICE_NOTREGISTERED       = 102;
        public static final int FAIL_REASON_SERVICE_OUT                 = 103;
        public static final int FAIL_REASON_SERVICE_NOTSUPPORTCALL      = 104;
        public static final int FAIL_REASON_SERVICE_POWEROFF            = 105;
        public static final int FAIL_REASON_SERVICE_LOWBATTERY          = 106;
        public static final int FAIL_REASON_SERVICE_INCSCALL            = 107;
        public static final int FAIL_REASON_SERVICE_INVTCALL            = 108;
        public static final int FAIL_REASON_SERVICE_INOTHERSCALL        = 109;
        public static final int FAIL_REASON_SERVICE_MAXCALL             = 110;

        public static final int FAIL_REASON_NETWORK_OUTOFCOVERAGE       = 201;
        public static final int FAIL_REASON_NETWORK_NO_LTE_COVERAGE     = 202;
        public static final int FAIL_REASON_NETWORK_NO_WIFI_COVERAGE    = 203;
        public static final int FAIL_REASON_NETWORK_NO_VOLTE            = 204;
        public static final int FAIL_REASON_NETWORK_ROAMING             = 205;
        public static final int FAIL_REASON_NETWORK_IPCHANGE            = 206;
        public static final int FAIL_REASON_NETWORK_BARRING             = 207;
        public static final int FAIL_REASON_NETWORK_SSAC                = 208;

        public static final int FAIL_REASON_SESSION_SERVERERROR         = 301;
        public static final int FAIL_REASON_SESSION_FORBIDDEN           = 302;
        public static final int FAIL_REASON_SESSION_NOTFOUND            = 303;
        public static final int FAIL_REASON_SESSION_NOTSUPPORTED        = 304;
        public static final int FAIL_REASON_SESSION_TEMPUNAVAILABLE     = 305;
        public static final int FAIL_REASON_SESSION_BADADDRESS          = 306;
        public static final int FAIL_REASON_SESSION_BUSY                = 307;
        public static final int FAIL_REASON_SESSION_IN_SETUP            = 308;
        public static final int FAIL_REASON_SESSION_REJECTED            = 309;
        public static final int FAIL_REASON_SESSION_CANCELED            = 310;

        public static final int FAIL_REASON_SESSION_USERTERMINATE       = 311;
        public static final int FAIL_REASON_SESSION_TERMINATED          = 312;
        public static final int FAIL_REASON_SESSION_UPDATE_FAILED       = 313;
        public static final int FAIL_REASON_SESSION_BADRESPONSE         = 314;
        public static final int FAIL_REASON_SESSION_NOTACCEPTABLE       = 315;
        public static final int FAIL_REASON_SESSION_NOTACCEPTABLEHERE   = 316;
        public static final int FAIL_REASON_SESSION_TIMEOUT             = 317;
        public static final int FAIL_REASON_SESSION_REFRESH_OUT         = 318;
        public static final int FAIL_REASON_SESSION_NOTREACHABLE        = 319;
        public static final int FAIL_REASON_SESSION_REDIRECTION         = 320;
        public static final int FAIL_REASON_SESSION_EARLYDIALOG         = 321;
        public static final int FAIL_REASON_SESSION_IN_UPDATE           = 322;
        public static final int FAIL_REASON_SESSION_SRVCC               = 323;
        public static final int FAIL_REASON_SESSION_PRECONDITION        = 324;
        public static final int FAIL_REASON_SESSION_SETUPFAILED         = 325;
        public static final int FAIL_REASON_SESSION_IN_REFER = 326;
        public static final int FAIL_REASON_SESSION_DESTROYED = 327;
        public static final int FAIL_REASON_SESSION_RES_TIMEOUT = 328;

        public static final int FAIL_REASON_SESSION_SERVER_AUTH = 401;
        public static final int FAIL_REASON_SESSION_SERVER_REQUEST_TERMINATED = 402;
        public static final int FAIL_REASON_SESSION_SERVER_INTERNAL_ERROR = 403;
        public static final int FAIL_REASON_SESSION_SERVER_SERVICE_UNAVAILABLE = 404;
        public static final int FAIL_REASON_SESSION_SERVER_DECLINE = 405;

        public static final int FAIL_REASON_SESSION_RETRY = 500;
        public static final int FAIL_REASON_SESSION_RETRY1X = 501;
        public static final int FAIL_REASON_SESSION_RETRYVOLTE = 502;
        public static final int FAIL_REASON_SESSION_RETRY_RAT = 503;
        public static final int FAIL_REASON_SESSION_RETRY_SILENT = 504;
        public static final int FAIL_REASON_SESSION_RETRY_E_1X = 505;
        public static final int FAIL_REASON_SESSION_RETRY_E_VOLTE = 506;
        public static final int FAIL_REASON_SESSION_RETRY_E_RAT = 507;
        public static final int FAIL_REASON_SESSION_RETRY_R_RAT = 508;
        public static final int FAIL_REASON_SESSION_RETRY1X_FORCE = 509;

        public static final int FAIL_REASON_SESSION_MULTIDEVICE_ACCEPTED = 551;
        public static final int FAIL_REASON_SESSION_MULTIDEVICE_REJECTED = 552;
        public static final int FAIL_REASON_SESSION_MULTIDEVICE_LIMITED = 553;
        public static final int FAIL_REASON_SESSION_MULTIDEVICE_PULLED = 554;

        public static final int FAIL_REASON_FORBIDDEN_BARRING = 561;
        public static final int FAIL_REASON_FORBIDDEN_NOPROFILE = 562;
        public static final int FAIL_REASON_FORBIDDEN_EXPIRATION = 563;
        public static final int FAIL_REASON_FORBIDDEN_NOSUBSCRIBER = 564;

        public static final int FAIL_REASON_ECT_COMPLETED = 571;
        public static final int FAIL_REASON_ECT_REPLACED = 572;

        public static final int FAIL_REASON_MEDIA_INITFAIL = 601;
        public static final int FAIL_REASON_MEDIA_CODEC = 602;
        public static final int FAIL_REASON_MEDIA_LOWEST_BIT_RATE = 603;
        public static final int FAIL_REASON_MEDIA_CHECK_RADIO_CONNECTION = 604;
        public static final int FAIL_REASON_MEDIA_NEGOFAIL = 605;
        public static final int FAIL_REASON_MEDIA_NOMATCH = 606;
        public static final int FAIL_REASON_MEDIA_NODATA = 607;
        public static final int FAIL_REASON_MEDIA_UNKNOWN = 608;

        public static final int FAIL_REASON_CONF_JOINED = 701;
        public static final int FAIL_REASON_CONF_NOT_ACCEPTABLE = 702;
        public static final int FAIL_REASON_CONF_BUSY = 703;
        public static final int FAIL_REASON_CONF_NOT_ACCEPTABLE_HERE = 704;
        public static final int FAIL_REASON_CONF_TIMEOUT = 705;
        public static final int FAIL_REASON_CONF_INTERNAL_ERROR = 706;
        public static final int FAIL_REASON_CONF_ALONE = 707;
        public static final int FAIL_REASON_CONF_UNKNOWN = 708;

        public static final int FAIL_REASON_TO_MO_PROGRESSING = 801;
        public static final int FAIL_REASON_TO_MO_STARTED = 802;
        public static final int FAIL_REASON_TO_MO_UPDATE = 803;
        public static final int FAIL_REASON_TO_MT_NOANSWER = 804;
        public static final int FAIL_REASON_TO_MT_UPDATE = 805;
        public static final int FAIL_REASON_TO_MT_PRACK = 806;

        public static final int FAIL_REASON_MAX = 999;
        // 1000 ~ : Define By Com

        public static final int CODE_SETUPFAILED_NONE           = 0;
        public static final int CODE_SETUPFAILED_OPENFAIL       = 1;
        public static final int CODE_SETUPFAILED_OPENFAIL_E     = 2;
        public static final int CODE_SETUPFAILED_STARTFAIL      = 3;
        public static final int CODE_SETUPFAILED_STARTFAIL_E    = 4;
        public static final int CODE_SETUPFAILED_ACCEPTFAIL     = 5;
        public static final int CODE_SETUPFAILED_BLOCK_1XRETRY  = 6;

        // FAIL_REASON_SESSION_RETRY1X :: CODE
        public static final int CODE_1XRETRY_NONE               = 0;
        public static final int CODE_1XRETRY_NORMAL             = 1;
        public static final int CODE_1XRETRY_SILENT_REDIAL      = 2;
        public static final int CODE_1XRETRY_PRIORITY_SET       = 3;

        // FAIL_REASON_SESSION_RETRYVoLTE :: CODE
        public static final int CODE_VOLTERETRY_NORMAL          = 0;
        public static final int CODE_VOLTERETRY_EMERGENCY       = 1;
        public static final int CODE_VOLTERETRY_OFFLINE         = 2;

        // FAIL_REASON_SESSION_RETRY1X_E_1X|VOLTE :: CODE
        public static final int CODE_EMERGENCYSERVICE_INVALIDE = -1;
        public static final int CODE_EMERGENCYSERVICE_GENERIC = 0;
        public static final int CODE_EMERGENCYSERVICE_AMBULANCE = 1;
        public static final int CODE_EMERGENCYSERVICE_ANIMAL_CONTROL = 2;
        public static final int CODE_EMERGENCYSERVICE_FIRE = 3;
        public static final int CODE_EMERGENCYSERVICE_GAS = 4;
        public static final int CODE_EMERGENCYSERVICE_MARINE = 5;
        public static final int CODE_EMERGENCYSERVICE_MOUNTAIN = 6;
        public static final int CODE_EMERGENCYSERVICE_PHYSICIAN = 7;
        public static final int CODE_EMERGENCYSERVICE_POISON = 8;
        public static final int CODE_EMERGENCYSERVICE_POLICE = 9;

    };

    public class Reject_Reason {

        public static final int REJECT_REASON_UNKNOWN                    = 0;

        public static final int REJECT_REASON_DECLINE_USER          = 2100;
        public static final int REJECT_REASON_DECLINE_NOANSWER      = 2101;
        public static final int REJECT_REASON_DECLINE_NOBATTERY     = 2102;
        public static final int REJECT_REASON_DECLINE_NORMAL        = 2103;
        public static final int REJECT_REASON_DECLINE_CW            = 2104;
        public static final int REJECT_REASON_DECLINE_UPDATE        = 2105;

        public static final int REJECT_REASON_SERVICE_UNAVAILABLE   = 2110;
        public static final int REJECT_REASON_SERVICE_TTY           = 2111;

        public static final int REJECT_REASON_BUSY_ISCSCALL         = 2120;

        public static final int REJECT_REASON_BUSY_ISEMERGENCY      = 2123;
        public static final int REJECT_REASON_BUSY_ISWIFICALL       = 2124;
        public static final int REJECT_REASON_BUSY_ISOTHERSCALL     = 2125;
        public static final int REJECT_REASON_BUSY_ESTABLISHING     = 2126;
        public static final int REJECT_REASON_BUSY_ALERTING         = 2127;
        public static final int REJECT_REASON_BUSY_MAXCALL          = 2128;
        public static final int REJECT_REASON_BUSY_NORMAL           = 2129;
        public static final int REJECT_REASON_BUSY_IGNORE           = 2130;

        public static final int REJECT_REASON_SESSION_NOTSUPPORT                = 2140;
        public static final int REJECT_REASON_SESSION_NOTACCEPTABLE             = 2141;
        public static final int REJECT_REASON_SESSION_NOTACCEPTABLE_HERE        = 2142;
        public static final int REJECT_REASON_SESSION_UPDATE                    = 2143;
        public static final int REJECT_REASON_SESSION_BAD                       = 2144;
        public static final int REJECT_REASON_SESSION_FAIL                      = 2145;
        public static final int REJECT_REASON_SESSION_FAIL_PRECONDITION         = 2146;
        public static final int REJECT_REASON_SESSION_INVALID_REFERRER_IDENTITY = 2147;

        public static final int REJECT_REASON_MEDIA_INITFAIL                = 2160;
        public static final int REJECT_REASON_MEDIA_CODEC                   = 2161;
        public static final int REJECT_REASON_MEDIA_LOWEST_BIT_RATE         = 2162;
        public static final int REJECT_REASON_MEDIA_CHECK_RADIO_CONNECTION  = 2163;
        public static final int REJECT_REASON_MEDIA_NEGOFAIL                = 2164;
        public static final int REJECT_REASON_MEDIA_FORMFAIL                = 2165;
        public static final int REJECT_REASON_MEDIA_NODATA                  = 2166;
        public static final int REJECT_REASON_MEDIA_FAIL                    = 2167;

        public static final int REJECT_REASON_CONF_JOINED           = 2170;

        public static final int REJECT_REASON_TO_MO_PROGRESSING     = 2181;
        public static final int REJECT_REASON_TO_MO_STARTED         = 2182;
        public static final int REJECT_REASON_TO_MO_UPDATE          = 2183;
        public static final int REJECT_REASON_TO_MT_NOANSWER        = 2184;
        public static final int REJECT_REASON_TO_MT_UPDATE          = 2185;
        public static final int REJECT_REASON_TO_MT_PRACK           = 2186;
        public static final int REJECT_REASON_TO_MO_WAIT_TERMINATED     = 2187;
        public static final int REJECT_REASON_TO_MT_WAIT_TERMINATED     = 2188;
        public static final int REJECT_REASON_TO_CONV_WAIT_TERMINATED   = 2189;

    }; /* REJECT_REASON_ */

    public class Terminate_Reason {

        public static final int TERMINATE_REASON_UNKNOWN            = 0;

        public static final int TERMINATE_REASON_NORMAL             = 10;
        public static final int TERMINATE_REASON_USER                = 11;
        public static final int TERMINATE_REASON_LOW_BATTERY        = 12;
        public static final int TERMINATE_REASON_POWER_OFF            = 13;
        public static final int TERMINATE_REASON_VCC                = 14;

    }; /* Terminate_Reason */

    // Action
    public static final String ACTION_REMOTE_MEDIA =
            "com.android.imsstack.action.REMOTE_MEDIA";

};
