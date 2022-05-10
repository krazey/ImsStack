#ifndef __ISIPLOGGERUTIL_H__
#define __ISIPLOGGERUTIL_H__

#include "sip_pf_datatypes.h"

class ISipLoggerUtil
{
public:
    ISipLoggerUtil(){};
    virtual ~ISipLoggerUtil(){};

    virtual SIP_VOID DumpLog(SIP_UINT32 nCategory, const SIP_CHAR* pszFile, SIP_UINT16 nLine,
            const SIP_CHAR* pszFormat, ...) = 0;
};

#endif  // __ISIPLOGGERUTIL_H__
