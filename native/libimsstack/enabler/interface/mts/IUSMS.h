/*
    Author
    IMSers
    <table>
    Date            Author                    Description
    --------      -----------------        ------------------
    20100929      changik.jeong@           Created
    20191016      rian.kim@                Modify Rety to connectSC
    </table>

    Description
--------------------------------------------------
*/

#ifndef _IUSMS_H_
#define _IUSMS_H_

#include "ImsMessageDef.h"

typedef int (*CBJniSmsService)(IN IMS_UINT32 nType, IN IMS_UINTP pParam, IN IMS_UINT32 slotId);


typedef int (*CBJniSmsSCBMService)(IN IMS_UINT32 nType, IN IMS_UINTP pParam, IN IMS_UINT32 slotId);

static const IMS_CHAR* STR_SMS_SVC_THREAD_NAME[]= {"JNISMSServiceThread0", "JNISMSServiceThread1"};


static const IMS_CHAR* STR_SMS_SCBM_SVC_THREAD_NAME[]= {"JNISMSSCBMServiceThread0", "JNISMSSCBMServiceThread1"};

//----------------------------------------------------------------------
// Messages for SMS
//----------------------------------------------------------------------
class IUSMS
{
    public:
        static const IMS_SINT32 SMS_SERVICEBLE            = (IMS_MSG_BASE_SESSION + 1);
        static const IMS_SINT32 SMSMO_SEND_REQUEST        = (IMS_MSG_BASE_SESSION + 2);
        static const IMS_SINT32 SMSMO_SEND_RESULT        = (IMS_MSG_BASE_SESSION + 3);
        static const IMS_SINT32 SMSMT_RECVD_REPORT        = (IMS_MSG_BASE_SESSION + 4);
        // SMS_Patch_0664
        static const IMS_SINT32 SMS_RECONNECT_SC        = (IMS_MSG_BASE_SESSION + 5);

        // Control messages for SMS app's communication channel
        static const IMS_SINT32 SMS_SERVICE_CONTROL = (IMS_MSG_BASE_SESSION + 11);
};

class SmsSvcInternal
{
    public:
        static const IMS_SINT32 SMSMT_RECVD                = (IMS_MSG_SMS + 0);

        // IMS_CHANGE_S [SMS_Patch_0621][VZW], MSG_SMS_TO_911 [
        static const IMS_SINT32 SMSMT_E_RECVD                       = (IMS_MSG_SMS + 1); // 15700 + 1
        static const IMS_SINT32 SMSMO_SEND_EMERGENCY_REQUEST        = (IMS_MSG_SMS + 2); // 15700 + 2
        static const IMS_SINT32 SMSMO_RESULT_RAT_SELECTION          = (IMS_MSG_SMS + 3); // 15700 + 3
        static const IMS_SINT32 SMS_REQUEST_EXIT_SCBM               = (IMS_MSG_SMS + 4); // 15700 + 4
        static const IMS_SINT32 SMS_REQUEST_EXIT_RAT_SELECTION      = (IMS_MSG_SMS + 5); // 15700 + 5
        static const IMS_SINT32 SMS_REQUEST_SET_E911_STATE          = (IMS_MSG_SMS + 6); // 15700 + 6
        // IMS_CHANGE_E [SMS_Patch_0621][VZW], MSG_TO_911 ]


};

class IUSmsMtResultCode
{
    public:
        static const int SMSMT_RESULT_NONE                    = 0;
        static const int SMSMT_RESULT_SUCCESS                = 1;
        static const int SMSMT_RESULT_NOSERVICE              = 2;
        static const int SMSMT_RESULT_OUTOFRESOURCES        = 3;
        static const int SMSMT_RESULT_DECODINGFAILURE        = 4;
        static const int SMSMT_RESULT_WRONGDESTINATION        = 5;
        static const int SMSMT_RESULT_MAX                    = 6;
};

class IUSmsFormat
{
    public:
                                                  /* IS-95 */
        static const int WMS_CDMA                    = 0;
                                                  /* IS-91 */
        static const int WMS_ANALOG_CLI                = 1;
                                                  /* IS-91 */
        static const int WMS_ANALOG_VOICE_MAIL        = 2;
                                                  /* IS-91 */
        static const int WMS_ANALOG_SMS                = 3;
                                                  /* IS-95 Alert With Information SMS */
        static const int WMS_ANALOG_AWISMS            = 4;
                                                  /* Message Waiting Indication as voice mail */
        static const int WMS_MWI                    = 5;
                                                  /* GW Point-to-Point SMS */
        static const int WMS_GW_PP                    = 6;
                                                  /* GW CB SMS */
        static const int WMS_GW_CB                    = 7;
                                                  /* PC TEST. insert it temporary for skipping l3 ack */
        static const int WMS_CDMA_PC_TEST            = 8;
        static const int FORMAT_MAX                    = 9;
};

class IUSmsSentResult
{
    public:
        static const int SENDRESULT_NONE                = 0;
        static const int SENDRESULT_OK                    = 1;
        static const int SENDRESULT_NOSRV                = 2;
        static const int SENDRESULT_ABORT                = 3;
        static const int SENDRESULT_OUTOFRESOURCES        = 4;
        static const int SENDRESULT_SIPTEMFAILURE        = 5;
        static const int SENDRESULT_SIPTXNTIMERTIMEOUT    = 6;
        static const int SENDRESULT_SIPPERMANENTFAILURE    = 7;
        static const int SENDRESULT_SIPRESP4XX            = 8;
        static const int SENDRESULT_SIPRESP5XX            = 9;
        static const int SENDRESULT_SIPRESP6XX            = 10;
        static const int SENDRESULT_MAX                    = 11;
};

class IUSmsServicebleParam{};

class IUSmsSendRequestParam
{
    public:
        IMS_UINT32     nSmsType;
        IMS_CHAR    szDestAddr[IMS_SOLUTION_URI_LEN+1];
        IMS_UINT32    nSmsDataLen;
        IMS_BYTE    baSmsData[512];
        IMS_SINT32  nMsgID;
        IMS_BOOL    bIsAckorError;
        IMS_SINT32  nSeqId;
        IMS_SINT32  nSlotId;
};

class IUSmsSendResultParam
{
    public:
        IMS_SINT32        nResult;
};

class IUReceivedSmsReportParam
{
    public:
        IMS_UINT32    nSmsType;
        IMS_UINT32    nSmsDataLen;
        IMS_BYTE    baSmsData[512];
};

class IUReceivedSmsConfirmParam
{
    public:
        IMS_SINT32        nResult;
};

// Control messages for SMS app's communication channel
class IUSmsServiceControlParam
{
public:
    // SMS control event
    enum
    {
        // If the modem interface is abnormally broken,
        // it needs to recover the communication channel.
        // So, if the connection recovery is required, this event will be posted.
        CMD_RECOVER_COMM_CHANNEL = 1,
        CMD_RECOVER_COMM_CHANNEL_BY_MODEM_RESET = 2  // IMS_CHANGE, [SMS_Patch_0585][COMMON], MSG_SMS_IMPROVE_MODEM_RESET_EVT_OVER_HAL
    };

public:
    IMS_SINT32 nCmd;
    IMS_SINT32 nSlotId;
};

// SMS_Patch_0664
class IUSmsReconnectScParam
{};

#endif //_IUSMS_H_
