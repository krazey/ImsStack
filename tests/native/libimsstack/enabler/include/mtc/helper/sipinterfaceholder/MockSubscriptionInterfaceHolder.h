#ifndef MOCK_SUBSCRIPTION_INTERFACE_HOLDER_H_
#define MOCK_SUBSCRIPTION_INTERFACE_HOLDER_H_

#include <gmock/gmock.h>

#include "SubscriptionInterfaceHolder.h"
class ISession;
class ICoreService;
class ISubscription;
class IInterfaceHolderListener;

class MockSubscriptionInterfaceHolder : public SubscriptionInterfaceHolder
{
public:
    explicit MockSubscriptionInterfaceHolder(IN IInterfaceHolderListener& objListener) :
            SubscriptionInterfaceHolder(objListener)
    {
    }
    ~MockSubscriptionInterfaceHolder() {}
    MOCK_METHOD(void, SubscriptionForkedNotify, (IN ISubscription*, IN ISubscription*), (override));
    MOCK_METHOD(void, SubscriptionNotify, (IN ISubscription*, IN IMessage*), (override));
    MOCK_METHOD(void, SubscriptionStarted, (IN ISubscription*), (override));
    MOCK_METHOD(void, SubscriptionStartFailed, (IN ISubscription*), (override));
    MOCK_METHOD(void, SubscriptionTerminated, (IN ISubscription * piSubscription), (override));
    MOCK_METHOD(void, Timer_TimerExpired, (IN ITimer * piTimer), (override));
    MOCK_METHOD(ISubscription*, GetISubscription,
            (IN ISession * piSession, IN const AString& strEvent), (override));
    MOCK_METHOD(ISubscription*, GetISubscription,
            (IN ICoreService * piCoreService, IN const AString& strFrom, IN const AString& strTo,
                    IN const AString& strEvent),
            (override));
    MOCK_METHOD(void, ReleaseISubscription,
            (IN ISubscription * piSubscription, IN IMS_BOOL bTerminated), (override));
    MOCK_METHOD(IMS_BOOL, IsReadyToDestroy, (IN ISubscription * piSubscription), ());
    MOCK_METHOD(void, ClearISubscriptions, (), ());
    MOCK_METHOD(IMS_RESULT, StartTimer,
            (IN ISubscription * piSubscription, IN IMS_SINT32 nDuration), ());
    MOCK_METHOD(void, StopTimer, (IN ITimer * piTimer), ());
    MOCK_METHOD(ITimer*, GetTimer, (IN ISubscription * piSubscription), ());
};

#endif
