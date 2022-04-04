/***********************************************************
 * Project Name : SIP_RTP
 * Group        : MSG-2
 * Security     : Confidential
 ***********************************************************/

/**********************************************************
 * Filename          : SipRefBase.h
 * Purpose           :
 * Platform          : Windows XP
 * Author(s)         : Saurabh Srivastava
 * E-mail id.        : saurabh31.srivastava@
 * Creation date     : July 26, 2010
 *
 * Modifications:
 * 1. Modified by    : <Name>
 *    Date           : <mmm. dd, yyyy> (E.g. Apr. 21, 2006)
 *    Description    :
 *    Version Number : 0.0a
 *
 * 2. Modified by    : <Name>
 *    Date           : <mmm. dd, yyyy> (E.g. Apr. 21, 2006)
 *    Description    :
 *    Version Number : 0.0b
 **********************************************************/
#ifndef __SIP_REF_BASE_H__
#define __SIP_REF_BASE_H__

/*****************************************************************************
  Header Inclusions
 *****************************************************************************/
#include "sip_pf_datatypes.h"

/****************************************************************************
  Macro Definitions
 *****************************************************************************/

/****************************************************************************
  Class Declaration Starts
 *****************************************************************************/
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
        /* TEST */ inline SIP_INT16 GetRefCount() const
        {
            return m_nRefCount;
        }
    private:
        SipRefBase& operator=(IN const SipRefBase& objRHS);
        SipRefBase(IN const SipRefBase& objRHS);
};



#endif //__SIP_REF_BASE_H__
