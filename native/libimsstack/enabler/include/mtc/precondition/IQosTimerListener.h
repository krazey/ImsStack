#ifndef INTERFACE_QOS_TIMER_LISTENER_H_
#define INTERFACE_QOS_TIMER_LISTENER_H_

#include "IMSTypeDef.h"

class QosTimer;

class IQosTimerListener
{
public:
    virtual ~IQosTimerListener(){};

    /**
     * @brief This method is to notify the timer expiration waiting for the QoS connection.
     * @param pQosTimer The QosTimer instance which has the expired timer.
     */
    virtual void OnWaitTimerExpired(IN QosTimer* pQosTimer) = 0;

    /**
     * @brief This method is to notify the timer expiration guarding the QoS inactivation.
     * @param pQosTimer The QosTimer instance which has the expired timer.
     */
    virtual void OnGuardInactiveTimerExpired(IN QosTimer* pQosTimer) = 0;

    /**
     * @brief This method is to notify the timer expiration to enable the QoS by force.
     *        It is called only in Test Mode.
     * @param pQosTimer The QosTimer instance which has the expired timer.
     */
    virtual void OnForceAvailableTimerExpired(IN QosTimer* pQosTimer) = 0;
};
#endif
