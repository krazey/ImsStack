#ifndef IU_MTS_SERVICE_H_
#define IU_MTS_SERVICE_H_

#include "AString.h"
#include "ByteArray.h"
#include "ImsMessageDef.h"
#include "IUBaseParam.h"

#define JAVA2MTSENABLER         IMS_MSG_BASE_SERVICE
#define MTSENABLER2JAVA         IMS_MSG_BASE_SERVICE + 50

class IUMtsService
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

class IUMtsServiceReportMoStatusParam
{
public:
    inline IUMtsServiceReportMoStatusParam() :
            nReason(0),
            nSmsFormat(0),
            nRetryAfter(0),
            nSeqId(-1),
            nSlotId(0)
    {}
    inline virtual ~IUMtsServiceReportMoStatusParam()
    {}

public:
    IMS_SINT32      nReason;
    IMS_UINT32      nSmsFormat;
    IMS_UINT8       nRetryAfter;
    IMS_SINT32      nSeqId;
    IMS_SINT32      nSlotId;
};

class IUMtsServiceReportMtSmsParam
{
public:
    inline IUMtsServiceReportMtSmsParam() :
            nSmsFormat(0),
            objData(ByteArray::ConstNull()),
            nSlotId(0)
    {}
    inline virtual ~IUMtsServiceReportMtSmsParam()
    {}

public:
    IMS_UINT32      nSmsFormat;
    ByteArray       objData;
    IMS_SINT32      nSlotId;
};

class IUMtsServiceSendMoSmsParam
{
public:
    inline IUMtsServiceSendMoSmsParam() :
            nSmsFormat(0),
            objData(ByteArray::ConstNull()),
            strAddr(AString::ConstNull()),
            nSeqId(-1)
    {}
    inline virtual ~IUMtsServiceSendMoSmsParam()
    {}

public:
    IMS_UINT32      nSmsFormat;
    ByteArray       objData;
    AString         strAddr;
    IMS_SINT32      nSeqId;
};

#endif
