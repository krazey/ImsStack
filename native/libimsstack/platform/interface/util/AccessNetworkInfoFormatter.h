/*
    Author
    <table>
    date      author                    description
    --------  --------------            ----------
    20151001  hwangoo.park@             Created
    </table>

    Description

*/

#ifndef _ACCESS_NETWORK_INFO_FORMATTER_H_
#define _ACCESS_NETWORK_INFO_FORMATTER_H_

#include "ImsAccessNetworkInfoType.h"

class AccessNetworkInfoFormatter
{
private:
    AccessNetworkInfoFormatter();

public:
    static IMS_BOOL Encode(IN CONST AccessNetworkInfo &objANInfo,
            OUT AString &strHeader, IN CONST AString &strCellInfo = AString::ConstNull());
};

#endif // _ACCESS_NETWORK_INFO_FORMATTER_H_
