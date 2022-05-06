
/*
    Author
    <table>
    date      author                    description
    --------  --------------            ----------
    20161017  hwangoo.park@             Created
    </table>

    Description

*/

#ifndef _INTERFACE_FRAMEWORK_THREAD_LISTENER_H_
#define _INTERFACE_FRAMEWORK_THREAD_LISTENER_H_

class IFrameworkThreadListener
{
public:
    /*
     Notifies the application for a start event of IMS Framework thread.

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
    virtual void FrameworkThread_OnStarted() = 0;

    /*
     Notifies the application for a termination event of IMS Framework thread.

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
    virtual void FrameworkThread_OnTerminated() = 0;
};

#endif  // _INTERFACE_FRAMEWORK_THREAD_LISTENER_H_
