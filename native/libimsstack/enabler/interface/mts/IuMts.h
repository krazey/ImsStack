#ifndef IUMTS_H_
#define IUMTS_H_

#include "ImsMessageDef.h"

LOCAL const IMS_CHAR* STR_MTS_SVC_THREAD_NAME[]= {"JniMtsServiceThread_0", "JniMtsServiceThread_1"};

#define MTS_MAX_PDU_DATA_LEN 512

class IuMts
{
    public:
        static const IMS_SINT32 MTS_MO_SEND_REQUEST                 = (IMS_MSG_BASE_SESSION + 1);
        static const IMS_SINT32 MTS_MO_SEND_RESULT                  = (IMS_MSG_BASE_SESSION + 2);
        static const IMS_SINT32 MTS_MT_RECVD_REPORT                 = (IMS_MSG_BASE_SESSION + 3);
};

class MtsServiceInternal
{
    public:
        static const IMS_SINT32 MTS_MT_RECVD                        = (IMS_MSG_SMS + 0);

        // TODO: These will be used for E911 SMS case
        static const IMS_SINT32 MTS_MT_E_RECVD                      = (IMS_MSG_SMS + 1);
        static const IMS_SINT32 MTS_MO_SEND_EMERGENCY_REQUEST       = (IMS_MSG_SMS + 2);
        static const IMS_SINT32 MTS_MO_RESULT_RAT_SELECTION         = (IMS_MSG_SMS + 3);
        static const IMS_SINT32 MTS_REQUEST_EXIT_SCBM               = (IMS_MSG_SMS + 4);
        static const IMS_SINT32 MTS_REQUEST_EXIT_RAT_SELECTION      = (IMS_MSG_SMS + 5);
        static const IMS_SINT32 MTS_REQUEST_SET_E911_STATE          = (IMS_MSG_SMS + 6);
};

enum class IuMtsMoResultCode
{
    MTS_MO_RESULT_NONE = 0,
    MTS_MO_RESULT_OK,
    MTS_MO_RESULT_NOSRV,
    MTS_MO_RESULT_ABORT,
    MTS_MO_RESULT_OUTOFRESOURCES,
    MTS_MO_RESULT_SIPTEMFAILURE,
    MTS_MO_RESULT_SIPTXNTIMERTIMEOUT,
    MTS_MO_RESULT_SIPPERMANENTFAILURE,
    MTS_MO_RESULT_SIPRESP4XX,
    MTS_MO_RESULT_SIPRESP5XX,
    MTS_MO_RESULT_SIPRESP6XX,
    MTS_MO_RESULT_MAX
};

enum class IuMtsMtResultCode
{
    MTS_MT_RESULT_NONE = 0,
    MTS_MT_RESULT_SUCCESS,
    MTS_MT_RESULT_NOSERVICE,
    MTS_MT_RESULT_OUTOFRESOURCES,
    MTS_MT_RESULT_DECODINGFAILURE,
    MTS_MT_RESULT_WRONGDESTINATION,
    MTS_MT_RESULT_MAX
};

enum class IuSmsFormat
{
    // IS-95
    WMS_CDMA = 0,
    // IS-91
    WMS_ANALOG_CLI,
    // IS-91
    WMS_ANALOG_VOICE_MAIL,
    // IS-91
    WMS_ANALOG_SMS,
    // IS-95 Alert With Information SMS
    WMS_ANALOG_AWISMS,
    // Message Waiting Indication as voice mail
    WMS_MWI,
    // GW Point-to-Point SMS
    WMS_GW_PP,
    // GW CB SMS
    WMS_GW_CB,
    // PC TEST. insert it temporary for skipping l3 ack
    WMS_CDMA_PC_TEST,
    FORMAT_MAX
};

#endif
