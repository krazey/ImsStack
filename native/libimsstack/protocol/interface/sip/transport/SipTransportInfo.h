/*
 * Copyright (C) 2022 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#ifndef __SIP_TRANSPORT_INFO_H__
#define __SIP_TRANSPORT_INFO_H__

#include "ISipUserData.h"
#include "SipDatatypes.h"
#include "msg/SipMessage.h"
#include "msg/SipMsgUtil.h"
#include "transport/SipTransportBuffer.h"
#include "transport/SipTransportParameter.h"

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
    SipTransportInfo& operator=(IN const SipTransportInfo& objRHS);
    SipTransportInfo(IN const SipTransportInfo& objRHS);

public:
    SipTransportInfo();
    SipTransportInfo(
            const SipTransportParameter* pTranspParam, SipTransportBuffer* pTransSipBuffer);
    virtual ~SipTransportInfo();

    SipTransportParameter* GetMsgSentTranspParam();

    inline SIP_VOID SetMsgSentTranspParam(SipTransportParameter* pTranspParam)
    {
        m_pActualDestParam = pTranspParam;
    }

    /* Returns the Transmitting SIP Buffer */
    SipTransportBuffer* GetTranspSipBuffer();

    SIP_VOID SetSentSipMsg(SipMessage* _pSipMsg);
    SipMessage* GetSentSipMsg();
};

#endif  //__SIP_TRANSPORT_INFO_H__
