#ifndef INTERFACE_MTS_SERVICE_LISTENER_H_
#define INTERFACE_MTS_SERVICE_LISTENER_H_

#include "IPageMessage.h"

class IMtsServiceListener
{
public:
    virtual void NotifyMoSms(IN IMS_UINT32 nSmsFormat, IN const ByteArray& objData,
            IN const AString& strAddress, IN IMS_SINT32 nSeqId) = 0;
    virtual void NotifyMtSms(IN IPageMessage* piMessage) = 0;
};

#endif
