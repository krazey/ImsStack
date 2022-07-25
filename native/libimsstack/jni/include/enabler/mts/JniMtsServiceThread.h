#ifndef JNI_MTS_SERVICE_THREAD_H_
#define JNI_MTS_SERVICE_THREAD_H_

#include "BaseServiceThread.h"
#include "IuMtsService.h"
#include "MtsDef.h"

class JniMtsServiceThread : public BaseServiceThread
{
public:
    JniMtsServiceThread();
    virtual ~JniMtsServiceThread();

    void ReportMoStatus(IN IMS_UINT32 nReason, IN SmsFormatType eSmsFormat,
            IN IMS_UINT8 nRetryAfter, IN IMS_SINT32 nSeqId, IN IMS_SINT32 nSlotId);
    void ReportMtSms(
            IN SmsFormatType eSmsFormat, IN const ByteArray& objData, IN IMS_SINT32 nSlotId);

private:
    IMS_UINT32 ConvertSmsFormatToInt(IN SmsFormatType eSmsFormat);
};

#endif
