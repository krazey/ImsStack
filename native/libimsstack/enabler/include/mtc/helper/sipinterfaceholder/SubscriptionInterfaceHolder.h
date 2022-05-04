#ifndef SUBSCRIPTION_INTERFACE_HOLDER_H_
#define SUBSCRIPTION_INTERFACE_HOLDER_H_

#include "IMSList.h"
#include "ServiceTimer.h"
#include "IMSMap.h"
#include "ISubscriptionListener.h"

class ISession;
class ICoreService;
class ISubscription;
class IInterfaceHolderListener;

class SubscriptionInterfaceHolder final : public ISubscriptionListener, public ITimerListener
{
public:
    explicit SubscriptionInterfaceHolder(IN IInterfaceHolderListener& objListener);
    ~SubscriptionInterfaceHolder();
    SubscriptionInterfaceHolder(IN const SubscriptionInterfaceHolder&) = delete;
    SubscriptionInterfaceHolder& operator=(IN const SubscriptionInterfaceHolder&) = delete;

public:
    // ISubscriptionListener interface implementation
    inline void SubscriptionForkedNotify(IN ISubscription*, IN ISubscription*) override {}
    inline void SubscriptionNotify(IN ISubscription*, IN IMessage*) override {}
    inline void SubscriptionStarted(IN ISubscription*) override {}
    inline void SubscriptionStartFailed(IN ISubscription*) override {}
    void SubscriptionTerminated(IN ISubscription* piSubscription) override;

    // ITimerListener interfaces implementation.
    void Timer_TimerExpired(IN ITimer* piTimer) override;

    ISubscription* GetISubscription(IN ISession* piSession, IN const AString& strEvent);
    ISubscription* GetISubscription(IN ICoreService* piCoreService, IN const AString& strFrom,
            IN const AString& strTo, IN const AString& strEvent);

    void ReleaseISubscription(
            IN ISubscription* piSubscription, IN IMS_BOOL bTerminated = IMS_FALSE);

private:
    IMS_BOOL IsReadyToDestroy(IN ISubscription* piSubscription);

    void ClearISubscriptions();

    IMS_RESULT StartTimer(IN ISubscription* piSubscription, IN IMS_SINT32 nDuration);
    void StopTimer(IN ITimer* piTimer);

    ITimer* GetTimer(IN ISubscription* piSubscription);

private:
    IInterfaceHolderListener& m_objListener;
    IMSList<ISubscription*> m_objISubscriptions;
    IMSMap<ITimer*, ISubscription*> m_objSubscriptionTerminatedGuardTimers;

    static const IMS_UINT32 TIME_TRANSACTION_TERMINATED_GUARD = 32000;
};

#endif
