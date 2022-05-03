#ifndef INTERFACE_MTC_CALL_STATE_LISTENER_H_
#define INTERFACE_MTC_CALL_STATE_LISTENER_H_

#include "IMtcService.h"
#include "call/IMtcCall.h"

class IMtcCallStateListener
{
public:
    using State = IMtcCall::State;
    using Type = CallType;

    virtual void OnCallStateChanged(IN CallKey nCallKey, IN State eState, IN Type eType,
            IN IMS_BOOL bEmergency, IN IMS_SINT32 nReason) = 0;
    virtual void OnTotalCallStateChanged(IN State eState) = 0;
    inline virtual IMS_BOOL IsSynchronousCallRequired() { return IMS_FALSE; }
};

#endif
