/*
    Author
    <table>
    date      author                    description
    --------  --------------            ----------
    20090302  toastops@                 Created
    </table>

    Description
     This class defines a SIP transport error listener interface.
*/

#ifndef _INTERFACE_SIP_TRANSPORT_ERROR_LISTENER_H_
#define _INTERFACE_SIP_TRANSPORT_ERROR_LISTENER_H_

class AString;

/*
SIP transport error listener interface

Example

See Also
*/
class ISIPTransportErrorListener
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
    virtual void TransportError_NotifyError(IN IMS_SINT32 nCode, IN CONST AString& strMessage) = 0;
};

#endif  // _INTERFACE_SIP_TRANSPORT_ERROR_LISTENER_H_
