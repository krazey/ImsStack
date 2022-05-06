/*
    Author
    <table>
    date      author                    description
    --------  --------------            ----------
    20100722  hwangoo.park@             Created
    </table>

    Description

*/

#ifndef _RETRY_CODE_H_
#define _RETRY_CODE_H_

#include "IMSTypeDef.h"

class RetryCode
{
public:
    inline RetryCode() {}
    inline RetryCode(IN CONST RetryCode&) {}
    inline virtual ~RetryCode() {}

public:
    inline RetryCode& operator=(IN CONST RetryCode&) { return (*this); }

public:
    virtual IMS_BOOL IsIn(IN IMS_SINT32 nCode) const = 0;
};

#endif  // _RETRY_CODE_H_
