/*
    Author
    <table>
    date      author                    description
    --------  --------------            ----------
    20100428  hwangoo.park@             Created
    20121105  hyunho.shin@              Add refresh started, completed api.
    </table>

    Description

*/

#ifndef _INTERFACE_ON_PUBLICATION_LISTENER_H_
#define _INTERFACE_ON_PUBLICATION_LISTENER_H_

class Publication;

/*

This listener type is used to notify the application of the status of requested publications.

Example

See Also
Publication

*/
class IOnPublicationListener
{
public:
    /*

     Notifies the application that the publication request was successfully delivered.

    Parameters
    <table>
    parameter               description
    ----------              ----------
    pPublication            Pointer to Publication object
    </table>

    Returns
    <table>
    return                  description
    ----------              ----------
    </table>

    */
    virtual void OnPublication_Delivered(IN Publication* pPublication) = 0;

    /*

     Notifies the application that the publication request was not successfully delivered.

    Parameters
    <table>
    parameter               description
    ----------              ----------
    pPublication            Pointer to Publication object
    </table>

    Returns
    <table>
    return                  description
    ----------              ----------
    </table>

    */
    virtual void OnPublication_DeliveryFailed(IN Publication* pPublication) = 0;

    /*

     Notifies the application that the publication was terminated.

    Parameters
    <table>
    parameter               description
    ----------              ----------
    pPublication            Pointer to Publication object
    </table>

    Returns
    <table>
    return                  description
    ----------              ----------
    </table>

    */
    virtual void OnPublication_Terminated(IN Publication* pPublication) = 0;

    //// IMS Extenions
    /*
     Notifies the application that the publication was terminated.

    Parameters
    <table>
    parameter               description
    ----------              ----------
    pPublication            Pointer to Publication object
    </table>

    Returns
    <table>
    return                  description
    ----------              ----------
    </table>

    */
    virtual void OnPublication_RefreshStarted(IN Publication* pPublication) = 0;

    /*
     Notifies the application that the publication was terminated.

    Parameters
    <table>
    parameter               description
    ----------              ----------
    pPublication            Pointer to Publication object
    </table>

    Returns
    <table>
    return                  description
    ----------              ----------
    </table>

    */
    virtual void OnPublication_RefreshCompleted(IN Publication* pPublication) = 0;
};

#endif  // _INTERFACE_ON_PUBLICATION_LISTENER_H_
