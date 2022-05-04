#ifndef INTERFACE_MEDIA_QOS_EVENT_LISTENER_H_
#define INTERFACE_MEDIA_QOS_EVENT_LISTENER_H_

#include "IMSTypeDef.h"
#include "ISession.h"
#include "precondition/IMtcPreconditionManager.h"
#include "precondition/QosDef.h"

class IMediaQosEventListener
{
public:
    virtual ~IMediaQosEventListener(){};

    virtual void OnQosStatusChanged(
            IN ISession* piSession, IN QosStatus eStatus, IN IMS_UINT32 eMediaType) = 0;
};
#endif
