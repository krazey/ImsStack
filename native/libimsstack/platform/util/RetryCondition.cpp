/*
    Author
    <table>
    date      author                    description
    --------  --------------            ----------
    20100722  hwangoo.park@             Created
    </table>

    Description

*/

#include "ServiceMemory.h"
#include "RetrySingleCode.h"
#include "RetryRangeCode.h"
#include "RetryCondition.h"

PUBLIC
RetryCondition::RetryCondition() {}

PUBLIC
RetryCondition::~RetryCondition() {}

/*
 Adds the single code value to this condition.

Remarks

*/
PUBLIC
IMS_BOOL RetryCondition::Add(IN IMS_SINT32 nCode)
{
    RetrySingleCode* pCode = new RetrySingleCode(nCode);

    if (pCode == IMS_NULL)
    {
        return IMS_FALSE;
    }

    if (!objRetryCodes.Append(pCode))
    {
        delete pCode;
        return IMS_FALSE;
    }

    return IMS_TRUE;
}

/*
 Adds the range code value (x > MIN && x < MAX) to this condition.

Remarks

*/
PUBLIC
IMS_BOOL RetryCondition::Add(IN IMS_SINT32 nMinCode, IN IMS_SINT32 nMaxCode)
{
    RetryRangeCode* pCode = new RetryRangeCode(nMinCode, nMaxCode);

    if (pCode == IMS_NULL)
    {
        return IMS_FALSE;
    }

    if (!objRetryCodes.Append(pCode))
    {
        delete pCode;
        return IMS_FALSE;
    }

    return IMS_TRUE;
}

/*
 Verify the condition value if the condition meets or not.

Remarks

*/
PRIVATE
IMS_BOOL RetryCondition::Verify(IN IMS_SINT32 nCode) const
{
    for (IMS_UINT32 i = 0; i < objRetryCodes.GetSize(); ++i)
    {
        RetryCode* pCode = objRetryCodes.GetAt(i);

        if (pCode->IsIn(nCode))
        {
            return IMS_TRUE;
        }
    }

    return IMS_FALSE;
}
