#ifndef JNI_MTS_SERVICE_THREAD_H_
#define JNI_MTS_SERVICE_THREAD_H_

#include "BaseServiceThread.h"
#include "IuMtsService.h"

class JniMtsServiceThread : public BaseServiceThread
{
public:
    JniMtsServiceThread();
    virtual ~JniMtsServiceThread();

    void ReportMoStatus(IN IMS_UINT32 nReason, IN IMS_UINT32 nSmsFormat, IN IMS_UINT8 nRetryAfter,
            IN IMS_SINT32 nSeqId, IN IMS_SINT32 nSlotId);
    void ReportMtSms(IN IMS_UINT32 nSmsFormat, IN const ByteArray& objData, IN IMS_SINT32 nSlotId);
};

#endif
