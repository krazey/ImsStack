/*
    Author
    <table>
    date      author                    description
    --------  --------------            ----------
    20100726  hwangoo.park@             Created
    </table>

    Description

*/

#ifndef _RETRY_TASK_HELPER_H_
#define _RETRY_TASK_HELPER_H_

#include "IRetryCmdListener.h"
#include "IRetryTimerListener.h"
#include "RetryCmd.h"
#include "RetryCondition.h"
#include "RetryTimer.h"

class IRetryTaskHelperListener;

class RetryTaskHelper : public IRetryCmdListener, public IRetryTimerListener
{
public:
    explicit RetryTaskHelper(IN IMS_BOOL bTimerOnCmdCompleted = IMS_FALSE);
    virtual ~RetryTaskHelper();

public:
    IMS_SINT32 GetState() const;
    RetryCmd* SetCommand(IN RetryCmd* pCmd);
    RetryCondition* SetCondition(IN RetryCondition* pCondition);
    RetryTimer* SetTimer(IN RetryTimer* pTimer);

    void SetListener(IN IRetryTaskHelperListener* piListener);
    IMS_BOOL Start(IN IMS_SINT32 nParam = START_COMMAND);
    void Terminate();

protected:
    // IRetryCmdListener class
    virtual void RetryCmd_OnCompleted(
            IN RetryCmd* pCmd, IN IMS_SINT32 nResultCode, IN IMS_SINT32 nRetryAfter = 0);

    // IRetryTimerListener class
    virtual IMS_SINT32 RetryTimer_OnInterimExpired(IN RetryTimer* pTimer);
    virtual void RetryTimer_OnFinalExpired(IN RetryTimer* pTimer);

private:
    void CallListener(IN IMS_SINT32 nResultCode);

public:
    // State of retry task
    enum
    {
        STATE_INACTIVE = 0,
        STATE_ACTIVE
    };

    // Codes of result
    enum
    {
        RESULT_OK = 0,
        RESULT_NOK_TIMER_EXPIRED = (-1),
        RESULT_NOK_INTERNAL_OPERATION = (-2),
        RESULT_NOK_COMMAND_FAILED = (-3)
    };

    // Start parameter of retry task
    enum
    {
        START_COMMAND = 0,
        START_COMMAND_N_TIMER,
        START_TIMER,
        START_TIMER_N_COMMAND,
    };

private:
    IMS_SINT32 nState;
    // If it is TRUE, the timer will be started after completing the command
    // If it is FALSE, the timer will be started after executing the command
    IMS_BOOL bFlag_TimerOnCmdCompleted;
    RetryCmd* pCmd;
    RetryCondition* pCondition;
    RetryTimer* pTimer;
    IRetryTaskHelperListener* piListener;
};

#endif  // _RETRY_TASK_HELPER_H_
