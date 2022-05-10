#ifndef __SIP_VISITED_NETWORK_ID_HEADER_H__
#define __SIP_VISITED_NETWORK_ID_HEADER_H__

#include "msg/SipHeaderBase.h"

class SipVisitedNetworkIdHeader : public SipParameters
{
private:
    SIP_CHAR* m_pszVisitedNetwork;

public:
    /*constructor*/
    SipVisitedNetworkIdHeader();

    /*destructor*/
    ~SipVisitedNetworkIdHeader();

    /*virtual methods*/
    /*Function for encoding of headers*/
    SIP_BOOL EncodeHdr(SIP_CHAR** ppCurrPos, SIP_BOOL bParams = SIP_TRUE);

    /*Function for decoding of headers*/
    SIP_BOOL DecodeHdr(SIP_CHAR* pStartPt, SIP_UINT32 nDecLen);

    /*Gets the visited nw name*/
    inline const SIP_CHAR* GetVisitedNetwork() const { return m_pszVisitedNetwork; }
};
#endif  //__SIP_VISITED_NETWORK_ID_HEADER_H__
