#ifndef INTERFACE_SRVCC_STATE_LISTENER_H_
#define INTERFACE_SRVCC_STATE_LISTENER_H_

#include "IMSTypeDef.h"
#include "IMtcService.h"

enum class SrvccState
{
    IDLE = -1,
    STARTED,
    SUCCEEDED,
    FAILED,
    CANCELED
};

class ISrvccStateListener
{
public:
    virtual void OnStateUpdated(IN SrvccState eState) = 0;
};

#endif
