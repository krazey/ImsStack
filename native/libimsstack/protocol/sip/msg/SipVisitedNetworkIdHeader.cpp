#include "msg/SipVisitedNetworkIdHeader.h"
#include "platform/sip_pf_string.h"

/*constructor*/
SipVisitedNetworkIdHeader::SipVisitedNetworkIdHeader() :
        SipParameters(),
        m_pszVisitedNetwork(SIP_NULL)
{
}

/*destructor*/
SipVisitedNetworkIdHeader::~SipVisitedNetworkIdHeader() {}

/*virtual methods*/
/*Function for encoding of headers*/
SIP_BOOL SipVisitedNetworkIdHeader::EncodeHdr(
        SIP_CHAR** ppCurrPos, SIP_BOOL bParams /*Default = SIP_TRUE*/)
{
    (void)ppCurrPos;
    (void)bParams;
    return SIP_TRUE;
}

/*Function for decoding of headers*/
SIP_BOOL SipVisitedNetworkIdHeader::DecodeHdr(SIP_CHAR* pStartPt, SIP_UINT32 nDecLen)
{
    (void)pStartPt;
    (void)nDecLen;
    return SIP_TRUE;
}
