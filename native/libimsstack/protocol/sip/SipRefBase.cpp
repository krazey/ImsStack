/***********************************************************
 * Project Name : SIP_RTP
 * Group        : MSG-2
 * Security     : Confidential
 ***********************************************************/

/**********************************************************
 * Filename          : SipRefBase.cpp
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

/*****************************************************************************
  Header Inclusions
 *****************************************************************************/
#include "SipRefBase.h"

/****************************************************************************
  Class Member Function Implementations
 *****************************************************************************/

/******************************************************************************
 * Function name      : SipRefBase::SipRefBase
 *
 * Description     :
 *
 * Preconditions      :
 *
 * Side Effects      : none
 *****************************************************************************/
SipRefBase::SipRefBase()
{
    m_nRefCount = SIP_ONE;
}

/******************************************************************************
 * Function name      : SipRefBase::~SipRefBase
 *
 * Description     :
 *
 * Preconditions      :
 *
 * Side Effects      : none
 *****************************************************************************/
SipRefBase::~SipRefBase()
{}

/******************************************************************************
 * Function name      : SipRefBase::increment
 *
 * Description     :
 *
 * Preconditions      :
 *
 * Side Effects      : none
 *****************************************************************************/
/**
  Increment the total references to the object
 */
SIP_VOID SipRefBase::increment()
{
    m_nRefCount++;
}

/******************************************************************************
 * Function name      : SipRefBase::decrement
 *
 * Description     :
 *
 * Preconditions      :
 *
 * Side Effects      : none
 *****************************************************************************/
/**
  Decrement the total references to the object
 */
SIP_VOID SipRefBase::decrement()
{
    m_nRefCount--;
}

/******************************************************************************
 * Function name      : SipRefBase::SipDelete
 *
 * Description     :
 *
 * Preconditions      :
 *
 * Side Effects      : none
 *****************************************************************************/
/**
  Delete the object only if there are no other references to it
 */
SIP_VOID SipRefBase::SipDelete()
{
    m_nRefCount--;
    if (m_nRefCount == SIP_ZERO)
    {
        delete this;
    }
}
