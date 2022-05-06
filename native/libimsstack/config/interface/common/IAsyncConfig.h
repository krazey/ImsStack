/*
    Author
    <table>
    date      author                    description
    --------  --------------            ----------
    20101022  hwangoo.park@             Created
    </table>

    Description

*/

#ifndef _INTERFACE_ASYNC_CONFIG_H_
#define _INTERFACE_ASYNC_CONFIG_H_

class IAsyncConfig
{
public:
    /*
     Dispatches the asynchronous message for the configuration.

    Parameters
    <table>
    parameter               description
    ----------              ----------
    nMSG                    Message type to be handled
    nParam1                 Item to be handled
    nParam2                 Value of the specified item (action)
    </table>

    Returns
    <table>
    return                  description
    ----------              ----------
    </table>
    */
    virtual void HandleMessage(IN IMS_SINT32 nMSG, IN IMS_SINTP nParam1, IN IMS_SINTP nParam2) = 0;

public:
    enum
    {
        // Initialization operation
        // IAsyncConfig will register itself to the AsynConfigHelper
        // Param1 : AsyncConfigHelper*
        ACMSG_START = 1,

        // Base of user-defined Async Config Message
        ACMSG_USER = 10
    };
};

#endif  // _INTERFACE_ASYNC_CONFIG_H_
