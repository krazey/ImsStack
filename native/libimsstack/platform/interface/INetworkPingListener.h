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

class INetPing;

class INetPingListener
{
public:
    /*
     Notify the application that the specified data connection is alive or not.
    Application can query the result using INetPing interface.

    Parameters
    <table>
    parameter               description
    ----------              ----------
    piNetPing               Ping interface to check connection aliveness
    nResult                 INetPing::PING_STATUS_OK
                            INetPing::PING_STATUS_DEAD_PEER
                            INetPing::PING_STATUS_TIMEDOUT
    </table>

    Returns
    <table>
    return                  description
    ----------              ----------
    </table>

    */
    virtual void NetPing_NotifyResult(IN INetPing* piNetPing, IN IMS_SINT32 nResult) = 0;
};

#endif // _INTERFACE_IMS_NET_PING_LISTENER_H_
