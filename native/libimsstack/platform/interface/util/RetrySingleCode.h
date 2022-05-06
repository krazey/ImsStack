/*
    Author
    <table>
    date      author                    description
    --------  --------------            ----------
    20100722  hwangoo.park@             Created
    </table>

    Description

*/

#ifndef _RETRY_SINGLE_CODE_H_
#define _RETRY_SINGLE_CODE_H_

#include "RetryCode.h"

class RetrySingleCode : public RetryCode
{
public:
    inline RetrySingleCode() :
            RetryCode(),
            nCode(0)
    {
    }
    inline explicit RetrySingleCode(IN IMS_SINT32 nCode_) :
            RetryCode(),
            nCode(nCode_)
    {
    }
    inline RetrySingleCode(IN CONST RetrySingleCode& objRHS) :
            RetryCode(objRHS),
            nCode(objRHS.nCode)
    {
    }
    inline virtual ~RetrySingleCode() {}

public:
    inline RetrySingleCode& operator=(IN CONST RetrySingleCode& objRHS)
    {
        if (this != &objRHS)
        {
            nCode = objRHS.nCode;
        }

        return (*this);
    }

public:
    inline virtual IMS_BOOL IsIn(IN IMS_SINT32 nCode) const { return (this->nCode == nCode); }

private:
    IMS_SINT32 nCode;
};

#endif  // _RETRY_SINGLE_CODE_H_
