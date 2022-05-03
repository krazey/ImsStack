#ifndef INTERFACE_MTC_PRECONDITION_LISTENER_H_
#define INTERFACE_MTC_PRECONDITION_LISTENER_H_

#include "IMSTypeDef.h"
#include "ISession.h"

enum class QosLossPolicy;

class IMtcPreconditionListener
{
public:
    virtual ~IMtcPreconditionListener(){};

    virtual void QosReserved(IN ISession* piSession, IN IMS_UINT32 eMediaType) = 0;
    virtual void QosReserveFailed(IN ISession* piSession, IN QosLossPolicy eNextAction) = 0;
};

#endif
