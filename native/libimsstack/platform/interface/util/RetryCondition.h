/*
    Author
    <table>
    date      author                    description
    --------  --------------            ----------
    20100722  hwangoo.park@             Created
    </table>

    Description

*/

#ifndef _RETRY_CONDITION_H_
#define _RETRY_CONDITION_H_

#include "ImsList.h"

class RetryCode;

class RetryCondition
{
public:
    RetryCondition();
    ~RetryCondition();

public:
    IMS_BOOL Add(IN IMS_SINT32 nCode);
    IMS_BOOL Add(IN IMS_SINT32 nMinCode, IN IMS_SINT32 nMaxCode);

private:
    IMS_BOOL Verify(IN IMS_SINT32 nCode) const;

private:
    friend class RetryTaskHelper;

    IMSList<RetryCode*> objRetryCodes;
};

#endif  // _RETRY_CONDITION_H_
