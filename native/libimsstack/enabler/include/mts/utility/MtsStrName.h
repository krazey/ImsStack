#ifndef MTS_STR_NAME_H_
#define MTS_STR_NAME_H_

#include "AString.h"

class MtsStrName final
{
public:
    MtsStrName() {}
    ~MtsStrName() {}

    AString GetMtsAppId();
    AString GetMtsServiceId();
    AString GetMtsConnectorName();
};

#endif
