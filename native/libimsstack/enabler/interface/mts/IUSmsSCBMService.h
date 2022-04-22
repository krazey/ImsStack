#ifndef IU_SMS_SCBM_SERVICE_H_
#define IU_SMS_SCBM_SERVICE_H_

#include "AString.h"
#include "ByteArray.h"
#include "ImsMessageDef.h"
#include "IUBaseParam.h"

#define JAVA2SMSSERVICE         IMS_MSG_BASE_SERVICE
#define SMSSERVICE2JAVA         IMS_MSG_BASE_SERVICE + 50

class IUSmsSCBMService
{
public:
    // Event : Java to IMS
    static const IMS_SINT32 NOTI_SMSSERVICE_SEND_MO_SMS     = JAVA2SMSSERVICE+ 1;
    static const IMS_SINT32 NOTI_IMS_PHONE_RESTARTED        = JAVA2SMSSERVICE+ 10;
    static const IMS_SINT32 NOTI_SMS_SERVER_READY           = JAVA2SMSSERVICE+ 11;
    static const IMS_SINT32 NOTI_SMS_SERVER_NOT_READY       = JAVA2SMSSERVICE+ 12;
    static const IMS_SINT32 NOTI_SMSSERVICE_SEND_MT_RESULT  = JAVA2SMSSERVICE+ 13;
    static const IMS_SINT32 NOTI_SMS_RAT_SELECTION          = JAVA2SMSSERVICE+ 14;

    static const IMS_SINT32 NOTI_EXIT_SCBM                  = JAVA2SMSSERVICE +15;
    static const IMS_SINT32 NOTI_SMS_SCBM_SERVER_READY      = JAVA2SMSSERVICE+ 16;
    static const IMS_SINT32 NOTI_SMS_SCBM_SERVER_NOT_READY  = JAVA2SMSSERVICE+ 17;

    // Event : IMS to Java
    static const IMS_UINT32 CREATE_SCBM_MANAGER             = SMSSERVICE2JAVA + 1;
    static const IMS_UINT32 DESTROY_SCBM_MANAGER            = SMSSERVICE2JAVA + 2;
    static const IMS_UINT32 UPDATE_SCBM_SVC_STATUS          = SMSSERVICE2JAVA + 3;
    static const IMS_UINT32 REPORT_MTS_MO_STATUS            = SMSSERVICE2JAVA + 5;
    static const IMS_UINT32 REQUEST_SMS_RAT_SELECTION       = SMSSERVICE2JAVA + 6;
    static const IMS_UINT32 REQUEST_SMS_EXIT_RAT_SELECTION  = SMSSERVICE2JAVA + 7;
    static const IMS_UINT32 REQUEST_SMS_SET_E911_STATE      = SMSSERVICE2JAVA + 8;
};


class IUSmsServiceUpdateServiceStatusParam
{
public:
    inline IUSmsServiceUpdateServiceStatusParam()
        : nImsStatus(0)
        , nSlotId(0)
    {}
    inline virtual ~IUSmsServiceUpdateServiceStatusParam()
    {}

public:
    IMS_SINT32 nImsStatus;
    IMS_SINT32  nSlotId;
};

class IUSmsServiceUpdateSCBMParam
{
public:
    inline IUSmsServiceUpdateSCBMParam()
        : nScbmMode(0)
    {}
    inline virtual ~IUSmsServiceUpdateSCBMParam()
    {}

public:
    IMS_SINT32 nScbmMode;
};

class IUSmsServiceCreateSCBMManagerParam
{
public:
    inline IUSmsServiceCreateSCBMManagerParam()
        : nSmsFormat(0)
    {}
    inline virtual ~IUSmsServiceCreateSCBMManagerParam()
    {}

public:
    IMS_SINT32 nSmsFormat;
};

#endif
