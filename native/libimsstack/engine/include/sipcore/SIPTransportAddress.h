/*
    Author
    <table>
    date      author                    description
    --------  --------------            ----------
    20100322  hwangoo.park@             Change the class name from SIPTransportTuple
    </table>

    Description

*/

#ifndef _SIP_TRANSPORT_ADDRESS_H_
#define _SIP_TRANSPORT_ADDRESS_H_

#include "IPAddress.h"

class SIPTransportAddress
{
public:
    SIPTransportAddress();
    SIPTransportAddress(IN CONST SIPTransportAddress& objRHS);
    SIPTransportAddress(
            IN IMS_SINT32 nProtocol_, IN IMS_SINT32 nPort_, IN CONST AString& strAddress_);
    ~SIPTransportAddress();

public:
    SIPTransportAddress& operator=(IN CONST SIPTransportAddress& objRHS);

public:
    IMS_BOOL Equals(IN CONST SIPTransportAddress& objTA) const;

    inline const IPAddress& GetIPAddress() const { return objIPAddress; }
    inline IMS_SINT32 GetPort() const { return nPort; }
    inline IMS_SINT32 GetProtocol() const { return nProtocol; }
    inline void SetIPAddress(IN CONST IPAddress& objIPAddress)
    {
        this->objIPAddress = objIPAddress;
    }
    inline void SetPort(IN IMS_UINT32 nPort) { this->nPort = nPort; }
    inline void SetProtocol(IN IMS_SINT32 nProtocol) { this->nProtocol = nProtocol; }

public:
    // Types of the transport supported by SIP stack
    enum
    {
        PROTOCOL_ANY = 0,

        PROTOCOL_UDP,
        PROTOCOL_TCP,
        PROTOCOL_TLS,

        PROTOCOL_MAX
    };

private:
    // TRANSPORT_UDP, ... in SIP class or TYPE_UDP, ... in SIPSocket class
    IMS_SINT32 nProtocol;
    // Port; If -1, the default port number will be selected according to the transport protocol
    IMS_SINT32 nPort;
    // IP address (null-terminating dotted numeric IP address)
    IPAddress objIPAddress;
};

#endif  // _SIP_TRANSPORT_ADDRESS_H_
