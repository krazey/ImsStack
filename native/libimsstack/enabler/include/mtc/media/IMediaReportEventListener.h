#ifndef INTERFACE_MEDIA_REPORT_EVENT_LISTENER_H_
#define INTERFACE_MEDIA_REPORT_EVENT_LISTENER_H_

#include "ImsTypeDef.h"

class IMediaReportEventListener
{
public:
    virtual void OnReceivingMediaDataFailed(IN IMS_UINT32 eMediaType) = 0;
    virtual void OnVideoLowestBitRate() = 0;
    virtual void OnReceivingNetworkToneStarted() = 0;
    virtual void OnReceivingNetworkToneFailed() = 0;
    virtual void OnMediaFailed(IN FailReason objReason) = 0;
};

#endif
