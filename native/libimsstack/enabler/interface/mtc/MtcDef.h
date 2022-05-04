#ifndef MTC_DEF_H_
#define MTC_DEF_H_

#include "AString.h"
#include "IMSTypeDef.h"
#include "IMtcService.h"
#include "ServiceTrace.h"

// TODO, MTC BUILD
#ifndef UNUSED_PARAM
#define UNUSED_PARAM(A) (A == 0)
#endif

enum
{
    SIDETYPE_LOCAL = 0,
    SIDETYPE_REMOTE = 1,
};

enum
{
    OATYPE_OFFER = 0,
    OATYPE_ANSWER = 1,
};

enum
{
    MSGTYPE_REQUEST = 0,
    MSGTYPE_RESPONSE = 1,
};

enum class UpdateType
{
    NONE = 0,
    NORMAL = 1,
    HOLD = 2,
    RESUME = 3,
    SESSION = 4,
    CONF = 5,
    SRVCC_RECOVERED_CANCEL = 6,
    SRVCC_RECOVERED_FAILURE = 7,
};

enum REFER_TYPE
{
    REFER_TYPE_NONE = 0,
    REFER_TYPE_CONF_JOIN = 1,
    REFER_TYPE_CONF_DROP = 2,
};

enum
{
    COPYCONTROLTYPE_TO = 0,
    COPYCONTROLTYPE_CC = 1,
    COPYCONTROLTYPE_BCC = 2,

}; /*COPYCONTROLTYPE */

enum
{
    DTMFTYPE_INBAND_2833 = 0,
    DTMFTYPE_INBAND_BYPASS = 1,
    DTMFTYPE_OUTBAND_INFO = 2,

}; /* DTMFTYPE */

enum
{
    FEATURE_NONE = 0,
    FEATURE_TIMER = (0x00000001),
    FEATURE_100REL = (0x00000002),
    FEATURE_PRECONDITION = (0x00000004),
    FEATURE_PEM = (0x00000008),
    FEATURE_CONFSUB = (0x00000010),
    FEATURE_UPDATE_PROVISION = (0x00000020),
    FEATURE_TERMINATE_EARLYDIALOG = (0x00000040),
    FEATURE_199 = (0x00000080),

}; /* FEATURE_ */

enum class PemType
{
    NONE = 0,
    SENDRECV = 1,
    SENDONLY = 2,
    RECVONLY = 3,
    INACTIVE = 4,
};

enum
{
    MEDIATYPE_NONE = 0,
    MEDIATYPE_AUDIO = (0x00000001),
    MEDIATYPE_VIDEO = (0x00000002),
    MEDIATYPE_TEXT = (0x00000004),
};

enum
{
    MEDIAPROTOCOL_INVALID = -1,
    MEDIAPROTOCOL_NONE = 0,
    MEDIAPROTOCOL_RTP = (0x00000001),
    MEDIAPROTOCOL_RTCP = (0x00000002),
};

enum
{
    MEDIAINFO_DIRECTION = 0,
    MEDIAINFO_QUALITY = 1,
};

enum
{
    NEGOSTATE_INVALID = -1,
    NEGOSTATE_IDLE = 0,
    NEGOSTATE_OFFER_RECEIVED = 1,
    NEGOSTATE_OFFER_SENT = 2,
    NEGOSTATE_NEGOTIATED = 3,
};

enum
{
    FORM_RESULT_NONE = 0,
    FORM_RESULT_NOTEXPECTED = 1,
    FORM_RESULT_UPDATED = 2,
    FORM_RESULT_FAILED = 3,
};

enum NEGO_RESULT
{
    NEGO_RESULT_NONE = 0,
    NEGO_RESULT_NOSDP = 1,
    NEGO_RESULT_NOTEXPECTED = 2,
    NEGO_RESULT_UPDATED = 3,
    NEGO_RESULT_FAILED = 4,
};

enum
{
    DIRECTION_INVALID = -1,
    DIRECTION_INACTIVE = 0,
    DIRECTION_RECEIVE = 1,
    DIRECTION_SEND = 2,
    DIRECTION_SEND_RECEIVE = 3,
};

enum
{
    VIDEO_QUALITY_NONE = 0,

    VIDEO_QUALITY_QCIF = 1,
    VIDEO_QUALITY_QVGA_LS = 2,
    VIDEO_QUALITY_QVGA_PR = 3,
    VIDEO_QUALITY_VGA_LS = 4,
    VIDEO_QUALITY_VGA_PR = 5,
    VIDEO_QUALITY_CIF_LS = 6,
    VIDEO_QUALITY_CIF_PR = 7,
    VIDEO_QUALITY_QCIF_PR = 8,
    VIDEO_QUALITY_SQCIF_LS = 9,
    VIDEO_QUALITY_SQCIF_PR = 10,
    VIDEO_QUALITY_SIF_LS = 11,
    VIDEO_QUALITY_SIF_PR = 12,
    VIDEO_QUALITY_HD_LS = 13,
    VIDEO_QUALITY_HD_PR = 14,

    VIDEO_QUALITY_NOTUSED = 99,
};

enum
{
    AUDIO_QUALITY_NONE = 0,

    AUDIO_QUALITY_AMR_NB = 1,
    AUDIO_QUALITY_AMR_WB = 2,
    AUDIO_QUALITY_EVS = 3,
    AUDIO_QUALITY_G711_PCMU = 4,
    AUDIO_QUALITY_G711_PCMA = 5,
    AUDIO_QUALITY_EVS_NB = 6,
    AUDIO_QUALITY_EVS_WB = 7,
    AUDIO_QUALITY_EVS_SWB = 8,
    AUDIO_QUALITY_EVS_FB = 9,
    AUDIO_QUALITY_MAX = 10,

    AUDIO_QUALITY_NOTUSED = 99,
};

enum
{
    TEXT_QUALITY_NONE = 0,
    TEXT_QUALITY_T140 = 1,
    TEXT_QUALITY_T140_RED = 2,
    TEXT_QUALITY_NOTUSED = 99,
};

enum
{
    GTT_MODE_INVALID = -1,
    GTT_MODE_INACTIVE = 0,
    GTT_MODE_FULL = 1,
    GTT_MODE_HCO = 2,
    GTT_MODE_VCO = 3,
};

enum MEDIA_REPORT_TYPE
{
    MEDIA_REPORT_INVALID = -1,
    MEDIA_REPORT_NOTUSED = 0,

    MEDIA_REPORT_SUCCESS = 1,
    MEDIA_REPORT_DATA_RECEIVE_FAILED,
    MEDIA_REPORT_INTERNAL_ERROR,

    MEDIA_REPORT_VIDEO_LOWEST_BIT_RATE,
    MEDIA_REPORT_CHECK_RADIO_CONNECTION,
    MEDIA_REPORT_DATA_RECEIVE_STARTED,
    MEDIA_REPORT_RE_NEGOTIATION_NEEDED,

    MEDIA_REPORT_NW_TONE_RTP_RECEIVE_STARTED,
    MEDIA_REPORT_NW_TONE_RTP_RECEIVE_FAILED,

    MEDIA_REPORT_RECEIVED_DTMF_EVENT,

    MEDIA_REPORT_UC_TERMINATED = 100,
};

enum class SuppType
{
    NONE = -1,
    CALLER_ID = 0,
    CNAP = 1,
    CNAP_EX = 2,
    MMC = 3,
    GTT = 4,
    CDIV_CAUSE = 5,
    CDIV_HISTORY = 6,
    CW = 7,
    USSD = 8,
    VM = 9,
    HD = 10,
    ANSWER_HOLD = 11,
    MCID = 12,
    DUAL_NUMBER = 13,
    ENFORCE_LT = 14,
    TARGET_URI = 15,
    CALLING_NUM_VERIFICATION = 16,
    VRBT = 17,
    TIP = 18,
    GEOLOCATION = 101,
};

enum class OipType
{
    INVALID = -1,
    NONE = 0,
    IDENTITY = 1,
    RESTRICTED = 2
};

enum class CdivCause
{
    NONE = 0,
    UNCONDITION = 1,
    BUSY = 2,
    REJECT = 3,
    NOANSWER = 4,
    NO_REPLY = 5,
    DEFLECTION = 6,
    NOT_LOGGED_IN = 7,
    DEFLECTION_ALERTING = 8,
    NOT_REACHABLE = 9

};

enum
{
    INFO_TYPE_NONE = 0,
    INFO_TYPE_MYNUMBER = 1,   // String
    INFO_TYPE_BALANCE = 2,    // String
    INFO_TYPE_MINUTES = 3,    // String
    INFO_TYPE_TEXTUSAGE = 4,  // String
    INFO_TYPE_PREPAID = 5,    // String
    INFO_TYPE_SRVCC = 6,

    INFO_TYPE_MEDIA_LOWEST_BIT_RATE = 12,
    INFO_TYPE_MEDIA_NO_DATA = 13,
    INFO_TYPE_MEDIA_DTMF_RECEIVED = 14,

    INFO_TYPE_MEDIA_CVO_CAPABILITY = 112,
};

enum
{
    CW_TYPE_NONE = 0,
    CW_TYPE_TERMINAL = 1, /* CW NOTIFICATION */
    CW_TYPE_NETWORK = 2,  /* CW INDICATION */
};

enum
{
    CNAP_SCHEME_PAID = 0,
    CNAP_SCHEME_FROM = 1,
    CNAP_SCHEME_PAID_FROM = 2, /* CNAP INDICATION */
};

enum
{
    CALLERID_NONE = 0,
    CALLERID_NETWORK = 1,
    CALLERID_RESTRICTED = 2,
    CALLERID_IDENTITY = 3,
};

enum
{
    TIP_TYPE_NONE = 0,
    TIP_TYPE_IDENTITY = 1,
    TIP_TYPE_RESTRICTED = 2,
};

enum
{
    TIP_MODE_NONE = 0,
    TIP_MODE_TEMPORARY = 1,
    TIP_MODE_PERMANENT = 2,
};

enum
{
    CALLING_NUM_VERSTAT_NONE = 0,
    CALLING_NUM_VERSTAT_VERIFIED = 1,
    CALLING_NUM_VERSTAT_NOT_VERIFIED = 2,
};

enum
{
    SRVCC_STATUS_NONE = 0,
    SRVCC_STATUS_START,
    SRVCC_STATUS_SUCCESS,
    SRVCC_STATUS_FAILURE,
    SRVCC_STATUS_CANCEL,
};

enum
{
    SRVCC_TYPE_NOTSUPPORTED = 0,
    SRVCC_TYPE_NORMAL = (0x00000001),
    SRVCC_TYPE_MIDCALL = (0x00000002),
    SRVCC_TYPE_ALERTING = (0x00000004),
    SRVCC_TYPE_REVERSE = (0x00000008),
    SRVCC_TYPE_BEFOREALERTING = (0x00000010),
};

enum
{
    VOLTE_CALL_STATE_IDLE = 0,
    VOLTE_CALL_STATE_TERMINATING = 1,
    VOLTE_CALL_STATE_RINGBACK = 2,
    VOLTE_CALL_STATE_RINGING = 3,
    VOLTE_CALL_STATE_ALERTING = 4,
    VOLTE_CALL_STATE_OFFHOOK = 5,
    VOLTE_CALL_STATE_UNDEFINED = 10
};

enum
{
    BLOCK_FEATURE_NONE = 0,
    BLOCK_FEATURE_START = (0x00000001),
    BLOCK_FEATURE_STARTCONF = (0x00000002),
    BLOCK_FEATURE_ACCEPT = (0x00000004),
    BLOCK_FEATURE_REJECT = (0x00000008),
    BLOCK_FEATURE_HOLD = (0x00000010),
    BLOCK_FEATURE_RESUME = (0x00000020),
    BLOCK_FEATURE_SEND_DTMF = (0x00000040),
    BLOCK_FEATURE_TERMINATE = (0x00000080),
    BLOCK_FEATURE_UPDATE = (0x00000100),
    BLOCK_FEATURE_CONF_EXPAND = (0x00000200),
    BLOCK_FEATURE_CONF_MERGE = (0x00000400),
    BLOCK_FEATURE_CONF_JOIN = (0x00000800),
    BLOCK_FEATURE_CONF_DROP = (0x00001000),
}; /* BLOCK_FEATURE_ */

enum
{
    SESSTIMER_MO_1XX_WAIT = 0,
    SESSTIMER_MO_NOANSWER = 1,
    SESSTIMER_MT_ALERTING = 2,
    SESSTIMER_MT_PRACK_WAIT = 3,
    SESSTIMER_MO_UPDATE = 4,
    SESSTIMER_MT_UPDATE = 5,

    SESSTIMER_GLARECONDITION = 6,
    SESSTIMER_SRVCC_TERMINATED = 7,
    SESSTIMER_CONV_SRVCC = 8,
    SESSTIMER_QOS_WAIT = 9,
    SESSTIMER_QOS_FORCE_FAKE = 10,

    SESSTIMER_UDP_KEEP_ALIVE = 11,

    SESSTIMER_CONF_SUB_WAIT_NOTIFY = 12,
    SESSTIMER_QOS_INACTIVE_GUARD = 13,
    SESSTIMER_WAIT_NETWORK_RBT = 14,
    SESSTIMER_WAIT_NETWORK_HOLDTONE = 15,
    SESSTIMER_E911_LTE_OPEN = 16,
    SESSTIMER_E911_LTE_START = 17,
    SESSTIMER_E911_LTE_RINGBACK = 18,
    SESSTIMER_E911_WIFI_OPEN = 19,
    SESSTIMER_E911_WIFI_START = 20,
    SESSTIMER_E911_WIFI_RINGBACK = 21,
    SESSTIMER_SESS_WAIT_TERMINATED = 22,
    SESSTIMER_QOS_NOTI_PARTIAL = 23,
};

enum
{
    CONF_CREATE_NONE = 0,

    CONF_CREATE_START = 1,
    CONF_CREATE_STARTED = 2,
    CONF_CREATE_MERGE = 3,
    CONF_CREATE_EXPAND = 4,
    CONF_CREATE_EXPANDED_BY = 5,
};

enum CONFINFO_STATUS
{
    CONFINFO_STATUS_IDLE = 0,

    CONFINFO_STATUS_PROGRESSING = 1,  // ByConfMngr, Intrtnal information for UX
    CONFINFO_STATUS_CONNECTED = 2,
    CONFINFO_STATUS_DISCONNECTED = 3,
    CONFINFO_STATUS_ONHOLD = 4,
    CONFINFO_STATUS_MUTEDVIAFOCUS = 5,
    CONFINFO_STATUS_PENDING = 6,
    CONFINFO_STATUS_ALERTING = 7,
    CONFINFO_STATUS_DIALING_IN = 8,
    CONFINFO_STATUS_DIALING_OUT = 9,
    CONFINFO_STATUS_DISCONNECTING = 10,

    CONFINFO_STATUS_FAIL = 20,  // ByConfMngr, Intrtnal information for UX
    CONFINFO_STATUS_REJECT = 21,
    CONFINFO_STATUS_BUSY = 22,
    CONFINFO_STATUS_SERVERERROR = 23,
    CONFINFO_STATUS_NOTSUPPORTED = 24,
    CONFINFO_STATUS_NOTACCEPTABLE = 25,
    CONFINFO_STATUS_NOANSWER = 26,
    CONFINFO_STATUS_NOTREACHABLE = 27,
    CONFINFO_STATUS_LOWBATTERY = 28,
    CONFINFO_STATUS_FORBIDDEN = 29,
    CONFINFO_STATUS_INTSERVERERROR = 30,

    CONFINFO_STATUS_IDLING = 98,
    CONFINFO_STATUS_UNKNOWN = 99,
};

enum
{
    REPALCED_BY_TYPE_ECT = 0,
};

enum
{
    MEDIATHRESHOLD_TYPE_NONE = 0,
    MEDIATHRESHOLD_TYPE_ACTIVE = 1,
    MEDIATHRESHOLD_TYPE_HOLD = 2,
};

enum
{
    MULTIPLE_REEFER_SENDTYPE_SIMULTANEITY = 0,
    MULTIPLE_REEFER_SENDTYPE_DELIVERIED = 1,
    MULTIPLE_REEFER_SENDTYPE_TERMINATED = 2,
    MULTIPLE_REEFER_SENDTYPE_SUBSCRIBED = 3,
    MULTIPLE_REEFER_SENDTYPE_SUB_AFTER_REFER = 4
};

enum
{
    QOS_TYPE_AUDIO = 0,
    QOS_TYPE_VIDEO,
    QOS_TYPE_TEXT,
};  // QoS Type for dedicated bearer

enum
{
    QOS_STATUS_NONE = -1,
    QOS_STATUS_ENABLED = 0,
    QOS_STATUS_ACTIVED,
    QOS_STATUS_INACTIVE,
    QOS_STATUS_SUSPENDED,
    QOS_STATUS_DISABLED,
    QOS_STATUS_PARAM_INVALID,
};  // QoS status for dedicated bearer

enum
{
    SEGMENT_LOCALSEND = 0,
    SEGMENT_LOCALRECV,
    SEGMENT_REMOTESEND,
    SEGMENT_REMOTERECV,

    E2E_SEND,
    E2E_RECV,
};  // DIRECTION For Table

enum
{
    TIMER_BASE_DEFAULT = 0,
    TIMER_FORCE_QOS_ENABLED = 1,
    TIMER_WAIT_QOS_ENABLED = 2,
    TIMER_QOS_INACTIVE_GUARD = 3,
    TIMER_QOS_NOTI_PARTIAL = 4,
    TIMER_COM_DEFAULT = 10,
}; /*TIMERTYPE */

#define UC_SLOT_0 0
#define UC_SLOT_1 1

/* ------------------------------------------------------------------------------------------------
------------------------------------------------------------------------------------------------ */
inline const IMS_CHAR* Str_MediaType(IN IMS_UINT32 eType)
{
    switch (eType)
    {
        case MEDIATYPE_AUDIO:
            return "MEDIATYPE_AUDIO";
        case MEDIATYPE_VIDEO:
            return "MEDIATYPE_VIDEO";
        case MEDIATYPE_TEXT:
            return "MEDIATYPE_TEXT";
        default:
            return "__INVALID__";
    }
}

/* ------------------------------------------------------------------------------------------------
------------------------------------------------------------------------------------------------ */
inline const IMS_CHAR* Str_SuppType(IN SuppType eType)
{
    switch (eType)
    {
        case SuppType::CALLER_ID:
            return "SuppType::CALLER_ID";
        case SuppType::CNAP:
            return "SuppType::CNAP";
        case SuppType::MMC:
            return "SuppType::MMC";
        case SuppType::GTT:
            return "SuppType::GTT";
        case SuppType::CDIV_CAUSE:
            return "SuppType::CDIV_CAUSE";
        case SuppType::CDIV_HISTORY:
            return "SuppType::CDIV_HISTORY";
        case SuppType::CW:
            return "SuppType::CW";
        case SuppType::VM:
            return "SuppType::VM";
        default:
            return "__INVALID__";
    }
}

/* ------------------------------------------------------------------------------------------------
------------------------------------------------------------------------------------------------ */
inline const IMS_CHAR* Str_CW(IN IMS_UINT32 eType)
{
    switch (eType)
    {
        case CW_TYPE_NONE:
            return "CW_TYPE_NONE";
        case CW_TYPE_TERMINAL:
            return "CW_TYPE_TERMINAL";
        case CW_TYPE_NETWORK:
            return "CW_TYPE_NETWORK";
        default:
            return "__INVALID__";
    }
}

/* ------------------------------------------------------------------------------------------------
------------------------------------------------------------------------------------------------ */
inline const IMS_CHAR* Str_VoLTE_Call_State(IN IMS_UINT32 eState)
{
    switch (eState)
    {
        case VOLTE_CALL_STATE_IDLE:
            return "VOLTE_CALL_STATE_IDLE";
        case VOLTE_CALL_STATE_TERMINATING:
            return "VOLTE_CALL_STATE_TERMINATING";
        case VOLTE_CALL_STATE_RINGBACK:
            return "VOLTE_CALL_STATE_RINGBACK";
        case VOLTE_CALL_STATE_RINGING:
            return "VOLTE_CALL_STATE_RINGING";
        case VOLTE_CALL_STATE_ALERTING:
            return "VOLTE_CALL_STATE_ALERTING";
        case VOLTE_CALL_STATE_OFFHOOK:
            return "VOLTE_CALL_STATE_OFFHOOK";
        default:
            return "__INVALID__";
    }
}

/* ------------------------------------------------------------------------------------------------
------------------------------------------------------------------------------------------------ */
inline const IMS_CHAR* Str_CNAP(IN IMS_UINT32 eType)
{
    switch (eType)
    {
        case CNAP_SCHEME_PAID:
            return "CNAP_SCHEME_PAID ";
        case CNAP_SCHEME_FROM:
            return "CNAP_SCHEME_FROM";
        case CNAP_SCHEME_PAID_FROM:
            return "CNAP_SCHEME_PAID_FROM";
        default:
            return "__INVALID__";
    }
}

class MediaThreshold
{
public:
    inline MediaThreshold() :
            nProtocol(MEDIAPROTOCOL_INVALID),
            nType(MEDIATYPE_NONE),
            nTime(-1)
    {
    }
    inline MediaThreshold(IN IMS_SINT32 _nProtocol, IN IMS_SINT32 _nType, IN IMS_SINT32 _nTime) :
            nProtocol(_nProtocol),
            nType(_nType),
            nTime(_nTime)
    {
    }
    inline MediaThreshold(IN const MediaThreshold& objRHS) :
            nProtocol(objRHS.nProtocol),
            nType(objRHS.nType),
            nTime(objRHS.nTime)
    {
    }

public:
    IMS_SINT32 nProtocol;
    IMS_SINT32 nType;
    IMS_SINT32 nTime;
};

class UCKey
{
public:
    inline UCKey() :
            aStrUIKey(AString::ConstNull()),
            nIMSKey(0)
    {
    }
    inline UCKey(IN AString _aStrUIKey, IN IMS_UINTP _nIMSKey) :
            aStrUIKey(_aStrUIKey),
            nIMSKey(_nIMSKey)
    {
    }
    inline UCKey(IN const UCKey& objRHS) :
            aStrUIKey(objRHS.aStrUIKey),
            nIMSKey(objRHS.nIMSKey)
    {
    }

public:
    AString aStrUIKey;
    IMS_UINTP nIMSKey;
};

class MediaInfo
{
public:
    inline MediaInfo() :
            eADir(-1),
            eVDir(-1),
            eTDir(-1),
            eAQuality(0),
            eVQuality(0),
            eGTTMode(-1)
    {
        IMS_TRACE_MEM(
                "uc", "uc_M : MediaInfo[%" PFLS_u "][%" PFLS_x "]", sizeof(MediaInfo), this, 0);
    }
    inline MediaInfo(IN const MediaInfo& objRHS) :
            eADir(objRHS.eADir),
            eVDir(objRHS.eVDir),
            eTDir(objRHS.eTDir),
            eAQuality(objRHS.eAQuality),
            eVQuality(objRHS.eVQuality),
            eGTTMode(objRHS.eGTTMode)
    {
        IMS_TRACE_MEM(
                "uc", "uc_M : MediaInfo[%" PFLS_u "][%" PFLS_x "]", sizeof(MediaInfo), this, 0);
    }
    inline MediaInfo(IN IMS_SINT32 eInitADir, IN IMS_SINT32 eInitVDir, IN IMS_SINT32 eInitTDir,
            IN IMS_UINT32 eInitAQuality, IN IMS_UINT32 eInitVQuality, IN IMS_SINT32 eInitGTTMode) :
            eADir(eInitADir),
            eVDir(eInitVDir),
            eTDir(eInitTDir),
            eAQuality(eInitAQuality),
            eVQuality(eInitVQuality),
            eGTTMode(eInitGTTMode)
    {
        IMS_TRACE_MEM(
                "uc", "uc_M : MediaInfo[%" PFLS_u "][%" PFLS_x "]", sizeof(MediaInfo), this, 0);
    }
    inline ~MediaInfo()
    {
        IMS_TRACE_MEM(
                "uc", "uc_F : MediaInfo[%" PFLS_u "][%" PFLS_x "]", sizeof(MediaInfo), this, 0);
    }

public:
    inline MediaInfo& operator=(IN const MediaInfo& objRHS)
    {
        if (this != &objRHS)
        {
            eADir = objRHS.eADir;
            eVDir = objRHS.eVDir;
            eTDir = objRHS.eTDir;
            eAQuality = objRHS.eAQuality;
            eVQuality = objRHS.eVQuality;
            eGTTMode = objRHS.eGTTMode;
        }

        return (*this);
    }

public:
    IMS_SINT32 eADir;
    IMS_SINT32 eVDir;
    IMS_SINT32 eTDir;

    IMS_UINT32 eAQuality;
    IMS_UINT32 eVQuality;

    IMS_SINT32 eGTTMode;
};

class SuppService
{
public:
    inline SuppService() :
            strValue(AString::ConstNull()),
            nValue(0),
            bValue(IMS_FALSE)
    {
        IMS_TRACE_MEM(
                "uc", "uc_M : SuppService[%" PFLS_u "][%" PFLS_x "]", sizeof(SuppService), this, 0);
    }
    inline SuppService(IN const SuppService& objRHS) :
            strValue(objRHS.strValue),
            nValue(objRHS.nValue),
            bValue(objRHS.bValue)
    {
        IMS_TRACE_MEM(
                "uc", "uc_M : SuppService[%" PFLS_u "][%" PFLS_x "]", sizeof(SuppService), this, 0);
    }
    inline ~SuppService()
    {
        IMS_TRACE_MEM(
                "uc", "uc_F : SuppService[%" PFLS_u "][%" PFLS_x "]", sizeof(SuppService), this, 0);
    }

public:
    inline SuppService& operator=(IN const SuppService& objRHS)
    {
        if (this != &objRHS)
        {
            strValue = objRHS.strValue;
            nValue = objRHS.nValue;
            bValue = objRHS.bValue;
        }

        return (*this);
    }

public:
    AString strValue;
    IMS_SINT32 nValue;
    IMS_BOOL bValue;
};

class Recipient
{
public:
    inline Recipient() :
            strTarget(AString::ConstNull()),
            eCCType(COPYCONTROLTYPE_TO),
            bAnonymize(IMS_FALSE)
    {
        IMS_TRACE_MEM(
                "uc", "uc_M : Recipient[%" PFLS_u "][%" PFLS_x "]", sizeof(Recipient), this, 0);
    }
    inline Recipient(IN const Recipient& objRHS) :
            strTarget(objRHS.strTarget),
            eCCType(objRHS.eCCType),
            bAnonymize(objRHS.bAnonymize)
    {
        IMS_TRACE_MEM(
                "uc", "uc_M : Recipient[%" PFLS_u "][%" PFLS_x "]", sizeof(Recipient), this, 0);
    }
    inline ~Recipient()
    {
        IMS_TRACE_MEM(
                "uc", "uc_F : Recipient[%" PFLS_u "][%" PFLS_x "]", sizeof(Recipient), this, 0);
    }

public:
    inline Recipient& operator=(IN const Recipient& objRHS)
    {
        if (this != &objRHS)
        {
            strTarget = objRHS.strTarget;
            eCCType = objRHS.eCCType;
            bAnonymize = objRHS.bAnonymize;
        }

        return (*this);
    }

public:
    AString strTarget;
    IMS_UINT32 eCCType;
    IMS_BOOL bAnonymize;
};

// TODO: move to conference header / struct / ConferenceUser / aStr to str / remove unused.
class ConfUser
{
public:
    inline ConfUser() :
            nConnectionId(0),
            aStrTarget(AString::ConstNull()),
            aStrUserEntity(AString::ConstNull()),
            aStrEPEntity(AString::ConstNull()),
            aStrDisplayName(AString::ConstNull()),
            eStatus(CONFINFO_STATUS_IDLE),
            eStatusCode(-1),
            eCCType(COPYCONTROLTYPE_TO),
            bAnonymize(IMS_FALSE)
    {
    }
    inline ConfUser(IN const ConfUser& objRHS) :

            nConnectionId(objRHS.nConnectionId),
            aStrTarget(objRHS.aStrTarget),
            aStrUserEntity(objRHS.aStrUserEntity),
            aStrEPEntity(objRHS.aStrEPEntity),
            aStrDisplayName(objRHS.aStrDisplayName),
            eStatus(objRHS.eStatus),
            eStatusCode(objRHS.eStatusCode),
            eCCType(objRHS.eCCType),
            bAnonymize(objRHS.bAnonymize)
    {
    }
    inline ~ConfUser() {}

    ConfUser& operator=(IN const ConfUser&) = delete;

public:
    // connection id for a specific MtcCall
    IMS_UINT32 nConnectionId;
    // Phone Number for User Paricinpant - ex) Join
    AString aStrTarget;
    // Main Key after subscription for confernece from NOTIFY
    AString aStrUserEntity;
    // from NOTIFY about Conference Event package
    AString aStrEPEntity;
    // from NOTIFY about Conference Event package, by converting the operator's requirement
    AString aStrDisplayName;
    // Main Information from NOTIFY about Conference Event package
    IMS_UINT32 eStatus;
    // the detail code for eStatus
    IMS_SINT32 eStatusCode;
    // from NOTIFY about Conference Event package
    IMS_UINT32 eCCType;
    // from NOTIFY about Conference Event package
    IMS_BOOL bAnonymize;
};

class DialogInfo
{
public:
    inline DialogInfo() :
            aStrID(AString::ConstNull()),
            aStrCallID(AString::ConstNull()),
            aStrLocalTag(AString::ConstNull()),
            aStrRemoteTag(AString::ConstNull()),
            eState(0),
            eReason(0),
            eCode(0),
            aStrLocalName(AString::ConstNull()),
            aStrLocalNumber(AString::ConstNull()),
            aStrRemoteName(AString::ConstNull()),
            aStrRemoteNumber(AString::ConstNull()),
            bInitiator(IMS_FALSE),
            bEnablePull(IMS_FALSE),
            bConference(IMS_FALSE),
            pMediaInfo(IMS_NULL)
    {
        IMS_TRACE_MEM(
                "uc", "uc_M : DialogInfo[%" PFLS_u "][%" PFLS_x "]", sizeof(DialogInfo), this, 0);
    }
    inline DialogInfo(IN const DialogInfo& objRHS) :
            aStrID(objRHS.aStrID),
            aStrCallID(objRHS.aStrCallID),
            aStrLocalTag(objRHS.aStrLocalTag),
            aStrRemoteTag(objRHS.aStrRemoteTag),
            eState(objRHS.eState),
            eReason(objRHS.eReason),
            eCode(objRHS.eCode),
            aStrLocalName(objRHS.aStrLocalName),
            aStrLocalNumber(objRHS.aStrLocalNumber),
            aStrRemoteName(objRHS.aStrRemoteName),
            aStrRemoteNumber(objRHS.aStrRemoteNumber),
            bInitiator(objRHS.bInitiator),
            bEnablePull(objRHS.bEnablePull),
            bConference(objRHS.bConference),
            pMediaInfo(new MediaInfo(*(objRHS.pMediaInfo)))
    {
        IMS_TRACE_MEM(
                "uc", "uc_M : DialogInfo[%" PFLS_u "][%" PFLS_x "]", sizeof(DialogInfo), this, 0);
    }
    inline ~DialogInfo()
    {
        IMS_TRACE_MEM(
                "uc", "uc_F : DialogInfo[%" PFLS_u "][%" PFLS_x "]", sizeof(DialogInfo), this, 0);

        if (pMediaInfo != IMS_NULL)
        {
            delete pMediaInfo;
        }
    }

public:
    inline DialogInfo& operator=(IN const DialogInfo& objRHS)
    {
        if (this != &objRHS)
        {
            aStrID = objRHS.aStrID;
            aStrCallID = objRHS.aStrCallID;
            aStrLocalTag = objRHS.aStrLocalTag;
            aStrRemoteTag = objRHS.aStrRemoteTag;
            eState = objRHS.eState;
            eReason = objRHS.eReason;
            eCode = objRHS.eCode;
            aStrLocalName = objRHS.aStrLocalName;
            aStrLocalNumber = objRHS.aStrRemoteNumber;
            aStrRemoteName = objRHS.aStrLocalNumber;
            aStrRemoteNumber = objRHS.aStrRemoteNumber;
            bInitiator = objRHS.bInitiator;
            bEnablePull = objRHS.bEnablePull;
            bConference = objRHS.bConference;
            pMediaInfo->eADir = objRHS.pMediaInfo->eADir;
            pMediaInfo->eVDir = objRHS.pMediaInfo->eVDir;
            pMediaInfo->eTDir = objRHS.pMediaInfo->eTDir;
            pMediaInfo->eAQuality = objRHS.pMediaInfo->eAQuality;
            pMediaInfo->eVQuality = objRHS.pMediaInfo->eVQuality;
            pMediaInfo->eGTTMode = objRHS.pMediaInfo->eGTTMode;
        }

        return (*this);
    }

public:
    AString aStrID;
    AString aStrCallID;
    AString aStrLocalTag;
    AString aStrRemoteTag;

    IMS_UINT32 eState;
    IMS_SINT32 eReason;
    IMS_SINT32 eCode;

    AString aStrLocalName;
    AString aStrLocalNumber;

    AString aStrRemoteName;
    AString aStrRemoteNumber;

    IMS_BOOL bInitiator;
    IMS_BOOL bEnablePull;
    IMS_BOOL bConference;

    MediaInfo* pMediaInfo;
};

#endif
