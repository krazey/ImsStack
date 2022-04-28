#ifndef INTERFACE_MTS_CALL_TRACKER_LISTENER_H_
#define INTERFACE_MTS_CALL_TRACKER_LISTENER_H_

#include "IMSTypeDef.h"

class IMtsCallTrackerListener
{
public:
    virtual void CallTracker_StateChanged(IN IMS_UINT32 nType, IN IMS_UINT32 nState) = 0;
};

#endif
