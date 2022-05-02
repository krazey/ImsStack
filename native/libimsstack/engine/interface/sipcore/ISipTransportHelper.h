#ifndef _INTERFACE_SIP_TRANSPORT_HELPER_H_
#define _INTERFACE_SIP_TRANSPORT_HELPER_H_

#include "IPAddress.h"
#include "SipRtConfig.h"

class ISIPLocalDnsQueryListener;

/**
 * @brief This class provides an interface to access/control SIP transport layer.
 */
class ISIPTransportHelper
{
public:
    /**
     * @brief Applies SIP server sockets and new IPSec SA information.
     */
    virtual void ApplyIpSecForServerSockets() = 0;

    /**
     * @brief Destroys all the socket objects which are managed by the SIP transport helper.
     *
     * @param nMethod Specify the method which all the sockets are destroyed\n
     *                0 - destroy directly (sync.) (default)\n
     *                1 - destroy indirectly (async.)
     * @param objLocalIP Specify local(bound) IP address to destroy a socket
     */
    virtual void DestroyAllSockets(IN IMS_SINT32 nMethod = 0,
            IN CONST IPAddress &objLocalIP = IPAddress::NONE) = 0;

    /**
     * @brief Destroys the TCP client socket which matches with the specified
     *        destination IP address and port.
     *
     * It only destroys the UE-initiated TCP socket.\n
     * The source IP address can be a NONE address and source port can be a zero.\n
     * If the source IP address is NONE, it does not compare the local socket information
     * (IP & port).\n
     * If the source port is zero, it looks the IP matched socket object and destroys it.\n
     * If the source port is non-zero, it looks the IP & port matched socket object
     * and destroys it.
     *
     * @param objSrcIP Source IP address
     * @param objSrcPort Source port number
     * @param objDestIP Destination IP address
     * @param objDestPort Destination port number
     * @param bIsConnectionByPeer Flag to indicate which endpoint the TCP connection is requested
     */
    virtual void DestroyTCPSocket(IN CONST IPAddress &objSrcIP, IN IMS_UINT32 nSrcPort,
            IN CONST IPAddress &objDestIP, IN IMS_UINT32 nDestPort,
            IN IMS_BOOL bIsConnectionByPeer = IMS_FALSE) = 0;

    /**
     * @brief Sets IP QoS for all the existing sockets which are assigned to the same IP address.
     *
     * @param pIPQoS IP QoS (IP, value)
     */
    virtual void SetIPQoS(IN SIPRTConfig::IPQoS *pIPQoS) = 0;

    /**
     * @brief Sets the keep-alive policy of the TCP client socket which matches
     *        with the specified destination IP address and port.
     *
     * It only destroys the UE-initiated TCP socket.\n
     * The source IP address can be a NONE address and source port can be a zero.\n
     * If the source IP address is NONE, it does not compare the local socket information
     * (IP & port).\n
     * If the source port is zero, it looks the IP matched socket object and destroys it.\n
     * If the source port is non-zero, it looks the IP & port matched socket object
     * and destroys it.
     *
     * @param objSrcIP Source IP address
     * @param objSrcPort Source port number
     * @param objDestIP Destination IP address
     * @param objDestPort Destination port number
     * @param nPolicy Policy for keep-alive timer
     *                - 0 : Transaction-based
     *                - (-2) : Permanent
     *                - (-1) : Inactivity timer by configuration
     *                - ( > 0) : Inactivity timer by this value
     */
    virtual void SetKeepAlivePolicy(IN CONST IPAddress &objSrcIP, IN IMS_UINT32 nSrcPort,
            IN CONST IPAddress &objDestIP, IN IMS_UINT32 nDestPort,
            IN IMS_SINT32 nPolicy = (-1) /* default */) = 0;

    /**
     * @brief Sets the listener to resolve the host name in the local DNS table
     *        before querying the host name to DNS server.
     *
     * @param piListener Listener to be set
     * @note LOCAL_DNS_QUERY
     */
    virtual void SetLocalDnsQueryListener(IN ISIPLocalDnsQueryListener *piListener) = 0;
};

#endif // _INTERFACE_SIP_TRANSPORT_HELPER_H_
