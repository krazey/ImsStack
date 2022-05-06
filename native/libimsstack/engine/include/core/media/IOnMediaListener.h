/*
    Author
    <table>
    date      author                    description
    --------  --------------            ----------
    20100513  hwangoo.park@             Created
    </table>

    Description
     A listener type for receiving notification of when some mode property has changed
    on the media flow.
*/

#ifndef _INTERFACE_ON_MEDIA_LISTENER_H_
#define _INTERFACE_ON_MEDIA_LISTENER_H_

class Media;

class IOnMediaListener
{
public:
    /*
     The method is called when the fictitious media is created.
    The fictitious media that is only meant to track changes that are about to be made
    to the media.

    NOTE:
     After the Session has been accepted or rejected, this proposal media should be considered
    discarded.

    Parameters
    <table>
    parameter               description
    ----------              ----------
    pMedia                  Concerned Media object
    </table>

    Returns
    <table>
    return                  description
    ----------              ----------
    </table>

    */
    virtual void OnMedia_FictitiousMediaCreated(IN Media* pMedia) = 0;

    /*
     The method is called when the fictitious media is destroyed.
    The fictitious media that is only meant to track changes that are about to be made
    to the media.

    NOTE:
     After the Session has been accepted or rejected, this proposal media should be considered
    discarded.

    Parameters
    <table>
    parameter               description
    ----------              ----------
    pMedia                  Concerned Media object
    </table>

    Returns
    <table>
    return                  description
    ----------              ----------
    </table>

    */
    virtual void OnMedia_FictitiousMediaDestroyed(IN Media* pMedia) = 0;
};

#endif  // _INTERFACE_ON_MEDIA_LISTENER_H_
