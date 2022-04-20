package com.android.imsstack.system;

public class ImsEventDef
{
    // External Intent
    public static final String ACTION_IMS_STARTED =
            "com.android.imsstack.action.IMS_STARTED";
    public static final String ACTION_CONFIG_UPDATE =
            "com.android.imsstack.action.CONFIG_UPDATE";
    public static final String ACTION_DOMAIN_NOTIFICATION =
            "com.android.imsstack.action.DOMAIN_NOTIFICATION";
    public static final String ACTION_IMS_PDN_PRECONDITION_CHANGED =
            "com.android.imsstack.action.PDN_PRECONDITION_CHANGED";
    public static final String ACTION_RADIO_OFF = "radio_off";
    public static final String ACTION_WFC_REGISTRATION_STATE =
            "com.android.imsstack.action.REGISTRATION_STATE";
    public static final String ACTION_WFC_REGISTRATION_NOREADY =
            "com.android.imsstack.action.REGISTRATION_NOT_READY";
    public static final String ACTION_VOWIFI_REGISTRATION_STATE =
            "com.android.imsstack.action.VOWIFI_STATUS_IND";
    public static final String ACTION_IMS_DEREGISTERED =
            "com.android.imsstack.action.IMS_DEREGISTERED";


    //1 RVI to IMS native ----- starts{
    public static final int IMS_EVENT_POWER_LOW_BATTERY = 0x00000008;
        // wParam
        public static final int IMS_POWER_LOW_BATTERY = 0;
        public static final int IMS_POWER_LOW_CHANGED = 1;

    public static final int IMS_EVENT_PHONE_NUMBER_AVAILABLE = 0x00000010;
        // wParam
        public static final int IMS_PHONE_NUMBER_INITIAL = 0;
        public static final int IMS_PHONE_NUMBER_REFRESH = 1;

        // lParam
        public static final int IMS_PHONE_NUMBER_SIM_LOADED = 0;
        public static final int IMS_PHONE_NUMBER_RETRY_SUCCESS = 1;
        public static final int IMS_PHONE_NUMBER_RETRY_FAILURE = 2;

    public static final int IMS_EVENT_CSCALL_STATE = 0x00000020;
        // wParam
        public static final int IMS_CSCALL_STATE_IDLE = 0;
        public static final int IMS_CSCALL_STATE_INCOMING = 1;
        public static final int IMS_CSCALL_STATE_ACTIVE = 2;

    public static final int IMS_EVENT_CONFIG_UPDATE = 0x00000040;
        // wParam
        public static final int IMS_CONFIG_CAT_0 = 0;
        public static final int IMS_CONFIG_CAT_1 = 1;
        public static final int IMS_CONFIG_CAT_100 = 100;
        public static final int IMS_CONFIG_CAT_10002 = 10002;

    public static final int IMS_EVENT_DATA_MODE = 0x00000080;
        // wParam
        public static final int IMS_DATA_MODE_ALLOWED = 0;
        public static final int IMS_DATA_MODE_BLOCKED = 1;

    public static final int IMS_EVENT_RADIO_OFF = 0x00000100;

    public static final int IMS_EVENT_ROAMING_STATE = 0x00000200;
        // wParam (PS roaming), lParam (CS roaming)
        public static final int IMS_ROAMING_STATE_OFF = 0;
        public static final int IMS_ROAMING_STATE_ON = 1;

    public static final int IMS_EVENT_ROAMING_TYPE = 0x00000300;
        // wParam
        public static final int IMS_ROAMING_TYPE_VOICE = 0;
        public static final int IMS_ROAMING_TYPE_DATA  = 1;

        // lParam
        public static final int ROAMING_TYPE_NOT_ROAMING   = 0;
        public static final int ROAMING_TYPE_UNKNOWN       = 1;
        public static final int ROAMING_TYPE_DOMESTIC      = 2;
        public static final int ROAMING_TYPE_INTERNATIONAL = 3;

    public static final int IMS_EVENT_ECM_STATE = 0x00000400;
        // wParam
        public static final int IMS_ECM_STATE_OFF = 0;
        public static final int IMS_ECM_STATE_ON = 1;
        public static final int IMS_ECM_STATE_OFF_BY_NEW_ECALL = 2;

    public static final int IMS_EVENT_VOICE_SERVICE_STATE = 0x00000500;
        // wParam
        public static final int IMS_VOICE_SERVICE_IN_SERVICE = 0;
        public static final int IMS_VOICE_SERVICE_OUT_OF_SERVICE = 1;
        public static final int IMS_VOICE_SERVICE_EMERGENCY_ONLY = 2;
        public static final int IMS_VOICE_SERVICE_POWER_OFF = 3;

    public static final int IMS_EVENT_LTE_INFO = 0x00000600;
         // wParam
        public static final int IMS_LTE_INFO_UNKNOWN = 0;
        public static final int IMS_LTE_INFO_DETACHED = 1;
        public static final int IMS_LTE_INFO_EPS_ONLY_ATTACHED = 2;
        public static final int IMS_LTE_INFO_NORMAL_ATTACHED = 3;
        public static final int IMS_LTE_INFO_EMERGENCY_ATTACHED = 4;
        public static final int IMS_LTE_INFO_REATTACH_REQUIRED = 5;
         // lParam
        public static final int IMS_LTE_INFO_UPDATE_RESULT_NO_ADD_INFO = 0;
        public static final int IMS_LTE_INFO_UPDATE_RESULT_CSFB_NOT_PREFERRED = 1;
        public static final int IMS_LTE_INFO_UPDATE_RESULT_SMS_ONLY = 2;
        public static final int IMS_LTE_INFO_UPDATE_RESULT_RESERVED = 3;

    public static final int IMS_EVENT_NR_INFO = 0x00000700;
        // wParam
        public static final int IMS_NR_INFO_UNKNOWN = 0;
        public static final int IMS_NR_INFO_REGISTRATION = 1;
        public static final int IMS_NR_INFO_DEREGISTRATION = 2;
        public static final int IMS_NR_INFO_EMERGENCY_REGISTRATION = 3;

    public static final int IMS_EVENT_IMS_VOICE_OVER_PS_STATE = 0x00000800;
        // wParam
        public static final int IMS_VOICE_OVER_PS_NOT_SUPPORTED = 0;
        public static final int IMS_VOICE_OVER_PS_SUPPORTED = 1;

    public static final int IMS_EVENT_LTE_STATE = 0x00001000;
        // wParam
        public static final int IMS_LTE_RACH_REJECT_WITH_WAITTIME = 11;
            // LParam (waittime : ms)

        public static final int IMS_LTE_RACH_IGNORE_DURING_T300_3TIMES = 12;
        public static final int IMS_LTE_SR_REJECT_WITH_EMM9_EMM10 = 13;
        public static final int IMS_LTE_SR_REJECT_WITH_EMM17_3TIMES = 14;
        public static final int IMS_LTE_SR_IGNORE_DURING_5SEC = 15;
        public static final int IMS_LTE_BARRING_MO_DATA = 16;
        public static final int IMS_LTE_SR_REJECT_WITH_EMM = 17;

        public static final int IMS_LTE_BARRING_SSAC = 18;
        public static final int IMS_LTE_BARRING_SSAC_EX = 19;
            // IMS_LTE_BARRING_SSAC lParam
            public static final int IMS_LTE_BARRING_SSAC_VOICE = 0x00000000;
            public static final int IMS_LTE_BARRING_SSAC_VIDEO = 0x01000000;

        public static final int IMS_LTE_QOS_DEDICATED_BEARER_COMPLETED = 31;
            // LParam (QCI)

        // uicc refresh '0' type
        public static final int IMS_LTE_TRIGGER_DEREGISTRATION = 32;
        // no update ims state(0) for hVoLTE
        public static final int IMS_LTE_DEACTIVATE_IMS_PDN = 33;
        // update current ims reg state for hVoLTE when substate is 0 (CSFB)
        public static final int IMS_LTE_CSFB_PREF_SUB_STATE = 34;
        // update current ims reg state for hVoLTE
        public static final int IMS_LTE_UPDATE_CURRENT_REG_STATE = 35;
        // update current ims reg state for SA
        public static final int IMS_LTE_SA_UPDATE_CURRENT_REG_STATE = 36;

        public static final int IMS_LTE_BLOCK_WITH_TIME = 51;
            // LParam (time : ms)

        //AC Barring - SKT
        public static final int IMS_LTE_ACCESS_BARRED_FOR_MO_DATA = 52;

        //Ims Network Info
        public static final int IMS_LTE_VOPS = 101;
        public static final int IMS_WCDMA_VOPS = 102; // (WCDMA, GSM)
        public static final int IMS_LTE_EMC_BS = 103;
        public static final int IMS_LTE_EMERGENCY_SUPPORT = 104;
        public static final int IMS_LTE_AC_BARRING_FOR_EMERGENCY = 105;

    public static final int IMS_EVENT_ROVE_STATE_CHANGED = 0x00002000;
        // wParam
        public static final int IMS_ROVE_IN = 1;
        public static final int IMS_ROVE_OUT = 0;

    public static final int IMS_EVENT_LTE_ROVE_STATE_CHANGED = 0x00002200;
        // wParam
        public static final int IMS_LTE_ROVE_IN = 1;
        public static final int IMS_LTE_ROVE_OUT = 0;

    public static final int IMS_EVENT_REG_STATE_INTERNAL = 0x00004000;
        // wParam
        public static final int IMS_REG_INTERNAL_ON = 0;
        public static final int IMS_REG_INTERNAL_OFF = 1;

    public static final int IMS_EVENT_SERVICE_SETTING = 0x00008000;
        // wParam
        public static final int IMS_SERVICE_OFF = 0;
        public static final int IMS_SERVICE_ON = 1;
        // Service availability (on/off) will be determined by the lParam
        public static final int IMS_SERVICE_PRESENTITY = 2;
        // lParam - IUIMS.java (bitmask)

    public static final int IMS_EVENT_SYNC_TO_NATIVE = 0x00010002;
        // wParam
        public static final int IMS_SYNC_TO_NATIVE_INCOMPLETED = 0;
        public static final int IMS_SYNC_TO_NATIVE_COMPLETED = 1;

    public static final int IMS_EVENT_REG_PREF_STATE = 0x00020000;
        // wParam
        public static final int IMS_REG_PREF_SYS_NONE = 0;
        public static final int IMS_REG_PREF_SYS_CDMA = 1; // RADIO_IF_CDMA_1XEVDO
        public static final int IMS_REG_PREF_SYS_LTE = 2; // RADIO_IF_LTE

        // lParam
        public static final int IMS_REG_PREF_REG_NONE = 0;
        public static final int IMS_REG_PREF_REG_VOIP = 1; // VOIP+SMS
        public static final int IMS_REG_PREF_REG_SMS = 2; // SMS

    public static final int IMS_EVENT_WFC_SETTING_CHANGED = 0x00040000;
        // wParam
        public static final int IMS_WFC_ON = 1;
        public static final int IMS_WFC_OFF = 0;
        // lParam
        public static final int MODE_WFC_PREFERRED = 2;
        public static final int MODE_WFC_ONLY = 0;
        public static final int MODE_CELLULAR_PREFERRED = 1;
        public static final int MODE_IMS_PREFERRED = 10;

    public static final int IMS_EVENT_WFC_AP_SETTING_CHANGED = 0x00040002;
        // wParam
        public static final int IMS_WFC_AP_ON = 1;
        public static final int IMS_WFC_AP_OFF = 0;

    public static final int IMS_EVENT_CALLINGPLUS_SETTING_CHANGED = 0x00040004;
        // wParam
        public static final int IMS_CALLINGPLUS_ON = 1;
        public static final int IMS_CALLINGPLUS_OFF = 0;

    public static final int IMS_EVENT_OMADM_UPDATED = 0x00080000;
        // wParam
        public static final int IMS_OMADM_VLT = 0;
        public static final int IMS_OMADM_LVC = 1;
        public static final int IMS_OMADM_EAB = 2;
        public static final int IMS_OMADM_ALLOW_VT_REGARDLESS_OF_EAB = 4;
        public static final int IMS_OMADM_MULTIDEVICE = 5;
        public static final int IMS_OMADM_VOWIFI = 6;
        public static final int IMS_OMADM_SERVICE_OVER_LTE = 7;
        public static final int IMS_OMADM_ACTIVE_CALL_HO = 8;

        // lParam
        public static final int IMS_OMADM_DISABLED = 0;
        public static final int IMS_OMADM_ENABLED = 1;

    public static final int IMS_EVENT_MODEM_STATE = 0x00100000;
        // wParam
        public static final int IMS_MODEM_STATE_READY = 1;
        public static final int IMS_MODEM_STATE_IN_SERVICE = 2;
        public static final int IMS_MODEM_STATE_OUT_OF_SERVICE = 3;
        public static final int IMS_MODEM_STATE_MODEM_RESET = 4;

    public static final int IMS_EVENT_VOIP_SETTING = 0x00200000;
        // wParam
        public static final int IMS_VOIP_SETTING_OFF = 0;
        public static final int IMS_VOIP_SETTING_ON = 1;

    public static final int IMS_EVENT_VOLTE_SETTING = 0x00400000;
        // wParam
        public static final int IMS_VOLTE_SETTING_OFF = 0x00000000;
        public static final int IMS_VOLTE_SETTING_ON = 0x00000001;

    public static final int IMS_EVENT_NETWORK_MODE_SETTING = 0x00400001;
        // wParam
        public static final int IMS_NETWORK_MODE_OFF = 0x00000000;
        public static final int IMS_NETWORK_MODE_ON = 0x00000001;

    public static final int IMS_EVENT_PS_BARRING_STATE = 0x00800000;

    public static final int IMS_EVENT_AC_BARRING_STATE = 0x00800001;

    public static final int IMS_EVENT_VOIP_NETWORK_CAPAVILITY = 0x01000000;
        // wParam
        public static final int IMS_VOIP_CAPAVILITY_OFF = 0;
        public static final int IMS_VOIP_CAPAVILITY_ON = 1;

    public static final int IMS_EVENT_UEINITIATED_IMSPDN_DISCONNECTION = 0x02000000;

    public static final int IMS_EVENT_EPDG_KEEPING_SERVICE = 0x04000000;
        // wParam
        public static final int EPDG_KEEPING_ON = 1;
        public static final int EPDG_KEEPING_OFF = 2;

    public static final int IMS_EVENT_WPS_CALL_STATE = 0x08000000;
        // wParam
        public static final int IMS_WPS_CALL_STATE_END = 0;
        public static final int IMS_WPS_CALL_STATE_START = 1;

    public static final int IMS_EVENT_CALL_BLOCK = 0x09000000;
        // wParam
        public static final int IMS_CALL_BLOCK_STOP = 0;
        public static final int IMS_CALL_BLOCK_START = 1;

    public static final int IMS_EVENT_MEM_DEBUG = 0x10000000;
        // wParam
        public static final int IMS_MEM_DEBUG_ENABLE_FILE_WRITE = 0;
            // lParam
            public static final int IMS_MEM_DEBUG_ENABLE_FILE_WRITE_OFF = 0;
            public static final int IMS_MEM_DEBUG_ENABLE_FILE_WRITE_ON = 1;

        // wParam
        public static final int IMS_MEM_DEBUG_PRINT_HEAP_LEAKAGE = 1;

    public static final int IMS_EVENT_MOBILE_DATA_SETTING = 0x20000000;
        // wParam
        public static final int IMS_MOBILE_DATA_SETTING_OFF = 0;
        public static final int IMS_MOBILE_DATA_SETTING_ON = 1;

    public static final int IMS_EVENT_MOBILE_DATA_LIMIT_CHANGED = 0x20000001;
        // WParam
        public static final int IMS_MOBILE_DATA_LIMITED = 0;
        public static final int IMS_MOBILE_DATA_NOT_LIMITED = 1;

    public static final int IMS_EVENT_ROAMING_PREFERRED_VOICE_CALL_NETWORK = 0x20000004;
        // wParam
        public static final int IMS_ROAMING_PREFERRED_CELLULAR_NETWORK = 0;
        public static final int IMS_ROAMING_PREFERRED_WIFI_NETWORK = 1;
        // lParam ON : 1 / OFF : 0

    public static final int IMS_EVENT_VIDEO_SETTING = 0x20000008;
        // wParam
        public static final int IMS_VIDEO_SETTING_OFF = 0;
        public static final int IMS_VIDEO_SETTING_ON = 1;

    // TODO : delete it
    public static final int IMS_EVENT_DATA_ROAMING_SETTING = 0x20000010;
        // wParam
        public static final int IMS_DATA_ROAMING_DENYED = 0;
        public static final int IMS_DATA_ROAMING_ALLOWED = 1;

    public static final int IMS_EVENT_NETWORK_MODE_SUPPORTS_CDMA = 0x20000020;
        // wParam
        public static final int IMS_CDMA_NOT_SUPPORTED = 0;
        public static final int IMS_CDMA_SUPPORTED = 1;

    public static final int IMS_EVENT_REG_CONTROL = 0x30000000;
        // wParam
        public static final int IMS_REG_CONTROL_RECOVER = 1;
            // IMS_REG_CONTROL_RECOVER lParam
            public static final int IMS_REG_CONTROL_KEEP_DATA_CONNECTION = 11;

        public static final int IMS_REG_CONTROL_UPDATE = 2;

        public static final int IMS_REG_CONTROL_DESTROY = 3;
            // IMS_REG_CONTROL_DESTROY lParam
            public static final int IMS_REG_CONTROL_DESTROY_DCN = 31;

        public static final int IMS_REG_CONTROL_IPCAN = 4;
            // IMS_REG_CONTROL_IPCAN lParam
            public static final int IMS_REG_CONTROL_IPCAN_STOP = 41;
            public static final int IMS_REG_CONTROL_IPCAN_WIFITOLTE = 42;

        public static final int IMS_REG_CONTROL_STOP = 5;

        public static final int IMS_REG_CONTROL_WIFICALL_STOP = 6;

        public static final int IMS_REG_CONTROL_PCSCF = 7;
            // IMS_REG_CONTROL_PCSCF lParam
            public static final int IMS_REG_CONTROL_PCSCF_SAME_CHANGED = 71;

    public static final int IMS_EVENT_SRVCC_NOTIFICATION = 0x40000000;
        // wParam
        public static final int IMS_SRVCC_EVT_START = 0x01;
        public static final int IMS_SRVCC_EVT_FAILURE = 0x02;
        public static final int IMS_SRVCC_EVT_SUCCESS = 0x03;
        public static final int IMS_SRVCC_EVT_CANCEL = 0x04;
        public static final int IMS_SRVCC_CALLEVENT_NOTUSED = 0x05;

    public static final int IMS_EVENT_IPCAN_HO_NOTIFICATION = 0x40000001;
        // wParam
        public static final int IMS_IPCAN_HANDOVER_UNKNOWN = 0x00;
        public static final int IMS_IPCAN_HANDOVER_STARTED = 0x01;
        public static final int IMS_IPCAN_HANDOVER_SUCCESS = 0x02;
        public static final int IMS_IPCAN_HANDOVER_FAILURE = 0x03;

    public static final int IMS_EVENT_IMS_PDN_PREFERENCE = 0x40000002;
        // wParam
        public static final int IMS_PDN_PREFERENCE_CELLULAR = 0x01;
        public static final int IMS_PDN_PREFERENCE_WLAN = 0x02;
        public static final int IMS_PDN_PREFERENCE_WLAN_WAITING = 0x03;
        public static final int IMS_PDN_PREFERENCE_WLAN_WAITING_DONE = 0x04;

    public static final int IMS_EVENT_NETWORK_CAPABILITY = 0x40000004;
        // wParam
        public static final int IMS_SERVICE_CAPA_VIDEO = 0;

        // lParam
        public static final int IMS_CAPA_OFF = 0;
        public static final int IMS_CAPA_ON = 1;

    public static final int IMS_EVENT_MEDIA_MOCA_ENABLE_CHECK = 0x40000011;
        // wParam
        public static final int IMS_MEDIA_MOCA_OFF  = 0;
        public static final int IMS_MEDIA_MOCA_ON = 1;

    public static final int IMS_EVENT_RTT_SETTING = 0x40000020;
        // wParam
        public static final int IMS_RTT_MODE_NONE = -1;
        public static final int IMS_RTT_CAPABLE_OFF = 0;
        public static final int IMS_RTT_VISIBLE_DURING_CALLS = 1;
        public static final int IMS_RTT_ALWAYS_VISIBLE = 2;

    public static final int IMS_EVENT_DDS_CHANGED = 0x50000000;
        // qParam : slot id

    // CELLULAR Signal Strength
    public static final int IMS_EVENT_CELL_SIGNAL_STRENGTH_CHANGE = 0x60020000;
        // WParam
        public static final int  IMS_CELL_SIGNAL_BAD = 0;
        public static final int  IMS_CELL_SIGNAL_GOOD = 1;

    public static final int IMS_EVENT_AVAIL_RAT_INFO_CHANGED = 0x70100000;
        //wParam : Available RAT INFO
        public static final int IMS_AVAIL_RAT_INFO_NONE = 0x0;
        public static final int IMS_AVAIL_RAT_INFO_2G = 0x1;
        public static final int IMS_AVAIL_RAT_INFO_3G = 0x2;
        public static final int IMS_AVAIL_RAT_INFO_LTE = 0x4;
        public static final int IMS_AVAIL_RAT_INFO_WIFI = 0x8;

    public static final int IMS_EVENT_PHONE_RESTARTED = 0x71000000;

    //1  MUST NOT use the above(>=) 0x80000000 from java to native

    //1 RVI to IMS native ----- ends}


    //1 IMS native to RVI ----- starts{

    public static final int IMS_EVENT_REG_STATE = 0x00000002;
        // wParam HWORD (network type)
        public static final int IMS_NET_LTE = 0x00000001;
        public static final int IMS_NET_WIFI = 0x00000002;
        // wParam LWORD (service type :: refer IUIMS.java)

        // lParam
        public static final int IMS_REG_ON = 0x00000001;
        public static final int IMS_REG_OFF = 0x00000000;
        public static final int IMS_REG_OFF_DONE = 0x00000002;

    public static final int IMS_EVENT_VOLTE_INDICATOR = 0x00000006;
        // wParam
        public static final int IMS_VOLTE_INDICATOR_OFF = 0x00000000;
        public static final int IMS_VOLTE_INDICATOR_ON = 0x00000001;

    public static final int IMS_EVENT_VOWIFI_INDICATOR = 0x00000007;
        // wParam
        public static final int IMS_VOWIFI_INDICATOR_OFF = 0x00000000;
        public static final int IMS_VOWIFI_INDICATOR_ON = 0x00000001;

    public static final int IMS_EVENT_REG_SERVICE = 0x00000010;
        // wParam
        public static final int IMS_REG_SERVICE_NONE = 0x00000000;
        public static final int IMS_REG_SERVICE_VOIP = 0x00000001;
        public static final int IMS_REG_SERVICE_SMS = 0x00000002;

        // lParam
        public static final int IMS_REG_SERVICE_TYPE_NORMAL = 0x00000000;
        public static final int IMS_REG_SERVICE_TYPE_UPDATE = 0x00000001;
        public static final int IMS_REG_SERVICE_TYPE_SYNCUP = 0x00000002;

    public static final int IMS_EVENT_REG_FAILURE = 0x00000011;
        // wParam (response code)
        // lParam (consecutive failure count)

    public static final int IMS_EVENT_NATIVE_BOOT_COMPLETED = 0x00000012;

    public static final int IMS_EVENT_WAKE_LOCK = 0x00000013;

    public static final int IMS_EVENT_DATA_CONNECTION = 0x00000015;
        // wParam
        public static final int IMS_DEACTIVATE_REQ = 0x00000001;

        // lParam
        public static final int IMS_DETACH_TIMER = 0x00000001;
        public static final int IMS_DETACH_PERMANENT = 0x00000002;
        public static final int IMS_DETACH_AND_ATTACH = 0x00000003;
        public static final int IMS_REFRESH_LEAVE_LTE_TIMER = 0x00000004;

    public static final int IMS_EVENT_OBTAIN_PHONE_NUMBER = 0x00000017;
        // wParam
        public static final int IMS_OBTAIN_PHONE_NUMBER_INITIAL = 0;
        public static final int IMS_OBTAIN_PHONE_NUMBER_REFRESH = 1;
        public static final int IMS_OBTAIN_PHONE_NUMBER_CLEAR = 2;

    public static final int IMS_EVENT_DCN = 0x00000019;
        // wParam
        public static final int IMS_DCN_NOT_SENDING = 0;
        public static final int IMS_DCN_SENDING = 1;
        public static final int IMS_DCN_FORCE_SENDING = 2;

    public static final int IMS_EVENT_DM_SYNC = 0x00000020;
        // wParam
        public static final int IMS_DM_SYNC_ONLY = 1;
        public static final int IMS_DM_SYNC_VLT_DISABLED = 2;

    public static final int IMS_EVENT_MLT = 0x00000021;
        // wParam
        public static final int IMS_MLT_REGISTRATION_SUCCESS = 10;
        public static final int IMS_MLT_REREGISTRATION_SUCCESS = 11;

        public static final int IMS_MLT_LDB_LOSS_PACKET_COUNT = 12;
        public static final int IMS_MLT_LDB_TOTAL_PACKET_COUNT = 13;
        public static final int IMS_MLT_MOCA_PACKET_JITTER_REPORT = 14;

        public static final int IMS_MLT_CALL_FAILURE = 20;

    public static final int IMS_EVENT_REG_DESTROYED = 0x00000022;
        //WParam
        public static final int IMS_REG_DESTROYED_SILENT_REDIAL = 1;

    public static final int IMS_EVENT_NOTIFY_STATE = 0x00000023;
        //WParam
        public static final int IMS_REG_NOTIFY_STATE_ACTIVE = 1;
        public static final int IMS_REG_NOTIFY_STATE_INVALID = 2;

    public static final int IMS_EVENT_DM_CONFIGURATION_RETRIEVAL = 0x00000071;
        public static final int FLAG_STARTAOS_NOTFORCED = 0;
        public static final int FLAG_STARTAOS_FORCED = 1;

    public static final int IMS_EVENT_SHOW_MESSAGE = 0x00000080;
        // lParam
        public static final int IMS_MESSAGE_REGISTRATION_FAILED = 1;
        public static final int IMS_MESSAGE_SERVICE_NOT_PROVISIONED = 2;

    public static final int IMS_EVENT_REGISTRATION = 0x00000100;
        /* WPARAM (HWORD) state
            0 : not registered
            1 : registered
        */
        // WPARAM (LWORD) service state (bit masking)
        public static final int IMS_REGISTRATION_SERVICE_NONE  = 0x0;
        public static final int IMS_REGISTRATION_SERVICE_VOICE = 0x1;
        public static final int IMS_REGISTRATION_SERVICE_VIDEO = 0x2;
        public static final int IMS_REGISTRATION_SERVICE_SMS = 0x4;

        /* LPARAM (HWORD) reason
        0 : Unspecified
        1 : Power off (N/A MTK)
        2 : RF off (N/A MTK)
        3 : PLMN blocked
        4 : T3402 blocked
        5 : IMS initialization
        6 : Refresh leave LTE timer (KDDI only)
        */
        public static final int IMS_REGISTRATION_REASON_UNSPECIFIED  = 0x0;
        public static final int IMS_REGISTRATION_REASON_PLMN_BLOCKED = 0x3;
        public static final int IMS_REGISTRATION_REASON_T3402_BLOCKED = 0x4;
        public static final int IMS_REGISTRATION_REASON_IMS_INITIALIZATION = 0x5;
        public static final int IMS_REGISTRATION_REASON_REFRESH_LEAVE_LTE_TIMER = 0x6;

        // WPARAM (LWORD) detail state
        public static final int IMS_REGISTRATION_INVALID = -1;
        public static final int IMS_REGISTRATION_OFFLINE = 0;
        public static final int IMS_REGISTRATION_REGISTERING = 1;
        public static final int IMS_REGISTRATION_REGISTERED = 2;
        public static final int IMS_REGISTRATION_REREGISTERING = 3;
        public static final int IMS_REGISTRATION_DEREGISTERING = 4;
        public static final int IMS_REGISTRATION_STOP = 5;

    public static final int IMS_EVENT_TRACE_MOCA = 0x00000500;
        // wParam
        public static final int IMS_TRACE_MOCA_STOP = 0x00000001;

    public static final int IMS_EVENT_ISIM_STATE = 0x00000700;
        // wParam
        public static final int IMS_ISIM_STATE_NONE = -1;
        public static final int IMS_ISIM_STATE_INVALID = 0;
        public static final int IMS_ISIM_STATE_VALID = 1;
        public static final int IMS_ISIM_STATE_REFRESH_STARTED = 2;
        public static final int IMS_ISIM_STATE_REFRESH_COMPLETE = 3;

    public static final int IMS_EVENT_PDN_PRECONDITION_CHANGED = 0x00000701;
        // wParam
        public static final int PRECONDITION_ISIMINVALID = 1;
        public static final int PRECONDITION_ISIMVALID = 2;
        public static final int PRECONDITION_ISIMREFRESHSTARTED = 3;
        public static final int PRECONDITION_ISIMREFRESHED = 4;
        public static final int PRECONDITION_SIMLOADED = 5;
        public static final int PRECONDITION_403FORBIDDEN = 11;
        public static final int IMS_PDN_BLOCK_REASON_NONE = 100;
        public static final int IMS_PDN_BLOCK_REASON_REGI_FAIL = 111;
        public static final int IMS_PDN_BLOCK_REASON_OTHERS = 121;
        public static final int SERVICE_CHECKED_AVAILABLE = 122;
        public static final int SERVICE_CHECKED_NOTAVAILABLE = 123;
        public static final int IMS_PDN_RELEASE_DEREG_SUCCESS = 124;
        public static final int IMS_PDN_RECONNECT = 125;

    public static final int IMS_EVENT_DEBUG = 0x00000800;

    public static final int IMS_EVENT_DEBUG_AWT_UPDATED = 0x00000801;

    public static final int IMS_EVENT_NO_RTP_AND_PING_FAIL = 0x00040010;

    public static final int IMS_EVENT_REGI_REPORT_TO_WFC = 0x04000000;
        //wParam
        public static final int REPORT_REG_STATUS_NOT_READY = 1;
        public static final int REPORT_REG_STATUS_IDLE = 2;
        public static final int REPORT_REG_STATUS_REGISTERING = 3;
        public static final int REPORT_REG_STATUS_REGFAILED = 4;
        public static final int REPORT_REG_STATUS_REGISTERED = 5;
        public static final int REPORT_REG_STATUS_REFRESHING = 6;
        public static final int REPORT_REG_STATUS_REFRESHFAILED = 7;
        public static final int REPORT_REG_STATUS_DEREGISTERING = 8;

        // lParam upper
        public static final int IPCAN_CAT_NONE = 0x00010000;
        public static final int IPCAN_CAT_MOBILE = 0x00020000;
        public static final int IPCAN_CAT_EPDG = 0x00040000;

        // lParam lower
        public static final int REASON_PCSCF_FAILED = 0x00000001;
        public static final int REASON_LOCAL_IP_FAILED = 0x00000002;
        public static final int REASON_NO_USER_INFO = 0x00000004;
        public static final int REASON_NO_SERVICE_RAT = 0x00000008;
        public static final int REASON_IMS_DISABLED = 0x00000010;
        public static final int REASON_TTY_ENABLED = 0x00000020;
        public static final int REASON_GBA_NOT_SUPPORTED = 0x00000040;
        public static final int REASON_AIRPLANE_MODE_ENABLED = 0x00000080;
        public static final int REASON_IPSEC_FAILED = 0x00000100;
        public static final int REASON_TXN_TIMEOUT = 0x00000200;
        public static final int REASON_NOTIFICATION_TERMINATED = 0x00000400;
        public static final int REASON_SUBSCRIPTION_FORBIDDEN = 0x00000800;
        public static final int REASON_ALL_OTHER_FAILURES = 0x00001000;
        public static final int REASON_SUBSCRIPTION_FORBIDDEN_TERMINATE = 0x00002000;

    public static final int IMS_EVENT_EPDG_PREFERENCE = 0x05000000;
        // wParam
        public static final int EPDG_PREFERRED = 0;
        public static final int EPDG_ONLY = 1;
        public static final int CELLULAR_PREFERRED = 2;
        public static final int CELLULAR_ONLY = 3;

    public static final int IMS_EVENT_SERVICE_STATUS = 0x05000001;
        // wParam
        public static final int IMS_SERVICE_NONE = 0;
        public static final int IMS_SERVICE_UC = 1;
        public static final int IMS_SERVICE_SMS = 2;
        public static final int IMS_SERVICE_UCE = 3;
        public static final int IMS_SERVICE_MAX = IMS_SERVICE_UCE + 1;

        // lParam
        public static final int IMS_SERVICE_STOP = 0;
        public static final int IMS_SERVICE_START = 1;

    public static final int IMS_EVENT_REPORT_BAD_NETWORK = 0x05000002;
        // wParam upper - access network
        public static final int CAT_CELLULAR = 0x00010000;
        public static final int CAT_IWLAN = 0x00020000;

        // wParam lower - capability
        public static final int PDN_TYPE_IMS = 0x00000001;
        public static final int PDN_TYPE_EMERGENCY = 0x00000002;
        public static final int PDN_TYPE_INTERNET = 0x00000004;
        public static final int PDN_TYPE_XCAP = 0x00000008;

        //lParam - score
        public static final int QUALITY_BAD = 20;
        public static final int QUALITY_GOOD = 100;

    public static final int IMS_EVENT_WIFI_SERVICE = 0x10000000;
        // wParam
        public static final int IMS_WIFI_OFF = 0;
        public static final int IMS_WIFI_ON = 1;

    public static final int IMS_EVENT_CALL_INFO_TO_WFC = 0x30000000;
        // wParam
        public static final int IMS_EVENT_CALL_STATE_INACTIVE = 0;
        public static final int IMS_EVENT_CALL_STATE_ACTIVE = 1;

        // lParam
        public static final int IMS_EVENT_CALL_TYPE_WIFI = 0;
        public static final int IMS_EVENT_CALL_TYPE_LTE = 1;

        // wParam
        //    - HIWORD : Category (SIP - 0, Media - 1, ...)
        //    - LOWORD : Free format according to the category
        // lParam
        //    - Free format according to the category

    public static final int IMS_EVENT_SEND_DATA_TO_MODEM = 0x80000001;
        // wParam
        public static final int IMS_EVENT_DATA_FLUSH_ENABLED = 1;
        public static final int IMS_EVENT_ACB_SKIP_NOTI = 2;

        // lParm
        public static final int IMS_EVENT_REGISTRATION_START = 0;
        public static final int IMS_EVENT_REGISTRATION_END = 1;
        public static final int IMS_EVENT_REG_EVENT_START = 2;
        public static final int IMS_EVENT_REG_EVENT_END = 3;

    public static final int IMS_EVENT_SEND_SCM_TO_MODEM = 0x80000002;
        // for legacy interface
        // wParam
        public static final int IMS_EVENT_CALL_START = 0;
        public static final int IMS_EVENT_CALL_END = 1;
        // lParm
        public static final int IMS_EVENT_CALL_TYPE_VOICE = 0;
        public static final int IMS_EVENT_CALL_TYPE_VIDEO = 1;

        // for IMS RIL integration
        // wParam
        public static final int IMS_SCM_START = 0;
        public static final int IMS_SCM_END = 1;
        // lParm
        public static final int IMS_SCM_TYPE_VOICE = 1;
        public static final int IMS_SCM_TYPE_VIDEO = 2;
        public static final int IMS_SCM_TYPE_SMS = 3;
        public static final int IMS_SCM_TYPE_REGISTRATION = 4;
        public static final int IMS_SCM_TYPE_REG_EVENT = 11;

    public static final int IMS_EVENT_CALL_INFO = 0x90000000;
        // wPARAM
        //#define IMS_CALL_TOTAL_STATE_XXX (0xff000000)
        //#define IMS_CALL_STATE_XXX (0x00ff0000)
        //#define IMS_CALL_STATE_REASON (0x0000ffff)

    public static final int IMS_EVENT_CALL_MEIDA_INFO = 0x90000001;
        // wPARAM
        //#define IMS_CALL_MEDIA_TYPE (0x0000000f)
        //#define IMS_CALL_MEDIA_CODEC (0x000000f0)
        //#define IMS_CALL_MEDIA_BANDWIDTH (0x00000f00)
        //#define IMS_CALL_MEDIA_BITRATE (0x00ff0000)

    //1 IMS native to RVI ----- ends}
}
