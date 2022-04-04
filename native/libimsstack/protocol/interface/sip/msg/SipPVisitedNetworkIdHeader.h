/******************************************************************************
 * Project Name     : SIP_RTP
 * Group            : IP-CS [MSG-2]
 * Security         : Confidential
 *****************************************************************************/

/******************************************************************************

 * Filename              : SipPVisitedNetworkIdHeader.h
 * Purpose               :
 * Platform              : Windows OR Android
 * Author(s)           : Saurabh Srivastava
 * E-mail id.            : saurabh31.srivastava@
 * Creation date       : Month. Date,10
 *
 * Edit HisAcceptEncodingry         Modification description(s)
 * Date                Name            Version        Bug-ID        Description
 * ----------        ----------        -------        ------        -------------
 * Month. Date,10        Name                 0.0a            Initial creation
 *****************************************************************************/

#ifndef __SIP_P_VISITED_NETWORK_ID_HEADER_H__
#define __SIP_P_VISITED_NETWORK_ID_HEADER_H__

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

class SipPVisitedNetworkIdHeader : public SipHeaderBase
{
    private:
        /*List of obj of CSipNameValue*/
        //    SipParameterList        *m_pMPrm;

    public:
        /*constructor*/
        SipPVisitedNetworkIdHeader();

        /*Copy constructor*/
        SipPVisitedNetworkIdHeader(const SipPVisitedNetworkIdHeader& objHeader);

        /*destructor*/
        ~SipPVisitedNetworkIdHeader();

        static SipHeaderBase* GetNewObj(SIP_INT32 eHeaderType, SipHeaderBase* pHeader);

        /*virtual methods*/
        /*Function for decoding of headers*/
        SIP_BOOL DecodeHdr(SIP_CHAR* pStartPt, SIP_UINT32 nDecLen);
};
#endif //__SIP_P_VISITED_NETWORK_ID_HEADER_H__
