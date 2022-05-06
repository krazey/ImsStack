/*
    Author
    <table>
    date      author                    description
    --------  --------------            ----------
    20100428  hwangoo.park@             Created
    </table>

    Description
*/

#ifndef _INTERFACE_ON_REFERENCE_LISTENER_H_
#define _INTERFACE_ON_REFERENCE_LISTENER_H_

class Message;
class Reference;

/*

This listener type is used to notify an application about events regarding a Reference.

Example

See Also
Reference

*/
class IOnReferenceListener
{
public:
    /*

     Notifies the application that the reference was successfully delivered.

    Remarks

    Parameters
    <table>
    parameter               description
    ----------              ----------
    pReference              Pointer to Reference object
    </table>

    Returns
    <table>
    return                  description
    ----------              ----------
    </table>

    */
    virtual void OnReference_Delivered(IN Reference* pReference) = 0;

    /*

     Notifies the application that the reference was not successfully delivered.

    Remarks

    Parameters
    <table>
    parameter               description
    ----------              ----------
    pReference              Pointer to Reference object
    </table>

    Returns
    <table>
    return                  description
    ----------              ----------

    </table>

    */
    virtual void OnReference_DeliveryFailed(IN Reference* pReference) = 0;

    /*

     Notifies the application with status reports regarding the Reference.

    Remarks

    Parameters
    <table>
    parameter               description
    ----------              ----------
    pReference              Pointer to Reference object
    pNotify                 Pointer to Message object (including NOTIFY request)
    </table>

    Returns
    <table>
    return                  description
    ----------              ----------

    </table>

    */
    virtual void OnReference_NotifyReceived(IN Reference* pReference, IN Message* pNotify) = 0;

    /*

     Notifies the application that a reference has been terminated.

    Remarks

    Parameters
    <table>
    parameter               description
    ----------              ----------
    pReference              Pointer to Reference object
    </table>

    Returns
    <table>
    return                  description
    ----------              ----------

    </table>

    */
    virtual void OnReference_Terminated(IN Reference* pReference) = 0;
};

#endif  // _INTERFACE_ON_REFERENCE_LISTENER_H_
