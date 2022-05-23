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
    static const IMS_SINT32 NOTI_EXIT_SCBM                  = JAVA2MTSENABLER + 11;


    // Event : IMS to Java
    static const IMS_UINT32 REPORT_MTS_MO_STATUS            = MTSENABLER2JAVA + 1;
    static const IMS_UINT32 REPORT_MTS_MT_SMS               = MTSENABLER2JAVA + 2;

    static const IMS_UINT32 REQUEST_MTS_RAT_SELECTION       = MTSENABLER2JAVA + 10;
    static const IMS_UINT32 REQUEST_MTS_EXIT_RAT_SELECTION  = MTSENABLER2JAVA + 11;
};

#endif
