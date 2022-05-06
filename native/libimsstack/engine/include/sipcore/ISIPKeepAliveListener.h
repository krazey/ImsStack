/*
    Author
    <table>
    date      author                    description
    --------  --------------            ----------
    20121030  hwangoo.park@             Created
    </table>

    Description
     This class defines a SIP keep-alive listener interface.
*/

#ifndef _INTERFACE_SIP_KEEP_ALIVE_LISTENER_H_
#define _INTERFACE_SIP_KEEP_ALIVE_LISTENER_H_

/*
SIP keep-alive listener interface

Example

See Also

*/
class ISIPKeepAliveListener
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
    virtual void KeepAlive_PongReceived() = 0;
};

#endif  // _INTERFACE_SIP_KEEP_ALIVE_LISTENER_H_
