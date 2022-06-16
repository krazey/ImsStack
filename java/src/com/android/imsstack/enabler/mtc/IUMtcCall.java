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
    public static final int REQUEST_CALL_PUSH   = (EVENT_U2I + 62);
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

    // Action
    public static final String ACTION_REMOTE_MEDIA =
            "com.android.imsstack.action.REMOTE_MEDIA";

};
