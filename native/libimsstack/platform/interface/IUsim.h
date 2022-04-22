
/*
    Author
    <table>
    date      author                    description
    --------  --------------            ----------
    20121120  hwangoo.park@             Created
    </table>

    Description

*/

#ifndef _INTERFACE_IMS_USIM_H_
#define _INTERFACE_IMS_USIM_H_

#include "ImsTypeDef.h"

class IDigestAKA;

class IUSIM
{
public:
    /*

    Creates the Digest AKA.

    Parameters
    <table>
    parameter               description
    ----------              ----------
    </table>

    Returns
    <table>
    return                  description
    ----------              ----------
    IDigestAKA*             Pointer to IDigestAKA
    </table>

    */
    virtual IDigestAKA* CreateDigestAKA() = 0;
};

#endif // _INTERFACE_IMS_USIM_H_
