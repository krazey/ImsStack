#ifndef INTERFACE_MTS_SERVICE_H_
#define INTERFACE_MTS_SERVICE_H_

#include "IuMtsService.h"

class JniMtsService;

class IMtsService
{
public:
    virtual void SetJniMtsService(IN JniMtsService* pJniMtsService) = 0;
    virtual void SendMoSms(IN IMS_UINT32 nSmsFormat, IN const ByteArray& objData,
            IN const AString& strAddress, IN IMS_SINT32 nSeqId) = 0;
    virtual void SendMtResult(IN IMS_BOOL bMtResult) = 0;
};

#endif
