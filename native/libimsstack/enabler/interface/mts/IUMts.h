#ifndef IUMTS_H_
#define IUMTS_H_

#include "ImsMessageDef.h"

typedef int (*CBJniMtsService)(IN IMS_UINT32 nType, IN IMS_UINTP pParam, IN IMS_UINT32 nSlotId);
LOCAL const IMS_CHAR* STR_MTS_SVC_THREAD_NAME[]= {"JniMtsServiceThread0", "JniMtsServiceThread1"};

class IUMts
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

enum class IUSmsMtResultCode
{
    MTS_MT_RESULT_NONE = 0,
    MTS_MT_RESULT_SUCCESS,
    MTS_MT_RESULT_NOSERVICE,
    MTS_MT_RESULT_OUTOFRESOURCES,
    MTS_MT_RESULT_DECODINGFAILURE,
    MTS_MT_RESULT_WRONGDESTINATION,
    MTS_MT_RESULT_MAX
};

enum class IUSmsFormat
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

enum class IUSmsSentResult
{
    SENDRESULT_NONE = 0,
    SENDRESULT_OK,
    SENDRESULT_NOSRV,
    SENDRESULT_ABORT,
    SENDRESULT_OUTOFRESOURCES,
    SENDRESULT_SIPTEMFAILURE,
    SENDRESULT_SIPTXNTIMERTIMEOUT,
    SENDRESULT_SIPPERMANENTFAILURE,
    SENDRESULT_SIPRESP4XX,
    SENDRESULT_SIPRESP5XX,
    SENDRESULT_SIPRESP6XX,
    SENDRESULT_MAX
};

class IUSendSmsRequestParam
{
    public:
        IMS_UINT32      m_nSmsType;
        IMS_CHAR        m_szDestAddr[IMS_SOLUTION_URI_LEN+1];
        IMS_UINT32      m_nSmsDataLen;
        IMS_BYTE        m_baSmsData[512];
        IMS_SINT32      m_nMsgId;
        IMS_BOOL        m_bAckOrError;
        IMS_SINT32      m_nSeqId;
        IMS_SINT32      m_nSlotId;
};

class IUSendSmsResultParam
{
    public:
        IMS_SINT32      m_nResult;
};

class IUReceivedSmsReportParam
{
    public:
        IMS_UINT32      m_nSmsType;
        IMS_UINT32      m_nSmsDataLen;
        IMS_BYTE        m_baSmsData[512];
};

class IUReceivedSmsConfirmParam
{
    public:
        IMS_SINT32      m_nResult;
};

#endif
