/*
    Author
    <table>
    date      author                    description
    --------  --------------            ----------
    20090326  toastops@                 Created
    </table>

    Description

*/

#ifndef _SIP_SERVER_TRANSPORT_H_
#define _SIP_SERVER_TRANSPORT_H_

#include "SipTransport.h"

class SIPServerTransport : public SIPTransport
{
public:
    SIPServerTransport(IN IMS_SINT32 nSlotId, IN CONST SIPTransportAddress& objTA_NearEnd_,
            IN CONST SIPTransportAddress& objTA_FarEnd_);
    virtual ~SIPServerTransport();

private:
    SIPServerTransport(IN CONST SIPServerTransport& objRHS);
    SIPServerTransport& operator=(IN CONST SIPServerTransport& objRHS);

public:
    // MULTI_REG_SIP_PROFILE
    virtual IMS_BOOL FormViaHeader(
            IN_OUT SipMessage*& pstMessage, IN CONST SipProfile* pSIPProfile = IMS_NULL);
    virtual IMS_BOOL UpdateDestinationInfo(IN SipMessage* pstMessage,
            IN IMS_BOOL bRoutingLR = IMS_TRUE, IN SipAddrSpec* pstImplicitRoute = IMS_NULL);
    virtual IMS_SINT32 ValidateViaHeader(IN SipMessage* pstMessage);
};

#endif  // _SIP_SERVER_TRANSPORT_H_
