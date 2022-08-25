#ifndef JNI_MTS_SERVICE_THREAD_H_
#define JNI_MTS_SERVICE_THREAD_H_

#include "BaseServiceThread.h"
#include "IJniMtsServiceThread.h"
#include "MtsDef.h"

class JniMtsServiceThread : public BaseServiceThread, public IJniMtsServiceThread
{
public:
    JniMtsServiceThread();
    virtual ~JniMtsServiceThread();

    void ReportMoStatus(IN IMS_SINT32 nReason, IN SmsFormatType eSmsFormat,
            IN IMS_UINT8 nRetryAfter, IN IMS_SINT32 nSeqId, IN IMS_SINT32 nSlotId) override;
    void ReportMtSms(IN SmsFormatType eSmsFormat, IN const ByteArray& objData,
            IN IMS_SINT32 nSlotId) override;

private:
    IMS_UINT32 ConvertSmsFormatToInt(IN SmsFormatType eSmsFormat);
};

#endif
