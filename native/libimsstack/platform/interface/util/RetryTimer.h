/*
    Author
    <table>
    date      author                    description
    --------  --------------            ----------
    20100722  hwangoo.park@             Created
    </table>

    Description

*/

#ifndef _RETRY_TIMER_H_
#define _RETRY_TIMER_H_

#include "ImsList.h"
#include "ITimer.h"

class IRetryTimerListener;

class RetryTimer : public ITimerListener
{
public:
    explicit RetryTimer(IN IMS_BOOL bRepeatable_ = IMS_FALSE);
    virtual ~RetryTimer();

public:
    // Add a time interval; in milli-seconds
    IMS_BOOL AddValue(IN IMS_UINT32 nInterval);
    // Adds the set of time intervals; in milli-seconds
    IMS_BOOL AddValues(IN IMSList<IMS_UINT32>& objIntervals);
    // Returns the next time interval; in milli-seconds
    IMS_UINT32 GetNextInterval() const;
    IMS_SINT32 GetState() const;
    // Resume the timer in PENDING state
    IMS_BOOL Resume();
    // Sets a listener when the timer expired
    void SetListener(IN IRetryTimerListener* piListener);
    // Apply the retry rule according to the timer
    IMS_BOOL Start();
    // Terminates the retry operation
    void Terminate();

private:
    // ITimerListener interface
    virtual void Timer_TimerExpired(IN ITimer* piTimer);

    IMS_BOOL StartTimer();

public:
    // Return value for the method of IRetryTimerListener
    enum
    {
        // Apply the timer rule continuously
        RESULT_CONTINUE = 0,
        // Do not apply the timer rule & store the current state
        // The timer will be re-started when receiving the result of the command
        RESULT_PENDING,
        // Do not apply the timer rule & go to the initial state
        RESULT_STOP
    };

    enum
    {
        // It is an initial state & the timer is not started
        STATE_INACTIVE = 0,
        // The timer is pending until the command is completed
        // After receiving the result of command, the timer will be re-started
        STATE_PENDING,
        // The timer is started
        STATE_ACTIVE
    };

private:
    // State of RetryTimer
    IMS_SINT32 nState;

    // Pointer to the timer interface
    ITimer* piTimer;
    // To track the count of retry operation
    IMS_UINT32 nTracker;
    // List of time intervals
    IMSList<IMS_UINT32> objIntervals;

    // Flag to indicate if the repeatable(infinite) timer needs to be started
    // The last interval value will be used for the repeatable(infinite) timer
    IMS_BOOL bFlag_Repeatable;

    IRetryTimerListener* piListener;
};

#endif  // _RETRY_TIMER_H_
