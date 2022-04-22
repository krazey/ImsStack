/*
    Author
    <table>
    date      author                    description
    --------  --------------            ----------
    20190625  hwangoo.park@             Created
    </table>

    Description

*/
#ifndef _INTERFACE_IMS_NET_PING_H_
#define _INTERFACE_IMS_NET_PING_H_

#include "IPAddress.h"

class INetPingListener;

class INetPing
{
public:
    /*
     Destroy this INetPing object.

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
    virtual void Destroy() = 0;

    /*
     Check the connection status if the destination node is alive or not.

    Parameters
    <table>
    parameter               description
    ----------              ----------
    objSrcIP                Source IP address
    objDstIP                Destination IP address
    nDstPort                Destination port number
    nWaitTime               Waiting time for ping test (milli-seconds)
    </table>

    Returns
    <table>
    return                  description
    ----------              ----------
    IMS_SINT32              PING_STATUS_PENDING
                            PING_STATUS_OK
                            PING_STATUS_NOK
    </table>

    */
    virtual IMS_SINT32 Ping(IN const IPAddress& objSrcIP,
            IN const IPAddress& objDstIP, IN IMS_SINT32 nDstPort,
            IN IMS_SINT32 nWaitTime) = 0;

    /*
     Sets the listener to receive the connection status.

    Parameters
    <table>
    parameter               description
    ----------              ----------
    piListener              Listener to be notified the result of connection status
    </table>

    Returns
    <table>
    return                  description
    ----------              ----------
    </table>

    */
    virtual void SetListener(IN INetPingListener* piListener) = 0;

public:
    // Ping result status
    enum
    {
        // Peer is alive
        PING_STATUS_OK = 0,
        // PING check pending
        PING_STATUS_PENDING = 1,
        // Internal operation failure
        PING_STATUS_NOK = 2,
        // PING check and dead peer detected
        PING_STATUS_DEAD_PEER = 3,
        // PING check and timed out
        PING_STATUS_TIMEDOUT = 4
    };
};

#endif // _INTERFACE_IMS_NET_PING_H_
