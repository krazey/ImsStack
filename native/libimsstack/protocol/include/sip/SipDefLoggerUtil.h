#ifndef __SIP_DEFLOGGERUTIL_H__
#define __SIP_DEFLOGGERUTIL_H__

#include "sip_pf_datatypes.h"
#include "ISipLoggerUtil.h"

#define MAX_FILENAME_SIZE 256

class SipDefLoggerUtil : public ISipLoggerUtil
{
public:
    SipDefLoggerUtil();
    ~SipDefLoggerUtil();

public:
    void DumpLog(SIP_UINT32 nCategory, const SIP_CHAR* pszFile, SIP_UINT16 nLine,
            const SIP_CHAR* pszFormat, ...);
};

#endif
