/*
    Author
    <table>
    date      author                    description
    --------  --------------            ----------
    20141113  hwangoo.park@             Created
    </table>

    Description

*/

#ifndef _INTERFACE_IMS_SYSTEM_UTIL_H_
#define _INTERFACE_IMS_SYSTEM_UTIL_H_

#include "AString.h"



class ISystemUtil
{
public:
    /*
     Gets Digest result using SHA1 algorithm.

    Parameters
    <table>
    parameter               description
    ----------              ----------
    strIN                   Raw string to be hashed
    strOUT                  Digest result using SHA1 algorithm
    </table>

    Returns
    <table>
    return                  description
    ----------              ----------
    </table>
    */
    virtual void DigestSha1(IN const AString &strIN, OUT AString &strOUT) = 0;

    /*
     Gets the time or random based UUID (version 1).
    The time-based UUID will be prioritized.

    Parameters
    <table>
    parameter               description
    ----------              ----------
    nOption                 Options for UUID generation; It's not used in the moment
    </table>

    Returns
    <table>
    return                  description
    ----------              ----------
    AString                 Time or random based UUID
    </table>
    */
    virtual AString GetUuid(IN IMS_SINT32 nOption = 0) = 0;
};

#endif // _INTERFACE_IMS_SYSTEM_UTIL_H_
