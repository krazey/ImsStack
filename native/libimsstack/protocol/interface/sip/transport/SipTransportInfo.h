#ifndef __SIP_TRANSPORT_INFO_H__
#define __SIP_TRANSPORT_INFO_H__

#include "sip_pf_datatypes.h"
#include "msg/sip_comdef.h"
#include "ISipUserData.h"
#include "transport/SipTransportParameter.h"
#include "transport/SipTransportBuffer.h"
#include "msg/SipMessage.h"

class SipTransportInfo
{
public:
    enum
    {
        PROTOCOL_INVALID = SIP_INVALID,
        PROTOCOL_TCP = 0,
        PROTOCOL_UDP,
        PROTOCOL_TLS,
        PROTOCOL_SCTP,
        PROTOCOL_OTHERS,
        PROTOCOL_END
    };

    enum
    {
        NETWORK_INVALID = SIP_INVALID,
        NETWORK_IPV4 = 0,
        NETWORK_IPV6,
        NETWORK_END
    };

    enum
    {
        ERROR_INVALID = SIP_INVALID,
        /* TCP Errors */
        TCP_CONN_ESTB_FAILURE_ERROR = 0,
        TCP_DESTINATION_NOT_FOUND_ERROR,
        TCP_OTHER_ERROR,

        /* ICMP Errors */
        ICMP_HOST_ERROR,
        ICMP_NETWORK_ERROR,
        ICMP_PORT_ERROR,
        ICMP_PROTOCOL_UNREACHABLEE_RROR,
        ICMP_PRAMETER_PROBLEM_ERROR,
        ICMP_SOURCE_QUENCH_ERROR,
        ICMP_TTL_EXCEED_ERROR,
        ICMP_OTHER_ERROR,

        ERROR_END
    };

private:
    /************************************************************
      Private    Member Variables
     **************************************************************/
    /* Number of Times message has been retransmitted */
    SIP_CHAR m_cNumTimeReqSent;

    /* Actual Transport used for Data Transmission :
       UPD on Size Constraint results
       in retransmssion, when TCP fails retry with the same UDP */

    SipTransportParameter* m_pActualDestParam;

    /*Transport parameter given by User */
    SipTransportParameter* m_pTranspParam;

    /* Actual Sent Buffer*/
    SipTransportBuffer* m_pSentBuffer;

    /* SipMessage Corresponding to actual sent buffer. used in callbacks to network */
    SipMessage* m_pSentSipMsg;

    /* Len > 200 bytes of MTU or Len > 1300 when MTU not known */
    SIP_BOOL m_bExceedMTU;

    /***********************************************************
      Private Member Functions
     ************************************************************/
    SipTransportInfo& operator=(IN const SipTransportInfo& objRHS);
    SipTransportInfo(IN const SipTransportInfo& objRHS);

public:
    /************************************************************
      Member Functions
     **************************************************************/
    SipTransportInfo();
    SipTransportInfo(SipTransportParameter* pTranspParam, SipTransportBuffer* pTransSipBuffer);
    virtual ~SipTransportInfo();
    /* Set APIs */

    /* Get APIs*/

    /* Returns the Transport parameter to which the Req/Resp was sent initially */
    SipTransportParameter* GetMsgSentTranspParam();

    SIP_BOOL SetMsgSentTranspParam(SipTransportParameter* pTranspParam);

    /* Returns the Transmitting SIP Buffer */
    SipTransportBuffer* GetTranspSipBuffer();

    SIP_BOOL IsExceedMTU();

    SIP_BOOL SetExceedMTUFlag(SIP_BOOL bFlag);
    SIP_VOID SetSentSipMsg(SipMessage* _pSipMsg);
    SipMessage* GetSentSipMsg();
};

#endif  //__SIP_TRANSPORT_INFO_H__
