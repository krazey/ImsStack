/*
    Author
    <table>
    date      author                    description
    --------  --------------            ----------
    20190625  hwangoo.park@             Created
    </table>

    Description

*/
#ifndef _INTERFACE_IMS_NET_PING_LISTENER_H_
#define _INTERFACE_IMS_NET_PING_LISTENER_H_

#include "ImsTypeDef.h"

class INetworkPing;

class INetworkPingListener
{
public:
    /*
     Notify the application that the specified data connection is alive or not.
    Application can query the result using INetworkPing interface.

    Parameters
    <table>
    parameter               description
    ----------              ----------
    piNetPing               Ping interface to check connection aliveness
    nResult                 INetworkPing::PING_STATUS_OK
                            INetworkPing::PING_STATUS_DEAD_PEER
                            INetworkPing::PING_STATUS_TIMEDOUT
    </table>

    Returns
    <table>
    return                  description
    ----------              ----------
    </table>

    */
    virtual void NetworkPing_NotifyResult(IN INetworkPing* piPing, IN IMS_SINT32 nResult) = 0;
};

#endif // _INTERFACE_IMS_NET_PING_LISTENER_H_
