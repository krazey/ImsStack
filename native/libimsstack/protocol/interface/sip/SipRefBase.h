#ifndef __SIP_REF_BASE_H__
#define __SIP_REF_BASE_H__

#include "sip_pf_datatypes.h"

/**
  This is a base class for all classes that intend to implement reference counting.
  increment() function should be used when either reference is getting stored
  and decrement() function should be used when either reference is permanently deleted.
  SipDelete() should always be used to delete the derived class objects. This ensure that
  objects which have reference in other modules shall not be deleted.
 */

class SipRefBase
{
protected:
    SIP_INT16 m_nRefCount;

public:
    SipRefBase();
    virtual ~SipRefBase();
    SIP_VOID increment();
    SIP_VOID decrement();
    virtual SIP_VOID SipDelete();
    /* TEST */ inline SIP_INT16 GetRefCount() const { return m_nRefCount; }

private:
    SipRefBase& operator=(IN const SipRefBase& objRHS);
    SipRefBase(IN const SipRefBase& objRHS);
};

#endif  //__SIP_REF_BASE_H__
