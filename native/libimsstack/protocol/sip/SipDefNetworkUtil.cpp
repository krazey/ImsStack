#include "sip_pf_datatypes.h"
#include "platform/sip_pf_string.h"
#include "SipDefNetworkUtil.h"

SipDefNetworkUtil::SipDefNetworkUtil()
{
}
SipDefNetworkUtil::~SipDefNetworkUtil()
{
}

SIP_BOOL SipDefNetworkUtil::SendToNetwork(SipTransportBuffer* pTransportBuffer,
        SipTransportParameter* pTransportParam, ISipUserData* pUserData)
{
    (void)pTransportBuffer;
    (void)pTransportParam;
    (void)pUserData;
    return SIP_TRUE;
}
