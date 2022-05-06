/*
    Author
    <table>
    date      author                    description
    --------  --------------            ----------
    20100722  hwangoo.park@             Created
    </table>

    Description

*/

#ifndef _RETRY_RANGE_CODE_H_
#define _RETRY_RANGE_CODE_H_

#include "RetryCode.h"

class RetryRangeCode : public RetryCode
{
public:
    inline RetryRangeCode() :
            RetryCode(),
            nMin(0),
            nMax(0)
    {
    }
    inline RetryRangeCode(IN IMS_SINT32 nMin_, IN IMS_SINT32 nMax_) :
            RetryCode(),
            nMin(nMin_),
            nMax(nMax_)
    {
    }
    inline RetryRangeCode(IN CONST RetryRangeCode& objRHS) :
            RetryCode(objRHS),
            nMin(objRHS.nMin),
            nMax(objRHS.nMax)
    {
    }
    inline virtual ~RetryRangeCode() {}

public:
    inline RetryRangeCode& operator=(IN CONST RetryRangeCode& objRHS)
    {
        if (this != &objRHS)
        {
            nMin = objRHS.nMin;
            nMax = objRHS.nMax;
        }

        return (*this);
    }

public:
    inline virtual IMS_BOOL IsIn(IN IMS_SINT32 nCode) const
    {
        return ((nCode > nMin) && (nCode < nMax));
    }

private:
    // Its value is exclusive
    IMS_SINT32 nMin;
    IMS_SINT32 nMax;
};

#endif  // _RETRY_RANGE_CODE_H_
