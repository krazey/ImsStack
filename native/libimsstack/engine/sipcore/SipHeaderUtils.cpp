/*
    Author
    <table>
    date      author                    description
    --------  --------------            ----------
    20090923  toastops@                 Created
    </table>

    Description

*/

#include "ServiceMemory.h"
#include "ServiceSystemTime.h"
#include "SipHeaderUtils.h"



PUBLIC GLOBAL
IMS_SINT32 SIPHeaderUtil::GenerateRetryAfterSeconds(IN IMS_SINT32 nExtent /* = 0 */)
{
    IMS_SINT32 nMilliSeconds = IMS_SYS_GetTimeInMicroSeconds();

    //---------------------------------------------------------------------------------------------

    if (nExtent <= 0)
        return (nMilliSeconds % 10 + 1);
    else
        return (nMilliSeconds % nExtent + 1);
}
