/*
    Author
    <table>
    date      author                    description
    --------  --------------            ----------
    20100208  hwangoo.park@             From IThreadListener
    </table>

    Description

*/

#ifndef _INTERFACE_IMS_RUNNABLE_H_
#define _INTERFACE_IMS_RUNNABLE_H_

#include "ImsMessage.h"

class IRunnable
{
public:
    /*

    Parameters
    <table>
    parameter               description
    ----------              ----------
    </table>

    Returns
    <table>
    return                  description
    ----------              ----------
    IMS_BOOL
    </table>

    */
    virtual IMS_BOOL Runnable_Run(IN IMSMSG &objMSG) = 0;
};

#endif // _INTERFACE_IMS_RUNNABLE_H_
