/*
    Author
    <table>
    date      author                    description
    --------  --------------            ----------
    20101010  hwangoo.park@             Created
    </table>

    Description

*/

#ifndef _IMS_PROVISIONING_H_
#define _IMS_PROVISIONING_H_

#include "ImsTypeDef.h"

class ImsProvisioning
{
private:
    ImsProvisioning();

public:
    static IMS_BOOL Initialize();
};

#endif // _IMS_PROVISIONING_H_
