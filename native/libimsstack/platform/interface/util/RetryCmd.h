/*
    Author
    <table>
    date      author                    description
    --------  --------------            ----------
    20100722  hwangoo.park@             Created
    </table>

    Description

*/

#ifndef _RETRY_CMD_H_
#define _RETRY_CMD_H_

#include "IMSTypeDef.h"

class IRetryCmdListener;

class RetryCmd
{
public:
    explicit RetryCmd(IN IMS_UINT32 nIDCmd_ = 0);
    virtual ~RetryCmd();

public:
    // Executes the command
    virtual IMS_RESULT ExecuteCmd() = 0;

    inline IMS_UINT32 GetIDCmd() const { return nIDCmd; }
    inline void SetCmdListener(IN IRetryCmdListener* piListener) { this->piListener = piListener; }

protected:
    void OnCmdCompleted(IN IMS_SINT32 nResultCode, IN IMS_SINT32 nRetryAfter = 0);

private:
    IMS_UINT32 nIDCmd;
    IRetryCmdListener* piListener;
};

#endif  // _RETRY_CMD_H_
