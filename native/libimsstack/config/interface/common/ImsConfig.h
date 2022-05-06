/*
    Author
    <table>
    Date      Author                    Description
    --------  -----------------         ------------------
    20120819  hwangoo.park@             Created
    </table>

    Description

*/

#ifndef _IMS_CONFIG_H_
#define _IMS_CONFIG_H_

class ImsConfig
{
public:
    // Flags to identify the configuration repository
    // It will be used in lParam or LOWORD(wParam) of IMS_EVENT_CONFIG_UPDATE
    enum
    {
        FLAG_IMS_NONE = 0x0000,

        FLAG_IMS_SUBSCRIBER = 0x0001,
        FLAG_IMS_ENGINE = 0x0002,
        FLAG_IMS_SIP = 0x0004,
        FLAG_IMS_AOS_CONNECTION = 0x0010,
        FLAG_IMS_AOS_REG = 0x0020,
        FLAG_IMS_COM_SIP = 0x0100,
        FLAG_IMS_COM_MEDIA = 0x0200,

        // Service specific
        FLAG_IMS_COM_SERVICE_SMS = 0x1000,

        F_IMS_ALL = 0xFFFF,

        // It will be used in LOWORD(wParam) of IMS_EVENT_CONFIG_UPDATE
        // SDM
        // SMS
        SDM_I_HOME_DOMAIN_NAME = 1,
        SDM_I_TV_T1 = 2,
        SDM_I_TV_T2 = 3,
        SDM_I_TV_TF = 4,
        SDM_I_SMS_FORMAT = 5,
        SDM_I_SMS_OVER_IP_NETWORK_INDICATION = 6,
        // VoLTE
        SDM_I_AMR_MODE_SET = 11,
        SDM_I_AMR_WB_MODE_SET = 12,
        SDM_I_PUBLISH_TIMER = 13,
        SDM_I_PUBLISH_TIMER_EXTENDED = 14,
        SDM_I_CAPABILITIES_CACHE_EXPIRATION = 15,
        SDM_I_AVAILABILITY_CACHE_EXPIRATION = 16,
        SDM_I_CAPABILITIES_POLL_INTERVAL = 17,
        SDM_I_SOURCE_THROTTLE_PUBLISH = 18,
        SDM_I_MAX_NUMBER_OF_ENTRIES_IN_REQUEST_CONTAINED_LIST = 19,
        SDM_I_CAPABILITIES_POLL_LIST_SUBSCRIPTION_EXPIRATION = 20,
        SDM_I_GZIP_FLAG = 21,
        SDM_I_SIP_SESSION_TIMER = 22,
        SDM_I_MIN_SE = 23,
        SDM_I_TIMER_VZW = 24,
        SDM_I_TDELAY = 25,
        SDM_I_SILENT_REDIAL_ENABLE = 26,
        SDM_I_TLTE_911FAIL = 27,
        SDM_I_VLT = 28,
        SDM_I_EAB = 29,
        SDM_I_LVC = 30,
        SDM_I_MULTIDEVICE = 31,
        SDM_I_VOWIFI = 32,

        SDM_I_SUBSCRIBER = 40,
        SDM_I_PCSCF = 41,

        // PST
        // SMS
        PST_I_PCSCF_ADDRESS = 1,
        PST_I_PCSCF_PORT = 2,
        PST_I_TV_T1 = 3,
        PST_I_TV_T2 = 4,
        PST_I_TV_TF = 5,
        PST_I_SMS_FORMAT = 6,
        PST_I_SMS_OVER_IP_NETWORK_INDICATION = 7,

        // VOLTE
        PST_I_URI_MEDIA_RESOURCE_SERVER = 11,
        PST_I_SESSION_TIMER = 12,
        PST_I_MIN_SE = 13,
        PST_I_AMR_WB = 14,
        PST_I_SCR_AMR = 15,
        PST_I_SCR_AMR_WB = 16,
        PST_I_AMR_MODE_SET = 17,
        PST_I_AMR_WB_MODE_SET = 18,
        PST_I_CAPABILITY_DISCOVERY_VOLTE = 19,
        PST_I_RINGING_TIMER = 20,
        PST_I_RINGBACK_TIMER = 21,
        PST_I_RTP_RTCP_INACTIVITY_TIMER = 22,
        PST_I_UDP_KEEP_ALIVE = 23,
        PST_I_MULTIDEVICE = 24,
        PST_I_VOWIFI = 25,
        PST_I_TIMER_VZW = 26,
        PST_I_TDELAY = 27,
        PST_I_SILENT_REDIAL_ENABLE = 28,
        PST_I_TLTE_911FAIL = 29,

        // RCS
        PST_I_EAB = 31,
        PST_I_PUBLISH_TIMER = 32,
        PST_I_PUBLISH_TIMER_EXTENDED = 33,
        PST_I_CAPABILITIES_CACHE_EXPIRATION = 34,
        PST_I_AVAILABILITY_CACHE_EXPIRATION = 35,
        PST_I_SOURCE_THROTTLE_PUBLISH = 36,
        PST_I_CAPABILITY_POLL_INTERVAL = 37,
        PST_I_MAX_NUMBER_OF_ENTRIES_IN_REQUEST_CONTAINED_LIST = 38,
        PST_I_CAPABILITY_POLL_LIST_SUBSCRIPTION_EXPIRATION = 39,
        PST_I_CAPABILITY_DISCOVERY = 40,
        PST_I_GZIP_ENABLE = 41,
        PST_I_PUBLISH_ERROR_RETRY_TIMER = 42,

        // Subscriber
        PST_I_HOME_DOMAIN_NAME = 51,
        PST_I_IMPU = 52,
        PST_I_IMPI = 53,
        PST_I_PHONE_CONTEXT = 54,
        PST_I_PASSWORD = 55,
        PST_I_DOMAIN_FOR_IMPU = 56
    };
};

#endif  // _IMS_CONFIG_H_
