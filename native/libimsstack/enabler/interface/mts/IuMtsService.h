#ifndef IU_MTS_SERVICE_H_
#define IU_MTS_SERVICE_H_

#include "AString.h"
#include "ByteArray.h"
#include "ImsMessageDef.h"
#include "IUBaseParam.h"

#define JAVA2MTSENABLER         IMS_MSG_BASE_SERVICE
#define MTSENABLER2JAVA         IMS_MSG_BASE_SERVICE + 50

class IuMtsService
{
public:
    // Event : Java to IMS
    static const IMS_SINT32 NOTI_MTSENABLER_SEND_MO_SMS     = JAVA2MTSENABLER + 1;
    static const IMS_SINT32 NOTI_MTSENABLER_SEND_MT_RESULT  = JAVA2MTSENABLER + 2;

    static const IMS_SINT32 NOTI_MTS_RAT_SELECTION          = JAVA2MTSENABLER + 10;
    static const IMS_SINT32 NOTI_SCBM_STATE                 = JAVA2MTSENABLER + 11;


    // Event : IMS to Java
    static const IMS_UINT32 REPORT_MTS_MO_STATUS            = MTSENABLER2JAVA + 1;
    static const IMS_UINT32 REPORT_MTS_MT_SMS               = MTSENABLER2JAVA + 2;

    static const IMS_UINT32 REQUEST_MTS_RAT_SELECTION       = MTSENABLER2JAVA + 10;
    static const IMS_UINT32 REQUEST_MTS_EXIT_RAT_SELECTION  = MTSENABLER2JAVA + 11;
};

enum
{
    MO_INVALID = 0,
    MO_SUCCESS = 1,
    MO_IMS_TEMP_FAILURE = 2,
    MO_IMS_PERM_FAILURE = 3,
    MO_IMS_LIMITEDSMSSVCREGI = 4,
    MO_RETRY_CS = 5,
    MO_RETRY_CS_OR_SGS = 6,
};

enum
{
    MT_INVALID = 0,
    MT_SUCCESS = 1,
    MT_FAILURE = 2,
    MT_SMS_FORMAT_FAILURE = 3,
    MT_SMS_NODATA_FAILURE = 4,
};

enum
{
    NOTIFY_SCBM_STARTED = 1,
    NOTIFY_SCBM_TERMINATED = 2,
    NOTIFY_SCBM_TERMINATED_BY_ECALL = 3,
};

enum
{
    SMSFORMAT_3GPP = 1,
    SMSFORMAT_3GPP2 = 2,
    SMSFORMAT_INVALID = 3,
};

#endif
