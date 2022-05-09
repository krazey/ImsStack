#ifndef __SIP_MUTEX_H__
#define __SIP_MUTEX_H__

#include "sip_pf_datatypes.h"

class SipMutex
{
public:
    SipMutex();
    virtual ~SipMutex();

private:
    SipMutex(const SipMutex& objRHS);
    SipMutex& operator=(const SipMutex& objRHS);

public:
    void Lock();
    void TryLock();
    void Unlock();

private:
    void* pMutex;
};

#endif  // __SIP_MUTEX_H__
