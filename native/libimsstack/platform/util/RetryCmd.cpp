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
#include "ServiceTrace.h"
#include "IRetryCmdListener.h"
#include "RetryCmd.h"

__IMS_TRACE_TAG_BASE__;

PUBLIC
RetryCmd::RetryCmd(IN IMS_UINT32 nIDCmd_ /* = 0 */) :
        nIDCmd(nIDCmd_),
        piListener(IMS_NULL)
{
}

PUBLIC VIRTUAL RetryCmd::~RetryCmd()
{
    IMS_TRACE_D("Destructor :: RetryCmd (%d)", nIDCmd, 0, 0);
}

/*
 Notify the result of this common execution.

Remarks

*/
PROTECTED
void RetryCmd::OnCmdCompleted(IN IMS_SINT32 nResultCode, IN IMS_SINT32 nRetryAfter /* = 0 */)
{
    if (piListener != IMS_NULL)
    {
        piListener->RetryCmd_OnCompleted(this, nResultCode, nRetryAfter);
    }
}
