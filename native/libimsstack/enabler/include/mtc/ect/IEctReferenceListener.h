#ifndef INTERFACE_ECT_REFERENCE_LISTENER_H_
#define INTERFACE_ECT_REFERENCE_LISTENER_H_

#include "SipStatusCode.h"

class IEctReferenceListener
{
public:
    virtual void OnReferenceStarted() = 0;
    virtual void OnReferenceStartFailed() = 0;
    virtual void OnReferenceUpdated(IN SipStatusCode nSipFragCode) = 0;
};

#endif
