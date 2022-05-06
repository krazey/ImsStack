/*
    Author
    <table>
    date      author                    description
    --------  --------------            ----------
    20100524  hwangoo.park@             Created
    </table>

    Description

*/

#ifndef _INTERFACE_ON_SERVICE_LISTENER_H_
#define _INTERFACE_ON_SERVICE_LISTENER_H_

class Service;

/*

IOnServiceListener interface

Example

See Also

*/
class IOnServiceListener
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
    virtual void OnService_RegistrationStarted(
            IN Service* pService, IN IMS_SINT32 nStatusCode, IN IMS_SINT32 nReason) = 0;

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
    virtual void OnService_RegistrationUpdated(
            IN Service* pService, IN IMS_SINT32 nStatusCode, IN IMS_SINT32 nReason) = 0;

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
    virtual void OnService_RegistrationRemoved(
            IN Service* pService, IN IMS_SINT32 nStatusCode, IN IMS_SINT32 nReason) = 0;

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
    virtual void OnService_RegistrationTerminated(IN Service* pService, IN IMS_SINT32 nReason) = 0;
};

#endif  // _INTERFACE_ON_SERVICE_LISTENER_H_
