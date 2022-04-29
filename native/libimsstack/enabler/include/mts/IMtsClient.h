#ifndef INTERFACE_IMS_IMTSCLIENT_H_
#define INTERFACE_IMS_IMTSCLIENT_H_

#include "ImsWmsLiteTypeDef.h"
#include "IMtsClientListener.h"

class IMtsClient
{
public:
    virtual IMSWMS_RESULT Init() = 0;
    virtual IMSWMS_RESULT Release() = 0;
    virtual IMSWMS_UINT32 GetFeature() = 0;
    virtual IMSWMS_UINT32 ReportMtSMS(
            IN IMSWMS_UINT32 smsformat,
            IN IMSWMS_UINT32 smslength,
            IN CONST IMSWMS_BYTE* smsdata,
            IN IMSWMS_SINT32 nSlotID) = 0;
    virtual IMSWMS_RESULT ReportMoStatus(
            IN IMSWMS_UINT32 reason,
            IN IMSWMS_UINT32 smsformat,
            IN IMSWMS_UINT8 nRetryAfter = 0,
            IN IMSWMS_SINT32 nSeqId = -1,
            IN IMSWMS_SINT32 nSlotID = -1) = 0;
    virtual void SetListener(IN IMtsClientListener* iscl) = 0;

public:

    // State of Service
    enum
    {
        STATE_INIT = 0,
        STATE_READY,
        STATE_LIMITED,
        STATE_NOTREADY
    };

    enum
    {
        SMSFORMAT_3GPP = 1,
        SMSFORMAT_3GPP2,
        SMSFORMAT_INVALID
    };

    /* these are sms mo (mobile originated) status codes of success and failure.
       ims and wms had a discussion for the status codes and we decided to have
       4 status codes. they seems to be fair enough now. if you need, you can
       add more but after a talk between wms and ims.
    */
    enum
    {
        MO_SUCCESS = 1,
        MO_IMS_TEMP_FAILURE = 2,              /* sending the sms failed but it's still ok to retry to send the sms. */
        MO_IMS_PERM_FAILURE = 3,
        /* sending the sms failed and it's not possible to retry to send the sms.
                                                       sms over ims service is still supportive. */
        MO_IMS_LIMITEDSMSSVCREGI = 4,
        /* ims is currently registered with imsi based uri. it means the administrative sms service mode.
                                                       phone is in a situation that sending sms is not allowed. */
        // MO_IMS_REQUEST_RETRY_TIMER = 5    // Check Mts Retry-After Header
    };

    /* These are result codes of processing received messages in WMS.
       IMS relays received sms messages to WMS and then WMS processes and gives back
       the result of processing the sms messages. below describes the results.
    */
    enum
    {
        MT_SUCCESS = 1,
        MT_FAILURE = 2,
        MT_SMS_FORMAT_FAILURE = 3,
        MT_SMS_NODATA_FAILURE = 4
    };

    enum
    {
        FEATURE_NONE = 0x0,
        FEATURE_IPC_ROUTER = 0x1
    };

    enum
    {
        SLOT_NONE = -1,
    };
};

#endif
