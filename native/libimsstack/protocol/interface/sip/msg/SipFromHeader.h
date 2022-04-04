/******************************************************************************
 * Project Name     : SIP_RTP
 * Group            : IP-CS [MSG-2]
 * Security         : Confidential
 *****************************************************************************/

/******************************************************************************

 * Filename              : SipFromHeader.h
 * Purpose               :
 * Platform              : Windows OR Android
 * Author(s)           :
 * E-mail id.            : saurabh31.srivastava@
 * Creation date       : Month. Date,10
 *
 * Edit History         Modification description(s)
 * Date                Name            Version        Bug-ID        Description
 * ----------        ----------        -------        ------        -------------
 * Month. Date,10        Name                 0.0a            Initial creation
 *****************************************************************************/

#ifndef __SIP_FROM_HEADER_H__
#define __SIP_FROM_HEADER_H__

/*****************************************************************************
  Header Inclusions
 *****************************************************************************/
#include "msg/SipHeaderBase.h"


/****************************************************************************
  Macro Definitions
 *****************************************************************************/


/****************************************************************************
  Enum Declaration
 *****************************************************************************/

/****************************************************************************
  Class Declaration Starts
 *****************************************************************************/

class SipFromHeader : public SipNameAddrHeader
{
    private:
        //    SIP_CHAR    *m_pszDispName;

    public:
        /*constructor*/
        SipFromHeader();

        /* ### TODO */
        SipFromHeader(SIP_CHAR* pszDispName);
        SipFromHeader(const SipFromHeader& objHeader);
        /*destructor*/
        ~SipFromHeader();
        static SipHeaderBase* GetNewObj(SIP_INT32 eHeaderType, SipHeaderBase* pHeader);

        SIP_CHAR* GetTag();

        SIP_BOOL SetTag(SIP_CHAR* pszFromTag);
};
#endif //__SIP_FROM_HEADER_H__
