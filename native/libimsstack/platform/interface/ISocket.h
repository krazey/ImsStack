/*
    Author
    <table>
    date      author                    description
    --------  --------------            ----------
    20090309  toastops@                 Created
    </table>

    Description
     This file defines IMS socket interface.
*/

#ifndef _INTERFACE_IMS_NET_SOCKET_H_
#define _INTERFACE_IMS_NET_SOCKET_H_

#include "IPAddress.h"

class INetSocketListener;



class INetSocket
{
public:
    // Return types of socket operation
    enum SOCKET_RESULT
    {
        RESULT_ERROR = (-1),
        RESULT_WOULDBLOCK = 0,
        RESULT_SUCCESS = 1
    };

    // Type of socket
    enum SOCKET_ENTYPE
    {
        TYPE_STREAM = 1,
        TYPE_DGRAM = 2,
    };

    // Type of address family
    enum ADDRESS_FAMILY_ENTYPE
    {
        ADDRESS_FAMILY_INET = 1,
        ADDRESS_FAMILY_INET6 = 2
    };

    enum { MAX_BACKLOG = 3 };

    // Type of socket option
    enum
    {
        OPT_BASE = 0,

        // GET/SET - QoS of IP level : IP_TOS (IPv4), IP_TCLASS (IPv6)
        OPT_IP_QOS,
        // GET/SET - KeepAlive configuration of TCP level
        // Number of unacknowledged probes (count)
        OPT_TCP_KEEPCNT,
        // Interval between the last data packet sent & the first keepalive probe (seconds)
        OPT_TCP_KEEPIDLE,
        // Interval between subsequential keepalive probes (seconds)
        OPT_TCP_KEEPINTVL,
        // The maximum segment size for outgoing TCP packets
        OPT_TCP_MAXSEG,
        // GET/SET - set buffer size for sending/receiving the data
        OPT_RCVBUF,
        OPT_SNDBUF,
        // SET
        OPT_REUSEADDR,
        // SET
        OPT_LINGER,
        // SET
        OPT_KEEPALIVE,
        // SET
        OPT_UDP_ENCAP,

        // IMS extensions
        // SET - set the shutdown method for TCP connection
        OPT_SHUTDOWN,

        OPT_MAX
    };

    // Parameter for specific socket option
    enum
    {
        // OPT_UDP_ENCAP
        OPT_UDP_ENCAP_ESPINUDP_NON_IKE = 1,
        OPT_UDP_ENCAP_ESPINUDP = 2,
        OPT_UDP_ENCAP_L2TPINUDP = 3
    };

    // Type of close reason
    enum
    {
        CLOSE_REASON_UNKNOWN = 0,
        CLOSE_REASON_USER_ACTION,
        CLOSE_REASON_REMOTE_ACTION,
        CLOSE_REASON_DATA_CONNECTION_LOST,
        CLOSE_REASON_INTERNAL_ERROR
    };

public:
    /**
     * @brief Returns a socket identifier of the lower layer.
     *        If the platform is the Linux, it returns a file descriptor of this socket.
     *        If the socket is not opened or already closed, it returns INVALID_SOCKET.
     *
     * @return a socket identifier or INVALID_SOCKET
     */
    virtual IMS_SINT32 GetSocketId() const = 0;

    /**
     * @brief Returns the socket type of this socket.
     *
     * @return a socket type (SOCKET_ENTYPE)
     */
    virtual SOCKET_ENTYPE GetSocketType() const = 0;

    /*

    Remarks

    Parameters
    <table>
    parameter               description
    ----------              ----------
    </table>

    Returns
    <table>
    return                  description
    ----------              ----------
    </table>
    */
    virtual SOCKET_RESULT Open(IN SOCKET_ENTYPE eType,
            IN INetSocketListener *piListener,
            IN ADDRESS_FAMILY_ENTYPE eAF = ADDRESS_FAMILY_INET) = 0;

    /*

    Remarks

    Parameters
    <table>
    parameter               description
    ----------              ----------
    </table>

    Returns
    <table>
    return                  description
    ----------              ----------
    </table>
    */
    virtual SOCKET_RESULT Open(IN SOCKET_ENTYPE eType,
            IN ADDRESS_FAMILY_ENTYPE eAF = ADDRESS_FAMILY_INET) = 0;

    /*

    Remarks

    Parameters
    <table>
    parameter               description
    ----------              ----------
    </table>

    Returns
    <table>
    return                  description
    ----------              ----------
    </table>
    */
    virtual void SetListener( IN INetSocketListener* piListener ) = 0;
    /*

    Remarks

    Parameters
    <table>
    parameter               description
    ----------              ----------
    </table>

    Returns
    <table>
    return                  description
    ----------              ----------
    </table>
    */
    virtual SOCKET_RESULT Close() = 0;

    /*

    Remarks
        TCP

    Parameters
    <table>
    parameter               description
    ----------              ----------
    </table>

    Returns
    <table>
    return                  description
    ----------              ----------
    </table>
    */
    virtual INetSocket* Accept() = 0;

    /*

    Remarks

    Parameters
    <table>
    parameter               description
    ----------              ----------
    </table>

    Returns
    <table>
    return                  description
    ----------              ----------
    </table>
    */
    virtual SOCKET_RESULT Bind(IN const IPAddress &objSocketAddress,
            IN IMS_UINT32 nSocketPort) = 0;

    /*

    Remarks

    Parameters
    <table>
    parameter               description
    ----------              ----------
    </table>

    Returns
    <table>
    return                  description
    ----------              ----------
    </table>
    */
    virtual SOCKET_RESULT Connect(IN const IPAddress &objHostAddress,
            IN IMS_UINT32 nHostPort) = 0;

    /*

    Remarks

    Parameters
    <table>
    parameter               description
    ----------              ----------
    </table>

    Returns
    <table>
    return                  description
    ----------              ----------
    </table>
    */
    virtual SOCKET_RESULT Listen(IN IMS_SINT32 nBackLog = MAX_BACKLOG) = 0;

    /*

    Remarks

    Parameters
    <table>
    parameter               description
    ----------              ----------
    </table>

    Returns
    <table>
    return                  description
    ----------              ----------
    </table>
    */
    virtual IMS_SINT32 Receive(OUT IMS_BYTE *pBuffer, IN IMS_SINT32 nBuffLen) = 0;

    /*

    Remarks

    Parameters
    <table>
    parameter               description
    ----------              ----------
    </table>

    Returns
    <table>
    return                  description
    ----------              ----------
    IMS_SINT32              -1 error
                            0 would block
                            n num of sent data.
    </table>
    */
    virtual IMS_SINT32 Send(IN const IMS_BYTE *pBuffer, IN IMS_SINT32 nBuffLen) = 0;

    /*

    Remarks

    Parameters
    <table>
    parameter               description
    ----------              ----------
    </table>

    Returns
    <table>
    return                  description
    ----------              ----------
    </table>
    */
    virtual IMS_SINT32 ReceiveFrom(OUT IMS_BYTE *pBuffer, IN IMS_SINT32 nBuffLen,
            OUT IPAddress &objHostAddress, OUT IMS_UINT32 &nHostPort) = 0;

    /*

    Remarks

    Parameters
    <table>
    parameter               description
    ----------              ----------
    </table>

    Returns
    <table>
    return                  description
    ----------              ----------
    </table>
    */
    virtual IMS_SINT32 SendTo(IN const IMS_BYTE *pBuffer, IN IMS_SINT32 nBuffLen,
            IN const IPAddress &objHostAddress, IN IMS_UINT32 nHostPort) = 0;

    /*

    Remarks

    Parameters
    <table>
    parameter               description
    ----------              ----------
    </table>

    Returns
    <table>
    return                  description
    ----------              ----------
    </table>
    */
    virtual SOCKET_RESULT GetPeerName(OUT IPAddress &objPeerAddress,
            OUT IMS_UINT32 &nPeerPort) = 0;

    /*

    Remarks

    Parameters
    <table>
    parameter               description
    ----------              ----------
    </table>

    Returns
    <table>
    return                  description
    ----------              ----------
    </table>
    */
    virtual SOCKET_RESULT GetSockName(OUT IPAddress &objSocketAddress,
            OUT IMS_UINT32 &nSocketPort) = 0;

    /*

    Remarks

    Parameters
    <table>
    parameter               description
    ----------              ----------
    </table>

    Returns
    <table>
    return                  description
    ----------              ----------
    </table>
    */
    virtual IMS_BOOL Equals(IN const INetSocket *piNetSocket) = 0;

    /*

    Remarks

    Parameters
    <table>
    parameter               description
    ----------              ----------
    nOption                 Type of socket option
    </table>

    Returns
    <table>
    return                  description
    ----------              ----------
    </table>
    */
    virtual IMS_SINT32 GetOption(IN IMS_SINT32 nOption) = 0;

    /*

    Remarks

    Parameters
    <table>
    parameter               description
    ----------              ----------
    nOption                 Type of socket option
    nOptionValue            Value to be set
    </table>

    Returns
    <table>
    return                  description
    ----------              ----------
    </table>
    */
    virtual IMS_BOOL SetOption(IN IMS_SINT32 nOption, IN IMS_SINT32 nOptionValue) = 0;
};



class INetSocketListener
{
public:
    /*

    Remarks

    Parameters
    <table>
    parameter               description
    ----------              ----------
    </table>

    Returns
    <table>
    return                  description
    ----------              ----------
    </table>
    */
    virtual void Socket_DataReceived(IN INetSocket *piNetSocket) = 0;

    /*

    Remarks

    Parameters
    <table>
    parameter               description
    ----------              ----------
    </table>

    Returns
    <table>
    return                  description
    ----------              ----------
    </table>
    */
    virtual void Socket_SendEnabled(IN INetSocket *piNetSocket) = 0;

    /*

    Remarks

    Parameters
    <table>
    parameter               description
    ----------              ----------
    </table>

    Returns
    <table>
    return                  description
    ----------              ----------
    </table>
    */
    virtual void Socket_ConnectionReceived(IN INetSocket *piNetSocket) = 0;

    /*

    Remarks

    Parameters
    <table>
    parameter               description
    ----------              ----------
    </table>

    Returns
    <table>
    return                  description
    ----------              ----------
    </table>
    */
    virtual void Socket_Connected(IN INetSocket *piNetSocket) = 0;

    /*

    Remarks

    Parameters
    <table>
    parameter               description
    ----------              ----------
    </table>

    Returns
    <table>
    return                  description
    ----------              ----------
    </table>
    */
    virtual void Socket_Closed(IN INetSocket *piNetSocket,
            IN IMS_SINT32 nReason = INetSocket::CLOSE_REASON_UNKNOWN) = 0;
};

#endif // _INTERFACE_IMS_NET_SOCKET_H_
